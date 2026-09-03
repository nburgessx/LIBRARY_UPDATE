#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLGaussianQuadrature.h"


// ID for AQLGaussLegendre
#define FN_GAUSSLEGENDRE     1404
// Function name for AQLGaussLegendre
#define FN_GAUSSLEGENDRE_STR	"fn_gausslegendre"


class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration of integration by Gauss Legendre methods

	This class realizes pure virtual method "get" from a base class.

*/
class AQLGaussLegendre : public AQLGaussianQuadrature
{
public:
//  LIFECYCLE
	// constructor
	explicit AQLGaussLegendre(unsigned short numberOfPoints);
	// destructor
	virtual ~AQLGaussLegendre();

//  QUERY
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;



	// Calculate abscissas and weights
	virtual void	get(DoubleArray& abscissas, 
								DoubleArray& weights,
								double a, double b)const;

    //// Integrate by splitting intervals
    //double IntegrateBySplit(const AQLFunctionBase& f, double xl, double xu, int splitSize = 10) const;

private:

protected:

};

