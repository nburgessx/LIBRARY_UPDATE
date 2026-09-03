/*! @file
    @brief Class declaration to represent volatility function for SZDD volatility factor

	This class derives from LAMathVolFuncHW

*/
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncSZDDVolFactor.h
//
//  SYNOPSIS    :       LAMathVolFuncSZDDVolFactor
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
#include "LAMathVolFuncSZDDVolFactor.h"
#include "LAModelDynamicsHW1FCurve.h"
#include <algorithm>

using namespace std;
//
//------------------------------ LAMathVolFuncSZDDVolFactor ------------------------------
//

/*!
	@brief constructor

	@param[in] LAMathHWFuncMR 
	@param[in] LAMathHWFuncSigma
	@param[in] integrate_n

*/
LAMathVolFuncSZDDVolFactor::LAMathVolFuncSZDDVolFactor( LAMathHWFuncMR& HW_a_, LAMathHWFuncSigma& HW_s_, LA1DDataSet& HW_theta_)
:
LAMathVolFuncHW(HW_a_, HW_s_),
mpThetaFunc(dynamic_cast<LA1DDataSet*>(HW_theta_.clone()))
{
}




/*!
	@brief destructor
*/
LAMathVolFuncSZDDVolFactor::~LAMathVolFuncSZDDVolFactor(void)
{
	if (mpThetaFunc != 0) delete mpThetaFunc;
}

/*!
	@brief copy constructor
*/
LAMathVolFuncSZDDVolFactor::LAMathVolFuncSZDDVolFactor(const LAMathVolFuncSZDDVolFactor &rhs) 
:
LAMathVolFuncHW(rhs)
{
	mpThetaFunc = rhs.mpThetaFunc == 0 ? 0 : dynamic_cast<LA1DDataSet*>(rhs.mpThetaFunc->clone());
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathVolFuncSZDDVolFactor::clone() const
{
    try 
	{
		return new LAMathVolFuncSZDDVolFactor(*this);
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
LAMathVolFuncSZDDVolFactor::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCSZDDVOLFACTOR ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathVolFuncSZDDVolFactor::getType() const
{
	return FN_VOLFUNCSZDDVOLFACTOR;
}