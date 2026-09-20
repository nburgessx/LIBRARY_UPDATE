/*! @file
    @brief Source code for class to generate YieldCurve.
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "ConstantDeclarations.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLMathDefine.h"
#include "AQLObject.h"
#include "AQLDataProcedure.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLInterpolationBase.h"
#include "AQLBasic.h"
#include "AQLMatrix.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataManager.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "AQLObjectPool.h"
#include "AQLDataInstance.h"

#include <algorithm>
#include "AQLMathYieldCurvePro.h"
#include "AQLDateCalculations.h"
#include "AQLAlgorithm.h"
#include "AQLLinearInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"

#define O_N		"O_N"
#define T_N		"T_N"
#define ON		"ON"
#define TN		"TN"
#define ZERO	"ZERORATE"
#define PAR		"PARRATE"
#define FWD		"FWDRATE"
#define BASIS	"BASISRATE"
// Represents Monetary Policy Swaps i.e. central bank meeting date swaps e.g. BOJ, FedFunds, ECB et al.
#define BOJ	    "BOJRATE"
// Represents Monetary Policy Swaps i.e. central bank meeting date swaps e.g. BOJ, FedFunds, ECB et al.
#define FEDFUNDRATE	"FFRATE"
#define STD	"STD"
#define FRA3M	"FRA3M"	
#define FRA6M	"FRA6M"
#define TERM_3M	"3M"	
#define TERM_6M	"6M"
#define TERM_1M	"1M"

#define SIMPLE		"SIMPLE"
#define ANNUAL		"ANNUAL"
#define SEMI_ANNUAL	"SEMI-ANNUAL"
#define QUARTERLY	"QUARTERLY"
#define MONTHLY		"MONTHLY"
#define LUNAR		"LUNAR"

#define ROLLCONV_NORMAL "NORMAL"
#define ROLLCONV_EOM "EOM"
#define ROLLCONV_LUNAR "LUNAR"

#define LIBOR		"LIBOR"
#define FUTURE		"FUTURE"
#define SWAP		"SWAP"

#define ARITHAVG	"ARITHMETICAVERAGE"
#define SHORTMARKET	"SHORTMARKET"
//#define LOBASIS	"LOBASIS"

using namespace std;

const int GRIDSPAN = 20;
/*!
	@brief get date for moneymarket except O_N and T_N
    @param[in] basedate basedate
	@param[in] termstr string of such as "2D_1D" or "2D_1W"
	@param[in] calendar calendar
	@param[in] srule holiday sliding rule
	@param[in,out] start start date
	@param[in,out] end end date
*/
static void getDate(const AQLDate& basedate, const AQLString& termstr, const AQLPriceDataCalendar& calendar, const AQLPriceDataSlidingRule& srule, AQLDate& start, AQLDate& end)
{	
	AQLString str = termstr;
	str.trimLeft();
	str.trimRight();
	AQLStringVector str_v = str.toToken('_');
	if (str_v.size() != 2 || str_v[0].size() < 2 || str_v[1].size() < 2)
	{
		//error
		throw AQLCoreInvalidData("Input string format is something wrong", __FILE__, __LINE__);
	}
	unsigned int size = str_v[0].size();
	str = str_v[0].subString(size - 1, size - 1);
	str.toUpper();
	if (str != "D")
	{
		//error
		throw AQLCoreInvalidData("Input string format is something wrong", __FILE__, __LINE__);
	}
	AQLString str2 = str_v[0].subString(0, size - 2);
	char * pFirstNonNumber;
    int d = strtol( str2.getCString(), &pFirstNonNumber, 10 ); // base 10 numbers
	start = calendar.getBusinessDay(basedate, d);
	end = start;
	size = str_v[1].size();
	str = str_v[1].subString(size - 1, size - 1);
	str.toUpper();
	str2 = str_v[1].subString(0, size - 2);
	if (str == "D")
	{
        char * pFirstNonNumber;
		d = strtol(str2.getCString(), &pFirstNonNumber, 10 ); // base 10 numbers
		end.addDays(d);		
	}
	else if (str == "W")
	{
        char * pFirstNonNumber;
		d = strtol( str2.getCString(), &pFirstNonNumber, 10 ) * 7; // base 10 numbers
		end.addDays(d);		
	}
	else if (str == "M")
	{
        char * pFirstNonNumber;
		end.addMonths( strtol(str2.getCString(), &pFirstNonNumber, 10 ) ); // base 10 number
	}
	else if (str == "Y")
	{
        char * pFirstNonNumber;
		end.addYears( strtol(str2.getCString(), &pFirstNonNumber, 10 ) ); // base 10 number
	}
	else
	{
		//error
		throw AQLCoreInvalidData("Input string format is something wrong", __FILE__, __LINE__);
	}
	end = srule.getDate(end, calendar);

}

/*
	@Desctiption return a roll convention from a frequency and an end-of-month flag

	@param frequency [in] frequency
	@param eom [in] end-of-month flag
	@return roll convention
*/
AQLString
AQLPriceYieldGenerator::deduceRollConvention(const AQLString& freq, bool eom)
{
	AQLString roll_conv(ROLLCONV_NORMAL);
	if (eom) roll_conv = ROLLCONV_EOM;
	if (freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
	return roll_conv;
}

/*!
	@brief calculate swap payment dates, terms and intervals
	@param[in] basedate basedate of term claculation
    @param[in] sdate startdate
	@param[in] edate enddate
	@param[in] freq frequency
	@param[in] calendar calendar
	@param[in] slidingRule sliding rule
	@param[in] dayCount daycount
	@param[out] dates payment dates
	@param[out] terms payment grid (daycount = ACT/365_ISDA)
	@param[out] terms_interval payment span (daycount = dayCount)
*/
void 
AQLPriceYieldGenerator::getPaymentDates(const AQLDate& basedate, const AQLDate& sdate, const AQLDate& edate, const AQLString& freq,
							const AQLPriceDataCalendar& calendar, const AQLPriceDataSlidingRule& slidingRule, const AQLPriceDataDayCount& dayCount,
							DateVector& dates, DoubleArray& terms, DoubleArray& terms_interval, bool is_eomroll,
							bool isBackward)
{
	dates.clear();
	terms.clear();
	terms_interval.clear();

	if (freq == SIMPLE)
	{
		return;
	}

	AQLString roll_conv(deduceRollConvention(freq, is_eomroll));
	int roll_day = sdate.dayOfMonth();
	AQLDateCalculations::generateSchedule(sdate, edate, freq, true, NULL, NULL, &roll_day, dates, &slidingRule, &calendar, !isBackward, &roll_conv);

	if (dates.size() == 0)
	{
		return;
	}

	terms.resize(dates.size());
	terms_interval.resize(dates.size());

	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	terms[0] = dc_act365.getTerm(basedate, dates[0]);
	terms_interval[0] = dayCount.getTerm(sdate, dates[0], false);
	for (unsigned int i = 1; i < dates.size(); ++i)
	{
		terms[i] = dc_act365.getTerm(basedate, dates[i]);
		terms_interval[i] = dayCount.getTerm(dates[i - 1], dates[i], false);
	}
}

/*!
	@brief calculate swap payment dates, terms and intervals
	@param[in] sdate startdate and base date of term calculation
	@param[in] edate enddate
	@param[in] freq frequency
	@param[in] calendar calendar
	@param[in] slidingRule sliding rule
	@param[in] dayCount daycount
	@param[out] dates payment dates
	@param[out] terms payment grid (daycount = ACT/365_ISDA)
	@param[out] terms_interval payment span (daycount = dayCount)
*/
void
AQLPriceYieldGenerator::getPaymentDates(const AQLDate& sdate, const AQLDate& edate, const AQLString& freq,
							const AQLPriceDataCalendar& calendar, const AQLPriceDataSlidingRule& slidingRule, const AQLPriceDataDayCount& dayCount,
							DateVector& dates, DoubleArray& terms, DoubleArray& terms_interval, bool is_eomroll,
							bool isBackward)
{
	getPaymentDates(sdate, sdate, edate, freq, calendar, slidingRule, dayCount, dates, terms, terms_interval, is_eomroll, isBackward);
}

/*!
	@brief calculate swap rate
    @param[in] df discount factor
    @param[in] sdate startdate
	@param[in] edate enddate
	@param[in] freq roll
	@param[in] calendar calendar
	@param[in] slidingRule holiday sliding rule
	@param[in] dayCount daycount
*/
static double getSwapRate(const AQLInterpolationBase& df, const AQLDate& sdate, const AQLDate& edate, const AQLString& freq,
							const AQLPriceDataCalendar& calendar, const AQLPriceDataSlidingRule& slidingRule, const AQLPriceDataDayCount& dayCount, const bool is_eomroll)
{
	DoubleArray terms, terms_interval;
	DateVector dates;

	AQLPriceYieldGenerator::getPaymentDates(sdate, edate, freq, calendar, slidingRule, dayCount, dates, terms, terms_interval, is_eomroll);

	double sum = 0.0;
	for (unsigned int i = 0; i < terms.size(); i++)
		sum += df.value(terms[i]) * terms_interval[i];
	return (1.0 - df.value(terms.back())) / sum;
}



/*!
    @brief  constructor
*/
AQLPriceYieldGenerator::AQLPriceYieldGenerator()
{
}
/*!
    @brief  destructor
*/
AQLPriceYieldGenerator::~AQLPriceYieldGenerator()
{
}
/*!
    @brief  Check function for this class ID

    @param[in] id type of class(function_t)
    @return true or false
*/
bool
AQLPriceYieldGenerator::isTypeOf(function_t id) const
{
    return (id == FN_IRYIELDGENERATOR ? true :
                        AQLCoreProcedure::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

    @return copy object
*/
AQLCoreFunctionBase*     
AQLPriceYieldGenerator::clone() const
{
    try 
    {
        return new AQLPriceYieldGenerator();
    }
    catch (bad_alloc & e)
    {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }   
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceYieldGenerator::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(CALIBRATION_DATA_ASOFDATE, DATA_DATE);
	dm.setData(CALIBRATION_DATA_TERMS, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_DFS, DATA_DOUBLES);
	dm.setData(CALIBRATION_DATA_CALENDAR, DATA_CALENDAR);
	dm.setData(CALIBRATION_DATA_SLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_PRICE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_STARTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ENDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_SPREAD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_FUTUREVOLATILITY, DATA_DOUBLE);
	dm.setData(IR_CALIBRATION_DATA_DATATYPE, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_TERM, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_FREQUENCY, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_SPOTDATE, DATA_DATE);
	dm.setData(CALIBRATION_DATA_RATE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RATEPRIORITY, DATA_STRINGS);
	dm.setData(IR_CALIBRATION_DATA_DAYCOUNT, DATA_DAYCOUNT);
}

/*!
    @brief  Return this class type
    @return Function Type(FN_IRYIELDGENERATOR)
*/
function_t          
AQLPriceYieldGenerator::getType() const
{
    return FN_IRYIELDGENERATOR;
}


/*!
    @brief  function of generating yieldcurve
            get market data ,and caclation DF yield by moving function "calcDiscountFactor"
    @param[in] basedate spotdate
	@param[in,out] Reference of AQLMathYieldCurve Object 
    @param[in] att Data of Estimate Procedure (we don't use for this method)

*/
void                
AQLPriceYieldGenerator::calibrateModel( const AQLDate& basedate, 
                                 AQLObject& object, 
                                 const AQLDataProcedure& att ) const
{
	(void)att;
    const AQLMathYieldCurvePro& yg = dynamic_cast<AQLMathYieldCurvePro&>(object);

	const std::map<AQLString, bool>& genCurveGenMap = dynamic_cast<AQLMathYieldCurvePro &>(object).getGCurveGenerateMap();
	const std::map<AQLString, AQLString>& assignedCurveMktMap = dynamic_cast<AQLMathYieldCurvePro &>(object).getAssignedCurveMktMap();

	// check target curve
	bool isAll = true;
	AQLString targetDF = "";
	const AQLDataHolder *dh = &yg.getData(IR_CALIBRATION_DATA_GENTARGETDF, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		targetDF = dynamic_cast<const AQLDataString &>(dh->get()).get();
		isAll = false;
	}
	if (isAll) dynamic_cast<AQLMathYieldCurvePro &>(object).clearGCurveGenerateMap();

	bool isEnabled = false;
	dh = &yg.getData(IR_CALIBRATION_DATA_ENABLECALCULATION, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		isEnabled = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	
	// get DF Curve Name
	AQLString dfCurveName = ITSELF;
	dh = &yg.getData(IR_CALIBRATION_DATA_DFCURVENAME, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		dfCurveName = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}
	// get IsFutureUse
	const bool f_use = dynamic_cast<const AQLDataBool&>(yg.getIsFutureUse()).get();
	// get IsFRAUse
	const bool fra_use = dynamic_cast<const AQLDataBool&>(yg.getIsFRAUse()).get();
    // get Interpolation (make new instance by clone method)
    AQLInterpolationBase* pInter = 
        dynamic_cast<AQLInterpolationBase*>(yg.getInterpolation().getMethod().clone());
	AQLCoreFunctionHolder fh(pInter, true);
	AQLInterpolationBase* pInter_yg = 
        dynamic_cast<AQLInterpolationBase*>(yg.getInterpolation_yg().getMethod().clone());
	AQLCoreFunctionHolder fh_yg(pInter_yg, true);
	AQLInterpolationBase* pInter_fw = NULL;
	AQLCoreFunctionHolder fh_fw;
	if(f_use)
	{
		pInter_fw = dynamic_cast<AQLInterpolationBase*>(yg.getInterpolation_fw().getMethod().clone());
		fh_fw.set(pInter_fw, true);
	}
    // valarray of result
    DoubleArray terms;
	DoubleArray dfs;
	DoubleMatrix fwd_termsmtx;
	DoubleArray fwds;
	unsigned int fwd_samegridstart_pos;

    // Rate Priority
	const AQLStringVector* pRatePriority = 0;
	dh = &yg.getData(PRICING_DATA_RATEPRIORITY);
	if (dh->isDefined() && !dh->isNull())
		pRatePriority = &(dynamic_cast<const AQLDataStrings&>(dh->get()).get());
	
	// Swap Tenor Change
	const bool isSwapTenorAdjust = dynamic_cast<const AQLDataBool&>(yg.getIsSwapTenorAdjust()).get();

	const AQLDataReference& ref = yg.getYieldData();
	AQLObjectHolder objHolder = ref.get();
	//DF2 name
	AQLString df2CurveName;
	dh = &yg.getData(IR_CALIBRATION_DATA_MAINBASISDF, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		df2CurveName = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}
	AQLString targetMarketName = yg.getMarketForCurve(targetDF);
	if ((isAll || targetMarketName == SWAP) && genCurveGenMap.find(STD) == genCurveGenMap.end()
		|| (isEnabled && targetDF == STD))
	{
		if (dfCurveName != ITSELF && genCurveGenMap.find(dfCurveName) == genCurveGenMap.end() && !isEnabled)
		{
			estimateSwapDFCurve(basedate, object, att, dfCurveName);
		}
		// get market data
		const AQLDataMultiReference& mr = yg.getMarketData();
		vector<AQLObject*> data;
		for(unsigned int i = 0; i < mr.getSize(); i++)
		data.push_back(&mr.get(i).get());
		// tenor basis market rates for swap tenor adjust 
		AQLString tenorSwapName;
		AQLString addtionalCalibGrid;
		AQLInterpolationBase* pInter_ts = 0;
		if (isSwapTenorAdjust)
		{
			tenorSwapName = dynamic_cast<const AQLDataString&> (yg.getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
			tenorSwapName.toUpper();
			const AQLDataMultiReference& mr_tenorSwap = 
				dynamic_cast<const AQLDataMultiReference&> (yg.getData(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName, ISNOTNULL).get());
			if (mr_tenorSwap.getSize() == 0)
				throw AQLCoreInvalidData("no data for tenor swap.", __FILE__, __LINE__);
			for(unsigned int i = 0; i < mr_tenorSwap.getSize(); i++)
				data.push_back(&mr_tenorSwap.get(i).get());

			// get addtional calibration tenor basis grid
			// it is necessary to calibrate tenor basis curve to tenor basis spread interpolated in swap tenor adjustment. 
			setAddtionalCalibGridForTenorBasis(mr, mr_tenorSwap, addtionalCalibGrid);
			if(addtionalCalibGrid.size() > 0)
			{
				dynamic_cast<AQLMathYieldCurvePro &>(object).AQLObject::remove(IR_CALIBRATION_DATA_ADDITIONALCALIBGRID + AQLString("_") + tenorSwapName);
				dynamic_cast<AQLMathYieldCurvePro &>(object).AQLObject::add(IR_CALIBRATION_DATA_ADDITIONALCALIBGRID + AQLString("_") + tenorSwapName,
					                                                    new AQLDataStrings()).convertFromString(addtionalCalibGrid);
			}

			// create basis rate interpolation object used in tenor basis adjustment
			dh = &(mr_tenorSwap.get(0).get().getData(CALIBRATION_DATA_INTERPOLATION, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				pInter_ts = dynamic_cast<AQLInterpolationBase *>(dynamic_cast<const AQLPriceDataInterpolation&>(dh->get()).getMethod().clone());
			}
			else
			{
				pInter_ts = dynamic_cast<AQLInterpolationBase *>(pInter_yg->clone());
			}
			if(pInter_ts != 0)
			{
				dynamic_cast<AQLMathYieldCurvePro &>(object).AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONBS + AQLString("_") + tenorSwapName);
				dynamic_cast<AQLMathYieldCurvePro &>(object).AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONBS + AQLString("_") + tenorSwapName,
					                                                    new AQLPriceDataInterpolation(pInter_ts, tenorSwapName));
			}


		}
		// data check
		bool isCheckCurves = false;
		dh = &yg.getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
		if (dh->isDefined() && !dh->isNull()) isCheckCurves = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		bool isCurveAttrExist = yg.checkCurveAttr(SWAP);
		if (!isCheckCurves || !isCurveAttrExist)
		{
			// calc
			double interpolationJoinDateAsDouble = 0.0;
			AQLDate interpolationJoinDate;

			calcDiscountFactor(basedate, data, objHolder, targetMarketName, terms, dfs ,fwd_termsmtx, fwds, 
				interpolationJoinDateAsDouble, interpolationJoinDate, fwd_samegridstart_pos, 
				f_use, fra_use, pInter, pInter_yg, pInter_fw, 
				isSwapTenorAdjust, pInter_ts, pRatePriority, &dfCurveName);
			DoubleArray::const_iterator min_it = min_element(terms.begin(), terms.end());
			if (!terms.empty() && *min_it < 0.0)
			{
				throw AQLCoreInvalidData("terms, term must be positive.", __FILE__, __LINE__);
			}
			
			if (!fwd_termsmtx.empty() && !fwd_termsmtx[0].empty())
			{
				min_it = min_element(fwd_termsmtx[0].begin(), fwd_termsmtx[0].end());
				if (*min_it < 0.0)
				{
					throw AQLCoreInvalidData("fwd_terms, term must be positive.", __FILE__, __LINE__);
				}
			}

			// set result
			objHolder.remove(CALIBRATION_DATA_ASOFDATE);
			objHolder.remove(CALIBRATION_DATA_TERMS);
			objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX);
			objHolder.remove(IR_CALIBRATION_DATA_DFS);
			objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE);
			objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE);

			objHolder.add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(basedate));
			objHolder.add(CALIBRATION_DATA_TERMS, new AQLDataDoubles(terms));
			objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX, new AQLDataDoubleMatrix(fwd_termsmtx));
			objHolder.add(IR_CALIBRATION_DATA_DFS, new AQLDataDoubles(dfs));
			objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE, new AQLDataDouble(interpolationJoinDateAsDouble));
			objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE, new AQLDataDate(interpolationJoinDate));
			dynamic_cast<AQLMathYieldCurvePro &>(object).setdNPVdm(STD);
			setCurveConvention(objHolder, data, STD);
			//dynamic_cast<AQLMathYieldCurvePro &>(object).setGCurveGenerateMap(STD);

			for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
			{
				if (it->second == SWAP)
				{
					const AQLString& curveName = it->first;
					AQLString suffix;
					if (curveName != STD) suffix = "_" + curveName;
					// set result
					objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
					objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + suffix);
					objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
					objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix);
					objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE + suffix);

					objHolder.add(CALIBRATION_DATA_TERMS + suffix, new AQLDataDoubles(terms));
					objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, new AQLDataDoubleMatrix(fwd_termsmtx));
					objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new AQLDataDoubles(dfs));
					objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, new AQLDataDouble(interpolationJoinDateAsDouble));
					objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE + suffix, new AQLDataDate(interpolationJoinDate));

					//dynamic_cast<AQLMathYieldCurvePro &>(object).setGCurveGenerateMap(curveName);
					dynamic_cast<AQLMathYieldCurvePro &>(object).setdNPVdm(curveName);
					setCurveConvention(objHolder, data, curveName);
				}
				else
				{
					const AQLString& curveName = it->first;
					AQLString suffix;
					if (curveName != STD)
					{
						suffix = "_" + curveName;
					}
					objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix);
					objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, new AQLDataDouble(0.0));
				}
			}

		}

		dynamic_cast<AQLMathYieldCurvePro &>(object).setGCurveGenerateMap(STD);
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second == SWAP)
			{
				const AQLString& curveName = it->first;
				AQLString suffix;
				if (curveName != STD) suffix = "_" + curveName;
				dynamic_cast<AQLMathYieldCurvePro &>(object).setGCurveGenerateMap(curveName);
			}
		}
	}

	dh = &yg.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLStringVector &gDFs = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
		for (unsigned int i = 0; i < gDFs.size(); ++i)
		{
			if (!isAll && targetDF != gDFs[i])
			{
				continue;
			}

			if (genCurveGenMap.find(gDFs[i]) != genCurveGenMap.end())
			{
				continue;
			}
			AQLString suffix_mkt = "";
			if (gDFs[i] != STD)
			{
				suffix_mkt = AQLString("_") + yg.getMarketForCurve(gDFs[i]);
			}
			else
				continue;
			// get market data
			const AQLDataMultiReference& mr_ = dynamic_cast<const AQLDataMultiReference&>
												(yg.getData(CALIBRATION_DATA_MARKETDATA + suffix_mkt, ISNOTNULL).get());
			vector<AQLObject*> data_;
			for(unsigned int j = 0; j < mr_.getSize(); j++)
			{
				data_.push_back(&mr_.get(j).get());
			}
			// get IsFutureUse
			bool f_use_ = f_use; 
			dh = &yg.getData(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_mkt, NOCHECK);
			if (dh->isDefined() && !dh->isNull() && gDFs[i] != STD)
			{
				f_use_ = dynamic_cast<const AQLDataBool&>(dh->get());
			}
			// get IsFRAUse
			bool fra_use_ = fra_use; 
			dh = &yg.getData(IR_CALIBRATION_DATA_ISFRAUSE + suffix_mkt, NOCHECK);
			if (dh->isDefined() && !dh->isNull() && gDFs[i] != STD)
			{
				fra_use_ = dynamic_cast<const AQLDataBool&>(dh->get());
			}
			// get isSwapTenorAdjust
			bool isSwapTenorAdjust_ = false;
			dh = &yg.getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + suffix_mkt, NOCHECK);
			if (dh->isDefined() && !dh->isNull() && gDFs[i] != STD)
			{
				isSwapTenorAdjust_ = dynamic_cast<const AQLDataBool&>(dh->get());
			}
			AQLString tenorSwapName_;
			AQLString addtionalCalibGrid_;
			AQLInterpolationBase* pInter_ts_ = 0;
			if (isSwapTenorAdjust_)
			{
				tenorSwapName_ = dynamic_cast<const AQLDataString&> (yg.getData(IR_CALIBRATION_DATA_TENORSWAPNAME + suffix_mkt, ISNOTNULL).get());
				tenorSwapName_.toUpper();
				const AQLDataMultiReference& mr_tenorSwap = 
					dynamic_cast<const AQLDataMultiReference&> (yg.getData(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName_, ISNOTNULL).get());
				for(unsigned int i = 0; i < mr_tenorSwap.getSize(); i++)
					data_.push_back(&mr_tenorSwap.get(i).get());

				// get addtional calibration tenor basis grid
				// it is necessary to calibrate tenor basis curve to tenor basis spread interpolated in swap tenor adjustment. 
				setAddtionalCalibGridForTenorBasis(mr_, mr_tenorSwap, addtionalCalibGrid_);
				if(addtionalCalibGrid_.size() > 0)
				{
					dynamic_cast<AQLMathYieldCurvePro &>(object).AQLObject::remove(IR_CALIBRATION_DATA_ADDITIONALCALIBGRID + AQLString("_") + tenorSwapName_);
					dynamic_cast<AQLMathYieldCurvePro &>(object).AQLObject::add(IR_CALIBRATION_DATA_ADDITIONALCALIBGRID + AQLString("_") + tenorSwapName_,
					                                                        new AQLDataStrings()).convertFromString(addtionalCalibGrid_);
				}

				// create basis rate interpolation object used in tenor basis adjustment
				dh = &(mr_tenorSwap.get(0).get().getData(CALIBRATION_DATA_INTERPOLATION, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					pInter_ts_ = dynamic_cast<AQLInterpolationBase *>(dynamic_cast<const AQLPriceDataInterpolation&>(dh->get()).getMethod().clone());
				}
				else
				{
					pInter_ts_ = dynamic_cast<AQLInterpolationBase *>(pInter_yg->clone());
				}
				if(pInter_ts_ != 0)
				{
					dynamic_cast<AQLMathYieldCurvePro &>(object).AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONBS + AQLString("_") + tenorSwapName_);
					dynamic_cast<AQLMathYieldCurvePro &>(object).AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONBS + AQLString("_") + tenorSwapName_,
					                                                        new AQLPriceDataInterpolation(pInter_ts_, tenorSwapName_));
				}

			}
			// libor-ois basis swap market data for ois curve calibration
			dh = &yg.getData(IR_CALIBRATION_DATA_LOBASISNAME + suffix_mkt, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				AQLString loBasisName = dynamic_cast<const AQLDataString& >(yg.getData(AQLString(IR_CALIBRATION_DATA_LOBASISNAME) + suffix_mkt, ISNOTNULL).get()).get();
				const AQLDataMultiReference& mr_loBasisSwap = dynamic_cast<const AQLDataMultiReference&> (yg.getData(CALIBRATION_DATA_MARKETDATA + AQLString("_") + loBasisName, ISNOTNULL).get());
				for(unsigned int i = 0; i < mr_loBasisSwap.getSize(); i++)
				{
					data_.push_back(&mr_loBasisSwap.get(i).get());
				}
			}

			// get Interpolation
			AQLInterpolationBase *pInter_ = pInter;
			dh = &yg.getData(CALIBRATION_DATA_INTERPOLATION + suffix_mkt, NOCHECK);
			AQLCoreFunctionHolder fh_;
			if (dh->isDefined() && !dh->isNull() && gDFs[i] != STD)
			{
				pInter_ = dynamic_cast<AQLInterpolationBase*>(dynamic_cast<const AQLPriceDataInterpolation &>(dh->get()).getMethod().clone());
				fh_.set(pInter_, true);
			}

			AQLInterpolationBase *pInter_yg_ = pInter_yg;
			dh = &yg.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_mkt, NOCHECK);
			AQLCoreFunctionHolder fh_yg_;
			if (dh->isDefined() && !dh->isNull() && gDFs[i] != STD)
			{
				pInter_yg_ = dynamic_cast<AQLInterpolationBase*>(dynamic_cast<const AQLPriceDataInterpolation &>(dh->get()).getMethod().clone());
				fh_yg_.set(pInter_yg_, true);
			}

			AQLInterpolationBase *pInter_fw_ = pInter_fw;
			dh = &yg.getData(IR_CALIBRATION_DATA_INTERPOLATIONFW + suffix_mkt, NOCHECK);
			AQLCoreFunctionHolder fh_fw_;
			if (dh->isDefined() && !dh->isNull() && gDFs[i] != STD)
			{
				pInter_fw_ = dynamic_cast<AQLInterpolationBase*>(dynamic_cast<const AQLPriceDataInterpolation &>(dh->get()).getMethod().clone());
				fh_fw_.set(pInter_fw_, true);
			}
			// valarray of result
			DoubleArray terms_;
			DoubleArray dfs_;
			DoubleMatrix fwd_termsmtx_;
			DoubleArray fwds_;
			unsigned int fwd_samegridstart_pos_;
			// Rate Priority
			//const AQLStringVector* pRatePriority_ = nullptr;
			const AQLStringVector* pRatePriority_ = pRatePriority;
			const AQLDataHolder* dh = &yg.getData(PRICING_DATA_RATEPRIORITY + suffix_mkt, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				pRatePriority_ = &(dynamic_cast<const AQLDataStrings&>(dh->get()).get());
			}
			dfCurveName = ITSELF;
			dh = &yg.getData(IR_CALIBRATION_DATA_DFCURVENAME + suffix_mkt, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				dfCurveName = dynamic_cast<const AQLDataString &>(dh->get()).get();
			}
			
			if (dfCurveName != ITSELF && genCurveGenMap.find(dfCurveName) == genCurveGenMap.end() && !isEnabled)
			{
				estimateSwapDFCurve(basedate, object, att, dfCurveName);
			}
			// data check
			bool isCheckCurves = false;
			dh = &yg.getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
			if (dh->isDefined() && !dh->isNull()) isCheckCurves = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			bool isCurveAttrExist = yg.checkCurveAttr(yg.getMarketForCurve(gDFs[i]));
			if (!isCheckCurves || !isCurveAttrExist)
			{
				// calc
				double interpolationJoinDateAsDouble = 0.0;
				AQLDate interpolationJoinDate;

				calcDiscountFactor(basedate, data_, objHolder, targetMarketName, terms_, dfs_, fwd_termsmtx_, fwds_, 
					interpolationJoinDateAsDouble, interpolationJoinDate, fwd_samegridstart_pos_, 
					f_use_, fra_use_, pInter_, pInter_yg_, pInter_fw_, 
					isSwapTenorAdjust_, pInter_ts_, pRatePriority_, &dfCurveName);
				DoubleArray::const_iterator min_it = min_element(terms_.begin(), terms_.end());
				if (!terms_.empty() && *min_it < 0.0)
				{
					AQLString msg = gDFs[i] + " terms, term must be positive.";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				if (!fwd_termsmtx_.empty() && !fwd_termsmtx_[0].empty())
				{
					min_it = min_element(fwd_termsmtx_[0].begin(), fwd_termsmtx_[0].end());
					if (*min_it < 0.0)
					{
						AQLString msg = gDFs[i] + " fwd_terms, term must be positive.";
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
				}

				for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
				{
					if (it->second == yg.getMarketForCurve(gDFs[i]))
					{
						const AQLString& curveName = it->first;
						AQLString suffix_curve = "";
						if (curveName != STD) suffix_curve = "_" + curveName;
						// set result
						objHolder.remove(CALIBRATION_DATA_TERMS + suffix_curve);
						objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + suffix_curve);
						objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix_curve);
						objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix_curve);
						objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE + suffix_curve);

						objHolder.add(CALIBRATION_DATA_TERMS + suffix_curve, new AQLDataDoubles(terms_));
						objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX + suffix_curve, new AQLDataDoubleMatrix(fwd_termsmtx_));
						objHolder.add(IR_CALIBRATION_DATA_DFS + suffix_curve, new AQLDataDoubles(dfs_));
						objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix_curve, new AQLDataDouble(interpolationJoinDateAsDouble));
						objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE + suffix_curve, new AQLDataDate(interpolationJoinDate));

						//dynamic_cast<AQLMathYieldCurvePro &>(object).setGCurveGenerateMap(curveName);
						dynamic_cast<AQLMathYieldCurvePro &>(object).setdNPVdm(curveName);
						setCurveConvention(objHolder, data_, curveName);
					}
					else
					{
						const AQLString& curveName = it->first;
						AQLString suffix_curve = "";
						if (curveName != STD) suffix_curve = "_" + curveName;
						objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix_curve);
						objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix_curve, new AQLDataDouble(0.0));
					}
				}
			}

			for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
			{
				if (it->second == yg.getMarketForCurve(gDFs[i]))
				{
					const AQLString& curveName = it->first;
					AQLString suffix_curve = "";
					if (curveName != STD)
					{
						suffix_curve = "_" + curveName;
					}
					dynamic_cast<AQLMathYieldCurvePro &>(object).setGCurveGenerateMap(curveName);
				}
			}
		}
	}

	// set DF2
	if ((isAll || targetDF == df2CurveName) && genCurveGenMap.find(STD) != genCurveGenMap.end() 
		&& genCurveGenMap.find(df2CurveName) != genCurveGenMap.end())
	{
		AQLString suffix = "";
		if (df2CurveName != STD)
		{
			suffix = "_" + df2CurveName;
		}
		DoubleArray dfs_mod;
		const DoubleArray &terms = dynamic_cast<const AQLDataDoubles &>(objHolder.getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();
		DoubleArray terms_mod = dynamic_cast<const AQLDataDoubles &>(objHolder.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get()).get();;
		DoubleArray dfs_mod_ = dynamic_cast<const AQLDataDoubles &>(objHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get()).get();
		// adjust terms
		dh = &objHolder.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			pInter->setJoinDateAsDouble(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
		}
		pInter->set(terms_mod, dfs_mod_);
		dfs_mod.resize(terms.size());
		for (unsigned int i = 0; i < terms.size(); ++i)
		{
			dfs_mod[i] = pInter->value(terms[i]);
		}
		// set DF2, grid is same with attr CALIBRATION_DATA_TERMS
		objHolder.remove(IR_CALIBRATION_DATA_DFS2);
		objHolder.add(IR_CALIBRATION_DATA_DFS2, new AQLDataDoubles(dfs_mod));
	}
}

