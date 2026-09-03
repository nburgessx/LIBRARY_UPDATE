/*! @file
    @brief  Swaption vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolSwaptionVega.cpp
//
//  DESCRIPTION :         Swaption vega setup class 
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


#include "LARiskConfigurationVolSwaptionVega.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLObjectPool.h"
#include "AQLPriceDataFunction.h"
#include "LAPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LADefinitionsCalibration.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "LAScenarioConfiguration.h"
#include "LAMarketData.h"
#include "LAScenarioConfigurationManager.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "LADefinitionsHW.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationVolSwaptionVega::LARiskConfigurationVolSwaptionVega(void)
: LARiskConfigurationVolParallel(false)
{
}

// destructor
/*!

*/
LARiskConfigurationVolSwaptionVega::~LARiskConfigurationVolSwaptionVega(void)
{
}

/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
LARiskConfigurationVolSwaptionVega::createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return LARiskConfigurationVolParallel::createVolatilityEntity(ccy, dataInstance, scenarioNum, index);
	}
	else
	{
		return vector<AQLObject *>(0); 
	}
}

/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
LARiskConfigurationVolSwaptionVega::createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return LARiskConfigurationVolParallel::createVolatilityEntityOld(ccy, dataInstance, scenarioNum);
	}
	else
	{
		return vector<AQLObject *>(0); 
	}
}


/*!
    @brief create extra senario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
LARiskConfigurationVolSwaptionVega::createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	if (isRealCalib())
	{
		vector<vector<AQLObject *> > ret(0);

		AQLString tmpCurrency = ccy;
		tmpCurrency.toUpper();
 		const AQLString riskName = getRiskName();
		const AQLString irCalcType = ccy + "_" + riskName + "_" + AQLString(SCENARIO_1);
		AQLObjectPool &objPool = dataInstance.getObjectPool();
		
		AQLStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
		unsigned int fxSize = targetFXVec.size();
		for (unsigned int i = 0; i < fxSize; ++i)
		{
			if (isCalibTarget(targetFXVec[i]))
			{
				AQLString key_fx = targetFXVec[i].toLower();
				const AQLString model = LAMarketData::getModelName(key_fx);
				AQLString inputType = LAMarketData::getVolInputType(model, key_fx, riskName);
				inputType.toUpper();

				MAScenarioParam param;
				param.ccy = key_fx;
				param.calcType = ccy + "_" + targetFXVec[i] + "_" + riskName;
				param.model = model;
				param.shiftType = RISK_SHIFTTYPE_DIFF;
				param.bumpDirection = RISK_BUMPDIRECTION_UPSHIFT;
				param.targetName = LAMarketData::getBaseVolatilityName(key_fx);
				param.inputType = inputType;
				param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
				param.isCalib = true;
				param.isParallel = true;
				param.isGrid = false;
				// set file dmy
				param.paraFile.push_back(CALIB_DMY_FILE);
				// dataout
				if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
				{
					param.isOutPut = true;
				}
				else 
				{
					param.isOutPut = false;
				}
				LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
				AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, key_fx);
				delete calibInfoCreator;
				
				param.refName.push_back(infoName);

				AQLStringVector ccys = key_fx.toToken(FX_DELIMITER);
				ccys[0].toUpper();
				ccys[1].toUpper();
				AQLString dBaseYieldName = LAMarketData::getBaseYieldName(ccys[0]);
				AQLString dBaseCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dBaseYieldName));
				AQLString fBaseYieldName = LAMarketData::getBaseYieldName(ccys[1]);
				AQLString fBaseCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, fBaseYieldName));
				// set reference
				if (ccys[0] == tmpCurrency)
				{
					// set domestic curve name
					param.refName.push_back(dBaseYieldName);
					param.refName.push_back(LAMarketData::getCalibDataName(irCalcType, LAMarketData::getYieldDataName(objPool, dBaseYieldName)));
					// set foreign curve name
					param.refName.push_back(fBaseYieldName);
					param.refName.push_back(fBaseCalibDataName);
				}
				else
				{
					// set domestic curve name
					param.refName.push_back(dBaseYieldName);
					param.refName.push_back(dBaseCalibDataName);
					// set foreign curve name
					param.refName.push_back(fBaseYieldName);
					param.refName.push_back(LAMarketData::getCalibDataName(irCalcType, LAMarketData::getYieldDataName(objPool, fBaseYieldName)));
				}
				// create scenario
				LAScenarioConfiguration *sceCreator = 
					LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

				vector<AQLObject *> sce_fx = sceCreator->createScenario(dataInstance, param);
				delete sceCreator;

				ret.push_back(sce_fx);
			}
		}
		return ret;
	}
	else
	{
		return vector<vector<AQLObject *> >(0);
	}
}


/*!
    @brief return extra scenario1 target names

	@param[in] ccy
	@return DoubleArray
*/
AQLStringVector
LARiskConfigurationVolSwaptionVega::getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const
{
	if (isRealCalib())
	{
		AQLStringVector ret(0);
		AQLStringVector targetNames = getTargetNames(ccy, dataInstance).toToken(MULTI_STATIC_DATA_DELIMITER);
		unsigned int targetSize = targetNames.size();

		AQLStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
		unsigned int fxSize = targetFXVec.size();
		for (unsigned int i = 0; i < fxSize; ++i)
		{
			AQLString fx_targetNames1;
			AQLString fx_bVolName = LAMarketData::getBaseVolatilityName(targetFXVec[i]);
			for (unsigned int j = 0; j < targetSize; ++j)
			{
				fx_targetNames1 += fx_bVolName + ":";
			}

			if (fx_targetNames1.size() < 2)
			{
				throw AQLCoreInvalidData("Extra targetname1 can not set !!", __FILE__, __LINE__);
			}
			ret.push_back(fx_targetNames1.subString(0, fx_targetNames1.size() - 2));
		}
		
		return ret;
	}
	else
	{
		return AQLStringVector(0);
	}

}


/*!
    @brief return operator1

	@return AQLString
*/
AQLString
LARiskConfigurationVolSwaptionVega::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_OPERATOR);
}


/*!
    @brief return coefficient1

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationVolSwaptionVega::getCoefficient1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_COEFFICIENT + getCurveSuffix(ccy));
}


/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationVolSwaptionVega::getOutPutName1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_OUTPUT + getCurveSuffix(ccy));
}


/*!
    @brief return riskname

	@return AQLString
*/
AQLString
LARiskConfigurationVolSwaptionVega::getRiskName(void) const
{
	return RISK_OFFICIAL_VOL_SWAPTIONVEGA;
}

/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
LARiskConfigurationVolSwaptionVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
LARiskConfigurationVolSwaptionVega::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_CALIBRATION_TARGET_CURRENCY);
}

/*!
    @brief return parallel shift val

	@return double 
*/
double
LARiskConfigurationVolSwaptionVega::getParallelShiftVal(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	if (LAMarketData::getModelName(ccy).toUpper() == MODEL_HW)
	{
		LAStaticData &calibPropAccessor = LACoreDataService::getStaticDataManager().getCalibStaticData();
		AQLString strIsZeroVol = calibPropAccessor.getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_CALIB_HW_ISZEROVOL);
		if (strIsZeroVol != AQ_NO_DATA && convertBoolFromStr(strIsZeroVol))
		{
			return 0.0;
		}
	}
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
							STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_SHIFTVAL + getCurveSuffix(ccy)).getDoubleValue() / 100;
}

bool
LARiskConfigurationVolSwaptionVega::isRiskCurrencyMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}
