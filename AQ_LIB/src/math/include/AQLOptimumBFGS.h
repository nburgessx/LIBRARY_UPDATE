#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLOptimumBase.h"


// ID for AQLOptimumBFGS
#define FN_OPTIMUMBFGS     1503
// Function name for AQLOptimumBFGS
#define FN_OPTIMUMBFGS_STR	"fn_optimumbfgs"



class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration for optimization(minimization) with BFGS method.

    This class realizes pure virtual method "findMinimum" in base class
	and has inner function "lnsrch".

*/
class AQLOptimumBFGS : public AQLOptimumBase
{
public:
	// constructor
	AQLOptimumBFGS();
	// destructor
	virtual ~AQLOptimumBFGS();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;



	// method to find function minimum by BFGS method.
	virtual double findMinimum(const AQLFunctionBase& f,
								DoubleArray& x) const;
	
	// assignment operator
	AQLOptimumBFGS & operator=( const AQLOptimumBFGS & ) { return *this; }

private:
	// inner function for line search
	void	lnsrch(DoubleArray &xold, const double fold, DoubleArray &g,
					DoubleArray &p, DoubleArray &x, double &f, const double stpmax,
					bool &check, const AQLFunctionBase &method) const;
protected:

};

