// AQLParabolicInterpolation.cpp

#ifdef __GNUG__
    #pragma implementation
#else
    #pragma warning(disable:4786)
#endif

#include "AQLParabolicInterpolation.h"

using namespace std;

//================ AQLInterpolationBase ===================================
/*!
    @brief constructor
*/
AQLParabolicInterpolation::AQLParabolicInterpolation()
: AQLInterpolationBase()
{
	mpDataProvider=NULL;
}


/*!
    @brief destructor
*/
AQLParabolicInterpolation::~AQLParabolicInterpolation()
{
	if (mpDataProvider != NULL)
	{
		delete mpDataProvider;
	}
}
/*!
    @brief copy constructor
*/
AQLParabolicInterpolation::AQLParabolicInterpolation(const AQLParabolicInterpolation& v)
: AQLInterpolationBase(v), mpDataProvider(NULL)
{
    try 
	{	
		mpDataProvider = new AQLParabolicInterpolationDataProvider(*v.mpDataProvider);

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
AQLParabolicInterpolation::clone() const
{
    try 
	{
		AQLParabolicInterpolation* pTmp;
		pTmp = new AQLParabolicInterpolation;

		if (mpDataProvider != NULL)
		{
			pTmp->mpDataProvider = new AQLParabolicInterpolationDataProvider(this->mpDataProvider->index ,this->mpDataProvider->value ,this->mpDataProvider->coeff ,this->mpDataProvider->size);

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
AQLParabolicInterpolation::isTypeOf(function_t id) const
{
    if ( id == FN_PARABOLICINTERPOLATION ) 
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
AQLParabolicInterpolation::getType() const
{
	return FN_PARABOLICINTERPOLATION;
}

/*!
    @brief a value of one-dimensional complement of the curve
	@param[in] x1
	@return a value of one-dimensional complement of the curve
*/
double
AQLParabolicInterpolation::value(const double x1) const
{
	throw AQLCoreInvalidData("#Error: Parabolic Interpolation under development and not currently supported.", __FILE__, __LINE__);
    return 0.0;
} 

	/*!
    @brief set the information of one-dimensional curve
	@param[in] index X-axis value of the data to be interpolated
	@param[in] value Y-axis value of the data to be interpolated
	*/
void
AQLParabolicInterpolation::set( const DoubleArray& index, const DoubleArray& value )
{
    throw AQLCoreInvalidData("#Error: Parabolic Interpolation under development and not currently supported.", __FILE__, __LINE__);
	interpolationData_ = std::make_shared<InterpolationData>( index, value );
	return;
};

std::tuple<std::vector<double>,std::vector<double>> AQLParabolicInterpolation::getXY() const
{
	return std::make_tuple(mpDataProvider->index, mpDataProvider->value);
};
