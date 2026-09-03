#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAGaussianQuadrature.h"


// ID for LAGaussHermite
#define FN_GAUSSHERMITE     1405
// Function name for LAGaussHermite
#define FN_GAUSSHERMITE_STR	"fn_gausshermite"


class LAFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration of integration by Gauss Legendre methods

	This class realizes pure virtual method "get" from a base class.

*/
class LAGaussHermite : public LAGaussianQuadrature
{
public:
//  LIFECYCLE
	// constructor
	explicit LAGaussHermite(unsigned short numberOfPoints);
	// destructor
	virtual ~LAGaussHermite();

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



	// Calculate abscissas and weights
	virtual void	get(DoubleArray& abscissas, 
								DoubleArray& weights,
								double a = NEGATIVE_INFINITY, double b = POSITIVE_INFINITY) 
								const;
private:

protected:

};

