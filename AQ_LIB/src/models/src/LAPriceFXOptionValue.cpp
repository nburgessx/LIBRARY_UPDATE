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
#include "LABasic.h"
#include "LAMathDateCalculations.h"
#include "LACoreComponentManager.h"
#include "LAPriceFXOptionValue.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LALinearRatesOptionValueDataProvider.h"

using namespace std;

LAPriceFXOptionValue::LAPriceFXOptionValue()
: LALinearRatesOptionValue()
{}

//LAPriceFXOptionValue::LAPriceFXOptionValue(LAPriceFXOptionValue& v)
//: LALinearRatesOptionValue(v)
//{}

LAPriceFXOptionValue::~LAPriceFXOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAPriceFXOptionValue::getType() const
{
	return FN_FXOPTIONVALUE;
}
/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceFXOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_FXOPTIONVALUE ? true : LALinearRatesOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
LAString 
LAPriceFXOptionValue::getOptionPayoffName() const
{
	return FN_FXOPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXOptionValue::registerData(LAPriceDataManager& dm) const
{

	LALinearRatesOptionValue::registerData(dm);

	
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

LACoreFunctionBase*
LAPriceFXOptionValue::clone() const
{
    try 
	{
		return new LAPriceFXOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
LAPriceFXOptionValue::calcOption(const LADataValuation& att, LADataProvider* dp, LAObject& e) const
{
	dp;
	LALinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<LALinearRatesOptionValueDataProvider &>(att.getDataProvider());
	double ret = dataProvider->mAnalyticMethod[0][0]->calc(*(dataProvider->mParam[0][0]));
	ret *= adjustForAnalyticalRisk(att,dataProvider,e,*(dataProvider->mParam[0][0]));
	
	return ret;
	
}

// calc option
double 
LAPriceFXOptionValue::calcFXDigitalCallSpreadOption(const LADataValuation& att, LADataProvider* dp, LAObject& e) const
{
	LALinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<LALinearRatesOptionValueDataProvider &>(att.getDataProvider());
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
LAPriceFXOptionValue::calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dp, LAObject& e) const
{
	//dp;
	LAPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dp);
	
	LADataHolder* dh = &(e.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	LAString optiontype = dynamic_cast<LADataString &>(dh->get()).get();
	optiontype.toUpper();
	
	double ret = 0.0;
	AnalyticGKParam* gkparam = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][0]);
	if (LAString("CALL") == optiontype && gkparam->S > gkparam->K)
	{
		ret = gkparam->S - gkparam->K;
	}
	else if(LAString("PUT") == optiontype && gkparam->K > gkparam->S)
	{
		ret = gkparam->K - gkparam->S;
	}
	else
		ret = 0.0;
	return ret;
}

double
LAPriceFXOptionValue::calcPayOffFXDigitalCallSpreadAterMaturity(const LADataValuation& att, LADataProvider* dp, LAObject& e) const
{
	//dp;
	LAPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dp);
	
	LADataHolder* dh = &(e.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	LAString optiontype = dynamic_cast<LADataString &>(dh->get()).get();
	optiontype.toUpper();
	
	double val1 = 0.0;
	AnalyticGKParam* gkparam = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][0]);
	if (LAString("CALL") == optiontype && gkparam->S > gkparam->K)
	{
		val1 = gkparam->S - gkparam->K;
	}
	else if(LAString("PUT") == optiontype && gkparam->K > gkparam->S)
	{
		val1 = gkparam->K - gkparam->S;
	}
	else
		val1 = 0.0;

	double val2 = 0.0;
	gkparam = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][1]);
	if (LAString("CALL") == optiontype && gkparam->S > gkparam->K)
	{
		val2 = gkparam->S - gkparam->K;
	}
	else if(LAString("PUT") == optiontype && gkparam->K > gkparam->S)
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
LAPriceFXOptionValue::adjustForAnalyticalRisk(const LADataValuation& att, LADataProvider* dp, LAObject& e, AnalyticParam& param) const
{
	(void)e;
	(void)dp;
	LALinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<LALinearRatesOptionValueDataProvider &>(att.getDataProvider());
	if (!dataProvider->mIsAnalyticalRisk)
		return 1.0;

	
	//shiftval
	double ret = dataProvider->mShiftValForRisk;




	////at first check theta
	if (dataProvider->mAnalyticalRiskType == LAString(THETA))
	{
		ret/= 365.0;
		return ret;
	}

	//diff or ratio
	if (dataProvider->mIsDiffForRisk)
		return ret;


	AnalyticGKParam& pGk = dynamic_cast<AnalyticGKParam& >(param);
	//if ratio pick up base
	if (dataProvider->mAnalyticalRiskType == LAString(DELTA) || 
		dataProvider->mAnalyticalRiskType == LAString(GAMMA))
	{
		ret *= pGk.S;
		ret *= 0.01; //change from %
	}
	else if(dataProvider->mAnalyticalRiskType == LAString(VEGA))
	{
		ret *= pGk.Vol;
	}
	else if (dataProvider->mAnalyticalRiskType == LAString(RHO))
	{
		ret *= pGk.rd;
	}
	else if (dataProvider->mAnalyticalRiskType == LAString(PHI))
	{
		ret *= pGk.rf;
	}


	return ret;
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*					
LAPriceFXOptionValue::setUpDataProvider(const LADate& basedate, LAObject& object, const LADataValuation& att) const
{
	
	LADataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider *>(LALinearRatesOptionValue::setUpDataProvider(basedate,object,att));
	

	//unit
	dh = &(object.getData(PRICING_DATA_FOREIGNQUANTITY, ISNOTNULL));
	dataProvider->unit = dynamic_cast<const LADataDouble &>(dh->get()).get();
	
	//domcurrency
	dh = &(object.getData(PRICING_DATA_DOMESTICCURRENCY, ISNOTNULL));
	dataProvider->mdomcur = dynamic_cast<LADataString &>(dh->get()).get();
	dataProvider->mdomcur.toUpper();
	if(dataProvider->mdomcur != dataProvider->mnumerairecur)
		throw LACoreInvalidData("DomesticCurrency must be the same as NumeraireCurrency",__FILE__,__LINE__);
	
	//foreigncurrency
	dh = &(object.getData(PRICING_DATA_FOREIGNCURRENCY, ISNOTNULL));
	dataProvider->mforcur = dynamic_cast<LADataString &>(dh->get()).get();
	dataProvider->mforcur.toUpper();

	//fxcur
	dataProvider->mfxcur = dataProvider->mdomcur + "/" + dataProvider->mforcur;

	//special case if we use libor df for fxoption
	dh = &(object.getData(PRICING_DATA_ISLIBORDISCOUNTFORFXOPTION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->mIsLiborDFForFX = dynamic_cast<LADataBool &>(dh->get()).get();

	dataProvider->mSpotDate = dataProvider->mAsofDate;

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

std::vector< std::vector<AnalyticParam*> >
LAPriceFXOptionValue::createAnalyticParam(LAObject& object, LADataProvider* dp) const
{
	(void)object;
	(void)dp;
	std::vector<AnalyticParam*> retvec(1);
	std::vector< std::vector<AnalyticParam*> > ret(1,retvec);
	ret[0][0] = new AnalyticGKParam();
	return ret;
}

void
LAPriceFXOptionValue::setUpAnalyticParam(LAObject& object, LADataProvider* dp) const
{
	LADataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dp);

	dataProvider->mSpotDate = (dataProvider->mSpotDate < dataProvider->mAsofDate) ? dataProvider->mAsofDate : dataProvider->mSpotDate;
	
	LADate mdydelivdate = (dataProvider->mSpotDate < dataProvider->mDeliveryDate) ? dataProvider->mDeliveryDate : dataProvider->mSpotDate;
	LADate mdymatudate = (dataProvider->mAsofDate < dataProvider->mMaturityDate) ? dataProvider->mMaturityDate : dataProvider->mAsofDate;

	//even if maturity date has passed, set up gkparam for obtaining the temporary volatility, forward
	//Td, Te, actT
	LAPriceDataDayCount dc(ACT_365_ISDA);
	double td = dc.getTerm(dataProvider->mSpotDate, mdydelivdate,false);
	double te = dataProvider->mBlackDayCount.getTerm(dataProvider->mAsofDate, mdymatudate,false);
	double actt = dc.getTerm(dataProvider->mAsofDate, mdymatudate,false);


	
	//get spot
	LAMathFXEntity& fxe = dynamic_cast<LAMathFXEntity &>(dataProvider->mpvanilla->getFXEntity().get().get());
	////.S .F
	double spot = fxe.getForwardRate(dataProvider->mforcur,dataProvider->mdomcur,dataProvider->mAsofDate,dataProvider->mAsofDate);
	double forward = fxe.getForwardRate(dataProvider->mforcur,dataProvider->mdomcur,dataProvider->mAsofDate,mdydelivdate);

	//.rd.rf
	double rdrate = 0.0;
	double rfrate = 0.0;

	const LAMathYieldCurve& bcurve = dataProvider->mpvanilla->getIRCurve(dataProvider->mdomcur);
	double dombasisrate = bcurve.getBasisZeroRate(dataProvider->mAsofDate,mdydelivdate);

	const LAMathYieldCurve& fcurve = dataProvider->mpvanilla->getIRCurve(dataProvider->mforcur);
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
LAPriceFXOptionValue::getAdditionalPremium(const LAObject& object, LADataProvider* dp) const
{
	LADataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dp);
	return dataProvider->mMargin;

}


/*!
	@brief create new cache class
	@return cache class
*/
LADataProvider*
LAPriceFXOptionValue::createNewDataProvider() const
{
	LAPriceFXOptionValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new LAPriceFXOptionValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}