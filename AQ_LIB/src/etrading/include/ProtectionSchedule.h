/*
 * @brief			Class the defines the Protection Leg Schedule of a Credit Default Swap
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
    class ProtectionSchedule : public Schedule
    {
    public:
		ProtectionSchedule(const std::string& instanceName);
		ProtectionSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);
		ProtectionSchedule(const ProtectionSchedule& rhs);
		virtual ~ProtectionSchedule() {};
        SchedulePtr clone();


		/* @brief Updates the cashflow survival / default probabilities using the hazard rate and recovery rate parameters
		*
		* @param[in]	asOfDate				The valuation date of the leg
		* @param[in]	hazardRate				The CDS hazard rate parameter, used to calculate survival probabilities
		* @param[in]	recoveryRate			The estimated amount of capital recovered after default
		* @param[in]	includeAccruedInterest	Not used for the Protection Schedule / Cashflows
		*/
		void setSurvivalProbabilitiesUsingHazardRate( const LADate& asOfDate, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest );

		/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
		*
		* @param[in]	asOfDate				The valuation date of the leg
		* @param[in]	creditModel				The calibrated credit model
		*/
		void setSurvivalProbabilitiesUsingCreditModel( const LADate& asOfDate, const CreditModel& creditModel );

        const DataSchema generateCashflowSchema(const std::string& schemaName="") const;
        std::map<std::string, std::vector<std::string>> getCashflowDataMap() const;

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

   	protected:
		void populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB=LabelValueBlock());
        virtual void createCashflows();

	private:
        
    };

}

