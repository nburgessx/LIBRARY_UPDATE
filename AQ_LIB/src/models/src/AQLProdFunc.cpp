/*! @file
    @brief Source code of class to represent Product function

    This class derives from AQLFunctionBase

*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLProdFunc.h"
#include <algorithm>

using namespace std;
//================ AQLProductMethod ===================================
/*!
	@brief default constructor
*/
AQLProductMethod::AQLProductMethod() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
AQLProductMethod::~AQLProductMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLProductMethod::clone() const
{
    try 
	{
		return new AQLProductMethod(*this);
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
AQLProductMethod::isTypeOf(function_t id) const
{
	return (id == FN_PRODUCT ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLProductMethod::getType() const
{
	return FN_PRODUCT;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLProductMethod::operator()(const DoubleArray& x) const
{
	double ret = 1.0;
	for (unsigned int i = 0; i < x.size() ; i++) ret *= x[i];
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
AQLProductMethod::partialDerivative(const DoubleArray& x, unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (pos >= x.size())
	{
        throw AQLCoreInvalidData("pos is over x size", __FILE__, __LINE__);
	}
	
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);
	else
	{
		double ret = 1.0;
		for (unsigned int i = 0; i < x.size() ; i++) 
			if (i != pos) ret *= x[i];
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
AQLProductMethod::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		return 0;
	}
}
