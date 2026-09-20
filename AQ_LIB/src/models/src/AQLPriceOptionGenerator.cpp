/*! @file
    @brief Implementation of class to generate cashlets and trigger/call schedule
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceOptionGenerator.h"
#include "AQLPriceCashFlowGenerator.h"

#include "AQLObject.h"
#include "AQLDataProcedure.h"
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

#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLConstant.h"

#include "AQLDateCalculations.h"
#include "AQLPriceCFGenUtility.h"

#include "AQLMathIndexEntity.h"
#include "AQLMathPathEntity.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"

#include "AQLMathPlainVanillaEntity.h"
#include "AQLInterpolationBase.h"
#include "AQLCompoundingFunc.h"
#include "AQLLinearFunc.h"
#include "AQLLinearRatesOptionValueDataProvider.h"

#include "AQLPriceConvergenceValue.h"
#include "AQLMathValuableEntity.h"
#include "AQLLinearRatesOptionValue.h"


using namespace std;

#define OPTION		"Option"
#define CASHLET		"Cashlet"
#define COUPONINFO	"CouponInfo"
#define INDEXINFO	"IndexInfo"
#define ODDINDEXINFO	"OddIndexInfo"
#define RANGEACCUREINDEXINFO	"RangeAccrueIndexInfo"
#define RANGEACCUREBOUNDARYINDEXINFO	"RangeAccrueBoundaryIndexInfo"
#define BUSINESS_DAYS "BUSINESS_DAYS"
#define LEG_S		"Leg"
#define EQUALIZATION	"EQUALIZATION"
#define AMOUNTSETTING	"AMOUNTSETTING"

#define DETAIL "DETAIL"
#define MANUAL	"MANUAL"
#define PAYMENTDATE "PAYMENTDATE"

#define BOTH	"BOTH"

#define FIXED_RATE	"FIXEDRATE"
#define OIS	"OIS"

#define FRN	"FRN"
#define ESTIMATION	"ESTIMATION"

#ifndef	CURRENTINDEX
#define CURRENTINDEX		"CurrentIndex"
#endif
#ifndef	CLOSESTINDEX
#define CLOSESTINDEX		"ClosestIndex"
#endif
#ifndef	NEXTINDEX
#define NEXTINDEX			"NextIndex"
#endif
#ifndef	PREVIOUSINDEX
#define PREVIOUSINDEX		"PreviousIndex"
#endif
#ifndef	STARTENDDATEMETHOD
#define STARTENDDATEMETHOD	"StartEndDateMethod"
#endif
#ifndef	SPOTRATERATIOMETHOD
#define SPOTRATERATIOMETHOD "SpotRateRatioMethod"
#endif
#ifndef	GRIDTERMRATIOMETHOD
#define GRIDTERMRATIOMETHOD "GridTermRatioMethod"
#endif

///////////////////////////////////////////////////////////////////////
/*!
    @brief default constructor
*/
AQLPriceOptionGenerator::AQLPriceOptionGenerator()
: AQLPriceCashFlowGenerator()
{
}
/*!
    @brief destructor

*/
AQLPriceOptionGenerator::~AQLPriceOptionGenerator()
{
}
/*!
    @brief  Check this class ID is the same or not

	@param[in] id function ID
	@return true or false
*/
bool
AQLPriceOptionGenerator::isTypeOf(function_t id) const
{
	return (id == FN_IR_OPTIONGENERATOR ? true :
				AQLPriceCashFlowGenerator::isTypeOf(id));
}
/*!
    @brief  Copy this class

	@return pointer to copied object
*/
AQLCoreFunctionBase*		
AQLPriceOptionGenerator::clone() const
{
    try 
	{
        return new AQLPriceOptionGenerator();
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Retern class type

	@return class type
*/
function_t			
AQLPriceOptionGenerator::getType() const
{
	return FN_IR_OPTIONGENERATOR;
}

/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceOptionGenerator::registerData(AQLPriceDataManager& dm) const
{
	AQLPriceCashFlowGenerator::registerData(dm);
	dm.setData(CALIBRATION_DATA_OPTIONLEGS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_OPTIONINFORS, DATA_MULTIREFERENCE);

	
	
	
}


/*!
	@brief Generate CashFlow

	@param[in] basedate basedate
	@param[in,out] leg evaluated Object 
	@param[in] att Data with estimation procedure class 

	@note basedate is not used in estimation
*/
void	            
AQLPriceOptionGenerator::calibrateModel(const AQLDate& basedate, 
										AQLObject& object, 
										const AQLDataProcedure& att) const
{

	(void)basedate; (void)att; // gcc
	//setup leg maturity
	setUpLegMaturity(object);


	AQLDataHolder* dh;


	//leg object
	dh = &(object.getData(CALIBRATION_DATA_OPTIONLEGS, ISNOTNULL));
	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference&>(dh->get());
	//create Cashlets
	for (unsigned int i = 0; i < legs.getSize(); i++)
		createOptions(legs.get(i).get(), object);

	object.remove(CALIBRATION_DATA_OPTIONLEGS);
}

/*!
	@brief Create cashlets

	@param[in, out] leg object
*/
void
AQLPriceOptionGenerator::createOptions(AQLObject& leg, AQLObject& trade) const  
{
	AQLDataHolder* dh;

	//input type
	dh = &(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
	AQLString schtype = dynamic_cast<const AQLDataString&>(dh->get()).get();
	schtype.toUpper();
	if (schtype == MANUAL) return;//manual input

    const AQLString& freq         = dynamic_cast<const AQLDataString&>(leg.getData(PRICING_DATA_FREQUENCY, ISNOTNULL).get()).get();
    const AQLDate* first_odd_date = (dh=&leg.getData(PRICING_DATA_FIRSTODDDATE))->isDefined() && ! dh->isNull() ? &dynamic_cast<const AQLDataDate&>(dh->get()).get() : NULL;
    const AQLDate* last_odd_date  = (dh=&leg.getData(PRICING_DATA_LASTODDDATE))->isDefined() && ! dh->isNull() ? &dynamic_cast<const AQLDataDate&>(dh->get()).get() : NULL;
    const AQLDate& start_date     = dynamic_cast<const AQLDataDate&>(leg.getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
    const AQLDate& end_date       = dynamic_cast<const AQLDataDate&>(leg.getData(PRICING_DATA_ENDDATE, ISNOTNULL).get()).get();

	// calc cf dates and interest calc span
	DateVector paymentdates, paymentdates_unadjust, startdates, enddates, startdates_unadjust, enddates_unadjust;
	calcPaymentDates(leg, paymentdates, paymentdates_unadjust, startdates, startdates_unadjust, enddates, enddates_unadjust, start_date, 
		end_date, freq, first_odd_date, last_odd_date);

	// cfsize
	unsigned int cfsize = paymentdates.size();

	// calc notoinal array
	DoubleArray notional_array;
	//calcNotionalArray(leg, cfsize, notional_array);

	// name of this leg object
	dh = &(leg.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
	const AQLString& leg_name = dynamic_cast<const AQLDataString&>(dh->get()).get();

	// object pool
	AQLObjectPool& objPool = leg.getDataInstance()->getObjectPool();

	//// isArrear payment
	//dh = &(leg.getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
	//const AQLString& timing = dynamic_cast<const AQLDataString&>(dh->get()).get(); 
	//bool isarrear = AQLPriceCFGenUtility::isArrear(timing);

	//// isNotionalChangeAtStat
	//dh = &(leg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, ISNOTNULL));
	//bool isnotionalchangestart = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	//
	//// isNotionalChangeAtEnd
	//dh = &(leg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, ISNOTNULL));
	//bool isnotionalchangeend = dynamic_cast<const AQLDataBool&>(dh->get()).get();



	////////////////////
	// create cashlet //
	////////////////////
	AQLString cashlet_str;
	unsigned int k = 0;//for interest calc startdate and enddate position
	for (unsigned int i = 0; i < cfsize; i++)
	{
		if (i != 0) cashlet_str += ":";
		
		// name of this cashlet
		AQLString cashlet_name = leg_name + "_" + OPTION + AQLDataInt(i + 1).convertToString();
		cashlet_str += cashlet_name;

		//copy option object
		dh = &(leg.getData(PRICING_DATA_OPTIONINFORS, ISNOTNULL));
		AQLDataMultiReference & optioninfos = dynamic_cast<AQLDataMultiReference &>(dh->get());
		if (optioninfos.getSize() != 1)
			throw AQLCoreInvalidData("OptionInfo size Error",__FILE__,__LINE__);

		if (!optioninfos.get(0).get().isTypeOf(ENTITY_VENTITY))
			throw AQLCoreInvalidData("OptionInfo valuemethod Error",__FILE__,__LINE__);

		AQLMathObjectValue* opval = dynamic_cast<AQLMathObjectValue *>(optioninfos.get(0).get().clone());
		// name
		opval->getName().convertFromString(cashlet_name);
		
		
		objPool.set(cashlet_name, opval);

		// register cashlet to object pool
		// delivery date
		opval->AQLObject::add(PRICING_DATA_DELIVERYDATE, new AQLDataDate(paymentdates[i]));

		//get expiry date
		AQLDate expirydate;
		AQLDate expirybasedate;

		//if maturity date exists in leg, it should be regarded as first priority date
		dh = &(leg.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			expirydate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
			opval->AQLObject::add(PRICING_DATA_EXPIRYDATE, new AQLDataDate(expirydate));
			continue;
		}

		
		dh = &(leg.getData(PRICING_DATA_ISFIXINGPAYMENTDATEBASE, NOCHECK));
		if (dh->isDefined() && !dh->isNull()
			&& dynamic_cast<const AQLDataBool&>(dh->get()).get() == true)
			expirybasedate = paymentdates[i];
		else
			expirybasedate = paymentdates_unadjust[i];	

		dh = &(leg.getData(PRICING_DATA_OFFSET, ISNOTNULL));
		int offset = dynamic_cast<const AQLDataInt&>(dh->get()).get();		
		if (offset == 0) 
			expirydate = expirybasedate;
		else
		{
			//calendar
			AQLPriceDataCalendar cal;
			dh = &(leg.getData(PRICING_DATA_FIXINGCALENDAR, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				cal = dynamic_cast<const AQLPriceDataCalendar&>(dh->get());
			}
			else
			{
				dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
				cal = dynamic_cast<const AQLPriceDataCalendar&>(dh->get());
			}
			expirydate = cal.getBusinessDay(expirybasedate, -offset);
		}
		
		opval->AQLObject::add(PRICING_DATA_EXPIRYDATE, new AQLDataDate(expirydate));
	}

	AQLString underlyings_str;
	dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		AQLDataMultiReference &refs = dynamic_cast<AQLDataMultiReference &>(dh->get());
		underlyings_str = dh->convertToString();
		int locate = underlyings_str.findString('"');
		underlyings_str = underlyings_str.subString(1,underlyings_str.size()-2);
		underlyings_str += ":" + cashlet_str;
	}
	else
		underlyings_str = cashlet_str;

	trade.remove(CALIBRATION_DATA_UNDERLYINGS);
	dh = &trade.add(CALIBRATION_DATA_UNDERLYINGS, new AQLDataMultiReference());
	dh->convertFromString(underlyings_str);

}

/*!
	@brief	setup leg maturity 
	@param[in, out] trade trade object

*/
void
AQLPriceOptionGenerator::setUpLegMaturity(AQLObject& trade) const
{
	AQLDataHolder* dh;
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_OPTIONLEGS, ISNOTNULL));
	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference&>(dh->get());

	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_ENDDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) continue;
		
		//set end date
		dh = &(legs.get(i).getData(PRICING_DATA_ENDTERM, ISNOTNULL));
		//term
		const AQLString& termstr = dynamic_cast<const AQLDataString&>(dh->get()).get();
		//start date
		dh = &(legs.get(i).getData(PRICING_DATA_STARTDATE, ISNOTNULL));
		const AQLDate& startdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
		//end date
		const AQLDate& enddate = AQLDateCalculations::getDate(startdate, termstr, true);
		dh = &(legs.get(i).getData(PRICING_DATA_ENDDATE, NOCHECK));
		if (!dh->isDefined())
			dh = &(legs.get(i).add(PRICING_DATA_ENDDATE, new AQLDataDate()));
		dynamic_cast<AQLDataDate&>(dh->get()).set(enddate);
		
	}
}



