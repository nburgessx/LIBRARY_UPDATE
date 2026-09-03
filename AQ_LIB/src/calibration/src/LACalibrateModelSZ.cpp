#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LACalibrateModelSZ.h"
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
#include "LADefinitionsSZ.h"
#include "LAMarketData.h"
#include "LAMathVolFuncFX.h"
#include "LAMathFXAdjuster.h"
#include "LAMathVolatility.h"
#include "LAScenarioConfiguration.h"
#include "LACalibrateVolatilitySZ.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "LAConstant.h"
#include "LAPriceFXVolatility.h"
#include "LAPriceSZDDIntegralMelstein.h"
#include "LAPriceDriftSZDDVolFactor.h"
#include "LARatesScalarLinearInterpolation.h"
#include "LAMathVolFuncSZDD.h"
#include "LAMathVolFuncSZDDVolFactor.h"
#include "LAStepInterpolation.h"
#include "LARatesHWIntegral.h"
#include "LACalibrationParametersSZ.h"
#include "LACalibrationFunc.h"

using namespace std;

// constructor
/*!

*/
LACalibrateModelSZ::LACalibrateModelSZ()
: LACalibrateModelFX()
{
}

// destructor
/*!

*/
LACalibrateModelSZ::~LACalibrateModelSZ(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
LARatesSDEBase *
LACalibrateModelSZ::createSDEInstance(const LAString &fx, LADataInstance &dataInstance) const
{
	dataInstance;
	SDE_TYPE type = getSDEType(fx);
		return new LARatesSpotSDE(type);
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
LACalibrateModelSZ::getSDEType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString type = mpStaticData->getStaticData(key_fx + FX_KEY_SZ_TYPE);
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
		LAString msg = LAString("Sde type is not support. type = ") + type;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

/*!
	@brief check is Long Jump

	@param[in] fx

*/
bool 
LACalibrateModelSZ::isLJ(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString type = mpStaticData->getStaticData(key_fx + FX_KEY_SZ_INTEGRAL_STEP);
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
LACalibrateModelSZ::setVolatility(const LAString &fx, LARatesSDEBase &sde) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString sdeName = mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_NAME);

	vector<vector<LAFunctionBase *> > volMtx(1);
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
LACalibrateModelSZ::setDrift(const LAString &fx, LARatesSDEBase &sde) const
{
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	LAString sdeName_d = mpStaticData->getStaticData(ccys[0].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	LAString sdeName_f = mpStaticData->getStaticData(ccys[1].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	
	vector<LAFunctionBase*> drift(1,  new LAPriceDriftFX(sdeName_d, sdeName_f));
	sde.setDrift(drift);
}



/*!
	@brief set integral function

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelSZ::setIntegralFunction(const LAString &fx, LARatesSDEBase &sde) const
{
	LAString sdeName = getSDEAttrName(fx);

	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);

	LAString volsdeName = mpStaticData->getStaticData(key_fx + ".volatility" + STATIC_DATA_FX_KEY_SDE_NAME);
	LAString integralType = mpStaticData->getStaticData(key_fx + FX_KEY_SZ_INTEGRAL_TYPE);
	integralType.toUpper();
	if (integralType == "MELSTEIN")
	{
		sde.setIntegralFunction(new LAPriceSZDDIntegralMelstein(sdeName, volsdeName));
	}
	else
	{
		LAString msg = LAString("Integraltype is not support integraltype = ") +integralType;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

/*!
	@brief get function master regist name

	@param[in] fx
*/
LAString 
LACalibrateModelSZ::getFunctionMasterResistName(const LAString &fx) const
{
	LAString tmpFX = fx;
	return mpStaticData->getStaticData(tmpFX.toLower() + FX_KEY_SZ_FUNCTION_NAME);
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
LAString
LACalibrateModelSZ::getVolType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpStaticData->getStaticData(key_fx + FX_KEY_SZ_VOLATILITY_TYPE);
}

// 
/*!
    @brief set volatility function

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelSZ::setUpVolFunc(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const
{
	setUpVolEntity(fx,vol);
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAStringVector fileVec(2);
	fileVec[0] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_fx + FX_KEY_SZ_VOLATILITY_CALIB_FILE));
	fileVec[1] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_fx + ".volatility" + STATIC_DATA_FX_KEY_SDE_INITIALVALUE_FILE));
	
	MAScenarioParam param;
	param.calcType = KEY_PV;
	// set fx
	param.ccy = fx;
	LAObjectPool &objPool = dataInstance.getObjectPool();

	param.isCalib = isCalibTarget(fx);
	if (param.isCalib)
	{
		// create calib info
		LACalibrationParametersSZ *pCInfo = createCalibInfoCreator();
		LAString cInfoName = pCInfo->createCalibrationInfo(dataInstance.getObjectPool(), fx);
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
	LACalibrateVolatilitySZ volCreator;
	LAFunctionBase *method = volCreator.createVolatility(fileVec, &param, &objPool);
	
	if (isLJ(fx))
	{
		LAPriceFXVolatility *fxVolatility = new LAPriceFXVolatility(method, new LAConstant(1.0), dynamic_cast<LAMathVolFuncFX *>(method)->getTimeGrid());
		vol.setVolatility(fxVolatility);
	}
	else
	{
		vol.setVolatility(method);
	}

	// set initialvalue of stochastic factor in spot volatility.
	// In the case of LAMathVolFuncSZDD, the parameter "Sigma" denotes the initial value.
	double initvalue;
	if (param.isCalib)
	{
		MACalibrationFunc *func_calb = dynamic_cast<MACalibrationFunc*>(method);
		LAMathVolFuncFX *func_fx = dynamic_cast<LAMathVolFuncFX*>(func_calb->clone());
		initvalue = func_fx->getSigma()[0];
		delete func_fx;
	}
	else
	{
		initvalue = dynamic_cast<LAMathVolFuncSZDD*>(method)->getSigma()[0];
	}
	vol.setInitialValue(initvalue);
}


// 
/*!
    @brief setup volatility data

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelSZ::setUpVolData(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const
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
LACalibrateModelSZ::setUpVolEntity(const LAString &fx, LAMathVolatility &vol) const
{
	// set interpolation
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString interp = mpStaticData->getStaticData(key_fx.toLower() + FX_KEY_SZ_VOLATILITY_INTERPOLATION);
	vol.getInterpolation().convertFromString(interp);
}


// constructor
/*!
	@param[in] baseCurrency

*/
LACalibrateModelSZ::VF::VF()
: LACalibrateModelVolFactor()
{
}

// destructor
/*!

*/
LACalibrateModelSZ::VF::~VF(void)
{
}


//----------------------------------------------------------------------------
/*!
	@brief return lmm sde type

	@param[in] currency

*/
SDE_TYPE
LACalibrateModelSZ::VF::getSDEType(const LAString &currency) const
{
	LAString key_ccy(getSpotIndex(currency));
	LAString type = mpStaticData->getStaticData(key_ccy.toLower() + ".volatility" + FX_KEY_SZ_TYPE);
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
		LAString msg = LAString("sde type is not support. type = ") + type;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

/*!
	@brief check is Long Jump

	@param[in] currency

*/
bool 
LACalibrateModelSZ::VF::isLJ(const LAString &currency) const
{
	LAString key_ccy(getSpotIndex(currency));
	
	LAString type = mpStaticData->getStaticData(key_ccy.toLower() + ".volatility" + FX_KEY_SZ_INTEGRAL_STEP);
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

	@param[in] currency
	@param[out] sde
*/
void
LACalibrateModelSZ::VF::setVolatility(const LAString &currency, LARatesSDEBase &sde) const
{
	LAString sdeName = getSDEAttrName(currency);

	vector<vector<LAFunctionBase *> > volMtx(1);
	volMtx[0] = vector<LAFunctionBase *>(1, new LAMathVolFuncBase(sdeName, 0, 0, true));
	sde.setVolatility(volMtx);
}

/*!
	@brief set drift class to sde

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelSZ::VF::setDrift(const LAString &currency, LARatesSDEBase &sde) const
{
	LAString sdeName = getSDEAttrName(currency);

	vector<LAFunctionBase *> driftVec(1);
	driftVec[0] = new LAPriceDriftSZDDVolFactor(sdeName);
	sde.setDrift(driftVec);
}

/*!
	@brief set integral function

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelSZ::VF::setIntegralFunction(const LAString &currency, LARatesSDEBase &sde) const
{
	LAString sdeName = getSDEAttrName(currency);
	sde.setIntegralFunction(new LARatesHWIntegral(LOG_INTEGRAL, sdeName));
}

/*!
	@brief get function master regist name

	@param[in] currency
*/
LAString 
LACalibrateModelSZ::VF::getFunctionMasterResistName(const LAString &currency) const
{
	LAString tmpCurrency(getSpotIndex(currency));
	return mpStaticData->getStaticData(tmpCurrency.toLower() + ".volatility" + FX_KEY_SZ_FUNCTION_NAME);
}

// 
/*!
    @brief get volatility input type

	@param[in]  currency 
*/
LAString
LACalibrateModelSZ::VF::getVolType(const LAString &currency) const
{
	LAString key_ccy(getSpotIndex(currency));
	return mpStaticData->getStaticData(key_ccy.toLower() + ".volatility" + FX_KEY_SZ_VOLATILITY_TYPE);
}

// 
/*!
    @brief set volatility function

	@param[in] currency
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelSZ::VF::setUpVolFunc(const LAString &currency, LAMathVolatility &vol, LADataInstance &dataInstance) const
{
	// get spot volatility function
	LAString fxkey(getSpotIndex(currency));

	LACalibrateModelSZ* pSpotGenerator = new LACalibrateModelSZ();
	LAString sdeSpotName = pSpotGenerator->getSDEAttrName(fxkey);
	delete pSpotGenerator;
	LAString volSpotName = PREFIX_VOL + sdeSpotName;
	const LAMathVolatility *volSpotEntity = &dynamic_cast<const LAMathVolatility &>(dataInstance.getObjectPool().getObject(volSpotName, ENCHKTYPE_ISDEFINED).get());
	const LAMathVolFuncSZDD *volFunc;
	if (isCalibTarget(fxkey))
	{
		const MACalibrationFunc *volFunc_fx = dynamic_cast<const MACalibrationFunc*>(volSpotEntity->getVolatilityFunc());
		volFunc = dynamic_cast<const LAMathVolFuncSZDD*>(volFunc_fx->clone());
	}
	else
	{
		volFunc = dynamic_cast<const LAMathVolFuncSZDD*>(volSpotEntity->getVolatilityFunc());
	}
	if (volSpotEntity == 0)
	{
		LAString msg = LAString("Cast error on spot volatility.");
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	DoubleVector volterm = volFunc->getTimeGrid();
	DoubleVector theta = volFunc->getTheta().getParam();
	DoubleVector kappa = volFunc->getKappa().getParam();
	DoubleVector epsilon = volFunc->getEpsilon().getParam();

	if(kappa.empty())
		throw LACoreInvalidData("Error Mean Reversion is empty",__FILE__,__LINE__);
	
	LAMathHWFuncMRTMDPT* pafunc = new LAMathHWFuncMRTMDPT(volterm,kappa,*(new LAStepInterpolation()));
	
	LAMathHWFuncSigmaTMDPT* psfunc = new LAMathHWFuncSigmaTMDPT(volterm,epsilon,*(new LAStepInterpolation()));

	LA1DDataSet* pthetafunc = new LA1DDataSet();
	LAStepInterpolation inter;
	pthetafunc->setInterpolation(inter);
	pthetafunc->set(volterm,theta);

	vector<LAFunctionBase *> funcVec(1);
	funcVec[0] = new LAMathVolFuncSZDDVolFactor(*pafunc,*psfunc,*pthetafunc);	
	vol.setVolatility(funcVec);

}

// 
/*!
    @brief set volatility data

	@param[in] currency
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelSZ::VF::setUpVolData(const LAString &currency, LAMathVolatility &vol, LADataInstance &dataInstance) const
{
	(void)currency;
	(void)vol;
	(void)dataInstance;

	LAString msg = LAString("Volatility matrix is not support.");
	throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
}
// 
/*!
@brief  create calibinfocreator

*/
LACalibrationParametersSZ*
LACalibrateModelSZ::createCalibInfoCreator(void) const
{
	return new LACalibrationParametersSZ;
}