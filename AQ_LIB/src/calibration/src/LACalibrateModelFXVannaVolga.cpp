/*! @file
    @brief FXVannaVolga sde generator class
*/
//  2011, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelFXVannaVolga.cpp
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


#include "LACalibrateModelFXVannaVolga.h"
#include "AQLFunctionBase.h"
#include "AQLFunctionManager.h"
#include "LAMathVolFuncBase.h"
#include "AQLPriceDataInterpolation.h"
#include "LAPriceDriftFX.h"
#include "LARatesSpotSDE.h"
#include "LACalibrateModelFX.h"
#include "LADefinitionsPtberg.h"
#include "LAMarketData.h"
#include "LAMathVolFuncFX.h"
#include "LAMathVolatility.h"
#include "LAScenarioConfiguration.h"
#include "LACalibrateVolatilityFXVannaVolga.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "AQLConstant.h"
#include "LAPriceFXVolatility.h"
#include "LACalibrationParametersFXVannaVolga.h"
#include "LACalibrateFXVannaVolga.h"
#include "LACalibrationFunc.h"

using namespace std;

// constructor
/*!

*/
LACalibrateModelFXVannaVolga::LACalibrateModelFXVannaVolga()
: LACalibrateModelFX()
{
}

// destructor
/*!

*/
LACalibrateModelFXVannaVolga::~LACalibrateModelFXVannaVolga(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
LARatesSDEBase *
LACalibrateModelFXVannaVolga::createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	return 0;
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
LACalibrateModelFXVannaVolga::getSDEType(const AQLString &fx) const
{
	SDE_TYPE a;
	return a;
}

/*!
	@brief check is Long Jump

	@param[in] fx

*/
bool 
LACalibrateModelFXVannaVolga::isLJ(const AQLString &fx) const
{
	return false;
}

/*!
	@brief set volatility to sde

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFXVannaVolga::setVolatility(const AQLString &fx, LARatesSDEBase &sde) const
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
LACalibrateModelFXVannaVolga::setDrift(const AQLString &fx, LARatesSDEBase &sde) const
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
LACalibrateModelFXVannaVolga::setIntegralFunction(const AQLString &fx, LARatesSDEBase &sde) const
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
LACalibrateModelFXVannaVolga::setOutputTemplate(const AQLString &fx, LARatesSDEBase &sde) const
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
LACalibrateModelFXVannaVolga::setInterpolationMethod(const AQLString &fx, LARatesSDEBase &sde) const
{
	fx;sde;
	return;
}



/*!
	@brief get function master regist name

	@param[in] fx
*/
AQLString 
LACalibrateModelFXVannaVolga::getFunctionMasterResistName(const AQLString &fx) const
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
LACalibrateModelFXVannaVolga::getVolType(const AQLString &fx) const
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
LACalibrateModelFXVannaVolga::setUpVolFunc(const AQLString &fx, LAMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	setUpVolEntity(fx,vol);
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLStringVector fileVec(1);
	
	MAScenarioParam param;
	param.calcType = KEY_PV;
	// set fx
	param.ccy = fx;
	
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	
	param.isCalib = isCalibTarget(fx);
	if (param.isCalib)
	{
		// create calib info
		LACalibrationParametersFXVannaVolga cInfo;
		AQLString cInfoName = cInfo.createCalibrationInfo(dataInstance.getObjectPool(), fx);
		// first element set calib info
		param.refName.push_back(cInfoName);

		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}
		// set yield and calibdata
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
		param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));
	}
	else
	{
		throw AQLCoreInvalidData("Only Calibration supports",__FILE__,__LINE__);
	}
	
	// create function
	LACalibrateVolatilityFXVannaVolga volCreator;
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
LACalibrateModelFXVannaVolga::setUpVolData(const AQLString &fx, LAMathVolatility &vol, AQLDataInstance &dataInstance) const
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
LACalibrateModelFXVannaVolga::setUpVolEntity(const AQLString &fx, LAMathVolatility &vol) const
{
	// set interpolation
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString interp = "fn_linearinterpolation";
	vol.getInterpolation().convertFromString(interp);
}

