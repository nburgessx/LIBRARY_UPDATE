/*! @file
    @brief Source code of class to represent base structure volatility function

	This class derives from AQLFunctionBase

*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncStructureBase.h
//
//  SYNOPSIS    :       LAMathVolFuncStructureBase
//  DESCRIPTION :       Class declaration to represent base structure volatility function
//						This class derives from AQLFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathVolFuncStructureBase.h"
#include "AQLBasic.h"

using namespace std;

//================ LAMathVolFuncStructureBase ===================================
/*!
	@brief constructor
	@param[in] a
	@param[in] b
	@param[in] c
	@param[in] d
*/

LAMathVolFuncStructureBase::LAMathVolFuncStructureBase(double a, double b, double c, double d)
: AQLFunctionBase(), ma(a), mb(b), mc(c), md(d)
{
}

/*!
	@brief destructor
*/
LAMathVolFuncStructureBase::~LAMathVolFuncStructureBase(void)
{

}


/*!
	@brief copy constructor
*/
LAMathVolFuncStructureBase::LAMathVolFuncStructureBase(const LAMathVolFuncStructureBase &rhs) 
: AQLFunctionBase(), ma(rhs.ma), mb(rhs.mb), mc(rhs.mc), md(rhs.md)
{

}

/*!
    @brief make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAMathVolFuncStructureBase::clone() const
{
    try 
	{
		return new LAMathVolFuncStructureBase(*this);
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
LAMathVolFuncStructureBase::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCSTRUCTUREBASE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathVolFuncStructureBase::getType() const
{
	return FN_VOLFUNCSTRUCTUREBASE;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
LAMathVolFuncStructureBase::operator()(const DoubleArray& x) const
{
	if (static_cast<int>(x.size()) < 1)
	{
		throw AQLCoreInvalidData("The argument DoubleArray's size must be more than zero !", __FILE__, __LINE__);
	}

	return operator()(x[0]);
}


/*!
    @brief return function value
	@param[in] t term
    @return function value
*/
double
LAMathVolFuncStructureBase::operator()(double t) const
{
	return ((ma + mb * t) * AQLMath::exp(-mc * t) + md);
}
