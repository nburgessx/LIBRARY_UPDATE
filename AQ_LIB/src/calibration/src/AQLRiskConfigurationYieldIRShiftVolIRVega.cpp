/*! @file
    @brief  IR Vega setup class 
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "AQLRiskConfigurationYieldIRShiftVolIRVega.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
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
AQLRiskConfigurationYieldIRShiftVolIRVega::AQLRiskConfigurationYieldIRShiftVolIRVega(void)
: AQLRiskConfigurationVolIRVega()
{
}

// destructor
/*!

*/
AQLRiskConfigurationYieldIRShiftVolIRVega::~AQLRiskConfigurationYieldIRShiftVolIRVega(void)
{
}

/*!
    @brief get grid matrix

	@param[in] ccy
	@return DoubleMatrix
*/
DoubleMatrix
AQLRiskConfigurationYieldIRShiftVolIRVega::getCoordinatesMatrix(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLStringVector grid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
		STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_COORDINATES + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);
	// day count
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT));
	// asOfDate
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	const unsigned int COORDINATESNUM = 4;
	const unsigned int size = grid.size();
	DoubleMatrix ret(size);
	for (unsigned int i = 0; i < size; ++i)
	{
		AQLStringVector cdns = grid[i].toToken('_');
		if (cdns.size() != COORDINATESNUM)
		{
			throw AQLCoreInvalidData(" IR vega coordinate grid format is wrong .", __FILE__, __LINE__);
		}
		ret[i].resize(COORDINATESNUM);
		for (unsigned  int j = 0; j < COORDINATESNUM; ++j)
		{
			ret[i][j] = dayCount.getTerm(asOfDate, AQLDateCalculations::getDate(asOfDate, cdns[j], true));
		}
	}

	return ret;
}


/*!
    @brief get file val matrix

	@param[in] ccy
	@return DoubleMatrix
*/
DoubleMatrix
AQLRiskConfigurationYieldIRShiftVolIRVega::getFileValMatrix(const AQLString &ccy) const
{
	
	AQLString tmpCurrency = ccy;
	AQLString strMatrixFile = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_SHIFTVAL_FILE + getCurveSuffix(ccy));
	AQLFileAccessor matrixFile(AQLMarketData::getNumFileName(strMatrixFile));
	AQLStringMatrix f_dataMatrix;
	matrixFile.readAllData(MARKET_DATA_DELIMITER, f_dataMatrix);
	matrixFile.close();

	const unsigned int size_t = f_dataMatrix.size();
	DoubleMatrix ret(size_t);
	for (unsigned int i = 0; i < size_t; ++i)
	{
		const unsigned int size_T = f_dataMatrix[i].size();
		ret[i].resize(size_T);
		for  (unsigned int j = 0; j < size_T; ++j)
		{
			ret[i][j] = f_dataMatrix[i][j].getDoubleValue();
		}
	}

	return ret;
}


/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationYieldIRShiftVolIRVega::getCoordinates(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
		STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_COORDINATES + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);

}

/*!
    @brief return bump type

	@param[in] ccy
	@return string
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getBumpType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString gridType = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_TYPE + getCurveSuffix(ccy));
	gridType.toUpper();

	if (gridType == RISK_GRID_MARKET)
	{
		return RISK_MARKET_BUMP;
	}
	else if (gridType == RISK_GRID_COORDINATES)
	{
		return RISK_MODEL_VOL_BUMP;
	}
	else
	{
		AQLString msg = "This grid type is not supported by bumpType. grid type = " + gridType;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}


/*!
    @brief return grid type

	@param[in] ccy
	@return string
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getGridType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_TYPE + getCurveSuffix(ccy));
}


/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<AQLString>
*/
AQLStringVector
AQLRiskConfigurationYieldIRShiftVolIRVega::getBucketGridTerm(const AQLString &ccy) const
{
	ccy;
	vector<AQLString> ret;
	ret.push_back(AQ_NO_DATA);
	return ret;
	
}

/*!
    @brief return market  term

	@param[in] ccy
	@return vector<AQLString>
*/
AQLStringVector
AQLRiskConfigurationYieldIRShiftVolIRVega::getMarketTerm(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLStringVector mktTems_prop = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
			STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_MARKET + getCurveSuffix(ccy)).toToken(':');

	return reduceTargetGrids(ccy, mktTems_prop);

}

/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getOutPutName1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getRiskName(void) const
{
	return RISK_FRONT_VOL_IRSHIFTIRVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftVolIRVega::isGridSensitivity(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
AQLRiskConfigurationYieldIRShiftVolIRVega::isParallelShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISPARALLEL + getCurveSuffix(ccy)));
}

/*!
    @brief return shiftvalfileuse

	@param[in] ccy
	@return bool 
*/
bool
AQLRiskConfigurationYieldIRShiftVolIRVega::isShiftValFileUse(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISFILEUSE + getCurveSuffix(ccy)));
}

