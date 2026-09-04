/*! @file
    @brief Source code of abstract base class of integration method

	This class has pure virtual method "integrate".
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLIntegralBase.h"


//================ AQLIntegralBase ===================================
/*!
	@brief Default constructor
*/
AQLIntegralBase::AQLIntegralBase() 
{
}

/*!
	@brief Destructor
*/
AQLIntegralBase::~AQLIntegralBase() 
{
}

/*!
    @brief Check function for this class type 
    @param[in] id class type to check
    @return true or false
*/
bool
AQLIntegralBase::isTypeOf(function_t id) const
{
    return (id == FN_INTEGRALBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return class type
*/
function_t
AQLIntegralBase::getType() const
{
    return FN_INTEGRALBASE;  
}
