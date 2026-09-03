#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LARiskConfigurationYieldBasis.h"
#include "LADataInstance.h"
#include "LAObjectPool.h"
#include "LADataReference.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LADefinitionsRisk.h"
#include "LAStaticData.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldBasis::LARiskConfigurationYieldBasis(void)
:LARiskConfigurationYield()
{
	mBasisCurrency = mpStaticData->getStaticData(KEY_SDE_BASIS_BASE_CURRENCY);
	mBasisCurrency.toUpper();
}

// destructor
/*!

*/
LARiskConfigurationYieldBasis::~LARiskConfigurationYieldBasis(void)
{
}

/*!
    @brief create yield object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationYieldBasis::createYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
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

	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();
	// set up param
	MAScenarioParam param;
	param.ccy = ccy;
	param.calcType= ccy + "_" + riskName + "_" + LAString(scenarioNum) + "_" + LAString(index);
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
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELDZEROBASIS);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;	
}

/*!
    @brief return istargetccy

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationYieldBasis::isTarget(const LAString &ccy) const
{

	LAString tmpCurrency = ccy;
	if (mBasisCurrency == tmpCurrency.toUpper())
	{
		return false;
	}
	else if (tmpCurrency.findString("/") >= 0)
	{
		return false;
	}

	LAString targetccys = getTargetCurrencies();
	targetccys.toUpper();

	if (targetccys == "ALL")
	{
		return true;
	}
	else
	{
		LAStringVector targetVec = targetccys.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (targetVec.end() != find(targetVec.begin(), targetVec.end(), tmpCurrency))
		{
			return true;
		}
		else
		{
			return false;
		}
	}



}

 
/*!
    @brief return operator1

	@return LAString
*/
LAString
LARiskConfigurationYieldBasis::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_BASIS_OPERATOR1);
}


/*!
    @brief return operator2

	@return LAString
*/
LAString
LARiskConfigurationYieldBasis::getOperator2(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_BASIS_OPERATOR2);
}

/*!
    @brief return coefficient1
	
	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasis::getCoefficient1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_COEFFICIENT1);
}


/*!
    @brief return coefficient2

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasis::getCoefficient2(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_COEFFICIENT2);
}


/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasis::getOutPutName1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_OUTPUT1);
}

/*!
    @brief return outputname2

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasis::getOutPutName2(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_OUTPUT2);
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasis::isGridSensitivity(const LAString &ccy) const
{
	(void)ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_BASIS_ISGRIDSENSITIVITY));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationYieldBasis::isParallelShift(const LAString &ccy) const
{
	(void)ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_BASIS_ISPARALLEL));
}


/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationYieldBasis::getRiskName(void) const
{
	return RISK_OFFICIAL_YIELD_BASIS;
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldBasis::getGridTerm(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_TERM);

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldBasis::getBucketGridTerm(const LAString &ccy) const
{
	vector<LAString> ret;
	ret.push_back(AQ_NO_DATA);
	return ret;
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldBasis::getScenario1ParallelShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strBPShift = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_PARALLEL_BPSHIFT1);

	double bpShift = strBPShift.getDoubleValue();
	return bpShift / 10000.0;
}

/*!
    @brief return scenario2 shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldBasis::getScenario2ParallelShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strBPShift = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_PARALLEL_BPSHIFT2);

	double bpShift = strBPShift.getDoubleValue();
	return bpShift / 10000.0;
}

/*!
    @brief return scenario1 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldBasis::getScenario1GridShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strBPShifts = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_GRID_BPSHIFT1);

	return convertToRateValues(strBPShifts.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return scenario2 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldBasis::getScenario2GridShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strBPShifts = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_GRID_BPSHIFT2);

	return convertToRateValues(strBPShifts.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
LARiskConfigurationYieldBasis::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_BASIS_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldBasis::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_BASIS_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldBasis::getCalibTargetCurrencies() const
{
	if (!isRealCalib())
	{
		return AQ_NO_DATA;
	}
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_YIELD_BASIS_CALIBRATION_TARGET_CURRENCY);
}

/*!
    @brief return getCoefficientRatio1

	@param[in] fx
	@return double
*/
double
LARiskConfigurationYieldBasis::getCoefficientRatio1(const LAString &ccy, LAObject& e, LADataInstance& dataInstance) const
{
	LAString ircur = ccy;
	LAString isadjstr = mpRiskStaticData->getStaticData(ircur.toLower() + 
													STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_ISCOEFFICIENTRATIO1);
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
LARiskConfigurationYieldBasis::getCoefficientRatio2(const LAString &ccy, LAObject& e, LADataInstance& dataInstance) const
{
	LAString ircur = ccy;
	LAString isadjstr = mpRiskStaticData->getStaticData(ircur.toLower() + 
													STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_ISCOEFFICIENTRATIO2);
	if (isadjstr == AQ_NO_DATA)
		return 1.0;

	return 1.0;
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasis::isRiskCurrencyMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);

}
