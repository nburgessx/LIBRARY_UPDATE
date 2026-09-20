
#include "CurveInterpolation.h"

#include "CurveUtilities.h"
#include "CurveValidation.h"			// convert curve dates to terms
#include "ExceptionMacros.h"
#include "ScheduleValidation.h"
#include "ParameterValidation.h"		// stringToDate
#include "DataUtilities.h"				// trimStandardStringMatrix, AQ_TO_STRING_FROM_INT
#include "CurveInstruments.h"			// getInterpolatedForwardRate
#include "AQLDateSchedule.h"		// getDate() to get the forward endDate given the startDate
#include "AQLCurveForwardRateHelpers.h"	// Legacy method 'getMultiForwardRate()' for use when isFwdInter = true

namespace etrading
{
	// FORWARD ADJUSTMENT HELPER CLASS
	// ----------------------------------------------------------------------------------------------------------------------
	
	ForwardAdjustments::ForwardAdjustments( const AQLDate& asOfDate, const StandardStringMatrix & forwardAdjustments )
		: asOfDate_(asOfDate), forwardAdjustments_(forwardAdjustments)
	{
		forwardAdjustments_ = trimStandardStringMatrix( forwardAdjustments );

		// The valid format is three columns: start date, end date, and adjustment (double in rate)
		if ( !forwardAdjustments_.empty() )
		{
			const AQLDate asOfDate = asOfDate_;
			
			// Base 0
			const size_t turnTypeColumn		= 0; // i.e. column 1
			const size_t startDateColumn	= 1; // i.e. column 2
			const size_t endDateColumn		= 2; // i.e. column 3
			const size_t spreadOrRateColumn = 3; // i.e. column 4

			AQLDate previousStartDate;
			AQLDate previousEndDate;
			for ( size_t i = 0; i < forwardAdjustments_.size(); ++i )
			{
				const StandardStringVector& thisRow = forwardAdjustments[i];

				// Check Dimensions of Forward Adjustment Table
				AQ_REQUIRE( thisRow.size() == 4, "Forward rate adjustment table must have exactly 4 columns: AdjustmentType, StartDate, EndDate, RateOrSpread")

				const TurnOfYearAdjustmentTypeEnum turnType = toTurnOfYearAdjustmentTypeEnum( thisRow[turnTypeColumn] );
				const AQLDate startDate = stringToDate( thisRow[startDateColumn] );
				const AQLDate endDate = stringToDate( thisRow[endDateColumn] );

				AQ_REQUIRE( endDate >= startDate, "Invalid Forward Adjustment Data in Row " + AQ_TO_STRING_FROM_SIZE_T( i ) + " : StartDate cannot be after the EndDate" )
				AQ_REQUIRE( startDate > previousStartDate, "Invalid Forward Adjustment Data in Row " + AQ_TO_STRING_FROM_SIZE_T( i ) + " : StartDates must be sorted in ascending order with no duplicates" )
				AQ_REQUIRE( startDate > previousEndDate, "Invalid Forward Adjustment Data in Row " + AQ_TO_STRING_FROM_SIZE_T( i ) + " : Overlapping adjustments are not allowed, StartDates must be greater than previous EndDates" )

				// Populate Turn-of-Year Structure: Key: StartDateAsDouble, Value: TurnDataStruct( turnType, startDateAsDouble, endDateAsDouble, Adjustment )
				turnData_[ convertCurveDateToTerm( asOfDate, startDate ) ] 
					= TurnData( turnType, convertCurveDateToTerm( asOfDate, startDate ), convertCurveDateToTerm( asOfDate, endDate ), std::stod( thisRow[spreadOrRateColumn] ) );

				// Keep track of the start date to validate and check dates are sorted and have no overlaps
				previousStartDate = startDate;
				previousEndDate = endDate;
			}
		}
	}


	// Turns adjustment for forward rates
	std::vector<double> ForwardAdjustments::adjustForwardRates(const std::vector<double> & fixingDatesAsTerms, std::vector<double>& forwardRates) const
	{
		std::vector<double> adjustedForwards = forwardRates;
		AQ_REQUIRE( fixingDatesAsTerms.size() == forwardRates.size(), "Invalid Forward Rates: Unable to apply the forward rate adjustment(s). The number of fixingDates and forwardRates must match" )

		// Simply return if there is no turns data
		if ( !turnData_.empty() )
		{
			for (size_t i = 0; i < fixingDatesAsTerms.size(); ++i)
			{
				auto it = turnData_.upper_bound(fixingDatesAsTerms[i]);

				if (it != turnData_.begin())
				{
					it--;
					const TurnData& turnData = it->second;
					if(fixingDatesAsTerms[i] <= turnData.endDateAsDouble_)
					{
						switch ( turnData.turnType_ )
						{
							case RELATIVE_TURN_ADJUSTMENT:
							{
								// Add Spread
								adjustedForwards[i] += turnData.adjustment_;
								break;
							}
							case ABSOLUTE_TURN_ADJUSTMENT:
							{
								// Override Rate
								adjustedForwards[i] = turnData.adjustment_;
								break;
							}
							case NO_TURN_ADJUSTMENT:
							{
								// do nothing
								break;
							}
							default:
							{
								AQ_THROW("Invalid TurnAdjustmentType");
							}
						}
					}
				}
			}
		}
		return adjustedForwards;
	}

