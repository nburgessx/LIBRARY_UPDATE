//  2008, AlgoQuantHub.
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
#include "AQLDataProcedure.h"
#include "AQLMathDefine.h"
#include "AQLMathDateCalculations.h"
#include "AQLMathValuableEntity.h"
#include "AQLPricePayOff.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLPriceIRSwaptionValueFromCashFlow.h"
#include "AQLMathYieldCurve.h"
#include "AQLLinearRatesOptionValueDataProvider.h"
#include "AQLPriceCashFlowGenerator.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLSplineInterpolation.h"
#include "AQLMathIRVanillaFuncUtility.h"
#include "AQLMathIndexEntity.h"
#include "AQLPriceConvergenceValue.h"

#define STD "STD"

using namespace std;

AQLPriceIRSwaptionValueFromCashFlow::AQLPriceIRSwaptionValueFromCashFlow()
: AQLPriceIRSwaptionValue()
{}

//AQLPriceIRSwaptionValueFromCashFlow::AQLPriceIRSwaptionValueFromCashFlow(LAPriceIRSwaptionValue2& v)
//: AQLLinearRatesOptionValue(v)
//{}

AQLPriceIRSwaptionValueFromCashFlow::~AQLPriceIRSwaptionValueFromCashFlow()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLPriceIRSwaptionValueFromCashFlow::getType() const
{
	return FN_IR_SWAPTIONVALUEFROMCASHFLOW;
}
/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceIRSwaptionValueFromCashFlow::isTypeOf(function_t id) const
{
	return (id == FN_IR_SWAPTIONVALUEFROMCASHFLOW ? true : AQLPriceIRSwaptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
AQLString 
AQLPriceIRSwaptionValueFromCashFlow::getOptionPayoffName() const
{
	return FN_IR_SWAPTIONVALUEFROMCASHFLOW_STR;
}


AQLCoreFunctionBase*
AQLPriceIRSwaptionValueFromCashFlow::clone() const
{
    try 
	{
		return new AQLPriceIRSwaptionValueFromCashFlow(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLPriceIRSwaptionValueFromCashFlow::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	
	const AQLDataHolder* dh;
	AQLPriceIROptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<AQLPriceIROptionValueDataProvider *>(AQLLinearRatesOptionValue::setUpDataProvider(basedate,object,att));
	//in this class forward rate is made by cashflow.
	dataProvider->mIsFRateFromCurve = false;

	//set legno
	//setLegNo(object,dataProvider);
	dataProvider->mFloatLegNo = getFloatLegNum(object);
	if ( dataProvider->mFloatLegNo == 0 )
	{
		dataProvider->mFixedLegNo = 1;
	}
	else
	{
		dataProvider->mFixedLegNo = 0;
	}

	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& refs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
	if (refs.getSize() != 2)
		throw AQLCoreInvalidData("Swaption Underlyings error",__FILE__,__LINE__);
	AQLObject& floatleg = refs.get(dataProvider->mFloatLegNo).get();

	//unit
	dataProvider->unit = dynamic_cast<const AQLDataDouble &>(refs.get(dataProvider->mFloatLegNo).get().getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();

	//check detail
	dh = &(floatleg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
	AQLString inputtype = dynamic_cast<const AQLDataString &>(dh->get()).get();
	if (inputtype.toUpper() != "MANUAL")
		throw AQLCoreInvalidData("Swaption CashFlowMode error",__FILE__,__LINE__);

	dataProvider->mUnFrequency = getFrequencyFromIndexGenerator(object, dataProvider->mFloatLegNo);

	dataProvider->mUnTenor.resize(1);
	dataProvider->mUnTenor[0] = getNearestTenorString(floatleg, dataProvider->mUnFrequency);

	// get DiscountCurve
	dataProvider->mDCurveType = STD;
	dh = &(floatleg.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mDCurveType = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}


	//mFCurveTypes must exist for irsabr calibration
	dataProvider->mFCurveTypes.resize(1);
	dataProvider->mFCurveTypes[0] = STD;
	const AQLObject& eindex = getLiborEntity(object, dataProvider->mFloatLegNo);
	dh = &(eindex.getData(PRICING_DATA_BASISCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mFCurveTypes[0] = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}
	//forward interpolation
	dh = &(eindex.getData(PRICING_DATA_ISFWDINTERPOLATION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mIsFWDInter = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	//strike
	dh = &(object.getData(PRICING_DATA_STRIKE, ISNOTNULL));
	double strike = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	std::vector<AnalyticBKParam *> bkparam(dataProvider->mParam[0].size());
	//volatility underlying
	dh = &(object.getData(PRICING_DATA_VOLATILITYUNDERLYING, ISNOTNULL));
	dataProvider->mUnderlying.resize(1);
	dataProvider->mUnderlying[0] = dynamic_cast<const AQLDataString &>(dh->get()).get();

	for (unsigned int i = 0; i < bkparam.size(); i++)
	{
		bkparam[i] = dynamic_cast<AnalyticBKParam *>(dataProvider->mParam[0][i]);
		bkparam[i]->K = strike;
	}

	//is calc equivalent strike
	dh = &(object.getData(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->mIsCalcEquivStrike = dynamic_cast<const AQLDataBool &>(dh->get()).get();

	dh = &(object.getData(PRICING_DATA_ISIRRMODEL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->mIsIRRModel = dynamic_cast<const AQLDataBool &>(dh->get()).get();

	dh = &(object.getData(PRICING_DATA_PREMIUMPAYMENTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mIsPremAdjust = true;

		dataProvider->mPremPayDate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
		dataProvider->mPremAmount = 0.0;
		dh = &(object.getData(PRICING_DATA_PREMIUMAMOUT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			dataProvider->mPremAmount = dynamic_cast<const AQLDataDouble &>(dh->get()).get();

		dh = &(object.getData(PRICING_DATA_PREMIUMPAYCURRENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			AQLString chkccy = dynamic_cast<const AQLDataString &>(dh->get()).get();
			if (chkccy.toUpper () != dataProvider->mpvcur)
				throw AQLCoreInvalidData("PremiumCurrency Error",__FILE__,__LINE__); 
		}

		dataProvider->mIsAddFwdPremPV = false;
		dh = &(object.getData( PRICING_DATA_ISADDFWDPREMPV, NOCHECK ));
		if (dh->isDefined() && !dh->isNull())
			dataProvider->mIsAddFwdPremPV = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	dh = &(object.getData(PRICING_DATA_CASHSETTLEMENTPAYMENTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mIsCashSettlementAdjust = true;

		dataProvider->mCashSettlementPayDate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
		
		dataProvider->mCashSettlementAmount = 0.0;
		dh = &(object.getData(PRICING_DATA_CASHSETTLEMENTAMOUNT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			dataProvider->mCashSettlementAmount = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}


	return dataProvider;
}


void
AQLPriceIRSwaptionValueFromCashFlow::setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const
{
	AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dp);
	
	//even if maturity date has passed, set up bkparam for obtaining the temporary volatility, forward
	
	AQLDataInstance* dataInstance = object.getDataInstance(); 
	AQLDataHolder*dh;

	AQLDate mdymatudate = (dataProvider->mAsofDate < dataProvider->mMaturityDate) ? dataProvider->mMaturityDate : dataProvider->mAsofDate;
	std::vector <AnalyticBKParam *> bkparam(dataProvider->mParam[0].size());
	
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	
	double te = dataProvider->mBlackDayCount.getTerm(dataProvider->mAsofDate,mdymatudate,true);
	double actt = dc_act365.getTerm(dataProvider->mAsofDate, mdymatudate,true);
	double forward = 0.0;
	double nu = 1.0;
	double equivstrike = 0.0;
	double avenotional = 0.0;
	double converror = getCalcDataFromCashFlow(object,dataProvider, forward, nu, equivstrike, avenotional);
//#ifdef ZEROFLOOR
//	forward = AQLMath::max(forward, MIN_RATE);
//#endif
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		bkparam[i] = dynamic_cast<AnalyticBKParam* >(dataProvider->mParam[0][i]);
		bkparam[i]->Te = te;
		bkparam[i]->Nu = nu;
		bkparam[i]->F = forward;
		bkparam[i]->actT = actt;
		//for equivalent strike
		bkparam[i]->K = (dataProvider->mIsCalcEquivStrike) ? equivstrike : bkparam[i]->K;
	}

	//for amotize
	dataProvider->unit = (dataProvider->mIsCalcEquivStrike) ? avenotional : dataProvider->unit;

	dataProvider->mMaturityDates.resize(1);
	dataProvider->mMaturityDates[0] = dataProvider->mMaturityDate;
	return;
}


AQLString 
AQLPriceIRSwaptionValueFromCashFlow::getFrequencyFromIndexGenerator(const AQLObject& object, unsigned int floatLegNo, bool isMonthString) const
{
	const AQLObject& eindex = getLiborEntity(object, floatLegNo);

	//accessory
	const AQLDataHolder* dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
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

//get Libor Object
const AQLObject& 
AQLPriceIRSwaptionValueFromCashFlow::getLiborEntity(const AQLObject& object, unsigned int floatLegNo) const
{
	//AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dataProvider);

	//this object is swaption trade object not indexgenerator object.
	const AQLDataHolder* dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
	//unsigned int floatLegNo = dataProvider->mFloatLegNo;
	const AQLObject& liborleg = legs.get(floatLegNo).get();

	dh = &(liborleg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
	AQLString inputtype = dynamic_cast<const AQLDataString &>(dh->get()).get();
	if (inputtype.toUpper() != "MANUAL")
		throw AQLCoreInvalidData("getFrequencyFromSwaption error", __FILE__, __LINE__);

	dh = &(liborleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference &>(dh->get());
	
	for (unsigned int i = 0; i < cashlets.getSize(); i++)
	{
		const AQLObject& ecash = cashlets.get(i).get();

		//for notional exchange
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		const AQLDataMultiReference& coupons = dynamic_cast<const AQLDataMultiReference &>(dh->get());
		const AQLObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			throw AQLCoreInvalidData("getFrequencyFromSwaption error", __FILE__, __LINE__);
		
		const AQLDataMultiReference& indexs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
		const AQLObject& eindex = indexs.get(0).get();

		dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
		AQLString indextype = dynamic_cast<const AQLDataString &>(dh->get()).get();
		if (indextype.toUpper() == "LIBOR")
		{
			return eindex;
		}
	}

	//if we can not find Libor object throw Error;
	throw AQLCoreInvalidData("getFrequencyFromSwaption error", __FILE__, __LINE__);
}

//get float leg number
unsigned int
AQLPriceIRSwaptionValueFromCashFlow::getFloatLegNum(const AQLObject& object) const
{
	//VirIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dataProvider);
	const AQLDataHolder* dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& refs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
	if (refs.getSize() != 2)
		throw AQLCoreInvalidData("Swaption Underlyings error",__FILE__,__LINE__);
	const AQLObject& leg1 = refs.get(0).get();

	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference &>(dh->get());
	unsigned int cSize = cashlets.getSize();
	for (unsigned int i = 0; i < cSize; i++)
	{
		const AQLObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;
			

		const AQLDataMultiReference& coupons = dynamic_cast<const AQLDataMultiReference &>(dh->get());
		const AQLObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
		const AQLDataMultiReference& indexs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
		const AQLObject& eindex = indexs.get(0).get();

		dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
		AQLString indextype = dynamic_cast<const AQLDataString &>(dh->get()).get();
		indextype.toUpper();
		if (indextype == "FIXEDRATE")
		{
			/*dataProvider->mFixedLegNo = 0;
			dataProvider->mFloatLegNo = 1;*/
			return 1;
		}
		else
		{
			//dataProvider->mFixedLegNo = 1;
			//dataProvider->mFloatLegNo = 0;
			return 0;
		}
	}

	//in case of no coupon it means that all of fixed rate might be type of Extra CF
	//dataProvider->mFixedLegNo = 0;
	//dataProvider->mFloatLegNo = 1;
	return 1;
}

//getCalcDataFromCashFlow
double 
AQLPriceIRSwaptionValueFromCashFlow::getCalcDataFromCashFlow(const AQLObject& object, AQLDataProvider* dp, double& frate, double& annuity, double& equivstrike, double& avenotional) const
{
	//the below is main source of swaption
	const AQLDataHolder*dh;
	AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dp);

	AQLDataInstance* dataInstance = object.getDataInstance();
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	
	AQLString nb = dynamic_cast<const AQLDataString& >(object.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	double targetPV = 0.0;
	AQLMathObjectValue* orgswap =NULL;
	AQLDate swap_basedate = dataProvider->mMaturityDate;
	if (dataProvider->mMaturityDate < dataProvider->mAsofDate)
	{
		swap_basedate = dataProvider->mAsofDate;
	}
	bool isResultOut = false;
	dh = &object.getData( PRICING_DATA_ISRESULTOUTPUT, NOCHECK );
	if (dh->isDefined() && !dh->isNull())
	{
		isResultOut = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	}
	if (dataProvider->mIsCalcEquivStrike || isResultOut)
	{
		AQLString orgname = "ChangeToOriginalSwap" + nb;
		const AQLObjectHolder ehOrg = objPool.getObject(orgname);
		if(!ehOrg.isDefined())
		{	
			//orgswap= new AQLMathObjectValue(object.getDataInstance());
			orgswap= dynamic_cast<AQLMathObjectValue *>(object.clone());
			objPool.set(orgname,orgswap);
			//name
			orgswap->getName().convertFromString(orgname);
			////path object
			AQLString pathname = dataProvider->mpvanilla->getName().get();
			orgswap->remove(PRICING_DATA_PATHENTITY);
			orgswap->AQLObject::add(PRICING_DATA_PATHENTITY, new AQLDataReference()).convertFromString(pathname);
			//isdetailoutput
			orgswap->remove(PRICING_DATA_ISDETAILOUTPUT);
			orgswap->AQLObject::add(PRICING_DATA_ISDETAILOUTPUT, new AQLDataString()).convertFromString("TRUE");
			//settledate
			orgswap->remove(PRICING_DATA_SETTLEDATE);
			orgswap->AQLObject::add(PRICING_DATA_SETTLEDATE, new AQLDataDate(swap_basedate));
			//valuedate
			orgswap->remove(PRICING_DATA_VALUEDATE);
			orgswap->AQLObject::add(PRICING_DATA_VALUEDATE, new AQLDataDate(swap_basedate));
			//today
			orgswap->remove(PRICING_DATA_TODAY);
			orgswap->AQLObject::add(PRICING_DATA_TODAY, new AQLDataDate(dataProvider->mAsofDate));
			//underlyings
			//AQLString understrs = object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).convertToString();
			//orgswap->AQLObject::add(CALIBRATION_DATA_UNDERLYINGS, new AQLDataMultiReference()).convertFromString(understrs);
			//currency
			dh = &(object.getData(PRICING_DATA_PREMIUMCURRENCY,ISNOTNULL));
			AQLString premcur = dynamic_cast<const AQLDataString &>(dh->get()).get();
			orgswap->remove(PRICING_DATA_CURRENCY);
			orgswap->AQLObject::add(PRICING_DATA_CURRENCY, new AQLDataString(premcur));
			////cfgenerator
			orgswap->remove(PRICING_DATA_CFGENERATOR);
			orgswap->AQLObject::add(PRICING_DATA_CFGENERATOR, new AQLDataProcedure()).convertFromString(FN_IR_CASHFLOWGENERATOR_STR);
			//resultoutput
			orgswap->remove(PRICING_DATA_ISRESULTOUTPUT);
			orgswap->AQLObject::add(PRICING_DATA_ISRESULTOUTPUT, new AQLDataBool(true));

			//even if basdate > fixing date(swaption maturity date), never throw error
			//that is why we must add PRICING_DATA_ISSAVEPASTFIXING
			orgswap->remove(PRICING_DATA_ISSAVEPASTFIXING);
			orgswap->AQLObject::add(PRICING_DATA_ISSAVEPASTFIXING, new AQLDataBool(true));
				
			//about theta
			//dh = &(e.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
			//if (dh->isDefined() && !dh->isNull())
			//{
			//	parswap->AQLObject::add(PRICING_DATA_ISSAVEPASTFIXING, dh->clone());
			//}	

			//the below is difference of parswap
			orgswap->setValuationMethod(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
		}
		else
		{
			orgswap	= &dynamic_cast<AQLMathObjectValue &>(objPool.getObject(orgname).get());
			//settledate
			dynamic_cast<AQLDataDate &>(orgswap->getData(PRICING_DATA_SETTLEDATE, ISNOTNULL).get()).set(swap_basedate);
			//valuedate
			dynamic_cast<AQLDataDate &>(orgswap->getData(PRICING_DATA_VALUEDATE, ISNOTNULL).get()).set(swap_basedate);
			//today
			dynamic_cast<AQLDataDate &>(orgswap->getData(PRICING_DATA_TODAY, ISNOTNULL).get()).set(dataProvider->mAsofDate);
			//currency
			dh = &(object.getData(PRICING_DATA_PREMIUMCURRENCY,ISNOTNULL));
			AQLString premcur = dynamic_cast<const AQLDataString &>(dh->get()).get();
			dynamic_cast<AQLDataString &>(orgswap->getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).set(premcur);
			
			AQLString pathname = dataProvider->mpvanilla->getName().get();
			orgswap->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).convertFromString(pathname);
		}

		orgswap->remove(PRICING_DATA_ISSETUPPAYOFF);
		orgswap->AQLObject::add(PRICING_DATA_ISSETUPPAYOFF, new AQLDataBool(false));

		//in case of libor spreads mode
		dh = &(orgswap->getData(PRICING_DATA_ORIGINALLIBORSPREADNAMES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const AQLStringVector& libornames = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

			dh = &(orgswap->getData(PRICING_DATA_ORIGINALLIBORSPREADS, ISNOTNULL));
			DoubleVector spreads = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();
			if (libornames.size() != spreads.size())
				throw AQLCoreInvalidData("Original Libor Size Error",__FILE__,__LINE__);
			
			for (unsigned int i = 0; i < libornames.size(); i++)
			{
				AQLObject& ecoupon = objPool.getObject(libornames[i], ENCHKTYPE_ISDEFINED).get();
				
				DoubleVector coeffs = dynamic_cast<AQLDataDoubles &>
										(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).get();
				if (coeffs.size() < 2)
					throw AQLCoreInvalidData("Original Libor Size Error",__FILE__,__LINE__);
				coeffs.back() = spreads[i];
				dynamic_cast<AQLDataDoubles &>(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).set(coeffs);
			}
		}
		//in case of amortize mode
		dh = &(orgswap->getData(PRICING_DATA_ORIGINALNOTIONALNAMES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const AQLStringVector& notionalnames = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

			dh = &(orgswap->getData(PRICING_DATA_ORIGINALNOTIONALS, ISNOTNULL));
			DoubleVector notionals = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();
			if (notionalnames.size() != notionals.size())
				throw AQLCoreInvalidData("Original Notional Size Error",__FILE__,__LINE__);
			
			for (unsigned int i = 0; i < notionalnames.size(); i++)
			{
				AQLObject& ecash = objPool.getObject(notionalnames[i], ENCHKTYPE_ISDEFINED).get();
				double notional = dynamic_cast<AQLDataDouble &>
										(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
				notional = notionals[i];
				dynamic_cast<AQLDataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).set(notional);
			}
		}
		orgswap->value(swap_basedate);
		if (dataProvider->mIsCalcEquivStrike)
		{
			dh = &(orgswap->getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL));
			targetPV = dynamic_cast<const AQLDataDouble&>(dh->get()).get(); 
		}
	}
	

	AQLString tempname = "ChangeToSwap" + nb;
	AQLMathObjectValue* parswap =NULL;
	const AQLObjectHolder ehCur = objPool.getObject(tempname);
	if(!ehCur.isDefined())
	{	
		//parswap= new AQLMathObjectValue(object.getDataInstance());
		parswap= dynamic_cast<AQLMathObjectValue *>(object.clone());
		objPool.set(tempname,parswap);
		//name
		parswap->getName().convertFromString(tempname);
		//path object
		AQLString pathname = dataProvider->mpvanilla->getName().get();
		parswap->remove(PRICING_DATA_PATHENTITY);
		parswap->AQLObject::add(PRICING_DATA_PATHENTITY, new AQLDataReference()).convertFromString(pathname);
		//isdetailoutput
		parswap->remove(PRICING_DATA_ISDETAILOUTPUT);
		parswap->AQLObject::add(PRICING_DATA_ISDETAILOUTPUT, new AQLDataString()).convertFromString("TRUE");
		//settledate
		parswap->remove(PRICING_DATA_SETTLEDATE);
		parswap->AQLObject::add(PRICING_DATA_SETTLEDATE, new AQLDataDate(swap_basedate));
		//valuedate
		parswap->remove(PRICING_DATA_VALUEDATE);
		parswap->AQLObject::add(PRICING_DATA_VALUEDATE, new AQLDataDate(swap_basedate));
		//today
		parswap->remove(PRICING_DATA_TODAY);
		parswap->AQLObject::add(PRICING_DATA_TODAY, new AQLDataDate(dataProvider->mAsofDate));
		////underlyings
		//AQLString understrs = object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).convertToString();
		//parswap->AQLObject::add(CALIBRATION_DATA_UNDERLYINGS, new AQLDataMultiReference()).convertFromString(understrs);
		//currency
		dh = &(object.getData(PRICING_DATA_PREMIUMCURRENCY,ISNOTNULL));
		AQLString premcur = dynamic_cast<const AQLDataString &>(dh->get()).get();
		parswap->remove(PRICING_DATA_CURRENCY);
		parswap->AQLObject::add(PRICING_DATA_CURRENCY, new AQLDataString(premcur));
		//cfgenerator
		parswap->remove(PRICING_DATA_CFGENERATOR);
		parswap->AQLObject::add(PRICING_DATA_CFGENERATOR, new AQLDataProcedure()).convertFromString(FN_IR_CASHFLOWGENERATOR_STR);
		//resultoutput
		parswap->remove(PRICING_DATA_ISRESULTOUTPUT);
		parswap->AQLObject::add(PRICING_DATA_ISRESULTOUTPUT, new AQLDataBool(true));

		//even if basdate > fixing date(swaption maturity date), never throw error
		//that is why we must add PRICING_DATA_ISSAVEPASTFIXING
		parswap->remove(PRICING_DATA_ISSAVEPASTFIXING);
		parswap->AQLObject::add(PRICING_DATA_ISSAVEPASTFIXING, new AQLDataBool(true));

		
		//about theta
		//dh = &(e.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
		//if (dh->isDefined() && !dh->isNull())
		//{
		//	parswap->AQLObject::add(PRICING_DATA_ISSAVEPASTFIXING, dh->clone());
		//}	

		
		//value
		parswap->setValuationMethod(FN_IR_CONVERGENCEVALUE_STR);

		parswap->AQLObject::remove(PRICING_DATA_CONVERGENCETARGET);
		parswap->AQLObject::remove(PRICING_DATA_SUBVALUE);
		//find target leg
		if (dataProvider->mFixedLegNo == 0)
			parswap->AQLObject::add(PRICING_DATA_CONVERGENCETARGET, new AQLDataString()).convertFromString("LEG1FIXEDRATE");
		else
			parswap->AQLObject::add(PRICING_DATA_CONVERGENCETARGET, new AQLDataString()).convertFromString("LEG2FIXEDRATE");
		//sub value
		parswap->AQLObject::add(PRICING_DATA_SUBVALUE, new AQLDataValuation()).convertFromString(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
		//annuity calc mode
		parswap->AQLObject::add(PRICING_DATA_ISANNUITYCALC, new AQLDataBool(true));
		// omit fee from par rate calculation
		parswap->AQLObject::remove(CALIBRATION_DATA_FEEAMOUNTS);
		parswap->AQLObject::remove(CALIBRATION_DATA_FEEPAYMENTDATES);
		parswap->AQLObject::remove(CALIBRATION_DATA_FEECURRENCIES);
		parswap->AQLObject::remove(CALIBRATION_DATA_FEEDISCOUNTCURVES);
	}
	else
	{
		parswap	= &dynamic_cast<AQLMathObjectValue &>(objPool.getObject(tempname).get());
		//settledate
		dynamic_cast<AQLDataDate &>(parswap->getData(PRICING_DATA_SETTLEDATE, ISNOTNULL).get()).set(swap_basedate);
		//valuedate
		dynamic_cast<AQLDataDate &>(parswap->getData(PRICING_DATA_VALUEDATE, ISNOTNULL).get()).set(swap_basedate);
		//today
		dynamic_cast<AQLDataDate &>(parswap->getData(PRICING_DATA_TODAY, ISNOTNULL).get()).set(dataProvider->mAsofDate);
		//currency
		dh = &(object.getData(PRICING_DATA_PREMIUMCURRENCY,ISNOTNULL));
		AQLString premcur = dynamic_cast<const AQLDataString &>(dh->get()).get();
		dynamic_cast<AQLDataString &>(parswap->getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).set(premcur);
		
		AQLString pathname = dataProvider->mpvanilla->getName().get();
		parswap->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).convertFromString(pathname);
	}

	//send a iscalc risk message
	bool iscalcrisk = false;
	dh = &(object.getData(PRICING_DATA_ISCALCRISK, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	parswap->remove(PRICING_DATA_ISCALCRISK);
	parswap->AQLObject::add(PRICING_DATA_ISCALCRISK, new AQLDataBool(iscalcrisk));

	parswap->remove(PRICING_DATA_ISSETUPPAYOFF);
	parswap->AQLObject::add(PRICING_DATA_ISSETUPPAYOFF, new AQLDataBool(false));

	parswap->remove(PRICING_DATA_ISSTUBSETCOMPLETE);
	parswap->AQLObject::add(PRICING_DATA_ISSTUBSETCOMPLETE, new AQLDataBool(true));

	if (dataProvider->mIsCalcEquivStrike && orgswap != 0)
	{
		//set libor spreads
		AQLDataHolder* attpar = &(orgswap->getData(PRICING_DATA_ORIGINALLIBORSPREADNAMES, NOCHECK));
		if (attpar->isDefined() && !attpar->isNull())
		{
			const AQLStringVector& libornames = dynamic_cast<const AQLDataStrings &>(attpar->get()).get();
			for (unsigned int i = 0; i < libornames.size(); i++)
			{
				AQLObject& ecoupon = objPool.getObject(libornames[i], ENCHKTYPE_ISDEFINED).get();
				
				DoubleVector coeffs = dynamic_cast<AQLDataDoubles &>
										(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).get();
				if (coeffs.size() < 2)
					throw AQLCoreInvalidData("Original Libor Size Error",__FILE__,__LINE__);
				coeffs.back() = 0.0;
				dynamic_cast<AQLDataDoubles &>(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).set(coeffs);
			}
		}
		else 
		{
			attpar = &(orgswap->getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
			AQLObject& liborleg = dynamic_cast<const AQLDataMultiReference &>(attpar->get()).get(dataProvider->mFloatLegNo).get();
			attpar = &(liborleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
			const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference &>(attpar->get());
			unsigned int cSize = cashlets.getSize();

			AQLStringVector spreadnames;
			DoubleVector spreads;
			for (unsigned int i = 0; i < cSize; i++)
			{
				AQLObject& ecash = cashlets.get(i).get();
				attpar = &(ecash.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
				const AQLDataMultiReference& coupons = dynamic_cast<const AQLDataMultiReference &>(attpar->get());
				AQLObject& coupon = coupons.get(0).get();

				AQLString name = dynamic_cast<const AQLDataString &>(coupon.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
				

				attpar = &(coupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
				DoubleVector coeffs = dynamic_cast<const AQLDataDoubles &>(attpar->get()).get();
				if (coeffs.size() < 2)
					throw AQLCoreInvalidData("Coefficient Size Error",__FILE__,__LINE__);

				spreadnames.push_back(name);
				spreads.push_back(coeffs.back());
				coeffs.back() = 0.0;
				dynamic_cast<AQLDataDoubles &>(coupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).set(coeffs);
			}
			orgswap->AQLObject::remove(PRICING_DATA_ORIGINALLIBORSPREADNAMES);
			orgswap->AQLObject::add(PRICING_DATA_ORIGINALLIBORSPREADNAMES, new AQLDataStrings(spreadnames));
			orgswap->AQLObject::remove(PRICING_DATA_ORIGINALLIBORSPREADS);
			orgswap->AQLObject::add(PRICING_DATA_ORIGINALLIBORSPREADS, new AQLDataDoubles(spreads));
		}

		//set amorticase
		attpar = &(orgswap->getData(PRICING_DATA_ORIGINALNOTIONALNAMES, NOCHECK));
		if (attpar->isDefined() && !attpar->isNull())
		{
			const AQLStringVector& notionalnames = dynamic_cast<const AQLDataStrings &>(attpar->get()).get();
			
			//be careful about object
			attpar = &(parswap->getData(PRICING_DATA_AVERAGENOTIONAL,ISDEFINED));
			double avenotional = dynamic_cast<const AQLDataDouble &>(attpar->get()).get();
			for (unsigned int i = 0; i < notionalnames.size(); i++)
			{
				AQLObject& ecash = objPool.getObject(notionalnames[i], ENCHKTYPE_ISDEFINED).get();
				dynamic_cast<AQLDataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).set(avenotional);
			}
		}
		else 
		{
			double averagenotional = 0.0;
			attpar = &(parswap->getData(PRICING_DATA_AVERAGENOTIONAL, NOCHECK)); 
			if (!attpar->isDefined() || attpar->isNull())
			{
				double notonalamount = 0.0;
				int countnum = 0;
				attpar = &(orgswap->getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
				AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference &>(attpar->get());
				for (unsigned int i = 0; i < legs.getSize(); i++)
				{
					AQLObject& eleg = legs.get(i).get();
					attpar = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
					AQLDataMultiReference& cashlets = dynamic_cast<AQLDataMultiReference &>(attpar->get());
					for (unsigned int j = 0; j < cashlets.getSize(); j++)
					{
						AQLObject& ecash = cashlets.get(j).get();
						notonalamount += dynamic_cast<AQLDataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
						countnum++;
					}
				}

				if (0 == countnum)
					throw AQLCoreInvalidData("Notional Error",__FILE__,__LINE__);

				averagenotional = notonalamount / static_cast<double>(countnum);
				parswap->AQLObject::add(PRICING_DATA_AVERAGENOTIONAL, new AQLDataDouble(averagenotional));
			}
			else
			{
				averagenotional = dynamic_cast<const AQLDataDouble &>(attpar->get()).get();
				
			}
				
			
			AQLStringVector nationalnames;
			DoubleVector notionals;
			attpar = &(orgswap->getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
			AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference &>(attpar->get());
			for (unsigned int i = 0; i < legs.getSize(); i++)
			{
				AQLObject& eleg = legs.get(i).get();
				attpar = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
				AQLDataMultiReference& cashlets = dynamic_cast<AQLDataMultiReference &>(attpar->get());
				for (unsigned int j = 0; j < cashlets.getSize(); j++)
				{
					AQLObject& ecash = cashlets.get(j).get();
					AQLString name = dynamic_cast<const AQLDataString &>(ecash.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
					double notional = dynamic_cast<AQLDataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
				
					nationalnames.push_back(name);
					notionals.push_back(notional);

					//change and set
					dynamic_cast<AQLDataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).set(averagenotional);

				}
			}
			orgswap->AQLObject::remove(PRICING_DATA_ORIGINALNOTIONALNAMES);
			orgswap->AQLObject::add(PRICING_DATA_ORIGINALNOTIONALNAMES, new AQLDataStrings(nationalnames));
			orgswap->AQLObject::remove(PRICING_DATA_ORIGINALNOTIONALS);
			orgswap->AQLObject::add(PRICING_DATA_ORIGINALNOTIONALS, new AQLDataDoubles(notionals));
		}
	}
	//target pv target pv can be always changed
	parswap->AQLObject::remove(PRICING_DATA_DIRTYPRICE);
	parswap->AQLObject::add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(targetPV));
	double convergent = parswap->value(swap_basedate);
	
	if (dataProvider->mIsCalcEquivStrike && parswap != 0)
	{
		dh = &(parswap->getData(PRICING_DATA_CONVERGENCEVALUE, ISNOTNULL));
		equivstrike = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
		parswap->AQLObject::remove(PRICING_DATA_DIRTYPRICE);
		parswap->AQLObject::add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(0.0));
		convergent = parswap->value(dataProvider->mAsofDate);
		dh = &(parswap->getData(PRICING_DATA_AVERAGENOTIONAL,ISDEFINED));
		avenotional = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}

	dh = &(parswap->getData(PRICING_DATA_CONVERGENCEVALUE, ISNOTNULL));
	frate = dynamic_cast<const AQLDataDouble &>(dh->get()).get();

	//in case of exotic cashflow, negative rate can be considered;
	//if (dataProvider->mIsCalcEquivStrike && frate <= 0.0)
	//	frate = eps_NegativeFLibor;

	if (dataProvider->mIsIRRModel)
	{
		dh = &(parswap->getData(PRICING_DATA_ANNUITYTERMS, ISNOTNULL));
		const DoubleVector& termvec = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();

		dh = &(parswap->getData(PRICING_DATA_FIRSTNUMERAIREFORIRR, ISNOTNULL));
		double firstnumeraire = dynamic_cast<const AQLDataDouble &>(dh->get()).get();

		//irrmodel is unadjusted convention
		AQLString termstr = getFrequencyFromIndexGenerator(object, dataProvider->mFloatLegNo, true);
		termstr = termstr.subString(0,termstr.findString("M")-1);
		AQLDataDouble tmp;
		tmp.convertFromString(termstr);
		double unadjterm = tmp.get()/ 12.0;

		annuity = 0.0;
		double dfval = 1.0;
		for (unsigned int i = 0; i < termvec.size(); i++)
		{
			//notionalexchange cfcase
			if (0.0 == termvec[i])
				continue;

			dfval /= (1.0 + unadjterm * frate);
			annuity += dfval * unadjterm;
		
		}
		annuity *= firstnumeraire;
	}
	else
	{
		dh = &(parswap->getData(PRICING_DATA_ANNUITYVALUE, ISNOTNULL));
		annuity = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}
	
	// discounted by basis curve based on the option collateral currency
	const AQLMathYieldCurve& yc(dataProvider->mpvanilla->getIRCurve(dataProvider->mPremiumCurrency));
	double optDF = yc.getBasisDF(dataProvider->mAsofDate, swap_basedate);
	annuity *= optDF;

	return convergent;
}



//
//----------
//

// Output cashflow
void
AQLPriceIRSwaptionValueFromCashFlow::outputResult( AQLObject& object, AQLDataProvider* dp ) const
{
	// !Output Cashflow or OptionParams 
	AQLLinearRatesOptionValue::outputResult( object, dp );

	// ! Get valuable object
	AQLString nb = dynamic_cast<const AQLDataString& >(object.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();

	//AQLString tempname = "ChangeToSwap" + nb;
	AQLString tempname;
	AQLPriceIROptionValueDataProvider* dataProvider = dynamic_cast<AQLPriceIROptionValueDataProvider *>(dataProvider);
	AQLDataHolder* dh = &object.getData( PRICING_DATA_ISRESULTOUTPUT, NOCHECK );
	bool isResultOut = false;
	if (dh->isDefined() && !dh->isNull())
		isResultOut = dynamic_cast<AQLDataBool&>(dh->get()).get();
	if (dataProvider->mIsCalcEquivStrike || isResultOut)
		tempname = "ChangeToOriginalSwap" + nb;
	else
		tempname = "ChangeToSwap" + nb;
	
	AQLObjectPool& objPool = object.getDataInstance()->getObjectPool(); 
	AQLObjectHolder objHolder = objPool.getObject( tempname, ENCHKTYPE_NOCHECK );
	AQLMathObjectValue* vswaption = NULL;
	if ( objHolder.isDefined() )
		vswaption = dynamic_cast< AQLMathObjectValue* >( &( objHolder.get() ) );
	else
		throw AQLCoreInvalidData( "vswaption is not set! AQLPriceIRSwaptionValueFromCashFlow::outputResult() ",__FILE__,__LINE__);

	dh = &(vswaption->getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
	if (legs.getSize() != 2)
		throw AQLCoreInvalidData("LegSize must be 2 on Swaption underlying",__FILE__,__LINE__);


	for (unsigned int i = 0; i < 2; i++)
	{
		const int num = i + 1;
	
		// ! Set Cashlet to TradeEntity
		// ! CFCalcStartDate
		AQLString cfCalcStartDateName = PRICING_DATA_CFCALCSTARTDATE_LEG + AQLString( num );
		object.remove( cfCalcStartDateName );
		object.add( cfCalcStartDateName, new AQLDataDates() ).convertFromString( vswaption->getData( cfCalcStartDateName, ISDEFINED ).convertToString() );	
		
		// ! CFCalcEndDate
		AQLString cfCalcEndDateName = PRICING_DATA_CFCALCENDDATE_LEG + AQLString( num );
		object.remove( cfCalcEndDateName );
		object.add( cfCalcEndDateName, new AQLDataDates() ).convertFromString( vswaption->getData( cfCalcEndDateName, ISDEFINED ).convertToString() );

		// ! PaymentDate
		AQLString paymentDateName = PRICING_DATA_PAYMENTDATE_LEG + AQLString( num );
		object.remove( paymentDateName );
		object.add( paymentDateName, new AQLDataDates() ).convertFromString( vswaption->getData( paymentDateName, ISDEFINED ).convertToString() );
		
		// ! PaymentTime
		AQLString time = PRICING_DATA_CASHLETVALUETIME_LEG + AQLString( num );		
		object.remove(time);
		object.add( time, new AQLDataDoubles() ).convertFromString( vswaption->getData( time, ISDEFINED ).convertToString() );
		
		// ! Cachflow
		AQLString cfname = PRICING_DATA_CASHLETVALUE_LEG + AQLString( num );		
		object.remove( cfname );
		object.add( cfname, new AQLDataDoubles() ).convertFromString( vswaption->getData( cfname, ISDEFINED ).convertToString() );

		// ! PV
		AQLString pvname = PRICING_DATA_PVVALUE_LEG + AQLString( num );
		object.remove( pvname );
		object.add( pvname, new AQLDataDoubles() ).convertFromString( vswaption->getData( pvname, ISDEFINED ).convertToString() );

		// ! DF
		AQLString dfName = PRICING_DATA_DF_LEG + AQLString( num );
		object.remove( dfName );
		object.add( dfName, new AQLDataDoubles() ).convertFromString( vswaption->getData( dfName, ISDEFINED ).convertToString() );
			
		// ! Gearing
		AQLString gearingName = PRICING_DATA_GEARING_LEG + AQLString( num );
		object.remove( gearingName );
		object.add( gearingName, new AQLDataDoubles() ).convertFromString( vswaption->getData( gearingName, ISDEFINED ).convertToString() );
		
		// ! Forward
		AQLString forwardName = PRICING_DATA_FORWARD_LEG + AQLString( num );
		object.remove( forwardName );
		object.add( forwardName, new AQLDataDoubles() ).convertFromString( vswaption->getData( forwardName, ISDEFINED ).convertToString() );

		// ! Spreads
		AQLString spreadName = PRICING_DATA_SPREAD_LEG + AQLString( num );
		object.remove( spreadName );
		object.add( spreadName, new AQLDataDoubles() ).convertFromString( vswaption->getData( spreadName, ISDEFINED ).convertToString() );

		// ! AccrualDays
		AQLString accDaysName = PRICING_DATA_ACCRUALDAYS_LEG + AQLString( num );
		object.remove( accDaysName );
		object.add( accDaysName, new AQLDataDoubles() ).convertFromString( vswaption->getData( accDaysName, ISDEFINED ).convertToString() );

		// ! Notional
		AQLString notionalName = PRICING_CALIBRATION_DATAOTIONAL_LEG + AQLString( num );
		object.remove( notionalName );
		object.add( notionalName, new AQLDataDoubles() ).convertFromString( vswaption->getData( notionalName, ISDEFINED ).convertToString() );

		// ! FixingDate
		AQLString fixingDateName = PRICING_DATA_FIXINGDATE_LEG + AQLString( num );
		object.remove( fixingDateName );
		object.add( fixingDateName, new AQLDataDates() ).convertFromString( vswaption->getData( fixingDateName, ISDEFINED ).convertToString() );
	
	}
}