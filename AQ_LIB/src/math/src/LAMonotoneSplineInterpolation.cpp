/*
 * @brief			Class which defines a 1D MonotoneCubic interpolator
 *
 * @Created:		19 Sept 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "LAMonotoneSplineInterpolation.h"


// @brief Default constructor
LAMonotoneSplineInterpolation::LAMonotoneSplineInterpolation()
	: LAInterpolationBase()
{
	// Default to MonotoneCubic
	monotoneInterpolationType_ = LAMonotoneSplineInterpolation::FRITSCH_BUTLAND;

}

// @brief Constructor allowing the setting of the monotone interpolation sub-type
LAMonotoneSplineInterpolation::LAMonotoneSplineInterpolation( const MonotoneInterpolationEnum monotoneInterpolationType )
	: LAInterpolationBase()
{
	monotoneInterpolationType_ = monotoneInterpolationType;

    if ( ! ( monotoneInterpolationType == LAMonotoneSplineInterpolation::FRITSCH_BUTLAND || monotoneInterpolationType == LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC ) )
	{
        throw LACoreInvalidData("#Error: Unsupported Montone Spline method.", __FILE__, __LINE__);
	}
}


// @brief Destructor
LAMonotoneSplineInterpolation::~LAMonotoneSplineInterpolation()
{}

// @brief Copy constructor
LAMonotoneSplineInterpolation::LAMonotoneSplineInterpolation(const LAMonotoneSplineInterpolation& v)
: LAInterpolationBase(v), monotoneInterpolationType_(v.monotoneInterpolationType_), qlInterpolator_( v.qlInterpolator_), xValues_( v.xValues_ ), yValues_( v.yValues_ )
{
	// Base Class Data
	interpolationData_ = std::make_shared<InterpolationData>( v.xValues_, v.yValues_ );
}

// @brief  Deep copy of this object
LACoreFunctionBase*	LAMonotoneSplineInterpolation::clone() const
{
    try 
	{
		LAMonotoneSplineInterpolation* pTmp;
		pTmp = new LAMonotoneSplineInterpolation();
		pTmp->monotoneInterpolationType_ = monotoneInterpolationType_;

		if ( qlInterpolator_ != nullptr )
		{
			pTmp->qlInterpolator_ = qlInterpolator_;
			pTmp->xValues_ = xValues_;
			pTmp->yValues_ = yValues_;
			
			// Base Class Data
			pTmp->interpolationData_ = std::make_shared<InterpolationData>( pTmp->xValues(), pTmp->yValues() );
		}

		return pTmp;
    }
    catch (std::bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// @brief  Returns whether this class derives from base class with specified type id
bool LAMonotoneSplineInterpolation::isTypeOf(function_t id) const
{
    if ( id == FN_MONOTONESPLINEINTERPOLATION ||  id ==FN_MONOTONEPARABOLICINTERPOLATION )
    {
        return true;
    }
    else
    {
        return  LAInterpolationBase::isTypeOf( id );
    }
}

// @brief Returns the interpolator type
function_t LAMonotoneSplineInterpolation::getType() const
{
	switch (monotoneInterpolationType_)
	{
	case FRITSCH_BUTLAND:
		return FN_MONOTONESPLINEINTERPOLATION;
		break;
	
	case MONOTONE_PARABOLIC:
		return FN_MONOTONEPARABOLICINTERPOLATION;
		break;
	default:
		throw LACoreInvalidData("#Error: Unsupported Monotone-Spline method.", __FILE__, __LINE__);
	}
	
}

/* @brief Returns the value interpolated at the specified point
*  @param[in]	xValue	Specifies the point where to calculate an interpolated value
*/
double LAMonotoneSplineInterpolation::value(const double x1) const
{
	if ( qlInterpolator_ == nullptr )
	{
		throw LACoreInvalidData("#Error: Monotone-Spline interpolation data has not been set", __FILE__, __LINE__);
	}
	
	if (yValues_.size() == 1 ) return yValues_[0];

	double interpValue = 0.0;
	try
	{
		interpValue = (*qlInterpolator_)( x1, true ); // true = allow extrapolation
	}
	catch( std::exception & e)
	{
		std::string errorMsg = "#Error: Problem detected in Monotone-Spline interpolator: ";
		errorMsg += e.what();
		throw LACoreInvalidData( errorMsg.c_str(), __FILE__, __LINE__ );
	}
	return interpValue;
} 

/* @brief	Initializes the interpolator with the specified vectors of X and Y values
*  @param[in]	index	Vector of X values
*  @param[in]	value	Vector of Y values
*/
void LAMonotoneSplineInterpolation::set( const DoubleArray& index, const DoubleArray& value )
{
    if (index.size() != value.size())
	{
        throw LACoreInvalidData("#Error: Monotone-Spline interpolation error, the index size and value size are not same.", __FILE__, __LINE__);
	}
	if (index.size() == 0)
	{
        throw LACoreInvalidData("# Error: Monotone-Spline interpolation error. No data provided.", __FILE__, __LINE__);
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

	xValues_ = index;
	yValues_ = value;

	switch (monotoneInterpolationType_)
	{
	case FRITSCH_BUTLAND:
		// FritschButlandCubic guarantees monotonicity between points, even if the points themselves are not monotonic.
		qlInterpolator_ = std::shared_ptr<QuantLib::Interpolation>( new QuantLib::FritschButlandCubic( xValues_.begin(), xValues_.end(), yValues_.begin() ));
		break;
	
	case MONOTONE_PARABOLIC:
		qlInterpolator_ = std::shared_ptr<QuantLib::Interpolation>( new QuantLib::MonotonicParabolic( xValues_.begin(), xValues_.end(), yValues_.begin() ));
		break;
	default:
		throw LACoreInvalidData("#Error: Unsupported Montone-Spline method.", __FILE__, __LINE__);
	}

	interpolationData_ = std::make_shared<InterpolationData>( index, value );
	return;
};

/* @brief	Returns the pillar points used to initialize the interpolator.
*/
std::tuple<std::vector<double>,std::vector<double>> LAMonotoneSplineInterpolation::getXY() const
{
	return std::make_tuple(xValues_, yValues_);
};

