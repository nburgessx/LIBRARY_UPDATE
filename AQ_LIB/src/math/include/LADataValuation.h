#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreSystemError.h"
#include "LAPriceDataType.h"
#include "LACoreValuation.h"
#include "LACoreFunctionHolder.h"
#include "LADate.h"

// LADataValuation Data ID
#define DATA_VALUATION  30


/*! 
    @brief Base class of LADataProvider class. It is used to hand data between multiple dataValues in the Data class and Function class
	
*/
class LADataProvider
{
public:
    // default constructor
    LADataProvider(void) : dataProvider(NULL) {}
    // destructor
    virtual ~LADataProvider(void) {}

    // LADataProvider object
    LADataProvider*  dataProvider;
};

/*! 
    @brief Class of the Data that holds the evaluation class

    the evaluation method is value()
*/
class LADataValuation : public LAPriceDataType
{
public:
    // default constructor
    LADataValuation(void);
    // copy constructor
    LADataValuation(const LADataValuation& attr);
    // constructor
    explicit LADataValuation(const LACoreFunctionHolder& h, const LAString& name);
    // constructor
    // retain ownership of the pointer
    explicit LADataValuation(const LACoreValuation* b, const LAString& name);
    // destructor
    virtual ~LADataValuation(void);

//  QUERY
    // deep copy of the data object
    virtual LAPriceDataType*        clone() const; 
    
    // string consisting of the result of a function and function name
    virtual LAString            convertToString() const;

    // check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id) const;

    // return Function ID of this class
    function_t                  getType() const;
     
	// return Method function to be set
    const LACoreValuation&			getMethod(void) const;

    // get result
    double                      getValue(void) const {return mValue;}

//  OPERATION
   // set the Data from string representation(with function name and the resultof the function)
    virtual void                convertFromString(const LAString& str);
    
    // set the result of a function and the function name from the specified string
    void                        setMethod(const LAString& name);

    // set function name and Function class to implement the function
    void                        setMethod(const LACoreValuation* b, const LAString& name);

    // set the calculation results
    void                        setValue(const double b);

//  Valuation METHODS
    // perform the evaluation method of the Function class(value), returning the result of the calculation
    double                      value(const LADate& basedate);

    // perform the re-evaluation method of the Function class, returning the result of the calculation
    double                      revalue(void);

    // not calculate when called calibrateModel()
    virtual void                setNoValuation(void) const;

    // unset the set of setNoValuation(), and calculation is performed when called value()
    virtual void                setValuation(void) const;
    
    // set the LADataProvider to member functions
    virtual void                setDataProvider(LADataProvider* dataProvider) const;

    // get LADataProvider object
    virtual LADataProvider&          getDataProvider(void) const {return *mpDataProvider;}
    
	// get whether the LADataProvider object is Null or not
    bool			          isNullDataProvider(void) const {return mpDataProvider == NULL;}

protected:
    // the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&        assignment(const LAPriceDataType& a);
	// compare function (with respect to the other object), and it compares whether Functio ID is bigger or smaller
    virtual int                 compare(const LAPriceDataType& a) const;
    // set a pointer to the Holder of the Data
    virtual void                setHolder(LADataHolder* holder);

private:
    // get Function object from Function Master based on mName, and set it to mpMethods
    void                        setMethod();

    mutable int             mVersion;    // calc flag(initial value -1, not calculate in case of -1)
    const LACoreValuation*      mpMethods;   // pointer to Function class
    mutable LADataProvider*      mpDataProvider;       // LADataProvider object
    mutable double          mValue;      // evaluation result
    mutable LADate          mDate;       // date to be evaluated
    LACoreFunctionHolder        mFuncHolder; // Function Holder
    LAString                mName;       // function name(e.g. "fn_EuropeanOption")
};

