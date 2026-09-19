#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreSystemError.h"
#include "AQLPriceDataType.h"
#include "AQLCoreValuation.h"
#include "AQLCoreFunctionHolder.h"
#include "AQLDate.h"

// AQLDataValuation Data ID
#define DATA_VALUATION  30


/*! 
    @brief Base class of AQLDataProvider class. It is used to hand data between multiple dataValues in the Data class and Function class
	
*/
class AQLDataProvider
{
public:
    // default constructor
    AQLDataProvider(void) : dataProvider(NULL) {}
    // destructor
    virtual ~AQLDataProvider(void) {}

    // AQLDataProvider object
    AQLDataProvider*  dataProvider;
};

/*! 
    @brief Class of the Data that holds the evaluation class

    the evaluation method is value()
*/
class AQLDataValuation : public AQLPriceDataType
{
public:
    // default constructor
    AQLDataValuation(void);
    // copy constructor
    AQLDataValuation(const AQLDataValuation& attr);
    // constructor
    explicit AQLDataValuation(const AQLCoreFunctionHolder& h, const AQLString& name);
    // constructor
    // retain ownership of the pointer
    explicit AQLDataValuation(const AQLCoreValuation* b, const AQLString& name);
    // destructor
    virtual ~AQLDataValuation(void);

    // deep copy of the data object
    virtual AQLPriceDataType*        clone() const; 
    
    // string consisting of the result of a function and function name
    virtual AQLString            convertToString() const;

    // check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id) const;

    // return Function ID of this class
    function_t                  getType() const;
     
	// return Method function to be set
    const AQLCoreValuation&			getMethod(void) const;

    // get result
    double                      getValue(void) const {return mValue;}

   // set the Data from string representation(with function name and the resultof the function)
    virtual void                convertFromString(const AQLString& str);
    
    // set the result of a function and the function name from the specified string
    void                        setMethod(const AQLString& name);

    // set function name and Function class to implement the function
    void                        setMethod(const AQLCoreValuation* b, const AQLString& name);

    // set the calculation results
    void                        setValue(const double b);

//  Valuation METHODS
    // perform the evaluation method of the Function class(value), returning the result of the calculation
    double                      value(const AQLDate& basedate);

    // perform the re-evaluation method of the Function class, returning the result of the calculation
    double                      revalue(void);

    // not calculate when called calibrateModel()
    virtual void                setNoValuation(void) const;

    // unset the set of setNoValuation(), and calculation is performed when called value()
    virtual void                setValuation(void) const;
    
    // set the AQLDataProvider to member functions
    virtual void                setDataProvider(AQLDataProvider* dataProvider) const;

    // get AQLDataProvider object
    virtual AQLDataProvider&          getDataProvider(void) const {return *mpDataProvider;}
    
	// get whether the AQLDataProvider object is Null or not
    bool			          isNullDataProvider(void) const {return mpDataProvider == NULL;}

protected:
    // the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&        assignment(const AQLPriceDataType& a);
	// compare function (with respect to the other object), and it compares whether Functio ID is bigger or smaller
    virtual int                 compare(const AQLPriceDataType& a) const;
    // set a pointer to the Holder of the Data
    virtual void                setHolder(AQLDataHolder* holder);

private:
    // get Function object from Function Master based on mName, and set it to mpMethods
    void                        setMethod();

    mutable int             mVersion;    // calc flag(initial value -1, not calculate in case of -1)
    const AQLCoreValuation*      mpMethods;   // pointer to Function class
    mutable AQLDataProvider*      mpDataProvider;       // AQLDataProvider object
    mutable double          mValue;      // evaluation result
    mutable AQLDate          mDate;       // date to be evaluated
    AQLCoreFunctionHolder        mFuncHolder; // Function Holder
    AQLString                mName;       // function name(e.g. "fn_EuropeanOption")
};

