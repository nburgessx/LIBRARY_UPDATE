/*
 * @brief			Class the defines the Protection Leg of a Credit Default Swap
 * @Created:		15 Feb 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "ProtectionLeg.h"

namespace etrading
{
   
	ProtectionLeg::ProtectionLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule) : CDSLeg(legLVB, instanceName, schedule)
	{
        const std::string inputLVB = "legLVB";

        legStaticData_ = LegStaticDataPtr (new LegStaticData(legLVB));

		//Schedule
		if (schedule==nullptr)
		{
			schedule_ = SchedulePtr(new ProtectionSchedule(legLVB, instanceName));
		}
	}

    ProtectionLeg::ProtectionLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : CDSLeg(instanceName, legStaticData, schedule)
    {}

	LegPtr ProtectionLeg::clone()
    {
        LegPtr leg = LegPtr(new ProtectionLeg(*this));
        return leg;
    }

	ProtectionLeg::ProtectionLeg( const ProtectionLeg& rhs) : CDSLeg(rhs)
	{}


    ScheduleTypeEnum ProtectionLeg::getType() const
    {
        return PROTECTION_SCHEDULE_TYPE;
    }
	
	/* @brief Updates the cashflow survival / default probabilities using the hazard rate and recovery rate parameters
	*
	* @param[in]	asOfDate				The valuation date of the leg
	* @param[in]	hazardRate				The CDS hazard rate parameter, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	includeAccruedInterest	Specifies whether cashflows should include the accruedInterest
	*/
	void ProtectionLeg::setSurvivalProbabilitiesUsingHazardRate( const LADate& asOfDate, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest )
	{
		// OK to use static_pointer_cast here because we know the schedule type for sure (the ProtectionLeg created it).
		std::shared_ptr<ProtectionSchedule> protectionSchedule = std::static_pointer_cast<ProtectionSchedule> ( schedule_ );
		protectionSchedule->setSurvivalProbabilitiesUsingHazardRate( asOfDate, hazardRate, recoveryRate, includeAccruedInterest );
	}
	
	/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
	*
	* @param[in]	asOfDate				The valuation date of the leg
	* @param[in]	creditModel				The calibrated credit model
	*/
	void ProtectionLeg::setSurvivalProbabilitiesUsingCreditModel( const LADate& asOfDate, const CreditModel& creditModel )
	{
		// OK to use static_pointer_cast here because we know the schedule type for sure (the ProtectionLeg created it).
		std::shared_ptr<ProtectionSchedule> protectionSchedule = std::static_pointer_cast<ProtectionSchedule> ( schedule_ );
		protectionSchedule->setSurvivalProbabilitiesUsingCreditModel( asOfDate, creditModel );
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
	double ProtectionLeg::riskFreePVtoStoppingDate( DataProvider& dataProvider, const CreditModel& creditModel, const LADate& stoppingDate, const double discountFactorAtStoppingDate, const bool payDefaultCashflowsOnNextCouponDate )
	{
		const LADate effectiveDate = schedule_->getEffectiveDate();
		const LADate maturityDate  = schedule_->getMaturityDate();

		if ( (stoppingDate < effectiveDate ) || ( stoppingDate > maturityDate ) )
		{
			// The default occurs outside the date range where this CDS provides protection.
			// This means the protection leg does not pay out.
			// So the PV of the leg is 0.0.
			return 0.0;
		}

		// Calculate: (1-RR) * notional * leverage * DF(stoppingDate)
		const double recoveryRate = creditModel.getRecoveryRate();
		const double notional = schedule_->getNotional();

		const double recoveryPayout = ( 1.0 - recoveryRate ) * notional;
		
		if ( payDefaultCashflowsOnNextCouponDate )
		{
			// Find the next coupon payment date which is after or on the stopping date
			const DateVector& couponPaymentDates = schedule_->getPaymentDates();
			auto it = std::lower_bound( couponPaymentDates.begin(), couponPaymentDates.end(), stoppingDate );
			const size_t index = std::distance( couponPaymentDates.begin(), it );
			const double recoveryDiscountFactor = dataProvider.getCashflowDataIncludingUpfront( index ).discountFactor;
			const double recoveryPV = recoveryPayout * recoveryDiscountFactor;
			return recoveryPV;
		}
		else
		{
			const double recoveryPV = recoveryPayout * discountFactorAtStoppingDate;
			return recoveryPV;
		}

	}

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> ProtectionLeg::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			SURVIVAL_PROBABILITY_HEADER
			, MARGINAL_DEFAULT_PROBABILITY_HEADER
			, COUPON_HEADER
			, DISCOUNT_FACTOR_HEADER
			, COUPON_PV_HEADER
		};

		return expectedList;
	}

	PaymentTriggerEnum ProtectionLeg::getPaymentTrigger() const
	{
		return PAY_ON_DEFAULT;
	}


}

