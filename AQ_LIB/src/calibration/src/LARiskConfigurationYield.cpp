#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LARiskConfigurationYield.h"
#include "LADataInstance.h"
#include "LAObjectPool.h"
#include "LADataReference.h"
#include "LAPricePortfolioValue.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LAMathDateCalculations.h"
#include "LADefinitionsRisk.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LALogger.h"
#include "LALogManager.h"
#include "LAStaticData.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "LADefinitionsCalibration.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationYield::LARiskConfigurationYield(void)
:LARiskConfiguration(false)
{
}

// destructor
/*!

*/
LARiskConfigurationYield::~LARiskConfigurationYield(void)
{
}

 
/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYield::createScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	return createYieldEntity(ccy, dataInstance, SCENARIO_1, index);
}

/*!
    @brief create scenario2 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationYield::createScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	return createYieldEntity(ccy, dataInstance, SCENARIO_2, index);
}


/*!
    @brief get targetNames

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYield::getTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	unsigned int gridIndex = getMaxGridIndex(ccy);
	const LAString yieldName = LAMarketData::getBaseYieldName(ccy);

	LAString ret;
	if (isParallelShift(ccy))
	{
		ret += yieldName + ":";
	}

	if (isGridSensitivity(ccy))
	{
		LAStringVector bucketterm = getBucketGridTerm(ccy);
		if (bucketterm[0] != AQ_NO_DATA)
			gridIndex = bucketterm.size() - 1;
		for (unsigned int i = 0; i <= gridIndex; ++i)
		{
			ret += yieldName + ":";
		}
	}

	if (ret.size() < 2)
	{
		throw LACoreInvalidData("Grid nor parallel is not set. can not create targetnames!!", __FILE__, __LINE__); 
	}
	return ret.subString(0, ret.size() - 2);
}

/*!
    @brief get max grid index

	max grid index for calc

	@param[in] ccy
	@return int
*/
int 
LARiskConfigurationYield::getMaxGridIndex(const LAString &ccy) const
{
	const int dealMax = LACoreDataService::getContext(CONTEXT_KEY_DEAL_MAXTERM).getIntValue();

	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	LAPriceDataDayCount dayCount(ACT_365_ISDA);

	LAStringVector grids = getShiftGridTerm(ccy);
	const unsigned int gSize = grids.size();
	if (gSize < 1)
	{
		throw LACoreInvalidData("Grid size must be more than zero.", __FILE__, __LINE__);
	}
	int ret = gSize - 1;
	for (unsigned int i = 0; i < gSize; ++i)
	{
		grids[i].toUpper();
		// remove prefix
		int pre_pos = grids[i].findString("_");
		if (pre_pos >= 0)
		{
			grids[i] = grids[i].subString(pre_pos + 1, grids[i].size() - 1);
		}
		if (grids[i].findString("ED") != -1) 
		{
			LADate date = LAMathDateCalculations::getIMMDateFromTerm(asOfDate, grids[i]);
			date.addMonths(3);
			
			int term = asOfDate.intervalYears(date);

			if (term > dealMax)
			{
				ret = i;
				break;
			}
		}
		else if (grids[i].findString("FF") != -1) 
		{
			continue;
		}
		else if (grids[i].findString("D") > 0 || 
			grids[i].findString("M") > 0  || 
			grids[i].findString("Y") > 0)
		{
			LADate date = LAMathDateCalculations::getDate(asOfDate, grids[i], true);
			int term = asOfDate.intervalYears(date);

			if (term > dealMax)
			{
				ret = i;
				break;
			}
		}
	}
	// add buffer
	ret += getGridCalcBuffer();
	if (ret < static_cast<int>(gSize - 1))
	{
		return ret;
	}
	else
	{
		return gSize - 1;
	}
}
/*!
    @brief return extra scenario1 target names

	@param[in] ccy
	@return DoubleArray
*/
LAStringVector
LARiskConfigurationYield::getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const
{
	if (isZeroBump(ccy))
	{
		return LARiskConfiguration::getExtraTargetNames1(ccy, dataInstance);
	}
	LAStringVector ret(0);
	LAStringVector targetNames = getTargetNames(ccy, dataInstance).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int size = targetNames.size();

	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());

	// get affecting ccys
	const LAStringVector &fCurveCcys = ycPro.getAffectingCcy();
	// get col affecting ccys
	StringSet cCurveCcys;
	LAStringVector tmpCcys = ycPro.getColAffectingCcy();
	cCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	for (int i = 0; i < fCurveCcys.size(); ++i)
	{
		const LAMathYieldCurvePro &fYcPro = dynamic_cast<LAMathYieldCurvePro &>
							(objPool.getObject(LAMarketData::getBaseYieldProName(fCurveCcys[i]), ENCHKTYPE_ISDEFINED).get());
		tmpCcys = fYcPro.getColAffectingCcy();
		cCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	}
	// get affecting ccys of col affecting ccys
	StringSet cfCurveCcys;
	for (StringSet::const_iterator it = cCurveCcys.begin(); it != cCurveCcys.end(); ++it)
	{
		const LAMathYieldCurvePro &cYcPro = dynamic_cast<LAMathYieldCurvePro &>
									(objPool.getObject(LAMarketData::getBaseYieldProName(*it), ENCHKTYPE_ISDEFINED).get());
		tmpCcys = cYcPro.getAffectingCcy();
		cfCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	}
	cfCurveCcys.erase(ccy);
	// get all affecting ccys
	StringSet curveCcys;
	curveCcys.insert(fCurveCcys.begin(), fCurveCcys.end());
	curveCcys.insert(cCurveCcys.begin(), cCurveCcys.end());
	curveCcys.insert(cfCurveCcys.begin(), cfCurveCcys.end());
	curveCcys.erase(ccy);

	for (StringSet::const_iterator it = curveCcys.begin(); it != curveCcys.end(); ++it)
	{
		const LAString yieldName = LAMarketData::getBaseYieldName(*it);
		LAString ccy_fYldTargetNames;
		for (unsigned int j = 0; j < size; ++j)
		{
			ccy_fYldTargetNames += yieldName + ":";
		}
		ret.push_back(ccy_fYldTargetNames.subString(0, ccy_fYldTargetNames.size() - 2));
	}
	for (StringSet::const_iterator it = curveCcys.begin(); it != curveCcys.end(); ++it)
	{
		// check calibration target
		if (isCalibTarget(*it))
		{
			LAString ccy_fTargetName;
			LAString ccy_fBVolName = LAMarketData::getBaseVolatilityName(*it);
			for (unsigned int j = 0; j < size; ++j)
			{
				ccy_fTargetName += ccy_fBVolName + ":";
			}
			if (ccy_fTargetName.size() < 2)
			{
				throw LACoreInvalidData("Extra targetname1 can not set !!", __FILE__, __LINE__);
			}
			ret.push_back(ccy_fTargetName.subString(0, ccy_fTargetName.size() - 2));
		}
	}
	// check calibration target
	if (isCalibTarget(ccy))
	{
		LAString ccy_targetNames1;
		LAString ccy_bVolName = LAMarketData::getBaseVolatilityName(ccy);
		for (unsigned int i = 0; i < size; ++i)
		{
			ccy_targetNames1 += ccy_bVolName + ":";
		}

		if (ccy_targetNames1.size() < 2)
		{
			throw LACoreInvalidData("Extra targetname1 can not set !!", __FILE__, __LINE__);
		}
		ret.push_back(ccy_targetNames1.subString(0, ccy_targetNames1.size() - 2));
	}
	LAStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		// if cross check fx is target
		LAStringVector ccys = targetFXVec[i].toToken(FX_DELIMITER);
		LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
		LAString fx_targetNames1;
		LAString fx_bVolName = LAMarketData::getBaseVolatilityName(key_fx);
		for (unsigned int i = 0; i < size; ++i)
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
LARiskConfigurationYield::createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	if (isZeroBump(ccy))
	{
		return LARiskConfiguration::createExtraScenario1Entity(ccy, dataInstance, index);
	}
	if (isRealCalib())
	{
		return createExtraScenarioEntity(ccy, dataInstance, SCENARIO_1, index);
	}
	else
	{
		return createExtraScenarioEntityOld(ccy, dataInstance, SCENARIO_1, index);
	}
}

