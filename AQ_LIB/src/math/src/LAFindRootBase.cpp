/*! @file
    @brief Definition of abstract base class for dataInstance finding.

	   

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAFindRootBase.cpp
//
//  SYNOPSIS    :       LAFindRootBase
//  DESCRIPTION :       Definitioin of abstract base class for dataInstance finding.
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


#include "LAFindRootBase.h"
//================ LAOptimumOption ===================================
/*!
	@brief Default constructor
*/
LAFindRootOption::LAFindRootOption() 
:mPrecision(ROOT_PRECISION), mIterMax(ROOT_ITMAX), mException(true)

{
}

/*!
	@brief Destructor
*/
LAFindRootOption::~LAFindRootOption() 
{
}


//================ LAFindRootBase ===================================
/*!
	@brief Default constructor
*/
LAFindRootBase::LAFindRootBase() 
{
}

/*!
	@brief Destructor
*/
LAFindRootBase::~LAFindRootBase() 
{
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAFindRootBase::isTypeOf(function_t id) const
{
    return (id == FN_FINDROOTBASE ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAFindRootBase::getType() const
{
    return FN_FINDROOTBASE;  
}

