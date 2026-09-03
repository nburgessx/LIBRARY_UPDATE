/*
 * @brief			Class the defines the float leg cashflow
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once

#include "Cashflow.h"

namespace etrading
{
    class FloatCashflow : public Cashflow
    {
    public:
        FloatCashflow();
        FloatCashflow(const PayReceiveEnum& payReceive, double spread, 
				const LADate& fixingDate, const LADate& accrualStartDate, const LADate& accrualEndDate, int accrualDays, double accrualYearFraction, const LADate& paymentDate, 
                double notional, double leverage, double couponMultiplier, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType, const CashFlowBespokeInfo& bespokeInfo);
		virtual ~FloatCashflow() {}

        CashflowPtr clone();

		double getCompoundRate( const CashflowData& cashflowData ) const;
		double getSpread() const;
        void setSpread(double spread) ;
        double calculateCompounding(double coupon, const CashflowData& cashflowData ) const;
        double getSpreadAmount(const CashflowData& cashflowData) const;
		virtual double getAnnuity( const CashflowData& cashflowData ) const;
		virtual double getNotionalExchangePv( const CashflowData& cashflowData ) const;

		//Coupon shown in the cashflow display 
        virtual double getCoupon( const CashflowData& cashflowData ) const;

		// The probability of survival up to the end of this cashflow period
		double getSurvivalProbability() const;
		void setSurvivalProbability( double survivalProbability );

		// The probability of default within this cashflow period
		double getMarginalDefaultProbability() const;
		void setMarginalDefaultProbability( double marginalDefaultProbability );

	private:

		//The following members are Schedule related
		double spread_;
		double survivalProbability_;         // The probability of survival up to the end of this cashflow
		double marginalDefaultProbability_;  // The probability of default during this cashflow accrual period
    };


}
