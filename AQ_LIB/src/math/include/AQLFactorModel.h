#pragma once

#include "AQLRandBase.h"

// Function ID of AQLFactorModel
#define FN_RAND_FACTORMODEL     1109
// Function name of AQLFactorModel
#define FN_RAND_FACTORMODEL_STR	"fn_rand_factormodel"

///////////////////////////////////////////////////////////////////////
/*!
    @brief Class declaration to generate a set of random numbers according to factor model

	This class derives from MMRandoBase.
*/

class AQLFactorModel : public AQLRandBase
{
public:

//  LIFECYCLE
	// constructor
    AQLFactorModel();

	// copy constructor
	AQLFactorModel(const AQLFactorModel&);
	
	// destructor
    virtual ~AQLFactorModel();

	AQLFactorModel & operator=( const AQLFactorModel & ) { return *this; }


//  QUERY
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;								
                                //==========================================
                                // generate uniform random numbers
    virtual void                getUniforms(DoubleArray& variates); 
								// generate normal random numbers
    virtual void                getGaussians(DoubleArray& variates); 
                                //==========================================
								// set dimension (overriding)
	virtual void				setDim(const UintArray& dimValue);
	                            //==========================================
								// set seed (overriding)
	virtual void                setSeed(const UlongArray& seed);



//  OPERATION
								//======================================
								// set a random number generator
	virtual void				setRandGenerator(const AQLRandBase& v);
								// set parameter
    virtual void				setParam(const DoubleMatrix& param);
					

private:
	DoubleArray	mOmega;		// coefficient for common factor
	AQLRandBase* mpRand;			// pointer to random number generator
	DoubleArray mVar;		// tempolary random number container
	
protected:

};