/*
    @brief function to calclation DiscountFactor from market data
    
    @param[in] basedate		basedate
	@param[in] data			input market data			   
	@param[in] objHolder			input object data(get DF from this object)
    @param[out]terms		output terms(this include initial term and market term)
    @param[out]dfs			output DiscountFactors at terms
    @param[out]fwd_termsmtx	output forward rates terms
    @param[out]fwds			output forward rates at terms
	@param[in] is_f_use		use future or not
	@param[in] is_fra_use	use fra or not
	@param[in] pInter		InterPolation for df
    @param[in] pInter_yg	InterPolation for Swap Rate
	@param[in] pInter_fw	InterPolation for future
	@param[in] isSwapTenorAdjust swap tenor adjust or not
	@param[in] pRatePriority RatePriority
	@param[in] pDFCurveName DF curve name
*/
void 
AQLPriceYieldGenerator::calcDiscountFactor(const AQLDate& basedate,
										std::vector<AQLObject*>& data,
										const AQLObjectHolder& objHolder,
										const AQLString& targetMarketName,
                                        DoubleArray& terms,
										DoubleArray& dfs,
										DoubleMatrix& fwd_termsmtx,
										DoubleArray& fwds,
										double& interpolationJoinDateAsDouble,
										AQLDate& interpolationJoinDate,
										unsigned int& fwd_samegridstart_pos,
										const bool is_f_use,
										const bool is_fra_use,
                                        AQLInterpolationBase* pInter,
										AQLInterpolationBase* pInter_yg,
										AQLInterpolationBase* pInter_fw,
										const bool isSwapTenorAdjust,
										AQLInterpolationBase* pInter_ts,
										const AQLStringVector* pRatePriority,
										const AQLString* pDFCurveName) 
{
    //////////////////////////////////////
	//Classify data object by data type //
	//////////////////////////////////////
	vector<AQLObject*> data_libor, data_swap, data_future, data_ois_swaps, data_fra6m, data_fra3m, data_tenorswap;
	map<pair<AQLDate, AQLDate>, const AQLObject*> data_moneymarket;//map key is (startdate,enddate) 
	unsigned int size_data = data.size();
	AQLString datatype_str;
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	bool isO_N = false, isT_N = false;
	fwd_termsmtx.clear();
	fwd_termsmtx.resize(2);
	fwds.clear();
	for(unsigned i = 0; i < size_data; i++)
	{
		// check use grid
		const AQLDataHolder *dh = &data[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;

		datatype_str = dynamic_cast<const AQLDataString&> ((data[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		datatype_str.toUpper();
		
		if (datatype_str == ZERO) data_libor.push_back(data[i]);//libor case
		else if (datatype_str == PAR) data_swap.push_back(data[i]);//swap case
		else if (datatype_str == FUTURE) data_future.push_back(data[i]);//future case
		else if (datatype_str == BOJ) data_ois_swaps.push_back(data[i]);//mpc case = monetary policy committee swaps
		else if (datatype_str == FEDFUNDRATE) 
		{
			const AQLDate& sdate = dynamic_cast<const AQLDataDate&> ((data[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
			//for fedfund first grid
			data_ois_swaps.push_back(data[i]);//mpc case = monetary policy committee swaps
		}
		else if (datatype_str == FRA6M) data_fra6m.push_back(data[i]);//6mfra case
		else if (datatype_str == FRA3M) data_fra3m.push_back(data[i]);//3mfra case
		else if (datatype_str == BASIS) data_tenorswap.push_back(data[i]);//tenor swap
		else //money market case
		{
			const AQLPriceDataCalendar& calendar  = dynamic_cast<const AQLPriceDataCalendar&> ((data[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const AQLPriceDataSlidingRule& slidingRule = dynamic_cast<const AQLPriceDataSlidingRule&> ((data[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			if (datatype_str == O_N)
			{
				AQLDate end = basedate;
				end.addDays(1);
				end = slidingRule.getDate(end, calendar);
				data_moneymarket[pair<AQLDate, AQLDate>(basedate, end)] = data[i];
				isO_N = true;
				DateVector depocalcdates(2,basedate);
				depocalcdates[1] = end;
				if(!data[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
				{
					data[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(depocalcdates));
				}
				else
				{
					dynamic_cast<AQLDataDates&>(data[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(depocalcdates);
				}
			}
			else if (datatype_str == T_N)
			{

				AQLDate start = basedate;
				start.addDays(1);
				start = slidingRule.getDate(start, calendar);
				AQLDate end = start;
				end.addDays(1);
				end = slidingRule.getDate(end, calendar);
				data_moneymarket[pair<AQLDate, AQLDate>(start, end)] = data[i];
				isT_N = true;
				DateVector depocalcdates(2,start);
				depocalcdates[1] = end;
				if(!data[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
				{
					data[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(depocalcdates));
				}
				else
				{
					dynamic_cast<AQLDataDates&>(data[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(depocalcdates);
				}
			}
			else
			{
				AQLDate start, end;
				if (datatype_str == "0D_1D") isO_N = true;
				else if (datatype_str == "1D_1D") isT_N = true;
				getDate(basedate, datatype_str, calendar, slidingRule, start, end);
				data_moneymarket[pair<AQLDate, AQLDate>(start, end)] = data[i];
			}
		}
	}


	// ascending sort libor , swap and future data entities  
	sort(data_libor.begin(), data_libor.end(), Comp_term());
	sort(data_swap.begin(), data_swap.end(), Comp_term());
	sort(data_ois_swaps.begin(), data_ois_swaps.end(), Comp_term());
	sort(data_tenorswap.begin(), data_tenorswap.end(), Comp_term());
	if (is_fra_use) 
	{
		sort(data_fra3m.begin(), data_fra3m.end(), Comp_term());
		sort(data_fra6m.begin(), data_fra6m.end(), Comp_term());
	}
	if (is_f_use) sort(data_future.begin(), data_future.end(), Comp_term());

	unsigned int size_libor_fixings = data_libor.size();//libor size
	unsigned int size_swaps = data_swap.size();//swap size
	unsigned int size_futures = data_future.size();//future size
	unsigned int size_ois_swaps = data_ois_swaps.size(); //ois swaps size, constains ois swaps and ois monetary policy committee swaps
	unsigned int size_3mfra = data_fra3m.size();//3mfra size
	unsigned int size_6mfra = data_fra6m.size();//3mfra size

	//
	// Calculate the First Swap Date
	// ------------------------------------------------------------
	//
	const AQLDataHolder* dh = 0;

	AQLDate firstSwapDate;
	if (is_fra_use || is_f_use)
	{
		if (size_swaps == 0)
		{
			throw AQLCoreInvalidData("We need swap market when we use fra market!", __FILE__, __LINE__);
		}
		const AQLDate& spotdate = dynamic_cast<const AQLDataDate&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		const AQLString& term_str = dynamic_cast<const AQLDataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		const AQLString& freq = dynamic_cast<const AQLDataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		const AQLPriceDataCalendar& calendar = dynamic_cast<const AQLPriceDataCalendar&> ((data_swap[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		const AQLPriceDataSlidingRule& slidingRule = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		bool eom = false;
		const AQLDataHolder *dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}
		// set roll convention
		AQLString roll_conv("");
		if (freq == LUNAR)
		{
			roll_conv = ROLLCONV_LUNAR;
		}
		else if (eom) roll_conv = ROLLCONV_EOM;
		else roll_conv = ROLLCONV_NORMAL;
		firstSwapDate = AQLDateCalculations::getDate(spotdate, term_str, slidingRule, &calendar, true, &roll_conv);
	}
	// ------------------------------------------------------------


	//
	// *** OIS Swap Calibration ***
	// ===============================================================================================================

	if (size_ois_swaps || !size_libor_fixings)
	{
		return calcOISDiscountFactor(basedate,data,objHolder,terms,dfs,is_f_use,pInter,pInter_yg,pInter_fw,pDFCurveName);
	}

	// ===============================================================================================================


	if (!isO_N || !isT_N)
	{
		throw AQLCoreInvalidData("Both O_N and T_N are needed", __FILE__, __LINE__);
	}
	
	if (size_libor_fixings == 0 || size_swaps == 0)
	{
		throw AQLCoreInvalidData("Both Libor and Swap size must be more than one", __FILE__, __LINE__);
	}
	if (is_fra_use && is_f_use) 
	{
		throw AQLCoreInvalidData("We can not use fra and future at a same time!", __FILE__, __LINE__);
	}

	//////////////////////
	//Spot date of Libor//
	//////////////////////
	AQLDate spotdate_l;
	for (unsigned int i = 0; i < size_libor_fixings; i++)
	{
		const AQLDate& spotdate = dynamic_cast<const AQLDataDate&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		if (i == 0) spotdate_l = spotdate;
		else if (spotdate_l != spotdate)
		{
			AQLString msg = "Same rate type must have same spotdate";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
		}
	}
	const AQLPriceDataDayCount &dc_l = dynamic_cast<const AQLPriceDataDayCount&> ((data_libor[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	///////////////////////////////////////////
	//Spot date, Calendar, Convention of Swap//
	///////////////////////////////////////////
	AQLDate spotdate_s;
	vector<const AQLPriceDataCalendar *> swap_calendar(size_swaps, 0);
	vector<const AQLPriceDataSlidingRule *> swap_slidingRule(size_swaps, 0);
	vector<const AQLPriceDataDayCount *> swap_daycount(size_swaps, 0);
	vector<const AQLPriceDataDayCount *> swap_floatLeg_daycount(size_swaps, 0);
	AQLStringVector swap_frequency(size_swaps, "");
	AQLStringVector swap_floatLeg_frequency(size_swaps, "");
	AQLStringVector swap_compounding_frequency(size_swaps, "");
	vector<int> swap_compoundingTimes(size_swaps);
	AQLString baseFreq = "";
	bool isAllSame_s = true;
	vector<bool> swap_useEOMRollConvention(size_swaps, false);
	AQLStringVector swap_rollConvention(size_swaps, "");

	for (unsigned int i = 0; i < size_swaps; i++)
	{
		const AQLDate& spotdate = dynamic_cast<const AQLDataDate&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		swap_frequency[i] = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		swap_frequency[i].toUpper();
		if (swap_frequency[i] == SIMPLE)
		{
			//error
			throw AQLCoreInvalidData("Simple frequency is not supported in IRS Market", __FILE__, __LINE__);
		}
		swap_calendar[i] =  &dynamic_cast<const AQLPriceDataCalendar&> ((data_swap[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		swap_slidingRule[i]  = &dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		swap_daycount[i] = &dynamic_cast<const AQLPriceDataDayCount&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		const AQLDataHolder *dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			swap_floatLeg_daycount[i] = &dynamic_cast<const AQLPriceDataDayCount&> (dh->get());
		}
		dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			swap_floatLeg_frequency[i] = dynamic_cast<const AQLDataString&> (dh->get());
		}
	
		dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_COMPOUND, NOCHECK); 
		if (dh->isDefined() && !dh->isNull()) 
		{ 
			swap_compounding_frequency[i] = dynamic_cast<const AQLDataString&> (dh->get());
		}
		else
		{
			swap_compounding_frequency[i] = swap_floatLeg_frequency[i];
		}
		
		swap_compoundingTimes[i] = AQLDateCalculations::calcCompoundingTimes(swap_floatLeg_frequency[i], swap_compounding_frequency[i]);

		if (i == 0)
		{
			spotdate_s = spotdate;
		}
		else if (!(*swap_calendar[i] == *swap_calendar[i - 1]) || !(*swap_slidingRule[i] == *swap_slidingRule[i - 1]) || !(swap_frequency[i] == swap_frequency[i - 1]) || !(*swap_daycount[i] == *swap_daycount[i - 1]))
		{
			isAllSame_s = false;
		}
		else if (spotdate_s != spotdate)
		{
			AQLString msg = "Each swap must be same spotdate";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
		}

		if (i == 0)
		{	
			const AQLDataHolder *dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				baseFreq = dynamic_cast<const AQLDataString&> (dh->get());
			}
		}

		dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			swap_useEOMRollConvention[i] = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		// set roll convention
		if (swap_frequency[i] == LUNAR) swap_rollConvention[i] = ROLLCONV_LUNAR;
		else if (swap_useEOMRollConvention[i]) swap_rollConvention[i] = ROLLCONV_EOM;
		else swap_rollConvention[i] = ROLLCONV_NORMAL;
	}
	// check Swap Market and assign corresponding fra market information
	AQLString refRateTerm;
	vector<AQLObject*> data_fra;
	if (baseFreq == SEMI_ANNUAL)
	{
		refRateTerm = TERM_6M;
		data_fra = data_fra6m;
	}
	else if (baseFreq == QUARTERLY)
	{
		refRateTerm = TERM_3M;
		data_fra = data_fra3m;
	}
	else if (baseFreq == LUNAR)
	{
		refRateTerm = TERM_1M;
	}
	else
	{
		throw AQLCoreInvalidData("We support only 6M or 3M in swap floating leg!", __FILE__, __LINE__);
	}
	unsigned int size_fras = data_fra.size();

	// get fwd swap
	bool is_fwdswap = false;
	dh = &(data_swap[0]->getData(PRICING_DATA_ISFWDSWAP, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		is_fwdswap = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// Locate previous solution and gradient, if available
	AQLString targetSuffix = "";
	if (targetMarketName != SWAP)
	{
		targetSuffix = AQLString("_") + targetMarketName;
	}

	//
	// Set the Join Date for the Linear-Spline Interpolation Method
	// ------------------------------------------------------------
	//
	bool pInter_IsHybrid = false;
	bool pInter_yg_IsHybrid = false;
	bool pInter_fw_IsHybrid = false;
	if (pInter != NULL)
	{
		pInter_IsHybrid = (pInter->isHybrid()) ? true : false;
	}
	if (pInter_yg != NULL)
	{
		pInter_yg_IsHybrid = (pInter_yg->isHybrid()) ? true : false;
	}
	if (pInter_fw != NULL)
	{
		pInter_fw_IsHybrid = (pInter_fw->isHybrid()) ? true : false;
	}

	// Set Output Variable: linear-spline interpolation join date
	interpolationJoinDateAsDouble = 0.0;
	bool is_joinDateGiven = false;
	bool iscalculateJoinDate = true;

	if (pInter_IsHybrid || pInter_yg_IsHybrid || pInter_fw_IsHybrid)
	{
		// If a join date is given by user as an input, we use this join date and don't calculate join date;
		// If no user-given join date exists, we will decide whether we always re-calculate the dynamic
		// join date, or only calculate it once and re-use thie join date in subsequent calibrations of the same curve.

		// Calculate the initial linear spline join date
		const AQLDataHolder* handle = &(objHolder.getData(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE, NOCHECK));
		if (handle->isDefined() && !handle->isNull())
		{
			// Use a linear spline join date supplied by user
			is_joinDateGiven = true;
			AQLDate joinDate = dynamic_cast< const AQLDataDate& >(handle->get()).get();
			interpolationJoinDate = joinDate;
			interpolationJoinDateAsDouble = dc_act365.getTerm(spotdate_s, joinDate);
		}
		else
		{
			// Always recalculate the dynamic join date?
			bool isalwaysRecalcJoinDate = false;
			const AQLDataHolder* handle = &(objHolder.getData(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE, NOCHECK));
			if (handle->isDefined() && !handle->isNull())
			{
				isalwaysRecalcJoinDate = dynamic_cast< const AQLDataBool& >(handle->get()).get();
			}

			if (!isalwaysRecalcJoinDate)
			{
				iscalculateJoinDate = false;
				handle = &(objHolder.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE, NOCHECK));
				if (handle->isDefined() && !handle->isNull())
				{
					double existingJoinDateAsDouble = dynamic_cast< const AQLDataDouble& >(handle->get()).get();
					interpolationJoinDateAsDouble = existingJoinDateAsDouble;

					handle = &(objHolder.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE + targetSuffix, NOCHECK));
					if (handle->isDefined() && !handle->isNull())
					{
						interpolationJoinDate = dynamic_cast<const AQLDataDate&>(handle->get()).get();
					}
				}
			}

			// AlgoQuantLib calculates linear spline join date when:
			// 1. User specifies to ALWAYS recalculate join date; or
			// 2. User specifies not to always recalculate join date, but no pre-calculated join date currently exists;

			if (iscalculateJoinDate)
			{
				//throw AQLCoreInvalidData("Autometed calculation of linear spline joint date is not supported vanilla add-in at the moment.", __FILE__, __LINE__);
				double joinDateAsDouble = 0.0;
				if (is_fra_use && size_fras > 0)
				{
					const AQLDate spotDate = dynamic_cast< const AQLDataDate& >((data_fra[size_fras - 1]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get()).get();
					const AQLPriceDataDayCount& fraDayCount = dynamic_cast< const AQLPriceDataDayCount& >((data_fra[size_fras - 1]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());

					const AQLDataHolder *dh = &data_swap[size_swaps - 1]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK);
					AQLString baseFreq;
					if (dh->isDefined() && !dh->isNull())
					{
						baseFreq = dynamic_cast< const AQLDataString& >(dh->get());
					}
					else
					{
						throw AQLCoreInvalidData("#Error: Linear-Spline Interpolation Error. Unable to calculate the FRA end date needed for the Linear-Spline join date", __FILE__, __LINE__);
					}

					AQLString refRateTerm;
					if (baseFreq == SEMI_ANNUAL)
					{
						refRateTerm = TERM_6M;
					}
					else if (baseFreq == QUARTERLY)
					{
						refRateTerm = TERM_3M;
					}
					else
					{
						throw AQLCoreInvalidData("#Error: STD Curve only supports swaps with 3M or 6M floating leg frequency", __FILE__, __LINE__);
					}

					AQLDate lastFraStartDate, lastFraEndDate;
					calculateFraDates(lastFraStartDate, lastFraEndDate, spotDate, data_fra[size_fras - 1], refRateTerm, baseFreq, fraDayCount);

					// Do not allow join date to be beyond the first swap maturity date
					if (lastFraEndDate >= firstSwapDate)
					{
						lastFraEndDate = firstSwapDate;
					}

					interpolationJoinDate = lastFraEndDate;
					joinDateAsDouble = dc_act365.getTerm(spotdate_s, lastFraEndDate);
				}
				else if (is_f_use && size_futures > 0)
				{
					interpolationJoinDate = determineLinearSplineInterpolationJoinDate(data_future[size_futures - 1], data_swap[0], spotdate_s, is_fwdswap);
					joinDateAsDouble = dc_act365.getTerm(spotdate_s, interpolationJoinDate);
				}

				interpolationJoinDateAsDouble = joinDateAsDouble;
			}
		}

		// Set the Linear-Spline Interpolation Join Date for the Disc Factor Interpolation Table
		if (pInter_IsHybrid)
		{
			pInter->setJoinDateAsDouble(interpolationJoinDateAsDouble);
		}

		// Set the Linear-Spline Interpolation Join Date for the Swap Rate Interpolation Table
		if (pInter_yg_IsHybrid)
		{
			pInter_yg->setJoinDateAsDouble(interpolationJoinDateAsDouble);
		}

		// Set the Linear-Spline Interpolation Join Date for the Forward Interpolation Table
		if (pInter_fw_IsHybrid)
		{
			pInter_fw->setJoinDateAsDouble(interpolationJoinDateAsDouble);
		}
	}
	// ------------------------------------------------------------

	/////////////////
	//Rate Priority//
	/////////////////
	bool isFuturePriority = false;// future priority than libor
	bool isSwapPriority = false; //swap priority than other rates
	if (pRatePriority != 0)
	{
			
		if (is_f_use && size_futures != 0)
		{
			if (pRatePriority->size() != 3)
			{
                //error
				AQLString msg = "Rate priority setting is something wrong";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else 
			{
           		// Added to prevent Excel crashing due to access violation / non-bounds check on vector (*pRatePriority)[1]
            	if (pRatePriority->size() < 2)
            	{
					AQLString msg = "Rate priority parameter error. This parameter must be a list of colon separated parameters of size 2.";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
            	}

				AQLString str = (*pRatePriority)[1];
				str.toUpper();
				if (str == SWAP)
				{
	                //error
					AQLString msg = "Rate priority setting error! Swap must not be between other rates";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);					
				}
			}
		}

		AQLString str = (*pRatePriority)[0];
		str.toUpper();
		if (str == SWAP)
		{
            // Added to prevent Excel crashing due to access violation / non-bounds check on vector (*pRatePriority)[1]
            if (pRatePriority->size() < 2)
            {
				AQLString msg = "Rate priority parameter error. This parameter must be a list of colon separated parameters of size 2.";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
            }

			isSwapPriority = true;
			str = (*pRatePriority)[1];
			str.toUpper();
			if (is_f_use && size_futures != 0 && str == FUTURE)
				isFuturePriority = true;
		}
		else if (is_f_use && size_futures != 0 && str == FUTURE)
			isFuturePriority = true;
	}

	/////////////////////////
	//calculate df by libor//
	/////////////////////////
	DoubleMatrix df_1(2), df_2(2);
	df_1[0].push_back(0.0);//first term=0.0;
	df_1[1].push_back(1.0);//first df = 1.0;
	df_2[0].push_back(0.0);//first term=0.0;
	df_2[1].push_back(1.0);//first df = 1.0;

	DateVector df_2_date;
	df_2_date.push_back(spotdate_s);
	AQLString freq;
	double df_adj_spots_spotl = 1.0;//df from spotdate_s to spotdate_l or from spotdate_l to spotdate_s 

	map<pair<AQLDate, AQLDate>, const AQLObject*>::const_iterator it_ = data_moneymarket.begin();
	const double rate_on = dynamic_cast<const AQLDataDouble&> ((it_->second->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
	const AQLPriceDataDayCount& dc_on  = dynamic_cast<const AQLPriceDataDayCount&> ((it_->second->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	RateConvention rc_on = AQLMathYieldCurve::setRC(SIMPLE);
	AQLPriceDataConvention conv_on(dc_on.getDayCount(), rc_on);

	bool is_on_adj = false;
	dh = &(data_libor[0]->getData(IR_CALIBRATION_DATA_ISONFORSPOTADJUST, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		is_on_adj = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	double fwd_rate_l = 0.0;
	double fwd_term_l = 0.0;
	bool is_fwd_l = false;
	bool isOldMethod = false;
	if (is_f_use && size_futures != 0)
	{
		const AQLDataHolder *dh = &(data_future[0]->getData(PRICING_DATA_FUTUREOLDMETHOD, NOCHECK));
		if(dh->isDefined() && !dh->isNull()) isOldMethod = dynamic_cast<const AQLDataBool&> (dh->get());
	}

    AQLDate lastPushedDate;
	for (unsigned int i = 0; i < size_libor_fixings; i++)
	{
		freq = dynamic_cast<const AQLDataString&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		freq.toUpper();
		const AQLString& term_str = dynamic_cast<const AQLDataString&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		if ((is_fra_use || (is_f_use && !isOldMethod)) && term_str != refRateTerm) continue;		//if (is_fra_use && term_str != refRateTerm) continue;
		double rate = dynamic_cast<const AQLDataDouble&> ((data_libor[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		const AQLPriceDataCalendar& calendar  = dynamic_cast<const AQLPriceDataCalendar&> ((data_libor[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		const AQLPriceDataSlidingRule& slidingRule  = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_libor[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		const AQLPriceDataDayCount& dayCount = dynamic_cast<const AQLPriceDataDayCount&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		bool eom = false;
		dh = &(data_libor[i]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}
		// set roll convention
		AQLString roll_conv("");
		if (freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
		else if (eom) roll_conv = ROLLCONV_EOM;
		else roll_conv = ROLLCONV_NORMAL;
		AQLDate date = AQLDateCalculations::getDate(spotdate_l, term_str, slidingRule, &calendar, true, &roll_conv);
		
		RateConvention rc = AQLMathYieldCurve::setRC(freq);
		AQLPriceDataConvention conv(dayCount.getDayCount(), rc);
		double df = conv.getDF(rate, spotdate_l, date);
		double term = dc_act365.getTerm(spotdate_l, date);
		
		// When date <= spotdate_s, do not edit df_1 and df_2_date.
		if(date > spotdate_s)
		{
			if(i == 0 || date != lastPushedDate)
			{
				df_2_date.push_back(date);
				df_1[0].push_back(term);
				df_1[1].push_back(df);
			}
			else
			{
				df_1[1].back() = df;
			}
		}
        lastPushedDate = date;
		if (!is_fwd_l && term_str == refRateTerm)
		{
			fwd_rate_l = rate;
			fwd_term_l = term;
			is_fwd_l = true;
		}
		if (is_fra_use || i == 0)
		{
			if (spotdate_l > spotdate_s)
			{
				if (is_on_adj)
				{
					df_adj_spots_spotl = conv_on.getDF(rate_on, spotdate_s, spotdate_l);
				}
				else
				{
					df_adj_spots_spotl = conv.getDF(rate, spotdate_s, spotdate_l);
				}
			}
			else if (spotdate_l < spotdate_s)
			{
				if (is_on_adj)
				{
					df_adj_spots_spotl = conv_on.getDF(rate_on, spotdate_l, spotdate_s);
				}
				else
				{
					df_adj_spots_spotl = conv.getDF(rate, spotdate_l, spotdate_s);
				}
			}
		}
		DateVector liborcalcdates(2,spotdate_l);
		liborcalcdates[1] = date;
		if(!data_libor[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
		{
			data_libor[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(liborcalcdates));
		}
		else
		{
			dynamic_cast<AQLDataDates&>(data_libor[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(liborcalcdates);
		}
	}

	// size check 
	if (is_fra_use && df_1[0].size() <= 1) throw AQLCoreInvalidData("No 3M Libor or No 6M Libor!", __FILE__, __LINE__);

	double term_offset = 0.0;
	if (spotdate_l < spotdate_s)
	{
		double term = dc_act365.getTerm(spotdate_l, spotdate_s);
		for (unsigned int i = 0; i < df_1[0].size()-1; i++)
		{
			if (df_1[0][i + 1] <= term) continue;
			df_2[0].push_back(df_1[0][i + 1] - term);
			df_2[1].push_back(df_1[1][i + 1] / df_adj_spots_spotl);
		}
		term_offset = -term;
	}
	else if (spotdate_l > spotdate_s)
	{
		double term = dc_act365.getTerm(spotdate_s, spotdate_l);
		for (unsigned int i = 0; i < df_1[0].size()-1; i++)
		{
			df_2[0].push_back(df_1[0][i + 1] + term);
			df_2[1].push_back(df_1[1][i + 1] * df_adj_spots_spotl);
		}
		term_offset = term;
	}
	else
	{
		df_2[0] = df_1[0];
		df_2[1] = df_1[1];
	}

	if (is_fwd_l)
	{
		fwd_termsmtx[0].push_back(term_offset);
		fwd_termsmtx[1].push_back(fwd_term_l + term_offset);
		fwds.push_back(fwd_rate_l);
	}


	//////////////////////////
	//calculate df by Future//
	//////////////////////////
	if (is_f_use && size_futures != 0)
	{
		if (isOldMethod) 
		{
			if (pRatePriority != 0 && !isFuturePriority)// Libor priority case
			{
				vector<AQLObject*>::iterator it = data_future.end();
				do
				{
					it--;
					const AQLDate& sdate = dynamic_cast<const AQLDataDate&> (((*it)->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
					if (sdate < df_2_date.back())
					{
						//enddate of future
						const AQLDataHolder* dh = &((*it)->getData(IR_CALIBRATION_DATA_TERM, NOCHECK));
						AQLDate edate;
						if (dh->isDefined() && !dh->isNull()) 
						{
							const AQLPriceDataCalendar& calendar  = dynamic_cast<const AQLPriceDataCalendar&> (((*it)->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
							const AQLPriceDataSlidingRule& slidingRule  = dynamic_cast<const AQLPriceDataSlidingRule&> (((*it)->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
							const AQLString& term_str = dynamic_cast<const AQLDataString&> (dh->get()).get();
							edate = AQLDateCalculations::getDate(sdate, term_str, true);
							edate = (!slidingRule.isNull() && !calendar.isNull()) ? slidingRule.getDate(edate, calendar) : edate;

						}
						else
							edate = dynamic_cast<const AQLDataDate&> (((*it)->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
						if (edate <= df_2_date.back())
							it++;					
						data_future.erase(data_future.begin(), it);
						break;
					}
					else if (sdate == df_2_date.back())
					{
						data_future.erase(data_future.begin(), it);
						break;
					}
				} while (it != data_future.begin());	
				size_futures = data_future.size();
			}
			
			AQLDate edate;
			for (unsigned int i = 0; i < size_futures; i++)
			{
				const AQLPriceDataDayCount& dayCount   = dynamic_cast<const AQLPriceDataDayCount&> ((data_future[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
				const AQLDate& sdate = dynamic_cast<const AQLDataDate&> ((data_future[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
				freq = dynamic_cast<const AQLDataString&> ((data_future[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
				freq.toUpper();
			
				if (i > 0 && (sdate.yearOfEra() != edate.yearOfEra() || sdate.monthOfYear() !=  edate.monthOfYear()))
				{
					AQLString msg = "Fwd end date and next fwd start date must be same up to month";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				
				const AQLDataHolder* dh;
				// get term
				dh = &(data_future[i]->getData(IR_CALIBRATION_DATA_TERM, NOCHECK));
				if (dh->isDefined() && !dh->isNull()) 
				{
					const AQLPriceDataCalendar& calendar  = dynamic_cast<const AQLPriceDataCalendar&> ((data_future[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
					const AQLPriceDataSlidingRule& slidingRule  = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_future[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
					const AQLString& term_str = dynamic_cast<const AQLDataString&> (dh->get()).get();
					edate = AQLDateCalculations::getDate(sdate, term_str, true);
					edate = (!slidingRule.isNull() && !calendar.isNull()) ? slidingRule.getDate(edate, calendar) : edate;
				}
				else
					edate = dynamic_cast<const AQLDataDate&> ((data_future[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
				
				
				if (i + 1 != size_futures)
				{
					const AQLDate& sdate_next = dynamic_cast<const AQLDataDate&> ((data_future[i + 1]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
					if (edate != sdate_next)
					{
						//warning
						int span = edate.intervalDays(sdate_next);
						fprintf(stderr, "Warning!!! One Future enddate is not next future startdate. Blank is %d days\n", span);

						edate = sdate_next;
					}				
				}

				double term = dayCount.getTerm(sdate, edate, false);
				
				// get rate
				double rate;
				dh = &(data_future[i]->getData(CALIBRATION_DATA_RATE, NOCHECK));
				if (dh->isDefined() && !dh->isNull()) 
					rate = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
				else 
				{
					double price = dynamic_cast<const AQLDataDouble&> ((data_future[i]->getData(PRICING_DATA_PRICE, ISNOTNULL)).get()).get(); 
					rate = 1.0 - price * 0.01;
				}			
				
				// get spread
				double sp = 0.0;
				dh = &(data_future[i]->getData(PRICING_DATA_SPREAD, NOCHECK));
				if (dh->isDefined() && !dh->isNull()) 
					sp = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
				rate += sp;
				
				// get future volatility
				double f_vol = 0.0;
				dh = &(data_future[i]->getData(PRICING_DATA_FUTUREVOLATILITY, NOCHECK));
				if (dh->isDefined() && !dh->isNull()) 
					f_vol = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
				
				double start_term = dc_act365.getTerm(spotdate_l, sdate);

				RateConvention rc = AQLMathYieldCurve::setRC(freq);
				AQLPriceDataConvention conv(dayCount.getDayCount(), rc);
				if(f_vol > 0.0)
				{
					double term_rate = 1.0 / conv.getDF(rate, sdate, edate) - 1; 
					rate = (term_rate - 1) 
							+ AQLMath::sqrt((AQLMath::sqr(term_rate - 1) +
								   4 * term_rate * AQLMath::exp(AQLMath::sqr(f_vol) * start_term)));
					rate = rate / (2 * term * AQLMath::exp(AQLMath::sqr(f_vol) * start_term));
				}
				
				// df
				double df = conv.getDF(rate, sdate, edate); 
				start_term = dc_act365.getTerm(spotdate_s, sdate);
				
				if (i == 0)
				{
					if (pRatePriority != 0 && !isFuturePriority && df_2_date.back() > sdate)
					{
						df = conv.getDF(rate, df_2_date.back(), edate); 
						df *= df_2[1].back();
					}
					else if (size_libor_fixings == 1 || df_2_date.back() < sdate) //this case, use last libor
					{
						freq = dynamic_cast<const AQLDataString&> ((data_libor.back()->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
						freq.toUpper();
						rate = dynamic_cast<const AQLDataDouble&> ((data_libor.back()->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
						const AQLPriceDataDayCount& dc_l = dynamic_cast<const AQLPriceDataDayCount&> ((data_libor.back()->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
						rc = AQLMathYieldCurve::setRC(freq);
						AQLPriceDataConvention conv_l(dc_l.getDayCount(), rc);
						if (size_libor_fixings == 1 && df_2_date.back()  >= sdate)					
							df *= conv_l.getDF(rate, spotdate_s, sdate);
						else
						{
							df *= df_2[1].back();
							df *= conv_l.getDF(rate, df_2_date.back(), sdate);
							/////////////
							//warning!!//
							/////////////
							int span = df_2_date.back().intervalDays(sdate);
							fprintf(stderr, "Warning!!! Libor is not reached to Future. Expand Last Libor rate %d days\n", span); 
						}
					}
					else // this case, use interpolated df
					{
						pInter_fw->set(df_2[0], df_2[1]);
						df *= pInter_fw->value(start_term);
					}			
				}
				else
				{
					if (df_2_date.back() == sdate)
					{
						df *= df_2[1].back();
					}
					else
					{
						pInter_fw->set(df_2[0], df_2[1]);
						df *= pInter_fw->value(start_term);
					}
				}
				
				
				// If future is prior to libor, remove libor whish is same or after first future startdate 
				if (i == 0 && pRatePriority != 0 && isFuturePriority)
				{
					DoubleArray::iterator it, it2;		
					DateVector::iterator it2_d = df_2_date.begin();
					it = df_2[0].begin();
					it2 = df_2[1].begin();
					
					for (; it != df_2[0].end(); it++, it2++, it2_d++)
					{
						if (*it2_d >= sdate)
						{
							df_2[0].erase(it, df_2[0].end());
							df_2[1].erase(it2, df_2[1].end());
							df_2_date.erase(it2_d, df_2_date.end());
							break;
						}
					}
				}
				term = dc_act365.getTerm(spotdate_s, edate);
				// insert new term and df 
				if (pRatePriority != 0 || term > df_2[0].back()) 
				{
					df_2[0].push_back(term);
					df_2[1].push_back(df);
					df_2_date.push_back(edate);
				}
				else
				{
					DoubleArray::iterator it,it2;
					DateVector::iterator it2_d = df_2_date.begin();
					it = df_2[0].begin();
					it2 = df_2[1].begin();

					for (; it != df_2[0].end(); it++, it2++, it2_d++)
					{
						if (*it2_d < edate) continue;
						else if (*it2_d == edate)
						{
							//error
							AQLString msg = "Future grid and Libor grid are same, in such a case, rate priority must be set.";
							throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						else
						{
							df_2[0].insert(it, term);
							df_2[1].insert(it2, df);
							df_2_date.insert(it2_d, edate);
							break;	
						}
					}
				}
			}
		}
		else
		{

        // This struct stores interpolation data from bootstrapping routine needed within 'insertSyntheticTensionPoints'
        // Must be initialized outside the instrument data bootstrapping routine.
        tensionMarketData futuresInstrumentData;

			AQLInterpolationBase *tmp_inter = dynamic_cast<AQLInterpolationBase *>(pInter_yg->clone());
			for (unsigned int i = 0; i < size_futures; i++)
			{
				tmp_inter->set(df_2[0], df_2[1]);
				const AQLPriceDataDayCount& dayCount   = dynamic_cast<const AQLPriceDataDayCount&> ((data_future[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
				const AQLDate& sdate = dynamic_cast<const AQLDataDate&> ((data_future[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
				if (sdate <= spotdate_s) continue;

				const AQLDataHolder* dh;
				// get term
				const AQLDate& edate = dynamic_cast<const AQLDataDate&> ((data_future[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
				if (edate >= firstSwapDate) break;

			// get apply tension
			bool applyTensionFutures = false;
			const AQLDataHolder *ahApplyTensionFutures = &(data_future[i]->getData(IR_CALIBRATION_DATA_APPLYTENSION, NOCHECK));
			if (ahApplyTensionFutures->isDefined() && !ahApplyTensionFutures->isNull())
			{
				applyTensionFutures = dynamic_cast<const AQLDataBool &>(ahApplyTensionFutures->get()).get();
			}

            // get tension gap
            int tensionGapFutures = dynamic_cast<const AQLDataInt&> ((data_future[i]->getData(IR_CALIBRATION_DATA_TENSIONGAP, ISNOTNULL)).get()).get();
            if ( applyTensionFutures && tensionGapFutures < 1 )
            {
				AQLString msg = "#Error: The TensionGap parameter in the Futures conventions table must be a positive integer.";
  				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
            }

				// get rate
				double rate;
				dh = &(data_future[i]->getData(CALIBRATION_DATA_RATE, NOCHECK));
				if (dh->isDefined() && !dh->isNull()) 
					rate = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
				else 
				{
					double price = dynamic_cast<const AQLDataDouble&> ((data_future[i]->getData(PRICING_DATA_PRICE, ISNOTNULL)).get()).get(); 
					rate = 1.0 - price * 0.01;
				}			
				
				// get spread
				double sp = 0.0;
				dh = &(data_future[i]->getData(PRICING_DATA_SPREAD, NOCHECK));
				if (dh->isDefined() && !dh->isNull()) 
					sp = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
				rate += sp;
				
				// get future volatility
				double f_vol = 0.0;
				dh = &(data_future[i]->getData(PRICING_DATA_FUTUREVOLATILITY, NOCHECK));
				if (dh->isDefined() && !dh->isNull()) 
					f_vol = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
				
				RateConvention rc = AQLMathYieldCurve::setRC(SIMPLE);
				AQLPriceDataConvention conv(dayCount.getDayCount(), rc);
				if(f_vol > 0.0)
				{
					//Convexity Adjust by Ho-Lee Model or Hull-White (1 Factor) Model
					double start_term = dc_act365.getTerm(basedate, sdate);
					double end_term = dc_act365.getTerm(basedate, edate);

                    if ( start_term > end_term )
                    {
                        throw AQLCoreInvalidData("AQLPriceYieldGenerator::calcDiscountFactor failed. In a future grid, an end date is before a start date!", 
								__FILE__, __LINE__);
                    }

					bool isConvAdjPrecise = false;
					dh = &(data_future[i]->getData(IR_CALIBRATION_DATA_ISCONVADJPRECISE, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						isConvAdjPrecise = dynamic_cast<const AQLDataBool &>(dh->get()).get();
					}

                    // Use the Hull-White 1 Factor Model if the mean reversion parameter is populated else use the Ho-Lee Model
                    double meanReversion = 0.0;
                    dh = &( data_future[i]->getData( IR_CALIBRATION_DATA_MEANREVERSION, NOCHECK ));
                    if( dh->isDefined() && !dh->isNull() )
                    {
                        meanReversion = dynamic_cast< const AQLDataDouble & >( dh->get() ).get();
                    }
                    
                    // The Hull-White mean reversion parameter must be a value between 0 and 1. Throw an error if this is not the case.
                    if ( meanReversion < 0.0 || meanReversion > 1.0 )
                    {
                        throw AQLCoreInvalidData("AQLPriceYieldGenerator::calcDiscountFactor failed. In the future's grid the convexity mean reversion parameter must be a value between 0 and 1.", 
                            __FILE__, __LINE__);
                    }
                    
					if (isConvAdjPrecise)
					{
						// refer to a document of Bloomberg about convexity adjust of euro dollar future
						double tau = dayCount.getTerm(sdate, edate);
						if (tau <= 0.0)
							throw AQLCoreInvalidData("AQLPriceYieldGenerator::calcDiscountFactor failed. In a future grid, an end date is before a start date!", 
								__FILE__, __LINE__);

						double rate_continuous = AQLMath::log(1. + tau * rate) / tau;
						double conv_adjust_continuous = 0.0;
                        if ( meanReversion == 0.0 )
                        {
                            // DEFAULT METHOD:
                            // Use the Ho-Lee Model by default if the HW mean reversion parameter is not populated
                            conv_adjust_continuous = 0.5 * start_term * end_term * f_vol * f_vol;
                        }
                        else
                        {
                            // ALTERNATIVE METHOD:
                            // Use the Hull-White 1 Factor model if the mean reversion parameter is populated
                            double a        = meanReversion;    // We already checked to ensure meanReversion > 0
                            double T1       = start_term;       // We already checked start_term < end_term
                            double T2       = end_term;
                            double B_T1_T2  = ( 1.0 - AQLMath::exp( -a * ( T2 - T1 ) ) ) / a;
                            double B_0_T1   = ( 1.0 - AQLMath::exp( -a * T1 ) ) / a;

                            // This is the Hull-White 1 Factor Futures Convexity Adjustment
                            // See Options, Futures and Other Derivatives 9ed by John Hull
                            // Specifically see Technical Note 1 - Convexity Adjustments to Eurodollar Futures
                            conv_adjust_continuous = ( B_T1_T2 / ( T2 - T1 ) )
                                * ( B_T1_T2 * ( 1 - AQLMath::exp( -2 * a * T1 ) ) + ( 2 * a * B_0_T1 * B_0_T1 ) )
                                * ( f_vol * f_vol / ( 4 * a ) );
                        }

						double conv_adjust_simple =	(1. - AQLMath::exp(- conv_adjust_continuous * tau)) * (rate + 1. / tau);
						rate -= conv_adjust_simple;
					}
					else
					{
                        if ( meanReversion == 0.0 )
                        {
						    // DEFAULT METHOD:
                            // Use the Ho-Lee Model if the HW mean reversion parameter is not populated
                            rate -= 0.5 * start_term * end_term * f_vol * f_vol;
                        }
                        else
                        {
                            // ALTERNATIVE METHOD:
                            // Use the Hull-White 1 Factor model if the mean reversion parameter is populated
                            double a        = meanReversion;    // We already checked to ensure meanReversion > 0
                            double T1       = start_term;       // We already checked start_term < end_term
                            double T2       = end_term;
                            double B_T1_T2  = ( 1.0 - AQLMath::exp( -a * ( T2 - T1 ) ) ) / a;
                            double B_0_T1   = ( 1.0 - AQLMath::exp( -a * T1 ) ) / a;

                            // This is the Hull-White 1 Factor Futures Convexity Adjustment
                            // See Options, Futures and Other Derivatives 9ed by John Hull
                            // Specifically see Technical Note 1 - Convexity Adjustments to Eurodollar Futures
                            rate    -= ( B_T1_T2 / ( T2 - T1 ) )
                                    * ( B_T1_T2 * ( 1 - AQLMath::exp( -2 * a * T1 ) ) + ( 2 * a * B_0_T1 * B_0_T1 ) )
                                    * ( f_vol * f_vol / ( 4 * a ) );
                        }
					}
				}
				
				// df
				double start_term = dc_act365.getTerm(spotdate_s, sdate);
				double end_term = dc_act365.getTerm(spotdate_s, edate);
				double startDF = tmp_inter->value(start_term);
				double endDF = startDF * conv.getDF(rate, sdate, edate);
				// insert startDate, startDF, startTerm
				insertDFData(df_2, df_2_date, startDF, start_term, sdate);
				insertDFData(df_2, df_2_date, endDF, end_term, edate);
				//fwd_termsmtx[0].push_back(start_term);
				//fwd_termsmtx[1].push_back(end_term);
				//fwds.push_back(rate);		
			insertForwardRateData(fwd_termsmtx, fwds, start_term, end_term, rate);

            // Apply Tension to the Futures part of the Curve by Linear Interpolating on Forwards to Create Synthetic Points 
            const unsigned int dontApplyTensionToLastNFutures = 2;

            if ( applyTensionFutures && size_futures > 1 && i < size_futures - dontApplyTensionToLastNFutures )
            {
                tmp_inter->set( df_2[0], df_2[1] ); // Set( terms, DFs )

                insertSyntheticTensionPoints( df_2_date,
                                              df_2,
                                              fwds,
                                              fwd_termsmtx,
                                              tmp_inter,
                                              spotdate_s,
                                              sdate,
                                              edate,
                                              rate,
                                              ( i == 0 ) ? true : false , // isFirstDataPoint
                                              dc_act365,
                                              dayCount.getDayCount(),
                                              tensionGapFutures,
                                              futuresInstrumentData,
                                              firstSwapDate );
            }
                
            // For Analytical Risk
				DateVector futureCalcDates(2);
				futureCalcDates[0] =sdate;futureCalcDates[1] = edate;
				if(!data_future[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
				{
					data_future[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(futureCalcDates));
				}
				else
				{
					dynamic_cast<AQLDataDates&>(data_future[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(futureCalcDates);
				}
			}
			delete tmp_inter;
		}
	}

	if (is_fra_use)
	{
		AQLInterpolationBase *tmp_inter = dynamic_cast<AQLInterpolationBase *>(pInter_yg->clone());
		
	    // This struct stores interpolation data from bootstrapping routine needed within 'insertSyntheticTensionPoints'
        // Must be initialized outside the instrument data bootstrapping routine.
        tensionMarketData fraInstrumentData; 

		for (size_t i = 0; i < data_fra.size(); i++)
		{
			tmp_inter->set(df_2[0], df_2[1]);
			double rate = dynamic_cast<const AQLDataDouble&> ((data_fra[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			const AQLString& terms_str_x = dynamic_cast<const AQLDataString&> ((data_fra[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			const AQLString& terms_str = changeFRATermFormat(terms_str_x);
			const AQLDate spotDate = dynamic_cast<const AQLDataDate&> ((data_fra[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get()).get();
			const AQLPriceDataSlidingRule& slidingRule = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_fra[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			const AQLPriceDataDayCount& dayCount = dynamic_cast<const AQLPriceDataDayCount&> ((data_fra[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			const AQLPriceDataCalendar& calendar = dynamic_cast<const AQLPriceDataCalendar&> ((data_fra[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());		
            
			// set applyTensionFRAs flag
            bool applyTensionFRAs = false;
            const AQLDataHolder *ahApplyTensionFRAs = &(data_fra[i]->getData(IR_CALIBRATION_DATA_APPLYTENSION, NOCHECK));
            if (ahApplyTensionFRAs->isDefined() && !ahApplyTensionFRAs->isNull())
		    {
			    applyTensionFRAs = dynamic_cast<const AQLDataBool &>(ahApplyTensionFRAs->get()).get();
		    }

            // set TensionGap flag
            int tensionGapFRAs = dynamic_cast<const AQLDataInt&> ((data_fra[i]->getData(IR_CALIBRATION_DATA_TENSIONGAP, ISNOTNULL)).get()).get();
            if ( applyTensionFRAs && tensionGapFRAs < 1 )
            {
				AQLString msg = "#Error: The TensionGap parameter in the FRA conventions table must be a positive integer.";
  				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
            }
		    
            bool eom = false;
			dh = &(data_fra[i]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			}
			// set roll convention
			AQLString roll_conv("");
			if (baseFreq == LUNAR) roll_conv = ROLLCONV_LUNAR;
			else if (eom) roll_conv = ROLLCONV_EOM;
			else roll_conv = ROLLCONV_NORMAL;

			AQLDate startDate, endDate;
			if (is_fwdswap)
			{
				const bool is_date = dynamic_cast<const AQLDataBool&> ((data_fra[i]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
				if (is_date)
				{
					startDate = dynamic_cast<const AQLDataDate&> ((data_fra[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
					endDate = dynamic_cast<const AQLDataDate&> ((data_fra[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
				}
				else
				{
					const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((data_fra[i]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
					startDate = AQLDateCalculations::getDate(spotDate, sterm_str, *swap_slidingRule[i], swap_calendar[i], true, &swap_rollConvention[i]);
					const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((data_fra[i]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
					endDate = AQLDateCalculations::getDate(startDate, tenor_str, *swap_slidingRule[i], swap_calendar[i], true, &swap_rollConvention[i]);
				}
			}
			else
			{
				startDate = AQLDateCalculations::getDate(spotDate, terms_str, slidingRule, &calendar, true, &roll_conv);
				endDate = AQLDateCalculations::getDate(startDate, refRateTerm, slidingRule, &calendar, true, &roll_conv);
			}

            // Don't Insert Any Forwards if EndDate >= FirstSwapDate
            if (endDate >= firstSwapDate) break;
			
            // Interest Rate Convention - Stores instrument daycount and compounding conventions e.g. Simple Interest Act/Act.
            RateConvention rc = AQLMathYieldCurve::setRC(SIMPLE);
			AQLPriceDataConvention conv(dayCount.getDayCount(), rc);

            double startTerm = dc_act365.getTerm(spotdate_s, startDate);
			double endTerm = dc_act365.getTerm(spotdate_s, endDate);
			double startDF = tmp_inter->value(startTerm);
			double endDF = startDF * conv.getDF(rate, startDate, endDate);
			
            // Update Forward Rate and Discount Factor at Start and End of Forward Period
			insertDFData(df_2, df_2_date, startDF, startTerm, startDate);
            insertDFData(df_2, df_2_date, endDF, endTerm, endDate);
            insertForwardRateData(fwd_termsmtx, fwds, startTerm, endTerm, rate);

            // Apply Tension to the FRA part of the Curve by Linear Interpolating on Forwards to Create Synthetic Points 
            if ( applyTensionFRAs && data_fra.size() > 1 )
            {
                tmp_inter->set( df_2[0], df_2[1] );  // Set( terms, DFs )

                insertSyntheticTensionPoints( df_2_date,
                                              df_2,
                                              fwds,
                                              fwd_termsmtx,
                                              tmp_inter,
                                              spotdate_s,
                                              startDate,
                                              endDate,
                                              rate,
                                              ( i == 0 ) ? true : false, // isFirstDataPoint
                                              dc_act365,
                                              dayCount.getDayCount(),
                                              tensionGapFRAs,
                                              fraInstrumentData,
                                              firstSwapDate );
            }

            // for analytic risk
			DateVector fraCalcDates(2);
			fraCalcDates[0] =startDate;fraCalcDates[1] = endDate;
			if(!data_fra[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
			{
				data_fra[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(fraCalcDates));
			}
			else
			{
				dynamic_cast<AQLDataDates&>(data_fra[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(fraCalcDates);
			}
		}
		delete tmp_inter;
	}
	
	////////////////////////
	//calculate df by Swap//
	////////////////////////
	//First, change df(by libor and future) to rate with swap rate convention
	unsigned int size_libor_fixings2 = df_2[0].size() - 1;
	DoubleMatrix rate_2(2, DoubleArray(size_libor_fixings2));
	DateVector rate_2_date = df_2_date;
	AQLDate l_last = df_2_date.back();
	rate_2_date.erase(rate_2_date.begin());
	if (size_libor_fixings2 == 1)
	{
		rate_2[0][0] = df_2[0][1];

		AQLString freq = dynamic_cast<const AQLDataString&> ((data_libor.back()->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		freq.toUpper();
		double rate = dynamic_cast<const AQLDataDouble&> ((data_libor.back()->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		const AQLPriceDataDayCount& dc_l = dynamic_cast<const AQLPriceDataDayCount&> ((data_libor.back()->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		RateConvention rc = AQLMathYieldCurve::setRC(freq);
		AQLPriceDataConvention conv_l(dc_l.getDayCount(), rc);
							
		DoubleArray terms, terms_interval;
		DateVector dates;
		getPaymentDates(spotdate_s, df_2_date[1], swap_frequency[0], *swap_calendar[0], *swap_slidingRule[0], *swap_daycount[0], dates, terms, terms_interval, swap_useEOMRollConvention[0]);
		double sum = 0.0;
		for (unsigned int i = 0; i < terms.size(); i++)
			sum += conv_l.getDF(rate, spotdate_s, dates[i]) * terms_interval[i];
		rate_2[1][0] = (1.0 - df_2[1][1]) / sum;
	}
	else
	{
		pInter->set(df_2[0], df_2[1]);
		for (unsigned int i = 0; i < size_libor_fixings2; i++)
		{
			rate_2[0][i] = df_2[0][i + 1];
			rate_2[1][i] = getSwapRate(*pInter, spotdate_s, df_2_date[i + 1], swap_frequency[0], *swap_calendar[0], *swap_slidingRule[0], *swap_daycount[0], swap_useEOMRollConvention[0]);
		}
	}
	DateVector dates_s(size_swaps);
	DoubleVector rate_s(size_swaps);
	bool isTimeInter  = false;
	dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONSW, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isTimeInter = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	DoubleVector tenorSwapSpreadVec(size_swaps, 0.0);
	bool isLeg1BaseFreq = false;
	bool isLeg2Spread = false;
	DoubleArray b_t_grid(1, 0.0);
	DoubleArray b_termstruct_grid(1, 0.0);
	AQLPriceDataDayCount *pDaycount_tenorswap = NULL;
	AQLPriceDataCalendar *pCal_tenorswap = NULL;
	AQLPriceDataSlidingRule *pSld_tenorswap = NULL;
	AQLString tenorSwapFreq = "";
	bool eom_tenorswap = false;
	AQLString roll_conv_ts("");
	bool isTimeInter_tenorswap = false;
	if (isSwapTenorAdjust)
	{
		if (data_tenorswap.size() == 0) 
			throw AQLCoreInvalidData("No tenor swap!", __FILE__, __LINE__);
		isLeg2Spread = dynamic_cast<const AQLDataBool&> ((data_tenorswap[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL)).get()).get();
		// set frequency 
		const AQLString& leg1Freq = dynamic_cast<const AQLDataString&> ((data_tenorswap[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL)).get()).get();
		const AQLString& leg2Freq = dynamic_cast<const AQLDataString&> ((data_tenorswap[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, ISNOTNULL)).get()).get();
		

		if (isLeg2Spread)
		{
			tenorSwapFreq = leg2Freq;
		}
		else
		{
			tenorSwapFreq = leg1Freq;
		}

		if (leg1Freq == baseFreq) 
		{
			isLeg1BaseFreq = true;		
		}
		else if (leg2Freq == baseFreq) 
		{
			isLeg1BaseFreq = false;
		}
		else throw AQLCoreInvalidData("Tenor swap frequency and swap frequency do not match!", __FILE__, __LINE__);

		pDaycount_tenorswap = &dynamic_cast<AQLPriceDataDayCount &>(data_tenorswap[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
		pCal_tenorswap = &dynamic_cast<AQLPriceDataCalendar &>(data_tenorswap[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
		pSld_tenorswap = &dynamic_cast<AQLPriceDataSlidingRule &>(data_tenorswap[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
		dh = &(data_tenorswap[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			eom_tenorswap = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}
		// set roll convention
		if (tenorSwapFreq == LUNAR) roll_conv_ts = ROLLCONV_LUNAR;
		else if (eom_tenorswap) roll_conv_ts = ROLLCONV_EOM;
		else roll_conv_ts = ROLLCONV_NORMAL;
		dh = &data_tenorswap[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, ISNOTNULL);
		if (dh->isDefined() && !dh->isNull())
		{
			isTimeInter_tenorswap = dynamic_cast<const AQLDataBool&>(dh->get()).get();
		}
		for (unsigned int i = 0; i < data_tenorswap.size(); i++)
		{
			const AQLString &strTerm  = dynamic_cast<const AQLDataString &>((data_tenorswap[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			double rate = dynamic_cast<const AQLDataDouble &>((data_tenorswap[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			AQLDate tmpDate = AQLDateCalculations::getDate(spotdate_s, strTerm, *pSld_tenorswap, pCal_tenorswap, true, &roll_conv_ts);
			double term = pDaycount_tenorswap->getTerm(spotdate_s, tmpDate);
			b_t_grid.push_back(term);
			if (isTimeInter_tenorswap)
			{
				b_termstruct_grid.push_back(rate * term);
			}
			else
			{
				b_termstruct_grid.push_back(rate);
			}
		}

		if (pInter_ts->isHybrid())
		{
			double linearSplineJoinDateAsDouble = 0.;
			pInter_ts->setJoinDateAsDouble(linearSplineJoinDateAsDouble);
		}
		pInter_ts->set(b_t_grid, b_termstruct_grid);
	}

	for (unsigned int i = 0; i < size_swaps; i++)
	{		
		const AQLString& term_str = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		AQLDate date = AQLDateCalculations::getDate(spotdate_s, term_str, *swap_slidingRule[i], swap_calendar[i], true, &swap_rollConvention[i]);
		dates_s[i] = date;
		
		if (pRatePriority != 0 && !isSwapPriority
				&& l_last.intervalDays(date) <= GRIDSPAN) continue;
		
		double term = dc_act365.getTerm(spotdate_s, date);		
		double rate = dynamic_cast<const AQLDataDouble&> ((data_swap[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		rate_s[i] = rate;
		
		if (isSwapTenorAdjust)
		{
			if (swap_floatLeg_frequency[i] != baseFreq)
			{
				const AQLString& term_str_swap = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				AQLDate tmpDate = AQLDateCalculations::getDate(spotdate_s, term_str_swap, *pSld_tenorswap, pCal_tenorswap, true, &roll_conv_ts);
				double term_basis = pDaycount_tenorswap->getTerm(spotdate_s, tmpDate);				
				if (isTimeInter_tenorswap)
				{
					tenorSwapSpreadVec[i] = pInter_ts->value(term_basis) / term_basis;
				}
				else
				{
					tenorSwapSpreadVec[i] = pInter_ts->value(term_basis);
				}					
				if ((isLeg2Spread && !isLeg1BaseFreq) || (!isLeg2Spread && isLeg1BaseFreq)) tenorSwapSpreadVec[i] *= -1.;
			}
		}
	
		if (pRatePriority == 0) // no rate priority
		{
			unsigned int pos;
			AQLAlgorithm::locate(rate_2_date, date, rate_2_date.size(), pos);
			if (pos == rate_2_date.size())
			{
				rate_2[0].push_back(term);
				rate_2[1].push_back(rate);
				rate_2_date.push_back(date);
			}
			else if (date == rate_2_date[pos])
			{
				//error
				AQLString msg = "Swap grid and Future or Libor grid are same, in such a case, rate priority must be set.";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			else
			{
				rate_2[0].insert(rate_2[0].begin() + pos, term);
				rate_2[1].insert(rate_2[1].begin() + pos, rate);
				rate_2_date.insert(rate_2_date.begin() + pos, date);
			}

		}
		else if (isSwapPriority) // swap is prior to libor and future
		{
			if (i == 0)
			{
				DoubleArray::iterator it, it2, it_f, it2_f;
				it = rate_2[0].begin();
				it2 = rate_2[1].begin();
				it_f= df_2[0].begin();
				it2_f = df_2[1].begin();
				it_f++;//skip first element (term = 0.0);
				it2_f++;//skip first element (df = 1.0);
				DateVector::iterator it2_d = rate_2_date.begin();
				DateVector::iterator it2_d_f = df_2_date.begin();
				it2_d_f++;//skip first element (date = basedate)
				for (;it != rate_2[0].end(); it++, it2++, it_f++, it2_f++, it2_d++, it2_d_f++)
				{
					if ((*it2_d).intervalDays(date) <= GRIDSPAN)
					{
						rate_2[0].erase(it, rate_2[0].end());
						rate_2[1].erase(it2, rate_2[1].end());
						rate_2_date.erase(it2_d, rate_2_date.end());
						df_2[0].erase(it_f, df_2[0].end());
						df_2[1].erase(it2_f, df_2[1].end());
						df_2_date.erase(it2_d_f, df_2_date.end());
						break;
					}
				}
			}
			rate_2[0].push_back(term);
			rate_2[1].push_back(rate);
			rate_2_date.push_back(date);
		}
		else //libor and future are prior to swap
		{
			rate_2[0].push_back(term);
			rate_2[1].push_back(rate);
			rate_2_date.push_back(date);
		}
	}

	DoubleMatrix df_3(2);
	df_3[0] = df_2[0];
	df_3[1] = df_2[1];

	if (rate_2[0].size() > 2)
	{
		if (isTimeInter)
		{
			DoubleArray index = rate_2[0];
			DoubleArray val(index.size());
			for (unsigned int i = 0; i < index.size(); ++i)
			{
				val[i] = (rate_2[1][i] * rate_2[0][i]);
			}
			pInter_yg->set(index, val);
		}
		else
		{
			pInter_yg->set(rate_2[0], rate_2[1]);
		}
	}

	bool is_newton = false;
	dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISNEWTONRAPHSONSW, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		is_newton = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	bool is_simueq = false;
	dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQSW, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		is_simueq = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	if (pDFCurveName && *pDFCurveName != ITSELF)
	{
        if (!is_newton || baseFreq == AQLString(""))
			throw AQLCoreInvalidData("We have to set newton raphson and floating leg frequency when df curve is not STD!", __FILE__, __LINE__);
		for (size_t i=0; i<swap_floatLeg_daycount.size(); i++)
		{
			if (!swap_floatLeg_daycount[i]) 
				throw AQLCoreInvalidData("We have to set floating leg daycount when df curve is not STD!", __FILE__, __LINE__);
		}
	}
	if (is_newton)
	{
		const double EPS_PV = 1.0e-15;
		const double MIN_VAL = 1.0e-10;
		const unsigned int MAX_LOOP = 100;
		AQLInterpolationBase *tmp_inter = dynamic_cast<AQLInterpolationBase *>(pInter_yg->clone());
		AQLInterpolationBase *df_inter = dynamic_cast<AQLInterpolationBase *>(pInter->clone());
		double df_spot_s =1.0;
		double term_spot_s = 0.0;
		if (pDFCurveName && *pDFCurveName != ITSELF)
		{
			AQLString suffix = "";
			if (*pDFCurveName != STD)
			{
				suffix = AQLString("_") + *pDFCurveName;
			}

			if (!objHolder.getData(CALIBRATION_DATA_TERMS + suffix).isDefined())
				throw AQLCoreInvalidData("terms error", __FILE__,__LINE__);
			const DoubleVector &terms_ = dynamic_cast<const AQLDataDoubles&> (objHolder.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get()).get();
			
			if (!objHolder.getData(IR_CALIBRATION_DATA_DFS + suffix).isDefined())
				throw AQLCoreInvalidData("discount error", __FILE__,__LINE__);
			const DoubleVector &dfs_ = dynamic_cast<const AQLDataDoubles&> (objHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get()).get();

			df_inter->set(terms_, dfs_);
			term_spot_s = dc_act365.getTerm(basedate, spotdate_s);
			df_spot_s = df_inter->value(term_spot_s);
		}

		if (is_simueq)
		{
			vector<DoubleArray> terms_gridVec(size_swaps), terms_gridVec_float(size_swaps), terms_gridVec_ts(size_swaps), 
				rate_tauVec(size_swaps), rate_tauVec_float(size_swaps), rate_tauVec_ts(size_swaps);
			vector<DateVector> datesVec(size_swaps), datesVec_float(size_swaps), datesVec_ts(size_swaps);
			
			DoubleArray time_index = df_2[0];
			DoubleArray rate_timeVec(df_2[0].size());
			for (unsigned int j = 0; j < df_2[0].size(); j++)
			{
				rate_timeVec[j] = -AQLMath::log(df_2[1][j]);
			}

			for (unsigned int i = 0; i < size_swaps; i++)
			{
				DoubleArray terms_interval,terms_interval_ts;
				getPaymentDates(spotdate_s, dates_s[i], swap_frequency[i], *swap_calendar[i], *swap_slidingRule[i], *swap_daycount[i], datesVec[i], terms_gridVec[i], terms_interval, swap_useEOMRollConvention[i]);
				DateVector tmpdates(datesVec[i]);
				tmpdates.insert(tmpdates.begin(), spotdate_s);
				if(!data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
				{
					data_swap[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(tmpdates));
				}
				else
				{
					dynamic_cast<AQLDataDates&>(data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(tmpdates);
				}

				unsigned int size_grid = datesVec[i].size();
				rate_tauVec[i].resize(size_grid);
				time_index.push_back(terms_gridVec[i].back());
				rate_timeVec.push_back(0.0);
				for (unsigned int j = 0; j < size_grid; j++)
				{
					rate_tauVec[i][j] = rate_s[i] * terms_interval[j];
				}
				
				if (isSwapTenorAdjust)
				{
					getPaymentDates(spotdate_s, dates_s[i], tenorSwapFreq, *pCal_tenorswap, *pSld_tenorswap, *pDaycount_tenorswap, datesVec_ts[i], terms_gridVec_ts[i], terms_interval_ts, eom_tenorswap);
					unsigned int size_grid_ts = datesVec_ts[i].size();
					rate_tauVec_ts[i].resize(size_grid_ts);
					for (unsigned int j = 0; j < size_grid_ts; j++)
					{
						rate_tauVec_ts[i][j] = tenorSwapSpreadVec[i] * terms_interval_ts[j];
					}
				}

				if (baseFreq != "" && swap_floatLeg_daycount[i])
				{
					DoubleArray tmp;
					// calc floatside date information
					getPaymentDates(spotdate_s, dates_s[i], baseFreq, *swap_calendar[i], *swap_slidingRule[i], *swap_floatLeg_daycount[i], 
											  datesVec_float[i], terms_gridVec_float[i], tmp, swap_useEOMRollConvention[i]);
				}
			}

			tmp_inter->set(time_index, rate_timeVec);
			DoubleArray valVec0(size_swaps);
			for (unsigned int i = 0; i < size_swaps; ++i)
			{
				double df = 1.0;
				double val = 0.0;
				if (pDFCurveName && *pDFCurveName != ITSELF)
				{
					for (unsigned int j = 0; j < terms_gridVec[i].size(); j++)
					{
						df = df_inter->value(terms_gridVec[i][j] + term_spot_s) / df_spot_s;
						val += rate_tauVec[i][j] * df;
					}

					if (isSwapTenorAdjust)
					{
						for (unsigned int j = 0; j < terms_gridVec_ts[i].size(); j++)
						{
							df = df_inter->value(terms_gridVec_ts[i][j] + term_spot_s) / df_spot_s;
							val += rate_tauVec_ts[i][j] * df;
						}
					}
					val -= calcFloatSide(*tmp_inter, *df_inter, df_spot_s, term_spot_s, terms_gridVec_float[i], swap_compoundingTimes[i]);
				}
				else
				{
					for (unsigned int j = 0; j < terms_gridVec[i].size(); j++)
					{
						df = AQLMath::exp(-tmp_inter->value(terms_gridVec[i][j]));
						val += rate_tauVec[i][j] * df;
					}

					if (isSwapTenorAdjust)
					{
						for (unsigned int j = 0; j < terms_gridVec_ts[i].size(); j++)
						{
							df = AQLMath::exp(-tmp_inter->value(terms_gridVec_ts[i][j]));
							val += rate_tauVec_ts[i][j] * df;
						}
					}
					val += 1.0 *  df;
					val -= 1.0;
				}
				valVec0[i] = -1.0 * val;
			}
			unsigned int df2_size = df_2[0].size();
			for (unsigned int i = 0; i < size_swaps; ++i)
			{
				rate_timeVec[df2_size + i] += 1.0e-5;
			}
			
			tmp_inter->set(time_index, rate_timeVec);
			DoubleArray valVec1(size_swaps);
			for (unsigned int i = 0; i < size_swaps; ++i)
			{
				double df = 1.0;
				double val = 0.0;
				if (pDFCurveName && *pDFCurveName != ITSELF)
				{
					for (unsigned int j = 0; j < terms_gridVec[i].size(); j++)
					{
						df = df_inter->value(terms_gridVec[i][j] + term_spot_s) / df_spot_s;
						val += rate_tauVec[i][j] * df;
					}

					if (isSwapTenorAdjust)
					{
						for (unsigned int j = 0; j < terms_gridVec_ts[i].size(); j++)
						{
							df = df_inter->value(terms_gridVec_ts[i][j] + term_spot_s) / df_spot_s;
							val += rate_tauVec_ts[i][j] * df;
						}
					}

					val -= calcFloatSide(*tmp_inter, *df_inter, df_spot_s, term_spot_s, terms_gridVec_float[i], swap_compoundingTimes[i]);
				}
				else
				{
					for (unsigned int j = 0; j < terms_gridVec[i].size(); j++)
					{
						df = AQLMath::exp(-tmp_inter->value(terms_gridVec[i][j]));
						val += rate_tauVec[i][j] * df;
					}

					if (isSwapTenorAdjust)
					{
						for (unsigned int j = 0; j < terms_gridVec_ts[i].size(); j++)
						{
							df = AQLMath::exp(-tmp_inter->value(terms_gridVec_ts[i][j]));
							val += rate_tauVec_ts[i][j] * df;
						}
					}

					val += 1.0 *  df;
					val -= 1.0;
				}
				valVec1[i] = -1.0 * val;
			}
		
			int loopNum = MAX_LOOP;
			while (loopNum--)
			{
				bool isEnd = true;
				for (unsigned int i = 0; i < size_swaps; ++i)
				{
					if (AQLMath::abs(valVec0[i] - valVec1[i]) >= EPS_PV)
					{
						isEnd = false;
						break;
					}
				}
				if (isEnd)
				{
					break;
				}
				AQLMatrix divMat(size_swaps, size_swaps);
				// create divMat
				for (unsigned int i = 0; i < size_swaps; ++i)
				{
					DoubleArray rate_timeVec_t = rate_timeVec;
					rate_timeVec_t[df2_size + i] += MIN_VAL;
					tmp_inter->set(time_index, rate_timeVec_t);
					for (unsigned int j = 0; j < size_swaps; ++j)
					{
						double df = 1.0;
						double val = 0.0;
						if (pDFCurveName && *pDFCurveName != ITSELF)
						{
							for (unsigned int k = 0; k < terms_gridVec[j].size(); ++k)
							{
								df = df_inter->value(terms_gridVec[j][k] + term_spot_s) / df_spot_s;
								val += rate_tauVec[j][k] * df;
							}

							if (isSwapTenorAdjust)
							{
								for (unsigned int k = 0; k < terms_gridVec_ts[j].size(); k++)
								{
									df = df_inter->value(terms_gridVec_ts[j][k] + term_spot_s) / df_spot_s;
									val += rate_tauVec_ts[j][k] * df;
								}
							}

							val -= calcFloatSide(*tmp_inter, *df_inter, df_spot_s, term_spot_s, terms_gridVec_float[j], swap_compoundingTimes[i]);
						}
						else
						{
							for (unsigned int k = 0; k < terms_gridVec[j].size(); ++k)
							{
								df = AQLMath::exp(-tmp_inter->value(terms_gridVec[j][k]));
								val += rate_tauVec[j][k] * df;
							}

							if (isSwapTenorAdjust)
							{
								for (unsigned int k = 0; k < terms_gridVec_ts[j].size(); k++)
								{
									df = AQLMath::exp(-tmp_inter->value(terms_gridVec_ts[j][k]));
									val += rate_tauVec_ts[j][k] * df;
								}
							}

							val += 1.0 *  df;
							val -= 1.0;
						}
						const double divVal = (val + valVec1[j]) / MIN_VAL;
						divMat.setValue(j, i, divVal);
					}
				}

				AQLMatrix invMat = divMat.inverseMatrix();
				AQLMatrix valMat(valVec1);
				AQLMatrix deltaMat = invMat * valMat;
				// plus delta
				for (unsigned int i = 0; i < size_swaps; ++i)
				{
					rate_timeVec[df2_size + i] += deltaMat.getValue(i, 0);
				}
				tmp_inter->set(time_index, rate_timeVec);
				valVec0 = valVec1;
				for (unsigned int i = 0; i < size_swaps; ++i)
				{
					double df = 1.0;
					double val = 0.0;
					if (pDFCurveName && *pDFCurveName != ITSELF)
					{
						for (unsigned int j = 0; j < terms_gridVec[i].size(); j++)
						{
							df = df_inter->value(terms_gridVec[i][j] + term_spot_s) / df_spot_s;
							val += rate_tauVec[i][j] * df;
						}

						if (isSwapTenorAdjust)
						{
							for (unsigned int j = 0; j < terms_gridVec_ts[i].size(); j++)
							{
								df = df_inter->value(terms_gridVec_ts[i][j] + term_spot_s) / df_spot_s;
								val += rate_tauVec_ts[i][j] * df;
							}
						}
						
						val -= calcFloatSide(*tmp_inter, *df_inter, df_spot_s, term_spot_s, terms_gridVec_float[i], swap_compoundingTimes[i]);
					}
					else
					{
						for (unsigned int j = 0; j < terms_gridVec[i].size(); j++)
						{
							df = AQLMath::exp(-tmp_inter->value(terms_gridVec[i][j]));
							val += rate_tauVec[i][j] * df;
						}

						if (isSwapTenorAdjust)
						{
							for (unsigned int j = 0; j < terms_gridVec_ts[i].size(); j++)
							{
								df = AQLMath::exp(-tmp_inter->value(terms_gridVec_ts[i][j]));
								val += rate_tauVec_ts[i][j] * df;
							}
						}

						val += 1.0 *  df;
						val -= 1.0;
					}
					valVec1[i] = -1.0 * val;
				}
			}
			if (loopNum < 0)
			{
				for (unsigned int i = 0; i < size_swaps; ++i)
				{
					// check error is within 0.00001bp
					if (AQLMath::abs(valVec1[i]) > 1.0E-9)
					{
						throw AQLCoreInvalidData("Convergence error in DF calc (Newton Raphson)", __FILE__, __LINE__); 
					}
				}
			}

			for (unsigned int i = 0; i < size_swaps; ++i)
			{
				// As long as floating leg exists, we use floating leg dates as pillar dates
				if (!datesVec_float.back().empty())
				{
					for (unsigned int j = 0; j < terms_gridVec_float[i].size(); ++j)
					{
						insertDFData(df_2, df_2_date, AQLMath::exp(-tmp_inter->value(terms_gridVec_float[i][j])), terms_gridVec_float[i][j], datesVec_float[i][j]);
					}
				}
				else
				{
					for (unsigned int j = 0; j < terms_gridVec[i].size(); ++j)
					{
						insertDFData(df_2, df_2_date, AQLMath::exp(-tmp_inter->value(terms_gridVec[i][j])), terms_gridVec[i][j], datesVec[i][j]);
					}
				}
			}
			const DateVector *p_dateVec;
			DoubleArray *p_terms;
			if (!datesVec_float.back().empty())
			{
				p_dateVec = &datesVec_float.back();
				p_terms = &terms_gridVec_float.back();
			}
			else
			{
				p_dateVec = &datesVec.back();
				p_terms = &terms_gridVec.back();
			}
			DoubleArray tau_swap(p_dateVec->size());
			tau_swap[0] = dc_l.getTerm(spotdate_s, (*p_dateVec)[0]);
			for (unsigned int i = 1; i < tau_swap.size(); ++i)
			{
				tau_swap[i] = dc_l.getTerm((*p_dateVec)[i - 1], (*p_dateVec)[i]);
			}
			p_terms->insert(p_terms->begin(), 0.0);
			tau_swap.insert(tau_swap.begin(), 0.0);
			updateImpliedForwardRates(*tmp_inter, *p_terms, tau_swap, fwd_termsmtx, fwds);
		}
		else
		{
			throw AQLCoreInvalidData("We support Simultaneous-Equation method only.", __FILE__, __LINE__); 
		}
		//{
		//	for (unsigned int i = 0; i < size_swaps; i++)
		//	{
		//		DoubleArray terms_grid, terms_interval, terms_grid_float;
		//		DateVector dates, dates_float;

		//		unsigned int size_grid;
		//		double rate_time0 = 0.0;
		//		double rate_time1 = 0.0;

		//		DoubleArray time_index = df_2[0];
		//		DoubleVector rate_timeVec(df_2[0].size());
		//		for (unsigned int j = 0; j < df_2[0].size(); j++)
		//		{
		//			rate_timeVec[j] = -AQLMath::log(df_2[1][j]);
		//		}

		//		getPaymentDates(spotdate_s, dates_s[i], swap_frequency[i], *swap_calendar[i], *swap_slidingRule[i], *swap_daycount[i], dates, terms_grid, terms_interval, swap_useEOMRollConvention[i]);
		//		DateVector tmpdates(dates);
		//		tmpdates.insert(tmpdates.begin(), spotdate_s);
		//		if(!data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
		//		{
		//			data_swap[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(tmpdates));
		//		}
		//		else
		//		{
		//			dynamic_cast<AQLDataDates&>(data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(tmpdates);
		//		}
		//		if (pDFCurveName && *pDFCurveName != ITSELF)
		//		{
		//			DoubleArray tmp;
		//			// calc floatside date information
		//			getPaymentDates(spotdate_s, dates_s[i], swap_floatLeg_frequency[i], *swap_calendar[i], *swap_slidingRule[i], *swap_floatLeg_daycount[i], 
		//									  dates_float, terms_grid_float, tmp, swap_useEOMRollConvention[i]);
		//		}

		//		size_grid = dates.size();
		//		time_index.push_back(terms_grid.back());
		//		rate_timeVec.push_back(rate_time0);
		//		double val0 = 0.0;
		//		tmp_inter->set(time_index, rate_timeVec);
		//		double df = 1.0;
		//		if (pDFCurveName && *pDFCurveName != ITSELF)
		//		{
		//			for (unsigned int j = 0; j < size_grid; j++)
		//			{
		//				df = df_inter->value(terms_grid[j] + term_spot_s) / df_spot_s;
		//				val0 += (rate_s[i] + tenorSwapSpreadVec[i]) * terms_interval[j] * df;
		//			}
		//			val0 -= calcFloatSide(*tmp_inter, *df_inter, df_spot_s, term_spot_s, terms_grid_float, swap_compoundingTimes[i]);
		//		}
		//		else
		//		{
		//			for (unsigned int j = 0; j < size_grid; j++)
		//			{
		//				df = AQLMath::exp(-tmp_inter->value(terms_grid[j]));
		//				val0 += (rate_s[i] + tenorSwapSpreadVec[i]) * terms_interval[j] * df;
		//			}
		//			val0 += 1.0 *  df;
		//			val0 -= 1.0;
		//		}

		//		rate_time1 = rate_time0 + 1.0e-5;
		//		rate_timeVec.back() = rate_time1;
		//		double val1 = 0.0;
		//		tmp_inter->set(time_index, rate_timeVec);
		//		if (pDFCurveName && *pDFCurveName != ITSELF)
		//		{
		//			for (unsigned int j = 0; j < size_grid; j++)
		//			{
		//				df = df_inter->value(terms_grid[j] + term_spot_s) / df_spot_s;
		//				val1 += (rate_s[i] + tenorSwapSpreadVec[i]) * terms_interval[j] * df;
		//			}
		//			val1 -= calcFloatSide(*tmp_inter, *df_inter, df_spot_s, term_spot_s, terms_grid_float, swap_compoundingTimes[i]);
		//		}
		//		else
		//		{
		//			for (unsigned int j = 0; j < size_grid; j++)
		//			{
		//				df = AQLMath::exp(-tmp_inter->value(terms_grid[j]));
		//				val1 += (rate_s[i] + tenorSwapSpreadVec[i]) * terms_interval[j] * df;
		//			}
		//			val1 += 1.0 *  df;
		//			val1 -= 1.0;
		//		}


		//		int loopNum = MAX_LOOP;
		//		while (loopNum--)
		//		{
		//			if (AQLMath::abs(val0 - val1) < EPS_PV)
		//			{
		//				break;
		//			}
		//			double dval = (val1 - val0) / (rate_time1 - rate_time0);
		//			double rate_time2 = rate_time1 - val1 / dval;
		//			rate_time0 = rate_time1;
		//			rate_time1 = rate_time2;
		//			val0 = val1;
		//			rate_timeVec.back() = rate_time1;
		//			val1 = 0.0;
		//			tmp_inter->set(time_index, rate_timeVec);
		//		
		//			if (pDFCurveName && *pDFCurveName != ITSELF)
		//			{
		//				for (unsigned int j = 0; j < size_grid; j++)
		//				{
		//					df = df_inter->value(terms_grid[j] + term_spot_s) / df_spot_s;
		//					val1 += (rate_s[i] + tenorSwapSpreadVec[i]) * terms_interval[j] * df;
		//				}
		//				val1 -= calcFloatSide(*tmp_inter, *df_inter, df_spot_s, term_spot_s, terms_grid_float, swap_compoundingTimes[i]);
		//			}
		//			else
		//			{
		//				for (unsigned int j = 0; j < size_grid; j++)
		//				{
		//					df = AQLMath::exp(-tmp_inter->value(terms_grid[j]));
		//					val1 += (rate_s[i] + tenorSwapSpreadVec[i]) * terms_interval[j] * df;
		//				}
		//				val1 += 1.0 *  df;
		//				val1 -= 1.0;
		//			}
		//		}
		//		if (loopNum < 0)
		//		{
		//			throw AQLCoreInvalidData("Convergence error in DF calc (Newton Raphson)", __FILE__, __LINE__); 
		//		}

		//		for (unsigned int j = 0; j < size_grid; j++)
		//		{
		//			insertDFData(df_2, df_2_date, AQLMath::exp(-tmp_inter->value(terms_grid[j])), terms_grid[j], dates[j]);
		//		}

		//		df_3[0].push_back(terms_grid.back());
		//		df_3[1].push_back(AQLMath::exp(-rate_time1));
		//		if (i == size_swaps - 1)
		//		{
		//			DoubleArray tau_swap(dates.size());
		//			tau_swap[0] = dc_l.getTerm(spotdate_s, dates[0]);
		//			for (unsigned int i = 1; i < tau_swap.size(); ++i)
		//			{
		//				tau_swap[i] = dc_l.getTerm(dates[i - 1], dates[i]);
		//			}
		//			updateImpliedForwardRates(*tmp_inter, terms_grid, tau_swap, fwd_termsmtx, fwds);
		//		}
		//	}
		//}
		delete tmp_inter;
		delete df_inter;
	}
	else
	{
		throw AQLCoreInvalidData("We support Newton-Raphson method only.", __FILE__, __LINE__); 
	}
	//{
	//	if (isAllSame_s)
	//	{
	//		DoubleArray terms_grid, terms_interval;
	//		DateVector dates;

	//		getPaymentDates(spotdate_s, rate_2_date.back(), swap_frequency[0], *swap_calendar[0], *swap_slidingRule[0], *swap_daycount[0], dates, terms_grid, terms_interval, swap_useEOMRollConvention[0]);
	//		for (unsigned int i = 0; i < size_swaps; i++)
	//		{
	//			// dates[gridpos-1] < dates_s[i] < dates[gridpos]
	//			unsigned int gridpos = 0;
	//			AQLAlgorithm::locate<DateVector, AQLDate>(dates,dates_s[i],dates.size(),gridpos);
	//			if (gridpos > 1 && AQLMath::abs(dates_s[i].intervalDays(dates[gridpos])) > AQLMath::abs( dates_s[i].intervalDays(dates[gridpos -1])))
	//				gridpos -= 1;

	//			DateVector tmpdates;
	//			tmpdates.insert(tmpdates.begin(),dates.begin(),dates.begin()+gridpos+1);
	//			tmpdates.insert(tmpdates.begin(),spotdate_s);
	//			if(!data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
	//			{
	//				data_swap[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(tmpdates));
	//			}
	//			else
	//			{
	//				dynamic_cast<AQLDataDates&>(data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(tmpdates);
	//			}
	//		}

	//		unsigned int size_grid = dates.size();
	//		double sum = 0.0;
	//		unsigned int count = 0;
	//		for (unsigned int i = 0; i < size_grid; i++)
	//		{
	//			double df;
	//			unsigned int pos = 0;
	//			if (AQLAlgorithm::find(df_2_date, dates[i], pos, df_2_date.size() - 1, pos))
	//			{
	//				df = df_2[1][pos];
	//			}
	//			else
	//			{
	//				
	//				double rate;
	//				if (dates[i] <= rate_2_date[0])
	//					rate = rate_2[1][0];
	//				else if (dates[i] >= rate_2_date.back())
	//					rate = rate_2[1].back();
	//				else if (rate_2[0].size() > 2)
	//				{
	//					if (isTimeInter)
	//					{
	//						rate = pInter_yg->value(terms_grid[i]) / terms_grid[i];
	//					}
	//					else
	//					{
	//						rate= pInter_yg->value(terms_grid[i]);
	//					}
	//				}				
	//				else
	//					rate = ((terms_grid[i] - rate_2[0][0]) * rate_2[1][1] + (rate_2[0][1] - terms_grid[i]) * rate_2[1][0]) / (rate_2[0][1] - rate_2[0][0]);//linear interpolation 

	//				df = (1.0 - sum * rate) / (1.0 + rate * terms_interval[i]);
	//			}
	//			sum += df * terms_interval[i];
	//			
	//			insertDFData(df_2, df_2_date, df, terms_grid[i], dates[i]);
	//		}
	//	}
	//	else
	//	{
	//		for (unsigned int i = 0; i < size_swaps; i++)
	//		{
	//			DoubleArray terms_grid, terms_interval;
	//			DateVector dates;
	//			getPaymentDates(spotdate_s, dates_s[i], swap_frequency[i], *swap_calendar[i], *swap_slidingRule[i], *swap_daycount[i], dates, terms_grid, terms_interval, swap_useEOMRollConvention[i]);
	//			DateVector tmpdates(dates);
	//			tmpdates.insert(tmpdates.begin(), spotdate_s);
	//			if(!data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
	//			{
	//				data_swap[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(tmpdates));
	//			}
	//			else
	//			{
	//				dynamic_cast<AQLDataDates&>(data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(tmpdates);
	//			}

	//			unsigned int size_grid = dates.size();
	//			double sum = 0.0;
	//			unsigned int count = 0;
	//			for (unsigned int j = 0; j < size_grid; j++)
	//			{
	//				double df;
	//				unsigned int pos = 0;
	//				if (AQLAlgorithm::find(df_2_date, dates[j], pos, df_2_date.size() - 1, pos))
	//				{
	//					df = df_2[1][pos];
	//				}
	//				else
	//				{
	//					
	//					double rate;
	//					if (dates[j] <= rate_2_date[0])
	//						rate = rate_2[1][0];
	//					else if (dates[j] >= rate_2_date.back())
	//						rate = rate_2[1].back();
	//					else if (rate_2[0].size() > 2)
	//					{
	//						if (isTimeInter)
	//						{
	//							rate = pInter_yg->value(terms_grid[j]) / terms_grid[j];
	//						}
	//						else
	//						{
	//							rate= pInter_yg->value(terms_grid[j]);
	//						}
	//					}			
	//					else
	//						rate = ((terms_grid[j] - rate_2[0][0]) * rate_2[1][1] + (rate_2[0][1] - terms_grid[j]) * rate_2[1][0]) / (rate_2[0][1] - rate_2[0][0]);//linear interpolation 

	//					df = (1.0 - sum * rate) / (1.0 + rate * terms_interval[j]);
	//				}
	//				sum += df * terms_interval[j];
	//				
	//				insertDFData(df_2, df_2_date, df, terms_grid[j], dates[j]);
	//			}
	//		}
	//	}
	//}
	
	//////////////////////////////
	//calculate AsofDate base df//
	//////////////////////////////
	DoubleMatrix df_moneymarket(2);
	DateVector df_moneymarket_date;
	map<pair<AQLDate, AQLDate>, const AQLObject*>::const_iterator it, it_last, it_tmp;
	double term_from_asof = 0.0;
	double df = 1.0;
	double dfSpot = 1.0;
	it_last = data_moneymarket.end();
	it_last--;
	for (it = data_moneymarket.begin(); it != data_moneymarket.end(); it++)
	{
		it_tmp = it;
		it_tmp--;
		if (it != data_moneymarket.begin() && it->first.first != it_tmp->first.second)
		{
			//error
			AQLString msg = "one money market rate enddate is must be startdate of next rate";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		double rate = dynamic_cast<const AQLDataDouble&> ((it->second->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		const AQLPriceDataDayCount& dayCount  = dynamic_cast<const AQLPriceDataDayCount&> ((it->second->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		double term = dayCount.getTerm(it->first.first, it->first.second, false);
		term_from_asof += dc_act365.getTerm(it->first.first, it->first.second);
		double df_tmp = 1.0 / (1.0 + rate * term);					
		df *= df_tmp;
		df_moneymarket[0].push_back(term_from_asof);
		df_moneymarket[1].push_back(df);
		df_moneymarket_date.push_back(it->first.second);

		if (it->first.second < spotdate_s)
		{
			it_tmp = it;
			it_tmp++;
			if (it == it_last)
			{
				df_tmp *= 1.0 / (1.0 + rate * dayCount.getTerm(it->first.second, spotdate_s, false));
				//warning
				fprintf(stderr, "Warning!!! Money Market is not reached to Swap spotdate\n");
			}
			dfSpot *= df_tmp;
		}
		else if (it->first.second > spotdate_s)
		{
			if (it->first.first < spotdate_s)
			{
				dfSpot *= 1.0 / (1.0 + rate * dayCount.getTerm(it->first.first, spotdate_s, false));
			}
		}
		else
		{
			dfSpot *= df_tmp;
		}		
	}
	
	if (df_moneymarket_date.end() != find(df_moneymarket_date.begin(), df_moneymarket_date.end(), df_2_date[0]))
	{
		//erase first element(df=1,t=0,date=spodate of swap)
		df_2[0].erase(df_2[0].begin());
		df_2[1].erase(df_2[1].begin());
		df_2_date.erase(df_2_date.begin());
	}

	const double spotTerm = dc_act365.getTerm(basedate, spotdate_s);
	for (unsigned int i = 0; i < fwd_termsmtx[0].size(); i++)
	{
		fwd_termsmtx[0][i] += spotTerm;
		fwd_termsmtx[1][i] += spotTerm;
	}

	unsigned int size_data2 = df_2[1].size();
	for (unsigned int i = 0; i < size_data2; i++)
	{
		df_2[0][i] += spotTerm;
		df_2[1][i] *= dfSpot;
	}

	if (is_newton && !is_simueq)
	{
		throw AQLCoreInvalidData("We support Simultaneous-Equation method only.", __FILE__, __LINE__); 
	}
	//{
	//	df_3[0].erase(df_3[0].begin());
	//	df_3[1].erase(df_3[1].begin());	
	//	unsigned int size_data3 = df_3[1].size();
	//	for (unsigned int i = 0; i < size_data3; i++)
	//	{
	//		df_3[0][i] += dc_act365.getTerm(basedate, spotdate_s);
	//		df_3[1][i] *= dfSpot;
	//	}
	//	//////////
	//	//output//
	//	//////////
	//	unsigned int pos;
	//	DoubleArray terms_tmp = df_3[0];
	//	DoubleArray dfs_tmp = df_3[1];
	//	size_data = df_moneymarket_date.size();
	//	AQLAlgorithm::locate(df_moneymarket_date, df_2_date[0], size_data, pos);
	//	if (pos == size_data)
	//	{
	//		terms_tmp.insert(terms_tmp.begin(), df_moneymarket[0].begin(), df_moneymarket[0].end());
	//		dfs_tmp.insert(dfs_tmp.begin(), df_moneymarket[1].begin(), df_moneymarket[1].end());
	//	}
	//	else
	//	{
	//		terms_tmp.insert(terms_tmp.begin(), df_moneymarket[0].begin(), df_moneymarket[0].begin() + pos);
	//		dfs_tmp.insert(dfs_tmp.begin(), df_moneymarket[1].begin(), df_moneymarket[1].begin() + pos);
	//		df_2_date.insert(df_2_date.begin(), df_moneymarket_date.begin(), df_moneymarket_date.begin() + pos); 
	//		unsigned int pos2;
	//		for (unsigned int i = pos; i < size_data; i++)
	//		{
	//			AQLAlgorithm::locate(df_2_date, df_moneymarket_date[i], df_2_date.size(), pos2);
	//			if (pos2 == df_2_date.size())
	//			{
	//				terms_tmp.push_back(df_moneymarket[0][i]);
	//				dfs_tmp.push_back(df_moneymarket[1][i]);	
	//				df_2_date.push_back(df_moneymarket_date[i]);
	//			}
	//			else if (df_2_date[pos2] != df_moneymarket_date[i])
	//			{
	//				terms_tmp.insert(terms_tmp.begin() + pos, df_moneymarket[0][i]);
	//				dfs_tmp.insert(dfs_tmp.begin() + pos, df_moneymarket[1][i]);	
	//				df_2_date.insert(df_2_date.begin() + pos, df_moneymarket_date[i]);
	//			}
	//		}
	//	}

	//	terms_tmp.insert(terms_tmp.begin(), 0.0);
	//	dfs_tmp.insert(dfs_tmp.begin(), 1.0);

	//	DoubleArray rate_termVec(terms_tmp.size());
	//	for (unsigned int i = 0; i < dfs_tmp.size(); i++)
	//	{
	//		rate_termVec[i] = -AQLMath::log(dfs_tmp[i]);
	//	}

	//	AQLInterpolationBase *tmp_inter = dynamic_cast<AQLInterpolationBase *>(pInter_yg->clone());
	//	tmp_inter->set(terms_tmp, rate_termVec);

	//	terms = df_2[0];
	//	size_data = df_moneymarket_date.size();
	//	AQLAlgorithm::locate(df_moneymarket_date, df_2_date[0], size_data, pos);
	//	if (pos == size_data)
	//	{
	//		terms.insert(terms.begin(), df_moneymarket[0].begin(), df_moneymarket[0].end());
	//	}
	//	else
	//	{
	//		terms.insert(terms.begin(), df_moneymarket[0].begin(), df_moneymarket[0].begin() + pos);
	//		df_2_date.insert(df_2_date.begin(), df_moneymarket_date.begin(), df_moneymarket_date.begin() + pos); 
	//		unsigned int pos2;
	//		for (unsigned int i = pos; i < size_data; i++)
	//		{
	//			AQLAlgorithm::locate(df_2_date, df_moneymarket_date[i], df_2_date.size(), pos2);
	//			if (pos2 == df_2_date.size())
	//			{
	//				terms.push_back(df_moneymarket[0][i]);
	//				df_2_date.push_back(df_moneymarket_date[i]);
	//			}
	//			else if (df_2_date[pos2] != df_moneymarket_date[i])
	//			{
	//				terms.insert(terms.begin() + pos2, df_moneymarket[0][i]);
	//				df_2_date.insert(df_2_date.begin() + pos2, df_moneymarket_date[i]);
	//			}
	//		}
	//	}
	//	dfs.resize(terms.size());
	//	for (unsigned int i = 0; i < terms.size(); i++)
	//	{
	//		double rate_term = tmp_inter->value(terms[i]);
	//		dfs[i] = AQLMath::exp(-rate_term);
	//	}
	//	delete tmp_inter;
	//}
	else
	{
		//////////
		//output//
		//////////
		terms = df_2[0];
		dfs = df_2[1];

		unsigned int pos;
		size_data = df_moneymarket_date.size();
		AQLAlgorithm::locate(df_moneymarket_date, df_2_date[0], size_data, pos);
		if (pos == size_data)
		{
			terms.insert(terms.begin(), df_moneymarket[0].begin(), df_moneymarket[0].end());
			dfs.insert(dfs.begin(), df_moneymarket[1].begin(), df_moneymarket[1].end());
			df_2_date.insert(df_2_date.begin(), df_moneymarket_date.begin(), df_moneymarket_date.end());
		}
		else
		{
			terms.insert(terms.begin(), df_moneymarket[0].begin(), df_moneymarket[0].begin() + pos);
			dfs.insert(dfs.begin(), df_moneymarket[1].begin(), df_moneymarket[1].begin() + pos);
			df_2_date.insert(df_2_date.begin(), df_moneymarket_date.begin(), df_moneymarket_date.begin() + pos); 
			unsigned int pos2;
			for (unsigned int i = pos; i < size_data; i++)
			{
				AQLAlgorithm::locate(df_2_date, df_moneymarket_date[i], df_2_date.size(), pos2);
				if (pos2 == df_2_date.size())
				{
					terms.push_back(df_moneymarket[0][i]);
					dfs.push_back(df_moneymarket[1][i]);	
					df_2_date.push_back(df_moneymarket_date[i]);
				}
				else if (df_2_date[pos2] != df_moneymarket_date[i])
				{
					terms.insert(terms.begin() + pos2, df_moneymarket[0][i]);
					dfs.insert(dfs.begin() + pos2, df_moneymarket[1][i]);	
					df_2_date.insert(df_2_date.begin() + pos2, df_moneymarket_date[i]);
				}
			}
		}
	}

	//add term =0 and df = 1.0
	if (terms[0] != 0.0)
	{
		terms.insert(terms.begin(), 0.0);
		dfs.insert(dfs.begin(), 1.0);
	}

	// insert extrapolation terms
	dh = &objHolder.getData(IR_CALIBRATION_DATA_MAXTERM);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLString maxTerm = dynamic_cast<const AQLDataString&>(dh->get()).get() + "Y";
		const AQLDate& maxDate = AQLDateCalculations::getDate(spotdate_s, maxTerm, *swap_slidingRule[0], swap_calendar[0], true, &swap_rollConvention[0]);
		if (maxDate > dates_s.back())
		{
			const AQLString maxFreq = dynamic_cast<const AQLDataString&>(objHolder.getData(IR_CALIBRATION_DATA_MAXTERMFREQ).get()).get();
			DoubleArray extra_terms;
			DateVector tmp_dates; DoubleArray tmp_taus;
			getPaymentDates(spotdate_s, maxDate, maxFreq, *swap_calendar[0], *swap_slidingRule[0], *swap_daycount[0], tmp_dates, extra_terms, tmp_taus, swap_useEOMRollConvention[0]);
			for (int i = 0; i < extra_terms.size(); ++i)
			{
				const double term = extra_terms[i] + spotTerm;
				if (term <= terms.back())
					continue;
				terms.push_back(term);
				dfs.push_back(dfs.back());
				if (!fwd_termsmtx[0].empty())
				{
					fwd_termsmtx[0].push_back(fwd_termsmtx[1].back());
					fwd_termsmtx[1].push_back(term);
				}
			}
		}
	}
}


/*
    @brief function to calclation OISDiscountFactor from market data
    
    @param[in] basedate		basedate
	@param[in] data			input market data
	@param[in] objHolder			input object data(get DF from this object)
    @param[out]terms		output terms(this include initial term and market term)
    @param[out]dfs			output DiscountFactors at terms
	@param[in] is_f_use		use future or not
    @param[in] pInter		InterPolation for df
    @param[in] pInter_yg	InterPolation for Swap Rate
	@param[in] pInter_fw	InterPolation for future
	@param[in] pDFCurveName	DF curve name
*/
void 
AQLPriceYieldGenerator::calcOISDiscountFactor(const AQLDate& basedate,
										std::vector<AQLObject*>& data,
										const AQLObjectHolder& objHolder,
                                        DoubleArray& terms,
										DoubleArray& dfs,
										const bool is_f_use,
                                        AQLInterpolationBase* pInter,
										AQLInterpolationBase* pInter_yg,
										AQLInterpolationBase* pInter_fw,
										const AQLString* pDFCurveName) 
{
	 //////////////////////////////////////
	//Classify data object by data type //
	//////////////////////////////////////
	vector<AQLObject*> data_ois_swaps, data_swap, data_on, data_tn;
	unsigned int size_data = data.size();
	AQLString datatype_str;
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	for(unsigned i = 0; i < size_data; i++)
	{
		// check use grid
		const AQLDataHolder *dh = &data[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;

		datatype_str = dynamic_cast<const AQLDataString&> ((data[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		datatype_str.toUpper();
		
		if (datatype_str == BOJ) // TODO: BOJ (Bank of Japan) is the bad keyname for OIS MPC Swaps - Fix This
		{
			data_ois_swaps.push_back(data[i]);  // ois  mpc case = ois monetary policy committee swaps
		}
		else if (datatype_str == FEDFUNDRATE) 
		{
			//const AQLDate& sdate = dynamic_cast<const AQLDataDate&> ((data[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
			//for fed fund first grids
			data_ois_swaps.push_back(data[i]); // ois mpc case = ois monetary policy committee swaps
		}
		else if (datatype_str == PAR || datatype_str == BASIS)
		{
			const AQLString& term = dynamic_cast<const AQLDataString&> ((data[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			if (term == ON) data_on.push_back(data[i]);
			else if (term == TN) data_tn.push_back(data[i]);
			else data_swap.push_back(data[i]);//swap case
		}
	}
	// ascending sort boj, swap data entities  
	sort(data_ois_swaps.begin(), data_ois_swaps.end(), Comp_term());
	sort(data_swap.begin(), data_swap.end(), Comp_term());
	unsigned int size_ois_swaps = data_ois_swaps.size();
	unsigned int size_swaps = data_swap.size();//swap size
	unsigned int size_on = data_on.size();//on size
	unsigned int size_tn = data_tn.size();//tn size
	if (size_swaps == 0)
	{
		throw AQLCoreInvalidData("OIS Swap size must be more than one", __FILE__, __LINE__);
	}
	//////////////////////
	//Spot date /
	//////////////////////
	AQLDate spotdate, spotdate_lobasis;
	bool isFirstRead = true, isFirstReadLOBasis = true;
	size_data = size_ois_swaps + size_swaps;
	for (unsigned int i = 0; i < size_data; ++i)
	{
		const AQLObject *data = 0;
		if (i < size_ois_swaps)
		{
			data = data_ois_swaps[i];
		}
		else
		{
			data = data_swap[i - size_ois_swaps];
		}
		const AQLDate& spotdate_ = dynamic_cast<const AQLDataDate&> ((data->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		const AQLString& data_type = dynamic_cast<const AQLDataString&>((data->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get());
		if (data_type == BASIS)
		{
			if (isFirstReadLOBasis) 
			{
				spotdate_lobasis = spotdate_;
				isFirstReadLOBasis = false;
			}
			else if (spotdate_ != spotdate_lobasis)
			{
				throw AQLCoreInvalidData("All market data must have same spotdate", __FILE__, __LINE__);			
			}
		}
		else
		{
			if (isFirstRead) 
			{
				spotdate = spotdate_;
				isFirstRead = false;
			}
			else if (spotdate_ != spotdate)
			{
				throw AQLCoreInvalidData("All market data must have same spotdate", __FILE__, __LINE__);			
			}
		}
	}

	// calc onforward by boj rate
	map<AQLDate, double> shortterm_onforward;
	//for fed fund
	bool isfedimplied = false;
	for (unsigned int i = 0; i < size_ois_swaps; ++i)
	{
		const AQLDate& startdate = dynamic_cast<const AQLDataDate&> ((data_ois_swaps[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
		const AQLDate& enddate = dynamic_cast<const AQLDataDate&> ((data_ois_swaps[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
		const AQLPriceDataDayCount& dayCount = dynamic_cast<const AQLPriceDataDayCount&> ((data_ois_swaps[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		const AQLPriceDataCalendar& calendar  = dynamic_cast<const AQLPriceDataCalendar&> ((data_ois_swaps[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		const double rate = dynamic_cast<const AQLDataDouble&> ((data_ois_swaps[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		const AQLString& shortTermConv = 
			dynamic_cast<const AQLDataString&> ((data_ois_swaps[i]->getData(IR_CALIBRATION_DATA_SHORTTERMCONVENTION, ISNOTNULL)).get()).get();
		if (shortTermConv == ARITHAVG)
		{
			//for first grid
			if (basedate >= enddate)
			{
				continue;
			}

			if (startdate < basedate)
			{
				// skip a central bank swap rate to the next one to prevent an error caused by no historical rates. (especially the error of EUR realtime theta calculation on the day of the ECB meeting) 
				AQLDataHolder* dh = &(data_ois_swaps[i]->getData(IR_CALIBRATION_DATA_HISTORICALDATES, NOCHECK));
				if (!dh->isDefined() || dh->isNull())
				{
					continue;
				}

				//if exists on rate, then today's ff rate should be the same as on rate.
				double todayffrate = 0.0;
				bool istodayrateexist = false;
				if (data_on.size() > 0)
				{
					todayffrate = dynamic_cast<const AQLDataDouble&> ((data_on[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
					istodayrateexist = true;
				}
				double implyrate = getAverageRateFromHistRates(startdate, enddate, dayCount, calendar, rate, data_ois_swaps[i], basedate, istodayrateexist, todayffrate);
				shortterm_onforward.insert(make_pair(basedate, implyrate));
				isfedimplied = true;
			}
			else
				shortterm_onforward.insert(make_pair(startdate, rate));
		}
		else
		{
			
			const double onforward = solveOISRate(startdate, enddate, dayCount, calendar, rate);	

			shortterm_onforward.insert(make_pair(startdate, onforward));
		}

		DateVector boj_calcdates(2, startdate);
		boj_calcdates[1] = enddate;
		if (!data_ois_swaps[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
		{
			data_ois_swaps[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(boj_calcdates));
		}
		else
		{
			dynamic_cast<AQLDataDates&>(data_ois_swaps[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(boj_calcdates);
		}

	}
	// calc onforward by swap rate
	AQLDate tmp_date = spotdate;
	map<AQLDate, double>::const_iterator it = shortterm_onforward.begin();
	if (size_ois_swaps)
	{
		if (tmp_date < it->first)
		{
			const AQLString& firstRate = dynamic_cast<const AQLDataString&> ((data_ois_swaps[0]->getData(IR_CALIBRATION_DATA_FIRSTRATE, ISNOTNULL)).get()).get();
			
			if (firstRate == SHORTMARKET)
			{
				const double onforward = dynamic_cast<const AQLDataDouble&> ((data_ois_swaps.front()->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				shortterm_onforward.insert(make_pair(tmp_date, onforward));
			}
			else
			{
				const double onforward = dynamic_cast<const AQLDataDouble&> ((data_swap.front()->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				shortterm_onforward.insert(make_pair(tmp_date, onforward));
			}
		}
	}

	DoubleArray grid(1, 0.0);
	DoubleArray yields(1, 0.0);

	const AQLPriceDataDayCount& dayCount = dynamic_cast<const AQLPriceDataDayCount&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	const AQLPriceDataCalendar& calendar  = dynamic_cast<const AQLPriceDataCalendar&> ((data_swap[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule& slidingRule  = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
	RateConvention rc = AQLMathYieldCurve::setRC(SIMPLE);
	AQLPriceDataConvention conv(dayCount.getDayCount(), rc);

	//get shortterm_date
	AQLDate shortterm_date = spotdate;
	const AQLDataHolder *dh = &data_swap[0]->getData(IR_CALIBRATION_DATA_SHORTTERMDATE, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		shortterm_date = dynamic_cast<const AQLDataDate&>(dh->get());
		if (size_ois_swaps)
		{
			const AQLDate &mpc_swap_enddate = dynamic_cast<const AQLDataDate&> ((data_ois_swaps.back()->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
			if (shortterm_date > mpc_swap_enddate)
			{
				shortterm_date = mpc_swap_enddate;
			}
		}
	}
	else
	{
		if (size_ois_swaps)
		{
			shortterm_date = dynamic_cast<const AQLDataDate&> ((data_ois_swaps.back()->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
		}
	}	
	shortterm_date = slidingRule.getDate(shortterm_date, calendar);

	bool eom = false;
	dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISEOMROLLOIS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	AQLString freq = dynamic_cast<const AQLDataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
	freq.toUpper();
	// set roll convention
	AQLString roll_conv("");
	if (freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
	else if (eom) roll_conv = ROLLCONV_EOM;
	else roll_conv = ROLLCONV_NORMAL;

	// get Generate Method and wether we compound all days or just business days
	AQLString GenerateMethod = dynamic_cast<const AQLDataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_GENERATEMETHOD, ISNOTNULL)).get()).get();	
	
	if (size_ois_swaps)
	{
		it = shortterm_onforward.begin();
		tmp_date = spotdate;
		double df = 1.0;
		while (tmp_date < shortterm_date)
		{
			map<AQLDate, double>::const_iterator it_n = it;
			if (it != (--shortterm_onforward.end()) && (++it_n)->first <= tmp_date)
			{
				++it;
			}
			AQLDate n_date = calendar.getBusinessDay(tmp_date, 1);
			df *= conv.getDF(it->second, tmp_date, n_date);
			const double term = dc_act365.getTerm(spotdate, n_date);
			grid.push_back(term);
			yields.push_back(-AQLMath::log(df) / term);
			tmp_date = n_date;
		}
	}
	else
	{
		const double firstRate = dynamic_cast<const AQLDataDouble&> ((data_swap[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		const AQLString& term = dynamic_cast<const AQLDataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get());
		AQLDate date = AQLDateCalculations::getDate(spotdate, term, slidingRule, &calendar, true, &roll_conv);
		// solveOISRate, date is startdatebase
		AQLDate date_m1d = calendar.getBusinessDay(date, -1);

		double tmp;
		if(GenerateMethod == DAILYCOMPOUNDING)
		{
			tmp = solveOISRateS(spotdate, date_m1d, dayCount, calendar, firstRate, shortterm_onforward);
		}
		else if (GenerateMethod == DAILYAVERAGING)
		{
			tmp = solveOISRateSAverage(spotdate, date_m1d, dayCount, calendar, firstRate, shortterm_onforward, true);	
		}
		else
		{
			AQLString msg = "GenerateMethod: " + GenerateMethod + " is not supported.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
		}

		const double onforward = tmp; 

		tmp_date = spotdate;
		double df = 1.0;

		if (spotdate == basedate && size_on)
		{
			// from base date to tomorrow
			const double rate_on = dynamic_cast<const AQLDataDouble&> ((data_on[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			const AQLPriceDataDayCount& dc_on
				= dynamic_cast<const AQLPriceDataDayCount&> ((data_on[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			RateConvention rc_on = AQLMathYieldCurve::setRC(SIMPLE);
			AQLPriceDataConvention conv_on(dc_on.getDayCount(), rc_on);
			tmp_date = calendar.getBusinessDay(basedate, 1);
			df = conv.getDF(rate_on, basedate, tmp_date);
			double term_on = dc_act365.getTerm(basedate, tmp_date);
			yields.push_back(-AQLMath::log(df) / term_on);
			grid.push_back(term_on);

			// from tomorrow to base date + 2
			if (tmp_date < date && size_tn)
			{
				const double rate_tn = dynamic_cast<const AQLDataDouble&> ((data_tn[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				const AQLPriceDataDayCount& dc_tn
					= dynamic_cast<const AQLPriceDataDayCount&> ((data_tn[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
				RateConvention rc_tn = AQLMathYieldCurve::setRC(SIMPLE);
				AQLPriceDataConvention conv_tn(dc_tn.getDayCount(), rc_tn);
				AQLDate date_on = tmp_date;
				tmp_date = calendar.getBusinessDay(date_on, 1);
				df *= conv.getDF(rate_tn, date_on, tmp_date);
				double term_tn = dc_act365.getTerm(basedate, tmp_date);
				yields.push_back(-AQLMath::log(df) / term_tn);
				grid.push_back(term_tn);
			}
		}

		while (tmp_date < date)
		{
			AQLDate n_date = calendar.getBusinessDay(tmp_date, 1);
			df *= conv.getDF(onforward, tmp_date, n_date);
			const double term = dc_act365.getTerm(spotdate, n_date);
			grid.push_back(term);
			yields.push_back(-AQLMath::log(df) / term);
			tmp_date = n_date;
		}
	}

	// calc df by swap rate
	AQLInterpolationBase *pInter_yield;
	if (pInter_yg->getType() == FN_LINEARINTERPOLATION)
	{
		pInter_yield = new AQLLinearInterpolation(LINEAR_EXTRAPOLATION_TYPE);
	}
	else
	{
		pInter_yield = dynamic_cast<AQLInterpolationBase *>(pInter_yg->clone());
	}

	// if interplation method is LinearSpline, set linearSplineJoinDateAsDouble as 0
	if (pInter_yg->isHybrid())
	{
		double linearSplineJoinDateAsDouble = 0.;
		pInter_yield->setJoinDateAsDouble(linearSplineJoinDateAsDouble);
	}

	AQLString suffix = "";
	AQLInterpolationBase *df_inter = NULL;		
	const double spotterm = dc_act365.getTerm(basedate, spotdate);
	double d_spotdf;
	bool is_selfdf = true;
	if (pDFCurveName && *pDFCurveName != ITSELF)
	{
		is_selfdf = false;
		if (*pDFCurveName != STD) suffix = AQLString("_") + *pDFCurveName;
		df_inter = dynamic_cast<AQLInterpolationBase *>(pInter->clone());
		
		if (!objHolder.getData(CALIBRATION_DATA_TERMS + suffix).isDefined())
			throw AQLCoreInvalidData("terms error", __FILE__,__LINE__);
		const DoubleVector &terms_ = dynamic_cast<const AQLDataDoubles&> ((objHolder.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
		const DoubleVector &dfs_ = dynamic_cast<const AQLDataDoubles&> ((objHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
		df_inter->set(terms_, dfs_);
		d_spotdf = df_inter->value(spotterm);
	}

	const double EPS_RATE = 1.0e-10;
	const int MAX_LOOP = 100;

	unsigned int calced_size = 0;
	double _sigmaCF = 0.0;
	double _sigmaT = 0.0;
	double l_CF = 0.0;
	double l_T = 0.0;
	DoubleArray terms_grid, terms_interval;
	DateVector dates;

	AQLString last_data_type;
	AQLString longTermGen;
	bool reset = false;
	unsigned int calced_size_swaps = 0;
	double _sigmaT_s = 0.0;
	double l_T_s = 0.0;
	DoubleArray terms_grid_s, terms_interval_s;
	DateVector dates_s;

	double settlerate = 0.0;
	tmp_date = spotdate;
	for (unsigned int i = 0; i < size_swaps; ++i)
	{
		const AQLString& term_str = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get());

		const AQLString& data_type = dynamic_cast<const AQLDataString&>(data_swap[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get());
		if (data_type == BASIS)
		{
			longTermGen = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_GENERATEMETHOD, ISNOTNULL)).get());
		}
		else
		{
			longTermGen = GenerateMethod;
		}
		if (data_type != last_data_type)
		{
			// if market is changed, reset sums.
			tmp_date = spotdate;
			calced_size = 0;
			_sigmaCF = 0.0;
			_sigmaT = 0.0;
			l_CF = 0.0;
			l_T = 0.0;
			calced_size_swaps = 0;
			_sigmaT_s = 0.0;
			l_T_s = 0.0;
		}
		last_data_type = data_type;

		double marketrate = 0.0;
		const AQLPriceDataDayCount* dayCount;
		const AQLPriceDataCalendar* calendar;
		const AQLPriceDataSlidingRule* slidingRule;
		// load ois market data
		marketrate = dynamic_cast<const AQLDataDouble&> ((data_swap[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		dayCount = &dynamic_cast<const AQLPriceDataDayCount&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		calendar  = &dynamic_cast<const AQLPriceDataCalendar&> ((data_swap[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		slidingRule  = &dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		freq = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		freq.toUpper();
		if (freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
		else if (eom) roll_conv = ROLLCONV_EOM;
		else roll_conv = ROLLCONV_NORMAL;

		AQLDate date = AQLDateCalculations::getDate(spotdate, term_str, *slidingRule, calendar, true, &roll_conv);
		if ((size_ois_swaps && date <= shortterm_date) || (!size_ois_swaps && i == 0) || (tmp_date >= date))
		{
			continue; // use boj rate
		}
		double term = dc_act365.getTerm(spotdate, date);

		terms_grid.clear();
		terms_interval.clear();
		dates.clear();
		getPaymentDates(spotdate, date, freq, *calendar, *slidingRule, *dayCount, dates, terms_grid, terms_interval, eom);

		unsigned int size_cashlet = dates.size();
		if (!size_cashlet)
		{
			throw AQLCoreInvalidData("cashlets are empty.", __FILE__, __LINE__);
		}
		if ((calced_size > 0) && tmp_date != dates[calced_size - 1])
		{
			_sigmaCF -= l_CF;
			_sigmaT -= l_T;
			--calced_size;
			if (calced_size == 0)
			{
				tmp_date = spotdate;
			}
			else
			{
				tmp_date = dates[calced_size - 1];
			}
		}
		unsigned int size_calc = size_cashlet - calced_size;
		if (!size_calc)
		{
			continue;
		}

		// load libor swap data
		double marketrate_s = 0.0;
		unsigned int size_cashlet_s = 0;
		unsigned int size_calc_swaps = 0;
		if (data_type == BASIS)
		{
			const AQLObject* data_swap_irs = &dynamic_cast<const AQLDataReference&>(data_swap[i]->getData(CALIBRATION_DATA_MARKETDATA_EXT, ISNOTNULL).get()).get().get();
			if (data_swap_irs == 0)
			{
				throw AQLCoreInvalidData("External swap market object is empty.", __FILE__, __LINE__);
			}
			marketrate_s = dynamic_cast<const AQLDataDouble&> (data_swap_irs->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();
			const AQLPriceDataDayCount& swap_daycount = dynamic_cast<const AQLPriceDataDayCount&> ((data_swap_irs->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			const AQLPriceDataCalendar& swap_calendar = dynamic_cast<const AQLPriceDataCalendar&> ((data_swap_irs->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const AQLPriceDataSlidingRule& swap_slidingRule = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap_irs->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			AQLString swap_frequency = dynamic_cast<const AQLDataString&> ((data_swap_irs->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
			swap_frequency.toUpper();
			AQLString swap_rollConvention("");
			if (freq == LUNAR) swap_rollConvention = ROLLCONV_LUNAR;
			else if (eom) swap_rollConvention = ROLLCONV_EOM;
			else swap_rollConvention = ROLLCONV_NORMAL;
			// scheduling libor swap
			const AQLDate date_s = AQLDateCalculations::getDate(spotdate, term_str, swap_slidingRule, &swap_calendar, true, &swap_rollConvention);
			terms_grid_s.clear();
			terms_interval_s.clear();
			dates_s.clear();
			getPaymentDates(spotdate, date_s, swap_frequency, swap_calendar, swap_slidingRule, swap_daycount, dates_s, terms_grid_s, terms_interval_s, eom);
			size_cashlet_s = dates_s.size();
			if (!size_cashlet_s)
			{
				throw AQLCoreInvalidData("cashlets are empty.", __FILE__, __LINE__);
			}
			size_calc_swaps = size_cashlet_s - calced_size_swaps;
		}

		grid.push_back(term);
		yields.push_back(data_type == BASIS ? marketrate_s - marketrate : marketrate);
		pInter_yield->set(grid, yields);

		// date info, enddate is next date to startdate
		vector<DoubleVector> startterms(size_calc);
		vector<DoubleVector> endterms(size_calc); 
		vector<DoubleVector> deltas(size_calc);
		vector<DoubleVector> weights(size_calc);
		vector<DoubleVector> onforward_rates(size_calc); //onforward rate
		double sigmaT = 0.0;
		double sigmaCF = 0.0;
		DoubleVector dfs_(size_calc);
		double sigmaT_s = 0.0;
		DoubleVector dfs_s(size_calc_swaps);
		for (unsigned int j = 0; j < size_calc; ++j)
		{
			unsigned int pos = j + calced_size;
			// calc df & annuity
			if (is_selfdf)
			{
				dfs_[j] = AQLMath::exp(-pInter_yield->value(terms_grid[pos]) * terms_grid[pos]);
			}
			else
			{
				dfs_[j] = df_inter->value(terms_grid[pos] + spotterm);
				dfs_[j] /= d_spotdf;
			}
			sigmaT += terms_interval[pos] * dfs_[j];
			// calc settle rate & CF
			const AQLDate startdate = pos == 0 ? spotdate : dates[pos - 1];
			if (longTermGen == DAILYCOMPOUNDING)
			{
				tmp_date = dates[pos];
				const double startterm = dc_act365.getTerm(spotdate, startdate);
				const double endterm = dc_act365.getTerm(spotdate, dates[pos]);
				settlerate = (AQLMath::exp(pInter_yield->value(endterm) * endterm - pInter_yield->value(startterm) * startterm) - 1.0) / terms_interval[pos];
			}
			else if (longTermGen == DAILYAVERAGING)
			{
				tmp_date = startdate;
				while (tmp_date < dates[pos])
				{
					const AQLDate n_date = calendar->getBusinessDay(tmp_date, 1);
					startterms[j].push_back(dc_act365.getTerm(spotdate, tmp_date));
					endterms[j].push_back(dc_act365.getTerm(spotdate, n_date));
					deltas[j].push_back(dayCount->getTerm(tmp_date, n_date));
					weights[j].push_back(static_cast<double>(tmp_date.intervalDays(n_date)));
					tmp_date = n_date;
				}
				settlerate = calcSettleRatesA(startterms[j], endterms[j], pInter_yield, deltas[j], weights[j]);
			}
			else
			{
				AQLString msg(AQLString("OIS curve generate method: ") + GenerateMethod + " is not supported.");
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			sigmaCF += settlerate * terms_interval[pos] * dfs_[j];
		}
		if (data_type == BASIS)
		{
			for (unsigned int j = 0; j < size_calc_swaps; ++j)
			{
				unsigned int pos_s = j + calced_size_swaps;
				// calc df & annuity
				if (is_selfdf)
				{
					dfs_s[j] = AQLMath::exp(-pInter_yield->value(terms_grid_s[pos_s]) * terms_grid_s[pos_s]);
				}
				else
				{
					dfs_s[j] = df_inter->value(terms_grid_s[pos_s] + spotterm);
					dfs_s[j] /= d_spotdf;
				}
				sigmaT_s += terms_interval_s[pos_s] * dfs_s[j];
			}
		}
		sigmaCF += _sigmaCF;
		sigmaT += _sigmaT;
		sigmaT_s += _sigmaT_s;
		
		double calcrate = data_type == BASIS ? (marketrate_s * sigmaT_s - sigmaCF) / sigmaT : sigmaCF / sigmaT;
		double diff = marketrate - calcrate;
		double diff_yield = diff;

		// calc rate to fit market rate, an error of less than EPS
		int loop = MAX_LOOP;
		while (AQLMath::abs(diff) > EPS_RATE && loop-- > 0)
		{
			double calcrate_ori = calcrate;

			sigmaT = 0.0;
			sigmaT_s = 0.0;
			sigmaCF = 0.0;
			// move zero yield
			yields.back() += diff_yield;
			pInter_yield->set(grid, yields);
			for (unsigned int j = 0; j < size_calc; ++j)
			{
				unsigned int pos = j + calced_size;
				// calc df * tau
				if (is_selfdf)
				{
					dfs_[j] = AQLMath::exp(-pInter_yield->value(terms_grid[pos]) * terms_grid[pos]);
				}
				sigmaT += terms_interval[pos] * dfs_[j];
				// calc settle rate & CF
				if (longTermGen == DAILYCOMPOUNDING)
				{
					const AQLDate startdate = pos == 0 ? spotdate : dates[pos - 1];
					const double startterm = dc_act365.getTerm(spotdate, startdate);
					const double endterm = dc_act365.getTerm(spotdate, dates[pos]);
					settlerate = (AQLMath::exp(pInter_yield->value(endterm) * endterm - pInter_yield->value(startterm) * startterm) - 1.0) / terms_interval[pos];
				}
				else if(longTermGen == DAILYAVERAGING)
				{
					settlerate = calcSettleRatesA(startterms[j], endterms[j], pInter_yield, deltas[j], weights[j]);
				}
				else
				{
					AQLString msg(AQLString("OIS curve generate method: ") + GenerateMethod + " is not supported.");
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				sigmaCF += settlerate * terms_interval[pos] * dfs_[j];
			}
			if (data_type == BASIS)
			{
				for (unsigned int j = 0; j < size_calc_swaps; ++j)
				{
					unsigned int pos_s = j + calced_size_swaps;
					// calc df * tau
					if (is_selfdf)
					{
						dfs_s[j] = AQLMath::exp(-pInter_yield->value(terms_grid_s[pos_s]) * terms_grid_s[pos_s]);
					}
					sigmaT_s += terms_interval_s[pos_s] * dfs_s[j];
				}
			}
			sigmaCF += _sigmaCF;
			sigmaT += _sigmaT;
			sigmaT_s += _sigmaT_s;

			calcrate = data_type == BASIS ? (marketrate_s * sigmaT_s - sigmaCF) / sigmaT : sigmaCF / sigmaT;
			diff = marketrate - calcrate;
			if (calcrate == calcrate_ori)
			{
				throw AQLCoreInvalidData("swap rate does not change in newton raphson method!", __FILE__, __LINE__);
			}
			else
			{
				diff_yield *= (marketrate - calcrate) / (calcrate - calcrate_ori);
			}
		}
		if (loop < 0)
		{
			throw AQLCoreInvalidData("OIS curve can't converge in newton raphson method!", __FILE__, __LINE__);
		}
		grid.insert(grid.begin(), terms_grid.begin(), terms_grid.end());
		sort(grid.begin(), grid.end());
		grid.erase(unique(grid.begin(), grid.end()), grid.end());
		vector<double> tmpvals(grid.size(), 0.0);
		for (int j = 0; j < grid.size(); ++j)
		{
			tmpvals[j] = pInter_yield->value(grid[j]);
		}
		yields = tmpvals;

		_sigmaCF = sigmaCF;
		_sigmaT = sigmaT;
		l_CF = settlerate * terms_interval.back() * dfs_.back();
		l_T = terms_interval.back() * dfs_.back();
		calced_size = size_cashlet;

		if (data_type == BASIS)
		{
			_sigmaT_s = sigmaT_s;
			l_T_s = terms_interval_s.back() * dfs_s.back();
			calced_size_swaps = size_cashlet_s;
		}

		DateVector tmpdates(dates);
		tmpdates.insert(tmpdates.begin(), spotdate);
		if (!data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
		{
			data_swap[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(tmpdates));
		}
		else
		{
			dynamic_cast<AQLDataDates&>(data_swap[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(tmpdates);
		}
	}
	delete df_inter;

	if (grid.size() < 3)
	{
		throw AQLCoreInvalidData("Yield data size must be more than 1", __FILE__, __LINE__);
	}

	if (size_ois_swaps)
	{
		int days = spotdate.intervalDays(shortterm_date);
		int add_days = days / 2;
		days += add_days;
		AQLDate d_griddate = spotdate;
		d_griddate.addDays(days);
		const AQLPriceDataSlidingRule& slidingRule  = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		d_griddate = slidingRule.getDate(d_griddate, calendar);
		tmp_date = spotdate;
		DoubleArray add_grid;
		while (tmp_date < d_griddate)
		{
			AQLDate n_date = calendar.getBusinessDay(tmp_date, 1);
			const double term = dc_act365.getTerm(spotdate, n_date);
			add_grid.push_back(term);
			tmp_date = n_date;
		}
		grid.insert(grid.begin(), add_grid.begin(), add_grid.end());
	}

	grid.insert(grid.begin(), terms_grid.begin(), terms_grid.end());
	sort(grid.begin(), grid.end());
	grid.erase(unique(grid.begin(), grid.end()), grid.end());

	if (spotdate > basedate)
	{
		if (size_on)
		{
			// from base date to tomorrow
			const double rate_on = dynamic_cast<const AQLDataDouble&> ((data_on[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			const AQLPriceDataDayCount& dc_on
				= dynamic_cast<const AQLPriceDataDayCount&> ((data_on[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
			RateConvention rc_on = AQLMathYieldCurve::setRC(SIMPLE);
			AQLPriceDataConvention conv_on(dc_on.getDayCount(), rc_on);
			AQLDate date = calendar.getBusinessDay(basedate, 1);
			dfs.push_back(conv.getDF(rate_on, basedate, date));
			double term = dc_act365.getTerm(basedate, date);
			terms.push_back(term);

			// from tomorrow to base date + 2
			if (spotdate > date && size_tn != 0)
			{
				const double rate_tn = dynamic_cast<const AQLDataDouble&> ((data_tn[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				const AQLPriceDataDayCount& dc_tn
					= dynamic_cast<const AQLPriceDataDayCount&> ((data_tn[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
				RateConvention rc_tn = AQLMathYieldCurve::setRC(SIMPLE);
				AQLPriceDataConvention conv_tn(dc_tn.getDayCount(), rc_tn);
				AQLDate date_on = date;
				date = calendar.getBusinessDay(date_on, 1);
				dfs.push_back(dfs.back() * conv.getDF(rate_tn, date_on, date));
				term = dc_act365.getTerm(basedate, date);
				terms.push_back(term);
			}
            
            double shortTermRate;
			if (shortterm_onforward.size() == 0 && size_tn != 0)
			{ 
				const double rate_tn = dynamic_cast<const AQLDataDouble&> ((data_tn[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				shortTermRate = rate_tn; 
			}
            else if (shortterm_onforward.size() == 0)
			{ 
				shortTermRate = rate_on; 
			}
            else
			{ 
				shortTermRate = shortterm_onforward.begin()->second; 
			}

			AQLDate lastDate = date;
			date = calendar.getBusinessDay(date, 1);
			while (date <= spotdate)
			{
				term = dc_act365.getTerm(basedate, date);
				terms.push_back(term);
				dfs.push_back(dfs.back() * conv.getDF(shortTermRate, lastDate, date));	
				lastDate = date;
				date = calendar.getBusinessDay(date, 1);
			}
			
			const double spotdf = dfs.back();
			for (unsigned int i = 1; i < grid.size(); ++i)
			{
				terms.push_back(spotterm + grid[i]);
				dfs.push_back(AQLMath::exp(-pInter_yield->value(grid[i]) * grid[i]) * spotdf);		
			}
		}
		else
		{
			const double spotyield = yields[1];
			// set business date before spotdate
			AQLDate date = calendar.getBusinessDay(basedate, 1);
			while (date < spotdate)
			{
				const double term = dc_act365.getTerm(basedate, date);
				terms.push_back(term);
				dfs.push_back(AQLMath::exp(-spotyield * term));	
				date = calendar.getBusinessDay(date, 1);
			}
			
			terms.push_back(spotterm);
			const double spotdf = AQLMath::exp(-spotyield * spotterm);
			dfs.push_back(spotdf);
			for (unsigned int i = 1; i < grid.size(); ++i)
			{
				terms.push_back(spotterm + grid[i]);
				dfs.push_back(AQLMath::exp(-pInter_yield->value(grid[i]) * grid[i]) * spotdf);		
			}
		}
	}
	else
	{
		for (unsigned int i = 1; i < grid.size(); ++i)
		{
			terms.push_back(grid[i]);
			dfs.push_back(AQLMath::exp(-pInter_yield->value(grid[i]) * grid[i]));	
		}
	}
	delete pInter_yield;

	//add term =0 and df = 1.0
	terms.insert(terms.begin(), 0.0);
	dfs.insert(dfs.begin(), 1.0);

	// insert extrapolation terms
	dh = &objHolder.getData(IR_CALIBRATION_DATA_MAXTERM);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLString maxTerm = dynamic_cast<const AQLDataString&>(dh->get()).get() + "Y";
		const AQLDate& maxDate = AQLDateCalculations::getDate(spotdate, maxTerm, slidingRule, &calendar, true, &roll_conv);
		if (maxDate > dates.back())
		{
			const AQLString maxFreq = dynamic_cast<const AQLDataString&>(objHolder.getData(IR_CALIBRATION_DATA_MAXTERMFREQ).get()).get();
			DoubleArray extra_terms;
			DateVector tmp_dates; DoubleArray tmp_taus;
			getPaymentDates(spotdate, maxDate, maxFreq, calendar, slidingRule, dayCount, tmp_dates, extra_terms, tmp_taus, eom);
			for (int i = 0; i < extra_terms.size(); ++i)
			{
				const double term = extra_terms[i] + spotterm;
				if (term <= terms.back())
					continue;
				terms.push_back(term);
				dfs.push_back(dfs.back());
			}
		}
	}
}

/*
    @brief solve ois rate by newton raphson
    
    @param[in] startdate
	@param[in] enddate
	@param[in] dayCount          daycount
 	@param[in] calendar         calendar
	@param[in] market_rate targetrate
	
	@return oisrate
*/
double 
AQLPriceYieldGenerator::solveOISRateS(const AQLDate& startdate,
								  const AQLDate& enddate,
								  const AQLPriceDataDayCount& dayCount,
								  const AQLPriceDataCalendar& calendar,
								  double market_rate,
								  const map<AQLDate, double> &onforward_map)

{
	if (onforward_map.empty() || onforward_map.begin()->first > enddate)
	{
		return solveOISRate(startdate, enddate, dayCount, calendar, market_rate);
	}
	// calc target market rate
	RateConvention rc = AQLMathYieldCurve::setRC(SIMPLE);
	AQLPriceDataConvention conv(dayCount.getDayCount(), rc);
	double val = 1.0;
	map<AQLDate, double>::const_iterator it = onforward_map.begin();
	AQLDate date = it->first;
	while (date <= enddate)
	{
		map<AQLDate, double>::const_iterator n_it = it;
		++n_it;
		if (date >= n_it->first)
		{
			++it;
		}
		const double onforward_rate = it->second;
		AQLDate n_date = calendar.getBusinessDay(date, 1);
		const double term_d = dayCount.getTerm(date, n_date);
		val *= AQLPriceDataConvention::rateToRet(onforward_rate, term_d, conv);
		date = n_date;
	}
	const double term = dayCount.getTerm(startdate, calendar.getBusinessDay(enddate, 1));
	
	AQLDate enddate_ = onforward_map.begin()->first;
	const double term_s = dayCount.getTerm(startdate, enddate_);
	const double target_rate = ((1.0 + term * market_rate) / val - 1.0) / term_s;

	enddate_ = calendar.getBusinessDay(enddate_, -1);
	return solveOISRate(startdate, enddate_, dayCount, calendar, target_rate);

}


double 
AQLPriceYieldGenerator::solveOISRateSAverage(const AQLDate& startdate,
								  const AQLDate& enddate,
								  const AQLPriceDataDayCount& dayCount,
								  const AQLPriceDataCalendar& calendar,
								  double market_rate,
								  const map<AQLDate, double> &onforward_map,
								  bool compoundAllDays)

{
	if (onforward_map.empty() || onforward_map.begin()->first > enddate)
	{
		return solveOISRateAverage(startdate, enddate, dayCount, calendar, market_rate, compoundAllDays);
	}
	// calc target market rate
	double val = 0.0;
	map<AQLDate, double>::const_iterator it = onforward_map.begin();
	unsigned int  numOfRates = 0;
	AQLDate date = it->first;
	while (date <= enddate)
	{
		map<AQLDate, double>::const_iterator n_it = it;
		++n_it;
		if (date >= n_it->first)
		{
			++it;
		}
		const double onforward_rate = it->second;

		AQLDate tommorowdate = date; 
		AQLDate n_date = calendar.getBusinessDay(date, 1);

		val += onforward_rate;
		tommorowdate.addDays(1);
		numOfRates++;

		if(compoundAllDays)
		{
			while (tommorowdate < n_date)
			{
				val += onforward_rate;
				tommorowdate.addDays(1); 
				numOfRates=numOfRates++;
			}
		}
		date = n_date;
	}
	const double term = dayCount.getTerm(startdate, calendar.getBusinessDay(enddate, 1));
	
	AQLDate enddate_ = onforward_map.begin()->first;
	const double term_s = dayCount.getTerm(startdate, enddate_);
	const double target_rate = ((1.0 + term * market_rate) / (1.0 + val / (double)numOfRates) - 1.0) / term_s;

	enddate_ = calendar.getBusinessDay(enddate_, -1);

	return solveOISRateAverage(startdate, enddate_, dayCount, calendar, target_rate, compoundAllDays);

}


/*
    @brief solve ois rate by newton raphson
    
    @param[in] startdate
	@param[in] enddate
	@param[in] dayCount          daycount
 	@param[in] calendar         calendar
	@param[in] market_rate targetrate
	
	@return oisrate
*/
double 
AQLPriceYieldGenerator::solveOISRate(const AQLDate& startdate,
								const AQLDate& enddate,
								const AQLPriceDataDayCount& dayCount,
								const AQLPriceDataCalendar& calendar,
								double market_rate)

{
	const double EPS_PV = 1.0E-8;
	const int MAX_LOOP = 10000;

	double rate0 = market_rate;
	double val0 = market_rate - calcSettleRate(startdate, enddate, dayCount, calendar, rate0);
	double rate1 = rate0 + 0.01;
	double val1 = market_rate - calcSettleRate(startdate, enddate, dayCount, calendar, rate1);
	// newton raphson
	int loopNum = MAX_LOOP;
	try
	{
		while (loopNum--)
		{
			if (AQLMath::abs(val0 - val1) < EPS_PV)
			{
				break;
			}
			double dval = (val1 - val0) / (rate1 - rate0);
			double rate2 = rate1 - val1 / dval;
			rate0 = rate1;
			rate1 = rate2;
			val0 = val1;
			val1 = market_rate - calcSettleRate(startdate, enddate, dayCount, calendar, rate1);
		}
	}
	catch (AQLCoreError &e)
	{
		AQLString msg = "Convergence error in Rate calc (Newton Raphson). M-Lib error message is below\n";
		msg += e.getMsg();
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	if (loopNum < 0)
	{
		throw AQLCoreInvalidData("Convergence error in Rate calc (Newton Raphson)", __FILE__, __LINE__); 
	}

	return rate1;
}


double 
AQLPriceYieldGenerator::solveOISRateAverage(const AQLDate& startdate,
								const AQLDate& enddate,
								const AQLPriceDataDayCount& dayCount,
								const AQLPriceDataCalendar& calendar,
								double market_rate, bool compoundAllDays)

{
	const double EPS_PV = 1.0E-8;
	const int MAX_LOOP = 10000;

	double rate0 = market_rate;
	double val0 = market_rate - calcSettleRateDailyAverage(startdate, enddate, dayCount, calendar, rate0, compoundAllDays);
	double rate1 = rate0 + 0.01;
	double val1 = market_rate - calcSettleRateDailyAverage(startdate, enddate, dayCount, calendar, rate1, compoundAllDays);
	// newton raphson
	int loopNum = MAX_LOOP;
	try
	{
		while (loopNum--)
		{
			if (AQLMath::abs(val0 - val1) < EPS_PV)
			{
				break;
			}
			double dval = (val1 - val0) / (rate1 - rate0);
			double rate2 = rate1 - val1 / dval;
			rate0 = rate1;
			rate1 = rate2;
			val0 = val1;
			val1 = market_rate - calcSettleRateDailyAverage(startdate, enddate, dayCount, calendar, rate1, compoundAllDays);

		}
	}
	catch (AQLCoreError &e)
	{
		AQLString msg = "Convergence error in Rate calc (Newton Raphson). M-Lib error message is below\n";
		msg += e.getMsg();
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	if (loopNum < 0)
	{
		throw AQLCoreInvalidData("Convergence error in Rate calc (Newton Raphson)", __FILE__, __LINE__); 
	}

	return rate1;
}

/*
    @brief calc settle rate
    
    @param[in] startdate of clculated settle rate
	@param[in] enddate of clculated settle rate
	@param[in] dayCount          daycount
 	@param[in] calendar         calendar
	@param[in] onforwardrate
	
	@return settlerate
*/
double 
AQLPriceYieldGenerator::calcSettleRate(const AQLDate& startdate,
								const AQLDate& enddate,
								const AQLPriceDataDayCount& dayCount,
								const AQLPriceDataCalendar& calendar,
								double onforward_rate)

{
	RateConvention rc = AQLMathYieldCurve::setRC(SIMPLE);
	AQLPriceDataConvention conv(dayCount.getDayCount(), rc);
	AQLDate date = startdate;
	AQLPriceDataSlidingRule slidingRule(SLIDING_RULE_FOLLOWING);
	date = slidingRule.getDate(date, calendar);
	AQLDate adj_startdate = date;
	double val = 1.0;
	AQLDate n_date = date;
	while (date <= enddate)
	{
		n_date = calendar.getBusinessDay(date, 1);
		const double term = dayCount.getTerm(date, n_date);
		val *= AQLPriceDataConvention::rateToRet(onforward_rate, term, conv);
		date = n_date;
	}
	val -= 1.0;
	return val / dayCount.getTerm(adj_startdate, n_date);
}


/*
    @brief calc settle rate
    
    @param[in] startdate of clculated settle rate
	@param[in] enddate of clculated settle rate
	@param[in] dayCount          daycount
 	@param[in] calendar         calendar
	@param[in] onforwardrate
	
	@return settlerate
*/
double 
AQLPriceYieldGenerator::calcSettleRateDailyAverage(const AQLDate& startdate,
								const AQLDate& enddate,
								const AQLPriceDataDayCount& dayCount,
								const AQLPriceDataCalendar& calendar,
								double onforward_rate, 
								bool compoundAllDays)

{
	AQLDate date = startdate;
	AQLPriceDataSlidingRule slidingRule(SLIDING_RULE_FOLLOWING);
	date = slidingRule.getDate(date, calendar);
	double val = 0.;
	unsigned int numOfRates = 0;
	AQLDate n_date = date;
	AQLDate tommorowdate = date; 
	while (date <= enddate)
	{
		tommorowdate.addDays(1);
		n_date = calendar.getBusinessDay(date, 1);

		val += onforward_rate;
		tommorowdate.addDays(1); 
		numOfRates++;

		if(compoundAllDays)
		{
			while (tommorowdate < n_date)
			{
				val += onforward_rate;
				tommorowdate.addDays(1); 
				numOfRates++;
			}
		}

		date = n_date;
	}

	return val / (double)numOfRates;
}



/*
    @brief calc settle rate
    
    @param[in] startdates
	@param[in] enddates
	@param[in] onforward_rates
	@param[in] dayCount          daycount
 	@param[in] calendar         calendar
	
	@return settlerate
*/
double 
AQLPriceYieldGenerator::calcSettleRates(const DateVector& startdates,
								const DateVector& enddates,
								const DoubleVector& onforward_rates,
								const AQLPriceDataDayCount& dayCount)

{
	unsigned int size = startdates.size();
	if (!size)
	{
		return 0.0;
	}
	if (size != enddates.size() || size != onforward_rates.size())
	{
		throw AQLCoreInvalidData("data size is not consistent.", __FILE__, __LINE__);
	}
	RateConvention rc = AQLMathYieldCurve::setRC(SIMPLE);
	AQLPriceDataConvention conv(dayCount.getDayCount(), rc);
	double val = 1.0;
	for (unsigned int i = 0; i < size; ++i)
	{
		const double term = dayCount.getTerm(startdates[i], enddates[i]);
		val *= AQLPriceDataConvention::rateToRet(onforward_rates[i], term, conv);
	}
	val -= 1.0;
	return val / dayCount.getTerm(startdates[0], enddates.back());
}

/*
    @brief calc settle rate by arithmetic average
    
    @param[in] startterms
	@param[in] endterms
	@param[in] pInter inter of onfwd_rates
	@param[in] accrual terms of overnight rates
	
	@return settlerate
*/
double 
AQLPriceYieldGenerator::calcSettleRatesA(const DoubleVector& startterms,
								const DoubleVector& endterms,
								const AQLInterpolationBase* pInter,
								const DoubleVector& onrateaccrualterms,
								const DoubleVector& weights)
{
	unsigned int size = startterms.size();
	//for debug
	std::vector<double> onratestore(size, 0.);
	if (!size)
	{
		return 0.0;
	}
	if (size != endterms.size() || size != onrateaccrualterms.size() || size != weights.size())
	{
		throw AQLCoreInvalidData("data size is not consistent.", __FILE__, __LINE__);
	}
	double val = 0.0;
	double sumweights = 0.0;
	for (unsigned int i = 0; i < size; ++i)
	{
		double onrate = AQLMath::exp(pInter->value(endterms[i]) * endterms[i] - pInter->value(startterms[i]) * startterms[i]) - 1.0;
		onrate /= onrateaccrualterms[i];
		onratestore[i] = onrate;
		val += weights[i] * onrate;
		sumweights += weights[i];
	}
	return val / sumweights;
}

/*

    @brief calculates the settle rate through averaging the Daily OIS
    
    @param[in] startdates of onforward_rates
	@param[in] enddates of onforward_rates
	@param[in] onforward_rates
	@param[in] dayCount          daycount
 	@param[in] calendar         calendar
 	@param[in] compoundAllDays  compoundAllDays or only business days	
	@return settlerate
*/

double 
AQLPriceYieldGenerator::calcSettleRateDailyAverage(const DateVector& startdates,
								const DateVector& enddates,
								const DoubleVector& onforward_rates,
								const AQLPriceDataDayCount& dayCount,
								const AQLPriceDataCalendar& calendar,
								bool compoundAllDays)

{
	unsigned int size = startdates.size();
	if (!size)
	{
		return 0.0;
	}
	if (size != enddates.size() || size != onforward_rates.size())
	{
		throw AQLCoreInvalidData("data size is not consistent.", __FILE__, __LINE__);
	}
	double val = 0.0;
	unsigned int numOfRates = 0;
	AQLDate tommorowdate;
	for (unsigned int i = 0; i < size; ++i)
	{
		tommorowdate = startdates[i];
		val += onforward_rates[i];
		tommorowdate.addDays(1); 
		numOfRates++;

		if(compoundAllDays)
		{
			while (tommorowdate < enddates[i])
			{
				val += onforward_rates[i];
				tommorowdate.addDays(1); 
				numOfRates++;
			}
		}

	}

	return val / (double)numOfRates;
}



/*
    @brief change FRA term from "X" format to "M" format
    
    @param[in] term in "X" format  
	
	@return term in "M" format  
*/
AQLString
AQLPriceYieldGenerator::
changeFRATermFormat(const AQLString& inputTerm)
{
	int where = -1;
	where = inputTerm.findString("X");
	if (where == -1) throw AQLCoreInvalidData("x does not exist in FRA Term.", __FILE__, __LINE__);
	return inputTerm.subString(0, where - 1) + AQLString("M");
}

/*
    @brief insert df, term and date into df data
    
    @param[out] dfs
	@param[out] dates
	@param[in] df_in
	@param[in] term_in
	@param[in] date_in
	
	@return settlerate
*/
void
AQLPriceYieldGenerator::
insertDFData(DoubleMatrix& dfs, DateVector& dates, double df_insert, double term_insert, const AQLDate date_insert)
{
	if (dfs.size() < 2) throw AQLCoreInvalidData("Size of DF Data must be 2!", __FILE__, __LINE__);
	if (dfs[0].size() != dates.size()) throw AQLCoreInvalidData("Size of DF data and Size of dates must be same!", __FILE__, __LINE__);

	if (dates.back() < date_insert)
	{
		dates.push_back(date_insert);
		dfs[0].push_back(term_insert);
		dfs[1].push_back(df_insert);
	}
	else
	{
		for (size_t pos = 0; pos < dates.size(); pos++)
		{
			if (dates[pos] == date_insert)
			{
				pos++;
				break;
			}
			else if (dates[pos] > date_insert)
			{
				dates.insert(dates.begin() + pos, date_insert);
				dfs[0].insert(dfs[0].begin() + pos, term_insert);
				dfs[1].insert(dfs[1].begin() + pos, df_insert);
				pos++;
				break;
			}
		}
	}
}

/*
    @brief calc float side value
    
    @param[in] inter : inter of index
	@param[in] df_inter : inter of DF
	@param[in] df_spot : spotdate DF
	@param[in] term_spot : spotdate term
	@param[in] terms_grid : grid
	@param[in] swap_compoundingTimes : compounding times

	@return floatside value
*/
double
AQLPriceYieldGenerator::
calcFloatSide(AQLInterpolationBase &inter, AQLInterpolationBase &df_inter, 
			  const double df_spot, const double term_spot, const DoubleArray &terms_grid, const int swap_compoundingTimes)
{
	double ret = 0.0;
	if (terms_grid.empty())
	{
		return ret;
	}

	double df = 1.0;
	double df_index = 1.0;
	double df_index_b = df_index;
	unsigned int l = 1;
	for (l = 1; l * swap_compoundingTimes - 1 < terms_grid.size(); l++) // compound grid
	{
		// calc compouding
		double cpd = 1.0;
		for (unsigned int j = (l - 1) * swap_compoundingTimes; j < l * swap_compoundingTimes; j++)
		{
			df_index =  AQLMath::exp(-inter.value(terms_grid[j]));
			const double df_ratio = df_index_b / df_index;
			// calc (1+tau*F)^n
			cpd *= df_ratio;
			df_index_b = df_index;
		}
		df = df_inter.value(terms_grid[l * swap_compoundingTimes - 1] + term_spot) / df_spot;
		ret += (cpd - 1.0) * df;
	}
	if (terms_grid.size() % swap_compoundingTimes != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int j = (l - 1) * swap_compoundingTimes; j < terms_grid.size(); j++)   			
		{
			df_index =  AQLMath::exp(-inter.value(terms_grid[j]));
			const double df_ratio = df_index_b / df_index;
			df = df_inter.value(terms_grid[j] + term_spot) / df_spot;
			// calc L*tau*df
			ret += (df_ratio - 1.0) * df;
			df_index_b = df_index;
		}	
	}		
	return ret;
}

/*
    @brief calc forwardrates for swap grid
    
	@param[in] inter
	@param[in] grid_swap
	@param[in] tau_swap
	@param[out] fwd_termsmtx
	@param[out] fwds

*/
void
AQLPriceYieldGenerator::
updateImpliedForwardRates(const AQLInterpolationBase &inter, const DoubleArray &grid_swap, const DoubleArray &tau_swap, DoubleMatrix &fwd_termsmtx, DoubleArray &fwds)
{

	if (fwd_termsmtx.size() != 2)
	{
		throw AQLCoreInvalidData("fwd_termsmtx size must be 2", __FILE__, __LINE__);
	}
	if (grid_swap.empty())
	{
		throw AQLCoreInvalidData("fwd_grid is empty", __FILE__, __LINE__);
	}
	double l_df = AQLMath::exp(-inter.value(grid_swap[0])); 
	DoubleArray fwds_swap(grid_swap.size() - 1);
	for (unsigned int i = 0; i < grid_swap.size() - 1; ++i)
	{
		const double df = AQLMath::exp(-inter.value(grid_swap[i + 1]));
		fwds_swap[i] = (l_df - df) / (tau_swap[i + 1] * df);
		l_df = df;
	}
	unsigned int pos = 0;
	for (unsigned int i = 0; i < grid_swap.size() - 1; ++i)
	{
		AQLAlgorithm::locate<DoubleArray, double>(fwd_termsmtx[0], grid_swap[i], fwd_termsmtx[0].size(), pos);
		if (pos == fwd_termsmtx[0].size())
		{
			fwd_termsmtx[0].push_back(grid_swap[i]);
			fwd_termsmtx[1].push_back(grid_swap[i + 1]);
			fwds.push_back(fwds_swap[i]);
		}
		else
		{
			if (fwd_termsmtx[0][pos] != grid_swap[i])
			{
				fwd_termsmtx[0].insert(fwd_termsmtx[0].begin() + pos, grid_swap[i]);
				fwd_termsmtx[1].insert(fwd_termsmtx[1].begin() + pos, grid_swap[i + 1]);
				fwds.insert(fwds.begin() + pos, fwds_swap[i]);
			}
			else
			{
				fwd_termsmtx[0][pos] = grid_swap[i];
				fwd_termsmtx[1][pos] = grid_swap[i + 1];
				fwds[pos] = fwds_swap[i];
			}
		}
	}
}


/*
    @brief solve ois rate by newton raphson
    
    @param[in] startdate
	@param[in] enddate
	@param[in] dayCount          daycount
 	@param[in] calendar         calendar
	@param[in] market_rate targetrate
	
	@return oisrate
*/
double 
AQLPriceYieldGenerator::getAverageRateFromHistRates(const AQLDate& startdate,
												const AQLDate& enddate,
												const AQLPriceDataDayCount& dayCount,
												const AQLPriceDataCalendar& calendar,
												double market_rate,
												AQLObject* data,
												const AQLDate& basedate,
												bool istodayrateexist,
												double todayffrate)
{
	AQLDataHolder* dh;
	dh = &(data->getData(IR_CALIBRATION_DATA_HISTORICALDATES, ISNOTNULL));
	DateVector histdates = dynamic_cast<const AQLDataDates &>(dh->get()).get();

	dh = &(data->getData(IR_CALIBRATION_DATA_HISTORICALRATES, ISNOTNULL));
	DoubleVector histrates = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();

	if (histdates.size() != histrates.size())
		throw AQLCoreInvalidData("histrocial rate size error",__FILE__,__LINE__);

	//sort histrical datas
	map<AQLDate, double> map_date_rate;
	for (size_t i = 0; i < histdates.size(); ++i)
	{
		if (map_date_rate.find(histdates[i]) != map_date_rate.end())
			throw AQLCoreInvalidData("same dates are input in the historical ON rates",__FILE__,__LINE__);
		map_date_rate[histdates[i]] = histrates[i];
	}
	sort(histdates.begin(), histdates.end());
	histrates.clear();
	for (size_t i = 0; i < histdates.size(); ++i)
	{
		histrates.push_back(map_date_rate[histdates[i]]);
	}

	AQLDate tmpdate = startdate;
	double val = 0.0;
	unsigned int pos = 0;
	int realizednumber = startdate.intervalDays(basedate) +1;
	for (int i = 0; i < realizednumber; i++)
	{
		if (map_date_rate.find(tmpdate) != map_date_rate.end())
		{
			val += map_date_rate[tmpdate];
		}
		else
		{
			AQLAlgorithm::locate<DateVector, AQLDate>(histdates, tmpdate, histdates.size(), pos);
			if (tmpdate == basedate && istodayrateexist)
				val += todayffrate;
			else if (pos == histdates.size())
				val += histrates.back();
			else if (pos == 0)
				val += histrates[pos];
			else 
				val += histrates[pos-1];
		}

		tmpdate.addDays(1);
	}

	int averagenum = startdate.intervalDays(enddate) + 1;

	if (enddate <= basedate)
		throw AQLCoreInvalidData("average rate exception error",__FILE__,__LINE__);


	double implyrate = (static_cast<double>(averagenum) * market_rate - val) / static_cast<double>(averagenum - realizednumber);
	return implyrate;
}

/*
    @brief set a rate convention into a curve data object
    
    @param[in] objHolder	object holder
	@param[in] mktData	market object
	@param[in] curveName          
*/
void
AQLPriceYieldGenerator::setCurveConvention(AQLObjectHolder& objHolder,
									   std::vector<AQLObject*>& mktData,
									   const AQLString& curveName)
{
	const AQLDataHolder *dh;

	AQLString suffix = "";
	if (curveName != STD) suffix = "_" + curveName;

	vector<AQLObject*> mktData_swap, mktData_libor;
	for(unsigned i = 0; i < mktData.size(); i++)
	{
		// check use grid
		dh = &mktData[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;

		AQLString datatype_str = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		datatype_str.toUpper();
		
		if (datatype_str == PAR) mktData_swap.push_back(mktData[i]);//swap case
		else if (datatype_str == ZERO) mktData_libor.push_back(mktData[i]);//libor case
	}
	
	if (mktData_swap.size() == 0)
	{
		throw AQLCoreInvalidData("Swap size must be more than one", __FILE__, __LINE__);
	}

	AQLString freq = "";
	dh = &mktData_swap[0]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		freq = dynamic_cast<const AQLDataString&> (dh->get());
	}
	else
	{
		const AQLDataHolder *dh = &mktData_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			freq = dynamic_cast<const AQLDataString&> (dh->get());
		}
		else
		{
			freq = dynamic_cast<const AQLDataString&> ((mktData_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		}
	}
	
	const AQLPriceDataCalendar* calendar = NULL;
	if (mktData_libor.size() != 0)
	{
		calendar = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData_libor[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	}
	
	const AQLPriceDataSlidingRule* slidingRule = NULL;
	if (mktData_libor.size() != 0)
	{
		slidingRule = &dynamic_cast<const AQLPriceDataSlidingRule&> ((mktData_libor[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
	}
	
	const AQLPriceDataDayCount* dayCount = NULL;
	dh = &mktData_swap[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		dayCount = &dynamic_cast<const AQLPriceDataDayCount&> (dh->get());
	}
	else
	{
		dayCount = &dynamic_cast<const AQLPriceDataDayCount&> ((mktData_swap[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	}

	AQLString accessary = "";
	if (freq == ANNUAL) accessary = "12M";
	else if (freq == SEMI_ANNUAL) accessary = "6M";
	else if (freq == QUARTERLY) accessary = "3M";
	else if (freq == MONTHLY || LUNAR) accessary = "1M";
	else throw AQLCoreInvalidData("AQLPriceYieldGenerator failed. Check frequency!!",__FILE__,__LINE__);

	objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
	objHolder.remove(CALIBRATION_DATA_CALENDAR + suffix);
	objHolder.remove(CALIBRATION_DATA_SLIDINGRULE + suffix);
	objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
	objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);
	
	objHolder.add(IR_CALIBRATION_DATA_FREQUENCY + suffix, new AQLDataString(SIMPLE));
	if (calendar) objHolder.add(CALIBRATION_DATA_CALENDAR + suffix, new AQLPriceDataCalendar(*calendar));
	if (slidingRule) objHolder.add(CALIBRATION_DATA_SLIDINGRULE + suffix, new AQLPriceDataSlidingRule(*slidingRule));
	objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT + suffix, new AQLPriceDataDayCount(*dayCount));
	objHolder.add(IR_CALIBRATION_DATA_ACCESSARY + suffix, new AQLDataString(accessary));
}

/*
    @brief set additional calibration grid for tenor basis to its instance
    
    @param[in] mr_mktdata    references of IRS market datas 
	@param[in] mr_mktdata_ts    references of tenor basis market datas

	@param[out] addtionalCalibGrid    instance of additional calibration grid of tenor basis    
*/
void
AQLPriceYieldGenerator::setAddtionalCalibGridForTenorBasis(const AQLDataMultiReference& mr_mktdata,
		                                               const AQLDataMultiReference& mr_mktdata_ts,
													   AQLString& addtionalCalibGrid)
{
	addtionalCalibGrid = "";
	for (unsigned int i = 0; i < mr_mktdata.getSize(); ++i)
	{
		const AQLString& type(dynamic_cast<const AQLDataString& >(mr_mktdata.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()));
		if (type != PAR) continue;

		const AQLString term(dynamic_cast<const AQLDataString& >(mr_mktdata.get(i).getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()));
		const bool usegridflag(dynamic_cast<const AQLDataBool& >(mr_mktdata.get(i).getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, ISNOTNULL).get()));
		if (!usegridflag) continue;

		unsigned int j = 0;
		for (j = 0; j < mr_mktdata_ts.getSize(); ++j)
		{
			const AQLString term_tenorswap(dynamic_cast<const AQLDataString& >(mr_mktdata_ts.get(j).getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()));
			const bool usegridflag_tenorswap(dynamic_cast<const AQLDataBool& >(mr_mktdata_ts.get(j).getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, ISNOTNULL).get()));
			if (usegridflag_tenorswap && term == term_tenorswap)
				break;						
		}
		if (j == mr_mktdata_ts.getSize())
			addtionalCalibGrid = addtionalCalibGrid + term + DATA_COLL_DEL;
	}
	if (addtionalCalibGrid.size() > 1)
		addtionalCalibGrid = addtionalCalibGrid.subString(0, addtionalCalibGrid.size() - 2);

}



/*
    @brief Function to insert a ***SINGLE*** additional forward rate into an existing forward rate array in a sorted manner. Used for insertion of tension data points

    @param[out] fwd_termsmtx
	@param[out] fwds

	@param[in] insertStartTerm
	@param[in] insertEndTerm
	@param[in] insertFwdRate

*/
void
AQLPriceYieldGenerator::
insertForwardRateData( DoubleMatrix &fwd_termsmtx, DoubleArray &fwds, const double &insertStartTerm, const double &insertEndTerm, const double &insertFwdRate )
{
    // Data Validation
    if ( fwd_termsmtx.size() != 2)
        throw AQLCoreInvalidData("#Error: Unable to update forward rates, fwd_termsmtx size must be 2", __FILE__, __LINE__);

    // Sort and Insert Forward Data into "fwd_termsmtx" and "fwds" containers
    unsigned int pos = 0;
	AQLAlgorithm::locate<DoubleArray, double>( fwd_termsmtx[0], insertStartTerm, fwd_termsmtx[0].size(), pos );
		
    if (pos == fwd_termsmtx[0].size())
	{
		fwd_termsmtx[0].push_back( insertStartTerm );
		fwd_termsmtx[1].push_back( insertEndTerm );
		fwds.push_back( insertFwdRate );
	}
	else
	{
		if ( fwd_termsmtx[0][pos] != insertStartTerm )
		{
			fwd_termsmtx[0].insert( fwd_termsmtx[0].begin() + pos, insertStartTerm );
			fwd_termsmtx[1].insert( fwd_termsmtx[1].begin() + pos, insertEndTerm );
			fwds.insert( fwds.begin() + pos, insertFwdRate );
		}
		else
		{
			fwd_termsmtx[0][pos] = insertStartTerm;
			fwd_termsmtx[1][pos] = insertEndTerm;
			fwds[pos] = insertFwdRate;
		}
	}
}


/*
    @brief Function to insert ***MULTIPLE*** additional forward rates into an existing forward rate array in a sorted manner. Used for insertion of tension data points
    
	@param[out] fwd_termsmtx
	@param[out] fwds

    @param[in] insertStartTerms
	@param[in] insertEndTerms
	@param[in] insertFwdRates

*/
void
AQLPriceYieldGenerator::
insertForwardRatesData( DoubleMatrix &fwd_termsmtx, DoubleArray &fwds, const DoubleArray &insesrtStartTerms, const DoubleArray &insertEndTerms, const DoubleArray &insertFwdRates )
{
    // Data Validation
    if ( fwd_termsmtx.size() != 2)
        throw AQLCoreInvalidData("#Error: Unable to update forward rates, fwd_termsmtx size must be 2", __FILE__, __LINE__);

	if ( insesrtStartTerms.empty() || insertEndTerms.empty() || insertFwdRates.empty() )
        throw AQLCoreInvalidData("#Error: Unable to update forward rates, startTerms, endTerms and fwdRates data cannot be empty", __FILE__, __LINE__);
	
    if ( insesrtStartTerms.size() != insertEndTerms.size() || insesrtStartTerms.size() != insertFwdRates.size() )
        throw AQLCoreInvalidData("#Error: Unable to update forward rates, inconsistent startTerms, endTerms and fwdRates data", __FILE__, __LINE__);

    // Sort and Insert Forward Data into "fwd_termsmtx" and "fwds" containers
    unsigned int pos = 0;
	for (unsigned int i = 0; i < insertFwdRates.size() - 1; ++i)
	{

        AQLAlgorithm::locate<DoubleArray, double>( fwd_termsmtx[0], insesrtStartTerms[i], fwd_termsmtx[0].size(), pos );
		
        if (pos == fwd_termsmtx[0].size())
		{
			fwd_termsmtx[0].push_back( insesrtStartTerms[i] );
			fwd_termsmtx[1].push_back( insertEndTerms[i] );
			fwds.push_back( insertFwdRates[i] );
		}
		else
		{
			if ( fwd_termsmtx[0][pos] != insesrtStartTerms[i] )
			{
				fwd_termsmtx[0].insert( fwd_termsmtx[0].begin() + pos, insesrtStartTerms[i] );
				fwd_termsmtx[1].insert( fwd_termsmtx[1].begin() + pos, insertEndTerms[i] );
				fwds.insert( fwds.begin() + pos, insertFwdRates[i] );
			}
			else
			{
				fwd_termsmtx[0][pos] = insesrtStartTerms[i];
				fwd_termsmtx[1][pos] = insertEndTerms[i];
				fwds[pos] = insertFwdRates[i];
			}
		}
	}
}


/*
    @brief Function to insert synthetic discount factors and forward data points to simulate and apply tension
    
    @param[out]     discountFactorDatesVector
    @param[out]     discountFactorMatrix
    @param[out]     forwardsRatesVector
    @param[out]     forwardTermsMatrix 
    @param[in]      discountFactorInterpolationTable
    @param[in]      spotDate
    @param[in]      startDate
    @param[in]      endDate
    @param[in]      forwardRate
    @param[in]      isFirstDataPoint
    @param[in]      termsToDateDaycount
    @param[in]      instrumentDaycount
    @param[in]      tensionGap
    @param[in]      instrumentData  ( tensionMarketData struct used here )
    @param[in]      cutoffDate      ( This is the start of the next instrument group and prevents data being inserted into ...
                                       ... the next / adjacent instrument group e.g futures being mixed with swaps data )
    
    // Note: The following struct is declared in the header file for use with this function.
    // This struct is needed for the insertSyntheticTensionPoints function to carry the tension iteration data
    
    struct tensionMarketData
    {
        double           frontForwardRate;
        AQLDate           frontStartDate;
        AQLDate           frontEndDate;
        double           frontStartTerm;
        double           frontEndTerm;
        double           backForwardRate;
        AQLDate           backStartDate;
        AQLDate           backEndDate;
        double           backStartTerm;
        double           backEndTerm;
    };
*/
void
AQLPriceYieldGenerator::
insertSyntheticTensionPoints(           DateVector &            discountFactorDatesVector,          // [Output]
                                        DoubleMatrix &          discountFactorMatrix,               // [Output]
                                        DoubleArray &           forwardRatesVector,                 // [Output]
                                        DoubleMatrix &          forwardTermsMatrix,                 // [Output]
                                        AQLInterpolationBase *   discountFactorInterpolationTable,   // [Input]
                                const   AQLDate &                spotDate,                           // [Input]
                                const   AQLDate &                startDate,                          // [Input]
                                const   AQLDate &                endDate,                            // [Input]
                                const   double &                forwardRate,                        // [Input]
                                const   bool &                  isFirstDataPoint,                   // [Input]              ( The first point collects data, no interpolation is performed until we have at least 2 points )
                                const   AQLPriceDataDayCount &        termsToDateDaycount,                // [Input]              ( Daycount used to convert terms or year fractions to dates )
                                const   AQLPriceDataDayCount &        instrumentDaycount,                 // [Input]              ( Instrument Daycount )
                                const   unsigned int &          tensionGap,                         // [Input]
                                        tensionMarketData &     instrumentData,                     // [Input]              ( tensionMarketData struct used here )
                                const   AQLDate &                cutoffDate )                        // [Input] OPTIONAL     ( Next instrument start date. Used to prevent futures / fra's being mixed with swaps )
                                        
{
    // Collect Data for front and end Forwards in first iteration of the Forwards loop
    if ( !isFirstDataPoint )
    {
        // Update Back Forward rate and Corresponding Start and End-Dates
        instrumentData.backForwardRate      = forwardRate;
        instrumentData.backStartDate        = startDate;
	    instrumentData.backEndDate          = endDate;
        instrumentData.backStartTerm        = termsToDateDaycount.getTerm( spotDate, startDate );
        instrumentData.backEndTerm          = termsToDateDaycount.getTerm( spotDate, endDate );
        
        // Generate Synthetic Start- and End-Date
        AQLDate syntheticStartDate   = instrumentData.frontStartDate;
        syntheticStartDate.addDays( tensionGap );
                    
        AQLDate syntheticEndDate = instrumentData.frontEndDate;
        syntheticEndDate.addDays( tensionGap );
        
        RateConvention rateConvention = AQLMathYieldCurve::setRC( SIMPLE );
		AQLPriceDataConvention discountConvention( instrumentDaycount.getDayCount(), rateConvention );

        // Linear Interpolate on Forward Rates
        const double linearInterpIntercept = instrumentData.frontForwardRate;
        const double dRate = instrumentData.backForwardRate - instrumentData.frontForwardRate;        // dRate = Change in Rate
        const double dTerm = instrumentData.backStartTerm - instrumentData.frontStartTerm;            // dTerm = Change in Term
        const double linearInterpSlope = ( dTerm == 0 ) ? 0 : dRate / dTerm;

        //
        // Synthetic points are inserted both as forwards and as pairs of discount factors corresponding start and end discount factors for the forward period.
        // Pairs of discount factors must not overlap or clash. If consecutive futures have a gap, daily synthetic discount factor pairs will overlap and this
        // is not allowed. Hence the below while loop restriction "syntheticStartDate < instrumentData.frontEndDate"
        //

        // Insert Synthetic Tension Points
        while ( syntheticStartDate < instrumentData.backStartDate && syntheticEndDate < instrumentData.backEndDate && syntheticStartDate < instrumentData.frontEndDate )
        {
            // Must ensure new synthetic points don't overlap into the next existing Forward or Adjacent Instrument Group e.g. Swaps
            // Note: cutoffDate is optional and set to AQLDate() by default
            if ( cutoffDate != AQLDate() ) 
            {   
                if ( syntheticEndDate >= cutoffDate ) break;
            }

            double syntheticStartTerm = termsToDateDaycount.getTerm( spotDate, syntheticStartDate );
            double syntheticEndTerm = termsToDateDaycount.getTerm( spotDate, syntheticEndDate );

            // Interest Rate Convention - Controls instrument daycount and interest rate compounding conventions e.g. Simple Interest Act/Act.
            RateConvention rateCompoundingMethod = AQLMathYieldCurve::setRC (SIMPLE );
			AQLPriceDataConvention discFactConvention( instrumentDaycount.getDayCount(), rateCompoundingMethod );

            // Linear interpolate existing FRAs to imply an artificial / synthetic FRA rate
            double linearInterpSlopeCoefft = ( syntheticStartTerm - instrumentData.frontStartTerm );
            double syntheticForwardRate = linearInterpIntercept + ( linearInterpSlopeCoefft * linearInterpSlope );

            double syntheticStartDF = discountFactorInterpolationTable->value( syntheticStartTerm );
            double syntheticEndDF = syntheticStartDF * discFactConvention.getDF(syntheticForwardRate, syntheticStartDate, syntheticEndDate );
                       
            // Update Discount Factors for Start and End of FRA Period
            insertDFData( discountFactorMatrix, discountFactorDatesVector, syntheticStartDF, syntheticStartTerm, syntheticStartDate );
            insertDFData( discountFactorMatrix, discountFactorDatesVector, syntheticEndDF, syntheticEndTerm, syntheticEndDate );
            insertForwardRateData( forwardTermsMatrix, forwardRatesVector, syntheticStartTerm, syntheticEndTerm, syntheticForwardRate );

            // Update the next synthetic FRA start- and end date
            syntheticStartDate.addDays( tensionGap );
            syntheticEndDate.addDays( tensionGap );
        }
    }

    // Update the front Forward rate, start and end date for the next group of synthetic Forward points
    instrumentData.frontForwardRate         = forwardRate;
    instrumentData.frontStartDate           = startDate;
	instrumentData.frontEndDate             = endDate;
    instrumentData.frontStartTerm           = termsToDateDaycount.getTerm( spotDate, startDate );
    instrumentData.frontEndTerm             = termsToDateDaycount.getTerm( spotDate, endDate );
}

// calibrate DiscountFactor
void
AQLPriceYieldGenerator::estimateSwapDFCurve(const AQLDate& basedate, 
										AQLObject& object, 
										const AQLDataProcedure& att,
										const AQLString& dfCurveName) const
{
	const AQLMathYieldCurvePro& yg = dynamic_cast<AQLMathYieldCurvePro&>(object);

	const std::map<AQLString, bool>& genCurveGenMap = dynamic_cast<AQLMathYieldCurvePro &>(object).getGCurveGenerateMap();
	const std::map<AQLString, AQLString>& assignedCurveMktMap = dynamic_cast<AQLMathYieldCurvePro &>(object).getAssignedCurveMktMap();

	// check initial target curve
	AQLString initialTargetDF = "";
	const AQLDataHolder *dh = &yg.getData(IR_CALIBRATION_DATA_GENTARGETDF, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		initialTargetDF = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}


	if (yg.getMarketForCurve(dfCurveName) == IR_NO_DATA)
	{
		throw AQLCoreInvalidData("df curve name is not in generate dfs!", __FILE__, __LINE__); 
	}
	AQLString marketName = yg.getMarketForCurve(dfCurveName);
	AQLMathYieldCurvePro* pYcPro = &dynamic_cast<AQLMathYieldCurvePro&>(object);
	if (marketName == "OISCURVE" || marketName == "LOBASIS")
	{
		pYcPro->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
		pYcPro->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(dfCurveName));
		calibrateModel(basedate, object, att);
		pYcPro->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
		if (initialTargetDF != AQLString("")) pYcPro->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(initialTargetDF));
	}
	else if (marketName == "XCCYBASIS")
	{
		pYcPro->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
		pYcPro->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new AQLDataString(dfCurveName));
		pYcPro->setBasisRates(dfCurveName);
		pYcPro->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
		if (initialTargetDF != AQLString("")) pYcPro->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new AQLDataString(initialTargetDF));
	}
	else if (marketName == "FWDFXCONST") 
	{
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); ++it)
		{
			if(it->second == "XCCYBASIS")
			{
				pYcPro->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
				pYcPro->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new AQLDataString(it->first));
				pYcPro->setBasisRates(it->first);
				pYcPro->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
				if (initialTargetDF != AQLString("")) pYcPro->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new AQLDataString(initialTargetDF));
					break;
			}
		}
		pYcPro->calcFwdFXConstantCurve();
	}
	else
	{
		throw AQLCoreInvalidData("df curve name is not in generate dfs!", __FILE__, __LINE__); 
	}
}


/*!
@brief Determine the join date used by linear spline interpolation
@param[in] lastFuture	Last future calibration instrument
@param[in] firstSwap	First swap calibration instrument
@param[in] spotDate		Spot date
@param[in] is_fwdswap	Is forward starting swap used?
@return	   Linear spline join date
*/
AQLDate 
AQLPriceYieldGenerator::determineLinearSplineInterpolationJoinDate(const AQLObject* lastFuture,
	const AQLObject* firstSwap,
	const AQLDate& spotDate,
	bool is_fwdswap
)
{
	AQLDate joinDate;
	AQLDate lastFutureEndDate = dynamic_cast< const AQLDataDate& >((lastFuture->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
	AQLDate lastFutureStartDate = dynamic_cast< const AQLDataDate& >((lastFuture->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();

	//-------------------------------------------------------------------------------------------------
	// Find the date schedule of the first swap

	AQLString freq = dynamic_cast<const AQLDataString&> ((firstSwap->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL)).get()).get();
	const AQLPriceDataCalendar * calendar = &dynamic_cast<const AQLPriceDataCalendar&> ((firstSwap->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule * slidingRule = &dynamic_cast<const AQLPriceDataSlidingRule&> ((firstSwap->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
	const AQLPriceDataDayCount * dayCount = &dynamic_cast<const AQLPriceDataDayCount&> ((firstSwap->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());

	bool eom = false;
	const AQLDataHolder* dh = &firstSwap->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	AQLString roll_conv;
	if (freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
	else if (eom) roll_conv = ROLLCONV_EOM;
	else roll_conv = ROLLCONV_NORMAL;

	AQLDate startDate, endDate;
	DateVector datesVec;
	DoubleArray temp;
	if (is_fwdswap)
	{
		const bool is_date = dynamic_cast<const AQLDataBool&> ((firstSwap->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
		if (is_date)
		{
			startDate = dynamic_cast<const AQLDataDate&> ((firstSwap->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
			endDate = dynamic_cast<const AQLDataDate&> ((firstSwap->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
		}
		else
		{
			const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((firstSwap->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
			startDate = AQLDateCalculations::getDate(spotDate, sterm_str, *slidingRule, calendar, true, &roll_conv);
			const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((firstSwap->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
			endDate = AQLDateCalculations::getDate(startDate, tenor_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
		}

		AQLPriceYieldGenerator::getPaymentDates(startDate, endDate, freq, *calendar, *slidingRule, *dayCount, datesVec, temp, temp, eom);
	}
	else
	{
		const AQLString& term_str = dynamic_cast<const AQLDataString&> ((firstSwap->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		endDate = AQLDateCalculations::getDate(spotDate, term_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);

		AQLPriceYieldGenerator::getPaymentDates(spotDate, endDate, freq, *calendar, *slidingRule, *dayCount, datesVec, temp, temp, eom);
	}

	//-------------------------------------------------------------------------------------------------
	// determine the future/swap join date

	AQLDate firstDate, secondDate;
	size_t dateCount = datesVec.size();
	for (size_t i = 0; i < dateCount - 1; ++i)
	{
		if (datesVec[i] < lastFutureEndDate && datesVec[i + 1] >= lastFutureEndDate)
		{
			if (datesVec[i] == lastFutureStartDate)
			{
				// Don't use the last future's start date as join date as empirical study
				// shows it would give too much freedom to later part of the curve.
				continue;
			}
			else
			{
				// Select join date as the first swap Libor fixing date before the last future ends
				joinDate = datesVec[i];
				break;
			}
		}
		else if (datesVec[i] == lastFutureEndDate)
		{
			// Don't use the last future end date as the join date.
			// In this case pick the middle date between the current and previous Libor fixing dates
			firstDate = datesVec[i - 1];
			secondDate = datesVec[i];
			int intervalDays = firstDate.intervalDays(secondDate);
			firstDate.addDays(int(intervalDays >> 1));	// bitwise shift to divide by 2 to get to mid point
			joinDate = firstDate;
		}
	}

	// If joinDate fails to be set by now, set it to be the middle date between the start and end of the last future.
	if (joinDate == AQLDate())
	{
		firstDate = lastFutureStartDate;
		secondDate = lastFutureEndDate;
		int intervalDays = firstDate.intervalDays(secondDate);
		firstDate.addDays(int(intervalDays >> 1));	// bitwise shift to divide by 2 to get to mid point
		joinDate = firstDate;
	}

	return joinDate;
}

/*
@brief Calculate start and end dates of a FRA instrument

@param[out] startDate		FRA start date
@param[out] endDate			FRA end date
@param[in]  spotDate		Spot date of curve
@param[in]  data_fra		FRA instrument
@param[in]	refRateTerm		The tenor of the curve
@param[in]	baseFreq		The frequency of the curve
@param[in]	dayCount				FRA day count
*/
void 
AQLPriceYieldGenerator::calculateFraDates(AQLDate& startDate,
	AQLDate& endDate,
	const AQLDate& spotDate,
	const AQLObject* data_fra,
	const AQLString& refRateTerm,
	const AQLString& baseFreq,
	const AQLPriceDataDayCount& dayCount)
{
	AQLString roll_conv("");
	bool eom = false;
	const AQLDataHolder* dh = &(data_fra->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// set roll convention			
	if (baseFreq == LUNAR)
	{
		roll_conv = ROLLCONV_LUNAR;
	}
	else if (eom)
	{
		roll_conv = ROLLCONV_EOM;
	}
	else
	{
		roll_conv = ROLLCONV_NORMAL;
	}

	const AQLPriceDataSlidingRule& slidingRule = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_fra->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
	const AQLPriceDataCalendar& calendar = dynamic_cast<const AQLPriceDataCalendar&> ((data_fra->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());

	dh = &data_fra->getData(PRICING_DATA_ISDATE, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const bool is_date = dynamic_cast<const AQLDataBool&> (dh->get()).get();
		if (is_date)
		{
			startDate = dynamic_cast<const AQLDataDate&> ((data_fra->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
			endDate = dynamic_cast<const AQLDataDate&> ((data_fra->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
		}
		else
		{
			const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((data_fra->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
			startDate = AQLDateCalculations::getDate(spotDate, sterm_str, slidingRule, &calendar, true, &roll_conv);
			const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((data_fra->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
			endDate = AQLDateCalculations::getDate(startDate, tenor_str, slidingRule, &calendar, true, &roll_conv);
		}
	}
	else
	{
		const AQLString& terms_str_x = dynamic_cast<const AQLDataString&> ((data_fra->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		const AQLString& terms_str = AQLPriceYieldGenerator::changeFRATermFormat(terms_str_x);
		startDate = AQLDateCalculations::getDate(spotDate, terms_str, slidingRule, &calendar, true, &roll_conv);
		endDate = AQLDateCalculations::getDate(startDate, refRateTerm, slidingRule, &calendar, true, &roll_conv);
	}
}
