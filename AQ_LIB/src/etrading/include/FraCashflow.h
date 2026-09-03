#pragma once

#include "Cashflow.h"

namespace etrading
{
    class FraCashflow : public Cashflow
    {
    public:
        FraCashflow();
        FraCashflow(const PayReceiveEnum& payReceive, double strikeRate, const LADate& fixingDate, const LADate& accrualStartDate, const LADate& accrualEndDate, 
					int accrualDays, double accrualYearFraction, const LADate& paymentDate, double notional, double leverage);
		virtual ~FraCashflow() {}

        CashflowPtr clone();

		double getCompoundRate( const CashflowData& cashflowData ) const;

		//Get/set strikeRate
		double getFixedRate() const;
 
	private:
        double strikeRate_;
	
    };


}
