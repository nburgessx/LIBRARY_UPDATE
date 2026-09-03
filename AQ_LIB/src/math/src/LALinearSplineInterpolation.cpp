/*! @file
    @brief Class declaration to perform linear-spline interpolation for the yield curve.

    In this class, based on the data set of n pieces of data to be interpolated n \n
	returns the value of one-dimensional. Data every time you set the data, \
	the data you set just before is destroyed. \n
	Outside is constant interpolation.

*/
//  3rd April 2017, AlgoQuantHub


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LALinearSplineInterpolation.h"
#include "LAAlgorithm.h"

using namespace std;

//================ LAInterpolationBase ===================================

/*!
    @brief default constructor
    // FLAT_EXTRAPOLATION = Piecewise-Constant Extrapolation
	// LINEAR_EXTRAPOLATION = Linear Extrapolation
	// For Spline, true = natural spline
*/
LALinearSplineInterpolation::LALinearSplineInterpolation() 
: LAInterpolationBase(), 
  isJoinDateSet_(false), 
  linearInitialised_(false),
  splineInitialised_(false),
  extrapolationType_(LINEAR_EXTRAPOLATION_TYPE)
{
	// Initialize the underlying methods and their data containers aka interpolation LADataProvider
	linearInterpolation_ = std::shared_ptr<LALinearInterpolation>(new LALinearInterpolation(LINEAR_EXTRAPOLATION_TYPE));
	splineInterpolation_ = std::shared_ptr<LASplineInterpolation>(new LASplineInterpolation(true));
}

/*!
    @brief alternative constructor
    // FLAT_EXTRAPOLATION = Piecewise-Constant Extrapolation
	// LINEAR_EXTRAPOLATION = Linear Extrapolation
*/
LALinearSplineInterpolation::LALinearSplineInterpolation( ExtrapolationType extrapolationType, const bool useNaturalSpline ) 
: LAInterpolationBase(), 
  isJoinDateSet_(false), 
  linearInitialised_(false),
  splineInitialised_(false),
  extrapolationType_(extrapolationType)
{
    // Initialize the underlying methods and their data containers aka interpolation LADataProvider
	linearInterpolation_ = std::shared_ptr<LALinearInterpolation>(new LALinearInterpolation(extrapolationType_));
	splineInterpolation_ = std::shared_ptr<LASplineInterpolation>(new LASplineInterpolation(useNaturalSpline));
}

/*!
	@brief destructor
*/
LALinearSplineInterpolation::~LALinearSplineInterpolation() 
{
}

/*!
    @brief copy constructor
*/
LALinearSplineInterpolation::LALinearSplineInterpolation( const LALinearSplineInterpolation& v )
: LAInterpolationBase(v),
    extrapolationType_(v.extrapolationType_),
    joinDateAsDouble_(v.joinDateAsDouble_),
    isJoinDateSet_(v.isJoinDateSet_),
    indices_(v.indices_),
    values_(v.values_),
	linearInitialised_(v.linearInitialised_),
	splineInitialised_(v.splineInitialised_)
{
	splineInterpolation_    = std::shared_ptr<LASplineInterpolation>(dynamic_cast<LASplineInterpolation*>(v.splineInterpolation_->clone()));
    linearInterpolation_    = std::shared_ptr<LALinearInterpolation>(dynamic_cast<LALinearInterpolation*>(v.linearInterpolation_->clone()));
	
	// Base Class Data
	interpolationData_		= std::make_shared<InterpolationData>(v.indices_, v.values_);
}

