#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreSystemError.h"
#include "AQLPriceDataType.h"
#include "AQLCoreProcedure.h"
#include "AQLCoreFunctionHolder.h"
#include "AQLDataValuation.h"
#include "AQLDate.h"

// AQLDataProcedure Data ID
#define DATA_PROCEDURE  31


/*! 
    @brief Class of the Data that holds the procedure class
*/
class AQLDataProcedure : public AQLPriceDataType
{
public:
    
    // default constructor
    AQLDataProcedure(void);
    
    // copy constructor
    AQLDataProcedure(const AQLDataProcedure& attr);
    
    // constructor
    explicit AQLDataProcedure(const AQLCoreFunctionHolder& h, const AQLString& name);
    
    // constructor
    explicit AQLDataProcedure(const AQLCoreProcedure* b, const AQLString& name);
    
    // destructor
    virtual ~AQLDataProcedure(void);

    // deep copy of the data object
    virtual AQLPriceDataType*        clone() const; 
    
    // return (function name) in the string representation
    virtual AQLString            convertToString() const;

    // check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id) const;

    // return Function ID of this class
    function_t                  getType() const;

	// return Method function to be set
    const AQLCoreProcedure&			getMethod(void) const;

    // set the Data from string representation(function name)
    virtual void                convertFromString(const AQLString& str);
    
    // set function name
    void                        setMethod(const AQLString& name);

    // set function name and Function class to implement the function
    void                        setMethod(const AQLCoreProcedure* b, const AQLString& name);

    // calibrate model
    void                        calibrateModel(const AQLDate& basedate);

    // calibrate curve model
    void                        calibrateOISAndSwapCurve( const AQLDate& basedate, const AQLString & curveCollection, const AQLString & curveIndex );

    // Re-calibrate Model
    void                        reCalibrateModel(void);

    // not calculate when called calibrateModel()
    virtual void                setNoValuation(void) const;

    // unset the set of setNoValuation(), and calculation is performed when called value()
    virtual void                setValuation(void) const;
    
    // set the AQLDataProvider to member functions
    virtual void                setDataProvider(AQLDataProvider* dataProvider) const;

    // get AQLDataProvider object
    /*!
        @return AQLDataProvider object
    */
    virtual AQLDataProvider&          getDataProvider(void) const {return *mpDataProvider;}

	// get whether the AQLDataProvider object is Null or not
    /*!
        @return true if the NULL, false if it is not NULL
    */
    bool						isNullDataProvider(void) const {return mpDataProvider == NULL;}
                                                    
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

    mutable int                 mVersion;           // calc flag(initial value -1, not calculate in case of -1)
    const AQLCoreProcedure*      mpMethods;          // pointer to Function class
    mutable AQLDataProvider*     mpDataProvider;     // AQLDataProvider object
    mutable AQLDate              mDate;              // date to be evaluated
    AQLCoreFunctionHolder        mFuncHolder;        // Function Holder
    AQLString                    mName;              // function name(e.g. "fn_EuropeanOption")
};