	// Turns adjustment for discount factors
	std::vector<double> ForwardAdjustments::adjustDiscountFactors(const std::vector<double> & paymentDatesAsTerms, std::vector<double>& discountFactors) const
	{
		std::vector<double> adjustedDiscountFactors = discountFactors;
		AQ_REQUIRE( paymentDatesAsTerms.size() == discountFactors.size(), "Invalid Discount Factors: Unable to apply the forward rate adjustment(s). The number of paymentDates and discountFactors must match" )

		// Simply return if there is no turns data
		if ( !turnData_.empty() )
		{
			for (size_t i = 0; i < paymentDatesAsTerms.size(); ++i)
			{
				auto it = turnData_.upper_bound(paymentDatesAsTerms[i]);

				if ( it != turnData_.begin() )
				{
					it--;

					double integral = 0.0;
					for (auto it2 = turnData_.cbegin(); it2 != it; it2++)
					{
						const TurnData& turnData2 = it2->second;
						switch ( turnData2.turnType_ )
						{
							case RELATIVE_TURN_ADJUSTMENT:
							{
								// *** Add Spread ***
								integral += (turnData2.endDateAsDouble_ - turnData2.startDateAsDouble_) * turnData2.adjustment_;
								break;
							}
							case ABSOLUTE_TURN_ADJUSTMENT:
							{
								// *** Override Rate ***
								integral = (turnData2.endDateAsDouble_ - turnData2.startDateAsDouble_) * turnData2.adjustment_;
								break;
							}
							case NO_TURN_ADJUSTMENT:
							{
								// do nothing
								break;
							}
							default:
							{
								AQ_THROW("Invalid TurnAdjustmentType");
							}
						}
					}

					// Add partial turn period if applicable
					const TurnData& turnData = it->second;
					if ( paymentDatesAsTerms[i] > turnData.startDateAsDouble_ && paymentDatesAsTerms[i] <= turnData.endDateAsDouble_ )
					{
						switch ( turnData.turnType_ )
						{
							case RELATIVE_TURN_ADJUSTMENT:
							{
								// *** Add Spread ***
								integral += (paymentDatesAsTerms[i] - turnData.startDateAsDouble_) * turnData.adjustment_;
								break;
							}
							case ABSOLUTE_TURN_ADJUSTMENT:
							{
								// *** Override Rate ***
								integral = (paymentDatesAsTerms[i] - turnData.startDateAsDouble_) * turnData.adjustment_;
								break;
							}
							case NO_TURN_ADJUSTMENT:
							{
								// do nothing
								break;
							}
							default:
							{
								AQ_THROW("Invalid TurnAdjustmentType");
							}
						}
					}

					adjustedDiscountFactors[i] *= exp(-integral);
				}
			}
		}
		return adjustedDiscountFactors;
	}

	// INTERPOLATION HELPER STRUCT
	// ----------------------------------------------------------------------------------------------------------------------
	
	// Constructor using Dates - *** Piecewise Constant = FALSE ***
	Interpolator::Interpolator( const AQLDate & asOfDate,
								const InterpolationEnum interpolationEnum,
								const StateVariableEnum stateVariableEnum,
								const std::vector<AQLDate> & xValues,
								const std::vector<double> & yValues,
								const DayCountEnum accrualDaycount,
								const std::string & curveFrequencyTenor,
							    const BusinessDayAdjustmentEnum fixingBusDayAdj,
							    const std::string & fixingCalendar,
								const AQLDate & joinDate,
								const size_t polynomialOrder )
		
	{
		const std::vector<double> xValuesAsDouble = convertCurveDatesToTerms( asOfDate, xValues );
		const double joinDateAsDouble = convertCurveDateToTerm( asOfDate, joinDate );

		// Private Constructor Helper
		init( asOfDate, interpolationEnum, stateVariableEnum, xValuesAsDouble, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder );
	}

	// Constructor with Dates Represented as Terms - *** Piecewise Constant = FALSE ***
	Interpolator::Interpolator( const AQLDate & asOfDate,
								const InterpolationEnum interpolationEnum,
								const StateVariableEnum stateVariableEnum,
								const std::vector<double> & xValues,
								const std::vector<double> & yValues,
								const DayCountEnum accrualDaycount,
								const std::string & curveFrequencyTenor,
							    const BusinessDayAdjustmentEnum fixingBusDayAdj,
							    const std::string & fixingCalendar,
								const double & joinDateAsDouble,
								const size_t polynomialOrder )
	{
		// Private Constructor Helper
		init( asOfDate, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar, joinDateAsDouble, polynomialOrder );
	}
	
	// Constructor for Legacy Interpolators & Backwards Compatibility - *** Piecewise Constant = TRUE ***
	// Allows isFwdInter = true, where curveCollection and curveIndex is required
	Interpolator::Interpolator( const AQLDate & asOfDate,
								const std::string & curveCollection,
								const std::string & curveIndex,
								const InterpolationEnum interpolationEnum,
								const StateVariableEnum stateVariableEnum,
								const std::shared_ptr<AQLInterpolationBase> & aqInterpolationPtr,
								const DayCountEnum accrualDaycount,
								const std::string & curveFrequencyTenor,
								const BusinessDayAdjustmentEnum fixingBusDayAdj,
								const std::string & fixingCalendar )
	{
		// Private Constructor Helper
		init( asOfDate, curveCollection, curveIndex, interpolationEnum, stateVariableEnum, aqInterpolationPtr, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar );
	}

	// Constructor for Legacy Interpolators & Backwards Compatibility - *** Piecewise Constant = TRUE ***
	// Does not allows isFwdInter = true, where curveCollection and curveIndex is required
	Interpolator::Interpolator( const AQLDate & asOfDate,
								const InterpolationEnum interpolationEnum,
								const StateVariableEnum stateVariableEnum,
								const std::shared_ptr<AQLInterpolationBase> & aqInterpolationPtr,
								const DayCountEnum accrualDaycount,
								const std::string & curveFrequencyTenor,
								const BusinessDayAdjustmentEnum fixingBusDayAdj,
								const std::string & fixingCalendar )
	{
		// Private Constructor Helper
		init( asOfDate, "", "", interpolationEnum, stateVariableEnum, aqInterpolationPtr, accrualDaycount, curveFrequencyTenor, fixingBusDayAdj, fixingCalendar );
	}

	double Interpolator::joinDateAsDouble( const AQLDate& asOfDate, const AQLDate& joinDate )
	{
		AQ_REQUIRE( asOfDate != AQLDate(), "Invalid Interpolator Parameters: 'AsOfDate' is missing")
		double joinDateAsDouble = 0.0;
		if ( joinDate != AQLDate() )
		{
			joinDateAsDouble = convertCurveDateToTerm( asOfDate, joinDate );
		}
		return joinDateAsDouble;
	}

