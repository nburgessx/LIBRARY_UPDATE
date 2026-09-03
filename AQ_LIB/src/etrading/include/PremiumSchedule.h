/*
 * @brief			Class the defines the Premium Leg Schedule of a Credit Default Swap
 * @Created:		15 Feb 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "Schedule.h"
#include "CreditModel.h"

namespace etrading
{
    class PremiumSchedule : public Schedule
    {
    public:
		PremiumSchedule(const std::string& instanceName);
		PremiumSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
		PremiumSchedule(const PremiumSchedule& rhs);
		virtual ~PremiumSchedule() {};
        SchedulePtr clone();

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

		/* @brief	Get the Asset Performance calculation type
		*			i.e. wether the underlying asset performance should be included in the cashflow payments
		*/
		TRSAssetPerformanceEnum getAssetPerformanceEnum() const;

		/* @brief Updates the Schedule cashflows to include the Total-Return-Swap performance of the underlying asset.
		*  @param[in]	creditModel		The credit model used to obtain survival and default probabilities
		*  @param[in]	paymentTrigger	Specifies whether the cashflows have credit risk: i.e. do they pay always, or pay on survival	
		*/
		void updateTRSPerformance( const CreditModel& creditModel, const PaymentTriggerEnum& paymentTrigger );

		double getCDSSpread() const;
		void setCDSSpread( double cdsSpread );

        const DataSchema generateCashflowSchema(const std::string& schemaName="") const;
        std::map<std::string, std::vector<std::string>> getCashflowDataMap() const;

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

   	protected:
		void populateNotionalAndPaymentFreqEnum( const LabelValueBlock& scheduleLVB=LabelValueBlock() );
        virtual void createCashflows();
		virtual void createUpfrontCashflow( const LADate& paymentDate, double leverage );

	private:
        double cdsSpread_;
		
		// Attributes specific to the Total Return Swap performance
		std::string bondName_;								// The underlying asset
		TRSAssetPerformanceEnum assetPerformanceEnum_;		// Specifies how the performance is paid. For example at maturity of the contract
		double bondBaseDirtyPriceFixing_;					// dirty price quote of the underlying bond asset as of the TRS contract effective date

    };

}

