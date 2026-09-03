#pragma once

#include "Cashflow.h"

namespace etrading
{
    class InflationCashflow : public Cashflow
    {
    public:
        InflationCashflow();
        InflationCashflow( const PayReceiveEnum& payReceive, const AQLDate& fixingDate,
							const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, const int accrualDays, const double accrualYearFraction,
							const AQLDate& paymentDate, const double notional, double leverage,
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
