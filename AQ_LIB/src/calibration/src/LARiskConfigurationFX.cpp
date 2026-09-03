/*! @file
    @brief FX setup class for calc risk
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFX.cpp
//
//  DESCRIPTION :       FX setup class for calc risk 
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


#include "LARiskConfigurationFX.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADealUtils.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LAMathFXEntity.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LADefinitionsCalibration.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "LAMathFXUtility.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationFX::LARiskConfigurationFX()
:LARiskConfigurationYield()
{
	mFXFlg = true;
}

// destructor
/*!

*/
LARiskConfigurationFX::~LARiskConfigurationFX(void)
{
}

/*!
    @brief get targetNames

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFX::getTargetNames(const LAString &fx, LADataInstance &dataInstance) const
{
	LAString ret = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	// fx vol calibration has been replaced to extra scenario
	//if (isCalibTarget(fx))
	//	ret += (":" + LAMarketData::getBaseVolatilityName(fx));
	return ret;
}
 
/*!
    @brief create scenario1 object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationFX::createScenario1Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	vector<LAObject *> ret = createFXEntity(fx, dataInstance, SCENARIO_1,index);
	// fx vol calibration has been replaced to extra scenario
	//if (isCalibTarget(fx))
	//{
	//	vector<LAObject *> volVec = createFXVolEntity(fx, dataInstance, SCENARIO_1, index);
	//	ret.insert(ret.end(), volVec.begin(), volVec.end());
	//}
	return ret;
}


/*!
    @brief create scenario2 object 

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationFX::createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	LAString bumpDirection = getBumpDirection(fx);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}
	vector<LAObject *> ret = createFXEntity(fx, dataInstance, SCENARIO_2,index);
	// fx vol calibration has been replaced to extra scenario
	//if (isCalibTarget(fx))
	//{
	//	vector<LAObject *> volVec = createFXVolEntity(fx, dataInstance, SCENARIO_2, index);
	//	ret.insert(ret.end(), volVec.begin(), volVec.end());
	//}
	return ret;
}

/*!
    @brief createFXEntity

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationFX::createFXEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	// scenario param
	MAScenarioParam param;
	setFXEntityParams(fx, fx, dataInstance, scenarioNum, index, param);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_FX);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return ret;
}

/*!
    @brief create volatility scenario object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationFX::createFXVolEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	// scenario param
	MAScenarioParam param;
	setFXVolEntityParams(MLIB_NO_DATA, fx, dataInstance, scenarioNum, index, param, false);

	// set file path dmy
	param.paraFile.push_back(CALIB_DMY_FILE);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
		
	delete sceCreator;

	return ret;
}

/*!
    @brief return isGridSensitivity

	@param[in] fx
	@return bool
*/
bool
LARiskConfigurationFX::isGridSensitivity(const LAString &fx) const
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
LARiskConfigurationFX::isParallelShift(const LAString &fx) const
{
	//fx;
	return true;
}

LAStringVector
LARiskConfigurationFX::getRiskCurrencys(LAObjectPool& objPool) const
{
	return MADealUtils::getSDECurrencys();
}

