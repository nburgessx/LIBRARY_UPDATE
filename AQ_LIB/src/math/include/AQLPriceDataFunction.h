#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "AQLString.h"
#include "AQLPriceDataType.h"
#include "AQLCoreFunctionHolder.h"
#include "AQLDataValuation.h"
#include "AQLCoreTemplateType.h"


// AQLPriceDataFunction Data ID
#define DATA_FUNCTION 58


class AQLFunctionBase;
class AQLIntegralBase;

//============== DECLARE AQLPriceDataFunction ============================
/*! 
    @brief Declaration of class representing a mathmatical function
*/
class AQLPriceDataFunction : public AQLPriceDataType
{
public:
	// default constructor
    AQLPriceDataFunction(void);

	// copy constructor
    AQLPriceDataFunction(const AQLPriceDataFunction& attr);

	// constructor
    AQLPriceDataFunction(const AQLCoreFunctionHolder& h, 
                                    const AQLString& name);

    // constructor
	AQLPriceDataFunction(AQLFunctionBase* b, 
                                    const AQLString& name);
	// destructor
    virtual ~AQLPriceDataFunction(void);

	// copy this object
    virtual AQLPriceDataType*        clone() const; 

	// get string representation of function which this object holds
    virtual AQLString            convertToString(void) const;
	
	// check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id) const;
    
	// return class type
    function_t                  getType() const;

	// return the function object which this class holds
    const AQLFunctionBase&		getFunction(void) const;
	// return the function object which this class holds
    AQLFunctionBase&				getFunction(void);
    // set function object with string repsentation "str"
    virtual void                convertFromString(const AQLString& str);
        
	// set function object specified by "name"
    void                        setFunction(const AQLString& name);
	// set function object with name "name"
    void                        setFunction(AQLFunctionBase* b, 
                                            const AQLString& name);
//  VARIATION METHODS

	// return function value
    double                      operator()(const DoubleArray& x) const;
	// return function value to be one-dimensionalized
	double                      operator()(double x) const; 

	// get function parameters
	const DoubleArray&		getParam()const;
//	DoubleArray&				getParam();

	// set function paramters
    void                        setParam(const DoubleArray& param);

protected:
	// initialize this class with another
    virtual AQLPriceDataType&        assignment(const AQLPriceDataType& a);
	// compare this object with another
    virtual int                 compare(const AQLPriceDataType& a) const;

	//	set up the pointer to data holder
    virtual void                setHolder(AQLDataHolder* holder);

private:
                                //=========================================
	// inner method to set up AttrFunction with name "mName"
    void                        setFunction();

    AQLFunctionBase*				mpMethods;		// pointer to function object 
    AQLIntegralBase*				mpIntegral;		// pointer to integral method
	AQLCoreFunctionHolder            mFnHolder;		// pointer to function holder
	AQLString                    mName;			// function name
	AQLString					mParamStr;		// function parameter


};

