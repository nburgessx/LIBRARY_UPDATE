#include "FloatBondSchedule.h"
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

	FloatBondSchedule::FloatBondSchedule(const std::string& instanceName) : BondSchedule(instanceName)
	{
		scheduleType_ = BONDSCHEDULE_FLOATBOND;
	}

	void FloatBondSchedule::initialize(const LabelValueBlock& scheduleLVB)
	{
		const std::string inputLVB = "scheduleLVB";

		BondSchedule::initialize(scheduleLVB);
	}

	FloatBondSchedule::FloatBondSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : BondSchedule(scheduleLVB, instanceName)
	{
		scheduleType_ = BONDSCHEDULE_FLOATBOND;


		initialize(scheduleLVB);

		populateAccrualStartDates(scheduleLVB);

		//Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates, fixingDates
		BondSchedule::calculateScheduleDates();

		//Populate the cashflows based on schedule inputs
		createCashflows();

	}

    void FloatBondSchedule::createUpfrontCashflow(const LADate& paymentDate, double leverage)
    {
		if (notionalExchangeEnum_ == START_NE || notionalExchangeEnum_ == START_AND_END_NE)
		{
			auto nanDoubleValue = std::numeric_limits<double>::quiet_NaN();

			auto exDividendDate = LADate(); 

			upfrontCashflow_ = CashflowPtr(new FixedBondCashflow(payerReceiver_, nanDoubleValue, LADate(), LADate(), 0, nanDoubleValue, paymentDate, nanDoubleValue, leverage, paymentFreqEnum_, FIRST_NOTIONAL_EXCHANGE_CASHFLOW_TYPE, nanDoubleValue, exDividendDate));
			upfrontCashflow_->setFwdFxRate(nanDoubleValue);
		}
	}

    void FloatBondSchedule::createCashflows()
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

			// *** TODO Should this be a FloatBondCashflow
			const double fixedRate = std::numeric_limits<double>::quiet_NaN();
			CashflowPtr cf = CashflowPtr(new FixedBondCashflow(payerReceiver_, fixedRate, accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFractions_[i], paymentDates_[i], cashflowNotional, leverage_, paymentFreqEnum_, cashflowType, bondTrueYieldYearFractions_[i], exDividendDates_[i]));

			cashflows_.push_back(cf);
		}

		// Update notional exchanges based on notionals of cashflows
		updateNotionalExchange();
    }

	SchedulePtr FloatBondSchedule::clone()
	{
		SchedulePtr sch = SchedulePtr(new FloatBondSchedule(*this));
		return sch;
	}

	FloatBondSchedule::FloatBondSchedule(const FloatBondSchedule& rhs) : BondSchedule(rhs)
	{}


}
