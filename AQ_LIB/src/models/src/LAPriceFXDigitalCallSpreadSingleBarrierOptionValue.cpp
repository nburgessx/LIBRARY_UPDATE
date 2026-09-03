//  2009, Mizuho International London.
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
LAString 
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::getOptionPayoffName() const
{
	return FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::registerData(LAPriceDataManager& dm) const
{
	LAPriceFXSingleBarrierOptionValue::registerData(dm);
	dm.setData(PRICING_DATA_DIGITALPAYOFF, DATA_DOUBLE);
	dm.setData(PRICING_DATA_DIGITALSPREAD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_BUYSELLDISTINGUISH, DATA_BOOL);
	
}

LACoreFunctionBase*
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::clone() const
{
    try 
	{
		return new LAPriceFXDigitalCallSpreadSingleBarrierOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::calcOption(const LADataValuation& att, LADataProvider* dataProvider, LAObject& e) const
{
	return LAPriceFXOptionValue::calcFXDigitalCallSpreadOption(att,dataProvider,e);
}

// calc payoff after maturity
double				
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dataProvider, LAObject& e) const
{
	return LAPriceFXOptionValue::calcPayOffFXDigitalCallSpreadAterMaturity(att,dataProvider,e);	
}





/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*					
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const
{
	
	LADataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider *>(LAPriceFXSingleBarrierOptionValue::setUpDataProvider(basedate,object,att));

	dh = &(object.getData(PRICING_DATA_DIGITALPAYOFF, ISNOTNULL));
	double digpayoff = dynamic_cast<const LADataDouble &>(dh->get()).get();
	
	dh = &(object.getData(PRICING_DATA_DIGITALSPREAD, ISNOTNULL));
	double spread = dynamic_cast<LADataDouble &>(dh->get()).get();
	if (spread == 0.0)
		throw LACoreInvalidData("0 Digital Spread",__FILE__,__LINE__);

	//adjust unit
	dataProvider->unit *= digpayoff/spread;

	dh = &(object.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	LAString optiontype = dynamic_cast<LADataString &>(dh->get()).get();
	optiontype.toUpper();
	bool iscall = (optiontype == "CALL");

	//buyselldistinguish
	dh = &(object.getData(PRICING_DATA_BUYSELLDISTINGUISH, ISNOTNULL));
	bool isdiffbybuysell = dynamic_cast<LADataBool &>(dh->get()).get();


	dh = &(object.getData(PRICING_DATA_STRIKE, ISNOTNULL));
	double orgstrike = dynamic_cast<LADataDouble &>(dh->get()).get();
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
LAPriceFXDigitalCallSpreadSingleBarrierOptionValue::createAnalyticParam(LAObject& object, LADataProvider* dataProvider) const
{
	(void)object;(void)dataProvider;
	std::vector<AnalyticParam*> retvec(2);
	std::vector< std::vector<AnalyticParam*> > ret(1, retvec);
	ret[0][0] = new AnalyticSBParam();
	ret[0][1] = new AnalyticSBParam();
	return ret;
}
