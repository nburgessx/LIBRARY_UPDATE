/*! @file
    @brief Ptberg sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelPtberg.cpp
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


#include "LACalibrateModelPtberg.h"
#include "AQLFunctionBase.h"
#include "AQLFunctionManager.h"
#include "LAMathVolFuncBase.h"
#include "AQLPriceDataInterpolation.h"
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
#include "LACalibrateVolatilityPtberg.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "AQLConstant.h"
#include "LAPriceFXVolatility.h"
#include "LAPriceFXDDIntegral.h"
#include "LAPriceFXDDIntegralMelstein.h"
#include "LACalibrationParametersPtberg.h"

using namespace std;

// constructor
/*!

*/
LACalibrateModelPtberg::LACalibrateModelPtberg()
: LACalibrateModelFX()
{
}

// destructor
/*!

*/
LACalibrateModelPtberg::~LACalibrateModelPtberg(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
LARatesSDEBase *
LACalibrateModelPtberg::createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	dataInstance;
	SDE_TYPE type = getSDEType(fx);
	// check LJ
	if (isLJ(fx))
	{
		return new LARatesLJSpotSDE(type);
	}
	else
	{
		return new LARatesSpotSDE(type);
	}
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
LACalibrateModelPtberg::getSDEType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
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
LACalibrateModelPtberg::isLJ(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
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
LACalibrateModelPtberg::setVolatility(const AQLString &fx, LARatesSDEBase &sde) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString sdeName = mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_NAME);

	vector<vector<AQLFunctionBase *> > volMtx(1);
	volMtx[0].push_back(new LAMathVolFuncBase(sdeName, 0, 0, true));
	sde.setVolatility(volMtx);
}

/*!
	@brief set drift class to sde

	note: fx's first cccy (ex.ccy1/cccy2 ccy2) must be domestic ccy

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelPtberg::setDrift(const AQLString &fx, LARatesSDEBase &sde) const
{
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	AQLString sdeName_d = mpStaticData->getStaticData(ccys[0].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	AQLString sdeName_f = mpStaticData->getStaticData(ccys[1].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	
	vector<AQLFunctionBase*> drift(1,  new LAPriceDriftFX(sdeName_d, sdeName_f));
	sde.setDrift(drift);
}



/*!
	@brief set integral function

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelPtberg::setIntegralFunction(const AQLString &fx, LARatesSDEBase &sde) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);

	AQLDataBool tmp;
	tmp.convertFromString(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
	bool isDDL = tmp.get();
	if (isDDL)
	{
		AQLString integralType = mpStaticData->getStaticData(key_fx + FX_KEY_PTBERG_DD_INTEGRAL_TYPE);
		integralType.toUpper();
		if (integralType == "MELSTEIN")
		{
			sde.setIntegralFunction(new LAPriceFXDDIntegralMelstein());
		}
		else if (integralType == "EXPLICIT")
		{
			sde.setIntegralFunction(new LAPriceFXDDIntegral());
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
			sde.setIntegralFunction(new LARatesEulerMaruyama(LOG_INTEGRAL));
		}
		else if (integralType == "NORMAL_INTEGRAL")
		{
			sde.setIntegralFunction(new LARatesEulerMaruyama(NORMAL_INTEGRAL));
		}
		else if (integralType == "LOG_INTEGRAL_LOG_OUTPUT")
		{
			sde.setIntegralFunction(new LARatesEulerMaruyama(LOG_INTEGRAL_LOG_OUTPUT));
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
LACalibrateModelPtberg::getFunctionMasterResistName(const AQLString &fx) const
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
LACalibrateModelPtberg::getVolType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
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
LACalibrateModelPtberg::setUpVolFunc(const AQLString &fx, LAMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	setUpVolEntity(fx,vol);
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLStringVector fileVec(1);
	fileVec[0] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_fx + FX_KEY_PTBERG_VOLATILITY_CALIB_FILE));
	
	MAScenarioParam param;
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
		LACalibrationParametersPtberg *pCInfo = createCalibInfoCreator();
		AQLString cInfoName = pCInfo->createCalibrationInfo(dataInstance.getObjectPool(), fx);
		delete pCInfo;
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
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
	}
	
	// create function
	LACalibrateVolatilityPtberg volCreator;
	AQLFunctionBase *method = volCreator.createVolatility(fileVec, &param, &objPool);
	
	if (isLJ(fx))
	{
		LAPriceFXVolatility *fxVolatility = new LAPriceFXVolatility(method, new AQLConstant(1.0), dynamic_cast<LAMathVolFuncFX *>(method)->getTimeGrid());
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
	//	LAPriceFXVolatility *fxVolatility = new LAPriceFXVolatility(method, new AQLConstant(1.0), dynamic_cast<LAMathVolFuncFX *>(method)->getTimeGrid());
	//	dataInstance.getFunctionMaster().setFunction(fxVolatility->clone(), volName + AQLString("_0_0"));
	//	vol.setVolatility(fxVolatility);
	//}
	//else
	//{
	//	dataInstance.getFunctionMaster().setFunction(method->clone(), volName + AQLString("_0_0"));
	//	vol.setVolatility(method);
	//}


	//MAPtbergUtils::setUpVolFunc(LAMarketData::getNumFileName(filePath), dataInstance, vol, fx);
}


// 
/*!
    @brief setup volatility data

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelPtberg::setUpVolData(const AQLString &fx, LAMathVolatility &vol, AQLDataInstance &dataInstance) const
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
LACalibrateModelPtberg::setUpVolEntity(const AQLString &fx, LAMathVolatility &vol) const
{
	// set interpolation
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString interp = mpStaticData->getStaticData(key_fx.toLower() + FX_KEY_PTBERG_VOLATILITY_INTERPOLATION);
	vol.getInterpolation().convertFromString(interp);
}

// 
/*!
    @brief  create calibinfocreator

*/
LACalibrationParametersPtberg*
LACalibrateModelPtberg::createCalibInfoCreator(void) const
{
	return new LACalibrationParametersPtberg;
}
