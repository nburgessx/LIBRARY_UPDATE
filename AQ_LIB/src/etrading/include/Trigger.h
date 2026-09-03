#pragma once

#include "LoanCalculations.h"
#include "CoreEnumerations.h"

#include <vector>

namespace etrading
{
	// Contains the monthly periodic state of the trigger object.
	struct TriggerStatus
	{
		std::vector<int> period;	// The period number
		std::vector<double> level;	// The trigger level
		std::vector<bool> isActive;	// Whether the trigger is active
		std::vector<double> value;	
		std::vector<bool> state;	
	};

	class Trigger
	{
	public:
		/* @brief		Main constructor.
		*  @param[in]	name		The name of this trigger
		*  @param[in]	nPeriods	The number of periods this trigger is active
		*/
		Trigger( const std::string& name, const size_t nPeriods = 0 );

		/* @brief		Returns the name of this trigger.
		*/
		std::string getName() const;

		/* @brief		Sets the threshold level and period of time the trigger is active.
		*  @param[in]	threshold		The threshold level
		*  @param[in]	activePeriod	Specifies the period up to which the trigger is active.
		*								i.e. after the activePeriod, the trigger is no longer active
		*/
		void setThreshold( const double& threshold, const size_t activePeriod );

		/* @brief		Sets the threshold level of this trigger and the active range of this trigger
		*  @param[in]	threshold			The threshold level
		*  @param[in]	activePeriodStart	Specifies the start of the active period
		*  @param[in]	activePeriodEnd		Specifies the end of the active period
		*/
		void setThresholdRange( const double& threshold, const size_t activePeriodStart, const size_t activePeriodEnd );

		/* @brief		Sets the threshold level of this trigger and the active range of this trigger
		*  @param[in]	thresholds			A vectof of threshold levels, one per period of the trigger
		*  @param[in]	isActive			A vector of bool specifying whether the trigger is active in each period
		*/
		void setThresholdVector( const std::vector<double>& thresholds, const std::vector<bool>& isActive );

		/* @brief		Decides whether the specified value falls within the trigger threshold level
		*				i.e. decides if the trigger will fire
		*  @param[in]	period			The time period to consider.
		*  @param[in]	value			The input value to use when determining whether the trigger should fire.
		*  @returns		A boolean which specifies whether the input value causes the trigger to fire.
		*/
		bool verify( const size_t period, const double value ) const;

		TriggerStatus triggerStatus_;

	private:
		std::string name_;
		size_t nPeriods_;

		
	};




}

