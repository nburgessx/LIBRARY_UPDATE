/*! @file
    @brief FX shift delta setup class
*/
//  2008, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftDelta.cpp
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


#include "LARiskConfigurationFXShiftDelta.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LALinearFunc.h"
#include "LADealUtils.h"
#include "LACalibrationParametersManager.h"
#include "LACalibrationParameters.h"
#include "LADefinitionsCalibration.h"
#include "LAMathFXUtility.h"
using namespace std;

// constructor
/*!

*/
LARiskConfigurationFXShiftDelta::LARiskConfigurationFXShiftDelta()
: LARiskConfigurationFXDeltaEx1()
{
}

// destructor
/*!

*/
LARiskConfigurationFXShiftDelta::~LARiskConfigurationFXShiftDelta(void)
{
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationFXShiftDelta::getBaseOutPutName(const LAString &ccy , int index) const
{
	double val = getBaseSpotVal(ccy,index);
	return LAString(ccy) + LAString("_") + LAString(val,3) + LAString("SHIFT_DirtyPrice");
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationFXShiftDelta::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationFXShiftDelta::getBaseCoefficient(const LAString &ccy) const
{
	ccy;
	return LAString("0.0:1.0:0.0");
}

/*!
    @brief get FX shiftvals
	@param [in] ccy
	@return shiftvals
*/
DoubleArray  
LARiskConfigurationFXShiftDelta::getBaseShiftVals(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_FX_FXSHIFTDELTA_SHIFTVAL);
	LADataDoubles dbs;
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
LARiskConfigurationFXShiftDelta::getBaseSpotVal(const LAString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationFXShiftDelta::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief return outputname1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXShiftDelta::getOutPutName1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
										FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_OUTPUTNAME);	
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationFXShiftDelta::getOutPutName1(const LAString &ccy , int index) const
{
	LAString outName = getOutPutName1(ccy);
	double val = getBaseSpotVal(ccy,index);
	
	return outName + LAString("_") + LAString(val,3);
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationFXShiftDelta::getRiskName(void) const
{
	return RISK_FRONT_FX_SHIFTDELTA;
}

/*!
    @brief return bump direction

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXShiftDelta::getBumpDirection(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_BUMPDIRECTION);

}

/*!
    @brief return shift type

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXShiftDelta::getShiftType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_SHIFTTYPE);

}

/*!
    @brief return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationFXShiftDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_FXSHIFTDELTA_TARGET_FX);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationFXShiftDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_FXSHIFTDELTA_CALIBRATION_TARGET_FX);
}


/*!
    @brief return divid unit

	@param[in] fx
	@return double
*/
double
LARiskConfigurationFXShiftDelta::getDivUnit(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString strDivUnit = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_DIVUNIT);
	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return shift value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationFXShiftDelta::getShiftVal(const LAString &fx, SCENARIONUM scenarioNum) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey = LAMarketData::getFXKey(ccys[0], ccys[1]);

	double shiftVal = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_SPOT_SHIFTVAL).getDoubleValue();
	if (scenarioNum == SCENARIO_1)
	{
		LAString bumpDirection = getBumpDirection(fx);
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
vector<LAObject *>
LARiskConfigurationFXShiftDelta::createBaseScenarioEntity(const LAString &fx, LADataInstance &dataInstance, int index) const
{
	return LARiskConfiguration::createFXBaseScenarioEntity(fx, dataInstance, index);
}

/*!
    @brief return Base target names

	@param[in] ccy
	@return LAStringVector
*/
LAString
LARiskConfigurationFXShiftDelta::getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	return LARiskConfiguration::getFXBaseTargetNames(ccy, dataInstance);
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] fx
	@return bool
*/
bool
LARiskConfigurationFXShiftDelta::isRiskCurrencyMode(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString proprslt = mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}

vector<LAObject *>
LARiskConfigurationFXShiftDelta::createBaseExtraScenarioEntity(const LAString &fx, LADataInstance &dataInstance, int index) const
{
	vector<LAObject *> ret(0);
	LAObjectPool &objPool = dataInstance.getObjectPool();
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

	map<LAString, LAString> FwdFXGrids;
	map<LAString, LAString> AffectedCcy;
	map<LAString, LAString> ColAffectedCcy;
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		LAString ccy_tmp = curs[i];
		FwdFXGrids[curs[i]] = mpStaticData->getStaticData(ccy_tmp.toLower() + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID);

		if(FwdFXGrids[curs[i]].isDefined() && FwdFXGrids[curs[i]] != MLIB_NO_DATA)
		{
			LAMathYieldCurvePro &ycPro_fCcy = dynamic_cast<LAMathYieldCurvePro &>
				(objPool.getObject(LAMarketData::getBaseYieldProName(ccy_tmp), ENCHKTYPE_ISDEFINED).get());
			AffectedCcy[curs[i]] = ycPro_fCcy.getAffectedCcy();
			ColAffectedCcy[curs[i]] = ycPro_fCcy.getColAffectedCcy();
		}
	}

	// Yield curve
	for(int unsigned i = 0; i < curs.size(); ++i)
	{
		LAString ccy_tmp = curs[i];
		if(FwdFXGrids[ccy_tmp].isDefined() && FwdFXGrids[ccy_tmp] != MLIB_NO_DATA)
		{
			// XccyBasis curve
			sce = createBaseExtraYieldEntity(fx, ccy_tmp, dataInstance, index);
			scemap[ccy_tmp] = sce;		

			// FwdFXConst curve
			if(ColAffectedCcy[ccy_tmp].isDefined() && ColAffectedCcy[ccy_tmp] != MLIB_NO_DATA)
			{
				sce = createBaseCollateralYieldEntity(fx, ccy_tmp, dataInstance, index);
				scemap[ccy_tmp] = sce;
			}
		}
	}

	// insert
	for (int unsigned i = 0; i < curs.size(); ++i)
	{
		const vector<LAObject *>& sce = scemap[curs[i]];
		ret.insert(ret.end(), sce.begin(), sce.end());
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
		if (FwdFXGrids[ccy_tmp].isDefined() && FwdFXGrids[ccy_tmp] != MLIB_NO_DATA && isCalibTarget(ccy_tmp))
		{
			sce = createExtraIRVolEntityBase(fx, ccy_tmp, dataInstance, index);
			ret.insert(ret.end(), sce.begin(), sce.end());
		}
	}

	// FX Vol
	if(isCalibTarget(fx))
	{
		sce = LARiskConfiguration::createBaseFXVolEntity(fx, dataInstance, index);
		ret.insert(ret.end(), sce.begin(), sce.end());
	}

	// FX Vol depending on the risk target FX
	LAStringVector SDEccys = MADealUtils::getSDECurrencys(); 
	for(unsigned int i = 0; i < SDEccys.size(); ++i)
	{
		if(SDEccys[i].findString(FX_DELIMITER) < 0 || SDEccys[i] == fx)
			continue;

		for(unsigned int j = 0; j < curs.size(); ++j)
		{
			if(!FwdFXGrids[curs[j]].isDefined() || FwdFXGrids[curs[j]] == MLIB_NO_DATA)
				continue;

			if(SDEccys[i].findString(curs[j]) > 0 && isCalibTarget(SDEccys[i]))
			{
				sce = LARiskConfiguration::createBaseFXVolEntity(SDEccys[i], dataInstance, index);
				ret.insert(ret.end(), sce.begin(), sce.end());
			}
		}
	}

	return ret;
}

LAStringVector
LARiskConfigurationFXShiftDelta::getBaseExtraTargetNames(const LAString &fx, LADataInstance &dataInstance) const
{
	LAStringVector ret;
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
	for (unsigned int i = 0; i < curs.size(); ++i)
	{
		LAString ccy_tmp = curs[i];
		if(FwdFXGrids[ccy_tmp].isDefined() && FwdFXGrids[ccy_tmp] != MLIB_NO_DATA)
		{
			const LAString yieldName = LAMarketData::getBaseYieldName(ccy_tmp);
			ret.push_back(yieldName);
		}
	}

	// IR Vol
	for (unsigned int i = 0; i < curs.size(); ++i)
	{
		LAString ccy_tmp = curs[i];
		if (FwdFXGrids[ccy_tmp].isDefined() && FwdFXGrids[ccy_tmp] != MLIB_NO_DATA && isCalibTarget(ccy_tmp))
		{
			LAString IRVolName = LAMarketData::getBaseVolatilityName(ccy_tmp);
			ret.push_back(IRVolName);
		}
	}

	// FX Vol
	if(isCalibTarget(fx))
	{
		const LAString FXVolName = LAMarketData::getBaseVolatilityName(fx);
		ret.push_back(FXVolName);
	}

	// FX Vol depending on the risk target FX
	LAStringVector SDEccys = MADealUtils::getSDECurrencys();
	for(unsigned int i = 0; i < SDEccys.size(); ++i)
	{
		if(SDEccys[i].findString(FX_DELIMITER) < 0 || SDEccys[i] == fx)
			continue;

		for(unsigned int j = 0; j < curs.size(); ++j)
		{
			if(!FwdFXGrids[curs[j]].isDefined() || FwdFXGrids[curs[j]] == MLIB_NO_DATA)
				continue;

			if(SDEccys[i].findString(curs[j]) > 0 && isCalibTarget(SDEccys[i]))
			{
				LAString FXVolName = LAMarketData::getBaseVolatilityName(SDEccys[i]);	
				ret.push_back(FXVolName);
			}
		}
	}

	return ret;
}

vector<LAObject *> 
LARiskConfigurationFXShiftDelta::createBaseExtraYieldEntity(const LAString &fx, const LAString &ccy, LADataInstance &dataInstance, int index, const bool isFirst)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	// set up param
	MAScenarioParam param;
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	LAString calcType = getBaseCalcType(fx, index);
	LAString refFX = targetFX->getName();
	refFX += "_" + calcType;
	param.refName.push_back(refFX);

	param.ccy = ccy;
	param.calcType = getBaseExtraCalcType(fx, index);
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

vector<LAObject *> 
LARiskConfigurationFXShiftDelta::createBaseCollateralYieldEntity(const LAString &fx, const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	MAScenarioParam param;
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	LAString calcType = getBaseCalcType(fx, index);
	LAString refFX = targetFX->getName();
	refFX += "_" + calcType;

	param.ccy = ccy;
	param.calcType = getBaseExtraCalcType(fx, index);
	param.targetName = LAMarketData::getBaseYieldName(ccy);
	param.isAdjustDF = isAdjustDf(ccy);
	param.isExtraFwdFXConstCurveForFXDelta = true;

	param.isParallel = true;
	LAMathYieldCurvePro &ycPro_ccy = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());
	const LAString& affectedCcy = ycPro_ccy.getColAffectedCcy();
	const LAString& baseYieldDataName_baseccy = LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(affectedCcy));
	param.refName.push_back(baseYieldDataName_baseccy);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}