	// Private Constructor Helper - *** Piecewise Constant = FALSE ***
	void Interpolator::init( const AQLDate & asOfDate,
							 const InterpolationEnum interpolationEnum,
							 const StateVariableEnum stateVariableEnum,
							 const std::vector<double> & xValues,
							 const std::vector<double> & yValues,
							 const DayCountEnum accrualDaycount,
							 const std::string & curveFrequencyTenor,
					         const BusinessDayAdjustmentEnum fixingBusDayAdj,
					         const std::string & fixingCalendar,
							 const double & joinDateAsDouble,
							 const size_t polynomialOrder )
	{
		asOfDate_						= asOfDate;
		curveCollection_				= "";			// Not required for this constructor
		curveIndex_						= "";			// Not required for this constructor

		interpolationEnum_				= interpolationEnum;
		stateVariableEnum_				= stateVariableEnum;
		xValues_						= xValues;
		yValues_						= yValues;
		accrualDaycount_				= accrualDaycount;
		curveFrequencyTenor_			= curveFrequencyTenor;
		fixingBusDayAdj_				= fixingBusDayAdj;
		fixingBusDayAdjString_			= toString( fixingBusDayAdj );
		fixingCalendar_					= fixingCalendar;
		joinDateAsDouble_				= joinDateAsDouble;
		polynomialOrder_				= polynomialOrder;
		
		// For Backward Compatibility
		assumePiecewiseConstant_		= false;
	
		AQ_REQUIRE( xValues_.size() == yValues_.size(),	"Interpolation x and y vectors must be the same size" )
		AQ_REQUIRE( xValues_.size() > 0,					"Interpolation x and y data is empty" )
	
		// Initialize the Polynomial Class Interpolator
		if( interpolationEnum_ == POLYNOMIAL_INTERPOLATION )
		{
			polynomialInterpolator_ = std::shared_ptr<PolynomialInterpolation>( new PolynomialInterpolation( xValues_, yValues_, polynomialOrder_ ) );
		}

		// *** IMPORTANT *** Calculate xVaules in the accrual daycount basis not the internal curve Act/365 daycount basis
		xValuesAccrualDaycountBasis_ = xValues;
		for( size_t i = 0; i < xValuesAccrualDaycountBasis_.size(); ++i )
		{
			xValuesAccrualDaycountBasis_[i] *= daycountConversionFactor( 0.0, xValues_[i], asOfDate_, accrualDaycount_, SIMPLE_COMPOUNDING );
		}

	}

	// Private Constructor Helper - *** Piecewise Constant = TRUE ***
	void Interpolator::init( const AQLDate & asOfDate,
							 const std::string & curveCollection,
							 const std::string & curveIndex,
							 const InterpolationEnum interpolationEnum,
							 const StateVariableEnum stateVariableEnum,
							 const std::shared_ptr<AQLInterpolationBase> & aqInterpolationPtr,
							 const DayCountEnum accrualDaycount,
					         const std::string & curveFrequencyTenor,
					         const BusinessDayAdjustmentEnum fixingBusDayAdj,
					         const std::string & fixingCalendar )
	{
		asOfDate_						= asOfDate;
		curveCollection_				= curveCollection;
		curveIndex_						= curveIndex;
		interpolationEnum_				= interpolationEnum;
		stateVariableEnum_				= stateVariableEnum;
		xValues_						= aqInterpolationPtr->interpolationData()->xValues_;
		yValues_						= aqInterpolationPtr->interpolationData()->xValues_;
		accrualDaycount_				= accrualDaycount;
		curveFrequencyTenor_			= curveFrequencyTenor;
		fixingBusDayAdj_				= fixingBusDayAdj;
		fixingBusDayAdjString_			= toString( fixingBusDayAdj );
		fixingCalendar_					= fixingCalendar;
		joinDateAsDouble_				= 0.0;	// not required for this constructor
		polynomialOrder_				= 0;	// not required for this constructor

		// For Backward Compatibility
		assumePiecewiseConstant_		= true;
		aqInterpolationPtr_				= aqInterpolationPtr;

		// *** IMPORTANT *** Calculate xVaules in the accrual daycount basis not the internal curve Act/365 daycount basis
		xValuesAccrualDaycountBasis_ = xValues_;
		for( size_t i = 0; i < xValuesAccrualDaycountBasis_.size(); ++i )
		{
			xValuesAccrualDaycountBasis_[i] *= daycountConversionFactor( 0.0, xValues_[i], asOfDate_, accrualDaycount_, SIMPLE_COMPOUNDING );
		}
	}

	// ----------------------------------------------------------------------------------------------------------------------

	CurveInterpolation::CurveInterpolation( const InterpolationPtr & interpolator,
											const ForwardAdjustmentsPtr & forwardAdjustments )
	{
		// Initialize Members
		interpolator_			= interpolator;
		forwardAdjustments_		= forwardAdjustments;
	}

	// Constructor with interpolation raw inputs as dates
	CurveInterpolation::CurveInterpolation( const AQLDate & asOfDate,
											const InterpolationEnum interpolationEnum,
											const StateVariableEnum stateVariableEnum,
											const std::vector<AQLDate> & xValues,
											const std::vector<double> & yValues,
											const DayCountEnum accrualDaycount,
											const std::string & curveFrequencyTenor,
											const BusinessDayAdjustmentEnum fixingBusDayAdj,
											const std::string & fixingCalendar,
											const AQLDate & joinDate,
											const size_t polynomialOrder,
											const StandardStringMatrix & forwardAdjustmentsStringMatrix )
	{
		// Initialize Members
		interpolator_			= std::shared_ptr<Interpolator>( new Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   xValues,
																				   yValues,
																				   accrualDaycount,
																				   curveFrequencyTenor,
																				   fixingBusDayAdj,
																				   fixingCalendar,
																				   joinDate,
																				   polynomialOrder ) );

