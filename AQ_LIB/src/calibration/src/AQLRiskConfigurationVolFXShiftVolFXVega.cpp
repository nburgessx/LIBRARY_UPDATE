/*! @file
    @brief  FX Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationVolFXShiftVolFXVega.cpp
//
//  DESCRIPTION :         FX Vega setup class 
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
#include "AQLRiskConfigurationVolFXShiftVolFXVega.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
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
AQLRiskConfigurationVolFXShiftVolFXVega::AQLRiskConfigurationVolFXShiftVolFXVega(void)
: AQLRiskConfigurationVolFXVega()
{
}

// destructor
/*!

*/
AQLRiskConfigurationVolFXShiftVolFXVega::~AQLRiskConfigurationVolFXShiftVolFXVega(void)
{
}

/*!
    @brief return deltatype

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getDeltaType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_DELTATYPE);
}

/*!
    @brief return grid term

	@param[in] fx
	@return vector<AQLString>
*/
AQLStringVector
AQLRiskConfigurationVolFXShiftVolFXVega::getGridTerm(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString dtype = getDeltaType(fx);
	AQLStringVector grid = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_GRID).toToken(':');
	AQLStringVector dtypes(grid.size(), dtype + '_');
	transform(dtypes.begin(), dtypes.end(), grid.begin(), grid.begin(), plus<AQLString>());
	return grid;
}

/*!
    @brief return bucket grid term

	@param[in] fx
	@return vector<AQLString>
*/
AQLStringVector
AQLRiskConfigurationVolFXShiftVolFXVega::getBucketGridTerm(const AQLString &fx) const
{
	AQLStringVector ret;
	AQLString tmpfx = fx;
	AQLString strBucketGrid = mpRiskStaticData->getStaticData(tmpfx.toLower() + 
								FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BUCKET_GRID_TERM);
	AQLStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == AQ_NO_DATA)
	{
		ret.push_back(AQ_NO_DATA);
		return ret;
	}
	else
	{
		AQLString dtype = getDeltaType(fx);
		AQLStringVector dtypes(BucketTerm.size(), dtype + '_');
		transform(dtypes.begin(), dtypes.end(), BucketTerm.begin(), BucketTerm.begin(), plus<AQLString>());
		AQLStringVector tmpgridTerm = getGridTerm(fx);
		unsigned int gridMax = tmpgridTerm.size();
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
			if (i == BucketTerm.size() - 1 && pos < gridMax - 1)
				ret.push_back(tmpgridTerm[gridMax - 1]);
		}
		return ret;
	}
}

/*!
    @brief return propety bucket grid term

	@param[in] fx
	@return vector<AQLString>
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getPropertyBucketGridTerm(const AQLString &fx) const
{
	AQLString tmpfx = fx;
	return mpRiskStaticData->getStaticData(tmpfx.toLower() + 
									FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BUCKET_GRID_TERM);
}


/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_OUTPUTNAME);
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getOutPutName1(const AQLString &ccy , int index) const
{
	AQLString outName = getOutPutName1(ccy);
	double val = getBaseVolVal(ccy,index);
	return outName + AQLString("_") + AQLString(val * 100.0 ,3);
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getRiskName(void) const
{
	return RISK_FRONT_FXVOLSHIFT_VOL_FXVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] fx
	@return bool
*/
bool
AQLRiskConfigurationVolFXShiftVolFXVega::isGridSensitivity(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISGRIDSENSITIVITY));
}

/*!
    @brief return isparallelshift

	@param[in] fx
	@return bool 
*/
bool
AQLRiskConfigurationVolFXShiftVolFXVega::isParallelShift(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISPARALLEL));
}

