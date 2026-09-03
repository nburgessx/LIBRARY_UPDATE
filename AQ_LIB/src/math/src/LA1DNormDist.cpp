/*! @file
    @brief Source code to represent nomarl distribution function
	
	This class derives from LAFunctionBase, and implement pure virtual method "operator()"

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LA1DNormDist.cpp
//
//  SYNOPSIS    :       LA1DNormDist
//  DESCRIPTION :       Source code to represent normal distribution function
//						This class derives from LAFunctionBase, and implement
//						pure virtual method "operator()"
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


#include "LA1DNormDist.h"
#include "LABasic.h"
#include "LADist.h"

using namespace std;
//================ LA1DNormDist ===================================
/*!
	@brief default constructor
*/
LA1DNormDist::LA1DNormDist() 
: LAFunctionBase()
{
}

/*!
	@brief constructor
	@param[in] myu average
	@param[in] sigma standard deviation
*/
LA1DNormDist::LA1DNormDist(double myu,double sigma) 
: LAFunctionBase()
{
	DoubleArray param(2);
	param[0] = myu;
	param[1] = sigma;
	mParam = param;
}

/*!
	@brief destructor
*/
LA1DNormDist::~LA1DNormDist() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LA1DNormDist::clone() const
{
    try 
	{
		return new LA1DNormDist(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LA1DNormDist::isTypeOf(function_t id) const
{
	return (id == FN_1DNORMDIST ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LA1DNormDist::getType() const
{
	return FN_1DNORMDIST;
}

/*!
    @brief Return distribution density 1/(2pi)^(0.5)exp[-1/(2sigma)(x-myu)^2]
	@param[in] x point
    @return distribution density
*/
double
LA1DNormDist::operator()(const DoubleArray& x) const
{
	if ((mCheckFlag && x.size() != 1) || x.size() == 0)
	{
		throw LACoreInvalidData("x size is wrong", __FILE__, __LINE__);
	}
	return operator()(x[0]);
}

/*!
    @brief Return distribution density 1/(2pi)^(0.5)exp[-1/(2sigma)(x-myu)^2]
	@param[in] x point
    @return distribution density
*/
double
LA1DNormDist::operator()(double x) const
{
	return LAMath::exp(-0.5 * LAMath::sqr(
		(x - mParam[0]) / mParam[1])) / mParam[1] / LAMath::sqrt(2 * LAMath::pi());
}

/*!
    @brief Set parameters
	@param[in] param param[0]myu, param[1]sigma
*/
void
LA1DNormDist::setParam(const DoubleArray& param)
{
	if (param.size() != 2)
	{
		throw LACoreInvalidData("Data size must be two", __FILE__, __LINE__);
	}
	mParam = param;
}
/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
LA1DNormDist::integral(const std::vector<std::pair<double,double> >& x)const
{
	if (x.size() != 1)
	{
		throw LACoreInvalidData("vecotr size must be one", __FILE__, __LINE__);
	}
	return LAFunctionBase::integral(x[0].first, x[0].second);
}
/*!
    @brief Return integral result
	@param[in] xl lower side of integral region
	@param[in] xu upper side of integral region
    @return integral result
*/
double
LA1DNormDist::integral(double xl,double xu) const
{
	if (xu < xl) return -integral(xu, xl);
	
	if (xl != NEGATIVE_INFINITY && xu != POSITIVE_INFINITY)
		return LADist::normsdist((xu - mParam[0]) / mParam[1])
				- LADist::normsdist((xl - mParam[0]) / mParam[1]);
	else if (xl == NEGATIVE_INFINITY && xu != POSITIVE_INFINITY)
		return LADist::normsdist((xu - mParam[0]) / mParam[1]);
	else if (xl != NEGATIVE_INFINITY && xu == POSITIVE_INFINITY)
		return LADist::normsdist((-xl - mParam[0]) / mParam[1]);
	else return 1.0;
}


/*!
    @brief Return partial derivative value
	@param[in] x: integral point
	@param[in] pos: variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] difftype both side or one side
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
LA1DNormDist::partialDerivative(const DoubleArray& x, unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (pos != 0 || x.size() != 1)
	{
		throw LACoreInvalidData("x or pos are something wrong", __FILE__, __LINE__);
	}
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);
	else
		return operator()(x[0]) * -(x[0] - mParam[0]) / mParam[1] / mParam[1];

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
LA1DNormDist::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (posi != 0 || posj != 0 || x.size() != 1)
	{
		throw LACoreInvalidData("x or pos are something wrong", __FILE__, __LINE__);
	}
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		double tmp = partialDerivative(x, 0, ANALYTICAL);
		return tmp * -(x[0] - mParam[0]) / mParam[1] / mParam[1]
				- operator()(x[0]) / mParam[1] / mParam[1];
	}
}
