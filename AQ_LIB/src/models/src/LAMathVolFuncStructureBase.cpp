/*! @file
    @brief Source code of class to represent base structure volatility function

	This class derives from LAFunctionBase

*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncStructureBase.h
//
//  SYNOPSIS    :       LAMathVolFuncStructureBase
//  DESCRIPTION :       Class declaration to represent base structure volatility function
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathVolFuncStructureBase.h"
#include "LABasic.h"

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
: LAFunctionBase(), ma(a), mb(b), mc(c), md(d)
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
: LAFunctionBase(), ma(rhs.ma), mb(rhs.mb), mc(rhs.mc), md(rhs.md)
{

}

/*!
    @brief make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathVolFuncStructureBase::clone() const
{
    try 
	{
		return new LAMathVolFuncStructureBase(*this);
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
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
	return (id == FN_VOLFUNCSTRUCTUREBASE ? true : LAFunctionBase::isTypeOf(id));
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
		throw LACoreInvalidData("The argument DoubleArray's size must be more than zero !", __FILE__, __LINE__);
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
	return ((ma + mb * t) * LAMath::exp(-mc * t) + md);
}
