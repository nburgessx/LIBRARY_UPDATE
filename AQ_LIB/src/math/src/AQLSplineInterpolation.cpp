// AQLSplineInterpolation.cpp

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLSplineInterpolation.h"

using namespace std;

// Const parameter to help avoid division operators for performance
const double oneDividedbySix = 0.166666666666666666666666666666666666666666666666666666666666666666666666666666666667;

//================ AQLInterpolationBase ===================================
/*!
    @brief constructor
*/
AQLSplineInterpolation::AQLSplineInterpolation()
: AQLInterpolationBase()
{
	mpDataProvider=NULL;
    useNaturalSpline_ = true; // Default Behaviour: True = Natural Spline
}

/*!
    @brief constructor allowing the setting of the 'useNaturalSpline' constructor
*/
AQLSplineInterpolation::AQLSplineInterpolation( const bool useNaturalSpline )
: AQLInterpolationBase()
{
	mpDataProvider=NULL;
    useNaturalSpline_ = useNaturalSpline; // True = Use Natural Spline, False = Use Clamped Spline
}

/*!
    @brief destructor
*/
AQLSplineInterpolation::~AQLSplineInterpolation()
{
	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
	}
}
/*!
    @brief copy constructor
*/
AQLSplineInterpolation::AQLSplineInterpolation(const AQLSplineInterpolation& v)
: AQLInterpolationBase(v), mpDataProvider(NULL)
{
    try 
	{	
        useNaturalSpline_ = v.useNaturalSpline_;
		mpDataProvider = new AQLSplineInterpolationDataProvider(*v.mpDataProvider);

		// Base Class Data
		interpolationData_ = std::make_shared<InterpolationData>( mpDataProvider->index, mpDataProvider->value );
	}
    catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
    @brief clone
*/
AQLCoreFunctionBase*	
AQLSplineInterpolation::clone() const
{
    try 
	{
		AQLSplineInterpolation* pTmp;
		pTmp = new AQLSplineInterpolation;
        pTmp->useNaturalSpline_ = this->useNaturalSpline_;

		if (mpDataProvider != NULL)
		{
			pTmp->mpDataProvider = new AQLSplineInterpolationDataProvider(this->mpDataProvider->index ,this->mpDataProvider->value ,this->mpDataProvider->coeff ,this->mpDataProvider->size);
			
			// Base Class Data	
			pTmp->interpolationData_ = std::make_shared<InterpolationData>( pTmp->mpDataProvider->index, pTmp->mpDataProvider->value );
		}

		

		return pTmp;
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief  check whether this class derives from base class with type id

	@param[in] id ID to check
	@return True or false
*/
bool
AQLSplineInterpolation::isTypeOf(function_t id) const
{
    if ( id == FN_SPLINEINTERPOLATION || id == FN_NATURALSPLINEINTERPOLATION || id == FN_CLAMPEDSPLINEINTERPOLATION )
    {
        return true;
    }
    else
    {
        return  AQLInterpolationBase::isTypeOf( id );
    }
}

/*!
    @brief class type
	@return class type
*/
function_t
AQLSplineInterpolation::getType() const
{
    if ( useNaturalSpline_ == false )
    {
        return FN_CLAMPEDSPLINEINTERPOLATION;
    }

    // Default Spline Type is Natural Spline ... Note Natural Spline is an alias for the default SPLINE
	return FN_SPLINEINTERPOLATION;
}

/*!
    @brief a value of one-dimensional complement of the curve
	@param[in] x1
	@return a value of one-dimensional complement of the curve
*/
double
AQLSplineInterpolation::value(const double x1) const
{
	if (mpDataProvider == NULL)
	{
		throw AQLCoreInvalidData("#Error: Spline interpolation data has not been set", __FILE__, __LINE__);
	}

	if ( mpDataProvider->size == 1) return mpDataProvider->value[0];

	int klo,khi,k;
	double h,b,a;
	
	// Set k_low and k_high
	klo=1;
	khi = mpDataProvider->size;
	
	// Biscection Search for the node points either side of our search value
	// Works well with unordered data
	while(khi - klo > 1)
	{
		k = (khi + klo) >> 1;
		if (mpDataProvider->index[k - 1] > x1) khi = k;
		else klo = k;
	}
    
	// Set data provider variables
	const double index_klo = mpDataProvider->index[klo-1];
	const double index_khi = mpDataProvider->index[khi-1];
	const double value_klo = mpDataProvider->value[klo-1];
	const double value_khi = mpDataProvider->value[khi-1];
	const double coeff_klo = mpDataProvider->coeff[klo-1];
	const double coeff_khi = mpDataProvider->coeff[khi-1];

	// Set h and h_inverse
	h = index_khi - index_klo;
	AQ_REQUIRE( !AQ_IS_EQUAL_ZERO( h ), "Invalid Spline Interpolation Data: Unable to Calculate Spline Value" )
	const double inverse_h = 1.0/h;

	//Numerical Recipes in C
	a = ( index_khi - x1 ) * inverse_h;
	b = ( x1 - index_klo ) * inverse_h;

	// Avoid division operators for performance, since this method is heavily used in nested functions
	const double result = a*value_klo + b*value_khi + ( (a*a*a-a)*coeff_klo + (b*b*b-b)*coeff_khi ) * (h*h*oneDividedbySix);
	
	return result;
} 

// Differentiate the Interpolator at Point x1
double AQLSplineInterpolation::differentiate(const double & x1) const
{
	AQ_REQUIRE( interpolationData_ != nullptr, "Spline interpolation data has not been set" )
	AQ_REQUIRE( interpolationData_->size_ > 0, "Spline interpolation data has not been set" )
	
	// Function is flat when there is only one point, therefore slope is zero
	if ( interpolationData_->size_ == 1 )
	{
		return 0.0;
	}

	// Find x1 Supremum i.e. the next node point relative to x1
	const unsigned int position = supremum( x1 );

	// Differentiation Formula
	Interval interval = getInterval( position );
	const double dydx = intervalSlope( interval, x1 );
	return dydx;
} 

// Integrate the Interpolator Over the Lower- and UpperBounds
double AQLSplineInterpolation::integrate(const double & lowerBound, const double & upperBound ) const
{
	AQ_REQUIRE( interpolationData_ != nullptr, "Spline interpolation data has not been set" )
	AQ_REQUIRE( interpolationData_->size_ > 0, "Spline interpolation data has not been set" )
	AQ_REQUIRE( lowerBound <= upperBound, "Invalid Spline Interpolation Integrand: The Lowerbound must not be greater than the UpperBound")
	
	// The integral result variable
	double integral = 0.0;

	// Boundary Condition: Zero Width
	if( AQ_IS_EQUAL( lowerBound, upperBound ) )
	{
		return 0.0;
	}

	// Boundary Condition: Single Interpolation Point, Assume Flat
	if ( interpolationData_->size_ == 1 )
	{
		return interpolationData_->yValues_[0] * ( upperBound - lowerBound );
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

/*!
@brief set the information of one-dimensional curve
@param[in] index X-axis value of the data to be interpolated
@param[in] value Y-axis value of the data to be interpolated
*/
void
AQLSplineInterpolation::set( const DoubleArray& index, const DoubleArray& value )
{
    // Important Note:
    // ---------------
    // This method has been implemented as per Numerical Recipess 2ed p115
    // Note: Original author placed minus one in the index everywhere because the original source code is from Fortran having index base 1

    if (index.size() != value.size())
	{
        throw AQLCoreInvalidData("#Error: Spline interpolation error, the index size and value size are not same.", __FILE__, __LINE__);
	}
	if (index.size() == 0)
	{
        throw AQLCoreInvalidData("# Error: Spline interpolation error. No data provided.", __FILE__, __LINE__);
	}
	if (index.size() == 1) return;

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

	unsigned int i,k;                   
	double p,qn,sig,tmpn;               
	unsigned int size = index.size();
	DoubleArray tmp(1);                 
	DoubleArray secDev(1);              
	tmp.resize(size);	                
	secDev.resize(size);                

    // Set First and Second Derivatives for the start node
    if ( useNaturalSpline_ )
    {
		// First & Second Derivative at start node
		tmp[0]=0.0;
        secDev[0]=0.0;
    }
    else
    {
        // Set Clamped Spline Start Derivatives
        const double height1    = value[1] - value[0];
        const double width1     = index[1] - index[0];

        // Divide by zero guard
        if ( width1 == 0 )
        {
            throw AQLCoreInvalidData("#Error: Invalid spline first derivative at start-point. The underlying data is invalid having an infinite slope.", __FILE__, __LINE__);
        }

        const double slope1     = height1 / width1;
        tmp[0]                  = ( 3.0 / width1 ) * ( height1 / width1 - slope1 );     // First Derivative at start node
        secDev[0]               = -0.5;                                                 // Second Derivative at start node
    }

	// LU Decomposition See Discription Numerical Recipes in C.
	for (i=2;i<=size-1;i++)
	{
		sig=(index[i-1]-index[i-1-1])/(index[i+1-1]-index[i-1-1]);
		p=sig*secDev[i-1-1]+2.0;
		secDev[i-1]=(sig-1.0)/p;
		tmp[i-1]=(value[i+1-1]-value[i-1])/(index[i+1-1]-index[i-1])-(value[i-1]-value[i-1-1])/(index[i-1]-index[i-1-1]);
		tmp[i-1]=(6.0*tmp[i-1]/(index[i+1-1]-index[i-1-1])-sig*tmp[i-1-1])/p;
	}
	
    // Set First and Second Derivatives for the end node
    if ( useNaturalSpline_ )
    {
		// First Derivative
		qn=0.0;
        tmpn=0.0;    
    }
    else
    {
        // Set Clamped Spline Start Derivatives
        const double heightN    = value[size-1] - value[size-2];
        const double widthN     = index[size-1] - index[size-2];
        
        // Divide by zero guard
        if ( widthN == 0 )
        {
            throw AQLCoreInvalidData("#Error: Invalid spline first derivative at end-point. The underlying data is invalid having an infinite slope.", __FILE__, __LINE__);
        }
        
        const double slopeN     = heightN / widthN;
        qn                      = 0.5;      
        tmpn					= ( 3.0 / widthN ) * ( slopeN - heightN / widthN ); // First Derivative at end node
    }

    // Second Derivative at end node
    secDev[size-1]=(tmpn-qn*tmp[size-1-1])/(qn*secDev[size-1-1]+1.0);
	
    // Back Substitution Algorithm
    for (k=size-1;k>=1;k--)
	{
        secDev[k-1]=secDev[k-1]*secDev[k+1-1]+tmp[k-1];
	}
    
	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
	}
	
	mpDataProvider = new AQLSplineInterpolationDataProvider(index, value, secDev, size);
	interpolationData_ = std::make_shared<InterpolationData>( index, value );
	return;
};

std::tuple<std::vector<double>,std::vector<double>> AQLSplineInterpolation::getXY() const
{
	return std::make_tuple(mpDataProvider->index, mpDataProvider->value);
};

// Function to calculate the slope between two consecutive points
double AQLSplineInterpolation::intervalSlope( const Interval& interval, const double& x1) const
{
	int klo, khi;
	double h, b, a, dydx;

	klo = interval.startIndex_;
	khi = interval.endIndex_;

	h = mpDataProvider->index[khi] - mpDataProvider->index[klo];
	AQ_REQUIRE( !AQ_IS_EQUAL_ZERO( h ), "Invalid Spline Interpolation Data: Unable to Calculate Spline Inteval Slope" )
	const double inverse_h = 1.0/h;

	a = mpDataProvider->index[khi] - x1;
	b = x1 - mpDataProvider->index[klo];

	dydx = - mpDataProvider->value[klo] + mpDataProvider->value[khi] + ((-3*a*a + h*h)*mpDataProvider->coeff[klo] + (3*b*b - h*h)*mpDataProvider->coeff[khi]) * oneDividedbySix;
	return dydx * inverse_h;
}

// Function to calculate the area for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
double AQLSplineInterpolation::intervalArea( const size_t & endIndex, const double & lowerBound, const double & upperBound ) const
{
	double h, x_hi, x_lo, b_hi, b_lo, a_hi, a_lo, b3_hi, b3_lo, a3_hi, a3_lo;

	const Interval interval = getInterval( endIndex );
	auto intervalStart = interval.startIndex_;
	auto intervalEnd = interval.endIndex_;

	h = mpDataProvider->index[intervalEnd] - mpDataProvider->index[intervalStart];

	AQ_REQUIRE( !AQ_IS_EQUAL_ZERO( h ), "Invalid Spline Interpolation Data: Unable to Calculate Spline Interval Area" )
	const double inverse_h = 1.0/h;
	const double inverse_four_h_cubed = 0.25 * inverse_h * inverse_h * inverse_h; // = 1.0/(4.0*h*h*h);

	// if the segment is not stub
	x_lo = mpDataProvider->index[intervalStart]; 
	x_hi = mpDataProvider->index[intervalEnd]; 

	// lower stub
	if (x_lo < lowerBound)
		x_lo = lowerBound;

	// upper stub
	if (x_hi > upperBound)
		x_hi = upperBound;

	// Set data provider variables
	const double index_klo = mpDataProvider->index[intervalStart];
	const double index_khi = mpDataProvider->index[intervalEnd];
	const double value_klo = mpDataProvider->value[intervalStart];
	const double value_khi = mpDataProvider->value[intervalEnd];
	const double coeff_klo = mpDataProvider->coeff[intervalStart];
	const double coeff_khi = mpDataProvider->coeff[intervalEnd];

	// This method is used in several deeply nested loops
	// Therefore we avoid division and power operators for performance
	
	a_lo = (index_khi*x_lo - 0.5*x_lo*x_lo) * inverse_h;
	a_hi = (index_khi*x_hi - 0.5*x_hi*x_hi) * inverse_h;

	b_lo = (0.5*x_lo*x_lo - index_klo*x_lo) * inverse_h;
	b_hi = (0.5*x_hi*x_hi - index_klo*x_hi) * inverse_h;

	const double index_khi_minus_xlo_power_four = ( index_khi - x_lo ) * ( index_khi - x_lo ) * ( index_khi - x_lo ) * ( index_khi - x_lo );
	a3_lo = - index_khi_minus_xlo_power_four * inverse_four_h_cubed;

	const double index_khi_minus_xhi_power_four = ( index_khi - x_hi ) * ( index_khi - x_hi ) * ( index_khi - x_hi ) * ( index_khi - x_hi );
	a3_hi = - index_khi_minus_xhi_power_four  * inverse_four_h_cubed;

	const double xlo_minus_index_klo_power_four = ( x_lo - index_klo ) * ( x_lo - index_klo ) * ( x_lo - index_klo ) * ( x_lo - index_klo );
	b3_lo = xlo_minus_index_klo_power_four * inverse_four_h_cubed;
	
	const double xhi_minus_index_klo_power_four = ( x_hi - index_klo ) * ( x_hi - index_klo ) * ( x_hi - index_klo ) * ( x_hi - index_klo );
	b3_hi = xhi_minus_index_klo_power_four * inverse_four_h_cubed;

	double the_area = (a_hi - a_lo) * value_klo + (b_hi - b_lo) * value_khi;
	the_area += ( ((a3_hi - a3_lo) - (a_hi - a_lo)) * coeff_klo + ((b3_hi - b3_lo) - (b_hi - b_lo))* coeff_khi ) * (h*h*oneDividedbySix);

	return the_area;
}