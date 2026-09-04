/*! @file
    @brief Source code of abstruct base class of interpolation of path element
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesPEInterpolationBase.h"


using namespace std;
//================ AQLRatesPEInterpolationBase ===================================
/*!
	@brief default constructor

*/
AQLRatesPEInterpolationBase::AQLRatesPEInterpolationBase()
{

}
/*!
	@brief copy constructor
*/
AQLRatesPEInterpolationBase::AQLRatesPEInterpolationBase(const AQLRatesPEInterpolationBase& v) 
: AQLCoreFunctionBase(v)
{

}

/*!
	@brief destructor
*/
AQLRatesPEInterpolationBase::~AQLRatesPEInterpolationBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPEInterpolationBase::isTypeOf(function_t id) const
{
	return (id==FN_PEINTERPOLATIONBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesPEInterpolationBase::getType() const
{
	return FN_PEINTERPOLATIONBASE;
}


