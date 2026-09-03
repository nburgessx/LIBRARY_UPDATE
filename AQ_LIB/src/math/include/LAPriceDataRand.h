#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "LAString.h"
#include "LAPriceDataType.h"
#include "LARandBase.h"
#include "LACoreFunctionHolder.h"



//LAPriceDataRand Data ID
#define DATA_RAND 56

//============== DECLARE LAPriceDataRand ============================
/*! 
    @brief Data class declaration to generate random numbers.
*/
// if we add virtual in case of gcc, then
// debugger has fatal error in Access violation since it tries to
// refer to cotents of attrrand after dynami_cast.
// dynami_cast from parent class to child class seems to have fatal error on gcc.
// class LAPriceDataRand : virtual public LAPriceDataType
class LAPriceDataRand : public LAPriceDataType
{
public:
//	LIFECYCLE
	// default constructor
	LAPriceDataRand(void);

	// copy constructor
    LAPriceDataRand(const LAPriceDataRand& attr);

	// constructor
    explicit LAPriceDataRand(const LACoreFunctionHolder& h, 
									const LAString& name);
	// hold ownershipf of constructor Pointe
	explicit LAPriceDataRand(LARandBase* b, 
									const LAString& name);
	// destructor
    virtual ~LAPriceDataRand(void);

//  QUERY
	// deep copy of this object
    virtual LAPriceDataType*	    clone() const; 
	
	// return (random generators method) in the string representation
    virtual LAString			convertToString(void) const;

	// check whether this class derives from base class with type id
	bool						isTypeOf(function_t id) const;

	// get function type
	function_t					getType() const;

	// return Method function to be set
	const LARandBase&			getMethod(void) const;

    // function to generate uniform random numbers
    void						getUniforms(DoubleArray& variates); 
	
	// function to generate normal random numbers
    void						getGaussians(DoubleArray& variates); 
	
	// function to get seed 
    const UlongArray&			getSeed(void)const;

	// function to get dimension information to be set
    const UintArray&			getDim(void)const;

	// function to set seed
    void				        setSeed(const UlongArray& seedValue);
								
	// function to set dimension information(first element should be the number of dimension)
    void                        setDim(const UintArray& dimValue);
	// function to set parameters
    void						setParam(const DoubleMatrix& param);
//  OPERATOR	
    // set the method of function to generate rondom numbers from the selected string format
	virtual void			    convertFromString(const LAString& str);
		
	// set the method of function from the name
	void						setMethod(const LAString& name);

	// set name and the method of function, and holds ownership of the pointer
	void						setMethod(LARandBase* b, 
											const LAString& name);
////////////////////////////////////////////////////////////////////////////
//  VARIATION METHODS
protected:
	// the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&		assignment(const LAPriceDataType& a);
	// compare the contents against the other objecet
	virtual int			        compare(const LAPriceDataType& a) const;

	// set a pointer to the Holder of the Data
	virtual void				setHolder(LADataHolder* holder);

private:
	// set the Object from a function name
	void						setMethod();

	// hold a pointer to the function to generate random numbers
	LARandBase*					mpMethods; 

	// hold a function to generate random numbers
	LACoreFunctionHolder			mFnHolder;

	// function name to generate random numbers
	LAString					mName; 
};

