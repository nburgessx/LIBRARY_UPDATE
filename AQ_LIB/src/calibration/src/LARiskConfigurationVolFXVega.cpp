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
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataFunction.h"
#include "LAPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADealUtils.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "AQLLinearFunc.h"
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
	@return vector<AQLObject *>
*/
vector<AQLObject *>
LARiskConfigurationVolFXVega::createVolatilityEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{	
	const AQLString model = LAMarketData::getModelName(fx);
 	const AQLString riskName = getRiskName();

	AQLString bumpDirection = getBumpDirection(fx);
	bumpDirection.toUpper();
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}

	AQLString inputType = LAMarketData::getVolInputType(model, fx, riskName);
	inputType.toUpper();
	if (inputType == INPUT_T_DATA_MATRIX)
	{
		throw AQLCoreInvalidData("FX vega does not support data type, now.", __FILE__, __LINE__);
	}

	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);

	MAScenarioParam param;
	param.ccy = key_fx;
	param.model = model;
	param.isCalib = true;
	//param.calcType= ccys[0] + "_" + ccys[1] + "_" + riskName + "_" + AQLString(scenarioNum) + "_" + AQLString(index) ;
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
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else 
	{
		param.isOutPut = false;
	}

	AQLObjectPool &objPool = dataInstance.getObjectPool();
	// set reference
	LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, fx);
	delete calibInfoCreator;
	param.refName.push_back(infoName);
	
	AQLString dYieldName = LAMarketData::getBaseYieldName(ccys[0]);
	AQLString dCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dYieldName));
	AQLString fYieldName = LAMarketData::getBaseYieldName(ccys[1]);
	AQLString fCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, fYieldName));

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
		AQLStringVector bucketterm = getBucketGridTerm(fx);

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
	storeFXAdditionalInfo(objPool, param);


	// create scenario
	LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);

	delete sceCreator;
	return ret;
}


/*!
    @brief create volatility object

	@param[in] fx
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
LARiskConfigurationVolFXVega::createVolatilityEntityOld(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{	
	const AQLString model = LAMarketData::getModelName(fx);
 	const AQLString riskName = getRiskName();
	AQLString inputType = LAMarketData::getVolInputType(model, fx, riskName);
	AQLString bumpDirection = getBumpDirection(fx);
	inputType.toUpper();
	bumpDirection.toUpper();
	
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}

	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);

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
		throw AQLCoreInvalidData("FX vega does not support data type, now.", __FILE__, __LINE__);
	}
	else
	{
		vector<AQLObject *> ret(0);
		// create scenario
		LAScenarioConfiguration *sceCreator = 
				LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		// function use pattern
		if (isParallelShift(fx))
		{
			param.calcType= key_fx + "_" + riskName + "_" + AQLString(scenarioNum) + "_Parallel";
			param.isParallel = true;
			// set filePath
			LAMarketData::getVolFuncFilePath(model, fx, fx, riskName, scenarioNum, param.paraFile, false);

			vector<AQLObject *> sce_tmp = sceCreator->createScenario(dataInstance, param);
			if (sce_tmp.size() != 1)
			{
				throw AQLCoreInvalidData("Parallel shift scenaro size must be one", __FILE__, __LINE__);
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
				param.calcType= key_fx + "_" + riskName + "_" + AQLString(scenarioNum) + "_Grid_" + AQLString(static_cast<int>(i));

				LAMarketData::getVolFuncFilePath(model, fx, fx, riskName, scenarioNum, param.paraFile, true, &param.gridTerm[i]);
				vector<AQLObject *> sce_tmp = sceCreator->createScenario(dataInstance, param);
				if (sce_tmp.size() != 1)
				{
					throw AQLCoreInvalidData("Parallel shift scenaro size must be one", __FILE__, __LINE__);
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
	@return AQLString
*/
AQLString
LARiskConfigurationVolFXVega::getTargetNames(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	const AQLString targetName = LAMarketData::getBaseVolatilityName(fx);

	AQLString ret;
	if (isParallelShift(fx))
	{
		ret += targetName + ":";
	}

	if (isGridSensitivity(fx))
	{
		unsigned int num = getGridTerm(fx).size();
		AQLStringVector bucketterm = getBucketGridTerm(fx);
		if (bucketterm[0] != AQ_NO_DATA)
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
		throw AQLCoreInvalidData("Grid nor parallel is not set. can not create targetnames!!", __FILE__, __LINE__); 
	}

	return ret.subString(0, ret.size() - 2);

}

/*!
    @brief return deltatype

	@param[in] fx
	@return AQLString
*/
AQLString 
LARiskConfigurationVolFXVega::getDeltaType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_DELTATYPE);
}

/*!
    @brief return grid term

	@param[in] fx
	@return vector<AQLString>
*/
AQLStringVector
LARiskConfigurationVolFXVega::getGridTerm(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString dtype = getDeltaType(fx);
	AQLStringVector grid = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_GRID).toToken(':');
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
LARiskConfigurationVolFXVega::getBucketGridTerm(const AQLString &fx) const
{
	AQLStringVector ret;
	AQLString tmpfx = fx;
	AQLString strBucketGrid = mpRiskStaticData->getStaticData(tmpfx.toLower() + FX_KEY_RISK_FRONT_VOL_FXVEGA_BUCKET_GRID_TERM);
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
LARiskConfigurationVolFXVega::getPropertyBucketGridTerm(const AQLString &fx) const
{
	AQLString tmpfx = fx;
	return mpRiskStaticData->getStaticData(tmpfx.toLower() + 
									FX_KEY_RISK_FRONT_VOL_FXVEGA_BUCKET_GRID_TERM);
}

/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationVolFXVega::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_OUTPUTNAME);
}


