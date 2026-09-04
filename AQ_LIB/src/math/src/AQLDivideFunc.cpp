/*! @file
    @brief Source code of class to represent divide function

    This class derives from AQLFunctionBase

*/
//  2012, AlgoQuantHub.

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLDivideFunc.h"
#include "AQLBasic.h"

double boundary = 1E-5;

using namespace std;
//================ AQLDivideMethod ===================================
/*!
	@brief default constructor
*/
AQLDivideMethod::AQLDivideMethod() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
AQLDivideMethod::~AQLDivideMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLDivideMethod::clone() const
{
    try 
	{
		return new AQLDivideMethod(*this);
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
AQLDivideMethod::isTypeOf(function_t id) const
{
	return (id == FN_DIVIDE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLDivideMethod::getType() const
{
	return FN_DIVIDE;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLDivideMethod::operator()(const DoubleArray& x) const
{
	if (x.size() != 1 || mParam.size() != 2)
	{
		throw AQLCoreInvalidData("input data size must be 1 and parameter size must be 2!", __FILE__, __LINE__);
	}

	//if (x[0] == 0.)
	//{
	//	throw AQLCoreInvalidData("input data is zero!", __FILE__, __LINE__);
	//}

	double denominator = x[0];
	if (AQLMath::abs(x[0]) < boundary)
		denominator = boundary * AQLMath::sign(1., x[0]);
    double ret = mParam[0] / denominator + mParam[1];
	return ret;
}