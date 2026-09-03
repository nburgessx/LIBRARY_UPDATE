// LALinearInterpolation.cpp

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LALinearInterpolation.h"
#include "LAAlgorithm.h"
#include "ExceptionMacros.h"
#include <algorithm>

using namespace std;

//================ LAInterpolationBase ===================================

LALinearInterpolation::LALinearInterpolation(ExtrapolationType type) 
: LAInterpolationBase(), mpDataProvider(NULL), mExtrapolationType(type)
{}

LALinearInterpolation::~LALinearInterpolation() 
{
	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
	}
}

LALinearInterpolation::LALinearInterpolation(const LALinearInterpolation& v)
: LAInterpolationBase(v), mpDataProvider(NULL)
{
	mExtrapolationType = v.mExtrapolationType;	
    try 
	{	
		mpDataProvider = new LALinearInterpolationDataProvider(*v.mpDataProvider);
		
		// Base Class Data
		interpolationData_ = std::make_shared<InterpolationData>( mpDataProvider->index1, mpDataProvider->value1 );
	}
    catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

LACoreFunctionBase*	LALinearInterpolation::clone() const
{
    LALinearInterpolation* pTmp = nullptr;
    try 
	{
		pTmp =new LALinearInterpolation(mExtrapolationType);

		if (mpDataProvider != NULL)
		{
			pTmp->mpDataProvider = new LALinearInterpolationDataProvider(this->mpDataProvider->index1,this->mpDataProvider->value1,this->mpDataProvider->size1);

			// Base Class Data
			pTmp->interpolationData_ = std::make_shared<InterpolationData>( pTmp->mpDataProvider->index1, pTmp->mpDataProvider->value1 );
		}

		return pTmp;
    }
    catch (bad_alloc & e)
	{
        delete pTmp;
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }

//===================	LALinearInterpolation.cpp  =======================//
	
}
/*  @brief check whether this class derives from base class with type id
	
	@param[in] id ID to check
	@return True or false
*/
bool LALinearInterpolation::isTypeOf(function_t id) const
{
	return (id == FN_LINEARINTERPOLATION ? true : LAInterpolationBase::isTypeOf(id));
}

/*  @brief Return this function type
    @return function type
*/
function_t LALinearInterpolation::getType() const
{
	return FN_LINEARINTERPOLATION;
}

// Interpolation Value
double LALinearInterpolation::value(const double x1) const
{
	AQ_REQUIRE( mpDataProvider != nullptr, "Linear interpolation data has not been set" )
	AQ_REQUIRE( mpDataProvider->size1 > 0, "Linear interpolation data has not been set" )
	
	if ( mpDataProvider->size1 == 1 )
	{
		// Function is flat when there is only one point
		return mpDataProvider->value1[0];
	}

	// Find x1 Supremum i.e. the next node point relative to x1
	unsigned int position;
	LAAlgorithm::locate<DoubleArray,double>( mpDataProvider->index1, x1, mpDataProvider->size1, position );

	// Lower and Upper Boundary Conditions
	if( position == 0 )
	{
		// Lower Bound
		if (mExtrapolationType == FLAT_EXTRAPOLATION_TYPE) 
		{	
			return mpDataProvider->value1[0];
		}
		position++;
	}
	else if( position == mpDataProvider->size1 )
	{
		// Upper Bound
		if (mExtrapolationType == FLAT_EXTRAPOLATION_TYPE)
		{
			return mpDataProvider->value1[position - 1];
		}
		position--;
	}

	// Coordinates:
	const double xLow	= mpDataProvider->index1[position - 1];
	const double yLow	= mpDataProvider->value1[position - 1];
	const double distanceToX1		= x1 - xLow;
	
	// Slope
	Interval interval = getInterval( position );
	const double dydx = intervalSlope( interval );

	// Linear Interpolation Formula
	const double interpolationValue = yLow + ( dydx * distanceToX1 );

	return interpolationValue;
}

// Differentiate the Interpolator at Point x1
double LALinearInterpolation::differentiate(const double & x1) const
{
	AQ_REQUIRE( mpDataProvider != nullptr, "Linear interpolation data has not been set" )
	AQ_REQUIRE( mpDataProvider->size1 > 0, "Linear interpolation data has not been set" )
	
	// Function is flat when there is only one point, therefore slope is zero
	if ( mpDataProvider->size1 == 1 )
	{
		return 0.0;
	}

	// Find x1 Supremum i.e. the next node point relative to x1
	unsigned int position;
	LAAlgorithm::locate<DoubleArray,double>( mpDataProvider->index1, x1, mpDataProvider->size1, position );

	// LOWER BOUND
	if( position == 0 )
	{
		// Lower Bound
		if (mExtrapolationType == FLAT_EXTRAPOLATION_TYPE) 
		{	
			return 0.0;
		}
	}
	
	// UPPER BOUND
	const double maxIndex = mpDataProvider->index1[mpDataProvider->size1-1];
	if( AQ_IS_GREATER_THAN_OR_EQUAL( x1, maxIndex ) )
	{
		if (mExtrapolationType == FLAT_EXTRAPOLATION_TYPE)
		{
			return 0.0;
		}
	}

	// Linear Differentiation Formula
	Interval interval = getInterval( position );
	const double dydx = intervalSlope( interval );
	return dydx;
} 

// Integrate the Interpolator Over the Lower- and UpperBounds
double LALinearInterpolation::integrate(const double & lowerBound, const double & upperBound ) const
{
	AQ_REQUIRE( mpDataProvider != nullptr, "Linear interpolation data has not been set" )
	AQ_REQUIRE( mpDataProvider->size1 > 0, "Linear interpolation data has not been set" )
	AQ_REQUIRE( lowerBound <= upperBound, "Invalid Linear Interpolation Integrand: The Lowerbound must not be greater than the UpperBound")
	
	// The integral result variable
	double integral = 0.0;

	// Boundary Condition: Zero Width
	if( AQ_IS_EQUAL( lowerBound, upperBound ) )
	{
		return 0.0;
	}

	// Boundary Condition: Single Interpolation Point, Assume Flat
	if ( mpDataProvider->size1 == 1 )
	{
		return mpDataProvider->value1[0] * ( upperBound - lowerBound );
	}
	
	// Find lowerBound Supremum i.e. the next node point greater than or equal to the lowerBound - Will throw if less than 2 data points
	// Note we floor at position 1 and disallow 0, since end index cannot be the start index point
	const unsigned int lowerEndIndex = supremum( lowerBound );
	
	// Find upperBound Supremum i.e. the next node point relative to the integration upperBound variable - Will throw if less than 2 data points
	// Note we floor at position 1 and disallow 0, since end index cannot be the start index point 
	const unsigned int upperEndIndex = supremum( upperBound );

	// Integrate over the Interval over the LowerBound- and UpperBound Range
	for( size_t i = lowerEndIndex; i <= upperEndIndex; ++i )
	{
		integral += intervalArea(i, lowerBound, upperBound );
	}
	
	return integral;
} 

/*  @brief set the information of one-dimensional curve
	@param[in] index X-axis value of the data to be interpolated
	@param[in] value Y-axis value of the data to be interpolated
*/
void LALinearInterpolation::set(const DoubleArray& index, const DoubleArray& value)
{
	AQ_REQUIRE(index.size() == value.size(), "Invalid Linear Interpolation Data: The number of x and y points must be the same")

	// Require data is sorted, start at index 1
	for( size_t i = 1; i < index.size(); ++i )
	{
		AQ_REQUIRE( index[i] > index[i-1], "Invalid Linear Interpolation Data: The x values must be must in ascending order with no duplicates")
	}

	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
	}
	mpDataProvider = new LALinearInterpolationDataProvider(index, value, index.size());
	interpolationData_ = std::make_shared<InterpolationData>( index, value );
	return;
};


