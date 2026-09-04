/*! @file
    @brief Source code of class to represent IR volatility function

	This class derives from AQLFunctionBase
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathVolFuncIRSABR.h"
#include "AQLMathSwaptionVolUtility.h"
#include "AQLBasic.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLMathSwaptionSABR.h"


using namespace std;

//================ AQLMathVolFuncIRSABR ===================================
/*!
	@brief constructor

	@param[in] timeGrid 
	@param[in] sigma function value
	@param[in] fx0    function value
	@param[in] beta  function value

*/

AQLMathVolFuncIRSABR::AQLMathVolFuncIRSABR(AQLDataInstance* dataInstance)
: mpDataInstance(dataInstance), mApproxmethod(APPROXIMATION_ANTONOV), mForwardShiftValue(0.)
{
}

/*!
	@brief destructor
*/
AQLMathVolFuncIRSABR::~AQLMathVolFuncIRSABR(void)
{
}


/*!
	@brief copy constructor
*/
AQLMathVolFuncIRSABR::AQLMathVolFuncIRSABR(const AQLMathVolFuncIRSABR &rhs) 
:mpDataInstance(rhs.mpDataInstance),mAlphaID(rhs.mAlphaID),mBetaID(rhs.mBetaID),
mNuID(rhs.mNuID),mRhoID(rhs.mRhoID),mSwapConvID(rhs.mSwapConvID),mCapConvID(rhs.mCapConvID),
mTargetUnderlying(rhs.mTargetUnderlying), mApproxmethod(rhs.mApproxmethod), mForwardShiftValue(rhs.mForwardShiftValue)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathVolFuncIRSABR::clone() const
{
    try 
	{
		//return const_cast<AQLMathVolFuncIRSABR *>(this);
		return new AQLMathVolFuncIRSABR(*this);
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
AQLMathVolFuncIRSABR::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCIRSABR ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathVolFuncIRSABR::getType() const
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
AQLMathVolFuncIRSABR::operator()(const DoubleArray& x) const
{
	
	if (x.size() != 4)
		throw AQLCoreInvalidData("VolFuncSABR size error",__FILE__,__LINE__);

	double ret = AQLMathSwaptionVolUtility::getSABRVol3(mpDataInstance,
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
AQLMathVolFuncIRSABR::setParamID(const AQLString& ID, const AQLString& paramName)
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
		AQLString msg = paramName + "is not sabr parmeter!";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

}

/*!
    @brief set target underlying
    @return void
*/
void	
AQLMathVolFuncIRSABR::setUnderlying(const AQLString& underlying) const
{
	AQLString tmpUnderlying = underlying;
	mTargetUnderlying = tmpUnderlying.toLower();
}

/*!
    @brief set target underlying
    @return void
*/
void	
AQLMathVolFuncIRSABR::setApproxmethod(const AQLString& approxmethod)
{
	AQLString tmpApproxmethod = approxmethod;
	mApproxmethod = tmpApproxmethod.toUpper();
}

/*!
    @brief set target underlying
    @return void
*/
void	
AQLMathVolFuncIRSABR::setForwardShiftValue(const double forwardShiftValue)
{
	mForwardShiftValue = forwardShiftValue;
}


/*!
    @return swapconv id
*/
AQLString	
AQLMathVolFuncIRSABR::getSwapConvID(void) const
{
	std::map<AQLString, AQLString>::const_iterator it = mSwapConvID.find(mTargetUnderlying);
	if (it == mSwapConvID.end())
	{
		AQLString msg = mTargetUnderlying + " is not set in swap convention ID!";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	return it->second;
}

/*!
    @return capconv id
*/
AQLString	
AQLMathVolFuncIRSABR::getCapConvID(void) const
{
	std::map<AQLString, AQLString>::const_iterator it = mCapConvID.find(mTargetUnderlying);
	if (it == mCapConvID.end())
	{
		AQLString msg = mTargetUnderlying + " is not set in cap convention ID!";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	return it->second;
}

/*!
	@param[in] paramName	sabr parameter name
    @return sabr parmeter ID
*/
AQLString	
AQLMathVolFuncIRSABR::getParamID(const AQLString& paramName) const
{
	std::map<AQLString, AQLString>::const_iterator it;
	if (paramName == SABR_ALPHA)
	{
		it = mAlphaID.find(mTargetUnderlying);
		if (it == mAlphaID.end())
		{
			AQLString msg = mTargetUnderlying + "is not set in " + SABR_ALPHA + "!";
			throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
		}
	}
	else if (paramName == SABR_BETA)
	{
		it = mBetaID.find(mTargetUnderlying);
		if (it == mBetaID.end())
		{
			AQLString msg = mTargetUnderlying + "is not set in " + SABR_BETA + "!";
			throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
		}
	}
	else if (paramName == SABR_NU)
	{
		it = mNuID.find(mTargetUnderlying);
		if (it == mNuID.end())
		{
			AQLString msg = mTargetUnderlying + "is not set in " + SABR_NU + "!";
			throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
		}
	}
	else if (paramName == SABR_RHO)
	{
		it = mRhoID.find(mTargetUnderlying);
		if (it == mRhoID.end())
		{
			AQLString msg = mTargetUnderlying + "is not set in " + SABR_RHO + "!";
			throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
		}
	}
	else
	{
		AQLString msg = paramName + "is not sabr parmeter!";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
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
AQLMathVolFuncIRSABR::getSABRParam(const AQLString& paramName, double expPoint, double tenorPoint) const
{
	// get tenor vector and expiry vector
	const AQLString& alphaID = getParamID(SABR_ALPHA);
	const DoubleVector& tenorVec = dynamic_cast<const AQLDataDoubles& >
		(mpDataInstance->getObjectPool().getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
	const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
		(mpDataInstance->getObjectPool().getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
	
	// get parmeter matrix
	const AQLString& paramID = getParamID(paramName);
	const DoubleMatrix& paramMat = dynamic_cast<AQLDataDoubleMatrix& >
		(mpDataInstance->getObjectPool().getObject( paramID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	AQLMathSwaptionMatrix paramMat_(paramMat, expiryVec, tenorVec);

	return paramMat_.lookUpMatrix(expPoint, tenorPoint);
}
