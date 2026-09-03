/*! @file
    @brief Implementation of base class of the Function class that provides valuation with base date and LAObject as input.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LACoreValuation.h"
#include "LADate.h"
#include "LAObject.h"


/*!
    @brief default constructor
*/
LACoreValuation::LACoreValuation() 
: LACoreFunctionBase()
{
}

/*!
    @brief copy constructor

    @param[in] v not used
*/
LACoreValuation::LACoreValuation(const LACoreValuation& v) : LACoreFunctionBase(v)
{
}

/*!
    @brief destructor
*/
LACoreValuation::~LACoreValuation() 
{
}

/*!
    @brief check whether this class derives from base class with type id

    @param[in] id Function ID to be checked

    @retval true match the specified id
    @retval false not match the specified id
*/
bool
LACoreValuation::isTypeOf(function_t id) const
{
    return (id == FN_VALUATION ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief return Function ID of this class

    @return Function ID
*/
function_t          
LACoreValuation::getType() const
{
    return FN_VALUATION;
}
