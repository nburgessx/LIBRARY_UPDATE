#pragma once

#include "LARandBase.h"
#include "LAMatrix.h"


// LAGaussianCopula Function ID
#define FN_RAND_GAUSSIANCOPULA     1110
// LAGaussianCopula Function Name
#define FN_RAND_GAUSSIANCOPULA_STR	"fn_rand_gaussiancopula"


///////////////////////////////////////////////////////////////////////
/*!
    @brief Class declaration to generate a set of random numbers according to Gaussian Copula
*/

class LAGaussianCopula : public LARandBase
{
public:

//  LIFECYCLE
	// constructor
    LAGaussianCopula();

	// copy constructor
	LAGaussianCopula(const LAGaussianCopula&);
	
	// destructor
    virtual ~LAGaussianCopula();

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


//  OPERATION
                                //==========================================
								// set dimension (overriding)
	virtual void				setDim(const UintArray& dimValue);
	                            //==========================================
								// set seed (overriding)
	virtual void                setSeed(const UlongArray& seed);
								//======================================
								// set a random number generator for negative correlation method.
	virtual void				setRandGenerator(const LARandBase& v);
								// set parameter
    virtual void				setParam(const DoubleMatrix& param);

	LAGaussianCopula & operator=( const LAGaussianCopula & ) { return *this; }


private:
	LARandBase* mpRand;			// pointer to random number generator
	LAMatrix mMat;				// correlation matrix after cholesky
	DoubleArray mVar;		// tempolary container for randomnumber

protected:

};

