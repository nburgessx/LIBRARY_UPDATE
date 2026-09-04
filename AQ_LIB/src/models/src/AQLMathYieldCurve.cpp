/*!  
	@file	
    @brief Source code for class to represent YieldCurve.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(AQLDataString)<BR>
			2.CALIBRATION_DATA_INTERPOLATION(AQLPriceDataInterpolation)<BR>
			3.IR_CALIBRATION_DATA_DAYCOUNT(AQLPriceDataDayCount)<BR>
			4.CALIBRATION_DATA_CALENDAR(AQLPriceDataCalendar)<BR>
			5.CALIBRATION_DATA_SLIDINGRULE(AQLPriceDataSlidingRule)<BR>
			6.IR_CALIBRATION_DATA_FREQUENCY(AQLDataString)<BR>
			7.IR_CALIBRATION_DATA_YIELDDATA(AQLDataReference)<BR>
*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//#define SIMPLE		"SIMPLE"
//#define ANNUAL		"ANNUAL"
//#define SEMI_ANNUAL	"SEMI-ANNUAL"
//#define QUARTERLY	"QUARTERLY"
//#define MONTHLY		"MONTHLY"
//#define CONTINUOUS	"CONTINUOUS"
//#define LUNAR		"LUNAR"
//
//#define	EPS		1.0e-5	// rate eps
//#define STD     "STD"

#include "AQLMathYieldCurve.h"
#include "AQLMathDefine.h"
#include "AQLBasic.h"
#include "AQLPriceDataConvention.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataManager.h"
#include "AQLDataProcedure.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLDataMultiReference.h"
#include "AQLDataMatrix.h"
#include "AQLDataInstance.h"
#include "AQLFunctionUtilities.h"
#include "AQLMathDateCalculations.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathDateUtilities.h"
#include "AQLAlgorithm.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include "AQLFindRootBrent.h"

#include <cmath>
#include <algorithm>
#include <sstream>


namespace 
{
	template<typename F>
	class PortableMMFunc final : public AQLFunctionBase
	{
	public:
		template<typename G>
		explicit PortableMMFunc(G&& f) : f_(std::forward<G>(f))
		{}

		virtual ~PortableMMFunc() {};

		virtual AQLCoreFunctionBase*	clone() const override
		{
			return new PortableMMFunc(f_);
		}

		virtual double operator()(const DoubleArray& x) const override
		{
			return f_(x[0]);
		}

	private:
		F f_;
	};

	template <typename F>
	PortableMMFunc<typename std::decay<F>::type> make_LAFunction(F&& u)
	{
		return PortableMMFunc<typename std::decay<F>::type>(std::forward<F>(u));
	}

	template <typename F>
	double findRootBrent(F&& f, double lowerBound, double upperBound)
	{
		const auto pv = make_LAFunction(std::forward<F>(f));
		const std::vector<std::pair<double, double>> x = { { lowerBound, upperBound } };
		DoubleArray out;

		AQLFindRootBrent().findRoot(pv, x, out);

		return out[0];
	}

	// Find the index for the next cashflow, so that we can ignore cashflows in the past
	size_t getNextCashFlowIndex(const DateVector& floatAccrualDates, const AQLDate& asOf)
	{
        unsigned int nextCashflowIndex = 1;
        for (unsigned int i = 0; i < floatAccrualDates.size(); ++i)
        {
            if ( i == floatAccrualDates.size()-1  )
            {
                if ( floatAccrualDates[i] < asOf ) 
                    throw AQLCoreInvalidData("#Error: Par rate error; The underlying swap has expired.", __FILE__, __LINE__ );

                nextCashflowIndex++;
                break; 
            }
            if ( floatAccrualDates[i+1] >= asOf ) break;
            nextCashflowIndex++;
        }
		return nextCashflowIndex;
	}

	// Porduce a compounding rate for each of the accrual periods
	void getCompoundingRates(DoubleArray& equivalentRates, 
							const DateVector& floatAccrualDates, 
							const DoubleVector& accrualDateYearFractions, 
							size_t nextCashflowIndex,
							AQLDataInstance* dataInstance,
							double floatSpread,
							const AQLDate& asOf,
							const AQLString& curveID,
							const AQLString& foreCurveName,							
							const AQLString& slidingRule,
							const AQLString& calendar,
							const AQLString& rollConvention,
							const AQLString& dayCount,
							const AQLString& interpolation,
							const AQLString& oisCompoundingType)
	{
		if (floatAccrualDates[0] < asOf)
		{
			throw AQLCoreInvalidData("#Error: Only Spot or Forward Starting OIS Swaps supported", __FILE__, __LINE__ );
		}

		// Get equivalent rates over accrual periods
		for( size_t i = 1; i < floatAccrualDates.size(); ++i )
		{
			if (i < nextCashflowIndex)
			{
				equivalentRates[i-1] = 0;
			}
			else
			{
				// Get the equivalent rate of each **non-past** accrual period. 
				// The equivalent rate is obtained either through daily compounding over this period or finding the arithmetic average.
				double equivalentRate = AQLMathCurveFuncUtility::compound(dataInstance,
																curveID,
																foreCurveName,				// The forecast curve given by user is expected to be an OIS curve
																floatAccrualDates[i-1],
																floatAccrualDates[i],
																floatSpread,
																"Business_Days",			//compoundingFrequency,
																false,						// is_start_roll
																slidingRule,
																calendar,
																rollConvention,
																dayCount,
																interpolation,
																oisCompoundingType);

				// Rate must be annualised
				equivalentRates[i-1] = equivalentRate / accrualDateYearFractions[i-1];
			}
		}
	}

}

//#include <afx.h>
using namespace std;

//====================================================================
// YIELD CURVE
/*!
    @brief constructor

	@param[in] dataInstance pointer of AQLDataInstance

*/
AQLMathYieldCurve::AQLMathYieldCurve(AQLDataInstance* dataInstance, const AQLString *const pCurveType) : 
AQLObject(), mCurveSuffix(""), mCurveType(STD), mpBasisCurveType(NULL)
{
	setDataInstance(dataInstance);
	AQLPriceDataManager& dm = dataInstance->getDataMaster();

	dm.setData(CALIBRATION_DATA_NAME,			DATA_STRING			);
	dm.setData(CALIBRATION_DATA_INTERPOLATION,	DATA_INTERPOLATION	);
	dm.setData(IR_CALIBRATION_DATA_DAYCOUNT,		DATA_DAYCOUNT		);
	dm.setData(CALIBRATION_DATA_CALENDAR,		DATA_CALENDAR		);
	dm.setData(CALIBRATION_DATA_SLIDINGRULE,		DATA_SLIDINGRULE	);
	dm.setData(IR_CALIBRATION_DATA_FREQUENCY,	DATA_STRING			);
	dm.setData(IR_CALIBRATION_DATA_YIELDDATA,	DATA_REFERENCE		);
	dm.setData(CALIBRATION_DATA_TERMS,			DATA_DOUBLES		);
    dm.setData(IR_CALIBRATION_DATA_FORWARDRATES,	DATA_DOUBLES		);
    dm.setData(IR_CALIBRATION_DATA_DFS,			DATA_DOUBLES		);
	dm.setData(IR_CALIBRATION_DATA_DFS2,			DATA_DOUBLES		);
	dm.setData(IR_CALIBRATION_DATA_DFSVALUETOINTERPOLATE, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_DFS2VALUETOINTERPOLATE, DATA_STRING);

	
	mpName		 = &add(CALIBRATION_DATA_NAME);
	mpInter		 = &add(CALIBRATION_DATA_INTERPOLATION);
	mpDayCount	 = &add(IR_CALIBRATION_DATA_DAYCOUNT);		
	mpCalendar	 = &add(CALIBRATION_DATA_CALENDAR);
	mpSlidingRule= &add(CALIBRATION_DATA_SLIDINGRULE);
	mpFreq		 = &add(IR_CALIBRATION_DATA_FREQUENCY);	
	mpYieldData  = &add(IR_CALIBRATION_DATA_YIELDDATA);
	mpValueToInterp = &add(IR_CALIBRATION_DATA_DFSVALUETOINTERPOLATE);
	mpValueToInterp2 = &add(IR_CALIBRATION_DATA_DFS2VALUETOINTERPOLATE);
	dynamic_cast<AQLDataString &>(mpValueToInterp->get()).set("DiscountFactor");
	dynamic_cast<AQLDataString &>(mpValueToInterp2->get()).set("DiscountFactor");
	if (pCurveType)
	{
		if (*pCurveType != STD)
		{
			mCurveSuffix = "_" + *pCurveType;
			mCurveType = *pCurveType;
		}
	}

}
/*!
    @brief copy constructor

	@param[in] curve original object
*/
AQLMathYieldCurve::AQLMathYieldCurve(
	const AQLMathYieldCurve& curve) : 
AQLObject(curve), mCurveSuffix(curve.mCurveSuffix), mCurveType(curve.mCurveType)
{
	mpName		 = &getData(CALIBRATION_DATA_NAME);
	mpInter		 = &getData(CALIBRATION_DATA_INTERPOLATION);
	mpDayCount	 = &getData(IR_CALIBRATION_DATA_DAYCOUNT);		
	mpCalendar	 = &getData(CALIBRATION_DATA_CALENDAR);
	mpSlidingRule= &getData(CALIBRATION_DATA_SLIDINGRULE);
	mpFreq		 = &getData(IR_CALIBRATION_DATA_FREQUENCY);	
	mpYieldData  = &getData(IR_CALIBRATION_DATA_YIELDDATA);
	mpValueToInterp = &getData(IR_CALIBRATION_DATA_DFSVALUETOINTERPOLATE);
	mpValueToInterp2 = &getData(IR_CALIBRATION_DATA_DFS2VALUETOINTERPOLATE);

	mpValueToInterp->convertFromString(curve.getDFInterpolationMethod().get());
	mpValueToInterp2->convertFromString(curve.getDF2InterpolationMethod().get());


	// df inter
	map<AQLString, AQLPriceDataInterpolation *>::const_iterator it = curve.mDFInterMap.begin();
	while (it != curve.mDFInterMap.end())
	{
		AQLPriceDataInterpolation *inter = dynamic_cast<AQLPriceDataInterpolation *>(it->second->clone());
		dynamic_cast<AQLPriceDataType *>(inter)->setHolder(mpInter);
		mDFInterMap.insert(make_pair(it->first, inter));
		mCurveVersionMap[it->first] = 0;
		++it;
	}

	// fwd inter
	it = curve.mFWDInterMap.begin();
	while (it != curve.mFWDInterMap.end())
	{
		AQLPriceDataInterpolation *inter = dynamic_cast<AQLPriceDataInterpolation *>(it->second->clone());
		dynamic_cast<AQLPriceDataType *>(inter)->setHolder(mpInter);
		mFWDInterMap.insert(make_pair(it->first, inter));
		++it;
	}

	// daycount
	map<AQLString, AQLPriceDataDayCount *>::const_iterator it_ = curve.mDayCountMap.begin();
	while (it_ != curve.mDayCountMap.end())
	{
		AQLPriceDataDayCount *dc = dynamic_cast<AQLPriceDataDayCount *>(it_->second->clone());
		dynamic_cast<AQLPriceDataType *>(dc)->setHolder(mpDayCount);
		mDayCountMap.insert(make_pair(it_->first, dc));
		++it_;
	}

	// basis curve type
	if (curve.mpBasisCurveType)	mpBasisCurveType = new AQLString(*curve.mpBasisCurveType);
	else mpBasisCurveType = NULL;

}
/*!
    @brief destructor	
*/
AQLMathYieldCurve::~AQLMathYieldCurve()
{
	delDFInterpolations();
	delFWDInterpolations();
	delDayCounts();
	if (mpBasisCurveType) delete mpBasisCurveType;

}

// QUERY
/*!
    @brief get EntityType
	
	@return EntityType
*/
object_t	
AQLMathYieldCurve::getType(void) const
{
	return ENTITY_IRYIELDCURVE;
}
/*!
    @brief this object have specified object or not
	@param[in] id Object type
	@return true  :this (inheriated) object have specified id object
			false :this (inheriated) object doesn't have specified id object
*/
bool
AQLMathYieldCurve::isTypeOf(object_t id) const
{
	return (id == ENTITY_IRYIELDCURVE ? true : AQLObject::isTypeOf(id));
}

/*!
    @brief get name

	@return name
*/
const AQLDataString&	
AQLMathYieldCurve::getName() const	
{
	return dynamic_cast<const AQLDataString&>(mpName->get());
}
/*!
    @brief get name

	@return name
*/
AQLDataString&	
AQLMathYieldCurve::getName()
{
	return dynamic_cast<AQLDataString&>(mpName->get());
}

/*!
    @brief get master Interpolation
			
	@return Interpolation
*/
const AQLPriceDataInterpolation&
AQLMathYieldCurve::getInterpolation() const	
{
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get());
}

/*!
    @brief get master Interpolation
			
	@return Interpolation
*/
AQLPriceDataInterpolation&
AQLMathYieldCurve::getInterpolation() 
{
	return dynamic_cast<AQLPriceDataInterpolation&>(mpInter->get());
}

/*!
    @brief get Interpolation
	@param[in] cuveType

	@return Interpolation
*/
const AQLPriceDataInterpolation&
AQLMathYieldCurve::getInterpolation(const AQLString &curveType) const	
{
	return getCurveTypeInterpolation(curveType);
}

/*!
    @brief get Interpolation
	@param[in] cuveType
			
	@return Interpolation
*/
AQLPriceDataInterpolation&
AQLMathYieldCurve::getInterpolation(const AQLString &curveType) 
{
	return getCurveTypeInterpolation(curveType);
}

/*!
    @brief get cuve type Interpolation
	@param[in] cuveType
			
	@return Interpolation
*/
AQLPriceDataInterpolation&
AQLMathYieldCurve::getCurveTypeInterpolation(const AQLString &curveType) const 
{
	map<AQLString, AQLPriceDataInterpolation *>::const_iterator it = mDFInterMap.find(curveType);
	if (it != mDFInterMap.end())
	{
		return *it->second;
	}
	else
	{
		addDFInterpolation(curveType);
		return *mDFInterMap[curveType];
	}
}

/*!
    @brief get DayCount Convension
			
	@return DayCount Convension
*/

const AQLPriceDataDayCount&
AQLMathYieldCurve::getDayCount() const	
{
	return dynamic_cast<const AQLPriceDataDayCount&>(mpDayCount->get());
}

/*!
    @brief get DayCount Convension and set DayCount Convension
			
	@return DayCount Convension
*/
AQLPriceDataDayCount&
AQLMathYieldCurve::getDayCount() 
{
	return dynamic_cast<AQLPriceDataDayCount&>(mpDayCount->get());
}

/*!
    @brief get convention from yield curve data object
*/
void 
AQLMathYieldCurve::getCurveConvention(AQLString& freq, 
								   AQLPriceDataCalendar& cal, 
								   AQLPriceDataSlidingRule& sld, 
								   AQLPriceDataDayCount& dc, 
								   AQLString& accessary,
								   const AQLString& curveName)
{
	const AQLObject& yieldData = getYieldData().get().get();

	AQLString suffix = "";
	if (curveName != STD) suffix = "_" + curveName;

	const AQLDataHolder *dh;
	dh = &yieldData.getData(IR_CALIBRATION_DATA_FREQUENCY + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		freq = dynamic_cast<const AQLDataString&> (dh->get()); 
	}
	dh = &yieldData.getData(CALIBRATION_DATA_CALENDAR + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		cal = dynamic_cast<const AQLPriceDataCalendar&> (dh->get());
	}
	dh = &yieldData.getData(CALIBRATION_DATA_SLIDINGRULE + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		sld = dynamic_cast<const AQLPriceDataSlidingRule&>(dh->get());
	}
	dh = &yieldData.getData(IR_CALIBRATION_DATA_DAYCOUNT + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		dc = dynamic_cast<const AQLPriceDataDayCount&>(dh->get());
	}
	dh = &yieldData.getData(IR_CALIBRATION_DATA_ACCESSARY + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		accessary = dynamic_cast<const AQLDataString&>(dh->get());
	}
}

/*!
    @brief get convention from yield curve data object
*/
void 
AQLMathYieldCurve::getCurveConvention(AQLString& freq, 
								   AQLPriceDataCalendar& cal, 
								   AQLPriceDataSlidingRule& sld, 
								   AQLPriceDataDayCount& dc, 
								   AQLString& accessary,
								   const AQLString& curveName) const
{
	const AQLObject& yieldData = getYieldData().get().get();

	AQLString suffix = "";
	if (curveName != STD) suffix = "_" + curveName;

	const AQLDataHolder *dh;
	dh = &yieldData.getData(IR_CALIBRATION_DATA_FREQUENCY + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		freq = dynamic_cast<const AQLDataString&> (dh->get()); 
	}
	dh = &yieldData.getData(CALIBRATION_DATA_CALENDAR + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		cal = dynamic_cast<const AQLPriceDataCalendar&> (dh->get());
	}
	dh = &yieldData.getData(CALIBRATION_DATA_SLIDINGRULE + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		sld = dynamic_cast<const AQLPriceDataSlidingRule&>(dh->get());
	}
	dh = &yieldData.getData(IR_CALIBRATION_DATA_DAYCOUNT + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		dc = dynamic_cast<const AQLPriceDataDayCount&>(dh->get());
	}
	dh = &yieldData.getData(IR_CALIBRATION_DATA_ACCESSARY + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		accessary = dynamic_cast<const AQLDataString&>(dh->get());
	}
}

/*!
    @brief  

	@return 
*/
const AQLDataReference&
AQLMathYieldCurve::getYieldData() const
{
	return dynamic_cast<const AQLDataReference&>(mpYieldData->get());
}
/*!
    @brief 
			
	@return 
*/
AQLDataReference&
AQLMathYieldCurve::getYieldData()
{
	return dynamic_cast<AQLDataReference&>(mpYieldData->get());
}

const AQLPriceDataCalendar&
AQLMathYieldCurve::getCalendar() const
{
	return dynamic_cast<const AQLPriceDataCalendar&>(mpCalendar->get());
}

AQLPriceDataCalendar&
AQLMathYieldCurve::getCalendar()
{
	return dynamic_cast<AQLPriceDataCalendar&>(mpCalendar->get());
}
const AQLPriceDataSlidingRule&
AQLMathYieldCurve::getSlidingRule() const
{
	return dynamic_cast<const AQLPriceDataSlidingRule&>(mpSlidingRule->get());
}
AQLPriceDataSlidingRule&
AQLMathYieldCurve::getSlidingRule()
{
	return dynamic_cast<AQLPriceDataSlidingRule&>(mpSlidingRule->get());
}
const AQLDataString&
AQLMathYieldCurve::getFrequency() const
{
	return dynamic_cast<const AQLDataString&>(mpFreq->get());
}
AQLDataString&
AQLMathYieldCurve::getFrequency()
{
	return dynamic_cast<AQLDataString&>(mpFreq->get());
}

const AQLDataString&
AQLMathYieldCurve::getDFInterpolationMethod() const
{
	return dynamic_cast<const AQLDataString&>(mpValueToInterp->get());
}
AQLDataString&
AQLMathYieldCurve::getDFInterpolationMethod()
{
	return dynamic_cast<AQLDataString&>(mpValueToInterp->get());
}

const AQLDataString&
AQLMathYieldCurve::getDF2InterpolationMethod() const
{
	return dynamic_cast<const AQLDataString&>(mpValueToInterp2->get());
}
AQLDataString&
AQLMathYieldCurve::getDF2InterpolationMethod()
{
	return dynamic_cast<AQLDataString&>(mpValueToInterp2->get());
}
/*!
    @brief get ZeroRate from fromDate to toDate
			
	@param[in] fromDate StartDate
	@param[in] toDate	EndDate
	
	@return ZeroRate

double
AQLMathYieldCurve::getZeroRate(const AQLDate& fromDate, const AQLDate& toDate) const
{
	RateConvention rc = setRC(getFrequency().get());
	
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);

	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();

	AQLDate fdate = fromDate;
	AQLDate tdate = toDate;
	if(!cal.isNull() && !sr.isNull())
	{
		fdate = sr.getDate(fdate, cal);
		tdate = sr.getDate(tdate, cal);
	}
	double termf = conv.getTerm(asOf, fdate);
	double termt = conv.getTerm(asOf, tdate);
	double dff = getDF(termf);
	double dft = getDF(termt);
	
	return AQLPriceDataConvention::retToRate(dff / dft, termt - termf, conv);
}
*/

