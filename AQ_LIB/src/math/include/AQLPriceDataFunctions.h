#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "AQLString.h"
#include "AQLPriceDataType.h"
#include "AQLFunctionBase.h"
#include "AQLCoreFunctionHolder.h"
#include "AQLDataValuation.h"


// AQLPriceDataFunctions Data ID
#define DATA_FUNCTIONS 59

typedef	std::vector<AQLFunctionBase*>			FunctionVector;// Array of functions
typedef	std::vector<AQLCoreFunctionHolder>			FunctionHolderVector;// Array of function holders


//============== DECLARE AQLPriceDataFunctions ============================
/*! 
    @brief Declaration of data class to hold a function array
*/
class AQLPriceDataFunctions : public AQLPriceDataType
{
public:
	// default constructor
    AQLPriceDataFunctions(void);

	// copy constructor
    AQLPriceDataFunctions(const AQLPriceDataFunctions& attr);

	// constructor
    explicit AQLPriceDataFunctions(const FunctionHolderVector& h, 
                                    const AQLStringVector& names);

    // constructor
    explicit AQLPriceDataFunctions(FunctionVector& b, 
                                    const AQLStringVector& names);
	// destructor
    virtual ~AQLPriceDataFunctions(void);

	// copy(clone) this object
    virtual AQLPriceDataType*        clone() const; 

	// get string representation of function array which this object holds
    virtual AQLString            convertToString(void) const;
	
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

    // set function array object with string repsentation "str"
    virtual void                convertFromString(const AQLString& str);
        
	// set function object array specified by "name"
    void                        setFunctions(const AQLStringVector& names);
	// set function object array with name "name"
    void                        setFunctions(FunctionVector& b, 
                                            const AQLStringVector& names);

    // add function object with name "b" to last of array
    void						push_back(const AQLString& b);
	// add function object with name "name"
    void						push_back(AQLFunctionBase* b,
											const AQLString& name);
    // clear all function object
    void						clear();	
	
	// operator[]
    const AQLFunctionBase&		operator[](int i) const;
	// operator[]
    AQLFunctionBase&				operator[](int i);

	//  VARIATION METHODS


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
    void                        setFunctions();

	// array of pointer to function objects
	FunctionVector        mMethods; 

	// array of pointer to function holders
    FunctionHolderVector  mFnHolders;

	// array of function objects
    AQLStringVector           mNames; 
	// array of function parameters(string representation)
	AQLStringVector		   mParamStrs;	

};

