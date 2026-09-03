#pragma once

#include "Cashflow.h"

namespace etrading
{
    class FraCashflow : public Cashflow
    {
    public:
        FraCashflow();
        FraCashflow(const PayReceiveEnum& payReceive, double strikeRate, const AQLDate& fixingDate, const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, 
					int accrualDays, double accrualYearFraction, const AQLDate& paymentDate, double notional, double leverage);
		virtual ~FraCashflow() {}

        CashflowPtr clone();

		double getCompoundRate( const CashflowData& cashflowData ) const;

		//Get/set strikeRate
		double getFixedRate() const;
 
	private:
        double strikeRate_;
	
    };


}
