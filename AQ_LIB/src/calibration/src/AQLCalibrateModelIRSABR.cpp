#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <fstream>
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLPriceDataManager.h"
#include "AQLDataReference.h"
#include "AQLMathCorrelation.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLRatesTermStructureSDE.h"
#include "AQLRatesLJTermStructureSDE.h"
#include "AQLRatesCurveLinearInterpolation.h"
#include "AQLCoreDataService.h"
#include "AQLStaticData.h"
#include "AQLDealUtils.h"
#include "AQLCalibrateModelIRSABR.h"
#include "AQLDataVector.h"
#include "AQLDataProcedure.h"
#include "AQLMathYieldCurve.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"
#include "AQLMarketData.h"
#include "AQLMathVolFuncBase.h"
#include "AQLCalibrateVolatilityIRSABR.h"
#include "AQLCalibrationParametersIRSABR.h"
#include "AQLMathVolatility.h"
#include "AQLScenarioConfiguration.h"
#include "AQLDefinitionsIRSABR.h"
#include "AQLDataMatrix.h"

using namespace std;

// constructor
/*!

*/
AQLCalibrateModelIRSABR::AQLCalibrateModelIRSABR(const AQLString &baseCurrency)
: AQLCalibrateModelIR(baseCurrency)
{
}

// destructor
/*!

*/
AQLCalibrateModelIRSABR::~AQLCalibrateModelIRSABR(void)
{
}

/*!
	@brief set volatility to sde

	@param[in] currency
	@param[out] sde
*/
void
AQLCalibrateModelIRSABR::setVolatility(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	return ;
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
AQLString
AQLCalibrateModelIRSABR::getVolType(const AQLString &key) const
{
	return INPUT_FUNC;
}

// 
/*!
    @brief set volatility function

	@param[in] currency
	@param[out] vol
	@param[out] dataInstance
*/
void
AQLCalibrateModelIRSABR::setUpVolFunc(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	AQLString key_ccy = currency;
	key_ccy.toLower();
	AQLStringVector fileVec(1);
	AQLCalibrateVolatilityIRSABR volCreator;
	// create method vec
	vector<AQLFunctionBase *> funcVec;

	AQLScenarioParam param;
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	param.isCalib = isCalibTarget(currency);
	param.calcType = KEY_PV;
	if (param.isCalib)
	{
		AQLStaticData &calibProp = AQLCoreDataService::getStaticDataManager().getCalibStaticData();
		AQLString underlying = calibProp.getStaticData(key_ccy + STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING);
		underlying.toLower();
		AQLStringVector underlyings = underlying.toToken(':');
		//get a flag to calibrate all grids
		bool isAllGridsCalibrate = false;
		AQLString strIsAllCalib= calibProp.getStaticData(key_ccy + STATIC_DATA_KEY_CALIB_IRSABR_ISALLGRIDSCALIBRATE);
		if (strIsAllCalib != AQ_NO_DATA)
		{
			AQLDataBool tmpAttrBool;
			tmpAttrBool.convertFromString(strIsAllCalib);
			isAllGridsCalibrate = tmpAttrBool.get();
		}
		for (size_t j = 0; j < underlyings.size(); ++j)
		{
			const AQLString key = key_ccy + "." CONTEXT_KEY_DEAL_IRVOL + "." + underlyings[j];
			const AQLString val = AQLCoreDataService::getContext(key);
			const AQLString fileName = calibProp.getStaticData(key_ccy + STATIC_DATA_KEY_CALIB_IRSABR_TARGETVOLGRID_FILE + "." + underlyings[j]);
			if (val == AQ_NO_DATA && fileName != AQ_NO_DATA && !isAllGridsCalibrate)
			{
				AQLDate asofDate = AQLMarketData::getAsofDate(objPool);
				BoolMatrix calibTarget = AQLDealUtils::getCalibTargetIRVolGrids(objPool, asofDate, key_ccy, underlyings[j], true);
				AQLDataBoolMatrix tmp(calibTarget);
				AQLCoreDataService::setContext(key, tmp.convertToString());
			}
		}

		// create calib info
		AQLCalibrationParametersIRSABR cInfo;
		AQLString cInfoName = cInfo.createCalibrationInfo(objPool, currency);
		// first element set calib info
		param.refName.push_back(cInfoName);

		if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}
	}
	param.refName.push_back(AQLMarketData::getBaseYieldName(currency));	
	volCreator.createVolatility(funcVec, fileVec, &param, &objPool);

	vol.setVolatility(funcVec);
}





