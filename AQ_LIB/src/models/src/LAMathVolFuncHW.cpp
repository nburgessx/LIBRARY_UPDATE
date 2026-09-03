/*! @file
    @brief Source code of class to represent HW volatility function

	This class derives from LAFunctionBase

*/

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncHW.h
//
//  SYNOPSIS    :       LAMathVolFuncHW
//  DESCRIPTION :       Source code of class  to represent volatility of FX
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//+++++ include +++++
#include "LAMathVolFuncHW.h"
#include "LAModelDynamicsHW1FCurve.h"
#include <algorithm>

using namespace std;
//
//------------------------------ LAMathVolFuncHW ------------------------------
//

/*!
	@brief constructor

	@param[in] LAMathHWFuncMR 
	@param[in] LAMathHWFuncSigma
	@param[in] integrate_n

*/
LAMathVolFuncHW::LAMathVolFuncHW( LAMathHWFuncMR& HW_a_, LAMathHWFuncSigma& HW_s_)
:
LAFunctionBase(),
mGL(HWGAUSSLEGENDRENUM),
is_cloned(false)
{
	mpforVar = new LAMathHWFuncToolForVar(HW_a_,HW_s_);
}




/*!
	@brief destructor
*/
LAMathVolFuncHW::~LAMathVolFuncHW(void)
{
	delete mpforVar;
	/*   if(is_cloned)
    {
		delete mpforVar;
    }*/
}

/*!
	@brief copy constructor
*/
LAMathVolFuncHW::LAMathVolFuncHW(const LAMathVolFuncHW &rhs) 
:
LAFunctionBase(),
mGL(rhs.mGL),
mpforVar(rhs.mpforVar != 0 ? dynamic_cast<LAMathHWFuncToolForVar*>(rhs.mpforVar->clone()) : 0),
is_cloned(true),
integrate_cache(rhs.integrate_cache),
is_cache(rhs.is_cache)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathVolFuncHW::clone() const
{
    try 
	{
		return new LAMathVolFuncHW(*this);
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
LAMathVolFuncHW::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCHW ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathVolFuncHW::getType() const
{
	return FN_VOLFUNCHW;
}

/*!
    @brief return function value
	@param[in] x point
    @return function value

	x[1] is fx value
*/
double
LAMathVolFuncHW::operator()(const DoubleArray& x) const
{
	double te = x[1];
	if ( !is_cache[te] )
    {
		double ts = x[0];
		double val = LAMath::sqrt(mGL.integrate((*mpforVar),ts,te)/(te-ts));
		is_cache[te] = true;
		integrate_cache[te] = val;
    }
    return integrate_cache[te];
}