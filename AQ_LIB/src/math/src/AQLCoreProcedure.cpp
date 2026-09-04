/*! @file
    @brief Implementation of base class of the Function class that provides procedure with base date and AQLObject as input.
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLCoreProcedure.h"
#include "AQLDate.h"
#include "AQLObject.h"
#include "AQLString.h"


/*!
    @brief default constructor
*/
AQLCoreProcedure::AQLCoreProcedure() 
: AQLCoreFunctionBase()
{
}

/*!
    @brief copy constructor

    @param[in] v not used
*/
AQLCoreProcedure::AQLCoreProcedure(const AQLCoreProcedure& v) 
: AQLCoreFunctionBase(v)
{
}

/*!
    @brief destructor
*/
AQLCoreProcedure::~AQLCoreProcedure() 
{
}

/*!
    @brief check whether this class derives from base class with type id

    @param[in] id Function ID to be checked

    @retval true match the specified id
    @retval false not match the specified id
*/
bool
AQLCoreProcedure::isTypeOf(function_t id) const
{
    return (id == FN_PROCEDURE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief return Function ID of this class

    @return Function ID
*/
function_t
AQLCoreProcedure::getType(void) const
{
    return FN_PROCEDURE;
}

// Calibrate curve  model: baseDate, object instance, data attributes, curveCollection (curveID) and curveIndex (marketName)
void AQLCoreProcedure::calibrateOISAndSwapCurve( const AQLDate& basedate,
										        AQLObject& inst, 
										        const AQLDataProcedure& att,
                                                const AQLString & curveCollection,
                                                const AQLString & curveIndex ) const
{
    // Do nothing. This is a base class, must be implemented in the derived class as required.
	// This  method is implemented and overloaded within AQLDataProcedure.cpp
    throw AQLCoreInvalidData("calibrateOISAndSwapCurve: Not implemeted for this class", __FILE__,__LINE__);
}
