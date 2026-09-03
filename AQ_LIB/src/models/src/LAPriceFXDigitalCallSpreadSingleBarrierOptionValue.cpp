//  2009, AlgoQuantHub.
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
#include "LAMathDateCalculations.h"
#include "AQLCoreComponentManager.h"
#include "LAPriceFXDigitalCallSpreadSingleBarrierOptionValue.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LALinearRatesOptionValueDataProvider.h"

using namespace std;

LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::LAPriceFXDigitalCallSpreadSingleBarrierOptionValue()
: LAPriceFXSingleBarrierOptionValue()
{}

//LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::LAPriceFXDigitalCallSpreadSingleBarrierOptionValue(LAPriceFXDigitalCallSpreadSingleBarrierOptionValue& v)
//: LALinearRatesOptionValue(v)
//{}

LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::~LAPriceFXDigitalCallSpreadSingleBarrierOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::getType() const
{
	return FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE;
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE ? true : LAPriceFXSingleBarrierOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
AQLString 
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::getOptionPayoffName() const
{
	return FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::registerData(AQLPriceDataManager& dm) const
{
	LAPriceFXSingleBarrierOptionValue::registerData(dm);
	dm.setData(PRICING_DATA_DIGITALPAYOFF, DATA_DOUBLE);
	dm.setData(PRICING_DATA_DIGITALSPREAD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_BUYSELLDISTINGUISH, DATA_BOOL);
	
}

AQLCoreFunctionBase*
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::clone() const
{
    try 
	{
		return new LAPriceFXDigitalCallSpreadSingleBarrierOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::calcOption(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const
{
	return LAPriceFXOptionValue::calcFXDigitalCallSpreadOption(att,dataProvider,e);
}

// calc payoff after maturity
double				
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const
{
	return LAPriceFXOptionValue::calcPayOffFXDigitalCallSpreadAterMaturity(att,dataProvider,e);	
}





/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider *>(LAPriceFXSingleBarrierOptionValue::setUpDataProvider(basedate,object,att));

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
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dataProvider) const
{
	(void)object;(void)dataProvider;
	std::vector<AnalyticParam*> retvec(2);
	std::vector< std::vector<AnalyticParam*> > ret(1, retvec);
	ret[0][0] = new AnalyticSBParam();
	ret[0][1] = new AnalyticSBParam();
	return ret;
}
