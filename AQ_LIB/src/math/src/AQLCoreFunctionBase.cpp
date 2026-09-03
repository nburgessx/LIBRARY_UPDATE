/*! @file
    @brief Implementation of virtual base class for valuation, intepolation, procedure etc.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLCoreFunctionBase.h"

/*!
    @brief default constructor
*/
AQLCoreFunctionBase::AQLCoreFunctionBase(void)
{
}

/*!
    @brief destructor
*/
AQLCoreFunctionBase::~AQLCoreFunctionBase()
{
}

/*!
    @brief check whether this class derives from base class with type id
	
	@param[in] id Function ID to be checked

    @retval true match the specified id
    @retval false not match the specified id
*/
bool
AQLCoreFunctionBase::isTypeOf(function_t id) const
{
    return id == FN_BASE;
}

/*!
    @brief return Function ID of this class

    @return Function ID
*/
function_t
AQLCoreFunctionBase::getType() const 
{
    return FN_BASE;
}

//////////// PROTECTED /////////////////
/*!
    @brief copy constructor

    @param[in] eq not used
*/
AQLCoreFunctionBase::AQLCoreFunctionBase(const AQLCoreFunctionBase& eq)
{
	(void)eq;
}