void AQLMathYieldCurve::analyzeLeg(
	const DateVector& dates,
	const AQLString& foreCurveName,
	const AQLString& dfCurveName,
	bool isFWDInter,
	DoubleArray& rates,
	DoubleArray& terms,
	DoubleArray& dfs_start,
	DoubleArray& dfs_end)
{
	const auto orgName = getCurveType();

	rates    .resize(dates.size() - 1);
	terms    .resize(dates.size() - 1);
	dfs_start.resize(dates.size() - 1);
	dfs_end  .resize(dates.size() - 1);

	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((getYieldData().get().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	const DayCount dc_act(ACT_365_ISDA);
	const AQLPriceDataDayCount data_dc_act(dc_act);

	size_t nextCashflowIndex = 0;
	AQLString freq = "";
	AQLPriceDataCalendar data_cal;
	AQLPriceDataSlidingRule data_sld;
	AQLPriceDataDayCount data_dc;
	AQLString accessary = "";

	if(foreCurveName.size() == 0)
	{
		setCurveType(dfCurveName);
		nextCashflowIndex = getNextCashFlowIndex(dates, asOf);
		getCurveConvention(freq, data_cal, data_sld, data_dc, accessary, dfCurveName);

		for (size_t i = 1; i < dates.size(); i++)
		{
			terms[i - 1] = data_dc.getTerm(dates[i - 1], dates[i]);
			rates[i - 1] = 0.0;
		}
	}
	else
	{
		setCurveType(foreCurveName);
		nextCashflowIndex = getNextCashFlowIndex(dates, asOf);
		getCurveConvention(freq, data_cal, data_sld, data_dc, accessary, foreCurveName);

		if (isFWDInter)
		{
			const AQLInterpolationBase &fwd_inter = getFWDInterpolation(&foreCurveName);
			for (size_t i = 1; i < dates.size(); i++)
			{
				terms[i - 1] = data_dc.getTerm(dates[i - 1], dates[i]);
				const double term = data_dc_act.getTerm(asOf, dates[i - 1]);
				rates[i - 1] = (i < nextCashflowIndex) ? 0.0 : fwd_inter.value(term);
			}
		}
		else
		{
			for (size_t i = 1; i < dates.size(); i++)
			{
				terms[i - 1] = data_dc.getTerm(dates[i - 1], dates[i]);
				rates[i - 1] = (i < nextCashflowIndex) ? 0.0 : (getDF(data_dc_act.getTerm(asOf, dates[i - 1]), &dc_act) / getDF(data_dc_act.getTerm(asOf, dates[i]), &dc_act) - 1.) / terms[i - 1];
			}
		}
	}

	setCurveType(dfCurveName);
	for (size_t i = 1; i < dates.size(); i++)
	{
		dfs_end  [i - 1] = (i < nextCashflowIndex) ? 0.0 : getDF(data_dc_act.getTerm(asOf, dates[i    ]), &dc_act);
		dfs_start[i - 1] = (i < nextCashflowIndex) ? 0.0 : getDF(data_dc_act.getTerm(asOf, dates[i - 1]), &dc_act);
	}

	setCurveType(orgName);
}

double
AQLMathYieldCurve::getZeroRate(const AQLDate& fromDate, const AQLDate& toDate, bool isFWDInter, bool useFwdData ) const
{
	
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	
	if (fromDate > toDate)
	{
		AQLString err = "#Error: Unable to calculate a zero rate in the past. The toDate must be after fromDate";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

    RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);
	DayCount dc_act(ACT_365_ISDA);
	AQLPriceDataDayCount dc(dc_act);
	
    AQLDate fdate = (fromDate < asOf) ? asOf : fromDate;
	AQLDate tdate = (toDate < asOf) ? asOf : toDate;
	
    //const AQLPriceDataCalendar& cal = getCalendar();
	//const AQLPriceDataSlidingRule& sr = getSlidingRule();
    //if(!cal.isNull() && !sr.isNull())
	//{
	//	if (sr.getDate(fdate, cal) >= asOf) fdate = sr.getDate(fdate, cal);
	//	if (sr.getDate(tdate, cal) >= asOf) tdate = sr.getDate(tdate, cal);
	//}
	
	if (fdate == tdate) return 0.0;
    	
	
	if (rc == SIMPL && isFWDInter)
	{
		// 0 is null pointer
        const AQLInterpolationBase &fwd_inter = getFWDInterpolation(0, useFwdData);
		AQLPriceDataDayCount dc_act(ACT_365_ISDA);
		const double term = dc_act.getTerm(asOf, fdate);
		return fwd_inter.value(term);
	}
	else
	{
		double termf = dc.getTerm(asOf, fdate);
		double termt = dc.getTerm(asOf, tdate);
		double dff = getDF(termf, &dc_act);
		double dft = getDF(termt, &dc_act);

		termf = conv.getTerm(asOf, fdate);
		termt = conv.getTerm(asOf, tdate);

		return AQLPriceDataConvention::retToRate(dff / dft, termt - termf, conv);
	}
}

double
AQLMathYieldCurve::getBasisZeroRate(const AQLDate& fromDate, const AQLDate& toDate) const
{
	
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	
	if (fromDate > toDate)
	{
		AQLString err = "#Error: Unable to calculate a zero rate in the past. The toDate must be after fromDate";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	//if (fromDate < asOf)
	//{
	//	AQLString err = "fromDate must be after asOf";
	//	throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	//}

	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);
	DayCount dc_act(ACT_365_ISDA);
	AQLPriceDataDayCount dc(dc_act);
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();

	AQLDate fdate = (fromDate < asOf) ? asOf : fromDate;
	AQLDate tdate = (toDate < asOf) ? asOf : toDate;
	if (!cal.isNull() && !sr.isNull())
	{
		if (sr.getDate(fdate, cal) >= asOf) fdate = sr.getDate(fdate, cal);
		if (sr.getDate(tdate, cal) >= asOf) tdate = sr.getDate(tdate, cal);
	}
	
	if (fdate == tdate) return 0.0;

	double termf = dc.getTerm(asOf, fdate);
	double termt = dc.getTerm(asOf, tdate);
	double dff = getBasisDF(termf, &dc_act);
	double dft = getBasisDF(termt, &dc_act);

	termf = conv.getTerm(asOf, fdate);
	termt = conv.getTerm(asOf, tdate);

	return AQLPriceDataConvention::retToRate(dff / dft, termt - termf, conv);
}

/*!
    @brief get ZeroRate for specified term from fromDate 
			
	@param[in] fromDate StartDate
	@param[in] term		Term
	
	@return ZeroRate 
*/

double
AQLMathYieldCurve::getZeroRate(const AQLDate& fromDate,	const double term, bool isFWDInter, bool useFwdData ) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

	if (term < 0.0)
	{
        AQLString err = "#Error: Unable to calculate a zero rate in the past";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();

	AQLDate fdate = (fromDate < asOf) ? asOf : fromDate;
	if(!cal.isNull() && !sr.isNull())
	{
		if (sr.getDate(fdate, cal) >= asOf) fdate = sr.getDate(fdate, cal);
	}
	
	if (term < EPS) return 0.0;

	if (rc == SIMPL && isFWDInter)
	{        
		// 0 is null pointer
		const AQLInterpolationBase &fwd_inter = getFWDInterpolation(0, useFwdData);
		AQLPriceDataDayCount dc_act(ACT_365_ISDA);
		const double term = dc_act.getTerm(asOf, fdate);
		return fwd_inter.value(term);
	}
	else
	{
		double termTmp = conv.getTerm(asOf, asOf.intervalDays(fromDate) + conv.getDayTerm(fromDate, term));
		double dff = getDF(asOf, fdate);
		double dft = getDF(termTmp);
		
		return AQLPriceDataConvention::retToRate(dff / dft, term, conv);
	}

}

/*!
    @brief get BasisZeroRate for specified term from fromDate 
			
	@param[in] fromDate StartDate
	@param[in] term		Term
	
	@return ZeroRate 
*/

double
AQLMathYieldCurve::getBasisZeroRate(const AQLDate& fromDate, const double term)const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

	if (term < 0.0)
	{
        AQLString err = "#Error: Unable to calculate a zero rate in the past";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	//if (fromDate < asOf)
	//{
	//	AQLString err = "fromDate must be after asOf";
	//	throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	//}
	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();

	AQLDate fdate = (fromDate < asOf) ? asOf : fromDate;
	if(!cal.isNull() && !sr.isNull())
	{
		if (sr.getDate(fdate, cal) >= asOf) fdate = sr.getDate(fdate, cal);
	}
	
	if (term < EPS) return 0.0;
	
	double termTmp = conv.getTerm(asOf, asOf.intervalDays(fromDate) + conv.getDayTerm(fromDate, term));
	double dff = getBasisDF(asOf, fdate);
	double dft = getBasisDF(termTmp);
	
	return AQLPriceDataConvention::retToRate(dff / dft, term, conv);
}

/*!
    @brief get ZeroRate for specified term from AsOfDate 
			
	@param[in] term Term
	
	@return ZeroRate
*/

double
AQLMathYieldCurve::getZeroRate(const double term, bool isFWDInter) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

	return getZeroRate(asOf, term, isFWDInter);
}

/*!
    @brief get BasisZeroRate for specified term from AsOfDate 
			
	@param[in] term Term
	
	@return ZeroRate
*/

double
AQLMathYieldCurve::getBasisZeroRate(const double term) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

	return getBasisZeroRate(asOf, term);
}


/*!
    @brief get ZeroRate for specified term from fromDate 
			
	@param[in] fromDate StartDate
	@param[in] term		Term of String
	
	@return ZeroRate 
*/

double
AQLMathYieldCurve::getZeroRate(const AQLDate& fromDate,	const AQLString& term_str, bool isFWDInter) const
{
	AQLDate toDate = AQLMathDateCalculations::getDate(fromDate, term_str, true);
	return getZeroRate(fromDate, toDate, isFWDInter);
}

/*!
    @brief get BasisZeroRate for specified term from fromDate 
			
	@param[in] fromDate StartDate
	@param[in] term		Term of String
	
	@return ZeroRate 
*/

double
AQLMathYieldCurve::getBasisZeroRate(const AQLDate& fromDate, const AQLString& term_str) const
{
	AQLDate toDate = AQLMathDateCalculations::getDate(fromDate, term_str, true);
	return getBasisZeroRate(fromDate, toDate);
}

/*!
    @brief get ZeroRate for specified term from AsOfDate 
			
	@param[in] term Term of String
	
	@return ZeroRate
*/

double
AQLMathYieldCurve::getZeroRate(const AQLString& term_str, bool isFWDInter) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	return getZeroRate(asOf, term_str, isFWDInter);
}

/*!
    @brief get BasisZeroRate for specified term from AsOfDate 
			
	@param[in] term Term of String
	
	@return ZeroRate
*/

double
AQLMathYieldCurve::getBasisZeroRate(const AQLString& term_str) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	return getBasisZeroRate(asOf, term_str);
}

/*!
    @brief get ParRate for specified term from AsOfDate 
			
	@param[in] term Term of String
	
	@return ParRate
*/

double
AQLMathYieldCurve::getParRate(const AQLString& term_str, AQLString foreCurveName, AQLString dfCurveName, bool isFWDInter, const AQLString* roll_convention)
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	return getParRate(asOf, term_str,NULL,NULL,NULL, foreCurveName, dfCurveName, isFWDInter, roll_convention);
}
	
/*!
    @brief get Annuity from DateVector 
			
	@param[in] Dates DateVector
	
	@return Annuity
*/

double
AQLMathYieldCurve::getAnnuity(const DateVector& Dates) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();

	AQLPriceDataDayCount dc(getDayCount());
	DayCount dc_act(ACT_365_ISDA);
	AQLPriceDataDayCount data_dc_act(dc_act);
	
	double ret = 0.0;
	AQLDate fromDate,  tmpDate;
	if (Dates[0] != asOf)
	{
        fromDate = sr.getDate(Dates[0], cal);
	}
	else
	{
		fromDate = asOf;
	}
	if (fromDate < asOf)
	{
		fromDate = asOf;
//		AQLString err = "fromDate must be after asOf";
//        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	double tmp = dc.getTerm(asOf, fromDate);

	AQLDate lastDate = Dates[0];
	lastDate = sr.getDate(lastDate, cal);
	for(unsigned int i = 1; i < Dates.size(); i++)
	{
		// lastDate = startDate
        // tmpDate = endDate
        
        tmpDate = Dates[i];
		tmpDate = sr.getDate(tmpDate, cal);
		
		double term = data_dc_act.getTerm(asOf, tmpDate);
        
        // Ignore annuity terms if they are fully in the past
        // Annuity should not include past cashflows
        if (term>=0 && tmpDate>=asOf)
        {
            ret += getDF(term, &dc_act) * dc.getTerm(lastDate, tmpDate);
        }
		tmp = dc.getTerm(asOf, tmpDate);
		lastDate = tmpDate;
	}
	return ret;
}

/*!
    //	get Annuity from AccrualDates, PaymentDates and Daycount fraction
			
	@param[in] DateVector       accrualDates 
    @param[in] DateVector       paymentDates 
    @param[in] AQLPriceDataDayCount   daycount
    @param[in] AQLDate           settlementDate - Optional: required for bond calculations
    @param[in] bool             deductAccruedInterest - Optional: defaults to false, this is for bond / asset swap spread calculations
	@param[in] bool             isCleanPrice - Optional: defaults to true, this is for bond / asset swap spread calculations

	@return Annuity
*/
double
AQLMathYieldCurve::getAnnuity( const DateVector& accrualDates, const DateVector& paymentDates, const AQLPriceDataDayCount& daycount, const AQLDate settlementDate, const bool deductAccruedInterest, const bool isCleanPrice ) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();

	AQLPriceDataDayCount dc( getDayCount() );
	DayCount dc_act( ACT_365_ISDA );
	AQLPriceDataDayCount data_dc_act( dc_act );
	
	double ret = 0.0;
	AQLDate fromDate;
    AQLDate tmpAccrualEndDate;
    AQLDate tmpPaymentDate;
	
    if ( accrualDates[0] != asOf )
	{
        fromDate = sr.getDate( accrualDates[0], cal );
	}
	else
	{
		fromDate = asOf;
	}

	if (fromDate < asOf)
	{
		fromDate = asOf;
	}

	AQLDate accrualStartDate = accrualDates[0];
	accrualStartDate = sr.getDate( accrualStartDate, cal );

	for( unsigned int i = 1; i < accrualDates.size(); i++ )
	{
		tmpAccrualEndDate = accrualDates[i];
		tmpAccrualEndDate = sr.getDate( tmpAccrualEndDate, cal );
		
        tmpPaymentDate = paymentDates[i];
        tmpPaymentDate = sr.getDate( tmpPaymentDate, cal );

        if ( tmpPaymentDate < tmpAccrualEndDate )
            throw AQLCoreInvalidData("#Error: Annuity Payment Date cannot be before the Accrual End Date.", __FILE__, __LINE__ );
            
        // ret += discount factor * year fraction
        // discount factor is based on ACT/ACT daycount
        // year fraction is based on client specified daycount
        double term = data_dc_act.getTerm( asOf, tmpPaymentDate );
        
        // Ignore annuity terms if they are fully in the past
        if ( term >= 0 && tmpPaymentDate >= asOf )
        {
            const double discountFactor   = getDF(term, &dc_act );
            const double yearFraction     = daycount.getTerm( accrualStartDate, tmpAccrualEndDate );

		    ret += discountFactor * yearFraction;

            // Optional: Deduct Accrued Interest - This is for Bond / Asset Swap Spread Calculations
            if ( deductAccruedInterest && accrualStartDate < asOf)
            {
                if ( settlementDate == AQLDate() )
                {
                    throw AQLCoreInvalidData("#Error: Annuity Calculation Error: For Bond-type calculations the settlement date is required to deduct accrued interest.", __FILE__, __LINE__ );
                }

                const double accruedYearFraction    = daycount.getTerm( accrualStartDate, settlementDate );
                const double accruedInterest        = accruedYearFraction;
                const double accruedInterestPV      = accruedYearFraction * discountFactor;

                // Note: When calculating asset swap spreads the par-par adjustment is based on the the dirty bond price
                // i.e. ( 100 - Dirty Bond Price ) / 100. The first bond coupon in the asset swap is full and the par 
                // adjustment corrects the coupon overpayment of accrued interest in the fixed leg of the asset swap, which
                // is in the future. Bond accrued interest however is settled immediately having a timing / discounting difference.
                // 
                // As such if we are given a clean price we must add back the discounted accrued interest and if we are 
                // provided the dirty price we must add back only the par adjustment discounting difference.
                double parAccruedInterestAdjustment = 0;

                if ( isCleanPrice )
                {
                    // Remove the Accrued Interest PV i.e. the discounted accrued interest
                    parAccruedInterestAdjustment = accruedInterestPV;
                }
                else
                {
                    // Remove just the discounting difference i.e. discounted minus non-discounted accrued interest
                    parAccruedInterestAdjustment = accruedInterestPV - accruedInterest;
                }

                // Convert the Asset Swap Par-Par Adjustment from using the Clean to Dirty Bond Price
                ret -= parAccruedInterestAdjustment;
            }

        }
        accrualStartDate = tmpAccrualEndDate;
	}

	return ret;
}

/*!
    @brief get ParRate from DateVector 
			
	@param[in] Dates DateVector
	
	@return ParRate
*/
double
AQLMathYieldCurve::getParRate(const DateVector& dates, const DateVector& dates_float, AQLString foreCurveName, AQLString dfCurveName, bool isFWDInter)
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();

	AQLDate fromDate = sr.getDate(dates.front(), cal);
	AQLDate tmpDate  = sr.getDate(dates.back(), cal);

	DayCount dc_act(ACT_365_ISDA);
	AQLPriceDataDayCount data_dc_act(dc_act);

	setCurveType(dfCurveName);
	double annuity = getAnnuity(dates);
	double floaterPV = 0.;

	//if (foreCurveName != STD || dfCurveName != STD)
	if (foreCurveName != dfCurveName)
	{
		AQLString freq_forecast = ""; 
		AQLPriceDataCalendar cal_forecast; 
		AQLPriceDataSlidingRule sld_forecast; 
		AQLPriceDataDayCount dc_forecast;
		AQLString accessary_forecast = "";
		
		getCurveConvention(freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName);
		
		DoubleArray rates(dates_float.size()-1),dfs(dates_float.size()-1),terms(dates_float.size()-1);  
		
        // Find the index for the next cashflow, so that we can ignore cashflows in the past
        unsigned int nextCashflowIndex = 1;
        for (unsigned int i = 0; i < dates_float.size(); ++i)
        {
            if ( i == dates_float.size()-1  )
            {
                if ( dates_float[i] < asOf ) 
                    throw AQLCoreInvalidData("#Error: Par rate error; the underlying swap has expired.", __FILE__, __LINE__ );
                
				nextCashflowIndex++;
                break; 
            }
            if ( dates_float[i+1] >= asOf ) break;
            nextCashflowIndex++;
        }
        
        //get rate
		if (isFWDInter)
		{
			const AQLInterpolationBase &fwd_inter = getFWDInterpolation(&foreCurveName);
			for(size_t i=1; i<dates_float.size(); i++)
			{
				terms[i-1] = dc_forecast.getTerm(dates_float[i-1],dates_float[i]);
				const double term = data_dc_act.getTerm(asOf, dates_float[i-1]); 
				rates[i-1] = (i<nextCashflowIndex) ? 0.0 : fwd_inter.value(term);
			}
		}
		else
		{
			setCurveType(foreCurveName);
			for(size_t i=1; i<dates_float.size(); i++)
			{
				terms[i-1] = dc_forecast.getTerm(dates_float[i-1], dates_float[i]);
				rates[i-1] = (i<nextCashflowIndex) ? 0.0 : (getDF(data_dc_act.getTerm(asOf, dates_float[i-1]), &dc_act) / getDF(data_dc_act.getTerm(asOf, dates_float[i]), &dc_act) - 1.) / terms[i-1];
			}
		}
		//get df
		setCurveType(dfCurveName);
		for(size_t i=1; i<dates_float.size(); i++)
		{
			dfs[i-1] = (i<nextCashflowIndex) ? 0.0 : getDF(data_dc_act.getTerm(asOf, dates_float[i]), &dc_act);
		}
		//calc floater PV
		for(size_t i=1; i<dates_float.size(); i++)
		{			
			floaterPV += rates[i-1] * dfs[i-1] * terms[i-1];
		}
	}
	else
	{
		floaterPV = getDF(data_dc_act.getTerm(asOf, fromDate), &dc_act) - getDF(data_dc_act.getTerm(asOf, tmpDate), &dc_act);
	}

	return floaterPV / annuity;
}

double
AQLMathYieldCurve::getParCompoundSpread(
	const DateVector& dates,
	const DateVector& dates_Cmp,
	AQLString foreCurveName,
	AQLString foreCurveName_Cmp,
	AQLString dfCurveName,
	bool isStraight,
	bool isFWDInter)
{
	if (dates.size() > dates_Cmp.size()) {
		return getParCompoundSpread(dates_Cmp, dates, foreCurveName_Cmp, foreCurveName, dfCurveName, isStraight, isFWDInter);
	}

	if (dates.size() <= 1 || dates_Cmp.size() <= 1) {
		return 0.0;
	}

	// valuate PV of non-compounding leg.
	DoubleArray rates;
	DoubleArray terms;
	DoubleArray dfs_start;
	DoubleArray dfs_end;
	analyzeLeg(dates, foreCurveName, dfCurveName, isFWDInter, rates, terms, dfs_start, dfs_end);

	double nonCmpLegPV = 0.0;
	for (size_t i = 1; i<dates.size(); i++)
	{
		nonCmpLegPV += rates[i - 1] * terms[i - 1] * dfs_end[i - 1];
	}

	// valuate PV of compounding leg.
	DoubleArray rates_Cmp;
	DoubleArray terms_Cmp;
	DoubleArray dfs_start_Cmp;
	DoubleArray dfs_end_Cmp;
	analyzeLeg(dates_Cmp, foreCurveName_Cmp, dfCurveName, isFWDInter, rates_Cmp, terms_Cmp, dfs_start_Cmp, dfs_end_Cmp);

	const auto cmpTimes = (dates_Cmp.size() - 1) / (dates.size() - 1);
	const auto pvFunc = [&](double spread)->double
	{
		double PV = nonCmpLegPV;

		const double cmpSpread = (isStraight ? spread : 0.0);

		for (size_t i = (cmpTimes - 1); i < (dates_Cmp.size() - 1); i += cmpTimes)
		{
			double rate    = 0.0;
			double cmpRate = 1.0;
			for (size_t k = 0, j = i; k < cmpTimes; ++k, j = i - k)
			{
				rate    += terms_Cmp[j] * (rates_Cmp[j] + spread) * cmpRate;
				cmpRate *= (1.0 + terms_Cmp[j] * (rates_Cmp[j] + cmpSpread));
			}

			PV -= rate * dfs_end_Cmp[i];
		}	

		return PV;
	};

	const double lowerSpread = -1.0;
	const double upperSpread = +1.0;
	const double parSpread = findRootBrent(pvFunc, lowerSpread, upperSpread);

	return parSpread;
}

