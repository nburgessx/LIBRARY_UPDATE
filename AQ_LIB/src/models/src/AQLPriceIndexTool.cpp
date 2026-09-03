/*! @file
    @brief source code of tool class of index calculation.

*/
//  2006, AlgoQuantHub..
///
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "AQLPriceIndexTool.h"

#include "AQLDataHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLObject.h"
#include "AQLObjectHolder.h"
#include "AQLCoreUtility.h"

#include "AQLAlgorithm.h"

#include "AQLMathDateCalculations.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathIndexEntity.h"
#include "AQLMathDefine.h"

#include "AQLConstant.h"

#include "AQLPriceCFGenUtility.h"
#include "AQLPricePayOff.h"

#include "AQLMathPlainVanillaEntity.h"
#include "AQLInterpolationBase.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLPriceDataInterpolation.h"

using namespace std;


#define ROUND_STR		"ROUND"
#define ROUND_UP_STR	"ROUNDUP"
#define ROUND_DOWN_STR	"ROUNDDOWN"



#define CPN		"CPN"
#define LEG		"LEG"
#define COUPON	"COUPON"

/*!
    @brief constructor
*/
AQLPriceIndexToolBase::AQLPriceIndexToolBase()
: mIsCap(false), mIsFloor(false), mDiscountRatio(1.0), mIsDFRatioUpdate(false), mIsRound(false), mRoundDigit(0)
{

}
/*!
    @brief destructor

*/
AQLPriceIndexToolBase::~AQLPriceIndexToolBase()
{

}
/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolBase::setUp(const AQLDate& basedate,
							const AQLDate& paydate,
							const AQLObject& trade,
							const AQLObject& indexinfo,
							const AQLPricePayOff& payoff)
{
	(void)payoff; (void)trade; (void)basedate; (void)paydate; //20070411--Nagase--gcc
	const AQLDataHolder* dh;
	//round function
	dh = &(indexinfo.getData(PRICING_DATA_ROUNDFUNCTION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsRound = true;
		AQLString roundfunction = dynamic_cast<const AQLDataString&>(dh->get()).get();
		roundfunction.toUpper();
		if (roundfunction == ROUND_STR) mRoundFunction = ROUND;
		else if (roundfunction == ROUND_UP_STR) mRoundFunction = ROUND_UP;
		else mRoundFunction = ROUND_DOWN;

		dh = &(indexinfo.getData(PRICING_DATA_ROUNDDIGIT, ISNOTNULL));
		mRoundDigit = dynamic_cast<const AQLDataInt&>(dh->get()).get();
	}
	else
		mIsRound = false;

	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDDATE, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
	{
		dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const AQLDate& fixingdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
			if (fixingdate > paydate)
			{
				//error
				AQLString msg = "Index fixingdate is after paymentdate!";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
	}

	mDiscountRatio = 1.0;
	mIsDFRatioUpdate = false;
	dh = &(indexinfo.getData(PRICING_DATA_DISCOUNTSTARTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLDate& startdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
		
		dh = &(indexinfo.getData(PRICING_DATA_DISCOUNTENDDATE, ISDEFINED));
		const AQLDate &enddate = dynamic_cast<const AQLDataDate&>(dh->get()).get();

		if (startdate > enddate)
			throw AQLCoreInvalidData("Startdate and Enddate error",__FILE__,__LINE__);

		//get path object
		dh = &(trade.getData("PathEntity", ISNOTNULL));
		const AQLDataReference& pathref = dynamic_cast<const AQLDataReference &>(dh->get());
		
		if(!pathref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
			throw AQLCoreInvalidData("Discount adjust only supports plain vanilla case",__FILE__,__LINE__);

		const AQLMathPlainVanillaEntity& vpath = (dynamic_cast<const AQLMathPlainVanillaEntity&>(pathref.get().get()));


		AQLString curvename = "STD";
		//get index currency
		AQLString indexcur;
		const AQLObject *pFra = 0;
		dh = &(indexinfo.getData(PRICING_DATA_FRAINDEXINFO, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pFra = &dynamic_cast<const AQLDataReference &>(dh->get()).get().get();
		}
		else
		{
			pFra = &indexinfo;
		}
		dh = &(pFra->getData(PRICING_DATA_CURRENCY, ISNOTNULL));
		indexcur = dynamic_cast<const AQLDataString &>(dh->get()).get();
		
		dh = &(pFra->getData(PRICING_DATA_BASISCURVE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			curvename = dynamic_cast<const AQLDataString &>(dh->get()).get();
		}

		// get curve convention
		AQLString freq;
		AQLPriceDataDayCount dc;
		AQLPriceDataCalendar cal;
		AQLPriceDataSlidingRule sld;
		AQLString accessory;
		vpath.getIRCurve(indexcur).getCurveConvention(freq, cal, sld, dc, accessory, curvename);	
		
		AQLPriceDataConvention conv(dc.getDayCount(), SIMPL);
		const double term = dc.getTerm(startdate, enddate, false);
		dh = &(pFra->getData(PRICING_DATA_FIXEDRATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const double rate = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
			mDiscountRatio = 1.0 / AQLPriceDataConvention::rateToRet(rate, term, conv);
		}
		else
		{
			AQLPriceDataDayCount act365_ISDA(ACT_365_ISDA);
			const double startterm = act365_ISDA.getTerm(basedate, startdate);
			const double endterm = act365_ISDA.getTerm(basedate, enddate);
			// check forward interpolation
			bool isFWDInter = false;
			dh = &(pFra->getData(PRICING_DATA_ISFWDINTERPOLATION, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				isFWDInter = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			}
			if (isFWDInter)
			{
				DoubleArray terms;
				DoubleMatrix termsMat;
				DoubleArray tau;
				DoubleArray fwds;	
				vpath.getIRCurve(indexcur).getBaseForwardRate(curvename, terms, termsMat, tau,  fwds);
				AQLPriceDataInterpolation inter = dynamic_cast<const AQLPriceDataInterpolation &>(pFra->getData(PRICING_DATA_FWDINTERPOLATION, ISNOTNULL).get());
				inter.set(terms, fwds);
				const double rate = inter.value(startterm);
				mDiscountRatio = 1.0 / AQLPriceDataConvention::rateToRet(rate, term, conv);
			}
			else
			{
				const AQLInterpolationBase& pInter = vpath.getIRCurve(indexcur).getDFInterpolation(&curvename);
				mDiscountRatio = pInter.value(endterm);
				mDiscountRatio /= pInter.value(startterm);
			}
		}
		mIsDFRatioUpdate = true;
	}
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolBase::setUp(const AQLDate& basedate,	
						const AQLObject& trade,
						const AQLObject& indexinfo,
						const AQLDate& fixingdate,
						const AQLPricePayOff& payoff)
{
	AQLPriceIndexToolBase::setUp(basedate, fixingdate, trade, indexinfo, payoff);
}

bool
AQLPriceIndexToolBase::isSavePastFixing(const AQLObject& trade)
{
	const AQLDataHolder* dh;
	// is save past fixing
	bool isSavePastFixing = false;
	dh = &(trade.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSavePastFixing = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	return isSavePastFixing;
}



/*!
    @brief constructor
*/
AQLPriceIndexTool::AQLPriceIndexTool()
: AQLPriceIndexToolBase(), mCap(0), mFloor(0), mpIndex(0), mpObservationOperator(0)
{

}
/*!
    @brief destructor
*/
AQLPriceIndexTool::~AQLPriceIndexTool()
{
}

/*!
	@brief make copy(clone) of this class
    @return deep copy of this class
*/
AQLPriceIndexToolBase*
AQLPriceIndexTool::clone() const
{
    try 
	{
        return new AQLPriceIndexTool(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexTool::setUp(const AQLDate& basedate,
							const AQLDate& paydate,
							const AQLObject& trade,
							const AQLObject& indexinfo,
							const AQLPricePayOff& payoff)
{
	AQLPriceIndexToolBase::setUp(basedate, paydate, trade, indexinfo, payoff);

	mX.clear();
	mpObservationOperator = NULL;
	mpIndex = NULL;

	const AQLDataHolder* dh;
	//index object
	dh = &(indexinfo.getData(PRICING_DATA_INDEXENTITY, ISNOTNULL));
	const AQLDataReference& ref = dynamic_cast<const AQLDataReference&>(dh->get());
	mpIndex = &dynamic_cast<const AQLMathIndexEntity&>(ref.get().get());

	// is save past fixing
	const bool isSavePFixing = isSavePastFixing(trade);
	//observation start
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLDate& start = dynamic_cast<const AQLDataDate&>(dh->get());
		//observation end
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL)); 
		const AQLDate& end = dynamic_cast<const AQLDataDate&>(dh->get());
		//frequency
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONFREQUENCY, ISNOTNULL)); 
		const AQLString& freq = dynamic_cast<const AQLDataString&>(dh->get());
		//day
		int* pday = NULL;
		int day;
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDAY, NOCHECK)); 
		if (dh->isDefined() && !dh->isNull())
		{
			day = dynamic_cast<const AQLDataInt&>(dh->get()).get();
			pday = &day;
		}
		//slidingrule & calendar
		const AQLPriceDataSlidingRule* psrule;
		const AQLPriceDataCalendar* pcal ;
		AQLPriceCFGenUtility::getBusDayRuleAndCalendar(indexinfo, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexinfo,
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,
													psrule, pcal);

		DateVector out;
		AQLMathDateCalculations::generateSchedule(start, end, freq,	
									false,
									NULL, NULL,
									pday,
									out,
									psrule,
									pcal
									);
		AQLDate tmpDate;
		if (pcal != NULL)
			tmpDate = psrule->getDate(end, *pcal);
		else 
			tmpDate = end;
		if (out.back() != tmpDate) out.push_back(tmpDate);

		// observation same days
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSAMEDAYS, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			int sameDays = dynamic_cast<const AQLDataInt&>(dh->get()).get();
			for (unsigned int i = 0; i < sameDays; ++i)
			{
				out.push_back(out.back());
			}
		}
	
		const DateVector& grid = mpIndex->getGrid();
		if (grid.size() == 0)
		{
			//error
			AQLString msg = "Grid is not set in AQLMathIndexEntity";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		
		unsigned int pos;
		bool isNeedPastRate = false;
		bool isBaseContain = false;
		for (unsigned int i = 0; i < out.size(); i++)
		{
			if (basedate <= out[i]) //future
			{
				if (!AQLAlgorithm::find<DateVector, AQLDate>(grid, out[i], 0, grid.size() - 1, pos))
				{
					//error
					AQLString msg = "AQLMathIndexEntity grid is inconsistent with index observation dates";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				if (basedate == out[i])
				{
					isBaseContain = true;
				}
				mIndexPos.push_back(pos);
			}
			else
			{
				isNeedPastRate = true;
			}
		}	
		
		//past rate
		if (isNeedPastRate)
		{
			if (isSavePFixing)
			{
				DateVector observationdates;
				DoubleArray observationrates;
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
					dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
					observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
				}
				for (unsigned int i = 0; i < out.size(); i++)
				{
					if (basedate > out[i]) //past
					{
						if (!AQLAlgorithm::find<DateVector, AQLDate>(observationdates, out[i], 0, observationdates.size() - 1, pos))
						{
							if (!observationdates.empty() && observationdates.back() > out[i])
							{
								//error
								AQLString msg = "Index rate at ";
								msg += AQLDataDate(out[i]).convertToString();
								msg += " is needed.";
								throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
							}
							mIndexPos.insert(mIndexPos.begin(), 0);
						}
						else
						{
							mFixedRates.push_back(observationrates.at(pos));
						}
					}
					else
					{
						break;
					}
				}
				
			}
			else
			{
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
				const DateVector& observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				const DoubleArray& observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
				if (observationdates.size() != observationrates.size())
				{
					throw AQLCoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
				}
				for (unsigned int i = 0; i < out.size(); i++)
				{
					if (basedate > out[i]) //past
					{
						if (!AQLAlgorithm::find<DateVector, AQLDate>(observationdates, out[i], 0, observationdates.size() - 1, pos))
						{
							//error
							AQLString msg = "Index rate at ";
							msg += AQLDataDate(out[i]).convertToString();
							msg += " is needed.";
							throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						mFixedRates.push_back(observationrates.at(pos));
					}
					else
					{
						break;
					}
				}
			}
		}
		
		// basedate rate
		if (isBaseContain)
		{
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const DateVector& observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				if (AQLAlgorithm::find<DateVector, AQLDate>(observationdates, basedate, 0, observationdates.size() - 1, pos))
				{
					dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
					const DoubleArray& observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
					if (observationdates.size() != observationrates.size())
					{
						throw AQLCoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
					}
					mFixedRates.push_back(observationrates.at(pos));
					mIndexPos.erase(mIndexPos.begin());
				}
			}
		}		

		// observation operator
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const AQLPriceDataFunction&>(dh->get()).getFunction();

		
	}
	else
	{
		//fixing date
		dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL));	 
		const AQLDate& fixingdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
		const DateVector& grid = mpIndex->getGrid();
		if (grid.size() == 0)
		{
			//error
			AQLString msg = "Grid is not set in AQLMathIndexEntity";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		if (isSavePFixing && basedate > fixingdate)
		{
			mIndexPos.push_back(0);
		}
		else
		{
			unsigned int pos;
			if (!AQLAlgorithm::find<DateVector, AQLDate>(grid, fixingdate, 0, grid.size() - 1, pos))
			{
				//error
				AQLString msg = "AQLMathIndexEntity grid is inconsistent with index fixing dates";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			mIndexPos.push_back(pos);
		}

	
	}

	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			AQLString msg = "Only AQLConstant function is available";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mCap = dynamic_cast<const AQLConstant&>(method.getFunction()).getParam().at(0);
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			AQLString msg = "Only AQLConstant function is available";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mFloor = dynamic_cast<const AQLConstant&>(method.getFunction()).getParam().at(0);
		mIsFloor = true;	
	}
	mX.resize(mFixedRates.size() + mIndexPos.size());
}


/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexTool::setUp(const AQLDate& basedate,	
						const AQLObject& trade,
						const AQLObject& indexinfo,
						const AQLDate& fixingdate,
						const AQLPricePayOff& payoff)
{
	AQLPriceIndexToolBase::setUp(basedate, trade, indexinfo, fixingdate, payoff);
	const AQLDataHolder* dh;

	mX.clear();
	mpObservationOperator = NULL;
	mpIndex = NULL;


	//index object
	dh = &(indexinfo.getData(PRICING_DATA_INDEXENTITY, ISNOTNULL));
	const AQLDataReference& ref = dynamic_cast<const AQLDataReference&>(dh->get());
	mpIndex = &dynamic_cast<const AQLMathIndexEntity&>(ref.get().get());
	// is save past fixing
	const bool isSavePFixing = isSavePastFixing(trade);
	// observation enddate
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLString& endstr = dynamic_cast<const AQLDataString&>(dh->get()).get();
	
		const AQLPriceDataSlidingRule* psrule;
		const AQLPriceDataCalendar* pcal;
		AQLPriceCFGenUtility::getBusDayRuleAndCalendar(indexinfo, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexinfo,
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,		
													psrule,
													pcal);	


		const AQLDate& end
			= AQLMathDateCalculations::getDate(fixingdate, endstr, 
										*psrule,
										pcal,
										false);

		// observation startdate
		AQLDate start;
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			start = dynamic_cast<const AQLDataDate&>(dh->get()).get();
			if (psrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
				start = psrule->getDate(start, *pcal);		
		}
		else
		{
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTTERM, ISNOTNULL));
			const AQLString& startstr = dynamic_cast<const AQLDataString&>(dh->get()).get();
			start = AQLMathDateCalculations::getDate(fixingdate, startstr, 
										*psrule,
										pcal,
										false);					
		}

		if (end < basedate)//past! 
		{
			if (isSavePFixing)
			{
				DateVector fixeddates;
				DoubleArray fixedrates;
				dh = &(indexinfo.getData(PRICING_DATA_FIXEDDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL));
					fixedrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();		
				}
				unsigned int pos;
				if (!AQLAlgorithm::find<DateVector, AQLDate>(fixeddates, end, 0, fixeddates.size() - 1, pos))
				{
					mIndexPos.push_back(0);
				}
				else
				{
					mFixedRates.push_back(fixedrates.at(pos));

				}				
			}
			else
			{
				dh = &(indexinfo.getData(PRICING_DATA_FIXEDDATES, ISNOTNULL));
				const DateVector& fixeddates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL));
				const DoubleArray& fixedrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();		
				unsigned int pos;
				if (!AQLAlgorithm::find<DateVector, AQLDate>(fixeddates, end, 0, fixeddates.size() - 1, pos))
				{
					//error	
					AQLString msg = "No fixied rate at " + AQLDataDate(end).convertToString();
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);				
				}
				mFixedRates.push_back(fixedrates.at(pos));
			}

			mIsCap = false;
			mIsFloor = false;
			return;
		}


		//frequency
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONFREQUENCY, ISNOTNULL)); 
		const AQLString& freq = dynamic_cast<const AQLDataString&>(dh->get());
		//day
		int* pday = NULL;
		int day;
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDAY, NOCHECK)); 
		if (dh->isDefined() && !dh->isNull())
		{
			day = dynamic_cast<const AQLDataInt&>(dh->get()).get();
			pday = &day;
		}

		DateVector out;
		AQLMathDateCalculations::generateSchedule(start, end, freq,	
									false,
									NULL, NULL,
									pday,
									out,
									psrule,
									pcal
									);

		if (pcal != NULL)
		{
			AQLDate tmp = psrule->getDate(end, *pcal);
			if (out.at(out.size() - 1) != tmp) out.push_back(tmp);
		}
	
		const DateVector& grid = mpIndex->getGrid();
		if (grid.size() == 0)
		{
			//error
			AQLString msg = "Grid is not set in AQLMathIndexEntity";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		unsigned int pos;
		bool isNeedPastRate = false;
		bool isBaseContain = false;
		for (unsigned int i = 0; i < out.size(); i++)
		{
			if (basedate <= out[i])
			{
				if (!AQLAlgorithm::find<DateVector, AQLDate>(grid, out[i], 0, grid.size() - 1, pos))
				{
					//error
					AQLString msg = "AQLMathIndexEntity grid is inconsistent with index observation dates";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				if (basedate == out[i])
				{
					isBaseContain = true;
				}
				mIndexPos.push_back(pos);
			}
			else 
			{
				isNeedPastRate = true;
			}
		}

		//past rate
		if (isNeedPastRate)
		{
			if (isSavePFixing)
			{
				DateVector observationdates;
				DoubleArray observationrates;
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
					dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
					observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
				}
				for (unsigned int i = 0; i < out.size(); i++)
				{
					if (basedate > out[i]) //past
					{
						if (!AQLAlgorithm::find<DateVector, AQLDate>(observationdates, out[i], 0, observationdates.size() - 1, pos))
						{
							if (!observationdates.empty() && observationdates.back() > out[i])
							{
								//error
								AQLString msg = "Index rate at ";
								msg += AQLDataDate(out[i]).convertToString();
								msg += " is needed.";
								throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
							}
							mIndexPos.insert(mIndexPos.begin(), 0);
						}
						else
						{
							mFixedRates.push_back(observationrates.at(pos));
						}
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
				const DateVector& observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				const DoubleArray& observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
				if (observationdates.size() != observationrates.size())
				{
					throw AQLCoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
				}
				for (unsigned int i = 0; i < out.size(); i++)
				{	
					if (basedate > out[i]) //past
					{
						if (!AQLAlgorithm::find<DateVector, AQLDate>(observationdates, out[i], 0, observationdates.size() - 1, pos))
						{
							//error
							AQLString msg = "Index rate at ";
							msg += AQLDataDate(out[i]).convertToString();
							msg += " is needed.";
							throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						mFixedRates.push_back(observationrates.at(pos));
					}
					else
					{
						break;
					}
				}
			}
		}

		// basedate rate
		if (isBaseContain)
		{
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const DateVector& observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				if (AQLAlgorithm::find<DateVector, AQLDate>(observationdates, basedate, 0, observationdates.size() - 1, pos))
				{
					dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
					const DoubleArray& observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
					if (observationdates.size() != observationrates.size())
					{
						throw AQLCoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
					}
					mFixedRates.push_back(observationrates.at(pos));
					mIndexPos.erase(mIndexPos.begin());
				}
			}
		}
		
		// observation operator		
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const AQLPriceDataFunction&>(dh->get()).getFunction();
		
	}
	else
	{
		const DateVector& grid = mpIndex->getGrid();
		if (grid.size() == 0)
		{
			//error
			AQLString msg = "Grid is not set in AQLMathIndexEntity";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		unsigned int pos;
		if (!AQLAlgorithm::find<DateVector, AQLDate>(grid, fixingdate, 0, grid.size() - 1, pos))
		{
			//error
			AQLString msg = "AQLMathIndexEntity grid is inconsistent with index fixing dates";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		mIndexPos.push_back(pos);

	}

	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			AQLString msg = "Only AQLConstant function is available";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mCap = dynamic_cast<const AQLConstant&>(method.getFunction()).getParam().at(0);
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			AQLString msg = "Only AQLConstant function is available";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mFloor = dynamic_cast<const AQLConstant&>(method.getFunction()).getParam().at(0);
		mIsFloor = true;	
	}

	mX.resize(mFixedRates.size() + mIndexPos.size());

}



/*!
    @brief constructor
*/
AQLPriceIndexToolFixed::AQLPriceIndexToolFixed()
: AQLPriceIndexToolBase(), mRate(NULL), mRates(NULL), mPos(0), mpFixingDate(NULL)
{

}

/*!
    @brief destructor
*/
AQLPriceIndexToolFixed::~AQLPriceIndexToolFixed()
{
	if (mpFixingDate) delete mpFixingDate;
}

/*!
	@brief make copy(clone) of this class
    @return deep copy of this class
*/
AQLPriceIndexToolBase*
AQLPriceIndexToolFixed::clone() const
{
    try 
	{
        return new AQLPriceIndexToolFixed(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolFixed::setUp(const AQLDate& basedate,
							const AQLDate& paydate,
							const AQLObject& trade,
							const AQLObject& indexinfo,
							const AQLPricePayOff& payoff)
{
	AQLPriceIndexToolBase::setUp(basedate, paydate, trade, indexinfo, payoff);	
	const AQLDataHolder* dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATE, ISNOTNULL));
	//mRate = dynamic_cast<const AQLDataDouble&>(dh->get()).get();
	mRate = &dynamic_cast<const AQLDataDouble&>(dh->get());
	dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mpFixingDate = new AQLDate(dynamic_cast<const AQLDataDate&>(dh->get()).get());
	}
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolFixed::setUp(const AQLDate& basedate,
						const AQLObject& trade,
						const AQLObject& indexinfo,
						const AQLDate& fixingdate,
						const AQLPricePayOff& payoff)
{
	AQLPriceIndexToolBase::setUp(basedate, trade, indexinfo, fixingdate, payoff);	

	const AQLDataHolder* dh;
	dh = &(indexinfo.getData(PRICING_DATA_FIXEDDATES, ISNOTNULL));
	const DateVector& fixeddates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
	dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL));
	const DoubleArray& fixedrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();

	// observation enddate
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLString& endstr = dynamic_cast<const AQLDataString&>(dh->get()).get();
	
		const AQLPriceDataSlidingRule* psrule;
		const AQLPriceDataCalendar* pcal;
		AQLPriceCFGenUtility::getBusDayRuleAndCalendar(indexinfo, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexinfo,
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,		
													psrule,
													pcal);		
	
		const AQLDate& end
			= AQLMathDateCalculations::getDate(fixingdate, endstr, 
										*psrule,
										pcal,
										false);

		if (end > fixingdate)
		{
			//error	
			AQLString msg = "Obserbation end date is after fixing date";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}
		
		unsigned int pos;
		if (!AQLAlgorithm::find<DateVector, AQLDate>(fixeddates, end, 0, fixeddates.size() - 1, pos))
		{
			//error	
			AQLString msg = "No fixied rate at " + AQLDataDate(end).convertToString();
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);				
		}

		mRate = NULL;
		mRates = &dynamic_cast<const AQLDataDoubles&>(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL).get());
		mPos = pos;
	}
	else
	{
		unsigned int pos;
		if (!AQLAlgorithm::find<DateVector, AQLDate>(fixeddates, fixingdate, 0, fixeddates.size() - 1, pos))
		{
			//error	
			AQLString msg = "No fixied rate at" + AQLDataDate(fixingdate).convertToString();;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}
		mRate = NULL;
		mRates = &dynamic_cast<const AQLDataDoubles&>(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL).get());
		mPos = pos;
	}

}



