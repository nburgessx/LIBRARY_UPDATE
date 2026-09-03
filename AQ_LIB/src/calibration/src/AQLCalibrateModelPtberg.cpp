/*! @file
    @brief Ptberg sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateModelPtberg.cpp
//
//  DESCRIPTION :        Ptberg SDE generator 
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


#include "AQLCalibrateModelPtberg.h"
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
#include "AQLCalibrateVolatilityPtberg.h"
#include "AQLDealUtils.h"
#include "AQLStaticData.h"
#include "AQLConstant.h"
#include "AQLPriceFXVolatility.h"
#include "AQLPriceFXDDIntegral.h"
#include "AQLPriceFXDDIntegralMelstein.h"
#include "AQLCalibrationParametersPtberg.h"

using namespace std;

// constructor
/*!

*/
AQLCalibrateModelPtberg::AQLCalibrateModelPtberg()
: AQLCalibrateModelFX()
{
}

// destructor
/*!

*/
AQLCalibrateModelPtberg::~AQLCalibrateModelPtberg(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
AQLRatesSDEBase *
AQLCalibrateModelPtberg::createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	dataInstance;
	SDE_TYPE type = getSDEType(fx);
	// check LJ
	if (isLJ(fx))
	{
		return new AQLRatesLJSpotSDE(type);
	}
	else
	{
		return new AQLRatesSpotSDE(type);
	}
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
AQLCalibrateModelPtberg::getSDEType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString type = mpStaticData->getStaticData(key_fx + FX_KEY_PTBERG_TYPE);
	type.toUpper();
	if (type == "DX/X")
	{
		return DIVIDEdXbyX;
	}
	else if (type == "DX")
	{
		return dX;
	}
	else
	{
		AQLString msg = AQLString("Sde type is not support. type = ") + type;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

/*!
	@brief check is Long Jump

	@param[in] fx

*/
bool 
AQLCalibrateModelPtberg::isLJ(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString type = mpStaticData->getStaticData(key_fx + FX_KEY_PTBERG_INTEGRAL_STEP);
	type.toUpper();
	if (type == "LONGJUMP")
	{
		return true;
	}
	else
	{
		return false;
	}

}

/*!
	@brief set volatility to sde

	@param[in] fx
	@param[out] sde
*/
void
AQLCalibrateModelPtberg::setVolatility(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString sdeName = mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_NAME);

	vector<vector<AQLFunctionBase *> > volMtx(1);
	volMtx[0].push_back(new AQLMathVolFuncBase(sdeName, 0, 0, true));
	sde.setVolatility(volMtx);
}

/*!
	@brief set drift class to sde

	note: fx's first cccy (ex.ccy1/cccy2 ccy2) must be domestic ccy

	@param[in] fx
	@param[out] sde

*/
void
AQLCalibrateModelPtberg::setDrift(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(fx, ccys);

	AQLString sdeName_d = mpStaticData->getStaticData(ccys[0].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	AQLString sdeName_f = mpStaticData->getStaticData(ccys[1].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	
	vector<AQLFunctionBase*> drift(1,  new AQLPriceDriftFX(sdeName_d, sdeName_f));
	sde.setDrift(drift);
}



/*!
	@brief set integral function

	@param[in] fx
	@param[out] sde

*/
void
AQLCalibrateModelPtberg::setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);

	AQLDataBool tmp;
	tmp.convertFromString(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
	bool isDDL = tmp.get();
	if (isDDL)
	{
		AQLString integralType = mpStaticData->getStaticData(key_fx + FX_KEY_PTBERG_DD_INTEGRAL_TYPE);
		integralType.toUpper();
		if (integralType == "MELSTEIN")
		{
			sde.setIntegralFunction(new AQLPriceFXDDIntegralMelstein());
		}
		else if (integralType == "EXPLICIT")
		{
			sde.setIntegralFunction(new AQLPriceFXDDIntegral());
		}
		else
		{
			AQLString msg = AQLString("Integraltype is not support integraltype = ") +integralType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
	else
	{
		AQLString integralType = mpStaticData->getStaticData(key_fx + FX_KEY_PTBERG_INTEGRAL_TYPE);
		integralType.toUpper();
		if (integralType == "LOG_INTEGRAL")
		{
			sde.setIntegralFunction(new AQLRatesEulerMaruyama(LOG_INTEGRAL));
		}
		else if (integralType == "NORMAL_INTEGRAL")
		{
			sde.setIntegralFunction(new AQLRatesEulerMaruyama(NORMAL_INTEGRAL));
		}
		else if (integralType == "LOG_INTEGRAL_LOG_OUTPUT")
		{
			sde.setIntegralFunction(new AQLRatesEulerMaruyama(LOG_INTEGRAL_LOG_OUTPUT));
		}
		else
		{
			AQLString msg = AQLString("Integraltype is not support integraltype = ") +integralType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
}

/*!
	@brief get function master regist name

	@param[in] fx
*/
AQLString 
AQLCalibrateModelPtberg::getFunctionMasterResistName(const AQLString &fx) const
{
	AQLString tmpFX = fx;
	return mpStaticData->getStaticData(tmpFX.toLower() + FX_KEY_PTBERG_FUNCTION_NAME);
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
AQLString
AQLCalibrateModelPtberg::getVolType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpStaticData->getStaticData(key_fx + FX_KEY_PTBERG_VOLATILITY_TYPE);
}

// 
/*!
    @brief set volatility function

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
AQLCalibrateModelPtberg::setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	setUpVolEntity(fx,vol);
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(fx, ccys);

	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLStringVector fileVec(1);
	fileVec[0] = AQLMarketData::getNumFileName(mpStaticData->getStaticData(key_fx + FX_KEY_PTBERG_VOLATILITY_CALIB_FILE));
	
	AQLScenarioParam param;
	param.calcType = KEY_PV;
	// set fx
	param.ccy = fx;
	// set LJ
	param.isLJ = isLJ(fx);
	// set DDL
	AQLDataBool tmp;
	tmp.convertFromString(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
	param.isDDL = tmp.get();
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	param.isCalib = isCalibTarget(fx);
	if (param.isCalib)
	{
		// create calib info
		AQLCalibrationParametersPtberg *pCInfo = createCalibInfoCreator();
		AQLString cInfoName = pCInfo->createCalibrationInfo(dataInstance.getObjectPool(), fx);
		delete pCInfo;
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
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccys[1]));
	}
	
	// create function
	AQLCalibrateVolatilityPtberg volCreator;
	AQLFunctionBase *method = volCreator.createVolatility(fileVec, &param, &objPool);
	
	if (isLJ(fx))
	{
		AQLPriceFXVolatility *fxVolatility = new AQLPriceFXVolatility(method, new AQLConstant(1.0), dynamic_cast<AQLMathVolFuncFX *>(method)->getTimeGrid());
		vol.setVolatility(fxVolatility);
	}
	else
	{
		vol.setVolatility(method);
	}

	// regist function master
	//const AQLString &volName = vol.getName().get();
	//if (isLJ(fx))
	//{
	//	AQLPriceFXVolatility *fxVolatility = new AQLPriceFXVolatility(method, new AQLConstant(1.0), dynamic_cast<AQLMathVolFuncFX *>(method)->getTimeGrid());
	//	dataInstance.getFunctionMaster().setFunction(fxVolatility->clone(), volName + AQLString("_0_0"));
	//	vol.setVolatility(fxVolatility);
	//}
	//else
	//{
	//	dataInstance.getFunctionMaster().setFunction(method->clone(), volName + AQLString("_0_0"));
	//	vol.setVolatility(method);
	//}


	//AQLPtbergUtils::setUpVolFunc(AQLMarketData::getNumFileName(filePath), dataInstance, vol, fx);
}


// 
/*!
    @brief setup volatility data

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
AQLCalibrateModelPtberg::setUpVolData(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
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
AQLCalibrateModelPtberg::setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const
{
	// set interpolation
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString interp = mpStaticData->getStaticData(key_fx.toLower() + FX_KEY_PTBERG_VOLATILITY_INTERPOLATION);
	vol.getInterpolation().convertFromString(interp);
}

// 
/*!
    @brief  create calibinfocreator

*/
AQLCalibrationParametersPtberg*
AQLCalibrateModelPtberg::createCalibInfoCreator(void) const
{
	return new AQLCalibrationParametersPtberg;
}