double
AQLMathYieldCurve::getParBasis(
	const DateVector& dates,
	const DateVector& dates_USD,
	AQLString modifiedDiscountName,
	AQLString foreCurveName,
	AQLString foreCurveName_USD,
	AQLString dfCurveName_USD,
	bool isMtMCCS,
	bool isFWDInter)
{
	if (dates.size() <= 1 || dates_USD.size() <= 1) {
		return 0.0;
	}


	// valuate PV of this curve leg.

	DoubleArray rates;
	DoubleArray terms;
	DoubleArray dfs_start;
	DoubleArray dfs_end;
	analyzeLeg(dates, foreCurveName, modifiedDiscountName, isFWDInter, rates, terms, dfs_start, dfs_end);

	double annuity = 0.0;
	double legPV = -1.0 + dfs_end.back() / dfs_start[0];
	for (size_t i = 1; i<dates.size(); i++)
	{
		const double annuity_i = dfs_end[i - 1] / dfs_start[0] * terms[i - 1];

		annuity += annuity_i;
		legPV   += annuity_i * rates[i - 1];
	}


	// valuate PV of the USD leg.

	AQLMathYieldCurve* usdCurve = nullptr;
	{
		const AQLObjectHolder objHolder = getDataInstance()->getObjectPool().getObject("YIELD_SDE_USD_IR");
		if (objHolder.isDefined())
		{
			usdCurve = &dynamic_cast<AQLMathYieldCurve&>(
				getDataInstance()->getObjectPool().getObject("YIELD_SDE_USD_IR", ENCHKTYPE_ISDEFINED).get());
		}
	}

	if (!usdCurve) {
		return 0.0;
	}

	DoubleArray rates_USD;
	DoubleArray terms_USD;
	DoubleArray dfs_USD_start;
	DoubleArray dfs_USD_end;
	usdCurve->analyzeLeg(dates, foreCurveName_USD, dfCurveName_USD, isFWDInter, rates_USD, terms_USD, dfs_USD_start, dfs_USD_end);

	double USDLegPV = 0.;
	for (size_t i = 1; i < dates_USD.size(); i++)
	{
		const double fwd_i = (dfs_USD_start[i - 1] / dfs_USD_end[i - 1] - 1) / terms_USD[i - 1];
		const double adj_i = (isMtMCCS ? (dfs_start[i - 1] / dfs_start[0] / dfs_USD_start[i - 1] * dfs_USD_start[0]) : 1);

		USDLegPV += adj_i * dfs_USD_end[i - 1] / dfs_USD_start[0] * terms_USD[i - 1] * (rates_USD[i - 1] - fwd_i);
	}

	return (USDLegPV - legPV) / annuity;
}

/*
* @brief	Calculate stub rate of a defined stub period
*/
double AQLMathYieldCurve::getStubRate( const DateVector& fixingDates,
									const AQLStringVector& curveNames,
									const AQLStringVector& curveTenors,
									const DoubleVector& tenorCurveFixings,									
									const AQLString& curveid,									
									const AQLString& stubType,
									const AQLString& dateCount,
									const AQLString& calendar,
									const AQLString& busDayAdj,
									bool  useNearbyCurve,
									bool  useGivenFixings,
									bool  isFwdInter,
									bool  useFwdData,
									const AQLString& toleranceTenor,
									const AQLString& useCurveName )
{
    AQLString STUBTYPE( stubType );
	upper( STUBTYPE );

	AQLString DATECOUNT( dateCount );
    upper( DATECOUNT );

	AQLString CALENDAR( calendar );
    upper( CALENDAR );

    AQLString BUSDAYADJ( busDayAdj );
	upper( BUSDAYADJ );


	// Get yield curve AsOf date
	const AQLObject& YieldData       = getYieldData().get().get();
	const AQLDate& asOf              = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
 
	double stubRate(0.);
	   
	size_t nFixingDates = fixingDates.size();

	DayCount dc_act_2( ACT_365_ISDA );			
	AQLPriceDataDayCount dc_act( dc_act_2 );

	AQLPriceDataSlidingRule sr;
	sr.convertFromString(BUSDAYADJ);

	AQLPriceDataCalendar cal;
	cal.convertFromString(CALENDAR);
		
	AQLPriceDataDayCount floatingLegDateCount;
	floatingLegDateCount.convertFromString(DATECOUNT);	

    // Bounds Check
    if ( fixingDates.size() < 2 )
    {
        throw AQLCoreInvalidData("#Error: Unable to calculate the swap stub rate. Invalid fixing dates.", __FILE__, __LINE__ );
    }

	// Determine the exact stub period. Must guaranteee that stub end date is after the asOf date.
	AQLDate stubStart;
	AQLDate stubEnd;
	if ( STUBTYPE == "NONE" || STUBTYPE == "SHORTSTART" || STUBTYPE == "SS" || STUBTYPE == "LONGSTART" || STUBTYPE == "LS")
	{
        stubStart   = fixingDates[0];
        stubEnd     = fixingDates[1];
        
        // We can't calculate historic stubs, since such stubs have already been fixed.
        if ( fixingDates[0] < asOf )
	    {
            // Throw an error if the stub rate is in the past, but paying in the future. Such a front stub needs to be set using the 'FirstFixing' parameter
            if ( stubEnd >= asOf )
            {
                throw AQLCoreInvalidData("#Error: Front stub fixing rate required.", __FILE__, __LINE__ );
            }

            // Set the Stub to Zero if it is in the past and the payment date is also in the past
		    stubRate = 0.0;
		    return stubRate;
	    }

		// Skip all expired coupons and get to the first valid cash flow
		size_t nextCashflowIndex = getNextCashFlowIndex(fixingDates, asOf);
			
		for (size_t i=1; i<fixingDates.size(); ++i)
		{
			if (i == nextCashflowIndex)
			{
				stubStart = fixingDates[i - 1];
				stubEnd = fixingDates[i];
			}
		}
	}
	else if ( STUBTYPE == "SHORTEND" || STUBTYPE == "SE" || STUBTYPE == "LONGEND" || STUBTYPE == "LE")
	{
		if (fixingDates[nFixingDates - 1] < asOf)
		{
			// Swap expired. 
			stubRate = 0.;
			return stubRate;
		}

		stubStart = fixingDates[nFixingDates - 2];
		stubEnd = fixingDates[nFixingDates - 1];
	}
	else
	{
		throw AQLCoreInvalidData("#Error: Stub Type must be None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).", __FILE__, __LINE__ );
	}

	// Calculate stub rate - either (1) use a given curve, (2.a) pick the nearest curve, or (2.b) interpolate between adjacent curves
	//if (useCurveName.size() != 0 && _stricmp(useCurveName.getCString(), "NATURAL") != 0)
	//20161207 - Fixed on compiler error for _stricmp
	AQLString useCurveNameTemp(useCurveName);
	useCurveNameTemp.toUpper();
	if (useCurveNameTemp.size() != 0 && useCurveNameTemp.findString( "NATURAL") != 0)
	{
		// (1) Use a user-specified yield curve

		// First check if a fixing has been provided to this curve name
		size_t idx = -1;
		for(size_t i=0; i<curveNames.size(); ++i)
		{
			//if (_stricmp(curveNames[i].getCString(), useCurveNameTemp.getCString()) == 0)
			//20161207 - Fixed on compiler error for _stricmp
			if (curveNames[i].findString(useCurveNameTemp.getCString()) == 0)
			{
				idx = i;
				break;
			}
		}

		if (idx == -1)
		{
			throw AQLCoreInvalidData("#Error: The 'useCurveName' should also be part of the curveNames list", __FILE__, __LINE__ );
		}

		if (useGivenFixings)
		{
			// If the fixing for the designated curve has been given, use it
			stubRate = tenorCurveFixings[idx];
		}
		else
		{
			// When we use a user-specified curve, we should use the corresponding curve tenor to calculate the
			// stub end date instead of using the old stub end date
			stubEnd = AQLMathDateCalculations::getDate(stubStart, curveTenors[idx], sr, &cal, true, NULL);

			// Set the right curve to use
			setCurveType(useCurveNameTemp);

            auto dh = YieldData.getData(CALIBRATION_DATA_CURVETYPE + AQLString("_OIS"));
            bool isCurveTypeOIS = dh.isDefined() && !dh.isNull();

			if (isFwdInter && !isCurveTypeOIS)
			{
				AQLPriceDataDayCount dc;
				if (!AQLMathCurveFuncUtility::setUpForwardDayCount(getDataInstance(), curveid, useCurveNameTemp, dc))
				{
					throw AQLCoreInvalidData("Interpolation on forward rate failed. Check whether the forward rate was generated by AQLMathYieldCurvePro !!",__FILE__,__LINE__);
				}
				else
				{
					getDayCount(useCurveNameTemp) = dc;
				}

				const AQLInterpolationBase &fwd_inter = getFWDInterpolation(&useCurveNameTemp, useFwdData);
				const double yearFraction = dc_act.getTerm( asOf, stubStart); 
				stubRate = fwd_inter.value( yearFraction );
			}
			else
			{				
				if (stubStart >= asOf)
				{
					const double lowerAccrualFraction = floatingLegDateCount.getTerm(stubStart, stubEnd);
					stubRate = ( getDF( dc_act.getTerm( asOf, stubStart ), &dc_act_2 ) 
							/ getDF( dc_act.getTerm( asOf, stubEnd ), &dc_act_2 ) - 1.) / lowerAccrualFraction;
				}
				else
				{
					throw AQLCoreInvalidData("#Error: Do not support past starting swap when isFwdInter is set FALSE", __FILE__, __LINE__ );
				}
			}
		}
	}
	//else if (useCurveName.size() == 0 || _stricmp(useCurveName.getCString(), "NATURAL") == 0)
	//20161207 - Fixed on compiler error for _stricmp
	else if (useCurveNameTemp.size() == 0 || useCurveNameTemp.findString("NATURAL") == 0)
	{
		// (2) Either pick the nearest curve, or interpolate stub rate from nearby standard forward rates		
		
		size_t lowerIndex = 0;
		size_t upperIndex = 0;
		AQLDate lowerDate;
		AQLDate upperDate;

		size_t curveCount = curveNames.size();
		if (curveCount == 0)
		{
			throw AQLCoreInvalidData("#Error: Please provide at least one curve to calculate stub rate", __FILE__, __LINE__ );
		}

		// Pick the curves to interpolate from
		AQLDate firstTenorDate = AQLMathDateCalculations::getDate(stubStart, curveTenors.at(0), sr, &cal, true, NULL);
		AQLDate lastTenorDate = AQLMathDateCalculations::getDate(stubStart, curveTenors.at(curveTenors.size() - 1), sr, &cal, true, NULL);
		if (stubEnd == firstTenorDate)
		{
			lowerIndex = 0;
			lowerDate = firstTenorDate;

			upperIndex = lowerIndex;
			upperDate = lowerDate;
		}
		else if (stubEnd == lastTenorDate)
		{
			upperIndex = curveCount - 1;
			upperDate = lastTenorDate;

			lowerIndex = upperIndex;
			lowerDate = upperDate;
		}
		else
		{
			for(size_t i = 0; i < curveCount; ++i)
			{
				AQLString curveTenor = curveTenors.at(i);
				AQLDate tenorEnd = AQLMathDateCalculations::getDate(stubStart, curveTenor, sr, &cal, true, NULL);

				if (tenorEnd < stubEnd)
				{
					lowerIndex = i;
					lowerDate = tenorEnd;
				}
				else
				{
					upperIndex = i;
					upperDate = tenorEnd;
					break;
				}
			}		

			if (upperIndex == 0)
			{
				throw AQLCoreInvalidData("#Error: Stub term is shorter than the shortest tenor term available in the curves. Stub rate can't be interpolated.", __FILE__, __LINE__ );
			}
			else if (lowerIndex == curveCount - 1)
			{
				throw AQLCoreInvalidData("#Error: Stub term is longer than the longest tenor term available in the curves. Stub rate can't be interpolated.", __FILE__, __LINE__ );
			}
		}

		
		{		
			// Try to determine if we can pick a nearby curve to use
			bool isUsingNearbyCurve = false;
			size_t nearbyIndex;
			AQLDate nearbyDate;
			if (useNearbyCurve)
			{
				AQLDate lowerToleranceDate = AQLMathDateCalculations::getDate(lowerDate, toleranceTenor, sr, &cal, true  /*forward add date*/ , NULL);
				AQLDate upperToleranceDate = AQLMathDateCalculations::getDate(upperDate, toleranceTenor, sr, &cal, false /*backward add date*/, NULL);
						
				if (lowerToleranceDate >= stubEnd)
				{
					nearbyIndex = lowerIndex;
					nearbyDate = lowerDate;
					isUsingNearbyCurve = true;
				}
				else if (upperToleranceDate <= stubEnd)
				{
					nearbyIndex = upperIndex;
					nearbyDate = upperDate;
					isUsingNearbyCurve = true;
				}
			}

			// Calculate stub rate now
			if (isUsingNearbyCurve)
			{
				// (2.a) Using nearby curve to get a forward rate out as an approximation of stub rate
				if (useGivenFixings)
				{
					stubRate = tenorCurveFixings[nearbyIndex];
				}
				else
				{
					// Set the right curve to use
					AQLString curveName = curveNames[nearbyIndex];
					setCurveType(curveName);

                    auto dh = YieldData.getData(CALIBRATION_DATA_CURVETYPE + AQLString("_OIS"));
                    bool isCurveTypeOIS = dh.isDefined() && !dh.isNull();

                    if (isFwdInter && !isCurveTypeOIS )
					{						
						AQLPriceDataDayCount dc;
						if (!AQLMathCurveFuncUtility::setUpForwardDayCount(getDataInstance(), curveid, curveName, dc))
						{
							throw AQLCoreInvalidData("Interpolation on forward rate failed. Check whether the forward rate was generated by AQLMathYieldCurvePro !!",__FILE__,__LINE__);
						}
						else
						{
							getDayCount(curveName) = dc;
						}

						const AQLInterpolationBase &fwd_inter = getFWDInterpolation(&curveName, useFwdData);
						const double yearFraction = dc_act.getTerm( asOf, nearbyDate); 
						stubRate = fwd_inter.value( yearFraction );
					}
					else
					{
						if (nearbyDate >= asOf)
						{
							const double accrualFraction = floatingLegDateCount.getTerm(stubStart, nearbyDate);
							stubRate = ( getDF( dc_act.getTerm( asOf, stubStart ), &dc_act_2 ) 
								/ getDF( dc_act.getTerm( asOf, nearbyDate ), &dc_act_2 ) - 1.) / accrualFraction;
						}
						else
						{
							stringstream s;
							s << "Error: Curve name '" << curveNames[nearbyIndex] << "' is chosen as the approximate curve but this curve's tenor is too short";
							throw AQLCoreInvalidData(s.str().c_str(), __FILE__, __LINE__ );
						}
					}
				}
			}
			else
			{
				// (2.b) Not able to pick a nearby curve for approximation. Go ahead with linear interpolation.

				const double lowerFraction = dc_act.getTerm(lowerDate, stubEnd);
				const double upperFraction = dc_act.getTerm(stubEnd, upperDate);
				const double fullTerm = lowerFraction + upperFraction;			

				if (useGivenFixings)
				{
                    // No need to interpolate when on a pillar point.     
                    if ( lowerDate == stubEnd || upperDate == stubEnd )
                    {
					    // Return Node Value
                        stubRate = tenorCurveFixings[lowerIndex];
                    }
                    else
                    {
                        // Linear Interpolation: Note when we are on a pillar we have fullTerm = 0.0 giving a divide by zero
                        stubRate = tenorCurveFixings[lowerIndex] + (tenorCurveFixings[upperIndex] - tenorCurveFixings[lowerIndex]) * (lowerFraction / fullTerm);
                    }
				}
				else
				{
					double lowerTenorRate(0.);
					double upperTenorRate(0.);
			
					// Calculate the lower and upper forward rates from which the target stub rate is to be interpolated 
                    setCurveType(curveNames[lowerIndex]);

                    auto dh = YieldData.getData(CALIBRATION_DATA_CURVETYPE + AQLString("_OIS"));
                    bool isLowerIndexCurveTypeOIS = dh.isDefined() && !dh.isNull();

                    if (isFwdInter && !isLowerIndexCurveTypeOIS)
					{
						// get lower rate

						AQLPriceDataDayCount dc;
						setCurveType(curveNames[lowerIndex]);
						if (!AQLMathCurveFuncUtility::setUpForwardDayCount(getDataInstance(), curveid, curveNames[lowerIndex], dc))
						{
							throw AQLCoreInvalidData("Interpolation on forward rate failed. Check whether the forward rate was generated by AQLMathYieldCurvePro !!",__FILE__,__LINE__);
						}
						else
						{
							getDayCount(curveNames[lowerIndex]) = dc;
						}					
					
						const AQLInterpolationBase &lowerCurve_fwd_inter = getFWDInterpolation(&curveNames[lowerIndex], useFwdData);
						const double lowerDateYearFraction = dc_act.getTerm( asOf, stubStart ); 
						lowerTenorRate = lowerCurve_fwd_inter.value( lowerDateYearFraction );

						// get upper rate
						setCurveType(curveNames[upperIndex]);
						if (!AQLMathCurveFuncUtility::setUpForwardDayCount(getDataInstance(), curveid, curveNames[upperIndex], dc))
						{
							throw AQLCoreInvalidData("Interpolation on forward rate failed. Check whether the forward rate was generated by AQLMathYieldCurvePro !!",__FILE__,__LINE__);
						}
						else
						{
							getDayCount(curveNames[upperIndex]) = dc;
						}

						const AQLInterpolationBase &upperCurve_fwd_inter = getFWDInterpolation(&curveNames[upperIndex], useFwdData);
						const double upperDateYearFraction = dc_act.getTerm( asOf, stubStart ); 
						upperTenorRate = upperCurve_fwd_inter.value( upperDateYearFraction );
					}
					else
					{
						if (stubStart >= asOf)
						{
							setCurveType(curveNames[lowerIndex]);
							const double lowerAccrualFraction = floatingLegDateCount.getTerm(stubStart, lowerDate);
							lowerTenorRate = ( getDF( dc_act.getTerm( asOf, stubStart ), &dc_act_2 ) 
									/ getDF( dc_act.getTerm( asOf, lowerDate ), &dc_act_2 ) - 1.) / lowerAccrualFraction;

							setCurveType(curveNames[upperIndex]);
							const double upperAccrualFraction = floatingLegDateCount.getTerm(stubStart, upperDate);
							upperTenorRate = ( getDF( dc_act.getTerm( asOf, stubStart ), &dc_act_2 ) 
									/ getDF( dc_act.getTerm( asOf, upperDate ), &dc_act_2 ) - 1.) / upperAccrualFraction;
						}
						else
						{
							throw AQLCoreInvalidData("#Error: Do not support past starting swap when isFwdInter is set FALSE", __FILE__, __LINE__ );
						}
					}				
			
					// Linear interpolation
					if (fullTerm == 0.0)
					{
						stubRate = lowerTenorRate;
					}
					else
					{
						stubRate = lowerTenorRate + (upperTenorRate - lowerTenorRate) * (lowerFraction / fullTerm);
					}
				}
			}
		}
	}

	return stubRate;
}

    /* @brief			Function to calculate the swap price and risk. This function has been written to centralize the calculation of these
    *                   parameters and remove the previous multiple duplication of calculations and copy pasting of the same code.
    *   
    *  @param [in]      bool                  isFixedRatePayerSwap; True = Payer Swap, False = Receiver Swap   
    *  @param [in]      double                notional
    *  @param [in]      double                fixedRate
    *  @param [in]      double                floatSpreadInBasisPoints
    *  @param [in]      DateVector            fixedAccrualDates
    *  @param [in]      DateVector            fixedPaymentDates
    *  @param [in]      DateVector            floatFixingDates
    *  @param [in]      DateVector            floatAccrualDates
    *  @param [in]      DateVector            floatPaymentDates
    *  @param [in]      bool                  useFirstFixing
    *  @param [in]      double                firstFixing
    *  @param [in]      bool                  useLastFixing
    *  @param [in]      double                lastFixing
    *  @param [in]      AQLString              fixedDaycount
    *  @param [in]      AQLString              floatDaycount
    *  @param [in]      AQLString              foreCurveName; forecast curve name, defaults to STD
    *  @param [in]      AQLString              dfCurveName; discount curve name, defaults OIS
    *  @param [in]      bool                  isFWDInter, This flag controls how forwards are calculated; True = interpolate forwards, False = imply forwards from discount factors         
	*  @param [in]		bool				  isOIS, Is the forecasting curve an OIS Curve?: True or False
	*  @param [in]		AQLString			  oisCompoundingType, Ois Curve Compounding Method
	*  @param [in]		AQLString			  calendar
	*  @param [in]		AQLString			  interpolation
	*  @param [in]		AQLString			  rollConvention
	*  @param [in]		AQLString			  slidingRule
	*/
