/*! @file
    @brief Implementation of class to generate cashlets and trigger/call schedule

*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceOptionGenerator.h"
#include "LAPriceCashFlowGenerator.h"

#include "LAObject.h"
#include "LADataProcedure.h"
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

#include "LAPriceDataFunction.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LAConstant.h"

#include "LAMathDateCalculations.h"
#include "LAPriceCFGenUtility.h"

#include "LAMathIndexEntity.h"
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"

#include "LAMathPlainVanillaEntity.h"
#include "LAInterpolationBase.h"
#include "LACompoundingFunc.h"
#include "LALinearFunc.h"
#include "LALinearRatesOptionValueDataProvider.h"

#include "LAPriceConvergenceValue.h"
#include "LAMathValuableEntity.h"
#include "LALinearRatesOptionValue.h"


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
LAPriceOptionGenerator::LAPriceOptionGenerator()
: LAPriceCashFlowGenerator()
{
}
/*!
    @brief destructor

*/
LAPriceOptionGenerator::~LAPriceOptionGenerator()
{
}
/*!
    @brief  Check this class ID is the same or not

	@param[in] id function ID
	@return true or false
*/
bool
LAPriceOptionGenerator::isTypeOf(function_t id) const
{
	return (id == FN_IR_OPTIONGENERATOR ? true :
				LAPriceCashFlowGenerator::isTypeOf(id));
}
/*!
    @brief  Copy this class

	@return pointer to copied object
*/
LACoreFunctionBase*		
LAPriceOptionGenerator::clone() const
{
    try 
	{
        return new LAPriceOptionGenerator();
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Retern class type

	@return class type
*/
function_t			
LAPriceOptionGenerator::getType() const
{
	return FN_IR_OPTIONGENERATOR;
}

/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceOptionGenerator::registerData(LAPriceDataManager& dm) const
{
	LAPriceCashFlowGenerator::registerData(dm);
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
LAPriceOptionGenerator::calibrateModel(const LADate& basedate, 
										LAObject& object, 
										const LADataProcedure& att) const
{

	(void)basedate; (void)att; //20070411--Nagase--gcc
	//setup leg maturity
	setUpLegMaturity(object);


	LADataHolder* dh;


	//leg object
	dh = &(object.getData(CALIBRATION_DATA_OPTIONLEGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());
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
LAPriceOptionGenerator::createOptions(LAObject& leg, LAObject& trade) const  
{
	LADataHolder* dh;

	//input type
	dh = &(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
	LAString schtype = dynamic_cast<const LADataString&>(dh->get()).get();
	schtype.toUpper();
	if (schtype == MANUAL) return;//manual input

    const LAString& freq         = dynamic_cast<const LADataString&>(leg.getData(PRICING_DATA_FREQUENCY, ISNOTNULL).get()).get();
    const LADate* first_odd_date = (dh=&leg.getData(PRICING_DATA_FIRSTODDDATE))->isDefined() && ! dh->isNull() ? &dynamic_cast<const LADataDate&>(dh->get()).get() : NULL;
    const LADate* last_odd_date  = (dh=&leg.getData(PRICING_DATA_LASTODDDATE))->isDefined() && ! dh->isNull() ? &dynamic_cast<const LADataDate&>(dh->get()).get() : NULL;
    const LADate& start_date     = dynamic_cast<const LADataDate&>(leg.getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
    const LADate& end_date       = dynamic_cast<const LADataDate&>(leg.getData(PRICING_DATA_ENDDATE, ISNOTNULL).get()).get();

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
	const LAString& leg_name = dynamic_cast<const LADataString&>(dh->get()).get();

	// object pool
	LAObjectPool& objPool = leg.getDataInstance()->getObjectPool();

	//// isArrear payment
	//dh = &(leg.getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
	//const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
	//bool isarrear = LAPriceCFGenUtility::isArrear(timing);

	//// isNotionalChangeAtStat
	//dh = &(leg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, ISNOTNULL));
	//bool isnotionalchangestart = dynamic_cast<const LADataBool&>(dh->get()).get();
	//
	//// isNotionalChangeAtEnd
	//dh = &(leg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, ISNOTNULL));
	//bool isnotionalchangeend = dynamic_cast<const LADataBool&>(dh->get()).get();



	////////////////////
	// create cashlet //
	////////////////////
	LAString cashlet_str;
	unsigned int k = 0;//for interest calc startdate and enddate position
	for (unsigned int i = 0; i < cfsize; i++)
	{
		if (i != 0) cashlet_str += ":";
		
		// name of this cashlet
		LAString cashlet_name = leg_name + "_" + OPTION + LADataInt(i + 1).convertToString();
		cashlet_str += cashlet_name;

		//copy option object
		dh = &(leg.getData(PRICING_DATA_OPTIONINFORS, ISNOTNULL));
		LADataMultiReference & optioninfos = dynamic_cast<LADataMultiReference &>(dh->get());
		if (optioninfos.getSize() != 1)
			throw LACoreInvalidData("OptionInfo size Error",__FILE__,__LINE__);

		if (!optioninfos.get(0).get().isTypeOf(ENTITY_VENTITY))
			throw LACoreInvalidData("OptionInfo valuemethod Error",__FILE__,__LINE__);

		LAMathObjectValue* opval = dynamic_cast<LAMathObjectValue *>(optioninfos.get(0).get().clone());
		// name
		opval->getName().convertFromString(cashlet_name);
		
		
		objPool.set(cashlet_name, opval);

		// register cashlet to object pool
		// delivery date
		opval->LAObject::add(PRICING_DATA_DELIVERYDATE, new LADataDate(paymentdates[i]));

		//get expiry date
		LADate expirydate;
		LADate expirybasedate;

		//if maturity date exists in leg, it should be regarded as first priority date
		dh = &(leg.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			expirydate = dynamic_cast<const LADataDate &>(dh->get()).get();
			opval->LAObject::add(PRICING_DATA_EXPIRYDATE, new LADataDate(expirydate));
			continue;
		}

		
		dh = &(leg.getData(PRICING_DATA_ISFIXINGPAYMENTDATEBASE, NOCHECK));
		if (dh->isDefined() && !dh->isNull()
			&& dynamic_cast<const LADataBool&>(dh->get()).get() == true)
			expirybasedate = paymentdates[i];
		else
			expirybasedate = paymentdates_unadjust[i];	

		dh = &(leg.getData(PRICING_DATA_OFFSET, ISNOTNULL));
		int offset = dynamic_cast<const LADataInt&>(dh->get()).get();		
		if (offset == 0) 
			expirydate = expirybasedate;
		else
		{
			//calendar
			LAPriceDataCalendar cal;
			dh = &(leg.getData(PRICING_DATA_FIXINGCALENDAR, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
			}
			else
			{
				dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
				cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
			}
			expirydate = cal.getBusinessDay(expirybasedate, -offset);
		}
		
		opval->LAObject::add(PRICING_DATA_EXPIRYDATE, new LADataDate(expirydate));
	}

	LAString underlyings_str;
	dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		LADataMultiReference &refs = dynamic_cast<LADataMultiReference &>(dh->get());
		underlyings_str = dh->convertToString();
		int locate = underlyings_str.findString('"');
		underlyings_str = underlyings_str.subString(1,underlyings_str.size()-2);
		underlyings_str += ":" + cashlet_str;
	}
	else
		underlyings_str = cashlet_str;

	trade.remove(CALIBRATION_DATA_UNDERLYINGS);
	dh = &trade.add(CALIBRATION_DATA_UNDERLYINGS, new LADataMultiReference());
	dh->convertFromString(underlyings_str);

}

/*!
	@brief	setup leg maturity 
	@param[in, out] trade trade object

*/
void
LAPriceOptionGenerator::setUpLegMaturity(LAObject& trade) const
{
	LADataHolder* dh;
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_OPTIONLEGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());

	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_ENDDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) continue;
		
		//set end date
		dh = &(legs.get(i).getData(PRICING_DATA_ENDTERM, ISNOTNULL));
		//term
		const LAString& termstr = dynamic_cast<const LADataString&>(dh->get()).get();
		//start date
		dh = &(legs.get(i).getData(PRICING_DATA_STARTDATE, ISNOTNULL));
		const LADate& startdate = dynamic_cast<const LADataDate&>(dh->get()).get();
		//end date
		const LADate& enddate = LAMathDateCalculations::getDate(startdate, termstr, true);
		dh = &(legs.get(i).getData(PRICING_DATA_ENDDATE, NOCHECK));
		if (!dh->isDefined())
			dh = &(legs.get(i).add(PRICING_DATA_ENDDATE, new LADataDate()));
		dynamic_cast<LADataDate&>(dh->get()).set(enddate);
		
	}
}



