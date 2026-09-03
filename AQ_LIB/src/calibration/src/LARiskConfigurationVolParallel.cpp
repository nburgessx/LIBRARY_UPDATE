/*! @file
    @brief Volatility setup class for calc parallel shift risk
*/
//  2008, Mizuho International London.
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
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LADataReference.h"
#include "LAPriceDataFunction.h"
#include "LAPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LAMarketData.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "LAMathYieldCurve.h"

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
	@return LAString
*/
LAString
LARiskConfigurationVolParallel::getTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	return LAMarketData::getBaseVolatilityName(ccy);
}

/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolParallel::createVolatilityEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();
	// scenario param
	MAScenarioParam param;
	param.ccy = ccy;
	param.calcType= ccy + "_" + riskName + "_" + LAString(scenarioNum);
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
		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
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
		LAString paraShiftTerm = getParallelShiftTerm(ccy);
		if (paraShiftTerm != MLIB_NO_DATA)
		{ 
			param.paraTerm.push_back(paraShiftTerm);
		}

		LAObjectPool &objPool = dataInstance.getObjectPool();
		LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
		LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
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

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return ret;
}


/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolParallel::createVolatilityEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();
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

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return ret;
}


///*!
//    @brief return operator2
//
//	@return LAString
//*/
//LAString
//LARiskConfigurationVolParallel::getOperator2(void) const
//{
//	return MLIB_NO_DATA;
//}
//
//
///*!
//    @brief return coefficient2
//
//	@param[in] ccy
//	@return LAString
//*/
//LAString
//LARiskConfigurationVolParallel::getCoefficient2(const LAString &ccy) const
//{
//	ccy;
//	return MLIB_NO_DATA;
//}
//
///*!
//    @brief return outputname2
//
//	@param[in] ccy
//	@return LAString
//*/
//LAString
//LARiskConfigurationVolParallel::getOutPutName2(const LAString &ccy) const
//{
//	ccy;
//	return MLIB_NO_DATA;
//}

/*!
    @brief return isGridSensitivity

	@param[in] ccy
	@return bool
*/
//bool
//LARiskConfigurationVolParallel::isGridSensitivity(const LAString &ccy) const
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
//LARiskConfigurationVolParallel::isParallelShift(const LAString &ccy) const
//{
//	ccy;
//	return true;
//}
//
///*!
//    @brief return grid term
//
//	@param[in] ccy
//	@return vector<LAString>
//*/
//vector<LAString>
//LARiskConfigurationVolParallel::getGridTerm(const LAString &ccy) const
//{
//	ccy;
//	return vector<LAString>();
//}