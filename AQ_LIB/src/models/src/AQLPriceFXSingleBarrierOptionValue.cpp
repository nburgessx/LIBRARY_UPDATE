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
#include "AQLDateCalculations.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceFXSingleBarrierOptionValue.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLMathVolFuncFXStrangleSolver.h"
#include "AQLLinearRatesOptionValueDataProvider.h"

using namespace std;

AQLPriceFXSingleBarrierOptionValue::AQLPriceFXSingleBarrierOptionValue()
: AQLPriceFXOptionValue()
{}

//AQLPriceFXSingleBarrierOptionValue::AQLPriceFXSingleBarrierOptionValue(AQLPriceFXSingleBarrierOptionValue& v)
//: AQLLinearRatesOptionValue(v)
//{}

AQLPriceFXSingleBarrierOptionValue::~AQLPriceFXSingleBarrierOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLPriceFXSingleBarrierOptionValue::getType() const
{
	return FN_FXSINGLEBARRIEROPTIONVALUE;
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceFXSingleBarrierOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_FXSINGLEBARRIEROPTIONVALUE ? true : AQLPriceFXOptionValue::isTypeOf(id));
}

// vanna-volga
/*
    @brief get option method name
     @return option method name
*/
AQLString 
AQLPriceFXSingleBarrierOptionValue::getOptionPayoffName() const
{
	return FN_FXSINGLEBARRIEROPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceFXSingleBarrierOptionValue::registerData(AQLPriceDataManager& dm) const
{

	AQLLinearRatesOptionValue::registerData(dm);
	dm.setData(PRICING_DATA_REBATEPAYMENTTIME, DATA_STRING);
	dm.setData(PRICING_DATA_REBATEPAYMENTCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_REBATEPAYMENTOFFSET, DATA_INT);
	dm.setData(PRICING_DATA_REBATEPAYMENTCALENDAR, DATA_CALENDAR);
}

AQLCoreFunctionBase*
AQLPriceFXSingleBarrierOptionValue::clone() const
{
    try 
	{
		return new AQLPriceFXSingleBarrierOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}





/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLPriceFXSingleBarrierOptionValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	AQLPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<AQLPriceFXOptionValueDataProvider *>(AQLPriceFXOptionValue::setUpDataProvider(basedate,object,att));

	//Limit val
	dh = &(object.getData(PRICING_DATA_LIMITVAL, ISNOTNULL));
	double limitval = dynamic_cast<AQLDataDouble &>(dh->get()).get();
	//option type
	int optypenum = 1;
	dh = &(object.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	AQLString optiontype = dynamic_cast<AQLDataString &>(dh->get()).get();
	optiontype.toUpper();
	if (AQLString("CALL") == optiontype)
		optypenum = 1;
	else
		optypenum = -1;

	//up and down
	int updownnum = 1;
	dh = &(object.getData(PRICING_DATA_UPANDDOWN, ISNOTNULL));
	AQLString updown = dynamic_cast<AQLDataString &>(dh->get()).get();
	updown.toUpper();
	if (AQLString("UP") == updown)
		updownnum = -1;
	else
		updownnum = 1;

	double rebateval = 0.0;
	int knocktimenum = 1;
	//judge in or out if in we don't need rebateval;
	dh = &(object.getData(PRICING_DATA_INANDOUT, ISNOTNULL));
	AQLString inout = dynamic_cast<AQLDataString &>(dh->get()).get();
	inout.toUpper();
	if (AQLString("IN") != inout)
	{
		//Rebate
		dh = &(object.getData(PRICING_DATA_REBATE,ISNOTNULL));
		 rebateval = dynamic_cast<AQLDataDouble &>(dh->get()).get();

		//knockout time
		dh = &(object.getData(PRICING_DATA_REBATEPAYMENTTIME, ISNOTNULL));
		AQLString retimestr = dynamic_cast<AQLDataString &>(dh->get()).get();
		retimestr.toUpper();
		if (retimestr == "KNOCKTIME")
			knocktimenum = 1;
		else if(retimestr == "MATURITYTIME")
			knocktimenum = 0;
		else 
			throw AQLCoreInvalidData("RebatePaymentTime Error",__FILE__,__LINE__);
	}

	//for phase6.5
	std::vector <AnalyticSBParam *> sbparam(dataProvider->mParam[0].size());
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		sbparam[i] = dynamic_cast<AnalyticSBParam *>(dataProvider->mParam[0][i]);
		//Limit val
		sbparam[i]->L = limitval;
		//option type
		sbparam[i]->cp = optypenum;
		//up and down
		sbparam[i]->du = updownnum;
		//Rebate
		sbparam[i]->R = rebateval;
		//knockout time
		sbparam[i]->ReTime = knocktimenum;
	}
	return dataProvider;
	
}

std::vector< std::vector<AnalyticParam*> >
AQLPriceFXSingleBarrierOptionValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dataProvider) const
{
	(void)object;(void)dataProvider;
	std::vector<AnalyticParam*> retvec(1);
	std::vector< std::vector<AnalyticParam*> > ret(1,retvec);
	ret[0][0] = new AnalyticSBParam();
	return ret;
}
