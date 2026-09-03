/*! @file
    @brief  IR Delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftYieldBasisDelta.cpp
//
//  DESCRIPTION :       FX Shift Basis Delta setup class
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
#include "LARiskConfigurationFXShiftYieldBasisDelta.h"
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
LARiskConfigurationFXShiftYieldBasisDelta::LARiskConfigurationFXShiftYieldBasisDelta(void)
:LARiskConfigurationYieldBasisDelta()
{
}

// destructor
/*!

*/
LARiskConfigurationFXShiftYieldBasisDelta::~LARiskConfigurationFXShiftYieldBasisDelta(void)
{
}

/*!
    @brief return extra scenario1 target names

	@param[in] ccy
	@return DoubleArray
*/
LAStringVector
LARiskConfigurationFXShiftYieldBasisDelta::getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const
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
LARiskConfigurationFXShiftYieldBasisDelta::createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
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
LARiskConfigurationFXShiftYieldBasisDelta::createExtraScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
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
LARiskConfigurationFXShiftYieldBasisDelta::getBaseShiftVals(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	const LAString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_BASESHIFTVAL + getCurveSuffix(ccy));
	LADataDoubles dbs;
	dbs.convertFromString(strVals);
	const DoubleVector ret = dbs.get();
	return ret;
}

/*!
    @brief return baseshiftval

	@param[in] ccy
	@param[in] index
	@return double
*/
double
LARiskConfigurationFXShiftYieldBasisDelta::getBaseSpotVal(const LAString &ccy, int index) const
{
	DoubleArray shiftVals = getBaseShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationFXShiftBasisDelta::getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVals[index];
}

/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationFXShiftYieldBasisDelta::getBucketGridTerm(const LAString &ccy) const
{
	LAStringVector ret;
	LAString tmpccy = ccy;
	LAString strBucketGrid = mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_BUCKET_TERM + getCurveSuffix(ccy));
	LAStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == MLIB_NO_DATA)
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
LARiskConfigurationFXShiftYieldBasisDelta::getPropertyBucketGridTerm(const LAString &ccy) const
{
	LAString tmpccy = ccy;
	return mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_BUCKET_TERM + getCurveSuffix(ccy));
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationFXShiftYieldBasisDelta::getShiftGridTerm(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_GRID_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getOutPutName1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getOutPutName1(const LAString &ccy , int index) const
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
LARiskConfigurationFXShiftYieldBasisDelta::isGridSensitivity(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationFXShiftYieldBasisDelta::isParallelShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_ISPARALLEL + getCurveSuffix(ccy)));
}


/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getRiskName(void) const
{
	return RISK_FRONT_FXSHIFT_YIELD_BASISDELTA;
}



/*!
    @brief return scenario1 parallel shift value(string)

	@param[in] ccy
	@return LAString
*/
LAString 
LARiskConfigurationFXShiftYieldBasisDelta::getScenario1ParallelShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_PARALLEL_SHIFTVAL + getCurveSuffix(ccy));
}


/*!
    @brief return scenario1 grid shift values(string)

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getScenario1GridShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
	    						STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_GRID_SHIFTVAL + getCurveSuffix(ccy));

}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getBumpDirection(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_BUMPDIRECTION + getCurveSuffix(ccy));

}


/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationFXShiftYieldBasisDelta::isWave(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief return shift type

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getShiftType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_SHIFTTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return basis type

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getBasisType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_BASISTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_FXSHIFTBASISDELTA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_FXSHIFTBASISDELTA_CALIBRATION_TARGET_CURRENCY);
}


/*!
    @brief return divid unit

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationFXShiftYieldBasisDelta::getDivUnit(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strDivUnit = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_DIVUNIT + getCurveSuffix(ccy));

	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
LARiskConfigurationFXShiftYieldBasisDelta::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_FXSHIFTBASISDELTA_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getBaseOutPutName(const LAString &ccy , int index) const
{
	double val = getBaseSpotVal(ccy,index);
	LAString fxcur = getCrossBaseCurrency(ccy);
	LAString curve = "";
	LAString curveType = getCurveType(ccy);
	if (curveType != STD)
	{
		curve = "_" + curveType;
	}
	return LAString(ccy) + curve + LAString("_") + fxcur.toUpper() + LAString(val,3) + LAString("SHIFT_DirtyPrice");
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getBaseCoefficient(const LAString &ccy) const
{
	ccy;
	return LAString("0.0:1.0:0.0");
}

/*!
    @brief return spot Currency

	@return LAString
*/
LAString  
LARiskConfigurationFXShiftYieldBasisDelta::getCrossBaseCurrency(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
							STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_CROSSBASECURRENCY + getCurveSuffix(ccy));
}

/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfigurationFXShiftYieldBasisDelta::createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
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
LARiskConfigurationFXShiftYieldBasisDelta::createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	return LARiskConfiguration::createFXBaseExtraScenarioEntity(ccy, dataInstance, index);
}

/*!
    @brief return Base target names

	@param[in] ccy
	@return LAStringVector
*/
LAString
LARiskConfigurationFXShiftYieldBasisDelta::getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	return LARiskConfiguration::getFXBaseTargetNames(ccy, dataInstance);
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return LAStringVector
*/
LAStringVector
LARiskConfigurationFXShiftYieldBasisDelta::getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	return LARiskConfiguration::getFXBaseExtraTargetNames(ccy, dataInstance);
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationFXShiftYieldBasisDelta::isRiskCurrencyMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}