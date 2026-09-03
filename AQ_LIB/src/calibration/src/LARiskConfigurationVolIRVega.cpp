/*! @file
    @brief  IR Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolIRVega.cpp
//
//  DESCRIPTION :         IR Vega setup class 
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


#include <functional>
#include <algorithm>
#include <memory>
#include "LARiskConfigurationVolIRVega.h"
#include "LAString.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LADataReference.h"
#include "LAPriceDataFunction.h"
#include "LAPriceIRVegaUnderlyingAsset.h"
#include "LAPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADealUtils.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LALinearFunc.h"
#include "LADefinitionsCalibration.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "LADataMatrix.h"
#include "LAFunctionUtilities.h"
#include "LADefinitionsIRSABR.h"
#include "LAMathCurveFuncUtility.h"

using namespace std;

namespace
{
	LAString getOptionalRiskStaticData(const LAString& keyL, const LAString& ccyL, const LAString& curveSuffixL)
	{
		LAString propValue = LAStaticDataManager::getRiskStaticData().getStaticData(ccyL + keyL + curveSuffixL);
		return (propValue == MLIB_NO_DATA) ? "" : std::move(propValue);
	}

	LAString getOptionalCalibStaticData(const LAString& keyL, const LAString& ccyL, const LAString& curveSuffixL)
	{
		LAString propValue = LAStaticDataManager::getCalibStaticData().getStaticData(ccyL + keyL + curveSuffixL);
		return (propValue == MLIB_NO_DATA) ? "" : std::move(propValue);
	}

	bool isTrue(const LAString& s)
	{
		if (s.size() > 0)
		{
			LADataBool attr;
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
		LAStringVector mCurveTypeKeys;
		LAStringVector mCurveTypes;

		LAString mCapSpotLag;
		LAPriceDataDayCount mCapDayCount;
		LAPriceDataSlidingRule mCapSlidingRule;
		LAPriceDataCalendar mCapPaymentCalendar;
		LAPriceDataCalendar mCapFixingCalendar;

		LAString mSwaptionSpotLag;
		LAPriceDataDayCount mSwaptionDayCount;
		LAPriceDataSlidingRule mSwaptionSlidingRule;
		LAPriceDataCalendar mSwaptionPaymentCalendar;
		LAPriceDataCalendar mSwaptionFixingCalendar;
		LAString mSwaptionFrequency;
	};
}

// constructor
/*!

*/
LARiskConfigurationVolIRVega::LARiskConfigurationVolIRVega(void)
: LARiskConfigurationVolatility(false)
{
}

// destructor
/*!

*/
LARiskConfigurationVolIRVega::~LARiskConfigurationVolIRVega(void)
{
}

/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolIRVega::createVolatilityEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{	
	const LAString model = LAMarketData::getModelName(ccy);
 	const LAString riskName = getRiskName();
	LAString inputType = LAMarketData::getVolInputType(model, ccy, riskName);
	LAString bumpDirection = getBumpDirection(ccy);
	inputType.toUpper();
	bumpDirection.toUpper();
	
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}	
	MAScenarioParam param;
	param.ccy = ccy;
	param.targetCurveType = getCurveType(param.ccy);
	param.targetCurveType.toLower();
	param.calcType = ccy + "_" + riskName + "_" + LAString(scenarioNum) + "_" + LAString(index) + getCurveSuffix(ccy);
	param.model = model;
	param.shiftType = getShiftType(ccy);
	param.bumpDirection = getBumpDirection(ccy);
	param.bumpType = getBumpType(ccy);
	param.targetName = LAMarketData::getBaseVolatilityName(ccy);
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
			LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
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
				LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.gridFile[i], true, &param.gridTerm[i]);
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
		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
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
			LAObjectPool &objPool = dataInstance.getObjectPool();
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
			LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
			LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
			delete calibInfoCreator;

			param.refName.push_back(infoName);
			LAString ycName = getScenarioBaseYieldName(ccy);
			param.refName.push_back(ycName);
			//param.refName.push_back(LAMarketData::getBaseYieldName(ccy));

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

			LAObjectPool &objPool = dataInstance.getObjectPool();
			if (param.refName.empty())
			{
				// set refName
				LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
				LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
				delete calibInfoCreator;
				param.refName.push_back(infoName);
			}

			//LAString ycName = LAMarketData::getBaseYieldName(ccy);
			LAString ycName = getScenarioBaseYieldName(ccy);
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
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	
	storeIRAdditionalInfo(dataInstance, param);
	
	return ret;
}


