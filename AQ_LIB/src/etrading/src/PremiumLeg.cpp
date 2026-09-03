#include "PremiumLeg.h"
#include "PremiumCashflow.h"

#include "BondUtilities.h"


namespace etrading
{
   
	PremiumLeg::PremiumLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule) : CDSLeg(legLVB, instanceName, schedule)
	{
        const std::string inputLVB = "legLVB";

        legStaticData_ = LegStaticDataPtr (new LegStaticData(legLVB));

		// Specifies whether the cashflows have credit risk: i.e. do they PayAlways, or PayOnSurvival	
		// The default value is PayOnSurvival
		paymentTrigger_ = toPaymentTriggerEnum(legLVB.getOptionalValueAsLAString(IRS_KEY::PAYMENT_TRIGGER, "PAYONSURVIVAL").getCString());

		//Schedule
		if (schedule==nullptr)
		{
			schedule_ = SchedulePtr(new PremiumSchedule(legLVB, instanceName));
		}
	}

    PremiumLeg::PremiumLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : CDSLeg(instanceName, legStaticData, schedule), paymentTrigger_(PAY_ON_SURVIVAL)
    {}
	
    LegPtr PremiumLeg::clone()
    {
        LegPtr leg = LegPtr(new PremiumLeg(*this));
        return leg;
    }

	PremiumLeg::PremiumLeg( const PremiumLeg& rhs) : CDSLeg(rhs), paymentTrigger_(rhs.paymentTrigger_)
	{}

    ScheduleTypeEnum PremiumLeg::getType() const
    {
        return PREMIUM_SCHEDULE_TYPE;
    }

	/* @brief Updates the cashflow survival / default probabilities using the hazard rate and recovery rate parameters
	*
	* @param[in]	asOfDate				The valuation date of the leg
	* @param[in]	hazardRate				The CDS hazard rate parameter, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	includeAccruedInterest	Specifies whether cashflows should include the accruedInterest
	*/
	void PremiumLeg::setSurvivalProbabilitiesUsingHazardRate( const LADate& asOfDate, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest  )
	{
		// OK to use static_pointer_cast here because we know the schedule type for sure (the PremiumLeg created it).
		std::shared_ptr<PremiumSchedule> premiumSchedule = std::static_pointer_cast<PremiumSchedule> ( schedule_ );

		switch ( paymentTrigger_ )
		{
			case PAY_ON_SURVIVAL:
				premiumSchedule->setSurvivalProbabilitiesUsingHazardRate( asOfDate, hazardRate, recoveryRate, includeAccruedInterest );
				break;

			case PAY_ALWAYS:
				// Set all leg cashflow survival probabilities to 1.0. i.e. the coupons always pay out.
				resetRiskFreeCashflows();
				break;

			default:
				// AQ_THROW( "Unsupported PaymentTrigger type: " + toString( getPaymentTrigger() + "Only 'PayAlways' and 'PayOnSurvival' is currently supported." ));
				AQ_THROW( "Unsupported PaymentTrigger type: " + toString( getPaymentTrigger() ));
				break;
		}

		AQ_REQUIRE( premiumSchedule->getAssetPerformanceEnum() == NO_PERFORMANCE, "Total Return Swap performance calculation requires a credit model." );
	}

	/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
	*
	* @param[in]	asOfDate				The valuation date of the leg
	* @param[in]	creditModel				The calibrated credit model
	*/
	void PremiumLeg::setSurvivalProbabilitiesUsingCreditModel( const LADate& asOfDate, const CreditModel& creditModel  )
	{
		// OK to use static_pointer_cast here because we know the schedule type for sure (the PremiumLeg created it).
		std::shared_ptr<PremiumSchedule> premiumSchedule = std::static_pointer_cast<PremiumSchedule> ( schedule_ );

		switch ( paymentTrigger_ )
		{
			case PAY_ON_SURVIVAL:
				premiumSchedule->setSurvivalProbabilitiesUsingCreditModel( asOfDate, creditModel );
				break;

			case PAY_ALWAYS:
				resetRiskFreeCashflows();
				break;

			default:
				AQ_THROW( "Unsupported PaymentTrigger type: " + toString( getPaymentTrigger() ));
				break;
		}
		
		premiumSchedule->updateTRSPerformance( creditModel, paymentTrigger_ );

	}
	
	/* @brief[in]	Computes the risky annuity of the Premium leg
	*
	* @param[in]	dataProvider			The data provider to hold the valuation settings
	* @param[in]	hazardRate			The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate		The estimated amount of capital recovered after default
	* @param[in]	legName				Optionally calculate the PV of the specified leg only
	* @returns: The risky annuity
	*/
	double PremiumLeg::RiskyAnnuityWithNotional( DataProvider& dataProvider, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest )
	{
		// Calculate Discount Factors
		initializeDataProvider( dataProvider);

		// Calculate Survival Probabilities
		const LADate& asOfDate = dataProvider.getValuationSettings().getValuationDate();
		setSurvivalProbabilitiesUsingHazardRate( asOfDate, hazardRate, recoveryRate, includeAccruedInterest );

        //get all the cashflows including upfront cashflow
        auto cashflows = schedule_->getAllCashflows();
        if (cashflows.size() == 0)
		{
			throw LACoreInvalidData("#Error: No cashflow has been built yet", __FILE__, __LINE__ );
		}

        double riskyAnnuity = 0;
 		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			auto curCashflow = cashflows[i];
			const std::shared_ptr<PremiumCashflow>& premiumCashflow = std::static_pointer_cast<PremiumCashflow>( curCashflow );
			riskyAnnuity += premiumCashflow->getRiskyAnnuity( dataProvider.getCashflowDataIncludingUpfront( i ) );
		}

		return riskyAnnuity;
	}

	/* @brief[in]	Computes the risky annuity of the Premium leg
	*
	* @param[in]	creditModel				The calibrated credit model
	* @returns: The risky annuity
	*/
	double PremiumLeg::RiskyAnnuityWithNotional( const CreditModel& creditModel)
	{
		// Calculate Discount Factors
		DataProvider dataProvider(ValuationSettings(creditModel, {}));
		initializeDataProvider( dataProvider);

		// Calculate Survival Probabilities
		const LADate& asOfDate = dataProvider.getValuationSettings().getValuationDate();
		setSurvivalProbabilitiesUsingCreditModel( asOfDate, creditModel );

        //get all the cashflows including upfront cashflow
        auto cashflows = schedule_->getAllCashflows();
        if (cashflows.size() == 0)
		{
			throw LACoreInvalidData("#Error: No cashflow has been built yet", __FILE__, __LINE__ );
		}

        double riskyAnnuity = 0;
 		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			auto curCashflow = cashflows[i];
			const std::shared_ptr<PremiumCashflow>& premiumCashflow = std::static_pointer_cast<PremiumCashflow>( curCashflow );
			riskyAnnuity += premiumCashflow->getRiskyAnnuity( dataProvider.getCashflowDataIncludingUpfront( i ) );
		}

		return riskyAnnuity;
	
	}

	/* @brief[in]	Computes the accrued year fraction from the previous coupon date to the specified date
	*				Used in accrued interest calculations.
	*
	* @param[in]	creditModel		The calibrated credit model
	* @param[in]	toDate			The date to which we wish to calculate the year fraction
	* @returns: The year fraction
	*/
	double PremiumLeg::accruedYearFraction( const CreditModel& creditModel, const LADate& toDate )
	{
		// Calculate Discount Factors
		DataProvider dataProvider(ValuationSettings(creditModel, {}));
		initializeDataProvider(dataProvider);

		// Calculate Survival Probabilities
		const LADate& asOfDate = dataProvider.getValuationSettings().getValuationDate();
		setSurvivalProbabilitiesUsingCreditModel(asOfDate, creditModel);

		//get all the cashflows including upfront cashflow
		auto cashflows = schedule_->getAllCashflows();
		AQ_REQUIRE(cashflows.size() > 0, "No cashflow has been built yet");

		// Find most recent cashflow date just passed
		for (size_t i = 0; i < cashflows.size(); i++)
		{
			auto curCashflow = cashflows[i];
			auto paymentDate = curCashflow->getPaymentDate();
			if ( paymentDate >= toDate )
			{
				double accruedDays;
				double accruedYearFraction;
				if (i == 0)
				{
					auto effectiveDate = getSchedule()->getEffectiveDate();
					accruedDays = getBondActualCouponPeriodDays( effectiveDate, toDate, schedule_->getAccrualDaycount());
					accruedYearFraction = getYearFraction( effectiveDate, toDate, schedule_->getAccrualDaycount() );
				}
				else
				{
					auto prevCashflow = cashflows[i - 1];
					auto paymentDate = prevCashflow->getPaymentDate();
					accruedDays = getBondActualCouponPeriodDays( paymentDate, toDate, schedule_->getAccrualDaycount());
					accruedYearFraction = getYearFraction( paymentDate, toDate, schedule_->getAccrualDaycount());
				}
				return accruedYearFraction;
			}
		}
		return 0.0;
	}

	/* @brief[in]	Computes the accrued interest from the previous coupon date to the specified date
	*				i.e. this corresponds to the amount of premium coupon that is accrued
	*
	* @param[in]	creditModel		The calibrated credit model
	* @param[in]	toDate			The date to which we wish to calculate the accrued interest for
	* @returns: The accrued interest
	*/
	double PremiumLeg::accruedInterest( const CreditModel& creditModel, const LADate& toDate )
	{
		const double accruedInterestYearFraction = accruedYearFraction( creditModel, toDate );

		//get all the cashflows including upfront cashflow
		auto cashflows = schedule_->getAllCashflows();
		AQ_REQUIRE ( cashflows.size() > 0, "No cashflow has been built yet" );

		auto curCashflow = cashflows[0];
		const double premiumRate = curCashflow->getFixedRate();
		const double accruedInterest = premiumRate * accruedInterestYearFraction;
		return accruedInterest;
	}



	/* @brief Calculates the leg PV assuming all coupons are paid with certainty right up to the stoppingDate. No coupons are paid after the stopping date.
	*  @param[in]	DataProvider						The data provider which holds the valuation settings, discount factors
	*  @param[in]	creditModel							The calibrated credit model
	*  @param[in]	stoppingDate						The date at which the underlying bond defaults
	*  @param[in]	discountFactorAtStoppingDate		The discount factor on the stopping date, if required.
	*  @param[in]	payDefaultCashflowsOnNextCouponDate	Whether to pay default-related cashflows on the stoppingDate, or on the next coupon date.
	*				When set to TRUE (pay on next coupon date ), the PV should match the analytic formula.
	*  @returns	The calculated PV value
	*/
	double PremiumLeg::riskFreePVtoStoppingDate( DataProvider& dataProvider, const CreditModel& creditModel, const LADate& stoppingDate, const double discountFactorAtStoppingDate, const bool payDefaultCashflowsOnNextCouponDate )
	{

        //get all the cashflows including the upfrontCashflow
        auto cashflows = schedule_->getAllCashflows();
        if (cashflows.size() == 0)
		{
			throw LACoreInvalidData("#Error: No cashflow has been built yet", __FILE__, __LINE__ );
		}

        double pv = 0.0;
 		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			auto curCashflow = cashflows[i];
			const std::shared_ptr<PremiumCashflow>& premiumCashflow = std::static_pointer_cast<PremiumCashflow>( curCashflow );

			// Check each coupon payment date to see if it occurs before or after stopping date
			if ( stoppingDate < premiumCashflow->getAccrualEndDate()  )
			{
				if ( premiumCashflow->getIncludeAccruedInterest() )
				{
					// The coupon pays out after the stopping date. We do not pay out the whole coupon.
					// Calculate the accrued amount of the coupon.

					std::shared_ptr<PremiumSchedule> premiumSchedule = std::static_pointer_cast<PremiumSchedule> ( schedule_ );
					const double notional = premiumSchedule->getNotional();
					const double cdsSpread = premiumSchedule->getCDSSpread();
				
					const LADate fromDate = premiumCashflow->getAccrualStartDate();
					const double accrualYearFraction = getYearFraction( fromDate, stoppingDate, schedule_->getAccrualDaycount() );
					const double accruedCoupon = notional * cdsSpread * accrualYearFraction;
					
					if ( payDefaultCashflowsOnNextCouponDate )
					{
						// Pay out the accrued interest on the next coupon date
						const double accruedDiscountFactor = dataProvider.getCashflowDataIncludingUpfront( i ).discountFactor;
						const double accruedCouponPV = accruedCoupon * accruedDiscountFactor;

						pv += accruedCouponPV;

						break;
					}
					else
					{
						const double accruedCouponPV = accruedCoupon * discountFactorAtStoppingDate;

						// Alternative calculation - assume the default occurs half way through the period, but paid out at end of period
						//const double couponPV2 = premiumCashflow->getRiskFreePremiumCouponPv( dataProvider.getCashflowDataIncludingUpfront( i ) );
						//const double accruedCouponPV2 = couponPV2 * 0.5; // Default on average halfway through the coupon period 

						pv += accruedCouponPV;
						break;		
					}
				}
			}
			else
			{
				// Coupon payment date is before stopping date. Pay out the entire coupon
				const double couponPV = premiumCashflow->getRiskFreePremiumCouponPv( dataProvider.getCashflowDataIncludingUpfront( i ) );
				pv += couponPV;
			}
			

		}

		return pv;
	}

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> PremiumLeg::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			SURVIVAL_PROBABILITY_HEADER
			, MARGINAL_DEFAULT_PROBABILITY_HEADER
			, PREMIUM_COUPON_HEADER
			, ACCRUAL_ON_DEFAULT_COUPON_HEADER
			, RISKY_COUPON_HEADER
			, DISCOUNT_FACTOR_HEADER
			, COUPON_PV_HEADER
		};

		return expectedList;
	}

	/*
	* @brief	Helper method which initializes the survival probability for all leg cashflows to 1.0.
	*			i.e. make all the cashflows risk-free.
	*/
	void PremiumLeg::resetRiskFreeCashflows()
	{
		auto cashflows = schedule_->getAllCashflows();
		size_t cashflowSize = cashflows.size();
		for (size_t i=0; i<cashflowSize; i++ )
		{
			CashflowPtr curCashflow = cashflows[i];

			std::shared_ptr<PremiumCashflow> premiumCashflow = std::static_pointer_cast<PremiumCashflow>( curCashflow );
			premiumCashflow->setSurvivalProbability( 1.0 );
			premiumCashflow->setMarginalDefaultProbability( 0.0 );
		}
	}

	PaymentTriggerEnum PremiumLeg::getPaymentTrigger() const
	{
		return paymentTrigger_;
	}
}

