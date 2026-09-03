#pragma once

#include "FloatCashflow.h"

namespace etrading
{
    class CMSCashflow : public FloatCashflow
    {
    public:
        CMSCashflow();
        CMSCashflow( const PayReceiveEnum& payReceive, double spread, 
					 const AQLDate& fixingDate, const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, int accrualDays, double accrualYearFraction, const AQLDate& paymentDate, 
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
