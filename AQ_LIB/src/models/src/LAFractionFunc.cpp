////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAFractionMethod.cpp
//
//  SYNOPSIS    :       LAFractionMethod
//  DESCRIPTION :       Source code of class to represent digital function
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


#include "LAFractionFunc.h"
#include "LABasic.h"
#include <algorithm>

using namespace std;
//================ LAFractionMethod ===================================
/*!
@brief default constructor
*/
LAFractionMethod::LAFractionMethod()
	: LAFunctionBase()
{

}

/*!
@brief destructor
*/
LAFractionMethod::~LAFractionMethod()
{
}

/*!
@brief Make copy(clone) of this class
@return Deep copy of this class
*/
LACoreFunctionBase*
LAFractionMethod::clone() const
{
	try
	{
		return new LAFractionMethod(*this);
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
LAFractionMethod::isTypeOf(function_t id) const
{
	return (id == FN_FRACTION ? true : LAFunctionBase::isTypeOf(id));
}

/*!
@brief Return this function type
@return function type
*/
function_t
LAFractionMethod::getType() const
{
	return FN_FRACTION;
}

/*!
@brief Return function value
@param[in] x, y index
@return a1 * y + a2 * y + a3 + a4 / x + a5 / y + a6 * y / x + a7 * x / y
*/
double
LAFractionMethod::operator()(const DoubleArray& x) const
{
	if (mParam.size() != 7 || x.size() != 2)
		throw LACoreInvalidData("parameter size should be equal to 7 and index size should be 2.", __FILE__, __LINE__);

	double threshold = 1E-5;

	double ret = mParam[0] * x[0] + mParam[1] * x[1] + mParam[2] +
		mParam[3] / (LAMath::abs(x[0]) < threshold ? threshold * LAMath::sign(1., x[0]) : x[0]) +
		mParam[4] / (LAMath::abs(x[1]) < threshold ? threshold * LAMath::sign(1., x[1]) : x[1]) +
		mParam[5] * x[1] / (LAMath::abs(x[0]) < threshold ? threshold * LAMath::sign(1., x[0]) : x[0]) +
		mParam[6] * x[0] / (LAMath::abs(x[1]) < threshold ? threshold * LAMath::sign(1., x[1]) : x[1]);

	return ret;
}