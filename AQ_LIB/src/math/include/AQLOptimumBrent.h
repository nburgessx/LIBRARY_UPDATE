#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLOptimumBase.h"


// ID for AQLOptimumBrent
#define FN_OPTIMUMBRENT     1502
// Function name for AQLOptimumBrent
#define FN_OPTIMUMBRENT_STR	"fn_optimumbrent"


class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration for a 1-D function optimization(minimization) with Brent method.

	This class realizes pure virtual method "findMinimum" in base class.
	This class sets a default search region in constructor and can modify it
	with "setRegion". There are inner functions, "mnbrak" for set-up and "shft3"
	for change value procedure.

*/
class AQLOptimumBrent : public AQLOptimumBase
{
public:
	// constructor
	AQLOptimumBrent();
	AQLOptimumBrent(double initPoint, double lwBound, double upBound, unsigned int maxIter, double tol);
	// destructor
	virtual ~AQLOptimumBrent();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;



	// method to find minimum
	virtual double findMinimum(const AQLFunctionBase& f,
								DoubleArray& x) const;

	// assignment operator
	AQLOptimumBrent & operator=( const AQLOptimumBrent & ) { return *this; }

	// set search regioin for finding minimum.
//	virtual void setRegion(const double a, const double b);

private:
//	double	mRegion_min; // minimum value of search region
//	double	mRegion_max; // maximum value of search region

	// select suitable initial points for Brent method
	virtual void mnbrak(double &a, double &b, double &c, const AQLFunctionBase &f) const;

	// change value
	inline void shft3(double &a, double &b, double &c, const double d) const
	{
		a = b; b = c; c = d;
	}

    bool mControlRange = false;
    double mLwBound, mUpBound, mInitPoint;

protected:

};

