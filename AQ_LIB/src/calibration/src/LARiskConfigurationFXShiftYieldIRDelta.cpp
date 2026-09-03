/*! @file
    @brief  IR Delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftYieldIRDelta.cpp
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
#include "LARiskConfigurationFXShiftYieldIRDelta.h"
#include "LADataInstance.h"
#include "LABasic.h"
#include "LAObjectPool.h"
#include "LADataReference.h"
#include "LADataProcedure.h"
#include "LADefinitionsRisk.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAFileAccessor.h"
#include "LAMarketData.h"
#include "LALinearFunc.h"
#include "LAStaticData.h"
#include "LADealUtils.h"
#include "LADefinitionsCalibration.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationFXShiftYieldIRDelta::LARiskConfigurationFXShiftYieldIRDelta(void)
:LARiskConfigurationYieldIRDelta()
{
}

// destructor
/*!

*/
LARiskConfigurationFXShiftYieldIRDelta::~LARiskConfigurationFXShiftYieldIRDelta(void)
{
}

/*!
    @brief return extra scenario1 target names

	@param[in] ccy
	@return DoubleArray
*/
LAStringVector
LARiskConfigurationFXShiftYieldIRDelta::getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAStringVector ret = LARiskConfigurationYield::getExtraTargetNames1(ccy, dataInstance);
	return ret;
}

/*!
    @brief create extra senario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<LAObject *> > 
*/
vector<vector<LAObject *> > 
LARiskConfigurationFXShiftYieldIRDelta::createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	// for nocalib
	vector<vector<LAObject *> > ret = LARiskConfigurationYield::createExtraScenario1Entity(ccy,dataInstance,index);
	return ret;
}

/*!
    @brief create extra senario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<LAObject *> > 
*/
vector<vector<LAObject *> > 
LARiskConfigurationFXShiftYieldIRDelta::createExtraScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	vector<vector<LAObject *> > ret = LARiskConfigurationYield::createExtraScenario2Entity(ccy,dataInstance,index);
	return ret;
}




/*!
    @brief get FX shiftvals
	@param [in] ccy
	@return shiftvals
*/
DoubleArray  
LARiskConfigurationFXShiftYieldIRDelta::getBaseShiftVals(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BASESHIFTVAL + getCurveSuffix(ccy));
	LADataDoubles dbs;
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
LARiskConfigurationFXShiftYieldIRDelta::getBaseSpotVal(const LAString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationFXShiftIRDelta::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}
/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationFXShiftYieldIRDelta::getBucketGridTerm(const LAString &ccy) const
{
	LAStringVector ret;
	LAString tmpccy = ccy;
	LAString strBucketGrid = mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BUCKET_TERM + getCurveSuffix(ccy));
	LAStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == AQ_NO_DATA)
	{
		ret.push_back(AQ_NO_DATA);
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
LARiskConfigurationFXShiftYieldIRDelta::getPropertyBucketGridTerm(const LAString &ccy) const
{
	LAString tmpccy = ccy;
	return mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BUCKET_TERM + getCurveSuffix(ccy));
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationFXShiftYieldIRDelta::getShiftGridTerm(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_GRID_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getOutPutName1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getOutPutName1(const LAString &ccy , int index) const
{
	LAString outName = getOutPutName1(ccy);
	double val = getBaseSpotVal(ccy,index);
	LAString fxcur = getCrossBaseCurrency(ccy);
	return outName + LAString("_") + fxcur.toUpper() + LAString(val,3);
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationFXShiftYieldIRDelta::isGridSensitivity(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationFXShiftYieldIRDelta::isParallelShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISPARALLEL + getCurveSuffix(ccy)));
}


/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getRiskName(void) const
{
	return RISK_FRONT_FXSHIFT_YIELD_IRDELTA;
}



/*!
    @brief return scenario1 parallel shift value(string)

	@param[in] ccy
	@return LAString
*/
LAString 
LARiskConfigurationFXShiftYieldIRDelta::getScenario1ParallelShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_PARALLEL_SHIFTVAL + getCurveSuffix(ccy));
}


/*!
    @brief return scenario1 grid shift values(string)

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getScenario1GridShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
	    						STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_GRID_SHIFTVAL + getCurveSuffix(ccy));


}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getBumpDirection(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BUMPDIRECTION + getCurveSuffix(ccy));

}


/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationFXShiftYieldIRDelta::isWave(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief return shift type

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getShiftType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_SHIFTTYPE + getCurveSuffix(ccy));

}


/*!
    @brief return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_FXSHIFTIRDELTA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_FXSHIFTIRDELTA_CALIBRATION_TARGET_CURRENCY);
}


/*!
    @brief return divid unit

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationFXShiftYieldIRDelta::getDivUnit(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strDivUnit = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_DIVUNIT + getCurveSuffix(ccy));

	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
LARiskConfigurationFXShiftYieldIRDelta::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_FXSHIFTIRDELTA_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getBaseOutPutName(const LAString &ccy , int index) const
{
	double val = getBaseSpotVal(ccy,index);
	LAString fxcur = getCrossBaseCurrency(ccy);
	LAString curve = "";
	LAString curveType = getCurveType(ccy);
	if (curveType != STD)
	{
		curve = curveType + "-";
	}
	return LAString(ccy) + LAString("_") + curve + fxcur.toUpper() + LAString(val,3) + LAString("SHIFT_DirtyPrice");
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getBaseCoefficient(const LAString &ccy) const
{
	ccy;
	return LAString("0.0:1.0:0.0");
}

/*!
    @brief return spot Currency

	@return LAString
*/
LAString  
LARiskConfigurationFXShiftYieldIRDelta::getCrossBaseCurrency(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
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
vector<LAObject *>
LARiskConfigurationFXShiftYieldIRDelta::createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	vector<LAObject *> ret;

	const LAString bYieldName = LAMarketData::getBaseYieldName(ccy); 
	LAObjectPool& objPool = dataInstance.getObjectPool();
	const LAMathYieldCurve &bYield = dynamic_cast<const LAMathYieldCurve &>
					(objPool.getObject(bYieldName, ENCHKTYPE_ISDEFINED).get());

	ret.push_back(bYield.clone());
	const LAString name = bYieldName + "_" + getBaseCalcType(ccy, index);
	ret[0]->getData(CALIBRATION_DATA_NAME,ISNOTNULL).convertFromString(name);

	return ret;
}

/*!
    @brief return Base extra scenario

	@param[in] ccy
	@return LAStringVector
*/
vector<LAObject *>
LARiskConfigurationFXShiftYieldIRDelta::createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return LARiskConfiguration::createFXBaseExtraScenarioEntity(ccy, dataInstance, index);
}

/*!
    @brief return Base target names

	@param[in] ccy
	@return LAStringVector
*/
LAString
LARiskConfigurationFXShiftYieldIRDelta::getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	return LARiskConfiguration::getFXBaseTargetNames(ccy, dataInstance);
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return LAStringVector
*/
LAStringVector
LARiskConfigurationFXShiftYieldIRDelta::getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	return LARiskConfiguration::getFXBaseExtraTargetNames(ccy, dataInstance);
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationFXShiftYieldIRDelta::isRiskCurrencyMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
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
LARiskConfigurationFXShiftYieldIRDelta::isZeroBump(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}