/*!
    @brief return riskname

	@return AQLString
*/
AQLString
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
LARiskConfigurationVolFXVega::isGridSensitivity(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_ISGRIDSENSITIVITY));
}

/*!
    @brief return isparallelshift

	@param[in] fx
	@return bool 
*/
bool
LARiskConfigurationVolFXVega::isParallelShift(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_ISPARALLEL));
}


/*!
    @brief return div unit value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationVolFXVega::getDivUnit(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + 
								FX_KEY_RISK_FRONT_VOL_FXVEGA_DIVUNIT).getDoubleValue();
}


/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationVolFXVega::getShiftType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_SHIFTTYPE);

}

/*!
    @brief return bump direction

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationVolFXVega::getBumpDirection(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVEGA_BUMPDIRECTION);
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolFXVega::isWave(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_VOL_FXVEGA_ISWAVE));
}

/*!
    @brief  return target currencies

	@return AQLString 
*/
AQLString
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
LARiskConfigurationVolFXVega::getScenario1ShiftValue(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString keyFX =  LAMarketData::getFXKey(ccys[0], ccys[1]);
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
LARiskConfigurationVolFXVega::getScenario2ShiftValue(const AQLString &fx) const
{
	return getScenario1ShiftValue(fx);
}

/*!
    @brief return divid unit

	@param[in] fx
	@return double
*/
bool
LARiskConfigurationVolFXVega::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_VOL_FXVEGA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
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
LARiskConfigurationVolFXVega::storeFXAdditionalInfo(AQLObjectPool &objPool, const MAScenarioParam& param) const 
{
	const AQLString& riskName = param.calcType;//dynamic_cast<const AQLDataString &>(riskEntity.getData("Name", ISNOTNULL).get()).get();
	AQLStringVector riskNameVector = riskName.toToken('_');
	size_t grifSize = param.gridTerm.size();

	// get Forward FX Rate
	DoubleArray fwdFXRates(grifSize);
	AQLStringVector fxCurrencies = riskNameVector[0].toToken('/');
	if (riskNameVector[1] != "FXVEGA" || param.model == "FXSTRGLSLV") 
	{
		return;
	}
	LAMathFXEntity* pFXEntity = LAMarketData::getFXEntity(objPool, "FORWARDRATE");;

	double Term;
	for (size_t i = 0; i < grifSize; ++i)
	{
		AQLString gridTerm = param.gridTerm[i].toToken('_')[1].toUpper(); // convert ; ATM_xM => xM.
		AQLDate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

		const AQLString &calibInfoName = param.refName[0];
		const AQLObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
		// attr for calc term
		const AQLPriceDataCalendar &termCal = dynamic_cast<const AQLPriceDataCalendar &>(calibInfo.getData(PRICING_DATA_TERMCALENDAR, ISNOTNULL).get());
		const AQLString strTermCal = termCal.convertToString();
		const AQLPriceDataSlidingRule &termSliding = dynamic_cast<const AQLPriceDataSlidingRule &>(calibInfo.getData(PRICING_DATA_TERMSLIDINGRULE, ISNOTNULL).get());
		const AQLPriceDataDayCount &termDC = dynamic_cast<const AQLPriceDataDayCount &>(calibInfo.getData(PRICING_DATA_TERMDAYCOUNT, ISNOTNULL).get());
		const bool isIncludeLast = dynamic_cast<const AQLDataBool &>(calibInfo.getData(PRICING_DATA_TERMISINCLUDELAST, ISNOTNULL).get()).get();
		// spotdate for calc term
		int spotlag = dynamic_cast<const AQLDataInt &>(calibInfo.getData(PRICING_DATA_TERMSPOTLAG, ISNOTNULL).get());
		AQLDate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		AQLDate optionSpotDate = LAMathDateCalculations::getFXSpotDate(param.ccy, asofDate, strTermCal, spotlag, true);
		AQLDate settleDate = LAMathDateCalculations::getDate(optionSpotDate, gridTerm, termSliding, &termCal, true);
		AQLDate calcDate = LAMathDateCalculations::getFXSpotDate(param.ccy, settleDate, strTermCal, -spotlag, true);
		const double term = termDC.getTerm(asofDate, calcDate, isIncludeLast);

		fwdFXRates[i] = pFXEntity->getRate(fxCurrencies[1], fxCurrencies[0], term);
	}

	// vol ATM
	const AQLString &calibInfoName = param.refName[0];
	const AQLObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
	DoubleVector volATMVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOLATM, ISNOTNULL).get()).get();

	AQLString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	AQLObject& mainTradeEntity = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED).get();

	if (dynamic_cast<const AQLDataValuation &>(mainTradeEntity.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
			(mainTradeEntity.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			AQLObject& tradeEntity = unders.get(i).get();
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDFX_FORPRINT);
			tradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FXVOL_FORPRINT);
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDFX_FORPRINT, new AQLDataDoubles(fwdFXRates));
			tradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FXVOL_FORPRINT, new AQLDataDoubles(volATMVec));
		}
	}
	else
	{
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDFX_FORPRINT);
		mainTradeEntity.remove(riskNameVector[0] + "_" + CALIBRATION_DATA_FXVOL_FORPRINT);
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDFX_FORPRINT, new AQLDataDoubles(fwdFXRates));
		mainTradeEntity.add(riskNameVector[0] + "_" + CALIBRATION_DATA_FXVOL_FORPRINT, new AQLDataDoubles(volATMVec));
	}
}
