/*! @file
    @brief Source code of class to represent IR volatility function

	This class derives from LAFunctionBase

*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncIRSABR.h
//
//  SYNOPSIS    :       LAMathVolFuncIRSABR
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


#include "LAMathVolFuncIRSABR.h"
#include "LAMathSwaptionVolUtility.h"
#include "LABasic.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LAMathSwaptionSABR.h"


using namespace std;

//================ LAMathVolFuncIRSABR ===================================
/*!
	@brief constructor

	@param[in] timeGrid 
	@param[in] sigma function value
	@param[in] fx0    function value
	@param[in] beta  function value

*/

LAMathVolFuncIRSABR::LAMathVolFuncIRSABR(LADataInstance* dataInstance)
: mpDataInstance(dataInstance), mApproxmethod(APPROXIMATION_ANTONOV), mForwardShiftValue(0.)
{
}

/*!
	@brief destructor
*/
LAMathVolFuncIRSABR::~LAMathVolFuncIRSABR(void)
{
}


/*!
	@brief copy constructor
*/
LAMathVolFuncIRSABR::LAMathVolFuncIRSABR(const LAMathVolFuncIRSABR &rhs) 
:mpDataInstance(rhs.mpDataInstance),mAlphaID(rhs.mAlphaID),mBetaID(rhs.mBetaID),
mNuID(rhs.mNuID),mRhoID(rhs.mRhoID),mSwapConvID(rhs.mSwapConvID),mCapConvID(rhs.mCapConvID),
mTargetUnderlying(rhs.mTargetUnderlying), mApproxmethod(rhs.mApproxmethod), mForwardShiftValue(rhs.mForwardShiftValue)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathVolFuncIRSABR::clone() const
{
    try 
	{
		//return const_cast<LAMathVolFuncIRSABR *>(this);
		return new LAMathVolFuncIRSABR(*this);
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
LAMathVolFuncIRSABR::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCIRSABR ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathVolFuncIRSABR::getType() const
{
	return FN_VOLFUNCIRSABR;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value

	x[1] is fx value
*/
double
LAMathVolFuncIRSABR::operator()(const DoubleArray& x) const
{
	
	if (x.size() != 4)
		throw LACoreInvalidData("VolFuncSABR size error",__FILE__,__LINE__);

	double ret = LAMathSwaptionVolUtility::getSABRVol3(mpDataInstance,
													x[0],x[1],x[2],x[3], 
													getParamID(SABR_ALPHA),
													getParamID(SABR_BETA),
													getParamID(SABR_NU),
													getParamID(SABR_RHO),
													mApproxmethod);
	return ret;
}

/*!
    @brief set sabr parmeter ID
    @return void
*/
void	
LAMathVolFuncIRSABR::setParamID(const LAString& ID, const LAString& paramName)
{
	if (paramName == SABR_ALPHA)
	{
		mAlphaID[mTargetUnderlying] = ID;
	}
	else if (paramName == SABR_BETA)
	{
		mBetaID[mTargetUnderlying] = ID;
	}
	else if (paramName == SABR_NU)
	{
		mNuID[mTargetUnderlying] = ID;
	}
	else if (paramName == SABR_RHO)
	{
		mRhoID[mTargetUnderlying] = ID;
	}
	else
	{
		LAString msg = paramName + "is not sabr parmeter!";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

}

/*!
    @brief set target underlying
    @return void
*/
void	
LAMathVolFuncIRSABR::setUnderlying(const LAString& underlying) const
{
	LAString tmpUnderlying = underlying;
	mTargetUnderlying = tmpUnderlying.toLower();
}

/*!
    @brief set target underlying
    @return void
*/
void	
LAMathVolFuncIRSABR::setApproxmethod(const LAString& approxmethod)
{
	LAString tmpApproxmethod = approxmethod;
	mApproxmethod = tmpApproxmethod.toUpper();
}

/*!
    @brief set target underlying
    @return void
*/
void	
LAMathVolFuncIRSABR::setForwardShiftValue(const double forwardShiftValue)
{
	mForwardShiftValue = forwardShiftValue;
}


/*!
    @return swapconv id
*/
LAString	
LAMathVolFuncIRSABR::getSwapConvID(void) const
{
	std::map<LAString, LAString>::const_iterator it = mSwapConvID.find(mTargetUnderlying);
	if (it == mSwapConvID.end())
	{
		LAString msg = mTargetUnderlying + " is not set in swap convention ID!";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	return it->second;
}

/*!
    @return capconv id
*/
LAString	
LAMathVolFuncIRSABR::getCapConvID(void) const
{
	std::map<LAString, LAString>::const_iterator it = mCapConvID.find(mTargetUnderlying);
	if (it == mCapConvID.end())
	{
		LAString msg = mTargetUnderlying + " is not set in cap convention ID!";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	return it->second;
}

/*!
	@param[in] paramName	sabr parameter name
    @return sabr parmeter ID
*/
LAString	
LAMathVolFuncIRSABR::getParamID(const LAString& paramName) const
{
	std::map<LAString, LAString>::const_iterator it;
	if (paramName == SABR_ALPHA)
	{
		it = mAlphaID.find(mTargetUnderlying);
		if (it == mAlphaID.end())
		{
			LAString msg = mTargetUnderlying + "is not set in " + SABR_ALPHA + "!";
			throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
		}
	}
	else if (paramName == SABR_BETA)
	{
		it = mBetaID.find(mTargetUnderlying);
		if (it == mBetaID.end())
		{
			LAString msg = mTargetUnderlying + "is not set in " + SABR_BETA + "!";
			throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
		}
	}
	else if (paramName == SABR_NU)
	{
		it = mNuID.find(mTargetUnderlying);
		if (it == mNuID.end())
		{
			LAString msg = mTargetUnderlying + "is not set in " + SABR_NU + "!";
			throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
		}
	}
	else if (paramName == SABR_RHO)
	{
		it = mRhoID.find(mTargetUnderlying);
		if (it == mRhoID.end())
		{
			LAString msg = mTargetUnderlying + "is not set in " + SABR_RHO + "!";
			throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
		}
	}
	else
	{
		LAString msg = paramName + "is not sabr parmeter!";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	return it->second;
}

/*!
	@param[in] paramName	sabr parameter name
	@param[in] expPoint		expiry point for interpolation
	@param[in] tenorPoint	tenor point for interpolation
	@return sabr parameter
*/
double
LAMathVolFuncIRSABR::getSABRParam(const LAString& paramName, double expPoint, double tenorPoint) const
{
	// get tenor vector and expiry vector
	const LAString& alphaID = getParamID(SABR_ALPHA);
	const DoubleVector& tenorVec = dynamic_cast<const LADataDoubles& >
		(mpDataInstance->getObjectPool().getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
	const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
		(mpDataInstance->getObjectPool().getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
	
	// get parmeter matrix
	const LAString& paramID = getParamID(paramName);
	const DoubleMatrix& paramMat = dynamic_cast<LADataDoubleMatrix& >
		(mpDataInstance->getObjectPool().getObject( paramID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	LAMathSwaptionMatrix paramMat_(paramMat, expiryVec, tenorVec);

	return paramMat_.lookUpMatrix(expPoint, tenorPoint);
}
