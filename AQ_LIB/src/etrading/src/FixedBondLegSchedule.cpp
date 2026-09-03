/*
 * @brief			Class the defines the fixed leg schedule 
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "FixedBondLegSchedule.h"
#include "FixedBondCashflow.h"
#include "BondUtilities.h"

namespace etrading
{

	FixedBondLegSchedule::FixedBondLegSchedule(const std::string& instanceName) : BondSchedule(instanceName), fixedRate_(std::numeric_limits<double>::quiet_NaN()), bondYieldParameters_()
	{
        scheduleType_ = SWAPSCHEDULE_FIXEDBOND;
    }

	void FixedBondLegSchedule::initialize(const LabelValueBlock& scheduleLVB)
	{
		const std::string inputLVB = "scheduleLVB";

		fixedRate_ = scheduleLVB.getCompulsoryValueAsDoubleFromKeys(IRS_KEY::FIXED_RATE, BOND_KEY::COUPON, inputLVB);

		BondSchedule::initialize(scheduleLVB);
	}

	void FixedBondLegSchedule::populateAccrualStartDates(const LabelValueBlock& scheduleLVB)
	{
		const std::string inputLVB = "scheduleLVB";
		accrualStartDate_ = scheduleLVB.getCompulsoryValueAsLAString(IRS_KEY::EFFECTIVE_DATE, inputLVB);
	}

	void FixedBondLegSchedule::calculateScheduleDates()
	{
		//Note***: Use schedule's date calculation instead of Bond's schedule calculation
		Schedule::calculateScheduleDates();

		populateExDividendDates();
	}

	FixedBondLegSchedule::FixedBondLegSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : BondSchedule(instanceName)
	{
		scheduleType_ = SWAPSCHEDULE_FIXEDBOND;

		initialize(scheduleLVB);

		populateAccrualStartDates(scheduleLVB);

		calculateScheduleDates();

		//Populate the cashflows based on schedule inputs
		createCashflows();

		populateBondYieldParameters(bondYieldParameters_, getPaymentFrequency(), getYieldFrequency(), getBondCalculationType(), getAccrualDaycount(), getAccrualCalendar().getCString(), getExDividendTenor());

	}

	void FixedBondLegSchedule::createUpfrontCashflow(const LADate& paymentDate, double leverage)
	{
		if (notionalExchangeEnum_ == START_NE || notionalExchangeEnum_ == START_AND_END_NE)
		{
			auto nanDoubleValue = std::numeric_limits<double>::quiet_NaN();

			// Use paymentDate as exDividendDate for upfront cashflow, so that the cashflow can distinguish whether to show exDividendDate column in display function
			auto exDividendDate = exDividendTenor_.empty() ? LADate() : paymentDate;

			upfrontCashflow_ = CashflowPtr(new FixedBondCashflow(payerReceiver_, nanDoubleValue, LADate(), LADate(), 0, nanDoubleValue, paymentDate, nanDoubleValue, leverage, paymentFreqEnum_, FIRST_NOTIONAL_EXCHANGE_CASHFLOW_TYPE, nanDoubleValue, exDividendDate));

			upfrontCashflow_->setFwdFxRate(nanDoubleValue);
		}
	}


	void FixedBondLegSchedule::createCashflows() 
    {
        //Use the first accrualStartDate as the paymentDate
        auto paymentDate = accrualStartDates_.at(0);
        createUpfrontCashflow(paymentDate, leverage_);

		auto cashflowSize = accrualStartDates_.size();
		const std::vector<double> cashflowNotionals = getCashflowNotionals(cashflowSize);

		const double bondCouponRate = fixedRate_;
		const double bondNotional = notional_;

		const double nanDouble = std::numeric_limits<double>::quiet_NaN();

        for (size_t i=0; i < cashflowSize; ++i)
		{
			//Formula: backout fixedRate for cashflow: cashflowFixedRateToUse = fixedCoupon /(cashflowNotional * accrualYearFraction)
		
			// Formula: fixedCoupon = (bondCouponRate * bondYearFraction * bondNotional)
			const bool firstCashflow = (i == 0);
			const bool isLast = (i == cashflowSize - 1);
			
			auto cashflowType = isLast ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

			const double bondYearFraction = calculateBondCashflowYearFraction(firstCashflow, isLast);

			const double fixedCoupon = bondNotional * bondCouponRate * bondYearFraction;

			const double cashflowNotional = cashflowNotionals[i];
			const double accrualYearFraction = accrualYearFractions_[i];

			const double fixedRateToUse = fixedCoupon / (cashflowNotional * accrualYearFraction);

			CashflowPtr cf = CashflowPtr(new FixedBondCashflow(payerReceiver_, fixedRateToUse, accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFraction, paymentDates_[i], cashflowNotional, leverage_, paymentFreqEnum_, cashflowType, nanDouble, exDividendDates_[i])); //bondTrueYieldYearFraction is populated as NaN

            cashflows_.push_back(cf);
		}

        // Update notional exchanges based on notionals of cashflows
        updateNotionalExchange();

    }

    SchedulePtr FixedBondLegSchedule::clone()
    {
        SchedulePtr sch = SchedulePtr(new FixedBondLegSchedule(*this));
        return sch;
    }

	FixedBondLegSchedule::FixedBondLegSchedule(const FixedBondLegSchedule& rhs) : BondSchedule(rhs), fixedRate_(rhs.fixedRate_), bondYieldParameters_(rhs.bondYieldParameters_)
	{};

	// For AssetSwap's Fixed leg, BOND convention should be used to calculate the accrued interest
	double FixedBondLegSchedule::calculateAccruedInterest(const DataProvider& dataProvider, bool nativeCurrencyPV) const
	{
		const double accruedInterest = BondSchedule::calculateBondAccruedInterest(dataProvider.getValuationSettings().getValuationDate(), bondYieldParameters_);

		return accruedInterest;
	}

}
