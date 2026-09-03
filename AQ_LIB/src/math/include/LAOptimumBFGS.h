#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAOptimumBase.h"


// ID for LAOptimumBFGS
#define FN_OPTIMUMBFGS     1503
// Function name for LAOptimumBFGS
#define FN_OPTIMUMBFGS_STR	"fn_optimumbfgs"



class LAFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration for optimization(minimization) with BFGS method.

    This class realizes pure virtual method "findMinimum" in base class
	and has inner function "lnsrch".

*/
class LAOptimumBFGS : public LAOptimumBase
{
public:
//  LIFECYCLE
	// constructor
	LAOptimumBFGS();
	// destructor
	virtual ~LAOptimumBFGS();

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



	// method to find function minimum by BFGS method.
	virtual double findMinimum(const LAFunctionBase& f,
								DoubleArray& x) const;
	
	// assignment operator
	LAOptimumBFGS & operator=( const LAOptimumBFGS & ) { return *this; }

private:
	// inner function for line search
	void	lnsrch(DoubleArray &xold, const double fold, DoubleArray &g,
					DoubleArray &p, DoubleArray &x, double &f, const double stpmax,
					bool &check, const LAFunctionBase &method) const;
protected:

};

