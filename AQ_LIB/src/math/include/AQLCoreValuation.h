#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"

// AQLCoreValuation Function ID
#define FN_VALUATION  1 

class AQLDate;
class AQLObject;
class AQLDataValuation;

/*! 
    @brief  Base class to run value function whose arguments are basedate and AQLObject.

    @sa AQLDataValuation
*/
class AQLCoreValuation : public AQLCoreFunctionBase
{
public:
    // default constructor
    AQLCoreValuation(void);
    // destructor
    virtual ~AQLCoreValuation(void);

    // check whether this class derives from base class with type id
    virtual bool                isTypeOf(function_t id) const;

    // deep copy of this object
    /*!
        @return the copied object
    */
    virtual AQLCoreFunctionBase*     clone() const =0;

    // return Function ID of this class
    virtual function_t          getType() const; 


    // return result from valuable function
    /*!
        @param[in] basedate the date to be evaluated
        @param[in] inst the Object to be evaluated
        @param[in] att  the Data that the valuable function has
    */
    virtual double              value(const AQLDate& basedate, AQLObject& inst, 
                                      const AQLDataValuation& att) const 
                                      = 0;

private:
protected:
    // copy constructor
    AQLCoreValuation(const AQLCoreValuation& v);
};

