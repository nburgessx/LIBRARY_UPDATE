/*! @file
    @brief Source code of class to represent divide function

    This class derives from LAFunctionBase

*/
//  2012, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADivideMethod.cpp
//
//  SYNOPSIS    :       LADivideMethod
//  DESCRIPTION :       Source code of class to represent divide function
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


#include "LADivideFunc.h"
#include "LABasic.h"

double boundary = 1E-5;

using namespace std;
//================ LADivideMethod ===================================
/*!
	@brief default constructor
*/
LADivideMethod::LADivideMethod() 
: LAFunctionBase()
{

}

/*!
	@brief destructor
*/
LADivideMethod::~LADivideMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LADivideMethod::clone() const
{
    try 
	{
		return new LADivideMethod(*this);
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
LADivideMethod::isTypeOf(function_t id) const
{
	return (id == FN_DIVIDE ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LADivideMethod::getType() const
{
	return FN_DIVIDE;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LADivideMethod::operator()(const DoubleArray& x) const
{
	if (x.size() != 1 || mParam.size() != 2)
	{
		throw LACoreInvalidData("input data size must be 1 and parameter size must be 2!", __FILE__, __LINE__);
	}

	//if (x[0] == 0.)
	//{
	//	throw LACoreInvalidData("input data is zero!", __FILE__, __LINE__);
	//}

	double denominator = x[0];
	if (LAMath::abs(x[0]) < boundary)
		denominator = boundary * LAMath::sign(1., x[0]);
    double ret = mParam[0] / denominator + mParam[1];
	return ret;
}