/*!
    @brief return extra scenario2 target names

	@param[in] ccy
	@return DoubleArray
*/
LAStringVector
LARiskConfigurationYield::getExtraTargetNames2(const LAString &ccy, LADataInstance &dataInstance) const
{
	if (isZeroBump(ccy))
	{
		return LARiskConfiguration::getExtraTargetNames2(ccy, dataInstance);
	}
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
LARiskConfigurationYield::createExtraScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	if (isZeroBump(ccy))
	{
		return LARiskConfiguration::createExtraScenario2Entity(ccy, dataInstance, index);
	}
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<vector<LAObject *> >(0);
	}

	if (isRealCalib())
	{
		return createExtraScenarioEntity(ccy, dataInstance, SCENARIO_2, index);
	}
	else
	{
		return createExtraScenarioEntityOld(ccy, dataInstance, SCENARIO_2, index);
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
LARiskConfigurationYield::createExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	vector<vector<LAObject *> > ret(0);

	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());

	map<LAString, vector<LAObject *> > scemap;
	vector<LAObject *> sce;

	// get affecting ccys
	const LAStringVector& fCurveCcys = ycPro.getAffectingCcy();
	// get col affecting ccys
	StringSet cCurveCcys;
	StringSet cfCurveCcys;
	LAStringVector tmpCcys = ycPro.getColAffectingCcy();
	cCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	for (int unsigned i = 0; i < fCurveCcys.size(); ++i)
	{
		// calc affecting curves
		sce = createForeignYieldEntity(ccy, fCurveCcys[i], dataInstance, scenarioNum, index);
		if (sce.size() > 0)
			scemap[fCurveCcys[i]] = sce;
		else
			cfCurveCcys.insert(fCurveCcys[i]);
		// get col affecting ccys
		const LAMathYieldCurvePro &fYcPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(fCurveCcys[i]), ENCHKTYPE_ISDEFINED).get());
		tmpCcys = fYcPro.getColAffectingCcy();
		cCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	}
	for (StringSet::const_iterator it = cCurveCcys.begin(); it != cCurveCcys.end(); ++it)
	{
		// calc affecting fwdfx const curves
		sce = createCollateralYieldEntity(ccy, *it, dataInstance, scenarioNum, index);
		if (sce.size() > 0)
			scemap[*it] = sce;
		// get affecting ccys of col affecting ccys
		const LAMathYieldCurvePro &cYcPro = dynamic_cast<LAMathYieldCurvePro &>
									(objPool.getObject(LAMarketData::getBaseYieldProName(*it), ENCHKTYPE_ISDEFINED).get());
		tmpCcys = cYcPro.getAffectingCcy();
		cfCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	}
	cfCurveCcys.erase(ccy);
	const LAString usd = "USD";
	if (cfCurveCcys.find(usd) != cfCurveCcys.end())
	{
		// calc usd curves first
		sce = createForeignYieldEntity(ccy, usd, dataInstance, scenarioNum, index, false);
		if (sce.size() > 0)
			scemap[usd] = sce;
	}
	for (StringSet::const_iterator it = cfCurveCcys.begin(); it != cfCurveCcys.end(); ++it)
	{
		if (*it == usd)
			continue;
		// calc affecting curves of fwdfx const curves
		sce = createForeignYieldEntity(ccy, *it, dataInstance, scenarioNum, index, false);
		if (sce.size() > 0)
			scemap[*it] = sce;
	}
	// get all affecting ccys
	StringSet curveCcys;
	curveCcys.insert(fCurveCcys.begin(), fCurveCcys.end());
	curveCcys.insert(cCurveCcys.begin(), cCurveCcys.end());
	curveCcys.insert(cfCurveCcys.begin(), cfCurveCcys.end());
	curveCcys.erase(ccy);

	// set in object pool
	for (map<LAString, vector<LAObject *> >::const_iterator it = scemap.begin(); it != scemap.end(); ++it)
	{
		sce = it->second;
		ret.push_back(sce);
		for (int unsigned j = 0; j < sce.size(); ++j)
		{
			const LAString &name = dynamic_cast<const LADataString &>(sce[j]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			sce[j]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
			sce[j]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new LADataBool(true));
			LAObjectHolder objHolder = objPool.getObject(name,  ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
				objPool.set(name, sce[j]);
		}
	}
	scemap.clear();

	for (set<LAString>::const_iterator it = curveCcys.begin(); it != curveCcys.end(); ++it)
	{
		if (isCalibTarget(*it))
		{
			sce = createForeignIRVolEntity(ccy, *it, dataInstance, scenarioNum, index);
			ret.push_back(sce);
		}
	}		
	if (isCalibTarget(ccy))
	{
		sce = createIRVolEntity(ccy, dataInstance, scenarioNum, index);
		ret.push_back(sce);
	}

	// set parameter for fx vol shift
	LAStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	// scenario check
	if (fxSize > 0 && mSce1Names.empty() && mSce2Names.empty())
	{
		throw LACoreInvalidData("Scenario entities have not been created yet", __FILE__, __LINE__);
	}
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		sce = createFXVolEntity(ccy, targetFXVec[i], dataInstance, scenarioNum, index);
		ret.push_back(sce);
	}
	return ret;
}

