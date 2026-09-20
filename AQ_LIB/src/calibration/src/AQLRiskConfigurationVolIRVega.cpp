/*! @file
    @brief  IR Vega setup class 
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <functional>
#include <algorithm>
#include <memory>
#include "AQLRiskConfigurationVolIRVega.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLDataReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLPriceIRVegaUnderlyingAsset.h"
#include "AQLPricePortfolioValue.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLDealUtils.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLLinearFunc.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLDataMatrix.h"
#include "AQLFunctionUtilities.h"
#include "AQLDefinitionsIRSABR.h"
#include "AQLMathCurveFuncUtility.h"

using namespace std;

namespace
{
	AQLString getOptionalRiskStaticData(const AQLString& keyL, const AQLString& ccyL, const AQLString& curveSuffixL)
	{
		AQLString propValue = AQLStaticDataManager::getRiskStaticData().getStaticData(ccyL + keyL + curveSuffixL);
		return (propValue == AQ_NO_DATA) ? "" : std::move(propValue);
	}

	AQLString getOptionalCalibStaticData(const AQLString& keyL, const AQLString& ccyL, const AQLString& curveSuffixL)
	{
		AQLString propValue = AQLStaticDataManager::getCalibStaticData().getStaticData(ccyL + keyL + curveSuffixL);
		return (propValue == AQ_NO_DATA) ? "" : std::move(propValue);
	}

	bool isTrue(const AQLString& s)
	{
		if (s.size() > 0)
		{
			AQLDataBool attr;
			attr.convertFromString(s);
			return attr.get();
		}
		else
		{
			return false;
		}
	}

	struct UnderlyingInfo
	{
		AQLStringVector mCurveTypeKeys;
		AQLStringVector mCurveTypes;

		AQLString mCapSpotLag;
		AQLPriceDataDayCount mCapDayCount;
		AQLPriceDataSlidingRule mCapSlidingRule;
		AQLPriceDataCalendar mCapPaymentCalendar;
		AQLPriceDataCalendar mCapFixingCalendar;

		AQLString mSwaptionSpotLag;
		AQLPriceDataDayCount mSwaptionDayCount;
		AQLPriceDataSlidingRule mSwaptionSlidingRule;
		AQLPriceDataCalendar mSwaptionPaymentCalendar;
		AQLPriceDataCalendar mSwaptionFixingCalendar;
		AQLString mSwaptionFrequency;
	};
}

// constructor
/*!

*/
AQLRiskConfigurationVolIRVega::AQLRiskConfigurationVolIRVega(void)
: AQLRiskConfigurationVolatility(false)
{
}

// destructor
/*!

*/
AQLRiskConfigurationVolIRVega::~AQLRiskConfigurationVolIRVega(void)
{
}

/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
AQLRiskConfigurationVolIRVega::createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{	
	const AQLString model = AQLMarketData::getModelName(ccy);
 	const AQLString riskName = getRiskName();
	AQLString inputType = AQLMarketData::getVolInputType(model, ccy, riskName);
	AQLString bumpDirection = getBumpDirection(ccy);
	inputType.toUpper();
	bumpDirection.toUpper();
	
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}	
	AQLScenarioParam param;
	param.ccy = ccy;
	param.targetCurveType = getCurveType(param.ccy);
	param.targetCurveType.toLower();
	param.calcType = ccy + "_" + riskName + "_" + AQLString(scenarioNum) + "_" + AQLString(index) + getCurveSuffix(ccy);
	param.model = model;
	param.shiftType = getShiftType(ccy);
	param.bumpDirection = getBumpDirection(ccy);
	param.bumpType = getBumpType(ccy);
	param.targetName = AQLMarketData::getBaseVolatilityName(ccy);
	param.inputType = inputType;
	param.isWave = isWave(ccy);
	param.vegaType = getVegaType(ccy);

	if (inputType == INPUT_T_DATA_MATRIX)
	{
		// data use pattern
		if (isParallelShift(ccy))
		{
			param.isParallel = true;
			// parallel shift
			if (!isShiftValFileUse(ccy))
			{
				if (scenarioNum == SCENARIO_1)
				{
					param.paraShiftVec.push_back(getScenario1ShiftValue(ccy));
				}
				else
				{
					param.paraShiftVec.push_back(getScenario2ShiftValue(ccy));
				}
			}
			else
			{
				param.paraShiftMtx = getFileValMatrix(ccy);
			}
			// set filePath
			AQLMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
		}
		if (isGridSensitivity(ccy))
		{	
			param.isGrid = true;
			// grid shift
			if (!isShiftValFileUse(ccy))
			{
				if (scenarioNum == SCENARIO_1)
				{
					param.gridShiftVec.push_back(getScenario1ShiftValue(ccy));
				}
				else
				{
					param.gridShiftVec.push_back(getScenario2ShiftValue(ccy));
				}
			}
			else
			{
				param.gridShiftMtx = getFileValMatrix(ccy);
			}

			// set grid file
			param.gridTerm = getGridTerm(ccy);
			unsigned int gridSize = param.gridTerm.size();
			param.gridFile.resize(gridSize);
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				AQLMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.gridFile[i], true, &param.gridTerm[i]);
			}
		}

		// minus value
		if ((scenarioNum == SCENARIO_1 && bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
			|| (scenarioNum == SCENARIO_2 && bumpDirection == RISK_BUMPDIRECTION_UPDOWNSHIFT))
		{
			if (!param.paraShiftVec.empty())
			{
				param.paraShiftVec[0] *= -1.0;
			}
			if (!param.gridShiftVec.empty())
			{
				param.gridShiftVec[0] *= -1.0;
			}

			unsigned int size_p = param.paraShiftMtx.size();
			for (unsigned int i = 0; i < size_p; ++i)
			{
				transform(param.paraShiftMtx[i].begin(), param.paraShiftMtx[i].end(), param.paraShiftMtx[i].begin(), negate<double>());
			}

			unsigned int size_g = param.gridShiftMtx.size();
			for (unsigned int i = 0; i < size_g; ++i)
			{
				transform(param.gridShiftMtx[i].begin(), param.gridShiftMtx[i].end(), param.gridShiftMtx[i].begin(), negate<double>());
			}
		}
	}
	else
	{
		param.isCalib = true;
		// dataout
		if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}
		// function use pattern
		if (isParallelShift(ccy))
		{
			param.isParallel = true;
			AQLObjectPool &objPool = dataInstance.getObjectPool();
			// push dmy file
			param.paraFile.push_back(CALIB_DMY_FILE);
			if (scenarioNum == SCENARIO_1)
			{
				param.paraShiftVec.push_back(getScenario1ShiftValue(ccy));
			}
			else
			{
				param.paraShiftVec.push_back(getScenario2ShiftValue(ccy));
			}
			// set refName
			AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
			AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
			delete calibInfoCreator;

			param.refName.push_back(infoName);
			AQLString ycName = getScenarioBaseYieldName(ccy);
			param.refName.push_back(ycName);
			//param.refName.push_back(AQLMarketData::getBaseYieldName(ccy));

		}
		if (isGridSensitivity(ccy))
		{
			param.isGrid = true;
			// set grid file
			param.gridTerm = getGridTerm(ccy);
			unsigned int size = param.gridTerm.size();
			// resize only
			param.gridFile.resize(size);
			// set shift val
			if (scenarioNum == SCENARIO_1)
			{
				param.gridShiftVec.resize(size, getScenario1ShiftValue(ccy));
			}
			else
			{
				param.gridShiftVec.resize(size, getScenario2ShiftValue(ccy));
			}

			AQLObjectPool &objPool = dataInstance.getObjectPool();
			if (param.refName.empty())
			{
				// set refName
				AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
				AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
				delete calibInfoCreator;
				param.refName.push_back(infoName);
			}

			//AQLString ycName = AQLMarketData::getBaseYieldName(ccy);
			AQLString ycName = getScenarioBaseYieldName(ccy);
			for (unsigned int i = 0; i < size; ++i)
			{
				param.refName.push_back(ycName);
			}

		}
		// minus value
		if ((scenarioNum == SCENARIO_1 && bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
			|| (scenarioNum == SCENARIO_2 && bumpDirection == RISK_BUMPDIRECTION_UPDOWNSHIFT))
		{
			if (!param.paraShiftVec.empty())
			{
				transform(param.paraShiftVec.begin(), param.paraShiftVec.end(), param.paraShiftVec.begin(), negate<double>());
			}
			if (!param.gridShiftVec.empty())
			{
				transform(param.gridShiftVec.begin(), param.gridShiftVec.end(), param.gridShiftVec.begin(), negate<double>());
			}
		}
	}

	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	
	storeIRAdditionalInfo(dataInstance, param);
	
	return ret;
}


