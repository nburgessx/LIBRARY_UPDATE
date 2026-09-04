/*! @file
    @brief FX setup class for calc risk
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRiskConfigurationFX.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLDealUtils.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLMathFXEntity.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLMathFXUtility.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationFX::AQLRiskConfigurationFX()
:AQLRiskConfigurationYield()
{
	mFXFlg = true;
}

// destructor
/*!

*/
AQLRiskConfigurationFX::~AQLRiskConfigurationFX(void)
{
}

/*!
    @brief get targetNames

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFX::getTargetNames(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	AQLString ret = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	// fx vol calibration has been replaced to extra scenario
	//if (isCalibTarget(fx))
	//	ret += (":" + AQLMarketData::getBaseVolatilityName(fx));
	return ret;
}
 
/*!
    @brief create scenario1 object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationFX::createScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	vector<AQLObject *> ret = createFXEntity(fx, dataInstance, SCENARIO_1,index);
	// fx vol calibration has been replaced to extra scenario
	//if (isCalibTarget(fx))
	//{
	//	vector<AQLObject *> volVec = createFXVolEntity(fx, dataInstance, SCENARIO_1, index);
	//	ret.insert(ret.end(), volVec.begin(), volVec.end());
	//}
	return ret;
}


/*!
    @brief create scenario2 object 

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationFX::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	AQLString bumpDirection = getBumpDirection(fx);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}
	vector<AQLObject *> ret = createFXEntity(fx, dataInstance, SCENARIO_2,index);
	// fx vol calibration has been replaced to extra scenario
	//if (isCalibTarget(fx))
	//{
	//	vector<AQLObject *> volVec = createFXVolEntity(fx, dataInstance, SCENARIO_2, index);
	//	ret.insert(ret.end(), volVec.begin(), volVec.end());
	//}
	return ret;
}

/*!
    @brief createFXEntity

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationFX::createFXEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	// scenario param
	AQLScenarioParam param;
	setFXEntityParams(fx, fx, dataInstance, scenarioNum, index, param);

	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_FX);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return ret;
}

/*!
    @brief create volatility scenario object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationFX::createFXVolEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	// scenario param
	AQLScenarioParam param;
	setFXVolEntityParams(AQ_NO_DATA, fx, dataInstance, scenarioNum, index, param, false);

	// set file path dmy
	param.paraFile.push_back(CALIB_DMY_FILE);

	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
		
	delete sceCreator;

	return ret;
}

/*!
    @brief return isGridSensitivity

	@param[in] fx
	@return bool
*/
bool
AQLRiskConfigurationFX::isGridSensitivity(const AQLString &fx) const
{
	//fx;
	return false;
}

/*!
	@brief return isParallelShift

	@param[in] fx
	@return bool
*/
bool
AQLRiskConfigurationFX::isParallelShift(const AQLString &fx) const
{
	//fx;
	return true;
}

AQLStringVector
AQLRiskConfigurationFX::getRiskCurrencys(AQLObjectPool& objPool) const
{
	return AQLDealUtils::getSDECurrencys();
}

