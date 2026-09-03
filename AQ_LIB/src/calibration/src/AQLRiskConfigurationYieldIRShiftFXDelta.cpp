/*! @file
    @brief YieldShift FX shift delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationYieldIRShiftFXDelta.cpp
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


#include "AQLRiskConfigurationYieldIRShiftFXDelta.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLLinearFunc.h"
#include "AQLDealUtils.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLCalibrationParameters.h"
#include "AQLDefinitionsCalibration.h"
using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationYieldIRShiftFXDelta::AQLRiskConfigurationYieldIRShiftFXDelta()
: AQLRiskConfigurationFXDeltaEx1()
{
}

// destructor
/*!

*/
AQLRiskConfigurationYieldIRShiftFXDelta::~AQLRiskConfigurationYieldIRShiftFXDelta(void)
{
}



/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getBaseOutPutName(const AQLString &ccy , int index) const
{
	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	AQLString ircur = getCrossBaseCurrency(ccy);
	AQLString curve = "";
	AQLString curveType = getCurveType(ircur);
	if (curveType != STD)
	{
		curve = "-" + curveType;
	}
	return AQLString(ccy) + AQLString("_") + ircur.toUpper() + curve + AQLString(irShiftVals[index] * 10000.0, 3) + AQLString("BP_DirtyPrice");
	
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getBaseCoefficient(const AQLString &ccy) const
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
AQLRiskConfigurationYieldIRShiftFXDelta::getBaseShiftVals(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_BASESHIFTVAL + getCurveSuffix(getCrossBaseCurrency(ccy)));
	return convertToRateValues(strVals.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return irshiftvals

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRShiftFXDelta::getIRShiftVals(const AQLString &ccy) const
{
	return getBaseShiftVals(ccy);
}

/*!
    @brief return baseshiftval

	@param[in] ccy
	@param[in] index
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftFXDelta::getBaseYieldVal(const AQLString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftFXDelta::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief return baseshiftvals

	@param[in] ccy
	@param[in] index
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRShiftFXDelta::getBaseShifts(const AQLString &ccy, int index) const
{
	return getIRBaseShifts(ccy, index);
}

/*!
    @brief get targetNames

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getTargetNames(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	return AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
}

/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
										FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_OUTPUTNAME + getCurveSuffix(getCrossBaseCurrency(fx)));	
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getOutPutName1(const AQLString &ccy , int index) const
{
	AQLString outName = getOutPutName1(ccy);

	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftFXDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	AQLString ircur = getCrossBaseCurrency(ccy);
	return outName + AQLString("_") + ircur.toUpper() + AQLString(irShiftVals[index] * 10000.0, 3);
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getRiskName(void) const
{
	return RISK_FRONT_IRSHIFT_FX_DELTA;
}

/*!
    @brief return bump direction

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getBumpDirection(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_BUMPDIRECTION + getCurveSuffix(getCrossBaseCurrency(fx)));

}

/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getShiftType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_SHIFTTYPE + getCurveSuffix(getCrossBaseCurrency(fx)));

}

/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_IRSHIFTFXDELTA_TARGET_FX);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_IRSHIFTFXDELTA_CALIBRATION_TARGET_FX);
}


/*!
    @brief return divid unit

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftFXDelta::getDivUnit(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString strDivUnit = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_DIVUNIT + getCurveSuffix(getCrossBaseCurrency(fx)));
	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return shift value

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftFXDelta::getShiftVal(const AQLString &fx, SCENARIONUM scenarioNum) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	double shiftVal = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_SPOT_SHIFTVAL + getCurveSuffix(getCrossBaseCurrency(fx))).getDoubleValue();
	if (scenarioNum == SCENARIO_1)
	{
		AQLString bumpDirection = getBumpDirection(fx);
		bumpDirection.toUpper();
		if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
		{
			shiftVal *= -1.0;		
		}
	}
	else
	{
		shiftVal *= -1.0;	
	}
	return shiftVal;
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationYieldIRShiftFXDelta::getShiftGridTerm(const AQLString &ccy) const
{
	AQLString crossbasecur = getCrossBaseCurrency(ccy);
	crossbasecur.toLower();

	AQLString tmpCurrency = ccy;
	AQLString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + /*"." + crossbasecur +*/  
									FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_GRID_TERM + getCurveSuffix(getCrossBaseCurrency(ccy)));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return spot Currency

	@return AQLString
*/
AQLString  
AQLRiskConfigurationYieldIRShiftFXDelta::getCrossBaseCurrency(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
							FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_CROSSBASECURRENCY);
}

