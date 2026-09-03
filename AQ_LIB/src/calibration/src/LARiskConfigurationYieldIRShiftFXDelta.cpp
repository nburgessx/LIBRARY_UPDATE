/*! @file
    @brief YieldShift FX shift delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRShiftFXDelta.cpp
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


#include "LARiskConfigurationYieldIRShiftFXDelta.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LALinearFunc.h"
#include "LADealUtils.h"
#include "LACalibrationParametersManager.h"
#include "LACalibrationParameters.h"
#include "LADefinitionsCalibration.h"
using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldIRShiftFXDelta::LARiskConfigurationYieldIRShiftFXDelta()
: LARiskConfigurationFXDeltaEx1()
{
}

// destructor
/*!

*/
LARiskConfigurationYieldIRShiftFXDelta::~LARiskConfigurationYieldIRShiftFXDelta(void)
{
}



/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getBaseOutPutName(const LAString &ccy , int index) const
{
	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	LAString ircur = getCrossBaseCurrency(ccy);
	LAString curve = "";
	LAString curveType = getCurveType(ircur);
	if (curveType != STD)
	{
		curve = "-" + curveType;
	}
	return LAString(ccy) + LAString("_") + ircur.toUpper() + curve + LAString(irShiftVals[index] * 10000.0, 3) + LAString("BP_DirtyPrice");
	
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getBaseCoefficient(const LAString &ccy) const
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
LARiskConfigurationYieldIRShiftFXDelta::getBaseShiftVals(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_BASESHIFTVAL + getCurveSuffix(getCrossBaseCurrency(ccy)));
	return convertToRateValues(strVals.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return irshiftvals

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldIRShiftFXDelta::getIRShiftVals(const LAString &ccy) const
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
LARiskConfigurationYieldIRShiftFXDelta::getBaseYieldVal(const LAString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationYieldIRShiftFXDelta::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
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
LARiskConfigurationYieldIRShiftFXDelta::getBaseShifts(const LAString &ccy, int index) const
{
	return getIRBaseShifts(ccy, index);
}

/*!
    @brief get targetNames

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getTargetNames(const LAString &fx, LADataInstance &dataInstance) const
{
	return LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
}

/*!
    @brief return outputname1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getOutPutName1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
										FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_OUTPUTNAME + getCurveSuffix(getCrossBaseCurrency(fx)));	
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getOutPutName1(const LAString &ccy , int index) const
{
	LAString outName = getOutPutName1(ccy);

	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationYieldIRShiftFXDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	LAString ircur = getCrossBaseCurrency(ccy);
	return outName + LAString("_") + ircur.toUpper() + LAString(irShiftVals[index] * 10000.0, 3);
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getRiskName(void) const
{
	return RISK_FRONT_IRSHIFT_FX_DELTA;
}

/*!
    @brief return bump direction

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getBumpDirection(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_BUMPDIRECTION + getCurveSuffix(getCrossBaseCurrency(fx)));

}

/*!
    @brief return shift type

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getShiftType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_SHIFTTYPE + getCurveSuffix(getCrossBaseCurrency(fx)));

}

/*!
    @brief return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_IRSHIFTFXDELTA_TARGET_FX);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_IRSHIFTFXDELTA_CALIBRATION_TARGET_FX);
}


/*!
    @brief return divid unit

	@param[in] fx
	@return double
*/
double
LARiskConfigurationYieldIRShiftFXDelta::getDivUnit(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString strDivUnit = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_DIVUNIT + getCurveSuffix(getCrossBaseCurrency(fx)));
	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return shift value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationYieldIRShiftFXDelta::getShiftVal(const LAString &fx, SCENARIONUM scenarioNum) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey = LAMarketData::getFXKey(ccys[0], ccys[1]);
	double shiftVal = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_SPOT_SHIFTVAL + getCurveSuffix(getCrossBaseCurrency(fx))).getDoubleValue();
	if (scenarioNum == SCENARIO_1)
	{
		LAString bumpDirection = getBumpDirection(fx);
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
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldIRShiftFXDelta::getShiftGridTerm(const LAString &ccy) const
{
	LAString crossbasecur = getCrossBaseCurrency(ccy);
	crossbasecur.toLower();

	LAString tmpCurrency = ccy;
	LAString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + /*"." + crossbasecur +*/  
									FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_GRID_TERM + getCurveSuffix(getCrossBaseCurrency(ccy)));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return spot Currency

	@return LAString
*/
LAString  
LARiskConfigurationYieldIRShiftFXDelta::getCrossBaseCurrency(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
							FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_CROSSBASECURRENCY);
}

/*!
    @brief create scenario1 object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYieldIRShiftFXDelta::createScenario1Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	return LARiskConfigurationYieldIRShiftFXDelta::createFXEntity(fx, dataInstance, SCENARIO_1, index);
}

/*!
    @brief create scenario2 object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYieldIRShiftFXDelta::createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	// if scenario2 only updownshift
	LAString bumpdirection = getBumpDirection(fx);
	bumpdirection.toUpper();
	if (bumpdirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}
	return LARiskConfigurationYieldIRShiftFXDelta::createFXEntity(fx,dataInstance,SCENARIO_2,index);
}

/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationYieldIRShiftFXDelta::createVolatilityEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{

	////////////////////////////////////////set fxvol
	vector<LAObject *> ret;

	LAObjectPool& objPool = dataInstance.getObjectPool();
	
	LAString fxcur = fx;
	const LAString modelfx = LAMarketData::getModelName(fxcur);
	const LAString riskName = getRiskName();
	const LAString ircur = getCrossBaseCurrency(fxcur);
	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
					(objPool.getObject(LAMarketData::getBaseYieldProName(ircur), ENCHKTYPE_ISDEFINED).get());
	const LAStringVector& fCurveCcys = ycPro.getAffectingCcy();
		
	//yield vol object
	// set target name
	LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	
	// search shift target currency(foreign currency)
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fxcur, ccys);
	const LAStringVector &fx_ccys = targetFX->getCurrencys().get();
	LAStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
	if (it == fx_ccys.end())
	{
		LAString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());
	
	//create base volatility
	// scenario param
	MAScenarioParam param;
	param.ccy = fxcur;
	param.isCalib = true;
	param.calcType= getExtraCalcType(fx, scenarioNum, index);
	param.model = modelfx;
	param.targetName = LAMarketData::getBaseVolatilityName(fxcur);
	param.inputType = LAMarketData::getVolInputType(modelfx, fxcur, riskName);
	param.isParallel = true;
	param.isGrid = false;
	param.targetCurveType = getCurveType(ircur);

	// set reference
	LACalibrationParameters *calibInfoCreatorVolFX = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	LAString infoName = calibInfoCreatorVolFX->createCalibrationInfo(objPool, fxcur);
	delete calibInfoCreatorVolFX;
	param.refName.push_back(infoName);

	LAObject & shiftcurve = objPool.getObject(mBaseSceNames[0],ENCHKTYPE_ISDEFINED).get();
	const LAString yieldshiftname = dynamic_cast<const LADataString &>(shiftcurve.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

	LAString dYieldName,dCalibDataName,fYieldName,fCalibDataName;
	// set yield and calibdata
	if(ccys[0] == ircur)
	{
		dYieldName = yieldshiftname;
		if(isCalibTarget(ircur))
		{
			dCalibDataName = LAMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), LAMarketData::getYieldDataName(objPool, dYieldName));
		}
		else
		{
			dCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[0])));
		}
	
		// set foreign curve name
		if (fCurveCcys.size() != 0 && find(fCurveCcys.begin(), fCurveCcys.end(), ccys[1]) != fCurveCcys.end())
		{
			const LAString& forBaseYieldName = LAMarketData::getBaseYieldName(ccys[1]);
			fYieldName = forBaseYieldName + "_" + getBaseExtraCalcType(ircur, index) + "_" + param.targetCurveType + "_Parallel";
			if (isCalibTarget(ccys[1]))
			{
				fCalibDataName = LAMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), LAMarketData::getYieldDataName(objPool, fYieldName));
			}
			else
			{
				fCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[1])));
			}
		}
		else
		{
			fYieldName = LAMarketData::getBaseYieldName(ccys[1]);
			fCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, fYieldName));
		}	
	}
	else 
	{
		// set domestic curve name
		if (fCurveCcys.size() != 0 && find(fCurveCcys.begin(), fCurveCcys.end(), ccys[0]) != fCurveCcys.end())
		{
			const LAString& forBaseYieldName = LAMarketData::getBaseYieldName(ccys[0]);
			dYieldName = forBaseYieldName + "_" + getBaseExtraCalcType(ircur, index) + "_" + param.targetCurveType + "_Parallel";
			if (isCalibTarget(ccys[0]))
			{
				dCalibDataName = LAMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), LAMarketData::getYieldDataName(objPool, dYieldName));
			}
			else
			{
				dCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[0])));
			}
		}
		else
		{
			dYieldName = LAMarketData::getBaseYieldName(ccys[0]);
			dCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dYieldName));
		}
		
		// set foreign curve name
		if (ccys[1] == ircur)
		{
			fYieldName = mBaseSceNames[0];
			if(isCalibTarget(ircur))
			{
				fCalibDataName = LAMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), LAMarketData::getYieldDataName(objPool, fYieldName));
			}
			else
			{
				fCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[1])));
			}
		}
		else
		{
			const LAString& forBaseYieldName = LAMarketData::getBaseYieldName(ccys[1]);
			fYieldName = forBaseYieldName + "_" + getBaseExtraCalcType(ircur, index) + "_" + param.targetCurveType + "_Parallel";
			if (isCalibTarget(ccys[1]))
			{
				fCalibDataName = LAMarketData::getCalibDataName(getBaseExtraCalcType(ircur, index), LAMarketData::getYieldDataName(objPool, fYieldName));
			}
			else 
			{	
				fCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[1])));
			}
		}
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

	// set DDL
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
	// set file path dmy
	param.paraFile.push_back(CALIB_DMY_FILE);
	// set yield and calibdata
	param.refName.push_back(dYieldName);
	param.refName.push_back(dCalibDataName);
	param.refName.push_back(fYieldName);
	param.refName.push_back(fCalibDataName);
	// set shift value and type
	LAString shiftType = getShiftType(fx);
	shiftType.toUpper();
	param.shiftType = shiftType;
	param.extraParam.resize(1, getShiftVal(fx, scenarioNum));
	if (shiftType == RISK_SHIFTTYPE_RATIO)
	{
		param.extraParam[0] /= 100.0;
	}

	// create scenario
	LAScenarioConfiguration *sceVolCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> base_vol = sceVolCreator->createScenario(dataInstance, param);
	ret.insert(ret.end(), base_vol.begin(), base_vol.end());
	delete sceVolCreator;

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
LARiskConfigurationYieldIRShiftFXDelta::createFXEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
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
    @brief set up base scenario

	@param[in] ccy
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfigurationYieldIRShiftFXDelta::createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return LARiskConfiguration::createIRBaseScenarioEntity(ccy,dataInstance,index);
}

/*!
    @brief returnisRiskCurrencyMode
	
	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftFXDelta::isRiskCurrencyMode(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	
	LAString proprslt = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;

	return convertBoolFromStr(proprslt);
}

/*!
    @brief return ir shift base currency

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getIRBaseCurrency(const LAString &ccy) const
{
	return getCrossBaseCurrency(ccy);
}

/*!
    @brief set up base extra scenario

	@param[in] ccy
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfigurationYieldIRShiftFXDelta::createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return LARiskConfiguration::createIRBaseExtraScenarioEntity(ccy,dataInstance,index);
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@param[in] dataInstance
	@return LAStringVector
*/
LAStringVector
LARiskConfigurationYieldIRShiftFXDelta::getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	return LARiskConfiguration::getIRBaseExtraTargetNames(ccy, dataInstance);
}

/*!
    @brief return extra target names1

	@param[in] ccy
	@param[in] dataInstance
	@return LAStringVector
*/
LAStringVector 
LARiskConfigurationYieldIRShiftFXDelta::getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAStringVector ret;
	unsigned int fxSize = getCalibTargetFX(ccy, dataInstance).size();
	if (fxSize != 0)
	{
		ret.push_back(LAMarketData::getBaseVolatilityName(ccy));
	}
	return ret;
}

