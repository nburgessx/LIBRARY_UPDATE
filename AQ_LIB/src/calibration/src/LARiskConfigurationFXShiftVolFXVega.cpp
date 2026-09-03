/*! @file
    @brief  FX Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftVolFXVega.cpp
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
#include "LARiskConfigurationFXShiftVolFXVega.h"
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


using namespace std;

// constructor
/*!

*/
LARiskConfigurationFXShiftVolFXVega::LARiskConfigurationFXShiftVolFXVega(void)
: LARiskConfigurationVolFXVega()
{
}

// destructor
/*!

*/
LARiskConfigurationFXShiftVolFXVega::~LARiskConfigurationFXShiftVolFXVega(void)
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
LARiskConfigurationFXShiftVolFXVega::createVolatilityEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
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
	
	// get domain yield
	AQLString dYieldName = LAMarketData::getBaseYieldName(ccys[0]);
	//if (isFwdFXYield(ccys[0]))
	//{
	//	const AQLString calcType = getBaseExtraCalcType(fx, index);
	//	const AQLString targetCurveType = getBaseShiftCurveType(ccys[0]);
	//	const AQLString yieldName = dYieldName + "_" + calcType + "_" + targetCurveType + "_Parallel";
	//	if (objPool.find(yieldName))
	//		dYieldName = yieldName;
	//}
	AQLString dCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dYieldName));
	// get foreign yield
	AQLString fYieldName = LAMarketData::getBaseYieldName(ccys[1]);
	//if (isFwdFXYield(ccys[1]))
	//{
	//	const AQLString calcType = getBaseExtraCalcType(fx, index);
	//	const AQLString targetCurveType = getBaseShiftCurveType(ccys[1]);
	//	const AQLString yieldName = fYieldName + "_" + calcType + "_" + targetCurveType + "_Parallel";
	//	if (objPool.find(yieldName))
	//		fYieldName = yieldName;
	//}
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


	// create scenario
	LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);

	delete sceCreator;
	return ret;
}

/*!
    @brief return deltatype

	@param[in] fx
	@return AQLString
*/
AQLString 
LARiskConfigurationFXShiftVolFXVega::getDeltaType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_DELTATYPE);
}

/*!
    @brief return grid term

	@param[in] fx
	@return vector<AQLString>
*/
AQLStringVector
LARiskConfigurationFXShiftVolFXVega::getGridTerm(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString dtype = getDeltaType(fx);
	AQLStringVector grid = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_GRID).toToken(':');
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
LARiskConfigurationFXShiftVolFXVega::getBucketGridTerm(const AQLString &fx) const
{
	AQLStringVector ret;
	AQLString tmpfx = fx;
	AQLString strBucketGrid = mpRiskStaticData->getStaticData(tmpfx.toLower() + 
								FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_BUCKET_GRID_TERM);
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
LARiskConfigurationFXShiftVolFXVega::getPropertyBucketGridTerm(const AQLString &fx) const
{
	AQLString tmpfx = fx;
	return mpRiskStaticData->getStaticData(tmpfx.toLower() + 
									FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_BUCKET_GRID_TERM);
}

/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationFXShiftVolFXVega::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_OUTPUTNAME);
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
LARiskConfigurationFXShiftVolFXVega::getOutPutName1(const AQLString &ccy , int index) const
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
LARiskConfigurationFXShiftVolFXVega::getRiskName(void) const
{
	return RISK_FRONT_FXSHIFT_VOL_FXVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] fx
	@return bool
*/
bool
LARiskConfigurationFXShiftVolFXVega::isGridSensitivity(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_ISGRIDSENSITIVITY));
}

/*!
    @brief return isparallelshift

	@param[in] fx
	@return bool 
*/
bool
LARiskConfigurationFXShiftVolFXVega::isParallelShift(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_ISPARALLEL));
}


/*!
    @brief return div unit value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationFXShiftVolFXVega::getDivUnit(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + 
								FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_DIVUNIT).getDoubleValue();
}


/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationFXShiftVolFXVega::getShiftType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_SHIFTTYPE);

}

/*!
    @brief return bump direction

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationFXShiftVolFXVega::getBumpDirection(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_BUMPDIRECTION);
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationFXShiftVolFXVega::isWave(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_ISWAVE));
}

/*!
    @brief  return target currencies

	@return AQLString 
*/
AQLString
LARiskConfigurationFXShiftVolFXVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_FXSHIFTFXVEGA_TARGET_FX);
}

