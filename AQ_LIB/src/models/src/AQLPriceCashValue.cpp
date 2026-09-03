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
#include "AQLPriceCashValue.h"
#include "AQLPricePortfolioValue.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLDate.h"
#include "AQLDataValuation.h"
#include "AQLDataProcedure.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectHolder.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLMathValuableEntity.h"
#include "AQLMathIndexEntity.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathFXEntity.h"
#include "AQLAlgorithm.h"
#include "AQLPricePayOff.h"
#include "AQLPricePayOffTool.h"
#include "AQLPriceAccruedInterest.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLBasic.h"
#include "AQLMathYieldCurve.h"
#include "AQLPriceDataInterpolation.h"

#include "AQLPriceCouponTool.h"
#include "AQLPriceCashFlowGenerator.h"

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
AQLPriceCashValue::AQLPriceCashValue()
{
}
/*!
    @brief copy constructor
	@param[in] v copy source 
*/
AQLPriceCashValue::AQLPriceCashValue(const AQLPriceCashValue& v)
: AQLPriceTradeValue(v)
{
}
/*!
    @brief destructor

*/
AQLPriceCashValue::~AQLPriceCashValue()
{
}
/*!
    @brief  Check function for this class ID

	@param[in] id this class type(function_t type)
	@return true or false
*/
bool
AQLPriceCashValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_CASHVALUE ? true : AQLPriceTradeValue::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

	@return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceCashValue::clone() const
{
    try 
	{
  		return new AQLPriceCashValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Return this class type

	@return this class type
*/
function_t			
AQLPriceCashValue::getType() const
{
	return FN_IR_CASHVALUE;
}
/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceCashValue::registerData(AQLPriceDataManager& dm) const
{
	AQLPriceTradeValue::registerData(dm);
}
/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to AQLMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return clean price
	
*/
double
AQLPriceCashValue::value(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const
{
	AQLDataHolder* dh;
	bool iscalcrisk = false;
	dh = &object.getData(PRICING_DATA_ISCALCRISK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	
	bool istraderecalc = false;
	dh = &object.getData(PRICING_DATA_ISRECALCTRADEDATA, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		istraderecalc = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	
	bool issetuppayoff = false;
	dh = &(object.getData(PRICING_DATA_ISSETUPPAYOFF, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		issetuppayoff = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	
	if (!iscalcrisk || istraderecalc || issetuppayoff )
	{
		att.setDataProvider(NULL);
	}

	AQLPriceCashValueDataProvider* dp = NULL;
	if (att.isNullDataProvider())
		dp = dynamic_cast<AQLPriceCashValueDataProvider*>(setUpDataProvider(basedate, object, att));
	else
		dp = &dynamic_cast<AQLPriceCashValueDataProvider&>(att.getDataProvider());

	AQLPriceCashValueDataProvider* dataProvider = dp;

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
		
		object.add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(dirtyprice1));
		object.add(PRICING_DATA_CURRENCY_LEG1, new AQLDataString(dataProvider->basecur));
	}

	return dirtyprice1;
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*
AQLPriceCashValue::setUpDataProvider(const AQLDate& basedate, 
							AQLObject& object, 
							const AQLDataValuation& att) const
{
	AQLDataHolder* dh;
	AQLPriceCashValueDataProvider* dataProvider = dynamic_cast<AQLPriceCashValueDataProvider*>(createNewDataProvider());
	att.setDataProvider(dataProvider);

	// base currency
	dh = &(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	dataProvider->basecur = dynamic_cast<const AQLDataString&>(dh->get()).get();

	dh = &(object.getData(PRICING_DATA_VALUATIONCURRENCY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->basecur = dynamic_cast<const AQLDataString&>(dh->get()).get();
	
	// cash
	dh = &(object.getData(PRICING_DATA_EXTRACF, ISNOTNULL));
	dataProvider->cash = dynamic_cast<const AQLDataDouble&>(dh->get()).get();
	dh = &(object.getData(PRICING_DATA_EXTRACFCURRENCY, ISNOTNULL));
	dataProvider->currency = dynamic_cast<const AQLDataString&>(dh->get()).get();

	// today
	dh = &(object.getData(PRICING_DATA_TODAY, ISNOTNULL));
	const AQLDate& today = dynamic_cast<const AQLDataDate&>(dh->get()).get();
	if (today > basedate)
	{
		//error
		throw AQLCoreInvalidData("basedate must be same or after today", __FILE__, __LINE__);	
	}

	//this is important flag : create isDataProvidergagerecalc flag
	object.remove(PRICING_DATA_ISRECALCTRADEDATA);
	object.add(PRICING_DATA_ISRECALCTRADEDATA,new AQLDataBool(true));

	return dataProvider;	
}

/*!
	@brief get FXEntity

	@param[in] object
	@return FXEntity Ref
*/
const AQLMathFXEntity &
AQLPriceCashValue::getFXEntity(AQLObject &object) const
{
	const AQLDataReference &paramref = dynamic_cast<const AQLDataReference &>(object.getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
	const AQLMathPlainVanillaEntity& pvanilla = dynamic_cast<const AQLMathPlainVanillaEntity &>(paramref.get().get());
	return dynamic_cast<const AQLMathFXEntity& >(pvanilla.getFXEntity().get().get());
}




/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
AQLPriceCashValue::createNewDataProvider() const
{
	AQLPriceCashValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new AQLPriceCashValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

AQLPriceCashValue::AQLPriceCashValueDataProvider::~AQLPriceCashValueDataProvider()
{
}
