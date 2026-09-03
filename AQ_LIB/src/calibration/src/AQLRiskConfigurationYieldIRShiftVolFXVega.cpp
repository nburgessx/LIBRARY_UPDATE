/*! @file
    @brief  FX Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationYieldIRShiftVolFXVega.cpp
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
#include "AQLRiskConfigurationYieldIRShiftVolFXVega.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLPricePortfolioValue.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLDealUtils.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLLinearFunc.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"


using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationYieldIRShiftVolFXVega::AQLRiskConfigurationYieldIRShiftVolFXVega(void)
: AQLRiskConfigurationVolFXVega()
{
}

// destructor
/*!

*/
AQLRiskConfigurationYieldIRShiftVolFXVega::~AQLRiskConfigurationYieldIRShiftVolFXVega(void)
{
}

/*!
    @brief return deltatype

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getDeltaType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_DELTATYPE + getCurveSuffix(getCrossBaseCurrency(fx)));
}

/*!
    @brief return grid term

	@param[in] fx
	@return vector<AQLString>
*/
AQLStringVector
AQLRiskConfigurationYieldIRShiftVolFXVega::getGridTerm(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString dtype = getDeltaType(fx);
	AQLStringVector grid = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_GRID + getCurveSuffix(getCrossBaseCurrency(fx))).toToken(':');
	AQLStringVector dtypes(grid.size(), dtype + '_');
	transform(dtypes.begin(), dtypes.end(), grid.begin(), grid.begin(), plus<AQLString>());
	return grid;
}

/*!
    @brief return bucket grid term

	@param[in] fx
	@return vector<AQLString>
*/
AQLStringVector
AQLRiskConfigurationYieldIRShiftVolFXVega::getBucketGridTerm(const AQLString &fx) const
{
	AQLStringVector ret;
	AQLString tmpfx = fx;
	AQLString strBucketGrid = mpRiskStaticData->getStaticData(tmpfx.toLower() + 
								FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BUCKET_GRID_TERM + getCurveSuffix(getCrossBaseCurrency(fx)));
	AQLStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == AQ_NO_DATA)
	{
		ret.push_back(AQ_NO_DATA);
		return ret;
	}
	else
	{
		AQLString dtype = getDeltaType(fx);
		AQLStringVector dtypes(BucketTerm.size(), dtype + '_');
		transform(dtypes.begin(), dtypes.end(), BucketTerm.begin(), BucketTerm.begin(), plus<AQLString>());
		AQLStringVector tmpgridTerm = getGridTerm(fx);
		unsigned int gridMax = tmpgridTerm.size();
		for (unsigned int i = 0;i < BucketTerm.size();++i)
		{

			AQLStringVector::iterator it;
			AQLString strgrid = BucketTerm[i].toUpper();
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
	@return vector<AQLString>
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getPropertyBucketGridTerm(const AQLString &fx) const
{
	AQLString tmpfx = fx;
	return mpRiskStaticData->getStaticData(tmpfx.toLower() + 
									FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BUCKET_GRID_TERM + getCurveSuffix(getCrossBaseCurrency(fx)));
}

/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_OUTPUTNAME + getCurveSuffix(getCrossBaseCurrency(fx)));
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getOutPutName1(const AQLString &ccy , int index) const
{
	AQLString outName = getOutPutName1(ccy);
	
	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	AQLString ircur = getCrossBaseCurrency(ccy);
	return outName + AQLString("_") + ircur.toUpper() + AQLString(irShiftVals[index] * 10000.0, 3);
}

/*!
    @brief return baseshiftval

	@param[in] ccy
	@param[in] index
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftVolFXVega::getBaseYieldVal(const AQLString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftVolFXVega::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief get FX shiftvals
	@param [in] ccy
	@return shiftvals
*/
DoubleArray  
AQLRiskConfigurationYieldIRShiftVolFXVega::getBaseShiftVals(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BASESHIFTVAL + getCurveSuffix(getCrossBaseCurrency(ccy)));
	return convertToRateValues(strVals.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getRiskName(void) const
{
	return RISK_FRONT_IRSHIFT_VOL_FXVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] fx
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftVolFXVega::isGridSensitivity(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISGRIDSENSITIVITY + getCurveSuffix(getCrossBaseCurrency(fx))));
}

/*!
    @brief return isparallelshift

	@param[in] fx
	@return bool 
*/
bool
AQLRiskConfigurationYieldIRShiftVolFXVega::isParallelShift(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISPARALLEL + getCurveSuffix(getCrossBaseCurrency(fx))));
}


/*!
    @brief return div unit value

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftVolFXVega::getDivUnit(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + 
								FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_DIVUNIT + getCurveSuffix(getCrossBaseCurrency(fx))).getDoubleValue();
}


/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getShiftType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_SHIFTTYPE + getCurveSuffix(getCrossBaseCurrency(fx)));

}

/*!
    @brief return bump direction

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getBumpDirection(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BUMPDIRECTION + getCurveSuffix(getCrossBaseCurrency(fx)));
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftVolFXVega::isWave(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISWAVE + getCurveSuffix(getCrossBaseCurrency(fx))));
}

/*!
    @brief  return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRSHIFTFXVEGA_TARGET_FX);
}


/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRSHIFTFXVEGA_CALIBRATION_TARGET_FX);
}


/*!
    @brief return scenario1 value

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftVolFXVega::getScenario1ShiftValue(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString keyFX =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	double shiftVal = mpRiskStaticData->getStaticData(keyFX + 
								FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_SHIFTVAL + getCurveSuffix(getCrossBaseCurrency(fx))).getDoubleValue();

	return shiftVal / 100.0;
}


/*!
    @brief return spot Currency

	@return AQLString
*/
AQLString  
AQLRiskConfigurationYieldIRShiftVolFXVega::getCrossBaseCurrency(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
							FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_CROSSBASECURRENCY);
}

