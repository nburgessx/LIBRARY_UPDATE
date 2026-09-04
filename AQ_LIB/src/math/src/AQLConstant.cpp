/*! @file
    @brief Source code for class to represent constant function

	This class derives from AQLFunctionBase
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLConstant.h"

using namespace std;
//================ AQLConstant ===================================
/*!
	@brief default constructor
*/
AQLConstant::AQLConstant() 
: AQLFunctionBase()
{
}
/*!
	@brief constructor
	@param[in] x function value
*/
AQLConstant::AQLConstant(double x) 
: AQLFunctionBase()
{
	set(x);
}

/*!
	@brief destructor
*/
AQLConstant::~AQLConstant() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLConstant::clone() const
{
    try 
	{
		return new AQLConstant(*this);
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
AQLConstant::isTypeOf(function_t id) const
{
	return (id == FN_CONSTANT ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLConstant::getType() const
{
	return FN_CONSTANT;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLConstant::operator()(const DoubleArray& x) const
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
AQLConstant::operator()(double x) const
{
	(void)x;
	return mParam[0];
}

/*!
    @brief Set function value
	@param[in] x function value to be set
*/
void
AQLConstant::set(double x)
{
	mParam.resize(1);
	mParam[0] = x;
}

/*!
    @brief Set function value
	@param[in] param param[0]function value to be set
*/
void
AQLConstant::setParam(const DoubleArray& param)
{
	if (param.size() != 1)
	{
		throw AQLCoreInvalidData("Data size must be one", __FILE__, __LINE__);
	}
	AQLFunctionBase::setParam(param);
}

/*!
    @brief Assignment operator
	@param[in] x assigned function value
*/
AQLConstant&
AQLConstant::operator=(double x)
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
AQLConstant::integral(const std::vector<std::pair<double,double> >& x)const
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
AQLConstant::integral(double xl, double xu) const
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
AQLConstant::partialDerivative(const DoubleArray& x,unsigned int pos,
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
AQLConstant::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	(void)x; (void)calctype; (void)posi; (void)posj; (void)delta;
	return 0;
}
			
