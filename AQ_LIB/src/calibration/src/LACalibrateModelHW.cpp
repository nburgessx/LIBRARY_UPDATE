#define _HAS_STD_BYTE 0

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <math.h>
#include "LACalibrateModelHW.h"
#include "AQLFunctionBase.h"
#include "AQLDataVector.h"
#include "AQLDataProcedure.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataReference.h"
#include "AQLAlgorithm.h"
#include "LAMathVolFuncBase.h"
#include "LAMathCorrelation.h"
#include "LAMathYieldCurve.h"
#include "LAPriceYieldGenerator.h"
#include "LAMathVolatility.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"
#include "LARatesNumeraireBankAccount.h"
#include "LARatesNumeraireDiscountBond.h"
#include "LAModelDynamicsHW1FCurve.h"
#include "LAPriceDriftQuantAdjustment.h"
#include "LARatesEulerMaruyama.h"
#include "LACalibrateModelIR.h"
#include "LAMathVolFuncWave.h"
#include "LAMathVolFuncStructureBase.h"
#include "LAMarketData.h"
#include "LAMarketDataHW.h"
#include "LARatesCurveLogLinearInterpolation.h"
#include "LADefinitionsHW.h"
#include "LARatesSpotSDE.h"
#include "LARatesLJSpotSDE.h"
#include "LARatesNumeraireBankAccountHW.h"
#include "LARatesHWIntegral.h"
#include "LAMathVolFuncHW.h"
#include "LAPriceDriftHW.h"
#include "LAPriceDriftHWQuantAdjustment.h"
#include "LAStaticData.h"
#include "LADealUtils.h"
#include "LACalibrateVolatilityHW.h"
#include "LARatesCurveHWInterpolation.h"
#include "LAScenarioConfiguration.h"
#include "LACalibrationParametersHW.h"

using namespace std;

// constructor
/*!
	@param[in] baseCurrency

*/
LACalibrateModelHW::LACalibrateModelHW(const AQLString &baseCurrency)
: LACalibrateModelIR(baseCurrency)
{
}

// destructor
/*!

*/
LACalibrateModelHW::~LACalibrateModelHW(void)
{
}


/*!
	@brief return lmm sde type

	@param[in] currency

*/
SDE_TYPE
LACalibrateModelHW::getSDEType(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	AQLString type = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_HW_TYPE);
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
LACalibrateModelHW::isLJ(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	
	AQLString type = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_HW_INTEGRAL_STEP);
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

// 
/*!
    @brief create sde instance

	@param[in]  currency
	@param[in]  dataInstance
*/
LARatesSDEBase *
LACalibrateModelHW::createSDEInstance(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	(void)dataInstance;
	SDE_TYPE type = getSDEType(currency);
	LARatesSpotSDE *psde = 0;
	// check LJ
	if (isLJ(currency))
	{
		psde =  new LARatesLJSpotSDE(type);
	}
	else
	{
		psde =  new LARatesSpotSDE(type);
	}

	return psde;
}


/*!
	@brief set volatility to sde

	@param[in] currency
	@param[out] sde
*/
void
LACalibrateModelHW::setVolatility(const AQLString &currency, LARatesSDEBase &sde) const
{
	AQLString key_ccy = currency;
	AQLString sdeName = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);

	vector<vector<AQLFunctionBase *> > volMtx(1);
	volMtx[0] = vector<AQLFunctionBase *>(1, new LAMathVolFuncBase(sdeName, 0, 0, true));
	sde.setVolatility(volMtx);
}

/*!
	@brief set drift class to sde

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelHW::setDrift(const AQLString &currency, LARatesSDEBase &sde) const
{
	AQLString key_ccy = currency;
	AQLString sdeName = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);

	vector<AQLFunctionBase *> driftVec(1);
	AQLString tmp_baseccy = mBaseCurrency;
	tmp_baseccy.toLower();

	if (MADealUtils::getSDECurrencys().size() != 1)
	{
		// for cross cccy
		if (key_ccy != tmp_baseccy && !isZeroVol(key_ccy) && !isSZModel(currency))
		{
			// avoiding the error in getFXKey
			AQLString bccy = MADealUtils::getSDECurrencys()[0];
			if (bccy.toLower() != tmp_baseccy && bccy != key_ccy)
				tmp_baseccy = bccy;
			
			// get domestic(base) ir sde name
			AQLString sdeBase = mpStaticData->getStaticData(tmp_baseccy + STATIC_DATA_FX_KEY_SDE_NAME);

			// forein drift
			// get fx sde name
			AQLString key_fx = LAMarketData::getFXKey(tmp_baseccy, key_ccy);
			AQLString fx_sdeName = mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_NAME);
			driftVec[0] = createForeinDrift(key_fx, sdeBase, sdeName, fx_sdeName);
		}
		else
		{
			// domestic drift
			driftVec[0] = new LAPriceDriftHW(sdeName);
		}
	}
	else
	{
		// check only one ccy
		if (key_ccy != tmp_baseccy)
		{
			throw AQLCoreInvalidData("Sigle currency support only one currency", __FILE__, __LINE__);
		}

		driftVec[0] = new LAPriceDriftHW(sdeName);
	}
	sde.setDrift(driftVec);
}


/*!
	@brief set numeraire

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelHW::setNumeraire(const AQLString &currency, LARatesSDEBase &sde) const
{
	(void)currency;
	sde.setNumeraire(new LARatesNumeraireBankAccountHW());
	/*if (MADealUtils::getSimulationSDECurrencys().size()==1)
    {
        sde.getNumeraire()->isCancelSpread(true);
    }*/
}

