#pragma once

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning( disable : 4505 )
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"

// AQLCoreProcedure Function ID
#define FN_PROCEDURE    3 

class AQLDate;
class AQLObject;
class AQLDataProcedure;
class AQLString;


/*! 
    @brief Base class to run procedure function whose arguments are basedate and AQLObject.

    @sa AQLCoreValuation, AQLDataProcedure
*/
class AQLCoreProcedure : virtual public AQLCoreFunctionBase
{
public:
    // default constructor
    AQLCoreProcedure();
    
	// destructor
    virtual ~AQLCoreProcedure();

    // check whether this class derives from base class with type id
    virtual bool					isTypeOf(function_t id) const;

    // deep copy of this object
    virtual AQLCoreFunctionBase*     clone() const=0;

    // return Function ID of this class
    virtual function_t				getType() const;

    // procedure function
    
    /*!
        @param[in] basedate the date to be evaluated
        @param[in] inst the Object to be evaluated
        @param[in] att  the Data that the procedure class has
    */
    virtual void			calibrateModel(const AQLDate& basedate,
										   AQLObject& inst, 
										   const AQLDataProcedure& att) const=0;

    // Calibrate curve  model: baseDate, object instance, data attributes, curveCollection (curveID) and curveIndex (marketName)
    virtual void            calibrateOISAndSwapCurve(const AQLDate& basedate,
										             AQLObject& inst, 
										             const AQLDataProcedure& att,
                                                     const AQLString & curveCollection,
                                                     const AQLString & curveIndex ) const;

private:
protected:
    // copy constructor
    AQLCoreProcedure(const AQLCoreProcedure& p);
};

