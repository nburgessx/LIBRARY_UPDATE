#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAGaussianQuadrature.h"


// ID for LAGaussLegendre
#define FN_GAUSSLEGENDRE     1404
// Function name for LAGaussLegendre
#define FN_GAUSSLEGENDRE_STR	"fn_gausslegendre"


class LAFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration of integration by Gauss Legendre methods

	This class realizes pure virtual method "get" from a base class.

*/
class LAGaussLegendre : public LAGaussianQuadrature
{
public:
//  LIFECYCLE
	// constructor
	explicit LAGaussLegendre(unsigned short numberOfPoints);
	// destructor
	virtual ~LAGaussLegendre();

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
								double a, double b)const;

    //// Integrate by splitting intervals
    //double IntegrateBySplit(const LAFunctionBase& f, double xl, double xu, int splitSize = 10) const;

private:

protected:

};