void 
AQLMathYieldCurve::calculateSwapPriceAndRisk( const bool              isFixedRatePayerSwap,      
                                           const double            notional,                  
                                           const double            fixedRate,                 
                                           const double            floatSpreadInBasisPoints,  
                                           const DateVector &      fixedAccrualDates,         
                                           const DateVector &      fixedPaymentDates,         
                                           const DateVector &      floatFixingDates,          
                                           const DateVector &      floatAccrualDates,         
                                           const DateVector &      floatPaymentDates,         
                                           const bool              useFirstFixing,            
                                           const double            firstFixing,               
                                           const bool              useLastFixing,             
                                           const double            lastFixing,                
                                           const AQLString &        fixedDaycount,             
                                           const AQLString &        floatDaycount,             
                                           const AQLString &        foreCurveName,       
                                           const AQLString &        dfCurveName,
                                           const bool              isFWDInter,
								           const bool		       isOIS,
								           const AQLString &	       oisCompoundingType,
								           const AQLString &	       calendar,
								           const AQLString &	       inputInterpolation,
								           const AQLString &	       rollConvention,
								           const AQLString &	       slidingRule )
{
    // Initialize Swap Calculations
    swapPV_                                 = 0.0;
    fixedLegPV_                             = 0.0;
    floatLegPV_                             = 0.0;
    floatLegPVExcludingSpread_              = 0.0;
    spreadPV_                               = 0.0;
    AnnuityFixed_                           = 0.0;
    AnnuityFloat_                           = 0.0;
    swapParRateMarket_                      = 0.0;
    swapParRateTrade_                       = 0.0;
    macaulaysDurationSwapExcludingSpread_   = 0.0;
    macaulaysDurationSpread_                = 0.0;
    modifiedDurationSwapExcludingSpread_    = 0.0;
    modifiedDurationSpread_                 = 0.0;
    swapPV01_                               = 0.0;
    swapDV01_                               = 0.0;
    
    // Constant(s)
    const double oneBasisPoint              = 0.0001;
    const double floatSpreadInPercent       = floatSpreadInBasisPoints * oneBasisPoint;
    

    //
    // 1. Load Yield Curve Data
    // -------------------------------------------------------------------------------
    

    const AQLObject& YieldData       = getYieldData().get().get();
	const AQLDate& asOf              = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

    // Check if Swap has Expired
    if ( floatPaymentDates[floatPaymentDates.size()-1] < asOf && fixedPaymentDates[fixedPaymentDates.size()-1] < asOf) 
    {
        return;
    }
        
    const AQLPriceDataCalendar& cal       = getCalendar();
	const AQLPriceDataSlidingRule& sr     = getSlidingRule();

	AQLDate fixedAccrualStartDate    = sr.getDate( fixedAccrualDates.front(), cal );
    AQLDate floatAccrualStartDate    = sr.getDate( floatAccrualDates.front(), cal );
        
    AQLDate fixedAccrualEndDate      = sr.getDate( fixedAccrualDates.back(), cal );
    AQLDate floatAccrualEndDate      = sr.getDate( floatAccrualDates.back(), cal );

	DayCount dc_act( ACT_365_ISDA );
	AQLPriceDataDayCount data_dc_act( dc_act );

    // Get client specified daycount conventions
    AQLPriceDataDayCount fixedLegDaycount;
    fixedLegDaycount.convertFromString( fixedDaycount );
            
    AQLPriceDataDayCount floatLegDaycount;
    floatLegDaycount.convertFromString( floatDaycount );

	setCurveType(dfCurveName);
    

    //
    // 2. Calculate Fixed Leg Vales
    // -------------------------------------------------------------------------------
    

    DoubleArray fixedLegAccrualDateYearFractions( fixedAccrualDates.size() -1 );
    DoubleArray fixedLegDiscFactors( fixedPaymentDates.size() -1 );
        
    if ( fixedAccrualDates.size() != fixedPaymentDates.size() )
        throw AQLCoreInvalidData("#Error: Fixed schedule error. Inconsistent number of acrrual and payment dates.", __FILE__, __LINE__ );

    for( size_t i = 1; i < fixedAccrualDates.size(); i++ )
    {
        // "fixedLegAccrualDateYearFractions" used for the coupon period
        fixedLegAccrualDateYearFractions[i-1] = fixedLegDaycount.getTerm( fixedAccrualDates[i-1], fixedAccrualDates[i] );
    }

    // Fixed Leg Annuity ( note scaled by notional )
    AnnuityFixed_ = notional * getAnnuity( fixedAccrualDates, fixedPaymentDates, fixedLegDaycount );
    

    //
    // 3. Calculate Floating Leg Values
    // -------------------------------------------------------------------------------
	

    AQLString            freq_forecast       = ""; 
	AQLPriceDataCalendar      cal_forecast; 
	AQLPriceDataSlidingRule   sld_forecast; 
	AQLPriceDataDayCount      dc_forecast;
	AQLString            accessary_forecast  = "";
	    	
	getCurveConvention( freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName );

    DoubleArray floatLegRates( floatFixingDates.size()  -1 );
    DoubleArray floatLegDiscFactors( floatPaymentDates.size() -1 );
            
    DoubleArray fixingDateYearFractions( floatFixingDates.size() -1 );  
	DoubleArray floatLegAccrualDateYearFractions( floatAccrualDates.size() -1 );  
    DoubleArray payDateYearFractions( floatPaymentDates.size() -1 );  

    if ( floatFixingDates.size() != floatAccrualDates.size() || floatAccrualDates.size() != floatPaymentDates.size() )
        throw AQLCoreInvalidData("#Error: Floating schedule error. Inconsistent number of fixing, acrrual and payment dates.", __FILE__, __LINE__ );
             
    // Calculate the floating coupon accrual periods
    for( size_t i = 1; i < floatAccrualDates.size(); i++ )
    {
        // "floatLegAccrualDateYearFractions" used for the coupon period
        floatLegAccrualDateYearFractions[i-1] = floatLegDaycount.getTerm( floatAccrualDates[i-1], floatAccrualDates[i] );
    }

    // Find the index for the next cashflow, so that we can ignore cashflows in the past
    unsigned int nextCashflowIndex = 1;
    for (unsigned int i = 0; i < floatAccrualDates.size(); ++i)
    {
        if ( i == floatAccrualDates.size()-1  )
        {
            if ( floatAccrualDates[i] < asOf ) nextCashflowIndex++;
            break; 
        }
        if ( floatAccrualDates[i+1] >= asOf ) break;
        nextCashflowIndex++;
    }

    // Get forward rates from interpolation
	if(isOIS)
	{
	    std::string curveID(getName().convertToString().getCString());
	    size_t pos = curveID.find(AQLMathCurveFuncUtility::YIELD_CURVE_NAME_PREFIX);
	    if ( pos != std::string::npos)
	    {
	    	size_t len = std::string(AQLMathCurveFuncUtility::YIELD_CURVE_NAME_PREFIX).length();
	    	curveID = curveID.substr(pos + len, curveID.length() - len - pos - 1); 
	    }

	    AQLString dayCount = getDayCount().convertToString();

	    // Get equivalent rate over accrual periods
	    getCompoundingRates(floatLegRates, 
	    					floatAccrualDates, 
	    					floatLegAccrualDateYearFractions, 
	    					nextCashflowIndex,
	    					getDataInstance(),
	    					floatSpreadInBasisPoints * oneBasisPoint,
	    					asOf,
	    					AQLString(curveID.c_str()),
	    					foreCurveName,							
	    					slidingRule,
	    					calendar,
	    					rollConvention,
	    					dayCount,
	    					inputInterpolation,
	    					oisCompoundingType);
	}
	else
	{
	    if ( isFWDInter )
	    {
	    	const AQLInterpolationBase &fwd_inter = getFWDInterpolation(&foreCurveName);
	    	for( size_t i = 1; i < floatAccrualDates.size(); i++ )
	    	{
	    		fixingDateYearFractions[i-1]            = dc_forecast.getTerm( floatFixingDates[i-1], floatFixingDates[i] );
	    		const double fixingDateYearFraction     = data_dc_act.getTerm( asOf, floatFixingDates[i-1] ); 
                    
	    		// Apply the first fixing rate for the first stub, if needed
	    		if ( i == nextCashflowIndex && useFirstFixing )
	    		{
	    			floatLegRates[i-1] = firstFixing;
	    		}
	    		// Apply the last fixing rate for the last stub, if needed
	    		else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
	    		{
	    			floatLegRates[i-1] = lastFixing;
	    		}
	    		else
	    		{
	    			// *** Calculate rates using FIXING DATES ***
	    			floatLegRates[i-1] = (i<nextCashflowIndex) ? 0.0 : fwd_inter.value( fixingDateYearFraction );
	    		}
	    	}
	    }
	    // Get forward rates implied from discount factors
	    else
	    {
	    	setCurveType( foreCurveName );
	    	for( size_t i = 1; i < floatPaymentDates.size(); i++ )
	    	{
	    		fixingDateYearFractions[i-1] = floatLegDaycount.getTerm( floatFixingDates[i-1], floatFixingDates[i] );
	    			
	    		// Apply the first fixing rate for the first stub, if needed
	    		if ( i == nextCashflowIndex && useFirstFixing )
	    		{
	    			floatLegRates[i-1] = firstFixing;
	    		}
	    		// Apply the last fixing rate for the last stub, if needed
	    		else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
	    		{
	    			floatLegRates[i-1] = lastFixing;
	    		}
	    		else
	    		{
	    			// *** Calculate rates using FIXING DATES ***
	    			floatLegRates[i-1] = (i<nextCashflowIndex) ? 0.0 :
	    				( getDF( data_dc_act.getTerm( asOf, floatFixingDates[i-1] ), &dc_act ) 
	    				/ getDF( data_dc_act.getTerm( asOf, floatFixingDates[i] ), &dc_act ) - 1.) / fixingDateYearFractions[i-1];
	    		}
	    	}
	    }
	}

    // Float Leg Annuity ( note scaled by notional )
    AnnuityFloat_ = notional * getAnnuity( floatAccrualDates, floatPaymentDates, floatLegDaycount );

    // Get Discount Factors
	setCurveType(dfCurveName);
    for( size_t i = 1; i < fixedPaymentDates.size(); i++ )
	{
        // *** FIXED LEG - Calculate discount factors using PAYMENT DATES ***
        // Note: All the date vectors have an extra date for the start date, so dates index using i and other parameters i-1
	    fixedLegDiscFactors[i-1] = (i<nextCashflowIndex) ? 0.0 : getDF( data_dc_act.getTerm( asOf, fixedPaymentDates[i] ), &dc_act );
	}
        
    for( size_t j = 1; j < floatPaymentDates.size(); j++ )
	{
        // *** FLOAT LEG Calculate discount factors using PAYMENT DATES ***
        // Note: All the date vectors have an extra date for the start date, so dates index using j and other parameters j-1
	    floatLegDiscFactors[j-1] = (j<nextCashflowIndex) ? 0.0 : getDF( data_dc_act.getTerm( asOf, floatPaymentDates[j] ), &dc_act );
	}
   


    //
    // 4. PV and Duration Parameters
    // -------------------------------------------------------------------------------
	

    double fixedLegTimeWeightedPVs                  = 0.0;
    double floatLegTimeWeightedPVs                  = 0.0;
    double floatLegExcludingSpreadTimeWeightedPVs   = 0.0;
    double spreadTimeWeightedPVs                    = 0.0;
    

    //
    // Fixed Leg
    // Note:    All the date vectors have an extra date for the start date, so dates index using i and other parameters i-1

    for( size_t i = 1; i < fixedAccrualDates.size(); i++ ) 
    {
        double fixedAccrualPeriod               = 0.0;
        double fixedDiscFactor                  = 0.0;
        double timeToFixedCouponPayment         = 0.0;

        if ( fixedPaymentDates[i] >= asOf )
        {
            fixedAccrualPeriod                  = fixedLegAccrualDateYearFractions[i-1];
            fixedDiscFactor                     = fixedLegDiscFactors[i-1];
            timeToFixedCouponPayment            = data_dc_act.getTerm( asOf, fixedPaymentDates[i] );
        
            double fixedCashflowPV              = notional * fixedRate * fixedAccrualPeriod * fixedDiscFactor;
            fixedLegPV_                         += fixedCashflowPV;
            fixedLegTimeWeightedPVs             += fixedCashflowPV * timeToFixedCouponPayment;
        }
    }


    //
    // Float Leg 
    // Note:    All the date vectors have an extra date for the start date, so dates index using j and other parameters j-1
    //          For the Fixing Dates however we need to use j-1 as an index to capture the start date fixing

    for( size_t j = 1; j < floatAccrualDates.size(); j++ )
	{			
	    if ( floatPaymentDates[j] >= asOf )
        {
            double floatAccrualPeriod           = floatLegAccrualDateYearFractions[j-1];
            double floatDiscFactor              = floatLegDiscFactors[j-1];
            double timeToFloatCouponPayment     = data_dc_act.getTerm( asOf, floatPaymentDates[j] );
            
            double floatCashflowPV              = notional * floatLegRates[j-1]   * floatLegAccrualDateYearFractions[j-1] * floatDiscFactor;
            double spreadCashflowPV             = notional * floatSpreadInPercent * floatLegAccrualDateYearFractions[j-1] * floatDiscFactor;

            floatLegPV_                         += ( floatCashflowPV + spreadCashflowPV );
            floatLegPVExcludingSpread_          += floatCashflowPV;
            spreadPV_                           += spreadCashflowPV;
        
            floatLegTimeWeightedPVs                 += floatCashflowPV  * timeToFloatCouponPayment;
            floatLegExcludingSpreadTimeWeightedPVs  += floatCashflowPV  * timeToFloatCouponPayment;
            spreadTimeWeightedPVs                   += spreadCashflowPV * timeToFloatCouponPayment;
        }
	}


    //
    // 5. Calculate Swap Par Rates
    // -------------------------------------------------------------------------------
    
    swapParRateTrade_                           = ( notional * AnnuityFixed_ == 0.0 ) ? 0.0 : floatLegPV_ / AnnuityFixed_;
    swapParRateMarket_                          = ( notional * AnnuityFixed_ == 0.0 ) ? 0.0 : floatLegPVExcludingSpread_ / AnnuityFixed_;


    //
    // 6. Calculate Macaulay's & Modified Duration Values ( by leg )
    // -------------------------------------------------------------------------------
    
    // Note: Macaulay's Duration Swap excluding Spread is used to evaluate the swap duration which is being priced as
    // Swap PV = Notional * [ ( Fixed Rate - Par Rate ) * Annuity(Fixed) - Spread * Annuity(Float) ]
    //
    // There Macaulay's Duration Swap must evaluate the time adjusted coupons from the fixed leg and float leg (excluding spread)
    // since the floating leg is considered a fixed leg with it's fixed rate being the swap par rate.

    macaulaysDurationSwapExcludingSpread_       = ( fixedLegPV_ - floatLegExcludingSpreadTimeWeightedPVs == 0.0 ) ? 0.0 
        : ( fixedLegTimeWeightedPVs - floatLegExcludingSpreadTimeWeightedPVs ) / ( fixedLegPV_ - floatLegPVExcludingSpread_ );
    
    modifiedDurationSwapExcludingSpread_        = ( 1.0 + swapParRateMarket_ == 0.0 ) ? 0.0 : macaulaysDurationSwapExcludingSpread_  / ( 1.0 + swapParRateMarket_ );
    
    if ( modifiedDurationSwapExcludingSpread_ < 0 )
    {
        modifiedDurationSwapExcludingSpread_ *= -1.0;
    }

    macaulaysDurationSpread_                    = ( spreadPV_ == 0.0 ) ? 0.0 : spreadTimeWeightedPVs / spreadPV_;
    modifiedDurationSpread_                     = ( 1.0 + swapParRateMarket_ == 0.0 ) ? 0.0 : macaulaysDurationSpread_ / ( 1.0 + swapParRateMarket_ );

    if ( modifiedDurationSpread_ < 0 )
    {
        modifiedDurationSpread_ *= -1.0;
    }


    //
    // 7. Calculate PV & Risk Totals
    // -------------------------------------------------------------------------------
    
    // payRecIndicator is an indicator function, taking a value of 1 for a receiver swap and -1 for a payer swap
    // i.e. payRecIndicator = 1 when receiving fixed coupons and -1 when paying fixed coupons
    double payRecIndicator = isFixedRatePayerSwap ? -1.0 : 1.0;
    
    swapPV_                                     = payRecIndicator * ( fixedLegPV_ - floatLegPV_ );
    swapPVExcludingSpread_                      = payRecIndicator * ( fixedLegPV_ - floatLegPVExcludingSpread_ );
    spreadPV_                                   = payRecIndicator * -1.0 * spreadPV_;
    swapPV01_                                   = AnnuityFixed_ * oneBasisPoint;

    swapDV01_                                   = payRecIndicator * swapPV01_ 
                                                + ( swapPVExcludingSpread_ * modifiedDurationSwapExcludingSpread_ * oneBasisPoint )
                                                + ( spreadPV_ * modifiedDurationSpread_ * oneBasisPoint );
}

double 
AQLMathYieldCurve::getParRate( const DateVector& fixedAccrualDates,
                            const DateVector& fixedPaymentDates,
                            const DateVector& floatFixingDates,
                            const DateVector& floatAccrualDates,
                            const DateVector& floatPaymentDates,
                            bool useFirstFixing,
                            double firstFixing,
                            bool useLastFixing,
                            double lastFixing,
                            AQLString fixedDaycount,
                            AQLString floatDaycount, 
                            AQLString foreCurveName,
                            AQLString dfCurveName,
                            bool isFWDInter,
                            double floatSpread,
                            bool useFwdData,
							bool isOIS,
							AQLString oisCompoundingType,
							AQLString calendar,
							AQLString interpolation,
							AQLString rollConvention,
							AQLString slidingRule
							)
{
	const AQLObject& YieldData       = getYieldData().get().get();
	const AQLDate& asOf              = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
    
    if ( floatPaymentDates[floatPaymentDates.size()-1] < asOf && fixedPaymentDates[fixedPaymentDates.size()-1] < asOf) 
        throw AQLCoreInvalidData("#Error: Par rate error; the underlying swap has expired.", __FILE__, __LINE__ );
    
    const AQLPriceDataCalendar& cal       = getCalendar();
	const AQLPriceDataSlidingRule& sr     = getSlidingRule();

	AQLDate fixedAccrualStartDate    = sr.getDate( fixedAccrualDates.front(), cal );
    AQLDate floatAccrualStartDate    = sr.getDate( floatAccrualDates.front(), cal );
    
    AQLDate fixedAccrualEndDate      = sr.getDate( fixedAccrualDates.back(), cal );
    AQLDate floatAccrualEndDate      = sr.getDate( floatAccrualDates.back(), cal );

	DayCount dc_act( ACT_365_ISDA );
	AQLPriceDataDayCount data_dc_act( dc_act );

    // Get client specified daycount conventions
    AQLPriceDataDayCount fixedLegDaycount;
    fixedLegDaycount.convertFromString( fixedDaycount );
        
    AQLPriceDataDayCount floatLegDaycount;
    floatLegDaycount.convertFromString( floatDaycount );

	setCurveType(dfCurveName);
	
    //
    // Calculate Fixed Leg Annuity value using client specified daycount convention
    double annuity = getAnnuity( fixedAccrualDates, fixedPaymentDates, fixedLegDaycount );
	
	// Calculate the floating coupon accrual periods
	DoubleArray accrualDateYearFractions( floatAccrualDates.size() -1 );  
	for( size_t i = 1; i < floatAccrualDates.size(); i++ )
	{
		// "accrualDateYearFractions" used for the coupon period
		accrualDateYearFractions[i-1] = floatLegDaycount.getTerm( floatAccrualDates[i-1], floatAccrualDates[i] );
	}

	size_t nextCashflowIndex = getNextCashFlowIndex(floatAccrualDates, asOf);

	double floaterPV = 0.;
	if (isOIS)
	{		
		std::string curveID(getName().convertToString().getCString());
		size_t pos = curveID.find(AQLMathCurveFuncUtility::YIELD_CURVE_NAME_PREFIX);
		if ( pos != std::string::npos)
		{
			size_t len = std::string(AQLMathCurveFuncUtility::YIELD_CURVE_NAME_PREFIX).length();
			curveID = curveID.substr(pos + len, curveID.length() - len - pos - 1); 
		}

		AQLString dayCount = getDayCount().convertToString();

		// Get equivalent rate over accrual periods
		DoubleArray equivalentRates( floatAccrualDates.size()  -1 );		
		getCompoundingRates(equivalentRates, 
							floatAccrualDates, 
							accrualDateYearFractions, 
							nextCashflowIndex,
							getDataInstance(),
							floatSpread,
							asOf,
							AQLString(curveID.c_str()),
							foreCurveName,							
							slidingRule,
							calendar,
							rollConvention,
							dayCount,
							interpolation,
							oisCompoundingType);
							
		// Get Discount Factors
		DoubleArray dfs  ( floatPaymentDates.size() -1 );
		setCurveType(dfCurveName);
		for( size_t i = 1; i < floatPaymentDates.size(); ++i )
		{
			const double term = data_dc_act.getTerm( asOf, floatPaymentDates[i] );

			// Calculate discount factors using PAYMENT DATES ***
			dfs[i-1] = ( i < nextCashflowIndex ) ? 0.0 : getDF( term, &dc_act );
		}		

		// Calculate Floater PV
		for( size_t i = 1; i < floatAccrualDates.size(); i++ )
		{			
			double rate             = equivalentRates[i-1];
			double accrualPeriod    = accrualDateYearFractions[i-1];
			double discFactor       = dfs[i-1];
            
			// Floating Spread is in Basis Points, which we convert to percent here
			floaterPV += rate * accrualPeriod * discFactor;
		}
	}
	else
	{		
		//
		// Calculate Floating Leg Value
		if (foreCurveName != dfCurveName)
		{
			AQLString            freq_forecast = ""; 
			AQLPriceDataCalendar      cal_forecast; 
			AQLPriceDataSlidingRule   sld_forecast; 
			AQLPriceDataDayCount      dc_forecast;
			AQLString            accessary_forecast = "";
		
			getCurveConvention( freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName );

			DoubleArray rates( floatFixingDates.size()  -1 );
			DoubleArray dfs  ( floatPaymentDates.size() -1 );
        
			DoubleArray fixingDateYearFractions( floatFixingDates.size() -1 );  
			DoubleArray payDateYearFractions( floatPaymentDates.size() -1 );  

			if ( floatFixingDates.size() != floatAccrualDates.size() || floatAccrualDates.size() != floatPaymentDates.size() )
				throw AQLCoreInvalidData("#Error: Floating schedule error. Inconsistent number of fixing, acrrual and payment dates.", __FILE__, __LINE__ );
         			
			// Get forward rates from interpolation
			if ( isFWDInter )
			{
				// Optional 'UseFwdData', False (default) = imply forward from discount factors, True = use forward data directly
				const AQLInterpolationBase &fwd_inter = getFWDInterpolation(&foreCurveName, useFwdData);
				for( size_t i = 1; i < floatAccrualDates.size(); ++i )
				{
					fixingDateYearFractions[i-1]            = dc_forecast.getTerm( floatFixingDates[i-1], floatFixingDates[i] );
					const double fixingDateYearFraction     = data_dc_act.getTerm( asOf, floatFixingDates[i-1] ); 
                
					// Apply the first fixing rate for the first stub, if needed
					if ( i == nextCashflowIndex && useFirstFixing )
					{
						rates[i-1] = firstFixing;
					}
					// Apply the last fixing rate for the last stub, if needed
					else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
					{
						rates[i-1] = lastFixing;
					}
					else
					{
						// *** Calculate rates using FIXING DATES ***
						rates[i-1] = ( i < nextCashflowIndex ) ? 0.0 : fwd_inter.value( fixingDateYearFraction );
                    
					}
				}
			}
			// Get forward rates implied from discount factors
			else
			{
				setCurveType( foreCurveName );
				for( size_t i = 1; i < floatPaymentDates.size(); ++i )
				{
					fixingDateYearFractions[i-1] = floatLegDaycount.getTerm( floatFixingDates[i-1], floatFixingDates[i] );
				
					// Apply the first fixing rate for the first stub, if needed
					if ( i == nextCashflowIndex && useFirstFixing )
					{
						rates[i-1] = firstFixing;
					}
					// Apply the last fixing rate for the last stub, if needed
					else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
					{
						rates[i-1] = lastFixing;
					}
					else
					{
						// *** Calculate rates using FIXING DATES ***
						// Note: set past rates to zero
						rates[i-1] = ( i  <nextCashflowIndex ) ? 0.0 : 
							( getDF( data_dc_act.getTerm( asOf, floatFixingDates[i-1] ), &dc_act ) 
							/ getDF( data_dc_act.getTerm( asOf, floatFixingDates[i] ), &dc_act ) - 1.) / fixingDateYearFractions[i-1];

						double thisRate         = rates[i-1];
						AQLDate thisFixingStart  = floatFixingDates[i-1];
						AQLDate thisFixingEnd    = floatFixingDates[i];
					}
				}
			}
		
			// Get Discount Factors
			setCurveType(dfCurveName);
			for( size_t i = 1; i < floatPaymentDates.size(); ++i )
			{
				const double term = data_dc_act.getTerm( asOf, floatPaymentDates[i] );

				// *** Calculate discount factors using PAYMENT DATES ***
				// Note: We must ignore past cashflows
				dfs[i-1] = ( i < nextCashflowIndex ) ? 0.0 : getDF( term, &dc_act );
			}

			// Calculate Floater PV
			for( size_t i = 1; i < floatAccrualDates.size(); i++ )
			{			
				double rate             = rates[i-1];
				double accrualPeriod    = accrualDateYearFractions[i-1];
				double discFactor       = dfs[i-1];
            
				// Floating Spread is in Basis Points, which we convert to percent here
				floaterPV += ( rate + ( floatSpread / 10000.0 ) ) * accrualPeriod * discFactor;
			}
		}
		else
		{
			// Note: Past Historic Cashflows not supported i.e. negative terms or yearFractions will return an error message.

			// Libor Discounting Scenario
			floaterPV   = getDF( data_dc_act.getTerm( asOf, floatAccrualStartDate ), &dc_act ) 
						- getDF( data_dc_act.getTerm( asOf, floatAccrualEndDate   ), &dc_act );
		}
	}

	return floaterPV / annuity;
}