/*!
    @brief constructor
*/
AQLPriceIndexToolCpn::AQLPriceIndexToolCpn()
: AQLPriceIndexToolBase(), mPayOff(0), mLegNo(0),
   mpObservationOperator(0), mpCap(0), mpFloor(0)
{

}
/*!
    @brief destructor

*/
AQLPriceIndexToolCpn::~AQLPriceIndexToolCpn()
{
	if (mpCap != NULL) delete mpCap;
	if (mpFloor != NULL) delete mpFloor;
}





/*!
    @brief copy constructor

	@param[in] v AQLPricePayOff object
*/
AQLPriceIndexToolCpn::AQLPriceIndexToolCpn(const AQLPriceIndexToolCpn& v)
: AQLPriceIndexToolBase(v), 
mPayOff(v.mPayOff), mLegNo(v.mLegNo), mPayOffPos(v.mPayOffPos),
   mpObservationOperator(v.mpObservationOperator), mFixedRates(v.mFixedRates),
   mX(v.mX), mpCap(0), mpFloor(0)
{
	if (v.mpCap != NULL) mpCap = dynamic_cast<AQLFunctionBase*>(v.mpCap->clone());
	if (v.mpFloor != NULL) mpFloor = dynamic_cast<AQLFunctionBase*>(v.mpFloor->clone());
}


