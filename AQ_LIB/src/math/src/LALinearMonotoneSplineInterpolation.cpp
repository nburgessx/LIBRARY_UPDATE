/*
 * @brief			Class which defines the split-interpolator which has a Linear front section
 *					followed by a MonotoneCubic section. A joinDate specifies the join between the two.
 * @Created:		19 Sept 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#include "LALinearMonotoneSplineInterpolation.h"

// @brief Default constructor
LALinearMonotoneSplineInterpolation::LALinearMonotoneSplineInterpolation() 
	: LAInterpolationBase(), 
		isJoinDateSet_(false), 
		linearInitialised_(false),
		splineInitialised_(false),
		extrapolationType_(LINEAR_EXTRAPOLATION_TYPE)
{
	// Initialize the underlying methods and their data containers aka interpolation LADataProvider
	linearInterpolation_ = std::shared_ptr<LALinearInterpolation>(new LALinearInterpolation(LINEAR_EXTRAPOLATION_TYPE));
	monotoneSplineInterpolation_ = std::shared_ptr<LAMonotoneSplineInterpolation>(new LAMonotoneSplineInterpolation());
}

/* @brief Alternative constructor
*  FLAT_EXTRAPOLATION = Piecewise-Constant Extrapolation
*  LINEAR_EXTRAPOLATION = Linear Extrapolation
*/
LALinearMonotoneSplineInterpolation::LALinearMonotoneSplineInterpolation( const LAMonotoneSplineInterpolation::MonotoneInterpolationEnum monotoneInterpolationType, ExtrapolationType extrapolationType )
	: LAInterpolationBase(), 
		isJoinDateSet_(false), 
		linearInitialised_(false),
		splineInitialised_(false),
		extrapolationType_(extrapolationType)
{
    // Initialize the underlying methods and their data containers aka interpolation LADataProvider
	linearInterpolation_ = std::shared_ptr<LALinearInterpolation>(new LALinearInterpolation(extrapolationType_));
	monotoneSplineInterpolation_ = std::shared_ptr<LAMonotoneSplineInterpolation>(new LAMonotoneSplineInterpolation(monotoneInterpolationType));
}

// @brief Destructor
LALinearMonotoneSplineInterpolation::~LALinearMonotoneSplineInterpolation() 
{}

// @brief Copy constructor
LALinearMonotoneSplineInterpolation::LALinearMonotoneSplineInterpolation( const LALinearMonotoneSplineInterpolation& v )
	: LAInterpolationBase(v),
		extrapolationType_(v.extrapolationType_),
		joinDateAsDouble_(v.joinDateAsDouble_),
		isJoinDateSet_(v.isJoinDateSet_),
		indices_(v.indices_),
		values_(v.values_),
		linearInitialised_(v.linearInitialised_),
		splineInitialised_(v.splineInitialised_)
{
	monotoneSplineInterpolation_ = std::shared_ptr<LAMonotoneSplineInterpolation>(dynamic_cast<LAMonotoneSplineInterpolation*>(v.monotoneSplineInterpolation_->clone()));
    linearInterpolation_         = std::shared_ptr<LALinearInterpolation>(dynamic_cast<LALinearInterpolation*>(v.linearInterpolation_->clone()));

	// Base Class Data
	interpolationData_		= std::make_shared<InterpolationData>(v.indices_, v.values_);
}

// @brief  Deep copy of this object
LACoreFunctionBase*	 LALinearMonotoneSplineInterpolation::clone() const
{
    LALinearMonotoneSplineInterpolation* pTmp = nullptr;
    try 
	{
	    pTmp = new LALinearMonotoneSplineInterpolation( *this );

		// Base Class Data
		pTmp->interpolationData_ = std::make_shared<InterpolationData>( pTmp->indices_, pTmp->values_ );

        return pTmp;
    }
    catch (std::bad_alloc & e)
	{
        delete pTmp;
        throw LACoreSystemError( e.what(), __FILE__, __LINE__ );
    }
	
}

// @brief  Returns whether this class derives from base class with specified type id
bool
LALinearMonotoneSplineInterpolation::isTypeOf(function_t id) const
{
	return ( id == FN_LINEARMONOTONESPLINEINTERPOLATION ? true : LAInterpolationBase::isTypeOf( id ) );
}

// @brief Returns the interpolator type
function_t
LALinearMonotoneSplineInterpolation::getType() const
{
	return FN_LINEARMONOTONESPLINEINTERPOLATION;
}

/* @brief Returns the value interpolated at the specified point
*  @param[in]	xValue	Specifies the point where to calculate an interpolated value
*/
double
LALinearMonotoneSplineInterpolation::value( const double xValue) const
{
    if( xValue < joinDateAsDouble_ )
    {
        return linearInterpolation_->value( xValue );
    }
    else
    {
		if (splineInitialised_)
		{
			return monotoneSplineInterpolation_->value( xValue );
		}
		else
		{
			return linearInterpolation_->value( xValue );
		}
    }
} 

/* @brief	Returns the date (as a double) at which the Linear interpolator joins the MonotoneCubic interpolator
*/
const double LALinearMonotoneSplineInterpolation::getJoinDateAsDouble() const
{
    return joinDateAsDouble_;
}

/*
	This method initialises the class data in the correct order
	Must set the joinDateAsDouble before setting the index and value parameters
*/
void LALinearMonotoneSplineInterpolation::set( const DoubleArray& index, const DoubleArray& value, const double& joinDateAsDouble )
{
	setJoinDateAsDouble( joinDateAsDouble );
	set( index, value );
}

/* @brief	Sets the date (as a double) at which the Linear interpolator joins the MonotoneCubic interpolator
*/
void LALinearMonotoneSplineInterpolation::setJoinDateAsDouble( const double& joinDateAsDouble )
{
    joinDateAsDouble_   = joinDateAsDouble;
    isJoinDateSet_      = true;
}

/* @brief	Initializes the interpolator with the specified vectors of X and Y values
*  @param[in]	index	Vector of X values
*  @param[in]	value	Vector of Y values
*/
void LALinearMonotoneSplineInterpolation::set( const DoubleArray& index, const DoubleArray& value )
{
    // This parameter is set in the 'setLinearInterpolationEndDateAsDouble' method above.
    if( !isJoinDateSet_ )
    {
        throw LACoreInvalidData("#Error: Join date has not been set in Linear Monotone-Spline interpolation scheme", __FILE__, __LINE__);
    }

    if( index.size() != value.size() )
    {
        throw LACoreInvalidData("#Error: Interpolation index size and value sizes are not same in Linear Monotone-Spline interpolation scheme.", __FILE__, __LINE__);
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
			std::shared_ptr<LAMonotoneSplineInterpolation> transitionInter(new LAMonotoneSplineInterpolation());
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
        monotoneSplineInterpolation_->set( splineIndex, splineValue );
		splineInitialised_ = true;
    }
    
	interpolationData_ = std::make_shared<InterpolationData>( index, value );
	return;
};

/* @brief	Returns the pillar points used to initialize the interpolator.
*/
std::tuple<std::vector<double>,std::vector<double>> LALinearMonotoneSplineInterpolation::getXY() const
{
	return std::make_tuple( indices_, values_ );
};

