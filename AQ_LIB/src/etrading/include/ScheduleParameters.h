#pragma once

#include "CoreEnumerations.h"
#include "LACoreTemplateType.h"

namespace etrading
{
	class ScheduleParameters
	{
	public:
		ScheduleParameters();
		ScheduleParameters(const std::string& accrualStartDate,
					const std::string& adjustedAccrualStartDate,
					const std::string& accrualEndDateOrTenor,
					const BusinessDayAdjustmentEnum& accrualbusinessDayAdj,
					const std::string& accrualCalendar,
					const FrequencyEnum& accrualFrequency,
					const DayCountEnum& accrualDaycount,
					const BusinessDayAdjustmentEnum& paymentbusinessDayAdj,
					const std::string& paymentCalendar,
					const FrequencyEnum& paymentFrequency,
					const std::string& payLag,
					const std::string& rollDayInput,
					const StubTypeEnum& stubType,
					const std::string& firstStub,
					const std::string& lastStub,
					const std::string& fixingAdvanceOrArrears,
					const BusinessDayAdjustmentEnum& fixingbusinessDayAdj,
					const std::string& fixingCalendar,
					const std::string& fixLag,
					const std::string& firstFixLag,
					const std::string& fxFixingLag,
					const BusinessDayAdjustmentEnum& fxFixingbusinessDayAdj,
					const std::string& fxFixingCalendar,

					const CCY& currency,
					const CCY& valuationCurrency,
					const double& notional,
					const double& leverage,
					const double& couponMultiplier,
					const double& spread,
					const double& fixedRateOrStrike, 
					const std::string& discountCurve,
					const std::string& forecastCurve,
					const CompoundingMethodEnum& couponCompoundMethod,
					const std::string& firstStubCurveIndex,
					const std::string& lastStubCurveIndex,
					const double& firstFixing,
					const double& lastFixing,
					const BooleanEnum& fwdInter,
			
					const std::string& spotLag,
					const BusinessDayAdjustmentEnum& spotBusinessDayAdj,
					const std::string& spotCalendar);

		std::shared_ptr<ScheduleParameters> clone();

		//Getter
		const std::string accrualStartDate() const	{ return accrualStartDate_; };
		const std::string adjustedAccrualStartDate() const { return adjustedAccrualStartDate_; };
		const std::string accrualEndDateOrTenor() const { return accrualEndDateOrTenor_; };
		const BusinessDayAdjustmentEnum accrualbusinessDayAdj() const { return accrualbusinessDayAdj_; };
		const std::string accrualCalendar() const { return accrualCalendar_; };
		const FrequencyEnum accrualFrequency() const { return accrualFrequency_; };
		const DayCountEnum accrualDaycount()const { return accrualDaycount_; };
		const BusinessDayAdjustmentEnum paymentbusinessDayAdj() const { return paymentbusinessDayAdj_; };
		const std::string paymentCalendar() const { return paymentCalendar_; };
		const FrequencyEnum paymentFrequency() const { return paymentFrequency_; };
		const std::string payLag() const { return payLag_; };
		const std::string rollDayInput() const { return rollDayInput_; };
		const StubTypeEnum stubType() const { return stubType_; };
		const std::string firstStub() const { return firstStub_; };
		const std::string lastStub() const { return lastStub_; };
		const std::string fixingAdvanceOrArrears() const { return fixingAdvanceOrArrears_; };
		const BusinessDayAdjustmentEnum fixingbusinessDayAdj() const { return fixingbusinessDayAdj_; };
		const std::string fixingCalendar() const { return fixingCalendar_; };
		const std::string fixLag() const { return fixLag_; };
		const std::string firstFixLag() const { return firstFixLag_; };
		const std::string fxFixingLag() const { return fxFixingLag_; };
		const BusinessDayAdjustmentEnum fxFixingbusinessDayAdj() const { return fxFixingbusinessDayAdj_; };
		const std::string fxFixingCalendar() const { return fxFixingCalendar_; };

		const CCY currency() const { return currency_; };
		const CCY valuationCurrency() const { return valuationCurrency_; };
		const double notional() const { return notional_; };
		const double leverage() const { return leverage_; };
		const double couponMultiplier() const { return couponMultiplier_; };
		const double spread() const { return spread_; };
		const double fixedRateOrStrike() const { return fixedRateOrStrike_; };
		const std::string discountCurve() const { return discountCurve_; };
		const std::string forecastCurve() const { return forecastCurve_; };
		const CompoundingMethodEnum couponCompoundMethod() const { return couponCompoundMethod_; };
		const std::string firstStubCurveIndex() const { return firstStubCurveIndex_; };
		const std::string lastStubCurveIndex() const { return lastStubCurveIndex_; };
		const double firstFixing() const { return firstFixing_; };
		const double lastFixing() const { return lastFixing_; };
		const BooleanEnum fwdInter() const { return fwdInter_; };

		const std::string spotLag() const { return spotLag_; };
		const BusinessDayAdjustmentEnum spotBusinessDayAdj() const { return spotBusinessDayAdj_; };
		const std::string spotCalendar() const { return spotCalendar_; };

	private:

		// It is unadjusted effective date
		std::string accrualStartDate_;

		//Adjusted effective date is used to generate schedule output. 
		//However for swap, unadjusted effective date is used for the first accrualStartDate, is it correct? 
		std::string adjustedAccrualStartDate_;

		std::string accrualEndDateOrTenor_;
		BusinessDayAdjustmentEnum accrualbusinessDayAdj_;
		std::string accrualCalendar_;
		FrequencyEnum accrualFrequency_;
		DayCountEnum accrualDaycount_;
		BusinessDayAdjustmentEnum paymentbusinessDayAdj_;
		std::string paymentCalendar_;
		FrequencyEnum paymentFrequency_;
		std::string payLag_;
		std::string rollDayInput_;
		StubTypeEnum stubType_;
		std::string firstStub_;
		std::string lastStub_;
		std::string fixingAdvanceOrArrears_;
		BusinessDayAdjustmentEnum fixingbusinessDayAdj_;
		std::string fixingCalendar_;
		std::string fixLag_;
		std::string firstFixLag_;
		std::string fxFixingLag_;
		BusinessDayAdjustmentEnum fxFixingbusinessDayAdj_;
		std::string fxFixingCalendar_;

		CCY currency_;
		CCY valuationCurrency_;
		double notional_;
		double leverage_;
		double couponMultiplier_;
		double spread_;
		double fixedRateOrStrike_;
		std::string discountCurve_;
		std::string forecastCurve_;
		CompoundingMethodEnum couponCompoundMethod_;
		std::string firstStubCurveIndex_;
		std::string lastStubCurveIndex_;
		double firstFixing_;
		double lastFixing_;
		BooleanEnum fwdInter_;

		//The lag from curve's asOfDate to valuationDate
		std::string spotLag_;
		BusinessDayAdjustmentEnum spotBusinessDayAdj_;
		std::string spotCalendar_;

	};


}
