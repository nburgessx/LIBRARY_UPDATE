/*
 * @brief			Class the defines the Premium Leg of a Credit Default Swap
 * @Created:		15 Feb 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "CDSLeg.h"
#include "PremiumSchedule.h"


namespace etrading
{
    class PremiumLeg : public CDSLeg
    {
    public:

		PremiumLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={} );
        PremiumLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule);
		PremiumLeg(const PremiumLeg& rhs);
		virtual ~PremiumLeg() {}
        
        LegPtr clone();

        ScheduleTypeEnum getType() const;

		/* @brief[in]	Computes the risky annuity of the Premium leg
		*
		* @param[in]	dataProvider			The data provider to hold the valuation settings
		* @param[in]	hazardRate			The CDS hazard rate, used to calculate survival probabilities
		* @param[in]	recoveryRate		The estimated amount of capital recovered after default
		* @param[in]	includeAccruedInterest	Specifies whether cashflows should include the accruedInterest
		* @returns: The risky annuity
		*/
		double RiskyAnnuityWithNotional(DataProvider& dataProvider, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest );

		/* @brief[in]	Computes the risky annuity of the Premium leg
		*
		* @param[in]	creditModel				The calibrated credit model
		* @returns: The risky annuity
		*/
		double RiskyAnnuityWithNotional( const CreditModel& creditModel);

		/* @brief[in]	Computes the accrued year fraction from the previous coupon date to the specified date
		*				Used in accrued interest calculations.
		*
		* @param[in]	creditModel		The calibrated credit model
		* @param[in]	toDate			The date to which we wish to calculate the year fraction
		* @returns: The year fraction
		*/
		double accruedYearFraction( const CreditModel& creditModel, const LADate& toDate );

		/* @brief[in]	Computes the accrued interest from the previous coupon date to the specified date
		*				i.e. this corresponds to the amount of premium coupon that is accrued
		*
		* @param[in]	creditModel		The calibrated credit model
		* @param[in]	toDate			The date to which we wish to calculate the accrued interest for
		* @returns: The accrued interest
		*/
		double accruedInterest( const CreditModel& creditModel, const LADate& toDate );

		static std::vector<std::string> legLVBWithoutScheduleKeys()
		{
			std::vector<std::string> expectedKeys = Leg::lvbKeys();
			expectedKeys.push_back( CDS_KEY::CDS_SPREAD );
			return expectedKeys;
		}

		static std::vector<std::string> lvbKeys()
		{
			std::vector<std::string> expectedKeys = PremiumLeg::legLVBWithoutScheduleKeys();
			std::vector<std::string> scheduleKeys = PremiumSchedule::lvbKeys();
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
		void setSurvivalProbabilitiesUsingHazardRate( const LADate& asOfDate, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest  );

		/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
		*
		* @param[in]	asOfDate				The valuation date of the leg
		* @param[in]	creditModel				The calibrated credit model
		*/
		void setSurvivalProbabilitiesUsingCreditModel( const LADate& asOfDate, const CreditModel& creditModel );

		/*
		* @brief	Helper method which initializes the survival probability for all leg cashflows to 1.0.
		*			i.e. make all the cashflows risk-free.
		*/
		void resetRiskFreeCashflows();

		// Specifies whether the leg cashflows contain credit risk: i.e. do they pay always, or pay on survival	
		PaymentTriggerEnum paymentTrigger_;
	};

}

