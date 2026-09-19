#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLGaussianQuadrature.h"


// ID for AQLGaussHermite
#define FN_GAUSSHERMITE     1405
// Function name for AQLGaussHermite
#define FN_GAUSSHERMITE_STR	"fn_gausshermite"


class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration of integration by Gauss Legendre methods

	This class realizes pure virtual method "get" from a base class.

*/
class AQLGaussHermite : public AQLGaussianQuadrature
{
public:
	// constructor
	explicit AQLGaussHermite(unsigned short numberOfPoints);
	// destructor
	virtual ~AQLGaussHermite();

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
								double a = NEGATIVE_INFINITY, double b = POSITIVE_INFINITY) 
								const;
private:

protected:

};

