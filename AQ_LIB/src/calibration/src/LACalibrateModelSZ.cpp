#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LACalibrateModelSZ.h"
#include "AQLFunctionBase.h"
#include "AQLFunctionManager.h"
#include "AQLMathVolFuncBase.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDriftLMMSpot.h"
#include "AQLPriceDriftFX.h"
#include "AQLRatesSpotSDE.h"
#include "AQLRatesLJSpotSDE.h"
#include "AQLRatesEulerMaruyama.h"
#include "LACalibrateModelFX.h"
#include "LADefinitionsSZ.h"
#include "LAMarketData.h"
#include "AQLMathVolFuncFX.h"
#include "AQLMathFXAdjuster.h"
#include "AQLMathVolatility.h"
#include "LAScenarioConfiguration.h"
#include "LACalibrateVolatilitySZ.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "AQLConstant.h"
#include "AQLPriceFXVolatility.h"
#include "AQLPriceSZDDIntegralMelstein.h"
#include "AQLPriceDriftSZDDVolFactor.h"
#include "AQLRatesScalarLinearInterpolation.h"
#include "AQLMathVolFuncSZDD.h"
#include "AQLMathVolFuncSZDDVolFactor.h"
#include "AQLStepInterpolation.h"
#include "AQLRatesHWIntegral.h"
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
AQLRatesSDEBase *
LACalibrateModelSZ::createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	dataInstance;
	SDE_TYPE type = getSDEType(fx);
		return new AQLRatesSpotSDE(type);
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
LACalibrateModelSZ::getSDEType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString type = mpStaticData->getStaticData(key_fx + FX_KEY_SZ_TYPE);
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
LACalibrateModelSZ::isLJ(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString type = mpStaticData->getStaticData(key_fx + FX_KEY_SZ_INTEGRAL_STEP);
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
LACalibrateModelSZ::setVolatility(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
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
LACalibrateModelSZ::setDrift(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

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
LACalibrateModelSZ::setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	AQLString sdeName = getSDEAttrName(fx);

	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);

	AQLString volsdeName = mpStaticData->getStaticData(key_fx + ".volatility" + STATIC_DATA_FX_KEY_SDE_NAME);
	AQLString integralType = mpStaticData->getStaticData(key_fx + FX_KEY_SZ_INTEGRAL_TYPE);
	integralType.toUpper();
	if (integralType == "MELSTEIN")
	{
		sde.setIntegralFunction(new AQLPriceSZDDIntegralMelstein(sdeName, volsdeName));
	}
	else
	{
		AQLString msg = AQLString("Integraltype is not support integraltype = ") +integralType;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

/*!
	@brief get function master regist name

	@param[in] fx
*/
AQLString 
LACalibrateModelSZ::getFunctionMasterResistName(const AQLString &fx) const
{
	AQLString tmpFX = fx;
	return mpStaticData->getStaticData(tmpFX.toLower() + FX_KEY_SZ_FUNCTION_NAME);
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
AQLString
LACalibrateModelSZ::getVolType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
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
LACalibrateModelSZ::setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	setUpVolEntity(fx,vol);
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLStringVector fileVec(2);
	fileVec[0] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_fx + FX_KEY_SZ_VOLATILITY_CALIB_FILE));
	fileVec[1] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_fx + ".volatility" + STATIC_DATA_FX_KEY_SDE_INITIALVALUE_FILE));
	
	MAScenarioParam param;
	param.calcType = KEY_PV;
	// set fx
	param.ccy = fx;
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	param.isCalib = isCalibTarget(fx);
	if (param.isCalib)
	{
		// create calib info
		LACalibrationParametersSZ *pCInfo = createCalibInfoCreator();
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
	LACalibrateVolatilitySZ volCreator;
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

	// set initialvalue of stochastic factor in spot volatility.
	// In the case of AQLMathVolFuncSZDD, the parameter "Sigma" denotes the initial value.
	double initvalue;
	if (param.isCalib)
	{
		MACalibrationFunc *func_calb = dynamic_cast<MACalibrationFunc*>(method);
		AQLMathVolFuncFX *func_fx = dynamic_cast<AQLMathVolFuncFX*>(func_calb->clone());
		initvalue = func_fx->getSigma()[0];
		delete func_fx;
	}
	else
	{
		initvalue = dynamic_cast<AQLMathVolFuncSZDD*>(method)->getSigma()[0];
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
LACalibrateModelSZ::setUpVolData(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
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
LACalibrateModelSZ::setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const
{
	// set interpolation
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString interp = mpStaticData->getStaticData(key_fx.toLower() + FX_KEY_SZ_VOLATILITY_INTERPOLATION);
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
LACalibrateModelSZ::VF::getSDEType(const AQLString &currency) const
{
	AQLString key_ccy(getSpotIndex(currency));
	AQLString type = mpStaticData->getStaticData(key_ccy.toLower() + ".volatility" + FX_KEY_SZ_TYPE);
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
		AQLString msg = AQLString("sde type is not support. type = ") + type;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

/*!
	@brief check is Long Jump

	@param[in] currency

*/
bool 
LACalibrateModelSZ::VF::isLJ(const AQLString &currency) const
{
	AQLString key_ccy(getSpotIndex(currency));
	
	AQLString type = mpStaticData->getStaticData(key_ccy.toLower() + ".volatility" + FX_KEY_SZ_INTEGRAL_STEP);
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
LACalibrateModelSZ::VF::setVolatility(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	AQLString sdeName = getSDEAttrName(currency);

	vector<vector<AQLFunctionBase *> > volMtx(1);
	volMtx[0] = vector<AQLFunctionBase *>(1, new AQLMathVolFuncBase(sdeName, 0, 0, true));
	sde.setVolatility(volMtx);
}

/*!
	@brief set drift class to sde

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelSZ::VF::setDrift(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	AQLString sdeName = getSDEAttrName(currency);

	vector<AQLFunctionBase *> driftVec(1);
	driftVec[0] = new AQLPriceDriftSZDDVolFactor(sdeName);
	sde.setDrift(driftVec);
}

/*!
	@brief set integral function

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelSZ::VF::setIntegralFunction(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	AQLString sdeName = getSDEAttrName(currency);
	sde.setIntegralFunction(new AQLRatesHWIntegral(LOG_INTEGRAL, sdeName));
}

/*!
	@brief get function master regist name

	@param[in] currency
*/
AQLString 
LACalibrateModelSZ::VF::getFunctionMasterResistName(const AQLString &currency) const
{
	AQLString tmpCurrency(getSpotIndex(currency));
	return mpStaticData->getStaticData(tmpCurrency.toLower() + ".volatility" + FX_KEY_SZ_FUNCTION_NAME);
}

// 
/*!
    @brief get volatility input type

	@param[in]  currency 
*/
AQLString
LACalibrateModelSZ::VF::getVolType(const AQLString &currency) const
{
	AQLString key_ccy(getSpotIndex(currency));
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
LACalibrateModelSZ::VF::setUpVolFunc(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	// get spot volatility function
	AQLString fxkey(getSpotIndex(currency));

	LACalibrateModelSZ* pSpotGenerator = new LACalibrateModelSZ();
	AQLString sdeSpotName = pSpotGenerator->getSDEAttrName(fxkey);
	delete pSpotGenerator;
	AQLString volSpotName = PREFIX_VOL + sdeSpotName;
	const AQLMathVolatility *volSpotEntity = &dynamic_cast<const AQLMathVolatility &>(dataInstance.getObjectPool().getObject(volSpotName, ENCHKTYPE_ISDEFINED).get());
	const AQLMathVolFuncSZDD *volFunc;
	if (isCalibTarget(fxkey))
	{
		const MACalibrationFunc *volFunc_fx = dynamic_cast<const MACalibrationFunc*>(volSpotEntity->getVolatilityFunc());
		volFunc = dynamic_cast<const AQLMathVolFuncSZDD*>(volFunc_fx->clone());
	}
	else
	{
		volFunc = dynamic_cast<const AQLMathVolFuncSZDD*>(volSpotEntity->getVolatilityFunc());
	}
	if (volSpotEntity == 0)
	{
		AQLString msg = AQLString("Cast error on spot volatility.");
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	DoubleVector volterm = volFunc->getTimeGrid();
	DoubleVector theta = volFunc->getTheta().getParam();
	DoubleVector kappa = volFunc->getKappa().getParam();
	DoubleVector epsilon = volFunc->getEpsilon().getParam();

	if(kappa.empty())
		throw AQLCoreInvalidData("Error Mean Reversion is empty",__FILE__,__LINE__);
	
	AQLMathHWFuncMRTMDPT* pafunc = new AQLMathHWFuncMRTMDPT(volterm,kappa,*(new AQLStepInterpolation()));
	
	AQLMathHWFuncSigmaTMDPT* psfunc = new AQLMathHWFuncSigmaTMDPT(volterm,epsilon,*(new AQLStepInterpolation()));

	AQL1DDataSet* pthetafunc = new AQL1DDataSet();
	AQLStepInterpolation inter;
	pthetafunc->setInterpolation(inter);
	pthetafunc->set(volterm,theta);

	vector<AQLFunctionBase *> funcVec(1);
	funcVec[0] = new AQLMathVolFuncSZDDVolFactor(*pafunc,*psfunc,*pthetafunc);	
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
LACalibrateModelSZ::VF::setUpVolData(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	(void)currency;
	(void)vol;
	(void)dataInstance;

	AQLString msg = AQLString("Volatility matrix is not support.");
	throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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