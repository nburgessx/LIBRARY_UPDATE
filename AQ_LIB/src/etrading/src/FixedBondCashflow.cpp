/*
 * @brief			Class the defines the leg cashflow
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "FixedBondCashflow.h"
#include "LADateScheduleHelpers.h"

namespace etrading
{

	FixedBondCashflow::FixedBondCashflow() : FixedCashflow(), bondTrueYieldYearFraction_(std::numeric_limits<double>::quiet_NaN()), bondExDividendDate_(LADate())
	{}

	FixedBondCashflow::FixedBondCashflow(const PayReceiveEnum& payReceive, double fixedRate, const LADate& accrualStartDate, const LADate& accrualEndDate, int accrualDays, double accrualYearFraction, const LADate& paymentDate,
										double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType,
										const double& bondTrueYieldYearFraction, const LADate& bondExDividendDate)
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
			throw LACoreInvalidData("#Error: fixedRate is not set for the cashflow", __FILE__, __LINE__);
		}
		//When includeCouponRate is false, the couponRate need to be excluded
		return cashflowData.includeCouponRate ? getFixedRate() : 0.0;
	}


}