/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolIRVega::createVolatilityEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum) const
{	
	const LAString model = LAMarketData::getModelName(ccy);
 	const LAString riskName = getRiskName();
	LAString inputType = LAMarketData::getVolInputType(model, ccy, riskName);
	LAString bumpDirection = getBumpDirection(ccy);
	inputType.toUpper();
	bumpDirection.toUpper();
	
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}	
	MAScenarioParam param;
	param.ccy = ccy;
	param.calcType = ccy + "_" + riskName + "_" + LAString(scenarioNum);
	param.model = model;
	param.shiftType = getShiftType(ccy);
	param.bumpDirection = getBumpDirection(ccy);
	param.bumpType = getBumpType(ccy);
	param.targetName = LAMarketData::getBaseVolatilityName(ccy);
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
			LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
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
				LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.gridFile[i], true, &param.gridTerm[i]);
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
			LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
			param.refName.push_back(LAMarketData::getBaseYieldName(ccy));

		}
		if (isGridSensitivity(ccy))
		{
			param.isGrid = true;
			// set grid file
			param.gridTerm = getGridTerm(ccy);
			unsigned int size = param.gridTerm.size();
			param.gridFile.resize(size);
			LAString ycName = LAMarketData::getBaseYieldName(ccy);
			for (unsigned int i = 0; i < size; ++i)
			{
				param.gridTerm[i].exchange("_",".");
				LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.gridFile[i], true, &param.gridTerm[i]);
				param.refName.push_back(ycName);
			}
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief setUp targetNames

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationVolIRVega::getTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	const LAString targetName = LAMarketData::getBaseVolatilityName(ccy);

	LAString ret = "";
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
LAStringVector
LARiskConfigurationVolIRVega::getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAStringVector ret(0);
	LAStringVector targetNames = getTargetNames(ccy, dataInstance).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int targetSize = targetNames.size();

	LAStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		LAString fx_targetNames1;
		LAString fx_bVolName = LAMarketData::getBaseVolatilityName(targetFXVec[i]);
		for (unsigned int j = 0; j < targetSize; ++j)
		{
			fx_targetNames1 += fx_bVolName + ":";
		}

		if (fx_targetNames1.size() < 2)
		{
			throw LACoreInvalidData("Extra targetname1 can not set !!", __FILE__, __LINE__);
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
	@return vector<vector<LAObject *> > 
*/
vector<vector<LAObject *> > 
LARiskConfigurationVolIRVega::createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
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
LAStringVector
LARiskConfigurationVolIRVega::getExtraTargetNames2(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return LAStringVector(0);
	}

	return getExtraTargetNames1(ccy, dataInstance);
}

/*!
    @brief create extra senario2 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<LAObject *> > 
*/
vector<vector<LAObject *> > 
LARiskConfigurationVolIRVega::createExtraScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<vector<LAObject *> >(0);
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
	@return vector<vector<LAObject *> > 
