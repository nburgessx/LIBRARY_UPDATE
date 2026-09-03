#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <fstream>
#include "LADataInstance.h"
#include "LAFunctionManager.h"
#include "LAPriceDataManager.h"
#include "LADataReference.h"
#include "LAMathCorrelation.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LARatesTermStructureSDE.h"
#include "LARatesLJTermStructureSDE.h"
#include "LARatesCurveLinearInterpolation.h"
#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LADealUtils.h"
#include "LACalibrateModelIRSABR.h"
#include "LADataVector.h"
#include "LADataProcedure.h"
#include "LAMathYieldCurve.h"
#include "LAPriceYieldGenerator.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataFunction.h"
#include "LAMarketData.h"
#include "LAMathVolFuncBase.h"
#include "LACalibrateVolatilityIRSABR.h"
#include "LACalibrationParametersIRSABR.h"
#include "LAMathVolatility.h"
#include "LAScenarioConfiguration.h"
#include "LADefinitionsIRSABR.h"
#include "LADataMatrix.h"

using namespace std;

// constructor
/*!

*/
LACalibrateModelIRSABR::LACalibrateModelIRSABR(const LAString &baseCurrency)
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
LACalibrateModelIRSABR::setVolatility(const LAString &currency, LARatesSDEBase &sde) const
{
	return ;
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
LAString
LACalibrateModelIRSABR::getVolType(const LAString &key) const
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
LACalibrateModelIRSABR::setUpVolFunc(const LAString &currency, LAMathVolatility &vol, LADataInstance &dataInstance) const
{
	LAString key_ccy = currency;
	key_ccy.toLower();
	LAStringVector fileVec(1);
	LACalibrateVolatilityIRSABR volCreator;
	// create method vec
	vector<LAFunctionBase *> funcVec;

	MAScenarioParam param;
	LAObjectPool &objPool = dataInstance.getObjectPool();
	param.isCalib = isCalibTarget(currency);
	param.calcType = KEY_PV;
	if (param.isCalib)
	{
		LAStaticData &calibProp = LACoreDataService::getStaticDataManager().getCalibStaticData();
		LAString underlying = calibProp.getStaticData(key_ccy + STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING);
		underlying.toLower();
		LAStringVector underlyings = underlying.toToken(':');
		//get a flag to calibrate all grids
		bool isAllGridsCalibrate = false;
		LAString strIsAllCalib= calibProp.getStaticData(key_ccy + STATIC_DATA_KEY_CALIB_IRSABR_ISALLGRIDSCALIBRATE);
		if (strIsAllCalib != AQ_NO_DATA)
		{
			LADataBool tmpAttrBool;
			tmpAttrBool.convertFromString(strIsAllCalib);
			isAllGridsCalibrate = tmpAttrBool.get();
		}
		for (size_t j = 0; j < underlyings.size(); ++j)
		{
			const LAString key = key_ccy + "." CONTEXT_KEY_DEAL_IRVOL + "." + underlyings[j];
			const LAString val = LACoreDataService::getContext(key);
			const LAString fileName = calibProp.getStaticData(key_ccy + STATIC_DATA_KEY_CALIB_IRSABR_TARGETVOLGRID_FILE + "." + underlyings[j]);
			if (val == AQ_NO_DATA && fileName != AQ_NO_DATA && !isAllGridsCalibrate)
			{
				LADate asofDate = LAMarketData::getAsofDate(objPool);
				BoolMatrix calibTarget = MADealUtils::getCalibTargetIRVolGrids(objPool, asofDate, key_ccy, underlyings[j], true);
				LADataBoolMatrix tmp(calibTarget);
				LACoreDataService::setContext(key, tmp.convertToString());
			}
		}

		// create calib info
		LACalibrationParametersIRSABR cInfo;
		LAString cInfoName = cInfo.createCalibrationInfo(objPool, currency);
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





