/*! @file
    @brief Source code of class to represent HW volatility function

	This class derives from AQLFunctionBase
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//+++++ include +++++
#include "AQLMathVolFuncHW.h"
#include "AQLModelDynamicsHW1FCurve.h"
#include <algorithm>

using namespace std;
//
//------------------------------ AQLMathVolFuncHW ------------------------------
//

/*!
	@brief constructor

	@param[in] AQLMathHWFuncMR 
	@param[in] AQLMathHWFuncSigma
	@param[in] integrate_n

*/
AQLMathVolFuncHW::AQLMathVolFuncHW( AQLMathHWFuncMR& HW_a_, AQLMathHWFuncSigma& HW_s_)
:
AQLFunctionBase(),
mGL(HWGAUSSLEGENDRENUM),
is_cloned(false)
{
	mpforVar = new AQLMathHWFuncToolForVar(HW_a_,HW_s_);
}




/*!
	@brief destructor
*/
AQLMathVolFuncHW::~AQLMathVolFuncHW(void)
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
AQLMathVolFuncHW::AQLMathVolFuncHW(const AQLMathVolFuncHW &rhs) 
:
AQLFunctionBase(),
mGL(rhs.mGL),
mpforVar(rhs.mpforVar != 0 ? dynamic_cast<AQLMathHWFuncToolForVar*>(rhs.mpforVar->clone()) : 0),
is_cloned(true),
integrate_cache(rhs.integrate_cache),
is_cache(rhs.is_cache)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathVolFuncHW::clone() const
{
    try 
	{
		return new AQLMathVolFuncHW(*this);
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
AQLMathVolFuncHW::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCHW ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathVolFuncHW::getType() const
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
AQLMathVolFuncHW::operator()(const DoubleArray& x) const
{
	double te = x[1];
	if ( !is_cache[te] )
    {
		double ts = x[0];
		double val = AQLMath::sqrt(mGL.integrate((*mpforVar),ts,te)/(te-ts));
		is_cache[te] = true;
		integrate_cache[te] = val;
    }
    return integrate_cache[te];
}