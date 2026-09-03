/*! @file
    @brief  FX Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRShiftVolFXVega.cpp
//
//  DESCRIPTION :         FX Vega setup class 
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
#include "LARiskConfigurationYieldIRShiftVolFXVega.h"
#include "LAString.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LAPriceDataFunction.h"
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


using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldIRShiftVolFXVega::LARiskConfigurationYieldIRShiftVolFXVega(void)
: LARiskConfigurationVolFXVega()
{
}

// destructor
/*!

*/
LARiskConfigurationYieldIRShiftVolFXVega::~LARiskConfigurationYieldIRShiftVolFXVega(void)
{
}

/*!
    @brief return deltatype

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getDeltaType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_DELTATYPE + getCurveSuffix(getCrossBaseCurrency(fx)));
}

/*!
    @brief return grid term

	@param[in] fx
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationYieldIRShiftVolFXVega::getGridTerm(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString dtype = getDeltaType(fx);
	LAStringVector grid = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_GRID + getCurveSuffix(getCrossBaseCurrency(fx))).toToken(':');
	LAStringVector dtypes(grid.size(), dtype + '_');
	transform(dtypes.begin(), dtypes.end(), grid.begin(), grid.begin(), plus<LAString>());
	return grid;
}

/*!
    @brief return bucket grid term

	@param[in] fx
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationYieldIRShiftVolFXVega::getBucketGridTerm(const LAString &fx) const
{
	LAStringVector ret;
	LAString tmpfx = fx;
	LAString strBucketGrid = mpRiskStaticData->getStaticData(tmpfx.toLower() + 
								FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BUCKET_GRID_TERM + getCurveSuffix(getCrossBaseCurrency(fx)));
	LAStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == AQ_NO_DATA)
	{
		ret.push_back(AQ_NO_DATA);
		return ret;
	}
	else
	{
		LAString dtype = getDeltaType(fx);
		LAStringVector dtypes(BucketTerm.size(), dtype + '_');
		transform(dtypes.begin(), dtypes.end(), BucketTerm.begin(), BucketTerm.begin(), plus<LAString>());
		LAStringVector tmpgridTerm = getGridTerm(fx);
		unsigned int gridMax = tmpgridTerm.size();
		for (unsigned int i = 0;i < BucketTerm.size();++i)
		{

			LAStringVector::iterator it;
			LAString strgrid = BucketTerm[i].toUpper();
			it = find(tmpgridTerm.begin(),tmpgridTerm.end(),strgrid);
			unsigned int pos = static_cast<unsigned int>(it - tmpgridTerm.begin());
			if (pos >= gridMax)
			{
				ret.push_back(BucketTerm[i]);
				break;
			}
			else
			{	
				ret.push_back(BucketTerm[i]);
			}
			if (i == BucketTerm.size() - 1 && pos < gridMax - 1)
				ret.push_back(tmpgridTerm[gridMax - 1]);
		}
		return ret;
	}
}

/*!
    @brief return property bucket grid term

	@param[in] fx
	@return vector<LAString>
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getPropertyBucketGridTerm(const LAString &fx) const
{
	LAString tmpfx = fx;
	return mpRiskStaticData->getStaticData(tmpfx.toLower() + 
									FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BUCKET_GRID_TERM + getCurveSuffix(getCrossBaseCurrency(fx)));
}

/*!
    @brief return outputname1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getOutPutName1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_OUTPUTNAME + getCurveSuffix(getCrossBaseCurrency(fx)));
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getOutPutName1(const LAString &ccy , int index) const
{
	LAString outName = getOutPutName1(ccy);
	
	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	LAString ircur = getCrossBaseCurrency(ccy);
	return outName + LAString("_") + ircur.toUpper() + LAString(irShiftVals[index] * 10000.0, 3);
}

/*!
    @brief return baseshiftval

	@param[in] ccy
	@param[in] index
	@return double
*/
double
LARiskConfigurationYieldIRShiftVolFXVega::getBaseYieldVal(const LAString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationYieldIRShiftVolFXVega::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief get FX shiftvals
	@param [in] ccy
	@return shiftvals
*/
DoubleArray  
LARiskConfigurationYieldIRShiftVolFXVega::getBaseShiftVals(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BASESHIFTVAL + getCurveSuffix(getCrossBaseCurrency(ccy)));
	return convertToRateValues(strVals.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getRiskName(void) const
{
	return RISK_FRONT_IRSHIFT_VOL_FXVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] fx
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftVolFXVega::isGridSensitivity(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISGRIDSENSITIVITY + getCurveSuffix(getCrossBaseCurrency(fx))));
}

