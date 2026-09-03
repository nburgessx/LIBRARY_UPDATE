/*! @file
    @brief Source code of abstract base class for Gaussian quadrature integral methods

	This class defines pure virtual method "integrate" from abstract class "AQLIntegralBase".
*/



////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLGaussianQuadrature.cpp
//
//  SYNOPSIS    :       AQLGaussianQuadrature
//  DESCRIPTION :       Source code of abstract base class for Gaussian quadrature integral methods
//						This class defines pure virtual method "integrate" from abstract class "AQLIntegralBase".
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


#include "AQLGaussianQuadrature.h"


using namespace std;
//================ AQLGaussianQuadrature ===================================
/*!
	@brief Constructor
	@param[in] numberOfPoints divident number in integral region
*/
AQLGaussianQuadrature::AQLGaussianQuadrature(unsigned short numberOfPoints)
: AQL1DIntegral(),mNumberOfPoints(numberOfPoints)
{
}

/*!
	@brief Destructor
*/
AQLGaussianQuadrature::~AQLGaussianQuadrature() 
{
}

/*!
    @brief Check function for this class type
    @param[in] id class type to check
    @return true or false
*/
bool
AQLGaussianQuadrature::isTypeOf(function_t id) const
{
    return (id == FN_GAUSSIANQUADRATURE ? true : AQLIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return class type
*/
function_t
AQLGaussianQuadrature::getType() const
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
AQLGaussianQuadrature::integrate(const AQLFunctionBase& f,double xl,double xu)const
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
