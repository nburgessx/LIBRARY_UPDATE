#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"

// Funciton ID of LALinearMethod
#define FN_QUADRATIC	1223
// Function Name of LALinearMethod
#define FN_QUADRATIC_STR	"fn_quadratic"

class LAQuadraticMethod :
	public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LAQuadraticMethod();
	// destructor
	virtual ~LAQuadraticMethod();

	LAQuadraticMethod & operator=( const LAQuadraticMethod & ) { return *this; }

								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								
	                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================

private:

protected:

};