/*!
    @brief return isparallelshift

	@param[in] fx
	@return bool 
*/
bool
LARiskConfigurationYieldIRShiftVolFXVega::isParallelShift(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISPARALLEL + getCurveSuffix(getCrossBaseCurrency(fx))));
}


/*!
    @brief return div unit value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationYieldIRShiftVolFXVega::getDivUnit(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + 
								FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_DIVUNIT + getCurveSuffix(getCrossBaseCurrency(fx))).getDoubleValue();
}


/*!
    @brief return shift type

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getShiftType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_SHIFTTYPE + getCurveSuffix(getCrossBaseCurrency(fx)));

}

/*!
    @brief return bump direction

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getBumpDirection(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BUMPDIRECTION + getCurveSuffix(getCrossBaseCurrency(fx)));
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftVolFXVega::isWave(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISWAVE + getCurveSuffix(getCrossBaseCurrency(fx))));
}

/*!
    @brief  return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRSHIFTFXVEGA_TARGET_FX);
}


/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRSHIFTFXVEGA_CALIBRATION_TARGET_FX);
}


/*!
    @brief return scenario1 value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationYieldIRShiftVolFXVega::getScenario1ShiftValue(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString keyFX =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	double shiftVal = mpRiskStaticData->getStaticData(keyFX + 
								FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_SHIFTVAL + getCurveSuffix(getCrossBaseCurrency(fx))).getDoubleValue();

	return shiftVal / 100.0;
}


/*!
    @brief return spot Currency

	@return LAString
*/
LAString  
LARiskConfigurationYieldIRShiftVolFXVega::getCrossBaseCurrency(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
							FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_CROSSBASECURRENCY);
}