/*!
	@brief make copy(clone) of this class
    @return deep copy of this class
*/
AQLPriceIndexToolBase*
AQLPriceIndexToolCpn::clone() const
{
    try 
	{
        return new AQLPriceIndexToolCpn(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}


/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolCpn::setUp(const AQLDate& basedate,
							const AQLDate& paydate,
							const AQLObject& trade,
							const AQLObject& indexinfo,
							const AQLPricePayOff& payoff)
{
	AQLPriceIndexToolBase::setUp(basedate, paydate, trade, indexinfo, payoff);	
	
	mPayOff = &payoff;
	mpObservationOperator = NULL;
	mPayOffPos.clear();
	mFixedRates.clear();
	mX.clear();

	const AQLDataHolder* dh;
	//index type
	dh = &(indexinfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));		
	AQLString indextype = dynamic_cast<const AQLDataString&>(dh->get()).get();
	indextype.toUpper();
	if (indextype != CPN && indextype != "CPNCF")
	{
		//error
		AQLString msg = "Index type is not CPN or CPNCF";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	//index accessories
	mIsActualCF = false;
	dh = &(indexinfo.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
	AQLString accessory  = dynamic_cast<const AQLDataString&>(dh->get()).get();
	accessory.toUpper();
	vector<AQLString> accessoryVec(accessory.toToken('_'));
	bool IsLegSelected(false);
	for (unsigned int i = 0; i < accessoryVec.size(); i++)
	{
		if (accessoryVec[i].findString(LEG) == 0 && accessoryVec[i].size() > 3)
		{
			mLegNo = accessoryVec[i].subString(3, accessory.size() - 1).getIntValue();
			if (mLegNo > 0)
			{
				mLegNo -= 1;
			}
			else
			{
				//error
				AQLString msg = "Accessory: ";
				msg += accessory;
				msg += " is wrong format";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			IsLegSelected = true;
			mIsActualCF = true;
		}
		else if (IsLegSelected && accessoryVec[i].findString(COUPON) == 0 && accessoryVec[i].size() > 6)
		{
			const AQLDataMultiReference& legs(dynamic_cast<const AQLDataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get()));
			const AQLDataMultiReference& cashlets(dynamic_cast<const AQLDataMultiReference&>(legs.get(mLegNo).get().getData(PRICING_DATA_CASHLETS, ISNOTNULL).get()));
			AQLString tmp1(AQLCoreUtility::removeQuotation(cashlets.convertToString()));
			tmp1.toUpper();
			std::vector<AQLString> cashletnames(tmp1.toToken(':'));

			mCpnNo.resize(cashlets.getSize(), 0);
			for (unsigned int j = 0; j < mCpnNo.size(); j++)
			{
				dh = &(cashlets.get(j).get().getData(PRICING_DATA_COUPONINFOS, NOCHECK));
				if (dh->isNull() || !(dh->isDefined()))
				{
					continue;
				}
				const AQLDataMultiReference& couponinfos(dynamic_cast<const AQLDataMultiReference&>(dh->get()));

				////constant coupon case
				//dh = &(couponinfos.get(0).get().getData(PRICING_DATA_OPERATOR, ISNOTNULL));
				//const function_t couponOperatorType(dynamic_cast<const AQLPriceDataFunction& >(dh->get()).getType());
				//if (couponOperatorType == FN_CONSTANT)
				//{
				//	continue;
				//}

				AQLString tmp2(AQLCoreUtility::removeQuotation(couponinfos.convertToString()));
				tmp2.toUpper();
				std::vector<AQLString> couponNames(tmp2.toToken(':'));

//				AQLString tgtCouponName(legNames[mLegNo] + "_" + cashletnames[j] + "_" + accessoryVec[i]);
				AQLString tgtCouponName(cashletnames[j] + "_" + accessoryVec[i]);

				std::vector<AQLString>::iterator iter(std::find(couponNames.begin(), couponNames.end(), tgtCouponName));
				unsigned int index = std::distance(couponNames.begin(), iter);

				if(index == couponNames.size())
				{
					////Although target coupon is not found, trade is valid when coupon of cashlet is already fixed.
					////under this case size of couponInfo and its index info must be 1, and the index type is fixed rate.
					//if (couponinfos.getSize() == 1)
					//{
					//	dh = &(couponinfos.get(0).get().getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
					//	const AQLDataMultiReference& indexes(dynamic_cast<const AQLDataMultiReference&>(dh->get()));
					//	if (indexes.getSize() == 1)
					//	{
					//		AQLString indextype(dynamic_cast<AQLDataString& >(indexes.get(0).get().getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get());
					//		if(indextype.toUpper() == FIXEDRATE)
					//		{
					//			continue;
					//		}
					//	}
					//}

					//case of not conitued
					AQLString msg;
					msg += accessoryVec[i] + " does not exists in " + cashletnames[j];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				mCpnNo[j] = index;

			}
			mIsActualCF = false;
		}

	}

	if (!IsLegSelected)
	{
		//error
		AQLString msg = "Accessory: ";
		msg += accessory;
		msg += " is wrong format";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//observation start
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLDate& startdate = dynamic_cast<const AQLDataDate&>(dh->get());
		//observation end
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL)); 
		const AQLDate& enddate = dynamic_cast<const AQLDataDate&>(dh->get());
		
		for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
		{
			const AQLDate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
			if (paymentdate >=startdate && paymentdate <= enddate && 
				mPayOff->getPayOff()[mLegNo][i].getPayOff().isCouponPayment())
				mPayOffPos.push_back(i);
		}
		setUpPastRate(basedate, trade, mLegNo, startdate, enddate);

		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const AQLPriceDataFunction&>(dh->get()).getFunction();
		
	}
	else
	{
		//fixing date
		dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL)); 
		const AQLDate& fixingdate = dynamic_cast<const AQLDataDate&>(dh->get());
		if (basedate < fixingdate)
		{
			for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
			{
				const AQLDate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
				if (fixingdate == paymentdate && mPayOff->getPayOff()[mLegNo][i].getPayOff().isCouponPayment())
				{
					mPayOffPos.push_back(i);
					break;
				}			
			}
		
			if (mPayOffPos.size() == 0)
			{
				//error
				AQLString msg = "Fixing date must be fitted some paymentdate";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
		else
		{
			dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATE, NOCHECK)); 
			if (dh->isDefined() && !dh->isNull())
				mFixedRates.push_back(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
			else
				setUpPastRate(trade, mLegNo, fixingdate);
		}


	}
	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		mpCap = dynamic_cast<AQLFunctionBase*>(method.getFunction().clone());
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		mpFloor = dynamic_cast<AQLFunctionBase*>(method.getFunction().clone());
		mIsFloor = true;
	}

	if (mIsCap || mIsFloor)
	{
		//fixing date
		dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL)); 
		const AQLDate& fixingdate = dynamic_cast<const AQLDataDate&>(dh->get());
		setUpCapandFloor(trade, mLegNo, fixingdate, payoff);
	}
	mX.resize(mFixedRates.size() + mPayOffPos.size());



}
/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolCpn::setUp(const AQLDate& basedate,	
						const AQLObject& trade,
						const AQLObject& indexinfo,
						const AQLDate& fixingdate,
						const AQLPricePayOff& payoff)
{
	AQLPriceIndexToolBase::setUp(basedate, trade, indexinfo, fixingdate, payoff);	
	
	mPayOff = &payoff;
	mpObservationOperator = NULL;
	mPayOffPos.clear();
	mFixedRates.clear();
	mX.clear();

	const AQLDataHolder* dh;
	//index type
	dh = &(indexinfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));		
	AQLString indextype = dynamic_cast<const AQLDataString&>(dh->get()).get();
	indextype.toUpper();
	if (indextype != CPN && indextype != "CPNCF")
	{
		//error
		AQLString msg = "Index type is not CPN or CPNCF";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	//index accessories
	mIsActualCF = false;
	dh = &(indexinfo.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
	AQLString accessory  = dynamic_cast<const AQLDataString&>(dh->get()).get();
	accessory.toUpper();
	vector<AQLString> accessoryVec(accessory.toToken('_'));
	bool IsLegSelected(false);
	for (unsigned int i = 0; i < accessoryVec.size(); i++)
	{
		if (accessoryVec[i].findString(LEG) == 0 && accessoryVec[i].size() > 3)
		{
			mLegNo = accessoryVec[i].subString(3, accessory.size() - 1).getIntValue();
			if (mLegNo > 0)
			{
				mLegNo -= 1;
			}
			else
			{
				//error
				AQLString msg = "Accessory: ";
				msg += accessory;
				msg += " is wrong format";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			IsLegSelected = true;
			mIsActualCF = true;
		}
		else if (IsLegSelected && accessoryVec[i].findString(COUPON) == 0 && accessoryVec[i].size() > 6)
		{
			const AQLDataMultiReference& legs(dynamic_cast<const AQLDataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get()));
			const AQLDataMultiReference& cashlets(dynamic_cast<const AQLDataMultiReference&>(legs.get(mLegNo).get().getData(PRICING_DATA_CASHLETS, ISNOTNULL).get()));
			AQLString tmp1(AQLCoreUtility::removeQuotation(cashlets.convertToString()));
			tmp1.toUpper();
			std::vector<AQLString> cashletnames(tmp1.toToken(':'));

			mCpnNo.resize(cashlets.getSize(), 0);
			for (unsigned int j = 0; j < mCpnNo.size(); j++)
			{
				dh = &(cashlets.get(j).get().getData(PRICING_DATA_COUPONINFOS, NOCHECK));
				if (dh->isNull() || !(dh->isDefined()))
				{
					continue;
				}
				const AQLDataMultiReference& couponinfos(dynamic_cast<const AQLDataMultiReference&>(dh->get()));

				////constant coupon case
				//dh = &(couponinfos.get(0).get().getData(PRICING_DATA_OPERATOR, ISNOTNULL));
				//const function_t couponOperatorType(dynamic_cast<const AQLPriceDataFunction& >(dh->get()).getType());
				//if (couponOperatorType == FN_CONSTANT)
				//{
				//	continue;
				//}

				AQLString tmp2(AQLCoreUtility::removeQuotation(couponinfos.convertToString()));
				tmp2.toUpper();
				std::vector<AQLString> couponNames(tmp2.toToken(':'));

//				AQLString tgtCouponName(legNames[mLegNo] + "_" + cashletnames[j] + "_" + accessoryVec[i]);
				AQLString tgtCouponName(cashletnames[j] + "_" + accessoryVec[i]);

				std::vector<AQLString>::iterator iter(std::find(couponNames.begin(), couponNames.end(), tgtCouponName));
				unsigned int index = std::distance(couponNames.begin(), iter);

				if(index == couponNames.size())
				{
					////Although target coupon is not found, trade is valid when coupon of cashlet is already fixed.
					////under this case size of couponInfo and its index info must be 1, and the index type is fixed rate.
					//if (couponinfos.getSize() == 1)
					//{
					//	dh = &(couponinfos.get(0).get().getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
					//	const AQLDataMultiReference& indexes(dynamic_cast<const AQLDataMultiReference&>(dh->get()));
					//	if (indexes.getSize() == 1)
					//	{
					//		AQLString indextype(dynamic_cast<AQLDataString& >(indexes.get(0).get().getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get());
					//		if(indextype.toUpper() == FIXEDRATE)
					//		{
					//			continue;
					//		}
					//	}
					//}

					//case of not conitued
					AQLString msg;
					msg += accessoryVec[i] + " does not exists in " + cashletnames[j];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				mCpnNo[j] = index;

			}
			mIsActualCF = false;
		}

	}

	if (!IsLegSelected)
	{
		//error
		AQLString msg = "Accessory: ";
		msg += accessory;
		msg += " is wrong format";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}


	// observation enddate
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLString& endstr = dynamic_cast<const AQLDataString&>(dh->get()).get();

		const AQLPriceDataSlidingRule* psrule;
		const AQLPriceDataCalendar* pcal;
		AQLPriceCFGenUtility::getBusDayRuleAndCalendar(indexinfo, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexinfo,
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,		
													psrule,
													pcal);

		const AQLDate& enddate
			= AQLMathDateCalculations::getDate(fixingdate, endstr, 
										*psrule,
										pcal,
										false);	



		// observation startdate
		AQLDate startdate;
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			startdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
			if (psrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
				startdate = psrule->getDate(startdate, *pcal);		
		}
		else
		{
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTTERM, ISNOTNULL));
			const AQLString& startstr = dynamic_cast<const AQLDataString&>(dh->get()).get();
			startdate = AQLMathDateCalculations::getDate(fixingdate, startstr, 
										*psrule,
										pcal,
										false);		
		}


		for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
		{
			const AQLDate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
			if (paymentdate >=startdate && paymentdate <= enddate && 
				mPayOff->getPayOff()[mLegNo][i].getPayOff().isCouponPayment())
				mPayOffPos.push_back(i);
		}
		setUpPastRate(basedate, trade, mLegNo, startdate, enddate);

		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const AQLPriceDataFunction&>(dh->get()).getFunction();
		
	}
	else
	{
		if (basedate < fixingdate)
		{
			for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
			{
				const AQLDate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
				if (fixingdate == paymentdate && mPayOff->getPayOff()[mLegNo][i].getPayOff().isCouponPayment())
				{
					mPayOffPos.push_back(i);
					break;
				}			
			}
			if (mPayOffPos.size() == 0)
			{
				//error
				AQLString msg = "Fixing date must be fitted some paymentdate";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
		else
		{
			dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATE, NOCHECK)); 
			if (dh->isDefined() && !dh->isNull())
				mFixedRates.push_back(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
			else
				setUpPastRate(trade, mLegNo, fixingdate);
		}
	}

	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		mpCap = dynamic_cast<AQLFunctionBase*>(method.getFunction().clone());
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		mpFloor = dynamic_cast<AQLFunctionBase*>(method.getFunction().clone());
		mIsFloor = true;
	}
	if (mIsCap || mIsFloor)
	{
		setUpCapandFloor(trade, mLegNo, fixingdate, payoff);
	}


	mX.resize(mFixedRates.size() + mPayOffPos.size());

}

