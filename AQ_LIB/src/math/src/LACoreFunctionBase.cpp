/*! @file
    @brief Implementation of virtual base class for valuation, intepolation, procedure etc.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LACoreFunctionBase.h"

/*!
    @brief default constructor
*/
LACoreFunctionBase::LACoreFunctionBase(void)
{
}

/*!
    @brief destructor
*/
LACoreFunctionBase::~LACoreFunctionBase()
{
}

/*!
    @brief check whether this class derives from base class with type id
	
	@param[in] id Function ID to be checked

    @retval true match the specified id
    @retval false not match the specified id
*/
bool
LACoreFunctionBase::isTypeOf(function_t id) const
{
    return id == FN_BASE;
}

/*!
    @brief return Function ID of this class

    @return Function ID
*/
function_t
LACoreFunctionBase::getType() const 
{
    return FN_BASE;
}

//////////// PROTECTED /////////////////
/*!
    @brief copy constructor

    @param[in] eq not used
*/
LACoreFunctionBase::LACoreFunctionBase(const LACoreFunctionBase& eq)
{
	(void)eq;
}
