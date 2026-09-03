/*! @file
    @brief  IR Shift Delta setup class
*/
//  2008, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRShiftDelta.cpp
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
#include "LARiskConfigurationYieldIRShiftDelta.h"
#include "LADataInstance.h"
#include "LAObjectPool.h"
#include "LADataReference.h"
#include "LADataProcedure.h"
#include "LADefinitionsRisk.h"
#include "LALinearFunc.h"
#include "LABasic.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LADealUtils.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "LADefinitionsCalibration.h"
#include "LAPricePortfolioValue.h"


using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldIRShiftDelta::LARiskConfigurationYieldIRShiftDelta(void)
:LARiskConfigurationYieldIRDelta()
{
}

// destructor
/*!

*/
LARiskConfigurationYieldIRShiftDelta::~LARiskConfigurationYieldIRShiftDelta(void)
{
}


/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfigurationYieldIRShiftDelta::createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return createIRBaseScenarioEntity(ccy,dataInstance,index);
}


/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldIRShiftDelta::getShiftGridTerm(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief create risk object
	@param [in] objPool
	@return vector<pair<LAString, vector<LAObject *> > >
*/
vector<pair<LAString, vector<LAObject *> > >
LARiskConfigurationYieldIRShiftDelta::createRiskEntity(LAObjectPool &objPool) const
{
	LAStringVector ccys = getRiskCurrencys(objPool);
	vector<pair<LAString, vector<LAObject *> > > ret;
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

		vector<LAObject *> eVec;
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
				LAString name = ccys[i].toUpper() + "_" + getRiskName() + "_" + getCurveType(ccys[i]) + "_Shift_" + LAString(shiftVals[j]);
				LAObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
				LAObject *e = 0;
				if (!objHolder.isDefined())
				{
					// create risk object
					e = new LAObject();
				}
				else
				{
					e = &objHolder.get();
					e->reset();
				}
				e->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
				e->add(PRICING_DATA_RISKCURVETYPENAME, new LADataString()).convertFromString(getCurveType(ccys[i]));
				e->add(PRICING_DATA_RISKCURVETYPECURRENCY, new LADataString()).convertFromString(ccys[i]);
				eVec[j] = e;
			}	
		}
		else
		{
			// scenario case
			//when no base shifts
			LAString name = ccys[i].toUpper() + "_" + getRiskName() + "_" + getCurveType(ccys[i]);
			LAObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
			LAObject *e = 0;
			if (!objHolder.isDefined())
			{
				e = new LAObject();
			}
			else
			{
				e = &objHolder.get();
				e->reset();
			}
			e->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
			e->add(PRICING_DATA_RISKCURVETYPENAME, new LADataString()).convertFromString(getCurveType(ccys[i]));
			e->add(PRICING_DATA_RISKCURVETYPECURRENCY, new LADataString()).convertFromString(ccys[i]);
			eVec.push_back(e);
		}
		ret.push_back(make_pair(ccys[i].toUpper(), eVec));
	}
	return ret;
}


