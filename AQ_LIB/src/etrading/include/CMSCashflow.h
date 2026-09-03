/*
 * @brief			Class the defines the Constant Maturity Swap Cashflow 
 *
 * @Created:		27 March 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once

#include "FloatCashflow.h"

namespace etrading
{
    class CMSCashflow : public FloatCashflow
    {
    public:
        CMSCashflow();
        CMSCashflow( const PayReceiveEnum& payReceive, double spread, 
					 const LADate& fixingDate, const LADate& accrualStartDate, const LADate& accrualEndDate, int accrualDays, double accrualYearFraction, const LADate& paymentDate, 
					 double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType);
		virtual ~CMSCashflow() {}

        CashflowPtr clone();

		void setParRate1( double parRate );
		void setParRate2( double parRate );
		void setMultiplier1( double multiplier );
		void setMultiplier2( double multiplier );
		
		double getParRate1() const;
		double getParRate2() const;
		double getMultiplier1() const;
		double getMultiplier2() const;

	private:
        double parRate1_;
		double parRate2_;

		double multiplier1_;
		double multiplier2_;

    };


}
