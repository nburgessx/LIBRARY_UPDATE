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
#include "LAPriceFXKnockoutRebateValue.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LALinearRatesOptionValueDataProvider.h"

using namespace std;

LAPriceFXKnockoutRebateValue::LAPriceFXKnockoutRebateValue()
: LAPriceFXOptionValue()
{}

//LAPriceFXKnockoutRebateValue::LAPriceFXKnockoutRebateValue(LAPriceFXKnockoutRebateValue& v)
//: LALinearRatesOptionValue(v)
//{}

LAPriceFXKnockoutRebateValue::~LAPriceFXKnockoutRebateValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAPriceFXKnockoutRebateValue::getType() const
{
	return FN_FXKNOCKOUTREBATEVALUE;
}
/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceFXKnockoutRebateValue::isTypeOf(function_t id) const
{
	return (id == FN_FXKNOCKOUTREBATEVALUE ? true : LAPriceFXOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
LAString 
LAPriceFXKnockoutRebateValue::getOptionPayoffName() const
{
	return FN_FXKNOCKOUTREBATEVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXKnockoutRebateValue::registerData(LAPriceDataManager& dm) const
{

	LAPriceFXOptionValue::registerData(dm);
}

LACoreFunctionBase*
LAPriceFXKnockoutRebateValue::clone() const
{
    try 
	{
		return new LAPriceFXKnockoutRebateValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
LAPriceFXKnockoutRebateValue::calcOption(const LADataValuation& att, LADataProvider* dp, LAObject& e) const
{
	LAPriceFXKnockoutRebateValueDataProvider* dataProvider = &dynamic_cast<LAPriceFXKnockoutRebateValueDataProvider &>(att.getDataProvider());
	//this means call spread value


	AnalyticSBParam* touch = dynamic_cast<AnalyticSBParam* >(dataProvider->mParam[0][0]);
		
	//basis rate forward df
	double basisforwarddf =  LAMath::exp(touch->rd * touch->Td);
	
	double touchprob = 1.0 - dataProvider->mAnalyticMethod[0][0]->calc(*touch) * basisforwarddf;

	AnalyticSBParam* notouch = dynamic_cast<AnalyticSBParam* >(dataProvider->mParam[0][1]);
	//libor rate df
	double libordf = LAMath::exp(-notouch->rd * notouch->Td);
	double notouchval = (1.0-touchprob) * dataProvider->rebate * libordf;
	
	double touchval = dataProvider->mAnalyticMethod[0][1]->calc(*notouch);

	return notouchval + touchval;
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*					
LAPriceFXKnockoutRebateValue::setUpDataProvider(const LADate& basedate, LAObject& object, const LADataValuation& att) const
{
	
	LADataHolder* dh;
	LAPriceFXKnockoutRebateValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceFXKnockoutRebateValueDataProvider*>(LAPriceFXOptionValue::setUpDataProvider(basedate,object,att));
	
	dh = &(object.getData(PRICING_DATA_REBATE,ISNOTNULL));
	dataProvider->rebate = dynamic_cast<const LADataDouble &>(dh->get()).get();
	dh = &(object.getData(PRICING_DATA_LIMITVAL,ISNOTNULL));
	dataProvider->limitval = dynamic_cast<const LADataDouble &>(dh->get()).get();
	
	return dataProvider;
}

std::vector< std::vector<AnalyticParam*> >
LAPriceFXKnockoutRebateValue::createAnalyticParam(LAObject& object, LADataProvider* dp) const
{
	std::vector<AnalyticParam*> retvec(2);
	std::vector< std::vector<AnalyticParam*> > ret(1,retvec);
	ret[0][0] = new AnalyticSBParam();
	ret[0][1] = new AnalyticSBParam();
	return ret;
}

void
LAPriceFXKnockoutRebateValue::setUpAnalyticParam(LAObject& object, LADataProvider* dp) const
{
	LADataHolder* dh;
	LAPriceFXKnockoutRebateValueDataProvider* dataProvider =  dynamic_cast<LAPriceFXKnockoutRebateValueDataProvider*>(dp);

	dataProvider->mSpotDate = (dataProvider->mSpotDate < dataProvider->mAsofDate) ? dataProvider->mAsofDate : dataProvider->mSpotDate;

	LADate mdydelivdate = (dataProvider->mSpotDate < dataProvider->mDeliveryDate) ? dataProvider->mDeliveryDate : dataProvider->mSpotDate;
	LADate mdymatudate = (dataProvider->mAsofDate < dataProvider->mMaturityDate) ? dataProvider->mMaturityDate : dataProvider->mAsofDate;
	
	//Td, Te
	LAPriceDataDayCount dc(ACT_365_ISDA);
	double td = dc.getTerm(dataProvider->mSpotDate, mdydelivdate,false);
	double te = dataProvider->mBlackDayCount.getTerm(dataProvider->mAsofDate, mdymatudate,false);
	double actt = dc.getTerm(dataProvider->mAsofDate, mdymatudate,false);

	//get spot
	LAMathFXEntity& fxe = dynamic_cast<LAMathFXEntity &>(dataProvider->mpvanilla->getFXEntity().get().get());
	////.S .F
	double spot = fxe.getForwardRate(dataProvider->mforcur,dataProvider->mdomcur,dataProvider->mAsofDate,dataProvider->mAsofDate);
	//double forward = fxe.getForwardRate(dataProvider->mforcur,dataProvider->mdomcur,dataProvider->mAsofDate,dataProvider->mDeliveryDate);

	//.rd
	const LAMathYieldCurve& bcurve = dataProvider->mpvanilla->getIRCurve(dataProvider->mdomcur);
	double domliborrate = bcurve.getZeroRate(dataProvider->mAsofDate,mdydelivdate);
	double dombasisrate = bcurve.getBasisZeroRate(dataProvider->mAsofDate,mdydelivdate);

	//.rf
	const LAMathYieldCurve& fcurve = dataProvider->mpvanilla->getIRCurve(dataProvider->mforcur);
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
LADataProvider*
LAPriceFXKnockoutRebateValue::createNewDataProvider() const
{
	LAPriceFXKnockoutRebateValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new LAPriceFXKnockoutRebateValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

