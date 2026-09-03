/*! @file
    @brief  FX Vega setup class 
*/
//  2008, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolFXShiftVolFXVega.cpp
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
#include "LARiskConfigurationVolFXShiftVolFXVega.h"
#include "LAString.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
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
LARiskConfigurationVolFXShiftVolFXVega::LARiskConfigurationVolFXShiftVolFXVega(void)
: LARiskConfigurationVolFXVega()
{
}

// destructor
/*!

*/
LARiskConfigurationVolFXShiftVolFXVega::~LARiskConfigurationVolFXShiftVolFXVega(void)
{
}

/*!
    @brief return deltatype

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getDeltaType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_DELTATYPE);
}

/*!
    @brief return grid term

	@param[in] fx
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationVolFXShiftVolFXVega::getGridTerm(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString dtype = getDeltaType(fx);
	LAStringVector grid = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_GRID).toToken(':');
	LAStringVector dtypes(grid.size(), dtype + '_');
	transform(dtypes.begin(), dtypes.end(), grid.begin(), grid.begin(), plus<LAString>());
	return grid;
}

/*!
    @brief return bucket grid term

	@param[in] fx
	@return vector<LAString>
*/
LAStringVector
LARiskConfigurationVolFXShiftVolFXVega::getBucketGridTerm(const LAString &fx) const
{
	LAStringVector ret;
	LAString tmpfx = fx;
	LAString strBucketGrid = mpRiskStaticData->getStaticData(tmpfx.toLower() + 
								FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BUCKET_GRID_TERM);
	LAStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == MLIB_NO_DATA)
	{
		ret.push_back(MLIB_NO_DATA);
		return ret;
	}
	else
	{
		LAString dtype = getDeltaType(fx);
		LAStringVector dtypes(BucketTerm.size(), dtype + '_');
		transform(dtypes.begin(), dtypes.end(), BucketTerm.begin(), BucketTerm.begin(), plus<LAString>());
		LAStringVector tmpgridTerm = getGridTerm(fx);
		unsigned int gridMax = tmpgridTerm.size();
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
			if (i == BucketTerm.size() - 1 && pos < gridMax - 1)
				ret.push_back(tmpgridTerm[gridMax - 1]);
		}
		return ret;
	}
}

/*!
    @brief return propety bucket grid term

	@param[in] fx
	@return vector<LAString>
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getPropertyBucketGridTerm(const LAString &fx) const
{
	LAString tmpfx = fx;
	return mpRiskStaticData->getStaticData(tmpfx.toLower() + 
									FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BUCKET_GRID_TERM);
}


/*!
    @brief return outputname1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getOutPutName1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_OUTPUTNAME);
}

/*!
    @brief return outputname1

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getOutPutName1(const LAString &ccy , int index) const
{
	LAString outName = getOutPutName1(ccy);
	double val = getBaseVolVal(ccy,index);
	return outName + LAString("_") + LAString(val * 100.0 ,3);
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getRiskName(void) const
{
	return RISK_FRONT_FXVOLSHIFT_VOL_FXVEGA;
}

/*!
    @brief return isgridsensitivity

	@param[in] fx
	@return bool
*/
bool
LARiskConfigurationVolFXShiftVolFXVega::isGridSensitivity(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISGRIDSENSITIVITY));
}

/*!
    @brief return isparallelshift

	@param[in] fx
	@return bool 
*/
bool
LARiskConfigurationVolFXShiftVolFXVega::isParallelShift(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISPARALLEL));
}

/*!
    @brief return div unit value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationVolFXShiftVolFXVega::getDivUnit(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey + 
								FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_DIVUNIT).getDoubleValue();
}

/*!
    @brief return shift type

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getShiftType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_SHIFTTYPE);

}

/*!
    @brief return bump direction

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getBumpDirection(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BUMPDIRECTION);
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolFXShiftVolFXVega::isWave(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return convertBoolFromStr(mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISWAVE));
}

/*!
    @brief  return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_TARGET_FX);
}

/*!
    @brief return scenario1 value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationVolFXShiftVolFXVega::getScenario1ShiftValue(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString keyFX =  LAMarketData::getFXKey(ccys[0], ccys[1]);
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
LARiskConfigurationVolFXShiftVolFXVega::getBaseShiftVals(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strVals = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BASESHIFTVOLVAL);
	LADataDoubles dbs;
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
LARiskConfigurationVolFXShiftVolFXVega::getBaseVolVal(const LAString &ccy, int index) const
{
	LAString tmpCurrency = ccy;
	DoubleArray shiftVolVals = getBaseShiftVals(ccy);
	
	const unsigned int shiftSize = shiftVolVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("LARiskConfigurationFXVOLShiftFXVega index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return shiftVolVals[index];
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getBaseOutPutName(const LAString &ccy , int index) const
{
	double val = getBaseVolVal(ccy,index);
	return LAString(ccy) + LAString("_") + LAString(val*100,3) +  LAString("%") + LAString("SHIFT_DirtyPrice");
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getBaseOperator() const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return base operateor

	@return LAString
*/
LAString
LARiskConfigurationVolFXShiftVolFXVega::getBaseCoefficient(const LAString &ccy) const
{
	ccy;
	return LAString("0.0:1.0:0.0");
}

/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfigurationVolFXShiftVolFXVega::createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();
	// get base shift and grid
	LAString tmpCurrency = ccy;
	tmpCurrency.toLower();
	// set target name
	LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	// search shift target currency(foreign currency)
	LAStringVector ccys;
	LAMarketData::convertToCurrency(ccy, ccys);

	const LAStringVector &fx_ccys = targetFX->getCurrencys().get();
	LAStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
	if (it == fx_ccys.end())
	{
		LAString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	
	//create base volatility
	// scenario param
	MAScenarioParam paramvol;
	paramvol.ccy = ccy;
	paramvol.isCalib = true;
	paramvol.calcType= ccys[0] + "_" + ccys[1] + "_" + riskName + "_BaseShift_" + LAString(index);
	paramvol.model = model;
	paramvol.targetName = LAMarketData::getBaseVolatilityName(ccy);
	paramvol.inputType = LAMarketData::getVolInputType(model, ccy, riskName);
	paramvol.isParallel = true;
	paramvol.isGrid = false;

	LAObjectPool &objPool = dataInstance.getObjectPool();
	// set reference
	LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(paramvol.model);
	LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, ccy);
	delete calibInfoCreator;
	paramvol.refName.push_back(infoName);

	// set yield and calibdata
	paramvol.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
	paramvol.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, paramvol.refName.back())));
	paramvol.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
	paramvol.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, paramvol.refName.back())));

	// dataout
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
	{
		paramvol.isOutPut = true;
	}
	else 
	{
		paramvol.isOutPut = false;
	}

	// set DDL
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	paramvol.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
	// set file path dmy
	paramvol.paraFile.push_back(CALIB_DMY_FILE);
	// set shift value and type
	paramvol.shiftType = RISK_BUMPDIRECTION_UPSHIFT;

	//baseshiftval
	paramvol.extraBaseVolParam = getBaseVolVal(ccy,index);
	
	// create scenario
	LAScenarioConfiguration *sceVolCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> ret = sceVolCreator->createScenario(dataInstance, paramvol);
	delete sceVolCreator;

	return ret;
}

/*!
    @brief returnisRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolFXShiftVolFXVega::isRiskCurrencyMode(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);

	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}