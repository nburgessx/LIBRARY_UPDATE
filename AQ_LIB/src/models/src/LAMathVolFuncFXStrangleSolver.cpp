/*! @file
    @brief Source code of class to represent FX volatility function

	This class derives from LAFunctionBase

*/
//  2007, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncFXStrangleSolver.h
//
//  SYNOPSIS    :       LAMathVolFuncFXStrangleSolver
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


#include "LAMathVolFuncFXStrangleSolver.h"
#include "LAMathFXVolatilitySurfaceGenerate.h"
#include "LABasic.h"

using namespace std;

//================ LAMathVolFuncFXStrangleSolver ===================================
/*!
	@brief constructor

	@param[in] timeGrid 
	@param[in] sigma function value
	@param[in] fx0    function value
	@param[in] beta  function value

*/
LAMathVolFuncFXStrangleSolver::LAMathVolFuncFXStrangleSolver(const InterpolationMethod &method, const InterpolationTarget &target, 
									const InterpolationVariable &variable, const ATMInterpolationMethod &atmMethod, 
									const std::vector<FXOptionData> &opdata, const std::vector<SmileData> &smiledata,
									bool iswing)
: LAFunctionBase(), mMethod(method), mTarget(target), mVariable(variable),mAtmMethod(atmMethod),
mOpData(opdata),mSmileData(smiledata), mIsWing(iswing)
{
	mAtmMethod = atmMethod;
}

/*!
	@brief destructor
*/
LAMathVolFuncFXStrangleSolver::~LAMathVolFuncFXStrangleSolver(void)
{
}


/*!
	@brief copy constructor
*/
LAMathVolFuncFXStrangleSolver::LAMathVolFuncFXStrangleSolver(const LAMathVolFuncFXStrangleSolver &rhs) 
: LAFunctionBase(), mMethod(rhs.mMethod), mTarget(rhs.mTarget), mVariable(rhs.mVariable), mAtmMethod(rhs.mAtmMethod),
mOpData(rhs.mOpData), mSmileData(rhs.mSmileData), mIsWing(rhs.mIsWing)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathVolFuncFXStrangleSolver::clone() const
{
    try 
	{
		//return const_cast<LAMathVolFuncFXStrangleSolver *>(this);
		return new LAMathVolFuncFXStrangleSolver(*this);
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
LAMathVolFuncFXStrangleSolver::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCFXSTRANGLESOLV ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathVolFuncFXStrangleSolver::getType() const
{
	return FN_VOLFUNCFXSTRANGLESOLV;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value

	x[1] is fx value
*/
double
LAMathVolFuncFXStrangleSolver::operator()(const DoubleArray& x) const
{
	
	if (x.size() == 1)
		return LAMathFXVolatilitySurfaceGenerate::GetATMVolatility(x[0],mAtmMethod,mOpData,mSmileData);

	double ret = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation
										(x[0],x[1],mMethod,mTarget,mVariable,mAtmMethod,mOpData,mSmileData, mIsWing);
	return ret;
}