/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
AQLRiskConfigurationVolIRVega::createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{	
	const AQLString model = AQLMarketData::getModelName(ccy);
 	const AQLString riskName = getRiskName();
	AQLString inputType = AQLMarketData::getVolInputType(model, ccy, riskName);
	AQLString bumpDirection = getBumpDirection(ccy);
	inputType.toUpper();
	bumpDirection.toUpper();
	
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}	
	AQLScenarioParam param;
	param.ccy = ccy;
	param.calcType = ccy + "_" + riskName + "_" + AQLString(scenarioNum);
	param.model = model;
	param.shiftType = getShiftType(ccy);
	param.bumpDirection = getBumpDirection(ccy);
	param.bumpType = getBumpType(ccy);
	param.targetName = AQLMarketData::getBaseVolatilityName(ccy);
	param.inputType = inputType;

	if (inputType == INPUT_T_DATA_MATRIX)
	{
		// data use pattern
		if (isParallelShift(ccy))
		{
			param.isParallel = true;
			// parallel shift
			if (!isShiftValFileUse(ccy))
			{
				if (scenarioNum == SCENARIO_1)
				{
					param.paraShiftVec.push_back(getScenario1ShiftValue(ccy));
				}
				else
				{
					param.paraShiftVec.push_back(getScenario2ShiftValue(ccy));
				}
			}
			else
			{
				param.paraShiftMtx = getFileValMatrix(ccy);
			}
			// set filePath
			AQLMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
		}
		if (isGridSensitivity(ccy))
		{	
			param.isGrid = true;
			// grid shift
			if (!isShiftValFileUse(ccy))
			{
				if (scenarioNum == SCENARIO_1)
				{
					param.gridShiftVec.push_back(getScenario1ShiftValue(ccy));
				}
				else
				{
					param.gridShiftVec.push_back(getScenario2ShiftValue(ccy));
				}
			}
			else
			{
				param.gridShiftMtx = getFileValMatrix(ccy);
			}

			// set grid file
			param.gridTerm = getGridTerm(ccy);
			unsigned int gridSize = param.gridTerm.size();
			param.gridFile.resize(gridSize);
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				AQLMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.gridFile[i], true, &param.gridTerm[i]);
			}
		}

		// minus value
		if ((scenarioNum == SCENARIO_1 && bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
			|| (scenarioNum == SCENARIO_2 && bumpDirection == RISK_BUMPDIRECTION_UPDOWNSHIFT))
		{
			if (!param.paraShiftVec.empty())
			{
				param.paraShiftVec[0] *= -1.0;
			}
			if (!param.gridShiftVec.empty())
			{
				param.gridShiftVec[0] *= -1.0;
			}

			unsigned int size_p = param.paraShiftMtx.size();
			for (unsigned int i = 0; i < size_p; ++i)
			{
				transform(param.paraShiftMtx[i].begin(), param.paraShiftMtx[i].end(), param.paraShiftMtx[i].begin(), negate<double>());
			}

			unsigned int size_g = param.gridShiftMtx.size();
			for (unsigned int i = 0; i < size_g; ++i)
			{
				transform(param.gridShiftMtx[i].begin(), param.gridShiftMtx[i].end(), param.gridShiftMtx[i].begin(), negate<double>());
			}
		}
	}
	else
	{
		// function use pattern
		if (isParallelShift(ccy))
		{
			param.isParallel = true;
			// set filePath
			AQLMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
			param.refName.push_back(AQLMarketData::getBaseYieldName(ccy));

		}
		if (isGridSensitivity(ccy))
		{
			param.isGrid = true;
			// set grid file
			param.gridTerm = getGridTerm(ccy);
			unsigned int size = param.gridTerm.size();
			param.gridFile.resize(size);
			AQLString ycName = AQLMarketData::getBaseYieldName(ccy);
			for (unsigned int i = 0; i < size; ++i)
			{
				param.gridTerm[i].exchange("_",".");
				AQLMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.gridFile[i], true, &param.gridTerm[i]);
				param.refName.push_back(ycName);
			}
		}
	}

	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief setUp targetNames

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolIRVega::getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	const AQLString targetName = AQLMarketData::getBaseVolatilityName(ccy);

	AQLString ret = "";
	if (isParallelShift(ccy))
	{
		ret += targetName + ":";
	}

	if (isGridSensitivity(ccy))
	{
		const unsigned int num = getGridTerm(ccy).size();
		for (unsigned int i = 0; i < num; ++i)
		{
			ret += targetName + ":";
		}
	}

	if (ret != "")
		ret = ret.subString(0, ret.size() - 2);

	return ret;

}

