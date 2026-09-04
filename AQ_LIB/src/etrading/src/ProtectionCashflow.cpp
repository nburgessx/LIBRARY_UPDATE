#include "ProtectionCashflow.h"
#include "AQLDateScheduleHelpers.h"

namespace etrading
{

	ProtectionCashflow::ProtectionCashflow() : recoveryRate_( std::numeric_limits<double>::quiet_NaN() ), marginalDefaultProbability_( std::numeric_limits<double>::quiet_NaN() ), Cashflow()
    {}
    
    ProtectionCashflow::ProtectionCashflow(const PayReceiveEnum& payReceive,
		const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, int accrualDays, double accrualYearFraction, const AQLDate& paymentDate, 
        double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType)
		: recoveryRate_( std::numeric_limits<double>::quiet_NaN() ), survivalProbability_( std::numeric_limits<double>::quiet_NaN() ), marginalDefaultProbability_( std::numeric_limits<double>::quiet_NaN() ),
		  Cashflow(payReceive, AQLDate(), accrualStartDate, accrualEndDate, accrualDays, accrualYearFraction, paymentDate, notional, leverage, 1.0/*couponMultiplier*/, zeroCouponSwapPaymentFreq, cashflowType, CashFlowBespokeInfo())
    {}

	/*
	* The clone() method invokes the ProtectionCashflow copy constructor. Note that we deliberately do not provide a copy constructor,
	* and instead use the compiler default. We do this because there are no pointer members that
	* need careful copying. More subtley, we want the string members variables of this class to deep copy (rather than shallow copy
	* with an increased reference count). This allows us to use the cloned ProtectionCashflow instance from a different thread.
	*/
    CashflowPtr ProtectionCashflow::clone()
    {
        CashflowPtr cf = CashflowPtr(new ProtectionCashflow(*this));
        return cf;
    }
	
	double ProtectionCashflow::getRecoveryRate() const
	{
		return recoveryRate_;
	}

	void ProtectionCashflow::setRecoveryRate( double recoveryRate )
	{
		recoveryRate_ = recoveryRate;
	}

	// The probability of survival up to the end of this cashflow period
	double ProtectionCashflow::getSurvivalProbability() const
	{
		return survivalProbability_;
	}

	void ProtectionCashflow::setSurvivalProbability( double survivalProbability )
	{
		survivalProbability_ = survivalProbability;
	}

	// The probability of default within this cashflow period
	double ProtectionCashflow::getMarginalDefaultProbability() const
	{
		return marginalDefaultProbability_;
	}

	void ProtectionCashflow::setMarginalDefaultProbability( double marginalDefaultProbability )
	{
		marginalDefaultProbability_ = marginalDefaultProbability;
	}

	// Returns the protection leg coupon, calculated  from the recovery rate
	double ProtectionCashflow::getRegularCoupon( const CashflowData& cashflowData ) const
	{
		// Notional is already adjusted by leverage factor
		// Coupons are adjusted by coupon multipler here
		const double coupon =  notional_ * couponMultiplier_ * ( 1 - recoveryRate_ ) * marginalDefaultProbability_;
		return coupon;
	}

}

