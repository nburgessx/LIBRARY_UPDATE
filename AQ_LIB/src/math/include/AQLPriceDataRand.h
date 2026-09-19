#pragma once

#ifdef __GNUG__
#pragma interface
#endif

// INCLUDE
#include "AQLString.h"
#include "AQLPriceDataType.h"
#include "AQLRandBase.h"
#include "AQLCoreFunctionHolder.h"



//AQLPriceDataRand Data ID
#define DATA_RAND 56

//============== DECLARE AQLPriceDataRand ============================
/*! 
    @brief Data class declaration to generate random numbers.
*/
// if we add virtual in case of gcc, then
// debugger has fatal error in Access violation since it tries to
// refer to cotents of attrrand after dynami_cast.
// dynami_cast from parent class to child class seems to have fatal error on gcc.
// class AQLPriceDataRand : virtual public AQLPriceDataType
class AQLPriceDataRand : public AQLPriceDataType
{
public:
	// default constructor
	AQLPriceDataRand(void);

	// copy constructor
    AQLPriceDataRand(const AQLPriceDataRand& attr);

	// constructor
    explicit AQLPriceDataRand(const AQLCoreFunctionHolder& h, 
									const AQLString& name);
	// hold ownershipf of constructor Pointe
	explicit AQLPriceDataRand(AQLRandBase* b, 
									const AQLString& name);
	// destructor
    virtual ~AQLPriceDataRand(void);

	// deep copy of this object
    virtual AQLPriceDataType*	    clone() const; 
	
	// return (random generators method) in the string representation
    virtual AQLString			convertToString(void) const;

	// check whether this class derives from base class with type id
	bool						isTypeOf(function_t id) const;

	// get function type
	function_t					getType() const;

	// return Method function to be set
	const AQLRandBase&			getMethod(void) const;

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
    // set the method of function to generate rondom numbers from the selected string format
	virtual void			    convertFromString(const AQLString& str);
		
	// set the method of function from the name
	void						setMethod(const AQLString& name);

	// set name and the method of function, and holds ownership of the pointer
	void						setMethod(AQLRandBase* b, 
											const AQLString& name);
////////////////////////////////////////////////////////////////////////////
//  VARIATION METHODS
protected:
	// the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&		assignment(const AQLPriceDataType& a);
	// compare the contents against the other objecet
	virtual int			        compare(const AQLPriceDataType& a) const;

	// set a pointer to the Holder of the Data
	virtual void				setHolder(AQLDataHolder* holder);

private:
	// set the Object from a function name
	void						setMethod();

	// hold a pointer to the function to generate random numbers
	AQLRandBase*					mpMethods; 

	// hold a function to generate random numbers
	AQLCoreFunctionHolder			mFnHolder;

	// function name to generate random numbers
	AQLString					mName; 
};

