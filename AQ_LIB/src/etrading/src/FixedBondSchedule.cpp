#include "FixedBondSchedule.h"
#include "ParameterValidation.h"
#include "SwapValidation.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "LACurvePricingObject.h"
#include "LACurveForwardRateHelpers.h"
#include "BondAccrualPeriods.h"
#include "BondUtilities.h"
#include "SwapUtilities.h"
#include "FixedBondCashflow.h"
#include <string> 

namespace etrading
{

	FixedBondSchedule::FixedBondSchedule(const std::string& instanceName) : BondSchedule(instanceName), fixedRate_(std::numeric_limits<double>::quiet_NaN())
	{
		scheduleType_ = BONDSCHEDULE_FIXEDBOND;
	}

	void FixedBondSchedule::initialize(const LabelValueBlock& scheduleLVB)
	{
		const std::string inputLVB = "scheduleLVB";

		fixedRate_ = scheduleLVB.getCompulsoryValueAsDoubleFromKeys(IRS_KEY::FIXED_RATE, BOND_KEY::COUPON, inputLVB);

		BondSchedule::initialize(scheduleLVB);
	}

	FixedBondSchedule::FixedBondSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : BondSchedule(scheduleLVB, instanceName)
	{
		scheduleType_ = BONDSCHEDULE_FIXEDBOND;


		initialize(scheduleLVB);

		populateAccrualStartDates(scheduleLVB);

		//Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates, fixingDates
		BondSchedule::calculateScheduleDates();

		//Populate the cashflows based on schedule inputs
		createCashflows();

	}

    void FixedBondSchedule::createUpfrontCashflow(const LADate& paymentDate, double leverage)
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

    void FixedBondSchedule::createCashflows()
    {
		//Use the first accrualStartDate as the paymentDate
		auto paymentDate = accrualStartDates_.at(0);
		createUpfrontCashflow(paymentDate, leverage_);

		auto cashflowSize = accrualStartDates_.size();

		std::vector<double> cashflowNotionals = getCashflowNotionals(cashflowSize);

		for (size_t i = 0; i < cashflowSize; ++i)
		{
			auto cashflowType = (i == cashflowSize - 1) ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

			double cashflowNotional = cashflowNotionals[i];

			CashflowPtr cf = CashflowPtr(new FixedBondCashflow(payerReceiver_, fixedRate_, accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFractions_[i], paymentDates_[i], cashflowNotional, leverage_, paymentFreqEnum_, cashflowType, bondTrueYieldYearFractions_[i], exDividendDates_[i]));

			cashflows_.push_back(cf);
		}

		// Update notional exchanges based on notionals of cashflows
		updateNotionalExchange();
    }


	SchedulePtr FixedBondSchedule::clone()
	{
		SchedulePtr sch = SchedulePtr(new FixedBondSchedule(*this));
		return sch;
	}

	FixedBondSchedule::FixedBondSchedule(const FixedBondSchedule& rhs) : BondSchedule(rhs), fixedRate_(rhs.fixedRate_)
	{}

	double FixedBondSchedule::getFixedRate() const
	{
		return fixedRate_;
	}

}
