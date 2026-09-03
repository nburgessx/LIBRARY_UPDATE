#pragma once

#include "Cashflow.h"

namespace etrading
{
    class FixedCashflow : public Cashflow
    {
    public:
        FixedCashflow();
        FixedCashflow(const PayReceiveEnum& payReceive, double fixedRate, 
					const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, int accrualDays, double accrualYearFraction, const AQLDate& paymentDate, 
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