/*!
    @brief return div unit value

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationVolFXShiftVolFXVega::getDivUnit(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + 
								FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_DIVUNIT).getDoubleValue();
}

/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getShiftType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_SHIFTTYPE);

}

/*!
    @brief return bump direction

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getBumpDirection(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BUMPDIRECTION);
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationVolFXShiftVolFXVega::isWave(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISWAVE));
}

/*!
    @brief  return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_TARGET_FX);
}

/*!
    @brief return scenario1 value

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationVolFXShiftVolFXVega::getScenario1ShiftValue(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString keyFX =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	double shiftVal = mpRiskStaticData->getStaticData(keyFX + 
								FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_SHIFTVAL).getDoubleValue();

	return shiftVal / 100.0;
}

/*!
    @brief get FX shiftvals
	@param [in] ccy
	@return shiftvals
*/
DoubleArray  
AQLRiskConfigurationVolFXShiftVolFXVega::getBaseShiftVals(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BASESHIFTVOLVAL);
	AQLDataDoubles dbs;
	dbs.convertFromString(strVals);
	DoubleVector ret = dbs.get();
	for (unsigned int i = 0 ;i < ret.size();i++)
		ret[i]/=100.0;
	
	return ret;
}

/*!
    @brief return basevolshiftval

	@param[in] ccy
	@param[in] index
	@return double
*/
double
AQLRiskConfigurationVolFXShiftVolFXVega::getBaseVolVal(const AQLString &ccy, int index) const
{
	AQLString tmpCurrency = ccy;
	DoubleArray shiftVolVals = getBaseShiftVals(ccy);
	
	const unsigned int shiftSize = shiftVolVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw AQLCoreInvalidData("LARiskConfigurationFXVOLShiftFXVega index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVolVals[index];
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getBaseOutPutName(const AQLString &ccy , int index) const
{
	double val = getBaseVolVal(ccy,index);
	return AQLString(ccy) + AQLString("_") + AQLString(val*100,3) +  AQLString("%") + AQLString("SHIFT_DirtyPrice");
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXShiftVolFXVega::getBaseCoefficient(const AQLString &ccy) const
{
	ccy;
	return AQLString("0.0:1.0:0.0");
}

/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationVolFXShiftVolFXVega::createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	const AQLString model = AQLMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	// get base shift and grid
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();
	// set target name
	AQLMathFXEntity *targetFX = AQLMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	// search shift target currency(foreign currency)
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(ccy, ccys);

	const AQLStringVector &fx_ccys = targetFX->getCurrencys().get();
	AQLStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
	if (it == fx_ccys.end())
	{
		AQLString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	
	//create base volatility
	// scenario param
	AQLScenarioParam paramvol;
	paramvol.ccy = ccy;
	paramvol.isCalib = true;
	paramvol.calcType= ccys[0] + "_" + ccys[1] + "_" + riskName + "_BaseShift_" + AQLString(index);
	paramvol.model = model;
	paramvol.targetName = AQLMarketData::getBaseVolatilityName(ccy);
	paramvol.inputType = AQLMarketData::getVolInputType(model, ccy, riskName);
	paramvol.isParallel = true;
	paramvol.isGrid = false;

	AQLObjectPool &objPool = dataInstance.getObjectPool();
	// set reference
	AQLCalibrationParameters *calibInfoCreator = AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(paramvol.model);
	AQLString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
	delete calibInfoCreator;
	paramvol.refName.push_back(infoName);

	// set yield and calibdata
	paramvol.refName.push_back(AQLMarketData::getBaseYieldName(ccys[0]));
	paramvol.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, paramvol.refName.back())));
	paramvol.refName.push_back(AQLMarketData::getBaseYieldName(ccys[1]));
	paramvol.refName.push_back(AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, paramvol.refName.back())));

	// dataout
	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		paramvol.isOutPut = true;
	}
	else 
	{
		paramvol.isOutPut = false;
	}

	// set DDL
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	paramvol.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
	// set file path dmy
	paramvol.paraFile.push_back(CALIB_DMY_FILE);
	// set shift value and type
	paramvol.shiftType = RISK_BUMPDIRECTION_UPSHIFT;

	//baseshiftval
	paramvol.extraBaseVolParam = getBaseVolVal(ccy,index);
	
	// create scenario
	AQLScenarioConfiguration *sceVolCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<AQLObject *> ret = sceVolCreator->createScenario(dataInstance, paramvol);
	delete sceVolCreator;

	return ret;
}

/*!
    @brief returnisRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationVolFXShiftVolFXVega::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);

	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}