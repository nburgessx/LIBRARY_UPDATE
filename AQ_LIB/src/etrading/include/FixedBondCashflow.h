#pragma once

#include "FixedCashflow.h"

namespace etrading
{

    class FixedBondCashflow : public FixedCashflow
    {
    public:
		FixedBondCashflow();
		FixedBondCashflow(const PayReceiveEnum& payReceive, double fixedRate,const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, int accrualDays, double accrualYearFraction, const AQLDate& paymentDate,
							double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType,
							const double& bondTrueYieldYearFraction, const AQLDate& bondExDividendDate);
        virtual ~FixedBondCashflow() {}

		CashflowPtr clone();

		// Override
		double getCompoundRate(const CashflowData& cashflowData) const;

		const AQLDate& getBondExDividendDate() const { return bondExDividendDate_; };
		double getBondTrueYieldYearFraction() const { return bondTrueYieldYearFraction_; };

	protected:

        // Actual year fraction for TRUE yield calculation, with consideration of accrual business adjustment 
        double bondTrueYieldYearFraction_;

		// For exDividend Bond, the exDividendDate is required so that we can design whether to include the coupon or not.
		AQLDate bondExDividendDate_;

    };

}
