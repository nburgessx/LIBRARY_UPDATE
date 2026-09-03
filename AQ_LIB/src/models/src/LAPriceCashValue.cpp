/*! @file
    @brief Class declaration to evaluate trade.

*/
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <algorithm>
#include "LAPriceCashValue.h"
#include "LAPricePortfolioValue.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADate.h"
#include "LADataValuation.h"
#include "LADataProcedure.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAPriceDataManager.h"
#include "LAObjectHolder.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAMathValuableEntity.h"
#include "LAMathIndexEntity.h"
#include "LAPriceDataFunction.h"
#include "LAMathFXEntity.h"
#include "LAAlgorithm.h"
#include "LAPricePayOff.h"
#include "LAPricePayOffTool.h"
#include "LAPriceAccruedInterest.h"
#include "LAMathPlainVanillaEntity.h"
#include "LABasic.h"
#include "LAMathYieldCurve.h"
#include "LAPriceDataInterpolation.h"

#include "LAPriceCouponTool.h"
#include "LAPriceCashFlowGenerator.h"

using namespace std;

#define RCV	"RCV"
#define PAY	"PAY"
#define STARTABLE	"STARTABLE"
#define CANCELABLE	"CANCELABLE"
#define CPN "CPN"
#define CPNCF "CPNCF"
#define DF2 "DF2"
#define STD     "STD"


/*!
    @brief constructor
	@param[in] pacc pointer to accured interest calculation class
*/
LAPriceCashValue::LAPriceCashValue()
{
}
/*!
    @brief copy constructor
	@param[in] v copy source 
*/
LAPriceCashValue::LAPriceCashValue(const LAPriceCashValue& v)
: LAPriceTradeValue(v)
{
}
/*!
    @brief destructor

*/
LAPriceCashValue::~LAPriceCashValue()
{
}
/*!
    @brief  Check function for this class ID

	@param[in] id this class type(function_t type)
	@return true or false
*/
bool
LAPriceCashValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_CASHVALUE ? true : LAPriceTradeValue::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

	@return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceCashValue::clone() const
{
    try 
	{
  		return new LAPriceCashValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Return this class type

	@return this class type
*/
function_t			
LAPriceCashValue::getType() const
{
	return FN_IR_CASHVALUE;
}
/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceCashValue::registerData(LAPriceDataManager& dm) const
{
	LAPriceTradeValue::registerData(dm);
}
/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to LAMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return clean price
	
*/
double
LAPriceCashValue::value(const LADate& basedate, LAObject& object, const LADataValuation& att) const
{
	LADataHolder* dh;
	bool iscalcrisk = false;
	dh = &object.getData(PRICING_DATA_ISCALCRISK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const LADataBool&>(dh->get()).get();
	
	bool istraderecalc = false;
	dh = &object.getData(PRICING_DATA_ISRECALCTRADEDATA, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		istraderecalc = dynamic_cast<const LADataBool&>(dh->get()).get();
	
	bool issetuppayoff = false;
	dh = &(object.getData(PRICING_DATA_ISSETUPPAYOFF, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		issetuppayoff = dynamic_cast<const LADataBool&>(dh->get()).get();
	
	if (!iscalcrisk || istraderecalc || issetuppayoff )
	{
		att.setDataProvider(NULL);
	}

	LAPriceCashValueDataProvider* dp = NULL;
	if (att.isNullDataProvider())
		dp = dynamic_cast<LAPriceCashValueDataProvider*>(setUpDataProvider(basedate, object, att));
	else
		dp = &dynamic_cast<LAPriceCashValueDataProvider&>(att.getDataProvider());

	LAPriceCashValueDataProvider* dataProvider = dp;

	double pv1 = dataProvider->cash;
	
	if (dataProvider->basecur != dataProvider->currency)
	{
		double fxrate = getFXEntity(object).getRate(dataProvider->currency, dataProvider->basecur, 0.);
		pv1 *= fxrate;
	}

	//////////
	//output//
	//////////
	double dirtyprice1 = pv1;

	// check option holder
	//if (!dataProvider->isoptionholder)
	//{
	//	dirtyprice1 *= -1.0;
	//}

	if (!iscalcrisk)
	{
		object.remove(PRICING_DATA_CLEANPRICE);
		object.remove(PRICING_DATA_DIRTYPRICE);
		object.remove(PRICING_DATA_ACCRUEDINTEREST);
		object.remove(PRICING_DATA_CLEANPRICESQUARE);
		object.remove(PRICING_DATA_CLEANPRICEDEVIATION);
		object.remove(PRICING_DATA_CALLTRIGGERVALUE);
		object.remove(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER);
		object.remove(PRICING_DATA_PV_LEG1);
		object.remove(PRICING_DATA_CURRENCY_LEG1);
		
		object.add(PRICING_DATA_DIRTYPRICE, new LADataDouble(dirtyprice1));
		object.add(PRICING_DATA_CURRENCY_LEG1, new LADataString(dataProvider->basecur));
	}

	return dirtyprice1;
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*
LAPriceCashValue::setUpDataProvider(const LADate& basedate, 
							LAObject& object, 
							const LADataValuation& att) const
{
	LADataHolder* dh;
	LAPriceCashValueDataProvider* dataProvider = dynamic_cast<LAPriceCashValueDataProvider*>(createNewDataProvider());
	att.setDataProvider(dataProvider);

	// base currency
	dh = &(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	dataProvider->basecur = dynamic_cast<const LADataString&>(dh->get()).get();

	dh = &(object.getData(PRICING_DATA_VALUATIONCURRENCY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->basecur = dynamic_cast<const LADataString&>(dh->get()).get();
	
	// cash
	dh = &(object.getData(PRICING_DATA_EXTRACF, ISNOTNULL));
	dataProvider->cash = dynamic_cast<const LADataDouble&>(dh->get()).get();
	dh = &(object.getData(PRICING_DATA_EXTRACFCURRENCY, ISNOTNULL));
	dataProvider->currency = dynamic_cast<const LADataString&>(dh->get()).get();

	// today
	dh = &(object.getData(PRICING_DATA_TODAY, ISNOTNULL));
	const LADate& today = dynamic_cast<const LADataDate&>(dh->get()).get();
	if (today > basedate)
	{
		//error
		throw LACoreInvalidData("basedate must be same or after today", __FILE__, __LINE__);	
	}

	//this is important flag : create isDataProvidergagerecalc flag
	object.remove(PRICING_DATA_ISRECALCTRADEDATA);
	object.add(PRICING_DATA_ISRECALCTRADEDATA,new LADataBool(true));

	return dataProvider;	
}

/*!
	@brief get FXEntity

	@param[in] object
	@return FXEntity Ref
*/
const LAMathFXEntity &
LAPriceCashValue::getFXEntity(LAObject &object) const
{
	const LADataReference &paramref = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
	const LAMathPlainVanillaEntity& pvanilla = dynamic_cast<const LAMathPlainVanillaEntity &>(paramref.get().get());
	return dynamic_cast<const LAMathFXEntity& >(pvanilla.getFXEntity().get().get());
}




/*!
	@brief create new cache class
	@return cache class
*/
LADataProvider*
LAPriceCashValue::createNewDataProvider() const
{
	LAPriceCashValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new LAPriceCashValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

LAPriceCashValue::LAPriceCashValueDataProvider::~LAPriceCashValueDataProvider()
{
}
