//  2008, Mizuho International London.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <algorithm>

#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAObjectPool.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LADataReference.h"
#include "LAPriceDataFunction.h"
#include "LABasic.h"
#include "LAMathDateCalculations.h"
#include "LACoreComponentManager.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAMathPlainVanillaEntity.h"
#include "LALinearRatesOptionValue.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LALinearRatesVolatility.h"
#include "LALinearRatesVolatilityManager.h"
#include "LAPriceTradeValue.h"
#include "LAPricePortfolioValue.h"
#include "LADataMatrix.h"
#include "LAMathVolFuncFXVannaVolga.h"
#include "LALinearRatesModel.h"
#include "LAMathVolFuncIRSABR.h"

#include "LALinearRatesSwapTradeValue.h"

using namespace std;

LALinearRatesOptionValue::LALinearRatesOptionValue()
: LACoreValuation()
{}

//LALinearRatesOptionValue::LALinearRatesOptionValue(LALinearRatesOptionValue& v)
//: LACoreValuation(v)
//{}

LALinearRatesOptionValue::~LALinearRatesOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LALinearRatesOptionValue::getType() const
{
	return FN_PLAINVANILLAVALUE;
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LALinearRatesOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_PLAINVANILLAVALUE ? true : LACoreValuation::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
LAString 
LALinearRatesOptionValue::getOptionPayoffName() const
{
	return FN_PLAINVANILLAVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LALinearRatesOptionValue::registerData(LAPriceDataManager& dm) const
{

	dm.setData(PRICING_DATA_TRADEDATE,				DATA_DATE);
	dm.setData(PRICING_DATA_EXPIRYDATE,			DATA_DATE);
	dm.setData(PRICING_DATA_DELIVERYDATE,			DATA_DATE);
	dm.setData(PRICING_DATA_OPTIONSTATUS,				DATA_STRING);
	dm.setData(PRICING_DATA_BUYSELL,				DATA_STRING);
	dm.setData(PRICING_DATA_OPTIONTYPE,			DATA_STRING);
	dm.setData(PRICING_DATA_STRIKE,				DATA_DOUBLE);
	dm.setData(PRICING_DATA_MARKETPARAM,			DATA_REFERENCE);
	dm.setData(PRICING_DATA_CACHECURRENCY,			DATA_STRING);
	dm.setData(PRICING_DATA_ANALYTICRISKTYPE,		DATA_REFERENCE);
	dm.setData(PRICING_DATA_VOLATILITYDIRECTINPUT, DATA_DOUBLE);
	dm.setData(PRICING_DATA_MARGIN,				DATA_DOUBLE);
	dm.setData(PRICING_DATA_ISDIFFFORRISK,			DATA_BOOL);
	dm.setData(PRICING_DATA_SHIFTVALFORRISK,		DATA_DOUBLE);
	dm.setData(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY, DATA_BOOL);
	dm.setData(PRICING_DATA_VALUEDATE,				DATA_DATE);
	dm.setData(PRICING_DATA_VOLATILITYDIRECTINPUTS, DATA_DOUBLES);
	dm.setData(PRICING_DATA_PAYOUT,				DATA_STRING);
	dm.setData(PRICING_DATA_ZEROCALC,				DATA_BOOL);
	dm.setData(PRICING_DATA_ISPVVOLUSE,			DATA_BOOL);
	dm.setData(PRICING_DATA_PREMIUMAMOUT, DATA_DOUBLE);
	dm.setData(PRICING_DATA_PREMIUMPAYMENTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_PREMIUMCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_CASHSETTLEMENTAMOUNT, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CASHSETTLEMENTPAYMENTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_CASHSETTLEMENTCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_DIGITALCOUPON, DATA_DOUBLE);
}

LACoreFunctionBase*
LALinearRatesOptionValue::clone() const
{
    try 
	{
		return new LALinearRatesOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to LAMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return swaption prem
	
*/
double
LALinearRatesOptionValue::value(const LADate& basedate, LAObject& object,
					const LADataValuation& att) const
{
	LADataHolder* dh;
	LALinearRatesOptionValueDataProvider* dataProvider = NULL;

	bool iscalcrisk = false;
	dh = &object.getData(PRICING_DATA_ISCALCRISK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const LADataBool&>(dh->get()).get();

	// check nocalc 
	bool isnocalc = false;
    dh = &object.getData(PRICING_DATA_ZEROCALC, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
    	isnocalc = dynamic_cast<const LADataBool&>(dh->get()).get();
	if (isnocalc)
	{
		double noCalcResult = 0;
		if (!iscalcrisk)
		{
			object.remove(PRICING_DATA_DIRTYPRICE);
			object.add(PRICING_DATA_DIRTYPRICE, new LADataDouble(noCalcResult));

			object.remove(PRICING_DATA_VOLATILITYRESULTOFPREMIUM);
			object.add(PRICING_DATA_VOLATILITYRESULTOFPREMIUM, new LADataDouble(noCalcResult));
		}
		return noCalcResult;
	}

	//pvvolmode
	bool ispvvoluse = false;
	dh = &object.getData(PRICING_DATA_ISPVVOLUSE, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		ispvvoluse = dynamic_cast<const LADataBool&>(dh->get()).get();


	if (!iscalcrisk) att.setDataProvider(NULL);

	if (att.isNullDataProvider())
	{
		dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(createNewDataProvider());
		att.setDataProvider(dataProvider);
		dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(setUpDataProvider(basedate, object, att));
	}
	else
	{
		dataProvider = &dynamic_cast<LALinearRatesOptionValueDataProvider&>(att.getDataProvider());
        setPVCurrency(object, dataProvider); 
        setPremiumCurrency(object, dataProvider);
		setCashSettlementCurrency(object, dataProvider);

		//for theta
		//get param object
		dh = &(object.getData(PRICING_DATA_MARKETPARAM, ISNOTNULL));
		LADataReference& refplain = dynamic_cast<LADataReference &>(dh->get());
		LAMathPlainVanillaEntity& eparam = dynamic_cast<LAMathPlainVanillaEntity &>(refplain.get().get());
		//check name
		LAString chkname = eparam.getName().get();
		dataProvider->mpvanilla = &eparam;
		dataProvider->mAsofDate = dataProvider->mpvanilla->getAsOfDate();
		
		dh = &(object.getData(PRICING_DATA_VALUEDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LADate& valuedate = dynamic_cast<const LADataDate&>(dh->get()).get();
			if (valuedate >= dataProvider->mAsofDate)
			{
				dataProvider->mValueDate = valuedate;
			}
			else
			{
				dataProvider->mValueDate = dataProvider->mAsofDate;
			}
		}
		else
		{
			dataProvider->mValueDate = dataProvider->mpvanilla->getAsOfDate();
		}
		
		dataProvider->mIsAsofAfterMaturity = (dataProvider->mAsofDate >= dataProvider->mMaturityDate);
		if (dataProvider->mIsAsofAfterMaturity)
		{
			dh = &(object.getData(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY));
			if (dh->isDefined() && !dh->isNull())
				dataProvider->mIsPayOffCalculateAfterMaturity = dynamic_cast<const LADataBool &>(dh->get()).get();
		}

		//for analytical risk
		dh = &(object.getData(PRICING_DATA_ANALYTICRISKTYPE,NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			LAObject& ref = dynamic_cast<LADataReference& >(dh->get()).get().get();
			
			dh = &(ref.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
			LAString risktype = dynamic_cast<LADataString &>(dh->get()).get();
			risktype.toUpper();
			if (risktype.findString(LAString("PREM")) < 0)
			{
				dataProvider->mIsAnalyticalRisk = true;
				dataProvider->mAnalyticMethod = getAnalyticMethod(object,dataProvider);
				dataProvider->mPayoffMethod = getPayoffMethod(object,dataProvider);

				dh = &(ref.getData(PRICING_DATA_SHIFTVALFORRISK,ISNOTNULL));
				dataProvider->mShiftValForRisk = dynamic_cast<LADataDouble &>(dh->get()).get();
				
				dh = &(ref.getData(PRICING_DATA_ISDIFFFORRISK, ISNOTNULL));
				dataProvider->mIsDiffForRisk = dynamic_cast<LADataBool &>(dh->get()).get();

				LAStringVector tmpvec = risktype.toToken('_');
				if (tmpvec.size() < 2)
					throw LACoreInvalidData("RiskType Error",__FILE__,__LINE__);

				dataProvider->mAnalyticalRiskType = tmpvec[0];
			}
			else
			{
				dataProvider->mIsAnalyticalRisk = false;
				dataProvider->mAnalyticMethod = getAnalyticMethod(object,dataProvider);
				dataProvider->mPayoffMethod = getPayoffMethod(object,dataProvider);
			}
		}
	}

	//set up analytic param
	setUpAnalyticParam(object,dataProvider);
	
	
	//set up vol
	LALinearRatesVolatility* pvol = 
		LALinearRatesVolatilityManager::getInstance()->createPlainVanillaVolatiltyGenerator(dataProvider,object,getType(),getOptionPayoffName(),dataProvider->mValueModel);
	pvol->setVolatility(dataProvider,object,LAString());
	
	double ret = 0.0;
	if (dataProvider->mIsAsofAfterMaturity)
	{
		if (dataProvider->mIsAnalyticalRisk)
			ret = 0.0;
		else if (!dataProvider->mIsPayOffCalculateAfterMaturity)
			ret = 0.0;
		else
			ret = calcPayOffAterMaturity(att,dataProvider,object);
	}
	else
	{
		LALinearRatesModel* pmodel = 
			LALinearRatesVolatilityManager::getInstance()->createPlainVanillaModelGenerator(dataProvider,object,getType(),getOptionPayoffName(),dataProvider->mValueModel);

		//hishida vannavolga
		ret = pmodel->calcValue(att,dataProvider,object, getOptionPayoffName());
	}

	double valueratio = dataProvider->mpvanilla->getIRCurve(dataProvider->mnumerairecur).getBasisDF(dataProvider->mAsofDate, dataProvider->mValueDate);
	ret /= valueratio;

	//margin
	if (!iscalcrisk)
		ret += getAdditionalPremium(object,dataProvider);
		//ret += dataProvider->mMargin;
	
	//unit
	ret *= multipleUnit(object,dataProvider);

	//option value
	dataProvider->mOptionValue = ret;
	
	//for forward premium
	if(dataProvider->mIsPremAdjust)
	{
		ret += getForwardPremium(object,dataProvider);
	}

	//add cash settlement amount
	if(dataProvider->mIsCashSettlementAdjust)
	{
		ret += getCashSettlementAmount(object,dataProvider);
	}

	//transform pvcurrency value
	double fx(1.);
	double ret_in_numerairecur(ret);
    if (dataProvider->mpvcur != dataProvider->mnumerairecur){
		fx = multipleFX(object, dataProvider, dataProvider->mnumerairecur, dataProvider->mpvcur, dataProvider->mBlackDayCount.getTerm(dataProvider->mAsofDate, dataProvider->mValueDate));
		dataProvider->mOptionValue *= fx;
		dataProvider->mPremiumValue *= fx;
		dataProvider->mCashSettlementValue *= fx;
        ret *= fx;
    }
	
	// Calculate Fee Value
	double feeExcludedPV = ret;
	double pvFee = LALinearRatesSwapTradeValue::calcFeeValueVanilla(object, dataProvider->mpvcur);
	ret += pvFee;

	//result output
	//dirty price is when only pv is calculated
	if (!iscalcrisk)
	{
		object.remove(PRICING_DATA_DIRTYPRICE);
		object.add(PRICING_DATA_DIRTYPRICE, new LADataDouble(ret));

		object.remove(PRICING_DATA_FEE_EXCLUDED_PV);
		object.add(PRICING_DATA_FEE_EXCLUDED_PV, new LADataDouble(feeExcludedPV));

		object.remove(PRICING_DATA_VOLATILITYRESULTOFPREMIUM);
		double volval = 0.0;
		//hishida vannavolga temporary
		dh = &object.getData("ImplyVolFromVannaVolga",NOCHECK);
		if (dh->isDefined() && !dh->isNull())
			volval = dynamic_cast<LADataDouble &>(dh->get()).get();
		else
			volval = getVolatilityResult(object,dataProvider);

		//double volval = getVolatilityResult(object,dataProvider);
		object.add(PRICING_DATA_VOLATILITYRESULTOFPREMIUM, new LADataDouble(volval));

		if (ispvvoluse)
		{
			object.remove(PRICING_DATA_PVVOLMATRIX);
			DoubleMatrix pvvolmat = getVolatilityMatrixResult(object,dataProvider);
			object.add(PRICING_DATA_PVVOLMATRIX, new LADataDoubleMatrix(pvvolmat));

		}

		//out put to leg dataValues
		dh = &object.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			unsigned int underlyingSize(dynamic_cast<LADataMultiReference& >(dh->get()).getSize());
			LAObject& legEntity(dynamic_cast<LADataMultiReference& >(dh->get()).get(0).get());
			dh = &legEntity.getData(PRICING_DATA_LEGNUMBER, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				if (underlyingSize != 1)
					throw LACoreInvalidData("Size of underlying must be 1 when leg number data is set.",__FILE__, __LINE__);
				std::vector<LAString> attrNameLegPV(2);
				attrNameLegPV[0] = PRICING_DATA_PV_LEG1; attrNameLegPV[1] = PRICING_DATA_PV_LEG2; 
				std::vector<LAString> attrNameLegCcy(2);
				attrNameLegCcy[0] = PRICING_DATA_CURRENCY_LEG1; attrNameLegCcy[1] = PRICING_DATA_CURRENCY_LEG2; 
				std::vector<LAString> attrNameLegTodayFX(2);
				attrNameLegTodayFX[0] = PRICING_DATA_TODAYFX_LEG1CCY; attrNameLegTodayFX[1] = PRICING_DATA_TODAYFX_LEG2CCY;

				const int legNumAssignedTo(dynamic_cast<const LADataInt& >(dh->get()).get());
				if (legNumAssignedTo < 1 || 2 < legNumAssignedTo)
					throw LACoreInvalidData("LegNumber Error",__FILE__,__LINE__);

				object.remove(attrNameLegPV[legNumAssignedTo - 1]);
				object.add(attrNameLegPV[legNumAssignedTo - 1], new LADataDouble(ret_in_numerairecur));
				object.remove(attrNameLegCcy[legNumAssignedTo - 1]);
				object.add(attrNameLegCcy[legNumAssignedTo - 1], new LADataString(dataProvider->mnumerairecur));
				object.remove(attrNameLegTodayFX[legNumAssignedTo - 1]);
				object.add(attrNameLegTodayFX[legNumAssignedTo - 1], new LADataDouble(fx));
			}
		}
	}
	
	bool isResultOut = false;
	dh = &( object.getData( PRICING_DATA_ISRESULTOUTPUT, NOCHECK ) );
	if ( dh->isDefined() && !dh->isNull() )
	{
		isResultOut = dynamic_cast< LADataBool& >( dh->get() ).get();
	}
	if ( isResultOut && !iscalcrisk )
	{
		// !Output Cashflow or OptionParams 
		outputResult( object, dataProvider );
	}
	return ret;
}

// calc option
double 
LALinearRatesOptionValue::calcOption(const LADataValuation& att, LADataProvider* dp, LAObject& e) const
{
	(void)dp;
	(void)e;
	LALinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<LALinearRatesOptionValueDataProvider &>(att.getDataProvider());

	double ret = 0.0;
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		if ( dataProvider->mVolfunc->isTypeOf(FN_VOLFUNCIRSABR) && dataProvider->mValueModel == BSVALUEMODEL)
		{
			AnalyticBKParam* tmpParam(dynamic_cast<AnalyticBKParam* >(dataProvider->mParam[0][i]->clone()));
			const double forwardShiftValue(dynamic_cast<const LAMathVolFuncIRSABR* >(dataProvider->mVolfunc)->getForwardShiftValue());
			tmpParam->F += forwardShiftValue;
			tmpParam->K += forwardShiftValue;

			//for swaption, calculation method will be changed to BKPayOff class when forward is nearly negative.  
			if (tmpParam->F > MIN_RATE && tmpParam->K > MIN_RATE)
			{
				ret += dataProvider->mAnalyticMethod[0][i]->calc(*tmpParam);
			}
			else
			{
				ret += dataProvider->mPayoffMethod[0][i]->calc(*tmpParam);
			}
			delete tmpParam;
		}
		else
		{
			ret += dataProvider->mAnalyticMethod[0][i]->calc(*(dataProvider->mParam[0][i]));
		}
	}
	return ret;
	
}

// calc payoff after maturity
double				
LALinearRatesOptionValue::calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dp, LAObject& e) const
{
	(void)att;
	(void)dp;
	(void)e;
	return 0.0;
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*					
LALinearRatesOptionValue::setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const
{
	(void)basedate;
	LADataHolder *dh;
	LALinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<LALinearRatesOptionValueDataProvider &>(att.getDataProvider());
	//get param object
	dh = &(object.getData(PRICING_DATA_MARKETPARAM, ISNOTNULL));
	LADataReference& refplain = dynamic_cast<LADataReference &>(dh->get());
	LAMathPlainVanillaEntity& eparam = dynamic_cast<LAMathPlainVanillaEntity &>(refplain.get().get());
	dataProvider->mpvanilla = &eparam;

	
	setUpNumeraireCurrency(object, dataProvider);

    setPVCurrency(object, dataProvider);
    setPremiumCurrency(object, dataProvider);
	setCashSettlementCurrency(object, dataProvider);

	
	//asof
	LADate asof = eparam.getAsOfDate().get();
	dataProvider->mAsofDate = asof;

	//value date
	dh = &(object.getData(PRICING_DATA_VALUEDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADate& valuedate = dynamic_cast<const LADataDate&>(dh->get()).get();
		if (valuedate >= dataProvider->mAsofDate)
		{
			dataProvider->mValueDate = valuedate;
		}
		else
		{
			dataProvider->mValueDate = dataProvider->mAsofDate;
		}
	}
	else
	{
		//value date
		dataProvider->mValueDate = asof;
	}


	//maturity
	dataProvider->mMaturityDate = getMaturityDate(object, dataProvider);
	
	dataProvider->mIsAsofAfterMaturity = (asof >= dataProvider->mMaturityDate);
	if (dataProvider->mIsAsofAfterMaturity)
	{
		dh = &(object.getData(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY));
		if (dh->isDefined() && !dh->isNull())
			dataProvider->mIsPayOffCalculateAfterMaturity = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	
	//delivery
	dataProvider->mDeliveryDate = getDeliveryDate(object,dataProvider);
	
	//trade
	dh = &(object.getData(PRICING_DATA_TRADEDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADate& tdate = dynamic_cast<const LADataDate &>(dh->get()).get();
		dataProvider->mTradeDate = tdate;
	}
	
	//buy sell
	dh = &(object.getData(PRICING_DATA_BUYSELL, ISNOTNULL));
	LAString buystr = dynamic_cast<LADataString &>(dh->get()).get();
	if ("BUY" == buystr.toUpper())
		dataProvider->buysell = true;
	else
		dataProvider->buysell = false;

	// option status on or after the expiry date
	dh = &(object.getData(PRICING_DATA_OPTIONSTATUS));
	if (dh->isDefined() && !dh->isNull() && dataProvider->mIsAsofAfterMaturity)
	{
		LAString status =  dynamic_cast<LADataString &>(dh->get()).get();
		if (status == "ALIVE")
			dataProvider->mIsStillAlive = true;
		else if(status == "DEAD")
			dataProvider->mIsStillAlive = false;
		else
			throw LACoreInvalidData("Invalid OptionStatus : only ALIVE and DEAD are available",__FILE__,__LINE__);
	}

	//margin
	dh = &(object.getData(PRICING_DATA_MARGIN));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->mMargin = dynamic_cast<LADataDouble &>(dh->get()).get();

	//get cashlet size
	dataProvider->mCashletSize = getCashletSize(object,dataProvider);
	
	//create analyticparam
	dataProvider->mParam = createAnalyticParam(object,dataProvider);

	//analytical method
	dataProvider->mAnalyticMethod = getAnalyticMethod(object,dataProvider);
	dataProvider->mPayoffMethod = getPayoffMethod(object,dataProvider);

	//strike
	dh = &(object.getData(PRICING_DATA_STRIKE, NOCHECK));
	if(dh->isDefined() && !dh->isNull())
		dataProvider->mParam[0][0]->K = dynamic_cast<const LADataDouble &>(dh->get()).get();

	if (dataProvider->mParam.size() != dataProvider->mAnalyticMethod.size() ||
		dataProvider->mParam.size() != dataProvider->mPayoffMethod.size() ||
		dataProvider->mParam.size() != dataProvider->mCashletSize)
	{
		throw LACoreInvalidData("Option Cashlet Size is wrong",__FILE__,__LINE__);
	}

	//blackdaycount
	//dataProvider->mBlackDayCount.convertFromString("ACT/365");
	dataProvider->mBlackDayCount.convertFromString("ACT/365_ISDA");

	//hishida vanna volga
	//temporary
	dataProvider->mValueModel = BSVALUEMODEL;
	const LAString productname = getOptionPayoffName();
	bool isfxproduct = (productname.findString("fn_fx") != -1);
	if (isfxproduct)
	{
		LAPriceFXOptionValueDataProvider* fxDataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider*>(dataProvider);
		const LAStringVector& fxccys = fxDataProvider->mpvanilla->getFXCurrencys().get();
		if (fxccys.size() < 1)
			throw LACoreInvalidData("FXCurrencys are not registered",__FILE__,__LINE__);

		//check only one currency pair
		const LAFunctionBase* basefunc = fxDataProvider->mpvanilla->getFXVolFunc(fxccys[0]);
		if (basefunc->isTypeOf(FN_VOLFUNCFXVANNAVOLGA))
		{
			dataProvider->mValueModel = VVVALUEMODEL;
		}
	}
	
    // Premium
	dh = &(object.getData(PRICING_DATA_PREMIUMPAYMENTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mIsPremAdjust = true;

		dataProvider->mPremPayDate = dynamic_cast<const LADataDate &>(dh->get()).get();
		dataProvider->mPremAmount = 0.0;
		dh = &(object.getData(PRICING_DATA_PREMIUMAMOUT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			dataProvider->mPremAmount = dynamic_cast<const LADataDouble &>(dh->get()).get();

		dh = &(object.getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK));
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

	// caps and floors
	if (productname == FN_IR_CAPFLOOROPTIONVALUE_STR && dataProvider->mIsAsofAfterMaturity)
	{
		dataProvider->mIsPayOffCalculateAfterMaturity = true;
	}

	// in case the option is still alive on the expiry date
	if (dataProvider->mIsStillAlive && dataProvider->mIsAsofAfterMaturity)
	{
		dataProvider->mIsPayOffCalculateAfterMaturity = true;
	}

	return dataProvider;

	
}

std::vector< std::vector<AnalyticParam*> >
LALinearRatesOptionValue::createAnalyticParam(LAObject& object, LADataProvider* dp) const
{
	(void)dp; (void)object;
	return std::vector< std::vector<AnalyticParam*> >(0);
}

std::vector< std::vector<LABlackScholesBase*> >
LALinearRatesOptionValue::getAnalyticMethod(LAObject& object, LADataProvider* dp) const
{
	(void)dp;
	std::vector< std::vector<LABlackScholesBase* > > ret;
	std::vector<LABlackScholesBase* > retvec;
	//std::vector<LABlackScholesBase* > ret;
	LADataHolder* dh;
	
	//knockout rebate is special case
	if (getType() == FN_FXKNOCKOUTREBATEVALUE)
	{
		std::vector<LABlackScholesBase*> tmpvec(2);
		ret.resize(1,tmpvec);
		//ret.resize(2);
		
		//call up in
		LAString bscomponent = LAString(SB) + LAString(PREM) + LAString(CALL) + LAString(SBUP) + LAString(SBIN);
		std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
		std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
		if(it==var.end())
			throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
		
		ret[0][0] = it->second;
		
		//call up out 
		bscomponent = LAString(SB) + LAString(PREM) + LAString(CALL) + LAString(SBUP) + LAString(SBOUT);
		it = var.find(bscomponent);
		if(it==var.end())
			throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);

		ret[0][1] = it->second;
		return ret;
	}
	
	dh = &(object.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	LAString optiontype = dynamic_cast<LADataString &>(dh->get()).get();
	optiontype.toUpper();

	LAString producttype;
	if (getType()== FN_FXOPTIONVALUE)
	{
		producttype = LAString(GK);
	}
	else if (getType() == FN_FXDIGITALOPTIONVALUE)
	{
		producttype = LAString(DG);
	}
	else if (getType() == FN_FXDIGITALCALLSPREADOPTIONVALUE)
	{
		producttype = LAString(GK);
	}
	else if (getType() == FN_FXSINGLEBARRIEROPTIONVALUE || 
				getType() == FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE)
	{
		producttype = LAString(SB);
	}
	else
	{
		throw LACoreInvalidData("AnalyticMethodError",__FILE__,__LINE__);
	}

	LAString risktype = LAString(PREM);
	dh = &(object.getData(PRICING_DATA_ANALYTICRISKTYPE,NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LAObject& ref = dynamic_cast<LADataReference &>(dh->get()).get().get();
		dh = &(ref.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
		LAString tmpname = dynamic_cast<LADataString &>(dh->get()).get();
		if (tmpname != LAString(PREM))
		{

			bool issuccess = false;
			dh = &(object.getData(tmpname));
			if (dh->isDefined() && !dh->isNull())
				issuccess = dynamic_cast<LADataBool &>(dh->get()).get();

			if (!issuccess)
			{
				//search map
				std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
				std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(LAString("ERROR"));
				if(it==var.end())
					throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
				
				retvec.push_back(it->second);
				ret.push_back(retvec);
				return ret;
			}
			
			LAStringVector tmpvec = tmpname.toToken('_');
			if (tmpvec.size() < 2)
				throw LACoreInvalidData("RiskType Error",__FILE__,__LINE__);
			
			risktype = tmpvec[0];
			risktype.toUpper();
		}
	}

	LAString bscomponent = producttype + risktype  + optiontype;
	
	//single barrier is special info
	if (getType() == FN_FXSINGLEBARRIEROPTIONVALUE || 
		getType() == FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE)
	{
		dh = &(object.getData(PRICING_DATA_UPANDDOWN, ISNOTNULL));
		LAString updown = dynamic_cast<LADataString &>(dh->get()).get();
		updown.toUpper();

		dh = &(object.getData(PRICING_DATA_INANDOUT, ISNOTNULL));
		LAString inout = dynamic_cast<LADataString &>(dh->get()).get();
		inout.toUpper();

		bscomponent += updown + inout;
	}

	//search map
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	
	retvec.push_back(it->second);
	ret.push_back(retvec);
	return ret;
	
}

std::vector< std::vector<LABlackScholesBase*> >
LALinearRatesOptionValue::getPayoffMethod(LAObject& object, LADataProvider* dp) const
{
	//return analytical method for dummy because this has not implemented yet except swaption sub class.
	return getAnalyticMethod(object, dp);
}

void
LALinearRatesOptionValue::setUpAnalyticParam(LAObject& object, LADataProvider* dp) const
{
	object;
	dp;
	return;
}

unsigned int 
LALinearRatesOptionValue::getCashletSize(const LAObject& object, LADataProvider* dp) const
{
	object;
	dp;
	return 1;
}

/*!
	@brief create new cache class
	@return cache class
*/
LADataProvider*
LALinearRatesOptionValue::createNewDataProvider() const
{
	LALinearRatesOptionValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new LALinearRatesOptionValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}


const LADate&
LALinearRatesOptionValue::getMaturityDate(const LAObject& object, LADataProvider* dp) const
{
	(void) dp;
	const LADataHolder* dh = &(object.getData(PRICING_DATA_EXPIRYDATE, ISNOTNULL));
	const LADate& edate = dynamic_cast<const LADataDate &>(dh->get()).get();
	return edate;	
}

const LADate&
LALinearRatesOptionValue::getDeliveryDate(const LAObject& object, LADataProvider* dp) const
{
	(void) dp;
	const LADataHolder* dh = &(object.getData(PRICING_DATA_DELIVERYDATE, ISNOTNULL));
	const LADate& ddate = dynamic_cast<const LADataDate &>(dh->get()).get();
	return ddate;	
}

//multiple unit
double  
LALinearRatesOptionValue::multipleUnit(const LAObject& object, LADataProvider* dp) const
{
	object;
	
	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider *>(dp);
	double ret = dataProvider->unit;
	if (!dataProvider->buysell)
		ret*= -1;

	return ret;
}

//multiple fxspot
double
LALinearRatesOptionValue::multipleFX(const LAObject& object, const LADataProvider* dp, const LAString& from, const LAString& to, double t) const
{
    if(from==to) return 1;
	const LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<const LALinearRatesOptionValueDataProvider *>(dp);
    return dynamic_cast<LAMathFXEntity &>(dataProvider->mpvanilla->getFXEntity().get().get()).getRate(from, to, t);
}

//get volatility result 
double 
LALinearRatesOptionValue::getVolatilityResult(const LAObject& object, LADataProvider* dp) const
{
	double ret = 0.0;
	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider *>(dp);
	double forwardShiftValue(0.);
	if(dataProvider->mVolfunc->isTypeOf(FN_VOLFUNCIRSABR))
	{
		forwardShiftValue = dynamic_cast<const LAMathVolFuncIRSABR*>(dataProvider->mVolfunc)->getForwardShiftValue();
	}
	for (unsigned int i = 0; i < dataProvider->mParam.size(); i++)
	{
		for (unsigned int j = 0; j < dataProvider->mParam[i].size(); j++)
		{
			if (dataProvider->mParam[i][j] != 0)
			{
				const AnalyticBKParam* bkParam = dynamic_cast<AnalyticBKParam*>(dataProvider->mParam[i][j]);
				if (bkParam != 0)
				{
					if(bkParam->F + forwardShiftValue > MIN_RATE && bkParam->K + forwardShiftValue > MIN_RATE)
					{
						ret = dataProvider->mParam[i][j]->Vol;
					}
					else
					{
						ret = 0.;
					}
				}
				else
				{
					ret = dataProvider->mParam[i][j]->Vol;
				}
				return ret;
			}
		}
	}
	return ret;
}

//get volatility result 
DoubleMatrix 
LALinearRatesOptionValue::getVolatilityMatrixResult(const LAObject& object, LADataProvider* dp) const
{
	
	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider *>(dp);
	
	DoubleMatrix ret(dataProvider->mParam.size());
	for (unsigned int i = 0; i < dataProvider->mParam.size(); i++)
	{
		ret[i].resize(dataProvider->mParam[i].size(),0.0);
		for (unsigned int j = 0; j < dataProvider->mParam[i].size(); j++)
		{
			if (dataProvider->mParam[i][j] != 0)
			{
				AnalyticBKParam bkParam;
				if (typeid( *dataProvider->mParam[i][j] ) == typeid(bkParam))
				{
					if(dynamic_cast<AnalyticBKParam*>(dataProvider->mParam[i][j])->F > MIN_RATE)
					{
						ret[i][j] = dataProvider->mParam[i][j]->Vol;
					}
					else
					{
						ret[i][j] = 0.;
					}
				}
				else
				{
					ret[i][j] = dataProvider->mParam[i][j]->Vol;
				}
			}
		}
	}
	return ret;
}

//
//----------
//

// Get analytic param result 
void 
LALinearRatesOptionValue::getAnalyticParamResult( const LAObject& object,
													 LADataProvider* dp,
													 LAStringVector& names,
													 DoubleVector& params 
												   ) const
{
	double ret = 0.0;
	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider *>(dp);
	for (unsigned int i = 0; i < dataProvider->mParam.size(); i++)
	{
		for (unsigned int j = 0; j < dataProvider->mParam[i].size(); j++)
		{
			if (dataProvider->mParam[i][j] != 0)
			{
				AnalyticGKParam* gk = dynamic_cast< AnalyticGKParam* >( dataProvider->mParam[i][j] );
				
				// ! Get S
				params.push_back( gk->S );
				names.push_back( "Spot" );

				// ! Get F
				params.push_back( gk->F );
				names.push_back( "Forward" );

				// ! Get Vol
				params.push_back( gk->Vol );
				names.push_back( "Volatility" );

				// ! Get Strike
				params.push_back( gk->K );
				names.push_back( "Strike" );

				// ! Get Te
				params.push_back( gk->Te );
				names.push_back( "ExpiryTerm" );

				// ! Get Td
				params.push_back( gk->Td );
				names.push_back( "DeliveryTerm" );

				// ! Get rd
				if ( gk->rd != 0. )
				{
					params.push_back( gk->rd );
					names.push_back( "DomesticRate" );
				}
				else
				{
					params.push_back( gk->DFd );
					names.push_back( "DomesticDF" );
				}

				// ! Get rf
				if ( gk->rf != 0. )
				{
					params.push_back( gk->rf );
					names.push_back( "ForeignRate" );
				}
				else
				{
					params.push_back( gk->DFf );
					names.push_back( "ForeignDF" );
				}
			}
		}
	}
	return;
}

//
//----------
//

// output cashflow or option params
void
LALinearRatesOptionValue::outputResult( LAObject& object, LADataProvider* dp ) const
{
	// ! Get analytic param result
	LAStringVector names;
	DoubleVector params;
	
	getAnalyticParamResult( object, dp, names, params );

	object.remove( PRICING_DATA_ANALYTICPARAMNAME );
	object.add( PRICING_DATA_ANALYTICPARAMNAME, new LADataStrings( names ) );

	object.remove( PRICING_DATA_ANALYTICPARAM );
	object.add( PRICING_DATA_ANALYTICPARAM, new LADataDoubles( params ) );

	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider *>(dataProvider);
	object.remove( PRICING_DATA_OPTIONVALUE );
	object.add( PRICING_DATA_OPTIONVALUE, new LADataDouble( dataProvider->mOptionValue + dataProvider->mCashSettlementValue ) );

	object.remove( PRICING_DATA_PREMIUMVALUE );
	object.add( PRICING_DATA_PREMIUMVALUE, new LADataDouble( dataProvider->mPremiumValue ) );
}


//
//----------
//

//get Additional Premium 
double 
LALinearRatesOptionValue::getAdditionalPremium(const LAObject& object, LADataProvider* dp) const
{
	double ret = 0.0;
	return ret;
}

//get forward premium
double 
LALinearRatesOptionValue::getForwardPremium(const LAObject& object, LADataProvider* dp) const
{
	object;
	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(dp);
    
    double ret;
	if (dataProvider->mPremPayDate > dataProvider->mAsofDate)
	{
		double df =	dataProvider->mpvanilla->getIRCurve(dataProvider->mPremiumCurrency).getBasisDF(dataProvider->mValueDate, dataProvider->mPremPayDate);
		double fx = multipleFX(object, dataProvider, dataProvider->mPremiumCurrency, dataProvider->mnumerairecur, 0);
		dataProvider->mPremiumValue = dataProvider->mPremAmount * df * fx;

		if (dataProvider->mIsAddFwdPremPV)
			ret = dataProvider->mPremAmount * df * fx;
		else if(!dataProvider->mIsAddFwdPremPV)
			ret = dataProvider->mPremAmount * (df-1) * fx;
	}
    else
    {
        ret = 0;
    }
	return ret;
}

//get cash settlement amount
double 
LALinearRatesOptionValue::getCashSettlementAmount(const LAObject& object, LADataProvider* dp) const
{
	object;
	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(dataProvider);
    
    double ret;
	if (dataProvider->mCashSettlementPayDate > dataProvider->mAsofDate)
	{
		LADate				valueDate					= dataProvider->mValueDate;
		LADate				cashSettlementPayDate		= dataProvider->mCashSettlementPayDate;
		LAString			cashSettlementCurrency		= dataProvider->mCashSettlementCurrency;
		LAString			numeraireCurrency			= dataProvider->mnumerairecur;
		LAMathYieldCurve	irCurve						= dataProvider->mpvanilla->getIRCurve(cashSettlementCurrency);

		double df	= irCurve.getBasisDF(valueDate, cashSettlementPayDate);
		double fx	= multipleFX(object, dataProvider, cashSettlementCurrency, numeraireCurrency, 0);
		
		dataProvider->mCashSettlementValue = dataProvider->mCashSettlementAmount * df * fx;
		ret = dataProvider->mCashSettlementValue;
	}
    else
    {
        ret = 0;
    }
	return ret;
}

void 
LALinearRatesOptionValue::setPVCurrency(const LAObject& object, LADataProvider* dp) const
{
    LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(dp);
    
    
    const LADataHolder& ah_1 = object.getData(PRICING_DATA_VALUATIONCURRENCY, NOCHECK);
    const LADataHolder& ah_2 = object.getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK);
	
    
    if(ah_1.isDefined() && !ah_1.isNull()){
        dataProvider->mpvcur = dynamic_cast<const LADataString &>(ah_1.get()).get();
    } 
    else if(ah_2.isDefined() && !ah_2.isNull()){
        dataProvider->mpvcur = dynamic_cast<const LADataString &>(ah_2.get()).get();
    }
    else{
        dataProvider->mpvcur = dataProvider->mnumerairecur;
    }
    dataProvider->mpvcur.toUpper();
}

void 
LALinearRatesOptionValue::setPremiumCurrency(const LAObject& object, LADataProvider* dp) const
{
    LALinearRatesOptionValueDataProvider *dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(dp);
    const LADataHolder& dh = object.getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK);
    
    
    if(dh.isDefined() && !dh.isNull()){
        dataProvider->mPremiumCurrency = dynamic_cast<const LADataString&>(dh.get()).get();
    }
    else{
        dataProvider->mPremiumCurrency = dataProvider->mnumerairecur;
    }
    dataProvider->mPremiumCurrency.toUpper();
}

void
LALinearRatesOptionValue::setUpNumeraireCurrency(const LAObject& trade, LALinearRatesOptionValueDataProvider* dp) const
{
	const LAStringVector& ircurs = dp->mpvanilla->getIRSimCurrencys().get();
	if (ircurs.size() == 0) throw LACoreInvalidData("IRCurrencys must be set", __FILE__, __LINE__);
	dp->mnumerairecur = ircurs[0];
}

void 
LALinearRatesOptionValue::setCashSettlementCurrency(const LAObject& object, LADataProvider* dp) const
{
    LALinearRatesOptionValueDataProvider *dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(dp);
    const LADataHolder& dh = object.getData(PRICING_DATA_CASHSETTLEMENTCURRENCY, NOCHECK);
    
    
    if(dh.isDefined() && !dh.isNull()){
        dataProvider->mCashSettlementCurrency = dynamic_cast<const LADataString&>(dh.get()).get();
    }
    else{
        dataProvider->mCashSettlementCurrency = dataProvider->mnumerairecur;
    }
    dataProvider->mCashSettlementCurrency.toUpper();
}
