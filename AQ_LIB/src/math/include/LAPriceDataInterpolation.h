#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "LAString.h"
#include "LAPriceDataType.h"
#include "LAInterpolationBase.h"
#include "LACoreFunctionHolder.h"
#include "LADataValuation.h"


// LAPriceDataInterpolation Data ID
#define DATA_INTERPOLATION 54

//============== DECLARE LAPriceDataInterpolation ============================
/*! 
    @brief Data class to set interpolation function.
*/
class LAPriceDataInterpolation : public LAPriceDataType
{
public:
//  LIFECYCLE
	// default constructor
    LAPriceDataInterpolation(void);

	// copy constructor
    LAPriceDataInterpolation(const LAPriceDataInterpolation& attr);

	// constructor
    explicit LAPriceDataInterpolation(const LACoreFunctionHolder& h, 
                                    const LAString& name);

    // hold ownership of pointer
    explicit LAPriceDataInterpolation(LAInterpolationBase* b, 
                                    const LAString& name);
	// destructor
    virtual ~LAPriceDataInterpolation(void);

//  QUERY
	// deep copy of the data object - calling code is resposible for memory clean-up.
	// Calling code must delete LAPriceDataType when finished with the object to avoid memory leaks.
    virtual LAPriceDataType*        clone() const; 

	// return (interpolation method) in the string representation
    virtual LAString            convertToString(void) const;
	
	// check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id) const;
    
	// get function type
    function_t                  getType() const;

	// Check if Interpolator is Null
	bool						isNullOrUndefined() const;

	// return method to be set
    const LAInterpolationBase&      getMethod(void) const;

//  OPERATOR    
    // set specified interpolation method from string format
    virtual void                convertFromString(const LAString& str);
        
	//set function specified by name
    void                        setMethod(const LAString& name);
	//set name and function
    void                        setMethod(LAInterpolationBase* b, 
                                            const LAString& name);
//  VARIATION METHODS

	// return a value of one-dimensional curve
    double                      value(const double x1) const; 

//  OPERATION
	// set a value of one-dimensional curve
    void                        set(const DoubleArray& index, const DoubleArray& value);

    // set the value of the interpolation join date double
    void                        setJoinDateAsDouble( const double& joinDateAsDouble );

	// return the value of the interpolation join date double
    double                      getJoinDateAsDouble() const;

	// return true if it's hybrid interpolation, e.g. linearSpline
	bool						isHybrid() const;


// makes sure the name starts with "fn_" and ends with "interpolation"
	static LAString adjustInterpolationName(LAString candidateName);
	
protected:
	// the contents of the object to be initialized with those of another object
    virtual LAPriceDataType&        assignment(const LAPriceDataType& a);
	// compare the contents against the other objecet
    virtual int                 compare(const LAPriceDataType& a) const;

	//	set up the pointer to the data holder
    virtual void                setHolder(LADataHolder* holder);

private:
                                //=========================================
	// set the object from the function name
    void                        setMethod();

	// member variable holding the pointer to the interpolation function
    LAInterpolationBase*        mpMethods; 

	// Holder to keep interpolation function
    LACoreFunctionHolder            mFnHolder;

	// name of interpolation function
    LAString                    mName; 
//    mutable LADataProvider*          mpDataProvider;
};

