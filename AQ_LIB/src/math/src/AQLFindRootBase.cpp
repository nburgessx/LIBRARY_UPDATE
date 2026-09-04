/*! @file
    @brief Definition of abstract base class for dataInstance finding.

	   

*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLFindRootBase.h"
//================ AQLOptimumOption ===================================
/*!
	@brief Default constructor
*/
AQLFindRootOption::AQLFindRootOption() 
:mPrecision(ROOT_PRECISION), mIterMax(ROOT_ITMAX), mException(true)

{
}

/*!
	@brief Destructor
*/
AQLFindRootOption::~AQLFindRootOption() 
{
}


//================ AQLFindRootBase ===================================
/*!
	@brief Default constructor
*/
AQLFindRootBase::AQLFindRootBase() 
{
}

/*!
	@brief Destructor
*/
AQLFindRootBase::~AQLFindRootBase() 
{
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLFindRootBase::isTypeOf(function_t id) const
{
    return (id == FN_FINDROOTBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLFindRootBase::getType() const
{
    return FN_FINDROOTBASE;  
}