/*!
	@brief set path element

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelHW::setOutputTemplate(const AQLString &currency, LARatesSDEBase &sde) const
{
	(void)currency;
	sde.setOutputTemplate(new LARatesPathElementHW1FCurveTMDPT(0.0));
}

/*!
	@brief set integral function

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelHW::setIntegralFunction(const AQLString &currency, LARatesSDEBase &sde) const
{
	AQLString sdeName = getSDEAttrName(currency);
	sde.setIntegralFunction(new LARatesHWIntegral(LOG_INTEGRAL, sdeName));
}

/*!
	@brief get function master regist name

	@param[in] currency
*/
AQLString 
LACalibrateModelHW::getFunctionMasterResistName(const AQLString &currency) const
{
	AQLString tmpCurrency = currency;
	return mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_HW_FUNCTION_NAME);
}

// 
/*!
    @brief set setInterpolationMethod

	@param[in] currency
	@param[out] sde
*/
void
LACalibrateModelHW::setInterpolationMethod(const AQLString &currency, LARatesSDEBase &sde) const
{
	currency;
	sde.setInterpolationMethod(new LARatesCurveHWInterpolation());
}


// 
/*!
    @brief get correlation input type

	@param[in] currency 
*/
AQLString
LACalibrateModelHW::getCorTye(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	return mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_HW_CORRELATION_TYPE);
}


// 
/*!
    @brief set correlation factor loading

	@param[in] currency
	@param[out] cor
	@param[out] dataInstance
*/
void
LACalibrateModelHW::setUpCorFactor(const AQLString &currency, LAMathCorrelation &cor, AQLDataInstance &dataInstance) const
{
	(void)currency;
	(void)cor;
	(void)dataInstance;
}


// 
/*!
    @brief set correlation data

	@param[in] currency
	@param[out] cor
	@param[out] dataInstance
*/
void
LACalibrateModelHW::setUpCorData(const AQLString &currency, LAMathCorrelation &cor, AQLDataInstance &dataInstance) const
{
	(void)currency;
	(void)cor;
	(void)dataInstance;
}



// 
/*!
    @brief set correlation data

	@param[in] currency
	@param[out] cor
	@param[out] dataInstance
*/
void
LACalibrateModelHW::setUpCorFunc(const AQLString &currency, LAMathCorrelation &cor, AQLDataInstance &dataInstance) const
{
	(void)currency;
	(void)cor;
	(void)dataInstance;
}

// 
/*!
    @brief get volatility input type

	@param[in]  currency 
*/
AQLString
LACalibrateModelHW::getVolType(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	return mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_HW_VOLATILITY_TYPE);
}

// 
/*!
    @brief set volatility function

	@param[in] currency
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelHW::setUpVolFunc(const AQLString &currency, LAMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	AQLString key_ccy = currency;
	key_ccy.toLower();
	AQLStringVector fileVec(1);
	// filePath
	fileVec[0] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_HW_PARAMETER_FILE));
	LACalibrateVolatilityHW volCreator;
	// create method vec
	vector<AQLFunctionBase *> funcVec;

	MAScenarioParam param;
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	param.isCalib = isCalibTarget(currency);
	param.calcType = KEY_PV;
	param.isZeroVol = isZeroVol(currency);
	if (param.isCalib)
	{
		// create calib info
		LACalibrationParametersHW cInfo;
		AQLString cInfoName = cInfo.createCalibrationInfo(objPool, currency);
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
	}
	param.refName.push_back(LAMarketData::getBaseYieldName(currency));	
	volCreator.createVolatility(funcVec, fileVec, &param, &objPool);

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
LACalibrateModelHW::setUpVolData(const AQLString &currency, LAMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	(void)currency;
	(void)vol;
	(void)dataInstance;

	AQLString msg = AQLString("Volatility matrix is not support.");
	throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
}


// 
/*!
    @brief create createForeinDrift

	@param[in] fx
	@param[in] sdeBase
	@param[in] sdeName
	@param[in] fx_sdeName

	@return drift pointer
*/
AQLFunctionBase *
LACalibrateModelHW::createForeinDrift(const AQLString &fx, const AQLString &sdeBase, const AQLString &sdeName, const AQLString &fx_sdeName) const
{
	return new LAPriceDriftHWQuantAdjustment(sdeBase, sdeName, fx_sdeName, new LAPriceDriftHW(sdeName));
}


// 
/*!
    @brief create isZeroVol

	@param[in] currency

	@return isZeroVol
*/
bool
LACalibrateModelHW::isZeroVol(const AQLString &currency) const
{
	AQLString key_ccy(currency);
	AQLDataBool* pIsZeroVol = new AQLDataBool(false);
	AQLString isZeroVolStr = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_HW_ISZEROVOL);
	if (isZeroVolStr != AQ_NO_DATA)
		pIsZeroVol->convertFromString(isZeroVolStr);
	bool ret = pIsZeroVol->get();
	delete pIsZeroVol;
	return ret;
}

// 
/*!
@brief check isSZModel 

@param[in] currency

@return isSZModel 
*/
bool 
LACalibrateModelHW::isSZModel(const AQLString &currency) const
{
	AQLString tmp_baseccy = mBaseCurrency;
	tmp_baseccy.toLower();
	AQLString key_ccy = currency;
	key_ccy.toLower();
	AQLString key_fx = LAMarketData::getFXKey(tmp_baseccy, key_ccy);
	const AQLString model_fx = mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_CROSS_MODEL).toUpper();
	if (model_fx == MODEL_SZ)
	{
		return true;
	}
	else
	{
		return false;
	}
}