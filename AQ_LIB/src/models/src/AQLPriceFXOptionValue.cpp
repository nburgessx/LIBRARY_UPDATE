//  2008, AlgoQuantHub.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <algorithm>
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectPool.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLDataReference.h"
#include "AQLBasic.h"
#include "AQLMathDateCalculations.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceFXOptionValue.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLMathVolFuncFXStrangleSolver.h"
#include "AQLLinearRatesOptionValueDataProvider.h"

using namespace std;

AQLPriceFXOptionValue::AQLPriceFXOptionValue()
: AQLLinearRatesOptionValue()
{}

//AQLPriceFXOptionValue::AQLPriceFXOptionValue(AQLPriceFXOptionValue& v)
//: AQLLinearRatesOptionValue(v)
//{}

AQLPriceFXOptionValue::~AQLPriceFXOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLPriceFXOptionValue::getType() const
{
	return FN_FXOPTIONVALUE;
}
/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceFXOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_FXOPTIONVALUE ? true : AQLLinearRatesOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
AQLString 
AQLPriceFXOptionValue::getOptionPayoffName() const
{
	return FN_FXOPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceFXOptionValue::registerData(AQLPriceDataManager& dm) const
{

	AQLLinearRatesOptionValue::registerData(dm);

	
	dm.setData(PRICING_DATA_DOMESTICCURRENCY,			DATA_STRING);
	dm.setData(PRICING_DATA_FOREIGNCURRENCY,			DATA_STRING);
	dm.setData(PRICING_DATA_DOMESTICQUANTITY,			DATA_DOUBLE);
	dm.setData(PRICING_DATA_FOREIGNQUANTITY,			DATA_DOUBLE);
	dm.setData(PRICING_DATA_CACHECURRENCY,				DATA_STRING);
	dm.setData(PRICING_DATA_ANALYTICRISKTYPE,			DATA_REFERENCE);
	dm.setData(PRICING_DATA_REBATE,					DATA_DOUBLE);
	dm.setData(PRICING_DATA_LIMITVAL,					DATA_DOUBLE);
	dm.setData(PRICING_DATA_UPANDDOWN,					DATA_STRING);
	dm.setData(PRICING_DATA_INANDOUT,					DATA_STRING);
	dm.setData(PRICING_DATA_ISLIBORDISCOUNTFORFXOPTION,DATA_BOOL);
	dm.setData(PRICING_DATA_PREMIUMAMOUT, DATA_DOUBLE);
	dm.setData(PRICING_DATA_PREMIUMPAYMENTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_PREMIUMPAYCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_CASHSETTLEMENTAMOUNT, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CASHSETTLEMENTPAYMENTDATE, DATA_DATE);
}

AQLCoreFunctionBase*
AQLPriceFXOptionValue::clone() const
{
    try 
	{
		return new AQLPriceFXOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
AQLPriceFXOptionValue::calcOption(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const
{
	dp;
	AQLLinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<AQLLinearRatesOptionValueDataProvider &>(att.getDataProvider());
	double ret = dataProvider->mAnalyticMethod[0][0]->calc(*(dataProvider->mParam[0][0]));
	ret *= adjustForAnalyticalRisk(att,dataProvider,e,*(dataProvider->mParam[0][0]));
	
	return ret;
	
}

// calc option
double 
AQLPriceFXOptionValue::calcFXDigitalCallSpreadOption(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const
{
	AQLLinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<AQLLinearRatesOptionValueDataProvider &>(att.getDataProvider());
	//this means call spread value
	double val1 = dataProvider->mAnalyticMethod[0][0]->calc(*(dataProvider->mParam[0][0]));
	val1 *= adjustForAnalyticalRisk(att,dataProvider,e,*(dataProvider->mParam[0][0]));
	
	double val2 = dataProvider->mAnalyticMethod[0][0]->calc(*(dataProvider->mParam[0][1]));
	val2 *= adjustForAnalyticalRisk(att,dataProvider,e,*(dataProvider->mParam[0][1]));
	
	if (dataProvider->buysell)
		return val1-val2;
	else
		return val2 - val1;
}

// calc payoff after maturity
double				
AQLPriceFXOptionValue::calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const
{
	//dp;
	AQLPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceFXOptionValueDataProvider*>(dp);
	
	AQLDataHolder* dh = &(e.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	AQLString optiontype = dynamic_cast<AQLDataString &>(dh->get()).get();
	optiontype.toUpper();
	
	double ret = 0.0;
	AnalyticGKParam* gkparam = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][0]);
	if (AQLString("CALL") == optiontype && gkparam->S > gkparam->K)
	{
		ret = gkparam->S - gkparam->K;
	}
	else if(AQLString("PUT") == optiontype && gkparam->K > gkparam->S)
	{
		ret = gkparam->K - gkparam->S;
	}
	else
		ret = 0.0;
	return ret;
}

double
AQLPriceFXOptionValue::calcPayOffFXDigitalCallSpreadAterMaturity(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const
{
	//dp;
	AQLPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceFXOptionValueDataProvider*>(dp);
	
	AQLDataHolder* dh = &(e.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	AQLString optiontype = dynamic_cast<AQLDataString &>(dh->get()).get();
	optiontype.toUpper();
	
	double val1 = 0.0;
	AnalyticGKParam* gkparam = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][0]);
	if (AQLString("CALL") == optiontype && gkparam->S > gkparam->K)
	{
		val1 = gkparam->S - gkparam->K;
	}
	else if(AQLString("PUT") == optiontype && gkparam->K > gkparam->S)
	{
		val1 = gkparam->K - gkparam->S;
	}
	else
		val1 = 0.0;

	double val2 = 0.0;
	gkparam = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][1]);
	if (AQLString("CALL") == optiontype && gkparam->S > gkparam->K)
	{
		val2 = gkparam->S - gkparam->K;
	}
	else if(AQLString("PUT") == optiontype && gkparam->K > gkparam->S)
	{
		val2 = gkparam->K - gkparam->S;
	}
	else
		val2 = 0.0;

	double ret = 0.0;
	if (dataProvider->buysell)
		ret = val1 - val2;
	else
		ret = val2 - val1;
	
	return ret;
}



double
AQLPriceFXOptionValue::adjustForAnalyticalRisk(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e, AnalyticParam& param) const
{
	(void)e;
	(void)dp;
	AQLLinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<AQLLinearRatesOptionValueDataProvider &>(att.getDataProvider());
	if (!dataProvider->mIsAnalyticalRisk)
		return 1.0;

	
	//shiftval
	double ret = dataProvider->mShiftValForRisk;




	////at first check theta
	if (dataProvider->mAnalyticalRiskType == AQLString(THETA))
	{
		ret/= 365.0;
		return ret;
	}

	//diff or ratio
	if (dataProvider->mIsDiffForRisk)
		return ret;


	AnalyticGKParam& pGk = dynamic_cast<AnalyticGKParam& >(param);
	//if ratio pick up base
	if (dataProvider->mAnalyticalRiskType == AQLString(DELTA) || 
		dataProvider->mAnalyticalRiskType == AQLString(GAMMA))
	{
		ret *= pGk.S;
		ret *= 0.01; //change from %
	}
	else if(dataProvider->mAnalyticalRiskType == AQLString(VEGA))
	{
		ret *= pGk.Vol;
	}
	else if (dataProvider->mAnalyticalRiskType == AQLString(RHO))
	{
		ret *= pGk.rd;
	}
	else if (dataProvider->mAnalyticalRiskType == AQLString(PHI))
	{
		ret *= pGk.rf;
	}


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
AQLPriceFXOptionValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	AQLPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<AQLPriceFXOptionValueDataProvider *>(AQLLinearRatesOptionValue::setUpDataProvider(basedate,object,att));
	

	//unit
	dh = &(object.getData(PRICING_DATA_FOREIGNQUANTITY, ISNOTNULL));
	dataProvider->unit = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	
	//domcurrency
	dh = &(object.getData(PRICING_DATA_DOMESTICCURRENCY, ISNOTNULL));
	dataProvider->mdomcur = dynamic_cast<AQLDataString &>(dh->get()).get();
	dataProvider->mdomcur.toUpper();
	if(dataProvider->mdomcur != dataProvider->mnumerairecur)
		throw AQLCoreInvalidData("DomesticCurrency must be the same as NumeraireCurrency",__FILE__,__LINE__);
	
	//foreigncurrency
	dh = &(object.getData(PRICING_DATA_FOREIGNCURRENCY, ISNOTNULL));
	dataProvider->mforcur = dynamic_cast<AQLDataString &>(dh->get()).get();
	dataProvider->mforcur.toUpper();

	//fxcur
	dataProvider->mfxcur = dataProvider->mdomcur + "/" + dataProvider->mforcur;

	//special case if we use libor df for fxoption
	dh = &(object.getData(PRICING_DATA_ISLIBORDISCOUNTFORFXOPTION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->mIsLiborDFForFX = dynamic_cast<AQLDataBool &>(dh->get()).get();

	dataProvider->mSpotDate = dataProvider->mAsofDate;

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

std::vector< std::vector<AnalyticParam*> >
AQLPriceFXOptionValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dp) const
{
	(void)object;
	(void)dp;
	std::vector<AnalyticParam*> retvec(1);
	std::vector< std::vector<AnalyticParam*> > ret(1,retvec);
	ret[0][0] = new AnalyticGKParam();
	return ret;
}

void
AQLPriceFXOptionValue::setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const
{
	AQLDataHolder* dh;
	AQLPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceFXOptionValueDataProvider*>(dp);

	dataProvider->mSpotDate = (dataProvider->mSpotDate < dataProvider->mAsofDate) ? dataProvider->mAsofDate : dataProvider->mSpotDate;
	
	AQLDate mdydelivdate = (dataProvider->mSpotDate < dataProvider->mDeliveryDate) ? dataProvider->mDeliveryDate : dataProvider->mSpotDate;
	AQLDate mdymatudate = (dataProvider->mAsofDate < dataProvider->mMaturityDate) ? dataProvider->mMaturityDate : dataProvider->mAsofDate;

	//even if maturity date has passed, set up gkparam for obtaining the temporary volatility, forward
	//Td, Te, actT
	AQLPriceDataDayCount dc(ACT_365_ISDA);
	double td = dc.getTerm(dataProvider->mSpotDate, mdydelivdate,false);
	double te = dataProvider->mBlackDayCount.getTerm(dataProvider->mAsofDate, mdymatudate,false);
	double actt = dc.getTerm(dataProvider->mAsofDate, mdymatudate,false);


	
	//get spot
	AQLMathFXEntity& fxe = dynamic_cast<AQLMathFXEntity &>(dataProvider->mpvanilla->getFXEntity().get().get());
	////.S .F
	double spot = fxe.getForwardRate(dataProvider->mforcur,dataProvider->mdomcur,dataProvider->mAsofDate,dataProvider->mAsofDate);
	double forward = fxe.getForwardRate(dataProvider->mforcur,dataProvider->mdomcur,dataProvider->mAsofDate,mdydelivdate);

	//.rd.rf
	double rdrate = 0.0;
	double rfrate = 0.0;

	const AQLMathYieldCurve& bcurve = dataProvider->mpvanilla->getIRCurve(dataProvider->mdomcur);
	double dombasisrate = bcurve.getBasisZeroRate(dataProvider->mAsofDate,mdydelivdate);

	const AQLMathYieldCurve& fcurve = dataProvider->mpvanilla->getIRCurve(dataProvider->mforcur);
	double forrate = fcurve.getBasisZeroRate(dataProvider->mAsofDate,mdydelivdate);
	if(!dataProvider->mIsLiborDFForFX)
	{
		rdrate = dombasisrate;
		rfrate = forrate;
	}
	else
	{
		double domstdrate = bcurve.getZeroRate(dataProvider->mAsofDate,mdydelivdate);
		rdrate = domstdrate;
		rfrate = forrate - dombasisrate + domstdrate;
	}
	
	std::vector <AnalyticGKParam *> gkparam(dataProvider->mParam[0].size());
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		gkparam[i] = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][i]);
		gkparam[i]->Td = td;
		gkparam[i]->Te = te;
		gkparam[i]->S = spot;
		gkparam[i]->F = forward;
		gkparam[i]->rd = rdrate;
		gkparam[i]->rf = rfrate;
		gkparam[i]->actT = actt;
	}
	return;
}


//get additional premium
double
AQLPriceFXOptionValue::getAdditionalPremium(const AQLObject& object, AQLDataProvider* dp) const
{
	AQLDataHolder* dh;
	AQLPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceFXOptionValueDataProvider*>(dp);
	return dataProvider->mMargin;

}


/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
AQLPriceFXOptionValue::createNewDataProvider() const
{
	AQLPriceFXOptionValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new AQLPriceFXOptionValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}