/*! @file
    @brief Implementation of base class of the Function class that provides valuation with base date and AQLObject as input.
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLCoreValuation.h"
#include "AQLDate.h"
#include "AQLObject.h"


/*!
    @brief default constructor
*/
AQLCoreValuation::AQLCoreValuation() 
: AQLCoreFunctionBase()
{
}

/*!
    @brief copy constructor

    @param[in] v not used
*/
AQLCoreValuation::AQLCoreValuation(const AQLCoreValuation& v) : AQLCoreFunctionBase(v)
{
}

/*!
    @brief destructor
*/
AQLCoreValuation::~AQLCoreValuation() 
{
}

/*!
    @brief check whether this class derives from base class with type id

    @param[in] id Function ID to be checked

    @retval true match the specified id
    @retval false not match the specified id
*/
bool
AQLCoreValuation::isTypeOf(function_t id) const
{
    return (id == FN_VALUATION ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief return Function ID of this class

    @return Function ID
*/
function_t          
AQLCoreValuation::getType() const
{
    return FN_VALUATION;
}