/*!
	@brief set up past rate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] fixingdate fixing date
*/
void
AQLPriceIndexToolCpn::setUpPastRate(const AQLObject& trade,
									unsigned int legNo,
									const AQLDate& fixingdate)
{
	//leg object
	const AQLDataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());
	const bool isSavePFixing = isSavePastFixing(trade);
	unsigned int pos;
	if (isSavePFixing)
	{
		DateVector paymentdates;
		DoubleArray coupons;
		dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL));
			coupons = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
		}

		if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, fixingdate, 0, paymentdates.size() - 1, pos))
		{
			mFixedRates.push_back(0.0);
		}
		else
		{
			mFixedRates.push_back(coupons.at(pos));
		}
	}
	else
	{
		// past coupon
		dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)); 
		const DateVector& paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
		const DoubleArray& coupons = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
		if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, fixingdate, 0, paymentdates.size() - 1, pos))
		{
			//error
			AQLString msg = "Past Coupon at fixingdate(";
			msg += AQLDataDate(fixingdate).convertToString();
			msg += ") is needed.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		mFixedRates.push_back(coupons.at(pos));
	}
}
/*!
	@brief set up past rate
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] observationstartdate observation start date
	@param[in] observationenddate observation end date
*/
void
AQLPriceIndexToolCpn::setUpPastRate(const AQLDate& basedate,	
								const AQLObject& trade,
									unsigned int legNo,
									const AQLDate& observationstartdate,
									const AQLDate& observationenddate)
{

	//leg object
	const AQLDataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());

	// past coupon
	//get past paymentdate
	DateVector needdates;
	dh = &(legs.get(legNo).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < cashlets.getSize(); i++)
	{
		dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment			
		dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const AQLDate& paymentdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
		if (paymentdate < observationstartdate) continue;
		if (paymentdate > basedate || paymentdate > observationenddate) break;
		needdates.push_back(paymentdate);
	}		
	if (needdates.size() > 0)
	{
		const bool isSavePFixing = isSavePastFixing(trade);
		if (isSavePFixing)
		{
			DateVector paymentdates;
			DoubleArray coupons;
			dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL));
				coupons = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			}
			for (unsigned int i = 0; i < needdates.size(); i++)
			{
				unsigned int pos;
				if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
				{
					mFixedRates.push_back(0.0);
				}
				else
				{
					mFixedRates.push_back(coupons.at(pos));	
				}
			}
		}
		else
		{
			// past coupon
			dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)); 
			const DateVector& paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			dh = &(legs.get(legNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
			const DoubleArray& coupons = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			for (unsigned int i = 0; i < needdates.size(); i++)
			{
				unsigned int pos;
				if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
				{
					//error
					AQLString msg = "Past Coupon at ";
					msg += AQLDataDate(needdates[i]).convertToString();
					msg += " is needed.";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				mFixedRates.push_back(coupons.at(pos));	
			}
		}
	}	


	// past coupon
/*	dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK)); 
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
		const DoubleArray& coupons = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
		for (unsigned int i = 0; i < paymentdates.size(); i++)
			if (paymentdates[i] >=observationstartdate && paymentdates[i] <= observationenddate)
				mFixedRates.push_back(coupons.at(i));
	}*/
}

