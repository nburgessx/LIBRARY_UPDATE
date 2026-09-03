/*! @file
    @brief Source code of class to represent 1D polynomial function

    This class derives from AQLFunctionBase

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQL1DPolynomialMethod.cpp
//
//  SYNOPSIS    :       AQL1DPolynomialMethod
//  DESCRIPTION :       Source code of class to represent 1D polynomial function
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


#include "AQL1DPolynomialFunc.h"
#include "AQLBasic.h"
#include "AQLDist.h"

using namespace std;
//================ AQL1DPolynomialMethod ===================================
/*!
	@brief default constructor
*/
AQL1DPolynomialMethod::AQL1DPolynomialMethod() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
AQL1DPolynomialMethod::~AQL1DPolynomialMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQL1DPolynomialMethod::clone() const
{
    try 
	{
		return new AQL1DPolynomialMethod(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQL1DPolynomialMethod::isTypeOf(function_t id) const
{
	return (id == FN_POLYNOMIAL ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQL1DPolynomialMethod::getType() const
{
	return FN_POLYNOMIAL;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQL1DPolynomialMethod::operator()(const DoubleArray& x) const
{
	if ((mCheckFlag && x.size() != 1) || x.size() == 0)
	{
		throw AQLCoreInvalidData("x size is wrong", __FILE__, __LINE__);
	}
	return operator()(x[0]);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQL1DPolynomialMethod::operator()(double x) const
{
	double ret = 0.0;
	double xx = 1.0;
	for (unsigned int i = 0; i < mParam.size(); i++)
	{
		ret += mParam[i] * xx;
		xx *= x;
	}
	return ret;
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
AQL1DPolynomialMethod::integral(const std::vector<std::pair<double,double> >& x)const
{
	if ((mCheckFlag && x.size() != 1) || x.size() == 0)
	{
		throw AQLCoreInvalidData("vecotr size must be one", __FILE__, __LINE__);
	}
	return integral(x[0].first, x[0].second);
}

/*!
    @brief Return integral result
	@param[in] xl lower side of integral region
	@param[in] xu upper side of integral region
    @return integral result
*/
double
AQL1DPolynomialMethod::integral(double xl,double xu) const
{
	if (xu < xl) return -integral(xu, xl);
	
	double ret = 0.0;
	double xxl = xl;
	double xxu = xu;
	for (unsigned int i = 0; i < mParam.size(); i++)
	{
		ret += mParam[i] / (i + 1.0) * (xxu - xxl);
		xxl *= xl;
		xxu *= xu;
	}
	return ret;

}

/*!
    @brief Return partial derivative value
	@param[in] x integral point
	@param[in] pos variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] difftype both side or one side
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
AQL1DPolynomialMethod::partialDerivative(const DoubleArray& x, unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (pos != 0 || x.size() != 1)
	{
		throw AQLCoreInvalidData("x or pos are something wrong", __FILE__, __LINE__);
	}
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative(x,pos,calctype,difftype,delta);
	else
	{
		double ret = 0.0;
		double xx = 1.0;
		for (unsigned int i = 1; i < mParam.size(); i++)
		{
			ret += i * mParam[i] * xx;
			xx *= x[0];
		}
		return ret;
	}

}

/*!
    @brief Return second partial derivative value
	@param[in] x integral point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
    @return second partial deribative value
*/
double
AQL1DPolynomialMethod::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (posi != 0 || posj != 0 || x.size() != 1)
	{
		throw AQLCoreInvalidData("x or pos are something wrong", __FILE__, __LINE__);
	}
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		double ret = 0.0;
		double xx = 1.0;
		for (unsigned int i = 2; i < mParam.size(); i++)
		{
			ret += i * (i - 1) * mParam[i] * xx;
			xx *= x[0];
		}
		return ret;
	}
}

