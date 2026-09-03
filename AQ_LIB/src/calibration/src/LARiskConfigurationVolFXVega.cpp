/*! @file
    @brief  FX Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolFXVega.cpp
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
#include "LARiskConfigurationVolFXVega.h"
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
#include "LADefinitionsCalibration.h"


using namespace std;

// constructor
/*!

*/
LARiskConfigurationVolFXVega::LARiskConfigurationVolFXVega(void)
: LARiskConfigurationVolatility(true)
{
}

// destructor
/*!

*/
LARiskConfigurationVolFXVega::~LARiskConfigurationVolFXVega(void)
{
}

/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolFXVega::createVolatilityEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{	
	const LAString model = LAMarketData::getModelName(fx);
 	const LAString riskName = getRiskName();

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

	MAScenarioParam param;
	param.ccy = key_fx;
	param.model = model;
	param.isCalib = true;
	//param.calcType= ccys[0] + "_" + ccys[1] + "_" + riskName + "_" + LAString(scenarioNum) + "_" + LAString(index) ;
	param.calcType= getCalcType(fx, scenarioNum, index);
	param.shiftType = getShiftType(key_fx);
	param.bumpDirection = getBumpDirection(key_fx);
	param.targetName = LAMarketData::getBaseVolatilityName(key_fx);
	param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
	param.inputType = inputType;
	param.isWave = isWave(key_fx);
	param.extraBaseVolParam = getBaseVolVal(key_fx,index);
	param.extraBaseParam = getBaseSpotVal(key_fx,index);

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
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
	{
		param.isOutPut = true;
	}
	else 
	{
		param.isOutPut = false;
	}

	LAObjectPool &objPool = dataInstance.getObjectPool();
	// set reference
	LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, fx);
	delete calibInfoCreator;
	param.refName.push_back(infoName);
	
	LAString dYieldName = LAMarketData::getBaseYieldName(ccys[0]);
	LAString dCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dYieldName));
	LAString fYieldName = LAMarketData::getBaseYieldName(ccys[1]);
	LAString fCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, fYieldName));

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
		LAStringVector bucketterm = getBucketGridTerm(fx);

		// set shift val
		param.gridShiftVec.resize(gridSize, shiftVal);
		// set file only resize
		param.gridFile.resize(gridSize);
		unsigned int j = 0;
		IntArray gridGroupID(param.gridTerm.size());
		// set yield and calibdata
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			param.refName.push_back(dYieldName);
			param.refName.push_back(dCalibDataName);
			param.refName.push_back(fYieldName);
			param.refName.push_back(fCalibDataName);

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
	storeFXAdditionalInfo(objPool, param);


	// create scenario
	LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);

	delete sceCreator;
	return ret;
}


/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolFXVega::createVolatilityEntityOld(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum) const
{	
	const LAString model = LAMarketData::getModelName(fx);
 	const LAString riskName = getRiskName();
	LAString inputType = LAMarketData::getVolInputType(model, fx, riskName);
	LAString bumpDirection = getBumpDirection(fx);
	inputType.toUpper();
	bumpDirection.toUpper();
	
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}

	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);

	MAScenarioParam param;
	param.ccy = key_fx;
	param.model = model;
	param.shiftType = getShiftType(key_fx);
	param.bumpDirection = getBumpDirection(key_fx);
	param.targetName = LAMarketData::getBaseVolatilityName(key_fx);
	param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
	param.inputType = inputType;
	param.refName.resize(2);
	// set domestic curve name
	param.refName[0] = LAMarketData::getBaseYieldName(ccys[0]);
	// set foreign curve name
	param.refName[1] = LAMarketData::getBaseYieldName(ccys[1]);
	
	if (inputType == INPUT_T_DATA_MATRIX)
	{
		throw LACoreInvalidData("FX vega does not support data type, now.", __FILE__, __LINE__);
	}
	else
	{
		vector<LAObject *> ret(0);
		// create scenario
		LAScenarioConfiguration *sceCreator = 
				LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		// function use pattern
		if (isParallelShift(fx))
		{
			param.calcType= key_fx + "_" + riskName + "_" + LAString(scenarioNum) + "_Parallel";
			param.isParallel = true;
			// set filePath
			LAMarketData::getVolFuncFilePath(model, fx, fx, riskName, scenarioNum, param.paraFile, false);

			vector<LAObject *> sce_tmp = sceCreator->createScenario(dataInstance, param);
			if (sce_tmp.size() != 1)
			{
				throw LACoreInvalidData("Parallel shift scenaro size must be one", __FILE__, __LINE__);
			}
			ret.push_back(sce_tmp[0]);
		}
		if (isGridSensitivity(fx))
		{
			// create as parallel shift
			param.isParallel = true;
			// set grid file
			param.gridTerm = getGridTerm(fx);
			unsigned int gridSize = param.gridTerm.size();
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				param.gridTerm[i].exchange("_",".");
				param.calcType= key_fx + "_" + riskName + "_" + LAString(scenarioNum) + "_Grid_" + LAString(static_cast<int>(i));

				LAMarketData::getVolFuncFilePath(model, fx, fx, riskName, scenarioNum, param.paraFile, true, &param.gridTerm[i]);
				vector<LAObject *> sce_tmp = sceCreator->createScenario(dataInstance, param);
				if (sce_tmp.size() != 1)
				{
					throw LACoreInvalidData("Parallel shift scenaro size must be one", __FILE__, __LINE__);
				}
				ret.push_back(sce_tmp[0]);
			}
		}
		delete sceCreator;

		return ret;
	}
}


