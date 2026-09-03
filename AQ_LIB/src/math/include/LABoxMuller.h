#pragma once

#include "LARandBase.h"


// Function ID of LABoxMuller
#define FN_RAND_BOXMULLER     1103
// Function Name of LABoxMuller
#define FN_RAND_BOXMULLER_STR	"fn_rand_boxmuller"


/*! 
    @brief Class to generate random number with Box-Muller method.
*/
class LABoxMuller : public LARandBase
{
public:
//  LIFECYCLE
    LABoxMuller(); // Default Construcotr
    virtual ~LABoxMuller(); // Destructor
    LABoxMuller(const LABoxMuller& v); // Copy Constructor

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

private:
	LARandBase*                 mpRand;     // pointer to random number generator
    DoubleArray                 mNextVar;   // next variates
    bool                        mIsOdd;     // state variable

protected:

};

