#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLGaussianQuadrature.h"
#include "AQLCoreFunctionBase.h"


// ID for AQLGaussLaguerre
#define FN_GAUSSLAGUERRE     1405
// Function name for AQLGaussLaguerre
#define FN_GAUSSLAGUERRE_STR	"fn_gausslaguerre"


class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration of integration by Gauss Legendre methods

	This class realizes pure virtual method "get" from a base class.

*/
class AQLGaussLaguerre : public AQLGaussianQuadrature
{
public:
	// constructor
	explicit AQLGaussLaguerre(unsigned short numberOfPoints);
	// destructor
	virtual ~AQLGaussLaguerre();

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
								DoubleArray& weights,double a=POSITIVE_INFINITY, 
								double b=0.0)const;
private:

protected:

};