/*!
    @brief create scenario1 object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationYieldIRShiftFXDelta::createScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	return AQLRiskConfigurationYieldIRShiftFXDelta::createFXEntity(fx, dataInstance, SCENARIO_1, index);
}

/*!
    @brief create scenario2 object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationYieldIRShiftFXDelta::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	// if scenario2 only updownshift
	AQLString bumpdirection = getBumpDirection(fx);
	bumpdirection.toUpper();
	if (bumpdirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}
	return AQLRiskConfigurationYieldIRShiftFXDelta::createFXEntity(fx,dataInstance,SCENARIO_2,index);
}

/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftFXDelta::createVolatilityEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{

	////////////////////////////////////////set fxvol
	vector<AQLObject *> ret;

	AQLObjectPool& objPool = dataInstance.getObjectPool();
	
	AQLString fxcur = fx;
	const AQLString modelfx = AQLMarketData::getModelName(fxcur);
	const AQLString riskName = getRiskName();
	const AQLString ircur = getCrossBaseCurrency(fxcur);
	AQLMathYieldCurvePro &ycPro = dynamic_cast<AQLMathYieldCurvePro &>
					(objPool.getObject(AQLMarketData::getBaseYieldProName(ircur), ENCHKTYPE_ISDEFINED).get());
	const AQLStringVector& fCurveCcys = ycPro.getAffectingCcy();
		
	//yield vol object
	// set target name
	AQLMathFXEntity *targetFX = AQLMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	
	// search shift target currency(foreign currency)
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(fxcur, ccys);
	const AQLStringVector &fx_ccys = targetFX->getCurrencys().get();
	AQLStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
	if (it == fx_ccys.end())
	{
		AQLString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());
	
	//create base volatility
	// scenario param
	AQLScenarioParam param;
	param.ccy = fxcur;
	param.isCalib = true;
	param.calcType= getExtraCalcType(fx, scenarioNum, index);
	param.model = modelfx;
	param.targetName = AQLMarketData::getBaseVolatilityName(fxcur);
	param.inputType = AQLMarketData::getVolInputType(modelfx, fxcur, riskName);
	param.isParallel = true;
	param.isGrid = false;
	param.targetCurveType = getCurveType(ircur);

	// set reference
	AQLCalibrationParameters *calibInfoCreatorVolFX = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	AQLString infoName = calibInfoCreatorVolFX->createCalibrationInfo(objPool, fxcur);
	delete calibInfoCreatorVolFX;
	param.refName.push_back(infoName);

	AQLObject & shiftcurve = objPool.getObject(mBaseSceNames[0],ENCHKTYPE_ISDEFINED).get();
	const AQLString yieldshiftname = dynamic_cast<const AQLDataString &>(shiftcurve.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

	AQLString dYieldName,dCalibDataName,fYieldName,fCalibDataName;
	// set yield and calibdata
	if(ccys[0] == ircur)
	{
		dYieldName = yieldshiftname;
		if(isCalibTarget(ircur))
		{
			dCalibDataName = AQLMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), AQLMarketData::getYieldDataName(objPool, dYieldName));
		}
		else
		{
			dCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, AQLMarketData::getBaseYieldName(ccys[0])));
		}
	
		// set foreign curve name
		if (fCurveCcys.size() != 0 && find(fCurveCcys.begin(), fCurveCcys.end(), ccys[1]) != fCurveCcys.end())
		{
			const AQLString& forBaseYieldName = AQLMarketData::getBaseYieldName(ccys[1]);
			fYieldName = forBaseYieldName + "_" + getBaseExtraCalcType(ircur, index) + "_" + param.targetCurveType + "_Parallel";
			if (isCalibTarget(ccys[1]))
			{
				fCalibDataName = AQLMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), AQLMarketData::getYieldDataName(objPool, fYieldName));
			}
			else
			{
				fCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, AQLMarketData::getBaseYieldName(ccys[1])));
			}
		}
		else
		{
			fYieldName = AQLMarketData::getBaseYieldName(ccys[1]);
			fCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, fYieldName));
		}	
	}
	else 
	{
		// set domestic curve name
		if (fCurveCcys.size() != 0 && find(fCurveCcys.begin(), fCurveCcys.end(), ccys[0]) != fCurveCcys.end())
		{
			const AQLString& forBaseYieldName = AQLMarketData::getBaseYieldName(ccys[0]);
			dYieldName = forBaseYieldName + "_" + getBaseExtraCalcType(ircur, index) + "_" + param.targetCurveType + "_Parallel";
			if (isCalibTarget(ccys[0]))
			{
				dCalibDataName = AQLMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), AQLMarketData::getYieldDataName(objPool, dYieldName));
			}
			else
			{
				dCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, AQLMarketData::getBaseYieldName(ccys[0])));
			}
		}
		else
		{
			dYieldName = AQLMarketData::getBaseYieldName(ccys[0]);
			dCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, dYieldName));
		}
		
		// set foreign curve name
		if (ccys[1] == ircur)
		{
			fYieldName = mBaseSceNames[0];
			if(isCalibTarget(ircur))
			{
				fCalibDataName = AQLMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), AQLMarketData::getYieldDataName(objPool, fYieldName));
			}
			else
			{
				fCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, AQLMarketData::getBaseYieldName(ccys[1])));
			}
		}
		else
		{
			const AQLString& forBaseYieldName = AQLMarketData::getBaseYieldName(ccys[1]);
			fYieldName = forBaseYieldName + "_" + getBaseExtraCalcType(ircur, index) + "_" + param.targetCurveType + "_Parallel";
			if (isCalibTarget(ccys[1]))
			{
				fCalibDataName = AQLMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), AQLMarketData::getYieldDataName(objPool, fYieldName));
			}
			else 
			{	
				fCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, AQLMarketData::getBaseYieldName(ccys[1])));
			}
		}
	}

	// dataout
	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else 
	{
		param.isOutPut = false;
	}

	// set DDL
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
	// set file path dmy
	param.paraFile.push_back(CALIB_DMY_FILE);
	// set yield and calibdata
	param.refName.push_back(dYieldName);
	param.refName.push_back(dCalibDataName);
	param.refName.push_back(fYieldName);
	param.refName.push_back(fCalibDataName);
	// set shift value and type
	AQLString shiftType = getShiftType(fx);
	shiftType.toUpper();
	param.shiftType = shiftType;
	param.extraParam.resize(1, getShiftVal(fx, scenarioNum));
	if (shiftType == RISK_SHIFTTYPE_RATIO)
	{
		param.extraParam[0] /= 100.0;
	}

	// create scenario
	AQLScenarioConfiguration *sceVolCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> base_vol = sceVolCreator->createScenario(dataInstance, param);
	ret.insert(ret.end(), base_vol.begin(), base_vol.end());
	delete sceVolCreator;

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
AQLRiskConfigurationYieldIRShiftFXDelta::createFXEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
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
    @brief set up base scenario

	@param[in] ccy
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftFXDelta::createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return AQLRiskConfiguration::createIRBaseScenarioEntity(ccy,dataInstance,index);
}

/*!
    @brief returnisRiskCurrencyMode
	
	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftFXDelta::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	
	AQLString proprslt = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;

	return convertBoolFromStr(proprslt);
}

/*!
    @brief return ir shift base currency

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getIRBaseCurrency(const AQLString &ccy) const
{
	return getCrossBaseCurrency(ccy);
}

/*!
    @brief set up base extra scenario

	@param[in] ccy
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftFXDelta::createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return AQLRiskConfiguration::createIRBaseExtraScenarioEntity(ccy,dataInstance,index);
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@param[in] dataInstance
	@return AQLStringVector
*/
AQLStringVector
AQLRiskConfigurationYieldIRShiftFXDelta::getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	return AQLRiskConfiguration::getIRBaseExtraTargetNames(ccy, dataInstance);
}

