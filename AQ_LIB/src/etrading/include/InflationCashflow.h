/*
 * @brief			Class the defines the Inflation Cashflow of an Inflation Leg
 * @Created:		21 May 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "Cashflow.h"

namespace etrading
{
    class InflationCashflow : public Cashflow
    {
    public:
        InflationCashflow();
        InflationCashflow( const PayReceiveEnum& payReceive, const LADate& fixingDate,
							const LADate& accrualStartDate, const LADate& accrualEndDate, const int accrualDays, const double accrualYearFraction,
							const LADate& paymentDate, const double notional, double leverage,
							const FrequencyEnum& paymentFrequency, const CashflowTypeEnum& cashflowType );
		virtual ~InflationCashflow() {}

        CashflowPtr clone();

		double getBaseIndex() const;
		void setBaseIndex(double resetPrice);

		double getResetIndex() const;
		void setResetIndex( double resetIndex );

        virtual double getRegularCoupon( const CashflowData& cashflowData ) const;

	private:
		double baseIndex_;	// The base or reference inflation index level
		double resetIndex_;	// The inflation index level fixing used to calculate the coupon of this cashflow

	};


}
