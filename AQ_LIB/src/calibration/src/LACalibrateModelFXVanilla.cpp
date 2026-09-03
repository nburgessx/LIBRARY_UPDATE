/*! @file
    @brief FXVanilla sde generator class
*/
//  2007, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelFXVanilla.cpp
//
//  DESCRIPTION :        FXVanilla SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LACalibrateModelFXVanilla.h"
#include "LAFunctionBase.h"
#include "LAFunctionManager.h"
#include "LAMathVolFuncBase.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceDriftLMMSpot.h"
#include "LAPriceDriftFX.h"
#include "LARatesSpotSDE.h"
#include "LARatesLJSpotSDE.h"
#include "LARatesEulerMaruyama.h"
#include "LACalibrateModelFX.h"
#include "LADefinitionsPtberg.h"
#include "LAMarketData.h"
#include "LAMathVolFuncFX.h"
#include "LAMathFXAdjuster.h"
#include "LAMathVolatility.h"
#include "LAScenarioConfiguration.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "LAConstant.h"
#include "LAPriceFXVolatility.h"
#include "LAPriceFXDDIntegral.h"
#include "LAPriceFXDDIntegralMelstein.h"

using namespace std;

// constructor
/*!

*/
LACalibrateModelFXVanilla::LACalibrateModelFXVanilla()
: LACalibrateModelFX()
{
}

// destructor
/*!

*/
LACalibrateModelFXVanilla::~LACalibrateModelFXVanilla(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
LARatesSDEBase *
LACalibrateModelFXVanilla::createSDEInstance(const LAString &fx, LADataInstance &dataInstance) const
{
	return 0;
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
LACalibrateModelFXVanilla::getSDEType(const LAString &fx) const
{
	SDE_TYPE a;
	return a;
}

/*!
	@brief check is Long Jump

	@param[in] fx

*/
bool 
LACalibrateModelFXVanilla::isLJ(const LAString &fx) const
{
	return false;
}

/*!
	@brief set volatility to sde

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFXVanilla::setVolatility(const LAString &fx, LARatesSDEBase &sde) const
{
	fx;sde;
	return;
}

/*!
	@brief set drift class to sde

	note: fx's first cccy (ex.ccy1/cccy2 ccy2) must be domestic ccy

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelFXVanilla::setDrift(const LAString &fx, LARatesSDEBase &sde) const
{
	fx;sde;
	return;
}



/*!
	@brief set integral function

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelFXVanilla::setIntegralFunction(const LAString &fx, LARatesSDEBase &sde) const
{
	fx;sde;
	return;
}

// 
/*!
    @brief set path

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFXVanilla::setOutputTemplate(const LAString &fx, LARatesSDEBase &sde) const
{
	fx;sde;
	return;
}

// 
/*!
    @brief set setInterpolationMethod

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFXVanilla::setInterpolationMethod(const LAString &fx, LARatesSDEBase &sde) const
{
	fx;sde;
	return;
}



/*!
	@brief get function master regist name

	@param[in] fx
*/
LAString 
LACalibrateModelFXVanilla::getFunctionMasterResistName(const LAString &fx) const
{
	LAString tmpFX = fx;
	return fx;
	
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
LAString
LACalibrateModelFXVanilla::getVolType(const LAString &fx) const
{
	return INPUT_FUNC;
}

// 
/*!
    @brief set volatility function

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelFXVanilla::setUpVolFunc(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const
{
	fx;
	vol;
	dataInstance;
}


// 
/*!
    @brief setup volatility data

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelFXVanilla::setUpVolData(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const
{
	fx;
	vol;
	dataInstance;
}

// 
/*!
    @brief  setup volatility object

	@param[in] fx
	@param[out] vol

*/
void
LACalibrateModelFXVanilla::setUpVolEntity(const LAString &fx, LAMathVolatility &vol) const
{
	//// set interpolation
	//LAStringVector ccys = fx.toToken(FX_DELIMITER);
	//LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	//LAString interp = "fn_linearinterpolation";
	//vol.getInterpolation().convertFromString(interp);
	fx;
	vol;
}