/*!
    @brief setUp targetNames

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXVega::getTargetNames(const LAString &fx, LADataInstance &dataInstance) const
{
	const LAString targetName = LAMarketData::getBaseVolatilityName(fx);

	LAString ret;
	if (isParallelShift(fx))
	{
		ret += targetName + ":";
	}

	if (isGridSensitivity(fx))
	{
		unsigned int num = getGridTerm(fx).size();
		LAStringVector bucketterm = getBucketGridTerm(fx);
		if (bucketterm[0] != MLIB_NO_DATA)
			num = bucketterm.size();
		for (unsigned int i = 0; i < num; ++i)
		{
			ret += targetName + ":";
		}
		//const unsigned int num = getGridTerm(fx).size();
		//for (unsigned int i = 0; i < num; ++i)
		//{
		//	ret += targetName + ":";
		//}
	}
	if (ret.size() < 2)
	{
		throw LACoreInvalidData("Grid nor parallel is not set. can not create targetnames!!", __FILE__, __LINE__); 
	}

	return ret.subString(0, ret.size() - 2);

}

/*!
    @brief return deltatype

	@param[in] fx
	@return LAString
*/
LAString 
LARiskConfigurationVolFXVega::getDeltaType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_DELTATYPE);
}

/*!
    @brief return grid term

	@param[in] fx
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationVolFXVega::getGridTerm(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString dtype = getDeltaType(fx);
	LAStringVector grid = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_GRID).toToken(':');
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
LARiskConfigurationVolFXVega::getBucketGridTerm(const LAString &fx) const
{
	LAStringVector ret;
	LAString tmpfx = fx;
	LAString strBucketGrid = mpRiskStaticData->getStaticData(tmpfx.toLower() + FX_KEY_RISK_FRONT_VOL_FXVEGA_BUCKET_GRID_TERM);
	LAStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == MLIB_NO_DATA)
	{
		ret.push_back(MLIB_NO_DATA);
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
LARiskConfigurationVolFXVega::getPropertyBucketGridTerm(const LAString &fx) const
{
	LAString tmpfx = fx;
	return mpRiskStaticData->getStaticData(tmpfx.toLower() + 
									FX_KEY_RISK_FRONT_VOL_FXVEGA_BUCKET_GRID_TERM);
}

/*!
    @brief return outputname1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXVega::getOutPutName1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_OUTPUTNAME);
}


/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationVolFXVega::getRiskName(void) const
{
	return RISK_FRONT_VOL_FXVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] fx
	@return bool
*/
bool
LARiskConfigurationVolFXVega::isGridSensitivity(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_ISGRIDSENSITIVITY));
}

/*!
    @brief return isparallelshift

	@param[in] fx
	@return bool 
*/
bool
LARiskConfigurationVolFXVega::isParallelShift(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_ISPARALLEL));
}


/*!
    @brief return div unit value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationVolFXVega::getDivUnit(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + 
								FX_KEY_RISK_FRONT_VOL_FXVEGA_DIVUNIT).getDoubleValue();
}


/*!
    @brief return shift type

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXVega::getShiftType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_SHIFTTYPE);

}

/*!
    @brief return bump direction

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXVega::getBumpDirection(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_BUMPDIRECTION);
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolFXVega::isWave(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_VOL_FXVEGA_ISWAVE));
}

/*!
    @brief  return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationVolFXVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_FXVEGA_TARGET_FX);
}

/*!
    @brief return scenario1 value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationVolFXVega::getScenario1ShiftValue(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString keyFX =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	double shiftVal = mpRiskStaticData->getStaticData(keyFX + 
								FX_KEY_RISK_FRONT_VOL_FXVEGA_SHIFTVAL).getDoubleValue();

	return shiftVal / 100.0;
}


/*!
    @brief return  scenario2 value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationVolFXVega::getScenario2ShiftValue(const LAString &fx) const
{
	return getScenario1ShiftValue(fx);
}

/*!
    @brief return divid unit

	@param[in] fx
	@return double
*/
bool
LARiskConfigurationVolFXVega::isRiskCurrencyMode(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_VOL_FXVEGA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);

}

