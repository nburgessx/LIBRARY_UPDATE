#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataManager.h"
#include "AQLMathDefine.h"
#include "AQLMathDateCalculations.h"
#include "AQLPricePayOff.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLPriceIRSwaptionValue.h"
#include "AQLMathYieldCurve.h"
#include "AQLLinearRatesOptionValueDataProvider.h"
#include "AQLPriceCashFlowGenerator.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLSplineInterpolation.h"
#include "AQLMathIRVanillaFuncUtility.h"
#include "AQLMathIndexEntity.h"
#include "AQLMathVolFuncIRSABR.h"
#include "AQLCoreComponentManager.h"

#define STD "STD"

using namespace std;

AQLPriceIRSwaptionValue::AQLPriceIRSwaptionValue()
: AQLLinearRatesOptionValue()
{}

//AQLPriceIRSwaptionValue::AQLPriceIRSwaptionValue(AQLPriceIRSwaptionValue& v)
//: AQLLinearRatesOptionValue(v)
//{}

AQLPriceIRSwaptionValue::~AQLPriceIRSwaptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLPriceIRSwaptionValue::getType() const
{
	return FN_IR_SWAPTIONVALUE;
}
/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceIRSwaptionValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_SWAPTIONVALUE ? true : AQLLinearRatesOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
AQLString 
AQLPriceIRSwaptionValue::getOptionPayoffName() const
{
	return FN_IR_SWAPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceIRSwaptionValue::registerData(AQLPriceDataManager& dm) const
{
	AQLLinearRatesOptionValue::registerData(dm);
	dm.setData(DATA_M_MV_IR_TENOR, DATA_STRING);
	dm.setData(PRICING_DATA_INDEXGENERATOR, DATA_REFERENCE);
	dm.setData(PRICING_DATA_FIXEDLEGDAYCOUNT, DATA_DAYCOUNT);
	dm.setData(PRICING_DATA_FIXEDLEGCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, DATA_BOOL);
	dm.setData(PRICING_DATA_ISIRRMODEL, DATA_BOOL);
	dm.setData(PRICING_DATA_PREMIUMAMOUT, DATA_DOUBLE);
	dm.setData(PRICING_DATA_PREMIUMPAYMENTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_PREMIUMPAYCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_ISCLEAREDPHYSICALSETTLE, DATA_BOOL);
	dm.setData(PRICING_DATA_CASHSETTLEMENTAMOUNT, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CASHSETTLEMENTPAYMENTDATE, DATA_DATE);
}

AQLCoreFunctionBase*
AQLPriceIRSwaptionValue::clone() const
{
    try 
	{
		return new AQLPriceIRSwaptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}



// calc payoff after maturity
double				
AQLPriceIRSwaptionValue::calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const
{
	//dataProvider;
	AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dp);
	
	AQLDataHolder* dh = &(e.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	AQLString optiontype = dynamic_cast<AQLDataString &>(dh->get()).get();
	optiontype.toUpper();

	dh = &(e.getData(PRICING_DATA_PAYOUT, ISNOTNULL));
	AQLString payout = dynamic_cast<AQLDataString &>(dh->get()).get();
	payout.toUpper();
	
	AnalyticBKParam* bkparam = dynamic_cast<AnalyticBKParam* >(dataProvider->mParam[0][0]);
	double val1 = 0.0;
	double val2 = 0.0;
	if (AQLString("STANDARD") == payout)
	{
		
		if( AQLString("PAYERS") == optiontype && bkparam->F > bkparam->K )
		{
			val1 = bkparam->F - bkparam->K;
		}
		else if(AQLString("RECEIVERS") == optiontype && bkparam->K > bkparam->F)
		{
			val1 = bkparam->K - bkparam->F;
		}
		else
			val1 = 0.0;
	
	}
	else if (AQLString("STRADDLE") == payout)
	{
		// in case of straddle first call, next put
		if (bkparam->F > bkparam->K)
		{
			val1 = bkparam->F - bkparam->K;
		}
		else
			val1 = 0.0;

		bkparam = dynamic_cast<AnalyticBKParam* >(dataProvider->mParam[0][1]);
		if( bkparam->K > bkparam->F)
		{
			val2 = bkparam->K - bkparam->F;
		}
		else
			val2 = 0.0;
	}
	
	double ret = bkparam->Nu * (val1 + val2);
	return ret;
}



/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLPriceIRSwaptionValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	AQLPriceIROptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<AQLPriceIROptionValueDataProvider *>(AQLLinearRatesOptionValue::setUpDataProvider(basedate,object,att));

	//unit
	dh = &(object.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL));
	dataProvider->unit = dynamic_cast<const AQLDataDouble &>(dh->get()).get();

	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	AQLDataMultiReference& refs = dynamic_cast<AQLDataMultiReference &>(dh->get());
	if (refs.getSize() != 1)
		throw AQLCoreInvalidData("Swaption Underlyings error",__FILE__,__LINE__);
	AQLObject& un = refs.get(0).get();

	//check detail
	dh = &(un.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
	AQLString inputtype = dynamic_cast<AQLDataString &>(dh->get()).get();
	if (inputtype.toUpper() != "DETAIL")
		throw AQLCoreInvalidData("Swaption InputType error",__FILE__,__LINE__);

	// get DiscountCurve
	dataProvider->mDCurveType = STD;
	dh = &(un.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mDCurveType = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}

	dh = &(un.getData(PRICING_DATA_INDEXGENERATOR, ISNOTNULL));
	AQLDataReference& genref = dynamic_cast<AQLDataReference &>(dh->get());
	AQLObject& gen = genref.get().get();


	dataProvider->mUnFrequency = getFrequencyFromIndexGenerator(gen,dataProvider);

	dataProvider->mUnTenor.resize(1);
	dataProvider->mUnTenor[0] = getNearestTenorString(un, dataProvider->mUnFrequency);

	dh = &(un.getData(PRICING_DATA_STARTDATE, ISNOTNULL));
	dataProvider->mUnStartDate = dynamic_cast<AQLDataDate &>(dh->get()).get();

	
	dh = &(gen.getData(PRICING_DATA_DAYCOUNT, ISNOTNULL));
	dataProvider->mpUnDayCount = &(dynamic_cast<AQLPriceDataDayCount &>(dh->get()));
	dh = &(un.getData(PRICING_DATA_FIXEDLEGDAYCOUNT, ISNOTNULL));
	dataProvider->mpUnDayCountOfFixedLeg = &(dynamic_cast<AQLPriceDataDayCount &>(dh->get()));

	dh = &(gen.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
	dataProvider->mpUnSlidingRule = &(dynamic_cast<AQLPriceDataSlidingRule &>(dh->get()));

	dh = &(gen.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
	dataProvider->mpUnCalendar = &(dynamic_cast<AQLPriceDataCalendar &>(dh->get()));
	dh = &(un.getData(PRICING_DATA_FIXEDLEGCALENDAR, ISNOTNULL));
	dataProvider->mpUnCalendarOfFixedLeg = &(dynamic_cast<AQLPriceDataCalendar &>(dh->get()));

	dh = &(gen.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	dataProvider->mUnCurrency = dynamic_cast<AQLDataString &>(dh->get()).get();

	//get ForecastCurve(BasisCurve)
	dataProvider->mFCurveType = STD;
	dh = &(gen.getData(PRICING_DATA_BASISCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mFCurveType = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}

	dataProvider->mFCurveTypes.resize(1);
	dataProvider->mFCurveTypes[0] = STD;
	dh = &(gen.getData(PRICING_DATA_BASISCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mFCurveTypes[0] = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}


	//strike
	dh = &(object.getData(PRICING_DATA_STRIKE, ISNOTNULL));
	double strike = dynamic_cast<AQLDataDouble &>(dh->get()).get();
	std::vector<AnalyticBKParam *> bkparam(dataProvider->mParam[0].size());

	//in case of swaption strike = strike - margin;
	double margin = 0.0;
	dh = &(object.getData(PRICING_DATA_MARGIN, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		margin = dynamic_cast<AQLDataDouble &>(dh->get()).get();

	double adjterm = 1.0;
	if (margin != 0.0)
	{
		//this ajust term is temporary
		AQLString strMonth = getFrequencyFromIndexGenerator(gen,dataProvider,true);
		AQLDate todate = AQLMathDateCalculations::getDate(dataProvider->mAsofDate,strMonth,true);
		double t1 = dataProvider->mpUnDayCountOfFixedLeg->getTerm(dataProvider->mAsofDate, todate);
		double t2 = dataProvider->mpUnDayCount->getTerm(dataProvider->mAsofDate, todate);
		if (0.0 == t1)
			throw AQLCoreInvalidData("DayCount Error",__FILE__,__LINE__);
		
		adjterm = t2/t1;
	}
	
	for (unsigned int i = 0; i < bkparam.size(); i++)
	{
		bkparam[i] = dynamic_cast<AnalyticBKParam *>(dataProvider->mParam[0][i]);
		bkparam[i]->K = strike - margin * adjterm;
	}
	return dataProvider;
}

std::vector< std::vector<AnalyticParam*> >
AQLPriceIRSwaptionValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dp) const
{
	(void)object;
	(void)dp;
	std::vector< std::vector<AnalyticParam*> > ret(1);
	const AQLDataHolder* dh = &(object.getData(PRICING_DATA_PAYOUT, ISNOTNULL));
	AQLString payout = dynamic_cast<const AQLDataString &>(dh->get()).get();
	payout.toUpper();
	if (payout == "STRADDLE")
	{
		ret[0].resize(2);
		ret[0][0] = new AnalyticBKParam();
		ret[0][1] = new AnalyticBKParam();
	}
	else
	{
		ret[0].resize(1);
		ret[0][0] = new AnalyticBKParam();
	}
	return ret;
}

void
AQLPriceIRSwaptionValue::setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const
{
	AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dp);
	if (dataProvider->mAsofDate >= dataProvider->mMaturityDate)
	{
		return;
	}
	AQLDataInstance* dataInstance = object.getDataInstance(); 
	AQLDataHolder*dh;

	AQLDate mdymatudate = (dataProvider->mAsofDate < dataProvider->mMaturityDate) ? dataProvider->mMaturityDate : dataProvider->mAsofDate;
	
	//AQLMathYieldCurve
	const AQLMathYieldCurve& baseyc = dataProvider->mpvanilla->getIRCurve(dataProvider->mUnCurrency);
	const AQLObject& ylddata = baseyc.getYieldData().get().get();
	AQLString curveid = dynamic_cast<const AQLDataString &>(ylddata.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	
	//get yc
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveid);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	//yc.getDayCount().setDayCount(dataProvider->mpUnDayCount->getDayCount());
	yc.getDayCount().setDayCount(dataProvider->mpUnDayCountOfFixedLeg->getDayCount());
	yc.getSlidingRule().convertFromString(dataProvider->mpUnSlidingRule->convertToString());
	
	//AQLString calstr = dataProvider->mpUnCalendar->convertToString();
	AQLString calstr = dataProvider->mpUnCalendarOfFixedLeg->convertToString();
	AQLMathCurveFuncUtility::setCalendarForCurveID(yc,calstr);
	yc.getFrequency().convertFromString(dataProvider->mUnFrequency);

	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	//double
	yc.setCurveType(dataProvider->mDCurveType);
	double nu = AQLMathIRVanillaFuncUtility::getAnnuity(dataProvider->mUnStartDate,dataProvider->mUnTenor[0],yc);
	double te = dataProvider->mBlackDayCount.getTerm(dataProvider->mAsofDate, dataProvider->mMaturityDate,true);
	double actt = dc_act365.getTerm(dataProvider->mAsofDate, dataProvider->mMaturityDate,true);

	//yc.setCurveType(dataProvider->mFCurveTypes[0]);
	AQLMathCurveFuncUtility::setUpForwardDayCount(dataInstance, curveid, dataProvider->mFCurveTypes[0], yc);
	double forward = yc.getParRate(dataProvider->mUnStartDate,dataProvider->mUnTenor[0],NULL,NULL,NULL,dataProvider->mFCurveTypes[0],dataProvider->mDCurveType);
//#ifdef ZEROFLOOR
//	forward = AQLMath::max(forward, MIN_RATE);
//#endif	
	std::vector <AnalyticBKParam *> bkparam(dataProvider->mParam[0].size());
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		bkparam[i] = dynamic_cast<AnalyticBKParam* >(dataProvider->mParam[0][i]);
		bkparam[i]->Te = te;
		bkparam[i]->Nu = nu;
		bkparam[i]->F = forward;
		bkparam[i]->actT = actt;
	}

	dataProvider->mMaturityDates.resize(1);
	dataProvider->mMaturityDates[0] = dataProvider->mMaturityDate;
	return;
}

const AQLDate&
AQLPriceIRSwaptionValue::getDeliveryDate(const AQLObject& object, AQLDataProvider* dp) const
{
	(void) dp;
	const AQLDataHolder* dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& refs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
	if (refs.getSize() > 2)
		throw AQLCoreInvalidData("Swaption Underlyings error",__FILE__,__LINE__);
	AQLObject& un = refs.get(0).get();

	dh = &(un.getData(PRICING_DATA_ENDDATE, ISNOTNULL));
	const AQLDate& edate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
	return edate;	
}


AQLString 
AQLPriceIRSwaptionValue::getNearestTenorString(const AQLObject& object, const AQLString& freq) const
{
	//AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dataProvider);

	AQLString ret;
	const AQLDataHolder* dh = &(object.getData(DATA_M_MV_IR_TENOR, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		ret = dynamic_cast<const AQLDataString &>(dh->get()).get();
		ret.toUpper();
		return ret;
	}

	dh = &(object.getData(PRICING_DATA_STARTDATE, ISNOTNULL));
	AQLDate sdate = dynamic_cast<const AQLDataDate &>(dh->get()).get();

	dh = &(object.getData(PRICING_DATA_ENDDATE, ISNOTNULL));
	AQLDate edate = dynamic_cast<const AQLDataDate &>(dh->get()).get();

	int y = sdate.intervalYears(edate);
	int m = 12*y + sdate.intervalMonths(edate);
	
	//AQLString freq = dataProvider->mUnFrequency;
	if (freq == MONTHLY)
	{
		ret = AQLString(m) + AQLString("M");
	}
	else if (freq == QUARTERLY)
	{
		/*if (m % 3 != 0)
		{
			throw AQLCoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}*/
		double val = static_cast<double>(m)/3;
		val = (ceil(val) - val > val - floor(val)) ? floor(val) : ceil(val);
		int adjustm = static_cast<int>(val);
		adjustm *= 3;
		if (0 == adjustm)
			adjustm = 3;

		ret = AQLString(adjustm) + AQLString("M");
		
	}
	else if (freq == SEMI_ANNUAL)
	{
		double val = static_cast<double>(m)/6;
		val = (ceil(val) - val > val - floor(val)) ? floor(val) : ceil(val);
		int adjustm = static_cast<int>(val);
		adjustm *= 6;
		if (0 == adjustm)
			adjustm = 6;
		ret = AQLString(adjustm) + AQLString("M");

	}
	else if (freq == ANNUAL)
	{
		double val = static_cast<double>(m)/12;
		val = (ceil(val) - val > val - floor(val)) ? floor(val) : ceil(val);
		int adjustm = static_cast<int>(val);
		adjustm *= 12;
		if (0 == adjustm)
			adjustm = 12;
		ret = AQLString(adjustm) + AQLString("M");
		
	}
	else
	{
		//error
		AQLString err = "Frequency: ";
		err += freq;
		err += " is not support";
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	ret.toUpper();
	return ret;

}

AQLString 
AQLPriceIRSwaptionValue::getFrequencyFromIndexGenerator(const AQLObject& object, AQLDataProvider* dp, bool isMonthString) const
{
	//check index type it must be LIBOR
	const AQLDataHolder* dh = &(object.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
	AQLString indextype = dynamic_cast<const AQLDataString &>(dh->get()).get();
	if (indextype.toUpper() != LIBOR)
		throw AQLCoreInvalidData("IndexGenerator must be Libor index",__FILE__,__LINE__);

	//accessory
	dh = &(object.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
	AQLString libormonth = dynamic_cast<const AQLDataString &>(dh->get()).get();
	int y, m, d, w;
	AQLMathDateCalculations::termStrtoYMDW(libormonth, y, m, d, w);
	m = 12*y + m;
	if (!isMonthString)
	{
		if (12 == m)
		return ANNUAL;
		else if (6 == m)
			return SEMI_ANNUAL;
		else if (3 == m)
			return QUARTERLY;
		else if (1 == m)
			return MONTHLY;
		else
			throw AQLCoreInvalidData("Accessory Error",__FILE__,__LINE__);
	}
	else
	{
		if (12 == m)
		return "12M";
		else if (6 == m)
			return "6M";
		else if (3 == m)
			return "3M";
		else if (1 == m)
			return "1M";
		else
			throw AQLCoreInvalidData("Accessory Error",__FILE__,__LINE__);
	}

	

}

/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
AQLPriceIRSwaptionValue::createNewDataProvider() const
{
	AQLPriceIROptionValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new AQLPriceIROptionValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

void
AQLPriceIRSwaptionValue::getAnalyticParamResult( const AQLObject& object,
											 AQLDataProvider* dp,
											 AQLStringVector& names,
											 DoubleVector& params 
										   ) const
{
	const AQLDataHolder* dh = &object.getData(PRICING_DATA_PAYOUT, ISNOTNULL);
	AQLString payout = dynamic_cast<const AQLDataString&>(dh->get()).get();
	payout.toUpper();

	double ret = 0.0;
	AQLLinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<AQLLinearRatesOptionValueDataProvider *>(dp);
	for (unsigned int i = 0; i < dataProvider->mParam.size(); i++)
	{
		for (unsigned int j = 0; j < dataProvider->mParam[i].size(); j++)
		{
			if (dataProvider->mParam[i][j] != 0)
			{
				AnalyticBKParam* bk = dynamic_cast< AnalyticBKParam* >( dataProvider->mParam[i][j] );

				// ! Get F
				params.push_back( bk->F );
				names.push_back( "Forward" );

				// ! Get Vol
				double forwardShiftValue(0.);
				if(dataProvider->mVolfunc->isTypeOf(FN_VOLFUNCIRSABR))
				{
					forwardShiftValue = dynamic_cast<const AQLMathVolFuncIRSABR*>(dataProvider->mVolfunc)->getForwardShiftValue();
				}
				if (bk->F + forwardShiftValue <= MIN_RATE || bk->K + forwardShiftValue <= MIN_RATE)
				{
					params.push_back( 0. );
				}
				else
				{
					params.push_back( bk->Vol );
				}
				names.push_back( "Volatility" );

				// ! Get Strike
				params.push_back( bk->K );
				names.push_back( "Strike" );

				// ! Get Te
				params.push_back( bk->Te );
				names.push_back( "ExpiryTerm" );

				//// ! Get Td
				//params.push_back( bk->Td );
				//names.push_back( "DeliveryTerm" );

				// ! Get Numeraire
				params.push_back( bk->Nu * dataProvider->unit);
				names.push_back( "Numeraire" );
			}
			if(payout == STRADDLE){
				// When the payout == "STRADDLE", there are two parameter sets each for call and put, and they are wholly same.
				// So we omit the second one.
				break;
			}
		}
	}
}

void 
AQLPriceIRSwaptionValue::setUpNumeraireCurrency(const AQLObject& trade, AQLLinearRatesOptionValueDataProvider* dataProvider) const
{
    const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
    const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference&>(legs.get(0).get().getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
    const AQLString& ccy = dynamic_cast<const AQLDataString&>(cashlets.get(0).get().getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
    dataProvider->mnumerairecur = ccy;
    dataProvider->mnumerairecur.toUpper();
}

double
AQLPriceIRSwaptionValue::value(const AQLDate& basedate, AQLObject& inst, const AQLDataValuation& att) const
{
    if(hasCashflow(inst)){
        return AQLLinearRatesOptionValue::value(basedate, inst, att);
    }
    else{
        const double pv = 0;
        inst.remove(PRICING_DATA_DIRTYPRICE);
        inst.add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(pv));
        inst.remove(PRICING_DATA_CLEANPRICE);
        inst.add(PRICING_DATA_CLEANPRICE, new AQLDataDouble(pv));
		return pv;
    }
}

bool 
AQLPriceIRSwaptionValue::hasCashflow(const AQLObject& trade) const
{
    const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
    const AQLObjectHolder& leg1 = legs.get(0);
    const AQLObjectHolder& leg2 = legs.get(0);
    
    const AQLDataHolder* dh;
    dh = &leg1.getData(PRICING_DATA_CASHLETS);
    const bool leg1_has_cashlets = dh->isDefined() && !dh->isNull();
    dh = &leg2.getData(PRICING_DATA_CASHLETS);
    const bool leg2_has_cashlets = dh->isDefined() && !dh->isNull();

    return leg1_has_cashlets && leg2_has_cashlets;
}

std::vector< std::vector<AQLBlackScholesBase* > > 
AQLPriceIRSwaptionValue::getAnalyticMethod(AQLObject& object, AQLDataProvider* dp) const
{
	(void)dp;

	const AQLStringMatrix bscomponentmat = getBSComponentMat(object, BK);

	std::vector< std::vector<AQLBlackScholesBase* > > ret;
	ret.resize(1, std::vector<AQLBlackScholesBase*>(bscomponentmat[0].size()));

	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	for (unsigned int i = 0; i < ret[0].size(); ++i)
	{
		std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponentmat[0][i]);
		if (it == var.end())
		{
			throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
		}
			
		ret[0][i] = it->second;
	}
	return ret;

}

std::vector< std::vector<AQLBlackScholesBase* > > 
AQLPriceIRSwaptionValue::getPayoffMethod(AQLObject& object, AQLDataProvider* dp) const
{
	(void)dp;

	const AQLStringMatrix bscomponentmat = getBSComponentMat(object, BKPAYOFF);

	std::vector< std::vector<AQLBlackScholesBase* > > ret;
	ret.resize(1, std::vector<AQLBlackScholesBase*>(bscomponentmat[0].size()));

	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	for (unsigned int i = 0; i < ret[0].size(); ++i)
	{
		std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponentmat[0][i]);
		if (it == var.end())
		{
			throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
		}
			
		ret[0][i] = it->second;
	}
	return ret;

}

AQLStringMatrix 
AQLPriceIRSwaptionValue::getBSComponentMat(AQLObject& trade, const AQLString& BKFuncType) const
{
	AQLDataHolder* dh;
	AQLStringMatrix ret;

	dh = &(trade.getData(PRICING_DATA_PAYOUT, ISNOTNULL));
	AQLString payout = dynamic_cast<AQLDataString &>(dh->get()).get();
	payout.toUpper();

	if (payout == "STRADDLE")
	{
		ret.resize(1, AQLStringVector(2));
			
		//call up in
		AQLString bscomponent = BKFuncType + AQLString(PREM) + AQLString(CALL);
		ret[0][0] = bscomponent;
			
		//call up out 
		bscomponent = BKFuncType + AQLString(PREM) + AQLString(PUT);
		ret[0][1] = bscomponent;
		return ret;
		
	}
	else if (payout == "STANDARD")
	{
		ret.resize(1, AQLStringVector(1));
			
		dh = &(trade.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
		AQLString optiontype = dynamic_cast<AQLDataString &>(dh->get()).get();
		optiontype.toUpper();
		AQLString key;
		if (optiontype == "PAYERS")
			key = "CALL";
		else if (optiontype == "RECEIVERS")
			key = "PUT";
		else 
			throw AQLCoreInvalidData("Option Type error",__FILE__,__LINE__);

		AQLString bscomponent = BKFuncType + AQLString(PREM) + key;			
		ret[0][0] = bscomponent;
		return ret;
	}
	else
	{
		throw AQLCoreInvalidData("PayOut error",__FILE__,__LINE__);
	}

}