/*!
    @brief return extra scenario1 target names

	@param[in] ccy
	@return DoubleArray
*/
AQLStringVector
AQLRiskConfigurationVolIRVega::getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	AQLStringVector ret(0);
	AQLStringVector targetNames = getTargetNames(ccy, dataInstance).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int targetSize = targetNames.size();

	AQLStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		AQLString fx_targetNames1;
		AQLString fx_bVolName = AQLMarketData::getBaseVolatilityName(targetFXVec[i]);
		for (unsigned int j = 0; j < targetSize; ++j)
		{
			fx_targetNames1 += fx_bVolName + ":";
		}

		if (fx_targetNames1.size() < 2)
		{
			throw AQLCoreInvalidData("Extra targetname1 can not set !!", __FILE__, __LINE__);
		}
		ret.push_back(fx_targetNames1.subString(0, fx_targetNames1.size() - 2));

	}
	
	return ret;
}



/*!
    @brief create extra senario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
AQLRiskConfigurationVolIRVega::createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	if (isRealCalib())
	{
		return createExtraFXScenarioEntity(ccy, dataInstance, SCENARIO_1, index);
	}
	else
	{
		return createExtraFXScenarioEntityOld(ccy, dataInstance, SCENARIO_1, index);
	}
}

/*!
    @brief return extra scenario2 target names

	@param[in] ccy
	@return DoubleArray
*/
AQLStringVector
AQLRiskConfigurationVolIRVega::getExtraTargetNames2(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return AQLStringVector(0);
	}

	return getExtraTargetNames1(ccy, dataInstance);
}

