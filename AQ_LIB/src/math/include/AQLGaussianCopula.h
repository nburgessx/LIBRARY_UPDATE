#pragma once

#include "AQLRandBase.h"
#include "AQLMatrix.h"


// AQLGaussianCopula Function ID
#define FN_RAND_GAUSSIANCOPULA     1110
// AQLGaussianCopula Function Name
#define FN_RAND_GAUSSIANCOPULA_STR	"fn_rand_gaussiancopula"


///////////////////////////////////////////////////////////////////////
/*!
    @brief Class declaration to generate a set of random numbers according to Gaussian Copula
*/

class AQLGaussianCopula : public AQLRandBase
{
public:

//  LIFECYCLE
	// constructor
    AQLGaussianCopula();

	// copy constructor
	AQLGaussianCopula(const AQLGaussianCopula&);
	
	// destructor
    virtual ~AQLGaussianCopula();

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


//  OPERATION
                                //==========================================
								// set dimension (overriding)
	virtual void				setDim(const UintArray& dimValue);
	                            //==========================================
								// set seed (overriding)
	virtual void                setSeed(const UlongArray& seed);
								//======================================
								// set a random number generator for negative correlation method.
	virtual void				setRandGenerator(const AQLRandBase& v);
								// set parameter
    virtual void				setParam(const DoubleMatrix& param);

	AQLGaussianCopula & operator=( const AQLGaussianCopula & ) { return *this; }


private:
	AQLRandBase* mpRand;			// pointer to random number generator
	AQLMatrix mMat;				// correlation matrix after cholesky
	DoubleArray mVar;		// tempolary container for randomnumber

protected:

};

