/*! @file
    @brief Source code for class to represent constant function

	This class derives from LAFunctionBase
*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAConstant.cpp
//
//  SYNOPSIS    :       LAConstant
//  DESCRIPTION :       Source code for class to represent constant function
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


#include "LAConstant.h"

using namespace std;
//================ LAConstant ===================================
/*!
	@brief default constructor
*/
LAConstant::LAConstant() 
: LAFunctionBase()
{
}
/*!
	@brief constructor
	@param[in] x function value
*/
LAConstant::LAConstant(double x) 
: LAFunctionBase()
{
	set(x);
}

/*!
	@brief destructor
*/
LAConstant::~LAConstant() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAConstant::clone() const
{
    try 
	{
		return new LAConstant(*this);
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
LAConstant::isTypeOf(function_t id) const
{
	return (id == FN_CONSTANT ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAConstant::getType() const
{
	return FN_CONSTANT;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAConstant::operator()(const DoubleArray& x) const
{
	(void)x;
	return mParam[0];
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAConstant::operator()(double x) const
{
	(void)x;
	return mParam[0];
}

/*!
    @brief Set function value
	@param[in] x function value to be set
*/
void
LAConstant::set(double x)
{
	mParam.resize(1);
	mParam[0] = x;
}

/*!
    @brief Set function value
	@param[in] param param[0]function value to be set
*/
void
LAConstant::setParam(const DoubleArray& param)
{
	if (param.size() != 1)
	{
		throw LACoreInvalidData("Data size must be one", __FILE__, __LINE__);
	}
	LAFunctionBase::setParam(param);
}

/*!
    @brief Assignment operator
	@param[in] x assigned function value
*/
LAConstant&
LAConstant::operator=(double x)
{
	set(x);
	return *this;
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
LAConstant::integral(const std::vector<std::pair<double,double> >& x)const
{
	double ret = 1.0;
	for (unsigned int i = 0; i < x.size(); i++)
	{
		ret *= x[i].second - x[i].first;
	}
	ret *= mParam[0];
	return ret;
}

/*!
    @brief Return integral result
	@param[in] xl lower side of integral region
	@param[in] xu upper side of integral region
    @return integral result
*/
double
LAConstant::integral(double xl, double xu) const
{
	return mParam[0] * (xu - xl);
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
LAConstant::partialDerivative(const DoubleArray& x,unsigned int pos,
										CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const
{
	(void)x; (void)pos; (void)delta; (void)difftype; (void)calctype;
	return 0;
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
LAConstant::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	(void)x; (void)calctype; (void)posi; (void)posj; (void)delta;
	return 0;
}
			