/*!
    @brief clone
*/
LACoreFunctionBase*	
LALinearSplineInterpolation::clone() const
{
    LALinearSplineInterpolation* pTmp = nullptr;
    try 
	{
	    pTmp = new LALinearSplineInterpolation( *this );

		// Base Class Data
		pTmp->interpolationData_ = std::make_shared<InterpolationData>( pTmp->indices_, pTmp->values_ );

        return pTmp;
    }
    catch ( bad_alloc e )
	{
        delete pTmp;
        throw LACoreSystemError( e.what(), __FILE__, __LINE__ );
    }
	
}
/*!
    @brief check whether this class derives from base class with type id
	
	@param[in] id ID to check
	@return True or false
*/
bool
LALinearSplineInterpolation::isTypeOf(function_t id) const
{
	return ( id == FN_LINEARSPLINEINTERPOLATION ? true : LAInterpolationBase::isTypeOf( id ) );
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LALinearSplineInterpolation::getType() const
{
	return FN_LINEARSPLINEINTERPOLATION;
}

/*!
    @brief a value of one-dimensional complement of the curve
	@param[in] x1
	@return a value of one-dimensional complement of the curve
*/
double
LALinearSplineInterpolation::value( const double x1 ) const
{
    if( x1 < joinDateAsDouble_ )
    {
        return linearInterpolation_->value( x1 );
    }
    else
    {
		if (splineInitialised_)
		{
			return splineInterpolation_->value( x1 );
		}
		else
		{
			return linearInterpolation_->value( x1 );
		}
    }
} 

// Differentiate the Interpolator at Point x1
double LALinearSplineInterpolation::differentiate(const double & x1) const
{
	MLIB_REQUIRE( interpolationData_ != nullptr, "Linear-Spline interpolation data has not been set" )
	MLIB_REQUIRE( interpolationData_->size_ > 0, "Linear-Spline interpolation data has not been set" )
	
	// Function is flat when there is only one point, therefore slope is zero
	if ( interpolationData_->size_ == 1 )
	{
		return 0.0;
	}

	if( x1 < joinDateAsDouble_ )
    {
        return linearInterpolation_->differentiate( x1 );
    }
    else
    {
		if (splineInitialised_)
		{
			return splineInterpolation_->differentiate( x1 );
		}
		else
		{
			return linearInterpolation_->differentiate( x1 );
		}
    }
} 

// Integrate the Interpolator Over the Lower- and UpperBounds
double LALinearSplineInterpolation::integrate(const double & lowerBound, const double & upperBound ) const
{
	MLIB_REQUIRE( interpolationData_ != nullptr, "Linear-Spline interpolation data has not been set" )
	MLIB_REQUIRE( interpolationData_->size_ > 0, "Linear-Spline interpolation data has not been set" )
	MLIB_REQUIRE( lowerBound <= upperBound, "Invalid Linear-Spline Integral: The Lowerbound must not be greater than the UpperBound")
	
	// The integral result variable
	double integral = 0.0;

	// Boundary Condition: Zero Width
	if( MLIB_IS_EQUAL( lowerBound, upperBound ) )
	{
		return 0.0;
	}

	// Boundary Condition: Single Interpolation Point, Assume Flat
	if ( interpolationData_->size_ == 1 )
	{
		return interpolationData_->yValues_[0] * ( upperBound - lowerBound );
	}
	
	if( MLIB_IS_LESS_THAN( lowerBound, joinDateAsDouble_ ) )
	{
		// The Linear Part of the Integral
		// From LowerBound to min( JoinDate, UpperBound )
		const double linearUpperBound = std::min( joinDateAsDouble_, upperBound );
		integral += linearInterpolation_->integrate( lowerBound, linearUpperBound );
	}

	if( MLIB_IS_GREATER_THAN( upperBound, joinDateAsDouble_ ) )
	{
		// The Spline Part of the Integral
		// From max( JoinDate, LowerBound ) to UpperBound
		const double splineLowerBound = std::max( joinDateAsDouble_, lowerBound );
		integral += splineInterpolation_->integrate( splineLowerBound, upperBound );
	}
	
	return integral;
}

/*!
@brief Get the linear-spline join date i.e. the linear interpolation end date
*/
const double
LALinearSplineInterpolation::getJoinDateAsDouble() const
{
    return joinDateAsDouble_;
}
/*
	This method initialises the class data in the correct order
	Must set the joinDateAsDouble before setting the index and value parameters
*/
void LALinearSplineInterpolation::set( const DoubleArray& index, const DoubleArray& value, const double& joinDateAsDouble )
{
	setJoinDateAsDouble( joinDateAsDouble );
	set( index, value );
}


/*!
@brief Set the linear-spline join date i.e. the linear interpolation end date
    Note: We also set a flag here to indicate the join date has been set. If this flag is not set then the set interpolation method will throw an error

	@param[in] linearInterpolationEndDateAsDouble   The end date for linear interpolation, input as a year fraction double

	Note: We sust set the joinDateAsDouble before setting the index and value parameters
*/
void
LALinearSplineInterpolation::setJoinDateAsDouble( const double& joinDateAsDouble )
{
    joinDateAsDouble_   = joinDateAsDouble;
    isJoinDateSet_      = true;
}

/*!
@brief set the information of one-dimensional curve

@param[in] index X-axis value of the data to be interpolated
@param[in] value Y-axis value of the data to be interpolated

Note: We sust set the joinDateAsDouble before setting the index and value parameters

*/
void
LALinearSplineInterpolation::set( const DoubleArray& index, const DoubleArray& value )
{
    // This parameter is set in the 'setLinearInterpolationEndDateAsDouble' method above.
    if( !isJoinDateSet_ )
    {
        throw LACoreInvalidData("#Error: Join date has not been set in Linear Spline interpolation scheme", __FILE__, __LINE__);
    }

    if( index.size() != value.size() )
    {
        throw LACoreInvalidData("#Error: Interpolation index size and value sizes are not same.", __FILE__, __LINE__);
    }
    
	// Check for Duplicates
	if ( index.size() > 1 )
	{
		double previousIndex = index[0];
		double thisIndex = index[1];
		for ( size_t i = 1; i < index.size(); ++i )
		{
			thisIndex		= index[i];
			previousIndex	= index[i-1];
			MLIB_THROW_IF(thisIndex == previousIndex, "Invalid Interpolation Data: Duplicate data found with time value: " + MLIB_TO_STRING_FROM_DOUBLE(thisIndex) + " years" )
		}
	}

    // Set the Global Linear-Spline Interpolations Indicies and Values
    indices_    = index;
    values_     = value;

    // Set the Local Linear and Spline Interpolations Indicies and Values
    DoubleArray linearIndex;
    DoubleArray linearValue;

    DoubleArray splineIndex;
    DoubleArray splineValue;

    // Partition and Set the Linear and Spline Index-Value Tables
    for( size_t i = 0; i < index.size(); ++i )
    {
        // Note we must include the joint node in both tables
        if ( index[i] <= joinDateAsDouble_ )
        {
            linearIndex.push_back( index[i] );
            linearValue.push_back( value[i] );
        }
        
        // Note we must include the joint node in both tables
        if ( index[i] >= joinDateAsDouble_ )
        {
            splineIndex.push_back( index[i] );
            splineValue.push_back( value[i] );
        }
    }

    // Validate Interpolation Data Table Sizes
    if ( linearIndex.size() == 0 && splineIndex.size() == 0 )
    {
        throw LACoreInvalidData("#Error: Linear-Spline interpolation data has not been set", __FILE__, __LINE__);
    }

    // Manage the Hybrid Interpolation Join Point and allow the Linear and Spline Interpolation tables to Extrapolate onto each other consistently
    // In the extreme case where there is no join point the hybrid linear-spline will collapse to a Linear or Spline interpolator
    // -------------------------------------------------------------------------------------------------------------------------------
    if( linearIndex.size() >= 2 && splineIndex.size() >= 2 ) // Extrapolation Conditions to prevent access violation errors
    {
		const double lastLinearIndex          = linearIndex[linearIndex.size()-1];
        const double lastLinearValue          = linearValue[linearValue.size()-1];

        const double firstSplineIndex         = splineIndex[0];
        const double firstSplineValue         = splineValue[0];

		if (lastLinearIndex < firstSplineIndex)
		{
			DoubleArray transitionIndex, transitionValue;

			// Use last two linear points and first two spline points as the transition section
			const double penultimateLinearIndex = linearIndex[linearIndex.size()-2];
			const double penultimateLinearValue = linearValue[linearValue.size()-2];

			transitionIndex.push_back(penultimateLinearIndex);
			transitionValue.push_back(penultimateLinearValue);

			transitionIndex.push_back(lastLinearIndex);
			transitionValue.push_back(lastLinearValue);

			transitionIndex.push_back(firstSplineIndex);
			transitionValue.push_back(firstSplineValue);

			const double secondSplineIndex = splineIndex[1];
			const double secondSplineValue = splineValue[1];

			transitionIndex.push_back(secondSplineIndex);
			transitionValue.push_back(secondSplineValue);

			// Use a spline to represent the transition section and derive the joint date value
			std::shared_ptr<LASplineInterpolation> transitionInter(new LASplineInterpolation(true)); // use natural spline
			transitionInter->set(transitionIndex, transitionValue);

			double joinDateValue = transitionInter->value(joinDateAsDouble_);

			if( lastLinearIndex < joinDateAsDouble_ ) // Strictly Less Than ...
			{
				linearIndex.push_back(joinDateAsDouble_);
				linearValue.push_back(joinDateValue);
			}

			if( firstSplineIndex > joinDateAsDouble_ ) // Strictly Greater Than ...
			{
				splineIndex.insert( splineIndex.begin(), joinDateAsDouble_);
				splineValue.insert( splineValue.begin(), joinDateValue);
			}		
		}
    }
    // -------------------------------------------------------------------------------------------------------------------------------
	 
    // Set the Interpolation Tables
    if ( linearIndex.size() >= 1 )
    {
        // Linear Interpolation Requires 1 Points - The Linear Interpolation Class will throw if not set
        linearInterpolation_->set( linearIndex, linearValue );
		linearInitialised_ = true;
    }

    if ( splineIndex.size() >= 2 )
    {
        // Spline Interpolation Requires 2 Points - The Spline Interpolation Class will throw if not set
        splineInterpolation_->set( splineIndex, splineValue );
		splineInitialised_ = true;
    }
    
	interpolationData_ = std::make_shared<InterpolationData>( indices_, values_ );
	return;
}

std::tuple<std::vector<double>,std::vector<double>> LALinearSplineInterpolation::getXY() const
{
	return std::make_tuple( indices_, values_ );
}
