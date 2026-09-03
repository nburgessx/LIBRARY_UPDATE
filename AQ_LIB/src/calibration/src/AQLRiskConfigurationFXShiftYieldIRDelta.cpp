/*! @file
    @brief  IR Delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationFXShiftYieldIRDelta.cpp
//
//  DESCRIPTION :       IR Delta setup class
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
#include "AQLRiskConfigurationFXShiftYieldIRDelta.h"
#include "AQLDataInstance.h"
#include "AQLBasic.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "AQLDefinitionsRisk.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLFileAccessor.h"
#include "AQLMarketData.h"
#include "AQLLinearFunc.h"
#include "AQLStaticData.h"
#include "AQLDealUtils.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationFXShiftYieldIRDelta::AQLRiskConfigurationFXShiftYieldIRDelta(void)
:AQLRiskConfigurationYieldIRDelta()
{
}

// destructor
/*!

*/
AQLRiskConfigurationFXShiftYieldIRDelta::~AQLRiskConfigurationFXShiftYieldIRDelta(void)
{
}

/*!
    @brief return extra scenario1 target names

	@param[in] ccy
	@return DoubleArray
*/
AQLStringVector
AQLRiskConfigurationFXShiftYieldIRDelta::getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	AQLStringVector ret = AQLRiskConfigurationYield::getExtraTargetNames1(ccy, dataInstance);
	return ret;
}

/*!
    @brief create extra senario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
AQLRiskConfigurationFXShiftYieldIRDelta::createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	// for nocalib
	vector<vector<AQLObject *> > ret = AQLRiskConfigurationYield::createExtraScenario1Entity(ccy,dataInstance,index);
	return ret;
}

/*!
    @brief create extra senario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
AQLRiskConfigurationFXShiftYieldIRDelta::createExtraScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	vector<vector<AQLObject *> > ret = AQLRiskConfigurationYield::createExtraScenario2Entity(ccy,dataInstance,index);
	return ret;
}




/*!
    @brief get FX shiftvals
	@param [in] ccy
	@return shiftvals
*/
DoubleArray  
AQLRiskConfigurationFXShiftYieldIRDelta::getBaseShiftVals(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BASESHIFTVAL + getCurveSuffix(ccy));
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
AQLRiskConfigurationFXShiftYieldIRDelta::getBaseSpotVal(const AQLString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("LARiskConfigurationFXShiftIRDelta::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}
/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationFXShiftYieldIRDelta::getBucketGridTerm(const AQLString &ccy) const
{
	AQLStringVector ret;
	AQLString tmpccy = ccy;
	AQLString strBucketGrid = mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BUCKET_TERM + getCurveSuffix(ccy));
	AQLStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == AQ_NO_DATA)
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
AQLRiskConfigurationFXShiftYieldIRDelta::getPropertyBucketGridTerm(const AQLString &ccy) const
{
	AQLString tmpccy = ccy;
	return mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BUCKET_TERM + getCurveSuffix(ccy));
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationFXShiftYieldIRDelta::getShiftGridTerm(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_GRID_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getOutPutName1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getOutPutName1(const AQLString &ccy , int index) const
{
	AQLString outName = getOutPutName1(ccy);
	double val = getBaseSpotVal(ccy,index);
	AQLString fxcur = getCrossBaseCurrency(ccy);
	return outName + AQLString("_") + fxcur.toUpper() + AQLString(val,3);
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationFXShiftYieldIRDelta::isGridSensitivity(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
AQLRiskConfigurationFXShiftYieldIRDelta::isParallelShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISPARALLEL + getCurveSuffix(ccy)));
}


/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getRiskName(void) const
{
	return RISK_FRONT_FXSHIFT_YIELD_IRDELTA;
}



/*!
    @brief return scenario1 parallel shift value(string)

	@param[in] ccy
	@return AQLString
*/
AQLString 
AQLRiskConfigurationFXShiftYieldIRDelta::getScenario1ParallelShiftStr(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_PARALLEL_SHIFTVAL + getCurveSuffix(ccy));
}


/*!
    @brief return scenario1 grid shift values(string)

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getScenario1GridShiftStr(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
	    						STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_GRID_SHIFTVAL + getCurveSuffix(ccy));


}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getBumpDirection(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BUMPDIRECTION + getCurveSuffix(ccy));

}


/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationFXShiftYieldIRDelta::isWave(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief return shift type

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getShiftType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_SHIFTTYPE + getCurveSuffix(ccy));

}


/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_FXSHIFTIRDELTA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_FXSHIFTIRDELTA_CALIBRATION_TARGET_CURRENCY);
}


/*!
    @brief return divid unit

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationFXShiftYieldIRDelta::getDivUnit(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strDivUnit = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_DIVUNIT + getCurveSuffix(ccy));

	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
AQLRiskConfigurationFXShiftYieldIRDelta::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_FXSHIFTIRDELTA_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getBaseOutPutName(const AQLString &ccy , int index) const
{
	double val = getBaseSpotVal(ccy,index);
	AQLString fxcur = getCrossBaseCurrency(ccy);
	AQLString curve = "";
	AQLString curveType = getCurveType(ccy);
	if (curveType != STD)
	{
		curve = curveType + "-";
	}
	return AQLString(ccy) + AQLString("_") + curve + fxcur.toUpper() + AQLString(val,3) + AQLString("SHIFT_DirtyPrice");
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getBaseCoefficient(const AQLString &ccy) const
{
	ccy;
	return AQLString("0.0:1.0:0.0");
}

/*!
    @brief return spot Currency

	@return AQLString
*/
AQLString  
AQLRiskConfigurationFXShiftYieldIRDelta::getCrossBaseCurrency(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
							STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_CROSSBASECURRENCY + getCurveSuffix(ccy));
}

/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationFXShiftYieldIRDelta::createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	vector<AQLObject *> ret;

	const AQLString bYieldName = AQLMarketData::getBaseYieldName(ccy); 
	AQLObjectPool& objPool = dataInstance.getObjectPool();
	const AQLMathYieldCurve &bYield = dynamic_cast<const AQLMathYieldCurve &>
					(objPool.getObject(bYieldName, ENCHKTYPE_ISDEFINED).get());

	ret.push_back(bYield.clone());
	const AQLString name = bYieldName + "_" + getBaseCalcType(ccy, index);
	ret[0]->getData(CALIBRATION_DATA_NAME,ISNOTNULL).convertFromString(name);

	return ret;
}

/*!
    @brief return Base extra scenario

	@param[in] ccy
	@return AQLStringVector
*/
vector<AQLObject *>
AQLRiskConfigurationFXShiftYieldIRDelta::createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	return AQLRiskConfiguration::createFXBaseExtraScenarioEntity(ccy, dataInstance, index);
}

/*!
    @brief return Base target names

	@param[in] ccy
	@return AQLStringVector
*/
AQLString
AQLRiskConfigurationFXShiftYieldIRDelta::getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	return AQLRiskConfiguration::getFXBaseTargetNames(ccy, dataInstance);
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return AQLStringVector
*/
AQLStringVector
AQLRiskConfigurationFXShiftYieldIRDelta::getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	return AQLRiskConfiguration::getFXBaseExtraTargetNames(ccy, dataInstance);
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationFXShiftYieldIRDelta::isRiskCurrencyMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISRISKCURRENCYMODE);
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
AQLRiskConfigurationFXShiftYieldIRDelta::isZeroBump(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}