/*!
    @brief create extra object(oldversion not calibrate)

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<vector<LAObject *> > 
*/
vector<vector<LAObject *> > 
LARiskConfigurationYield::createExtraScenarioEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	vector<vector<LAObject *> > ret(0);

	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();

	LAString riskNameSuffix;
	LAStringVector sceNames;
	if (scenarioNum == SCENARIO_1)
	{
		riskNameSuffix = "ExtraScenario1";
		sceNames = mSce1Names;
	}
	else
	{
		riskNameSuffix = "ExtraScenario2";
		sceNames = mSce2Names;
	}

	LAString bShiftStr = getBaseShiftStr(ccy, index); // base shirt value(string)
	if (isCalibTarget(ccy))
	{
		// set up param for volatility shift
		MAScenarioParam param;
		param.ccy = ccy;
		param.calcType = ccy + "_" + riskName + "_" + riskNameSuffix + "_1_" + LAString(index);
		param.model = model;
		param.inputType = LAMarketData::getVolInputType(model, ccy, riskName);
		param.targetName = LAMarketData::getBaseVolatilityName(ccy);

		bool isParallel = false;
		if (isParallelShift(ccy))
		{
			param.isParallel = true;
			isParallel = true;
			LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false, 0, &bShiftStr);
			param.refName.push_back(sceNames[0]);
		}

		if (isGridSensitivity(ccy))
		{
			param.isGrid = true;
			LAStringVector gridTerm = getGridTerm(ccy);   //M-Lib type grid
			LAStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
			unsigned int gridSize = gridTerm.size();
			const unsigned int maxIndex = getMaxGridIndex(ccy);
			unsigned int refAddIndex = 0;

			LAStringVector BucketTerm = getBucketGridTerm(ccy);
			if (BucketTerm[0] != AQ_NO_DATA)
			{
				gridSize = BucketTerm.size();
			}

			if (isParallel)
			{
				refAddIndex = 1;
			}

			for (unsigned int i = 0; i < gridSize; ++i)
			{
				if (i > maxIndex)
				{
					break;
				}
				int pre_pos = gridTerm[i].findString("_");
				if (pre_pos >= 0)
				{
					gridTerm[i] = gridTerm[i].subString(pre_pos + 1, gridTerm[i].size() - 1);
				}
				// search market grid pos
				unsigned int m_pos;
				if (!LAMarketData::searchMarketGridPos(marketGridTerm, gridTerm[i], m_pos))
				{
					LAString msg = "Market grid is not contain " + gridTerm[i];
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				LAStringVector filePath;
				LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, filePath, true, &marketGridTerm[m_pos], &bShiftStr);
				param.gridFile.push_back(filePath);
				param.refName.push_back(sceNames[i + refAddIndex]);
			}
		}

		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<LAObject *> sce = sceCreator->createScenario(dataInstance, param);
		delete sceCreator;

		ret.push_back(sce);

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
		
		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
		// scenario check
		if (sceNames.empty())
		{
			throw LACoreInvalidData("Scenario object is not created yet", __FILE__, __LINE__);
		}

		ccys[0].toUpper();
		LAString tmpCurrency = ccy;
		tmpCurrency.toUpper();
		bool isParallel = isParallelShift(ccy);
		if (isParallel)
		{
			param.calcType = ccy + "_" + key_fx + "_" + riskName + "_" + riskNameSuffix + "_2_" + LAString(index) + "_Parallel";
			param.isParallel = true;
			LAMarketData::getVolFuncFilePath(model_fx, ccy, key_fx, riskName, scenarioNum, param.paraFile, false, 0, &bShiftStr);
			param.refName.resize(2);
			// set reference
			if (ccys[0] == tmpCurrency)
			{
				// set domestic curve name
				param.refName[0] = sceNames[0];
				// set foreign curve name
				param.refName[1] = LAMarketData::getBaseYieldName(ccys[1]);
			}
			else
			{
				// set domestic curve name
				param.refName[0] = LAMarketData::getBaseYieldName(ccys[0]);
				// set foreign curve name
				param.refName[1] = sceNames[0];
			}
				
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
			param.refName.resize(2);
			LAStringVector gridTerm = getGridTerm(ccy);
			LAStringVector marketGridTerm = getShiftGridTerm(ccy);
			const unsigned int gridSize = gridTerm.size();
			const unsigned int maxIndex = getMaxGridIndex(ccy);
			unsigned int addIndex = 0;
			if (isParallel)
			{
				addIndex = 1;
			}
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				param.calcType = ccy + "_" + key_fx + "_" + riskName + "_" + riskNameSuffix + "_2_" + LAString(index) + "_Grid_" + LAString(static_cast<int>(i));
				if (i > maxIndex)
				{
					break;
				}
				int pre_pos = gridTerm[i].findString("_");
				if (pre_pos >= 0)
				{
					gridTerm[i] = gridTerm[i].subString(pre_pos + 1, gridTerm[i].size() - 1);
				}
				// search market grid pos
				unsigned int m_pos;
				if (!LAMarketData::searchMarketGridPos(marketGridTerm, gridTerm[i], m_pos))
				{
					LAString msg = "Market grid is not contain " + gridTerm[i];
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				// set reference
				if (ccys[0] == tmpCurrency)
				{
					// set domestic curve name
					param.refName[0] = sceNames[i + addIndex];
					// set foreign curve name
					param.refName[1] = LAMarketData::getBaseYieldName(ccys[1]);
				}
				else
				{
					// set domestic curve name
					param.refName[0] = LAMarketData::getBaseYieldName(ccys[0]);
					// set foreign curve name
					param.refName[1] = sceNames[i + addIndex];
				}

				LAMarketData::getVolFuncFilePath(model_fx, ccy, key_fx, riskName, scenarioNum, param.paraFile, true, &marketGridTerm[m_pos], &bShiftStr);
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
    @brief create ir vol scenarilo object

	@param[in] ccy
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYield::createIRVolEntity(const LAString& ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();

	// set up param for volatility shift
	MAScenarioParam param;
	param.isCalib = true;
	param.ccy = ccy;
	param.calcType = getExtraCalcType(ccy, scenarioNum, index);
	param.targetCurveType = getCurveType(ccy);
	param.model = model;
	param.inputType = LAMarketData::getVolInputType(model, ccy, riskName);
	param.targetName = LAMarketData::getBaseVolatilityName(ccy);

	// dataout
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else 
	{
		param.isOutPut = false;
	}

	LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
	delete calibInfoCreator;

	param.refName.push_back(infoName);

	LAStringVector sceNames;
	sceNames = scenarioNum == SCENARIO_1 ? mSce1Names : mSce2Names;

	bool isParallel = false;
	if (isParallelShift(ccy))
	{
		param.isParallel = true;
		isParallel = true;
		param.refName.push_back(sceNames[0]);
	}

	if (isGridSensitivity(ccy))
	{
		param.isGrid = true;
		LAStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
		const unsigned int maxIndex = getMaxGridIndex(ccy);
		unsigned int gridSize = marketGridTerm.size();

		LAStringVector BucketTerm = getBucketGridTerm(ccy);
		if (BucketTerm[0] != AQ_NO_DATA)
		{
			gridSize = BucketTerm.size();
		}
		unsigned int refAddIndex = 0;
		if (isParallel)
		{
			refAddIndex = 1;
		}

		
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			if (i > maxIndex)
			{
				break;
			}
			// set dmy
			param.gridFile.push_back(LAStringVector());
			param.refName.push_back(sceNames[i + refAddIndex]);
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
	vector<LAObject *> sce = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return sce;
}

/*!
    @brief create foreign ir vol scenarilo object

	@param[in] ccy
	@param[in] fCcy
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYield::createForeignIRVolEntity(const LAString& ccy, const LAString& fCcy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	const LAString model = LAMarketData::getModelName(fCcy);
	const LAString riskName = getRiskName();

	// set up param for volatility shift
	MAScenarioParam param;
	param.isCalib = true;
	param.ccy = fCcy;
	param.calcType = getExtraCalcType(ccy, scenarioNum, index);
	param.model = model;
	param.targetCurveType = getCurveType(ccy);
	param.inputType = LAMarketData::getVolInputType(model, fCcy, riskName);
	param.targetName = LAMarketData::getBaseVolatilityName(fCcy);

	// dataout
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else 
	{
		param.isOutPut = false;
	}

	LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, fCcy);
	delete calibInfoCreator;

	param.refName.push_back(infoName);

	bool isParallel = false;
	LAString forYieldName;
	const LAString& baseForYieldName = LAMarketData::getBaseYieldName(fCcy);
	if (isParallelShift(ccy))
	{
		param.isParallel = true;
		isParallel = true;
		forYieldName = baseForYieldName + "_" + param.calcType + "_" + param.targetCurveType + "_Parallel";
		param.refName.push_back(forYieldName);
	}

	if (isGridSensitivity(ccy))
	{
		param.isGrid = true;
		LAStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
		const unsigned int maxIndex = getMaxGridIndex(ccy);
		unsigned int gridSize = marketGridTerm.size();

		LAStringVector BucketTerm = getBucketGridTerm(ccy);
		if (BucketTerm[0] != AQ_NO_DATA)
		{
			gridSize = BucketTerm.size();
		}
		unsigned int refAddIndex = 0;
		if (isParallel)
		{
			refAddIndex = 1;
		}

		unsigned int counter = 0;
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			if (i > maxIndex)
			{
				break;
			}
			// set dmy
			param.gridFile.push_back(LAStringVector());
			forYieldName =  baseForYieldName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" +  LAString(static_cast<int>(counter++));
			param.refName.push_back(forYieldName);
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
	vector<LAObject *> sce = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return sce;
}

/*!
    @brief create yield object 

	@param[in] ccy
	@param[in] fCcy foreign currency
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationYield::createForeignYieldEntity(const LAString &ccy, const LAString &fCcy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index, const bool isFirst)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}

	// set up param
	MAScenarioParam param;
	param.ccy = fCcy;
	param.model = LAMarketData::getModelName(fCcy);
	param.calcType = getExtraCalcType(ccy, scenarioNum, index);
	param.shiftType = getShiftType(ccy);
	param.targetName = LAMarketData::getBaseYieldName(fCcy);
	param.targetCurveType = getCurveType(ccy);
	param.isAdjustDF = isAdjustDf(fCcy);
	param.isForeignCcy = true;
	param.isFirst = isFirst;

	//get domestic yield data name and calcType
	LAMathYieldCurvePro &ycPro_fCcy = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(fCcy), ENCHKTYPE_ISDEFINED).get());
	const LAString& affectedCcy = ycPro_fCcy.getAffectedCcy();
	const LAString& baseYieldDataName_baseccy = LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(affectedCcy));

	LAString calcType = param.calcType;
	if (affectedCcy == ccy)
		calcType = getCalcType(ccy, scenarioNum, index);

	bool isParallel = false;
	LAString yieldDataName_baseccy;
	if (isParallelShift(ccy))
	{
		param.isParallel = true;
		isParallel = true;
		yieldDataName_baseccy =  baseYieldDataName_baseccy + "_" + calcType + "_" + param.targetCurveType + "_Parallel";
		param.refName.push_back(yieldDataName_baseccy);
	}

	if (isGridSensitivity(ccy))
	{
		param.isGrid = true;
		LAStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
		const unsigned int maxIndex = getMaxGridIndex(ccy);
		unsigned int gridSize = marketGridTerm.size();

		LAStringVector BucketTerm = getBucketGridTerm(ccy);
		if (BucketTerm[0] != AQ_NO_DATA)
		{
			gridSize = BucketTerm.size();
		}
		unsigned int refAddIndex = 0;
		if (isParallel)
		{
			refAddIndex = 1;
		}
		
		unsigned int counter = 0;
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			if (i > maxIndex)
			{
				break;
			}
			yieldDataName_baseccy =  baseYieldDataName_baseccy + "_" + calcType + "_" + param.targetCurveType 
								+ "_Grid_" +  LAString(static_cast<int>(counter++));
			param.refName.push_back(yieldDataName_baseccy);
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief create yield object 

	@param[in] ccy
	@param[in] colCcy collateral currency
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationYield::createCollateralYieldEntity(const LAString &ccy, const LAString &colCcy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}

	// set up param
	MAScenarioParam param;
	param.ccy = colCcy;
	param.model = LAMarketData::getModelName(colCcy);
	if (ccy == colCcy)
		param.calcType = getCalcType(ccy, scenarioNum, index);
	else
		param.calcType = getExtraCalcType(ccy, scenarioNum, index);
	param.shiftType = getShiftType(ccy);
	param.targetName = LAMarketData::getBaseYieldName(colCcy);
	param.targetCurveType = getCurveType(ccy);
	param.isAdjustDF = isAdjustDf(colCcy);
	param.isCollateralCcy = true;

	//get domestic yield data name and calcType
	LAMathYieldCurvePro &ycPro_colCcy = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(colCcy), ENCHKTYPE_ISDEFINED).get());
	const LAString& affectedCcy = ycPro_colCcy.getColAffectedCcy();
	const LAString& baseYieldDataName_baseccy = LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(affectedCcy));

	LAString calcType;
	if (affectedCcy == ccy) 
		calcType = getCalcType(ccy, scenarioNum, index);
	else
		calcType = getExtraCalcType(ccy, scenarioNum, index);

	bool isParallel = false;
	LAString yieldDataName_baseccy;
	if (isParallelShift(ccy))
	{
		param.isParallel = true;
		isParallel = true;
		yieldDataName_baseccy =  baseYieldDataName_baseccy + "_" + calcType + "_" + param.targetCurveType + "_Parallel";
		param.refName.push_back(yieldDataName_baseccy);
	}

	if (isGridSensitivity(ccy))
	{
		param.isGrid = true;
		LAStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
		const unsigned int maxIndex = getMaxGridIndex(ccy);
		unsigned int gridSize = marketGridTerm.size();

		LAStringVector BucketTerm = getBucketGridTerm(ccy);
		if (BucketTerm[0] != AQ_NO_DATA)
		{
			gridSize = BucketTerm.size();
		}
		unsigned int refAddIndex = 0;
		if (isParallel)
		{
			refAddIndex = 1;
		}
		
		unsigned int counter = 0;
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			if (i > maxIndex)
			{
				break;
			}
			yieldDataName_baseccy =  baseYieldDataName_baseccy + "_" + calcType + "_" + param.targetCurveType 
								+ "_Grid_" +  LAString(static_cast<int>(counter++));
			param.refName.push_back(yieldDataName_baseccy);
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief create fx vol scenarilo object

	@param[in] ccy
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYield::createFXVolEntity(const LAString& ccy, const LAString& fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	// set up param
	MAScenarioParam param;
	setFXVolEntityParams(ccy, fx, dataInstance, scenarioNum, index, param, true);

	//get affecting ccys
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());

	const LAStringVector affectingCcy = ycPro.getAffectingCcy();
	const LAStringVector colAffectingCcy = ycPro.getColAffectingCcy();
	LAStringVector tmp = affectingCcy;
	for (unsigned int i = 0; i < colAffectingCcy.size(); ++i)
	{
		if (tmp.end() != std::find(tmp.begin(), tmp.end(), colAffectingCcy[i]))
			continue;

		tmp.push_back(colAffectingCcy[i]);
	}
	const LAStringVector& fCurveCcys = tmp;

	// get scenario names
	const LAString suffix = "_" + param.calcType + "_" + param.targetCurveType;
	LAStringVector sceNames;
	sceNames = scenarioNum == SCENARIO_1 ? mSce1Names : mSce2Names;

	// set references
	LAString tmpCcy = ccy; tmpCcy.toUpper();
	if (param.isParallel)
	{
		// set dmy
		param.paraFile.push_back(CALIB_DMY_FILE);
		// set reference
		setScenarioParamReferences(tmpCcy, ccys, fCurveCcys, objPool, param.refName, sceNames[0], param.calcType,/* 0,*/ suffix + "_Parallel", -1);
	}
	if (param.isGrid)
	{
		LAStringVector gridTerm = getGridTerm(ccy);
		LAStringVector marketGridTerm = getShiftGridTerm(ccy);
		unsigned int gridSize = gridTerm.size();

		LAStringVector bucketGridTerm = getBucketGridTerm(ccy);
		if (bucketGridTerm[0] != AQ_NO_DATA)
		{
			gridSize = bucketGridTerm.size();
		}
		const unsigned int maxIndex = getMaxGridIndex(ccy);
		unsigned int refAddIndex = 0;
		if (param.isParallel)
		{
			refAddIndex = 1;
		}
		unsigned int counter = 0;
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			if (i > maxIndex)
			{
				break;
			}
			int pre_pos = gridTerm[i].findString("_");
			if (pre_pos >= 0)
			{
				gridTerm[i] = gridTerm[i].subString(pre_pos + 1, gridTerm[i].size() - 1);
			}
			// search market grid pos
			unsigned int m_pos;
			if (!LAMarketData::searchMarketGridPos(marketGridTerm, gridTerm[i], m_pos))
			{
				LAString msg = "Market grid is not contain " + gridTerm[i];
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			// set reference
			setScenarioParamReferences(tmpCcy, ccys, fCurveCcys, objPool, param.refName, sceNames[i + refAddIndex], param.calcType/*, 0*/, suffix + "_Grid_" +  LAString(static_cast<int>(counter++)), i);
			param.gridFile.push_back(LAStringVector());
		}
	}
	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> sce_fx = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return sce_fx;
}
