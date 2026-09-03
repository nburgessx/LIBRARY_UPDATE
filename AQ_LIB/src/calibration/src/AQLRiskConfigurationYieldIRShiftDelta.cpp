/*! @file
    @brief  IR Shift Delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationYieldIRShiftDelta.cpp
//
//  DESCRIPTION :       IR Shift Delta setup class
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
#include "AQLRiskConfigurationYieldIRShiftDelta.h"
#include "AQLDataInstance.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "AQLDefinitionsRisk.h"
#include "AQLLinearFunc.h"
#include "AQLBasic.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLDealUtils.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLPricePortfolioValue.h"


using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationYieldIRShiftDelta::AQLRiskConfigurationYieldIRShiftDelta(void)
:AQLRiskConfigurationYieldIRDelta()
{
}

// destructor
/*!

*/
AQLRiskConfigurationYieldIRShiftDelta::~AQLRiskConfigurationYieldIRShiftDelta(void)
{
}


/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftDelta::createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return createIRBaseScenarioEntity(ccy,dataInstance,index);
}


/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationYieldIRShiftDelta::getShiftGridTerm(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief create risk object
	@param [in] objPool
	@return vector<pair<AQLString, vector<AQLObject *> > >
*/
vector<pair<AQLString, vector<AQLObject *> > >
AQLRiskConfigurationYieldIRShiftDelta::createRiskEntity(AQLObjectPool &objPool) const
{
	AQLStringVector ccys = getRiskCurrencys(objPool);
	vector<pair<AQLString, vector<AQLObject *> > > ret;
	const unsigned int ccyNum = ccys.size();

	for (unsigned int i = 0; i < ccyNum; ++i)
	{
		if (!isTarget(ccys[i]))
		{
			continue;
		}
		if (ccys[i].findString(FX_DELIMITER) >= 0)
		{
			break;
		}

		vector<AQLObject *> eVec;
		if (!isIRShiftScenario(ccys[i]))
		{
			// non scenario case
			DoubleArray shiftVals = getIRShiftVals(ccys[i]);
			const int shiftSize = shiftVals.size();
			eVec.resize(shiftSize);
			for (int j = 0; j < shiftSize; ++j)
			{
				if (!isBaseScenarioTarget(ccys[i], j))
				{
					eVec[j] = 0;
					continue;
				}
				//when no base shifts
				AQLString name = ccys[i].toUpper() + "_" + getRiskName() + "_" + getCurveType(ccys[i]) + "_Shift_" + AQLString(shiftVals[j]);
				AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
				AQLObject *e = 0;
				if (!objHolder.isDefined())
				{
					// create risk object
					e = new AQLObject();
				}
				else
				{
					e = &objHolder.get();
					e->reset();
				}
				e->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(name);
				e->add(PRICING_DATA_RISKCURVETYPENAME, new AQLDataString()).convertFromString(getCurveType(ccys[i]));
				e->add(PRICING_DATA_RISKCURVETYPECURRENCY, new AQLDataString()).convertFromString(ccys[i]);
				eVec[j] = e;
			}	
		}
		else
		{
			// scenario case
			//when no base shifts
			AQLString name = ccys[i].toUpper() + "_" + getRiskName() + "_" + getCurveType(ccys[i]);
			AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
			AQLObject *e = 0;
			if (!objHolder.isDefined())
			{
				e = new AQLObject();
			}
			else
			{
				e = &objHolder.get();
				e->reset();
			}
			e->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(name);
			e->add(PRICING_DATA_RISKCURVETYPENAME, new AQLDataString()).convertFromString(getCurveType(ccys[i]));
			e->add(PRICING_DATA_RISKCURVETYPECURRENCY, new AQLDataString()).convertFromString(ccys[i]);
			eVec.push_back(e);
		}
		ret.push_back(make_pair(ccys[i].toUpper(), eVec));
	}
	return ret;
}


/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getOutPutName1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getOutPutName1(const AQLString &ccy , int index) const
{
	AQLString outName = getOutPutName1(ccy);
	
	if (!isIRShiftScenario(ccy))
	{
		DoubleArray irShiftVals = getIRShiftVals(ccy);
		const unsigned int shiftSize = irShiftVals.size();
		if (index < 0 || index >= static_cast<int>(shiftSize))
		{
			throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
		}	
		return outName + AQLString("_") + AQLString(irShiftVals[index] * 10000.0, 3);
	}
	else
	{
		return outName + AQLString("_SCENARIO");	
	}
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getBaseOutPutName(const AQLString &ccy , int index) const
{
	if (!isIRShiftScenario(ccy))
	{
		DoubleArray irShiftVals = getIRShiftVals(ccy);
		const unsigned int shiftSize = irShiftVals.size();
		if (index < 0 || index >= static_cast<int>(shiftSize))
		{
			throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
		}
		AQLString curve = "";
		AQLString curveType = getCurveType(ccy);
		if (curveType != STD)
		{
			curve = curveType;
		}
		return AQLString(ccy) + AQLString("_") + curve + AQLString(irShiftVals[index] * 10000.0, 3) + AQLString("BP_DirtyPrice");
	}
	else
	{
		return AQLString(ccy) + AQLString("_") + AQLString("SCENARIO_DirtyPrice");
	}
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return AQLStringVector
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRShiftDelta::createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return createIRBaseExtraScenarioEntity(ccy,dataInstance,index);
}


/*!
    @brief return Base extra target names

	@param[in] ccy
	@return AQLStringVector
*/
AQLStringVector
AQLRiskConfigurationYieldIRShiftDelta::getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	return getIRBaseExtraTargetNames(ccy, dataInstance);
}


/*!
    @brief return baseshiftstr

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getBaseShiftStr(const AQLString &ccy , int index) const
{
	if (!isIRShiftScenario(ccy))
	{
		DoubleArray irShiftVals = getIRShiftVals(ccy);
		const unsigned int shiftSize = irShiftVals.size();
		if (index < 0 || index >= static_cast<int>(shiftSize))
		{
			throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
		}
		return AQLString(irShiftVals[index] * 10000.0, 0);
	}
	else
	{
		return AQLString("scenario");
	}
}
/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftDelta::isGridSensitivity(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
AQLRiskConfigurationYieldIRShiftDelta::isParallelShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISPARALLEL + getCurveSuffix(ccy)));
}


/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getRiskName(void) const
{
	return RISK_FRONT_YIELD_IRSHIFTDELTA;
}

/*!
    @brief return scenario1 parallel shift value(string)

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getScenario1ParallelShiftStr(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_PARALLEL_SHIFTVAL + getCurveSuffix(ccy));
}

/*!
    @brief return divid unit

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationYieldIRShiftDelta::getDivUnit(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strDivUnit = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_DIVUNIT + getCurveSuffix(ccy));

	return  strDivUnit.getDoubleValue();
}



/*!
    @brief return scenario1 grid shift values(string)

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getScenario1GridShiftStr(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_SHIFTVAL + getCurveSuffix(ccy));

}


/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRSHIFTDELTA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRSHIFTDELTA_CALIBRATION_TARGET_CURRENCY);
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getBumpDirection(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BUMPDIRECTION + getCurveSuffix(ccy));

}


/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftDelta::isWave(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief return shift type

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getShiftType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SHIFTTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return isirshiftscenario

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftDelta::isIRShiftScenario(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISIRSHIFTSCENARIO + getCurveSuffix(ccy)));
}


/*!
    @brief return irshiftvals

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRShiftDelta::getIRShiftVals(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_IRSHIFTVAL + getCurveSuffix(ccy));
	return convertToRateValues(strVals.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return baseshiftvals

	@param[in] ccy
	@param[in] index
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRShiftDelta::getBaseShifts(const AQLString &ccy, int index) const
{
	if (!isIRShiftScenario(ccy))
	{
		DoubleArray shiftVals = getIRShiftVals(ccy);
		const unsigned int shiftSize = shiftVals.size();
		if (index < 0 || index >= static_cast<int>(shiftSize))
		{
			throw AQLCoreInvalidData("AQLRiskConfigurationYieldIRShiftDelta::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
		}

		return DoubleArray(getShiftGridTerm(ccy).size(), shiftVals[index]);
	}
	else
	{
		return getIRShiftScenarioVals(ccy);

	}	
}

/*!
    @brief return irshift scenario vals

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRShiftDelta::getIRShiftScenarioVals(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCENARIO_IRSHIFTVAL + getCurveSuffix(ccy));
	return convertToRateValues(strVals.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getBaseCoefficient(const AQLString &ccy) const
{
	ccy;
	return AQLString("0.0:1.0:0.0");
}

/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
AQLRiskConfigurationYieldIRShiftDelta::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationYieldIRShiftDelta::getBucketGridTerm(const AQLString &ccy) const
{
	AQLStringVector ret;
	AQLString tmpccy = ccy;
	AQLString strBucketGrid = mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BUCKET_TERM + getCurveSuffix(ccy));
	AQLStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" ||BucketTerm[0] == AQ_NO_DATA)
	{
		ret.push_back(AQ_NO_DATA);
		return ret;
	}
	else
	{
		unsigned int gridMax = getMaxGridIndex(ccy);
		AQLStringVector tmpgridTerm = getShiftGridTerm(ccy);
	
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
			if (i == BucketTerm.size() - 1 && pos <= gridMax)
				ret.push_back(tmpgridTerm[gridMax]);
		}
		return ret;
	}
}

/*!
    @brief return property bucket grid term

	@param[in] ccy
	@return vector<AQLString>
*/
AQLString
AQLRiskConfigurationYieldIRShiftDelta::getPropertyBucketGridTerm(const AQLString &ccy) const
{
	AQLString tmpccy = ccy;
	return mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BUCKET_TERM + getCurveSuffix(ccy));
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftDelta::isRiskCurrencyMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}


/*!
    @brief check zero rate bump

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRShiftDelta::isZeroBump(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}