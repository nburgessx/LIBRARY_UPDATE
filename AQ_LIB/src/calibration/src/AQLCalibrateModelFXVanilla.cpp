/*! @file
    @brief FXVanilla sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateModelFXVanilla.cpp
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


#include "AQLCalibrateModelFXVanilla.h"
#include "AQLFunctionBase.h"
#include "AQLFunctionManager.h"
#include "AQLMathVolFuncBase.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDriftLMMSpot.h"
#include "AQLPriceDriftFX.h"
#include "AQLRatesSpotSDE.h"
#include "AQLRatesLJSpotSDE.h"
#include "AQLRatesEulerMaruyama.h"
#include "AQLCalibrateModelFX.h"
#include "AQLDefinitionsPtberg.h"
#include "AQLMarketData.h"
#include "AQLMathVolFuncFX.h"
#include "AQLMathFXAdjuster.h"
#include "AQLMathVolatility.h"
#include "AQLScenarioConfiguration.h"
#include "AQLDealUtils.h"
#include "AQLStaticData.h"
#include "AQLConstant.h"
#include "AQLPriceFXVolatility.h"
#include "AQLPriceFXDDIntegral.h"
#include "AQLPriceFXDDIntegralMelstein.h"

using namespace std;

// constructor
/*!

*/
AQLCalibrateModelFXVanilla::AQLCalibrateModelFXVanilla()
: AQLCalibrateModelFX()
{
}

// destructor
/*!

*/
AQLCalibrateModelFXVanilla::~AQLCalibrateModelFXVanilla(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
AQLRatesSDEBase *
AQLCalibrateModelFXVanilla::createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	return 0;
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
AQLCalibrateModelFXVanilla::getSDEType(const AQLString &fx) const
{
	SDE_TYPE a;
	return a;
}

/*!
	@brief check is Long Jump

	@param[in] fx

*/
bool 
AQLCalibrateModelFXVanilla::isLJ(const AQLString &fx) const
{
	return false;
}

/*!
	@brief set volatility to sde

	@param[in] fx
	@param[out] sde
*/
void
AQLCalibrateModelFXVanilla::setVolatility(const AQLString &fx, AQLRatesSDEBase &sde) const
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
AQLCalibrateModelFXVanilla::setDrift(const AQLString &fx, AQLRatesSDEBase &sde) const
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
AQLCalibrateModelFXVanilla::setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde) const
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
AQLCalibrateModelFXVanilla::setOutputTemplate(const AQLString &fx, AQLRatesSDEBase &sde) const
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
AQLCalibrateModelFXVanilla::setInterpolationMethod(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	fx;sde;
	return;
}



/*!
	@brief get function master regist name

	@param[in] fx
*/
AQLString 
AQLCalibrateModelFXVanilla::getFunctionMasterResistName(const AQLString &fx) const
{
	AQLString tmpFX = fx;
	return fx;
	
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
AQLString
AQLCalibrateModelFXVanilla::getVolType(const AQLString &fx) const
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
AQLCalibrateModelFXVanilla::setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
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
AQLCalibrateModelFXVanilla::setUpVolData(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
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
AQLCalibrateModelFXVanilla::setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const
{
	//// set interpolation
	//AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	//AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	//AQLString interp = "fn_linearinterpolation";
	//vol.getInterpolation().convertFromString(interp);
	fx;
	vol;
}

