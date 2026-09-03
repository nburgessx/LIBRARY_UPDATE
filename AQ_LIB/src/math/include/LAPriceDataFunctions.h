#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "LAString.h"
#include "LAPriceDataType.h"
#include "LAFunctionBase.h"
#include "LACoreFunctionHolder.h"
#include "LADataValuation.h"


// LAPriceDataFunctions Data ID
#define DATA_FUNCTIONS 59

typedef	std::vector<LAFunctionBase*>			FunctionVector;// Array of functions
typedef	std::vector<LACoreFunctionHolder>			FunctionHolderVector;// Array of function holders


//============== DECLARE LAPriceDataFunctions ============================
/*! 
    @brief Declaration of data class to hold a function array
*/
class LAPriceDataFunctions : public LAPriceDataType
{
public:
//  LIFECYCLE
	// default constructor
    LAPriceDataFunctions(void);

	// copy constructor
    LAPriceDataFunctions(const LAPriceDataFunctions& attr);

	// constructor
    explicit LAPriceDataFunctions(const FunctionHolderVector& h, 
                                    const LAStringVector& names);

    // constructor
    explicit LAPriceDataFunctions(FunctionVector& b, 
                                    const LAStringVector& names);
	// destructor
    virtual ~LAPriceDataFunctions(void);

//  QUERY
	// copy(clone) this object
    virtual LAPriceDataType*        clone() const; 

	// get string representation of function array which this object holds
    virtual LAString            convertToString(void) const;
	
	// check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id, unsigned int pos) const;
    
	// return this class type
    function_t                  getType(unsigned int pos) const;

	// return the function object array which this class holds
    const FunctionVector&		getFunctions(void) const;
 
    // get a number of function object to be hold
    /*!
        @return number of function object
    */
    unsigned int	            getSize(void) const {return mNames.size();}

//  OPERATOR    
    // set function array object with string repsentation "str"
    virtual void                convertFromString(const LAString& str);
        
	// set function object array specified by "name"
    void                        setFunctions(const LAStringVector& names);
	// set function object array with name "name"
    void                        setFunctions(FunctionVector& b, 
                                            const LAStringVector& names);

    // add function object with name "b" to last of array
    void						push_back(const LAString& b);
	// add function object with name "name"
    void						push_back(LAFunctionBase* b,
											const LAString& name);
    // clear all function object
    void						clear();	
	
	// operator[]
    const LAFunctionBase&		operator[](int i) const;
	// operator[]
    LAFunctionBase&				operator[](int i);

	//  VARIATION METHODS


//  OPERATION
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
    void                        setFunctions();

	// array of pointer to function objects
	FunctionVector        mMethods; 

	// array of pointer to function holders
    FunctionHolderVector  mFnHolders;

	// array of function objects
    LAStringVector           mNames; 
	// array of function parameters(string representation)
	LAStringVector		   mParamStrs;	

};