/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftVolFXVega::createVolatilityEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{	
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	const AQLString fxcur = fx;
	const AQLString ircur = getCrossBaseCurrency(fx);
	const AQLString model = AQLMarketData::getModelName(fx);
 	const AQLString riskName = getRiskName();
	AQLMathYieldCurvePro &ycPro = dynamic_cast<AQLMathYieldCurvePro &>
					(objPool.getObject(AQLMarketData::getBaseYieldProName(ircur), ENCHKTYPE_ISDEFINED).get());
	const AQLStringVector& fCurveCcys = ycPro.getAffectingCcy();

	AQLString bumpDirection = getBumpDirection(fx);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}

	AQLString inputType = AQLMarketData::getVolInputType(model, fx, riskName);
	inputType.toUpper();
	if (inputType == INPUT_T_DATA_MATRIX)
	{
		throw AQLCoreInvalidData("FX vega does not support data type, now.", __FILE__, __LINE__);
	}

	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = AQLMarketData::getFXKey(ccys[0], ccys[1]);

	//create base volatility
	// scenario param
	AQLScenarioParam param;
	param.ccy = key_fx;
	param.model = model;
	param.isCalib = true;
	param.calcType = "IR" + ircur + "FX" + fxcur + "_" + riskName + "_" + AQLString(scenarioNum) + "_" + AQLString(index);
	param.shiftType = getShiftType(key_fx);
	param.bumpDirection = getBumpDirection(key_fx);
	param.targetName = AQLMarketData::getBaseVolatilityName(key_fx);
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
	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else 
	{
		param.isOutPut = false;
	}

	// set reference
	AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, fx);
	delete calibInfoCreator;
	param.refName.push_back(infoName);

	AQLObject & shiftcurve = objPool.getObject(mBaseSceNames[0],ENCHKTYPE_ISDEFINED).get();
	const AQLString yieldshiftname = dynamic_cast<const AQLDataString &>(shiftcurve.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

	AQLString dYieldName,dCalibDataName,fYieldName,fCalibDataName;

	// set yield and calibdata
	if(ccys[0] == ircur)
	{
		dYieldName = yieldshiftname;
		if (isCalibTarget(ircur))
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
			if (isCalibTarget(ircur))
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
		AQLStringVector bucketterm = getBucketGridTerm(fx);

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
			AQLString term = param.gridTerm[i];
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
	AQLScenarioConfiguration *sceCreator = 
			AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);

	delete sceCreator;
	return ret;
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getBaseOutPutName(const AQLString &ccy , int index) const
{
	double val = getBaseYieldVal(ccy,index);
	AQLString ircur = getCrossBaseCurrency(ccy);
	AQLString curve = "";
	AQLString curveType = getCurveType(ircur);
	if (curveType != STD)
	{
		curve = "-" + curveType;
	}
	return AQLString(ccy) + AQLString("_") + ircur.toUpper() + curve + AQLString(val * 10000.0 ,3) + AQLString("BP_DirtyPrice");
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getBaseCoefficient(const AQLString &ccy) const
{
	ccy;
	return AQLString("0.0:1.0:0.0");
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationYieldIRShiftVolFXVega::getShiftGridTerm(const AQLString &ccy) const
{
	AQLString crossbasecur = getCrossBaseCurrency(ccy);
	crossbasecur.toLower();

	AQLString tmpCurrency = ccy;
	AQLString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + /*"." + crossbasecur +*/  
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
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftVolFXVega::createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return createIRBaseScenarioEntity(ccy,dataInstance,index);
}
	
/*!
    @brief returnisRiskCurrencyMode
	
	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftVolFXVega::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);

	AQLString proprslt = mpRiskStaticData->getStaticData(fxKey.toLower() + 
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
AQLRiskConfigurationYieldIRShiftVolFXVega::getBaseShifts(const AQLString &ccy, int index) const
{
	return getIRBaseShifts(ccy, index);
}

/*!
    @brief return ir shift base currency

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getIRBaseCurrency(const AQLString &ccy) const
{
	return getCrossBaseCurrency(ccy);
}

/*!
    @brief return irshiftvals
	
	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRShiftVolFXVega::getIRShiftVals(const AQLString &ccy) const
{
	return getBaseShiftVals(ccy);
}

/*!
    @brief set up base extra scenario

	@param[in] ccy
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftVolFXVega::createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return createIRBaseExtraScenarioEntity(ccy,dataInstance,index);
}

/*!
    @brief return Base extra target names
	
	@param[in] ccy
	@param[in] dataInstance
	@return AQLStringVector
*/
AQLStringVector
AQLRiskConfigurationYieldIRShiftVolFXVega::getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{	
	return getIRBaseExtraTargetNames(ccy, dataInstance);
}

/*!
    @brief get base targetNames

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolFXVega::getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	AQLString basecur = getCrossBaseCurrency(ccy);
	AQLString ret = AQLMarketData::getBaseYieldName(basecur);
	return ret;
}

/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationYieldIRShiftVolFXVega::createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	return createVolatilityEntity(ccy,dataInstance,SCENARIO_1,index);
}

/*!
    @brief create scenario2 object 
	
	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationYieldIRShiftVolFXVega::createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	return createVolatilityEntity(ccy,dataInstance,SCENARIO_2,index);
}