double 
AQLMathYieldCurve::getSwapPV( const bool&            isFixedRatePayerSwap,
                           const double&          notional,
                           const double&          fixedRate,
                           const double&          floatSpreadInBasisPoints,
                           const DateVector&      fixedAccrualDates,
                           const DateVector&      fixedPaymentDates,
                           const DateVector&      floatFixingDates,
                           const DateVector&      floatAccrualDates,
                           const DateVector&      floatPaymentDates,
                           bool                   useFirstFixing,
                           double                 firstFixing,
                           bool                   useLastFixing,
                           double                 lastFixing,
                           AQLString               fixedDaycount,
                           AQLString               floatDaycount, 
                           AQLString               foreCurveName,
                           AQLString               dfCurveName,
                           bool                   isFWDInter,
                           bool                   useFwdData,
						   bool					  isOIS, 
						   AQLString				  oisCompoundingType, 
						   AQLString				  calendar, 
						   AQLString				  inputInterpolation, 
						   AQLString				  rollConvention, 
						   AQLString				  slidingRule)
{
	const AQLObject& YieldData       = getYieldData().get().get();
	const AQLDate& asOf              = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
    
    if ( floatPaymentDates[floatPaymentDates.size()-1] < asOf && fixedPaymentDates[fixedPaymentDates.size()-1] < asOf) 
        return 0.0; // Swap has expired return swap pv = 0.0
    
    const AQLPriceDataCalendar& cal       = getCalendar();
	const AQLPriceDataSlidingRule& sr     = getSlidingRule();

	AQLDate fixedAccrualStartDate    = sr.getDate( fixedAccrualDates.front(), cal );
    AQLDate floatAccrualStartDate    = sr.getDate( floatAccrualDates.front(), cal );
    
    AQLDate fixedAccrualEndDate      = sr.getDate( fixedAccrualDates.back(), cal );
    AQLDate floatAccrualEndDate      = sr.getDate( floatAccrualDates.back(), cal );

	DayCount dc_act( ACT_365_ISDA );
	AQLPriceDataDayCount data_dc_act( dc_act );

    // Get client specified daycount conventions
    AQLPriceDataDayCount fixedLegDaycount;
    fixedLegDaycount.convertFromString( fixedDaycount );
        
    AQLPriceDataDayCount floatLegDaycount;
    floatLegDaycount.convertFromString( floatDaycount );

	setCurveType(dfCurveName);
	
    //
    // Calculate Fixed Leg Annuity value using client specified daycount convention
    const double annuity = getAnnuity( fixedAccrualDates, fixedPaymentDates, fixedLegDaycount );
	
    //
    // Calculate Floating Leg Values
	AQLString            freq_forecast       = ""; 
	AQLPriceDataCalendar      cal_forecast; 
	AQLPriceDataSlidingRule   sld_forecast; 
	AQLPriceDataDayCount      dc_forecast;
	AQLString            accessary_forecast  = "";
		
	getCurveConvention( freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName );

    DoubleArray rates( floatFixingDates.size()  -1 );
    DoubleArray dfs  ( floatPaymentDates.size() -1 );
        
    DoubleArray fixingDateYearFractions( floatFixingDates.size() -1 );  
	DoubleArray accrualDateYearFractions( floatAccrualDates.size() -1 );  
    DoubleArray payDateYearFractions( floatPaymentDates.size() -1 );  

    if ( floatFixingDates.size() != floatAccrualDates.size() || floatAccrualDates.size() != floatPaymentDates.size() )
        throw AQLCoreInvalidData("#Error: Floating schedule error. Inconsistent number of fixing, acrrual and payment dates.", __FILE__, __LINE__ );
         
    // Calculate the floating coupon accrual periods
    for( size_t i = 1; i < floatAccrualDates.size(); i++ )
    {
        // "accrualDateYearFractions" used for the coupon period
        accrualDateYearFractions[i-1] = floatLegDaycount.getTerm( floatAccrualDates[i-1], floatAccrualDates[i] );
    }

    // Find the index for the next cashflow, so that we can ignore cashflows in the past
    unsigned int nextCashflowIndex = 1;
    for (unsigned int i = 0; i < floatAccrualDates.size(); ++i)
    {
        if ( i == floatAccrualDates.size()-1  )
        {
            if ( floatAccrualDates[i] < asOf ) nextCashflowIndex++;
            break; 
        }
        if ( floatAccrualDates[i+1] >= asOf ) break;
        nextCashflowIndex++;
    }

	const double oneBasisPoint = 0.0001;

    // Get forward rates from interpolation
	if(isOIS)
	{
		std::string curveID(getName().convertToString().getCString());
		size_t pos = curveID.find(AQLMathCurveFuncUtility::YIELD_CURVE_NAME_PREFIX);
		if ( pos != std::string::npos)
		{
			size_t len = std::string(AQLMathCurveFuncUtility::YIELD_CURVE_NAME_PREFIX).length();
			curveID = curveID.substr(pos + len, curveID.length() - len - pos - 1); 
		}

		AQLString dayCount = getDayCount().convertToString();

		// Get equivalent rate over accrual periods
		getCompoundingRates(rates, 
							floatAccrualDates, 
							accrualDateYearFractions, 
							nextCashflowIndex,
							getDataInstance(),
							floatSpreadInBasisPoints * oneBasisPoint,
							asOf,
							AQLString(curveID.c_str()),
							foreCurveName,							
							slidingRule,
							calendar,
							rollConvention,
							dayCount,
							inputInterpolation,
							oisCompoundingType);
	}
	else
	{
		if ( isFWDInter )
		{
			//const AQLInterpolationBase &fwd_inter = getFWDInterpolation(&foreCurveName);
			const AQLInterpolationBase &fwd_inter = getFWDInterpolation(&foreCurveName, useFwdData);
	
			for( size_t i = 1; i < floatAccrualDates.size(); i++ )
			{
				fixingDateYearFractions[i-1]            = dc_forecast.getTerm( floatFixingDates[i-1], floatFixingDates[i] );
				const double fixingDateYearFraction     = data_dc_act.getTerm( asOf, floatFixingDates[i-1] ); 
                
				// Apply the first fixing rate for the first stub, if needed
				if ( i == nextCashflowIndex && useFirstFixing )
				{
					rates[i-1] = firstFixing;
				}
				// Apply the last fixing rate for the last stub, if needed
				else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
				{
					rates[i-1] = lastFixing;
				}
				else
				{
					// *** Calculate rates using FIXING DATES ***
					rates[i-1] = (i<nextCashflowIndex) ? 0.0 : fwd_inter.value( fixingDateYearFraction );
				}
			}
		}
		// Get forward rates implied from discount factors
		else
		{
			setCurveType( foreCurveName );
			for( size_t i = 1; i < floatPaymentDates.size(); i++ )
			{
				fixingDateYearFractions[i-1] = floatLegDaycount.getTerm( floatFixingDates[i-1], floatFixingDates[i] );
				
				// Apply the first fixing rate for the first stub, if needed
				if ( i == nextCashflowIndex && useFirstFixing )
				{
					rates[i-1] = firstFixing;
				}
				// Apply the last fixing rate for the last stub, if needed
				else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
				{
					rates[i-1] = lastFixing;
				}
				else
				{
					// *** Calculate rates using FIXING DATES ***
					rates[i-1] = (i<nextCashflowIndex) ? 0.0 :
						( getDF( data_dc_act.getTerm( asOf, floatFixingDates[i-1] ), &dc_act ) 
						/ getDF( data_dc_act.getTerm( asOf, floatFixingDates[i] ), &dc_act ) - 1.) / fixingDateYearFractions[i-1];
				}
			}
		}
	}
		
    // Get Discount Factors
	setCurveType(dfCurveName);
	for( size_t i = 1; i < floatPaymentDates.size(); i++ )
	{
        // *** Calculate discount factors using PAYMENT DATES ***
		dfs[i-1] = (i<nextCashflowIndex) ? 0.0 : getDF( data_dc_act.getTerm( asOf, floatPaymentDates[i] ), &dc_act );
	}

    // PV Parameter Specification
	double swapPV                           = 0.0;
    double fixedLegPV                       = 0.0;
    double floatLegPV                       = 0.0;    

    // PV from Fixed Leg
    fixedLegPV                              = fixedRate * annuity;

    // PV from Float Leg
	for( size_t i = 1; i < floatAccrualDates.size(); i++ )
	{			
		double floatRatePlusSpread          = rates[i-1] + ( floatSpreadInBasisPoints * oneBasisPoint );
        double floatAccrualPeriod           = accrualDateYearFractions[i-1];
        double floatDiscFactor              = dfs[i-1];
            
        floatLegPV += floatRatePlusSpread * floatAccrualPeriod * floatDiscFactor;
	}

    // Swap PV
    swapPV    = notional * ( fixedLegPV - floatLegPV );
    
    if ( isFixedRatePayerSwap )
        swapPV = -swapPV;

	return swapPV;
}

double 
AQLMathYieldCurve::getSwapDV01( bool                   isFixedRatePayerSwap,
                             double                 notional,
                             double                 fixedRate,
                             double                 floatSpreadInBasisPoints,
                             const DateVector&      fixedAccrualDates,
                             const DateVector&      fixedPaymentDates,
                             const DateVector&      floatFixingDates,
                             const DateVector&      floatAccrualDates,
                             const DateVector&      floatPaymentDates,
                             bool                   useFirstFixing,
                             double                 firstFixing,
                             bool                   useLastFixing,
                             double                 lastFixing,
                             AQLString               fixedDaycount,
                             AQLString               floatDaycount, 
                             AQLString               foreCurveName,
                             AQLString               dfCurveName,
                             bool                   isFWDInter,
						     bool					isOIS, 
						     AQLString				oisCompoundingType, 
						     AQLString				calendar, 
						     AQLString				inputInterpolation, 
						     AQLString				rollConvention, 
						     AQLString				slidingRule )
{


	calculateSwapPriceAndRisk( isFixedRatePayerSwap,      
                               notional,                  
                               fixedRate,                 
                               floatSpreadInBasisPoints,  
                               fixedAccrualDates,         
                               fixedPaymentDates,         
                               floatFixingDates,          
                               floatAccrualDates,         
                               floatPaymentDates,         
                               useFirstFixing,            
                               firstFixing,               
                               useLastFixing,             
                               lastFixing,                
                               fixedDaycount,             
                               floatDaycount,             
                               foreCurveName,       
                               dfCurveName,
                               isFWDInter,
							   isOIS,
							   oisCompoundingType,
							   calendar,
							   inputInterpolation,
							   rollConvention,
							   slidingRule );

    return swapDV01_;
}

double 
AQLMathYieldCurve::getSwapPV01( const bool&            isFixedRatePayerSwap,
                             const double&          notional,
                             const DateVector&      fixedAccrualDates,
                             const DateVector&      fixedPaymentDates,
                             AQLString               fixedDaycount,
                             AQLString               foreCurveName,
                             AQLString               dfCurveName )
{
	const AQLObject& YieldData       = getYieldData().get().get();
	const AQLDate& asOf              = dynamic_cast<const AQLDataDate&> ( ( YieldData.getData( CALIBRATION_DATA_ASOFDATE, ISNOTNULL ) ).get() ).get();
    const AQLPriceDataCalendar& cal       = getCalendar();
	const AQLPriceDataSlidingRule& sr     = getSlidingRule();

	AQLDate fixedAccrualStartDate    = sr.getDate( fixedAccrualDates.front(), cal );
    AQLDate fixedAccrualEndDate      = sr.getDate( fixedAccrualDates.back(), cal );

	DayCount dc_act( ACT_365_ISDA );
	AQLPriceDataDayCount data_dc_act( dc_act );

    // Get client specified daycount conventions
    AQLPriceDataDayCount fixedLegDaycount;
    fixedLegDaycount.convertFromString( fixedDaycount );

	setCurveType(dfCurveName);
	
    // Calculate Fixed Leg Annuity value using client specified daycount convention
    const double annuity = getAnnuity( fixedAccrualDates, fixedPaymentDates, fixedLegDaycount );
	
    // PV01 Parameter Specification
    double          swapPV01                = 0.0;
    double          fixedLegPV01            = 0.0;
    const double    oneBasisPoint           = 0.0001;
    
    // PV01 from Fixed Leg
    fixedLegPV01                            = oneBasisPoint * annuity; 

    // Swap PV01
    swapPV01    = notional * fixedLegPV01;

	return swapPV01;
}

double 
AQLMathYieldCurve::getAssetSwapSpread( const double&          bondPrice,
                                    const double&          fixedRate,
                                    const DateVector&      fixedAccrualDates,
                                    const DateVector&      fixedPaymentDates,
                                    const DateVector&      floatFixingDates,
                                    const DateVector&      floatAccrualDates,
                                    const DateVector&      floatPaymentDates,
                                    bool                   useFirstFixing,
                                    double                 firstFixing,
                                    bool                   useLastFixing,
                                    double                 lastFixing,
                                    AQLString               fixedDaycount,
                                    AQLString               floatDaycount, 
                                    AQLString               foreCurveName,
                                    AQLString               dfCurveName,
                                    bool                   isFWDInter,
                                    bool                   isCleanPrice,
                                    const AQLDate&          settlementDate )
{
    const AQLObject& YieldData       = getYieldData().get().get();
	const AQLDate& asOf              = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
    
    if ( floatPaymentDates[floatPaymentDates.size()-1] < asOf && fixedPaymentDates[fixedPaymentDates.size()-1] < asOf) 
        throw AQLCoreInvalidData("#Error: Asset swap spread error; the underlying swap has expired.", __FILE__, __LINE__ );
    
    const AQLPriceDataCalendar& cal       = getCalendar();
	const AQLPriceDataSlidingRule& sr     = getSlidingRule();

	AQLDate fixedAccrualStartDate    = sr.getDate( fixedAccrualDates.front(), cal );
    AQLDate floatAccrualStartDate    = sr.getDate( floatAccrualDates.front(), cal );
    
    AQLDate fixedAccrualEndDate      = sr.getDate( fixedAccrualDates.back(), cal );
    AQLDate floatAccrualEndDate      = sr.getDate( floatAccrualDates.back(), cal );

	DayCount dc_act( ACT_365_ISDA );
	AQLPriceDataDayCount data_dc_act( dc_act );

    // Get client specified daycount conventions
    AQLPriceDataDayCount fixedLegDaycount;
    fixedLegDaycount.convertFromString( fixedDaycount );
        
    AQLPriceDataDayCount floatLegDaycount;
    floatLegDaycount.convertFromString( floatDaycount );

	setCurveType(dfCurveName);
	
    //
    // Calculate Fixed Leg Annuity value using client specified daycount convention
    // Note: We deduct accrued interest when working with a dirty bond price

    // Asset Swap Calculations and Accrued Interest
    // --------------------------------------------
    // When working with the dirty bond price we need to deduct accrued interest from the Bond fixed leg
	const bool deductAccruedInterest = true;
    const double fixedAnnuity = getAnnuity( fixedAccrualDates, fixedPaymentDates, fixedLegDaycount, settlementDate, deductAccruedInterest, isCleanPrice );
	
    //
    // Calculate Floating Leg Values
	AQLString            freq_forecast       = ""; 
	AQLPriceDataCalendar      cal_forecast; 
	AQLPriceDataSlidingRule   sld_forecast; 
	AQLPriceDataDayCount      dc_forecast;
	AQLString            accessary_forecast  = "";
		
	getCurveConvention( freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName );

    DoubleArray rates( floatFixingDates.size()  -1 );
    DoubleArray dfs  ( floatPaymentDates.size() -1 );
        
    DoubleArray fixingDateYearFractions( floatFixingDates.size() -1 );  
	DoubleArray accrualDateYearFractions( floatAccrualDates.size() -1 );  
    DoubleArray payDateYearFractions( floatPaymentDates.size() -1 );  

    if ( floatFixingDates.size() != floatAccrualDates.size() || floatAccrualDates.size() != floatPaymentDates.size() )
        throw AQLCoreInvalidData("#Error: Floating schedule error. Inconsistent number of fixing, acrrual and payment dates.", __FILE__, __LINE__ );

    // Calculate the floating coupon accrual periods
    for( size_t i = 1; i < floatAccrualDates.size(); i++ )
    {
        // "accrualDateYearFractions" used for the coupon period
        accrualDateYearFractions[i-1]           = floatLegDaycount.getTerm( floatAccrualDates[i-1], floatAccrualDates[i] );
    }

    // Find the index for the next cashflow, so that we can ignore cashflows in the past
    unsigned int nextCashflowIndex = 1;
    for (unsigned int i = 0; i < floatAccrualDates.size(); ++i)
    {
        if ( i == floatAccrualDates.size()-1  )
        {
            if ( floatAccrualDates[i] < asOf ) 
                throw AQLCoreInvalidData("#Error: Asset swap spread error; the underlying swap has expired.", __FILE__, __LINE__ );
			
			nextCashflowIndex++;                
            break; 
        }
        if ( floatAccrualDates[i+1] >= asOf ) break;
        nextCashflowIndex++;
    }

    // Get forward rates from interpolation
	if ( isFWDInter )
	{
		const AQLInterpolationBase &fwd_inter = getFWDInterpolation(&foreCurveName);
		for( size_t i = 1; i < floatAccrualDates.size(); i++ )
		{
            fixingDateYearFractions[i-1]            = dc_forecast.getTerm( floatFixingDates[i-1], floatFixingDates[i] );
			const double fixingDateYearFraction     = data_dc_act.getTerm( asOf, floatFixingDates[i-1] ); 
                
            // Apply the first fixing rate for the first stub, if needed
            if ( i == nextCashflowIndex && useFirstFixing )
            {
                rates[i-1] = firstFixing;
            }
            // Apply the last fixing rate for the last stub, if needed
            else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
            {
                rates[i-1] = lastFixing;
            }
            else
            {
                // *** Calculate rates using FIXING DATES ***
                rates[i-1] = (i<nextCashflowIndex) ? 0.0 : fwd_inter.value( fixingDateYearFraction );
            }
		}
	}
    // Get forward rates implied from discount factors
	else
	{
		setCurveType( foreCurveName );
		for( size_t i = 1; i < floatPaymentDates.size(); i++ )
		{
			fixingDateYearFractions[i-1] = floatLegDaycount.getTerm( floatFixingDates[i-1], floatFixingDates[i] );
				
            // Apply the first fixing rate for the first stub, if needed
            if ( i == nextCashflowIndex && useFirstFixing )
            {
                rates[i-1] = firstFixing;
            }
            // Apply the last fixing rate for the last stub, if needed
            else if ( i == floatAccrualDates.size() - 1 && useLastFixing )
            {
                rates[i-1] = lastFixing;
            }
            else
            {
                // *** Calculate rates using FIXING DATES ***
                rates[i-1]  = (i<nextCashflowIndex) ? 0.0 : 
                    ( getDF( data_dc_act.getTerm( asOf, floatFixingDates[i-1] ), &dc_act ) 
                    / getDF( data_dc_act.getTerm( asOf, floatFixingDates[i] ), &dc_act ) - 1.) / fixingDateYearFractions[i-1];
            }
		}
	}
		
    // Get Discount Factors
	setCurveType(dfCurveName);
	for( size_t i = 1; i < floatPaymentDates.size(); i++ )
	{
        // *** Calculate discount factors using PAYMENT DATES ***
		dfs[i-1] = (i<nextCashflowIndex) ? 0.0 : getDF( data_dc_act.getTerm( asOf, floatPaymentDates[i] ), &dc_act );
	}

    //
    // Asset Swap Spread Parameter Specification
	double          swapSpread                  = 0.0;
    double          fixedLegPV                  = 0.0;
    double          floatLegPV                  = 0.0;
    double          parParAdjustmentInPercent   = 0.0;
    double          floatAnnuity                = 0.0;
    const double    oneBasisPoint               = 0.0001;

    //
    // Fixed Leg PV
    fixedLegPV                                  = fixedRate * fixedAnnuity;

    //
    // Float Leg PV
    for( size_t i = 1; i < floatAccrualDates.size(); i++ )
	{			
		double floatRate                        = rates[i-1];
        double floatAccrualPeriod               = accrualDateYearFractions[i-1];
        double floatDiscFactor                  = dfs[i-1];
        double floatAccruedInterest             = 0.0;

        floatLegPV                              += floatRate * floatAccrualPeriod * floatDiscFactor;
        floatAnnuity                            += floatAccrualPeriod * floatDiscFactor;
	}

    //
    // Par Par Adjustment
    const double Par                            = 100.0;
    parParAdjustmentInPercent                   = ( Par - bondPrice ) / 100.0;

    //
    // Asset Swap Spread
    if ( floatAnnuity == 0 )
        throw AQLCoreInvalidData("#Error: Floating schedule error. The float leg annuity value cannot be zero.", __FILE__, __LINE__ );

    // Note: Accrued Interest is deducted from the fixedLegPV ( via the Fixed Annuity ) when working with the dirty bond price
    swapSpread    = ( fixedLegPV - floatLegPV + parParAdjustmentInPercent ) / floatAnnuity;
    
	return swapSpread / oneBasisPoint;
}


