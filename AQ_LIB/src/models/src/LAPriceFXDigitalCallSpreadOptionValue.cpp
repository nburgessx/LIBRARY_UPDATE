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
#include "LABasic.h"
#include "LACoreComponentManager.h"
#include "LAPriceFXDigitalCallSpreadOptionValue.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LALinearRatesOptionValueDataProvider.h"

using namespace std;

LAPriceFXDigitalCallSpreadOptionValue::LAPriceFXDigitalCallSpreadOptionValue()
: LAPriceFXOptionValue()
{}

//LAPriceFXDigitalCallSpreadOptionValue::LAPriceFXDigitalCallSpreadOptionValue(LAPriceFXDigitalCallSpreadOptionValue& v)
//: LALinearRatesOptionValue(v)
//{}

LAPriceFXDigitalCallSpreadOptionValue::~LAPriceFXDigitalCallSpreadOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAPriceFXDigitalCallSpreadOptionValue::getType() const
{
	return FN_FXDIGITALCALLSPREADOPTIONVALUE;
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceFXDigitalCallSpreadOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_FXDIGITALCALLSPREADOPTIONVALUE ? true : LAPriceFXOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
LAString 
LAPriceFXDigitalCallSpreadOptionValue::getOptionPayoffName() const
{
	return FN_FXDIGITALCALLSPREADOPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXDigitalCallSpreadOptionValue::registerData(LAPriceDataManager& dm) const
{
	LALinearRatesOptionValue::registerData(dm);
	dm.setData(PRICING_DATA_DIGITALPAYOFF, DATA_DOUBLE);
	dm.setData(PRICING_DATA_DIGITALSPREAD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_BUYSELLDISTINGUISH, DATA_BOOL);
	
}

LACoreFunctionBase*
LAPriceFXDigitalCallSpreadOptionValue::clone() const
{
    try 
	{
		return new LAPriceFXDigitalCallSpreadOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
LAPriceFXDigitalCallSpreadOptionValue::calcOption(const LADataValuation& att, LADataProvider* dataProvider, LAObject& e) const
{
	return LAPriceFXOptionValue::calcFXDigitalCallSpreadOption(att,dataProvider,e);
}

// calc payoff after maturity
double				
LAPriceFXDigitalCallSpreadOptionValue::calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dataProvider, LAObject& e) const
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
LAPriceFXDigitalCallSpreadOptionValue::setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const
{
	
	LADataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider *>(LAPriceFXOptionValue::setUpDataProvider(basedate,object,att));

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

	return dataProvider;
}


std::vector< std::vector<AnalyticParam*> >
LAPriceFXDigitalCallSpreadOptionValue::createAnalyticParam(LAObject& object, LADataProvider* dataProvider) const
{
	//size 2 means
	std::vector<AnalyticParam*> retvec(2);
	std::vector< std::vector<AnalyticParam*> > ret(1, retvec);
	//std::vector<AnalyticParam*> ret(2);
	ret[0][0] = new AnalyticGKParam();
	ret[0][1] = new AnalyticGKParam();
	
	return ret;
}



