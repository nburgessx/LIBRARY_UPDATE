////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLQuadraticMethod.cpp
//
//  SYNOPSIS    :       AQLQuadraticMethod
//  DESCRIPTION :       Source code of class to represent quadratic function
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


#include "AQLQuadraticFunc.h"

using namespace std;


/*!
	@brief default constructor
*/
AQLQuadraticMethod::AQLQuadraticMethod() 
: AQLFunctionBase()
{
}

/*!
	@brief destructor
*/
AQLQuadraticMethod::~AQLQuadraticMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLQuadraticMethod::clone() const
{
    try 
	{
		return new AQLQuadraticMethod(*this);
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
AQLQuadraticMethod::isTypeOf(function_t id) const
{
	return (id == FN_QUADRATIC ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLQuadraticMethod::getType() const
{
	return FN_QUADRATIC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value : a*x1^2+b*x2^2+c*x1*x2+d*x1+e*x2+f
*/
double
AQLQuadraticMethod::operator()(const DoubleArray& x) const
{
	if (x.size() != 2)
	{
		throw AQLCoreInvalidData("AQLQuadraticMethod: Input data size must be 2.", __FILE__, __LINE__);
	}
	if (mParam.size() != 6)
	{
		throw AQLCoreInvalidData("AQLQuadraticMethod: Parameter data size must be 6.", __FILE__, __LINE__);
	}
	const double ret = x[0] * x[0] * mParam[0] + x[1] * x[1] * mParam[1] + x[0] * x[1] * mParam[2]
					+ x[0] * mParam[3] + x[1] * mParam[4] + mParam[5];
	return ret;
}