/*!
    @brief get ParRate by DateCalcUtility 
			
	@param[in] fromDate	fromDate
	@param[in] toDate	toDate
	@param[in] firstStubDate	front odd date
	@param[in] lastStubDate	last odd date
	@param[in] pday		payment day
	
	@return ParRate
*/

double
AQLMathYieldCurve::getParRate(const AQLDate& fromDate, const AQLDate& toDate, 
						   const AQLDate* firstStubDate, const AQLDate* lastStubDate, const int* pday, AQLString foreCurveName, 
						   AQLString dfCurveName, bool isFWDInter, const AQLString* roll_convention)
{
	DateVector out;
	AQLMathDateCalculations::generateSchedule(fromDate, toDate, getFrequency().get(),
							true, firstStubDate, lastStubDate, pday, out, &getSlidingRule(), &getCalendar(), true, roll_convention);
	if(out.front() != fromDate)
		out.insert(out.begin(), fromDate);

	DateVector dates_float;
	//if (foreCurveName != STD || dfCurveName != STD)
	if (foreCurveName != dfCurveName)
	{
		AQLString freq_forecast = ""; 
		AQLPriceDataCalendar cal_forecast; 
		AQLPriceDataSlidingRule sld_forecast; 
		AQLPriceDataDayCount dc_forecast;
		AQLString accessary_forecast = "";
		
		getCurveConvention(freq_forecast, cal_forecast, sld_forecast, dc_forecast, accessary_forecast, foreCurveName);
		if (accessary_forecast == "" || dc_forecast.isNull())
		{
			throw AQLCoreInvalidData("getParRate failed. Check whether daycount and frequency were set in forecast curve!!",__FILE__,__LINE__);
		}

		AQLString freq_float = "";
		if (accessary_forecast == AQLString("12M")) freq_float = ANNUAL;
		else if (accessary_forecast == AQLString("6M")) freq_float = SEMI_ANNUAL;
		else if (accessary_forecast == AQLString("3M")) freq_float = QUARTERLY;
		else if (accessary_forecast == AQLString("1M")) freq_float = MONTHLY;
		else throw AQLCoreInvalidData("getParRate failed. Check accessary of forecast curve!!",__FILE__,__LINE__);

		AQLMathDateCalculations::generateSchedule(fromDate, toDate, freq_float,
								true, firstStubDate, lastStubDate, pday, dates_float, &getSlidingRule(), &getCalendar(), true, roll_convention);
		if(dates_float.front() != fromDate)
			dates_float.insert(dates_float.begin(), fromDate);
	}

	return getParRate(out, dates_float, foreCurveName, dfCurveName, isFWDInter);
}
/*!
    @brief get ParRate by DateCalcUtility 
			
	@param[in] fromDate	fromDate
	@param[in] term_str	term
	@param[in] firstStubDate	front odd date
	@param[in] lastStubDate	last odd date
	@param[in] pday		payment day
	
	@return ParRate
*/

double
AQLMathYieldCurve::getParRate(const AQLDate& fromDate, const AQLString& term_str, 
						   const AQLDate* firstStubDate, const AQLDate* lastStubDate, const int* pday, 
						   AQLString foreCurveName, AQLString dfCurveName, bool isFWDInter, const AQLString* roll_convention)
{
	AQLDate toDate = AQLMathDateCalculations::getDate(fromDate, term_str, true, roll_convention);
	return getParRate(fromDate, toDate, firstStubDate, lastStubDate, pday, foreCurveName, dfCurveName, isFWDInter, roll_convention);
}

/*!
    @brief get DF from fromDate to toDate
			
	@param[in] fromDate StartDate
	@param[in] toDate	EndDate

	@return DiscountFactor
*/

double        
AQLMathYieldCurve::getDF(const AQLDate& fromDate, const AQLDate& toDate) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&>((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	
	if (fromDate > toDate)
	{
		AQLString err = "#Error: Unable to calculate a discount factor in the past. The toDate must be after fromDate";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	//if (fromDate < asOf)
	//{
	//	AQLString err = "fromDate must be after asOf";
	//	throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	//}
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();
	AQLDate fdate = (fromDate < asOf) ? asOf : fromDate;
	AQLDate tdate = (toDate < asOf) ? asOf : toDate;
	if(!cal.isNull() && !sr.isNull())
	{
		if (sr.getDate(fdate, cal) >= asOf) fdate = sr.getDate(fdate, cal);
		if (sr.getDate(tdate, cal) >= asOf) tdate = sr.getDate(tdate, cal);
	}
	
	if (fdate == tdate) return 1.0;

	AQLPriceDataDayCount dc(getDayCount());
	double dff = getDF(dc.getTerm(asOf, fdate));
	double dft = getDF(dc.getTerm(asOf, tdate));
	
	return dft / dff;
}

double        
AQLMathYieldCurve::getBasisDF(const AQLDate& fromDate, const AQLDate& toDate) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&>((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	
	if (fromDate > toDate)
	{
        AQLString err = "#Error: Unable to calculate a discount factor in the past. The toDate must be after fromDate";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
 //   if (fromDate < asOf)
	//{
	//	AQLString err = "fromDate must be after asOf";
	//	throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	//}
	
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();
	AQLDate fdate = (fromDate < asOf) ? asOf : fromDate;
	AQLDate tdate = (toDate < asOf) ? asOf : toDate;
	if(!cal.isNull() && !sr.isNull())
	{
		if (sr.getDate(fdate, cal) >= asOf) fdate = sr.getDate(fdate, cal);
		if (sr.getDate(tdate, cal) >= asOf) tdate = sr.getDate(tdate, cal);
	}
	
	if (fdate == tdate) return 1.0;

	AQLPriceDataDayCount dc(getDayCount());
	double dff = getBasisDF(dc.getTerm(asOf, fdate));
	double dft = getBasisDF(dc.getTerm(asOf, tdate));
	
	return dft / dff;
}

/*!
    @brief get DF for specified term from fromDate 
			
	@param[in] fromDate StartDate
	@param[in] term		Term
	
	@return DiscountFactor
*/

double        
AQLMathYieldCurve::getDF(const AQLDate& fromDate, const double& term) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

	if (term < 0.0)
	{
        AQLString err = "#Error: Unable to calculate a discount factor in the past";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	//if (fromDate < asOf)
	//{
	//	AQLString err = "fromDate must be after asOf";
	//       throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	//}
	AQLPriceDataDayCount dc(getDayCount());
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();

	AQLDate fdate = (fromDate < asOf) ? asOf : fromDate;
	if(!cal.isNull() && !sr.isNull())
	{
		if (sr.getDate(fdate, cal) >= asOf) fdate = sr.getDate(fdate, cal);
	}
	
	if (term < EPS) return 1.0;

	double termTmp = dc.getTerm(asOf, asOf.intervalDays(fromDate) + dc.getDayTerm(fromDate, term));
	double dff = getDF(asOf, fdate);
	double dft = getDF(termTmp);
	
	return dft / dff;
}

double        
AQLMathYieldCurve::getBasisDF(const AQLDate& fromDate, const double& term) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

	if (term < 0.0)
	{
        AQLString err = "#Error unable to calculate a discount factor in the past";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	//if (fromDate < asOf)
	//{
	//	AQLString err = "fromDate must be after asOf";
 //       throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	//}
	AQLPriceDataDayCount dc(getDayCount());
	const AQLPriceDataCalendar& cal = getCalendar();
	const AQLPriceDataSlidingRule& sr = getSlidingRule();

	AQLDate fdate = (fromDate < asOf) ? asOf : fromDate;
	if(!cal.isNull() && !sr.isNull())
	{
		if (sr.getDate(fdate, cal) >= asOf) fdate = sr.getDate(fdate, cal);
	}
	
	if (term < EPS) return 1.0;

	double termTmp = dc.getTerm(asOf, asOf.intervalDays(fromDate) + dc.getDayTerm(fromDate, term));
	double dff = getBasisDF(asOf, fdate);
	double dft = getBasisDF(termTmp);
	
	return dft / dff;
}

/*!
    @brief get DF for specified term from AsOfDate 
			
	@param[in] term Term

	@return DiscountFactor
double
AQLMathYieldCurve::getDF(const double term) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	if (mCurveVersion != getModel())
	{
		const DoubleArray& terms = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
		const DoubleArray& dfs   = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL)).get()).get();
		dynamic_cast<AQLPriceDataInterpolation&>(mpInter->get()).set(terms, dfs);
		mCurveVersion = getModel();
	}
	AQLPriceDataDayCount dc(getDayCount()), dc_act;
	dc_act.setDayCount(ACT_365_ISDA);

	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	double termTmp = dc_act.getTerm(asOf, dc.getDayTerm(asOf, term));
	
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).value(termTmp);
}
*/

/*!
    @brief get DF for specified term from AsOfDate 
	@param[in] term Term
	@return DiscountFactor
*/

double
AQLMathYieldCurve::getDF(const double term, const DayCount* pdc) const
{
	const AQLObject& YieldData = getYieldData().get().get();

	const AQLString interpmethod = getDFInterpolationMethod().get();
	bool isinterprateterm = false;
	if (interpmethod == "DiscountFactor")
		isinterprateterm = false;
	else if (interpmethod == "RateTerm")
		isinterprateterm = true;
	else
		throw AQLCoreInvalidData("DFInterpolation Error",__FILE__,__LINE__);
	
    if (mCurveVersionMap[mCurveType] != getModel())
	{
		setUpInterpolation();
	}

	if (term < 0.0)
	{
        AQLString err = "#Error: Unable to calculate a discount factor in the past";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (term < EPS) return 1.0;

	double ret = 1.0;
	if(pdc != 0 && *pdc == ACT_365_ISDA)
	{
		ret = getCurveTypeInterpolation(mCurveType).value(term);
		ret = (isinterprateterm) ? AQLMath::exp(-ret) : ret;
		return ret;
	}

	if(pdc == 0 && getDayCount().getDayCount() == ACT_365_ISDA)
	{
		ret = getCurveTypeInterpolation(mCurveType).value(term);
		ret = (isinterprateterm) ? AQLMath::exp(-ret) : ret;
		return ret;
	}
		
	
	AQLPriceDataDayCount dc, dc_act;

	if(pdc == 0)
		dc.setDayCount(getDayCount().getDayCount());
	else 
		dc.setDayCount(*pdc);

	dc_act.setDayCount(ACT_365_ISDA);

	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	double termTmp = dc_act.getTerm(asOf, dc.getDayTerm(asOf, term));
	
	ret =  getCurveTypeInterpolation(mCurveType).value(termTmp);
	ret = (isinterprateterm) ? AQLMath::exp(-ret) : ret;
	return ret;
	
	
	/*if(pdc != 0 && *pdc == ACT_365_ISDA)
		return dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).value(term);
	
	if(pdc == 0 && getDayCount().getDayCount() == ACT_365_ISDA)
		return dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).value(term);
	
	AQLPriceDataDayCount dc, dc_act;

	if(pdc == 0)
		dc.setDayCount(getDayCount().getDayCount());
	else 
		dc.setDayCount(*pdc);

	dc_act.setDayCount(ACT_365_ISDA);

	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	double termTmp = dc_act.getTerm(asOf, dc.getDayTerm(asOf, term));
	
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).value(termTmp);*/
}

/*!
    @brief get BasisDF for specified term from AsOfDate 
			
	@param[in] term Term

	@return DiscountFactor
*/

double
AQLMathYieldCurve::getBasisDF(const double term, const DayCount* pdc) const
{
	const AQLObject& YieldData = getYieldData().get().get();

	const AQLString interpmethod = getDF2InterpolationMethod().get();
	bool isinterprateterm = false;
	if (interpmethod == "DiscountFactor")
		isinterprateterm = false;
	else if (interpmethod == "RateTerm")
		isinterprateterm = true;
	else
		throw AQLCoreInvalidData("DFInterpolation Error",__FILE__,__LINE__);

	const AQLDataHolder* dh;
	dh = &(YieldData.getData(IR_CALIBRATION_DATA_DFS2,NOCHECK));
	if(!dh->isDefined() || dh->isNull())
		return getDF(term,pdc);

	AQLString curveName = IR_CALIBRATION_DATA_DFS2;
	if (mpBasisCurveType != NULL)
	{
		curveName = *mpBasisCurveType;
	}

    //if (mCurveVersionMap[IR_CALIBRATION_DATA_DFS2] != getModel())
	if (mCurveVersionMap[curveName] != getModel())
	{
		setUpInterpolation();
	}

	if (term < 0.0)
	{
        AQLString err = "#Error: Unable to calculate a discount factor in the past";
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (term < EPS) return 1.0;

	double ret = 1.0;
	if(pdc != 0 && *pdc == ACT_365_ISDA)
	{
		//ret = getCurveTypeInterpolation(IR_CALIBRATION_DATA_DFS2).value(term);
		ret = getCurveTypeInterpolation(curveName).value(term);
		ret = (isinterprateterm) ? AQLMath::exp(-ret) : ret;
		return ret;
	}

	if(pdc == 0 && getDayCount().getDayCount() == ACT_365_ISDA)
	{
		//ret = getCurveTypeInterpolation(IR_CALIBRATION_DATA_DFS2).value(term);
		ret = getCurveTypeInterpolation(curveName).value(term);
		ret = (isinterprateterm) ? AQLMath::exp(-ret) : ret;
		return ret;
	}

	AQLPriceDataDayCount dc, dc_act;

	if(pdc == 0)
		dc.setDayCount(getDayCount().getDayCount());
	else 
		dc.setDayCount(*pdc);

	dc_act.setDayCount(ACT_365_ISDA);

	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	double termTmp = dc_act.getTerm(asOf, dc.getDayTerm(asOf, term));
	
	//ret = getCurveTypeInterpolation(IR_CALIBRATION_DATA_DFS2).value(termTmp);
	ret = getCurveTypeInterpolation(curveName).value(termTmp);
	ret = (isinterprateterm) ? AQLMath::exp(-ret) : ret;
	return ret;
}


/*!
    @brief get raw DFs and terms
			
	@param[out] terms Terms
	@param[out] dfs raw DFs

*/

void
AQLMathYieldCurve::getRawDFs(DoubleArray& terms, DoubleArray& dfs) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const DoubleArray& _terms = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL)).get()).get();
	const DoubleArray& _dfs   = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(IR_CALIBRATION_DATA_DFS + mCurveSuffix, ISNOTNULL)).get()).get();
	terms = _terms;
	dfs = _dfs;
}

/*!
    @brief Instatiate interpolator object by setting indexes and values to it.
	       Also set extra interpolation paramters needed for linear-spline interpolation et al
*/
void
AQLMathYieldCurve::setValuesToInterpolator( const AQLString &curveType, const DoubleArray &index, const DoubleArray &value ) const
{	
	AQLPriceDataInterpolation* pInterpolationAttr;
	std::map<AQLString, AQLPriceDataInterpolation *>::const_iterator it = mDFInterMap.find(curveType);

	bool setUpInterp = true;	
	if (it != mDFInterMap.end())
	{
		// When trying to use an existing DF interpolator, not only check its existence 
		// but also check if the interpolator has been changed to a different scheme
		const AQLPriceDataInterpolation &inter = getInterpolation();
		if (inter.convertToString().toUpper() == it->second->convertToString().toUpper())
		{
			pInterpolationAttr = it->second;
            setUpInterp = false;
		}
	}

	if (setUpInterp)
	{
		addDFInterpolation(curveType);
		pInterpolationAttr = mDFInterMap[curveType];
	}

    setValuesToInterpolator( pInterpolationAttr, curveType, index, value);
}

/*!
    @brief Instatiate interpolator object by setting indexes and values to it.
	       Also set extra interpolation paramters needed for linear-spline interpolation et al
*/
void 
AQLMathYieldCurve::setValuesToInterpolator( AQLPriceDataInterpolation* pInter, const AQLString &curveType, const DoubleArray &index, const DoubleArray &value ) const
{
    // Linear-Spline only works with the STD / Basis Yield Curve
    	
	if ( pInter->isHybrid() )
	{
        // Linear-Spline Interpolation Requires the Join Date to be initialized
        const AQLObject& yieldData = getYieldData().get().get();
        
		// Get curve suffix
		AQLString suffix = (curveType == "STD") ? "" : AQLString( "_" ) + curveType;

		// Use ISDEFINED i.e. throw if join date not set
		const AQLDataHolder* cutOffAttr = &(yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK));
		if(cutOffAttr->isDefined() && !cutOffAttr->isNull())
		{
			double cuttOffDate = dynamic_cast<const AQLDataDouble&> (cutOffAttr->get()).get();
			pInter->setJoinDateAsDouble(cuttOffDate);
		}
		else
		{
			double cuttOffDate = 0.0;
			pInter->setJoinDateAsDouble(cuttOffDate);
		}
    }

    pInter->set( index, value );
}