/*!
    @brief return extra target names1

	@param[in] ccy
	@param[in] dataInstance
	@return LAStringVector
*/
LAStringVector 
LARiskConfigurationYieldIRShiftFXDelta::getExtraTargetNames2(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return LAStringVector(0);
	}
	return LARiskConfigurationYieldIRShiftFXDelta::getExtraTargetNames1(ccy, dataInstance);
}

/*!
    @brief create extra scenario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<vector<LAObject *> >
LARiskConfigurationYieldIRShiftFXDelta::createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	vector<vector<LAObject *> > ret;

	unsigned int fxSize = getCalibTargetFX(ccy, dataInstance).size();
	if (fxSize != 0)
	{
		//create FX Vol object
		vector<LAObject* > volp = createVolatilityEntity(ccy,dataInstance,SCENARIO_1,index);
		ret.push_back(volp);
	}

	return ret;
}

/*!
    @brief create extra scenario2 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<vector<LAObject *> >
LARiskConfigurationYieldIRShiftFXDelta::createExtraScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	// if scenario2 only updownshift
	LAString bumpdirection = getBumpDirection(ccy);
	bumpdirection.toUpper();
	if (bumpdirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<vector<LAObject *> >(0);
	}

	vector<vector<LAObject *> > ret;

	unsigned int fxSize = getCalibTargetFX(ccy, dataInstance).size();
	if (fxSize != 0)
	{
		//create FX Vol object
		vector<LAObject* > volp = createVolatilityEntity(ccy,dataInstance,SCENARIO_2,index);
		ret.push_back(volp);
	}		

	return ret;
}

/*!
    @brief get base targetNames

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftFXDelta::getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAString basecur = getCrossBaseCurrency(ccy);
	LAString ret = LAMarketData::getBaseYieldName(basecur);
	return ret;
}
