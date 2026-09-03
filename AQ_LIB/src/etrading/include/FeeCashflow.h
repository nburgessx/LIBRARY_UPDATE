#pragma once

#include "Cashflow.h"

namespace etrading
{
    class FeeCashflow : public Cashflow
    {
    public:
        FeeCashflow(const PayReceiveEnum& payReceive, const AQLDate& paymentDate, double amount);
		virtual ~FeeCashflow() {}

        CashflowPtr clone();

        void flipPayerReceiver();
        double getCoupon( const CashflowData& cashflowData ) const;
        double getAmount() const;

	private:
	    // paymentDate_ is defined in base class;
		double amount_;
	};


}