/*!
    @brief setup interpolation

*/
void
AQLMathYieldCurve::setUpInterpolation() const
{

	const AQLString interpmethod = getDFInterpolationMethod().get();
	bool isinterprateterm = false;
	if (interpmethod == "DiscountFactor")
		isinterprateterm = false;
	else if (interpmethod == "RateTerm")
		isinterprateterm = true;
	else
		throw AQLCoreInvalidData("DFInterpolation Error",__FILE__,__LINE__);

	const AQLString interpmethod2 = getDF2InterpolationMethod().get();
	bool isinterprateterm2 = false;
	if (interpmethod2 == "DiscountFactor")
		isinterprateterm2 = false;
	else if (interpmethod2 == "RateTerm")
		isinterprateterm2 = true;
	else
		throw AQLCoreInvalidData("DFInterpolation Error",__FILE__,__LINE__);


	const AQLObject& YieldData = getYieldData().get().get();
	const DoubleArray& terms = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL)).get()).get();
	const DoubleArray& dfs   = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(IR_CALIBRATION_DATA_DFS + mCurveSuffix, ISNOTNULL)).get()).get();
	const AQLDataHolder &ahDF2 = YieldData.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
	AQLObjectHolder objHolder = getYieldData().get();
	double interpolationJoinDateAsDouble = 0.0;
	objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + AQLString("_") + IR_CALIBRATION_DATA_DFS2);
	objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + AQLString("_") + IR_CALIBRATION_DATA_DFS2, new AQLDataDouble(interpolationJoinDateAsDouble));
	if (terms.at(0) != 0.0)
	{
		DoubleArray _terms = terms;
		_terms.insert(_terms.begin(), 0.0);
		DoubleArray _dfs = dfs;
		_dfs.insert(_dfs.begin(), 1.0);

		if (isinterprateterm)
		{
			for (unsigned int i = 0; i < _dfs.size(); i++)
			{
				if (_dfs[i] <= 0.0)
					throw AQLCoreInvalidData("Discount Error",__FILE__,__LINE__);
				
				_dfs[i] = -AQLMath::log(_dfs[i]);
			}
		}
		setValuesToInterpolator(mCurveType, _terms, _dfs);
        		
        // df2
		if (ahDF2.isDefined() && !ahDF2.isNull())
		{
			const DoubleArray& terms2 = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
			const DoubleArray& dfs2 = dynamic_cast<const AQLDataDoubles&> (ahDF2.get()).get();
			
            DoubleArray _terms2 = terms2;
			_terms2.insert(_terms2.begin(), 0.0);
			
            DoubleArray _dfs2 = dfs2;
			_dfs2.insert(_dfs2.begin(), 1.0);
			
            if (isinterprateterm2)
			{
				for (unsigned int i = 0; i < _dfs2.size(); i++)
				{
					if (_dfs2[i] <= 0.0)
						throw AQLCoreInvalidData("Discount Error",__FILE__,__LINE__);
					
					_dfs2[i] = -AQLMath::log(_dfs2[i]);
				}
			}
            setValuesToInterpolator(IR_CALIBRATION_DATA_DFS2, _terms2, _dfs2);
            mCurveVersionMap[IR_CALIBRATION_DATA_DFS2] = getModel();
		}
	}
	else 
	{
        if (isinterprateterm)
		{
			DoubleArray _dfs = dfs;
			for (unsigned int i = 0; i < _dfs.size(); i++)
			{
				if (_dfs[i] <= 0.0)
					throw AQLCoreInvalidData("Discount Error",__FILE__,__LINE__);
				
				_dfs[i] = -AQLMath::log(_dfs[i]);
			}
            setValuesToInterpolator(mCurveType, terms, _dfs);
        }
		else
		{
            setValuesToInterpolator(mCurveType, terms, dfs);
		}

		//df2
		if (ahDF2.isDefined() && !ahDF2.isNull())
		{
			
			const DoubleArray& dfs2 = dynamic_cast<const AQLDataDoubles&> (ahDF2.get()).get();
			const DoubleArray& terms2 = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
			
            if (isinterprateterm2)
			{
				DoubleArray _dfs2 = dfs2;
				for (unsigned int i = 0; i < _dfs2.size(); i++)
				{
					if (_dfs2[i] <= 0.0)
						throw AQLCoreInvalidData("Discount Error",__FILE__,__LINE__);
					
					_dfs2[i] = -AQLMath::log(_dfs2[i]);
				}
                setValuesToInterpolator(IR_CALIBRATION_DATA_DFS2, terms2, _dfs2);
			}
			else
			{
                setValuesToInterpolator(IR_CALIBRATION_DATA_DFS2, terms2, dfs2);
			}
			mCurveVersionMap[IR_CALIBRATION_DATA_DFS2] = getModel();
		}
	}
	mCurveVersionMap[mCurveType] = getModel();

	if (mpBasisCurveType)
	{
		const DoubleArray& terms_basis = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") + *mpBasisCurveType, ISNOTNULL)).get()).get();
		const DoubleArray& dfs_basis   = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + *mpBasisCurveType, ISNOTNULL)).get()).get();
		
        if (terms_basis.at(0) != 0.0)
		{
			DoubleArray _terms = terms_basis;
			_terms.insert(_terms.begin(), 0.0);
			DoubleArray _dfs = dfs_basis;
			_dfs.insert(_dfs.begin(), 1.0);

			if (isinterprateterm)
			{
				for (unsigned int i = 0; i < _dfs.size(); i++)
				{
					if (_dfs[i] <= 0.0)
						throw AQLCoreInvalidData("Discount Error",__FILE__,__LINE__);
					
					_dfs[i] = -AQLMath::log(_dfs[i]);
				}
			}
            setValuesToInterpolator(*mpBasisCurveType, _terms, _dfs);
		}
		else 
		{
            if (isinterprateterm)
			{
				DoubleArray _dfs = dfs_basis;
				for (unsigned int i = 0; i < _dfs.size(); i++)
				{
					if (_dfs[i] <= 0.0)
						throw AQLCoreInvalidData("Discount Error",__FILE__,__LINE__);
					
					_dfs[i] = -AQLMath::log(_dfs[i]);
				}
                setValuesToInterpolator(*mpBasisCurveType, terms_basis, _dfs);
			}
			else
			{
                setValuesToInterpolator(*mpBasisCurveType, terms_basis, dfs_basis);
			}
		}
		mCurveVersionMap[*mpBasisCurveType] = getModel();
	}
}
/*!
    @brief set discount factor curve
			
	@param[in] curveName discount factor curve name

*/
void
AQLMathYieldCurve::setCurveType(const AQLString &curveType)
{
	if (curveType == mCurveType)
	{
		return;
	}
	if (curveType == STD)
	{
		mCurveSuffix = "";
	}
	else
	{
		mCurveSuffix = "_" + curveType;
	}
	mCurveType = curveType;
	update();

}
/*!
    @brief set the current curve name and its interpolation
			
	@param[in] curveName		Name of the active curve
	@param[in] interpolation	Interpolation used by the active curve
*/
void
AQLMathYieldCurve::setCurveNameAndInterpolation(const AQLString &curveName, const AQLString &interpolation)
{
	setCurveType(curveName);
	setInterpolation(interpolation);
}

/*!
    @brief set basis discount factor curve
			
	@param[in] curveName discount factor curve name

*/
void
AQLMathYieldCurve::setBasisCurveType(const AQLString &basisCurveType)
{
	if (mpBasisCurveType != NULL && basisCurveType == *mpBasisCurveType)
	{
		return;
	}
	if (mpBasisCurveType) delete mpBasisCurveType;
	mpBasisCurveType = new AQLString(basisCurveType);

}

/*!
    @brief get curve type
			
	@return curve type

*/
AQLString 
AQLMathYieldCurve::getCurveType(void) const 
{
	return mCurveType;
}

/*!
    @brief get discount factor interpolation
		   If a curve ID doesn't exist in DFInterMap, create an interpolation entry.
		   If interpolation for a curve ID has changed, also create a new interpolation 
		   entry in the DFInterpMap for this curve ID.
		   				
	@param[in] pCurveType discount factor curve name

*/
const AQLInterpolationBase &
AQLMathYieldCurve::getDFInterpolation(const AQLString *pCurveType) const
{
	AQLString curveType("");
	AQLString suffix("");
	if (pCurveType)
	{
		curveType = *pCurveType;
		if (curveType != STD)
		{
			suffix = "_" + curveType;
		}
	}
	else
	{
		curveType = mCurveType;
		suffix = mCurveSuffix;
	}

	const DoubleArray &terms = dynamic_cast<const AQLDataDoubles &>((getYieldData().get().getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get())).get();
	const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles &>((getYieldData().get().getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get())).get();

	map<AQLString, AQLPriceDataInterpolation *>::const_iterator it = mDFInterMap.find(curveType);
	if (it != mDFInterMap.end())
	{
		const AQLPriceDataInterpolation &inter = getInterpolation();
		if (inter.convertToString().toUpper() == it->second->convertToString().toUpper())
		{
			if (mCurveVersionMap[curveType] != getModel())
			{
				setValuesToInterpolator(it->second, curveType, terms, dfs);
				mCurveVersionMap[curveType] = getModel();
			}
			return it->second->getMethod();
		}
	}

	addDFInterpolation(curveType);
	setValuesToInterpolator(mDFInterMap[curveType], curveType, terms, dfs);
	mCurveVersionMap[curveType] = getModel();
	return mDFInterMap[curveType]->getMethod();
}

/*!
    @brief get Forwards interpolation
			
	@param[in] pCurveType discount factor curve name

*/
const AQLInterpolationBase &
AQLMathYieldCurve::getForwardsInterpolation(const AQLString *pCurveType) const
{
	AQLString curveType("");
	AQLString suffix("");
	if (pCurveType)
	{
		curveType = *pCurveType;
		if (curveType != STD)
		{
			suffix = "_" + curveType;
		}
	}
	else
	{
		curveType = mCurveType;
		suffix = mCurveSuffix;
	}
		
	const DoubleMatrix &fwd_termsmtx = dynamic_cast<const AQLDataDoubleMatrix &>(getYieldData().get().getData(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, ISNOTNULL).get()).get();
	const DoubleArray &terms = fwd_termsmtx[0];
    const DoubleArray &forwardRates = dynamic_cast<const AQLDataDoubles &>((getYieldData().get().getData(IR_CALIBRATION_DATA_FORWARDRATES + suffix, ISNOTNULL).get())).get();

	// Check if we aleady have the interpolator for this curve type
	map<AQLString, AQLPriceDataInterpolation *>::const_iterator it = mFWDInterMap.find(curveType); 
	if (it != mFWDInterMap.end())
	{
		const AQLPriceDataInterpolation &inter = getInterpolation();
		if (inter.convertToString().toUpper() == it->second->convertToString().toUpper())
		{
			if (mCurveVersionMap[curveType] != getModel())
			{
				setValuesToInterpolator(it->second, curveType, terms, forwardRates);
				mCurveVersionMap[curveType] = getModel();
			}
			return it->second->getMethod();
		}
	}
	
	// Update the Forward Interpolation Map only if Required
	addFWDInterpolation(curveType);
	setValuesToInterpolator(mFWDInterMap[curveType], curveType, terms, forwardRates);
	mCurveVersionMap[curveType] = getModel();
	return mFWDInterMap[curveType]->getMethod();
	
}

/*!
    @brief get discount factor interpolation
	@param[in] pCurveType discount factor curve name
*/
const AQLInterpolationBase &
AQLMathYieldCurve::getBasisDFInterpolation() const
{
	const AQLDataHolder *dh = &getYieldData().get().getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const DoubleArray &terms = dynamic_cast<const AQLDataDoubles &>((getYieldData().get().getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get())).get();
		const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles &>((getYieldData().get().getData(IR_CALIBRATION_DATA_DFS2, ISNOTNULL).get())).get();

		map<AQLString, AQLPriceDataInterpolation *>::const_iterator it = mDFInterMap.find(IR_CALIBRATION_DATA_DFS2);
		if (it != mDFInterMap.end())
		{
			if (mCurveVersionMap[IR_CALIBRATION_DATA_DFS2] != getModel())
			{
				it->second->set(terms, dfs);
				mCurveVersionMap[IR_CALIBRATION_DATA_DFS2] = getModel();
			}
			return it->second->getMethod();
		}
		else
		{
			addDFInterpolation(IR_CALIBRATION_DATA_DFS2);
			if ( mDFInterMap[IR_CALIBRATION_DATA_DFS2]->isHybrid() )
			{
				mDFInterMap[IR_CALIBRATION_DATA_DFS2]->setJoinDateAsDouble(0.0);
			}
			mDFInterMap[IR_CALIBRATION_DATA_DFS2]->set(terms, dfs);
			mCurveVersionMap[IR_CALIBRATION_DATA_DFS2] = getModel();
			return mDFInterMap[IR_CALIBRATION_DATA_DFS2]->getMethod();
		}

	}
	else
	{
		AQLString curveType = STD;
		return getDFInterpolation(&curveType);
	}
}
/*!
    @brief delete DF calculators

*/
void
AQLMathYieldCurve::delDFInterpolations()
{
	map<AQLString, AQLPriceDataInterpolation *>::iterator it = mDFInterMap.begin();
	while (it != mDFInterMap.end())
	{
		delete it->second;
		++it;
	}
	mDFInterMap.clear();
}

/*!
    @brief Delete the DF interpolation of the given curve			
	@param[in] curve	Name of the curve

*/
void AQLMathYieldCurve::delDFInterpolationsByCurve(const AQLString& curve)
{
	map<AQLString, AQLPriceDataInterpolation *>::const_iterator it = mDFInterMap.find(curve);
	//if (it != mDFInterMap.end())
	//{
	//	mDFInterMap.erase(it);
	//}
	//20161207 - Fixed for Intel Compiler error
	while (it != mFWDInterMap.end())
	{
		delete it->second;
		++it;
	}
	mFWDInterMap.clear();
}

/*!
    @brief add DF interpolation
			
	@param[in] curveType

*/
void
AQLMathYieldCurve::addDFInterpolation(const AQLString &curveType) const
{
	map<AQLString, AQLPriceDataInterpolation *>::iterator it = mDFInterMap.find( curveType );
	if ( it != mDFInterMap.end() )
	{
		// An interpolator for this curveType is already in the map. Delete the old interpolator
		delete it->second;
		mDFInterMap.erase( it );
	}
	const AQLPriceDataInterpolation &inter = getInterpolation();
	AQLPriceDataInterpolation *calculator = dynamic_cast<AQLPriceDataInterpolation *>(inter.clone());
	mDFInterMap[curveType] = calculator;
	mCurveVersionMap[curveType] = 0;
	dynamic_cast<AQLPriceDataType *>(calculator)->setHolder(mpInter);
}
/*!
    @brief get DF for specified term from fromDate 
			
	@param[in] fromDate StartDate
	@param[in] term		Term of String
    @param[in] bdc		Optional: Sliding Rule / BusinessDayAdjustment, Defaults to "NO_CHANGE"
    @param[in] cal		Optional: Calendar, Defaults to Empty
	
	@return DiscountFactor
*/

//double        
//AQLMathYieldCurve::getDF(const AQLDate& fromDate, const AQLString& term_str) const
//{
//	AQLDate toDate = AQLMathDateCalculations::getDate(fromDate, term_str, true);
//	return getDF(fromDate, toDate);
//}

//double AQLMathYieldCurve::getDF(const AQLDate& fromDate, const AQLString& term_str, AQLString& bdc, AQLString& cal) const
//20161207 - Fixed on default values
double AQLMathYieldCurve::getDF(const AQLDate& fromDate, const AQLString& term_str, AQLString bdc, AQLString cal) const
{

    AQLDate toDate = AQLDate();
    
    // Adjust the term_str to a business day on request ... needed to fix a bug in the getDF using Tenor or TermString function
    bdc.toUpper();
    cal.toUpper();

    if ( bdc != "NO_CHANGE" )
    {
        AQLPriceDataCalendar attrCalendar;
		attrCalendar.convertFromString(cal);

        AQLPriceDataSlidingRule attrBusinessDayAdjustment;
        attrBusinessDayAdjustment.convertFromString(bdc);

        toDate = AQLMathDateCalculations::getDate( fromDate, 
                                               term_str, 
                                               attrBusinessDayAdjustment,   // SlidingRule or BusinessDayAdjustment
                                               &attrCalendar,
                                               true,                        // isAfter
							                   nullptr );                   // rollConvention
    }
    else
    {
        toDate = AQLMathDateCalculations::getDate(fromDate, term_str, true);
    }

	return getDF(fromDate, toDate);
}

double        
AQLMathYieldCurve::getBasisDF(const AQLDate& fromDate, const AQLString& term_str) const
{
	AQLDate toDate = AQLMathDateCalculations::getDate(fromDate, term_str, true);
	return getBasisDF(fromDate, toDate);
}

/*!
    @brief get DF for specified term from AsOfDate 
			
	@param[in] term     Term of String
    @param[in] bdc		Optional: Sliding Rule / BusinessDayAdjustment, Defaults to "NO_CHANGE"
    @param[in] cal		Optional: Calendar, Defaults to Empty

	@return DiscountFactor
*/
//double AQLMathYieldCurve::getDF(const AQLString& term_str, AQLString& bdc, AQLString& cal) const
//20161207 - Fixed compiler failure on default value 
double AQLMathYieldCurve::getDF(const AQLString& term_str, AQLString bdc, AQLString cal) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	
	return getDF(asOf, term_str, bdc, cal);
}

double
AQLMathYieldCurve::getBasisDF(const AQLString& term_str) const
{
	const AQLObject& YieldData = getYieldData().get().get();
	const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	
	return getBasisDF(asOf, term_str);
}
AQLMathPathYieldCurve
AQLMathYieldCurve::getCurve(double t, DayCount d) const
{
	return AQLMathPathYieldCurve(this, t, d);
}

