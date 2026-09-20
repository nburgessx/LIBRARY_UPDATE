#include "Cashflow.h"
#include "AQLDateSchedule.h"

namespace etrading
{

    Cashflow::Cashflow() :
        payReceive_(NONE_PAYRECEIVE_ENUM),  
        fixingDate_(AQLDate()), 
        accrualStartDate_(AQLDate()), 
        accrualEndDate_(AQLDate()), 
        accrualDays_(0), 
        accrualYearFraction_(0), 
        paymentDate_(AQLDate()), 
        notional_(0), 
        leverage_(1), 
		couponMultiplier_(1),
        notionalExchange_(0), 
        fwdFxRate_(1),
        fxFixingDate_(AQLDate()),
        fixingEndDate_(AQLDate()),
        paymentFreqEnum_(NONE_FREQUENCY),
		cashflowType_(NORMAL_CASHFLOW_TYPE),
        previousCashFlowsInfo_(std::vector<CashFlowInfo>()),
		bespokeInfo_(CashFlowBespokeInfo())
    {}

    Cashflow::Cashflow(const PayReceiveEnum& payReceive, const AQLDate& fixingDate, const AQLDate& accrualStartDate, const AQLDate& accrualEndDate, int accrualDays, 
					double accrualYearFraction, const AQLDate& paymentDate, double notional, double leverage, double couponMultiplier, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType, const CashFlowBespokeInfo& bespokeInfo)
					: payReceive_(payReceive), fixingDate_(fixingDate), accrualStartDate_(accrualStartDate), accrualEndDate_(accrualEndDate), accrualDays_(accrualDays), 
					accrualYearFraction_(accrualYearFraction), paymentDate_(paymentDate), leverage_(leverage), couponMultiplier_(couponMultiplier), fwdFxRate_(1), notionalExchange_(0),
                    fxFixingDate_(AQLDate()), fixingEndDate_(AQLDate()), paymentFreqEnum_(zeroCouponSwapPaymentFreq), cashflowType_(cashflowType), previousCashFlowsInfo_(std::vector<CashFlowInfo>()), bespokeInfo_(bespokeInfo)
	{
        notional_ = notional;
        //set the PayRec sign to the Notional
        if (!boost::math::isnan(notional) && notional != 0)
        {
            notional_ = std::abs(notional) * getIndicatorFromPayRec(payReceive_);
        }

		//Notional is leveraged
		notional_ *= leverage;
    }

	/*
	* The clone() method invokes the Cashflow copy constructor. Note that we deliberately do not provide a copy constructor,
	* and instead use the compiler default. We do this because there are no pointer members that
	* need careful copying. More subtley, we want the string members variables of this class to deep copy (rather than shallow copy
	* with an increased reference count). This allows us to use the cloned Cashflow instance from a different thread.
	*/
    CashflowPtr Cashflow::clone()
    {
        CashflowPtr cf = CashflowPtr(new Cashflow(*this));
        return cf;
    }

    void Cashflow::flipPayerReceiver()
    {
        payReceive_ = flipPayReceive(getPayReceive());

        if (!boost::math::isnan(notional_) && notional_ != 0)
        {
            notional_ *= -1;
        }

        if (notionalExchange_ != 0)
        {
            notionalExchange_ *= -1;
        }
    }

    PayReceiveEnum Cashflow::getPayReceive() const
    {
        return payReceive_;
    }

    double Cashflow::getPayReceiveIndicator() const
    {
        double result = 0.0;
        PayReceiveEnum payReceive = getPayReceive();
        switch( payReceive )
        {
            case PAY_PAYRECEIVE_ENUM:
            {
                result = -1.0;
                break;
            }
            case RECEIVE_PAYRECEIVE_ENUM:
            {
                 result = 1.0;
                 break;
            }
            default:
            {
                result = 0.0;
                break;
            }
        }
        return result;
    }

    double Cashflow::getCompoundRate( const CashflowData& cashflowData ) const
	{
    	AQ_THROW( "getCompoundRate not supported" );
	}

    double Cashflow::calculateCompounding(double cp, const CashflowData& cashflowData ) const 
	{
        return cp;
	}

    double Cashflow::getRegularCoupon( const CashflowData& cashflowData ) const
    {
        if (isUpfrontCashflow())
        {
            return getUpfrontCashflowCoupon();
        }

        double coupon = notional_ * getCompoundRate( cashflowData ) * accrualYearFraction_;
        
        coupon = calculateCompounding(coupon, cashflowData );
		
        coupon += notionalExchange_; 

        return coupon;
    }

    double Cashflow::getZeroCouponSwapCoupon( const CashflowData& cashflowData ) const 
    {
        if (isUpfrontCashflow())
        {
            return getUpfrontCashflowCoupon();
        }

        double coupon = 0.0;

        if (!isLastCashflow())
        {
			coupon += notionalExchange_;
        }
        //last cashflow
        else
        {
            coupon = getRegularCoupon( cashflowData ); 

            double sumOfPreviousCoupons = 0;
            for (size_t i = 0; i < previousCashFlowsInfo_.size(); ++i) 
            {
                sumOfPreviousCoupons += previousCashFlowsInfo_[i].coupon; 
            } 
            coupon += sumOfPreviousCoupons;
        }
           
        return coupon;
     }


