#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LARiskConfigurationYieldDelta.h"
#include "AQLDataInstance.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LADefinitionsRisk.h"
#include "LAStaticData.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldDelta::LARiskConfigurationYieldDelta(void)
:LARiskConfigurationYield()
{
}

// destructor
/*!

*/
LARiskConfigurationYieldDelta::~LARiskConfigurationYieldDelta(void)
{
}

/*!
    @brief create yield object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
LARiskConfigurationYieldDelta::createYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	(void)index;
	double pShift = 0.0;
	DoubleArray gShifts;	
	if (scenarioNum == SCENARIO_1)
	{
		pShift = getScenario1ParallelShift(ccy);
		gShifts = getScenario1GridShift(ccy);
		
	}
	else
	{
		pShift = getScenario2ParallelShift(ccy);
		gShifts = getScenario2GridShift(ccy);
	}

	const AQLString model = LAMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	// set up param
	MAScenarioParam param;
	param.ccy = ccy;
	param.calcType= ccy + "_" + riskName + "_" + AQLString(scenarioNum) + "_" + AQLString(index);
	param.model = model;
	param.shiftType = RISK_SHIFTTYPE_DIFF;
	param.targetName = LAMarketData::getBaseYieldName(ccy);
	param.targetCurveType = getCurveType(ccy);

	if (isParallelShift(ccy))
	{
		param.isParallel = true;
		param.paraShiftVec = DoubleArray(1, pShift);
	}

	if (isGridSensitivity(ccy))
	{
		param.isGrid = true;
		param.gridShiftVec = gShifts;
		param.gridTerm = getGridTerm(ccy);
		param.maxIndex = getMaxGridIndex(ccy);
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELDZERO);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;	
}

 
/*!
    @brief return operator1

	@return AQLString
*/
AQLString
LARiskConfigurationYieldDelta::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_DELTA_OPERATOR1);
}


/*!
    @brief return operator2

	@return AQLString
*/
AQLString
LARiskConfigurationYieldDelta::getOperator2(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_DELTA_OPERATOR2);
}

/*!
    @brief return coefficient1
	
	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYieldDelta::getCoefficient1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_COEFFICIENT1 + getCurveSuffix(ccy));
}


/*!
    @brief return coefficient2

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYieldDelta::getCoefficient2(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_COEFFICIENT2 + getCurveSuffix(ccy));
}


/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYieldDelta::getOutPutName1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_OUTPUT1 + getCurveSuffix(ccy));
}

/*!
    @brief return outputname2

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYieldDelta::getOutPutName2(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_OUTPUT2 + getCurveSuffix(ccy));
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldDelta::isGridSensitivity(const AQLString &ccy) const
{
	(void)ccy;	
	return  convertBoolFromStr(mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_DELTA_ISGRIDSENSITIVITY));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationYieldDelta::isParallelShift(const AQLString &ccy) const
{
	(void)ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_DELTA_ISPARALLEL));
}


/*!
    @brief return riskname

	@return AQLString
*/
AQLString
LARiskConfigurationYieldDelta::getRiskName(void) const
{
	return RISK_OFFICIAL_YIELD_DELTA;
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
LARiskConfigurationYieldDelta::getGridTerm(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
LARiskConfigurationYieldDelta::getBucketGridTerm(const AQLString &ccy) const
{
	vector<AQLString> ret;
	ret.push_back(AQ_NO_DATA);

	return ret;
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldDelta::getScenario1ParallelShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strBPShift = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_PARALLEL_BPSHIFT1 + getCurveSuffix(ccy));

	double bpShift = strBPShift.getDoubleValue();
	return bpShift / 10000.0;
}

/*!
    @brief return scenario2 parallel shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldDelta::getScenario2ParallelShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strBPShift = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_PARALLEL_BPSHIFT2 + getCurveSuffix(ccy));

	double bpShift = strBPShift.getDoubleValue();
	return bpShift / 10000.0;
}

/*!
    @brief return scenario1 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldDelta::getScenario1GridShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strBPShifts = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_GRID_BPSHIFT1 + getCurveSuffix(ccy));

	return convertToRateValues(strBPShifts.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return scenario2 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldDelta::getScenario2GridShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strBPShifts = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_GRID_BPSHIFT2 + getCurveSuffix(ccy));

	return convertToRateValues(strBPShifts.toToken(MULTI_STATIC_DATA_DELIMITER));
}


/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
LARiskConfigurationYieldDelta::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_DELTA_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
LARiskConfigurationYieldDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_DELTA_TARGET_CURRENCY);
}



/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
LARiskConfigurationYieldDelta::getCalibTargetCurrencies() const
{
	if (!isRealCalib())
	{
		return AQ_NO_DATA;
	}
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_DELTA_CALIBRATION_TARGET_CURRENCY);
}

/*!
    @brief return getCoefficientRatio1

	@param[in] fx
	@return double
*/
double
LARiskConfigurationYieldDelta::getCoefficientRatio1(const AQLString &ccy, AQLObject& e, AQLDataInstance& dataInstance) const
{
	AQLString ircur = ccy;
	AQLString isadjstr = mpRiskStaticData->getStaticData(ircur.toLower() + 
												STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_ISCOEFFICIENTRATIO1 + getCurveSuffix(ccy));
	if (isadjstr == AQ_NO_DATA)
		return 1.0;
		
	return 1.0;

}

/*!
    @brief return getCoefficientRatio2

	@param[in] fx
	@return double
*/
double
LARiskConfigurationYieldDelta::getCoefficientRatio2(const AQLString &ccy, AQLObject& e, AQLDataInstance& dataInstance) const
{
	AQLString ircur = ccy;
	AQLString isadjstr = mpRiskStaticData->getStaticData(ircur.toLower() + 
													STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_ISCOEFFICIENTRATIO2 + getCurveSuffix(ccy));
	if (isadjstr == AQ_NO_DATA)
		return 1.0;

	return 1.0;
}

/*!
    @brief returnisRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldDelta::isRiskCurrencyMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}




