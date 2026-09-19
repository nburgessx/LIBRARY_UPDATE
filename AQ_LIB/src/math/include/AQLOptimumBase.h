#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCoreFunctionBase.h"
#include <limits>


#include "AQLOptimumBase.h"

#define OPTIMUM_PRECISION	std::numeric_limits<double>::epsilon()*100 // convergence requirement on zeroing the value

#define OPTIMUM_ITMAX	200// number of iteration

#define GTOL	std::numeric_limits<double>::epsilon()*100	// convergence requirement on zeroing the gradient

#define STPMX	100// scaled maximum step length allowed in line searches

#define ALF		0.0001// ensures sufficient decrease in function value in lnsrch
						//recommendation by Numerical Recipe

#define ZEPS	std::numeric_limits<double>::epsilon()*0.001 
						// small number against accidental coincident

#define WIDTH	0.0001// small width for mnbrak

// ID for AQLOptimumBase
#define FN_OPTIMUMBASE     1501

// Function name for AQLOptimumBase
#define FN_OPTIMUMBASE_STR	"fn_optimumbase"



class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of optimization option
*/
class AQLOptimumOption
{
public:
	AQLOptimumOption();
	virtual ~AQLOptimumOption();	
	
	double mPrecision;		// precision to be required
	unsigned int mIterMax;	// maximum iteration number
	double mGTol;			// convergence requirement on zeroing the gradient
	unsigned int mStepMax;	// scaled maximum step length allowed in line searches
	double mAlf;			// ensures sufficient decrease in function value in lnsrch
	double mZeps;			// small number against accidental coincident
	double mWidth;			// small width
    bool mException;        // throw exception flag

	bool mBoundary;			// boundary flag;
	DoubleVector mBoundHigh;// high ranges for each variables
	DoubleVector mBoundLow;// < low ranges for each variables
};

/*! 
    @brief Declare abstract base class for optimization(minimization)

    This class has a pure virtual method "findMinimum" which searchs the minimum point
	and return the minimum value.

*/
class AQLOptimumBase : public AQLCoreFunctionBase
{
public:
	// costructor
	AQLOptimumBase();
	// destructor
	virtual ~AQLOptimumBase();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const =0;
								//======================================
								// Return this class type
    virtual function_t          getType() const;



								// pure virtual method for optimizatioin(minimization)
	virtual double				findMinimum(const AQLFunctionBase& f,
											DoubleArray& x)const = 0;
								// Return option for optimum.
	AQLOptimumOption&			getOption() {return mOption;};


private:

protected:
	AQLOptimumOption mOption;	// option for numerical methods
};