*/
vector<vector<LAObject *> > 
LARiskConfigurationVolIRVega::createExtraFXScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	vector<vector<LAObject *> > ret(0);

	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();
	const LAString irCalcType = ccy + "_" + riskName + "_" + LAString(scenarioNum) + "_" + LAString(index);

	LAString riskNameSuffix;
	if (scenarioNum == SCENARIO_1)
	{
		riskNameSuffix = "ExtraScenario1";
	}
	else
	{
		riskNameSuffix = "ExtraScenario2";
	}

	// set parameter for fx vol shift
	LAStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		//vector<LAObject *> sce_fx;
		LAObjectPool &objPool = dataInstance.getObjectPool();
		LAStringVector ccys = targetFXVec[i].toToken(FX_DELIMITER);
		LAString key_fx = targetFXVec[i].toLower();
		const LAString model_fx = LAMarketData::getModelName(key_fx);
		// set up param
		MAScenarioParam param;
		param.ccy = key_fx;
		param.isCalib = true;
        param.calcType=  ccy + "_" + ccys[0] + "_" + ccys[1] + "_" + riskName + "_" + riskNameSuffix + "_" + LAString(index);
		param.model = model_fx;
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
		param.inputType = LAMarketData::getVolInputType(model_fx, key_fx, riskName);
		param.targetName = LAMarketData::getBaseVolatilityName(key_fx);
		
		// dataout
		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}

		LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
		LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, key_fx);
		delete calibInfoCreator;
		
		param.refName.push_back(infoName);

		LAString dBaseYieldName = LAMarketData::getBaseYieldName(ccys[0]);
		LAString dBaseCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dBaseYieldName));
		LAString fBaseYieldName = LAMarketData::getBaseYieldName(ccys[1]);
		LAString fBaseCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, fBaseYieldName));

		ccys[0].toUpper();
		LAString tmpCurrency = ccy;
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
					param.refName.push_back(LAMarketData::getCalibDataName(irCalcType, LAMarketData::getYieldDataName(objPool, dBaseYieldName)));
				}
				else
				{
					param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dBaseYieldName)));
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
					param.refName.push_back(LAMarketData::getCalibDataName(irCalcType, LAMarketData::getYieldDataName(objPool, fBaseYieldName)));
				}
				else
				{
					param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, fBaseYieldName)));
				}
			}
		}
		if (isGridSensitivity(ccy))
		{
			// grid case
			param.isGrid = true;
			LAStringVector gridTerm = getGridTerm(ccy);
			const unsigned int gridSize = gridTerm.size();
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				param.gridFile.push_back(LAStringVector());
				// set reference
				if (ccys[0] == tmpCurrency)
				{
					// set domestic curve name
					param.refName.push_back(dBaseYieldName);
					if (isCalibTarget(ccy))
					{
						param.refName.push_back(LAMarketData::getCalibDataName(irCalcType, LAMarketData::getYieldDataName(objPool, dBaseYieldName), i));
					}
					else
					{
						param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dBaseYieldName)));
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
						param.refName.push_back(LAMarketData::getCalibDataName(irCalcType, LAMarketData::getYieldDataName(objPool, fBaseYieldName), i));
					}
					else
					{
						param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, fBaseYieldName)));
					}
				}
			}
		}
		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<LAObject *> sce_fx = sceCreator->createScenario(dataInstance, param);
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
	@return vector<vector<LAObject *> > 
