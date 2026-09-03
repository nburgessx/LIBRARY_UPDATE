/*! @file
    @brief FX shift delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationFXShiftDelta.cpp
//
//  DESCRIPTION :        FX delta setup class
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


#include "AQLRiskConfigurationFXShiftDelta.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLLinearFunc.h"
#include "AQLDealUtils.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLCalibrationParameters.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLMathFXUtility.h"
using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationFXShiftDelta::AQLRiskConfigurationFXShiftDelta()
: AQLRiskConfigurationFXDeltaEx1()
{
}

// destructor
/*!

*/
AQLRiskConfigurationFXShiftDelta::~AQLRiskConfigurationFXShiftDelta(void)
{
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getBaseOutPutName(const AQLString &ccy , int index) const
{
	double val = getBaseSpotVal(ccy,index);
	return AQLString(ccy) + AQLString("_") + AQLString(val,3) + AQLString("SHIFT_DirtyPrice");
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getBaseCoefficient(const AQLString &ccy) const
{
	ccy;
	return AQLString("0.0:1.0:0.0");
}

/*!
    @brief get FX shiftvals
	@param [in] ccy
	@return shiftvals
*/
DoubleArray  
AQLRiskConfigurationFXShiftDelta::getBaseShiftVals(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_FX_FXSHIFTDELTA_SHIFTVAL);
	AQLDataDoubles dbs;
	dbs.convertFromString(strVals);
	DoubleVector ret = dbs.get();
	return ret;
}

/*!
    @brief return baseshiftval

	@param[in] ccy
	@param[in] index
	@return double
*/
double
AQLRiskConfigurationFXShiftDelta::getBaseSpotVal(const AQLString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("AQLRiskConfigurationFXShiftDelta::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
										FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_OUTPUTNAME);	
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getOutPutName1(const AQLString &ccy , int index) const
{
	AQLString outName = getOutPutName1(ccy);
	double val = getBaseSpotVal(ccy,index);
	
	return outName + AQLString("_") + AQLString(val,3);
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getRiskName(void) const
{
	return RISK_FRONT_FX_SHIFTDELTA;
}

/*!
    @brief return bump direction

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getBumpDirection(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_BUMPDIRECTION);

}

/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getShiftType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_SHIFTTYPE);

}

/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_FXSHIFTDELTA_TARGET_FX);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_FXSHIFTDELTA_CALIBRATION_TARGET_FX);
}


/*!
    @brief return divid unit

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationFXShiftDelta::getDivUnit(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString strDivUnit = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_DIVUNIT);
	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return shift value

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationFXShiftDelta::getShiftVal(const AQLString &fx, SCENARIONUM scenarioNum) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey = AQLMarketData::getFXKey(ccys[0], ccys[1]);

	double shiftVal = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_SPOT_SHIFTVAL).getDoubleValue();
	if (scenarioNum == SCENARIO_1)
	{
		AQLString bumpDirection = getBumpDirection(fx);
		bumpDirection.toUpper();
		if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
		{
			shiftVal *= -1.0;		
		}
	}
	else if (scenarioNum == SCENARIO_2)
	{
		shiftVal *= -1.0;	
	}
	else
	{
		shiftVal = 0.0;
	}
	return shiftVal;
}

/*!
    @brief set up base scenario

	@param[in] fx
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationFXShiftDelta::createBaseScenarioEntity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const
{
	return AQLRiskConfiguration::createFXBaseScenarioEntity(fx, dataInstance, index);
}

/*!
    @brief return Base target names

	@param[in] ccy
	@return AQLStringVector
*/
AQLString
AQLRiskConfigurationFXShiftDelta::getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	return AQLRiskConfiguration::getFXBaseTargetNames(ccy, dataInstance);
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] fx
	@return bool
*/
bool
AQLRiskConfigurationFXShiftDelta::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString proprslt = mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}

vector<AQLObject *>
AQLRiskConfigurationFXShiftDelta::createBaseExtraScenarioEntity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const
{
	vector<AQLObject *> ret(0);
	AQLObjectPool &objPool = dataInstance.getObjectPool();
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

	map<AQLString, AQLString> FwdFXGrids;
	map<AQLString, AQLString> AffectedCcy;
	map<AQLString, AQLString> ColAffectedCcy;
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		AQLString ccy_tmp = curs[i];
		FwdFXGrids[curs[i]] = mpStaticData->getStaticData(ccy_tmp.toLower() + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID);

		if(FwdFXGrids[curs[i]].isDefined() && FwdFXGrids[curs[i]] != AQ_NO_DATA)
		{
			AQLMathYieldCurvePro &ycPro_fCcy = dynamic_cast<AQLMathYieldCurvePro &>
				(objPool.getObject(AQLMarketData::getBaseYieldProName(ccy_tmp), ENCHKTYPE_ISDEFINED).get());
			AffectedCcy[curs[i]] = ycPro_fCcy.getAffectedCcy();
			ColAffectedCcy[curs[i]] = ycPro_fCcy.getColAffectedCcy();
		}
	}

	// Yield curve
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		AQLString ccy_tmp = curs[i];
		if(FwdFXGrids[ccy_tmp].isDefined() && FwdFXGrids[ccy_tmp] != AQ_NO_DATA)
		{
			// XccyBasis curve
			sce = createBaseExtraYieldEntity(fx, ccy_tmp, dataInstance, index);
			scemap[ccy_tmp] = sce;		

			// FwdFXConst curve
			if(ColAffectedCcy[ccy_tmp].isDefined() && ColAffectedCcy[ccy_tmp] != AQ_NO_DATA)
			{
				sce = createBaseCollateralYieldEntity(fx, ccy_tmp, dataInstance, index);
				scemap[ccy_tmp] = sce;
			}
		}
	}

	// insert
	for (int unsigned i = 0; i < curs.size(); ++i)
	{
		const vector<AQLObject *>& sce = scemap[curs[i]];
		ret.insert(ret.end(), sce.begin(), sce.end());
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
		if (FwdFXGrids[ccy_tmp].isDefined() && FwdFXGrids[ccy_tmp] != AQ_NO_DATA && isCalibTarget(ccy_tmp))
		{
			sce = createExtraIRVolEntityBase(fx, ccy_tmp, dataInstance, index);
			ret.insert(ret.end(), sce.begin(), sce.end());
		}
	}

	// FX Vol
	if(isCalibTarget(fx))
	{
		sce = AQLRiskConfiguration::createBaseFXVolEntity(fx, dataInstance, index);
		ret.insert(ret.end(), sce.begin(), sce.end());
	}

	// FX Vol depending on the risk target FX
	AQLStringVector SDEccys = AQLDealUtils::getSDECurrencys(); 
	for(unsigned int i = 0; i < SDEccys.size(); ++i)
	{
		if(SDEccys[i].findString(FX_DELIMITER) < 0 || SDEccys[i] == fx)
			continue;

		for(unsigned int j = 0; j < curs.size(); ++j)
		{
			if(!FwdFXGrids[curs[j]].isDefined() || FwdFXGrids[curs[j]] == AQ_NO_DATA)
				continue;

			if(SDEccys[i].findString(curs[j]) > 0 && isCalibTarget(SDEccys[i]))
			{
				sce = AQLRiskConfiguration::createBaseFXVolEntity(SDEccys[i], dataInstance, index);
				ret.insert(ret.end(), sce.begin(), sce.end());
			}
		}
	}

	return ret;
}

