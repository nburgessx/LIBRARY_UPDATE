#pragma once

#include "AQLRandBase.h"


// Function ID of AQLBoxMuller
#define FN_RAND_BOXMULLER     1103
// Function Name of AQLBoxMuller
#define FN_RAND_BOXMULLER_STR	"fn_rand_boxmuller"


/*! 
    @brief Class to generate random number with Box-Muller method.
*/
class AQLBoxMuller : public AQLRandBase
{
public:
    AQLBoxMuller(); // Default Construcotr
    virtual ~AQLBoxMuller(); // Destructor
    AQLBoxMuller(const AQLBoxMuller& v); // Copy Constructor

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
								//======================================
								// set a random number generator for negative correlation method.
	virtual void				setRandGenerator(const AQLRandBase& v);

private:
	AQLRandBase*                 mpRand;     // pointer to random number generator
    DoubleArray                 mNextVar;   // next variates
    bool                        mIsOdd;     // state variable

protected:

};

