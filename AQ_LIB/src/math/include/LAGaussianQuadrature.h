#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LA1DIntegral.h"
#include "LAFunctionBase.h"


// ID for LAGaussianQuadrature
#define FN_GAUSSIANQUADRATURE     1403
// Function name for LAGaussianQuadrature
#define FN_GAUSSIANQUADRATURE_STR	"fn_gaussianquadrature"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class for Gaussian quadrature integral methods

    This class itself derives from abstract class "LAIntegralBase". This class has pure
	virtual method "get".

*/
class LAGaussianQuadrature : public LA1DIntegral
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAGaussianQuadrature(unsigned short numberOfPoints);
	// Destructor
	virtual ~LAGaussianQuadrature();

//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const=0;
								//======================================
								// Return this class type
    virtual function_t          getType() const;

	//======================================
	// method for 1-dim integration
	virtual double	integrate(const LAFunctionBase& f,double xl,double xu)const;

	// 20060929 override a virtual function of the base class
	virtual double	integrate(const LAFunctionBase& f,
								const std::vector<std::pair<double,double> >& x) const
					{
						return LA1DIntegral::integrate(f, x);
					};
	virtual double	integrate(const LAFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const
					{
						return LA1DIntegral::integrate(f, xl, xu, grids);
					};

	//======================================
	// pure virtual method for choosing abscissas and weights
	virtual void	get(DoubleArray& abscissas, 
								DoubleArray& weights,
								double a, double b)const =0 ;

protected:
	unsigned short mNumberOfPoints;		// Grid number
};