/*!
    @brief calculate Forward fx, fx volatility and store to trade object

	@param[in] objPool: object pool
	@param[in] param: Scenario param
	@byproduct add data that contains forward fx and vol to tarade object. 
*/
void
LARiskConfigurationVolFXVega::storeFXAdditionalInfo(LAObjectPool &objPool, const MAScenarioParam& param) const 
{
	const LAString& riskName = param.calcType;//dynamic_cast<const LADataString &>(riskEntity.getData("Name", ISNOTNULL).get()).get();
	LAStringVector riskNameVector = riskName.toToken('_');
	size_t grifSize = param.gridTerm.size();

	// get Forward FX Rate
	DoubleArray fwdFXRates(grifSize);
	LAStringVector fxCurrencies = riskNameVector[0].toToken('/');
	if (riskNameVector[1] != "FXVEGA" || param.model == "FXSTRGLSLV") 
	{
		return;
	}
	LAMathFXEntity* pFXEntity = LAMarketData::getFXEntity(objPool, "FORWARDRATE");;

	double Term;
	for (size_t i = 0; i < grifSize; ++i)
	{
		LAString gridTerm = param.gridTerm[i].toToken('_')[1].toUpper(); // convert ; ATM_xM => xM.
		LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

		const LAString &calibInfoName = param.refName[0];
		const LAObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
		// attr for calc term
		const LAPriceDataCalendar &termCal = dynamic_cast<const LAPriceDataCalendar &>(calibInfo.getData(PRICING_DATA_TERMCALENDAR, ISNOTNULL).get());
		const LAString strTermCal = termCal.convertToString();
		const LAPriceDataSlidingRule &termSliding = dynamic_cast<const LAPriceDataSlidingRule &>(calibInfo.getData(PRICING_DATA_TERMSLIDINGRULE, ISNOTNULL).get());
		const LAPriceDataDayCount &termDC = dynamic_cast<const LAPriceDataDayCount &>(calibInfo.getData(PRICING_DATA_TERMDAYCOUNT, ISNOTNULL).get());
		const bool isIncludeLast = dynamic_cast<const LADataBool &>(calibInfo.getData(PRICING_DATA_TERMISINCLUDELAST, ISNOTNULL).get()).get();
		// spotdate for calc term
		int spotlag = dynamic_cast<const LADataInt &>(calibInfo.getData(PRICING_DATA_TERMSPOTLAG, ISNOTNULL).get());
		LADate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		LADate optionSpotDate = LAMathDateCalculations::getFXSpotDate(param.ccy, asofDate, strTermCal, spotlag, true);
		LADate settleDate = LAMathDateCalculations::getDate(optionSpotDate, gridTerm, termSliding, &termCal, true);
		LADate calcDate = LAMathDateCalculations::getFXSpotDate(param.ccy, settleDate, strTermCal, -spotlag, true);
		const double term = termDC.getTerm(asofDate, calcDate, isIncludeLast);

		fwdFXRates[i] = pFXEntity->getRate(fxCurrencies[1], fxCurrencies[0], term);
	}

	// vol ATM
	const LAString &calibInfoName = param.refName[0];
	const LAObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
	DoubleVector volATMVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOLATM, ISNOTNULL).get()).get();

	LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObject& mainTradeEntity = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED).get();

	if (dynamic_cast<const LADataValuation &>(mainTradeEntity.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
			(mainTradeEntity.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			LAObject& tradeEntity = unders.get(i).get();
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDFX_FORPRINT);
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FXVOL_FORPRINT);
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDFX_FORPRINT, new LADataDoubles(fwdFXRates));
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FXVOL_FORPRINT, new LADataDoubles(volATMVec));
		}
	}
	else
	{
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDFX_FORPRINT);
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FXVOL_FORPRINT);
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDFX_FORPRINT, new LADataDoubles(fwdFXRates));
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FXVOL_FORPRINT, new LADataDoubles(volATMVec));
	}
}