/*!
    @brief get base forward rate 
			

	@param[in] curveType
	@param[out] terms
	@param[out] gridMat
	@param[out] taus
	@param[out] rates
	@param[out] joinDateAsDouble

*/
void 
AQLMathYieldCurve::getBaseForwardRate(const AQLString &curveType, DoubleArray &terms, DoubleMatrix &termsMat, DoubleArray &taus, DoubleArray &rates, double& joinDateAsDouble, vector<DateVector> *pDateMat_out, vector<DateVector> *pDateMat_in, const bool & useForwardData) const
{
	terms.clear();
	termsMat.clear();
	taus.clear();
	rates.clear();

	AQLString suffix = "";
	const AQLPriceDataDayCount *daycount = 0;
	const AQLInterpolationBase &dfInter = useForwardData ? getForwardsInterpolation(&curveType) : getDFInterpolation(&curveType);
	AQLString t_curveType = curveType;
	t_curveType.toUpper();
	if (t_curveType != STD)
	{
		suffix = "_" + curveType;
	}
	daycount = &getDayCount(curveType);

	const AQLObject& yieldData = getYieldData().get().get();
	const DoubleMatrix &fwd_termsmtx = dynamic_cast<const AQLDataDoubleMatrix &>(yieldData.getData(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, ISNOTNULL).get()).get();
	if (fwd_termsmtx.size() != 2 || fwd_termsmtx[0].size() != fwd_termsmtx[1].size())
	{
		throw AQLCoreInvalidData("forward terms matrix is invalid.", __FILE__, __LINE__);
	}
	terms = fwd_termsmtx[0];

	const double DAY_EPS = 0.1 / 365.25;

	const AQLDate& asofDate = dynamic_cast<const AQLDataDate&> ((yieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
	const double term_end = dynamic_cast<const AQLDataDoubles &>(yieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get()).get().back();
	if (fwd_termsmtx[0].back() >= term_end)
	{
		throw AQLCoreInvalidData("Forward Term is not consistent with DF Term", __FILE__, __LINE__);
	}
	//terms_.push_back(term_end);
	vector<DateVector> fwd_datesmtx;
	if (pDateMat_in)
	{
		fwd_datesmtx = *pDateMat_in;
	}
	else
	{
		fwd_datesmtx.resize(2);
		AQLMathDateCalculations::convertToDateGrid(asofDate, fwd_termsmtx[0], fwd_datesmtx[0]);
		DoubleArray s_fwdtermsmtx_1 = fwd_termsmtx[1];
		sort(s_fwdtermsmtx_1.begin(), s_fwdtermsmtx_1.end());
		DateVector tmpDates;
		AQLMathDateCalculations::convertToDateGrid(asofDate, s_fwdtermsmtx_1, tmpDates);
		unsigned int pos;
		for (unsigned int i = 0; i < fwd_termsmtx[1].size(); ++i)
		{
			if (!AQLAlgorithm::find<DoubleArray, double>(s_fwdtermsmtx_1, fwd_termsmtx[1][i], 0, s_fwdtermsmtx_1.size() - 1, pos))
			{
				//error
				throw AQLCoreInvalidData("forward rate grid is invalid", __FILE__, __LINE__);
			}
			fwd_datesmtx[1].push_back(tmpDates[pos]);
		}
	}

	const AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	termsMat.resize(terms.size());
	taus.resize(terms.size());
	rates.resize(terms.size());
	for (unsigned int i = 0; i < fwd_datesmtx[0].size(); ++i)
	{
		termsMat[i].resize(2);
		termsMat[i][0] = fwd_termsmtx[0][i];
		termsMat[i][1] = fwd_termsmtx[1][i];
		taus[i] = daycount->getTerm(fwd_datesmtx[0][i], fwd_datesmtx[1][i], false);
		
        if ( useForwardData )
        {
            const double rate = dfInter.value( fwd_termsmtx[0][i] );
            rates[i] = rate;
        }
        else
        {
            const double df1 = dfInter.value(fwd_termsmtx[0][i]);
		    const double df2 = dfInter.value(fwd_termsmtx[1][i]);
		    const double rate = (df1 - df2) / (taus[i] * df2);
            rates[i] = rate;
        }
	}

	joinDateAsDouble = dfInter.getJoinDateAsDouble();

	if (pDateMat_out)
	{
		pDateMat_out->resize(2);
		(*pDateMat_out)[0] = fwd_datesmtx[0];
		(*pDateMat_out)[1] = fwd_datesmtx[1];
	}
}

/*!
	@brief get base forward rate


	@param[in] curveType
	@param[out] terms
	@param[out] gridMat
	@param[out] taus
	@param[out] rates

*/
void
AQLMathYieldCurve::getBaseForwardRate(const AQLString &curveType, DoubleArray &terms, DoubleMatrix &termsMat, DoubleArray &taus, DoubleArray &rates, vector<DateVector> *pDateMat_out, vector<DateVector> *pDateMat_in, const bool & useForwardData) const
{
	double dummyJoinDateAsDouble;
	getBaseForwardRate(curveType, terms, termsMat, taus, rates, dummyJoinDateAsDouble, pDateMat_out, pDateMat_in, useForwardData);
}

/*!
    @brief get daycount adjusted forward rate 
			

	@param[in] curveType
	@param[out] terms
	@param[out] gridMat
	@param[out] taus
	@param[out] rates

*/
void 
AQLMathYieldCurve::getAdjForwardRate(const AQLString &curveType, const AQLPriceDataDayCount &dc, DoubleArray &terms, DoubleMatrix &termsMat, 
								  DoubleArray &taus, DoubleArray &rates, DateMatrix *pDateMat_in) const
{
	vector<DateVector> dateMat;
	getBaseForwardRate(curveType, terms, termsMat, taus, rates, &dateMat, pDateMat_in);

	if (dateMat.size() != 2)
	{
		throw AQLCoreInvalidData("Date Matrix format is wrong.", __FILE__, __LINE__);
	}
	unsigned int size = dateMat[0].size();
	if (dateMat[1].size() != size)
	{
		throw AQLCoreInvalidData("Date Matrix format is wrong.", __FILE__, __LINE__);
	}
	if (taus.size() != size)
	{
		throw AQLCoreInvalidData("tau vector format is wrong.", __FILE__, __LINE__);
	}
	if (rates.size() != size)
	{
		throw AQLCoreInvalidData("forward rate vector format is wrong.", __FILE__, __LINE__);
	}
	// adjust daycount
	for (unsigned int i = 0; i < size; ++i)
	{
		const double tau_ = dc.getTerm(dateMat[0][i], dateMat[1][i], false);
		rates[i] *= taus[i] / tau_;
	}
}

/*!
    @brief clone this class

	@return	pointer of this class
*/
AQLObject* 
AQLMathYieldCurve::clone() const
{
    try {
    	AQLMathYieldCurve*	pCurve = new AQLMathYieldCurve(*this);
    	return pCurve;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// OPERATION

/*!
    @brief set Interpolation

	@param[in] a	pointer of Interpolation function
	@param[in] name name of Interpolation 
*/

void
AQLMathYieldCurve::setInterpolation(
	AQLInterpolationBase* a, const AQLString& name)
{
	dynamic_cast<AQLPriceDataInterpolation*>(&(
		getData(CALIBRATION_DATA_INTERPOLATION).get()))->setMethod(a, name);
}
/*!
    @brief set Interpolation

	@param[in] name name of Interpolation 
*/
void
AQLMathYieldCurve::setInterpolation(const AQLString& name)
{
	dynamic_cast<AQLPriceDataInterpolation*>(&(
		getData(CALIBRATION_DATA_INTERPOLATION).get()))->setMethod(name);
}
 
/*!
    @brief Delete Data(except menber valuable)

	@param[in] dataName name of Data
*/
void                
AQLMathYieldCurve::remove(
	const AQLString& dataName)
{
	if (dataName == CALIBRATION_DATA_NAME
		|| dataName == CALIBRATION_DATA_INTERPOLATION
		|| dataName == IR_CALIBRATION_DATA_DAYCOUNT
		|| dataName == CALIBRATION_DATA_CALENDAR
		|| dataName == CALIBRATION_DATA_SLIDINGRULE
		|| dataName == IR_CALIBRATION_DATA_FREQUENCY
		|| dataName == IR_CALIBRATION_DATA_YIELDDATA
		|| dataName == IR_CALIBRATION_DATA_DFSVALUETOINTERPOLATE
		|| dataName == IR_CALIBRATION_DATA_DFS2VALUETOINTERPOLATE)
	{
		return; 
	}
	AQLObject::remove(dataName);
}

void               
AQLMathYieldCurve::reset(void)
{
	clear();
	mpName		 = &add(CALIBRATION_DATA_NAME);
	mpInter		 = &add(CALIBRATION_DATA_INTERPOLATION);
	mpDayCount	 = &add(IR_CALIBRATION_DATA_DAYCOUNT);		
	mpCalendar	 = &add(CALIBRATION_DATA_CALENDAR);
	mpSlidingRule= &add(CALIBRATION_DATA_SLIDINGRULE);
	mpFreq		 = &add(IR_CALIBRATION_DATA_FREQUENCY);	
	mpYieldData  = &add(IR_CALIBRATION_DATA_YIELDDATA);
	mpValueToInterp = &add(IR_CALIBRATION_DATA_DFSVALUETOINTERPOLATE);
	mpValueToInterp2 = &add(IR_CALIBRATION_DATA_DFS2VALUETOINTERPOLATE);
	dynamic_cast<AQLDataString &>(mpValueToInterp->get()).set("DiscountFactor");
	dynamic_cast<AQLDataString &>(mpValueToInterp2->get()).set("DiscountFactor");

    mCurveSuffix = "";
	mCurveType = STD;
	mCurveVersionMap.clear();
	if (mpBasisCurveType) delete mpBasisCurveType;
	mpBasisCurveType = NULL;
	delDFInterpolations();
	delFWDInterpolations();
	delDayCounts();

}

/////////////// PROTECTED METHODS /////////////////////
/*!
    @brief copy object

	@param[in] e copy object
	@return Reference of this object
*/
AQLObject&
AQLMathYieldCurve::copy(
	const AQLObject& e)
{
	if (this == &e) return *this;

	AQLObject::copy(e);
	if (!e.isTypeOf(ENTITY_IRYIELDCURVE))
	{
		AQLString err = "Assignement error for AQLMathYieldCurve : from ";
		err += AQLString(e.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mpName		 = &getData(CALIBRATION_DATA_NAME);
	mpInter		 = &getData(CALIBRATION_DATA_INTERPOLATION);
	mpDayCount	 = &getData(IR_CALIBRATION_DATA_DAYCOUNT);		
	mpCalendar	 = &getData(CALIBRATION_DATA_CALENDAR);
	mpSlidingRule= &getData(CALIBRATION_DATA_SLIDINGRULE);
	mpFreq		 = &getData(IR_CALIBRATION_DATA_FREQUENCY);	
	mpYieldData  = &getData(IR_CALIBRATION_DATA_YIELDDATA);
	mpValueToInterp = &getData(IR_CALIBRATION_DATA_DFSVALUETOINTERPOLATE);
	mpValueToInterp2 = &getData(IR_CALIBRATION_DATA_DFS2VALUETOINTERPOLATE);

    mCurveSuffix = "";
	mCurveType = STD;
	if (mpBasisCurveType) delete mpBasisCurveType;
	mpBasisCurveType = NULL;

	return *this;
}

/*!
    @brief Set data by name

	@param[in] name name of Data

	@return Reference of AQLDataHolder include this Data

*/
AQLDataHolder&
AQLMathYieldCurve::add(const AQLString& name)
{
	AQLDataInstance* dataInstance = getDataInstance();
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	const AQLDataHolder& dh = dm.getData(name);
	return AQLObject::add(name, dh);
}


const RateConvention
AQLMathYieldCurve::setRC(AQLString Freq)
{
	Freq.toUpper();
	if(Freq == SIMPLE || Freq == LUNAR)
		return SIMPL;
	else if(Freq == ANNUAL)
		return  ANN;
	else if(Freq == SEMI_ANNUAL || Freq == "SEMI_ANNUAL")
		return  SA;
	else if(Freq == QUARTERLY)
		return  QTR;
	else if(Freq == MONTHLY)
		return  MNT;
	else if(Freq == CONTINUOUS)
		return  CONT;
	else
	{
		AQLString msg = "Frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

unsigned int 
AQLMathYieldCurve::setSpanFromFrequency(const AQLString& freq )
{
    if (freq == ANNUAL) return 12;
	else if (freq == SEMI_ANNUAL) return 6;
	else if (freq == QUARTERLY) return 3;
	else if (freq == MONTHLY || freq == LUNAR) return 1;
	else 
	{
		AQLString msg = "frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

double 
AQLMathYieldCurve::getBasisSwapValue
(const AQLDate& valueDate, const AQLDate& startDate, const AQLString& term, double basis, 
 bool isPrincipal, const AQLString& frequency, const AQLPriceDataDayCount& daycount, 
  const AQLString& forecastCurveID, const AQLString& discountCurveID, double firstFixingRate, 
  bool isEOMRoll, bool isFRN)
{
	AQLDate endDate = AQLMathDateCalculations::getDate(startDate, term, getSlidingRule(), &getCalendar(), true);
    DateVector payDates;
	AQLMathDateCalculations::generateSchedule(startDate, endDate, frequency, true, NULL, NULL, 0, 
		payDates, &getSlidingRule(), &getCalendar());
    
	if (isEOMRoll)
	{
		const AQLPriceDataCalendar& calS = getCalendar();
		AQLDate eomDate = calS.getEOMDay(startDate);
		if (eomDate == startDate)
		{
			for (size_t i=0; i<payDates.size(); i++)
			{
				payDates[i] = calS.getEOMDay(payDates[i]);
			}

			endDate = calS.getEOMDay(endDate);
		}
	}

	unsigned int index=0;
    for(index=0; index<payDates.size(); index++)
    {
        if( startDate < payDates[index] )break;
    }

	DoubleArray accruTerms;
    accruTerms.push_back( daycount.getTerm(startDate, payDates[0], false) );
    for(size_t i=1; i<payDates.size(); i++)
    {
        accruTerms.push_back( daycount.getTerm(payDates[i-1], payDates[i], false) );
    }

	size_t legSize = payDates.size();
	if( legSize != accruTerms.size() ) 
		throw AQLCoreInvalidData("fixing,payment and accrual times are not same!",__FILE__,__LINE__);
	
	DoubleArray rates(legSize, 0.);
	AQLString indexTerm;
	if( forecastCurveID!=AQLString("") )
	{
		setCurveType(forecastCurveID);
		if( startDate < valueDate ) rates[index] = firstFixingRate;
		else rates[index] = getZeroRate(startDate, payDates[index]); 
		for(size_t i=1+index; i<legSize; i++)
		{
			if (isFRN)
			{
				if (frequency == ANNUAL) indexTerm = "12M";
				else if (frequency == SEMI_ANNUAL) indexTerm = "6M";
				else if (frequency == QUARTERLY) indexTerm = "3M";
				else if (frequency == MONTHLY) indexTerm = "1M";
				AQLDate calcEndDate = AQLMathDateCalculations::getDate(payDates[i-1], indexTerm, getSlidingRule(), &getCalendar(), true);
				rates[i] = getZeroRate(payDates[i-1], calcEndDate);
			}
			else
			{
				rates[i] = getZeroRate(payDates[i-1], payDates[i]);
			}
		}
	}
	
	const AQLDate& asOfDate = dynamic_cast<const AQLDataDate&> ((getYieldData().get().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	DayCount dc_act(ACT_365_ISDA);
	double PV=0.;

	setCurveType(STD);
	if (discountCurveID != AQLString("")) setCurveType(discountCurveID);

    for(size_t i=index; i<legSize; i++)
    {
		double df = getDF( dc_act365.getTerm(asOfDate,payDates[i]),&dc_act ) / getDF( dc_act365.getTerm(asOfDate,valueDate),&dc_act );
        PV += ( rates[i] + basis ) * accruTerms[i] * getDF( dc_act365.getTerm(asOfDate,payDates[i]),&dc_act ) / getDF( dc_act365.getTerm(asOfDate,valueDate),&dc_act );
    }
	if( isPrincipal ) PV += getDF( dc_act365.getTerm(asOfDate,endDate),&dc_act ) / getDF( dc_act365.getTerm(asOfDate,valueDate),&dc_act );

	return PV;
}

double 
AQLMathYieldCurve::getCurBasisSwapValue
(const AQLMathYieldCurve& yc, const AQLDate& valueDate, const AQLDate& startDate, const AQLString& term, 
 const AQLString& frequency, const AQLPriceDataDayCount& daycount, const AQLString& forecastCurveID, 
 const AQLString& discountCurveID, bool isEOMRoll, double firstFixingAmount, double firstFixingRate)
{
	AQLDate endDate = AQLMathDateCalculations::getDate(startDate, term, getSlidingRule(), &getCalendar(), true);

	DateVector payDates;
	AQLMathDateCalculations::generateSchedule(startDate, endDate, frequency, true, NULL, NULL, 0, 
		payDates, &getSlidingRule(), &getCalendar());

	if (isEOMRoll)
	{
		const AQLPriceDataCalendar& calS = getCalendar();
		AQLDate eomDate = calS.getEOMDay(startDate);
		if (eomDate == startDate)
		{
			for (size_t i=0; i<payDates.size(); i++)
			{
				payDates[i] = calS.getEOMDay(payDates[i]);
			}

			endDate = calS.getEOMDay(endDate);
		}
	}

	double accruTerm,rate,df_Dol_be,df_Dol,df_be,amount;
	unsigned int index=0;
	for(index=0; index<payDates.size(); index++)
	{
		if( valueDate < payDates[index] ) break;
	}

	double PV=0.;
	for(size_t i=index; i<payDates.size(); i++)
	{		
		setCurveType(STD);
		if (forecastCurveID != AQLString("")) setCurveType(forecastCurveID);
		
		if(i==index && startDate < valueDate)
		{
			rate = firstFixingRate;

			setCurveType(STD);
			if (discountCurveID != AQLString("")) setCurveType(discountCurveID);

			df_Dol = getDF(startDate, payDates[i]);
			accruTerm = daycount.getTerm(startDate, payDates[i], false);
			PV += accruTerm * rate * df_Dol * firstFixingAmount;
		}
		else if(i==index) 
		{
			rate = getZeroRate(startDate, payDates[i]); 

			setCurveType(STD);
			if (discountCurveID != AQLString("")) setCurveType(discountCurveID);

			df_be = yc.getDF(valueDate, startDate);
			df_Dol_be = getDF(valueDate, startDate);
			amount = yc.getDF(startDate, startDate) / getDF(startDate, startDate);
			df_Dol = getDF(valueDate, payDates[i]);
			accruTerm = daycount.getTerm(startDate, payDates[i], false);
			PV += ((1. + accruTerm * rate) * df_Dol - df_Dol_be) * amount;
		}
		else
		{
			rate = getZeroRate(payDates[i-1], payDates[i]);

			setCurveType(STD);
			if (discountCurveID != AQLString("")) setCurveType(discountCurveID);

			df_be = yc.getDF(valueDate, payDates[i-1]);
			df_Dol_be = getDF(valueDate, payDates[i-1]);
			amount = yc.getDF(startDate, payDates[i-1]) / getDF(startDate, payDates[i-1]);
			df_Dol = getDF(valueDate, payDates[i]);
			accruTerm = daycount.getTerm(payDates[i-1], payDates[i], false);
			PV += ((1. + accruTerm * rate) * df_Dol - df_Dol_be) * amount;
		}
	}
	PV += getDF(valueDate, startDate);

	return PV;
}

/*!
    @brief get DayCount Convension
			
	@return DayCount Convension
*/

const AQLPriceDataDayCount&
AQLMathYieldCurve::getDayCount(const AQLString &curveType) const	
{
	map<AQLString, AQLPriceDataDayCount *>::const_iterator it = mDayCountMap.find(curveType);
	if (it != mDayCountMap.end())
	{
		return *it->second;
	}
	else
	{
		mDayCountMap[curveType] = dynamic_cast<AQLPriceDataDayCount*>(mpDayCount->clone());
		return *mDayCountMap[curveType];
	}
}

/*!
    @brief get DayCount Convension and set DayCount Convension
			
	@return DayCount Convension
*/
AQLPriceDataDayCount&
AQLMathYieldCurve::getDayCount(const AQLString &curveType) 
{
	map<AQLString, AQLPriceDataDayCount *>::const_iterator it = mDayCountMap.find(curveType);
	if (it != mDayCountMap.end())
	{
		return *it->second;
	}
	else
	{
		mDayCountMap[curveType] = dynamic_cast<AQLPriceDataDayCount*>(mpDayCount->clone());
		return *mDayCountMap[curveType];
	}
}

/*!
@brief get DayCount Convension from yield data object

@return DayCount Convension
*/

const AQLPriceDataDayCount&
AQLMathYieldCurve::getDayCountFromYieldData(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD) suffix = "_" + curveType;

	return dynamic_cast<const AQLPriceDataDayCount&>(getYieldData().get().get().getData(IR_CALIBRATION_DATA_DAYCOUNT + suffix, ISNOTNULL).get());
}

/*!
    @brief delete DF calculators

*/
void
AQLMathYieldCurve::delDayCounts()
{
	map<AQLString, AQLPriceDataDayCount *>::iterator it = mDayCountMap.begin();
	while (it != mDayCountMap.end())
	{
		delete it->second;
		++it;
	}
	mDayCountMap.clear();
}

/*!
    @brief Delete the dayCount of the given curve			
	@param[in] curve	Name of the curve

*/
void AQLMathYieldCurve::delDayCountsByCurve(const AQLString& curve)
{
	map<AQLString, AQLPriceDataDayCount *>::const_iterator it = mDayCountMap.find(curve);
	//if (it != mDayCountMap.end())
	//{
	//	mDayCountMap.erase(it);
	//}
	//20161207 - Fixed for Intel Compiler error
	while (it != mDayCountMap.end())
	{
		delete it->second;
		++it;
	}
	mDayCountMap.clear();
}

/*!
    @brief get forward interpolation
		   If a curve ID doesn't exist in FwdInterMap, create an interpolation entry.
		   If interpolation for a curve ID has changed, also create a new interpolation 
		   entry in the FwdInterpMap for this curve ID.
	
	@param[in] pCurveType curve name
	@param[in] useFwdData When true, calculates forwards from discount factors using formula F(t,T) = [ ( P(0,t) / P(0,T) ) - 1 ] / ( T - t )
						  When false, calculates forwards using forward data directly when isFWDInter is set to TRUE
*/
const AQLInterpolationBase &
AQLMathYieldCurve::getFWDInterpolation(const AQLString *pCurveType, const bool & useFwdData) const
{
	DoubleArray terms;
	DoubleMatrix termsMat;
	DoubleArray tau;
	DoubleArray fwds;

	AQLString curveType;
	if (pCurveType)
	{
		curveType = *pCurveType;
	}
	else
	{
		curveType = mCurveType;
	}
	
	// Check if we aleady have the interpolator for this curve type
	map<AQLString, AQLPriceDataInterpolation *>::const_iterator it = mFWDInterMap.find(curveType);
	
	
	if (it != mFWDInterMap.end())
	{
		const AQLPriceDataInterpolation &inter = getInterpolation();
		if (inter.convertToString().toUpper() == it->second->convertToString().toUpper())
		{
			// Only re-use interpolator if it hasn't changed
			if (mCurveVersionMap[curveType] != getModel())
			{
				getBaseForwardRate(curveType, terms, termsMat, tau, fwds, 0, 0, useFwdData);		
				setValuesToInterpolator( it->second, curveType, terms, fwds );
			}
			return it->second->getMethod();
		}
	}

	// Update the Forward Interpolation Map if required 
	getBaseForwardRate(curveType, terms, termsMat, tau,  fwds, 0, 0, useFwdData);
	addFWDInterpolation(curveType);
	setValuesToInterpolator( mFWDInterMap[curveType], curveType, terms, fwds );
	return mFWDInterMap[curveType]->getMethod();

}

/*!
    @brief add DF interpolation
			
	@param[in] curveType

*/
void
AQLMathYieldCurve::addFWDInterpolation(const AQLString &curveType) const
{
	map<AQLString, AQLPriceDataInterpolation *>::iterator it = mFWDInterMap.find( curveType );
	if ( it != mFWDInterMap.end() )
	{
		// An interpolator for this curveType is already in the map. Delete the old interpolator
		delete it->second;
		mFWDInterMap.erase( it );
	}
	const AQLPriceDataInterpolation &inter = getInterpolation();
	AQLPriceDataInterpolation *fwd_inter = dynamic_cast<AQLPriceDataInterpolation *>(inter.clone());
	mFWDInterMap[curveType] = fwd_inter;
	dynamic_cast<AQLPriceDataType *>(fwd_inter)->setHolder(mpInter);
}

/*!
    @brief delete FWD Interpolations

*/
void
AQLMathYieldCurve::delFWDInterpolations()
{
	map<AQLString, AQLPriceDataInterpolation *>::iterator it = mFWDInterMap.begin();
	while (it != mFWDInterMap.end())
	{
		delete it->second;
		++it;
	}
	mFWDInterMap.clear();
}

/*!
    @brief Delete the Fwd interpolation of the given curve			
	@param[in] curve	Name of the curve

*/
void AQLMathYieldCurve::delFWDInterpolationsByCurve(const AQLString& curve)
{
	map<AQLString, AQLPriceDataInterpolation *>::const_iterator it = mFWDInterMap.find(curve);
	//if (it != mFWDInterMap.end())
	//{
	//	mFWDInterMap.erase(it);
	//}
	//20161207 - Fixed for Intel Compiler error
	while (it != mDFInterMap.end())
	{
		delete it->second;
		++it;
	}
	mDFInterMap.clear();
}

/*!
    @brief Delete the DF interpolation, Fwd interpolation and day count data of the given curve			
	@param[in] curve	Name of the curve

*/
void AQLMathYieldCurve::deleteCurveDataByCurveName(const AQLString& curveName)
{
	delDFInterpolationsByCurve(curveName);
	delFWDInterpolationsByCurve(curveName);
	delDayCountsByCurve(curveName);
}

/*!
    @brief delete FWD Interpolations

*/
int
AQLMathYieldCurve::getYieldDataVersion() const
{
	const AQLObject& yieldData = getYieldData().get().get();
	int version = yieldData.getModel();
	return yieldData.getModel();
}

bool
AQLMathYieldCurve::operator==(const AQLMathYieldCurve& a) const
{
	return getYieldData().get().getName() == a.getYieldData().get().getName()
		&& getCurveType() == a.getCurveType();
}

/////////////// AQLMathPathYieldCurve /////////////////////
// Default constructor
AQLMathPathYieldCurve::AQLMathPathYieldCurve(const AQLMathYieldCurve* yc, double t, DayCount dc)
: AQLRatesPathElementCurve(t), mpYC(yc), mDC(dc)
{
}
// Destructor
AQLMathPathYieldCurve::~AQLMathPathYieldCurve()
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/

AQLRatesPathElementBase*	
AQLMathPathYieldCurve::clone() const
{
    try 
	{
		return new AQLMathPathYieldCurve(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

double
AQLMathPathYieldCurve::getP(double T) const
{
	if(T < m_t)
	{
		AQLString msg = "T is before BaseDate";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
//	DayCount dc = mpYC->getDayCount().getDayCount();
//	if (mDC == dc)
//	{
		double dff = mpYC->getDF(T, &mDC);
		double dft = mpYC->getDF(m_t, &mDC);
		return dff / dft;
//	}
/*	else
	{
		AQLPriceDataDayCount data_dc1(mDC);
		AQLPriceDataDayCount data_dc2(dc);
		const AQLObject& YieldData = mpYC->getYieldData().get().get();
		const AQLDate& asOf = dynamic_cast<const AQLDataDate&> ((YieldData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

		double term_t = data_dc1.getTerm(asOf, data_dc2.getDayTerm(asOf, m_t));
		double term_T = data_dc1.getTerm(asOf, data_dc2.getDayTerm(asOf, T));
		

		double dff = mpYC->getDF(term_t);
		double dft = mpYC->getDF(term_T);
		return dff / dft;		
	}*/
}

bool
AQLMathPathYieldCurve::operator==(const AQLMathPathYieldCurve& a) const
{
	return get_t() == a.get_t() 
		&& getDC() == a.getDC()
		&& *getYC() == *(a.getYC());
}