		forwardAdjustments_		= std::shared_ptr<ForwardAdjustments>( new ForwardAdjustments( asOfDate, forwardAdjustmentsStringMatrix ) );
	}
		
	// Constructor with interpolation raw inputs as terms
	CurveInterpolation::CurveInterpolation( const AQLDate & asOfDate,
											const InterpolationEnum interpolationEnum,
											const StateVariableEnum stateVariableEnum,
											const std::vector<double> & xValues,
											const std::vector<double> & yValues,
											const DayCountEnum accrualDaycount,
											const std::string & curveFrequencyTenor,
											const BusinessDayAdjustmentEnum fixingBusDayAdj,
											const std::string & fixingCalendar,
											const double & joinDateAsDouble,
											const size_t polynomialOrder,
											const StandardStringMatrix & forwardAdjustmentsStringMatrix )
	{
		// Initialize Members
		interpolator_			= std::shared_ptr<Interpolator>( new Interpolator( asOfDate,
																				   interpolationEnum,
																				   stateVariableEnum,
																				   xValues,
																				   yValues,
																				   accrualDaycount,
																				   curveFrequencyTenor,
																				   fixingBusDayAdj,
																				   fixingCalendar,
																				   joinDateAsDouble,
																				   polynomialOrder ) );

		forwardAdjustments_		= std::shared_ptr<ForwardAdjustments>( new ForwardAdjustments( asOfDate, forwardAdjustmentsStringMatrix ) );
		
	}

	// *** Legacy *** Constructor with interpolation raw inputs as dates
	// Allows isFwdInter = true, where CurveCollection and CurveIndex are required
	CurveInterpolation::CurveInterpolation( const AQLDate & asOfDate,
											const std::string & curveCollection,
											const std::string & curveIndex,
											const InterpolationEnum interpolationEnum,
											const StateVariableEnum stateVariableEnum,
											const std::shared_ptr<AQLInterpolationBase> & aqInterpolationObject,
											const DayCountEnum accrualDaycount,
											const std::string & curveFrequencyTenor,
											const BusinessDayAdjustmentEnum fixingBusDayAdj,
											const std::string & fixingCalendar,
											const StandardStringMatrix & forwardAdjustmentsStringMatrix )
	{
		// Initialize Members
		interpolator_ = std::shared_ptr<Interpolator>( new Interpolator( asOfDate,
																		 curveCollection,
																		 curveIndex,
																		 interpolationEnum,
																		 stateVariableEnum,
																		 aqInterpolationObject,
																		 accrualDaycount,
																		 curveFrequencyTenor,
																		 fixingBusDayAdj,
																		 fixingCalendar ) );

		forwardAdjustments_		= std::shared_ptr<ForwardAdjustments>( new ForwardAdjustments( asOfDate, forwardAdjustmentsStringMatrix ) );
	}

	// *** Legacy *** Constructor with interpolation raw inputs as dates
	// Allows isFwdInter = true, where CurveCollection and CurveIndex are required
	CurveInterpolation::CurveInterpolation( const AQLDate & asOfDate,
											const InterpolationEnum interpolationEnum,
											const StateVariableEnum stateVariableEnum,
											const std::shared_ptr<AQLInterpolationBase> & aqInterpolationObject,
											const DayCountEnum accrualDaycount,
											const std::string & curveFrequencyTenor,
											const BusinessDayAdjustmentEnum fixingBusDayAdj,
											const std::string & fixingCalendar,
											const StandardStringMatrix & forwardAdjustmentsStringMatrix )
	{
		// Initialize Members
		interpolator_ = std::shared_ptr<Interpolator>( new Interpolator( asOfDate,
																		 "",
																		 "",
																		 interpolationEnum,
																		 stateVariableEnum,
																		 aqInterpolationObject,
																		 accrualDaycount,
																		 curveFrequencyTenor,
																		 fixingBusDayAdj,
																		 fixingCalendar ) );

		forwardAdjustments_		= std::shared_ptr<ForwardAdjustments>( new ForwardAdjustments( asOfDate, forwardAdjustmentsStringMatrix ) );
	}

	// Copy Constructor
	CurveInterpolation::CurveInterpolation(const CurveInterpolation & rhs)
		: interpolator_(rhs.interpolator_), forwardAdjustments_(rhs.forwardAdjustments_ )
	{}

	// Assignment Operator
	CurveInterpolation & CurveInterpolation::operator=(const CurveInterpolation & rhs)
	{
		if (this != &rhs)
		{
			interpolator_ = rhs.interpolator_;
			forwardAdjustments_ = rhs.forwardAdjustments_;
		}

		return *this;
	}

	// Clone
	std::shared_ptr<CurveInterpolation> CurveInterpolation::clone() const
	{
		return std::make_shared<CurveInterpolation>(CurveInterpolation(*this));
	}

	// Methods using Terms (Calibration Methods Provide Dates in Terms or Double Format)
	// *** Important Note: For backwards compatibility we support and assume piecewiseConstant rates between cashflows ***
	// -----------------------------------------------------------------------------------------------------------------------

	double CurveInterpolation::discountFactor(const double & paymentDateAsTerms, const CompoundingFrequencyEnum compoundFrequency ) const
	{
		const std::vector<double> discountFactorVector =  discountFactors( std::vector<double>( 1, paymentDateAsTerms ), compoundFrequency );
		AQ_REQUIRE( discountFactorVector.size() == 1, "Invalid Discount Factor Result")
		return discountFactorVector[0];
	}

	std::vector<double> CurveInterpolation::discountFactors(const std::vector<double> & paymentDatesAsTerms, const CompoundingFrequencyEnum compoundFrequency ) const
	{
		auto nTerms = paymentDatesAsTerms.size();
		std::vector<double> discountFactors( nTerms, 1.0 );

		if ( interpolator_->assumePiecewiseConstant_ )
		{
			// Case 1: For Backwards Compatibility: Assume piecewise constant Rates between cashflows. This is undesirable.
			switch (interpolator_->stateVariableEnum_)
			{
				case STATE_VARIABLE_DF:
				case STATE_VARIABLE_ZERO_RATE:
				case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
				case STATE_VARIABLE_LOG_DF:
				{
					for (std::size_t i = 0; i < nTerms; i++)
					{
						// Boundary Condition: Forwards at time zero (or before ) are zero i.e. Discount Factors are one
						if( AQ_IS_LESS_THAN_OR_EQUAL_TO_ZERO( paymentDatesAsTerms[i] ) )
						{
							discountFactors[i] = 1.0;
						}
						else
						{
							discountFactors[i] = getInterpolatedDiscountfactor( *interpolator_->aqInterpolationPtr_,
																				paymentDatesAsTerms[i],
																				interpolator_->stateVariableEnum_,
																				interpolator_->asOfDate_,
																				interpolator_->accrualDaycount_,
																				compoundFrequency );
						}
					}
					break;
				}
				default:
				{
					AQ_THROW("Invalid State Variable: '" + toString(interpolator_->stateVariableEnum_) + "'. When using 'AssumePiecewiseConstant' = TRUE we must use DF, LogDF, ZeroRate or ZeroRateTimesTime.")
				}
			}
		}
		else
		{
			// Case 2: Interpolate correctly *** WITHOUT *** assuming piecewise constant rates.
			switch (interpolator_->stateVariableEnum_)
			{
				case STATE_VARIABLE_DF:
				{
					// *** INTERPOLATE ***
					for( size_t i = 0; i < paymentDatesAsTerms.size(); ++i )
					{ 
						// Boundary Condition: Spot Discount Factors Equal 1.0
						if ( AQ_IS_EQUAL_ZERO( paymentDatesAsTerms[i] ) )
						{
							discountFactors[i] = 1.0;
							continue;
						}
						
						discountFactors[i] = interpolate( paymentDatesAsTerms[i] );
					}
					break;
				}
				case STATE_VARIABLE_ZERO_RATE:
				{
					// *** INTERPOLATE ***
					for( size_t i = 0; i < paymentDatesAsTerms.size(); ++i )
					{ 
						// Boundary Condition: Spot Discount Factors Equal 1.0
						if ( AQ_IS_EQUAL_ZERO( paymentDatesAsTerms[i] ) )
						{
							discountFactors[i] = 1.0;
							continue;
						}

						switch( compoundFrequency )
						{
							case SIMPLE_COMPOUNDING:
							{
								discountFactors[i] = 1.0 / ( 1.0 + interpolate( paymentDatesAsTerms[i] ) * paymentDatesAsTerms[i] );
								break;
							}
							case CONTINUOUS_COMPOUNDING:
							{
								discountFactors[i] = exp( - interpolate( paymentDatesAsTerms[i] ) * paymentDatesAsTerms[i] );
								break;
							}
							default:
							{
								AQ_THROW( "Invalid Compound Frequency: Only Simple and Continuous Compounding Supported" )
							}
						}
					}
					break;
				}
				case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
				{
					// *** INTERPOLATE ***
					for( size_t i = 0; i < paymentDatesAsTerms.size(); ++i )
					{ 
						// Boundary Condition: Spot Discount Factors Equal 1.0
						if ( AQ_IS_EQUAL_ZERO( paymentDatesAsTerms[i] ) )
						{
							discountFactors[i] = 1.0;
							continue;
						}

						switch( compoundFrequency )
						{
							case SIMPLE_COMPOUNDING:
							{
								discountFactors[i] = 1.0 / ( 1.0 + interpolate( paymentDatesAsTerms[i] ) );
								break;
							}
							case CONTINUOUS_COMPOUNDING:
							{
								discountFactors[i] = exp( - interpolate( paymentDatesAsTerms[i] ) );
								break;
							}
							default:
							{
								AQ_THROW( "Invalid Compound Frequency: Only Simple and Continuous Compounding Supported" )
							}
						}
					}
					break;
				}
				case STATE_VARIABLE_LOG_DF:
				{
					// *** INTERPOLATE ***
					for( size_t i = 0; i < paymentDatesAsTerms.size(); ++i )
					{ 
						// Boundary Condition: Spot Discount Factors Equal 1.0
						if ( AQ_IS_EQUAL_ZERO( paymentDatesAsTerms[i] ) )
						{
							discountFactors[i] = 1.0;
							continue;
						}
						discountFactors[i] = std::exp( interpolate( paymentDatesAsTerms[i] ) );
					}
					break;
				}
				case STATE_VARIABLE_FORWARD_RATE:
				{
					// *** INTEGRATE ***
					for (size_t i = 0; i < paymentDatesAsTerms.size(); ++i)
					{
						// Boundary Condition: Spot Discount Factors Equal 1.0
						if ( AQ_IS_EQUAL_ZERO( paymentDatesAsTerms[i] ) )
						{
							discountFactors[i] = 1.0;
							continue;
						}
						
						switch( compoundFrequency )
						{
							case SIMPLE_COMPOUNDING:
							{
								discountFactors[i] = 1.0 / ( 1.0 + integrate( 0, paymentDatesAsTerms[i] ) );
								break;
							}
							case CONTINUOUS_COMPOUNDING:
							{
								discountFactors[i] = exp( - integrate( 0, paymentDatesAsTerms[i] ) );
								break;
							}
							default:
							{
								AQ_THROW( "Invalid Compound Frequency: Only Simple and Continuous Compounding Supported" )
							}
						}
					}
					break;
				}
				default:
					AQ_THROW("Invalid State Variable: " + toString(interpolator_->stateVariableEnum_) )
			}
		}

		// Apply forward adjustments when available
		if ( forwardAdjustments_ != nullptr )
		{
			discountFactors = forwardAdjustments_->adjustDiscountFactors(paymentDatesAsTerms, discountFactors);
		}
		return discountFactors;
	}

	double CurveInterpolation::forwardRate(const double & fixingDateAsTerm, const bool useForwardInterpolation, const CompoundingFrequencyEnum compoundFrequency ) const
	{
		const std::vector<double> forwardRateVector =  forwardRates( std::vector<double>( 1, fixingDateAsTerm ), useForwardInterpolation, compoundFrequency );
		AQ_REQUIRE( forwardRateVector.size() == 1, "Invalid Forward Rate Result")
		return forwardRateVector[0];
	}

	std::vector<double> CurveInterpolation::forwardRates(const std::vector<double> & fixingDatesAsTerms, const bool useForwardInterpolation, const CompoundingFrequencyEnum compoundFrequency ) const
	{
		auto nTerms = fixingDatesAsTerms.size();
		std::vector<double> forwardRates( nTerms, 0.0 );
		
		if ( interpolator_->assumePiecewiseConstant_ )
		{
			// Case 1: For Backwards Compatibility: Assume piecewise constant Rates between cashflows. This is undesirable.
			switch (interpolator_->stateVariableEnum_)
			{
				case STATE_VARIABLE_DF:
				case STATE_VARIABLE_ZERO_RATE:
				case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
				case STATE_VARIABLE_LOG_DF:
				{
					// *** Legacy Scenario when using State Variable DF with useForwardInterpolation = TRUE ***
					if( useForwardInterpolation && interpolator_->stateVariableEnum_ == STATE_VARIABLE_DF )
					{
						AQ_REQUIRE( interpolator_->curveCollection_ != "", "Invalid Forward Rates: Curve Collection is missing and required when using forward interpolation i.e. isFwdInter = true" );
						AQ_REQUIRE( interpolator_->curveIndex_	  != "", "Invalid Forward Rates: Curve Index is missing and required when using forward interpolation i.e. isFwdInter = true" );
						
						const DateVector fixingDates = convertCurveTermsToDates( interpolator_->curveCollection_, fixingDatesAsTerms );
						
						forwardRates = getCurveForwardRates( fixingDates,
															 interpolator_->curveCollection_,
															 interpolator_->curveIndex_,
															 interpolator_->fixingBusDayAdj_,
															 "", // calendar
															 toBooleanEnumFromBool(useForwardInterpolation) );
						
						AQ_REQUIRE( forwardRates.size() ==  nTerms, "Invalid Forward Rates: Inconsistent number of forward rates." )
					}
					else
					{
						for (std::size_t i = 0; i < nTerms; i++)
						{
							// Boundary Condition: Forwards at time zero (or before ) are zero i.e. Discount Factors are one
							if( AQ_IS_LESS_THAN_ZERO( fixingDatesAsTerms[i] ) )
							{
								forwardRates[i] = 0.0;
							}
							else
							{
								const double startTerm	= fixingDatesAsTerms[i];
								const double endTerm	= fixingEndTerm( fixingDatesAsTerms[i],
																		 interpolator_->curveFrequencyTenor_,
																		 interpolator_->fixingBusDayAdjString_,
																		 interpolator_->fixingCalendar_ );
							
								forwardRates[i] = getForwardRate( startTerm,
																  endTerm,
																  *interpolator_->aqInterpolationPtr_,
																  interpolator_->stateVariableEnum_,
																  interpolator_->asOfDate_,
																  interpolator_->accrualDaycount_,
																  compoundFrequency );
							}
						}
					}
					break;
				}
				case STATE_VARIABLE_FORWARD_RATE:
				{
					for (std::size_t i = 0; i < nTerms; i++)
					{
						// Boundary Condition: Forwards in the past are valued as zero for backwards compatibility
						if( AQ_IS_LESS_THAN_ZERO( fixingDatesAsTerms[i] ) )
						{
							forwardRates[i] = 0.0;
						}
						else
						{
							forwardRates[i] = interpolator_->aqInterpolationPtr_->value( fixingDatesAsTerms[i] );
						}
					}
					break;
				}
				default:
				{
					AQ_THROW("Invalid State Variable: '" + toString(interpolator_->stateVariableEnum_) + "'. When using 'AssumePiecewiseConstant' = TRUE we must use DF, LogDF, ZeroRate or ZeroRateTimesTime.")
				}
			}
		}
		else
		{
			// Case 2: Interpolate correctly *** WITHOUT *** assuming piecewise constant rates.
			switch (interpolator_->stateVariableEnum_)
			{
				case STATE_VARIABLE_DF:
				{
					// Forward Rate = -dlnDF/dt = -1/DF * dDF/dt

					// Discount Factors on End Dates
					const std::vector<double> fixingEndDatesAsTerms = fixingEndTerms( fixingDatesAsTerms, interpolator_->curveFrequencyTenor_, interpolator_->fixingBusDayAdjString_, interpolator_->fixingCalendar_ );
					const std::vector<double> endDFs = interpolate(fixingEndDatesAsTerms);

					// Perform Discrete Differentiation over the fixingStart- to fixingEndDate Range
					forwardRates = differentiate(fixingDatesAsTerms, fixingEndDatesAsTerms);
					for (std::size_t i = 0; i < nTerms; i++)
					{
						forwardRates[i] *= -1.0 / endDFs[i];
					}
					break;
				}
				case STATE_VARIABLE_ZERO_RATE:
				{
					for (std::size_t i = 0; i < nTerms; i++)
					{
						if( AQ_IS_LESS_THAN_ZERO( fixingDatesAsTerms[i] ) )
						{
							forwardRates[i] = 0.0;
						}
						else
						{
							const double zeroRate = interpolate(fixingDatesAsTerms[i]);
							forwardRates[i] = differentiate(fixingDatesAsTerms[i]) * fixingDatesAsTerms[i] + zeroRate;
						}
					}
					break;
				}
				case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
				{
					for( size_t i = 0; i < fixingDatesAsTerms.size(); ++i )
					{
						if( AQ_IS_LESS_THAN_ZERO( fixingDatesAsTerms[i] ) )
						{
							forwardRates[i] = 0.0;
						}
						else
						{
							forwardRates[i] = differentiate( fixingDatesAsTerms[i] );
						}
					}
					break;
				}
				case STATE_VARIABLE_LOG_DF:
				{
					// TODO: This doesn't look correct, compare with above code for STATE_VARIABLE_DF
					forwardRates = differentiate(fixingDatesAsTerms);
					for (auto & rate : forwardRates)
					{ 
						rate = -rate;
					}
					break;
				}
				case STATE_VARIABLE_FORWARD_RATE:
				{
					for( size_t i = 0; i < fixingDatesAsTerms.size(); ++i )
					{
						if( AQ_IS_LESS_THAN_ZERO( fixingDatesAsTerms[i] ) )
						{
							forwardRates[i] = 0.0;
						}
						else
						{
							forwardRates[i] = interpolate( fixingDatesAsTerms[i] );
						}
					}
					break;
				}
				default:
				{
					AQ_THROW("Invalid State Variable: '" + toString(interpolator_->stateVariableEnum_) + "' is not valid")
				}
			}
		}

		// Apply forward adjustments when available
		if ( forwardAdjustments_ != nullptr )
		{
			forwardRates = forwardAdjustments_->adjustForwardRates(fixingDatesAsTerms, forwardRates);
		}
		return forwardRates;
	}

	// Method to calculate the OIS or ARR effective compound rate(s)
	std::vector<double> CurveInterpolation::compoundRates(const std::vector<double> & fixingDatesAsTerms, const OISCompoundingEnum & compoundingEnum ) const
	{
		AQ_THROW("Method 'compoundRates' Not Implemented")
	}

	// Methods using Dates (Server Users Provide Dates)
	// These methods will call the above methods taking dates in Date Format
	// *** Important Note: For backwards compatibility we support and assume piecewiseConstant rates between cashflows ***
	// -------------------------------------------------------------------------------------------------------------------------------------

	double CurveInterpolation::discountFactor( const AQLDate & paymentDate ) const
	{
		const std::vector<double> discountFactorVector =  discountFactors( std::vector<AQLDate>( 1, paymentDate ) );
		AQ_REQUIRE( discountFactorVector.size() == 1, "Invalid Discount Factor Result")
		return discountFactorVector[0];
	}
	
	
	std::vector<double> CurveInterpolation::discountFactors( const std::vector<AQLDate> & paymentDates ) const
	{
		std::vector<double> paymentDatesAsTerms;
		for (auto thisPaymentDate : paymentDates)
		{
			paymentDatesAsTerms.emplace_back( convertCurveDateToTerm(interpolator_->asOfDate_, thisPaymentDate ) );
		}
		return discountFactors(paymentDatesAsTerms);
	}

	double CurveInterpolation::forwardRate( const AQLDate & fixingDate, const bool & useForwardInterpolation ) const
	{
		const std::vector<double> forwardRateVector =  forwardRates( std::vector<AQLDate>( 1, fixingDate ), useForwardInterpolation );
		AQ_REQUIRE( forwardRateVector.size() == 1, "Invalid Forward Rate Result")
		return forwardRateVector[0];
	}

	std::vector<double> CurveInterpolation::forwardRates( const std::vector<AQLDate> & fixingDates, const bool useForwardInterpolation ) const
	{
		std::vector<double> fixingDatesAsTerms;
		for (auto thisFixingDate : fixingDates)
		{
			fixingDatesAsTerms.emplace_back( convertCurveDateToTerm( interpolator_->asOfDate_, thisFixingDate ) );
		}
		return forwardRates( fixingDatesAsTerms, useForwardInterpolation );
	}

	// Method to calculate the OIS or ARR effective compound rate(s)
	std::vector<double> CurveInterpolation::compoundRates( const std::vector<AQLDate> & fixingDates, const OISCompoundingEnum & compoundingEnum ) const
	{
		std::vector<double> fixingDatesAsTerms;
		for (auto thisFixingDate : fixingDates)
		{
			fixingDatesAsTerms.emplace_back( convertCurveDateToTerm( interpolator_->asOfDate_, thisFixingDate ) );
		}
		return compoundRates( fixingDatesAsTerms, compoundingEnum );
	}

	// Helper functions to do interpolation from state variables
	double CurveInterpolation::interpolate( const double & x ) const
	{
		double result = 0.0;

		switch( interpolator_->interpolationEnum_ )
		{
			case POLYNOMIAL_INTERPOLATION:
			{
				AQ_REQUIRE( interpolator_->polynomialInterpolator_ != nullptr, "Interpolation Error: Missing Polynomial Interpolation Data" )
				result = interpolator_->polynomialInterpolator_->interpolate( x );
				break;
			}
			default:
			{
				// Legacy Interpolation Methods from CurveUtilities.cpp
				result = etrading::interpolate( interpolator_->xValues_,
												interpolator_->yValues_,
												x,
												interpolator_->interpolationEnum_,
												interpolator_->joinDateAsDouble_ );
				break;
			}
		}
		
		return result;
	}

	// Method to interpolate using vectorised interpolation method to prevent the reconstuction of the interpolation state for each interpolated value, for better performance
	std::vector<double> CurveInterpolation::interpolate( const std::vector<double> & xVector ) const
	{
		std::vector<double> results( xVector.size() );

		switch( interpolator_->interpolationEnum_ )
		{
			case POLYNOMIAL_INTERPOLATION:
			{
				AQ_REQUIRE( interpolator_->polynomialInterpolator_ != nullptr, "Interpolation Error: Missing Polynomial Interpolation Data" )
				results = interpolator_->polynomialInterpolator_->interpolate( xVector );
				break;
			}
			default:
			{
				// Legacy Interpolation Methods from CurveUtilities.cpp
				results = etrading::interpolate( interpolator_->xValues_,
									             interpolator_->yValues_,
									             xVector,
									             interpolator_->interpolationEnum_,
									             interpolator_->joinDateAsDouble_ );
				break;
			}
		}
		
		return results;
	}

	// Helper functions to do interpolation from state variables
	double CurveInterpolation::integrate( const double & lowerBound, const double & upperBound ) const
	{
		double result = 0.0;

		switch( interpolator_->interpolationEnum_ )
		{
			case POLYNOMIAL_INTERPOLATION:
			{
				AQ_REQUIRE( interpolator_->polynomialInterpolator_ != nullptr, "Interpolation Error: Missing Polynomial Data" )
				result = interpolator_->polynomialInterpolator_->integrate( lowerBound, upperBound );
				break;
			}
			default:
			{
				// Legacy Interpolation Methods from CurveUtilities.cpp
				result = etrading::integrate( interpolator_->xValues_,
									          interpolator_->yValues_,
									          lowerBound,
									          upperBound,
									          interpolator_->interpolationEnum_,
											  interpolator_->asOfDate_,
											  interpolator_->accrualDaycount_,
											  SIMPLE_COMPOUNDING,
									          interpolator_->joinDateAsDouble_ );
			}
		}
		
		return result;
	}

	// Method to interpolate using vectorised integration method to prevent the reconstuction of the interpolation state for each interpolated value, for better performance
	std::vector<double> CurveInterpolation::integrate( const std::vector<double> & lowerBounds, const std::vector<double> & upperBounds ) const
	{
		AQ_REQUIRE( lowerBounds.size() == upperBounds.size(), "Invalid Integration Bounds: The number of intergrand lowerbounds and upperbounds must match" )
		std::vector<double> results( lowerBounds.size() );

		switch( interpolator_->interpolationEnum_ )
		{
			case POLYNOMIAL_INTERPOLATION:
			{
				AQ_REQUIRE( interpolator_->polynomialInterpolator_ != nullptr, "Interpolation Error: Missing Polynomial Data" )
				results = interpolator_->polynomialInterpolator_->integrate( lowerBounds, upperBounds );
				break;
			}
			default:
			{
				// Legacy Interpolation Methods from CurveUtilities.cpp
				results = etrading::integrate( interpolator_->xValues_,
											   interpolator_->yValues_,
											   lowerBounds,
											   upperBounds,
											   interpolator_->interpolationEnum_,
											   interpolator_->asOfDate_,
											   interpolator_->accrualDaycount_,
											   SIMPLE_COMPOUNDING,
											   interpolator_->joinDateAsDouble_ );
			}
		}

		return results;
	}

	// Helper functions to perform instantanesous differentiation
	double CurveInterpolation::differentiate(const double & x) const
	{
		double result = 0.0;
		switch (interpolator_->interpolationEnum_)
		{
			case POLYNOMIAL_INTERPOLATION:
			{
				AQ_REQUIRE(interpolator_->polynomialInterpolator_ != nullptr, "Interpolation Error: Missing Polynomial Data")
				result = interpolator_->polynomialInterpolator_->differentiate(x);
				break;
			}
			default:
			{
				// Legacy Interpolation Methods from CurveUtilities.cpp
				result = etrading::differentiate( interpolator_->xValues_,
												  interpolator_->yValues_,
												  x,
												  interpolator_->interpolationEnum_,
												  interpolator_->joinDateAsDouble_ );
			}
		}
		return result;
	}

	// Method to interpolate using vectorised integration method to prevent the reconstuction of the interpolation state for each interpolated value, for better performance
	std::vector<double> CurveInterpolation::differentiate( const std::vector<double> & xVector ) const
	{
		std::vector<double> results( xVector.size(), 0.0 );
		switch (interpolator_->interpolationEnum_)
		{
			case POLYNOMIAL_INTERPOLATION:
			{
				AQ_REQUIRE(interpolator_->polynomialInterpolator_ != nullptr, "Interpolation Error: Missing Polynomial Data")
				results = interpolator_->polynomialInterpolator_->differentiate( xVector );
				break;
			}
			default:
			{
				// Legacy Interpolation Methods from CurveUtilities.cpp
				results = etrading::differentiate( interpolator_->xValues_,
												   interpolator_->yValues_,
												   xVector,
												   interpolator_->interpolationEnum_,
												   interpolator_->joinDateAsDouble_ );
			}
		}
		return results;
	}
	
	// Helper functions to perform discrete differentiation over a range of x values
	double CurveInterpolation::differentiate( const double & fromXPoint, const double & toXPoint ) const
	{
		double result = 0.0;
		switch (interpolator_->interpolationEnum_)
		{
			case POLYNOMIAL_INTERPOLATION:
			{
				AQ_REQUIRE(interpolator_->polynomialInterpolator_ != nullptr, "Interpolation Error: Missing Polynomial Data")
				result = interpolator_->polynomialInterpolator_->differentiate(fromXPoint, toXPoint);
				break;
			}
			default:
			{
				// Legacy Interpolation Methods from CurveUtilities.cpp
				// Use xValues here, which are in curve Act/365 basis, we manage the accrual daycount basis internally
				result = etrading::differentiate( interpolator_->xValues_,
												  interpolator_->yValues_,
												  fromXPoint,
												  toXPoint,
												  interpolator_->interpolationEnum_,
												  interpolator_->asOfDate_,
												  interpolator_->accrualDaycount_,
												  SIMPLE_COMPOUNDING,
												  interpolator_->joinDateAsDouble_ );
			}
		}
		return result;
	}
	
	// Helper functions to perform discrete differentiation over a range of x values
	std::vector<double> CurveInterpolation::differentiate( const std::vector<double> & fromXPoints, const std::vector<double> & toXPoints ) const
	{
		std::vector<double> results( fromXPoints.size(), 0.0 );
		switch (interpolator_->interpolationEnum_)
		{
			case POLYNOMIAL_INTERPOLATION:
			{
				AQ_REQUIRE(interpolator_->polynomialInterpolator_ != nullptr, "Interpolation Error: Missing Polynomial Data")
				
				// Native Results will be in the curve Act365 daycount basis
				results = interpolator_->polynomialInterpolator_->differentiate( fromXPoints, toXPoints );
				
				// Convert results from curve daycount Act365 to the accrual daycount
				for( size_t i = 0; i < results.size(); ++i )
				{
					results[i] *= daycountConversionFactor( fromXPoints[i], toXPoints[i], interpolator_->asOfDate_, interpolator_->accrualDaycount_, SIMPLE_COMPOUNDING );
				}
				break;
			}
			default:
			{
				// Legacy Interpolation Methods from CurveUtilities.cpp
				// Use xValues here, which are in curve Act/365 basis, we manage the accrual daycount basis internally
				results = etrading::differentiate( interpolator_->xValues_,
												   interpolator_->yValues_,
												   fromXPoints,
												   toXPoints,
												   interpolator_->interpolationEnum_,
												   interpolator_->asOfDate_,
												   interpolator_->accrualDaycount_,
												   SIMPLE_COMPOUNDING,
												   interpolator_->joinDateAsDouble_ );
			}
		}
		return results;
	}

	// Method to calculate the fixing end date
	double CurveInterpolation::fixingEndTerm( const double & fixingStartTerm,
											  const std::string & curveFrequencyTenor,
											  const std::string & businessDayAdjustment,
											  const std::string & calendar ) const
	{
		// Imply the toDate(s) using the fromDate(s) and curve frequency e.g. todate = fromDate + 3M 
		const AQLDate fixingStartDate = convertCurveTermToDate( interpolator_->asOfDate_, fixingStartTerm );
		const AQLDate fixingEndDate = AQLDateSchedule::getDate( fixingStartDate, curveFrequencyTenor.c_str(), businessDayAdjustment.c_str(), calendar.c_str() );
		const double fixingEndTerm = convertCurveDateToTerm( interpolator_->asOfDate_, fixingEndDate );
		return fixingEndTerm;
	}

	// Method to calculate the fixing end dates as a vector of doubles relatative to the asOfDate
	std::vector<double> CurveInterpolation::fixingEndTerms( const std::vector<double> & fixingStartTerms,
															const std::string & curveFrequencyTenor,
															const std::string & businessDayAdjustment,
															const std::string & calendar ) const
	{
		std::vector<double> fixingEndDatesAsTerms( fixingStartTerms.size(), 0.0 );
		for( size_t i = 0; i < fixingEndDatesAsTerms.size(); ++i )
		{
			fixingEndDatesAsTerms[i] = fixingEndTerm( fixingStartTerms[i], interpolator_->curveFrequencyTenor_, interpolator_->fixingBusDayAdjString_, interpolator_->fixingCalendar_ );
		}
		return fixingEndDatesAsTerms;
	}

}