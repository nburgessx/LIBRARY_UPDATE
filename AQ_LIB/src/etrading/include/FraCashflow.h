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
