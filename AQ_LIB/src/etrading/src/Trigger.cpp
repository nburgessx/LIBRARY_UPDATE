/*
 * @brief			Structured Credit Trigger specification. Used in the Capital Structure
 *					A trigger controls whether capital reinvestment is allowed to take place
 *
 *					This code was ported from MGEN MHI R analytics library
 *					Author Andrew Friend
 *
 * @Created:		12th Dec 2019
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 */

#include "Trigger.h"

#include "ExceptionMacros.h"	// MLIB_REQUIRE


namespace etrading
{
	/* @brief		Main constructor.
	*  @param[in]	name		The name of this trigger
	*  @param[in]	nPeriods	The number of periods this trigger is active
	*/
	Trigger::Trigger( const std::string& name, const size_t nPeriods )
		: name_( name ), nPeriods_( nPeriods )
	{
		triggerStatus_.period.resize( nPeriods );
		triggerStatus_.level.resize( nPeriods );
		triggerStatus_.isActive.resize( nPeriods );
		triggerStatus_.value.resize( nPeriods );
		triggerStatus_.state.resize( nPeriods );

		for (size_t iPeriod = 0; iPeriod < nPeriods_; iPeriod++)
		{
			triggerStatus_.period[ iPeriod ] = iPeriod + 1;
			triggerStatus_.isActive[ iPeriod ] = false;
			triggerStatus_.value[ iPeriod ] = std::numeric_limits<double>::quiet_NaN();
			triggerStatus_.state[ iPeriod ] = false;
		}
	}

	/* @brief		Returns the name of this trigger.
	*/
	std::string Trigger::getName() const
	{
		return name_;
	}
	
	/* @brief		Sets the threshold level and period of time the trigger is active.
	*  @param[in]	threshold		The threshold level
	*  @param[in]	activePeriod	Specifies the period up to which the trigger is active.
	*								i.e. after the activePeriod, the trigger is no longer active
	*/
	void Trigger::setThreshold( const double& threshold, const size_t activePeriod )
	{
		for ( size_t iPeriod = 0; iPeriod < nPeriods_; iPeriod++ )
		{
			triggerStatus_.level[ iPeriod ] = threshold;
			if ( iPeriod < activePeriod )
			{
				triggerStatus_.isActive[ iPeriod ] = true;
			}
			else
			{
				triggerStatus_.isActive[ iPeriod ] = false;
			}
		}
	}

	/* @brief		Sets the threshold level of this trigger and the active range of this trigger
	*  @param[in]	threshold			The threshold level
	*  @param[in]	activePeriodStart	Specifies the start of the active period
	*  @param[in]	activePeriodEnd		Specifies the end of the active period
	*/
	void Trigger::setThresholdRange( const double& threshold, const size_t activePeriodStart, const size_t activePeriodEnd )
	{
		for ( size_t iPeriod = activePeriodStart-1; iPeriod < nPeriods_; iPeriod++ )
		{
			if ( iPeriod < activePeriodEnd )
			{
				triggerStatus_.level[ iPeriod ] = threshold;
				triggerStatus_.isActive[ iPeriod ] = true;
			}
			else
			{
				triggerStatus_.isActive[ iPeriod ] = false;
			}
		}
	}

	/* @brief		Sets the threshold level of this trigger and the active range of this trigger
	*  @param[in]	thresholds			A vectof of threshold levels, one per period of the trigger
	*  @param[in]	isActive			A vector of bool specifying whether the trigger is active in each period
	*/
	void Trigger::setThresholdVector( const std::vector<double>& thresholds, const std::vector<bool>& isActive )
	{
		MLIB_REQUIRE( thresholds.size() == isActive.size(), "Thresholds vector should be same length as isActive vector: " << thresholds.size() << "; vs: " << isActive.size() );
		MLIB_REQUIRE( thresholds.size() <= nPeriods_, "Threshold vector length should be less than or equal to the number of time periods: " << thresholds.size() << "; vs: " << nPeriods_  );

		const size_t nThresholds = thresholds.size();
		for ( size_t iPeriod = 0; iPeriod < nThresholds; iPeriod++ )
		{
			triggerStatus_.level[ iPeriod ] = thresholds[ iPeriod ];
			triggerStatus_.level[ iPeriod ] = isActive[ iPeriod ];
		}
	}

	/* @brief		Decides whether the specified value falls within the trigger threshold level
	*				i.e. decides if the trigger will fire
	*  @param[in]	period			The time period to consider.
	*  @param[in]	value			The input value to use when determining whether the trigger should fire.
	*  @returns		A boolean which specifies whether the input value causes the trigger to fire.
	*/
	bool Trigger::verify( const size_t period, const double value ) const
	{
		MLIB_REQUIRE( period < nPeriods_, "Supplied period is outside valid range. Must be below: " << nPeriods_ << "; Actial value: " << period );
		if ( triggerStatus_.isActive[ period ] )
		{
			if ( value < triggerStatus_.level[ period ] )
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		// # trigger is not active
		// # default state is return true
		return true;
	}
}

