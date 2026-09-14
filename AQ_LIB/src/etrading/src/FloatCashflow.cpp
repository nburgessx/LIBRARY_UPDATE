#include "FloatCashflow.h"
#include "AQLDateScheduleHelpers.h"

namespace etrading
{

	FloatCashflow::FloatCashflow() : Cashflow(), spread_(0), survivalProbability_( 1.0 ), marginalDefaultProbability_( 0.0 )
    {}
    
    FloatCashflow::FloatCashflow(const PayReceiveEnum& payReceive, double spread, 
				const AQLDate& fixingDate, const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, int accrualDays, double accrualYearFraction, const AQLDate& paymentDate, 
                double notional, double leverage, double couponMultiplier, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType, const CashFlowBespokeInfo& bespokeInfo)
				: spread_(spread), survivalProbability_( 1.0 ), marginalDefaultProbability_( 0.0 ),
                Cashflow(payReceive, fixingDate, accrualStartDate, accrualEndDate, accrualDays, accrualYearFraction, paymentDate, notional, leverage, couponMultiplier, zeroCouponSwapPaymentFreq, cashflowType, bespokeInfo)
    {}

    CashflowPtr FloatCashflow::clone()
    {
        CashflowPtr cf = CashflowPtr(new FloatCashflow(*this));
        return cf;
    }

	double FloatCashflow::getCompoundRate( const CashflowData& cashflowData ) const
	{
		// If there is an overrided compoundRate, use it, otherwise use the floatRate in the trade
		if (!boost::math::isnan(cashflowData.compoundRateOverride))
		{
			return cashflowData.compoundRateOverride;
		}

		const double oneBasisPoint = 0.0001;

		const double floatRate = cashflowData.floatRateData.resetRate;
        AQ_THROW_IF( boost::math::isnan(floatRate), "floatRate is not set for the cashflow" );

		// If there is an overrided floatSpread, use it, otherwise use the floatSpread in the trade
		double spreadToUse = !boost::math::isnan(cashflowData.floatSpreadOverride) ? cashflowData.floatSpreadOverride : spread_;

        double compoundRate = floatRate + spreadToUse * oneBasisPoint;
		
		compoundRate *= couponMultiplier_;

		return compoundRate;
	}


	double FloatCashflow::getSpread() const
	{
		return spread_;
	}

	void FloatCashflow::setSpread(double spread) 
	{
		spread_ = spread;
	}

  	double FloatCashflow::getSpreadAmount(const CashflowData& cashflowData) const
	{
         double oneBasisPoint = 0.0001;

		 // If there is an overrided floatSpread, use it, otherwise use the floatSpread in the trade
		 double spreadToUse = !boost::math::isnan(cashflowData.floatSpreadOverride) ? cashflowData.floatSpreadOverride : spread_;

		 double spreadAmount = notional_ * spreadToUse *oneBasisPoint * accrualYearFraction_;
         return spreadAmount;
    }

	double FloatCashflow::getCoupon( const CashflowData& cashflowData ) const 
	{
		double coupon = 0.0;
        if (paymentFreqEnum_ == AT_MATURITY_FREQUENCY)
        {
            coupon = getZeroCouponSwapCoupon( cashflowData );
        }
        else
        {
            coupon = getRegularCoupon( cashflowData );
        }

		coupon *= survivalProbability_;
		return coupon;
	}

	double FloatCashflow::getAnnuity( const CashflowData& cashflowData ) const
	{
		double riskyAnnuity = 0.0;

		if ( ! isUpfrontCashflow() )
		{
			const double riskFreeAnnuity = Cashflow::getAnnuity( cashflowData );
			riskyAnnuity = ( riskFreeAnnuity * survivalProbability_ );
		}

		return riskyAnnuity;

	}

	// Calculates the pv of the notional exchange, including the survivial probability
	double FloatCashflow::getNotionalExchangePv( const CashflowData& cashflowData ) const
	{
        if (boost::math::isnan( cashflowData.discountFactor ))
        {
            return std::numeric_limits<double>::quiet_NaN();
        }

		const double riskFreePV = Cashflow::getNotionalExchangePv( cashflowData );
		const double riskyPV = riskFreePV * survivalProbability_;

        return riskyPV;
	}