/*!
    @brief return extra target names1

	@param[in] ccy
	@param[in] dataInstance
	@return AQLStringVector
*/
AQLStringVector 
AQLRiskConfigurationYieldIRShiftFXDelta::getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	AQLStringVector ret;
	unsigned int fxSize = getCalibTargetFX(ccy, dataInstance).size();
	if (fxSize != 0)
	{
		ret.push_back(AQLMarketData::getBaseVolatilityName(ccy));
	}
	return ret;
}

/*!
    @brief return extra target names1

	@param[in] ccy
	@param[in] dataInstance
	@return AQLStringVector
*/
AQLStringVector 
AQLRiskConfigurationYieldIRShiftFXDelta::getExtraTargetNames2(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return AQLStringVector(0);
	}
	return AQLRiskConfigurationYieldIRShiftFXDelta::getExtraTargetNames1(ccy, dataInstance);
}

/*!
    @brief create extra scenario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *> 
*/
vector<vector<AQLObject *> >
AQLRiskConfigurationYieldIRShiftFXDelta::createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	vector<vector<AQLObject *> > ret;

	unsigned int fxSize = getCalibTargetFX(ccy, dataInstance).size();
	if (fxSize != 0)
	{
		//create FX Vol object
		vector<AQLObject* > volp = createVolatilityEntity(ccy,dataInstance,SCENARIO_1,index);
		ret.push_back(volp);
	}

	return ret;
}

/*!
    @brief create extra scenario2 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *> 
*/
vector<vector<AQLObject *> >
AQLRiskConfigurationYieldIRShiftFXDelta::createExtraScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	// if scenario2 only updownshift
	AQLString bumpdirection = getBumpDirection(ccy);
	bumpdirection.toUpper();
	if (bumpdirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<vector<AQLObject *> >(0);
	}

	vector<vector<AQLObject *> > ret;

	unsigned int fxSize = getCalibTargetFX(ccy, dataInstance).size();
	if (fxSize != 0)
	{
		//create FX Vol object
		vector<AQLObject* > volp = createVolatilityEntity(ccy,dataInstance,SCENARIO_2,index);
		ret.push_back(volp);
	}		

	return ret;
}

/*!
    @brief get base targetNames

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftFXDelta::getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	AQLString basecur = getCrossBaseCurrency(ccy);
	AQLString ret = AQLMarketData::getBaseYieldName(basecur);
	return ret;
}
