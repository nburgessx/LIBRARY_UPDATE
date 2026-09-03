/*
 * @brief			Class the defines the Protection Leg of a Credit Default Swap
 * @Created:		15 Feb 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "CDSLeg.h"
#include "ProtectionSchedule.h"

namespace etrading
{
    class ProtectionLeg : public CDSLeg
    {
    public:

		ProtectionLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={});
        ProtectionLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule);
		ProtectionLeg(const ProtectionLeg& rhs);
		virtual ~ProtectionLeg() {}
        
        LegPtr clone();

        ScheduleTypeEnum getType() const;
       	

		static std::vector<std::string> legLVBWithoutScheduleKeys()
		{
			std::vector<std::string> expectedKeys = Leg::lvbKeys();
			return expectedKeys;
		}

		static std::vector<std::string> lvbKeys()
		{
			std::vector<std::string> expectedKeys = ProtectionLeg::legLVBWithoutScheduleKeys();
			std::vector<std::string> scheduleKeys = ProtectionSchedule::lvbKeys();
			expectedKeys.insert(expectedKeys.end(), scheduleKeys.begin(), scheduleKeys.end());
			return expectedKeys;
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
		virtual double riskFreePVtoStoppingDate( DataProvider& dataProvider, const CreditModel& creditModel, const LADate& stoppingDate, const double discountFactorAtStoppingDate = std::numeric_limits<double>::quiet_NaN(), const bool payDefaultCashflowsOnNextCouponDate = false );

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

		//Override
		PaymentTriggerEnum getPaymentTrigger() const;

	private:
	
		/* @brief Updates the cashflow survival / default probabilities using the hazard rate and recovery rate parameters
		*
		* @param[in]	asOfDate				The valuation date of the leg
		* @param[in]	hazardRate				The CDS hazard rate parameter, used to calculate survival probabilities
		* @param[in]	recoveryRate			The estimated amount of capital recovered after default
		* @param[in]	includeAccruedInterest	Specifies whether cashflows should include the accruedInterest
		*/
		void setSurvivalProbabilitiesUsingHazardRate( const LADate& asOfDate, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest );

		/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
		*
		* @param[in]	asOfDate				The valuation date of the leg
		* @param[in]	creditModel				The calibrated credit model
		*/
		void setSurvivalProbabilitiesUsingCreditModel( const LADate& asOfDate, const CreditModel& creditModel );

	};

}

