/*! @file
    @brief Class declaration to represent volatility function for SZDD volatility factor

	This class derives from AQLMathVolFuncHW

*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//+++++ include +++++
#include "AQLMathVolFuncSZDDVolFactor.h"
#include "AQLModelDynamicsHW1FCurve.h"
#include <algorithm>

using namespace std;
//
//------------------------------ AQLMathVolFuncSZDDVolFactor ------------------------------
//

/*!
	@brief constructor

	@param[in] AQLMathHWFuncMR 
	@param[in] AQLMathHWFuncSigma
	@param[in] integrate_n

*/
AQLMathVolFuncSZDDVolFactor::AQLMathVolFuncSZDDVolFactor( AQLMathHWFuncMR& HW_a_, AQLMathHWFuncSigma& HW_s_, AQL1DDataSet& HW_theta_)
:
AQLMathVolFuncHW(HW_a_, HW_s_),
mpThetaFunc(dynamic_cast<AQL1DDataSet*>(HW_theta_.clone()))
{
}




/*!
	@brief destructor
*/
AQLMathVolFuncSZDDVolFactor::~AQLMathVolFuncSZDDVolFactor(void)
{
	if (mpThetaFunc != 0) delete mpThetaFunc;
}

/*!
	@brief copy constructor
*/
AQLMathVolFuncSZDDVolFactor::AQLMathVolFuncSZDDVolFactor(const AQLMathVolFuncSZDDVolFactor &rhs) 
:
AQLMathVolFuncHW(rhs)
{
	mpThetaFunc = rhs.mpThetaFunc == 0 ? 0 : dynamic_cast<AQL1DDataSet*>(rhs.mpThetaFunc->clone());
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathVolFuncSZDDVolFactor::clone() const
{
    try 
	{
		return new AQLMathVolFuncSZDDVolFactor(*this);
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
AQLMathVolFuncSZDDVolFactor::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCSZDDVOLFACTOR ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathVolFuncSZDDVolFactor::getType() const
{
	return FN_VOLFUNCSZDDVOLFACTOR;
}