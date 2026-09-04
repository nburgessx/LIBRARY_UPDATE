// AQLStepInterpolation.cpp

/* @file
    @brief Source code of class for interpolation with step function representing left- and right-continuous functions
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLStepInterpolation.h"
#include "AQLAlgorithm.h"
#include <algorithm>
#include <limits>
#include <utility>
#include <iterator> 

using namespace std;

/*
    @brief constructor
	@param[in] steptype step type
*/
AQLStepInterpolation::AQLStepInterpolation(StepType steptype) 
: AQLInterpolationBase(), mStepType(steptype), mpDataProvider(NULL)
{

}
/*
    @brief destructor
*/
AQLStepInterpolation::~AQLStepInterpolation() 
{
	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
	}
}

/*
    @brief copy constructor
*/
AQLStepInterpolation::AQLStepInterpolation(const AQLStepInterpolation& v)
: AQLInterpolationBase(v), mpDataProvider(NULL)
{
	mStepType = v.mStepType;	
    try 
	{	
		mpDataProvider = new AQLStepInterpolationDataProvider(*v.mpDataProvider);

		// Base Class Data
		interpolationData_ = std::make_shared<InterpolationData>( mpDataProvider->index1, mpDataProvider->value1 );
	}
    catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	AQLStepInterpolation::clone() const
{
    try 
	{
		AQLStepInterpolation* pTmp;
		pTmp = new AQLStepInterpolation(mStepType);

		if (mpDataProvider != nullptr )
		{
			pTmp->mpDataProvider = new AQLStepInterpolationDataProvider(this->mpDataProvider->index1, this->mpDataProvider->value1, this->mpDataProvider->size1);
			
			// Base Class Data
			pTmp->interpolationData_ = std::make_shared<InterpolationData>( pTmp->mpDataProvider->index1, pTmp->mpDataProvider->value1 );
		}

		

		return pTmp;
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
	
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool AQLStepInterpolation::isTypeOf(function_t id) const
{
	return (id == FN_STEPINTERPOLATION ? true : AQLInterpolationBase::isTypeOf(id));
}

/*
    @brief Return this function type
    @return function type
*/
function_t AQLStepInterpolation::getType() const
{
	return FN_STEPINTERPOLATION;
}

/*
    @brief			Return intepolated value for 1D curve
	@param[in]		x1 point to be interpolated
	@return			interpolated value
*/
double AQLStepInterpolation::value(const double x1) const
{
	// Data Class Uninitialized or Empty
	if (mpDataProvider == nullptr || mpDataProvider->size1 == 0 )
	{
        throw AQLCoreInvalidData("#Error: Step interpolation data has not been set", __FILE__, __LINE__);
	}

	// If data number is one, then return y value as interpolated value
	if ( mpDataProvider->size1 == 1 ) return mpDataProvider->value1[0];

	// std::lower_bound = Greater than or Equal to
	// std::upper_bound = Strictly Greater than
	auto it_GreaterThanOrEqualNodePoint = std::lower_bound( ( mpDataProvider->index1 ).cbegin(), ( mpDataProvider->index1 ).cend(), x1 ); 
	auto it_GreaterThanNodePoint		= std::upper_bound( ( mpDataProvider->index1 ).cbegin(), ( mpDataProvider->index1 ).cend(), x1 );

	// The input x1 is a node point if the lower_bound iterator != upper_bound_iterator
	const bool isNodePoint = (it_GreaterThanOrEqualNodePoint != it_GreaterThanNodePoint);

	// Boundary Condition: If at first Index Position
	if( it_GreaterThanOrEqualNodePoint == (mpDataProvider->index1).cbegin() )
    {
        return mpDataProvider->value1[0];
    }
	// Boundary Condition: If at last Index Position
    else if( it_GreaterThanOrEqualNodePoint == ( mpDataProvider->index1 ).cend() )
    {
        return mpDataProvider->value1[ mpDataProvider->value1.size() - 1 ];
    }
    else
    {
		auto indexOfGreaterThanOrEqualNodePoint = std::distance( ( mpDataProvider->index1 ).cbegin(), it_GreaterThanOrEqualNodePoint );
		if(mStepType == RIGHT_CONTINUOUS)
		{
            // RIGHT-CONTINUOUS: Use the greater than or equal to the node point - 1 unless we are at a node point whereby we use the greater than or equal to the node point
			return ( isNodePoint ) ? mpDataProvider->value1[indexOfGreaterThanOrEqualNodePoint] : mpDataProvider->value1[indexOfGreaterThanOrEqualNodePoint - 1];
		}
        else 
		{
			// LEFT-CONTINUOUS: Use the greater than or equal to the node point
			return  mpDataProvider->value1[indexOfGreaterThanOrEqualNodePoint];
		}
    }        
} 

// Differentiate the Interpolator at Point x1
double AQLStepInterpolation::differentiate(const double & x1) const
{
	// For Step Interpolation the Slope is always zero
	return 0.0;
} 

// Integrate the Interpolator Over the Lower- and UpperBounds
double AQLStepInterpolation::integrate(const double & lowerBound, const double & upperBound ) const
{
	AQ_REQUIRE( mpDataProvider != nullptr, "Step interpolation data has not been set" )
	AQ_REQUIRE( mpDataProvider->size1 > 0, "Step interpolation data has not been set" )
	AQ_REQUIRE( lowerBound <= upperBound, "Invalid Step Interpolation Integrand: The Lowerbound must not be greater than the UpperBound")
	
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

/*
    @brief Set function data
	@param[in] index points at x coordinate
	@param[in] value values at y coordinate
*/
void AQLStepInterpolation::set(const DoubleArray& index,const DoubleArray& value)
{
	if (index.size() != value.size())
	{
		throw AQLCoreInvalidData("#Error: Number of X Data Points does not match the number of Y Value Points", __FILE__, __LINE__);
	}
	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
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
			AQ_THROW_IF(thisIndex == previousIndex, "Invalid Interpolation Data: Duplicate data found with time value: " + AQ_TO_STRING_FROM_DOUBLE(thisIndex) + " years" )
		}
	}

	mpDataProvider = new AQLStepInterpolationDataProvider(index, value, index.size());
	interpolationData_ = std::make_shared<InterpolationData>( index, value );
	return;
};

std::tuple<std::vector<double>,std::vector<double>> AQLStepInterpolation::getXY() const
{
	return std::make_tuple(mpDataProvider->index1, mpDataProvider->value1);
};

// Function to calculate the slope between two consecutive points
double AQLStepInterpolation::intervalSlope( const Interval& interval ) const
{
	// Slope
	return 0.0;
}

// Function to calculate the area for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
double AQLStepInterpolation::intervalArea( const size_t & endIndex, const double & lowerBound, const double & upperBound ) const
{
	// Get the Interval or Interval Information
	Interval interval = getInterval( endIndex );

	// Interval Width
	const double dx	= intervalWidth( interval, lowerBound, upperBound );

	// Interval Height: Use End Height if LEFT_CONTINUOUS and Start Height if RIGHT_CONTINUOUS
	double height = 0.0;
	switch ( mStepType )
	{
		case RIGHT_CONTINUOUS:
		{
			height = intervalStartHeight( interval );
			break;
		}
		case LEFT_CONTINUOUS:
		{
			height = intervalEndHeight( interval );
			break;
		}
		default:
		{
			AQ_THROW( "Invalid Step Interpolation: Piecewise constant interpolation must be specified as LEFT_CONTINUOUS or RIGHT_CONTINUOUS")
		}

	}

	// Integral:
	// ------------------------------------
	// Area(Rectangle)	= height * dx
	const double integral = height * dx;
	return integral;
}