    double Cashflow::getCoupon( const CashflowData& cashflowData ) const 
	{
        if (paymentFreqEnum_ == AT_MATURITY_FREQUENCY)
        {
            return getZeroCouponSwapCoupon( cashflowData );
        }
        else
        {
            return getRegularCoupon( cashflowData );
        }
	}

    double Cashflow::getUpfrontCashflowCoupon() const 
    {
        double coupon = notionalExchange_;
        return coupon;
    }

    double Cashflow::getCouponPv( const CashflowData& cashflowData, bool nativeCurrencyPV) const
	{
        // If discount factor is not set, then no pv can be calculated. 
        // ** Note that upfrontCashflow will have pv if it is not past (i.e. the discountFactor is set)
        if (std::isnan( cashflowData.discountFactor))
        {
            return std::numeric_limits<double>::quiet_NaN();
        }

        double cpv = getCoupon( cashflowData ) * cashflowData.discountFactor;

        if (!nativeCurrencyPV)
        {
		    bool isCurrencySameAsValuationCurrency = ( cashflowData.currency == cashflowData.valuationCurrency );
            if (!isCurrencySameAsValuationCurrency)
            {
                cpv *= cashflowData.fxAsOfDate;
            }
        }

        return cpv;
	}

    double Cashflow::getAnnuity( const CashflowData& cashflowData ) const
	{
        double annuity = 0.0;
        
        // Ignore annuity terms if they are fully in the past
        if (cashflowData.discountFactor > 0)
        {       
            //annuity has no sign
            annuity = cashflowData.discountFactor * accrualYearFraction_ * std::abs(notional_);
        }

        if ( cashflowData.currency != cashflowData.valuationCurrency )
        {
            annuity *= cashflowData.fxAsOfDate;
        }

        return annuity;

    }

    double Cashflow::getNotionalExchangePv( const CashflowData& cashflowData ) const
	{
        if (boost::math::isnan( cashflowData.discountFactor ))
        {
            return std::numeric_limits<double>::quiet_NaN();
        }

        double cpv = notionalExchange_ * cashflowData.discountFactor;

        if ( cashflowData.currency != cashflowData.valuationCurrency )
        {
            cpv *= cashflowData.fxAsOfDate;
        }

        return cpv;
	}

	double Cashflow::getSpreadAmount(const CashflowData& cashflowData) const
	{
		return 0;
    }

    double Cashflow::getNotional() const
	{
		return notional_;
	}

	double Cashflow::getLeverage() const
	{
		return leverage_;
	}

	double Cashflow::getCouponMultiplier() const
	{
		return couponMultiplier_;
	}

	double Cashflow::getAccrualDays() const
	{
		return accrualDays_;
	}

    double Cashflow::getAccrualYearFraction() const
	{
		return accrualYearFraction_;
	}

	const AQLDate& Cashflow::getAccrualStartDate() const
	{
		return accrualStartDate_;
	}

	const AQLDate& Cashflow::getAccrualEndDate() const
	{
		return accrualEndDate_;
	}

	const AQLDate& Cashflow::getFixingDate() const
	{
		return fixingDate_;
	}

    void Cashflow::setFixingEndDate(const AQLDate& fixingEndDate) 
	{
		fixingEndDate_ = fixingEndDate;
	}

    const AQLDate& Cashflow::getFixingEndDate() const
	{
		return fixingEndDate_;
	}

	FrequencyEnum Cashflow::getPaymentFreqEnum() const
	{
		return paymentFreqEnum_;
	}

	bool Cashflow::isLastCashflow() const
	{
		return (cashflowType_ == NORMAL_LAST_CASHFLOW_TYPE);
	}

	//This is tempory function until the compounding calculation move to leg level
	void Cashflow::setCashflowTypeToLast()
	{
		cashflowType_ = NORMAL_LAST_CASHFLOW_TYPE;
	}

	const AQLDate& Cashflow::getPaymentDate() const
	{
		return paymentDate_;
	}

    void Cashflow::setPaymentDate(const AQLDate& paymentDate)
    {
        paymentDate_ = paymentDate;
    }

    void Cashflow::setFxFixingDate(const AQLDate& fxFixingDate)
    {
        fxFixingDate_ = fxFixingDate;
    }

	double Cashflow::getNotionalExchange() const
	{
		return notionalExchange_;
	}
	
    double Cashflow::getFwdFxRate() const
	{
		return fwdFxRate_;
	}

    void Cashflow::setNotional(double notional)
	{
        notional_ = notional;
	}
	
    void Cashflow::setNotionalExchange(double notionalExchange)
	{
        notionalExchange_ = notionalExchange;
	}

    void Cashflow::setFwdFxRate(double fwdFxRate)
	{
		fwdFxRate_ = fwdFxRate;
	}

    void Cashflow::setPreviousCashflowsInfo(const std::vector<CashFlowInfo>& previousCashFlowsInfo)
	{
		previousCashFlowsInfo_ = previousCashFlowsInfo;
	}

    bool Cashflow::isUpfrontCashflow() const
    {
        return (cashflowType_ == FIRST_NOTIONAL_EXCHANGE_CASHFLOW_TYPE);
	}
	
}

