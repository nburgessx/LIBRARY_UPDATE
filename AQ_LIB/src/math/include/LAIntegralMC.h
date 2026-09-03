#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAIntegralBase.h"


// ID for LAIntegralMC
#define FN_INTEGRALMC     1407
// Function name for LAIntegralMC
#define FN_INTEGRALMC_STR	"fn_integralmc"


class LAFunctionBase;
class LARandBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to integrate with Monte Carlo method
	
	This class derives from LAIntegralBase

*/
class LAIntegralMC : public LAIntegralBase
{
public:
//  LIFECYCLE
	// constructor
	LAIntegralMC(unsigned long num, const LARandBase& v);
	// destructor
	virtual ~LAIntegralMC();
	// copy constructor
	LAIntegralMC(const LAIntegralMC& v);

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

								//======================================
								// Return integral result
	virtual double				integrate(const LAFunctionBase& f,
											const std::vector<std::pair<double,double> >& x) const;
// OPERATION
								//======================================
                                // Set random number generator
	void						setRandGenerator(const LARandBase& v);
								//======================================
								// Set Monte Carlo simulation number
	void						setMCNum(unsigned long num) {mMCNum = num;};
								//======================================
								// Set seed
	void				        setSeed(const UlongArray& seedValue);		
								//======================================
								// Set dimension information, first argument should be total dim
	void                        setDim(const UintArray& dimValue);

	LAIntegralMC & operator=( const LAIntegralMC & ) { return *this; }

private:
	LARandBase* mpRand;			// pointer to random number generator
protected:
	unsigned long mMCNum;		// Monte Carlo simulation number
};

