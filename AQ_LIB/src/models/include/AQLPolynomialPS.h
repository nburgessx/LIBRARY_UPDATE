#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPolynomialBase.h"


// ID for AQLPolynomialPS
#define FN_POLYNOMIALPS	2402 
// Function Name of AQLSumMethod
#define FN_POLYNOMIALPS_STR	"fn_polynomial_ps"


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of power series plynomial class

*/
class AQLPolynomialPS : public AQLPolynomialBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLPolynomialPS(unsigned int order = 1, unsigned int varnum = 1);
	//	Copy constructor
//	AQLPolynomialPS(const AQLPolynomialPS& v);
	// Destructor
	virtual ~AQLPolynomialPS();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// calculate basis function
	virtual void				calcBasisFunc(const DoubleArray& x, DoubleArray& basis_func) const;										
	

	//Remove warning:C4512
	AQLPolynomialPS & operator=( const AQLPolynomialPS & ) { return *this; }


private:

protected:

};

