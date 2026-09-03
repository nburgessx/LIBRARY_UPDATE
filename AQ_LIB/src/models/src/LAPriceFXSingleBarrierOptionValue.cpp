//  2009, AlgoQuantHub.
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
#include "LAPriceFXSingleBarrierOptionValue.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LALinearRatesOptionValueDataProvider.h"

using namespace std;

LAPriceFXSingleBarrierOptionValue::LAPriceFXSingleBarrierOptionValue()
: LAPriceFXOptionValue()
{}

//LAPriceFXSingleBarrierOptionValue::LAPriceFXSingleBarrierOptionValue(LAPriceFXSingleBarrierOptionValue& v)
//: LALinearRatesOptionValue(v)
//{}

LAPriceFXSingleBarrierOptionValue::~LAPriceFXSingleBarrierOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAPriceFXSingleBarrierOptionValue::getType() const
{
	return FN_FXSINGLEBARRIEROPTIONVALUE;
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceFXSingleBarrierOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_FXSINGLEBARRIEROPTIONVALUE ? true : LAPriceFXOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
LAString 
LAPriceFXSingleBarrierOptionValue::getOptionPayoffName() const
{
	return FN_FXSINGLEBARRIEROPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXSingleBarrierOptionValue::registerData(LAPriceDataManager& dm) const
{

	LALinearRatesOptionValue::registerData(dm);
	dm.setData(PRICING_DATA_REBATEPAYMENTTIME, DATA_STRING);
	dm.setData(PRICING_DATA_REBATEPAYMENTCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_REBATEPAYMENTOFFSET, DATA_INT);
	dm.setData(PRICING_DATA_REBATEPAYMENTCALENDAR, DATA_CALENDAR);
}

LACoreFunctionBase*
LAPriceFXSingleBarrierOptionValue::clone() const
{
    try 
	{
		return new LAPriceFXSingleBarrierOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}





/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*					
LAPriceFXSingleBarrierOptionValue::setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const
{
	
	LADataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider *>(LAPriceFXOptionValue::setUpDataProvider(basedate,object,att));

	//Limit val
	dh = &(object.getData(PRICING_DATA_LIMITVAL, ISNOTNULL));
	double limitval = dynamic_cast<LADataDouble &>(dh->get()).get();
	//option type
	int optypenum = 1;
	dh = &(object.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	LAString optiontype = dynamic_cast<LADataString &>(dh->get()).get();
	optiontype.toUpper();
	if (LAString("CALL") == optiontype)
		optypenum = 1;
	else
		optypenum = -1;

	//up and down
	int updownnum = 1;
	dh = &(object.getData(PRICING_DATA_UPANDDOWN, ISNOTNULL));
	LAString updown = dynamic_cast<LADataString &>(dh->get()).get();
	updown.toUpper();
	if (LAString("UP") == updown)
		updownnum = -1;
	else
		updownnum = 1;

	double rebateval = 0.0;
	int knocktimenum = 1;
	//judge in or out if in we don't need rebateval;
	dh = &(object.getData(PRICING_DATA_INANDOUT, ISNOTNULL));
	LAString inout = dynamic_cast<LADataString &>(dh->get()).get();
	inout.toUpper();
	if (LAString("IN") != inout)
	{
		//Rebate
		dh = &(object.getData(PRICING_DATA_REBATE,ISNOTNULL));
		 rebateval = dynamic_cast<LADataDouble &>(dh->get()).get();

		//knockout time
		dh = &(object.getData(PRICING_DATA_REBATEPAYMENTTIME, ISNOTNULL));
		LAString retimestr = dynamic_cast<LADataString &>(dh->get()).get();
		retimestr.toUpper();
		if (retimestr == "KNOCKTIME")
			knocktimenum = 1;
		else if(retimestr == "MATURITYTIME")
			knocktimenum = 0;
		else 
			throw LACoreInvalidData("RebatePaymentTime Error",__FILE__,__LINE__);
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
LAPriceFXSingleBarrierOptionValue::createAnalyticParam(LAObject& object, LADataProvider* dataProvider) const
{
	(void)object;(void)dataProvider;
	std::vector<AnalyticParam*> retvec(1);
	std::vector< std::vector<AnalyticParam*> > ret(1,retvec);
	ret[0][0] = new AnalyticSBParam();
	return ret;
}
