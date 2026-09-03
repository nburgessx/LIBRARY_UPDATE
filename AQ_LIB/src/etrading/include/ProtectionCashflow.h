#pragma once

#include "Cashflow.h"

namespace etrading
{
    class ProtectionCashflow : public Cashflow
    {
    public:
        ProtectionCashflow();
        ProtectionCashflow(const PayReceiveEnum& payReceive,
					const LADate& accrualStartDate, const LADate& accrualEndDate, int accrualDays, double accrualYearFraction, const LADate& paymentDate, 
                    double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType);
		virtual ~ProtectionCashflow() {}

        CashflowPtr clone();

		double getRecoveryRate() const;
		void setRecoveryRate( double recoveryRate );

		// The probability of survival up to the end of this cashflow period
		double getSurvivalProbability() const;
		void setSurvivalProbability( double survivalProbability );

		// The probability of default within this cashflow period
		double getMarginalDefaultProbability() const;
		void setMarginalDefaultProbability( double marginalDefaultProbability );

		// Returns the protection leg casflow, calculated from the recovery rate
        virtual double getRegularCoupon( const CashflowData& cashflowData ) const;

	private:
		//The following members are Schedule related
		double recoveryRate_;
		double survivalProbability_;
		double marginalDefaultProbability_;

	};


}
