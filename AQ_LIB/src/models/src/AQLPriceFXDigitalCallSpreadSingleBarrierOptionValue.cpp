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
#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLMathDateCalculations.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLMathVolFuncFXStrangleSolver.h"
#include "AQLLinearRatesOptionValueDataProvider.h"

using namespace std;

AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue()
: AQLPriceFXSingleBarrierOptionValue()
{}

//AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue(AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue& v)
//: AQLLinearRatesOptionValue(v)
//{}

AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::~AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::getType() const
{
	return FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE;
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE ? true : AQLPriceFXSingleBarrierOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
AQLString 
AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::getOptionPayoffName() const
{
	return FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::registerData(AQLPriceDataManager& dm) const
{
	AQLPriceFXSingleBarrierOptionValue::registerData(dm);
	dm.setData(PRICING_DATA_DIGITALPAYOFF, DATA_DOUBLE);
	dm.setData(PRICING_DATA_DIGITALSPREAD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_BUYSELLDISTINGUISH, DATA_BOOL);
	
}

AQLCoreFunctionBase*
AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::clone() const
{
    try 
	{
		return new AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::calcOption(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const
{
	return AQLPriceFXOptionValue::calcFXDigitalCallSpreadOption(att,dataProvider,e);
}

// calc payoff after maturity
double				
AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const
{
	return AQLPriceFXOptionValue::calcPayOffFXDigitalCallSpreadAterMaturity(att,dataProvider,e);	
}





/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	AQLPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<AQLPriceFXOptionValueDataProvider *>(AQLPriceFXSingleBarrierOptionValue::setUpDataProvider(basedate,object,att));

	dh = &(object.getData(PRICING_DATA_DIGITALPAYOFF, ISNOTNULL));
	double digpayoff = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	
	dh = &(object.getData(PRICING_DATA_DIGITALSPREAD, ISNOTNULL));
	double spread = dynamic_cast<AQLDataDouble &>(dh->get()).get();
	if (spread == 0.0)
		throw AQLCoreInvalidData("0 Digital Spread",__FILE__,__LINE__);

	//adjust unit
	dataProvider->unit *= digpayoff/spread;

	dh = &(object.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	AQLString optiontype = dynamic_cast<AQLDataString &>(dh->get()).get();
	optiontype.toUpper();
	bool iscall = (optiontype == "CALL");

	//buyselldistinguish
	dh = &(object.getData(PRICING_DATA_BUYSELLDISTINGUISH, ISNOTNULL));
	bool isdiffbybuysell = dynamic_cast<AQLDataBool &>(dh->get()).get();


	dh = &(object.getData(PRICING_DATA_STRIKE, ISNOTNULL));
	double orgstrike = dynamic_cast<AQLDataDouble &>(dh->get()).get();
	if (dataProvider->buysell && iscall)
	{
		dataProvider->mParam[0][0]->K = orgstrike;
		dataProvider->mParam[0][1]->K = orgstrike + spread;
	
	}
	else if(!dataProvider->buysell && iscall)
	{
		if (isdiffbybuysell)
		{
			dataProvider->mParam[0][0]->K = orgstrike;
			dataProvider->mParam[0][1]->K = orgstrike - spread;
		}
		else
		{
			dataProvider->mParam[0][0]->K = orgstrike + spread;
			dataProvider->mParam[0][1]->K = orgstrike;
		}

	}
	else if(dataProvider->buysell && !iscall)
	{
		if (isdiffbybuysell)
		{
			dataProvider->mParam[0][0]->K = orgstrike;
			dataProvider->mParam[0][1]->K = orgstrike - spread;
		}
		else
		{
			dataProvider->mParam[0][0]->K = orgstrike + spread;
			dataProvider->mParam[0][1]->K = orgstrike;
		}
	
	}
	else if(!dataProvider->buysell && !iscall)
	{
		dataProvider->mParam[0][0]->K = orgstrike;
		dataProvider->mParam[0][1]->K = orgstrike + spread;
	}

	//when call spread valuation rebate value should not be cancelled
	//therefore we should set the other rebateval as 0.
	AnalyticSBParam* pm = dynamic_cast<AnalyticSBParam* >(dataProvider->mParam[0][1]);
	pm->R = 0.0;


	
	
	
	return dataProvider;
}

std::vector< std::vector<AnalyticParam*> >
AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dataProvider) const
{
	(void)object;(void)dataProvider;
	std::vector<AnalyticParam*> retvec(2);
	std::vector< std::vector<AnalyticParam*> > ret(1, retvec);
	ret[0][0] = new AnalyticSBParam();
	ret[0][1] = new AnalyticSBParam();
	return ret;
}
