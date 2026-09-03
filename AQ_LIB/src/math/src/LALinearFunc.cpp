/*! @file
    @brief Source code of class to represent linear function

    This class derives from LAFunctionBase

*/
//  2006, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LALinearMethod.cpp
//
//  SYNOPSIS    :       LALinearMethod
//  DESCRIPTION :       Source code of class to represent linear function
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


#include "LALinearFunc.h"
#include "LABasic.h"
#include "LADist.h"

using namespace std;
//================ LALinearMethod ===================================
/*!
	@brief default constructor
*/
LALinearMethod::LALinearMethod() 
: LAFunctionBase()
{

}

/*!
	@brief destructor
*/
LALinearMethod::~LALinearMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LALinearMethod::clone() const
{
    try 
	{
		return new LALinearMethod(*this);
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
LALinearMethod::isTypeOf(function_t id) const
{
	return (id == FN_LINEAR ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LALinearMethod::getType() const
{
	return FN_LINEAR;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LALinearMethod::operator()(const DoubleArray& x) const
{
	if (x.size() + 1 != mParam.size())
	{
		throw LACoreInvalidData("input data size must be parameter size minus one", __FILE__, __LINE__);
	}
    double ret = mParam.back();
	for (unsigned int i = 0; i < x.size(); i++)
		ret += x[i] * mParam[i];
	return ret;
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
LALinearMethod::integral(const std::vector<std::pair<double,double> >& x)const
{
	if (x.size() + 1 != mParam.size())
	{
		throw LACoreInvalidData("input data size must be parameter size minus one", __FILE__, __LINE__);
	}
	double multiple = 1.0;
	for (unsigned int i = 0; i < x.size(); i++)
		multiple *= x[i].second - x[i].first;
	double ret = mParam.back();
	for (unsigned int i = 0; i < x.size(); i++)
		ret += 0.5 * (x[i].second + x[i].first);
	ret *= multiple;

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
LALinearMethod::partialDerivative(const DoubleArray& x, unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);
	else
	{
		return mParam.at(pos);
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
LALinearMethod::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		return 0;
	}
}

