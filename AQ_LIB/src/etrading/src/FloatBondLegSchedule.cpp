#include "FloatBondLegSchedule.h"

namespace etrading
{

	FloatBondLegSchedule::FloatBondLegSchedule(const std::string& instanceName) : FloatSchedule(instanceName) , quotedMargin_(std::numeric_limits<double>::quiet_NaN())
	{
        scheduleType_ = SWAPSCHEDULE_FLOATBOND;
    }

	FloatBondLegSchedule::FloatBondLegSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : FloatSchedule(instanceName)
	{
		scheduleType_ = SWAPSCHEDULE_FLOATBOND;

		const std::string inputLVB = "FloatBondLegScheduleLVB";

		quotedMargin_ = scheduleLVB.getCompulsoryValueAsDouble(BOND_KEY::QUOTED_MARGIN, inputLVB);

		// Initialise the FloatSchedule from the supplied label-value block, and create schedule dates
		initialise(scheduleLVB);

		//Populate the cashflows based on schedule inputs
		createCashflows();

		// Determine if the Stubs are Irregular Stub
		determineIsIrregularStub();

	}

	DateVector FloatBondLegSchedule::generateAccrualAndPaymentSchedules()
	{
		DateVector accrualDates = Schedule::generateAccrualAndPaymentSchedules();
		
		auto accrualDateSize = accrualDates.size();
		auto cashflowSize = accrualDateSize - 1;

		const AQLDate adjustedMaturityDate = accrualDates[accrualDateSize - 1];
		const AQLDate unAdjustedMaturityDt = validateMaturityDate(getEffectiveDate(), accrualEndDateOrTenor_);

		// For bond, the maturity date is NEVER adjusted, so we need to update the last accrualEndDate, paymentDate, and accrualDate
		if (unAdjustedMaturityDt != adjustedMaturityDate)
		{
			accrualDates[accrualDateSize - 1] = unAdjustedMaturityDt;
			accrualEndDates_[cashflowSize - 1] = unAdjustedMaturityDt;
			paymentDates_[cashflowSize - 1] = unAdjustedMaturityDt;
		}

		return accrualDates;
	}

	FloatBondLegSchedule::FloatBondLegSchedule(const FloatBondLegSchedule& rhs) : FloatSchedule(rhs), quotedMargin_(rhs.quotedMargin_)
	{};

	SchedulePtr FloatBondLegSchedule::clone()
	{
		SchedulePtr sch = SchedulePtr(new FloatBondLegSchedule(*this));
		return sch;
	}

	// For AssetSwap's Float leg, use Bond's quoted margin for the accruedInterest
	double FloatBondLegSchedule::calculateAccruedInterest(const DataProvider& dataProvider, bool nativeCurrencyPV) const
	{
		// For accrued interest, ALWAYS use Floating Bond's coupon rate

		// If override floatBondCoupon is provided, use it as priority
		double floatingBondAnnualizedCouponRate = dataProvider.getValuationSettings().getFloatBondCurrentCouponRate();

		if (boost::math::isnan(floatingBondAnnualizedCouponRate))
		{
			//Derive from libor first fixing rate
			auto firstCashflowData = dataProvider.getCashflowDataExcludingUpfront(0);
			AQ_REQUIRE(!boost::math::isnan(firstCashflowData.floatRateData.resetRate), "Float leg's reset rate has not been populated.");

			//Quote margin is in bps
			floatingBondAnnualizedCouponRate = firstCashflowData.floatRateData.resetRate + quotedMargin_ * 0.0001;
		}

		double accruedInterest = 0.0;

		// If it's negative couponRate, accrued interest is zero
		if (AQ_IS_GREATER_THAN_ZERO(floatingBondAnnualizedCouponRate))
		{
			DataProvider dataProviderToUse = dataProvider;
			dataProviderToUse.setCompoundRateOverride(floatingBondAnnualizedCouponRate);

			accruedInterest = Schedule::calculateAccruedInterest(dataProviderToUse, nativeCurrencyPV);
		}

		return accruedInterest;
	}


}
