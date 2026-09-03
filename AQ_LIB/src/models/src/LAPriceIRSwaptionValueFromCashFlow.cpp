//  2008, AlgoQuantHub.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAPriceDataManager.h"
#include "LADataProcedure.h"
#include "LAMathDefine.h"
#include "LAMathDateCalculations.h"
#include "LAMathValuableEntity.h"
#include "LAPricePayOff.h"
#include "LAMathCurveFuncUtility.h"
#include "LAPriceIRSwaptionValueFromCashFlow.h"
#include "LAMathYieldCurve.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LAPriceCashFlowGenerator.h"
#include "LALinearRatesSwapTradeValue.h"
#include "LASplineInterpolation.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathIndexEntity.h"
#include "LAPriceConvergenceValue.h"

#define STD "STD"

using namespace std;

LAPriceIRSwaptionValueFromCashFlow::LAPriceIRSwaptionValueFromCashFlow()
: LAPriceIRSwaptionValue()
{}

//LAPriceIRSwaptionValueFromCashFlow::LAPriceIRSwaptionValueFromCashFlow(LAPriceIRSwaptionValue2& v)
//: LALinearRatesOptionValue(v)
//{}

LAPriceIRSwaptionValueFromCashFlow::~LAPriceIRSwaptionValueFromCashFlow()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAPriceIRSwaptionValueFromCashFlow::getType() const
{
	return FN_IR_SWAPTIONVALUEFROMCASHFLOW;
}
/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceIRSwaptionValueFromCashFlow::isTypeOf(function_t id) const
{
	return (id == FN_IR_SWAPTIONVALUEFROMCASHFLOW ? true : LAPriceIRSwaptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
LAString 
LAPriceIRSwaptionValueFromCashFlow::getOptionPayoffName() const
{
	return FN_IR_SWAPTIONVALUEFROMCASHFLOW_STR;
}


LACoreFunctionBase*
LAPriceIRSwaptionValueFromCashFlow::clone() const
{
    try 
	{
		return new LAPriceIRSwaptionValueFromCashFlow(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*					
LAPriceIRSwaptionValueFromCashFlow::setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const
{
	
	const LADataHolder* dh;
	LAPriceIROptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceIROptionValueDataProvider *>(LALinearRatesOptionValue::setUpDataProvider(basedate,object,att));
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
	const LADataMultiReference& refs = dynamic_cast<const LADataMultiReference &>(dh->get());
	if (refs.getSize() != 2)
		throw LACoreInvalidData("Swaption Underlyings error",__FILE__,__LINE__);
	LAObject& floatleg = refs.get(dataProvider->mFloatLegNo).get();

	//unit
	dataProvider->unit = dynamic_cast<const LADataDouble &>(refs.get(dataProvider->mFloatLegNo).get().getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();

	//check detail
	dh = &(floatleg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
	LAString inputtype = dynamic_cast<const LADataString &>(dh->get()).get();
	if (inputtype.toUpper() != "MANUAL")
		throw LACoreInvalidData("Swaption CashFlowMode error",__FILE__,__LINE__);

	dataProvider->mUnFrequency = getFrequencyFromIndexGenerator(object, dataProvider->mFloatLegNo);

	dataProvider->mUnTenor.resize(1);
	dataProvider->mUnTenor[0] = getNearestTenorString(floatleg, dataProvider->mUnFrequency);

	// get DiscountCurve
	dataProvider->mDCurveType = STD;
	dh = &(floatleg.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mDCurveType = dynamic_cast<const LADataString &>(dh->get()).get();
	}


	//mFCurveTypes must exist for irsabr calibration
	dataProvider->mFCurveTypes.resize(1);
	dataProvider->mFCurveTypes[0] = STD;
	const LAObject& eindex = getLiborEntity(object, dataProvider->mFloatLegNo);
	dh = &(eindex.getData(PRICING_DATA_BASISCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mFCurveTypes[0] = dynamic_cast<const LADataString &>(dh->get()).get();
	}
	//forward interpolation
	dh = &(eindex.getData(PRICING_DATA_ISFWDINTERPOLATION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mIsFWDInter = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	//strike
	dh = &(object.getData(PRICING_DATA_STRIKE, ISNOTNULL));
	double strike = dynamic_cast<const LADataDouble &>(dh->get()).get();
	std::vector<AnalyticBKParam *> bkparam(dataProvider->mParam[0].size());
	//volatility underlying
	dh = &(object.getData(PRICING_DATA_VOLATILITYUNDERLYING, ISNOTNULL));
	dataProvider->mUnderlying.resize(1);
	dataProvider->mUnderlying[0] = dynamic_cast<const LADataString &>(dh->get()).get();

	for (unsigned int i = 0; i < bkparam.size(); i++)
	{
		bkparam[i] = dynamic_cast<AnalyticBKParam *>(dataProvider->mParam[0][i]);
		bkparam[i]->K = strike;
	}

	//is calc equivalent strike
	dh = &(object.getData(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->mIsCalcEquivStrike = dynamic_cast<const LADataBool &>(dh->get()).get();

	dh = &(object.getData(PRICING_DATA_ISIRRMODEL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->mIsIRRModel = dynamic_cast<const LADataBool &>(dh->get()).get();

	dh = &(object.getData(PRICING_DATA_PREMIUMPAYMENTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mIsPremAdjust = true;

		dataProvider->mPremPayDate = dynamic_cast<const LADataDate &>(dh->get()).get();
		dataProvider->mPremAmount = 0.0;
		dh = &(object.getData(PRICING_DATA_PREMIUMAMOUT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			dataProvider->mPremAmount = dynamic_cast<const LADataDouble &>(dh->get()).get();

		dh = &(object.getData(PRICING_DATA_PREMIUMPAYCURRENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			LAString chkccy = dynamic_cast<const LADataString &>(dh->get()).get();
			if (chkccy.toUpper () != dataProvider->mpvcur)
				throw LACoreInvalidData("PremiumCurrency Error",__FILE__,__LINE__); 
		}

		dataProvider->mIsAddFwdPremPV = false;
		dh = &(object.getData( PRICING_DATA_ISADDFWDPREMPV, NOCHECK ));
		if (dh->isDefined() && !dh->isNull())
			dataProvider->mIsAddFwdPremPV = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	dh = &(object.getData(PRICING_DATA_CASHSETTLEMENTPAYMENTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mIsCashSettlementAdjust = true;

		dataProvider->mCashSettlementPayDate = dynamic_cast<const LADataDate &>(dh->get()).get();
		
		dataProvider->mCashSettlementAmount = 0.0;
		dh = &(object.getData(PRICING_DATA_CASHSETTLEMENTAMOUNT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			dataProvider->mCashSettlementAmount = dynamic_cast<const LADataDouble &>(dh->get()).get();
	}


	return dataProvider;
}


void
LAPriceIRSwaptionValueFromCashFlow::setUpAnalyticParam(LAObject& object, LADataProvider* dp) const
{
	LAPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dp);
	
	//even if maturity date has passed, set up bkparam for obtaining the temporary volatility, forward
	
	LADataInstance* dataInstance = object.getDataInstance(); 
	LADataHolder*dh;

	LADate mdymatudate = (dataProvider->mAsofDate < dataProvider->mMaturityDate) ? dataProvider->mMaturityDate : dataProvider->mAsofDate;
	std::vector <AnalyticBKParam *> bkparam(dataProvider->mParam[0].size());
	
	LAPriceDataDayCount dc_act365(ACT_365_ISDA);
	
	double te = dataProvider->mBlackDayCount.getTerm(dataProvider->mAsofDate,mdymatudate,true);
	double actt = dc_act365.getTerm(dataProvider->mAsofDate, mdymatudate,true);
	double forward = 0.0;
	double nu = 1.0;
	double equivstrike = 0.0;
	double avenotional = 0.0;
	double converror = getCalcDataFromCashFlow(object,dataProvider, forward, nu, equivstrike, avenotional);
//#ifdef ZEROFLOOR
//	forward = LAMath::max(forward, MIN_RATE);
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


LAString 
LAPriceIRSwaptionValueFromCashFlow::getFrequencyFromIndexGenerator(const LAObject& object, unsigned int floatLegNo, bool isMonthString) const
{
	const LAObject& eindex = getLiborEntity(object, floatLegNo);

	//accessory
	const LADataHolder* dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
	LAString libormonth = dynamic_cast<const LADataString &>(dh->get()).get();
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(libormonth, y, m, d, w);
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
			throw LACoreInvalidData("Accessory Error",__FILE__,__LINE__);
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
			throw LACoreInvalidData("Accessory Error",__FILE__,__LINE__);
	}
}

//get Libor Object
const LAObject& 
LAPriceIRSwaptionValueFromCashFlow::getLiborEntity(const LAObject& object, unsigned int floatLegNo) const
{
	//LAPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dataProvider);

	//this object is swaption trade object not indexgenerator object.
	const LADataHolder* dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference &>(dh->get());
	//unsigned int floatLegNo = dataProvider->mFloatLegNo;
	const LAObject& liborleg = legs.get(floatLegNo).get();

	dh = &(liborleg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
	LAString inputtype = dynamic_cast<const LADataString &>(dh->get()).get();
	if (inputtype.toUpper() != "MANUAL")
		throw LACoreInvalidData("getFrequencyFromSwaption error", __FILE__, __LINE__);

	dh = &(liborleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference &>(dh->get());
	
	for (unsigned int i = 0; i < cashlets.getSize(); i++)
	{
		const LAObject& ecash = cashlets.get(i).get();

		//for notional exchange
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		const LADataMultiReference& coupons = dynamic_cast<const LADataMultiReference &>(dh->get());
		const LAObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			throw LACoreInvalidData("getFrequencyFromSwaption error", __FILE__, __LINE__);
		
		const LADataMultiReference& indexs = dynamic_cast<const LADataMultiReference &>(dh->get());
		const LAObject& eindex = indexs.get(0).get();

		dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
		LAString indextype = dynamic_cast<const LADataString &>(dh->get()).get();
		if (indextype.toUpper() == "LIBOR")
		{
			return eindex;
		}
	}

	//if we can not find Libor object throw Error;
	throw LACoreInvalidData("getFrequencyFromSwaption error", __FILE__, __LINE__);
}

//get float leg number
unsigned int
LAPriceIRSwaptionValueFromCashFlow::getFloatLegNum(const LAObject& object) const
{
	//VirIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dataProvider);
	const LADataHolder* dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& refs = dynamic_cast<const LADataMultiReference &>(dh->get());
	if (refs.getSize() != 2)
		throw LACoreInvalidData("Swaption Underlyings error",__FILE__,__LINE__);
	const LAObject& leg1 = refs.get(0).get();

	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference &>(dh->get());
	unsigned int cSize = cashlets.getSize();
	for (unsigned int i = 0; i < cSize; i++)
	{
		const LAObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;
			

		const LADataMultiReference& coupons = dynamic_cast<const LADataMultiReference &>(dh->get());
		const LAObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
		const LADataMultiReference& indexs = dynamic_cast<const LADataMultiReference &>(dh->get());
		const LAObject& eindex = indexs.get(0).get();

		dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
		LAString indextype = dynamic_cast<const LADataString &>(dh->get()).get();
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
LAPriceIRSwaptionValueFromCashFlow::getCalcDataFromCashFlow(const LAObject& object, LADataProvider* dp, double& frate, double& annuity, double& equivstrike, double& avenotional) const
{
	//the below is main source of swaption
	const LADataHolder*dh;
	LAPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dp);

	LADataInstance* dataInstance = object.getDataInstance();
	LAObjectPool& objPool = dataInstance->getObjectPool();
	
	LAString nb = dynamic_cast<const LADataString& >(object.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	double targetPV = 0.0;
	LAMathObjectValue* orgswap =NULL;
	LADate swap_basedate = dataProvider->mMaturityDate;
	if (dataProvider->mMaturityDate < dataProvider->mAsofDate)
	{
		swap_basedate = dataProvider->mAsofDate;
	}
	bool isResultOut = false;
	dh = &object.getData( PRICING_DATA_ISRESULTOUTPUT, NOCHECK );
	if (dh->isDefined() && !dh->isNull())
	{
		isResultOut = dynamic_cast<const LADataBool&>(dh->get()).get();
	}
	if (dataProvider->mIsCalcEquivStrike || isResultOut)
	{
		LAString orgname = "ChangeToOriginalSwap" + nb;
		const LAObjectHolder ehOrg = objPool.getObject(orgname);
		if(!ehOrg.isDefined())
		{	
			//orgswap= new LAMathObjectValue(object.getDataInstance());
			orgswap= dynamic_cast<LAMathObjectValue *>(object.clone());
			objPool.set(orgname,orgswap);
			//name
			orgswap->getName().convertFromString(orgname);
			////path object
			LAString pathname = dataProvider->mpvanilla->getName().get();
			orgswap->remove(PRICING_DATA_PATHENTITY);
			orgswap->LAObject::add(PRICING_DATA_PATHENTITY, new LADataReference()).convertFromString(pathname);
			//isdetailoutput
			orgswap->remove(PRICING_DATA_ISDETAILOUTPUT);
			orgswap->LAObject::add(PRICING_DATA_ISDETAILOUTPUT, new LADataString()).convertFromString("TRUE");
			//settledate
			orgswap->remove(PRICING_DATA_SETTLEDATE);
			orgswap->LAObject::add(PRICING_DATA_SETTLEDATE, new LADataDate(swap_basedate));
			//valuedate
			orgswap->remove(PRICING_DATA_VALUEDATE);
			orgswap->LAObject::add(PRICING_DATA_VALUEDATE, new LADataDate(swap_basedate));
			//today
			orgswap->remove(PRICING_DATA_TODAY);
			orgswap->LAObject::add(PRICING_DATA_TODAY, new LADataDate(dataProvider->mAsofDate));
			//underlyings
			//LAString understrs = object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).convertToString();
			//orgswap->LAObject::add(CALIBRATION_DATA_UNDERLYINGS, new LADataMultiReference()).convertFromString(understrs);
			//currency
			dh = &(object.getData(PRICING_DATA_PREMIUMCURRENCY,ISNOTNULL));
			LAString premcur = dynamic_cast<const LADataString &>(dh->get()).get();
			orgswap->remove(PRICING_DATA_CURRENCY);
			orgswap->LAObject::add(PRICING_DATA_CURRENCY, new LADataString(premcur));
			////cfgenerator
			orgswap->remove(PRICING_DATA_CFGENERATOR);
			orgswap->LAObject::add(PRICING_DATA_CFGENERATOR, new LADataProcedure()).convertFromString(FN_IR_CASHFLOWGENERATOR_STR);
			//resultoutput
			orgswap->remove(PRICING_DATA_ISRESULTOUTPUT);
			orgswap->LAObject::add(PRICING_DATA_ISRESULTOUTPUT, new LADataBool(true));

			//even if basdate > fixing date(swaption maturity date), never throw error
			//that is why we must add PRICING_DATA_ISSAVEPASTFIXING
			orgswap->remove(PRICING_DATA_ISSAVEPASTFIXING);
			orgswap->LAObject::add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(true));
				
			//about theta
			//dh = &(e.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
			//if (dh->isDefined() && !dh->isNull())
			//{
			//	parswap->LAObject::add(PRICING_DATA_ISSAVEPASTFIXING, dh->clone());
			//}	

			//the below is difference of parswap
			orgswap->setValuationMethod(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
		}
		else
		{
			orgswap	= &dynamic_cast<LAMathObjectValue &>(objPool.getObject(orgname).get());
			//settledate
			dynamic_cast<LADataDate &>(orgswap->getData(PRICING_DATA_SETTLEDATE, ISNOTNULL).get()).set(swap_basedate);
			//valuedate
			dynamic_cast<LADataDate &>(orgswap->getData(PRICING_DATA_VALUEDATE, ISNOTNULL).get()).set(swap_basedate);
			//today
			dynamic_cast<LADataDate &>(orgswap->getData(PRICING_DATA_TODAY, ISNOTNULL).get()).set(dataProvider->mAsofDate);
			//currency
			dh = &(object.getData(PRICING_DATA_PREMIUMCURRENCY,ISNOTNULL));
			LAString premcur = dynamic_cast<const LADataString &>(dh->get()).get();
			dynamic_cast<LADataString &>(orgswap->getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).set(premcur);
			
			LAString pathname = dataProvider->mpvanilla->getName().get();
			orgswap->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).convertFromString(pathname);
		}

		orgswap->remove(PRICING_DATA_ISSETUPPAYOFF);
		orgswap->LAObject::add(PRICING_DATA_ISSETUPPAYOFF, new LADataBool(false));

		//in case of libor spreads mode
		dh = &(orgswap->getData(PRICING_DATA_ORIGINALLIBORSPREADNAMES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LAStringVector& libornames = dynamic_cast<const LADataStrings &>(dh->get()).get();

			dh = &(orgswap->getData(PRICING_DATA_ORIGINALLIBORSPREADS, ISNOTNULL));
			DoubleVector spreads = dynamic_cast<const LADataDoubles &>(dh->get()).get();
			if (libornames.size() != spreads.size())
				throw LACoreInvalidData("Original Libor Size Error",__FILE__,__LINE__);
			
			for (unsigned int i = 0; i < libornames.size(); i++)
			{
				LAObject& ecoupon = objPool.getObject(libornames[i], ENCHKTYPE_ISDEFINED).get();
				
				DoubleVector coeffs = dynamic_cast<LADataDoubles &>
										(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).get();
				if (coeffs.size() < 2)
					throw LACoreInvalidData("Original Libor Size Error",__FILE__,__LINE__);
				coeffs.back() = spreads[i];
				dynamic_cast<LADataDoubles &>(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).set(coeffs);
			}
		}
		//in case of amortize mode
		dh = &(orgswap->getData(PRICING_DATA_ORIGINALNOTIONALNAMES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LAStringVector& notionalnames = dynamic_cast<const LADataStrings &>(dh->get()).get();

			dh = &(orgswap->getData(PRICING_DATA_ORIGINALNOTIONALS, ISNOTNULL));
			DoubleVector notionals = dynamic_cast<const LADataDoubles &>(dh->get()).get();
			if (notionalnames.size() != notionals.size())
				throw LACoreInvalidData("Original Notional Size Error",__FILE__,__LINE__);
			
			for (unsigned int i = 0; i < notionalnames.size(); i++)
			{
				LAObject& ecash = objPool.getObject(notionalnames[i], ENCHKTYPE_ISDEFINED).get();
				double notional = dynamic_cast<LADataDouble &>
										(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
				notional = notionals[i];
				dynamic_cast<LADataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).set(notional);
			}
		}
		orgswap->value(swap_basedate);
		if (dataProvider->mIsCalcEquivStrike)
		{
			dh = &(orgswap->getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL));
			targetPV = dynamic_cast<const LADataDouble&>(dh->get()).get(); 
		}
	}
	

	LAString tempname = "ChangeToSwap" + nb;
	LAMathObjectValue* parswap =NULL;
	const LAObjectHolder ehCur = objPool.getObject(tempname);
	if(!ehCur.isDefined())
	{	
		//parswap= new LAMathObjectValue(object.getDataInstance());
		parswap= dynamic_cast<LAMathObjectValue *>(object.clone());
		objPool.set(tempname,parswap);
		//name
		parswap->getName().convertFromString(tempname);
		//path object
		LAString pathname = dataProvider->mpvanilla->getName().get();
		parswap->remove(PRICING_DATA_PATHENTITY);
		parswap->LAObject::add(PRICING_DATA_PATHENTITY, new LADataReference()).convertFromString(pathname);
		//isdetailoutput
		parswap->remove(PRICING_DATA_ISDETAILOUTPUT);
		parswap->LAObject::add(PRICING_DATA_ISDETAILOUTPUT, new LADataString()).convertFromString("TRUE");
		//settledate
		parswap->remove(PRICING_DATA_SETTLEDATE);
		parswap->LAObject::add(PRICING_DATA_SETTLEDATE, new LADataDate(swap_basedate));
		//valuedate
		parswap->remove(PRICING_DATA_VALUEDATE);
		parswap->LAObject::add(PRICING_DATA_VALUEDATE, new LADataDate(swap_basedate));
		//today
		parswap->remove(PRICING_DATA_TODAY);
		parswap->LAObject::add(PRICING_DATA_TODAY, new LADataDate(dataProvider->mAsofDate));
		////underlyings
		//LAString understrs = object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).convertToString();
		//parswap->LAObject::add(CALIBRATION_DATA_UNDERLYINGS, new LADataMultiReference()).convertFromString(understrs);
		//currency
		dh = &(object.getData(PRICING_DATA_PREMIUMCURRENCY,ISNOTNULL));
		LAString premcur = dynamic_cast<const LADataString &>(dh->get()).get();
		parswap->remove(PRICING_DATA_CURRENCY);
		parswap->LAObject::add(PRICING_DATA_CURRENCY, new LADataString(premcur));
		//cfgenerator
		parswap->remove(PRICING_DATA_CFGENERATOR);
		parswap->LAObject::add(PRICING_DATA_CFGENERATOR, new LADataProcedure()).convertFromString(FN_IR_CASHFLOWGENERATOR_STR);
		//resultoutput
		parswap->remove(PRICING_DATA_ISRESULTOUTPUT);
		parswap->LAObject::add(PRICING_DATA_ISRESULTOUTPUT, new LADataBool(true));

		//even if basdate > fixing date(swaption maturity date), never throw error
		//that is why we must add PRICING_DATA_ISSAVEPASTFIXING
		parswap->remove(PRICING_DATA_ISSAVEPASTFIXING);
		parswap->LAObject::add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(true));

		
		//about theta
		//dh = &(e.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
		//if (dh->isDefined() && !dh->isNull())
		//{
		//	parswap->LAObject::add(PRICING_DATA_ISSAVEPASTFIXING, dh->clone());
		//}	

		
		//value
		parswap->setValuationMethod(FN_IR_CONVERGENCEVALUE_STR);

		parswap->LAObject::remove(PRICING_DATA_CONVERGENCETARGET);
		parswap->LAObject::remove(PRICING_DATA_SUBVALUE);
		//find target leg
		if (dataProvider->mFixedLegNo == 0)
			parswap->LAObject::add(PRICING_DATA_CONVERGENCETARGET, new LADataString()).convertFromString("LEG1FIXEDRATE");
		else
			parswap->LAObject::add(PRICING_DATA_CONVERGENCETARGET, new LADataString()).convertFromString("LEG2FIXEDRATE");
		//sub value
		parswap->LAObject::add(PRICING_DATA_SUBVALUE, new LADataValuation()).convertFromString(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
		//annuity calc mode
		parswap->LAObject::add(PRICING_DATA_ISANNUITYCALC, new LADataBool(true));
		// omit fee from par rate calculation
		parswap->LAObject::remove(CALIBRATION_DATA_FEEAMOUNTS);
		parswap->LAObject::remove(CALIBRATION_DATA_FEEPAYMENTDATES);
		parswap->LAObject::remove(CALIBRATION_DATA_FEECURRENCIES);
		parswap->LAObject::remove(CALIBRATION_DATA_FEEDISCOUNTCURVES);
	}
	else
	{
		parswap	= &dynamic_cast<LAMathObjectValue &>(objPool.getObject(tempname).get());
		//settledate
		dynamic_cast<LADataDate &>(parswap->getData(PRICING_DATA_SETTLEDATE, ISNOTNULL).get()).set(swap_basedate);
		//valuedate
		dynamic_cast<LADataDate &>(parswap->getData(PRICING_DATA_VALUEDATE, ISNOTNULL).get()).set(swap_basedate);
		//today
		dynamic_cast<LADataDate &>(parswap->getData(PRICING_DATA_TODAY, ISNOTNULL).get()).set(dataProvider->mAsofDate);
		//currency
		dh = &(object.getData(PRICING_DATA_PREMIUMCURRENCY,ISNOTNULL));
		LAString premcur = dynamic_cast<const LADataString &>(dh->get()).get();
		dynamic_cast<LADataString &>(parswap->getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).set(premcur);
		
		LAString pathname = dataProvider->mpvanilla->getName().get();
		parswap->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).convertFromString(pathname);
	}

	//send a iscalc risk message
	bool iscalcrisk = false;
	dh = &(object.getData(PRICING_DATA_ISCALCRISK, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const LADataBool&>(dh->get()).get();
	parswap->remove(PRICING_DATA_ISCALCRISK);
	parswap->LAObject::add(PRICING_DATA_ISCALCRISK, new LADataBool(iscalcrisk));

	parswap->remove(PRICING_DATA_ISSETUPPAYOFF);
	parswap->LAObject::add(PRICING_DATA_ISSETUPPAYOFF, new LADataBool(false));

	parswap->remove(PRICING_DATA_ISSTUBSETCOMPLETE);
	parswap->LAObject::add(PRICING_DATA_ISSTUBSETCOMPLETE, new LADataBool(true));

	if (dataProvider->mIsCalcEquivStrike && orgswap != 0)
	{
		//set libor spreads
		LADataHolder* attpar = &(orgswap->getData(PRICING_DATA_ORIGINALLIBORSPREADNAMES, NOCHECK));
		if (attpar->isDefined() && !attpar->isNull())
		{
			const LAStringVector& libornames = dynamic_cast<const LADataStrings &>(attpar->get()).get();
			for (unsigned int i = 0; i < libornames.size(); i++)
			{
				LAObject& ecoupon = objPool.getObject(libornames[i], ENCHKTYPE_ISDEFINED).get();
				
				DoubleVector coeffs = dynamic_cast<LADataDoubles &>
										(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).get();
				if (coeffs.size() < 2)
					throw LACoreInvalidData("Original Libor Size Error",__FILE__,__LINE__);
				coeffs.back() = 0.0;
				dynamic_cast<LADataDoubles &>(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).set(coeffs);
			}
		}
		else 
		{
			attpar = &(orgswap->getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
			LAObject& liborleg = dynamic_cast<const LADataMultiReference &>(attpar->get()).get(dataProvider->mFloatLegNo).get();
			attpar = &(liborleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
			const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference &>(attpar->get());
			unsigned int cSize = cashlets.getSize();

			LAStringVector spreadnames;
			DoubleVector spreads;
			for (unsigned int i = 0; i < cSize; i++)
			{
				LAObject& ecash = cashlets.get(i).get();
				attpar = &(ecash.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
				const LADataMultiReference& coupons = dynamic_cast<const LADataMultiReference &>(attpar->get());
				LAObject& coupon = coupons.get(0).get();

				LAString name = dynamic_cast<const LADataString &>(coupon.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
				

				attpar = &(coupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
				DoubleVector coeffs = dynamic_cast<const LADataDoubles &>(attpar->get()).get();
				if (coeffs.size() < 2)
					throw LACoreInvalidData("Coefficient Size Error",__FILE__,__LINE__);

				spreadnames.push_back(name);
				spreads.push_back(coeffs.back());
				coeffs.back() = 0.0;
				dynamic_cast<LADataDoubles &>(coupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).set(coeffs);
			}
			orgswap->LAObject::remove(PRICING_DATA_ORIGINALLIBORSPREADNAMES);
			orgswap->LAObject::add(PRICING_DATA_ORIGINALLIBORSPREADNAMES, new LADataStrings(spreadnames));
			orgswap->LAObject::remove(PRICING_DATA_ORIGINALLIBORSPREADS);
			orgswap->LAObject::add(PRICING_DATA_ORIGINALLIBORSPREADS, new LADataDoubles(spreads));
		}

		//set amorticase
		attpar = &(orgswap->getData(PRICING_DATA_ORIGINALNOTIONALNAMES, NOCHECK));
		if (attpar->isDefined() && !attpar->isNull())
		{
			const LAStringVector& notionalnames = dynamic_cast<const LADataStrings &>(attpar->get()).get();
			
			//be careful about object
			attpar = &(parswap->getData(PRICING_DATA_AVERAGENOTIONAL,ISDEFINED));
			double avenotional = dynamic_cast<const LADataDouble &>(attpar->get()).get();
			for (unsigned int i = 0; i < notionalnames.size(); i++)
			{
				LAObject& ecash = objPool.getObject(notionalnames[i], ENCHKTYPE_ISDEFINED).get();
				dynamic_cast<LADataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).set(avenotional);
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
				LADataMultiReference& legs = dynamic_cast<LADataMultiReference &>(attpar->get());
				for (unsigned int i = 0; i < legs.getSize(); i++)
				{
					LAObject& eleg = legs.get(i).get();
					attpar = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
					LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference &>(attpar->get());
					for (unsigned int j = 0; j < cashlets.getSize(); j++)
					{
						LAObject& ecash = cashlets.get(j).get();
						notonalamount += dynamic_cast<LADataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
						countnum++;
					}
				}

				if (0 == countnum)
					throw LACoreInvalidData("Notional Error",__FILE__,__LINE__);

				averagenotional = notonalamount / static_cast<double>(countnum);
				parswap->LAObject::add(PRICING_DATA_AVERAGENOTIONAL, new LADataDouble(averagenotional));
			}
			else
			{
				averagenotional = dynamic_cast<const LADataDouble &>(attpar->get()).get();
				
			}
				
			
			LAStringVector nationalnames;
			DoubleVector notionals;
			attpar = &(orgswap->getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
			LADataMultiReference& legs = dynamic_cast<LADataMultiReference &>(attpar->get());
			for (unsigned int i = 0; i < legs.getSize(); i++)
			{
				LAObject& eleg = legs.get(i).get();
				attpar = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
				LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference &>(attpar->get());
				for (unsigned int j = 0; j < cashlets.getSize(); j++)
				{
					LAObject& ecash = cashlets.get(j).get();
					LAString name = dynamic_cast<const LADataString &>(ecash.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
					double notional = dynamic_cast<LADataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
				
					nationalnames.push_back(name);
					notionals.push_back(notional);

					//change and set
					dynamic_cast<LADataDouble &>(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).set(averagenotional);

				}
			}
			orgswap->LAObject::remove(PRICING_DATA_ORIGINALNOTIONALNAMES);
			orgswap->LAObject::add(PRICING_DATA_ORIGINALNOTIONALNAMES, new LADataStrings(nationalnames));
			orgswap->LAObject::remove(PRICING_DATA_ORIGINALNOTIONALS);
			orgswap->LAObject::add(PRICING_DATA_ORIGINALNOTIONALS, new LADataDoubles(notionals));
		}
	}
	//target pv target pv can be always changed
	parswap->LAObject::remove(PRICING_DATA_DIRTYPRICE);
	parswap->LAObject::add(PRICING_DATA_DIRTYPRICE, new LADataDouble(targetPV));
	double convergent = parswap->value(swap_basedate);
	
	if (dataProvider->mIsCalcEquivStrike && parswap != 0)
	{
		dh = &(parswap->getData(PRICING_DATA_CONVERGENCEVALUE, ISNOTNULL));
		equivstrike = dynamic_cast<const LADataDouble &>(dh->get()).get();
		parswap->LAObject::remove(PRICING_DATA_DIRTYPRICE);
		parswap->LAObject::add(PRICING_DATA_DIRTYPRICE, new LADataDouble(0.0));
		convergent = parswap->value(dataProvider->mAsofDate);
		dh = &(parswap->getData(PRICING_DATA_AVERAGENOTIONAL,ISDEFINED));
		avenotional = dynamic_cast<const LADataDouble &>(dh->get()).get();
	}

	dh = &(parswap->getData(PRICING_DATA_CONVERGENCEVALUE, ISNOTNULL));
	frate = dynamic_cast<const LADataDouble &>(dh->get()).get();

	//in case of exotic cashflow, negative rate can be considered;
	//if (dataProvider->mIsCalcEquivStrike && frate <= 0.0)
	//	frate = eps_NegativeFLibor;

	if (dataProvider->mIsIRRModel)
	{
		dh = &(parswap->getData(PRICING_DATA_ANNUITYTERMS, ISNOTNULL));
		const DoubleVector& termvec = dynamic_cast<const LADataDoubles &>(dh->get()).get();

		dh = &(parswap->getData(PRICING_DATA_FIRSTNUMERAIREFORIRR, ISNOTNULL));
		double firstnumeraire = dynamic_cast<const LADataDouble &>(dh->get()).get();

		//irrmodel is unadjusted convention
		LAString termstr = getFrequencyFromIndexGenerator(object, dataProvider->mFloatLegNo, true);
		termstr = termstr.subString(0,termstr.findString("M")-1);
		LADataDouble tmp;
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
		annuity = dynamic_cast<const LADataDouble &>(dh->get()).get();
	}
	
	// discounted by basis curve based on the option collateral currency
	const LAMathYieldCurve& yc(dataProvider->mpvanilla->getIRCurve(dataProvider->mPremiumCurrency));
	double optDF = yc.getBasisDF(dataProvider->mAsofDate, swap_basedate);
	annuity *= optDF;

	return convergent;
}



//
//----------
//

// Output cashflow
void
LAPriceIRSwaptionValueFromCashFlow::outputResult( LAObject& object, LADataProvider* dp ) const
{
	// !Output Cashflow or OptionParams 
	LALinearRatesOptionValue::outputResult( object, dp );

	// ! Get valuable object
	LAString nb = dynamic_cast<const LADataString& >(object.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();

	//LAString tempname = "ChangeToSwap" + nb;
	LAString tempname;
	LAPriceIROptionValueDataProvider* dataProvider = dynamic_cast<LAPriceIROptionValueDataProvider *>(dataProvider);
	LADataHolder* dh = &object.getData( PRICING_DATA_ISRESULTOUTPUT, NOCHECK );
	bool isResultOut = false;
	if (dh->isDefined() && !dh->isNull())
		isResultOut = dynamic_cast<LADataBool&>(dh->get()).get();
	if (dataProvider->mIsCalcEquivStrike || isResultOut)
		tempname = "ChangeToOriginalSwap" + nb;
	else
		tempname = "ChangeToSwap" + nb;
	
	LAObjectPool& objPool = object.getDataInstance()->getObjectPool(); 
	LAObjectHolder objHolder = objPool.getObject( tempname, ENCHKTYPE_NOCHECK );
	LAMathObjectValue* vswaption = NULL;
	if ( objHolder.isDefined() )
		vswaption = dynamic_cast< LAMathObjectValue* >( &( objHolder.get() ) );
	else
		throw LACoreInvalidData( "vswaption is not set! LAPriceIRSwaptionValueFromCashFlow::outputResult() ",__FILE__,__LINE__);

	dh = &(vswaption->getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference &>(dh->get());
	if (legs.getSize() != 2)
		throw LACoreInvalidData("LegSize must be 2 on Swaption underlying",__FILE__,__LINE__);


	for (unsigned int i = 0; i < 2; i++)
	{
		const int num = i + 1;
	
		// ! Set Cashlet to TradeEntity
		// ! CFCalcStartDate
		LAString cfCalcStartDateName = PRICING_DATA_CFCALCSTARTDATE_LEG + LAString( num );
		object.remove( cfCalcStartDateName );
		object.add( cfCalcStartDateName, new LADataDates() ).convertFromString( vswaption->getData( cfCalcStartDateName, ISDEFINED ).convertToString() );	
		
		// ! CFCalcEndDate
		LAString cfCalcEndDateName = PRICING_DATA_CFCALCENDDATE_LEG + LAString( num );
		object.remove( cfCalcEndDateName );
		object.add( cfCalcEndDateName, new LADataDates() ).convertFromString( vswaption->getData( cfCalcEndDateName, ISDEFINED ).convertToString() );

		// ! PaymentDate
		LAString paymentDateName = PRICING_DATA_PAYMENTDATE_LEG + LAString( num );
		object.remove( paymentDateName );
		object.add( paymentDateName, new LADataDates() ).convertFromString( vswaption->getData( paymentDateName, ISDEFINED ).convertToString() );
		
		// ! PaymentTime
		LAString time = PRICING_DATA_CASHLETVALUETIME_LEG + LAString( num );		
		object.remove(time);
		object.add( time, new LADataDoubles() ).convertFromString( vswaption->getData( time, ISDEFINED ).convertToString() );
		
		// ! Cachflow
		LAString cfname = PRICING_DATA_CASHLETVALUE_LEG + LAString( num );		
		object.remove( cfname );
		object.add( cfname, new LADataDoubles() ).convertFromString( vswaption->getData( cfname, ISDEFINED ).convertToString() );

		// ! PV
		LAString pvname = PRICING_DATA_PVVALUE_LEG + LAString( num );
		object.remove( pvname );
		object.add( pvname, new LADataDoubles() ).convertFromString( vswaption->getData( pvname, ISDEFINED ).convertToString() );

		// ! DF
		LAString dfName = PRICING_DATA_DF_LEG + LAString( num );
		object.remove( dfName );
		object.add( dfName, new LADataDoubles() ).convertFromString( vswaption->getData( dfName, ISDEFINED ).convertToString() );
			
		// ! Gearing
		LAString gearingName = PRICING_DATA_GEARING_LEG + LAString( num );
		object.remove( gearingName );
		object.add( gearingName, new LADataDoubles() ).convertFromString( vswaption->getData( gearingName, ISDEFINED ).convertToString() );
		
		// ! Forward
		LAString forwardName = PRICING_DATA_FORWARD_LEG + LAString( num );
		object.remove( forwardName );
		object.add( forwardName, new LADataDoubles() ).convertFromString( vswaption->getData( forwardName, ISDEFINED ).convertToString() );

		// ! Spreads
		LAString spreadName = PRICING_DATA_SPREAD_LEG + LAString( num );
		object.remove( spreadName );
		object.add( spreadName, new LADataDoubles() ).convertFromString( vswaption->getData( spreadName, ISDEFINED ).convertToString() );

		// ! AccrualDays
		LAString accDaysName = PRICING_DATA_ACCRUALDAYS_LEG + LAString( num );
		object.remove( accDaysName );
		object.add( accDaysName, new LADataDoubles() ).convertFromString( vswaption->getData( accDaysName, ISDEFINED ).convertToString() );

		// ! Notional
		LAString notionalName = PRICING_CALIBRATION_DATAOTIONAL_LEG + LAString( num );
		object.remove( notionalName );
		object.add( notionalName, new LADataDoubles() ).convertFromString( vswaption->getData( notionalName, ISDEFINED ).convertToString() );

		// ! FixingDate
		LAString fixingDateName = PRICING_DATA_FIXINGDATE_LEG + LAString( num );
		object.remove( fixingDateName );
		object.add( fixingDateName, new LADataDates() ).convertFromString( vswaption->getData( fixingDateName, ISDEFINED ).convertToString() );
	
	}
}