/*!
    @brief create extra senario2 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
AQLRiskConfigurationVolIRVega::createExtraScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<vector<AQLObject *> >(0);
	}
	if (isRealCalib())
	{
		return createExtraFXScenarioEntity(ccy, dataInstance, SCENARIO_2, index);
	}
	else
	{
		return createExtraFXScenarioEntityOld(ccy, dataInstance, SCENARIO_2, index);
	}
}

/*!
    @brief create extra object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
AQLRiskConfigurationVolIRVega::createExtraFXScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	vector<vector<AQLObject *> > ret(0);

	const AQLString model = AQLMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	const AQLString irCalcType = ccy + "_" + riskName + "_" + AQLString(scenarioNum) + "_" + AQLString(index);

	AQLString riskNameSuffix;
	if (scenarioNum == SCENARIO_1)
	{
		riskNameSuffix = "ExtraScenario1";
	}
	else
	{
		riskNameSuffix = "ExtraScenario2";
	}

	// set parameter for fx vol shift
	AQLStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		//vector<AQLObject *> sce_fx;
		AQLObjectPool &objPool = dataInstance.getObjectPool();
		AQLStringVector ccys = targetFXVec[i].toToken(FX_DELIMITER);
		AQLString key_fx = targetFXVec[i].toLower();
		const AQLString model_fx = AQLMarketData::getModelName(key_fx);
		// set up param
		AQLScenarioParam param;
		param.ccy = key_fx;
		param.isCalib = true;
        param.calcType=  ccy + "_" + ccys[0] + "_" + ccys[1] + "_" + riskName + "_" + riskNameSuffix + "_" + AQLString(index);
		param.model = model_fx;
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
		param.inputType = AQLMarketData::getVolInputType(model_fx, key_fx, riskName);
		param.targetName = AQLMarketData::getBaseVolatilityName(key_fx);
		
		// dataout
		if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}

		AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
		AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, key_fx);
		delete calibInfoCreator;
		
		param.refName.push_back(infoName);

		AQLString dBaseYieldName = AQLMarketData::getBaseYieldName(ccys[0]);
		AQLString dBaseCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, dBaseYieldName));
		AQLString fBaseYieldName = AQLMarketData::getBaseYieldName(ccys[1]);
		AQLString fBaseCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, fBaseYieldName));

		ccys[0].toUpper();
		AQLString tmpCurrency = ccy;
		tmpCurrency.toUpper();

		if (ccys[0] == tmpCurrency)
		{
			dBaseYieldName = getScenarioBaseYieldName(ccys[0]);
		}
		else if (ccys[1] == tmpCurrency)
		{
			fBaseYieldName = getScenarioBaseYieldName(ccys[1]);
		}

		bool isParallel = isParallelShift(ccy);
		if (isParallel)
		{
			param.isParallel = true;
			param.paraFile.push_back(CALIB_DMY_FILE);

			// set reference
			if (ccys[0] == tmpCurrency)
			{
				// set domestic curve name
				param.refName.push_back(dBaseYieldName);
				if (isCalibTarget(ccy))
				{
					param.refName.push_back(AQLMarketData::getCalibDataName(irCalcType, AQLMarketData::getYieldDataName(objPool, dBaseYieldName)));
				}
				else
				{
					param.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, dBaseYieldName)));
				}
				// set foreign curve name
				param.refName.push_back(fBaseYieldName);
				param.refName.push_back(fBaseCalibDataName);
			}
			else
			{
				// set domestic curve name
				param.refName.push_back(dBaseYieldName);
				param.refName.push_back(dBaseCalibDataName);
				// set foreign curve name
				param.refName.push_back(fBaseYieldName);
				if (isCalibTarget(ccy))
				{
					param.refName.push_back(AQLMarketData::getCalibDataName(irCalcType, AQLMarketData::getYieldDataName(objPool, fBaseYieldName)));
				}
				else
				{
					param.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, fBaseYieldName)));
				}
			}
		}
		if (isGridSensitivity(ccy))
		{
			// grid case
			param.isGrid = true;
			AQLStringVector gridTerm = getGridTerm(ccy);
			const unsigned int gridSize = gridTerm.size();
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				param.gridFile.push_back(AQLStringVector());
				// set reference
				if (ccys[0] == tmpCurrency)
				{
					// set domestic curve name
					param.refName.push_back(dBaseYieldName);
					if (isCalibTarget(ccy))
					{
						param.refName.push_back(AQLMarketData::getCalibDataName(irCalcType, AQLMarketData::getYieldDataName(objPool, dBaseYieldName), i));
					}
					else
					{
						param.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, dBaseYieldName)));
					}
					// set foreign curve name
					param.refName.push_back(fBaseYieldName);
					param.refName.push_back(fBaseCalibDataName);
				}
				else
				{
					// set domestic curve name
					param.refName.push_back(dBaseYieldName);
					param.refName.push_back(dBaseCalibDataName);
					// set foreign curve name
					param.refName.push_back(fBaseYieldName);
					if (isCalibTarget(ccy))
					{
						param.refName.push_back(AQLMarketData::getCalibDataName(irCalcType, AQLMarketData::getYieldDataName(objPool, fBaseYieldName), i));
					}
					else
					{
						param.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, fBaseYieldName)));
					}
				}
			}
		}
		// create scenario
		AQLScenarioConfiguration *sceCreator = 
			AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<AQLObject *> sce_fx = sceCreator->createScenario(dataInstance, param);
		delete sceCreator;

		ret.push_back(sce_fx);

	}

	return ret;
}

/*!
    @brief create extra object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
AQLRiskConfigurationVolIRVega::createExtraFXScenarioEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	vector<vector<AQLObject *> > ret(0);

	const AQLString model = AQLMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();

	AQLString riskNameSuffix;
	if (scenarioNum == SCENARIO_1)
	{
		riskNameSuffix = "ExtraScenario1";
	}
	else
	{
		riskNameSuffix = "ExtraScenario2";
	}

	// set parameter for fx vol shift
	AQLStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		vector<AQLObject *> sce_fx;
		AQLStringVector ccys = targetFXVec[i].toToken(FX_DELIMITER);
		AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
		const AQLString model_fx = AQLMarketData::getModelName(key_fx);
		// set up param
		AQLScenarioParam param;
		param.ccy = key_fx;
		param.model = model_fx;
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
		param.inputType = AQLMarketData::getVolInputType(model_fx, key_fx, riskName);
		param.targetName = AQLMarketData::getBaseVolatilityName(key_fx);
		param.refName.resize(2);
		// set domestic curve name
		param.refName[0] = AQLMarketData::getBaseYieldName(ccys[0]);
		// set foreign curve name
		param.refName[1] = AQLMarketData::getBaseYieldName(ccys[1]);
	
		// create scenario
		AQLScenarioConfiguration *sceCreator = 
			AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		ccys[0].toUpper();
		AQLString tmpCurrency = ccy;
		tmpCurrency.toUpper();
		bool isParallel = isParallelShift(ccy);
		if (isParallel)
		{
			param.calcType = ccy + "_" + key_fx + "_" + riskName + "_" + riskNameSuffix + "_1_" + AQLString(index) + "_Parallel";
			param.isParallel = true;
			AQLMarketData::getVolFuncFilePath(model_fx, ccy, key_fx, riskName, scenarioNum, param.paraFile, false);
				
			vector<AQLObject *> sce_tmp = sceCreator->createScenario(dataInstance, param);
			if (sce_tmp.size() != 1)
			{
				throw AQLCoreInvalidData("Parallel shift scenaro size must be one", __FILE__, __LINE__);
			}
			sce_fx.push_back(sce_tmp[0]);
		}
		if (isGridSensitivity(ccy))
		{
			// create as parallel shift
			param.isParallel = true;
			AQLStringVector gridTerm = getGridTerm(ccy);
			const unsigned int gridSize = gridTerm.size();
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				gridTerm[i].exchange("_",".");
				param.calcType = ccy + "_" + key_fx + "_" + riskName + "_" + riskNameSuffix + "_1_" + AQLString(index) + "_Grid_" + AQLString(static_cast<int>(i));

				AQLMarketData::getVolFuncFilePath(model_fx, ccy, key_fx, riskName, scenarioNum, param.paraFile, true, &gridTerm[i]);
				vector<AQLObject *> sce_tmp = sceCreator->createScenario(dataInstance, param);
				if (sce_tmp.size() != 1)
				{
					throw AQLCoreInvalidData("Parallel shift scenaro size must be one", __FILE__, __LINE__);
				}
				sce_fx.push_back(sce_tmp[0]);
			}
		}

		delete sceCreator;

		ret.push_back(sce_fx);
	}
	return ret;
}

/*!
    @brief get grid matrix

	@param[in] ccy
	@return DoubleMatrix
*/
DoubleMatrix
AQLRiskConfigurationVolIRVega::getCoordinatesMatrix(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLStringVector grid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
		STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_COORDINATES + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);
	// day count
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT));
	// asOfDate
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	const unsigned int COORDINATESNUM = 4;
	const unsigned int size = grid.size();
	DoubleMatrix ret(size);
	for (unsigned int i = 0; i < size; ++i)
	{
		AQLStringVector cdns = grid[i].toToken('_');
		if (cdns.size() != COORDINATESNUM)
		{
			throw AQLCoreInvalidData(" IR vega coordinate grid format is wrong .", __FILE__, __LINE__);
		}
		ret[i].resize(COORDINATESNUM);
		for (unsigned  int j = 0; j < COORDINATESNUM; ++j)
		{
			ret[i][j] = dayCount.getTerm(asOfDate, AQLDateCalculations::getDate(asOfDate, cdns[j], true));
		}
	}

	return ret;
}


