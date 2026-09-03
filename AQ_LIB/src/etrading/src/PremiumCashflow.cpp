/*
 * @brief			Class the defines the Premium Leg Cashflow of a Credit Default Swap
 * @Created:		15 Feb 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "PremiumCashflow.h"
#include "LADateScheduleHelpers.h"

namespace etrading
{

	PremiumCashflow::PremiumCashflow()
		:  cdsSpread_( std::numeric_limits<double>::quiet_NaN() ), survivalProbability_( std::numeric_limits<double>::quiet_NaN() ), marginalDefaultProbability_( std::numeric_limits<double>::quiet_NaN() ), includeAccruedInterest_( false ), performance_( std::numeric_limits<double>::quiet_NaN() ), Cashflow()
    {}
    
    PremiumCashflow::PremiumCashflow(const PayReceiveEnum& payReceive, double cdsSpread,
		const LADate& accrualStartDate, const LADate& accrualEndDate, int accrualDays, double accrualYearFraction, const LADate& paymentDate,
        double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType)
		: cdsSpread_( cdsSpread), survivalProbability_( std::numeric_limits<double>::quiet_NaN() ), marginalDefaultProbability_( std::numeric_limits<double>::quiet_NaN() ), includeAccruedInterest_( false ), performance_( std::numeric_limits<double>::quiet_NaN() ),
		  Cashflow(payReceive, LADate(), accrualStartDate, accrualEndDate, accrualDays, accrualYearFraction, paymentDate, notional, leverage, 1.0/*couponMultiplier*/, zeroCouponSwapPaymentFreq, cashflowType, CashFlowBespokeInfo())
    {}

	/*
	* The clone() method invokes the PremiumCashflow copy constructor. Note that we deliberately do not provide a copy constructor,
	* and instead use the compiler default. We do this because there are no pointer members that
	* need careful copying. More subtley, we want the string members variables of this class to deep copy (rather than shallow copy
	* with an increased reference count). This allows us to use the cloned PremiumCashflow instance from a different thread.
	*/
    CashflowPtr PremiumCashflow::clone()
    {
        CashflowPtr cf = CashflowPtr(new PremiumCashflow(*this));
        return cf;
    }
	
	// Whether to use the Accrued Interest in cashflow calculations
	void PremiumCashflow::setIncludeAccruedInterest( const bool includeAccruedInterest )
	{
		includeAccruedInterest_ = includeAccruedInterest;
	}

	bool PremiumCashflow::getIncludeAccruedInterest() const
	{
		return includeAccruedInterest_;
	}

	// Calculates the pv of the notional exchange, including the survivial probability
	double PremiumCashflow::getNotionalExchangePv( const CashflowData& cashflowData ) const
	{
		
        if (boost::math::isnan( cashflowData.discountFactor ))
        {
            return std::numeric_limits<double>::quiet_NaN();
        }

		double cpv = notionalExchange_ * cashflowData.discountFactor;

		if ( ! isUpfrontCashflow() )
		{
			cpv *= survivalProbability_;
		}

        if ( cashflowData.currency != cashflowData.valuationCurrency)
        {
            cpv *= cashflowData.fxAsOfDate;
        }

        return cpv;
	}

	// Calculates the constribution to the risky annuity from this cashflow
	double PremiumCashflow::getRiskyAnnuity( const CashflowData& cashflowData ) const
	{
		double riskyAnnuity = 0.0;

		if ( ! isUpfrontCashflow() )
		{
			const double riskFreeAnnuity = Cashflow::getAnnuity( cashflowData );
			riskyAnnuity = ( riskFreeAnnuity * survivalProbability_ );

			if ( includeAccruedInterest_ )
			{
				// Assume default occurs half-way through the period
				const double accruedInterestContribution = 0.5 * riskFreeAnnuity * marginalDefaultProbability_;
				riskyAnnuity += accruedInterestContribution;
			}
		}

		return riskyAnnuity;
	}

	// The probability of survival up to the end of this cashflow period
	double PremiumCashflow::getSurvivalProbability() const
	{
		return survivalProbability_;
	}

	void PremiumCashflow::setSurvivalProbability( double survivalProbability )
	{
		survivalProbability_ = survivalProbability;
	}

	// The probability of default within this cashflow period
	double PremiumCashflow::getMarginalDefaultProbability() const
	{
		return marginalDefaultProbability_;
	}

	void PremiumCashflow::setMarginalDefaultProbability( double marginalDefaultProbability )
	{
		marginalDefaultProbability_ = marginalDefaultProbability;
	}

	/* @brief Returns the TotalReturnSwap performance contribution to this cashflow
	*/
	double PremiumCashflow::getPerformance() const
	{
		return performance_;
	}

	/* @brief Sets the TotalReturnSwap performance contribution of this cashflow
	*/
	void PremiumCashflow::setPerformance( double performance )
	{
		performance_ = performance;
	}

	// Returns the total coupon ( premium coupon and accrual on default coupon )
	double PremiumCashflow::getRegularCoupon( const CashflowData& cashflowData ) const
	{
		if ( isUpfrontCashflow() )
        {
            return getUpfrontCashflowCoupon();
        }

		double coupon =  getPremiumCoupon() + getAccrualOnDefaultCoupon();

		coupon += notionalExchange_ * survivalProbability_; 

		// Include the total return performance component, if any
		coupon += getPerformance();

		return coupon;
	}

	// Returns the premium cashflow, calculated from the CDS Spread
	double PremiumCashflow::getPremiumCoupon() const
	{
		const double premiumCoupon = getRiskFreePremiumCoupon() * survivalProbability_;
		return premiumCoupon;
	}

	// Returns the premium cashflow, calculated from the CDS Spread. This cashflow is NOT scaled by survival-probability.
	double PremiumCashflow::getRiskFreePremiumCoupon() const
	{
		// Notional is adjusted by leverage factor already
		// Coupons are adjusted by coupon multipler here
		const double riskFreeCoupon = notional_ * couponMultiplier_ * cdsSpread_ * accrualYearFraction_;
		return riskFreeCoupon;
	}

	// Returns the premium cashflow PV, calculated from the CDS Spread. This PV is NOT scaled by survival-probability.
	double PremiumCashflow::getRiskFreePremiumCouponPv( const CashflowData& cashflowData ) const
	{
		if (std::isnan( cashflowData.discountFactor))
        {
            return std::numeric_limits<double>::quiet_NaN();
        }

		const double couponPV = getRiskFreePremiumCoupon() * cashflowData.discountFactor;
		return couponPV;
	}

	// Calculates the amount of accrued coupon, in the event that a default occurs mid accrual-period.
	double PremiumCashflow::getAccrualOnDefaultCoupon() const
	{
		double accrualOnDefaultCoupon = 0.;
		if ( includeAccruedInterest_ )
		{
			// Notional is adjusted by leverage factor already
			// Coupons are adjusted by coupon multipler here
			accrualOnDefaultCoupon = notional_ * couponMultiplier_ * cdsSpread_* ( 0.5 * accrualYearFraction_ )  * marginalDefaultProbability_;
		}
		
		return accrualOnDefaultCoupon;
	}

	double PremiumCashflow::getCdsSpread() const
	{
		return cdsSpread_;
	}

}


