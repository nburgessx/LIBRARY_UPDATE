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
    class FeeCashflow : public Cashflow
    {
    public:
        FeeCashflow(const PayReceiveEnum& payReceive, const LADate& paymentDate, double amount);
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