/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationYieldIRShiftVolFXVega::createVolatilityEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{	
	LAObjectPool &objPool = dataInstance.getObjectPool();

	const LAString fxcur = fx;
	const LAString ircur = getCrossBaseCurrency(fx);
	const LAString model = LAMarketData::getModelName(fx);
 	const LAString riskName = getRiskName();
	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
					(objPool.getObject(LAMarketData::getBaseYieldProName(ircur), ENCHKTYPE_ISDEFINED).get());
	const LAStringVector& fCurveCcys = ycPro.getAffectingCcy();

	LAString bumpDirection = getBumpDirection(fx);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}

	LAString inputType = LAMarketData::getVolInputType(model, fx, riskName);
	inputType.toUpper();
	if (inputType == INPUT_T_DATA_MATRIX)
	{
		throw LACoreInvalidData("FX vega does not support data type, now.", __FILE__, __LINE__);
	}

	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);

	//create base volatility
	// scenario param
	MAScenarioParam param;
	param.ccy = key_fx;
	param.model = model;
	param.isCalib = true;
	param.calcType = "IR" + ircur + "FX" + fxcur + "_" + riskName + "_" + LAString(scenarioNum) + "_" + LAString(index);
	param.shiftType = getShiftType(key_fx);
	param.bumpDirection = getBumpDirection(key_fx);
	param.targetName = LAMarketData::getBaseVolatilityName(key_fx);
	param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
	param.inputType = inputType;
	param.isWave = isWave(key_fx);
	param.targetCurveType = getCurveType(ircur);
	
	double shiftVal = 0.0;
	if (scenarioNum == SCENARIO_1)
	{
		shiftVal = getScenario1ShiftValue(key_fx);
	}
	else
	{
		shiftVal = getScenario2ShiftValue(key_fx);
	}
	// dataout
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else 
	{
		param.isOutPut = false;
	}

	// set reference
	LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, fx);
	delete calibInfoCreator;
	param.refName.push_back(infoName);

	LAObject & shiftcurve = objPool.getObject(mBaseSceNames[0],ENCHKTYPE_ISDEFINED).get();
	const LAString yieldshiftname = dynamic_cast<const LADataString &>(shiftcurve.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

	LAString dYieldName,dCalibDataName,fYieldName,fCalibDataName;

	// set yield and calibdata
	if(ccys[0] == ircur)
	{
		dYieldName = yieldshiftname;
		if (isCalibTarget(ircur))
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
			if (isCalibTarget(ircur))
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

	// function use pattern
	if (isParallelShift(fx))
	{
		param.isParallel = true;	
		// set shift val
		param.paraShiftVec.resize(1, shiftVal);
		// set dmy file
		param.paraFile.push_back(CALIB_DMY_FILE);
		// set yield and calibdata
		param.refName.push_back(dYieldName);
		param.refName.push_back(dCalibDataName);
		param.refName.push_back(fYieldName);
		param.refName.push_back(fCalibDataName);

	}
	if (isGridSensitivity(fx))
	{
		// grid shift
		param.isGrid = true;
		// set grid term
		param.gridTerm = getGridTerm(fx);
		unsigned int gridSize = param.gridTerm.size();
		// set shift val
		param.gridShiftVec.resize(gridSize, shiftVal);
		// set file only resize
		param.gridFile.resize(gridSize);
		// get brid bucket term
		LAStringVector bucketterm = getBucketGridTerm(fx);

		unsigned int j = 0;
		IntArray gridGroupID(param.gridTerm.size());
		// set yield and calibdata
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			param.refName.push_back(dYieldName);
			param.refName.push_back(dCalibDataName);
			param.refName.push_back(fYieldName);
			param.refName.push_back(fCalibDataName);
			// get gridGroupID
			LAString term = param.gridTerm[i];
			gridGroupID[i] = j;
			if(j != bucketterm.size() && term == bucketterm[j])
				++j;

		}
		param.gridGroupID = gridGroupID;
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


	// create scenario
	LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);

	delete sceCreator;
	return ret;
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getBaseOutPutName(const LAString &ccy , int index) const
{
	double val = getBaseYieldVal(ccy,index);
	LAString ircur = getCrossBaseCurrency(ccy);
	LAString curve = "";
	LAString curveType = getCurveType(ircur);
	if (curveType != STD)
	{
		curve = "-" + curveType;
	}
	return LAString(ccy) + LAString("_") + ircur.toUpper() + curve + LAString(val * 10000.0 ,3) + LAString("BP_DirtyPrice");
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getBaseCoefficient(const LAString &ccy) const
{
	ccy;
	return LAString("0.0:1.0:0.0");
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldIRShiftVolFXVega::getShiftGridTerm(const LAString &ccy) const
{
	LAString crossbasecur = getCrossBaseCurrency(ccy);
	crossbasecur.toLower();

	LAString tmpCurrency = ccy;
	LAString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + /*"." + crossbasecur +*/  
									FX_KEY_RISK_FRONT_FX_IRSHIFTFXVEGA_GRID_TERM + getCurveSuffix(getCrossBaseCurrency(ccy)));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfigurationYieldIRShiftVolFXVega::createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return createIRBaseScenarioEntity(ccy,dataInstance,index);
}
	
/*!
    @brief returnisRiskCurrencyMode
	
	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftVolFXVega::isRiskCurrencyMode(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);

	LAString proprslt = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;

	return convertBoolFromStr(proprslt);
}

/*!
    @brief return baseshiftvals

	@param[in] ccy
	@param[in] index
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldIRShiftVolFXVega::getBaseShifts(const LAString &ccy, int index) const
{
	return getIRBaseShifts(ccy, index);
}

/*!
    @brief return ir shift base currency

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getIRBaseCurrency(const LAString &ccy) const
{
	return getCrossBaseCurrency(ccy);
}

/*!
    @brief return irshiftvals
	
	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldIRShiftVolFXVega::getIRShiftVals(const LAString &ccy) const
{
	return getBaseShiftVals(ccy);
}

/*!
    @brief set up base extra scenario

	@param[in] ccy
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfigurationYieldIRShiftVolFXVega::createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return createIRBaseExtraScenarioEntity(ccy,dataInstance,index);
}

/*!
    @brief return Base extra target names
	
	@param[in] ccy
	@param[in] dataInstance
	@return LAStringVector
*/
LAStringVector
LARiskConfigurationYieldIRShiftVolFXVega::getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{	
	return getIRBaseExtraTargetNames(ccy, dataInstance);
}

/*!
    @brief get base targetNames

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolFXVega::getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAString basecur = getCrossBaseCurrency(ccy);
	LAString ret = LAMarketData::getBaseYieldName(basecur);
	return ret;
}

/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYieldIRShiftVolFXVega::createScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	return createVolatilityEntity(ccy,dataInstance,SCENARIO_1,index);
}

/*!
    @brief create scenario2 object 
	
	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYieldIRShiftVolFXVega::createScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	return createVolatilityEntity(ccy,dataInstance,SCENARIO_2,index);
}