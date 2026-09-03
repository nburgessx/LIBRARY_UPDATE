/*! @file
    @brief Implementation of base class of the Function class that provides procedure with base date and LAObject as input.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LACoreProcedure.h"
#include "LADate.h"
#include "LAObject.h"
#include "LAString.h"


/*!
    @brief default constructor
*/
LACoreProcedure::LACoreProcedure() 
: LACoreFunctionBase()
{
}

/*!
    @brief copy constructor

    @param[in] v not used
*/
LACoreProcedure::LACoreProcedure(const LACoreProcedure& v) 
: LACoreFunctionBase(v)
{
}

/*!
    @brief destructor
*/
LACoreProcedure::~LACoreProcedure() 
{
}

/*!
    @brief check whether this class derives from base class with type id

    @param[in] id Function ID to be checked

    @retval true match the specified id
    @retval false not match the specified id
*/
bool
LACoreProcedure::isTypeOf(function_t id) const
{
    return (id == FN_PROCEDURE ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief return Function ID of this class

    @return Function ID
*/
function_t
LACoreProcedure::getType(void) const
{
    return FN_PROCEDURE;
}

// Calibrate curve  model: baseDate, object instance, data attributes, curveCollection (curveID) and curveIndex (marketName)
void LACoreProcedure::calibrateOISAndSwapCurve( const LADate& basedate,
										        LAObject& inst, 
										        const LADataProcedure& att,
                                                const LAString & curveCollection,
                                                const LAString & curveIndex ) const
{
    // Do nothing. This is a base class, must be implemented in the derived class as required.
	// This  method is implemented and overloaded within LADataProcedure.cpp
    throw LACoreInvalidData("calibrateOISAndSwapCurve: Not implemeted for this class", __FILE__,__LINE__);
}
