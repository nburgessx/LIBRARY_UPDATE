/*! @file
    @brief  Swaption vega setup class 
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRiskConfigurationVolSwaptionVega.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLObjectPool.h"
#include "AQLPriceDataFunction.h"
#include "AQLPricePortfolioValue.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLDealUtils.h"
#include "AQLStaticData.h"
#include "AQLScenarioConfiguration.h"
#include "AQLMarketData.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLDefinitionsHW.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationVolSwaptionVega::AQLRiskConfigurationVolSwaptionVega(void)
: AQLRiskConfigurationVolParallel(false)
{
}

// destructor
/*!

*/
AQLRiskConfigurationVolSwaptionVega::~AQLRiskConfigurationVolSwaptionVega(void)
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
AQLRiskConfigurationVolSwaptionVega::createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return AQLRiskConfigurationVolParallel::createVolatilityEntity(ccy, dataInstance, scenarioNum, index);
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
AQLRiskConfigurationVolSwaptionVega::createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return AQLRiskConfigurationVolParallel::createVolatilityEntityOld(ccy, dataInstance, scenarioNum);
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
AQLRiskConfigurationVolSwaptionVega::createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
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
				const AQLString model = AQLMarketData::getModelName(key_fx);
				AQLString inputType = AQLMarketData::getVolInputType(model, key_fx, riskName);
				inputType.toUpper();

				AQLScenarioParam param;
				param.ccy = key_fx;
				param.calcType = ccy + "_" + targetFXVec[i] + "_" + riskName;
				param.model = model;
				param.shiftType = RISK_SHIFTTYPE_DIFF;
				param.bumpDirection = RISK_BUMPDIRECTION_UPSHIFT;
				param.targetName = AQLMarketData::getBaseVolatilityName(key_fx);
				param.inputType = inputType;
				param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
				param.isCalib = true;
				param.isParallel = true;
				param.isGrid = false;
				// set file dmy
				param.paraFile.push_back(CALIB_DMY_FILE);
				// dataout
				if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
				{
					param.isOutPut = true;
				}
				else 
				{
					param.isOutPut = false;
				}
				AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
				AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, key_fx);
				delete calibInfoCreator;
				
				param.refName.push_back(infoName);

				AQLStringVector ccys = key_fx.toToken(FX_DELIMITER);
				ccys[0].toUpper();
				ccys[1].toUpper();
				AQLString dBaseYieldName = AQLMarketData::getBaseYieldName(ccys[0]);
				AQLString dBaseCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, dBaseYieldName));
				AQLString fBaseYieldName = AQLMarketData::getBaseYieldName(ccys[1]);
				AQLString fBaseCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, fBaseYieldName));
				// set reference
				if (ccys[0] == tmpCurrency)
				{
					// set domestic curve name
					param.refName.push_back(dBaseYieldName);
					param.refName.push_back(AQLMarketData::getCalibDataName(irCalcType, AQLMarketData::getYieldDataName(objPool, dBaseYieldName)));
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
					param.refName.push_back(AQLMarketData::getCalibDataName(irCalcType, AQLMarketData::getYieldDataName(objPool, fBaseYieldName)));
				}
				// create scenario
				AQLScenarioConfiguration *sceCreator = 
					AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

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
AQLRiskConfigurationVolSwaptionVega::getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const
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
			AQLString fx_bVolName = AQLMarketData::getBaseVolatilityName(targetFXVec[i]);
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
AQLRiskConfigurationVolSwaptionVega::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_OPERATOR);
}


/*!
    @brief return coefficient1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolSwaptionVega::getCoefficient1(const AQLString &ccy) const
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
AQLRiskConfigurationVolSwaptionVega::getOutPutName1(const AQLString &ccy) const
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
AQLRiskConfigurationVolSwaptionVega::getRiskName(void) const
{
	return RISK_OFFICIAL_VOL_SWAPTIONVEGA;
}

/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationVolSwaptionVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationVolSwaptionVega::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_CALIBRATION_TARGET_CURRENCY);
}

/*!
    @brief return parallel shift val

	@return double 
*/
double
AQLRiskConfigurationVolSwaptionVega::getParallelShiftVal(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	if (AQLMarketData::getModelName(ccy).toUpper() == MODEL_HW)
	{
		AQLStaticData &calibPropAccessor = AQLCoreDataService::getStaticDataManager().getCalibStaticData();
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
AQLRiskConfigurationVolSwaptionVega::isRiskCurrencyMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}
