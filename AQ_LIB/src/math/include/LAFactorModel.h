#pragma once

#include "LARandBase.h"

// Function ID of LAFactorModel
#define FN_RAND_FACTORMODEL     1109
// Function name of LAFactorModel
#define FN_RAND_FACTORMODEL_STR	"fn_rand_factormodel"

///////////////////////////////////////////////////////////////////////
/*!
    @brief Class declaration to generate a set of random numbers according to factor model

	This class derives from MMRandoBase.
*/

class LAFactorModel : public LARandBase
{
public:

//  LIFECYCLE
	// constructor
    LAFactorModel();

	// copy constructor
	LAFactorModel(const LAFactorModel&);
	
	// destructor
    virtual ~LAFactorModel();

	LAFactorModel & operator=( const LAFactorModel & ) { return *this; }


//  QUERY
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;
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
	virtual void				setRandGenerator(const LARandBase& v);
								// set parameter
    virtual void				setParam(const DoubleMatrix& param);
					

private:
	DoubleArray	mOmega;		// coefficient for common factor
	LARandBase* mpRand;			// pointer to random number generator
	DoubleArray mVar;		// tempolary random number container
	
protected:

};


