/*! @file
    @brief  FX atm parallel vega setup class 
*/
//  2008, Mizuho International London.
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
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LAPriceDataFunction.h"
#include "LAPricePortfolioValue.h"
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
#include "LAMathJamshidianSwaption.h"

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
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolFXATMParallelVega::createVolatilityEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	if (scenarioNum == SCENARIO_1)
	{
		const LAString model = LAMarketData::getModelName(fx);
		const LAString riskName = getRiskName();
		LAStringVector ccys;
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

		LAObjectPool &objPool = dataInstance.getObjectPool();
		// set reference
		LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
		LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, fx);
		delete calibInfoCreator;
		param.refName.push_back(infoName);

		// set yield and calibdata
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
		param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));

		// get term
		const LAStringVector &terms = dynamic_cast<const LADataStrings &>(objPool.getObject(infoName, ENCHKTYPE_ISDEFINED).getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
		// create ATM grid
		const unsigned int termSize = terms.size();
		param.paraTerm.resize(termSize);
		for (unsigned int i = 0; i < termSize; ++i)
		{
			param.paraTerm[i] = LAString(FXVOL_ATM) + LAString("_") + terms[i];
		}

		// dataout
		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
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
		LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));

		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
		delete sceCreator;
		return ret;
	}
	else
	{
		return vector<LAObject *>(0); 
	}
}

/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolFXATMParallelVega::createVolatilityEntityOld(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	if (scenarioNum == SCENARIO_1)
	{
		const LAString model = LAMarketData::getModelName(fx);
		const LAString riskName = getRiskName();
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
		LAStringVector ccys;
		LAMarketData::convertToCurrency(fx, ccys);
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
		// set DDL
		LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
		// set file path
		LAMarketData::getVolFuncFilePath(model, fx, fx, riskName, scenarioNum, param.paraFile, false);
		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
		delete sceCreator;
		return ret;
	}
	else
	{
		return vector<LAObject *>(0); 
	}
}


/*!
    @brief return operator1

	@return LAString
*/
LAString
LARiskConfigurationVolFXATMParallelVega::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_OPERATOR);
}


/*!
    @brief return coefficient1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXATMParallelVega::getCoefficient1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_COEFFICIENT);
}


/*!
    @brief return outputname1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXATMParallelVega::getOutPutName1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_OUTPUT);
}


/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationVolFXATMParallelVega::getRiskName(void) const
{
	return RISK_OFFICIAL_VOL_FXATMPARALLELVEGA;
}

/*!
    @brief return parallel shift val

	@return double 
*/
double
LARiskConfigurationVolFXATMParallelVega::getParallelShiftVal(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
							FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_SHIFTVAL).getDoubleValue() / 100;
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolFXATMParallelVega::isRiskCurrencyMode(const LAString &fx) const
{
	LAString tmpCurrency = fx;
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);

}