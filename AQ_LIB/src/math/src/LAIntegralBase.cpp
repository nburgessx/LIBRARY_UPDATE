/*! @file
    @brief Source code of abstract base class of integration method

	This class has pure virtual method "integrate".
*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAIntegralBase.cpp
//
//  SYNOPSIS    :       LAIntegralBase
//  DESCRIPTION :       Source code of abstract base class of integration method
//						This class has pure virtual method "integrate".
//                      
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAIntegralBase.h"


//================ LAIntegralBase ===================================
/*!
	@brief Default constructor
*/
LAIntegralBase::LAIntegralBase() 
{
}

/*!
	@brief Destructor
*/
LAIntegralBase::~LAIntegralBase() 
{
}

/*!
    @brief Check function for this class type 
    @param[in] id class type to check
    @return true or false
*/
bool
LAIntegralBase::isTypeOf(function_t id) const
{
    return (id == FN_INTEGRALBASE ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return class type
*/
function_t
LAIntegralBase::getType() const
{
    return FN_INTEGRALBASE;  
}
