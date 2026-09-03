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
#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LADealUtils.h"
#include "LACalibrateModelIRSABR.h"
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
#include "LAMarketData.h"
#include "AQLMathVolFuncBase.h"
#include "LACalibrateVolatilityIRSABR.h"
#include "LACalibrationParametersIRSABR.h"
#include "AQLMathVolatility.h"
#include "LAScenarioConfiguration.h"
#include "LADefinitionsIRSABR.h"
#include "AQLDataMatrix.h"

using namespace std;

// constructor
/*!

*/
LACalibrateModelIRSABR::LACalibrateModelIRSABR(const AQLString &baseCurrency)
: LACalibrateModelIR(baseCurrency)
{
}

// destructor
/*!

*/
LACalibrateModelIRSABR::~LACalibrateModelIRSABR(void)
{
}

/*!
	@brief set volatility to sde

	@param[in] currency
	@param[out] sde
*/
void
LACalibrateModelIRSABR::setVolatility(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	return ;
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
AQLString
LACalibrateModelIRSABR::getVolType(const AQLString &key) const
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
LACalibrateModelIRSABR::setUpVolFunc(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	AQLString key_ccy = currency;
	key_ccy.toLower();
	AQLStringVector fileVec(1);
	LACalibrateVolatilityIRSABR volCreator;
	// create method vec
	vector<AQLFunctionBase *> funcVec;

	MAScenarioParam param;
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	param.isCalib = isCalibTarget(currency);
	param.calcType = KEY_PV;
	if (param.isCalib)
	{
		LAStaticData &calibProp = LACoreDataService::getStaticDataManager().getCalibStaticData();
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
			const AQLString val = LACoreDataService::getContext(key);
			const AQLString fileName = calibProp.getStaticData(key_ccy + STATIC_DATA_KEY_CALIB_IRSABR_TARGETVOLGRID_FILE + "." + underlyings[j]);
			if (val == AQ_NO_DATA && fileName != AQ_NO_DATA && !isAllGridsCalibrate)
			{
				AQLDate asofDate = LAMarketData::getAsofDate(objPool);
				BoolMatrix calibTarget = MADealUtils::getCalibTargetIRVolGrids(objPool, asofDate, key_ccy, underlyings[j], true);
				AQLDataBoolMatrix tmp(calibTarget);
				LACoreDataService::setContext(key, tmp.convertToString());
			}
		}

		// create calib info
		LACalibrationParametersIRSABR cInfo;
		AQLString cInfoName = cInfo.createCalibrationInfo(objPool, currency);
		// first element set calib info
		param.refName.push_back(cInfoName);

		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}
	}
	param.refName.push_back(LAMarketData::getBaseYieldName(currency));	
	volCreator.createVolatility(funcVec, fileVec, &param, &objPool);

	vol.setVolatility(funcVec);
}