    double FloatCashflow::calculateCompounding(double cp, const CashflowData& cashflowData ) const 
	{
		if (cashflowData.compoundType == NONE_COMPOUNDING_METHOD)
		{
			return cp;
		}
        double coupon = cp;
        double sumOfPreviousCoupons = 0;
        double sumOfPreviousSpreadAmount = 0;
        double sumOfPreviousFloatRate = 0;

		size_t prevCashflowSize = previousCashFlowsInfo_.size();

        for (size_t i = 0; i < prevCashflowSize; ++i) 
        {
            auto prevCoupon = previousCashFlowsInfo_[i].coupon;
            auto prevSpreadAmount = previousCashFlowsInfo_[i].spreadAmount;
            auto prevFloatRate = previousCashFlowsInfo_[i].floatRate;
            sumOfPreviousCoupons += prevCoupon; 
            sumOfPreviousSpreadAmount += prevSpreadAmount;
            sumOfPreviousFloatRate += prevFloatRate;
        }

		const double floatRate = cashflowData.floatRateData.resetRate;

        switch( cashflowData.compoundType )
		{
			case GEOMETRIC_COMPOUNDING_METHOD: //Normal Compounding (BB's INCLUDE_SPREAD)
			{
				// ISDA Normal Compounding method (ref: section3.1): coupon_j = N * (floatRate_j + spread_j) * tao_j + sum_of_coupon_i *  (floatRate_j + spread_j) * tao_j, where i < j
				double compoundAmount = leverage_ * sumOfPreviousCoupons * getCompoundRate(cashflowData) * accrualYearFraction_;

				coupon += compoundAmount;

				break;
			}
			case FLAT_COMPOUNDING_METHOD: //(BB's FLAT)
			{   
				// ISDA FLAT compounding method (ref: section3.2): coupon_j = N * (floatRate_j + spread_j) * tao_j + sum_of_coupon_i *  floatRate_j * tao_j, where i < j
				double compoundAmount = leverage_ * sumOfPreviousCoupons * floatRate * couponMultiplier_ * accrualYearFraction_;

				coupon += compoundAmount;

				break;
			}
			case SIMPLE_COMPOUNDING_METHOD: //(BB's ExcludeSprd)
            {
                // ISDA Simple Compounding method (ref: section3.3): coupon_j = N * (floatRate_j) * tao_j + sum_of_coupon_i *  (floatRate_j) * tao_j + spreadAmount_j, where i < j
                // spreadAmount_j = N*spread*tao_1 + ....+ N*spread*tao_i +N*spread*tao_j
                    
                //Need to exclude the spread from the coupon
                double currentSpreadAmount = getSpreadAmount(cashflowData);
                coupon -= currentSpreadAmount;

                //Add the spread to the last cashflow: 1) the last cashflow of the ZeroCoupon leg, or 2) the last cashflow under a given payment date for normal leg
                double spreadAmount = (sumOfPreviousSpreadAmount != 0 && isLastCashflow()) ? (currentSpreadAmount + sumOfPreviousSpreadAmount) : 0.0;

				double compoundAmount = leverage_ * sumOfPreviousCoupons * floatRate * couponMultiplier_ * accrualYearFraction_ + spreadAmount;

				coupon += compoundAmount;

				break;
			}
			case ARITHMETIC_COMPOUNDING_METHOD:
			{
				//From Bloomberg, the rates are averaged, and then apply spread to the averagedRate
				//Unweighted Avg: Unweighted Average. The arithmetic mean of all the reset rates for a pay accrual period, then using that average rate as the floating rate (leverage and spread are applied after the average). For example, for a monthly reset of Quarterly Pay Floater, the average is calculated as (r1+r2+r3)/3, where r1/r2/r3 are three reset rates for the pay period.

				double averageFloatRateData = (floatRate + sumOfPreviousFloatRate) / (1 + prevCashflowSize);

				//Notional has been leveraged
				coupon = notional_ * (averageFloatRateData + spread_ * 0.0001 ) * couponMultiplier_ * accrualYearFraction_;

				break;
			}
			case NONE_COMPOUNDING_METHOD:
				break;
			default:
                AQ_THROW( "CompoundingMethod must be 'GEOMETRIC', 'ARITHMETIC', 'FLAT', 'SIMPLE', or 'NONE'." );
				break;
		}

        return coupon;
	}

   	// The probability of survival up to the end of this cashflow period
	double FloatCashflow::getSurvivalProbability() const
	{
		return survivalProbability_;
	}

	void FloatCashflow::setSurvivalProbability( double survivalProbability )
	{
		survivalProbability_ = survivalProbability;
	}

	// The probability of default within this cashflow period
	double FloatCashflow::getMarginalDefaultProbability() const
	{
		return marginalDefaultProbability_;
	}

	void FloatCashflow::setMarginalDefaultProbability( double marginalDefaultProbability )
	{
		marginalDefaultProbability_ = marginalDefaultProbability;
	}

}