/*!
    @brief get file val matrix

	@param[in] ccy
	@return DoubleMatrix
*/
DoubleMatrix
AQLRiskConfigurationVolIRVega::getFileValMatrix(const AQLString &ccy) const
{
	
	AQLString tmpCurrency = ccy;
	AQLString strMatrixFile = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SHIFTVAL_FILE + getCurveSuffix(ccy));
	AQLFileAccessor matrixFile(AQLMarketData::getNumFileName(strMatrixFile));
	AQLStringMatrix f_dataMatrix;
	matrixFile.readAllData(MARKET_DATA_DELIMITER, f_dataMatrix);
	matrixFile.close();

	const unsigned int size_t = f_dataMatrix.size();
	DoubleMatrix ret(size_t);
	for (unsigned int i = 0; i < size_t; ++i)
	{
		const unsigned int size_T = f_dataMatrix[i].size();
		ret[i].resize(size_T);
		for  (unsigned int j = 0; j < size_T; ++j)
		{
			ret[i][j] = f_dataMatrix[i][j].getDoubleValue();
		}
	}

	return ret;
}


/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationVolIRVega::getCoordinates(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
		STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_COORDINATES + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);

}

/*!
    @brief return bump type

	@param[in] ccy
	@return string
*/
AQLString
AQLRiskConfigurationVolIRVega::getBumpType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString gridType = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_TYPE + getCurveSuffix(ccy));
	gridType.toUpper();

	if (gridType == RISK_GRID_MARKET)
	{
		return RISK_MARKET_BUMP;
	}
	else if (gridType == RISK_GRID_COORDINATES)
	{
		return RISK_MODEL_VOL_BUMP;
	}
	else
	{
		AQLString msg = "This grid type is not supported by bumpType. grid type = " + gridType;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}


/*!
    @brief return grid type

	@param[in] ccy
	@return string
*/
AQLString
AQLRiskConfigurationVolIRVega::getGridType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_TYPE + getCurveSuffix(ccy));
}


/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
AQLStringVector
AQLRiskConfigurationVolIRVega::getGridTerm(const AQLString &ccy) const
{
	AQLString gridType = getGridType(ccy);
	gridType.toUpper();

	if (gridType == RISK_GRID_MARKET)
	{
		AQLString tmpCurrency = ccy;
		//AQLString underlying = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_VOL_UNDERLYING);

		/*if (underlying == AQ_NO_DATA)
		{*/
		return getMarketTerm(ccy);
		/*}
		else
		{
			AQLStringVector gridTerm = getMarketTerm(ccy);
			for (unsigned int i=0; i<gridTerm.size(); i++)
			{
				gridTerm[i] = gridTerm[i];
			}
			return gridTerm;
		}*/
	}
	else if (gridType == RISK_GRID_COORDINATES)
	{
		return getCoordinates(ccy);
	}
	else
	{
		AQLString msg = "This grid type is not supported. grid type = " + gridType;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<AQLString>
*/
AQLStringVector
AQLRiskConfigurationVolIRVega::getBucketGridTerm(const AQLString &ccy) const
{
	ccy;
	vector<AQLString> ret;
	ret.push_back(AQ_NO_DATA);
	return ret;
	
}

/*!
    @brief return market  term

	@param[in] ccy
	@return vector<AQLString>
*/
AQLStringVector
AQLRiskConfigurationVolIRVega::getMarketTerm(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;

	AQLStringVector mktTems_prop = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
			STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_MARKET + getCurveSuffix(ccy)).toToken(':');

	return reduceTargetGrids(ccy, mktTems_prop);
}

/*!
    @brief reduce market grids in SABR model

	@param[in] ccy
	@param[in] market grids
	@return vector<AQLString>
*/
AQLStringVector 
AQLRiskConfigurationVolIRVega::reduceTargetGrids(const AQLString &ccy, const AQLStringVector &mktGrids) const
{
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();
	AQLString model = AQLMarketData::getModelName(ccy);
	model.toUpper();

	if (model == MODEL_IRSABR)
	{
		// calibration target flag
		AQLString calibTarget_str = AQLCoreDataService::getContext(tmpCurrency + "." CONTEXT_KEY_DEAL_IRVOL + getCurveSuffix(ccy));
		if (calibTarget_str == AQ_NO_DATA)
			return  mktGrids;

		AQLDataBoolMatrix tmp;
		tmp.convertFromString(calibTarget_str);
		BoolMatrix calibTarget = tmp.get();
		// optionmaturity
		AQLStringVector optionMatVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_OPTIONMATURITY + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);
		uppervec(optionMatVec);
		// swapterm
		AQLStringVector tenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTENOR + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);
		uppervec(tenorVec);
		// check size
		if (calibTarget.size() == 0)
			throw AQLCoreInvalidData("Calibration flag matrix has no data!",	__FILE__,__LINE__);
		if (calibTarget.size() != optionMatVec.size() || calibTarget[0].size() != tenorVec.size())
			throw AQLCoreInvalidData("Calibration flag matrix does not match option maturity or tenor!",	__FILE__,__LINE__);
		// make map
		std::map<AQLString, bool> map_calib_target;
		for(size_t i = 0; i < optionMatVec.size(); ++i)
		{
			for(size_t j = 0; j < tenorVec.size(); ++j)
			{
				map_calib_target[optionMatVec[i] + "_" + tenorVec[j]] = calibTarget[i][j];
			}
		}

		// sort 1st : expiry descending, 2nd : tenor ascending
		std::map<std::pair<int, int>, AQLString> targetGrids;
		for(size_t i = 0; i < mktGrids.size(); ++i)
		{
			AQLString tmpTerm = mktGrids[i];
			tmpTerm.toUpper();
			AQLStringVector termVec = tmpTerm.toToken('_');
			if (termVec.size() != 3)
				throw AQLCoreInvalidData("Format of market grid is wrong!",	__FILE__,__LINE__);
			if (map_calib_target[termVec[1] + "_" + termVec[2]]){
				int y, m, d, w;
				AQLDateCalculations::termStrtoYMDW(termVec[1], y, m, d, w);
				const int expiryMonth = y * 12 + m;
				AQLDateCalculations::termStrtoYMDW(termVec[2], y, m, d, w);
				const int tenorMonth = y * 12 + m;
				targetGrids[std::make_pair(-expiryMonth, tenorMonth)] = mktGrids[i];
			}
		}
		AQLStringVector ret;
		for(std::map<std::pair<int, int>, AQLString>::const_iterator i = targetGrids.begin(); i != targetGrids.end(); ++i){
			ret.push_back(i->second);
		}

		/*if (ret.size() == 0)
			throw AQLCoreInvalidData("The number of risk target grids is 0!",	__FILE__,__LINE__);*/

		return ret;
	}
	else
	{
		return  mktGrids;
	}
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolIRVega::getOutPutName1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationVolIRVega::getRiskName(void) const
{
	return RISK_FRONT_VOL_IRVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationVolIRVega::isGridSensitivity(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
AQLRiskConfigurationVolIRVega::isParallelShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISPARALLEL + getCurveSuffix(ccy)));
}

