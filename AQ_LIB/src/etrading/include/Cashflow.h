/*
 * @brief			Class the defines the leg cashflow
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once

#include <boost/math/special_functions/fpclassify.hpp> 
#include "SwapValidation.h"
#include "ScheduleValidation.h"
#include "ParameterValidation.h"
#include "CoreEnumerations.h"
#include "DataProvider.h"

namespace etrading
{

    // Struct to Store the useful cashflow's information
    struct CashFlowInfo
    {
        double coupon;
        double floatRate;
        double spreadAmount;
		LADate paymentDate;
		double notional;
	};

	struct CashFlowBespokeInfo
	{
		DayCountEnum accrualDaycount = NONE_DAYCOUNT;
		std::string forecastCurve = "";
	};


    class Cashflow
    {
    public:
        Cashflow();
        Cashflow(const PayReceiveEnum& payReceive, const LADate& fixingDate, const LADate& accrualStartDate, const LADate& accrualEndDate, int accrualDays, double accrualYearFraction, const LADate& paymentDate, 
                double notional, double leverage, double couponMultiplier, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType, const CashFlowBespokeInfo& bespokeInfo);
        virtual ~Cashflow() {}

        virtual std::shared_ptr<Cashflow> clone();

        virtual void flipPayerReceiver();
		PayReceiveEnum getPayReceive() const;
        double getPayReceiveIndicator() const;

		virtual double getCompoundRate( const CashflowData& cashflowData ) const;
        virtual double calculateCompounding(double coupon, const CashflowData& cashflowData ) const;
        //Used internallly for Coupon calculation and ZeroCoupon Leg's previous coupon calculation
        virtual double getRegularCoupon( const CashflowData& cashflowData ) const;
        //Coupon shown in the cashflow display 
        virtual double getCoupon( const CashflowData& cashflowData ) const;
        double getUpfrontCashflowCoupon() const;

        double getZeroCouponSwapCoupon( const CashflowData& cashflowData ) const;
		virtual double getCouponPv( const CashflowData& cashflowData, bool nativeCurrencyPV=false) const;
        virtual double getAnnuity( const CashflowData& cashflowData ) const;
        virtual double getNotionalExchangePv( const CashflowData& cashflowData ) const;
        //A vectors of pair of previousCoupon and previousSpreadAmount
		void setPreviousCashflowsInfo(const std::vector<CashFlowInfo>& previousCashFlowsInfo);
        virtual double getSpreadAmount(const CashflowData& cashflowData) const;
        
        double getNotional() const;
		double getLeverage() const;
		double getCouponMultiplier() const;
		double getAccrualDays() const;
		double getAccrualYearFraction() const;
		const LADate& getAccrualStartDate() const;
		const LADate& getAccrualEndDate() const;
		const LADate& getFixingDate() const;
		const LADate& getPaymentDate() const;
		double getNotionalExchange() const;
		double getFwdFxRate() const;

        void setNotional(double notional);
        void setNotionalExchange(double notionalExchange);
		void setFwdFxRate(double fwdFxRate);
		void setFxFixingDate(const LADate& fxFixingDate);

		void setPaymentDate(const LADate& paymentDate);
        bool isUpfrontCashflow() const;

        void setFixingEndDate(const LADate& fixingEndDate); 
        const LADate& getFixingEndDate() const;

		FrequencyEnum getPaymentFreqEnum() const;

		CashFlowBespokeInfo bespokeInfo() const { return bespokeInfo_; };
		LADate getFxFixingDate() const { return fxFixingDate_; };

		bool isLastCashflow() const;

		//This is tempory function until the compounding calculation move to leg level
		void setCashflowTypeToLast();

        //dummy methods, just to avoid downcasting
		virtual double getFixedRate() const { throw LACoreInvalidData("#Error: getFixedRate not supported", __FILE__, __LINE__); };
   		virtual double getSpread() const { throw LACoreInvalidData("#Error: getSpread not supported", __FILE__, __LINE__); };
   		virtual void setSpread(double spread) { throw LACoreInvalidData("#Error: setSpread not supported", __FILE__, __LINE__); };
   		virtual double getAmount() const { throw LACoreInvalidData("#Error: getAmount not supported", __FILE__, __LINE__); };

		virtual double getSurvivalProbability() const { throw LACoreInvalidData("#Error: getSurvivalProbability not supported", __FILE__, __LINE__); };
		virtual double getMarginalDefaultProbability() const { throw LACoreInvalidData("#Error: getMarginalDefaultProbability not supported", __FILE__, __LINE__); };
		virtual double getCdsSpread() const { throw LACoreInvalidData("#Error: getCdsSpread not supported", __FILE__, __LINE__); };
		virtual double getPremiumCoupon() const { throw LACoreInvalidData("#Error: getPremiumCoupon not supported", __FILE__, __LINE__); };
		virtual double getAccrualOnDefaultCoupon() const { throw LACoreInvalidData("#Error: getAccrualOnDefaultCoupon not supported", __FILE__, __LINE__); };
		virtual const LADate& getBondExDividendDate() const { throw LACoreInvalidData("#Error: getBondExDividendDate not supported", __FILE__, __LINE__); };
		virtual double getBondTrueYieldYearFraction() const { throw LACoreInvalidData("#Error: getBondTrueYieldYearFraction not supported", __FILE__, __LINE__); };


		static std::vector<std::string> bespokeCashflowLVBKeys()
		{
			// Keys are in upper case
			const std::string arr[] =
			{
				toUpperString(FIXING_DATE_HEADER),
				toUpperString(ACCRUAL_START_HEADER),
				toUpperString(ACCRUAL_END_HEADER),
				toUpperString(PAYMENT_DATE_HEADER),

				toUpperString(NOTIONAL_HEADER),
				toUpperString(LEVERAGE_HEADER),
				toUpperString(COUPON_MULTIPLIER_HEADER),
				toUpperString(FIXED_RATE_HEADER),
				toUpperString(FLOAT_SPREAD_HEADER),

				toUpperString(ACCRUAL_DAYCOUNT_HEADER),
				toUpperString(FORECAST_CURVE_HEADER),
			};

			std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));
			return expectedKeys;
		};

	protected:

        PayReceiveEnum payReceive_;

        // These fields are related to MTM xccy swap, user need to select a leg that will be notional adjusted by the fwdFxRate based on t he fxFixingDate
		LADate fxFixingDate_;		
   	    double fwdFxRate_; 

        // previous coupons based on the paymentDate
        std::vector<CashFlowInfo> previousCashFlowsInfo_;

		//The following members are Schedule related

		//Notional is scaled by leverage
		double notional_; 
		//Notional Exchange is scaled by leverage
		double notionalExchange_;
		
		double leverage_;
		// Note: couponMultipler is NOT applied to notionalExchange, but leverage is.
		double couponMultiplier_;
		LADate fixingDate_;	
        LADate accrualStartDate_;			
		LADate accrualEndDate_;			
		int accrualDays_;
        double accrualYearFraction_;		
		LADate paymentDate_;

		//Bespoke cashflow related:
		CashFlowBespokeInfo bespokeInfo_;

        //This is one required to calculate the OIS float rate
		LADate fixingEndDate_;			

        //These fields required for ZeroCouponSwap
        FrequencyEnum paymentFreqEnum_;

        // When cashflowType is NORMAL_LAST_CASHFLOW_TYPE, this field is to identify where to do the final compounding (e.g. adding spread for simple compounding) 
        //1) For a leg with compounding due to payment enum AT_MATURITY (i.e. zero coupon swap), it denotes a leg's last cashflow. In this case only one cashflow with NORMAL_LAST_CASHFLOW_TYPE true.
        //2) For other legs with compounding due to accrual freq less than payment freq, it denotes the last cashflow under the same payment date. In this case there may be serveral cashflows with NORMAL_LAST_CASHFLOW_TYPE true. 
		CashflowTypeEnum cashflowType_;

    };

    typedef std::shared_ptr<Cashflow> CashflowPtr;
}
