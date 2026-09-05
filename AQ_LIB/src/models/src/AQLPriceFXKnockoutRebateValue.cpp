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
#include "AQLPriceFXKnockoutRebateValue.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLMathVolFuncFXStrangleSolver.h"
#include "AQLLinearRatesOptionValueDataProvider.h"

using namespace std;

AQLPriceFXKnockoutRebateValue::AQLPriceFXKnockoutRebateValue()
: AQLPriceFXOptionValue()
{}

//AQLPriceFXKnockoutRebateValue::AQLPriceFXKnockoutRebateValue(AQLPriceFXKnockoutRebateValue& v)
//: AQLLinearRatesOptionValue(v)
//{}

AQLPriceFXKnockoutRebateValue::~AQLPriceFXKnockoutRebateValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLPriceFXKnockoutRebateValue::getType() const
{
	return FN_FXKNOCKOUTREBATEVALUE;
}
/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceFXKnockoutRebateValue::isTypeOf(function_t id) const
{
	return (id == FN_FXKNOCKOUTREBATEVALUE ? true : AQLPriceFXOptionValue::isTypeOf(id));
}

// vanna-volga
/*
    @brief get option method name
     @return option method name
*/
AQLString 
AQLPriceFXKnockoutRebateValue::getOptionPayoffName() const
{
	return FN_FXKNOCKOUTREBATEVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceFXKnockoutRebateValue::registerData(AQLPriceDataManager& dm) const
{

	AQLPriceFXOptionValue::registerData(dm);
}

AQLCoreFunctionBase*
AQLPriceFXKnockoutRebateValue::clone() const
{
    try 
	{
		return new AQLPriceFXKnockoutRebateValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
AQLPriceFXKnockoutRebateValue::calcOption(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const
{
	AQLPriceFXKnockoutRebateValueDataProvider* dataProvider = &dynamic_cast<AQLPriceFXKnockoutRebateValueDataProvider &>(att.getDataProvider());
	//this means call spread value


	AnalyticSBParam* touch = dynamic_cast<AnalyticSBParam* >(dataProvider->mParam[0][0]);
		
	//basis rate forward df
	double basisforwarddf =  AQLMath::exp(touch->rd * touch->Td);
	
	double touchprob = 1.0 - dataProvider->mAnalyticMethod[0][0]->calc(*touch) * basisforwarddf;

	AnalyticSBParam* notouch = dynamic_cast<AnalyticSBParam* >(dataProvider->mParam[0][1]);
	//libor rate df
	double libordf = AQLMath::exp(-notouch->rd * notouch->Td);
	double notouchval = (1.0-touchprob) * dataProvider->rebate * libordf;
	
	double touchval = dataProvider->mAnalyticMethod[0][1]->calc(*notouch);

	return notouchval + touchval;
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLPriceFXKnockoutRebateValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	AQLPriceFXKnockoutRebateValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<AQLPriceFXKnockoutRebateValueDataProvider*>(AQLPriceFXOptionValue::setUpDataProvider(basedate,object,att));
	
	dh = &(object.getData(PRICING_DATA_REBATE,ISNOTNULL));
	dataProvider->rebate = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	dh = &(object.getData(PRICING_DATA_LIMITVAL,ISNOTNULL));
	dataProvider->limitval = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	
	return dataProvider;
}

std::vector< std::vector<AnalyticParam*> >
AQLPriceFXKnockoutRebateValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dp) const
{
	std::vector<AnalyticParam*> retvec(2);
	std::vector< std::vector<AnalyticParam*> > ret(1,retvec);
	ret[0][0] = new AnalyticSBParam();
	ret[0][1] = new AnalyticSBParam();
	return ret;
}

void
AQLPriceFXKnockoutRebateValue::setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const
{
	AQLDataHolder* dh;
	AQLPriceFXKnockoutRebateValueDataProvider* dataProvider =  dynamic_cast<AQLPriceFXKnockoutRebateValueDataProvider*>(dp);

	dataProvider->mSpotDate = (dataProvider->mSpotDate < dataProvider->mAsofDate) ? dataProvider->mAsofDate : dataProvider->mSpotDate;

	AQLDate mdydelivdate = (dataProvider->mSpotDate < dataProvider->mDeliveryDate) ? dataProvider->mDeliveryDate : dataProvider->mSpotDate;
	AQLDate mdymatudate = (dataProvider->mAsofDate < dataProvider->mMaturityDate) ? dataProvider->mMaturityDate : dataProvider->mAsofDate;
	
	//Td, Te
	AQLPriceDataDayCount dc(ACT_365_ISDA);
	double td = dc.getTerm(dataProvider->mSpotDate, mdydelivdate,false);
	double te = dataProvider->mBlackDayCount.getTerm(dataProvider->mAsofDate, mdymatudate,false);
	double actt = dc.getTerm(dataProvider->mAsofDate, mdymatudate,false);

	//get spot
	AQLMathFXEntity& fxe = dynamic_cast<AQLMathFXEntity &>(dataProvider->mpvanilla->getFXEntity().get().get());
	////.S .F
	double spot = fxe.getForwardRate(dataProvider->mforcur,dataProvider->mdomcur,dataProvider->mAsofDate,dataProvider->mAsofDate);
	//double forward = fxe.getForwardRate(dataProvider->mforcur,dataProvider->mdomcur,dataProvider->mAsofDate,dataProvider->mDeliveryDate);

	//.rd
	const AQLMathYieldCurve& bcurve = dataProvider->mpvanilla->getIRCurve(dataProvider->mdomcur);
	double domliborrate = bcurve.getZeroRate(dataProvider->mAsofDate,mdydelivdate);
	double dombasisrate = bcurve.getBasisZeroRate(dataProvider->mAsofDate,mdydelivdate);

	//.rf
	const AQLMathYieldCurve& fcurve = dataProvider->mpvanilla->getIRCurve(dataProvider->mforcur);
	double forrate = fcurve.getBasisZeroRate(dataProvider->mAsofDate,mdymatudate);


	std::vector <AnalyticSBParam *> sbparam(dataProvider->mParam[0].size());
	//set call up in
	sbparam[0] = dynamic_cast<AnalyticSBParam* >(dataProvider->mParam[0][0]);
	sbparam[0]->S = spot;
	sbparam[0]->Td = td;
	sbparam[0]->Te = te;
	sbparam[0]->rd = dombasisrate;
	sbparam[0]->rf = forrate;
	sbparam[0]->L = dataProvider->limitval;
	sbparam[0]->R = 1.0;
	sbparam[0]->K = 1000000.0;
	sbparam[0]->cp = 1;
	sbparam[0]->du = -1;
	sbparam[0]->actT = actt;
	
	//set call up out
	sbparam[1] = dynamic_cast<AnalyticSBParam* >(dataProvider->mParam[0][1]);
	sbparam[1]->S = spot;
	sbparam[1]->Td = td;
	sbparam[1]->Te = te;
	sbparam[1]->rd = domliborrate;
	sbparam[1]->rf = forrate + domliborrate - dombasisrate;
	sbparam[1]->L = dataProvider->limitval;
	sbparam[1]->R = dataProvider->rebate;
	sbparam[1]->K = 1000000.0;
	sbparam[1]->cp = 1;
	sbparam[1]->du = -1;
	sbparam[0]->actT = actt;
	
	return;
}

/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
AQLPriceFXKnockoutRebateValue::createNewDataProvider() const
{
	AQLPriceFXKnockoutRebateValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new AQLPriceFXKnockoutRebateValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

