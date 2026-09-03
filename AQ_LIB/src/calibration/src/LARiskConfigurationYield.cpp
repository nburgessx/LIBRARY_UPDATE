#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LARiskConfigurationYield.h"
#include "AQLDataInstance.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
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
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
LARiskConfigurationYield::createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	return createYieldEntity(ccy, dataInstance, SCENARIO_1, index);
}

/*!
    @brief create scenario2 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
LARiskConfigurationYield::createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	return createYieldEntity(ccy, dataInstance, SCENARIO_2, index);
}


/*!
    @brief get targetNames

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYield::getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	unsigned int gridIndex = getMaxGridIndex(ccy);
	const AQLString yieldName = LAMarketData::getBaseYieldName(ccy);

	AQLString ret;
	if (isParallelShift(ccy))
	{
		ret += yieldName + ":";
	}

	if (isGridSensitivity(ccy))
	{
		AQLStringVector bucketterm = getBucketGridTerm(ccy);
		if (bucketterm[0] != AQ_NO_DATA)
			gridIndex = bucketterm.size() - 1;
		for (unsigned int i = 0; i <= gridIndex; ++i)
		{
			ret += yieldName + ":";
		}
	}

	if (ret.size() < 2)
	{
		throw AQLCoreInvalidData("Grid nor parallel is not set. can not create targetnames!!", __FILE__, __LINE__); 
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
LARiskConfigurationYield::getMaxGridIndex(const AQLString &ccy) const
{
	const int dealMax = LACoreDataService::getContext(CONTEXT_KEY_DEAL_MAXTERM).getIntValue();

	AQLDate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	AQLPriceDataDayCount dayCount(ACT_365_ISDA);

	AQLStringVector grids = getShiftGridTerm(ccy);
	const unsigned int gSize = grids.size();
	if (gSize < 1)
	{
		throw AQLCoreInvalidData("Grid size must be more than zero.", __FILE__, __LINE__);
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
			AQLDate date = LAMathDateCalculations::getIMMDateFromTerm(asOfDate, grids[i]);
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
			AQLDate date = LAMathDateCalculations::getDate(asOfDate, grids[i], true);
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
AQLStringVector
LARiskConfigurationYield::getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	if (isZeroBump(ccy))
	{
		return LARiskConfiguration::getExtraTargetNames1(ccy, dataInstance);
	}
	AQLStringVector ret(0);
	AQLStringVector targetNames = getTargetNames(ccy, dataInstance).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int size = targetNames.size();

	AQLObjectPool &objPool = dataInstance.getObjectPool();
	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());

	// get affecting ccys
	const AQLStringVector &fCurveCcys = ycPro.getAffectingCcy();
	// get col affecting ccys
	StringSet cCurveCcys;
	AQLStringVector tmpCcys = ycPro.getColAffectingCcy();
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
		const AQLString yieldName = LAMarketData::getBaseYieldName(*it);
		AQLString ccy_fYldTargetNames;
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
			AQLString ccy_fTargetName;
			AQLString ccy_fBVolName = LAMarketData::getBaseVolatilityName(*it);
			for (unsigned int j = 0; j < size; ++j)
			{
				ccy_fTargetName += ccy_fBVolName + ":";
			}
			if (ccy_fTargetName.size() < 2)
			{
				throw AQLCoreInvalidData("Extra targetname1 can not set !!", __FILE__, __LINE__);
			}
			ret.push_back(ccy_fTargetName.subString(0, ccy_fTargetName.size() - 2));
		}
	}
	// check calibration target
	if (isCalibTarget(ccy))
	{
		AQLString ccy_targetNames1;
		AQLString ccy_bVolName = LAMarketData::getBaseVolatilityName(ccy);
		for (unsigned int i = 0; i < size; ++i)
		{
			ccy_targetNames1 += ccy_bVolName + ":";
		}

		if (ccy_targetNames1.size() < 2)
		{
			throw AQLCoreInvalidData("Extra targetname1 can not set !!", __FILE__, __LINE__);
		}
		ret.push_back(ccy_targetNames1.subString(0, ccy_targetNames1.size() - 2));
	}
	AQLStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		// if cross check fx is target
		AQLStringVector ccys = targetFXVec[i].toToken(FX_DELIMITER);
		AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
		AQLString fx_targetNames1;
		AQLString fx_bVolName = LAMarketData::getBaseVolatilityName(key_fx);
		for (unsigned int i = 0; i < size; ++i)
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
LARiskConfigurationYield::createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
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
AQLStringVector
LARiskConfigurationYield::getExtraTargetNames2(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	if (isZeroBump(ccy))
	{
		return LARiskConfiguration::getExtraTargetNames2(ccy, dataInstance);
	}
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
LARiskConfigurationYield::createExtraScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	if (isZeroBump(ccy))
	{
		return LARiskConfiguration::createExtraScenario2Entity(ccy, dataInstance, index);
	}
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<vector<AQLObject *> >(0);
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
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
LARiskConfigurationYield::createExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	vector<vector<AQLObject *> > ret(0);

	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());

	map<AQLString, vector<AQLObject *> > scemap;
	vector<AQLObject *> sce;

	// get affecting ccys
	const AQLStringVector& fCurveCcys = ycPro.getAffectingCcy();
	// get col affecting ccys
	StringSet cCurveCcys;
	StringSet cfCurveCcys;
	AQLStringVector tmpCcys = ycPro.getColAffectingCcy();
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
	const AQLString usd = "USD";
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
	for (map<AQLString, vector<AQLObject *> >::const_iterator it = scemap.begin(); it != scemap.end(); ++it)
	{
		sce = it->second;
		ret.push_back(sce);
		for (int unsigned j = 0; j < sce.size(); ++j)
		{
			const AQLString &name = dynamic_cast<const AQLDataString &>(sce[j]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			sce[j]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
			sce[j]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new AQLDataBool(true));
			AQLObjectHolder objHolder = objPool.getObject(name,  ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
				objPool.set(name, sce[j]);
		}
	}
	scemap.clear();

	for (set<AQLString>::const_iterator it = curveCcys.begin(); it != curveCcys.end(); ++it)
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
	AQLStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	// scenario check
	if (fxSize > 0 && mSce1Names.empty() && mSce2Names.empty())
	{
		throw AQLCoreInvalidData("Scenario entities have not been created yet", __FILE__, __LINE__);
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
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
LARiskConfigurationYield::createExtraScenarioEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	vector<vector<AQLObject *> > ret(0);

	const AQLString model = LAMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();

	AQLString riskNameSuffix;
	AQLStringVector sceNames;
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

	AQLString bShiftStr = getBaseShiftStr(ccy, index); // base shirt value(string)
	if (isCalibTarget(ccy))
	{
		// set up param for volatility shift
		MAScenarioParam param;
		param.ccy = ccy;
		param.calcType = ccy + "_" + riskName + "_" + riskNameSuffix + "_1_" + AQLString(index);
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
			AQLStringVector gridTerm = getGridTerm(ccy);   //M-Lib type grid
			AQLStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
			unsigned int gridSize = gridTerm.size();
			const unsigned int maxIndex = getMaxGridIndex(ccy);
			unsigned int refAddIndex = 0;

			AQLStringVector BucketTerm = getBucketGridTerm(ccy);
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
					AQLString msg = "Market grid is not contain " + gridTerm[i];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				AQLStringVector filePath;
				LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, filePath, true, &marketGridTerm[m_pos], &bShiftStr);
				param.gridFile.push_back(filePath);
				param.refName.push_back(sceNames[i + refAddIndex]);
			}
		}

		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<AQLObject *> sce = sceCreator->createScenario(dataInstance, param);
		delete sceCreator;

		ret.push_back(sce);

	}
	// set parameter for fx vol shift
	AQLStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		vector<AQLObject *> sce_fx;
		AQLStringVector ccys = targetFXVec[i].toToken(FX_DELIMITER);
		AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
		const AQLString model_fx = LAMarketData::getModelName(key_fx);
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
			throw AQLCoreInvalidData("Scenario object is not created yet", __FILE__, __LINE__);
		}

		ccys[0].toUpper();
		AQLString tmpCurrency = ccy;
		tmpCurrency.toUpper();
		bool isParallel = isParallelShift(ccy);
		if (isParallel)
		{
			param.calcType = ccy + "_" + key_fx + "_" + riskName + "_" + riskNameSuffix + "_2_" + AQLString(index) + "_Parallel";
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
			param.refName.resize(2);
			AQLStringVector gridTerm = getGridTerm(ccy);
			AQLStringVector marketGridTerm = getShiftGridTerm(ccy);
			const unsigned int gridSize = gridTerm.size();
			const unsigned int maxIndex = getMaxGridIndex(ccy);
			unsigned int addIndex = 0;
			if (isParallel)
			{
				addIndex = 1;
			}
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				param.calcType = ccy + "_" + key_fx + "_" + riskName + "_" + riskNameSuffix + "_2_" + AQLString(index) + "_Grid_" + AQLString(static_cast<int>(i));
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
					AQLString msg = "Market grid is not contain " + gridTerm[i];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
    @brief create ir vol scenarilo object

	@param[in] ccy
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
LARiskConfigurationYield::createIRVolEntity(const AQLString& ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	const AQLString model = LAMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();

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
	AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
	delete calibInfoCreator;

	param.refName.push_back(infoName);

	AQLStringVector sceNames;
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
		AQLStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
		const unsigned int maxIndex = getMaxGridIndex(ccy);
		unsigned int gridSize = marketGridTerm.size();

		AQLStringVector BucketTerm = getBucketGridTerm(ccy);
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
			param.gridFile.push_back(AQLStringVector());
			param.refName.push_back(sceNames[i + refAddIndex]);
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
	vector<AQLObject *> sce = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return sce;
}

/*!
    @brief create foreign ir vol scenarilo object

	@param[in] ccy
	@param[in] fCcy
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
LARiskConfigurationYield::createForeignIRVolEntity(const AQLString& ccy, const AQLString& fCcy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	const AQLString model = LAMarketData::getModelName(fCcy);
	const AQLString riskName = getRiskName();

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
	AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, fCcy);
	delete calibInfoCreator;

	param.refName.push_back(infoName);

	bool isParallel = false;
	AQLString forYieldName;
	const AQLString& baseForYieldName = LAMarketData::getBaseYieldName(fCcy);
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
		AQLStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
		const unsigned int maxIndex = getMaxGridIndex(ccy);
		unsigned int gridSize = marketGridTerm.size();

		AQLStringVector BucketTerm = getBucketGridTerm(ccy);
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
			param.gridFile.push_back(AQLStringVector());
			forYieldName =  baseForYieldName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" +  AQLString(static_cast<int>(counter++));
			param.refName.push_back(forYieldName);
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
	vector<AQLObject *> sce = sceCreator->createScenario(dataInstance, param);
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
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
LARiskConfigurationYield::createForeignYieldEntity(const AQLString &ccy, const AQLString &fCcy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index, const bool isFirst)  const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
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
	const AQLString& affectedCcy = ycPro_fCcy.getAffectedCcy();
	const AQLString& baseYieldDataName_baseccy = LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(affectedCcy));

	AQLString calcType = param.calcType;
	if (affectedCcy == ccy)
		calcType = getCalcType(ccy, scenarioNum, index);

	bool isParallel = false;
	AQLString yieldDataName_baseccy;
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
		AQLStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
		const unsigned int maxIndex = getMaxGridIndex(ccy);
		unsigned int gridSize = marketGridTerm.size();

		AQLStringVector BucketTerm = getBucketGridTerm(ccy);
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
								+ "_Grid_" +  AQLString(static_cast<int>(counter++));
			param.refName.push_back(yieldDataName_baseccy);
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
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
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
LARiskConfigurationYield::createCollateralYieldEntity(const AQLString &ccy, const AQLString &colCcy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
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
	const AQLString& affectedCcy = ycPro_colCcy.getColAffectedCcy();
	const AQLString& baseYieldDataName_baseccy = LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(affectedCcy));

	AQLString calcType;
	if (affectedCcy == ccy) 
		calcType = getCalcType(ccy, scenarioNum, index);
	else
		calcType = getExtraCalcType(ccy, scenarioNum, index);

	bool isParallel = false;
	AQLString yieldDataName_baseccy;
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
		AQLStringVector marketGridTerm = getShiftGridTerm(ccy); // market(file set) grid
		const unsigned int maxIndex = getMaxGridIndex(ccy);
		unsigned int gridSize = marketGridTerm.size();

		AQLStringVector BucketTerm = getBucketGridTerm(ccy);
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
								+ "_Grid_" +  AQLString(static_cast<int>(counter++));
			param.refName.push_back(yieldDataName_baseccy);
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief create fx vol scenarilo object

	@param[in] ccy
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
LARiskConfigurationYield::createFXVolEntity(const AQLString& ccy, const AQLString& fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	// set up param
	MAScenarioParam param;
	setFXVolEntityParams(ccy, fx, dataInstance, scenarioNum, index, param, true);

	//get affecting ccys
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());

	const AQLStringVector affectingCcy = ycPro.getAffectingCcy();
	const AQLStringVector colAffectingCcy = ycPro.getColAffectingCcy();
	AQLStringVector tmp = affectingCcy;
	for (unsigned int i = 0; i < colAffectingCcy.size(); ++i)
	{
		if (tmp.end() != std::find(tmp.begin(), tmp.end(), colAffectingCcy[i]))
			continue;

		tmp.push_back(colAffectingCcy[i]);
	}
	const AQLStringVector& fCurveCcys = tmp;

	// get scenario names
	const AQLString suffix = "_" + param.calcType + "_" + param.targetCurveType;
	AQLStringVector sceNames;
	sceNames = scenarioNum == SCENARIO_1 ? mSce1Names : mSce2Names;

	// set references
	AQLString tmpCcy = ccy; tmpCcy.toUpper();
	if (param.isParallel)
	{
		// set dmy
		param.paraFile.push_back(CALIB_DMY_FILE);
		// set reference
		setScenarioParamReferences(tmpCcy, ccys, fCurveCcys, objPool, param.refName, sceNames[0], param.calcType,/* 0,*/ suffix + "_Parallel", -1);
	}
	if (param.isGrid)
	{
		AQLStringVector gridTerm = getGridTerm(ccy);
		AQLStringVector marketGridTerm = getShiftGridTerm(ccy);
		unsigned int gridSize = gridTerm.size();

		AQLStringVector bucketGridTerm = getBucketGridTerm(ccy);
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
				AQLString msg = "Market grid is not contain " + gridTerm[i];
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			// set reference
			setScenarioParamReferences(tmpCcy, ccys, fCurveCcys, objPool, param.refName, sceNames[i + refAddIndex], param.calcType/*, 0*/, suffix + "_Grid_" +  AQLString(static_cast<int>(counter++)), i);
			param.gridFile.push_back(AQLStringVector());
		}
	}
	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> sce_fx = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return sce_fx;
}