/*!
    @brief return shiftvalfileuse

	@param[in] ccy
	@return bool 
*/
bool
AQLRiskConfigurationVolIRVega::isShiftValFileUse(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISFILEUSE + getCurveSuffix(ccy)));
}

/*!
    @brief return div unit value

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationVolIRVega::getDivUnit(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_DIVUNIT + getCurveSuffix(ccy)).getDoubleValue();
}


/*!
    @brief return scenario1 value

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationVolIRVega::getScenario1ShiftValue(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	double shiftVal = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
						STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SHIFTVAL + getCurveSuffix(ccy)).getDoubleValue();

	return shiftVal / 100.0;
}


/*!
    @brief return  scenario2 value

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationVolIRVega::getScenario2ShiftValue(const AQLString &ccy) const
{
	return getScenario1ShiftValue(ccy);
}

/*!
    @brief return shift type

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolIRVega::getShiftType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SHIFTTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolIRVega::getBumpDirection(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_BUMPDIRECTION + getCurveSuffix(ccy));

}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationVolIRVega::isWave(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief  return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationVolIRVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRVEGA_TARGET_CURRENCY);
}


/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationVolIRVega::getCalibTargetCurrencies() const
{
	AQLString targetFX = mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRVEGA_CALIBRATION_TARGET_FX).toUpper();
	if (targetFX == "ALL")
	{
		return "ALL";
	}
	AQLString ret;
	AQLStringVector ccys = AQLDealUtils::getSDECurrencys();
	unsigned int ccySize = ccys.size();
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		if (ccys[i].findString(FX_DELIMITER) >= 0)
		{
			break;
		}
		ret += ccys[i] + ":";
	}
	if (ret.size() <= 0)
	{
		throw AQLCoreInvalidData("SDE currency is does not exist", __FILE__, __LINE__);
	}
	ret += targetFX;
	return ret;
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationVolIRVega::isRiskCurrencyMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISRISKCURRENCYMODE + getCurveSuffix(ccy));
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}


/*!
    @brief get calib target fx

	@param ccy
	get calibration target fx.
	if ccy is not calibtarget, fx is not calibrated.	

*/
AQLStringVector
AQLRiskConfigurationVolIRVega::getCalibTargetFX(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	AQLStringVector fCurveCcys;
	/*if (ccy.findString(FX_DELIMITER) < 0 )
	{
		AQLObjectPool &objPool = dataInstance.getObjectPool();
		AQLMathYieldCurvePro &ycPro = dynamic_cast<AQLMathYieldCurvePro &>
						(objPool.getObject(AQLMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());
		fCurveCcys = ycPro.getAffectingCcy();
	}*/

	AQLStringVector ret;
	//for nocalib
	AQLString tmpCurrency = ccy;
	tmpCurrency.toUpper();
	AQLStringVector simccys = AQLDealUtils::getSimulationSDECurrencys();
	unsigned int simccySize = simccys.size();
	bool isForeCcyInSimccy = false;
	if (simccySize != 0)
	{
		for (unsigned int i = 0; i < simccySize; ++i)
		{
			simccys[i].toUpper();
			for (unsigned int j = 0; j < fCurveCcys.size(); j++)
			{
				if (simccys[i].findString(fCurveCcys[j]) >= 0) isForeCcyInSimccy = true;
			}
			if (simccys[i].findString(FX_DELIMITER) >= 0 &&
				isCalibTarget(simccys[i]) && (simccys[i].findString(tmpCurrency) >= 0 || isForeCcyInSimccy))
			{
				ret.push_back(simccys[i]);
			}
		}
		return ret;
	}
	else
	{
		AQLStringVector ccys = AQLDealUtils::getSDECurrencys();
		unsigned int ccySize = ccys.size();
		for (unsigned int i = 0; i < ccySize; ++i)
		{
			ccys[i].toUpper();
			for (unsigned int j = 0; j < fCurveCcys.size(); j++)
			{
				if (ccys[i].findString(fCurveCcys[j]) >= 0) isForeCcyInSimccy = true;
			}
			if (ccys[i].findString(FX_DELIMITER) >= 0 &&
				isCalibTarget(ccys[i]) && (ccys[i].findString(tmpCurrency) >= 0 || isForeCcyInSimccy))
			{
				ret.push_back(ccys[i]);
			}
		}
		return ret;
	}	
}

