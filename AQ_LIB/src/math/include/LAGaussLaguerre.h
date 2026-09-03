#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAGaussianQuadrature.h"
#include "LACoreFunctionBase.h"


// ID for LAGaussLaguerre
#define FN_GAUSSLAGUERRE     1405
// Function name for LAGaussLaguerre
#define FN_GAUSSLAGUERRE_STR	"fn_gausslaguerre"


class LAFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration of integration by Gauss Legendre methods

	This class realizes pure virtual method "get" from a base class.

*/
class LAGaussLaguerre : public LAGaussianQuadrature
{
public:
//  LIFECYCLE
	// constructor
	explicit LAGaussLaguerre(unsigned short numberOfPoints);
	// destructor
	virtual ~LAGaussLaguerre();

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
								DoubleArray& weights,double a=POSITIVE_INFINITY, 
								double b=0.0)const;
private:

protected:

};