AQLStringVector
AQLRiskConfigurationFXShiftDelta::getBaseExtraTargetNames(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	AQLStringVector ret;
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
	for (unsigned int i = 0; i < curs.size(); ++i)
	{
		AQLString ccy_tmp = curs[i];
		if(FwdFXGrids[ccy_tmp].isDefined() && FwdFXGrids[ccy_tmp] != AQ_NO_DATA)
		{
			const AQLString yieldName = AQLMarketData::getBaseYieldName(ccy_tmp);
			ret.push_back(yieldName);
		}
	}

	// IR Vol
	for (unsigned int i = 0; i < curs.size(); ++i)
	{
		AQLString ccy_tmp = curs[i];
		if (FwdFXGrids[ccy_tmp].isDefined() && FwdFXGrids[ccy_tmp] != AQ_NO_DATA && isCalibTarget(ccy_tmp))
		{
			AQLString IRVolName = AQLMarketData::getBaseVolatilityName(ccy_tmp);
			ret.push_back(IRVolName);
		}
	}

	// FX Vol
	if(isCalibTarget(fx))
	{
		const AQLString FXVolName = AQLMarketData::getBaseVolatilityName(fx);
		ret.push_back(FXVolName);
	}

	// FX Vol depending on the risk target FX
	AQLStringVector SDEccys = AQLDealUtils::getSDECurrencys();
	for(unsigned int i = 0; i < SDEccys.size(); ++i)
	{
		if(SDEccys[i].findString(FX_DELIMITER) < 0 || SDEccys[i] == fx)
			continue;

		for(unsigned int j = 0; j < curs.size(); ++j)
		{
			if(!FwdFXGrids[curs[j]].isDefined() || FwdFXGrids[curs[j]] == AQ_NO_DATA)
				continue;

			if(SDEccys[i].findString(curs[j]) > 0 && isCalibTarget(SDEccys[i]))
			{
				AQLString FXVolName = AQLMarketData::getBaseVolatilityName(SDEccys[i]);	
				ret.push_back(FXVolName);
			}
		}
	}

	return ret;
}

vector<AQLObject *> 
AQLRiskConfigurationFXShiftDelta::createBaseExtraYieldEntity(const AQLString &fx, const AQLString &ccy, AQLDataInstance &dataInstance, int index, const bool isFirst)  const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	// set up param
	AQLScenarioParam param;
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLMathFXEntity *targetFX = AQLMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	AQLString calcType = getBaseCalcType(fx, index);
	AQLString refFX = targetFX->getName();
	refFX += "_" + calcType;
	param.refName.push_back(refFX);

	param.ccy = ccy;
	param.calcType = getBaseExtraCalcType(fx, index);
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

vector<AQLObject *> 
AQLRiskConfigurationFXShiftDelta::createBaseCollateralYieldEntity(const AQLString &fx, const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	AQLScenarioParam param;
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLMathFXEntity *targetFX = AQLMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	AQLString calcType = getBaseCalcType(fx, index);
	AQLString refFX = targetFX->getName();
	refFX += "_" + calcType;

	param.ccy = ccy;
	param.calcType = getBaseExtraCalcType(fx, index);
	param.targetName = AQLMarketData::getBaseYieldName(ccy);
	param.isAdjustDF = isAdjustDf(ccy);
	param.isExtraFwdFXConstCurveForFXDelta = true;

	param.isParallel = true;
	AQLMathYieldCurvePro &ycPro_ccy = dynamic_cast<AQLMathYieldCurvePro &>
						(objPool.getObject(AQLMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());
	const AQLString& affectedCcy = ycPro_ccy.getColAffectedCcy();
	const AQLString& baseYieldDataName_baseccy = AQLMarketData::getYieldDataName(objPool, AQLMarketData::getBaseYieldName(affectedCcy));
	param.refName.push_back(baseYieldDataName_baseccy);

	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}