/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getOutPutName1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getOutPutName1(const LAString &ccy , int index) const
{
	LAString outName = getOutPutName1(ccy);
	
	if (!isIRShiftScenario(ccy))
	{
		DoubleArray irShiftVals = getIRShiftVals(ccy);
		const unsigned int shiftSize = irShiftVals.size();
		if (index < 0 || index >= static_cast<int>(shiftSize))
		{
			throw LACoreInvalidData("LARiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
		}	
		return outName + LAString("_") + LAString(irShiftVals[index] * 10000.0, 3);
	}
	else
	{
		return outName + LAString("_SCENARIO");	
	}
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getBaseOutPutName(const LAString &ccy , int index) const
{
	if (!isIRShiftScenario(ccy))
	{
		DoubleArray irShiftVals = getIRShiftVals(ccy);
		const unsigned int shiftSize = irShiftVals.size();
		if (index < 0 || index >= static_cast<int>(shiftSize))
		{
			throw LACoreInvalidData("LARiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
		}
		LAString curve = "";
		LAString curveType = getCurveType(ccy);
		if (curveType != STD)
		{
			curve = curveType;
		}
		return LAString(ccy) + LAString("_") + curve + LAString(irShiftVals[index] * 10000.0, 3) + LAString("BP_DirtyPrice");
	}
	else
	{
		return LAString(ccy) + LAString("_") + LAString("SCENARIO_DirtyPrice");
	}
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return LAStringVector
*/
vector<LAObject *>
LARiskConfigurationYieldIRShiftDelta::createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return createIRBaseExtraScenarioEntity(ccy,dataInstance,index);
}


/*!
    @brief return Base extra target names

	@param[in] ccy
	@return LAStringVector
*/
LAStringVector
LARiskConfigurationYieldIRShiftDelta::getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	return getIRBaseExtraTargetNames(ccy, dataInstance);
}


/*!
    @brief return baseshiftstr

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getBaseShiftStr(const LAString &ccy , int index) const
{
	if (!isIRShiftScenario(ccy))
	{
		DoubleArray irShiftVals = getIRShiftVals(ccy);
		const unsigned int shiftSize = irShiftVals.size();
		if (index < 0 || index >= static_cast<int>(shiftSize))
		{
			throw LACoreInvalidData("LARiskConfigurationYieldIRShiftDelta::getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
		}
		return LAString(irShiftVals[index] * 10000.0, 0);
	}
	else
	{
		return LAString("scenario");
	}
}
/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftDelta::isGridSensitivity(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationYieldIRShiftDelta::isParallelShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISPARALLEL + getCurveSuffix(ccy)));
}


/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getRiskName(void) const
{
	return RISK_FRONT_YIELD_IRSHIFTDELTA;
}

/*!
    @brief return scenario1 parallel shift value(string)

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getScenario1ParallelShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_PARALLEL_SHIFTVAL + getCurveSuffix(ccy));
}

/*!
    @brief return divid unit

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldIRShiftDelta::getDivUnit(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strDivUnit = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_DIVUNIT + getCurveSuffix(ccy));

	return  strDivUnit.getDoubleValue();
}



/*!
    @brief return scenario1 grid shift values(string)

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getScenario1GridShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_SHIFTVAL + getCurveSuffix(ccy));

}


/*!
    @brief return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRSHIFTDELTA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRSHIFTDELTA_CALIBRATION_TARGET_CURRENCY);
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getBumpDirection(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BUMPDIRECTION + getCurveSuffix(ccy));

}


/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftDelta::isWave(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief return shift type

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getShiftType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SHIFTTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return isirshiftscenario

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftDelta::isIRShiftScenario(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISIRSHIFTSCENARIO + getCurveSuffix(ccy)));
}


/*!
    @brief return irshiftvals

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldIRShiftDelta::getIRShiftVals(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
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
LARiskConfigurationYieldIRShiftDelta::getBaseShifts(const LAString &ccy, int index) const
{
	if (!isIRShiftScenario(ccy))
	{
		DoubleArray shiftVals = getIRShiftVals(ccy);
		const unsigned int shiftSize = shiftVals.size();
		if (index < 0 || index >= static_cast<int>(shiftSize))
		{
			throw LACoreInvalidData("LARiskConfigurationYieldIRShiftDelta::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
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
LARiskConfigurationYieldIRShiftDelta::getIRShiftScenarioVals(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCENARIO_IRSHIFTVAL + getCurveSuffix(ccy));
	return convertToRateValues(strVals.toToken(MULTI_STATIC_DATA_DELIMITER));
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getBaseCoefficient(const LAString &ccy) const
{
	ccy;
	return LAString("0.0:1.0:0.0");
}

/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
LARiskConfigurationYieldIRShiftDelta::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldIRShiftDelta::getBucketGridTerm(const LAString &ccy) const
{
	LAStringVector ret;
	LAString tmpccy = ccy;
	LAString strBucketGrid = mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BUCKET_TERM + getCurveSuffix(ccy));
	LAStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" ||BucketTerm[0] == MLIB_NO_DATA)
	{
		ret.push_back(MLIB_NO_DATA);
		return ret;
	}
	else
	{
		unsigned int gridMax = getMaxGridIndex(ccy);
		LAStringVector tmpgridTerm = getShiftGridTerm(ccy);
	
		for (unsigned int i = 0;i < BucketTerm.size();++i)
		{
			LAStringVector::iterator it;
			LAString strgrid = BucketTerm[i].toUpper();
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
	@return vector<LAString>
*/
LAString
LARiskConfigurationYieldIRShiftDelta::getPropertyBucketGridTerm(const LAString &ccy) const
{
	LAString tmpccy = ccy;
	return mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BUCKET_TERM + getCurveSuffix(ccy));
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftDelta::isRiskCurrencyMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}


/*!
    @brief check zero rate bump

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRShiftDelta::isZeroBump(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}