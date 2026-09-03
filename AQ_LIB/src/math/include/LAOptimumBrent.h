#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAOptimumBase.h"


// ID for LAOptimumBrent
#define FN_OPTIMUMBRENT     1502
// Function name for LAOptimumBrent
#define FN_OPTIMUMBRENT_STR	"fn_optimumbrent"


class LAFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration for a 1-D function optimization(minimization) with Brent method.

	This class realizes pure virtual method "findMinimum" in base class.
	This class sets a default search region in constructor and can modify it
	with "setRegion". There are inner functions, "mnbrak" for set-up and "shft3"
	for change value procedure.

*/
class LAOptimumBrent : public LAOptimumBase
{
public:
//  LIFECYCLE
	// constructor
	LAOptimumBrent();
	LAOptimumBrent(double initPoint, double lwBound, double upBound, unsigned int maxIter, double tol);
	// destructor
	virtual ~LAOptimumBrent();

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



	// method to find minimum
	virtual double findMinimum(const LAFunctionBase& f,
								DoubleArray& x) const;

	// assignment operator
	LAOptimumBrent & operator=( const LAOptimumBrent & ) { return *this; }

	// set search regioin for finding minimum.
//	virtual void setRegion(const double a, const double b);

private:
//	double	mRegion_min; // minimum value of search region
//	double	mRegion_max; // maximum value of search region

	// select suitable initial points for Brent method
	virtual void mnbrak(double &a, double &b, double &c, const LAFunctionBase &f) const;

	// change value
	inline void shft3(double &a, double &b, double &c, const double d) const
	{
		a = b; b = c; c = d;
	}

    bool mControlRange = false;
    double mLwBound, mUpBound, mInitPoint;

protected:

};

