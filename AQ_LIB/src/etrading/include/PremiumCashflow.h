#pragma once

#include "Cashflow.h"

namespace etrading
{
    class PremiumCashflow : public Cashflow
    {
    public:
        PremiumCashflow();
        PremiumCashflow(const PayReceiveEnum& payReceive, double cdsSpread,
					const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, int accrualDays, double accrualYearFraction, const AQLDate& paymentDate,
                    double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType);
		virtual ~PremiumCashflow() {}

        CashflowPtr clone();

		// Whether to use the Accrued Interest in cashflow calculations
		void setIncludeAccruedInterest( const bool includeAccruedInterest );
		bool getIncludeAccruedInterest() const;

		// Calculates the constribution to the risky annuity from this cashflow
		double getRiskyAnnuity( const CashflowData& cashflowData ) const;

		// Calculates the pv of the notional exchange, including the survivial probability
		virtual double getNotionalExchangePv( const CashflowData& cashflowData ) const;

		// The probability of survival up to the end of this cashflow period
		double getSurvivalProbability() const;
		void setSurvivalProbability( double survivalProbability );

		// The probability of default within this cashflow period
		double getMarginalDefaultProbability() const;
		void setMarginalDefaultProbability( double marginalDefaultProbability );

		// The TotalReturnSwap performance component of this cashflow, if any.
		// This has a negative contribution arising from the probability of default,
		// when the underlying bond is only worth the recovery amount.
		// There is also a positive pull-to-par contribution as the bond approaches maturity.
		double getPerformance() const;
		void setPerformance( double performance );

		// Returns the total coupon ( premium coupon and accrual on default coupon )
        virtual double getRegularCoupon( const CashflowData& cashflowData ) const;

		// Returns the premium cashflow, calculated from the CDS Spread
		double getPremiumCoupon() const;

		// Returns the premium cashflow, calculated from the CDS Spread. This cashflow is NOT scaled by survival-probability.
		double getRiskFreePremiumCoupon() const;

		// Returns the premium cashflow PV, calculated from the CDS Spread. This PV is NOT scaled by survival-probability.
		double getRiskFreePremiumCouponPv( const CashflowData& cashflowData ) const;

		// Calculates the amount of accrued coupon, in the event that a default occurs mid accrual-period.
		double getAccrualOnDefaultCoupon() const;

		double getCdsSpread() const;

	private:
		//The following members are Schedule related

		double cdsSpread_;
		double survivalProbability_;         // The probability of survival up to the end of this cashflow
		double marginalDefaultProbability_;  // The probability of default during this cashflow accrual period
		bool includeAccruedInterest_;        // Whether to use the Accrued Interest in cashflow calculations
		double performance_;                 // The TotalReturnSwap performance contribution to the cashflow value
	};


}
