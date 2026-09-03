#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreFunctionBase.h"
#include "LACoreAppError.h"

// LACoreValuation Function ID
#define FN_VALUATION  1 

class LADate;
class LAObject;
class LADataValuation;

/*! 
    @brief  Base class to run value function whose arguments are basedate and LAObject.

    @sa LADataValuation
*/
class LACoreValuation : public LACoreFunctionBase
{
public:
    // default constructor
    LACoreValuation(void);
    // destructor
    virtual ~LACoreValuation(void);

    // check whether this class derives from base class with type id
    virtual bool                isTypeOf(function_t id) const;

    // deep copy of this object
    /*!
        @return the copied object
    */
    virtual LACoreFunctionBase*     clone() const =0;

    // return Function ID of this class
    virtual function_t          getType() const; 


    // return result from valuable function
    /*!
        @param[in] basedate the date to be evaluated
        @param[in] inst the Object to be evaluated
        @param[in] att  the Data that the valuable function has
    */
    virtual double              value(const LADate& basedate, LAObject& inst, 
                                      const LADataValuation& att) const 
                                      = 0;

private:
protected:
    // copy constructor
    LACoreValuation(const LACoreValuation& v);
};

