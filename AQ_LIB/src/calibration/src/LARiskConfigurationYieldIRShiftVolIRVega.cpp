/*! @file
    @brief  IR Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRShiftVolIRVega.cpp
//
//  DESCRIPTION :         IR Vega setup class 
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


#include <algorithm>
#include "LARiskConfigurationYieldIRShiftVolIRVega.h"
#include "LAString.h"
#include "LADataInstance.h"
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
#include "LADealUtils.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LALinearFunc.h"
#include "LADefinitionsCalibration.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldIRShiftVolIRVega::LARiskConfigurationYieldIRShiftVolIRVega(void)
: LARiskConfigurationVolIRVega()
{
}

// destructor
/*!

*/
LARiskConfigurationYieldIRShiftVolIRVega::~LARiskConfigurationYieldIRShiftVolIRVega(void)
{
}

/*!
    @brief get grid matrix

	@param[in] ccy
	@return DoubleMatrix
*/
DoubleMatrix
LARiskConfigurationYieldIRShiftVolIRVega::getCoordinatesMatrix(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAStringVector grid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
		STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_COORDINATES + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);
	// day count
	LAPriceDataDayCount dayCount;
	dayCount.convertFromString(LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT));
	// asOfDate
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	const unsigned int COORDINATESNUM = 4;
	const unsigned int size = grid.size();
	DoubleMatrix ret(size);
	for (unsigned int i = 0; i < size; ++i)
	{
		LAStringVector cdns = grid[i].toToken('_');
		if (cdns.size() != COORDINATESNUM)
		{
			throw LACoreInvalidData(" IR vega coordinate grid format is wrong .", __FILE__, __LINE__);
		}
		ret[i].resize(COORDINATESNUM);
		for (unsigned  int j = 0; j < COORDINATESNUM; ++j)
		{
			ret[i][j] = dayCount.getTerm(asOfDate, LAMathDateCalculations::getDate(asOfDate, cdns[j], true));
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
LARiskConfigurationYieldIRShiftVolIRVega::getFileValMatrix(const LAString &ccy) const
{
	
	LAString tmpCurrency = ccy;
	LAString strMatrixFile = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_SHIFTVAL_FILE + getCurveSuffix(ccy));
	MAFileAccessor matrixFile(LAMarketData::getNumFileName(strMatrixFile));
	LAStringMatrix f_dataMatrix;
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
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldIRShiftVolIRVega::getCoordinates(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
		STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_COORDINATES + getCurveSuffix(ccy)).toToken(MULTI_STATIC_DATA_DELIMITER);

}

/*!
    @brief return bump type

	@param[in] ccy
	@return string
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getBumpType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString gridType = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_TYPE + getCurveSuffix(ccy));
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
		LAString msg = "This grid type is not supported by bumpType. grid type = " + gridType;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}


/*!
    @brief return grid type

	@param[in] ccy
	@return string
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getGridType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_TYPE + getCurveSuffix(ccy));
}


/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationYieldIRShiftVolIRVega::getBucketGridTerm(const LAString &ccy) const
{
	ccy;
	vector<LAString> ret;
	ret.push_back(AQ_NO_DATA);
	return ret;
	
}

/*!
    @brief return market  term

	@param[in] ccy
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationYieldIRShiftVolIRVega::getMarketTerm(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAStringVector mktTems_prop = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
			STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_MARKET + getCurveSuffix(ccy)).toToken(':');

	return reduceTargetGrids(ccy, mktTems_prop);

}

/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getOutPutName1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getRiskName(void) const
{
	return RISK_FRONT_VOL_IRSHIFTIRVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftVolIRVega::isGridSensitivity(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationYieldIRShiftVolIRVega::isParallelShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISPARALLEL + getCurveSuffix(ccy)));
}

/*!
    @brief return shiftvalfileuse

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationYieldIRShiftVolIRVega::isShiftValFileUse(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISFILEUSE + getCurveSuffix(ccy)));
}

/*!
    @brief return div unit value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldIRShiftVolIRVega::getDivUnit(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_DIVUNIT + getCurveSuffix(ccy)).getDoubleValue();
}


/*!
    @brief return scenario1 value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldIRShiftVolIRVega::getScenario1ShiftValue(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
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
LARiskConfigurationYieldIRShiftVolIRVega::getScenario2ShiftValue(const LAString &ccy) const
{
	return getScenario1ShiftValue(ccy);
}

/*!
    @brief return shift type

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getShiftType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_SHIFTTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getBumpDirection(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_BUMPDIRECTION + getCurveSuffix(ccy));

}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftVolIRVega::isWave(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief  return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRSHIFTIRVEGA_TARGET_CURRENCY);
}


/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getCalibTargetCurrencies() const
{
	LAString targetFX = mpRiskStaticData->getStaticData(RISK_FRONT_VOL_IRSHIFTIRVEGA_CALIBRATION_TARGET_FX).toUpper();
	if (targetFX == "ALL")
	{
		return "ALL";
	}
	LAString ret;
	LAStringVector ccys = MADealUtils::getSDECurrencys();
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
		throw LACoreInvalidData("SDE currency is does not exist", __FILE__, __LINE__);
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
LARiskConfigurationYieldIRShiftVolIRVega::isRiskCurrencyMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
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
LARiskConfigurationYieldIRShiftVolIRVega::getBaseYieldVal(const LAString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationYieldIRShiftVolFXVega::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief get FX shiftvals
	@param [in] ccy
	@return shiftvals
*/
DoubleArray  
LARiskConfigurationYieldIRShiftVolIRVega::getBaseShiftVals(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
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
vector<LAObject *>
LARiskConfigurationYieldIRShiftVolIRVega::createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return createIRBaseScenarioEntity(ccy,dataInstance,index);
}


/*!
    @brief return Base extra target names

	@param[in] ccy
	@return LAStringVector
*/
vector<LAObject *>
LARiskConfigurationYieldIRShiftVolIRVega::createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return createIRBaseExtraScenarioEntity(ccy,dataInstance,index);
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return LAStringVector
*/
LAStringVector
LARiskConfigurationYieldIRShiftVolIRVega::getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	return getIRBaseExtraTargetNames(ccy, dataInstance);
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldIRShiftVolIRVega::getShiftGridTerm(const LAString &ccy) const
{
	
	LAString tmpCurrency = ccy;

	LAString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_TERM +getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

// getScenarioBaseYieldName
LAString 
LARiskConfigurationYieldIRShiftVolIRVega::getScenarioBaseYieldName(const LAString& ccy) const
{
	if (mBaseSceNames.size() != 1)
		throw LACoreInvalidData("Base scenario size Error",__FILE__,__LINE__);
	return mBaseSceNames[0];
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getBaseOutPutName(const LAString &ccy , int index) const
{
	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationYieldIRShiftVolIRVega::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
		
	LAString curve = "";
	LAString curveType = getBaseShiftCurveType(ccy);
	if (curveType != STD)
	{
		curve = curveType;
	}
	
	return LAString(ccy) + LAString("_") + curve + LAString(irShiftVals[index] * 10000.0, 3) + LAString("BP_DirtyPrice");
}

LAString
LARiskConfigurationYieldIRShiftVolIRVega::getOutPutName1(const LAString &ccy , int index) const
{
	LAString outName = getOutPutName1(ccy);
	DoubleArray irShiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationYieldIRShiftVolIRVega::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}	
	return outName + LAString("_") + LAString(irShiftVals[index] * 10000.0, 3);
	
}


/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getBaseCoefficient(const LAString &ccy) const
{
	ccy;
	return LAString("0.0:1.0:0.0");
}

/*!
    @brief get base targetNames

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAString ret = LAMarketData::getBaseYieldName(ccy);
	return ret;
}

/*!
    @brief return baseshiftvals

	@param[in] ccy
	@param[in] index
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldIRShiftVolIRVega::getBaseShifts(const LAString &ccy, int index) const
{
	return getIRBaseShifts(ccy, index);
}

/*!
    @brief return irshiftvals

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldIRShiftVolIRVega::getIRShiftVals(const LAString &ccy) const
{
	return getBaseShiftVals(ccy);
}

/*!
    @brief get base shift curve type

	@param[in] curveType
*/
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getBaseShiftCurveType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString ret = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
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
LAString
LARiskConfigurationYieldIRShiftVolIRVega::getBaseShiftCurveSuffix(const LAString &ccy) const
{
	LAString curveType = getBaseShiftCurveType(ccy);
	if (curveType != STD)
	{
		return "." + curveType.toLower();
	}
	else
	{
		return "";
	}
}
