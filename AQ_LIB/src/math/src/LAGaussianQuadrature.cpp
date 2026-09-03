/*! @file
    @brief Source code of abstract base class for Gaussian quadrature integral methods

	This class defines pure virtual method "integrate" from abstract class "LAIntegralBase".
*/



////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAGaussianQuadrature.cpp
//
//  SYNOPSIS    :       LAGaussianQuadrature
//  DESCRIPTION :       Source code of abstract base class for Gaussian quadrature integral methods
//						This class defines pure virtual method "integrate" from abstract class "LAIntegralBase".
//                      
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAGaussianQuadrature.h"


using namespace std;
//================ LAGaussianQuadrature ===================================
/*!
	@brief Constructor
	@param[in] numberOfPoints divident number in integral region
*/
LAGaussianQuadrature::LAGaussianQuadrature(unsigned short numberOfPoints)
: LA1DIntegral(),mNumberOfPoints(numberOfPoints)
{
}

/*!
	@brief Destructor
*/
LAGaussianQuadrature::~LAGaussianQuadrature() 
{
}

/*!
    @brief Check function for this class type
    @param[in] id class type to check
    @return true or false
*/
bool
LAGaussianQuadrature::isTypeOf(function_t id) const
{
    return (id == FN_GAUSSIANQUADRATURE ? true : LAIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return class type
*/
function_t
LAGaussianQuadrature::getType() const
{
    return FN_GAUSSIANQUADRATURE;  
}


/*!
    @brief  Integration by Gaussian quadrature methods.

    @param[in] f target function
	@param[in] xl integral lower bound
	@param[in] xu integral upper bound

	@return integral result
*/
double
LAGaussianQuadrature::integrate(const LAFunctionBase& f,double xl,double xu)const
{
	if (xl > xu)return integrate(f, xu, xl);

	DoubleArray xx(mNumberOfPoints);
	DoubleArray ww(mNumberOfPoints);
	
	get(xx, ww, xl, xu);

	DoubleArray method(mNumberOfPoints);
    for (unsigned short i = 0; i < mNumberOfPoints; i++) method[i] = f(xx[i]);
    double ret = 0.0;
	for(unsigned int i = 0; i < method.size(); i++)
		ret += method[i] * ww[i];
	return ret;
}
