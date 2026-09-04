#include "FixedBondCashflow.h"
#include "AQLDateScheduleHelpers.h"

namespace etrading
{

	FixedBondCashflow::FixedBondCashflow() : FixedCashflow(), bondTrueYieldYearFraction_(std::numeric_limits<double>::quiet_NaN()), bondExDividendDate_(AQLDate())
	{}

	FixedBondCashflow::FixedBondCashflow(const PayReceiveEnum& payReceive, double fixedRate, const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, int accrualDays, double accrualYearFraction, const AQLDate& paymentDate,
										double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType,
										const double& bondTrueYieldYearFraction, const AQLDate& bondExDividendDate)
		: FixedCashflow(payReceive, fixedRate, accrualStartDate, accrualEndDate, accrualDays, accrualYearFraction, paymentDate, notional, leverage, 1.0 /*couponMultiplier*/, zeroCouponSwapPaymentFreq, cashflowType, CashFlowBespokeInfo()),
		bondTrueYieldYearFraction_(bondTrueYieldYearFraction), bondExDividendDate_(bondExDividendDate)
	{}

	CashflowPtr FixedBondCashflow::clone()
	{
		CashflowPtr cf = CashflowPtr(new FixedBondCashflow(*this));
		return cf;
	}

	double FixedBondCashflow::getCompoundRate(const CashflowData& cashflowData) const
	{
		if (boost::math::isnan(getFixedRate()))
		{
			throw AQLCoreInvalidData("#Error: fixedRate is not set for the cashflow", __FILE__, __LINE__);
		}
		//When includeCouponRate is false, the couponRate need to be excluded
		return cashflowData.includeCouponRate ? getFixedRate() : 0.0;
	}


}


