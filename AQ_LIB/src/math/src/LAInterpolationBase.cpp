// LAInterpolationBase.cpp

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAInterpolationBase.h"


// Constructor
LAInterpolationBase::LAInterpolationBase() 
: LACoreFunctionBase()
{}

// Copy Constructor
LAInterpolationBase::LAInterpolationBase(const LAInterpolationBase& v) 
: LACoreFunctionBase(v)
{}

// Destructor
LAInterpolationBase::~LAInterpolationBase() 
{}

// Check for the Interpolation Type ID
bool LAInterpolationBase::isTypeOf(function_t id) const
{
	return (id == FN_INTERPOLATION ? true : LACoreFunctionBase::isTypeOf(id));
}

// Function to get the interpolation type
function_t LAInterpolationBase::getType() const
{
	return FN_INTERPOLATION;
}

// Method to differentiate the interpolation function - Must implement this separately in every derived interpolation class
double LAInterpolationBase::differentiate( const double xPoint ) const
{
	AQ_THROW( "Mathematical Differentiation Not Supported for this Interpolation Type" );
}

// Method to discretely differentiate the interpolation function between two points
double LAInterpolationBase::differentiateOverRange( const double fromXPoint, const double toXPoint ) const
{
	AQ_REQUIRE( AQ_IS_GREATER_THAN( toXPoint, fromXPoint ), "Invalid Differentiation Interval: The toXPoint must be greater than the fromXPoint" )
	
	const double fromYPoint	= value( fromXPoint );
	const double toYPoint	= value( toXPoint );

	const double dy		= toYPoint - fromYPoint;
	const double dx		= toXPoint - fromXPoint;
	
	return dy/dx;
}

// Method to integrate the interpolation function - Must implement this separately in every derived interpolation class
double LAInterpolationBase::integrate( const double lowerBound, const double upperBound ) const
{ 
	AQ_THROW( "Mathematical Integration Not Supported for this Interpolation Type" );
}

// Function to Calculate the Position of the Supremum i.e. the position of the first node point greater than or equal to a given value
size_t LAInterpolationBase::supremum( const double & value ) const
{
	AQ_REQUIRE( interpolationData_ != nullptr, "Invalid Data: Interpolation data has not been set");
	AQ_REQUIRE( interpolationData_->size_ >= 2, "Invalid Data: At least two data points are required");

	// Find upperBound Supremum i.e. the next node point relative to the integration upperBound variable
	// Note we floor at position 1 and disallow 0, since end index cannot be the start index point 
	// Also if we land on a node point we decrement the index, since we want to include the node if we land on it
	unsigned int upperEndIndex = std::upper_bound( interpolationData_->xValues_.begin(), interpolationData_->xValues_.end(), value ) - interpolationData_->xValues_.begin();
	if( upperEndIndex > 0 && value >= interpolationData_->xValues_[interpolationData_->size_-1] )
	{
		upperEndIndex = interpolationData_->size_-1;
	}
	upperEndIndex = std::max( upperEndIndex, 1u );

	return upperEndIndex;
}

// Function to get the end index and shift the index to account for extrapolation
Interval LAInterpolationBase::getInterval( const size_t & endIndex ) const
{
	size_t theEndIndex = endIndex;
	if( theEndIndex == 0 )
	{
		theEndIndex = 1;
	}
	else if( theEndIndex > interpolationData_->size_ - 1 )
	{
		theEndIndex = interpolationData_->size_ - 1;
	}
	
	Interval result;
	result.startIndex_ = theEndIndex - 1;
	result.endIndex_ = theEndIndex;

	return result;
}

// Function to calculate the start height at the start of a given interval
double LAInterpolationBase::intervalStartHeight( const Interval& interval ) const
{
	// Integrand Start Height
	const double height = interpolationData_->yValues_[interval.startIndex_];
	return height;
}

// Function to calculate the start height at the start of a given interval
double LAInterpolationBase::intervalEndHeight( const Interval& interval ) const
{
	// Integrand Start Height
	const double height = interpolationData_->yValues_[interval.endIndex_];
	return height;
}

// Function to calculate the integral width for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
double LAInterpolationBase::intervalWidth( const Interval& interval, const double & lowerBound, const double & upperBound  ) const
{
	// Get Coordianates and Ajdust for Partial Integrals
	double xStart	= std::max( interpolationData_->xValues_[interval.startIndex_], lowerBound );
	double xEnd		= std::min( interpolationData_->xValues_[interval.endIndex_], upperBound );
	
	// Extrapolate Front Interval, if lowerBound less than first interval start index
	if( interval.startIndex_ == 0 )
	{
		xStart = std::min( xStart, lowerBound );
	}
	// Extrapolate Back Interval, if upperBound greater than last interval end index
	else if( interval.endIndex_ == interpolationData_->size_ - 1)
	{
		xEnd = std::max( xEnd, upperBound );
	}

	const double width	= std::max( xEnd - xStart, 0.0 );
	return width;
}

// Function to calculate the slope between two consecutive points
double LAInterpolationBase::intervalSlope( const Interval& interval ) const
{
	AQ_THROW( "Mathematical Differentiation Not Supported for this Interpolation Type" );
}

// Function to calculate the area for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
double LAInterpolationBase::intervalArea( const size_t & endIndex, const double & lowerBound, const double & upperBound ) const
{
	AQ_THROW( "Mathematical Integration Not Supported for this Interpolation Type" );
}