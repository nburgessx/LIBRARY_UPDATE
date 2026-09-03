/*
* @brief			Class the defines the CapFloor Option
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/
#include "CapFloorOption.h"
#include "SwapValidation.h"
#include "CurveUtilities.h"
#include "CurveValidation.h"
#include "ExceptionMacros.h"
#include "CoreEnumerations.h"

namespace etrading
{

	CapFloorOption::CapFloorOption(const std::string& objectName, const LabelValueBlock& dealLVB ) : Option(objectName)
	{
		// Validate Object Type
        const std::string inputLVB = "CapFloorTradeLVB";
        CachedObjectEnum objectType = toCachedObjectEnum( dealLVB.getCompulsoryValueAsString( OBJECT_KEY::OBJECT_TYPE, inputLVB ) );
        MLIB_REQUIRE( objectType == OPTION, "Invalid Object Type - 'OPTION' type required" )

		inputParameters_ = dealLVB;

		const auto capFloorType = toCapFloorEnum(dealLVB.getCompulsoryValueAsString(OPTION_KEYS::OPTION_STYLE, inputLVB));
		callPut_ = (capFloorType == CAP_OPTION) ? CALL_OPTION : PUT_OPTION;

		const auto inputCurrency = dealLVB.getCompulsoryValueAsString(IRS_KEY::CURRENCY, inputLVB);
		const auto currency = toCCYEnum(inputCurrency);
		const auto valuationCurrency = toCCYEnum(dealLVB.getOptionalValueAsString(IRS_KEY::VALUATION_CURRENCY, inputCurrency));

		const auto notional = dealLVB.getCompulsoryValueAsDouble(IRS_KEY::NOTIONAL);
		const auto leverage = dealLVB.getOptionalValueAsDouble(IRS_KEY::LEVERAGE, 1.);
		const auto strike = dealLVB.getCompulsoryValueAsDoubleFromKeys(IRS_KEY::STRIKE_RATE, OPTION_KEYS::STRIKE, inputLVB);

		// -- 1) Schedule fields, used to populate scheduleParams_ and enrichedSchedule_ -- 

		// -- 1.1) Schedule general fields
		const std::string floatBusinessDayAdjustment = dealLVB.getOptionalValueAsStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT)(IRS_KEY::BUSINESSDAYADJUSTMENT));
		const std::string floatCalendar = dealLVB.getOptionalValueAsStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FLOAT_CALENDAR)(IRS_KEY::CALENDAR));
		const std::string floatLegFreq = dealLVB.getOptionalValueAsStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FLOAT_FREQUENCY)(IRS_KEY::FREQUENCY));
		const std::string floatDayCount = dealLVB.getOptionalValueAsStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FLOAT_DAYCOUNT)(IRS_KEY::DAYCOUNT));

		const auto accrualbusinessDayAdj = toBusinessDayAdjustmentEnum(dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT, IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT, floatBusinessDayAdjustment));
		const std::string accrualCalendar = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_ACCRUALCALENDAR, IRS_KEY::ACCRUALCALENDAR, floatCalendar);
		const auto accrualFrequency = toFrequencyEnum(dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_ACCRUALFREQUENCY, IRS_KEY::ACCRUALFREQUENCY, floatLegFreq));
		const auto accrualDaycount = toDayCountEnum(dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_ACCRUALDAYCOUNT, IRS_KEY::ACCRUALDAYCOUNT, floatDayCount));

		const auto defaultBusinessDayAdj = !floatBusinessDayAdjustment.empty() ? floatBusinessDayAdjustment : toString(accrualbusinessDayAdj).c_str();
		const auto defaultCalendar = !floatCalendar.empty() ? floatCalendar : accrualCalendar;
		const auto defaultFrequency = !floatLegFreq.empty() ? floatLegFreq : toString(accrualFrequency);

		const auto paymentbusinessDayAdj = toBusinessDayAdjustmentEnum(dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_PAYMENTBUSINESSDAYADJUSTMENT, IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj));
		const auto paymentCalendar = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_PAYMENTCALENDAR, IRS_KEY::PAYMENTCALENDAR, defaultCalendar);
		const auto paymentFrequency = toFrequencyEnum(dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_PAYMENTFREQUENCY, IRS_KEY::PAYMENTFREQUENCY, defaultFrequency));

		const auto firstStub = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_FIRSTSTUBDATE, IRS_KEY::FIRSTSTUBDATE);
		const auto lastStub = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_LASTSTUBDATE, IRS_KEY::LASTSTUBDATE);
		const auto rollDayInput = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_ROLLDAY, IRS_KEY::ROLLDAY);
		const auto payLag = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_PAYMENTLAG, IRS_KEY::PAYMENTLAG, "0D");
		const auto stubType = toStubTypeEnum(dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_STUBTYPE, IRS_KEY::STUBTYPE));

		const auto fixingbusinessDayAdj = toBusinessDayAdjustmentEnum(dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_FIXINGBUSINESSDAYADJUSTMENT, IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj));
		const auto fixingCalendar = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_FIXINGCALENDAR, IRS_KEY::FIXINGCALENDAR, defaultCalendar);
		const auto fixLag = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_FIXINGLAG, IRS_KEY::FIXINGLAG, "0D");
		const auto firstFixLag = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_FIRSTFIXINGLAG, IRS_KEY::FIRSTFIXINGLAG, fixLag);
		const auto fixingAdvanceOrArrears = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_FIXINGADVANCEORARREAR, IRS_KEY::FIXINGADVANCEORARREAR, "advance");

		const auto fxFixingLag = dealLVB.getOptionalValueAsString(IRS_KEY::FXFIXINGLAG, "0D");
		const auto fxFixingbusinessDayAdj = toBusinessDayAdjustmentEnum(dealLVB.getOptionalValueAsString(IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj));
		const auto fxFixingCalendar = dealLVB.getOptionalValueAsString(IRS_KEY::FXFIXINGCALENDAR, defaultCalendar);

		// Unadjusted effectiveDate
		const auto accrualStartDate = dealLVB.getCompulsoryValueAsString(IRS_KEY::EFFECTIVE_DATE, inputLVB);

		// BusinessDayAdjusted effectiveDate, which is used to generate schedule output. 
		const auto effectiveDate = stringToDate(accrualStartDate.c_str(), "#Error: Invalid 'EffectiveDate'");
		const auto adjustedEffectiveDate = getDateFromTenor(boost::assign::list_of(effectiveDate), "0D", toString(accrualbusinessDayAdj).c_str(), accrualCalendar.c_str(), "")[0];
		const std::string adjustedAccrualStartDate = boost::lexical_cast<std::string>(fromLADateToDouble(adjustedEffectiveDate));

		// Unadjusted maturityDate
		const auto accrualEndDateOrTenor = dealLVB.getCompulsoryValueAsString(IRS_KEY::MATURITY_DATE, inputLVB);

		// ValuationDate:
		const std::string spotLag = dealLVB.getOptionalValueAsString(OPTION_KEYS::SPOT_LAG, "0D"); //Lag from asOfDate
		const auto spotBusinessDayAdj = toBusinessDayAdjustmentEnum(dealLVB.getOptionalValueAsString(OPTION_KEYS::SPOT_BUSINESSDAYADJUSTMENT, defaultBusinessDayAdj));
		const auto spotCalendar = dealLVB.getOptionalValueAsString(OPTION_KEYS::SPOT_CALENDAR, defaultCalendar);

		// -- 1.2) Schedule more specific fields
		const auto discountCurve = dealLVB.getCompulsoryValueAsString(MARKET_KEY::DISCOUNT_CURVE, inputLVB);
		const auto forecastCurve = dealLVB.getCompulsoryValueAsString(MARKET_KEY::FORECAST_CURVE, inputLVB);

		const auto spread = dealLVB.getOptionalValueAsDouble(IRS_KEY::FLOAT_SPREAD, 0.);

		// For a OIS float leg, the compounding method is required for OIS float rate calculation
		// For non-OIS float leg, this field is required when the accrual frequencey is less than payment frequency (e.g. 3M vs 6M, or 3M vs AT_MATURITY)
		const auto couponCompoundMethod = toCompoundingMethodEnum(dealLVB.getOptionalValueAsString(IRS_KEY::COMPOUND_METHOD, ""));

		const auto firstStubCurveIndex = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_FIRSTSTUBCURVEINDEX, IRS_KEY::FIRSTSTUBCURVEINDEX, "NATURAL");
		const auto lastStubCurveIndex = dealLVB.getOptionalValueAsStringFromKeys(IRS_KEY::FLOAT_LASTSTUBCURVEINDEX, IRS_KEY::LASTSTUBCURVEINDEX, "NATURAL");
		const auto firstFixing = dealLVB.getOptionalValueAsDoubleFromKeys(IRS_KEY::FLOAT_FIRSTFIXING, IRS_KEY::FIRSTFIXING, std::numeric_limits<double>::quiet_NaN());
		const auto lastFixing = dealLVB.getOptionalValueAsDoubleFromKeys(IRS_KEY::FLOAT_LASTFIXING, IRS_KEY::LASTFIXING, std::numeric_limits<double>::quiet_NaN());

		//Get the input from user, default to NONE
		const auto fwdInter = toBooleanEnum(dealLVB.getOptionalValueAsString(IRS_KEY::IS_FWD_INTER, ""));

		const ScheduleParameters schParams (accrualStartDate,
											adjustedAccrualStartDate,
											accrualEndDateOrTenor,
											accrualbusinessDayAdj,
											accrualCalendar,
											accrualFrequency,
											accrualDaycount,
											paymentbusinessDayAdj,
											paymentCalendar,
											paymentFrequency,
											payLag,
											rollDayInput,
											stubType,
											firstStub,
											lastStub,
											fixingAdvanceOrArrears,
											fixingbusinessDayAdj,
											fixingCalendar,
											fixLag,
											firstFixLag,
											fxFixingLag,
											fxFixingbusinessDayAdj,
											fxFixingCalendar,
											currency,
											valuationCurrency,
											notional,
											leverage,
											1.0 /*couponMultiplier*/,
											spread,
											strike,
											discountCurve,
											forecastCurve,
											couponCompoundMethod,
											firstStubCurveIndex,
											lastStubCurveIndex,
											firstFixing,
											lastFixing,
											fwdInter,
											spotLag,
											spotBusinessDayAdj,
											spotCalendar);

		const EnrichedSchedule schOutput = populateScheduleDates(schParams, false); // CheckStub as false, do not calculate stub rates for CapFloor

		scheduleParams_ = std::make_shared<ScheduleParameters>(schParams);
		enrichedSchedule_ = std::make_shared<EnrichedSchedule>(schOutput);

	}

	CapFloorOption::CapFloorOption(const CapFloorOption& rhs) : Option(rhs)
	{}

	std::shared_ptr<Option> CapFloorOption::clone()
	{
		CapFloorOption temp(*this);
		return std::make_shared<CapFloorOption>(temp);
	}



}