/*!
@brief  return vega type

@return AQLString
*/
AQLString
AQLRiskConfigurationVolIRVega::getVegaType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString vegaType = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_VEGATYPE + getCurveSuffix(ccy));
	vegaType.toUpper();
	if ((vegaType != RISK_VEGATYPE_BETA) && (vegaType != RISK_VEGATYPE_NU) && (vegaType != RISK_VEGATYPE_RHO)) {
		return RISK_VEGATYPE_ATM;
	}
	else {
		return vegaType;
	}
}

/*!
    @brief calculate forward swap rate, swaption volatility, option maturity, settle term and store to trade object

	@param[in] objPool: object pool
	@param[in] param: Scenario param
	@byproduct add data that contains forward swap rate, swaption volatility, option maturity, settle term to tarade object. 
*/
void
AQLRiskConfigurationVolIRVega::storeIRAdditionalInfo(AQLDataInstance &dataInstance, const AQLScenarioParam& param) const
{
	AQLObjectPool& objPool = dataInstance.getObjectPool();
	const AQLString& riskName = param.calcType;
	AQLStringVector riskNameVector = riskName.toToken('_');

	if (riskNameVector[1] != "IRVEGA")
	{
		return;
	}

	if (param.bumpType == RISK_MODEL_VOL_BUMP)
	{
		return;
	}

	size_t gridSize = param.gridTerm.size();
	DoubleArray resForwards(gridSize, DBL_MAX), resVols(gridSize, DBL_MAX), optionMaturities(gridSize, DBL_MAX), setttleTerms(gridSize, DBL_MAX);

	// get swaption vol file
	AQLString swaptionFile = AQLMarketData::getVolFileName(param.model, param.ccy, param.targetCurveType);

	for (size_t i = 0; i < gridSize; ++i)
	{
		unsigned int curPos = 1 + i + (param.isParallel ? 1 : 0);
		const AQLString curveName = param.refName[curPos];

		AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());


		const AQLString optionMaturity = param.gridTerm[i].toToken('_')[1].toUpper(); // convert : Ccy_xM_yM => xM.
		const AQLString tenor = param.gridTerm[i].toToken('_')[2].toUpper(); // convert : Ccy_xM_yM => yM.
		if (optionMaturity.toToken('-').size() > 1 || tenor.toToken('-').size() > 1)
		{
			// This is backet risk case. We skip calculation
			return;
		}

		AQLSwapConvention swapConvention = AQLMarketData::getSwapConvention(objPool, optionMaturity, tenor, curveName, param);

		//calc swap rate
		AQLPriceDataSlidingRule slr_Fol; slr_Fol.convertFromString(FOL);

		AQLDate optionMaturityDate = AQLDateCalculations::getDate(asOfDate, optionMaturity, swapConvention.slidingRule, &swapConvention.fixingCalendar, true);
		AQLDate optionMaturityDateIncludingSpotLag = AQLDateCalculations::getDate(optionMaturityDate, swapConvention.spotLag, slr_Fol, &swapConvention.fixingCalendar, true);
		AQLDate endDate = AQLDateCalculations::getDate(optionMaturityDateIncludingSpotLag, tenor, swapConvention.slidingRule, &swapConvention.paymentCalendar, true);

		DateVector dates;
		double rate;
		if (swapConvention.frequency == SIMPLE)
		{
			dates.push_back(optionMaturityDateIncludingSpotLag);
			dates.push_back(endDate);
			rate = AQLMathCurveFuncUtility::getParRate(dates, &dataInstance, swapConvention.curveID, swapConvention.daycount.convertToString(), AQLString("SPLINE"), swapConvention.forecastCurveName, swapConvention.discountCurveName, true);
		}
		else
		{
			rate = AQLMathCurveFuncUtility::getParRate(&dataInstance, swapConvention.curveID, optionMaturityDateIncludingSpotLag, endDate, NULL, NULL, NULL, swapConvention.frequency, swapConvention.daycount.convertToString(),
				swapConvention.slidingRule.convertToString(), swapConvention.paymentCalendar.convertToString(), AQLString("SPLINE"), swapConvention.forecastCurveName, swapConvention.discountCurveName, true);
		}
		resForwards[i] = rate;


		if (swaptionFile != AQ_NO_DATA)
		{
			resVols[i] = AQLMarketData::getVolatilityVal(swaptionFile, optionMaturity, tenor);
		}
			
		// get option maturity term and settle term 
		AQLPriceDataDayCount act_365(ACT_365_ISDA);
		optionMaturities[i] = act_365.getTerm(asOfDate, optionMaturityDate);
		setttleTerms[i] = act_365.getTerm(asOfDate, endDate);
	}

	// store data
	AQLString mainTradeName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
	AQLObject& mainTradeEntity = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED).get();
	if (dynamic_cast<const AQLDataValuation &>(mainTradeEntity.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
			(mainTradeEntity.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			AQLObject& tradeEntity = unders.get(i).get();
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT);
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_SWAPTIONVOL_FORPRINT);
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_OPTION_MATURITY_FORPRINT);
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_SETTLE_TERM_FORPRINT);
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT, new AQLDataDoubles(resForwards));
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_SWAPTIONVOL_FORPRINT, new AQLDataDoubles(resVols));
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_OPTION_MATURITY_FORPRINT, new AQLDataDoubles(optionMaturities));
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_SETTLE_TERM_FORPRINT, new AQLDataDoubles(setttleTerms));
		}
	}
	else
	{
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT);
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_SWAPTIONVOL_FORPRINT);
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_OPTION_MATURITY_FORPRINT);
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_SETTLE_TERM_FORPRINT);
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT, new AQLDataDoubles(resForwards));
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_SWAPTIONVOL_FORPRINT, new AQLDataDoubles(resVols));
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_OPTION_MATURITY_FORPRINT, new AQLDataDoubles(optionMaturities));
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_SETTLE_TERM_FORPRINT, new AQLDataDoubles(setttleTerms));

	}
}