/*!
	@brief set up cap and floor condition
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] fixingdate fixing date
	@param[in] payoff payoff
*/
void
AQLPriceIndexToolCpn::setUpCapandFloor(const AQLObject& trade,	 
				 unsigned int legNo, 
				 const AQLDate& fixingdate,
				 const AQLPricePayOff& payoff)
{
	(void)payoff; //20070411--Nagase--gcc
	mLatestPos = -1;
	for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
	{
		const AQLDate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
		if (paymentdate <= fixingdate) 
			mLatestPos = i;
		else
			break;
	}
	//leg object
	const AQLDataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());
	// past coupon
	mFixedRatesforCapFloor.clear();
	dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK)); 
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
		const DoubleArray& coupons = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
		for (unsigned int i = 0; i < paymentdates.size(); i++)
			if (paymentdates[i] <= fixingdate)
				mFixedRatesforCapFloor.push_back(coupons.at(i));
	}
	mX2.clear();
	mX2.resize(mFixedRatesforCapFloor.size() + mLatestPos + 1 + 1);

}

/*!
    @brief constructor
*/
AQLPriceIndexToolCpnCF::AQLPriceIndexToolCpnCF()
: AQLPriceIndexToolCpn()
{

}
/*!
    @brief destructor

*/
AQLPriceIndexToolCpnCF::~AQLPriceIndexToolCpnCF()
{

}

