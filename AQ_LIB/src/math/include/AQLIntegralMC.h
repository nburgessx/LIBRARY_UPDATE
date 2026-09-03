#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLIntegralBase.h"


// ID for AQLIntegralMC
#define FN_INTEGRALMC     1407
// Function name for AQLIntegralMC
#define FN_INTEGRALMC_STR	"fn_integralmc"


class AQLFunctionBase;
class AQLRandBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to integrate with Monte Carlo method
	
	This class derives from AQLIntegralBase

*/
class AQLIntegralMC : public AQLIntegralBase
{
public:
//  LIFECYCLE
	// constructor
	AQLIntegralMC(unsigned long num, const AQLRandBase& v);
	// destructor
	virtual ~AQLIntegralMC();
	// copy constructor
	AQLIntegralMC(const AQLIntegralMC& v);

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

								//======================================
								// Return integral result
	virtual double				integrate(const AQLFunctionBase& f,
											const std::vector<std::pair<double,double> >& x) const;
// OPERATION
								//======================================
                                // Set random number generator
	void						setRandGenerator(const AQLRandBase& v);
								//======================================
								// Set Monte Carlo simulation number
	void						setMCNum(unsigned long num) {mMCNum = num;};
								//======================================
								// Set seed
	void				        setSeed(const UlongArray& seedValue);		
								//======================================
								// Set dimension information, first argument should be total dim
	void                        setDim(const UintArray& dimValue);

	AQLIntegralMC & operator=( const AQLIntegralMC & ) { return *this; }

private:
	AQLRandBase* mpRand;			// pointer to random number generator
protected:
	unsigned long mMCNum;		// Monte Carlo simulation number
};

