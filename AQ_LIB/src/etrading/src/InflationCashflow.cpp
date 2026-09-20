#include "InflationCashflow.h"
#include "AQLDateSchedule.h"

namespace etrading
{

	InflationCashflow::InflationCashflow() : baseIndex_( std::numeric_limits<double>::quiet_NaN() ), resetIndex_( std::numeric_limits<double>::quiet_NaN() ), Cashflow()
    {}
    
    InflationCashflow::InflationCashflow( const PayReceiveEnum& payReceive, const AQLDate& fixingDate,
											const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, const int accrualDays, const double accrualYearFraction,
											const AQLDate& paymentDate, const double notional, double leverage,
											const FrequencyEnum& paymentFrequency, const CashflowTypeEnum& cashflowType )

		: baseIndex_(  std::numeric_limits<double>::quiet_NaN() ),
		  resetIndex_( std::numeric_limits<double>::quiet_NaN() ),
		  Cashflow( payReceive, fixingDate, accrualStartDate, accrualEndDate, accrualDays, accrualYearFraction,
					paymentDate, notional, leverage, 1.0/*couponMultiplier*/, paymentFrequency, cashflowType, CashFlowBespokeInfo() )
    {}

	/*
	* The clone() method invokes the InflationCashflow copy constructor. Note that we deliberately do not provide a copy constructor,
	* and instead use the compiler default. We do this because there are no pointer members that
	* need careful copying. More subtley, we want the string members variables of this class to deep copy (rather than shallow copy
	* with an increased reference count). This allows us to use the cloned InflationCashflow instance from a different thread.
	*/
    CashflowPtr InflationCashflow::clone()
    {
        CashflowPtr cf = CashflowPtr( new InflationCashflow(*this) );
        return cf;
    }

	double InflationCashflow::getBaseIndex() const
	{
		return baseIndex_;
	}

	void InflationCashflow::setBaseIndex( double baseIndex )
	{
		baseIndex_ = baseIndex;
	}

	double InflationCashflow::getResetIndex() const
	{
		return resetIndex_;
	}

	void InflationCashflow::setResetIndex( double resetIndex )
	{
		resetIndex_ = resetIndex;
	}


	// Returns the inflation leg cashflow, calculated from the inflation index
	double InflationCashflow::getRegularCoupon( const CashflowData& cashflowData ) const
	{
		// Notional is already adjusted by leverage factor
		// Coupons are adjusted by coupon multipler here
		const double coupon =  notional_ * couponMultiplier_ * ( resetIndex_ - baseIndex_ ) / baseIndex_;
		return coupon;
	}

}

