/*! @file
    @brief Source code of class to represent FX volatility function

	This class derives from AQLFunctionBase

*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathVolFuncFXStrangleSolver.h"
#include "AQLMathFXVolatilitySurfaceGenerate.h"
#include "AQLBasic.h"

using namespace std;

//================ AQLMathVolFuncFXStrangleSolver ===================================
/*!
	@brief constructor

	@param[in] timeGrid 
	@param[in] sigma function value
	@param[in] fx0    function value
	@param[in] beta  function value

*/
AQLMathVolFuncFXStrangleSolver::AQLMathVolFuncFXStrangleSolver(const InterpolationMethod &method, const InterpolationTarget &target, 
									const InterpolationVariable &variable, const ATMInterpolationMethod &atmMethod, 
									const std::vector<FXOptionData> &opdata, const std::vector<SmileData> &smiledata,
									bool iswing)
: AQLFunctionBase(), mMethod(method), mTarget(target), mVariable(variable),mAtmMethod(atmMethod),
mOpData(opdata),mSmileData(smiledata), mIsWing(iswing)
{
	mAtmMethod = atmMethod;
}

/*!
	@brief destructor
*/
AQLMathVolFuncFXStrangleSolver::~AQLMathVolFuncFXStrangleSolver(void)
{
}


/*!
	@brief copy constructor
*/
AQLMathVolFuncFXStrangleSolver::AQLMathVolFuncFXStrangleSolver(const AQLMathVolFuncFXStrangleSolver &rhs) 
: AQLFunctionBase(), mMethod(rhs.mMethod), mTarget(rhs.mTarget), mVariable(rhs.mVariable), mAtmMethod(rhs.mAtmMethod),
mOpData(rhs.mOpData), mSmileData(rhs.mSmileData), mIsWing(rhs.mIsWing)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathVolFuncFXStrangleSolver::clone() const
{
    try 
	{
		//return const_cast<AQLMathVolFuncFXStrangleSolver *>(this);
		return new AQLMathVolFuncFXStrangleSolver(*this);
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
AQLMathVolFuncFXStrangleSolver::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCFXSTRANGLESOLV ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathVolFuncFXStrangleSolver::getType() const
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
AQLMathVolFuncFXStrangleSolver::operator()(const DoubleArray& x) const
{
	
	if (x.size() == 1)
		return AQLMathFXVolatilitySurfaceGenerate::GetATMVolatility(x[0],mAtmMethod,mOpData,mSmileData);

	double ret = AQLMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation
										(x[0],x[1],mMethod,mTarget,mVariable,mAtmMethod,mOpData,mSmileData, mIsWing);
	return ret;
}