/*!
	@brief make copy(clone) of this class
    @return deep copy of this class
*/
AQLPriceIndexToolBase*
AQLPriceIndexToolCpnCF::clone() const
{
    try 
	{
        return new AQLPriceIndexToolCpnCF(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolCpnCF::setUp(const AQLDate& basedate,
							const AQLDate& paydate,
							const AQLObject& trade,
							const AQLObject& indexinfo,
							const AQLPricePayOff& payoff)
{
	AQLPriceIndexToolCpn::setUp(basedate, paydate, trade, indexinfo, payoff);	
}
/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolCpnCF::setUp(const AQLDate& basedate,	
							const AQLObject& trade,
							const AQLObject& indexinfo,
							const AQLDate& fixingdate,
							const AQLPricePayOff& payoff)
{
	AQLPriceIndexToolCpn::setUp(basedate, trade, indexinfo, fixingdate, payoff);	
}

/*!
	@brief set up past rate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] fixingdate fixing date
*/
void
AQLPriceIndexToolCpnCF::setUpPastRate(const AQLObject& trade,
									unsigned int legNo,
									const AQLDate& fixingdate)
{
	//leg object
	const AQLDataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());
	const bool isSavePFixing = isSavePastFixing(trade);
	unsigned int pos;
	if (isSavePFixing)
	{
		DateVector paymentdates;
		DoubleArray couponcfs;
		dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL));
			couponcfs = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
		}

		if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, fixingdate, 0, paymentdates.size() - 1, pos))
		{
			mFixedRates.push_back(0.0);
		}
		else
		{
			mFixedRates.push_back(couponcfs.at(pos));
		}
	}
	else
	{
		// past coupon
		dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)); 
		const DateVector& paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL)); 
		const DoubleArray& couponcfs = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();

		if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, fixingdate, 0, paymentdates.size() - 1, pos))
		{
			//error
			AQLString msg = "Past CouponCF of fixingdate(";
			msg += AQLDataDate(fixingdate).convertToString();
			msg += ") is needed.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		mFixedRates.push_back(couponcfs.at(pos));
	}
}
/*!
	@brief set up past rate
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] observationstartdate observation start date
	@param[in] observationenddate observation end date
*/
void
AQLPriceIndexToolCpnCF::setUpPastRate(const AQLDate& basedate,	
								  const AQLObject& trade,
									unsigned int legNo,
									const AQLDate& observationstartdate,
									const AQLDate& observationenddate)
{

	//leg object
	const AQLDataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());
	
	// past couponcf
	//get past paymentdate
	DateVector needdates;
	dh = &(legs.get(legNo).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < cashlets.getSize(); i++)
	{
		dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment			
		dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const AQLDate& paymentdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
		if (paymentdate < observationstartdate) continue;
		if (paymentdate > basedate || paymentdate > observationenddate) break;
		needdates.push_back(paymentdate);
	}		
	if (needdates.size() > 0)
	{
		
		const bool isSavePFixing = isSavePastFixing(trade);
		if (isSavePFixing)
		{
			DateVector paymentdates;
			DoubleArray couponcfs;
			dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL));
				couponcfs = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			}
			for (unsigned int i = 0; i < needdates.size(); i++)
			{
				unsigned int pos;
				if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
				{
					mFixedRates.push_back(0.0);
				}
				else
				{
					mFixedRates.push_back(couponcfs.at(pos));
				}
			}
		}
		else
		{
			// past coupon
			dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)); 
			const DateVector& paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			dh = &(legs.get(legNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL)); 
			const DoubleArray& couponcfs = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			for (unsigned int i = 0; i < needdates.size(); i++)
			{
				unsigned int pos;
				if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
				{
					//error
					AQLString msg = "Past Coupon at ";
					msg += AQLDataDate(needdates[i]).convertToString();
					msg += " is needed.";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				mFixedRates.push_back(couponcfs.at(pos));
			}
		}
	}	
	
	
/*	// past coupon
	dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK)); 
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL)); 
		const DoubleArray& couponcfs = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
		for (unsigned int i = 0; i < paymentdates.size(); i++)
			if (paymentdates[i] >=observationstartdate && paymentdates[i] <= observationenddate)
				mFixedRates.push_back(couponcfs.at(i));
	}*/
}

/*!
	@brief set up cap and floor condition
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] fixingdate fixing date
	@param[in] payoff payoff
*/
void
AQLPriceIndexToolCpnCF::setUpCapandFloor(const AQLObject& trade,	 
				 unsigned int legNo, 
				 const AQLDate& fixingdate,
				 const AQLPricePayOff& payoff)
{
	(void)payoff; //20070411--Nagase--gcc
	mLatestPos = -1;
	for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
	{
		const AQLDate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
		if (paymentdate <= fixingdate) 
			mLatestPos = i;
		else
			break;
	}
	//leg object
	const AQLDataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());
	// past coupon
	mFixedRatesforCapFloor.clear();
	dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK)); 
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL)); 
		const DoubleArray& coupons = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
		for (unsigned int i = 0; i < paymentdates.size(); i++)
			if (paymentdates[i] <= fixingdate)
				mFixedRatesforCapFloor.push_back(coupons.at(i));
	}
	mX2.clear();
	mX2.resize(mFixedRatesforCapFloor.size() + mLatestPos + 1 + 1);

}










AQLPriceIndexToolCompound::AQLPriceIndexToolCompound() : AQLPriceIndexTool(), mCompoundTerms(), mMargins() {}
AQLPriceIndexToolCompound::~AQLPriceIndexToolCompound() {}

AQLPriceIndexToolBase* AQLPriceIndexToolCompound::clone() const
{
    try{
        return new AQLPriceIndexToolCompound(*this);
    } catch (bad_alloc & e) {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

void AQLPriceIndexToolCompound::setUpFixingDates(const AQLObject& indexInfo)
{	
    const AQLDataHolder* dh;
    dh = &indexInfo.getData(PRICING_DATA_FIXINGDATES);
    if(dh->isDefined() && !dh->isNull()){
        fixingDates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
        return;
    }




	const AQLDate& start  = dynamic_cast<const AQLDataDate&>(indexInfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, ISNOTNULL).get());
	const AQLDate& end    = dynamic_cast<const AQLDataDate&>(indexInfo.getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL).get());
	const AQLString& freq = dynamic_cast<const AQLDataString&>(indexInfo.getData(PRICING_DATA_OBSERVATIONFREQUENCY, ISNOTNULL).get());




    const int* pday = NULL;
    int temp_day;
	dh = &indexInfo.getData(PRICING_DATA_OBSERVATIONDAY); 
    if (dh->isDefined() && !dh->isNull()){
        temp_day = dynamic_cast<const AQLDataInt&>(dh->get()).get();
        pday = &temp_day;
    }
	
	const AQLPriceDataSlidingRule* psrule;
	const AQLPriceDataCalendar* pcal ;
	AQLPriceCFGenUtility::getBusDayRuleAndCalendar(indexInfo, 
                                               PRICING_DATA_OBSERVATIONSLIDINGRULE,
                                               PRICING_DATA_OBSERVATIONCALENDAR,
                                               indexInfo,
                                               CALIBRATION_DATA_SLIDINGRULE,
                                               CALIBRATION_DATA_CALENDAR,
                                               psrule, 
                                               pcal);

	
    fixingDates.clear();
	AQLMathDateCalculations::generateSchedule(start, 
                                   end, 
                                   freq,	
                                   false,
                                   NULL, 
                                   NULL,
                                   pday,
                                   fixingDates,
                                   psrule,
                                   pcal);

    const AQLDate tmpDate = pcal != NULL ? psrule->getDate(end, *pcal) : end;
	if (fixingDates.back() != tmpDate) fixingDates.push_back(tmpDate);
}







