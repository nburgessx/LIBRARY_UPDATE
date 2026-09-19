#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "AQLString.h"
#include "AQLPriceDataType.h"
#include "AQLInterpolationBase.h"
#include "AQLCoreFunctionHolder.h"
#include "AQLDataValuation.h"


// AQLPriceDataInterpolation Data ID
#define DATA_INTERPOLATION 54

//============== DECLARE AQLPriceDataInterpolation ============================
/*! 
    @brief Data class to set interpolation function.
*/
class AQLPriceDataInterpolation : public AQLPriceDataType
{
public:
	// default constructor
    AQLPriceDataInterpolation(void);

	// copy constructor
    AQLPriceDataInterpolation(const AQLPriceDataInterpolation& attr);

	// constructor
    explicit AQLPriceDataInterpolation(const AQLCoreFunctionHolder& h, 
                                    const AQLString& name);

    // hold ownership of pointer
    explicit AQLPriceDataInterpolation(AQLInterpolationBase* b, 
                                    const AQLString& name);
	// destructor
    virtual ~AQLPriceDataInterpolation(void);

	// deep copy of the data object - calling code is resposible for memory clean-up.
	// Calling code must delete AQLPriceDataType when finished with the object to avoid memory leaks.
    virtual AQLPriceDataType*        clone() const; 

	// return (interpolation method) in the string representation
    virtual AQLString            convertToString(void) const;
	
	// check whether this class derives from base class with type id
    bool                        isTypeOf(function_t id) const;
    
	// get function type
    function_t                  getType() const;

	// Check if Interpolator is Null
	bool						isNullOrUndefined() const;

	// return method to be set
    const AQLInterpolationBase&      getMethod(void) const;

    // set specified interpolation method from string format
    virtual void                convertFromString(const AQLString& str);
        
	//set function specified by name
    void                        setMethod(const AQLString& name);
	//set name and function
    void                        setMethod(AQLInterpolationBase* b, 
                                            const AQLString& name);
//  VARIATION METHODS

	// return a value of one-dimensional curve
    double                      value(const double x1) const; 

	// set a value of one-dimensional curve
    void                        set(const DoubleArray& index, const DoubleArray& value);

    // set the value of the interpolation join date double
    void                        setJoinDateAsDouble( const double& joinDateAsDouble );

	// return the value of the interpolation join date double
    double                      getJoinDateAsDouble() const;

	// return true if it's hybrid interpolation, e.g. linearSpline
	bool						isHybrid() const;


// makes sure the name starts with "fn_" and ends with "interpolation"
	static AQLString adjustInterpolationName(AQLString candidateName);
	
protected:
	// the contents of the object to be initialized with those of another object
    virtual AQLPriceDataType&        assignment(const AQLPriceDataType& a);
	// compare the contents against the other objecet
    virtual int                 compare(const AQLPriceDataType& a) const;

	//	set up the pointer to the data holder
    virtual void                setHolder(AQLDataHolder* holder);

private:
                                //=========================================
	// set the object from the function name
    void                        setMethod();

	// member variable holding the pointer to the interpolation function
    AQLInterpolationBase*        mpMethods; 

	// Holder to keep interpolation function
    AQLCoreFunctionHolder            mFnHolder;

	// name of interpolation function
    AQLString                    mName; 
//    mutable AQLDataProvider*          mpDataProvider;
};