/*!
    @brief return div unit value

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftVolIRVega::getDivUnit(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_DIVUNIT + getCurveSuffix(ccy)).getDoubleValue();
}


/*!
    @brief return scenario1 value

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftVolIRVega::getScenario1ShiftValue(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	double shiftVal = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
						STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_SHIFTVAL + getCurveSuffix(ccy)).getDoubleValue();

	return shiftVal / 100.0;
}


/*!
    @brief return  scenario2 value

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftVolIRVega::getScenario2ShiftValue(const AQLString &ccy) const
{
	return getScenario1ShiftValue(ccy);
}

/*!
    @brief return shift type

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getShiftType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_SHIFTTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getBumpDirection(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_BUMPDIRECTION + getCurveSuffix(ccy));

}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftVolIRVega::isWave(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief  return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRSHIFTIRVEGA_TARGET_CURRENCY);
}


/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getCalibTargetCurrencies() const
{
	AQLString targetFX = mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRSHIFTIRVEGA_CALIBRATION_TARGET_FX).toUpper();
	if (targetFX == "ALL")
	{
		return "ALL";
	}
	AQLString ret;
	AQLStringVector ccys = AQLDealUtils::getSDECurrencys();
	unsigned int ccySize = ccys.size();
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		if (ccys[i].findString(FX_DELIMITER) >= 0)
		{
			break;
		}
		ret += ccys[i] + ":";
	}
	if (ret.size() <= 0)
	{
		throw AQLCoreInvalidData("SDE currency is does not exist", __FILE__, __LINE__);
	}
	ret += targetFX;
	return ret;
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftVolIRVega::isRiskCurrencyMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISRISKCURRENCYMODE + getCurveSuffix(ccy));
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}


/*!
    @brief return baseshiftval

	@param[in] ccy
	@param[in] index
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseYieldVal(const AQLString &ccy, int index) const
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
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseShiftVals(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_BASESHIFTVAL + getCurveSuffix(ccy));

	return convertToRateValues(strVals.toToken(MULTI_STATIC_DATA_DELIMITER));
}


/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftVolIRVega::createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return createIRBaseScenarioEntity(ccy,dataInstance,index);
}


/*!
    @brief return Base extra target names

	@param[in] ccy
	@return AQLStringVector
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftVolIRVega::createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return createIRBaseExtraScenarioEntity(ccy,dataInstance,index);
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return AQLStringVector
*/
AQLStringVector
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	return getIRBaseExtraTargetNames(ccy, dataInstance);
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationYieldIRShiftVolIRVega::getShiftGridTerm(const AQLString &ccy) const
{
	
	AQLString tmpCurrency = ccy;

	AQLString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_TERM +getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

// getScenarioBaseYieldName
AQLString 
AQLRiskConfigurationYieldIRShiftVolIRVega::getScenarioBaseYieldName(const AQLString& ccy) const
{
	if (mBaseSceNames.size() != 1)
		throw AQLCoreInvalidData("Base scenario size Error",__FILE__,__LINE__);
	return mBaseSceNames[0];
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseOutPutName(const AQLString &ccy , int index) const
{
	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftVolIRVega::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
		
	AQLString curve = "";
	AQLString curveType = getBaseShiftCurveType(ccy);
	if (curveType != STD)
	{
		curve = curveType;
	}
	
	return AQLString(ccy) + AQLString("_") + curve + AQLString(irShiftVals[index] * 10000.0, 3) + AQLString("BP_DirtyPrice");
}

AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getOutPutName1(const AQLString &ccy , int index) const
{
	AQLString outName = getOutPutName1(ccy);
	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftVolIRVega::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}	
	return outName + AQLString("_") + AQLString(irShiftVals[index] * 10000.0, 3);
	
}


/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseCoefficient(const AQLString &ccy) const
{
	ccy;
	return AQLString("0.0:1.0:0.0");
}

/*!
    @brief get base targetNames

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	AQLString ret = AQLMarketData::getBaseYieldName(ccy);
	return ret;
}

/*!
    @brief return baseshiftvals

	@param[in] ccy
	@param[in] index
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseShifts(const AQLString &ccy, int index) const
{
	return getIRBaseShifts(ccy, index);
}

/*!
    @brief return irshiftvals

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRShiftVolIRVega::getIRShiftVals(const AQLString &ccy) const
{
	return getBaseShiftVals(ccy);
}

/*!
    @brief get base shift curve type

	@param[in] curveType
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseShiftCurveType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString ret = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_BASESHIFTCURVETYPE + getCurveSuffix(ccy));
	if (ret != AQ_NO_DATA)
	{
		return ret.toUpper();
	}
	else
	{
		return STD;
	}
}

/*!
    @brief get base shift curve suffix

	@param[in] curveType
*/
AQLString
AQLRiskConfigurationYieldIRShiftVolIRVega::getBaseShiftCurveSuffix(const AQLString &ccy) const
{
	AQLString curveType = getBaseShiftCurveType(ccy);
	if (curveType != STD)
	{
		return "." + curveType.toLower();
	}
	else
	{
		return "";
	}
}