/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolCompound::setUp(const AQLDate& basedate,
                                  const AQLDate& paydate,
                                  const AQLObject& trade,
                                  const AQLObject& indexinfo,
                                  const AQLPricePayOff& payoff)
{

	AQLPriceIndexToolBase::setUp(basedate, paydate, trade, indexinfo, payoff);

	mX.clear();
	mpObservationOperator = NULL;
	mpIndex = NULL;
	mCompoundTerms.clear();
	mMargins.clear();





	const AQLDataHolder* dh;
	dh = &(indexinfo.getData(PRICING_DATA_INDEXENTITY, ISNOTNULL));
	const AQLDataReference& ref = dynamic_cast<const AQLDataReference&>(dh->get());
	mpIndex = &dynamic_cast<const AQLMathIndexEntity&>(ref.get().get());
	const DateVector& grid = mpIndex->getGrid();





	
    flagForCompoundAllDays = (dh = &indexinfo.getData(PRICING_DATA_COMPOUND_ON_ALL_DAYS))->isDefined() && !dh->isNull() ? dynamic_cast<const AQLDataBool&>(dh->get()).get() : false;
    setUpFixingDates(indexinfo);
    setUpStartAndEndDates(indexinfo);
	setUpIndexPosAndFixedRate(basedate, indexinfo, trade);






    




	


	
	unsigned int pos;
	const bool isNeedPastRate = fixingDates[0] < basedate;
	const bool isSavePFixing = isSavePastFixing(trade);



	

    

	
	if(flagForCompoundAllDays){
		unsigned int comp_size = 0;
		for(unsigned int i = 0; i < fixingDates.size(); i++){
			comp_size += cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]);
		}
		mCompoundTerms.resize(comp_size);
		unsigned int temp_pos = 0;
		for(unsigned int i = 0; i < fixingDates.size(); i++){
			AQLDate temp_start, temp_end;
			temp_start = temp_end = cfCalcStartDates[i];
			temp_end.addDays(1);
			while(temp_start != cfCalcEndDates[i]){
				mCompoundTerms.at(temp_pos++) = mCompoundDC.getTerm(temp_start, temp_end, false);
				temp_start.addDays(1);
				temp_end.addDays(1);
			}
		}
	}
	else{
		unsigned int outSize = fixingDates.size();
		mCompoundTerms.resize(outSize);
		for (unsigned int j = 0; j < outSize; j++)
		{
			mCompoundTerms[j] = mCompoundDC.getTerm(cfCalcStartDates[j],cfCalcEndDates[j],false);
		}
	}
	unsigned int chksize = mFixedRates.size() + mIndexPos.size();
	if (chksize != mCompoundTerms.size())
		throw AQLCoreInvalidData("Compounding size error",__FILE__,__LINE__);
	
	//add margin rate
	double basemargin = 0.0;
	dh = &(indexinfo.getData(PRICING_DATA_COMPOUNDINGMARGIN, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		basemargin = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	
	mMargins.resize(chksize,basemargin);
	//chk compounding margins
	dh = &(indexinfo.getData(PRICING_DATA_COMPOUNDINGDATES, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& compdates = dynamic_cast<const AQLDataDates &>(dh->get()).get();
		dh = &(indexinfo.getData(PRICING_DATA_COMPOUNDINGMARGINS, ISNOTNULL));
		const DoubleVector& compmargins = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();

		if (compdates.size() != compmargins.size())
			throw AQLCoreInvalidData("Compounding dates size error",__FILE__,__LINE__);

		for (unsigned int i = 0; i < compdates.size(); i++)
		{
			if (!AQLAlgorithm::find<DateVector, AQLDate>(fixingDates, compdates[i], 0, fixingDates.size() - 1, pos))
			{
				//error
				AQLString msg = "Compounding dates error";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			mMargins[pos] = compmargins[i];
		}
	}

	//chk compounding past margins
	//past rate
	if (isNeedPastRate)
	{
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONMARGINS, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DoubleArray& observationmargins = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
			const DateVector& observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			if (observationdates.size() != observationmargins.size())
			{
				throw AQLCoreInvalidData("ObservationDates size and ObservationMargins size are not same.", __FILE__, __LINE__);
			}
			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
				if (basedate > fixingDates[i]) //past
				{
					if (!AQLAlgorithm::find<DateVector, AQLDate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						if (!isSavePFixing || (!observationdates.empty() && observationdates.back() > fixingDates[i]))
						{
							//error
							AQLString msg = "Index rate at ";
							msg += AQLDataDate(fixingDates[i]).convertToString();
							msg += " is needed.";
							throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
					}
					mMargins[i]= observationmargins[pos];
				}
				else
				{
					break;
				}
			}
		}			
	}

	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			AQLString msg = "Only AQLConstant function is available";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mCap = dynamic_cast<const AQLConstant&>(method.getFunction()).getParam().at(0);
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLPriceDataFunction& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			AQLString msg = "Only AQLConstant function is available";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mFloor = dynamic_cast<const AQLConstant&>(method.getFunction()).getParam().at(0);
		mIsFloor = true;	
	}

	mX.resize(chksize * 3);
}


/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff AQLPricePayOff object
*/
void
AQLPriceIndexToolCompound::setUp(const AQLDate& basedate,	
						const AQLObject& trade,
						const AQLObject& indexinfo,
						const AQLDate& fixingdate,
						const AQLPricePayOff& payoff)
{
	
}

void 
AQLPriceIndexToolCompound::setUpIndexPosAndFixedRate(const AQLDate& basedate,
													  const AQLObject& indexInfo,
													  const AQLObject& trade)
{
    const AQLDataHolder* dh;

    const DateVector& grid = mpIndex->getGrid();

	unsigned int pos;
	bool isNeedPastRate = false;
	bool isBaseContain = false;
	bool IsPastRateCompounded = false;
	for (unsigned int i = 0; i < fixingDates.size(); i++)
	{
        const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;
		
		if (basedate <= fixingDates[i]) //future
		{
			if (!AQLAlgorithm::find<DateVector, AQLDate>(grid, fixingDates[i], 0, grid.size() - 1, pos))
			{
				//error
				AQLString msg = "AQLMathIndexEntity grid is inconsistent with index observation dates";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			if (basedate == fixingDates[i])
			{
				isBaseContain = true;
			}
            for(unsigned int j = 0; j < pushed_num; j++) mIndexPos.push_back(pos);
		}
		else
		{
			isNeedPastRate = true;
		}
	}	

	dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATESTYPE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		AQLString ortype = dynamic_cast<const AQLDataString&>(dh->get()).get();
		if (ortype == "Compounded")
		{
			IsPastRateCompounded = true;
		}
		else
		{
			//error
			AQLString msg = "ObservationRatesType ";
			msg += AQLDataString(ortype).convertToString();
			msg += " is not defined.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}


	//past rate
	const bool isSavePFixing = isSavePastFixing(trade);
	if (isNeedPastRate && !IsPastRateCompounded)
	{

		if (isSavePFixing)
		{
			DateVector observationdates;
			DoubleArray observationrates;
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			}
			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
                const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;

				if (basedate > fixingDates[i]) //past
				{
					if (!AQLAlgorithm::find<DateVector, AQLDate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						if (!observationdates.empty() && observationdates.back() > fixingDates[i])
						{
							//error
							AQLString msg = "Index rate at ";
							msg += AQLDataDate(fixingDates[i]).convertToString();
							msg += " is needed.";
							throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						for(unsigned int j = 0; j < pushed_num; j++) mIndexPos.insert(mIndexPos.begin(), 0);
					}
					else
					{
						for(unsigned int j = 0; j < pushed_num; j++) mFixedRates.push_back(observationrates.at(pos));
					}
				}
				else
				{
					break;
				}
			}

		}
		else
		{
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
			const DateVector& observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
			const DoubleArray& observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			if (observationdates.size() != observationrates.size())
			{
				throw AQLCoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
			}
			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
                const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;
				if (basedate > fixingDates[i]) //past
				{
					if (!AQLAlgorithm::find<DateVector, AQLDate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						//error
						AQLString msg = "Index rate at ";
						msg += AQLDataDate(fixingDates[i]).convertToString();
						msg += " is needed.";
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					for(unsigned int j = 0; j < pushed_num; j++) mFixedRates.push_back(observationrates.at(pos));
				}
				else
				{
					break;
				}
			}
		}	
	}
	else if (isNeedPastRate && IsPastRateCompounded)
	{
		if (isSavePFixing)
		{
			DateVector observationdates;
			DoubleArray observationrates;
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();

				//check the size of observation dates and observation rates
				if (observationdates.size() != 1 || observationrates.size() != 1)
				{
					//error
					AQLString msg = "observationdates.size() != 1 || observationrates.size() != 1";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
			}

			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
				const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;

				if (basedate > fixingDates[i]) //past
				{
					if (!AQLAlgorithm::find<DateVector, AQLDate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						if (!observationdates.empty() && observationdates.back() > fixingDates[i])
						{
							mFixedRates.push_back(0);
							////error
							//AQLString msg = "Index rate at ";
							//msg += AQLDataDate(fixingDates[i]).convertToString();
							//msg += " is needed.";
							//throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						else
						{
							for (unsigned int j = 0; j < pushed_num; j++) mIndexPos.insert(mIndexPos.begin(), 0);
						}
					}
					else
					{
						const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&>(indexInfo.getData(PRICING_DATA_COMPOUNDINGDAYCOUNT, ISNOTNULL).get());
						double compoundingterm = dc.getTerm(cfCalcStartDates[i], cfCalcEndDates[i], false);
						if (compoundingterm < DBL_MIN)
						{
							//error
							AQLString msg = "compoundingterm < DBL_MIN";
							throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						for (unsigned int j = 0; j < pushed_num - 1; j++) mFixedRates.push_back(0);
						mFixedRates.push_back(observationrates.at(pos) / compoundingterm);
					}
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
			const DateVector& observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
			const DoubleArray& observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			if (observationdates.size() != 1 || observationrates.size() != 1)
			{
				throw AQLCoreInvalidData("ObservationDates size or ObservationRates size are not one.", __FILE__, __LINE__);
			}
			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
				const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;
				if (basedate > fixingDates[i]) //past
				{
					if (!AQLAlgorithm::find<DateVector, AQLDate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						mFixedRates.push_back(0);
						////error
						//AQLString msg = "Index rate at ";
						//msg += AQLDataDate(fixingDates[i]).convertToString();
						//msg += " is needed.";
						//throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					else 
					{
						const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&>(indexInfo.getData(PRICING_DATA_COMPOUNDINGDAYCOUNT, ISNOTNULL).get());
						double compoundingterm = dc.getTerm(cfCalcStartDates[i], cfCalcEndDates[i], false);
						if (compoundingterm < DBL_MIN)
						{
							//error
							AQLString msg = "compoundingterm < DBL_MIN";
							throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						for (unsigned int j = 0; j < pushed_num - 1; j++) mFixedRates.push_back(0);
						mFixedRates.push_back(observationrates.at(pos) / compoundingterm);
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	
	// basedate rate
	if (isBaseContain && !IsPastRateCompounded)
	{
		dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DateVector& observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			if (AQLAlgorithm::find<DateVector, AQLDate>(observationdates, basedate, 0, observationdates.size() - 1, pos))
			{
				dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				const DoubleArray& observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
				if (observationdates.size() != observationrates.size())
				{
					throw AQLCoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
				}

                unsigned int pos2;
                AQLAlgorithm::find(fixingDates, basedate, 0, fixingDates.size()-1, pos2);
                const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[pos2].intervalDays(cfCalcEndDates[pos2]) : 1;
                for(unsigned int i = 0; i < pushed_num; i++){
                    mFixedRates.push_back(observationrates.at(pos));
                    mIndexPos.erase(mIndexPos.begin());
                }
			}
		}
	}
	else if (isBaseContain && IsPastRateCompounded)
	{
		dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DateVector& observationdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			if (AQLAlgorithm::find<DateVector, AQLDate>(observationdates, basedate, 0, observationdates.size() - 1, pos))
			{
				dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				const DoubleArray& observationrates = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
				if (observationdates.size() != 1 || observationrates.size() != 1)
				{
					throw AQLCoreInvalidData("ObservationDates size or ObservationRates size are not one.", __FILE__, __LINE__);
				}

				unsigned int pos2;
				AQLAlgorithm::find(fixingDates, basedate, 0, fixingDates.size() - 1, pos2);
				const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[pos2].intervalDays(cfCalcEndDates[pos2]) : 1;
				for (unsigned int i = 0; i < pushed_num - 1; i++) {
					mFixedRates.push_back(0);
					mIndexPos.erase(mIndexPos.begin());
				}
				const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&>(indexInfo.getData(PRICING_DATA_COMPOUNDINGDAYCOUNT, ISNOTNULL).get());
				double compoundingterm = dc.getTerm(cfCalcStartDates[pos2], cfCalcEndDates[pos2], false);
				if (compoundingterm < DBL_MIN)
				{
					//error
					AQLString msg = "compoundingterm < DBL_MIN";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				mFixedRates.push_back(observationrates.at(pos) / compoundingterm);
				mIndexPos.erase(mIndexPos.begin());
			}
		}
	}
}

void 
AQLPriceIndexToolCompound::setFixingInfo(DateVector& fixing_date, AQLStringVector& fixing_flag) const
{
	if(flagForCompoundAllDays){
		for(size_t i = 0; i < fixingDates.size(); i++){
			for(AQLDate d = cfCalcStartDates[i]; d < cfCalcEndDates[i]; d.addDays(1)){
				fixing_date.push_back(fixingDates[i]);
			}
		}
	}
	else{
        for(DateVector::const_iterator it = fixingDates.begin(); it != fixingDates.end(); ++it){ fixing_date.push_back(*it); }
	}
	
    for(DoubleVector::const_iterator it = mFixedRates.begin(); it != mFixedRates.end(); ++it){ fixing_flag.push_back("Y"); }
	for(size_t i = 0; i < mIndexPos.size(); i++){ fixing_flag.push_back("N"); }
}

void AQLPriceIndexToolCompound::setUpStartAndEndDates(const AQLObject& indexInfo) 
{
    const AQLDataHolder* dh;
    
    
    
    
    dh = &indexInfo.getData(PRICING_DATA_CFCALCSTARTDATES);
    if(dh->isDefined() && !dh->isNull()){
        cfCalcStartDates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
        try{
            dh = &indexInfo.getData(PRICING_DATA_CFCALCENDDATES, ISNOTNULL);
            cfCalcEndDates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
        } catch(AQLCoreInvalidData& e){
            stringstream sst;
            sst << "There is CFCalcStartDates but is not CFCalcEndDates." << endl;
            e.addMsg(sst.str().c_str());
            throw e;
        }

        if(fixingDates.size() != cfCalcStartDates.size() ||
            fixingDates.size() != cfCalcEndDates.size()){
                throw AQLCoreInvalidData("FixingDates and CFCalcStart/EndDates sizes are inconsistent", __FILE__, __LINE__);
        }
        return;
    }




    cfCalcStartDates = fixingDates;
    cfCalcEndDates.resize(fixingDates.size());
    copy(++fixingDates.begin(), fixingDates.end(), cfCalcEndDates.begin());



	const AQLString& acstr = dynamic_cast<const AQLDataString&>(indexInfo.getData(PRICING_DATA_ACCESSORY, ISNOTNULL).get()).get();
	const AQLPriceDataSlidingRule* psrule;
	const AQLPriceDataCalendar* pcal ;
	AQLPriceCFGenUtility::getBusDayRuleAndCalendar(indexInfo, 
                                               PRICING_DATA_OBSERVATIONSLIDINGRULE,
                                               PRICING_DATA_OBSERVATIONCALENDAR,
                                               indexInfo,
                                               CALIBRATION_DATA_SLIDINGRULE,
                                               CALIBRATION_DATA_CALENDAR,
                                               psrule, pcal);
	const AQLDate finaldate = AQLMathDateCalculations::getDate(fixingDates.back(), acstr, *psrule, pcal, true);
    cfCalcEndDates.back() = finaldate;
}

void AQLPriceIndexToolCompound::calcIndices(DoubleVector& indices, const size_t start_pos, const size_t end_pos) const
{
    if(end_pos-start_pos != mFixedRates.size() + mIndexPos.size()) throw AQLCoreInvalidData("compound term size is inconsistent.", __FILE__, __LINE__);

    size_t pos = start_pos;
    for(size_t i = 0; i < mFixedRates.size(); i++) indices[pos++] = mFixedRates[i];
    for(size_t i = 0; i < mIndexPos.size(); i++) indices[pos++] = mpIndex->getIndex()[mIndexPos[i]];

    pos = start_pos;
    for(size_t i = 0; i < mFixedRates.size() + mIndexPos.size(); i++){
        round(indices[pos]);
        discountadjust(indices[pos]);
        capfloor(indices[pos]);
        pos++;
    }
}

void
AQLPriceIndexToolCompound::setFixingInfo(AQLDate& fixing_date, AQLString& fixing_flag) const
{
    fixing_date = fixingDates.back();
    fixing_flag = mFixedRates.size() == mCompoundTerms.size() ? "Y" : "N";
}
