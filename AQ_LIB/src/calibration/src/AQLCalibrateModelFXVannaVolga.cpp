/*! @file
    @brief FXVannaVolga sde generator class
*/
//  2011, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateModelFXVannaVolga.cpp
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


#include "AQLCalibrateModelFXVannaVolga.h"
#include "AQLFunctionBase.h"
#include "AQLFunctionManager.h"
#include "AQLMathVolFuncBase.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDriftFX.h"
#include "AQLRatesSpotSDE.h"
#include "AQLCalibrateModelFX.h"
#include "AQLDefinitionsPtberg.h"
#include "AQLMarketData.h"
#include "AQLMathVolFuncFX.h"
#include "AQLMathVolatility.h"
#include "AQLScenarioConfiguration.h"
#include "AQLCalibrateVolatilityFXVannaVolga.h"
#include "AQLDealUtils.h"
#include "AQLStaticData.h"
#include "AQLConstant.h"
#include "AQLPriceFXVolatility.h"
#include "AQLCalibrationParametersFXVannaVolga.h"
#include "AQLCalibrateFXVannaVolga.h"
#include "AQLCalibrationFunc.h"

using namespace std;

// constructor
/*!

*/
AQLCalibrateModelFXVannaVolga::AQLCalibrateModelFXVannaVolga()
: AQLCalibrateModelFX()
{
}

// destructor
/*!

*/
AQLCalibrateModelFXVannaVolga::~AQLCalibrateModelFXVannaVolga(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
AQLRatesSDEBase *
AQLCalibrateModelFXVannaVolga::createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	return 0;
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
AQLCalibrateModelFXVannaVolga::getSDEType(const AQLString &fx) const
{
	SDE_TYPE a;
	return a;
}

/*!
	@brief check is Long Jump

	@param[in] fx

*/
bool 
AQLCalibrateModelFXVannaVolga::isLJ(const AQLString &fx) const
{
	return false;
}

/*!
	@brief set volatility to sde

	@param[in] fx
	@param[out] sde
*/
void
AQLCalibrateModelFXVannaVolga::setVolatility(const AQLString &fx, AQLRatesSDEBase &sde) const
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
AQLCalibrateModelFXVannaVolga::setDrift(const AQLString &fx, AQLRatesSDEBase &sde) const
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
AQLCalibrateModelFXVannaVolga::setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde) const
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
AQLCalibrateModelFXVannaVolga::setOutputTemplate(const AQLString &fx, AQLRatesSDEBase &sde) const
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
AQLCalibrateModelFXVannaVolga::setInterpolationMethod(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	fx;sde;
	return;
}



/*!
	@brief get function master regist name

	@param[in] fx
*/
AQLString 
AQLCalibrateModelFXVannaVolga::getFunctionMasterResistName(const AQLString &fx) const
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
AQLCalibrateModelFXVannaVolga::getVolType(const AQLString &fx) const
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
AQLCalibrateModelFXVannaVolga::setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	setUpVolEntity(fx,vol);
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(fx, ccys);

	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLStringVector fileVec(1);
	
	AQLScenarioParam param;
	param.calcType = KEY_PV;
	// set fx
	param.ccy = fx;
	
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	
	param.isCalib = isCalibTarget(fx);
	if (param.isCalib)
	{
		// create calib info
		AQLCalibrationParametersFXVannaVolga cInfo;
		AQLString cInfoName = cInfo.createCalibrationInfo(dataInstance.getObjectPool(), fx);
		// first element set calib info
		param.refName.push_back(cInfoName);

		if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}
		// set yield and calibdata
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, param.refName.back())));
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccys[1]));
		param.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, param.refName.back())));
	}
	else
	{
		throw AQLCoreInvalidData("Only Calibration supports",__FILE__,__LINE__);
	}
	
	// create function
	AQLCalibrateVolatilityFXVannaVolga volCreator;
	AQLFunctionBase *method = volCreator.createVolatility(fileVec, &param, &objPool);
	vol.setVolatility(method);
}


// 
/*!
    @brief setup volatility data

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
AQLCalibrateModelFXVannaVolga::setUpVolData(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
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
AQLCalibrateModelFXVannaVolga::setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const
{
	// set interpolation
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString interp = "fn_linearinterpolation";
	vol.getInterpolation().convertFromString(interp);
}

