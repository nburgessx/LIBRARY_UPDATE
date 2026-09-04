/*! @file
    @brief Volatility setup class for calc parallel shift risk
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRiskConfigurationVolParallel.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLDataReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLPricePortfolioValue.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLMarketData.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLMathYieldCurve.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationVolParallel::AQLRiskConfigurationVolParallel(bool fxFlg)
: AQLRiskConfigurationVolatility(fxFlg)
{
}

// destructor
/*!

*/
AQLRiskConfigurationVolParallel::~AQLRiskConfigurationVolParallel(void)
{
}


 
/*!
    @brief setUp targetNames

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolParallel::getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	return AQLMarketData::getBaseVolatilityName(ccy);
}

/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
AQLRiskConfigurationVolParallel::createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	const AQLString model = AQLMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	// scenario param
	AQLScenarioParam param;
	param.ccy = ccy;
	param.calcType= ccy + "_" + riskName + "_" + AQLString(scenarioNum);
	param.model = model;
	param.targetName = AQLMarketData::getBaseVolatilityName(ccy);
	param.inputType = AQLMarketData::getVolInputType(model, ccy, riskName);
	param.isParallel = true;
	param.isGrid = false;
	param.isCalib = isCalibTarget(ccy);
	param.targetCurveType = getCurveType(ccy);
	param.targetCurveType.toLower();
	if (param.isCalib)
	{
		if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
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
		AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
		AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
		delete calibInfoCreator;
		param.refName.push_back(infoName);
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccy));
	}
	else
	{
		// set file path
		AQLMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
		param.refName.push_back(AQLMarketData::getBaseYieldName(ccy));
	}
	
	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

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
AQLRiskConfigurationVolParallel::createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	const AQLString model = AQLMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	// scenario param
	AQLScenarioParam param;
	param.ccy = ccy;
	param.calcType = ccy + "_" + riskName;
	param.model = model;
	param.targetName = AQLMarketData::getBaseVolatilityName(ccy);
	param.inputType = AQLMarketData::getVolInputType(model, ccy, riskName);
	param.isParallel = true;
	param.isGrid = false;
	// set file path
	AQLMarketData::getVolFuncFilePath(model, ccy, ccy, riskName, scenarioNum, param.paraFile, false);
	// set reference
	param.refName.push_back(AQLMarketData::getBaseYieldName(ccy));
	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

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
//AQLRiskConfigurationVolParallel::getOperator2(void) const
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
//AQLRiskConfigurationVolParallel::getCoefficient2(const AQLString &ccy) const
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
//AQLRiskConfigurationVolParallel::getOutPutName2(const AQLString &ccy) const
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
//AQLRiskConfigurationVolParallel::isGridSensitivity(const AQLString &ccy) const
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
//AQLRiskConfigurationVolParallel::isParallelShift(const AQLString &ccy) const
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
//AQLRiskConfigurationVolParallel::getGridTerm(const AQLString &ccy) const
//{
//	ccy;
//	return vector<AQLString>();
//}