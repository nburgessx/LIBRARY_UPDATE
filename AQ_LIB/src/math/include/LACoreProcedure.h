#pragma once

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning( disable : 4505 )
#endif

#include "LACoreFunctionBase.h"
#include "LACoreAppError.h"

// LACoreProcedure Function ID
#define FN_PROCEDURE    3 

class LADate;
class LAObject;
class LADataProcedure;
class LAString;


/*! 
    @brief Base class to run procedure function whose arguments are basedate and LAObject.

    @sa LACoreValuation, LADataProcedure
*/
class LACoreProcedure : virtual public LACoreFunctionBase
{
public:
    // default constructor
    LACoreProcedure();
    
	// destructor
    virtual ~LACoreProcedure();

    // check whether this class derives from base class with type id
    virtual bool					isTypeOf(function_t id) const;

    // deep copy of this object
    virtual LACoreFunctionBase*     clone() const=0;

    // return Function ID of this class
    virtual function_t				getType() const;

    // procedure function
    
    /*!
        @param[in] basedate the date to be evaluated
        @param[in] inst the Object to be evaluated
        @param[in] att  the Data that the procedure class has
    */
    virtual void			calibrateModel(const LADate& basedate,
										   LAObject& inst, 
										   const LADataProcedure& att) const=0;

    // Calibrate curve  model: baseDate, object instance, data attributes, curveCollection (curveID) and curveIndex (marketName)
    virtual void            calibrateOISAndSwapCurve(const LADate& basedate,
										             LAObject& inst, 
										             const LADataProcedure& att,
                                                     const LAString & curveCollection,
                                                     const LAString & curveIndex ) const;

private:
protected:
    // copy constructor
    LACoreProcedure(const LACoreProcedure& p);
};