/*!
    @brief return grid term

	@param[in] fx
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationFX::getGridTerm(const AQLString &fx) const
{
	//(void)fx;
	return vector<AQLString>();
}

/*!
    @brief return bucket grid term

	@param[in] fx
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationFX::getBucketGridTerm(const AQLString &fx) const
{
	//fx;
	vector<AQLString> ret;
	AQLString str = AQ_NO_DATA;
	ret.push_back(str);
	return ret;
}

vector<vector<AQLObject *> > 
AQLRiskConfigurationFX::createExtraScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	if (isRealCalib())
	{
		return createExtraScenarioEntity(fx, dataInstance, SCENARIO_1, index);
	}
	else
	{
		vector<vector<AQLObject *> > ret(0);
		return ret;
	}
}

vector<vector<AQLObject *> > 
AQLRiskConfigurationFX::createExtraScenarioEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	vector<vector<AQLObject *> > ret(0);
	map<AQLString, vector<AQLObject *> > scemap;
	vector<AQLObject *> sce;

	// get currencies (and FX pairs) whose SDE models should be recalibrated  
    AQLStringVector curs = AQLMathFXUtility::getCurrencyPair(fx);
	AQLStringVector fxcurs = curs;
	for(int unsigned i = 0; i < fxcurs.size(); ++i)
	{
		AQLString ccy_tmp = fxcurs[i];
		AQLMathYieldCurvePro &ycPro_fCcy = dynamic_cast<AQLMathYieldCurvePro &>
			(objPool.getObject(AQLMarketData::getBaseYieldProName(ccy_tmp), ENCHKTYPE_ISDEFINED).get());
		for(int unsigned j = 0; j < ycPro_fCcy.getAffectingCcy().size(); ++j)
		{
			AQLString ccy = (ycPro_fCcy.getAffectingCcy())[j];

			bool isExist = false;
			for(int unsigned k = 0; k < curs.size(); ++k)
			{
				if(ccy == curs[k]) 
					isExist = true;
			}
			if(!isExist)
				curs.push_back(ccy);
		}
	}

	map<AQLString, AQLString> AffectedCcy;
	map<AQLString, AQLString> ColAffectedCcy;
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		AQLString ccy_tmp = curs[i];

		AQLMathYieldCurvePro &ycPro_fCcy = dynamic_cast<AQLMathYieldCurvePro &>
			(objPool.getObject(AQLMarketData::getBaseYieldProName(ccy_tmp), ENCHKTYPE_ISDEFINED).get());
		AffectedCcy[curs[i]] = ycPro_fCcy.getAffectedCcy();
		ColAffectedCcy[curs[i]] = ycPro_fCcy.getColAffectedCcy();
	}

	// Yield curve 
	for (int unsigned i = 0; i < curs.size(); ++i)
	{
		// XccyBasis curve
		sce = createExtraYieldEntity(fx, curs[i], dataInstance, scenarioNum, index);
		scemap[curs[i]] = sce;
	}
	// FwdFXConst curve
	for (int unsigned i = 0; i < curs.size(); ++i)
	{
		if (ColAffectedCcy[curs[i]].isDefined() && ColAffectedCcy[curs[i]] != AQ_NO_DATA)
		{
			sce = createCollateralYieldEntity(fx, curs[i], dataInstance, scenarioNum, index);
			scemap[curs[i]] = sce;
		}
	}
	// insert
	for (int unsigned i = 0; i < curs.size(); ++i)
	{	
		ret.push_back(scemap[curs[i]]);
	}

	for (map<AQLString, vector<AQLObject *> >::const_iterator it = scemap.begin(); it != scemap.end(); ++it)
	{
		sce = it->second;
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

	// IR Vol
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		AQLString ccy_tmp = curs[i];
		if (isCalibTarget(ccy_tmp))
		{
			sce = createExtraIRVolEntity(fx, ccy_tmp, dataInstance, scenarioNum, index);
			ret.push_back(sce);
		}
	}

	// FX Vol
	if(isCalibTarget(fx))
	{
		sce = createExtraFXVolEntity(fx, dataInstance, scenarioNum, index);
		ret.push_back(sce);
	}

	// FX Vol depending on the risk target FX
	AQLStringVector SDEccys = AQLDealUtils::getSDECurrencys();
	for(unsigned int i = 0; i < SDEccys.size(); ++i)
	{
		if(SDEccys[i].findString(FX_DELIMITER) < 0 || SDEccys[i] == fx)
			continue;

		for(unsigned int j = 0; j < curs.size(); ++j)
		{
			if(SDEccys[i].findString(curs[j]) > 0 && isCalibTarget(SDEccys[i]))
			{
				sce = createExtraFXVolEntity(SDEccys[i], dataInstance, scenarioNum, index);
				ret.push_back(sce);
			}
		}
	}

	return ret;
}

vector<AQLObject *> 
AQLRiskConfigurationFX::createExtraYieldEntity(const AQLString &fx, const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index, const bool isFirst)  const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString bumpDirection = getBumpDirection(fx);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}

	// set up param
	AQLScenarioParam param;

	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLMathFXEntity *targetFX = AQLMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	AQLString calcType = getCalcType(fx, scenarioNum, index);
	AQLString refFX = targetFX->getName();
	refFX += "_" + calcType;
	param.refName.push_back(refFX);

	param.ccy = ccy;
	param.calcType = getExtraCalcType(fx, scenarioNum, index);
	param.targetName = AQLMarketData::getBaseYieldName(ccy);
	param.isExtraXccyCurveForFXDelta = true;
	param.isParallel = true;
	param.isFirst = isFirst;

	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

AQLStringVector
AQLRiskConfigurationFX::getExtraTargetNames1(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	AQLStringVector ret(0);
	AQLStringVector targetNames = getTargetNames(fx, dataInstance).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int size = targetNames.size();

	AQLObjectPool &objPool = dataInstance.getObjectPool();

	// get currencies (and FX pairs) whose SDE models should be recalibrated  
	AQLStringVector curs = AQLMathFXUtility::getCurrencyPair(fx);
	AQLStringVector fxcurs = curs;
	for(int unsigned i = 0; i < fxcurs.size(); ++i)
	{
		AQLString ccy_tmp = fxcurs[i];
		AQLMathYieldCurvePro &ycPro_fCcy = dynamic_cast<AQLMathYieldCurvePro &>
			(objPool.getObject(AQLMarketData::getBaseYieldProName(ccy_tmp), ENCHKTYPE_ISDEFINED).get());
		for(int unsigned j = 0; j < ycPro_fCcy.getAffectingCcy().size(); ++j)
		{
			AQLString ccy = (ycPro_fCcy.getAffectingCcy())[j];

			bool isExist = false;
			for(int unsigned k = 0; k < curs.size(); ++k)
			{
				if(ccy == curs[k]) 
					isExist = true;
			}
			if(!isExist)
				curs.push_back(ccy);
		}
	}

	map<AQLString, AQLString> FwdFXGrids;
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		AQLString ccy_tmp = curs[i];
		FwdFXGrids[curs[i]] = mpStaticData->getStaticData(ccy_tmp.toLower() + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID);
	}

	// Yield curve
	for (int unsigned i = 0; i < curs.size(); ++i)
	{
		AQLString ccy_tmp = curs[i];
		AQLString yieldNames;

		for(unsigned int j = 0; j < size; ++j)
		{
			const AQLString yieldName = AQLMarketData::getBaseYieldName(ccy_tmp);
			yieldNames += yieldName + ":";
		}
		ret.push_back(yieldNames.subString(0, yieldNames.size() - 2));
	}

	// IR Vol
	for (unsigned int i = 0; i < curs.size(); ++i)
	{
		AQLString ccy_tmp = curs[i];
		if (isCalibTarget(ccy_tmp))
		{
			AQLString IRVolNames;
			AQLString IRVolName = AQLMarketData::getBaseVolatilityName(ccy_tmp);

			for (unsigned int j = 0; j < size; ++j)
			{
				IRVolNames += IRVolName + ":";
			}
			ret.push_back(IRVolNames.subString(0, IRVolNames.size()-2));
		}
	}

	// FX Vol
	if(isCalibTarget(fx))
	{
		AQLString FXVolNames;
		AQLString FXVolName = AQLMarketData::getBaseVolatilityName(fx);

		for(unsigned int i = 0; i < size; ++i)
		{
			FXVolNames += FXVolName + ":";		
		}
		ret.push_back(FXVolNames.subString(0, FXVolNames.size()-2));
	}

	// FX Vol depending on the risk target FX
	AQLStringVector SDEccys = AQLDealUtils::getSDECurrencys();
	for(unsigned int i = 0; i < SDEccys.size(); ++i)
	{
		if(SDEccys[i].findString(FX_DELIMITER) < 0 || SDEccys[i] == fx)
			continue;

		for(unsigned int j = 0; j < curs.size(); ++j)
		{
			if(SDEccys[i].findString(curs[j]) > 0 && isCalibTarget(SDEccys[i]))
			{
				AQLString FXVolNames;
				AQLString FXVolName = AQLMarketData::getBaseVolatilityName(SDEccys[i]);

				for(unsigned int k = 0; k < size; ++k)
				{
					FXVolNames += FXVolName + ":";		
				}
				ret.push_back(FXVolNames.subString(0, FXVolNames.size()-2));
			}
		}
	}

	return ret;
}

vector<AQLObject *> 
AQLRiskConfigurationFX::createExtraIRVolEntity(const AQLString &fx, const AQLString& ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	const AQLString model = AQLMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();

	// set up param for volatility shift
	AQLScenarioParam param;
	param.isCalib = true;
	param.ccy = ccy;
	param.calcType = getExtraCalcType(fx, scenarioNum, index);
	param.targetCurveType = getCurveType(ccy);
	param.model = model;
	param.inputType = AQLMarketData::getVolInputType(model, ccy, riskName);
	param.targetName = AQLMarketData::getBaseVolatilityName(ccy);

	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else
	{
		param.isOutPut = false;
	}

	AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
	delete calibInfoCreator;

	param.refName.push_back(infoName);

	param.isParallel = true;
	AQLString forYieldName;
	const AQLString& baseForYieldName = AQLMarketData::getBaseYieldName(ccy);
	forYieldName = baseForYieldName + "_" + param.calcType;
	param.refName.push_back(forYieldName);

	AQLScenarioConfiguration *sceCreator
		= AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
	vector <AQLObject *> sce = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return sce;
}

vector<AQLObject *> 
AQLRiskConfigurationFX::createExtraFXVolEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	// scenario param
	AQLScenarioParam param;
	setFXVolEntityParams(AQ_NO_DATA, fx, dataInstance, scenarioNum, index, param, false);
	param.calcType = getExtraCalcType(fx, scenarioNum, index);
	param.targetCurveType = getCurveType(fx);

	// set file path dmy
	param.paraFile.push_back(CALIB_DMY_FILE);

	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
		
	delete sceCreator;

	return ret;
}

vector<AQLObject *> 
AQLRiskConfigurationFX::createCollateralYieldEntity(const AQLString &fx, const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString bumpDirection = getBumpDirection(fx);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}

	// set up param
	AQLScenarioParam param;

	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLMathFXEntity *targetFX = AQLMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	AQLString calcType = getCalcType(fx, scenarioNum, index);
	AQLString refFX = targetFX->getName();
	refFX += "_" + calcType;

	//param.refName.push_back(refFX);

	param.ccy = ccy;
	param.model = AQLMarketData::getModelName(ccy);
	
	param.calcType = getExtraCalcType(fx, scenarioNum, index);
	param.shiftType = getShiftType(fx);
	param.targetName = AQLMarketData::getBaseYieldName(ccy);
	param.isAdjustDF = isAdjustDf(ccy);
	param.isExtraFwdFXConstCurveForFXDelta = true;

	param.isParallel = true;

	//get domestic yield data name and calcType
	AQLMathYieldCurvePro &ycPro_ccy = dynamic_cast<AQLMathYieldCurvePro &>
						(objPool.getObject(AQLMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());
	const AQLString& affectedCcy = ycPro_ccy.getColAffectedCcy();
	const AQLString& baseYieldDataName_baseccy = AQLMarketData::getYieldDataName(objPool, AQLMarketData::getBaseYieldName(affectedCcy));
	const AQLString& baseYieldDataName_ExtraEntity = baseYieldDataName_baseccy + "_" + getExtraCalcType(fx, scenarioNum, index);

	if (dataInstance.getObjectPool().getObject(baseYieldDataName_ExtraEntity, ENCHKTYPE_NOCHECK).isDefined())
	{
		param.refName.push_back(baseYieldDataName_ExtraEntity);
	}
	else
	{
		param.refName.push_back(baseYieldDataName_baseccy);
	}

	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

vector<vector<AQLObject *> > 
AQLRiskConfigurationFX::createExtraScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	if (isRealCalib())
	{
		return createExtraScenarioEntity(fx, dataInstance, SCENARIO_2, index);
	}
	else
	{
		vector<vector<AQLObject *> > ret(0);
		return ret;
	}
}

vector<AQLObject *> 
AQLRiskConfigurationFX::createExtraIRVolEntityBase(const AQLString &fx, const AQLString& ccy, AQLDataInstance &dataInstance, int index) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	const AQLString model = AQLMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();

	// set up param for volatility shift
	AQLScenarioParam param;
	param.isCalib = true;
	param.ccy = ccy;
	param.calcType = getBaseExtraCalcType(fx, index);
	param.targetCurveType = getCurveType(ccy);
	param.model = model;
	param.inputType = AQLMarketData::getVolInputType(model, ccy, riskName);
	param.targetName = AQLMarketData::getBaseVolatilityName(ccy);

	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else
	{
		param.isOutPut = false;
	}

	AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
	delete calibInfoCreator;

	param.refName.push_back(infoName);

	param.isParallel = true;
	AQLString forYieldName;
	const AQLString& baseForYieldName = AQLMarketData::getBaseYieldName(ccy);
	forYieldName = baseForYieldName + "_" + param.calcType;
	param.refName.push_back(forYieldName);

	AQLScenarioConfiguration *sceCreator
		= AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
	vector <AQLObject *> sce = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return sce;
}
/*!
    @brief set up base scenario

	@param[in] fx
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationFX::createBaseScenarioEntity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const
{
	if (omitNotionalExposure(fx))
	{
		return AQLRiskConfiguration::createFXBaseScenarioEntity(fx, dataInstance, index);
	}
	else
	{
		return std::vector<AQLObject*>(0);
	}
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationFX::getBaseOutPutName(const AQLString &ccy , int index) const
{
	if (omitNotionalExposure(ccy))
	{
		AQLStringVector ccys = ccy.toToken(FX_DELIMITER);
		if (ccys.size() != 2) return AQ_NO_DATA;
		AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
		AQLString ret = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_OUTPUTNAME);
		ret += AQLString("_BasePV");
		return ret;
	}
	else
	{
		return AQ_NO_DATA;
	}
}

/*!
    @brief return baseshiftval

	@param[in] ccy
	@param[in] index
	@return double
*/
double
AQLRiskConfigurationFX::getBaseSpotVal(const AQLString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("AQLRiskConfigurationFX::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
bool
AQLRiskConfigurationFX::omitNotionalExposure(const AQLString &ccy) const
{
	AQLStringVector ccys = ccy.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return AQLRiskConfiguration::omitNotionalExposure(ccy);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString omitNotionalExposure = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_OMITNOTIONALEXPOSURE);
	if (omitNotionalExposure == AQ_NO_DATA)
	{
		return false;
	}
	else
	{
		return convertBoolFromStr(omitNotionalExposure);
	}
}
