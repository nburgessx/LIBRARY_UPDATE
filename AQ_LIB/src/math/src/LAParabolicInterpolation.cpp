// LAParabolicInterpolation.cpp

#ifdef __GNUG__
    #pragma implementation
#else
    #pragma warning(disable:4786)
#endif

#include "LAParabolicInterpolation.h"

using namespace std;

//================ LAInterpolationBase ===================================
/*!
    @brief constructor
*/
LAParabolicInterpolation::LAParabolicInterpolation()
: LAInterpolationBase()
{
	mpDataProvider=NULL;
}


/*!
    @brief destructor
*/
LAParabolicInterpolation::~LAParabolicInterpolation()
{
	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
	}
}
/*!
    @brief copy constructor
*/
LAParabolicInterpolation::LAParabolicInterpolation(const LAParabolicInterpolation& v)
: LAInterpolationBase(v), mpDataProvider(NULL)
{
    try 
	{	
		mpDataProvider = new LAParabolicInterpolationDataProvider(*v.mpDataProvider);

		// Base Class Data
		interpolationData_ = std::make_shared<InterpolationData>( mpDataProvider->index, mpDataProvider->value );
	}
    catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
    @brief clone
*/
LACoreFunctionBase*	
LAParabolicInterpolation::clone() const
{
    try 
	{
		LAParabolicInterpolation* pTmp;
		pTmp = new LAParabolicInterpolation;

		if (mpDataProvider != NULL)
		{
			pTmp->mpDataProvider = new LAParabolicInterpolationDataProvider(this->mpDataProvider->index ,this->mpDataProvider->value ,this->mpDataProvider->coeff ,this->mpDataProvider->size);

			// Base Class Data
			pTmp->interpolationData_ = std::make_shared<InterpolationData>( pTmp->mpDataProvider->index, pTmp->mpDataProvider->value );
		}

		return pTmp;
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief  check whether this class derives from base class with type id

	@param[in] id ID to check
	@return True or false
*/
bool
LAParabolicInterpolation::isTypeOf(function_t id) const
{
    if ( id == FN_PARABOLICINTERPOLATION ) 
    {
        return true;
    }
    else
    {
        return  LAInterpolationBase::isTypeOf( id );
    }
}

/*!
    @brief class type
	@return class type
*/
function_t
LAParabolicInterpolation::getType() const
{
	return FN_PARABOLICINTERPOLATION;
}

/*!
    @brief a value of one-dimensional complement of the curve
	@param[in] x1
	@return a value of one-dimensional complement of the curve
*/
double
LAParabolicInterpolation::value(const double x1) const
{
	throw LACoreInvalidData("#Error: Parabolic Interpolation under development and not currently supported.", __FILE__, __LINE__);
    return 0.0;
} 

	/*!
    @brief set the information of one-dimensional curve
	@param[in] index X-axis value of the data to be interpolated
	@param[in] value Y-axis value of the data to be interpolated
	*/
void
LAParabolicInterpolation::set( const DoubleArray& index, const DoubleArray& value )
{
    throw LACoreInvalidData("#Error: Parabolic Interpolation under development and not currently supported.", __FILE__, __LINE__);
	interpolationData_ = std::make_shared<InterpolationData>( index, value );
	return;
};

std::tuple<std::vector<double>,std::vector<double>> LAParabolicInterpolation::getXY() const
{
	return std::make_tuple(mpDataProvider->index, mpDataProvider->value);
};
