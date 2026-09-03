#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreSystemError.h"
#include "LAPriceDataType.h"
#include "LACoreProcedure.h"
#include "LACoreFunctionHolder.h"
#include "LADataValuation.h"
#include "LADate.h"

// LADataProcedure Data ID
#define DATA_PROCEDURE  31


/*! 
    @brief Class of the Data that holds the procedure class
*/
class LADataProcedure : public LAPriceDataType
{
public:
    
    // default constructor
    LADataProcedure(void);
    
    // copy constructor
    LADataProcedure(const LADataProcedure& attr);
    
    // constructor
    explicit LADataProcedure(const LACoreFunctionHolder& h, const LAString& name);
    
    // constructor
    explicit LADataProcedure(const LACoreProcedure* b, const LAString& name);
    
    // destructor
    virtual ~LADataProcedure(void);

    // deep copy of the data object
    virtual LAPriceDataType*        clone() const; 
    
    // return (function name) in the string representation
    virtual LAString            convertToString() const;

    // check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id) const;

    // return Function ID of this class
    function_t                  getType() const;

	// return Method function to be set
    const LACoreProcedure&			getMethod(void) const;

    // set the Data from string representation(function name)
    virtual void                convertFromString(const LAString& str);
    
    // set function name
    void                        setMethod(const LAString& name);

    // set function name and Function class to implement the function
    void                        setMethod(const LACoreProcedure* b, const LAString& name);

    // calibrate model
    void                        calibrateModel(const LADate& basedate);

    // calibrate curve model
    void                        calibrateOISAndSwapCurve( const LADate& basedate, const LAString & curveCollection, const LAString & curveIndex );

    // Re-calibrate Model
    void                        reCalibrateModel(void);

    // not calculate when called calibrateModel()
    virtual void                setNoValuation(void) const;

    // unset the set of setNoValuation(), and calculation is performed when called value()
    virtual void                setValuation(void) const;
    
    // set the LADataProvider to member functions
    virtual void                setDataProvider(LADataProvider* dataProvider) const;

    // get LADataProvider object
    /*!
        @return LADataProvider object
    */
    virtual LADataProvider&          getDataProvider(void) const {return *mpDataProvider;}

	// get whether the LADataProvider object is Null or not
    /*!
        @return true if the NULL, false if it is not NULL
    */
    bool						isNullDataProvider(void) const {return mpDataProvider == NULL;}
                                                    
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

    mutable int                 mVersion;           // calc flag(initial value -1, not calculate in case of -1)
    const LACoreProcedure*      mpMethods;          // pointer to Function class
    mutable LADataProvider*     mpDataProvider;     // LADataProvider object
    mutable LADate              mDate;              // date to be evaluated
    LACoreFunctionHolder        mFuncHolder;        // Function Holder
    LAString                    mName;              // function name(e.g. "fn_EuropeanOption")
};

