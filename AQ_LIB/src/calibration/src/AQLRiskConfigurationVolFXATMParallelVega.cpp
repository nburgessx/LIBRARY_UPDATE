/*! @file
    @brief  FX atm parallel vega setup class 
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRiskConfigurationVolFXATMParallelVega.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLPricePortfolioValue.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"
#include "AQLDealUtils.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLMathJamshidianSwaption.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationVolFXATMParallelVega::AQLRiskConfigurationVolFXATMParallelVega(void)
: AQLRiskConfigurationVolParallel(true)
{
}

// destructor
/*!

*/
AQLRiskConfigurationVolFXATMParallelVega::~AQLRiskConfigurationVolFXATMParallelVega(void)
{
}


/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
AQLRiskConfigurationVolFXATMParallelVega::createVolatilityEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	if (scenarioNum == SCENARIO_1)
	{
		const AQLString model = AQLMarketData::getModelName(fx);
		const AQLString riskName = getRiskName();
		AQLStringVector ccys;
		AQLMarketData::convertToCurrency(fx, ccys);
		// scenario param
		AQLScenarioParam param;
		param.ccy = fx;
		param.calcType = ccys[0] + "_" + ccys[1] + "_" + riskName;
		param.model = model;
		param.targetName = AQLMarketData::getBaseVolatilityName(fx);
		param.inputType = AQLMarketData::getVolInputType(model, fx, riskName);
		param.isCalib = true;
		param.isParallel = true;
		param.isGrid = false;
		param.shiftType = RISK_SHIFTTYPE_DIFF;

		AQLObjectPool &objPool = dataInstance.getObjectPool();
		// set reference
		AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
		AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, fx);
		delete calibInfoCreator;
		param.refName.push_back(infoName);

		// set yield and calibdata
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, param.refName.back())));
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccys[1]));
		param.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, param.refName.back())));

		// get term
		const AQLStringVector &terms = dynamic_cast<const AQLDataStrings &>(objPool.getObject(infoName, ENCHKTYPE_ISDEFINED).getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
		// create ATM grid
		const unsigned int termSize = terms.size();
		param.paraTerm.resize(termSize);
		for (unsigned int i = 0; i < termSize; ++i)
		{
			param.paraTerm[i] = AQLString(FXVOL_ATM) + AQLString("_") + terms[i];
		}

		// dataout
		if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}

		// set parallel shift
		param.paraShiftVec.resize(termSize, getParallelShiftVal(fx));
		// set dmy file
		param.paraFile.push_back(CALIB_DMY_FILE);
		// set DDL
		AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));

		// create scenario
		AQLScenarioConfiguration *sceCreator = 
			AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
		delete sceCreator;
		return ret;
	}
	else
	{
		return vector<AQLObject *>(0); 
	}
}

/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
AQLRiskConfigurationVolFXATMParallelVega::createVolatilityEntityOld(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	if (scenarioNum == SCENARIO_1)
	{
		const AQLString model = AQLMarketData::getModelName(fx);
		const AQLString riskName = getRiskName();
		// scenario param
		AQLScenarioParam param;
		param.ccy = fx;
		param.calcType= fx + "_" + riskName;
		param.model = model;
		param.targetName = AQLMarketData::getBaseVolatilityName(fx);
		param.inputType = AQLMarketData::getVolInputType(model, fx, riskName);
		param.isParallel = true;
		param.isGrid = false;
		// set reference
		AQLStringVector ccys;
		AQLMarketData::convertToCurrency(fx, ccys);
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccys[1]));
		// set DDL
		AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
		// set file path
		AQLMarketData::getVolFuncFilePath(model, fx, fx, riskName, scenarioNum, param.paraFile, false);
		// create scenario
		AQLScenarioConfiguration *sceCreator = 
			AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
		delete sceCreator;
		return ret;
	}
	else
	{
		return vector<AQLObject *>(0); 
	}
}


/*!
    @brief return operator1

	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXATMParallelVega::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_OPERATOR);
}


/*!
    @brief return coefficient1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXATMParallelVega::getCoefficient1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_COEFFICIENT);
}


/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXATMParallelVega::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_OUTPUT);
}


/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXATMParallelVega::getRiskName(void) const
{
	return RISK_OFFICIAL_VOL_FXATMPARALLELVEGA;
}

/*!
    @brief return parallel shift val

	@return double 
*/
double
AQLRiskConfigurationVolFXATMParallelVega::getParallelShiftVal(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
							FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_SHIFTVAL).getDoubleValue() / 100;
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationVolFXATMParallelVega::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLString tmpCurrency = fx;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);

}