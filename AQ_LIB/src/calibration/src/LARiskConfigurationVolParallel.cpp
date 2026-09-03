/*! @file
    @brief Volatility setup class for calc parallel shift risk
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolParallel.cpp
//
//  DESCRIPTION :       Volatility setup class for calc parallel shift risk
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


#include "LARiskConfigurationVolParallel.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLDataReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LAMarketData.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "AQLMathYieldCurve.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationVolParallel::LARiskConfigurationVolParallel(bool fxFlg)
: LARiskConfigurationVolatility(fxFlg)
{
}

// destructor
/*!

*/
LARiskConfigurationVolParallel::~LARiskConfigurationVolParallel(void)
{
}


 
/*!
    @brief setUp targetNames

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationVolParallel::getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	return LAMarketData::getBaseVolatilityName(ccy);
}

/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
LARiskConfigurationVolParallel::createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	const AQLString model = LAMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	// scenario param
	MAScenarioParam param;
	param.ccy = ccy;
	param.calcType= ccy + "_" + riskName + "_" + AQLString(scenarioNum);
	param.model = model;
	param.targetName = LAMarketData::getBaseVolatilityName(ccy);
	param.inputType = LAMarketData::getVolInputType(model, ccy, riskName);
	param.isParallel = true;
	param.isGrid = false;
	param.isCalib = isCalibTarget(ccy);
	param.targetCurveType = getCurveType(ccy);
	param.targetCurveType.toLower();
	if (param.isCalib)
	{
		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}

		param.shiftType = RISK_SHIFTTYPE_DIFF;
		param.bumpType = RISK_MARKET_BUMP;
		param.paraShiftVec.push_back(getParallelShiftVal(ccy));
		AQLString paraShiftTerm = getParallelShiftTerm(ccy);
		if (paraShiftTerm != AQ_NO_DATA)
		{ 
			param.paraTerm.push_back(paraShiftTerm);
		}

		AQLObjectPool &objPool = dataInstance.getObjectPool();
		LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
		AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
		delete calibInfoCreator;
		param.refName.push_back(infoName);
		param.refName.push_back(LAMarketData::getBaseYieldName(ccy));
	}
	else
	{
		// set file path
		LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
		param.refName.push_back(LAMarketData::getBaseYieldName(ccy));
	}
	
	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return ret;
}


/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
LARiskConfigurationVolParallel::createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	const AQLString model = LAMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	// scenario param
	MAScenarioParam param;
	param.ccy = ccy;
	param.calcType = ccy + "_" + riskName;
	param.model = model;
	param.targetName = LAMarketData::getBaseVolatilityName(ccy);
	param.inputType = LAMarketData::getVolInputType(model, ccy, riskName);
	param.isParallel = true;
	param.isGrid = false;
	// set file path
	LAMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
	// set reference
	param.refName.push_back(LAMarketData::getBaseYieldName(ccy));
	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return ret;
}


///*!
//    @brief return operator2
//
//	@return AQLString
//*/
//AQLString
//LARiskConfigurationVolParallel::getOperator2(void) const
//{
//	return AQ_NO_DATA;
//}
//
//
///*!
//    @brief return coefficient2
//
//	@param[in] ccy
//	@return AQLString
//*/
//AQLString
//LARiskConfigurationVolParallel::getCoefficient2(const AQLString &ccy) const
//{
//	ccy;
//	return AQ_NO_DATA;
//}
//
///*!
//    @brief return outputname2
//
//	@param[in] ccy
//	@return AQLString
//*/
//AQLString
//LARiskConfigurationVolParallel::getOutPutName2(const AQLString &ccy) const
//{
//	ccy;
//	return AQ_NO_DATA;
//}

/*!
    @brief return isGridSensitivity

	@param[in] ccy
	@return bool
*/
//bool
//LARiskConfigurationVolParallel::isGridSensitivity(const AQLString &ccy) const
//{
//	ccy;
//	return false;
//}
//
///*!
//    @brief return isParallelShift
//
//	@param[in] ccy
//	@return bool
//*/
//bool
//LARiskConfigurationVolParallel::isParallelShift(const AQLString &ccy) const
//{
//	ccy;
//	return true;
//}
//
///*!
//    @brief return grid term
//
//	@param[in] ccy
//	@return vector<AQLString>
//*/
//vector<AQLString>
//LARiskConfigurationVolParallel::getGridTerm(const AQLString &ccy) const
//{
//	ccy;
//	return vector<AQLString>();
//}