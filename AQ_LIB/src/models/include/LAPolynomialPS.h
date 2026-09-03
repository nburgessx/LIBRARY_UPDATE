#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPolynomialBase.h"


// ID for LAPolynomialPS
#define FN_POLYNOMIALPS	2402 
// Function Name of LASumMethod
#define FN_POLYNOMIALPS_STR	"fn_polynomial_ps"


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of power series plynomial class

*/
class LAPolynomialPS : public LAPolynomialBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPolynomialPS(unsigned int order = 1, unsigned int varnum = 1);
	//	Copy constructor
//	LAPolynomialPS(const LAPolynomialPS& v);
	// Destructor
	virtual ~LAPolynomialPS();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// calculate basis function
	virtual void				calcBasisFunc(const DoubleArray& x, DoubleArray& basis_func) const;										
	

	//Remove warning:C4512
	LAPolynomialPS & operator=( const LAPolynomialPS & ) { return *this; }


private:

protected:

};