*/
vector<vector<LAObject *> > 
LARiskConfigurationVolIRVega::createExtraFXScenarioEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	vector<vector<LAObject *> > ret(0);

	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();

	LAString riskNameSuffix;
	if (scenarioNum == SCENARIO_1)
	{
		riskNameSuffix = "ExtraScenario1";
	}
	else
	{
		riskNameSuffix = "ExtraScenario2";
	}

	// set parameter for fx vol shift
	LAStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		vector<LAObject *> sce_fx;
		LAStringVector ccys = targetFXVec[i].toToken(FX_DELIMITER);
		LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
		const LAString model_fx = LAMarketData::getModelName(key_fx);
		// set up param
		MAScenarioParam param;
		param.ccy = key_fx;
		param.model = model_fx;
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
		param.inputType = LAMarketData::getVolInputType(model_fx, key_fx, riskName);
		param.targetName = LAMarketData::getBaseVolatilityName(key_fx);
		param.refName.resize(2);
		// set domestic curve name
		param.refName[0] = LAMarketData::getBaseYieldName(ccys[0]);
		// set foreign curve name
		param.refName[1] = LAMarketData::getBaseYieldName(ccys[1]);
	
		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		ccys[0].toUpper();
		LAString tmpCurrency = ccy;
		tmpCurrency.toUpper();
		bool isParallel = isParallelShift(ccy);
		if (isParallel)
		{
			param.calcType = ccy + "_" + key_fx + "_" + riskName + "_" + riskNameSuffix + "_1_" + LAString(index) + "_Parallel";
			param.isParallel = true;
			LAMarketData::getVolFuncFilePath(model_fx, ccy, key_fx, riskName, scenarioNum, param.paraFile, false);
				
			vector<LAObject *> sce_tmp = sceCreator->createScenario(dataInstance, param);
			if (sce_tmp.size() != 1)
			{
				throw LACoreInvalidData("Parallel shift scenaro size must be one", __FILE__, __LINE__);
			}
			sce_fx.push_back(sce_tmp[0]);
		}
		if (isGridSensitivity(ccy))
		{
			// create as parallel shift
			param.isParallel = true;
			LAStringVector gridTerm = getGridTerm(ccy);
			const unsigned int gridSize = gridTerm.size();
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				gridTerm[i].exchange("_",".");
				param.calcType = ccy + "_" + key_fx + "_" + riskName + "_" + riskNameSuffix + "_1_" + LAString(index) + "_Grid_" + LAString(static_cast<int>(i));

				LAMarketData::getVolFuncFilePath(model_fx, ccy, key_fx, riskName, scenarioNum, param.paraFile, true, &gridTerm[i]);
				vector<LAObject *> sce_tmp = sceCreator->createScenario(dataInstance, param);
				if (sce_tmp.size() != 1)
				{
					throw LACoreInvalidData("Parallel shift scenaro size must be one", __FILE__, __LINE__);
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
LARiskConfigurationVolIRVega::getCoordinatesMatrix(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAStringVector grid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
		STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_COORDINATES + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);
	// day count
	LAPriceDataDayCount dayCount;
	dayCount.convertFromString(LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT));
	// asOfDate
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	const unsigned int COORDINATESNUM = 4;
	const unsigned int size = grid.size();
	DoubleMatrix ret(size);
	for (unsigned int i = 0; i < size; ++i)
	{
		LAStringVector cdns = grid[i].toToken('_');
		if (cdns.size() != COORDINATESNUM)
		{
			throw LACoreInvalidData(" IR vega coordinate grid format is wrong .", __FILE__, __LINE__);
		}
		ret[i].resize(COORDINATESNUM);
		for (unsigned  int j = 0; j < COORDINATESNUM; ++j)
		{
			ret[i][j] = dayCount.getTerm(asOfDate, LAMathDateCalculations::getDate(asOfDate, cdns[j], true));
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
LARiskConfigurationVolIRVega::getFileValMatrix(const LAString &ccy) const
{
	
	LAString tmpCurrency = ccy;
	LAString strMatrixFile = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SHIFTVAL_FILE + getCurveSuffix(ccy));
	MAFileAccessor matrixFile(LAMarketData::getNumFileName(strMatrixFile));
	LAStringMatrix f_dataMatrix;
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
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationVolIRVega::getCoordinates(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
		STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_COORDINATES + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);

}

/*!
    @brief return bump type

	@param[in] ccy
	@return string
*/
LAString
LARiskConfigurationVolIRVega::getBumpType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString gridType = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_TYPE + getCurveSuffix(ccy));
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
		LAString msg = "This grid type is not supported by bumpType. grid type = " + gridType;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}


/*!
    @brief return grid type

	@param[in] ccy
	@return string
*/
LAString
LARiskConfigurationVolIRVega::getGridType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_TYPE + getCurveSuffix(ccy));
}


