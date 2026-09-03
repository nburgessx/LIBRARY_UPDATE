#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "LAString.h"
#include "LAPriceDataType.h"
#include "LACoreFunctionHolder.h"
#include "LADataValuation.h"
#include "LACoreTemplateType.h"


// LAPriceDataFunction Data ID
#define DATA_FUNCTION 58


class LAFunctionBase;
class LAIntegralBase;

//============== DECLARE LAPriceDataFunction ============================
/*! 
    @brief Declaration of class representing a mathmatical function
*/
class LAPriceDataFunction : public LAPriceDataType
{
public:
//  LIFECYCLE
	// default constructor
    LAPriceDataFunction(void);

	// copy constructor
    LAPriceDataFunction(const LAPriceDataFunction& attr);

	// constructor
    LAPriceDataFunction(const LACoreFunctionHolder& h, 
                                    const LAString& name);

    // constructor
	LAPriceDataFunction(LAFunctionBase* b, 
                                    const LAString& name);
	// destructor
    virtual ~LAPriceDataFunction(void);

//  QUERY
	// copy this object
    virtual LAPriceDataType*        clone() const; 

	// get string representation of function which this object holds
    virtual LAString            convertToString(void) const;
	
	// check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id) const;
    
	// return class type
    function_t                  getType() const;

	// return the function object which this class holds
    const LAFunctionBase&		getFunction(void) const;
	// return the function object which this class holds
    LAFunctionBase&				getFunction(void);
//  OPERATOR    
    // set function object with string repsentation "str"
    virtual void                convertFromString(const LAString& str);
        
	// set function object specified by "name"
    void                        setFunction(const LAString& name);
	// set function object with name "name"
    void                        setFunction(LAFunctionBase* b, 
                                            const LAString& name);
//  VARIATION METHODS

	// return function value
    double                      operator()(const DoubleArray& x) const;
	// return function value to be one-dimensionalized
	double                      operator()(double x) const; 

	// get function parameters
	const DoubleArray&		getParam()const;
//	DoubleArray&				getParam();

//  OPERATION
	// set function paramters
    void                        setParam(const DoubleArray& param);

protected:
	// initialize this class with another
    virtual LAPriceDataType&        assignment(const LAPriceDataType& a);
	// compare this object with another
    virtual int                 compare(const LAPriceDataType& a) const;

	//	set up the pointer to data holder
    virtual void                setHolder(LADataHolder* holder);

private:
                                //=========================================
	// inner method to set up AttrFunction with name "mName"
    void                        setFunction();

    LAFunctionBase*				mpMethods;		// pointer to function object 
    LAIntegralBase*				mpIntegral;		// pointer to integral method
	LACoreFunctionHolder            mFnHolder;		// pointer to function holder
	LAString                    mName;			// function name
	LAString					mParamStr;		// function parameter


};

