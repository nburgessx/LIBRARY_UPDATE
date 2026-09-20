#include "FraCashflow.h"
#include "AQLDateSchedule.h"
#include "ExceptionMacros.h"

namespace etrading
{

	FraCashflow::FraCashflow() : Cashflow(), strikeRate_(std::numeric_limits<double>::quiet_NaN())
    {}
    
    FraCashflow::FraCashflow(const PayReceiveEnum& payReceive, double strikeRate, const AQLDate& fixingDate, const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, 
					int accrualDays, double accrualYearFraction, const AQLDate& paymentDate, double notional, double leverage)
					: strikeRate_(strikeRate), 
					Cashflow(payReceive, fixingDate, accrualStartDate, accrualEndDate, accrualDays, accrualYearFraction, paymentDate, notional, leverage, 1.0/*couponMultiplier*/, NONE_FREQUENCY, NORMAL_CASHFLOW_TYPE, CashFlowBespokeInfo())
    {
		//notional should be positive
		notional_ = std::abs(notional);
	}

    CashflowPtr FraCashflow::clone()
    {
        CashflowPtr cf = CashflowPtr(new FraCashflow(*this));
        return cf;
    }

	double FraCashflow::getCompoundRate( const CashflowData& cashflowData ) const
	{
		const double floatRate = cashflowData.floatRateData.resetRate;
		AQ_REQUIRE( !boost::math::isnan(floatRate), "FRA's forwardRate cannot be empty" );
		AQ_REQUIRE( !boost::math::isnan(strikeRate_), "FRA's strikeRate cannot be empty" );

		double compoundRate = 0.0;

		//Formula:	1) Pay fixedRate: (liborRate - strikeRate)/(1 + accrualYearFraction * liborRate)
		//			2) Receive fixedRate: (strikeRate - liborRate)/(1 + accrualYearFraction * liborRate)

		// It's SELL (RECEIVE fixed)
		if (getIndicatorFromPayRec(payReceive_) >= 0)
		{
			compoundRate = (strikeRate_ - floatRate);
		}
		// It's BUY (PAY fixed)
		else
		{
			compoundRate = (floatRate - strikeRate_) ;
		}

		compoundRate = compoundRate / (1.0 + accrualYearFraction_ * floatRate);

		return compoundRate;
	}


	double FraCashflow::getFixedRate() const
	{
		return strikeRate_;
	}

}