/*!
    @brief return scenario1 value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationFXShiftVolFXVega::getScenario1ShiftValue(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString keyFX =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	double shiftVal = mpRiskStaticData->getStaticData(keyFX + 
								FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_SHIFTVAL).getDoubleValue();

	return shiftVal / 100.0;
}



/*!
    @brief get FX shiftvals
	@param [in] ccy
	@return shiftvals
*/
DoubleArray  
LARiskConfigurationFXShiftVolFXVega::getBaseShiftVals(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_BASESHIFTVAL);
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
LARiskConfigurationFXShiftVolFXVega::getBaseSpotVal(const AQLString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("LARiskConfigurationFXShiftVolFXVega::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return AQLStringVector
*/
AQLStringVector
LARiskConfigurationFXShiftVolFXVega::getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	return LARiskConfiguration::getFXBaseExtraTargetNames(ccy, dataInstance);

	//AQLStringVector ret;
	//const AQLString targetName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	//ret.push_back(targetName);
	//return ret;
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return AQLStringVector
*/
vector<AQLObject *>
LARiskConfigurationFXShiftVolFXVega::createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return LARiskConfiguration::createBaseFXEntity(ccy, dataInstance, index);

	//vector<AQLObject *> ret;

	//const AQLString model = LAMarketData::getModelName(ccy);
	//const AQLString riskName = getRiskName();
	//// scenario param
	//MAScenarioParam param;
	//param.ccy = ccy;
	//param.shiftType = RISK_BUMPDIRECTION_UPSHIFT;
	//param.model = model;
	//param.isParallel = true;
	//param.isGrid = false;
	//// set target name
	//LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	//param.targetName = targetFX->getName();

	//// search shift target currency(foreign currency)
	//AQLStringVector ccys;
	//LAMarketData::convertToCurrency(ccy, ccys);
	//const AQLStringVector &fx_ccys = targetFX->getCurrencys().get();
	//AQLStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
	//if (it == fx_ccys.end())
	//{
	//	AQLString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
	//	throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//}
	//unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());
	//it = find(fx_ccys.begin(), fx_ccys.end(), ccys[0]);
	//if (it == fx_ccys.end())
	//{
	//	AQLString msg = "FX object ccy is not registed  ccy = " + ccys[0].toUpper();
	//	throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//}
	//unsigned int ccy0pos = static_cast<unsigned int>(it - fx_ccys.begin());
	//const DoubleArray& spotrates = targetFX->getSpotRates().get();
	//double ccy0ratio = spotrates[ccy0pos];


	//// set shift value
	//param.paraShiftVec.resize(fx_ccys.size(), 0.0);
	//
	////set base shift value
	//double baseShifVal = getBaseSpotVal(ccy, index);
	//param.extraBaseParamVec.resize(fx_ccys.size(), 0.0);
	//param.extraBaseParamVec[pos] = baseShifVal;
	//param.extraBaseParamVec[pos] *= ccy0ratio;
	//
	//// set reference
	//AQLString refNameStr = targetFX->getData(IR_MODEL_DATA_YIELDCURVES, ISNOTNULL).convertToString();
	//refNameStr.exchange("\"", "");
	//param.refName = refNameStr.toToken(MULTI_STATIC_DATA_DELIMITER);

	//// create scenario
	//LAScenarioConfiguration *sceCreator = 
	//	LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_FX);


	//param.calcType= ccy + "_" + riskName + "_BaseShift_" + AQLString(index) + "_ExtraBaseSce";
	//ret = sceCreator->createScenario(dataInstance, param);
	//delete sceCreator;
	//return ret;
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
LARiskConfigurationFXShiftVolFXVega::getBaseOutPutName(const AQLString &ccy , int index) const
{
	double val = getBaseSpotVal(ccy,index);
	return AQLString(ccy) + AQLString("_") + AQLString(val,3) + AQLString("SHIFT_DirtyPrice");
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
LARiskConfigurationFXShiftVolFXVega::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
LARiskConfigurationFXShiftVolFXVega::getBaseCoefficient(const AQLString &ccy) const
{
	ccy;
	return AQLString("0.0:1.0:0.0");
}

/*!
    @brief return extra scenario1 target names

	@param[in] ccy
	@return DoubleArray
*/
//AQLStringVector
//LARiskConfigurationFXShiftVolFXVega::getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const
//{
//	const AQLString targetName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
//	unsigned int num = 0;
//
//	if(isParallelShift(ccy))
//		num += 1;
//	if(isGridSensitivity(ccy))
//	{
//		AQLStringVector bucketterm = getBucketGridTerm(ccy);
//		if (bucketterm[0] != AQ_NO_DATA)
//			num += bucketterm.size();
//		else
//			num += getGridTerm(ccy).size();
//	}
//	AQLString str;
//	for (unsigned int i = 0 ; i < num; i++)
//		str += targetName + ":";
//
//	str = str.subString(0,str.size()-2);
//	AQLStringVector ret(1,str);
//	return ret;
//}

/*!
    @brief return extra scenario2 target names

	@param[in] ccy
	@return DoubleArray
*/
//AQLStringVector
//LARiskConfigurationFXShiftVolFXVega::getExtraTargetNames2(const AQLString &ccy, AQLDataInstance &dataInstance) const
//{
//	AQLString bumpDirection = getBumpDirection(ccy);
//	bumpDirection.toUpper();
//	// if scenario2 only updownshift
//	if (bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
//	{
//		return AQLStringVector(0);
//	}
//
//	return getExtraTargetNames1(ccy, dataInstance);
//}

/*!
    @brief create extra senario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
//vector<vector<AQLObject *> > 
//LARiskConfigurationFXShiftVolFXVega::createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
//{
//
//	 vector<vector<AQLObject *> > ret = LARiskConfiguration::createGridFXEntity(ccy,dataInstance,SCENARIO_1,index);
//	 return ret;
//}

/*!
    @brief create extra senario2 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
//vector<vector<AQLObject *> > 
//LARiskConfigurationFXShiftVolFXVega::createExtraScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
//{
//	vector<vector<AQLObject *> > ret = LARiskConfiguration::createGridFXEntity(ccy,dataInstance,SCENARIO_2,index);
//	return ret;
//}

/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
LARiskConfigurationFXShiftVolFXVega::createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return LARiskConfiguration::createBaseFXVolEntity(ccy, dataInstance, index);

	//const AQLString model = LAMarketData::getModelName(ccy);
	//const AQLString riskName = getRiskName();
	//// set target name
	//LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	//// search shift target currency(foreign currency)
	//AQLStringVector ccys;
	//LAMarketData::convertToCurrency(ccy, ccys);
	//const AQLStringVector &fx_ccys = targetFX->getCurrencys().get();
	//AQLStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
	//if (it == fx_ccys.end())
	//{
	//	AQLString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
	//	throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//}
	//unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());
	//
	//MAScenarioParam paramvol;
	//paramvol.ccy = ccy;
	//paramvol.isCalib = true;
	//paramvol.calcType= ccys[0] + "_" + ccys[1] + "_" + riskName + "_BaseShift_" + AQLString(index);
	//paramvol.model = model;
	//paramvol.targetName = LAMarketData::getBaseVolatilityName(ccy);
	//paramvol.inputType = LAMarketData::getVolInputType(model, ccy, riskName);
	//paramvol.isParallel = true;
	//paramvol.isGrid = false;

	//AQLObjectPool &objPool = dataInstance.getObjectPool();
	//// set reference
	//LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(paramvol.model);
	//AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
	//delete calibInfoCreator;
	//paramvol.refName.push_back(infoName);

	//// set yield and calibdata
	//paramvol.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
	//paramvol.refName.push_back(LAMarketData::getCalibDataName(PV, LAMarketData::getYieldDataName(objPool, paramvol.refName.back())));
	//paramvol.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
	//paramvol.refName.push_back(LAMarketData::getCalibDataName(PV, LAMarketData::getYieldDataName(objPool, paramvol.refName.back())));

	//// dataout
	//if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	//{
	//	paramvol.isOutPut = true;
	//}
	//else 
	//{
	//	paramvol.isOutPut = false;
	//}

	//// set DDL
	//AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	//paramvol.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
	//// set file path dmy
	//paramvol.paraFile.push_back(CALIB_DMY_FILE);
	//// set shift value and type
	//paramvol.shiftType = RISK_BUMPDIRECTION_UPSHIFT;
	//paramvol.extraBaseParam = getBaseSpotVal(ccy, index);
	//// create scenario
	//LAScenarioConfiguration *sceVolCreator = 
	//	LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	//vector<AQLObject *> ret = sceVolCreator->createScenario(dataInstance, paramvol);
	//delete sceVolCreator;

	//return ret;
}


/*!
    @brief returnisRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationFXShiftVolFXVega::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);

	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}