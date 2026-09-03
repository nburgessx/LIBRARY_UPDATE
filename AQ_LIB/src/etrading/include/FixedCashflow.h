/*
 * @brief			Class the defines the fixed leg cashflow
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "Cashflow.h"

namespace etrading
{
    class FixedCashflow : public Cashflow
    {
    public:
        FixedCashflow();
        FixedCashflow(const PayReceiveEnum& payReceive, double fixedRate, 
					const LADate& accrualStartDate, const LADate& accrualEndDate, int accrualDays, double accrualYearFraction, const LADate& paymentDate, 
                    double notional, double leverage, double couponMultiplier, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType, const CashFlowBespokeInfo& bespokeInfo);
		virtual ~FixedCashflow() {}

        virtual CashflowPtr clone();

		virtual double getCompoundRate( const CashflowData& cashflowData ) const;

		double getFixedRate() const;
        double calculateCompounding(double coupon, const CashflowData& cashflowData ) const;

	private:
		//The following members are Schedule related
		double fixedRate_;
	};


}
