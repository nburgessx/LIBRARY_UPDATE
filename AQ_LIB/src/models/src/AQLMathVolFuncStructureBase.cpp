/*! @file
    @brief Source code of class to represent base structure volatility function

	This class derives from AQLFunctionBase

*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathVolFuncStructureBase.h"
#include "AQLBasic.h"

using namespace std;

//================ AQLMathVolFuncStructureBase ===================================
/*!
	@brief constructor
	@param[in] a
	@param[in] b
	@param[in] c
	@param[in] d
*/

AQLMathVolFuncStructureBase::AQLMathVolFuncStructureBase(double a, double b, double c, double d)
: AQLFunctionBase(), ma(a), mb(b), mc(c), md(d)
{
}

/*!
	@brief destructor
*/
AQLMathVolFuncStructureBase::~AQLMathVolFuncStructureBase(void)
{

}


/*!
	@brief copy constructor
*/
AQLMathVolFuncStructureBase::AQLMathVolFuncStructureBase(const AQLMathVolFuncStructureBase &rhs) 
: AQLFunctionBase(), ma(rhs.ma), mb(rhs.mb), mc(rhs.mc), md(rhs.md)
{

}

/*!
    @brief make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathVolFuncStructureBase::clone() const
{
    try 
	{
		return new AQLMathVolFuncStructureBase(*this);
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
AQLMathVolFuncStructureBase::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCSTRUCTUREBASE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathVolFuncStructureBase::getType() const
{
	return FN_VOLFUNCSTRUCTUREBASE;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
AQLMathVolFuncStructureBase::operator()(const DoubleArray& x) const
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
AQLMathVolFuncStructureBase::operator()(double t) const
{
	return ((ma + mb * t) * AQLMath::exp(-mc * t) + md);
}