/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationVolIRVega::getGridTerm(const LAString &ccy) const
{
	LAString gridType = getGridType(ccy);
	gridType.toUpper();

	if (gridType == RISK_GRID_MARKET)
	{
		LAString tmpCurrency = ccy;
		//LAString underlying = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_VOL_UNDERLYING);

		/*if (underlying == MLIB_NO_DATA)
		{*/
		return getMarketTerm(ccy);
		/*}
		else
		{
			LAStringVector gridTerm = getMarketTerm(ccy);
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
		LAString msg = "This grid type is not supported. grid type = " + gridType;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationVolIRVega::getBucketGridTerm(const LAString &ccy) const
{
	ccy;
	vector<LAString> ret;
	ret.push_back(MLIB_NO_DATA);
	return ret;
	
}

/*!
    @brief return market  term

	@param[in] ccy
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationVolIRVega::getMarketTerm(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;

	LAStringVector mktTems_prop = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
			STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_MARKET + getCurveSuffix(ccy)).toToken(':');

	return reduceTargetGrids(ccy, mktTems_prop);
}

/*!
    @brief reduce market grids in SABR model

	@param[in] ccy
	@param[in] market grids
	@return vector<LAString>
*/
LAStringVector 
LARiskConfigurationVolIRVega::reduceTargetGrids(const LAString &ccy, const LAStringVector &mktGrids) const
{
	LAString tmpCurrency = ccy;
	tmpCurrency.toLower();
	LAString model = LAMarketData::getModelName(ccy);
	model.toUpper();

	if (model == MODEL_IRSABR)
	{
		// calibration target flag
		LAString calibTarget_str = LACoreDataService::getContext(tmpCurrency + "." CONTEXT_KEY_DEAL_IRVOL + getCurveSuffix(ccy));
		if (calibTarget_str == MLIB_NO_DATA)
			return  mktGrids;

		LADataBoolMatrix tmp;
		tmp.convertFromString(calibTarget_str);
		BoolMatrix calibTarget = tmp.get();
		// optionmaturity
		LAStringVector optionMatVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_OPTIONMATURITY + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);
		uppervec(optionMatVec);
		// swapterm
		LAStringVector tenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTENOR + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);
		uppervec(tenorVec);
		// check size
		if (calibTarget.size() == 0)
			throw LACoreInvalidData("Calibration flag matrix has no data!",	__FILE__,__LINE__);
		if (calibTarget.size() != optionMatVec.size() || calibTarget[0].size() != tenorVec.size())
			throw LACoreInvalidData("Calibration flag matrix does not match option maturity or tenor!",	__FILE__,__LINE__);
		// make map
		std::map<LAString, bool> map_calib_target;
		for(size_t i = 0; i < optionMatVec.size(); ++i)
		{
			for(size_t j = 0; j < tenorVec.size(); ++j)
			{
				map_calib_target[optionMatVec[i] + "_" + tenorVec[j]] = calibTarget[i][j];
			}
		}

		// sort 1st : expiry descending, 2nd : tenor ascending
		std::map<std::pair<int, int>, LAString> targetGrids;
		for(size_t i = 0; i < mktGrids.size(); ++i)
		{
			LAString tmpTerm = mktGrids[i];
			tmpTerm.toUpper();
			LAStringVector termVec = tmpTerm.toToken('_');
			if (termVec.size() != 3)
				throw LACoreInvalidData("Format of market grid is wrong!",	__FILE__,__LINE__);
			if (map_calib_target[termVec[1] + "_" + termVec[2]]){
				int y, m, d, w;
				LAMathDateCalculations::termStrtoYMDW(termVec[1], y, m, d, w);
				const int expiryMonth = y * 12 + m;
				LAMathDateCalculations::termStrtoYMDW(termVec[2], y, m, d, w);
				const int tenorMonth = y * 12 + m;
				targetGrids[std::make_pair(-expiryMonth, tenorMonth)] = mktGrids[i];
			}
		}
		LAStringVector ret;
		for(std::map<std::pair<int, int>, LAString>::const_iterator i = targetGrids.begin(); i != targetGrids.end(); ++i){
			ret.push_back(i->second);
		}

		/*if (ret.size() == 0)
			throw LACoreInvalidData("The number of risk target grids is 0!",	__FILE__,__LINE__);*/

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
	@return LAString
*/
LAString
LARiskConfigurationVolIRVega::getOutPutName1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationVolIRVega::getRiskName(void) const
{
	return RISK_FRONT_VOL_IRVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolIRVega::isGridSensitivity(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationVolIRVega::isParallelShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISPARALLEL + getCurveSuffix(ccy)));
}

/*!
    @brief return shiftvalfileuse

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationVolIRVega::isShiftValFileUse(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISFILEUSE + getCurveSuffix(ccy)));
}

/*!
    @brief return div unit value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationVolIRVega::getDivUnit(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_DIVUNIT + getCurveSuffix(ccy)).getDoubleValue();
}


/*!
    @brief return scenario1 value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationVolIRVega::getScenario1ShiftValue(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
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
LARiskConfigurationVolIRVega::getScenario2ShiftValue(const LAString &ccy) const
{
	return getScenario1ShiftValue(ccy);
}

/*!
    @brief return shift type

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationVolIRVega::getShiftType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SHIFTTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationVolIRVega::getBumpDirection(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_BUMPDIRECTION + getCurveSuffix(ccy));

}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolIRVega::isWave(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief  return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationVolIRVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRVEGA_TARGET_CURRENCY);
}


/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationVolIRVega::getCalibTargetCurrencies() const
{
	LAString targetFX = mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRVEGA_CALIBRATION_TARGET_FX).toUpper();
	if (targetFX == "ALL")
	{
		return "ALL";
	}
	LAString ret;
	LAStringVector ccys = MADealUtils::getSDECurrencys();
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
		throw LACoreInvalidData("SDE currency is does not exist", __FILE__, __LINE__);
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
LARiskConfigurationVolIRVega::isRiskCurrencyMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISRISKCURRENCYMODE + getCurveSuffix(ccy));
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}


/*!
    @brief get calib target fx

	@param ccy
	get calibration target fx.
	if ccy is not calibtarget, fx is not calibrated.	

*/
LAStringVector
LARiskConfigurationVolIRVega::getCalibTargetFX(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAStringVector fCurveCcys;
	/*if (ccy.findString(FX_DELIMITER) < 0 )
	{
		LAObjectPool &objPool = dataInstance.getObjectPool();
		LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());
		fCurveCcys = ycPro.getAffectingCcy();
	}*/

	LAStringVector ret;
	//for nocalib
	LAString tmpCurrency = ccy;
	tmpCurrency.toUpper();
	LAStringVector simccys = MADealUtils::getSimulationSDECurrencys();
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
		LAStringVector ccys = MADealUtils::getSDECurrencys();
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

@return LAString
*/
LAString
LARiskConfigurationVolIRVega::getVegaType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString vegaType = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_VEGATYPE + getCurveSuffix(ccy));
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
LARiskConfigurationVolIRVega::storeIRAdditionalInfo(LADataInstance &dataInstance, const MAScenarioParam& param) const
{
	LAObjectPool& objPool = dataInstance.getObjectPool();
	const LAString& riskName = param.calcType;
	LAStringVector riskNameVector = riskName.toToken('_');

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
	LAString swaptionFile = LAMarketData::getVolFileName(param.model, param.ccy, param.targetCurveType);

	for (size_t i = 0; i < gridSize; ++i)
	{
		unsigned int curPos = 1 + i + (param.isParallel ? 1 : 0);
		const LAString curveName = param.refName[curPos];

		LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());


		const LAString optionMaturity = param.gridTerm[i].toToken('_')[1].toUpper(); // convert : Ccy_xM_yM => xM.
		const LAString tenor = param.gridTerm[i].toToken('_')[2].toUpper(); // convert : Ccy_xM_yM => yM.
		if (optionMaturity.toToken('-').size() > 1 || tenor.toToken('-').size() > 1)
		{
			// This is backet risk case. We skip calculation
			return;
		}

		MASwapConvention swapConvention = LAMarketData::getSwapConvention(objPool, optionMaturity, tenor, curveName, param);

		//calc swap rate
		LAPriceDataSlidingRule slr_Fol; slr_Fol.convertFromString(FOL);

		LADate optionMaturityDate = LAMathDateCalculations::getDate(asOfDate, optionMaturity, swapConvention.slidingRule, &swapConvention.fixingCalendar, true);
		LADate optionMaturityDateIncludingSpotLag = LAMathDateCalculations::getDate(optionMaturityDate, swapConvention.spotLag, slr_Fol, &swapConvention.fixingCalendar, true);
		LADate endDate = LAMathDateCalculations::getDate(optionMaturityDateIncludingSpotLag, tenor, swapConvention.slidingRule, &swapConvention.paymentCalendar, true);

		DateVector dates;
		double rate;
		if (swapConvention.frequency == SIMPLE)
		{
			dates.push_back(optionMaturityDateIncludingSpotLag);
			dates.push_back(endDate);
			rate = LAMathCurveFuncUtility::getParRate(dates, &dataInstance, swapConvention.curveID, swapConvention.daycount.convertToString(), LAString("SPLINE"), swapConvention.forecastCurveName, swapConvention.discountCurveName, true);
		}
		else
		{
			rate = LAMathCurveFuncUtility::getParRate(&dataInstance, swapConvention.curveID, optionMaturityDateIncludingSpotLag, endDate, NULL, NULL, NULL, swapConvention.frequency, swapConvention.daycount.convertToString(),
				swapConvention.slidingRule.convertToString(), swapConvention.paymentCalendar.convertToString(), LAString("SPLINE"), swapConvention.forecastCurveName, swapConvention.discountCurveName, true);
		}
		resForwards[i] = rate;


		if (swaptionFile != MLIB_NO_DATA)
		{
			resVols[i] = LAMarketData::getVolatilityVal(swaptionFile, optionMaturity, tenor);
		}
			
		// get option maturity term and settle term 
		LAPriceDataDayCount act_365(ACT_365_ISDA);
		optionMaturities[i] = act_365.getTerm(asOfDate, optionMaturityDate);
		setttleTerms[i] = act_365.getTerm(asOfDate, endDate);
	}

	// store data
	LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObject& mainTradeEntity = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED).get();
	if (dynamic_cast<const LADataValuation &>(mainTradeEntity.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
			(mainTradeEntity.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			LAObject& tradeEntity = unders.get(i).get();
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT);
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_SWAPTIONVOL_FORPRINT);
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_OPTION_MATURITY_FORPRINT);
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_SETTLE_TERM_FORPRINT);
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT, new LADataDoubles(resForwards));
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_SWAPTIONVOL_FORPRINT, new LADataDoubles(resVols));
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_OPTION_MATURITY_FORPRINT, new LADataDoubles(optionMaturities));
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_SETTLE_TERM_FORPRINT, new LADataDoubles(setttleTerms));
		}
	}
	else
	{
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT);
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_SWAPTIONVOL_FORPRINT);
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_OPTION_MATURITY_FORPRINT);
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_SETTLE_TERM_FORPRINT);
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT, new LADataDoubles(resForwards));
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_SWAPTIONVOL_FORPRINT, new LADataDoubles(resVols));
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_OPTION_MATURITY_FORPRINT, new LADataDoubles(optionMaturities));
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_SETTLE_TERM_FORPRINT, new LADataDoubles(setttleTerms));

	}
}



