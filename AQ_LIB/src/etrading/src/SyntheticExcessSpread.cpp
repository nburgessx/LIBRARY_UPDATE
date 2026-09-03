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

#include "SyntheticExcessSpread.h"

#include "LabelValueBlockValidation.h"


namespace etrading
{
	// @brief Default constructor
	SyntheticExcessSpread::SyntheticExcessSpread()
		: name_( "" ), periods_( 0 ), resetFrequency_( 0 ), useOrLose_( false )
	{
	}


	// Helper function which contains common initialization used by all constructors
	void SyntheticExcessSpread::initialize( )
	{
		syntheticExcessSpreadAllocation_.resize( periods_ );

		for (size_t period = 0; period < periods_; period++)
		{
			syntheticExcessSpreadAllocation_[ period ].amount = 0.0;
			syntheticExcessSpreadAllocation_[ period ].levelStart = 0.0;
			syntheticExcessSpreadAllocation_[ period ].levelEnd = 0.0;
			syntheticExcessSpreadAllocation_[ period ].used = false;

			syntheticExcessSpreadAllocation_[ period ].period = (period + 1);
			syntheticExcessSpreadAllocation_[ period ].isResetPeriod = false;
			syntheticExcessSpreadAllocation_[ period ].isActive = false;
		}
	}

	/* @brief	Main constructor.
	*  @param[in]	name			The name of the SyntheticExcessSpread object
	*  @param[in]	periods			The lifetime of the SyntheticExcessSpread in monthly periods
	*  @param[in]	resetFrequency
	*  @param[in]	useOrLose
	*/
	SyntheticExcessSpread::SyntheticExcessSpread( const std::string& name, const size_t periods, const int resetFrequency, const bool useOrLose)
		: name_( name ), periods_( periods ), resetFrequency_( resetFrequency ), useOrLose_( useOrLose )
	{
		initialize();
	}

	/* @brief	Constructor from LabelValueBlock
	*  @param[in]	name			The name of the SyntheticExcessSpread object
	*  @param[in]	parameterLVB	A LabelValueBlock containing configuration parameters
	*  @param[in]	validateKeys	A boolean specifying whether to validate the keys in the LabelValueBlock
	*/
	SyntheticExcessSpread::SyntheticExcessSpread( const std::string& name, const LabelValueBlock& parameterLVB, const bool validateKeys )
		: name_(name)
	{
		std::string parameterLVBName = "ParameterLVB";

		validateKeysForLVB( SyntheticExcessSpread::lvbKeys(), parameterLVB.getKeys(), validateKeys );

		periods_		= (int) parameterLVB.getCompulsoryValueAsDouble( SYNTHETIC_EXCESS_SPREAD_KEYS::PERIODS, parameterLVBName );
		resetFrequency_ = (int)parameterLVB.getCompulsoryValueAsDouble( SYNTHETIC_EXCESS_SPREAD_KEYS::RESET_FREQUENCY, parameterLVBName );
		useOrLose_		= parameterLVB.getCompulsoryValueAsBool( SYNTHETIC_EXCESS_SPREAD_KEYS::USE_OR_LOSE, parameterLVBName );

		initialize();
	}

	/* @brief		Set the threshold level on the SyntheticExcessSpread object
	*  @param[in]	threshold		The threshold level
	*  @param[in]	activePeriod	Specifies the time period through which this threshold applies
	*/
	void SyntheticExcessSpread::setThreshold( const double& threshold, const size_t activePeriod )
	{
		// We define all periods (including activePeriod) to be zero based in MLIB.		
		MLIB_REQUIRE( activePeriod < periods_, "Specified activePeriod is larger than the SyntheticExcessSpread period size" );

		for ( size_t period = 0; period < periods_; period++ )
		{
			syntheticExcessSpreadAllocation_[ period ].level = threshold;

			const bool isActive = period <= activePeriod;
			syntheticExcessSpreadAllocation_[period].isActive = isActive;
		}
	}

	/* @brief
	*  @param[in]	period
	*  @param[in]	valueTrigger
	*/
	bool SyntheticExcessSpread::verify( size_t period, const double& valueTrigger ) const
	{
		MLIB_REQUIRE( period < periods_, "Specified period is larger than the SyntheticExcessSpread period size" );

		const double level = syntheticExcessSpreadAllocation_[period].level;

		return ( valueTrigger < level );
	}


	/* @brief
	*  @param[in]	period
	*/
	void SyntheticExcessSpread::reset( const size_t period )
	{
		// *** TODO df[period]
		// i.e. clear out the stored data and resize the df to period number of rows?
	}

	/* @brief Returns the name of this syntheticExcessSpread
	*/
	std::string SyntheticExcessSpread::getName() const
	{
		return name_;
	}

	/* @brief Returns the lifetime (number of time periods) of this syntheticExcessSpread
	*/
	size_t SyntheticExcessSpread::getPeriods() const
	{
		return periods_;
	}

	/* @brief Returns the amount of losses this syntheticExcessSpread can absorb
	*/
	double SyntheticExcessSpread::getAmount() const
	{
		return amount_;
	}

	/* @brief Returns the reset frequency
	*/
	int SyntheticExcessSpread::getResetFrequency() const
	{
		return resetFrequency_;
	}

	/* @brief	Returns the useorLose setting.
	*/
	bool SyntheticExcessSpread::getUseOrLose() const
	{
		return useOrLose_;
	}

	/* @brief	Returns the allocation state of this syntheticExcessSpread, at the specified time period
	*  @param[in]	period	Return the allocation at this specified period
	*/
	SyntheticExcessSpreadAllocation SyntheticExcessSpread::getAllocation( const size_t period ) const
	{
		MLIB_REQUIRE( period < periods_, "Specified period is larger than the SyntheticExcessSpread period size" );

		return syntheticExcessSpreadAllocation_[ period ];
	}

	/* @brief	Sets the allocation state of this syntheticExcessSpread, at the specified time period
	*  @param[in]	period	Update the allocation for this period
	*  @param[in]	syntheticExcessSpreadAllocation	The updated allocation to use
	*/
	void SyntheticExcessSpread::setAllocation( const size_t period, const SyntheticExcessSpreadAllocation syntheticExcessSpreadAllocation )
	{
		MLIB_REQUIRE(period < periods_, "Specified period is larger than the SyntheticExcessSpread period size" );

		syntheticExcessSpreadAllocation_[ period ] = syntheticExcessSpreadAllocation;
	}


}

