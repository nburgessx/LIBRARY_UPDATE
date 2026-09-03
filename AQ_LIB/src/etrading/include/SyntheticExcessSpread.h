/*
 * @brief			Structured Credit Synthetic Excess Spread (SXS). Used in the Capital Structure.
 *					The SXS is used as a synthetic tranxhe below equity which can absorb losses.
 *					The SXS can be periodically replenished.
 *
 *					This code was ported from MGEN AlgoQuantHub R analytics library
 *					Author Andrew Friend
 *
 * @Created:		17th Dec 2019
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 */

#pragma once

#include "LoanCalculations.h"
#include "DataFrame.h"
#include "Trigger.h"

#include "CoreEnumerations.h"
#include "CommonConstants.h"
#include "LabelValueBlock.h"

#include <vector>
#include <string>


namespace etrading
{
	struct SyntheticExcessSpreadAllocation
	{
		int period;
		double amount;
		double level;

		double levelStart;
		double levelEnd;
		double used;
		bool isActive;
		bool isResetPeriod;
	};

	/* @brief	Specifies a SyntheticExcessSpread (SXS) configuration: specifies the tranche size and the tranche type.
	*			The SXS is used as a synthetic tranxhe below equity which can absorb losses.
	*/
	class SyntheticExcessSpread
	{
	public:
		// @brief Default constructor
		SyntheticExcessSpread();

		/* @brief	Main constructor.
		*  @param[in]	name			The name of the SyntheticExcessSpread object
		*  @param[in]	periods			The lifetime of the SyntheticExcessSpread in monthly periods
		*  @param[in]	resetFrequency	
		*  @param[in]	useOrLose		
		*/
		SyntheticExcessSpread( const std::string& name, const size_t periods, const int resetFrequency, const bool useOrLose );

		/* @brief	Constructor from LabelValueBlock
		*  @param[in]	name			The name of the SyntheticExcessSpread object
		*  @param[in]	parameterLVB	A LabelValueBlock containing configuration parameters
		*  @param[in]	validateKeys	A boolean specifying whether to validate the keys in the LabelValueBlock
		*/
		SyntheticExcessSpread(const std::string& name, const LabelValueBlock& parameterLVB, const bool validateKeys );

		static std::vector<std::string> lvbKeys()
		{
			const std::string arr[] =
			{
				SYNTHETIC_EXCESS_SPREAD_KEYS::PERIODS
				, SYNTHETIC_EXCESS_SPREAD_KEYS::RESET_FREQUENCY
				, SYNTHETIC_EXCESS_SPREAD_KEYS::USE_OR_LOSE
			};
			std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));

			return expectedKeys;
		}

		/* @brief		Set the threshold level on the SyntheticExcessSpread object
		*  @param[in]	threshold		The threshold level
		*  @param[in]	activePeriod	Specifies the time period through which this threshold applies
		*/
		void setThreshold( const double& threshold, const size_t activePeriod );

		/* @brief
		*  @param[in]	period
		*  @param[in]	valueTrigger
		*/
		bool verify( size_t period, const double& valueTrigger ) const;

		/* @brief
		*  @param[in]	period
		*/
		void reset( const size_t period );

		/* @brief Returns the name of this syntheticExcessSpread
		*/
		std::string getName() const;

		/* @brief Returns the lifetime (number of time periods) of this syntheticExcessSpread
		*/
		size_t getPeriods() const;

		/* @brief Returns the amount of losses this syntheticExcessSpread can absorb
		*/
		double getAmount() const;

		/* @brief Returns the reset frequency
		*/
		int getResetFrequency() const;

		/* @brief	Returns the useorLose setting.
		*/
		bool getUseOrLose() const;

		/* @brief	Returns the allocation state of this syntheticExcessSpread, at the specified time period
		*  @param[in]	period	Return the allocation at this specified period
		*/
		SyntheticExcessSpreadAllocation getAllocation( const size_t period ) const;

		/* @brief	Sets the allocation state of this syntheticExcessSpread, at the specified time period
		*  @param[in]	period	Update the allocation for this period
		*  @param[in]	syntheticExcessSpreadAllocation	The updated allocation to use
		*/
		void setAllocation( const size_t period, const SyntheticExcessSpreadAllocation syntheticExcessSpreadAllocation );

	private:
		// Helper function which contains common initialization used by all constructors
		void initialize();

		std::string name_;
		size_t periods_;
		double amount_;
		int resetFrequency_;
		bool useOrLose_;

		// This is represented by a DataFrame in MGEN
		std::vector<SyntheticExcessSpreadAllocation> syntheticExcessSpreadAllocation_;
	};


}