vector<pair<AQLString, vector<AQLObject *> > >
AQLRiskConfigurationVolIRVega::createRiskEntity(AQLObjectPool &objPool) const
{
	vector<pair<AQLString, vector<AQLObject *> > > ps = AQLRiskConfiguration::createRiskEntity(objPool);

	for (auto& p : ps)
	{
		AQLString ccyL = p.first;
		ccyL.toLower();
		const AQLString curveSuffixL = getCurveSuffix(ccyL);

		if (isTrue(getOptionalRiskStaticData(STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISCONVERTED, ccyL, curveSuffixL)))
		{
			const AQLString shift = getOptionalRiskStaticData(STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDFORWARDSHIFTVALUE, ccyL, curveSuffixL);
			const AQLString underlying = getOptionalRiskStaticData(STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDUNDERLYING, ccyL, curveSuffixL);
			const AQLString capletTenor = getOptionalRiskStaticData(STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDCAPLETTENOR, ccyL, curveSuffixL);
			if ((shift.size() == 0) && (underlying.size() == 0) && (capletTenor.size() == 0))
			{
				throw AQLCoreInvalidData(
					"None of a shift value, an underlying nor a caplet tenor is specified for IR vega conversion",
					__FILE__, __LINE__);
			}

			double ratioUpperBound = -1.0;
			{
				const AQLString ratioBoundStr = getOptionalRiskStaticData(
					STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDRATIOUPPERBOUND, ccyL, curveSuffixL);
				if (ratioBoundStr.size() > 0)
				{
					ratioUpperBound = ratioBoundStr.getDoubleValue();
					if (ratioUpperBound < 0.0)
					{
						throw AQLCoreInvalidData(
							(ratioBoundStr + ": the upper bound of IR vega conversion ratio shall be non-negative").getCString(),
							__FILE__, __LINE__);
					}
				}
			}

			std::shared_ptr<UnderlyingInfo> info;
			if (underlying.size() > 0)
			{
				info.reset(new UnderlyingInfo);
				const AQLString suffixL = ("." + underlying).toLower();
				info->mCurveTypeKeys = getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_CURVETYPEKEYS, ccyL, suffixL).toToken(MULTI_STATIC_DATA_DELIMITER);
				info->mCurveTypes = getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_CURVETYPES, ccyL, suffixL).toToken(MULTI_STATIC_DATA_DELIMITER);
				info->mCapSpotLag = getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_CAPSPOTLAG, ccyL, suffixL);
				info->mCapDayCount.convertFromString(getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_CAPDAYCOUNT, ccyL, suffixL));
				info->mCapSlidingRule.convertFromString(getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_CAPSLIDINGRULE, ccyL, suffixL));
				info->mCapPaymentCalendar.convertFromString(getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_CAPPAYMENTCALENDAR, ccyL, suffixL));
				info->mCapFixingCalendar.convertFromString(getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_CAPFIXINGCALENDAR, ccyL, suffixL));
				info->mSwaptionSpotLag = getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONSPOTLAG, ccyL, suffixL);
				info->mSwaptionDayCount.convertFromString(getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONDAYCOUNT, ccyL, suffixL));
				info->mSwaptionSlidingRule.convertFromString(getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONSLIDINGRULE, ccyL, suffixL));
				info->mSwaptionPaymentCalendar.convertFromString(getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONPAYMENTCALENDAR, ccyL, suffixL));
				info->mSwaptionFixingCalendar.convertFromString(getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFIXINGCALENDAR, ccyL, suffixL));
				info->mSwaptionFrequency = getOptionalCalibStaticData(STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFREQUENCY, ccyL, suffixL);
			}

			for (auto& e : p.second)
			{
				if (shift.size() > 0)
				{
					e->add(PRICING_DATA_VEGACONVERTEDSHIFTVAL, new AQLDataDouble(shift.getDoubleValue()));
				}
				if (underlying.size() > 0)
				{
					e->add(PRICING_DATA_VEGACONVERTEDUNDERLYING, new AQLDataString(underlying));
				}
				if (capletTenor.size() > 0)
				{
					e->add(PRICING_DATA_VEGACONVERTEDCAPLETTENOR, new AQLDataString(capletTenor));
				}
				if (ratioUpperBound >= 0.0)
				{
					e->add(PRICING_DATA_VEGACONVERTEDRATIOUPPERBOUND, new AQLDataDouble(ratioUpperBound));
				}
				if (info)
				{
					e->add(PRICING_DATA_VEGACONVERTEDCURVETYPEKEYS, new AQLDataStrings(info->mCurveTypeKeys));
					e->add(PRICING_DATA_VEGACONVERTEDCURVETYPES, new AQLDataStrings(info->mCurveTypes));
					e->add(PRICING_DATA_VEGACONVERTEDCAPSPOTLAG, new AQLDataString(info->mCapSpotLag));
					e->add(PRICING_DATA_VEGACONVERTEDCAPDAYCOUNT, info->mCapDayCount.clone());
					e->add(PRICING_DATA_VEGACONVERTEDCAPSLIDINGRULE, info->mCapSlidingRule.clone());
					e->add(PRICING_DATA_VEGACONVERTEDCAPPAYMENTCALENDAR, info->mCapPaymentCalendar.clone());
					e->add(PRICING_DATA_VEGACONVERTEDCAPFIXINGCALENDAR, info->mCapFixingCalendar.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONSPOTLAG, new AQLDataString(info->mSwaptionSpotLag));
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONDAYCOUNT, info->mSwaptionDayCount.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONSLIDINGRULE, info->mSwaptionSlidingRule.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONPAYMENTCALENDAR, info->mSwaptionPaymentCalendar.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONFIXINGCALENDAR, info->mSwaptionFixingCalendar.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONFREQUENCY, new AQLDataString(info->mSwaptionFrequency));
				}
			}
		}
	}

	return ps;
}