vector<pair<LAString, vector<LAObject *> > >
LARiskConfigurationVolIRVega::createRiskEntity(LAObjectPool &objPool) const
{
	vector<pair<LAString, vector<LAObject *> > > ps = LARiskConfiguration::createRiskEntity(objPool);

	for (auto& p : ps)
	{
		LAString ccyL = p.first;
		ccyL.toLower();
		const LAString curveSuffixL = getCurveSuffix(ccyL);

		if (isTrue(getOptionalRiskStaticData(STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISCONVERTED, ccyL, curveSuffixL)))
		{
			const LAString shift = getOptionalRiskStaticData(STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDFORWARDSHIFTVALUE, ccyL, curveSuffixL);
			const LAString underlying = getOptionalRiskStaticData(STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDUNDERLYING, ccyL, curveSuffixL);
			const LAString capletTenor = getOptionalRiskStaticData(STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDCAPLETTENOR, ccyL, curveSuffixL);
			if ((shift.size() == 0) && (underlying.size() == 0) && (capletTenor.size() == 0))
			{
				throw LACoreInvalidData(
					"None of a shift value, an underlying nor a caplet tenor is specified for IR vega conversion",
					__FILE__, __LINE__);
			}

			double ratioUpperBound = -1.0;
			{
				const LAString ratioBoundStr = getOptionalRiskStaticData(
					STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDRATIOUPPERBOUND, ccyL, curveSuffixL);
				if (ratioBoundStr.size() > 0)
				{
					ratioUpperBound = ratioBoundStr.getDoubleValue();
					if (ratioUpperBound < 0.0)
					{
						throw LACoreInvalidData(
							(ratioBoundStr + ": the upper bound of IR vega conversion ratio shall be non-negative").getCString(),
							__FILE__, __LINE__);
					}
				}
			}

			std::shared_ptr<UnderlyingInfo> info;
			if (underlying.size() > 0)
			{
				info.reset(new UnderlyingInfo);
				const LAString suffixL = ("." + underlying).toLower();
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
					e->add(PRICING_DATA_VEGACONVERTEDSHIFTVAL, new LADataDouble(shift.getDoubleValue()));
				}
				if (underlying.size() > 0)
				{
					e->add(PRICING_DATA_VEGACONVERTEDUNDERLYING, new LADataString(underlying));
				}
				if (capletTenor.size() > 0)
				{
					e->add(PRICING_DATA_VEGACONVERTEDCAPLETTENOR, new LADataString(capletTenor));
				}
				if (ratioUpperBound >= 0.0)
				{
					e->add(PRICING_DATA_VEGACONVERTEDRATIOUPPERBOUND, new LADataDouble(ratioUpperBound));
				}
				if (info)
				{
					e->add(PRICING_DATA_VEGACONVERTEDCURVETYPEKEYS, new LADataStrings(info->mCurveTypeKeys));
					e->add(PRICING_DATA_VEGACONVERTEDCURVETYPES, new LADataStrings(info->mCurveTypes));
					e->add(PRICING_DATA_VEGACONVERTEDCAPSPOTLAG, new LADataString(info->mCapSpotLag));
					e->add(PRICING_DATA_VEGACONVERTEDCAPDAYCOUNT, info->mCapDayCount.clone());
					e->add(PRICING_DATA_VEGACONVERTEDCAPSLIDINGRULE, info->mCapSlidingRule.clone());
					e->add(PRICING_DATA_VEGACONVERTEDCAPPAYMENTCALENDAR, info->mCapPaymentCalendar.clone());
					e->add(PRICING_DATA_VEGACONVERTEDCAPFIXINGCALENDAR, info->mCapFixingCalendar.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONSPOTLAG, new LADataString(info->mSwaptionSpotLag));
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONDAYCOUNT, info->mSwaptionDayCount.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONSLIDINGRULE, info->mSwaptionSlidingRule.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONPAYMENTCALENDAR, info->mSwaptionPaymentCalendar.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONFIXINGCALENDAR, info->mSwaptionFixingCalendar.clone());
					e->add(PRICING_DATA_VEGACONVERTEDSWAPTIONFREQUENCY, new LADataString(info->mSwaptionFrequency));
				}
			}
		}
	}

	return ps;
}
