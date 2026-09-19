#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"

// Funciton ID of AQLLinearMethod
#define FN_QUADRATIC	1223
// Function Name of AQLLinearMethod
#define FN_QUADRATIC_STR	"fn_quadratic"

class AQLQuadraticMethod :
	public AQLFunctionBase
{
public:
	// constructor
	AQLQuadraticMethod();
	// destructor
	virtual ~AQLQuadraticMethod();

	AQLQuadraticMethod & operator=( const AQLQuadraticMethod & ) { return *this; }

								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
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