/* This function allows for direct access to the underlying numbers being interpolated */
std::tuple<std::vector<double>,std::vector<double>> LALinearInterpolation::getXY() const
{
	return std::make_tuple(mpDataProvider->index1, mpDataProvider->value1);
};

// Function to calculate the slope between two consecutive points
double LALinearInterpolation::intervalSlope( const Interval& interval ) const
{
	// Slope
	const double dy		= interpolationData_->yValues_[interval.endIndex_] - interpolationData_->yValues_[interval.startIndex_];
	const double dx		= interpolationData_->xValues_[interval.endIndex_] - interpolationData_->xValues_[interval.startIndex_];
	return dy/dx;
}

// Function to calculate the area for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
double LALinearInterpolation::intervalArea( const size_t & endIndex, const double & lowerBound, const double & upperBound ) const
{
	// Get the Interval or Interval Information
	Interval interval = getInterval( endIndex );

	// Slope
	const double dydx = intervalSlope( interval );
	
	// Interval Width
	const double dx	= intervalWidth( interval, lowerBound, upperBound );

	// Interval Height at Start
	const double startHeight = intervalStartHeight( interval );

	// Integral:
	// ------------------------------------
	// Area(Rectangle)	= startHeight * dx
	// Area(Triangle)	= 0.5 * startHeight * dydx * dx;
	const double integral = startHeight * ( dx + 0.5 * dydx * dx );
	return integral;
}