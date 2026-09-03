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
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LAObjectPool.h"
#include "LAPriceDataFunction.h"
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
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolSwaptionVega::createVolatilityEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return LARiskConfigurationVolParallel::createVolatilityEntity(ccy, dataInstance, scenarioNum, index);
	}
	else
	{
		return vector<LAObject *>(0); 
	}
}

/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<LAObject *>
*/
vector<LAObject *>
LARiskConfigurationVolSwaptionVega::createVolatilityEntityOld(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return LARiskConfigurationVolParallel::createVolatilityEntityOld(ccy, dataInstance, scenarioNum);
	}
	else
	{
		return vector<LAObject *>(0); 
	}
}


/*!
    @brief create extra senario1 object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<vector<LAObject *> > 
*/
vector<vector<LAObject *> > 
LARiskConfigurationVolSwaptionVega::createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	if (isRealCalib())
	{
		vector<vector<LAObject *> > ret(0);

		LAString tmpCurrency = ccy;
		tmpCurrency.toUpper();
 		const LAString riskName = getRiskName();
		const LAString irCalcType = ccy + "_" + riskName + "_" + LAString(SCENARIO_1);
		LAObjectPool &objPool = dataInstance.getObjectPool();
		
		LAStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
		unsigned int fxSize = targetFXVec.size();
		for (unsigned int i = 0; i < fxSize; ++i)
		{
			if (isCalibTarget(targetFXVec[i]))
			{
				LAString key_fx = targetFXVec[i].toLower();
				const LAString model = LAMarketData::getModelName(key_fx);
				LAString inputType = LAMarketData::getVolInputType(model, key_fx, riskName);
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
				LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, key_fx);
				delete calibInfoCreator;
				
				param.refName.push_back(infoName);

				LAStringVector ccys = key_fx.toToken(FX_DELIMITER);
				ccys[0].toUpper();
				ccys[1].toUpper();
				LAString dBaseYieldName = LAMarketData::getBaseYieldName(ccys[0]);
				LAString dBaseCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dBaseYieldName));
				LAString fBaseYieldName = LAMarketData::getBaseYieldName(ccys[1]);
				LAString fBaseCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, fBaseYieldName));
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

				vector<LAObject *> sce_fx = sceCreator->createScenario(dataInstance, param);
				delete sceCreator;

				ret.push_back(sce_fx);
			}
		}
		return ret;
	}
	else
	{
		return vector<vector<LAObject *> >(0);
	}
}


/*!
    @brief return extra scenario1 target names

	@param[in] ccy
	@return DoubleArray
*/
LAStringVector
LARiskConfigurationVolSwaptionVega::getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const
{
	if (isRealCalib())
	{
		LAStringVector ret(0);
		LAStringVector targetNames = getTargetNames(ccy, dataInstance).toToken(MULTI_STATIC_DATA_DELIMITER);
		unsigned int targetSize = targetNames.size();

		LAStringVector targetFXVec = getCalibTargetFX(ccy, dataInstance);
		unsigned int fxSize = targetFXVec.size();
		for (unsigned int i = 0; i < fxSize; ++i)
		{
			LAString fx_targetNames1;
			LAString fx_bVolName = LAMarketData::getBaseVolatilityName(targetFXVec[i]);
			for (unsigned int j = 0; j < targetSize; ++j)
			{
				fx_targetNames1 += fx_bVolName + ":";
			}

			if (fx_targetNames1.size() < 2)
			{
				throw LACoreInvalidData("Extra targetname1 can not set !!", __FILE__, __LINE__);
			}
			ret.push_back(fx_targetNames1.subString(0, fx_targetNames1.size() - 2));
		}
		
		return ret;
	}
	else
	{
		return LAStringVector(0);
	}

}


/*!
    @brief return operator1

	@return LAString
*/
LAString
LARiskConfigurationVolSwaptionVega::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_OPERATOR);
}


/*!
    @brief return coefficient1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationVolSwaptionVega::getCoefficient1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_COEFFICIENT + getCurveSuffix(ccy));
}


/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationVolSwaptionVega::getOutPutName1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_OUTPUT + getCurveSuffix(ccy));
}


/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationVolSwaptionVega::getRiskName(void) const
{
	return RISK_OFFICIAL_VOL_SWAPTIONVEGA;
}

/*!
    @brief return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationVolSwaptionVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationVolSwaptionVega::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_CALIBRATION_TARGET_CURRENCY);
}

/*!
    @brief return parallel shift val

	@return double 
*/
double
LARiskConfigurationVolSwaptionVega::getParallelShiftVal(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	if (LAMarketData::getModelName(ccy).toUpper() == MODEL_HW)
	{
		LAStaticData &calibPropAccessor = LACoreDataService::getStaticDataManager().getCalibStaticData();
		LAString strIsZeroVol = calibPropAccessor.getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_CALIB_HW_ISZEROVOL);
		if (strIsZeroVol != AQ_NO_DATA && convertBoolFromStr(strIsZeroVol))
		{
			return 0.0;
		}
	}
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
							STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_SHIFTVAL + getCurveSuffix(ccy)).getDoubleValue() / 100;
}

bool
LARiskConfigurationVolSwaptionVega::isRiskCurrencyMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}
