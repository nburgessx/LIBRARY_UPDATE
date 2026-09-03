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
#include "AQLBasic.h"
#include "AQLCoreComponentManager.h"
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
AQLString 
LAPriceFXDigitalCallSpreadOptionValue::getOptionPayoffName() const
{
	return FN_FXDIGITALCALLSPREADOPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXDigitalCallSpreadOptionValue::registerData(AQLPriceDataManager& dm) const
{
	LALinearRatesOptionValue::registerData(dm);
	dm.setData(PRICING_DATA_DIGITALPAYOFF, DATA_DOUBLE);
	dm.setData(PRICING_DATA_DIGITALSPREAD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_BUYSELLDISTINGUISH, DATA_BOOL);
	
}

AQLCoreFunctionBase*
LAPriceFXDigitalCallSpreadOptionValue::clone() const
{
    try 
	{
		return new LAPriceFXDigitalCallSpreadOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
LAPriceFXDigitalCallSpreadOptionValue::calcOption(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const
{
	return LAPriceFXOptionValue::calcFXDigitalCallSpreadOption(att,dataProvider,e);
}

// calc payoff after maturity
double				
LAPriceFXDigitalCallSpreadOptionValue::calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const
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
LAPriceFXDigitalCallSpreadOptionValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider *>(LAPriceFXOptionValue::setUpDataProvider(basedate,object,att));

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

	return dataProvider;
}


std::vector< std::vector<AnalyticParam*> >
LAPriceFXDigitalCallSpreadOptionValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dataProvider) const
{
	//size 2 means
	std::vector<AnalyticParam*> retvec(2);
	std::vector< std::vector<AnalyticParam*> > ret(1, retvec);
	//std::vector<AnalyticParam*> ret(2);
	ret[0][0] = new AnalyticGKParam();
	ret[0][1] = new AnalyticGKParam();
	
	return ret;
}



