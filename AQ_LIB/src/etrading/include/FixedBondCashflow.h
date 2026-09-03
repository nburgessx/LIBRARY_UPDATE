/*
 * @brief			Class the defines the leg cashflow
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once

#include "FixedCashflow.h"

namespace etrading
{

    class FixedBondCashflow : public FixedCashflow
    {
    public:
		FixedBondCashflow();
		FixedBondCashflow(const PayReceiveEnum& payReceive, double fixedRate,const LADate& accrualStartDate, const LADate& accrualEndDate, int accrualDays, double accrualYearFraction, const LADate& paymentDate,
							double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType,
							const double& bondTrueYieldYearFraction, const LADate& bondExDividendDate);
        virtual ~FixedBondCashflow() {}

		CashflowPtr clone();

		// Override
		double getCompoundRate(const CashflowData& cashflowData) const;

		const LADate& getBondExDividendDate() const { return bondExDividendDate_; };
		double getBondTrueYieldYearFraction() const { return bondTrueYieldYearFraction_; };

	protected:

        // Actual year fraction for TRUE yield calculation, with consideration of accrual business adjustment 
        double bondTrueYieldYearFraction_;

		// For exDividend Bond, the exDividendDate is required so that we can design whether to include the coupon or not.
		LADate bondExDividendDate_;

    };

}
