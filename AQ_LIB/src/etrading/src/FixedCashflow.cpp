#include "FixedCashflow.h"
#include "AQLDateSchedule.h"

namespace etrading
{

	FixedCashflow::FixedCashflow() : Cashflow(), fixedRate_(0)
    {}
    
    FixedCashflow::FixedCashflow(const PayReceiveEnum& payReceive, double fixedRate,  
		const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, int accrualDays, double accrualYearFraction, const AQLDate& paymentDate, 
        double notional, double leverage, double couponMultiplier, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType, const CashFlowBespokeInfo& bespokeInfo)
		: fixedRate_(fixedRate) , Cashflow(payReceive, AQLDate(), accrualStartDate, accrualEndDate, accrualDays, accrualYearFraction, paymentDate, notional, leverage, couponMultiplier, zeroCouponSwapPaymentFreq, cashflowType, bespokeInfo)
    {}

	/*
	* The clone() method invokes the FixedCashflow copy constructor. Note that we deliberately do not provide a copy constructor,
	* and instead use the compiler default. We do this because there are no pointer members that
	* need careful copying. More subtley, we want the string members variables of this class to deep copy (rather than shallow copy
	* with an increased reference count). This allows us to use the cloned FixedCashflow instance from a different thread.
	*/
    CashflowPtr FixedCashflow::clone()
    {
        CashflowPtr cf = CashflowPtr(new FixedCashflow(*this));
        return cf;
    }

	double FixedCashflow::getCompoundRate( const CashflowData& cashflowData ) const
	{
		double compoundRate = 0.0;

		// If there is an overrided compoundRate, use it, otherwise use the fixeRate in the trade
		if (!boost::math::isnan(cashflowData.compoundRateOverride))
		{
			compoundRate = cashflowData.compoundRateOverride;
		}
		else
		{
			AQ_THROW_IF( boost::math::isnan(fixedRate_), "fixedRate is not set for the cashflow" );
			compoundRate =  fixedRate_;
		}

		// apply couponMultiplier the compoundRate
		compoundRate *= couponMultiplier_;

		return compoundRate;

	}
	
	double FixedCashflow::getFixedRate() const
	{
		return fixedRate_;
	}

    double FixedCashflow::calculateCompounding(double cp, const CashflowData& cashflowData ) const 
	{
        double coupon = cp;
        if (previousCashFlowsInfo_.size() > 0)
        {
            double sumOfPreviousCoupons = 0;
            for (size_t i = 0; i < previousCashFlowsInfo_.size(); ++i) 
            {
                sumOfPreviousCoupons += previousCashFlowsInfo_[i].coupon; 
            } 
            
            // ISDA Normal Compounding method: coupon_j = N * (fixedRate_j) * DF_j + sum_of_coupon_i *  (fixedRate_j) * DF_j, where i < j
            coupon = coupon + leverage_ * sumOfPreviousCoupons * getCompoundRate(cashflowData) * accrualYearFraction_;
        }

        return coupon;
	}


}