/*!
    @brief return grid term

	@param[in] fx
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationFX::getGridTerm(const LAString &fx) const
{
	//(void)fx;
	return vector<LAString>();
}

/*!
    @brief return bucket grid term

	@param[in] fx
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationFX::getBucketGridTerm(const LAString &fx) const
{
	//fx;
	vector<LAString> ret;
	LAString str = MLIB_NO_DATA;
	ret.push_back(str);
	return ret;
}

vector<vector<LAObject *> > 
LARiskConfigurationFX::createExtraScenario1Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	if (isRealCalib())
	{
		return createExtraScenarioEntity(fx, dataInstance, SCENARIO_1, index);
	}
	else
	{
		vector<vector<LAObject *> > ret(0);
		return ret;
	}
}

vector<vector<LAObject *> > 
LARiskConfigurationFX::createExtraScenarioEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	vector<vector<LAObject *> > ret(0);
	map<LAString, vector<LAObject *> > scemap;
	vector<LAObject *> sce;

	// get currencies (and FX pairs) whose SDE models should be recalibrated  
    LAStringVector curs = LAMathFXUtility::getCurrencyPair(fx);
	LAStringVector fxcurs = curs;
	for(int unsigned i = 0; i < fxcurs.size(); ++i)
	{
		LAString ccy_tmp = fxcurs[i];
		LAMathYieldCurvePro &ycPro_fCcy = dynamic_cast<LAMathYieldCurvePro &>
			(objPool.getObject(LAMarketData::getBaseYieldProName(ccy_tmp), ENCHKTYPE_ISDEFINED).get());
		for(int unsigned j = 0; j < ycPro_fCcy.getAffectingCcy().size(); ++j)
		{
			LAString ccy = (ycPro_fCcy.getAffectingCcy())[j];

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

	map<LAString, LAString> AffectedCcy;
	map<LAString, LAString> ColAffectedCcy;
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		LAString ccy_tmp = curs[i];

		LAMathYieldCurvePro &ycPro_fCcy = dynamic_cast<LAMathYieldCurvePro &>
			(objPool.getObject(LAMarketData::getBaseYieldProName(ccy_tmp), ENCHKTYPE_ISDEFINED).get());
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
		if (ColAffectedCcy[curs[i]].isDefined() && ColAffectedCcy[curs[i]] != MLIB_NO_DATA)
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

	for (map<LAString, vector<LAObject *> >::const_iterator it = scemap.begin(); it != scemap.end(); ++it)
	{
		sce = it->second;
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

	// IR Vol
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		LAString ccy_tmp = curs[i];
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
	LAStringVector SDEccys = MADealUtils::getSDECurrencys();
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

vector<LAObject *> 
LARiskConfigurationFX::createExtraYieldEntity(const LAString &fx, const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index, const bool isFirst)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAString bumpDirection = getBumpDirection(fx);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}

	// set up param
	MAScenarioParam param;

	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	LAString calcType = getCalcType(fx, scenarioNum, index);
	LAString refFX = targetFX->getName();
	refFX += "_" + calcType;
	param.refName.push_back(refFX);

	param.ccy = ccy;
	param.calcType = getExtraCalcType(fx, scenarioNum, index);
	param.targetName = LAMarketData::getBaseYieldName(ccy);
	param.isExtraXccyCurveForFXDelta = true;
	param.isParallel = true;
	param.isFirst = isFirst;

	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

LAStringVector
LARiskConfigurationFX::getExtraTargetNames1(const LAString &fx, LADataInstance &dataInstance) const
{
	LAStringVector ret(0);
	LAStringVector targetNames = getTargetNames(fx, dataInstance).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int size = targetNames.size();

	LAObjectPool &objPool = dataInstance.getObjectPool();

	// get currencies (and FX pairs) whose SDE models should be recalibrated  
	LAStringVector curs = LAMathFXUtility::getCurrencyPair(fx);
	LAStringVector fxcurs = curs;
	for(int unsigned i = 0; i < fxcurs.size(); ++i)
	{
		LAString ccy_tmp = fxcurs[i];
		LAMathYieldCurvePro &ycPro_fCcy = dynamic_cast<LAMathYieldCurvePro &>
			(objPool.getObject(LAMarketData::getBaseYieldProName(ccy_tmp), ENCHKTYPE_ISDEFINED).get());
		for(int unsigned j = 0; j < ycPro_fCcy.getAffectingCcy().size(); ++j)
		{
			LAString ccy = (ycPro_fCcy.getAffectingCcy())[j];

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

	map<LAString, LAString> FwdFXGrids;
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		LAString ccy_tmp = curs[i];
		FwdFXGrids[curs[i]] = mpStaticData->getStaticData(ccy_tmp.toLower() + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID);
	}

	// Yield curve
	for (int unsigned i = 0; i < curs.size(); ++i)
	{
		LAString ccy_tmp = curs[i];
		LAString yieldNames;

		for(unsigned int j = 0; j < size; ++j)
		{
			const LAString yieldName = LAMarketData::getBaseYieldName(ccy_tmp);
			yieldNames += yieldName + ":";
		}
		ret.push_back(yieldNames.subString(0, yieldNames.size() - 2));
	}

	// IR Vol
	for (unsigned int i = 0; i < curs.size(); ++i)
	{
		LAString ccy_tmp = curs[i];
		if (isCalibTarget(ccy_tmp))
		{
			LAString IRVolNames;
			LAString IRVolName = LAMarketData::getBaseVolatilityName(ccy_tmp);

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
		LAString FXVolNames;
		LAString FXVolName = LAMarketData::getBaseVolatilityName(fx);

		for(unsigned int i = 0; i < size; ++i)
		{
			FXVolNames += FXVolName + ":";		
		}
		ret.push_back(FXVolNames.subString(0, FXVolNames.size()-2));
	}

	// FX Vol depending on the risk target FX
	LAStringVector SDEccys = MADealUtils::getSDECurrencys();
	for(unsigned int i = 0; i < SDEccys.size(); ++i)
	{
		if(SDEccys[i].findString(FX_DELIMITER) < 0 || SDEccys[i] == fx)
			continue;

		for(unsigned int j = 0; j < curs.size(); ++j)
		{
			if(SDEccys[i].findString(curs[j]) > 0 && isCalibTarget(SDEccys[i]))
			{
				LAString FXVolNames;
				LAString FXVolName = LAMarketData::getBaseVolatilityName(SDEccys[i]);

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

vector<LAObject *> 
LARiskConfigurationFX::createExtraIRVolEntity(const LAString &fx, const LAString& ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();

	// set up param for volatility shift
	MAScenarioParam param;
	param.isCalib = true;
	param.ccy = ccy;
	param.calcType = getExtraCalcType(fx, scenarioNum, index);
	param.targetCurveType = getCurveType(ccy);
	param.model = model;
	param.inputType = LAMarketData::getVolInputType(model, ccy, riskName);
	param.targetName = LAMarketData::getBaseVolatilityName(ccy);

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
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

	param.isParallel = true;
	LAString forYieldName;
	const LAString& baseForYieldName = LAMarketData::getBaseYieldName(ccy);
	forYieldName = baseForYieldName + "_" + param.calcType;
	param.refName.push_back(forYieldName);

	LAScenarioConfiguration *sceCreator
		= LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
	vector <LAObject *> sce = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return sce;
}

vector<LAObject *> 
LARiskConfigurationFX::createExtraFXVolEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	// scenario param
	MAScenarioParam param;
	setFXVolEntityParams(MLIB_NO_DATA, fx, dataInstance, scenarioNum, index, param, false);
	param.calcType = getExtraCalcType(fx, scenarioNum, index);
	param.targetCurveType = getCurveType(fx);

	// set file path dmy
	param.paraFile.push_back(CALIB_DMY_FILE);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
		
	delete sceCreator;

	return ret;
}

vector<LAObject *> 
LARiskConfigurationFX::createCollateralYieldEntity(const LAString &fx, const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAString bumpDirection = getBumpDirection(fx);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}

	// set up param
	MAScenarioParam param;

	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	LAString calcType = getCalcType(fx, scenarioNum, index);
	LAString refFX = targetFX->getName();
	refFX += "_" + calcType;

	//param.refName.push_back(refFX);

	param.ccy = ccy;
	param.model = LAMarketData::getModelName(ccy);
	
	param.calcType = getExtraCalcType(fx, scenarioNum, index);
	param.shiftType = getShiftType(fx);
	param.targetName = LAMarketData::getBaseYieldName(ccy);
	param.isAdjustDF = isAdjustDf(ccy);
	param.isExtraFwdFXConstCurveForFXDelta = true;

	param.isParallel = true;

	//get domestic yield data name and calcType
	LAMathYieldCurvePro &ycPro_ccy = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());
	const LAString& affectedCcy = ycPro_ccy.getColAffectedCcy();
	const LAString& baseYieldDataName_baseccy = LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(affectedCcy));
	const LAString& baseYieldDataName_ExtraEntity = baseYieldDataName_baseccy + "_" + getExtraCalcType(fx, scenarioNum, index);

	if (dataInstance.getObjectPool().getObject(baseYieldDataName_ExtraEntity, ENCHKTYPE_NOCHECK).isDefined())
	{
		param.refName.push_back(baseYieldDataName_ExtraEntity);
	}
	else
	{
		param.refName.push_back(baseYieldDataName_baseccy);
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

vector<vector<LAObject *> > 
LARiskConfigurationFX::createExtraScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	if (isRealCalib())
	{
		return createExtraScenarioEntity(fx, dataInstance, SCENARIO_2, index);
	}
	else
	{
		vector<vector<LAObject *> > ret(0);
		return ret;
	}
}

vector<LAObject *> 
LARiskConfigurationFX::createExtraIRVolEntityBase(const LAString &fx, const LAString& ccy, LADataInstance &dataInstance, int index) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();

	// set up param for volatility shift
	MAScenarioParam param;
	param.isCalib = true;
	param.ccy = ccy;
	param.calcType = getBaseExtraCalcType(fx, index);
	param.targetCurveType = getCurveType(ccy);
	param.model = model;
	param.inputType = LAMarketData::getVolInputType(model, ccy, riskName);
	param.targetName = LAMarketData::getBaseVolatilityName(ccy);

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
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

	param.isParallel = true;
	LAString forYieldName;
	const LAString& baseForYieldName = LAMarketData::getBaseYieldName(ccy);
	forYieldName = baseForYieldName + "_" + param.calcType;
	param.refName.push_back(forYieldName);

	LAScenarioConfiguration *sceCreator
		= LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
	vector <LAObject *> sce = sceCreator->createScenario(dataInstance, param);
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
vector<LAObject *>
LARiskConfigurationFX::createBaseScenarioEntity(const LAString &fx, LADataInstance &dataInstance, int index) const
{
	if (omitNotionalExposure(fx))
	{
		return LARiskConfiguration::createFXBaseScenarioEntity(fx, dataInstance, index);
	}
	else
	{
		return std::vector<LAObject*>(0);
	}
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationFX::getBaseOutPutName(const LAString &ccy , int index) const
{
	if (omitNotionalExposure(ccy))
	{
		LAStringVector ccys = ccy.toToken(FX_DELIMITER);
		if (ccys.size() != 2) return MLIB_NO_DATA;
		LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
		LAString ret = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_OUTPUTNAME);
		ret += LAString("_BasePV");
		return ret;
	}
	else
	{
		return MLIB_NO_DATA;
	}
}

/*!
    @brief return baseshiftval

	@param[in] ccy
	@param[in] index
	@return double
*/
double
LARiskConfigurationFX::getBaseSpotVal(const LAString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationFX::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief return shift type

	@param[in] fx
	@return LAString
*/
bool
LARiskConfigurationFX::omitNotionalExposure(const LAString &ccy) const
{
	LAStringVector ccys = ccy.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return LARiskConfiguration::omitNotionalExposure(ccy);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString omitNotionalExposure = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_OMITNOTIONALEXPOSURE);
	if (omitNotionalExposure == MLIB_NO_DATA)
	{
		return false;
	}
	else
	{
		return convertBoolFromStr(omitNotionalExposure);
	}
}
