// DiscountFactorResults.cpp

/*
 * @brief			Discount Factor Results Class
 * @Created:		15th July 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "DiscountFactorResults.h"
#include "ExceptionMacros.h"
#include "CurveValidation.h"			        // Convert Dates to Terms and vice versa
#include "LAMathInterpolationUtilities.h"		// Get Interpolation Values
#include "CurveUtilities.h"				        // DateFromTenor & accrualPeriod methods
#include "LACurvePricingObject.h"	            // Methods to get the curve daycount conventions
#include "AQLEnumConversion.h"		            // Methods to convert enum values to legacy enums

// Interpolation Methods
#include "AQLMonotoneConvexInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLParabolicInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include "AQLConstrainedSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLLinearInterpolation.h"

#include <cmath>	// for std::isnan

namespace etrading
{

    // =============================== DISCOUNT FACTOR RESULTS CLASS ==================================================================

     // Copy Constructor
    DiscountFactorResults::DiscountFactorResults( const DiscountFactorResults& rhs ) 
        : curveTenorEnum_(rhs.curveTenorEnum_),
          curveTenor_(rhs.curveTenor_ ),
          asOfDate_(rhs.asOfDate_),
	      spotDate_( rhs.spotDate_ ),
		  joinDate_(rhs.joinDate_),
		  joinDateAsDouble_(rhs.joinDateAsDouble_),
		  curveCollection_(rhs.curveCollection_),
	      curveIndex_(rhs.curveIndex_),
		  daycount_(rhs.daycount_),
		  fixingBusinessDayAdj_(rhs.fixingBusinessDayAdj_),
		  fixingCalendar_(rhs.fixingCalendar_),
		  baseInterpolationMethod_(rhs.baseInterpolationMethod_),
          interpolationEnum_(rhs.interpolationEnum_),
          paymentDatesInTermFormat_(rhs.paymentDatesInTermFormat_),
          discountFactors_(rhs.discountFactors_),
		  forwardAdjustments_( rhs.forwardAdjustments_)
	{
	}

	// Copy and Override Constructor
	DiscountFactorResults::DiscountFactorResults( const DiscountFactorResults& rhs, const VectorDate& paymentDates, const VectorDouble& discountFactors, const AQLDate & joinDate )
		:	curveTenorEnum_( rhs.curveTenorEnum_ ),
			curveTenor_( rhs.curveTenor_ ),
			asOfDate_( rhs.asOfDate_ ),
			spotDate_( rhs.spotDate_ ),
		    curveCollection_( rhs.curveCollection_ ),
	        curveIndex_( rhs.curveIndex_ ),
			daycount_( rhs.daycount_),
			fixingBusinessDayAdj_( rhs.fixingBusinessDayAdj_ ),
			fixingCalendar_( rhs.fixingCalendar_ ),
			baseInterpolationMethod_(rhs.baseInterpolationMethod_ ),
			interpolationEnum_( rhs.interpolationEnum_ ),
			forwardAdjustments_( rhs.forwardAdjustments_ )
	{
		paymentDatesInTermFormat_	= etrading::convertCurveDatesToTerms( rhs.asOfDate_, paymentDates );
		discountFactors_			= discountFactors;
		joinDate_					= joinDate;
		joinDateAsDouble_			= joinDate == AQLDate() ? 0.0 : etrading::convertCurveDateToTerm( spotDate_, joinDate );

		initializeInterpolator();
	}

	// Clone
	std::shared_ptr<DiscountFactorResults> DiscountFactorResults::clone() const
	{
		return std::make_shared<DiscountFactorResults>( DiscountFactorResults( *this ) );
	}

    // Assignment Operator
    DiscountFactorResults & DiscountFactorResults::operator=( const DiscountFactorResults & rhs )
    {
		// For Performance 
		if ( &rhs == this )
		{
			return *this;
		}

        // For Exception Safety
            
        // 1. Make a temp copy
        DiscountFactorResults temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( curveTenorEnum_,             temp.curveTenorEnum_ );
        std::swap( curveTenor_,                 temp.curveTenor_);
        std::swap( asOfDate_,                   temp.asOfDate_ );
		std::swap( spotDate_,					temp.spotDate_ );
		std::swap( curveCollection_,			temp.curveCollection_ );
	    std::swap( curveIndex_,					temp.curveIndex_ );
        std::swap( interpolationEnum_,          temp.interpolationEnum_ );
        std::swap( paymentDatesInTermFormat_,   temp.paymentDatesInTermFormat_ );
        std::swap( discountFactors_,            temp.discountFactors_ );
        std::swap( daycount_,					temp.daycount_ );
		std::swap( joinDate_,					temp.joinDate_ );
		std::swap( joinDateAsDouble_,			temp.joinDateAsDouble_ );
        std::swap( fixingBusinessDayAdj_,	    temp.fixingBusinessDayAdj_ );
        std::swap( fixingCalendar_,			    temp.fixingCalendar_ );
		std::swap( forwardAdjustments_,			temp.forwardAdjustments_ );

        return *this;
    }


    // Object Pool Constructor - Object Pool Stores Dates as Doubles representing Year Fractions
	// Note: Spot Date should be Curve Swap Calibration Instrument Spot Date - needed for the linear spline join date calculation
    DiscountFactorResults::DiscountFactorResults( const CurveTenorEnum & curveTenor,
                                                  const AQLDate & asOfDate,
												  const AQLDate & spotDate,
												  const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
												  const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
                                                  const InterpolationEnum & interpolationEnum,
                                                  const VectorDouble & paymentDatesInTermFormat,
                                                  const VectorDouble & discountFactors,
												  const DayCountEnum & daycount,
                                                  const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                  const StandardString & fixingCalendar,
                                                  const AQLDate & joinDate,
												  const StandardStringMatrix & forwardAdjustments )
    {
        initializeDiscountFactorResults( curveTenor, asOfDate, spotDate, curveCollection, curveIndex,
										 interpolationEnum, paymentDatesInTermFormat, discountFactors, daycount,
										 fixingBusinessDayAdj, fixingCalendar, joinDate, forwardAdjustments );
    }

    // Server Side (Non-Object Pool) Constructor - Dates entered as regular dates here instead of object pool doubles representing year fractions
	// Note: Spot Date should be Curve Swap Calibration Instrument Spot Date - needed for the linear spline join date calculation
    DiscountFactorResults::DiscountFactorResults( const CurveTenorEnum & curveTenor,
                                                  const AQLDate & asOfDate,
												  const AQLDate & spotDate,
												  const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
												  const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
                                                  const InterpolationEnum & interpolationEnum,
                                                  const VectorDate & paymentDates,
                                                  const VectorDouble & discountFactors,
												  const DayCountEnum & daycount,
                                                  const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                  const StandardString & fixingCalendar,
                                                  const AQLDate & joinDate,
												  const StandardStringMatrix & forwardAdjustments )
    {
        std::vector<double> paymentDatesInTermFormat = etrading::convertCurveDatesToTerms( asOfDate, paymentDates );
        initializeDiscountFactorResults( curveTenor, asOfDate, spotDate, curveCollection, curveIndex,
										 interpolationEnum, paymentDatesInTermFormat, discountFactors, daycount,
										 fixingBusinessDayAdj, fixingCalendar, joinDate, forwardAdjustments );
    }

	// Object Pool Constructor - Object Pool Stores Dates as Doubles representing Year Fractions
	// No Spot Date Required, but joinDateAsDouble instead
	DiscountFactorResults::DiscountFactorResults( const CurveTenorEnum & curveTenor,
												  const AQLDate & asOfDate,
												  const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
												  const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
												  const InterpolationEnum & interpolationEnum,
												  const VectorDouble & paymentDatesInTermFormat,
												  const VectorDouble & discountFactors,
												  const DayCountEnum & daycount,
												  const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                  const StandardString & fixingCalendar,
                                                  const double & joinDateAsDouble,
												  const StandardStringMatrix & forwardAdjustments )
	{
		initializeDiscountFactorResults( curveTenor, asOfDate, curveCollection, curveIndex,
										 interpolationEnum, paymentDatesInTermFormat, discountFactors, daycount,
										 fixingBusinessDayAdj, fixingCalendar, joinDateAsDouble, forwardAdjustments );
	}

	// Server Side (Non-Object Pool) Constructor - Dates entered as regular dates here instead of object pool doubles representing year fractions
	// No Spot Date Required, but joinDateAsDouble instead
    DiscountFactorResults::DiscountFactorResults( const CurveTenorEnum & curveTenor,
                                                  const AQLDate & asOfDate,
												  const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
												  const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
                                                  const InterpolationEnum & interpolationEnum,
                                                  const VectorDate & paymentDates,
                                                  const VectorDouble & discountFactors,
												  const DayCountEnum & daycount,
                                                  const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                  const StandardString & fixingCalendar,
                                                  const double & joinDateAsDouble,
												  const StandardStringMatrix & forwardAdjustments )
    {
        std::vector<double> paymentDatesInTermFormat = etrading::convertCurveDatesToTerms( asOfDate, paymentDates );
        
		initializeDiscountFactorResults( curveTenor, asOfDate, curveCollection, curveIndex, interpolationEnum,
			paymentDatesInTermFormat, discountFactors, daycount, fixingBusinessDayAdj, fixingCalendar, joinDateAsDouble,
			forwardAdjustments );
    }

	// Constructor and Copy Constructor Helper Method to initialize the Interpolation Object
	// This helper method initializes the interpolator object once all member data has been set up.
	void DiscountFactorResults::initializeInterpolator()
	{
		AQ_REQUIRE( paymentDatesInTermFormat_.size() == discountFactors_.size(), "Invalid Discount Factor Results: Curve Index '" + curveIndex_ + "' within Curve Collection '" + curveCollection_ + "' has an inconsistent number of paymentDates and discountFactors" )
		
		// Boundary Condition for Interpolation Object Only
		// *** IMPORTANT NOTE *** Interpolator Objects Fail if this boundary point is missing or duplicated
		// Term = 0.0; Discount Factor = 1.0
		// ---------------------------------------------------------------
		
		DoubleVector paymentTerms		= paymentDatesInTermFormat_;
		DoubleVector discountFactors	= discountFactors_;

		// Check Discount Factor Data for Errors
		for( size_t i = 0; i < discountFactors.size(); ++i )
		{
			AQ_THROW_IF( std::isnan( discountFactors[i] ), "Curve Index '" + curveIndex_ + "' within Curve Collection '" + curveCollection_ + " has not been built or has invalid discount factors" )
		}

		if( paymentTerms.size() > 0  )
		{
			if( AQ_IS_EQUAL_ZERO( paymentTerms[0] ) )
			{
				AQ_REQUIRE( AQ_IS_EQUAL( discountFactors[0], 1.0 ), "Invalid Discount Factors: Curve Index '" + curveIndex_ + "' within Curve Collection '" + curveCollection_ + "' has invalid discount factors, Boundary Condition Error: Spot Discount Factors must be set to One" )
			}
			else
			{
				// Only insert Boundary Condition if it is missing i.e. from curve overrides et al.
				paymentTerms.insert( paymentTerms.begin(), 0.0 );
				discountFactors.insert( discountFactors.begin(), 1.0 );
			}
		}

		// ---------------------------------------------------------------

		switch ( interpolationEnum_ )
		{
			// Supported Interpolation Types
			case STEP_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLStepInterpolation() );
				baseInterpolationMethod_->setJoinDateAsDouble( 0.0 );
				baseInterpolationMethod_->set( paymentTerms, discountFactors ); // Must set join date first
				break;
			}
			case RIGHT_CONTINUOUS_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLStepInterpolation(StepType::RIGHT_CONTINUOUS) );
				baseInterpolationMethod_->setJoinDateAsDouble( 0.0 );
				baseInterpolationMethod_->set( paymentTerms, discountFactors ); // Must set join date first
				break;
			}
			case LEFT_CONTINUOUS_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLStepInterpolation(StepType::LEFT_CONTINUOUS) );
				baseInterpolationMethod_->setJoinDateAsDouble( 0.0 );
				baseInterpolationMethod_->set( paymentTerms, discountFactors ); // Must set join date first
				break;
			}
			case LINEAR_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLLinearInterpolation() );
				baseInterpolationMethod_->setJoinDateAsDouble( 0.0 );
				baseInterpolationMethod_->set( paymentTerms, discountFactors ); // Must set join date first
				break;
			}
			case SPLINE_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLSplineInterpolation() );
				baseInterpolationMethod_->setJoinDateAsDouble( 0.0 );
				baseInterpolationMethod_->set( paymentTerms, discountFactors ); // Must set join date first
				break;
			}
			case MONOTONESPLINE_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLMonotoneSplineInterpolation( AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND ) );
				baseInterpolationMethod_->setJoinDateAsDouble( 0.0 );
				baseInterpolationMethod_->set( paymentTerms, discountFactors); // Must set join date first
				break;
			}
				case MONOTONEPARABOLIC_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLMonotoneSplineInterpolation( AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC ) );
				baseInterpolationMethod_->setJoinDateAsDouble( 0.0 );
				baseInterpolationMethod_->set( paymentTerms, discountFactors ); // Must set join date first
				break;
			}
			case LINEARSPLINE_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLLinearSplineInterpolation() );
				baseInterpolationMethod_->setJoinDateAsDouble( joinDateAsDouble_ );
				baseInterpolationMethod_->set( paymentTerms, discountFactors ); // Must set join date first
				break;
			}
			case LINEARMONOTONESPLINE_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND) );
				baseInterpolationMethod_->setJoinDateAsDouble( joinDateAsDouble_ );
				baseInterpolationMethod_->set( paymentTerms, discountFactors ); // Must set join date first
				break;
			}
			case LINEARMONOTONEPARABOLIC_INTERPOLATION:
			{
				// Update Interpolation Object
				baseInterpolationMethod_ = std::shared_ptr<AQLInterpolationBase>( new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC) );
				baseInterpolationMethod_->setJoinDateAsDouble( joinDateAsDouble_ );
				baseInterpolationMethod_->set( paymentTerms, discountFactors ); // Must set join date first
				break;
			}
			default:
			{
				AQ_THROW( "Unsupported Interpolation Type" + toString( interpolationEnum_ ) + " - Valid interpolation schemes include STEP, LINEAR, SPLINE, MONOTONESPLINE, LINEARSPLINE, MONOTONEPARABLIC, LINEARMONOTONEPARABOLIC and LINEARMONOTONESPLINE" );
			}
		}

		// *** Important Note *** Legacy curves assume piecewise constant forward rates when working with a discount factor ...
		// state variable, we want to relax this constraint.
		const bool assumePiecewiseConstantForwards = true;
		if ( assumePiecewiseConstantForwards )
		{
			// Legacy Interpolator Constructor *** New Piecewise Constant Forwards = TRUE ***
			// Supports forward adjustments, state variable choice and turn-of-year features
			interpolator_ = std::shared_ptr<CurveInterpolation>( new CurveInterpolation( asOfDate_,
																						 curveCollection_,
																						 curveIndex_,
																						 interpolationEnum_,
																						 STATE_VARIABLE_DF,
																						 baseInterpolationMethod_,
																						 daycount_,
																						 curveTenor_,
																						 fixingBusinessDayAdj_,
																						 fixingCalendar_,
																						 forwardAdjustments_ ) );
		}
		else
		{
			// New Interpolator Constructor *** New Piecewise Constant Forwards = FALSE ***
			// Supports forward adjustments, state variable choice and turn-of-year features
			interpolator_ = std::shared_ptr<CurveInterpolation>( new CurveInterpolation( asOfDate_,
																						 interpolationEnum_,
																						 STATE_VARIABLE_DF,
																						 baseInterpolationMethod_->getXValues(),
																						 baseInterpolationMethod_->getYValues(),
																						 daycount_,
																						 curveTenor_,
																						 fixingBusinessDayAdj_,
																						 fixingCalendar_,
																						 baseInterpolationMethod_->getJoinDateAsDouble(),
																						 0, // polynomialOrder
																						 forwardAdjustments_ ) ); 
		}
	}


	// Server Side (Non-Object Pool) Method - Dates entered as regular dates here instead of object pool doubles representing year fractions
    // Helper Method to initialize the discount factor class
	// No Spot Date Required, but joinDateAsDouble instead
    void DiscountFactorResults::initializeDiscountFactorResults( const CurveTenorEnum & curveTenor,
                                                                 const AQLDate & asOfDate,
																 const AQLDate & spotDate,
																 const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
																 const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
                                                                 const InterpolationEnum & interpolationEnum,
                                                                 const VectorDouble & paymentDatesInTermFormat,
                                                                 const VectorDouble & discountFactors,
																 const DayCountEnum & daycount,
                                                                 const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                                 const StandardString & fixingCalendar,
                                                                 const AQLDate & joinDate,
																 const StandardStringMatrix & forwardAdjustments )
    {
        AQ_REQUIRE( paymentDatesInTermFormat.size() == discountFactors.size(), "Invalid Discount Factor Results - Inconsistent number of paymentDates and discountFactors" )

		// Update Member Variables
        curveTenorEnum_                 = curveTenor;
        curveTenor_                     = toString( curveTenor );
        asOfDate_                       = asOfDate;
		spotDate_						= spotDate;
		curveCollection_				= curveCollection;
	    curveIndex_						= curveIndex;
        interpolationEnum_              = interpolationEnum;
        paymentDatesInTermFormat_       = paymentDatesInTermFormat;
        discountFactors_                = discountFactors;
        daycount_						= daycount;
		joinDate_						= joinDate;
		joinDateAsDouble_				= ( joinDate == AQLDate() ) ? 0.0 : etrading::convertCurveDateToTerm( spotDate, joinDate );
        fixingBusinessDayAdj_           = fixingBusinessDayAdj;
        fixingCalendar_                 = fixingCalendar;
		forwardAdjustments_				= forwardAdjustments;

		if ( !forwardAdjustments_.empty() )
        {
			AQ_REQUIRE( forwardAdjustments_[0].size() == 4, "Invalid Data: Forward adjustment tables must have 4 columns:- AdjustmentType, StartDate, EndDate, RateOrSpread ");
        }

		initializeInterpolator();
	}


	// Object Pool Method - Object Pool Stores Dates as Doubles representing Year Fractions
	// Helper Method to initialize the discount factor class
	// Note: Spot Date should be Curve Swap Calibration Instrument Spot Date - needed for the linear spline join date calculation
	void DiscountFactorResults::initializeDiscountFactorResults( const CurveTenorEnum & curveTenor,
																 const AQLDate & asOfDate,
																 const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
																 const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
																 const InterpolationEnum & interpolationEnum,
																 const VectorDouble & paymentDatesInTermFormat,
																 const VectorDouble & discountFactors,
																 const DayCountEnum & daycount,
                                                                 const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                                 const StandardString & fixingCalendar, 
																 const double & joinDateAsDouble,
																 const StandardStringMatrix & forwardAdjustments )
	{
		AQ_REQUIRE( paymentDatesInTermFormat.size() == discountFactors.size(), "Invalid Discount Factor Results - Inconsistent number of paymentDates and discountFactors" )

		// Update Member Variables
		curveTenorEnum_					= curveTenor;
		curveTenor_						= toString( curveTenor );
		asOfDate_						= asOfDate;
		spotDate_						= AQLDate();
		curveCollection_				= curveCollection;
	    curveIndex_						= curveIndex;
		interpolationEnum_				= interpolationEnum;
		paymentDatesInTermFormat_		= paymentDatesInTermFormat;
		discountFactors_				= discountFactors;
		daycount_						= daycount;
		joinDate_						= AQLDate();
		joinDateAsDouble_				= joinDateAsDouble;
        fixingBusinessDayAdj_           = fixingBusinessDayAdj;
        fixingCalendar_                 = fixingCalendar;
		forwardAdjustments_				= forwardAdjustments;

		if ( !forwardAdjustments_.empty() )
        {
			AQ_REQUIRE( forwardAdjustments_[0].size() == 4, "Invalid Data: Forward adjustment tables must have 4 columns:- AdjustmentType, StartDate, EndDate, RateOrSpread ");
        }

		initializeInterpolator();
	}

    // ================================================ DISCOUNT FACTOR METHODS ================================================

	void DiscountFactorResults::setDiscountFactorsUsingTerms( const VectorDouble & paymentDatesInTermsFormat, const VectorDouble & discountFactors )
	{
		paymentDatesInTermFormat_ = paymentDatesInTermsFormat;
		discountFactors_ = discountFactors;

		// Update the data interpolator
		initializeInterpolator();
	}

	void DiscountFactorResults::setDiscountFactors( const VectorDate & paymentDates, const VectorDouble & discountFactors )
	{
		paymentDatesInTermFormat_ = etrading::convertCurveDatesToTerms( asOfDate_, paymentDates );
		discountFactors_ = discountFactors;

		// Update the data interpolator
		initializeInterpolator();
	}

    double DiscountFactorResults::getDiscountFactor( const double & paymentDateInTermFormat ) const
    {
		// Boundary Conditions
		// -------------------
		// Discount Factors on the AsOfDate = 1.0
		// Discount Factors before the AsOfDate = 0.0
		// -------------------
		if( AQ_IS_LESS_THAN_OR_EQUAL_TO_ZERO( paymentDateInTermFormat ) )
		{
			if( AQ_IS_EQUAL_ZERO( paymentDateInTermFormat ) )
			{
				return 1.0;
			}
			return 0.0;
		}
		
        const double dfResult = interpolator_->discountFactor(paymentDateInTermFormat);
        return dfResult;
    }

    double DiscountFactorResults::getDiscountFactor( const AQLDate & paymentDate ) const
    {
        const double paymentDateInTermFormat =  etrading::convertCurveDateToTerm( asOfDate_, paymentDate );
        const double dfResult = getDiscountFactor( paymentDateInTermFormat);
        return dfResult;
    }

    VectorDouble DiscountFactorResults::getDiscountFactors( const VectorDate & paymentDates ) const
    {
        VectorDouble dfResults( paymentDates.size() );
        for ( size_t i = 0; i < paymentDates.size(); ++i )
        {
            dfResults[i] = getDiscountFactor( paymentDates[i] );
        }
        return dfResults;
    }
        
    VectorDouble DiscountFactorResults::getDiscountFactors( const VectorDouble & paymentDatesInTermFormat ) const
    {
        VectorDouble dfResults( paymentDatesInTermFormat.size() );
        for ( size_t i = 0; i < paymentDatesInTermFormat.size(); ++i )
        {
            dfResults[i] = getDiscountFactor( paymentDatesInTermFormat[i] );
        }
        return dfResults;
    }

    VectorDate DiscountFactorResults::paymentDates() const
    {
        const VectorDate paymentDates = etrading::convertCurveTermsToDates( asOfDate_, paymentDatesInTermFormat_ );
        return paymentDates;
    }

    VariantMatrix DiscountFactorResults::paymentDatesInTermsFormatAndDiscountFactors() const
    {
        AQ_REQUIRE( paymentDatesInTermFormat_.size() == discountFactors_.size(), "Invalid Discount Factor Results - Inconsistent number of paymentDates and discountFactors" )

        const size_t TWO_COLUMNS_OF_DATA = 2;
        VariantMatrix results( discountFactors_.size() );
        
        for ( size_t i = 0; i < discountFactors_.size(); ++i )
        {
            VariantVector currentRow(TWO_COLUMNS_OF_DATA);
            currentRow[0] = paymentDatesInTermFormat_[i];
            currentRow[1] = discountFactors_[i];
            results[i] = currentRow;
        }

        return results;
    }

    VariantMatrix DiscountFactorResults::paymentDatesAndDiscountFactors() const
    {
        AQ_REQUIRE( paymentDatesInTermFormat_.size() == discountFactors_.size(), "Invalid Discount Factor Results - Inconsistent number of paymentDates and discountFactors" )

        const size_t TWO_COLUMNS_OF_DATA = 2;
        VariantMatrix results( discountFactors_.size() );
        
        const DateVector paymentDates = etrading::convertCurveTermsToDates( asOfDate_, paymentDatesInTermFormat_ );

        for ( size_t i = 0; i < discountFactors_.size(); ++i )
        {
            VariantVector currentRow(TWO_COLUMNS_OF_DATA); 
            currentRow[0] = paymentDates[i];
            currentRow[1] = discountFactors_[i];
            results[i] = currentRow;
        }

        return results;
    }

    // ================================================ IMPLIED FORWARD RATE METHODS ================================================

    double DiscountFactorResults::implyForwardRate( const double & fromDateInTermFormat,
                                                    const double & toDateInTermFormat,
													const bool isFwdInter,
                                                    const CompoundingFrequencyEnum & compoundFrequency ) const
    {
		double forwardRate = 0.0;

		if ( !isFwdInter )
		{
			// For Backwards Compatibility we set historic forward rates to zero
			// --------------------------------------------------------------------------------------------------------------------------
			// AQ_REQUIRE( fromDateInTermFormat >= AsOfDateInTermsFormat, "Unable to imply Forward Rate(s) from Discount Factors - The reset date is before the curve asOfDate; Fixing Table required" )
			const double AsOfDateInTermsFormat = 0.0;
			if ( fromDateInTermFormat <  AsOfDateInTermsFormat )
			{
			    return forwardRate;
			}
			
			if (fromDateInTermFormat == toDateInTermFormat)
			{
			    return forwardRate;
			}
			
			// Important!!
			// The term here must be the converted from the internal ACT/365 daycount used for date transformation to a term in the curve daycount measure
			// --------------------------------------------------------------------------------------------------------------------------
			const double accrualTerm = accrualPeriod( fromDateInTermFormat, toDateInTermFormat, asOfDate_, daycount_, compoundFrequency );
			AQ_REQUIRE( AQ_IS_GREATER_THAN_ZERO( accrualTerm ), "Unable to imply Forward Rates(s) from Discount Factors - The reset start date must be before the reset end date" )
			
			const double fromDF				= getDiscountFactor( fromDateInTermFormat );
			const double toDF				= getDiscountFactor( toDateInTermFormat );
			
			switch ( compoundFrequency )
			{
			case SIMPLE_COMPOUNDING:
			    forwardRate					= ( ( fromDF / toDF ) - 1.0 ) / accrualTerm;
			    break;
			case CONTINUOUS_COMPOUNDING:
			    forwardRate					= std::log( fromDF / toDF ) / accrualTerm;
			    break;
			case ANNUAL_COMPOUNDING:
			    forwardRate					= std::pow( fromDF / toDF, 1.0 / accrualTerm ) - 1.0;
			    break;
			case SEMI_ANNUAL_COMPOUNDING:
			    forwardRate					= ( std::pow( fromDF / toDF, 1.0 / ( accrualTerm * 2.0 ) ) - 1.0 ) * 2.0;
			    break;
			case MONTHLY_COMPOUNDING:
			    forwardRate					= ( std::pow( fromDF / toDF, 1.0 / ( accrualTerm * 12.0 ) ) - 1.0 ) * 12.0;
			    break;
			case QUARTERLY_COMPOUNDING:
			    forwardRate					= ( std::pow( fromDF / toDF, 1.0 / ( accrualTerm * 4.0 ) ) - 1.0 ) * 4.0;
			    break;
			default:
			    AQ_THROW("Invalid Curve Compound Frequency: " + toString(compoundFrequency) );
			}
		}
		else
		{
			// Imply Forward Rate By Interpolating on Pseudo Forwards
			forwardRate = interpolator_->forwardRate( fromDateInTermFormat, isFwdInter );
		}
		return forwardRate;
    }

    double DiscountFactorResults::implyForwardRate( const AQLDate & fromDate,
                                                    const AQLDate & toDate,
													const bool isFwdInter,
                                                    const CompoundingFrequencyEnum & compoundFrequency ) const
    {
        // For Backwards Compatibility we set historic forward rates to zero
        if ( fromDate <  asOfDate_ )
        {
            return 0.0;
        }

        //AQ_REQUIRE( fromDate >= asOfDate_,    "Unable to imply Forward Rate(s) from Discount Factors - The reset date is before the curve asOfDate; Fixing Table required" )
        AQ_REQUIRE( toDate > fromDate,        "Unable to imply Forward Rates(s) from Discount Factors - The reset start date must be before the reset end date" )

        const double fromDateInTermFormat   = convertCurveDateToTerm( asOfDate_, fromDate );
        const double toDateInTermFormat     = convertCurveDateToTerm( asOfDate_, toDate );

        const double forwardRate            = implyForwardRate( fromDateInTermFormat, toDateInTermFormat, isFwdInter, compoundFrequency );
        return forwardRate;
    }

    double DiscountFactorResults::implyForwardRate( const double & fixingDateInTermFormat,
                                                    const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                    const StandardString & fixingCalendar,
													const bool isFwdInter,
                                                    const CompoundingFrequencyEnum & compoundFrequency ) const
    {
        const AQLDate fixingDate     = convertCurveTermToDate( asOfDate_, fixingDateInTermFormat );
        const double forwardRate    = implyForwardRate( fixingDate, fixingBusinessDayAdj, fixingCalendar, isFwdInter, compoundFrequency );
        return forwardRate;
    }

    double DiscountFactorResults::implyForwardRate( const AQLDate & fixingDate,
                                                    const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                    const StandardString & fixingCalendar,
													const bool isFwdInter,
                                                    const CompoundingFrequencyEnum & compoundFrequency ) const
    {
        const StandardString fixingbusDayAdj    = toString( fixingBusinessDayAdj );
        const AQLDate fixingEndDate              = getSingleDateFromTenor( fixingDate, curveTenor_.c_str(), fixingbusDayAdj.c_str(), fixingCalendar.c_str() );
        
        const double forwardRate                = implyForwardRate( fixingDate, fixingEndDate, isFwdInter, compoundFrequency );
        return forwardRate;
    }

    VectorDouble DiscountFactorResults::ImplyForwardRates( const VectorDouble & fromDatesInTermFormat,
                                                           const VectorDouble & toDatesInTermFormat,
														   const bool isFwdInter,
                                                           const CompoundingFrequencyEnum & compoundFrequency ) const
    {
        AQ_REQUIRE( fromDatesInTermFormat.size() == toDatesInTermFormat.size(), "Unable to imply Forward Rates - The number of reset start and end dates must match" )
        VectorDouble forwardResults( fromDatesInTermFormat.size() );
        for ( size_t i = 0; i < fromDatesInTermFormat.size(); ++i )
        {
            forwardResults[i] = implyForwardRate( fromDatesInTermFormat[i], toDatesInTermFormat[i], isFwdInter, compoundFrequency );
        }
        return forwardResults;
    }

    VectorDouble DiscountFactorResults::implyForwardRates( const VectorDate & fromDates,
                                                           const VectorDate & toDates,
														   const bool isFwdInter,
                                                           const CompoundingFrequencyEnum & compoundFrequency ) const
    {
        AQ_REQUIRE( fromDates.size() == toDates.size(), "Unable to imply Forward Rates - The number of reset start and end dates must match" )
        VectorDouble forwardResults( fromDates.size() );
        for ( size_t i = 0; i < fromDates.size(); ++i )
        {
            forwardResults[i] = implyForwardRate( fromDates[i], toDates[i], isFwdInter, compoundFrequency );
        }
        return forwardResults;
    }

    VectorDouble DiscountFactorResults::implyForwardRates( const VectorDouble & fixingDatesInTermsFormat,
                                                           const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                           const StandardString & fixingCalendar,
														   const bool isFwdInter,
                                                           const CompoundingFrequencyEnum & compoundFrequency ) const
    {
        VectorDouble forwardResults( fixingDatesInTermsFormat.size() );
        for ( size_t i = 0; i < fixingDatesInTermsFormat.size(); ++i )
        {
            forwardResults[i] = implyForwardRate( fixingDatesInTermsFormat[i], fixingBusinessDayAdj, fixingCalendar, isFwdInter, compoundFrequency );
        }
        return forwardResults;
    }

    VectorDouble DiscountFactorResults::implyForwardRates( const VectorDate & fixingDates,
                                                           const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                                           const StandardString & fixingCalendar,
														   const bool isFwdInter,
                                                           const CompoundingFrequencyEnum & compoundFrequency ) const
    {
        VectorDouble forwardResults( fixingDates.size() );
        for ( size_t i = 0; i < fixingDates.size(); ++i )
        {
            forwardResults[i] = implyForwardRate( fixingDates[i], fixingBusinessDayAdj, fixingCalendar, isFwdInter, compoundFrequency );
        }
        return forwardResults;
    }

	// Method to imply the forward rates on the discount factor pillar points referenced by payment date
	VectorDouble DiscountFactorResults::implyForwardRates( const bool isFwdInter ) const
	{
		return implyForwardRates( paymentDates(), fixingBusinessDayAdj_, fixingCalendar_, isFwdInter );
	}

}
