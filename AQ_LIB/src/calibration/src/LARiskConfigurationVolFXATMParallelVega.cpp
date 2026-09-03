/*! @file
    @brief  FX atm parallel vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolFXATMParallelVega.cpp
//
//  DESCRIPTION :        FX atm parallel vega setup class 
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


#include "LARiskConfigurationVolFXATMParallelVega.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LADealUtils.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LADefinitionsCalibration.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "AQLMathJamshidianSwaption.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationVolFXATMParallelVega::LARiskConfigurationVolFXATMParallelVega(void)
: LARiskConfigurationVolParallel(true)
{
}

// destructor
/*!

*/
LARiskConfigurationVolFXATMParallelVega::~LARiskConfigurationVolFXATMParallelVega(void)
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
LARiskConfigurationVolFXATMParallelVega::createVolatilityEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	if (scenarioNum == SCENARIO_1)
	{
		const AQLString model = LAMarketData::getModelName(fx);
		const AQLString riskName = getRiskName();
		AQLStringVector ccys;
		LAMarketData::convertToCurrency(fx, ccys);
		// scenario param
		MAScenarioParam param;
		param.ccy = fx;
		param.calcType = ccys[0] + "_" + ccys[1] + "_" + riskName;
		param.model = model;
		param.targetName = LAMarketData::getBaseVolatilityName(fx);
		param.inputType = LAMarketData::getVolInputType(model, fx, riskName);
		param.isCalib = true;
		param.isParallel = true;
		param.isGrid = false;
		param.shiftType = RISK_SHIFTTYPE_DIFF;

		AQLObjectPool &objPool = dataInstance.getObjectPool();
		// set reference
		LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
		AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, fx);
		delete calibInfoCreator;
		param.refName.push_back(infoName);

		// set yield and calibdata
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
		param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));

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
		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
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
		AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));

		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

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
LARiskConfigurationVolFXATMParallelVega::createVolatilityEntityOld(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	if (scenarioNum == SCENARIO_1)
	{
		const AQLString model = LAMarketData::getModelName(fx);
		const AQLString riskName = getRiskName();
		// scenario param
		MAScenarioParam param;
		param.ccy = fx;
		param.calcType= fx + "_" + riskName;
		param.model = model;
		param.targetName = LAMarketData::getBaseVolatilityName(fx);
		param.inputType = LAMarketData::getVolInputType(model, fx, riskName);
		param.isParallel = true;
		param.isGrid = false;
		// set reference
		AQLStringVector ccys;
		LAMarketData::convertToCurrency(fx, ccys);
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
		// set DDL
		AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
		// set file path
		LAMarketData::getVolFuncFilePath(model, fx, fx, riskName, scenarioNum, param.paraFile, false);
		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

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
LARiskConfigurationVolFXATMParallelVega::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_OPERATOR);
}


/*!
    @brief return coefficient1

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationVolFXATMParallelVega::getCoefficient1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_COEFFICIENT);
}


/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationVolFXATMParallelVega::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_OUTPUT);
}


/*!
    @brief return riskname

	@return AQLString
*/
AQLString
LARiskConfigurationVolFXATMParallelVega::getRiskName(void) const
{
	return RISK_OFFICIAL_VOL_FXATMPARALLELVEGA;
}

/*!
    @brief return parallel shift val

	@return double 
*/
double
LARiskConfigurationVolFXATMParallelVega::getParallelShiftVal(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
							FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_SHIFTVAL).getDoubleValue() / 100;
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolFXATMParallelVega::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLString tmpCurrency = fx;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);

}