//
// LAUpdateCurveObject.cpp
// This file used to be called CalibratorIR.cpp and before that LACalibrateModelIR.cpp
//
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAUpdateCurveObject.h"
#include <fstream>
#include "LADataInstance.h"
#include "LAFunctionManager.h"
#include "LAPriceDataManager.h"
#include "LADataReference.h"
#include "LADataMatrix.h"
#include "LAMathYieldCurve.h"
#include "LARatesCurveLinearInterpolation.h"
#include "LAStaticData.h"
#include "LADealUtils.h"
#include "LADataVector.h"
#include "LADataProcedure.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAMonotoneSplineInterpolation.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataFunction.h"
#include "LAMarketData.h"
#include "LAPriceArbFreeGenerator.h"
#include "LAFunctionUtilities.h"
#include "LADateHelpers.h"
#include "LADateScheduleHelpers.h"
#include "LACurveForwardRateHelpers.h"
#include "ParameterValidation.h"
#include "CoreEnumerations.h"

#include "LARatesTermStructureSDE.h"
#include "LARatesLJTermStructureSDE.h"
#include "LAMathCorrelation.h"
#include <time.h>

#include "CurveCalibrationData.h"
#include "CurveCalibration.h"
#include "LACurveMarketDataHelpers.h"
#include "CurveInstruments.h"
#include "CurveUtilities.h"

#include "ExceptionMacros.h"

#define STD "STD"
#define DUMMY "DUMMY"
#define LEG1FORECAST "LEG1FORECAST"
#define LEG2FORECAST "LEG2FORECAST"
#define LEG1DISCOUNT "LEG1DISCOUNT"
#define LEG2DISCOUNT "LEG2DISCOUNT"

#ifndef MARKETTYPE_SWAP
#define MARKETTYPE_SWAP "SWAP" 
#endif
#ifndef MARKETTYPE_BASIS
#define MARKETTYPE_BASIS "BASISSWAP" 
#endif
#ifndef MARKETTYPE_ZERORATE
#define MARKETTYPE_ZERORATE "ZERORATE" 
#endif
#ifndef CURVETYPE_FLOATER
#define CURVETYPE_FLOATER "FLOATER" 
#endif

using namespace std;


// constructor
LAUpdateCurveObject::LAUpdateCurveObject(const LAString &baseCurrency)
	: LAObjectPoolBase(), mBaseCurrency(baseCurrency)
{
}

// destructor
LAUpdateCurveObject::~LAUpdateCurveObject(void)
{
}

// 
/*!
	@brief set setInterpolationMethod

	@param[in] currency
	@param[out] sde
*/
void
LAUpdateCurveObject::setInterpolationMethod(const LAString &currency, LARatesSDEBase &sde) const
{
	(void)currency;
	sde.setInterpolationMethod(new LARatesCurveLinearInterpolation());
}

// 
/*!
	@brief generate sde market data

	IR is set initialvalue correlation volatility

	@param[in] currency
	@param[in] dataInstance
*/
void
LAUpdateCurveObject::loadModelDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, const bool isModel, const bool isModelData, const LAString & curveIndex, const LAString & marketName ) const
{
	if (isModel)
	{
		loadYieldCurveDataAndCalibrate( currency, dataInstance, curveIndex, marketName );
	}

	if (isModelData)
	{
		loadCorrelationDataAndCalibrate(currency, dataInstance);

		loadVolatilityDataAndCalibrate(currency, dataInstance);
	}

}

// 
/*!
	@brief generate sde initial curve data for fwdfx constant

	@param[in/out] dataInstance
*/
void
LAUpdateCurveObject::loadFwdFXConstCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, const LAString & curveID, const LAString & marketName) const
{
	if (!isFwdFXConst(currency))
		return;
	LAString ccy = currency; ccy.toLower();

	LAObjectPool &objPool = dataInstance.getObjectPool();
	bool isPricer = false;
	LAString strIsPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (strIsPricer == "TRUE") isPricer = true;
	bool isSetCurveID = false;
	LAString strIsSetCurveID = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);
	if (strIsSetCurveID == "TRUE") isSetCurveID = true;

	LAString market = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_FWDFXCONST_USEMARKET);

	// get curve ccy and name
	LAString strFCurve, strDCurve, strA_fCurve, strA_dCurve;
	LAString ccy_fCurve, ccy_dCurve, ccy_a_fCurve, ccy_a_dCurve;
	LAString fCurve, dCurve, a_fCurve, a_dCurve;
	LAString suffixLowerCase = "." + LAString(market).toLower();
	LAString epSuffix = LAString(market).toUpper();
	LAString basisTarget = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_TARGET + suffixLowerCase).toUpper();
	if (basisTarget == LEG1FORECAST || basisTarget == LEG2FORECAST)
	{
		throw LACoreInvalidData("fwdfx constant curve must be discount curve!", __FILE__, __LINE__);
	}
	else if (basisTarget == LEG1DISCOUNT)
	{
		strFCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + suffixLowerCase);
		strDCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + suffixLowerCase);
		strA_fCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + suffixLowerCase);
		strA_dCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + suffixLowerCase);
	}
	else if (basisTarget == LEG2DISCOUNT)
	{
		strFCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + suffixLowerCase);
		strDCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + suffixLowerCase);
		strA_fCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + suffixLowerCase);
		strA_dCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + suffixLowerCase);
	}
	else
	{
		throw LACoreInvalidData("basisTarget curve is invalid!", __FILE__, __LINE__);
	}
	convertCurveName(strFCurve, ccy, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(strDCurve, ccy, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(strA_fCurve, ccy, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(strA_dCurve, ccy, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	if (ccy_dCurve != ccy || ccy_fCurve != ccy || ccy_a_dCurve != ccy_a_fCurve)
		throw LACoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);

	const LAString &ycProName = LAMarketData::getBaseYieldProName(ccy);
	CurveCalibrationData &curveCalibrationData = dynamic_cast<CurveCalibrationData &>(objPool.getObject(ycProName).get());

	LAObjectHolder& yData = curveCalibrationData.getYieldData().get();
	LAString ydName = yData.getName();
	if (isSetCurveID)
		ydName = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATECURVEID);

	if (!isPricer && mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_a_fCurve))
	{
		// generate against leg ccy
		loadYieldCurveDataAndCalibrate(ccy_a_fCurve, dataInstance, curveID, marketName);
	}

	// save colateral ccy and curve
	CurveCalibrationData* colYCPro = NULL;
	if (!isPricer)
	{
		const LAString &colYCProName = LAMarketData::getBaseYieldProName(ccy_a_fCurve);
		colYCPro = &(dynamic_cast<CurveCalibrationData &>(objPool.getObject(colYCProName).get()));
		colYCPro->setColAffectingCcy(ccy.toUpper());
		curveCalibrationData.setColAffectedCcy(ccy_a_fCurve.toUpper());
	}
	const LAString &fYCName = LAMarketData::getBaseYieldName(ccy_a_fCurve);
	LAMathYieldCurve& fYC = dynamic_cast<LAMathYieldCurve &>(objPool.getObject(fYCName).get());
	const LAString &fYDName = fYC.getYieldData().get().getName();
	curveCalibrationData.getColYieldData().convertFromString(fYDName);

	// save market data
	LAObject *mktData = NULL;
	LAString basicCurveName = ydName + "_" + market.toUpper() + "_" + LAString(static_cast<int>(0));
	const LAObjectHolder ehbasis = objPool.getObject(basicCurveName);
	if (!ehbasis.isDefined())
	{
		mktData = new LAObject();
		objPool.set(basicCurveName, mktData);
	}
	else
	{
		objPool.getObject(basicCurveName).get().clear();
		mktData = &objPool.getObject(basicCurveName).get();
	}
	mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(basicCurveName);
	mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_FWDFXCONST);
	mktData->add(IR_CALIBRATION_DATA_FORECAST, new LADataString()).convertFromString(fCurve);
	mktData->add(IR_CALIBRATION_DATA_DISCOUNT, new LADataString()).convertFromString(dCurve);
	mktData->add(IR_CALIBRATION_DATA_AGTFORECAST, new LADataString()).convertFromString(a_fCurve);
	mktData->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new LADataString()).convertFromString(a_dCurve);

	LAStringVector tokens = strA_fCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (tokens.size() == 2)
	{
		LAString againstCurveCollectionID = tokens[0];
		mktData->add(IR_CALIBRATION_DATA_AGTCURVECOLLECTION, new LADataString()).convertFromString(againstCurveCollectionID);
	}

	curveCalibrationData.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + epSuffix);
	curveCalibrationData.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + epSuffix, new LADataMultiReference()).convertFromString(basicCurveName);

	// save assigned curves
	LAStringVector assignedCurves;
	LAString strAssignedCurves = mpStaticData->getStaticData(ccy.toLower() + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffixLowerCase);
	if (strAssignedCurves != MLIB_NO_DATA)
	{
		assignedCurves = strAssignedCurves.toToken(MULTI_STATIC_DATA_DELIMITER);
		for (size_t i = 0; i < assignedCurves.size(); ++i)
			curveCalibrationData.setAssignedCurveMktMap(assignedCurves[i], market);
	}
	else
	{
		assignedCurves.push_back(market);
		curveCalibrationData.setAssignedCurveMktMap(market, market);
	}

	// set curve type
	LAString curveType = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
	if (curveType != MLIB_NO_DATA)
	{
		curveCalibrationData.LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix);
		curveCalibrationData.LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix, new LADataString(curveType.toUpper()));

		yData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix);
		yData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix, new LADataString(curveType.toUpper()));
	}

	// set interpolation
	LAString genInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase);
	if (genInterp == MLIB_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}
	if (suffixLowerCase.size() == 0)
	{
		curveCalibrationData.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		curveCalibrationData.LAObject::remove(CALIBRATION_DATA_INTERPOLATION + LAString("_") + epSuffix);
		curveCalibrationData.LAObject::add(CALIBRATION_DATA_INTERPOLATION + LAString("_") + epSuffix, new LAPriceDataInterpolation()).convertFromString(genInterp);
	}

	// set yieldgen interpolation
	LAString genYieldGenInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffixLowerCase);
	if (genYieldGenInterp == MLIB_NO_DATA)
	{
		if (genInterp != MLIB_NO_DATA)
		{
			genYieldGenInterp = genInterp;
		}
		else
		{
			genYieldGenInterp = FN_SPLINEINTERPOLATION_STR;
		}
	}
	if (suffixLowerCase.size() == 0)
	{
		curveCalibrationData.getInterpolation_yg().convertFromString(genYieldGenInterp.toLower());
	}
	else
	{
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + LAString("_") + epSuffix);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + LAString("_") + epSuffix, new LAPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// calc curve
	//curveCalibrationData.calcFwdFXConstantCurve();
	curveCalibrationData.calcFwdFXConstantCurveUsingMarketName(market);// Calculate the FwdFXConstCurve but allow any FWDFXCONST marketName to be specified

	// dataout
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
		dataoutCurve(assignedCurves, yData.get(), ydName);
}

// 
/*!
	@brief generate sde initial curve data for cheapest-to-deliver curve

	@param[in/out] dataInstance
*/
void LAUpdateCurveObject::loadCheapestToDeliverCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const
{
	LAString ccy = currency;
	ccy.toLower();

	LAObjectPool &objPool = dataInstance.getObjectPool();

	bool isPricer = false;
	LAString strIsPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (strIsPricer == "TRUE")
	{
		isPricer = true;
	}

	bool isSetCurveID = false;
	LAString strIsSetCurveID = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);
	if (strIsSetCurveID == "TRUE")
	{
		isSetCurveID = true;
	}

	// Curve name and yieldCurvePro
	const LAString &ycProName = LAMarketData::getBaseYieldProName(ccy);
	LAObjectHolder objHolder = objPool.getObject(ycProName);
	if (!objHolder.isDefined())
	{
		LAString err = "#Error: CurveCollection for currency '" + ccy + "' is not defined. Please check your CurveCollection";
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	CurveCalibrationData &curveCalibrationData = dynamic_cast<CurveCalibrationData &>(objHolder.get());

	LAObjectHolder& yData = curveCalibrationData.getYieldData().get();
	LAString ydName = yData.getName();
	if (isSetCurveID)
	{
		ydName = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}

	// set asofdate
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	yData.remove(CALIBRATION_DATA_ASOFDATE);
	yData.add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));

	// set currency
	yData.remove(IR_CALIBRATION_DATA_CURRENCY);
	yData.add(IR_CALIBRATION_DATA_CURRENCY, new LADataString(currency));

	// Curve name
	LAString market = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET).toUpper();
	LAString suffixLowerCase = "." + market;
	suffixLowerCase.toLower();
	LAString epSuffix = market;
	epSuffix.toUpper();

	// save market data
	LAObject *mktData = NULL;
	LAString basicCurveName = ydName + "_" + epSuffix;
	const LAObjectHolder ehbasis = objPool.getObject(basicCurveName);
	if (!ehbasis.isDefined())
	{
		mktData = new LAObject();
		objPool.set(basicCurveName, mktData);
	}
	else
	{
		objPool.getObject(basicCurveName).get().clear();
		mktData = &objPool.getObject(basicCurveName).get();
	}

	// Convention data
	LAString calendar = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CALENDAR + suffixLowerCase).toUpper();
	LAString dayCount = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT + suffixLowerCase).toUpper();
	LAString businessAdj = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE + suffixLowerCase).toUpper();
	LAString frequency = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY + suffixLowerCase).toUpper();

	mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(calendar);
	mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(dayCount);
	mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(businessAdj);
	mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(frequency);
	mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_CTD);

	curveCalibrationData.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + epSuffix);
	curveCalibrationData.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + epSuffix, new LADataMultiReference()).convertFromString(basicCurveName);

	// save assigned curves
	LAStringVector assignedCurves;
	LAString strAssignedCurves = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffixLowerCase);
	if (strAssignedCurves != MLIB_NO_DATA)
	{
		assignedCurves = strAssignedCurves.toToken(MULTI_STATIC_DATA_DELIMITER);
		for (size_t i = 0; i < assignedCurves.size(); ++i)
			curveCalibrationData.setAssignedCurveMktMap(assignedCurves[i], market);
	}
	else
	{
		assignedCurves.push_back(market);
		curveCalibrationData.setAssignedCurveMktMap(market, market);
	}

	// set curve type
	LAString curveType = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
	if (curveType != MLIB_NO_DATA)
	{
		curveCalibrationData.LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix);
		curveCalibrationData.LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix, new LADataString(curveType.toUpper()));

		yData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix);
		yData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix, new LADataString(curveType.toUpper()));
	}

	// Get CSA collateral curves
	LAStringVector csaCurves;
	LAString csaCurvesTemp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_CTD_COLLATERALCURVES + suffixLowerCase);
	if (csaCurvesTemp != MLIB_NO_DATA)
	{
		csaCurves = csaCurvesTemp.toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		throw LACoreInvalidData("#Error: Must provide at least one CSA curve", __FILE__, __LINE__);
	}

	// set interpolation
	LAString genInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase);
	if (genInterp == MLIB_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}
	if (epSuffix.size() == 0)
	{
		curveCalibrationData.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		curveCalibrationData.LAObject::remove(CALIBRATION_DATA_INTERPOLATION + LAString("_") + epSuffix);
		curveCalibrationData.LAObject::add(CALIBRATION_DATA_INTERPOLATION + LAString("_") + epSuffix, new LAPriceDataInterpolation()).convertFromString(genInterp);
	}

	// set yieldgen interpolation
	LAString genYieldGenInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffixLowerCase);
	if (genYieldGenInterp == MLIB_NO_DATA)
	{
		if (genInterp != MLIB_NO_DATA)
		{
			genYieldGenInterp = genInterp;
		}
		else
		{
			genYieldGenInterp = FN_SPLINEINTERPOLATION_STR;
		}
	}
	if (epSuffix.size() == 0)
	{
		curveCalibrationData.getInterpolation_yg().convertFromString(genYieldGenInterp.toLower());
	}
	else
	{
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + LAString("_") + epSuffix);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + LAString("_") + epSuffix, new LAPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// calc curve
	curveCalibrationData.calcCheapestToDeliverCurve(market, csaCurves);

}

/*!
	@brief convert curve name

	@param[in] propCurve :from ir.properties
	@param[in] ccy
	@param[out] curveCcy
	@param[out] curveName
	@param[in] isPricer :call from addin or not
	@param[in] objPool
*/
void
LAUpdateCurveObject::convertCurveName(const LAString &propCurve, const LAString &ccy, LAString &curveCcy, LAString &curveName, const bool isPricer, LAObjectPool& objPool) const
{
	if (propCurve.findString(DUMMY) >= 0)
	{
		curveName = DUMMY;
	}
	else
	{
		LAStringVector propCurves = propCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (propCurves.size() == 1)
		{
			curveCcy = ccy;
			curveName = propCurves[0];
		}
		else if (propCurves.size() == 2)
		{
			if (isPricer)
			{
				LAObject* yieldData = &(objPool.getObject(propCurves[0], ENCHKTYPE_ISDEFINED).get());
				curveCcy = dynamic_cast<LADataString&>((yieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
				curveCcy.toLower();
			}
			else
			{
				curveCcy = propCurves[0]; curveCcy.toLower();
			}
			curveName = propCurves[1];
		}
		else
		{
			throw LACoreInvalidData("currency and curve name are not set!", __FILE__, __LINE__);
		}
	}
}

/*!
	@brief get Accessory from Frequency

	@param[in] freq : frequency
	@return accessory
*/
const LAString
LAUpdateCurveObject::getAccFromFreq(const LAString &freq_) const
{
	LAString freq = freq_;
	freq.toUpper();
	LAString acc;
	if (freq == ANNUAL)
	{
		acc = "1Y";
	}
	else if (freq == SEMI_ANNUAL)
	{
		acc = "6M";
	}
	else if (freq == QUARTERLY)
	{
		acc = "3M";
	}
	else if (freq == MONTHLY || freq == LUNAR)
	{
		acc = "1M";
	}
	else
	{
		LAString msg = "can't convert this frequency(" + freq_ + ") to accessory.";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return acc;
}


// 
/*!
	@brief generate curve data using dual-bootstrapping technique

	@param[in] currency  currency of the curve
	@param[out] dataInstance
*/
void
LAUpdateCurveObject::loadDualBootstrapCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

	// Get YieldCurve and CurveCalibrationData objects (as dataValues)
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAString isSetCurveID = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	LAString yieldName = LAMarketData::getBaseYieldName(currency);

	LAMathYieldCurve *yc = NULL;
	const LAObjectHolder ehyc = objPool.getObject(yieldName);
	if (!ehyc.isDefined())
	{
		yc = new LAMathYieldCurve(&dataInstance);
		objPool.set(yieldName, yc);
	}
	else
	{
		dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		yc = &dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	yc->getName().convertFromString(yieldName);

	LAString curvePropertiesName = "PRO_" + yieldName;
	CurveCalibrationData *curveCalibrationData = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(curvePropertiesName);
	if (!ehycpro.isDefined())
	{
		curveCalibrationData = new CurveCalibrationData(&dataInstance);
		objPool.set(curvePropertiesName, curveCalibrationData);
	}
	else
	{
		//we must not erase the reset method for only curveCalibrationData
		curveCalibrationData = &dynamic_cast<CurveCalibrationData &>(objPool.getObject(curvePropertiesName).get());
	}
	curveCalibrationData->getName().convertFromString(curvePropertiesName);
	bool isArbFree = false;		// London don't use this flag and hence hardcode it to False
	curveCalibrationData->getIsArbFree().set(isArbFree);

	// Get YieldData object (as an data)
	LAObject *objectPool = NULL;
	LAString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const LAObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		objectPool = new LAObject();
		objPool.set(yieldDataName, objectPool);
	}
	else
	{
		//we must not erase the reset method for only edata
		objectPool = &objPool.getObject(yieldDataName).get();
	}
	objectPool->remove(CALIBRATION_DATA_NAME);
	objectPool->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);

	// As of date
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// Main curve name
	bool enableCalculation = true;
	LAString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);

	if (target == MLIB_NO_DATA)
	{
		throw LACoreInvalidData("#Error: Dual bootstrapping curve name is not detected", __FILE__, __LINE__);
	}
	else
	{
		enableCalculation = false;
	}

	target.toUpper();
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(target));

	LAString suffixCurveObj = (target == STD || target == SWAP) ? "" : "_" + target;
	LAString suffixLowerCase = (target == STD || target == SWAP) ? "" : "." + target;
	suffixLowerCase.toLower();

	// OIS curve name and suffixLowerCase
	LAString currentCurveName_ois = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_OISCURVENAME + suffixLowerCase).toUpper();
	if (currentCurveName_ois == MLIB_NO_DATA)
	{
		throw LACoreInvalidData("#Error: OIS curve name is not found in performing dual bootstrapping", __FILE__, __LINE__);
	}

	LAString suffix_ois = (currentCurveName_ois == STD || currentCurveName_ois == SWAP) ? "" : "." + currentCurveName_ois;
	suffix_ois.toLower();

	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + suffixCurveObj, new LADataString(currentCurveName_ois));

	// Swap curve name and suffixLowerCase
	LAString currentCurveName_swap = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME + suffixLowerCase);
	currentCurveName_swap.toUpper();
	if (currentCurveName_swap == MLIB_NO_DATA)
	{
		throw LACoreInvalidData("#Error: Swap curve name is not found in performing dual bootstrapping", __FILE__, __LINE__);
	}

	LAString suffix_swap = (currentCurveName_swap == STD || currentCurveName_swap == SWAP) ? "" : "." + currentCurveName_swap;
	suffix_swap.toLower();

	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE + suffixCurveObj, new LADataString(currentCurveName_swap));

	//-----------------------------------------------------------------
	// Extract common curve building parameters

	LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	// Fast rebuild	
	LAString tmpFastRebuild_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_FASTREBUILD + suffixLowerCase).toUpper();
	bool fastRebuild = true;
	if (tmpFastRebuild_str == "FALSE")
	{
		fastRebuild = false;
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj, new LADataBool(fastRebuild));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj, new LADataBool(fastRebuild));

	//get constant for convergence
	double eps = 1.0e-9;
	LAString strEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_EPSILON + suffixLowerCase);
	if (strEPS.toUpper() != MLIB_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj, new LADataDouble(eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj, new LADataDouble(eps));

	double grad_eps = 1.0e-15;
	LAString strGEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_GRADIENTEPSILON + suffixLowerCase);
	if (strGEPS.toUpper() != MLIB_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj, new LADataDouble(grad_eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj, new LADataDouble(grad_eps));

	double delta = 1.0e-10;
	LAString strDLT = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_DELTA + suffixLowerCase);
	if (strDLT.toUpper() != MLIB_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj, new LADataDouble(delta));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj, new LADataDouble(delta));

	int maxLoop = 1000;
	LAString strMLP = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_MAXLOOP + suffixLowerCase);
	if (strMLP.toUpper() != MLIB_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj, new LADataInt(maxLoop));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj, new LADataInt(maxLoop));

	//-----------------------------------------------------------------
	// Extract individual curve's configuration parameters

	// Define a number of control modes
	bool useTenorBasis = false;
	bool useTenorBasisInstruments = false;
	bool isSpotStarting = false;

	// Set up curve Generate Config for the swap curve
	LAString suffixCurveObj_swap = (currentCurveName_swap == STD || currentCurveName_swap == SWAP) ? "" : "_" + currentCurveName_swap;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *curveCalibrationData, *objectPool, useTenorBasis, useTenorBasisInstruments, isSpotStarting, isArbFree, suffix_swap, suffixCurveObj_swap);

	// Set up curve Generate Config for the OIS curve
	LAString suffixCurveObj_ois = (currentCurveName_ois == STD || currentCurveName_ois == SWAP) ? "" : "_" + currentCurveName_ois;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *curveCalibrationData, *objectPool, useTenorBasis, useTenorBasisInstruments, isSpotStarting, isArbFree, suffix_ois, suffixCurveObj_ois);

	// Support on FX?
	LAString fxName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != MLIB_NO_DATA)
	{
		curveCalibrationData->getFXEntity().convertFromString(fxName);
	}

	// OIS curve configuration
	configureCurve(mpStaticData,
		curveCalibrationData,
		yc,
		dataInstance,
		asOfDate,
		tmpCurrency,
		currentCurveName_ois,
		yieldDataName,
		suffix_ois,
		suffixCurveObj_ois,
		enableCalculation,
		isSpotStarting,
		useTenorBasis,
		useTenorBasisInstruments);

	const std::map<LAString, LAString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
	for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		const LAString& curveName = it->first;
		curveCalibrationData->setDualBootstrapOISCurveMktMap(curveName, it->second);
	}

	// Store curve type to object pool under current engine name + current curve name
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_ois);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_ois, new LADataString("OIS"));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_ois);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_ois, new LADataString("OIS"));

	// Swap curve configuration
	configureCurve(mpStaticData,
		curveCalibrationData,
		yc,
		dataInstance,
		asOfDate,
		tmpCurrency,
		currentCurveName_swap,
		yieldDataName,
		suffix_swap,
		suffixCurveObj_swap,
		enableCalculation,
		isSpotStarting,
		useTenorBasis,
		useTenorBasisInstruments);

	// Store curve type to object pool under current engine name + current curve name
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_swap);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_swap, new LADataString("SWAP"));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_swap);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_swap, new LADataString("SWAP"));

	for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		const LAString& curveName = it->first;
		curveCalibrationData->setDualBootstrapSwapCurveMktMap(curveName, it->second);
	}

	mCurveGenCcyMap[currency] = true;

	// Store all curve names
	LAString allCurveNames = currentCurveName_ois + LAString(MULTI_STATIC_DATA_DELIMITER) + currentCurveName_swap;
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj, new LADataString(allCurveNames));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj, new LADataString(allCurveNames));

	// Get CurveCalibration out of CurveCalibrationData
	LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>(curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	const LACoreProcedure& yieldGenProcedure = modelDataObj.getMethod();
	const CurveCalibration& curveCalibrationEngine = dynamic_cast<const CurveCalibration&>(yieldGenProcedure);

	// Peform dual-bootstrapping
	LAObject* parent = dynamic_cast<LAObject*>(curveCalibrationData);
	curveCalibrationEngine.dualbootstrap(asOfDate, *parent, LADataProcedure());

}


// 
/*!
@brief generate curve data using dual-bootstrapping technique

@param[in] currency  currency of the curve
@param[out] dataInstance
*/
void LAUpdateCurveObject::loadGlobalCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

	// Get YieldCurve and CurveCalibrationData objects (as dataValues)
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAString isSetCurveID = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	LAString yieldName = LAMarketData::getBaseYieldName(currency);

	LAMathYieldCurve *yc = NULL;
	const LAObjectHolder ehyc = objPool.getObject(yieldName);
	if (!ehyc.isDefined())
	{
		yc = new LAMathYieldCurve(&dataInstance);
		objPool.set(yieldName, yc);
	}
	else
	{
		dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		yc = &dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	yc->getName().convertFromString(yieldName);

	LAString curvePropertiesName = "PRO_" + yieldName;
	CurveCalibrationData *curveCalibrationData = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(curvePropertiesName);
	if (!ehycpro.isDefined())
	{
		curveCalibrationData = new CurveCalibrationData(&dataInstance);
		objPool.set(curvePropertiesName, curveCalibrationData);
	}
	else
	{
		//we must not erase the reset method for only curveCalibrationData
		curveCalibrationData = &dynamic_cast<CurveCalibrationData &>(objPool.getObject(curvePropertiesName).get());
	}
	curveCalibrationData->getName().convertFromString(curvePropertiesName);
	bool isArbFree = false;		// London don't use this flag and hence hardcode it to False
	curveCalibrationData->getIsArbFree().set(isArbFree);

	// Get YieldData object (as an data)
	LAObject *objectPool = NULL;
	LAString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const LAObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		objectPool = new LAObject();
		objPool.set(yieldDataName, objectPool);
	}
	else
	{
		//we must not erase the reset method for only edata
		objectPool = &objPool.getObject(yieldDataName).get();
	}
	objectPool->remove(CALIBRATION_DATA_NAME);
	objectPool->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);

	LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	// As of date
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// Main curve name
	bool enableCalculation = true;
	LAString engineName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);

	if (engineName == MLIB_NO_DATA)
	{
		throw LACoreInvalidData("#Error: Name of the global yield curve calibration engine is not detected", __FILE__, __LINE__);
	}
	else
	{
		enableCalculation = false;
	}

	engineName.toUpper();
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(engineName));

	LAString suffixCurveObj = (engineName == STD || engineName == SWAP) ? "" : "_" + engineName;
	LAString suffixLowerCase = (engineName == STD || engineName == SWAP) ? "" : "." + engineName;
	suffixLowerCase.toLower();

	//-----------------------------------------------------------------
	// Extract common curve building parameters

	// Fast rebuild	
	LAString tmpFastRebuild_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_FASTREBUILD + suffixLowerCase).toUpper();
	bool fastRebuild = true;
	if (tmpFastRebuild_str == "FALSE")
	{
		fastRebuild = false;
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj, new LADataBool(fastRebuild));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj, new LADataBool(fastRebuild));

	//get constant for convergence
	double eps = 1.0e-9;
	LAString strEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_EPSILON + suffixLowerCase);
	if (strEPS.toUpper() != MLIB_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj, new LADataDouble(eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj, new LADataDouble(eps));

	double grad_eps = 1.0e-15;
	LAString strGEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_GRADIENTEPSILON + suffixLowerCase);
	if (strGEPS.toUpper() != MLIB_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj, new LADataDouble(grad_eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj, new LADataDouble(grad_eps));

	double delta = 1.0e-10;
	LAString strDLT = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_DELTA + suffixLowerCase);
	if (strDLT.toUpper() != MLIB_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj, new LADataDouble(delta));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj, new LADataDouble(delta));

	int maxLoop = 1000;
	LAString strMLP = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_MAXLOOP + suffixLowerCase);
	if (strMLP.toUpper() != MLIB_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj, new LADataInt(maxLoop));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj, new LADataInt(maxLoop));

	//-----------------------------------------------------------------------------------------------------------
	// Loop through each curve in the engine and prepare its calibration parameters and data in the object pool

	// Define a number of control modes
	bool useTenorBasis = false;
	bool useTenorBasisInstruments = false;
	bool isSpotStarting = false;

	LAString allCurveTypes = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVETYPES + suffixLowerCase);
	LAStringVector curveTypeVector = allCurveTypes.toToken(MULTI_STATIC_DATA_DELIMITER);

	LAString allCurveNames = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVENAMES + suffixLowerCase);
	LAStringVector curveNameVector = allCurveNames.toToken(MULTI_STATIC_DATA_DELIMITER);

	if (curveTypeVector.size() != curveNameVector.size())
	{
		MLIB_THROW("Not all the curves in the global curve engine has been given a curve type.");
	}

	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj, new LADataString(allCurveNames));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj, new LADataString(allCurveNames));

	for (unsigned int i = 0; i < curveNameVector.size(); ++i)
	{
		// Get single curve's name and suffixLowerCase
		LAString propertyManagerCurveName = curveNameVector[i].toUpper();
		LAString propertyManagerCurveType = curveTypeVector[i].toUpper();

		LAString suffix_currentCurve = (propertyManagerCurveName == STD || propertyManagerCurveName == SWAP) ? "" : "." + propertyManagerCurveName;
		suffix_currentCurve.toLower();

		LAString suffixCurveObj_currentCurve = (propertyManagerCurveName == STD || propertyManagerCurveName == SWAP) ? "" : "_" + propertyManagerCurveName;
		suffixCurveObj_currentCurve.toUpper();

		// Store curve type to object pool under current engine name + current curve name
		curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_currentCurve);
		curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_currentCurve, new LADataString(propertyManagerCurveType));
		yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_currentCurve);
		yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_currentCurve, new LADataString(propertyManagerCurveType));

		// Set up curve Generate Config for the current curve
		setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *curveCalibrationData, *objectPool, useTenorBasis, useTenorBasisInstruments, isSpotStarting, isArbFree, suffix_currentCurve, suffixCurveObj_currentCurve);

		// Configuration of the current single curve
		configureCurve(mpStaticData,
			curveCalibrationData,
			yc,
			dataInstance,
			asOfDate,
			tmpCurrency,
			propertyManagerCurveName,
			yieldDataName,
			suffix_currentCurve,
			suffixCurveObj_currentCurve,
			enableCalculation,
			isSpotStarting,
			useTenorBasis,
			useTenorBasisInstruments);

		//const std::map<LAString, LAString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
		//for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		//{
		//	const LAString& curveName = it->first;
		//	curveCalibrationData->setDualBootstrapOISCurveMktMap(curveName, it->second);
		//}
	}

	mCurveGenCcyMap[currency] = true;

	// Get CurveCalibration out of CurveCalibrationData
	LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>(curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	const LACoreProcedure& yieldGenProcedure = modelDataObj.getMethod();
	const CurveCalibration& curveCalibrationEngine = dynamic_cast<const CurveCalibration&>(yieldGenProcedure);

	// Peform dual-bootstrapping
	LAObject* parent = dynamic_cast<LAObject*>(curveCalibrationData);
	curveCalibrationEngine.buildEngineCurves(asOfDate, *parent, LADataProcedure());

}


/*!
	@brief
*/
void LAUpdateCurveObject::configureCurve(LAStaticData *mpStaticData,
	CurveCalibrationData *curveCalibrationData,
	LAMathYieldCurve *yc,
	LADataInstance &dataInstance,
	const LADate& asOfDate,
	const LAString& currency,
	const LAString& curveName,
	const LAString& yieldDataName,
	const LAString& suffixLowerCase,
	const LAString& suffixCurveObj,
	bool enableCalculation,
	bool isSpotStarting,
	bool useTenorBasis,
	bool useTenorBasisInstruments) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	LAString propertyManagerCurveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
	propertyManagerCurveType.toUpper();

	// reference data for market pro
	LAString staticDataName;
	LAString tenorBasisStaticDataName;
	std::map<LAString, std::map<LAString, double> > tenorBasisMarketQuotes;

	if (enableCalculation || propertyManagerCurveType == SWAP)
	{
		// Variable 'target' is essentially the name of the curve
		setUpGenCurveData(dataInstance, staticDataName, asOfDate, currency, SWAP, yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes, LAString(curveName).toUpper());
	}

	// Get isXccyMarkedToMarket
	bool isXccyMarkedToMarket = false;
	LAString isXccyMarkedToMarketString = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST + suffixLowerCase).toUpper();
	if (isXccyMarkedToMarketString == "TRUE")
	{
		isXccyMarkedToMarket = true;
	}

	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffixCurveObj, new LADataBool(isXccyMarkedToMarket));

	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET + suffixCurveObj);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET + suffixCurveObj, new LADataBool(isXccyMarkedToMarket));

	// Get the list of curve indexes
	LAStringVector listOfCurvesBuilt;
	LAString tmpMarket = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffixLowerCase).toUpper();
	if (tmpMarket == MLIB_NO_DATA)
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffixLowerCase).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(listOfCurvesBuilt);

	// set yield curve pro
	if (staticDataName.size() && (curveName == STD || curveName == SWAP))
	{
		staticDataName = staticDataName.subString(0, staticDataName.size() - 2);
		curveCalibrationData->getMarketData().convertFromString(staticDataName);
	}

	LADataHolder* dh;
	LAStringVector swapCurveDiscountFactors;
	dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		swapCurveDiscountFactors = dynamic_cast<LADataStrings &>(dh->get()).get();
	}

	LAStringVector basisCurves;
	dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		basisCurves = dynamic_cast<LADataStrings &>(dh->get()).get();
	}

	LAString tenorBasisDiscountFactors = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_DF2);
	LAStringVector tenorBasisMarkets;
	if (!listOfCurvesBuilt.empty() && listOfCurvesBuilt[0] != MLIB_NO_DATA)
	{
		for (unsigned int i = 0; i < listOfCurvesBuilt.size(); ++i)
		{
			LAString thisCurveStaticData;
			LAString suffixLowerCase = "." + listOfCurvesBuilt[i];
			suffixLowerCase.toLower();

			// set curve type			
			LAString curveType;
			LAObjectHolder& yieldData = curveCalibrationData->getYieldData().get();

			if (listOfCurvesBuilt[i] == SWAP)
			{
				curveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE);
				if (curveType != MLIB_NO_DATA)
				{
					curveCalibrationData->LAObject::remove(CALIBRATION_DATA_CURVETYPE);
					curveCalibrationData->LAObject::add(CALIBRATION_DATA_CURVETYPE, new LADataString(curveType.toUpper()));

					yieldData.remove(CALIBRATION_DATA_CURVETYPE);
					yieldData.add(CALIBRATION_DATA_CURVETYPE, new LADataString(curveType.toUpper()));
				}
			}
			else
			{
				curveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
				if (curveType != MLIB_NO_DATA)
				{
					curveCalibrationData->LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + listOfCurvesBuilt[i]);
					curveCalibrationData->LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + listOfCurvesBuilt[i], new LADataString(curveType.toUpper()));

					yieldData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + listOfCurvesBuilt[i]);
					yieldData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + listOfCurvesBuilt[i], new LADataString(curveType.toUpper()));
				}
			}

			if (listOfCurvesBuilt[i] == SWAP)
			{
				if (enableCalculation || curveName == STD)
				{
					LAString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE);
					if (aliasCurveNames != MLIB_NO_DATA)
					{
						LAStringVector assignedCurves = aliasCurveNames.toToken(':');
						for (size_t i = 0; i < assignedCurves.size(); i++)
						{
							if (swapCurveDiscountFactors.end() == std::find(swapCurveDiscountFactors.begin(), swapCurveDiscountFactors.end(), assignedCurves[i]))
							{
								swapCurveDiscountFactors.push_back(assignedCurves[i]);
							}
						}

						if (assignedCurves.end() == std::find(assignedCurves.begin(), assignedCurves.end(), STD))
						{
							curveCalibrationData->setAssignedCurveMktMap(STD, SWAP);
							if (swapCurveDiscountFactors.end() == std::find(swapCurveDiscountFactors.begin(), swapCurveDiscountFactors.end(), STD))
							{
								swapCurveDiscountFactors.push_back(STD);
							}
						}
					}
					else
					{
						if (swapCurveDiscountFactors.end() == std::find(swapCurveDiscountFactors.begin(), swapCurveDiscountFactors.end(), STD))
						{
							swapCurveDiscountFactors.push_back(STD);
						}
					}
				}
				continue;
			}

			LAString marketType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffixLowerCase).toUpper();
			curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_MARKETTYPE + LAString("_") + listOfCurvesBuilt[i]);
			curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_MARKETTYPE + LAString("_") + listOfCurvesBuilt[i], new LADataString(marketType));

			if (marketType == MLIB_NO_DATA)
			{
				LAString isBasisStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ISBASIS + suffixLowerCase).toUpper();
				LAString isReadFile = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffixLowerCase).toUpper();
				if (isBasisStr == "TRUE")
				{
					marketType = MARKETTYPE_BASIS;
				}
				else if (isReadFile == "TRUE")
				{
					marketType = MARKETTYPE_ZERORATE;
				}
				else
				{
					marketType = MARKETTYPE_SWAP;
				}
			}

			// set up interpolation
			LAString strInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase).toUpper();
			if (strInter != MLIB_NO_DATA)
			{
				yc->getInterpolation(listOfCurvesBuilt[i]).convertFromString(strInter.toLower());
				curveCalibrationData->getInterpolation(listOfCurvesBuilt[i]).convertFromString(strInter);
				if (tenorBasisDiscountFactors == listOfCurvesBuilt[i])
				{
					yc->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
					curveCalibrationData->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
				}
			}

			if (marketType == MARKETTYPE_BASIS)
			{
				if (propertyManagerCurveType == "OIS")
				{
					continue;
				}

				LAString thisMarketName = listOfCurvesBuilt[i];
				tenorBasisMarkets.push_back(listOfCurvesBuilt[i]);
				LAString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != MLIB_NO_DATA && (enableCalculation || curveName == listOfCurvesBuilt[i]))
				{
					LAStringVector assignedCurves = aliasCurveNames.toToken(':');
					for (unsigned int j = 0; j < assignedCurves.size(); j++)
					{
						if (basisCurves.end() == std::find(basisCurves.begin(), basisCurves.end(), assignedCurves[j]))
						{
							basisCurves.push_back(assignedCurves[j]);
						}
					}
					LAStringVector marketNames = listOfCurvesBuilt[i].toToken('_');
					if (marketNames.size() == 2)
					{
						setUpBasisCurveData(dataInstance, thisCurveStaticData, asOfDate, currency, marketNames[1], yieldDataName, isSpotStarting, *curveCalibrationData, curveName, &marketNames[0]);
					}
					else
					{
						setUpBasisCurveData(dataInstance, thisCurveStaticData, asOfDate, currency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, *curveCalibrationData, curveName);
					}
				}
				else
				{
					if (curveName == listOfCurvesBuilt[i])
					{
						if (basisCurves.end() == std::find(basisCurves.begin(), basisCurves.end(), listOfCurvesBuilt[i]))
						{
							basisCurves.push_back(listOfCurvesBuilt[i]);
						}
						setUpBasisCurveData(dataInstance, thisCurveStaticData, asOfDate, currency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, *curveCalibrationData, curveName);
					}
					else if (enableCalculation || (curveName == CURVETYPE_FLOATER && listOfCurvesBuilt[i] == XCCYBASIS))
					{
						setUpBasisCurveData(dataInstance, thisCurveStaticData, asOfDate, currency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, *curveCalibrationData, curveName);
					}
					else if (!enableCalculation && useTenorBasisInstruments)
					{
						//only entitysetup
						LAString tenorswapname = dynamic_cast<const LADataString &>(curveCalibrationData->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
						if (tenorswapname == listOfCurvesBuilt[i])
						{
							setUpBasisCurveData(dataInstance, thisCurveStaticData, asOfDate, currency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, *curveCalibrationData, curveName);
						}
						else
							continue;
					}
					else
					{
						continue;
					}
				}
			}
			else if (marketType == MARKETTYPE_ZERORATE)
			{
				if (!enableCalculation && curveName != listOfCurvesBuilt[i])
				{
					continue;
				}

				setUpCurveDataByReadFile(dataInstance, asOfDate, currency, listOfCurvesBuilt[i], yieldDataName, *curveCalibrationData);
				continue;
			}
			else if (marketType == MARKETTYPE_SWAP)
			{
				if (!enableCalculation && curveName != listOfCurvesBuilt[i])
				{
					continue;
				}

				LAString thisMarketName = listOfCurvesBuilt[i];
				LAString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != MLIB_NO_DATA)
				{
					LAStringVector assignedCurves = aliasCurveNames.toToken(':');
					for (size_t i = 0; i < assignedCurves.size(); i++)
					{
						if (swapCurveDiscountFactors.end() == std::find(swapCurveDiscountFactors.begin(), swapCurveDiscountFactors.end(), assignedCurves[i]))
						{
							swapCurveDiscountFactors.push_back(assignedCurves[i]);
						}
					}
				}
				else
				{
					if (swapCurveDiscountFactors.end() == std::find(swapCurveDiscountFactors.begin(), swapCurveDiscountFactors.end(), listOfCurvesBuilt[i]))
					{
						swapCurveDiscountFactors.push_back(listOfCurvesBuilt[i]);
					}
				}

				LAString oisCompoundType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffixLowerCase).toUpper();
				if (oisCompoundType.toUpper() == "DAILYCOMPOUNDING")
				{
					setUpGenCurveDataOIS(dataInstance, thisCurveStaticData, asOfDate, currency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes);
				}
				else
				{
					setUpGenCurveData(dataInstance, thisCurveStaticData, asOfDate, currency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes, curveName);
				}
			}
			else
			{
				throw LACoreInvalidData("Market type is not supported!!", __FILE__, __LINE__);
			}

			// set yield curve pro
			thisCurveStaticData = thisCurveStaticData.subString(0, thisCurveStaticData.size() - 2);
			LAStringVector marketNames = listOfCurvesBuilt[i].toToken('_');
			if (marketNames.size() == 2)
			{
				CurveCalibrationData &fYcPro = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(LAMarketData::getBaseYieldProName(marketNames[0]), ENCHKTYPE_ISDEFINED).get());
				fYcPro.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + marketNames[1]);
				fYcPro.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + marketNames[1], new LADataMultiReference()).convertFromString(thisCurveStaticData);
			}
			else
			{
				curveCalibrationData->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + listOfCurvesBuilt[i]);
				curveCalibrationData->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + listOfCurvesBuilt[i], new LADataMultiReference()).convertFromString(thisCurveStaticData);
			}
		}
	}

	//set tenorswap convention
	LAString tenorSwapName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(listOfCurvesBuilt.begin(), listOfCurvesBuilt.end(), tenorSwapName) == listOfCurvesBuilt.end() && tenorSwapName != MLIB_NO_DATA)
	{
		LAString refData_tenor = "";
		setUpBasisCurveData(dataInstance, refData_tenor, asOfDate, currency, tenorSwapName, yieldDataName, isSpotStarting, *curveCalibrationData, curveName);
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		curveCalibrationData->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName);
		curveCalibrationData->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName, new LADataMultiReference()).convertFromString(refData_tenor);
	}

	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(swapCurveDiscountFactors));

	if (tenorBasisDiscountFactors != MLIB_NO_DATA)
	{
		curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new LADataString(tenorBasisDiscountFactors));
	}

	dataInstance.getReferencePool().completeDependency();

	// Whether to generate output forward rates only from swaps or from swaps + futures/FRA
	LAString prioritizeSwaps = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY);
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY);
	if (prioritizeSwaps != MLIB_NO_DATA)
	{
		if (prioritizeSwaps.toUpper() == "TRUE")
		{
			curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new LADataBool(true));
		}
		else
		{
			curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new LADataBool(false));
		}
	}

	if (!tenorBasisMarketQuotes.empty())
	{
		map<LAString, map<LAString, double> >::const_iterator it = tenorBasisMarketQuotes.begin();
		while (it != tenorBasisMarketQuotes.end())
		{
			LACurveMarketDataHelpers::restoreSwapRateFromLibor(*curveCalibrationData, it->second, currency, &(it->first));
			++it;
		}
	}

	// Data settings that are consistent with the single-curve building mechanism of basis curve
	bool hasNoBasisCurve = false;
	hasNoBasisCurve = tenorBasisMarkets.end() == std::find(tenorBasisMarkets.begin(), tenorBasisMarkets.end(), curveName);
	LAString thisMarketName;
	if (curveName != STD)
	{
		thisMarketName = LAString(".") + curveName;
		thisMarketName.toLower();
	}

	LAString aliasCurveNames;
	if (hasNoBasisCurve)
	{
		aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + thisMarketName);
	}
	else
	{
		aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + thisMarketName);
	}

	LAString targetCurve;
	if (aliasCurveNames != MLIB_NO_DATA && aliasCurveNames != "")
	{
		LAStringVector assignedCurves = aliasCurveNames.toToken(':');
		targetCurve = assignedCurves[0];
	}
	else
	{
		MLIB_THROW("No target basis curve defined")
	}

	if (!hasNoBasisCurve)
	{
		LAString suffixLowerCase = LAString("_") + targetCurve;
		suffixLowerCase.toUpper();
		curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF + suffixLowerCase);
		curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_BASISTARGETDF + suffixLowerCase, new LADataString(targetCurve));
	}

	// basis
	bool isBasis = false;
	if (!basisCurves.empty() && !hasNoBasisCurve)
	{
		isBasis = true;
		curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_BASISDFS, new LADataStrings(basisCurves));
	}

}


// ********************************************************************************************************
// **************** HELPER METHODS FOR 'loadModelDataAndCalibrate' ROUTINE ********************************

// Method to Initialize and Check if Legacy Curves are to be Used, if boolean result is true must exit calibration
bool LAUpdateCurveObject::useLegacyCurves( const LAString &currency, LADataInstance &dataInstance ) const
{
	// Result Flag
	bool exitFlag = false;

	// Currency Names
	LAString curveCurrencyLowerCase = currency;
	curveCurrencyLowerCase.toLower();

	// Required for Legacy Method to Initialize the Curve for a Stand Alone Pricing Tool
	// ***********************************************************************************************
	LAString useLegacyYieldModel = LACoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
	if (useLegacyYieldModel != MLIB_NO_DATA)
	{
		LADataBool useYieldModelBool;
		useYieldModelBool.convertFromString(useLegacyYieldModel);
		if (useYieldModelBool.get())
		{
			generateInitialValueForPricer(currency, dataInstance);
			mCurveGenCcyMap[currency] = true;
			exitFlag = true;
			return exitFlag;
		}
	}
	
	// Required for Legacy Aribitrage-Free Curves
	// ***********************************************************************************************
	bool useLegacyArbitrageFreeCurve = false;
	LAString isArbitrageFreeString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_ISARBFREE);
	if (isArbitrageFreeString != MLIB_NO_DATA)
	{
		LADataBool useArbFreeModelBool;
		useArbFreeModelBool.convertFromString(isArbitrageFreeString);
		useLegacyArbitrageFreeCurve = useArbFreeModelBool.get();
	}
	if (useLegacyArbitrageFreeCurve)
	{
		generateInitialValueArbfree(currency, dataInstance);
		mCurveGenCcyMap[currency] = true;
		exitFlag = true;
		return exitFlag;
	}

	// Exit Flag Result;
	return exitFlag;
}

// Method to Initialize the Curve Object and Import Property Manager Data
void LAUpdateCurveObject::updateCurveObjectAndImportPropertyManagerData( CurveObject & curveObj ) const
{
	// This is a wrapper method to consolidate the legacy curve object mess
	// ---------------------------------------------------------------------------------------------------------------
	// The update order appears a bit random, however the the routine is fragile and sensitive as to data update order ...
	// ... and in particular when the updateCurveObjectMarketData method is called.
	
	// 1. Initialize the Curve Object importing property manager curve defaults
	initializeCurveObject( curveObj );

	// 2.	Update Curve Object by importing Property Manager Data
	updateCurveObjectPropertyMangerData( curveObj );

	// 3.	Update Curve Object Risk Data
	updateCurveObjectRiskData( curveObj );
	
	// 4.	Update STD Swap Curve Object Instrument Cashflows & Market Data
	updateCurveObjectSwapCurveInstrumentsAndCashflows( curveObj );

	// 5.	Get Xccy & Fwd FX Parameters
	updateCurveObjectXccyAndFXForwardParameters( curveObj );

	// 6.	Get List of Curves Built that can be Ignored
	updateCurveObjectListOfCurvesBuilt( curveObj );

	// 7.	Set-Up Tenor-Basis Curve Object if Required
	updateCurveObjectBasisCurveInstrumentParameters( curveObj );

	// 8.	Update Curve Object Market Data
	updateCurveObjectMarketData( curveObj );
	
	// 9.	Update Curve Object using Property Manager Settings
	updateCurveObjectBasisCurveInstrumentCashflows( curveObj );

	// 10.	Update Curve Object Instrument Priority
	updateCurveObjectInstrumentPriority( curveObj );

	// 11.	Update Curve Object Target Discount Factors
	updateCurveObjectTargetDiscountFactors( curveObj );
}

// Method to Initialize the Curve Object, where curve parameters are stored for calibration
void LAUpdateCurveObject::initializeCurveObject( CurveObject & curveObj ) const
{
	// Currency Names
	LAString curveCurrencyLowerCase = curveObj.currency_;
	curveCurrencyLowerCase.toLower();
	
	// Object Pool
	LAObjectPool & objPool = curveObj.dataInstance_.getObjectPool();

	LAString yieldName = LAMarketData::getBaseYieldName(curveObj.currency_);
	LAString isSetCurveID = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	curveObj.mathObj_ = NULL;
	const LAObjectHolder yieldCurveObjHolder = objPool.getObject(yieldName);
	if (!yieldCurveObjHolder.isDefined())
	{
		curveObj.mathObj_ = new LAMathYieldCurve(&curveObj.dataInstance_);
		objPool.set(yieldName, curveObj.mathObj_);
	}
	else
	{
		dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		curveObj.mathObj_ = &dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	curveObj.mathObj_->getName().convertFromString(yieldName);

	LAString curvePropertiesName = "PRO_" + yieldName;
	curveObj.calibrationData_ = NULL;
	const LAObjectHolder yieldCurvePropertiesObj = objPool.getObject(curvePropertiesName);
	if (!yieldCurvePropertiesObj.isDefined())
	{
		curveObj.calibrationData_ = new CurveCalibrationData(&curveObj.dataInstance_);
		objPool.set(curvePropertiesName, curveObj.calibrationData_);
	}
	else
	{
		//we must not erase the reset method for only curveCalibrationData
		curveObj.calibrationData_ = &dynamic_cast<CurveCalibrationData &>(objPool.getObject(curvePropertiesName).get());
	}
	curveObj.calibrationData_->getName().convertFromString(curvePropertiesName);
	curveObj.calibrationData_->getIsArbFree().set(false);

	curveObj.instance_ = NULL;
	curveObj.name_ = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		curveObj.name_ = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const LAObjectHolder ehdata = objPool.getObject(curveObj.name_);
	if (!ehdata.isDefined())
	{
		curveObj.instance_ = new LAObject();
		objPool.set(curveObj.name_, curveObj.instance_);
	}
	else
	{
		//we must not erase the reset method for only edata
		curveObj.instance_ = &objPool.getObject(curveObj.name_).get();
	}
	curveObj.instance_->remove(CALIBRATION_DATA_NAME);
	curveObj.instance_->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(curveObj.name_);


	// Enable Curve Calculations Only if Target Curve Type Specified
	// **************************************************************************************
	curveObj.enableCalculation_ = true;
	curveObj.targetCurveType_ = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET).toUpper();
	if (curveObj.targetCurveType_ != MLIB_NO_DATA)
	{
		curveObj.enableCalculation_ = false;
	}
}

// Method to Initialise the Curve Object using Property Manager Config Data
void LAUpdateCurveObject::updateCurveObjectPropertyMangerData( CurveObject & curveObj ) const
{
	LAString suffixLowerCase = (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP) ? "" : "." + curveObj.targetCurveType_;
	suffixLowerCase.toLower();

	LAString suffixCurveObj = (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP) ? "" : "_" + curveObj.targetCurveType_;
	bool useLegacyArbitrageFreeCurve = false;

	// Initialize Curve Object using Property Manager Config Data
	setUpGenerateConfig( curveObj.dataInstance_,
						 curveObj.asOfDate_,
						 curveObj.currency_,
						 *curveObj.mathObj_,
						 *curveObj.calibrationData_,
						 *curveObj.instance_,
						 curveObj.useTenorBasisExtrapolation_,
						 curveObj.useTenorBasisInstruments_,
						 curveObj.isSpotStarting_,
						 useLegacyArbitrageFreeCurve,
						 suffixLowerCase,
						 suffixCurveObj );

	LAString fxName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != MLIB_NO_DATA)
	{
		curveObj.calibrationData_->getFXEntity().convertFromString(fxName);
	}
}

// Method to Initialize Risk Data
void LAUpdateCurveObject::updateCurveObjectRiskData( CurveObject & curveObj ) const
{
	// Context Key for Curve Data Interface
	LAString contextKey = curveObj.currency_;
	contextKey.toLower();
	LAString curveContext = LACoreDataService::getContext(contextKey + CONTEXT_KEY_SDE_YIELD);

	// Do we have the Risk Results?
	bool doesRiskDataExist = false;
	
	// Use Bitwise OR here if LHS or RHS is true the result is true
	doesRiskDataExist |= LACoreDataService::getContext(ARG_KEY_OFFICIALRISK) != MLIB_NO_DATA ? true : false;
	doesRiskDataExist |= LACoreDataService::getContext(ARG_KEY_FRONTRISK)	 != MLIB_NO_DATA ? true : false;
		
	// Update Risk if we have the Risk Results and they are Missing from the Curve Context / Data Instance
	if ( !doesRiskDataExist && curveContext != MLIB_NO_DATA )
	{
		LADataDoubleMatrix matrix;
		matrix.convertFromString(curveContext);
		unsigned int rowSize = matrix.get1DSize();
		unsigned int colSize = matrix.getSize(0);

		bool isBasisDiscountFactor = colSize > 2 ? true : false;
		DoubleArray terms(rowSize);
		DoubleArray dfs(rowSize);
		DoubleArray dfs2(rowSize);
		for (unsigned int i = 0; i < rowSize; i++)
		{
			terms[i] = matrix.get(i, 0);
			dfs[i] = matrix.get(i, 1);

			if (isBasisDiscountFactor)
				dfs2[i] = matrix.get(i, 2);
		}
		dynamic_cast<LADataDoubles &>(curveObj.instance_->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<LADataDoubles &>(curveObj.instance_->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		if (isBasisDiscountFactor)
		{
			LADataDoubles* attrDF2 = NULL;
			LADataHolder* df2H = &curveObj.instance_->getData(IR_CALIBRATION_DATA_DFS2);
			if (!df2H->isDefined())
			{
				attrDF2 = new LADataDoubles();
				curveObj.instance_->remove(IR_CALIBRATION_DATA_DFS2);
				curveObj.instance_->add(IR_CALIBRATION_DATA_DFS2, attrDF2);
			}
			else
			{
				attrDF2 = dynamic_cast<LADataDoubles*>(&df2H->get());
			}
			attrDF2->set(dfs2);
		}
	}
}

// Method to Set-Up Curve Market Data
void LAUpdateCurveObject::updateCurveObjectMarketData( CurveObject & curveObj ) const
{
	// CURVE BUILDING
	// Rebuild curves if the curve index is missing from the useMarkets or listOfCurvesBuilt list
	
	// *** Important Note ***
	// The ir.Properties file hard codes the listOfCurvesBuilt list in some instances to indicate which curves *** NOT *** to build

	// Object Pool
	LAObjectPool & objPool = curveObj.dataInstance_.getObjectPool();

	// set curve properties
	if (curveObj.staticDataName_.size() > 0 && (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP))
	{
		curveObj.staticDataName_ = curveObj.staticDataName_.subString(0, curveObj.staticDataName_.size() - 2);
		curveObj.calibrationData_->getMarketData().convertFromString(curveObj.staticDataName_);
	}

	LADataHolder* dh;
	dh = &curveObj.calibrationData_->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		curveObj.swapCurveDiscountFactors_ = dynamic_cast<LADataStrings &>(dh->get()).get();
	}

	dh = &curveObj.calibrationData_->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		curveObj.tenorBasisCurves_ = dynamic_cast<LADataStrings &>(dh->get()).get();
	}

	curveObj.tenorBasisDiscountFactors_ = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_DF2);
	if (!curveObj.listOfCurvesBuilt_.empty() && curveObj.listOfCurvesBuilt_[0] != MLIB_NO_DATA)
	{
		for (unsigned int i = 0; i < curveObj.listOfCurvesBuilt_.size(); ++i)
		{
			LAString thisCurveStaticData;
			LAString suffixLowerCase = "." + curveObj.listOfCurvesBuilt_[i];
			suffixLowerCase.toLower();

			// set curve type			
			LAString curveType;
			LAObjectHolder& yieldData = curveObj.calibrationData_->getYieldData().get();

			if (curveObj.listOfCurvesBuilt_[i] == SWAP)
			{
				curveType = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE);
				if (curveType != MLIB_NO_DATA)
				{
					curveObj.calibrationData_->LAObject::remove(CALIBRATION_DATA_CURVETYPE);
					curveObj.calibrationData_->LAObject::add(CALIBRATION_DATA_CURVETYPE, new LADataString(curveType.toUpper()));

					yieldData.remove(CALIBRATION_DATA_CURVETYPE);
					yieldData.add(CALIBRATION_DATA_CURVETYPE, new LADataString(curveType.toUpper()));
				}
			}
			else
			{
				curveType = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
				if (curveType != MLIB_NO_DATA)
				{
					curveObj.calibrationData_->LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveObj.listOfCurvesBuilt_[i]);
					curveObj.calibrationData_->LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveObj.listOfCurvesBuilt_[i], new LADataString(curveType.toUpper()));

					yieldData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveObj.listOfCurvesBuilt_[i]);
					yieldData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveObj.listOfCurvesBuilt_[i], new LADataString(curveType.toUpper()));
				}
			}

			if (curveObj.listOfCurvesBuilt_[i] == SWAP)
			{
				if (curveObj.enableCalculation_ || curveObj.targetCurveType_ == STD)
				{
					LAString aliasCurveNames = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE);
					if (aliasCurveNames != MLIB_NO_DATA)
					{
						LAStringVector assignedCurves = aliasCurveNames.toToken(':');
						for (size_t i = 0; i < assignedCurves.size(); i++)
						{
							if (curveObj.swapCurveDiscountFactors_.end() == std::find(curveObj.swapCurveDiscountFactors_.begin(), curveObj.swapCurveDiscountFactors_.end(), assignedCurves[i]))
							{
								curveObj.swapCurveDiscountFactors_.push_back(assignedCurves[i]);
							}
						}

						if (assignedCurves.end() == std::find(assignedCurves.begin(), assignedCurves.end(), STD))
						{
							curveObj.calibrationData_->setAssignedCurveMktMap(STD, SWAP);
							if (curveObj.swapCurveDiscountFactors_.end() == std::find(curveObj.swapCurveDiscountFactors_.begin(), curveObj.swapCurveDiscountFactors_.end(), STD))
							{
								curveObj.swapCurveDiscountFactors_.push_back(STD);
							}
						}
					}
					else
					{
						if (curveObj.swapCurveDiscountFactors_.end() == std::find(curveObj.swapCurveDiscountFactors_.begin(), curveObj.swapCurveDiscountFactors_.end(), STD))
						{
							curveObj.swapCurveDiscountFactors_.push_back(STD);
						}
					}
				}
				continue;
			}

			LAString marketType = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffixLowerCase).toUpper();
			curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_MARKETTYPE + LAString("_") + curveObj.listOfCurvesBuilt_[i]);
			curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_MARKETTYPE + LAString("_") + curveObj.listOfCurvesBuilt_[i], new LADataString(marketType));

			if (marketType == MLIB_NO_DATA)
			{
				LAString isBasisStr = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ISBASIS + suffixLowerCase).toUpper();
				LAString isReadFile = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ISREADFILE + suffixLowerCase).toUpper();
				if (isBasisStr == "TRUE")
				{
					marketType = MARKETTYPE_BASIS;
				}
				else if (isReadFile == "TRUE")
				{
					marketType = MARKETTYPE_ZERORATE;
				}
				else
				{
					marketType = MARKETTYPE_SWAP;
				}
			}

			// set up interpolation
			LAString strInter = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase).toUpper();
			if (strInter != MLIB_NO_DATA)
			{
				curveObj.mathObj_->getInterpolation(curveObj.listOfCurvesBuilt_[i]).convertFromString(strInter.toLower());
				curveObj.calibrationData_->getInterpolation(curveObj.listOfCurvesBuilt_[i]).convertFromString(strInter);
				if (curveObj.tenorBasisDiscountFactors_ == curveObj.listOfCurvesBuilt_[i])
				{
					curveObj.mathObj_->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
					curveObj.calibrationData_->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
				}
			}

			if (marketType == MARKETTYPE_BASIS)
			{
				if (curveObj.propertyManagerCurveType_ == "OIS")
				{
					continue;
				}

				LAString thisMarketName = curveObj.listOfCurvesBuilt_[i];
				curveObj.tenorBasisMarkets_.push_back(curveObj.listOfCurvesBuilt_[i]);
				LAString  aliasCurveNames = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != MLIB_NO_DATA && (curveObj.enableCalculation_ || curveObj.targetCurveType_ == curveObj.listOfCurvesBuilt_[i]))
				{
					LAStringVector assignedCurves = aliasCurveNames.toToken(':');
					for (unsigned int j = 0; j < assignedCurves.size(); j++)
					{
						if (curveObj.tenorBasisCurves_.end() == std::find(curveObj.tenorBasisCurves_.begin(), curveObj.tenorBasisCurves_.end(), assignedCurves[j]))
						{
							curveObj.tenorBasisCurves_.push_back(assignedCurves[j]);
						}
					}
					LAStringVector marketNames = curveObj.listOfCurvesBuilt_[i].toToken('_');
					if (marketNames.size() == 2)
					{
						setUpBasisCurveData( curveObj.dataInstance_,
											 thisCurveStaticData,
											 curveObj.asOfDate_,
											 curveObj.currencyLowercase_,
											 marketNames[1],
											 curveObj.name_,
											 curveObj.isSpotStarting_,
											 *curveObj.calibrationData_,
											 curveObj.targetCurveType_,
											 &marketNames[0] );
					}
					else
					{
						setUpBasisCurveData( curveObj.dataInstance_,
											 thisCurveStaticData,
											 curveObj.asOfDate_,
											 curveObj.currencyLowercase_,
											 curveObj.listOfCurvesBuilt_[i],
											 curveObj.name_,
											 curveObj.isSpotStarting_,
											 *curveObj.calibrationData_,
											 curveObj.targetCurveType_ );
					}
				}
				else
				{
					if (curveObj.targetCurveType_ == curveObj.listOfCurvesBuilt_[i])
					{
						if (curveObj.tenorBasisCurves_.end() == std::find(curveObj.tenorBasisCurves_.begin(), curveObj.tenorBasisCurves_.end(), curveObj.listOfCurvesBuilt_[i]))
						{
							curveObj.tenorBasisCurves_.push_back(curveObj.listOfCurvesBuilt_[i]);
						}
						setUpBasisCurveData( curveObj.dataInstance_,
											 thisCurveStaticData,
											 curveObj.asOfDate_,
											 curveObj.currencyLowercase_,
											 curveObj.listOfCurvesBuilt_[i],
											 curveObj.name_,
											 curveObj.isSpotStarting_,
											 *curveObj.calibrationData_,
											 curveObj.targetCurveType_ );
					}
					else if (curveObj.enableCalculation_ || (curveObj.targetCurveType_ == CURVETYPE_FLOATER && curveObj.listOfCurvesBuilt_[i] == XCCYBASIS))
					{
						setUpBasisCurveData( curveObj.dataInstance_,
											 thisCurveStaticData,
											 curveObj.asOfDate_,
											 curveObj.currencyLowercase_,
											 curveObj.listOfCurvesBuilt_[i],
											 curveObj.name_,
											 curveObj.isSpotStarting_,
											 *curveObj.calibrationData_,
											 curveObj.targetCurveType_ );
					}
					else if (!curveObj.enableCalculation_ && curveObj.useTenorBasisInstruments_)
					{
						//only entitysetup
						LAString tenorswapname = dynamic_cast<const LADataString &>(curveObj.calibrationData_->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
						if (tenorswapname == curveObj.listOfCurvesBuilt_[i])
						{
							setUpBasisCurveData( curveObj.dataInstance_,
												 thisCurveStaticData,
												 curveObj.asOfDate_,
												 curveObj.currencyLowercase_,
												 curveObj.listOfCurvesBuilt_[i],
												 curveObj.name_,
												 curveObj.isSpotStarting_,
												 *curveObj.calibrationData_,
												 curveObj.targetCurveType_ );
						}
						else
							continue;
					}
					else
					{
						continue;
					}
				}
			}
			else if (marketType == MARKETTYPE_ZERORATE)
			{
				if (!curveObj.enableCalculation_ && curveObj.targetCurveType_ != curveObj.listOfCurvesBuilt_[i])
				{
					continue;
				}

				setUpCurveDataByReadFile( curveObj.dataInstance_,
										  curveObj.asOfDate_,
										  curveObj.currencyLowercase_,
										  curveObj.listOfCurvesBuilt_[i],
										  curveObj.name_,
										  *curveObj.calibrationData_ );
				continue;
			}
			else if (marketType == MARKETTYPE_SWAP)
			{
				if (!curveObj.enableCalculation_ && curveObj.targetCurveType_ != curveObj.listOfCurvesBuilt_[i])
				{
					continue;
				}

				LAString thisMarketName = curveObj.listOfCurvesBuilt_[i];
				LAString aliasCurveNames = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != MLIB_NO_DATA)
				{
					LAStringVector assignedCurves = aliasCurveNames.toToken(':');
					for (size_t i = 0; i < assignedCurves.size(); i++)
					{
						if (curveObj.swapCurveDiscountFactors_.end() == std::find(curveObj.swapCurveDiscountFactors_.begin(), curveObj.swapCurveDiscountFactors_.end(), assignedCurves[i]))
						{
							curveObj.swapCurveDiscountFactors_.push_back(assignedCurves[i]);
						}
					}
				}
				else
				{
					if (curveObj.swapCurveDiscountFactors_.end() == std::find(curveObj.swapCurveDiscountFactors_.begin(), curveObj.swapCurveDiscountFactors_.end(), curveObj.listOfCurvesBuilt_[i]))
					{
						curveObj.swapCurveDiscountFactors_.push_back(curveObj.listOfCurvesBuilt_[i]);
					}
				}

				LAString oisCompoundType = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffixLowerCase).toUpper();
				if (oisCompoundType.toUpper() == "DAILYCOMPOUNDING")
				{
					setUpGenCurveDataOIS( curveObj.dataInstance_,
										  thisCurveStaticData,
										  curveObj.asOfDate_,
										  curveObj.currencyLowercase_,
										  curveObj.listOfCurvesBuilt_[i],
										  curveObj.name_,
										  curveObj.isSpotStarting_,
										  curveObj.useTenorBasisExtrapolation_,
										  *curveObj.calibrationData_,
										  curveObj.tenorBasisMarketQuotes_ );
				}
				else
				{
					setUpGenCurveData( curveObj.dataInstance_,
									   thisCurveStaticData,
									   curveObj.asOfDate_,
									   curveObj.currencyLowercase_,
									   curveObj.listOfCurvesBuilt_[i],
									   curveObj.name_,
									   curveObj.isSpotStarting_,
									   curveObj.useTenorBasisExtrapolation_,
									   *curveObj.calibrationData_,
									   curveObj.tenorBasisMarketQuotes_,
									   curveObj.targetCurveType_ );
				}
			}
			else
			{
				throw LACoreInvalidData("Invalid Market Data Type", __FILE__, __LINE__);
			}

			// set yield curve properties
			thisCurveStaticData = thisCurveStaticData.subString(0, thisCurveStaticData.size() - 2);
			LAStringVector marketNames = curveObj.listOfCurvesBuilt_[i].toToken('_');
			if (marketNames.size() == 2)
			{
				CurveCalibrationData &yieldCurveProperties = dynamic_cast<CurveCalibrationData &> (objPool.getObject(LAMarketData::getBaseYieldProName(marketNames[0]), ENCHKTYPE_ISDEFINED).get());
				yieldCurveProperties.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + marketNames[1]);
				yieldCurveProperties.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + marketNames[1], new LADataMultiReference()).convertFromString(thisCurveStaticData);
			}
			else
			{
				curveObj.calibrationData_->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + curveObj.listOfCurvesBuilt_[i]);
				curveObj.calibrationData_->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + curveObj.listOfCurvesBuilt_[i], new LADataMultiReference()).convertFromString(thisCurveStaticData);
			}
		}
	}
}

// Method to Update the Curve Object with Swap Curve Instruments and Cashflows
void LAUpdateCurveObject::updateCurveObjectSwapCurveInstrumentsAndCashflows( CurveObject & curveObj ) const
{
	// Current curve type is the type of the curve currently being constructed
	// To be distinguished from the curve type within the for loop below
	LAString propertyManagerCurveName("");
	if (curveObj.targetCurveType_ != STD)
	{
		propertyManagerCurveName = "." + curveObj.targetCurveType_;
		propertyManagerCurveName.toLower();
	}
	curveObj.propertyManagerCurveType_ = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + propertyManagerCurveName);
	curveObj.propertyManagerCurveType_.toUpper();

	if ( curveObj.enableCalculation_ || curveObj.propertyManagerCurveType_ == SWAP)
	{
		// Variable 'targetCurveType' is essentially the name of the curve
		setUpGenCurveData( curveObj.dataInstance_,
						   curveObj.staticDataName_,
						   curveObj.asOfDate_,
						   curveObj.currencyLowercase_,
						   SWAP,
						   curveObj.name_,
						   curveObj.isSpotStarting_,
						   curveObj.useTenorBasisExtrapolation_,
						   *curveObj.calibrationData_,
						   curveObj.tenorBasisMarketQuotes_,
						   LAString(curveObj.targetCurveType_).toUpper() );
	}
}

// Method to Set-Up Tenor Basis Curve Data
void LAUpdateCurveObject::updateCurveObjectBasisCurveInstrumentParameters( CurveObject & curveObj ) const
{
	// Currency
	LAString curveCurrencyLowerCase = curveObj.currency_;
	curveCurrencyLowerCase.toLower();

	// Object Pool
	LAObjectPool & objPool = curveObj.dataInstance_.getObjectPool();

	// Set-Up Tenor-Basis Curve
	if ( curveObj.listOfCurvesBuilt_.empty() || curveObj.listOfCurvesBuilt_[0] == MLIB_NO_DATA)
	{
		LAString basisCurrency = mpStaticData->getStaticData(KEY_SDE_BASIS_BASE_CURRENCY);
		basisCurrency.toUpper();

		// Set-Up Basis Curve if Basis Currency Provided
		if ( ( MADealUtils::getSDECurrencys().size() > 1 )
			 && (curveCurrencyLowerCase.toUpper() != basisCurrency )
			 && (basisCurrency != MLIB_NO_DATA) )
		{
			curveCurrencyLowerCase.toLower();
			// if not base currency set basis curve
			LAString basisFileName = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASISSWAP_FILE);
			MAFileAccessor basisFile(LAMarketData::getNumFileName(basisFileName));
			LAStringMatrix basisDataMtx;
			basisFile.readAllData(MARKET_DATA_DELIMITER, basisDataMtx);
			basisFile.close();

			// get cal and calc spot date
			LAPriceDataCalendar tenorBasisCalendar;
			LAString tenorBasisCalendarString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_CALENDAR);
			tenorBasisCalendar.convertFromString(tenorBasisCalendarString);
			LADate tenorBasisSpotDate;
			if (curveObj.isSpotStarting_)
			{
				tenorBasisSpotDate.setDate(mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_SPOTDATE).getCString());
			}
			else
			{
				tenorBasisSpotDate = tenorBasisCalendar.getBusinessDay(curveObj.asOfDate_, mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_RESETLAG).getIntValue());
			}
			// get freq
			LAString tenorBasisFrequencyString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY).toUpper();
			// get daycount
			LAString tenorBasisDaycountString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT).toUpper();
			// get sliding
			LAString tenorBasisBusDayAdjString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE).toUpper();
			// get freq
			LAString tenorBasisIndexFrequencyString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_BASEFREQUENCY).toUpper();
			// get daycount
			LAString tenorBasisIndexDaycountString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_BASEDAYCOUNT).toUpper();
			// get sliding
			LAString tenorBasisIndexBusDayAdjString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_BASESLIDINGRULE).toUpper();
			// get calendar
			LAString tenorBasisIndexCalendarString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_BASECALENDAR);
			// baseyield name
			LAString baseCurrency = curveObj.currency_;
			LAString baseYieldName = PREFIX_YIELD + getSDEAttrName(baseCurrency.toUpper());

			const int basisSize = basisDataMtx.size();
			for (int i = 0; i < basisSize; ++i)
			{
				LAString term = basisDataMtx[i][0].toUpper();
				double basis = basisDataMtx[i][1].getDoubleValue();

				LAObject *mktData = new LAObject();
				LAString basicCurveName = curveObj.name_ + "_BASIS_" + LAString(i);
				curveObj.staticDataName_ += basicCurveName + ":";
				curveObj.tenorBasisStaticDataName_ += basicCurveName + ":";
				
				// set name
				mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(basicCurveName);
				// set spot date
				mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(tenorBasisSpotDate));
				// set calendar
				mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(tenorBasisCalendarString);
				// set daycount
				mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(tenorBasisDaycountString);
				// set data type
				mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BASIS);
				// set frequency
				mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(tenorBasisFrequencyString);
				// set slidingrule
				mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(tenorBasisBusDayAdjString);
				// set term
				mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));
				// set rate
				mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(basis / 10000.0) );
				// set base basis info
				// freq
				mktData->add(IR_CALIBRATION_DATA_FREQUENCYBASE, new LADataString).convertFromString(tenorBasisIndexFrequencyString);
				// calender
				mktData->add(IR_CALIBRATION_DATA_CALENDARBASE, new LAPriceDataCalendar()).convertFromString(tenorBasisIndexCalendarString);
				// daycount
				mktData->add(IR_CALIBRATION_DATA_DAYCOUNTBASE, new LAPriceDataDayCount()).convertFromString(tenorBasisIndexDaycountString);
				// sliding
				mktData->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE, new LAPriceDataSlidingRule()).convertFromString(tenorBasisIndexBusDayAdjString);

				// set object pool
				objPool.set(basicCurveName, mktData);
			}
			// set base curve reference
			curveObj.calibrationData_->getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(baseYieldName);
		}
	}
}

// Method to Update the Curve Object with Xccy and FX Forward Curve Parameters
void LAUpdateCurveObject::updateCurveObjectXccyAndFXForwardParameters( CurveObject & curveObj ) const
{
	LAString suffixLowerCase = (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP) ? "" : "." + curveObj.targetCurveType_;
	suffixLowerCase.toLower();

	LAString suffixCurveObj = (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP) ? "" : "_" + curveObj.targetCurveType_;

	// set isXccyMarkedToMarket
	bool isXccyMarkedToMarket = false;
	LAString isXccyMarkedToMarketString = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST + suffixLowerCase).toUpper();
	if (isXccyMarkedToMarketString == "TRUE")
	{
		isXccyMarkedToMarket = true;
	}

	// RENOTIONAL ADJUST HAS ALIAS XCCY MARKED TO MARKET
	curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffixCurveObj);
	curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffixCurveObj, new LADataBool(isXccyMarkedToMarket));

	curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET + suffixCurveObj);
	curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET + suffixCurveObj, new LADataBool(isXccyMarkedToMarket));

	// IsForwardFX
	curveObj.isForwardFXCurve_ = false;
	LAString isFwdFXStr = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX);
	if (isFwdFXStr.toUpper() == "TRUE")
	{
		curveObj.isForwardFXCurve_ = true;
	}
}

// Method to Update Curve Data with List of Curves Built
void LAUpdateCurveObject::updateCurveObjectListOfCurvesBuilt( CurveObject & curveObj ) const
{
	// Property Manager: 'useMarkets' / 'listOfCurvesBuilt' parameter
	// The useMarkets parameter controls which curves indices to calibrate. If a curve index / market is missing the property
	// manager code will try to build any missing curve index. Therefore adding a market to the list, tells this code *NOT* to 
	// calibrate it since the code will think it exists already. 

	LAString existingMarketsString = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (existingMarketsString == MLIB_NO_DATA)
	{
		curveObj.listOfCurvesBuilt_ = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		curveObj.listOfCurvesBuilt_ = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(curveObj.listOfCurvesBuilt_);
}

// Method to Update the Curve Object with Basis Curve Instruments and Cashflows
void LAUpdateCurveObject::updateCurveObjectBasisCurveInstrumentCashflows( CurveObject & curveObj ) const
{
	// Set tenorswap convention
	LAString tenorSwapName = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(curveObj.listOfCurvesBuilt_.begin(), curveObj.listOfCurvesBuilt_.end(), tenorSwapName) == curveObj.listOfCurvesBuilt_.end() && tenorSwapName != MLIB_NO_DATA)
	{
		LAString refData_tenor = "";

		// Set-Up Basis Curve Instruments and Cashflows
		setUpBasisCurveData( curveObj.dataInstance_, refData_tenor, curveObj.asOfDate_, curveObj.currencyLowercase_, tenorSwapName, curveObj.name_, curveObj.isSpotStarting_, *curveObj.calibrationData_, curveObj.targetCurveType_);
		
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		curveObj.calibrationData_->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName);
		curveObj.calibrationData_->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName, new LADataMultiReference()).convertFromString(refData_tenor);
	}

	curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(curveObj.swapCurveDiscountFactors_));

	if (curveObj.tenorBasisDiscountFactors_ != MLIB_NO_DATA)
	{
		curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new LADataString(curveObj.tenorBasisDiscountFactors_));
	}
}

// Method to Update the Curve Object Intrument Priority
void LAUpdateCurveObject::updateCurveObjectInstrumentPriority( CurveObject & curveObj ) const
{
	// Update parameters that control whether to prioritise swap calibration instruments
	// Whether to generate output forward rates only from swaps or from swaps + futures/FRA
	LAString prioritizeSwaps = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY);
	curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY);
	if (prioritizeSwaps != MLIB_NO_DATA)
	{
		if (prioritizeSwaps.toUpper() == "TRUE")
		{
			curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new LADataBool(true));
		}
		else
		{
			curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new LADataBool(false));
		}
	}
}

// Update the Curve Object with the Target Curve Discount Factors
void LAUpdateCurveObject::updateCurveObjectTargetDiscountFactors( CurveObject & curveObj ) const
{
	if (!curveObj.enableCalculation_)
	{
		curveObj.isTargetCurveFound_ = ( curveObj.targetCurveType_ != CURVETYPE_FLOATER ) && ( curveObj.tenorBasisMarkets_.end() == std::find(curveObj.tenorBasisMarkets_.begin(), curveObj.tenorBasisMarkets_.end(), curveObj.targetCurveType_) );
	
		if (curveObj.targetCurveType_ != CURVETYPE_FLOATER)
		{
			LAString curveMktName;
			if (curveObj.targetCurveType_ != STD)
			{
				curveMktName = LAString(".") + curveObj.targetCurveType_;
				curveMktName.toLower();
			}

			LAString targetCurve;
			if (curveObj.isTargetCurveFound_)
			{
				targetCurve = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + curveMktName);
			}
			else
			{
				targetCurve = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + curveMktName);
			}

			LAString targetDF;
			if (targetCurve != MLIB_NO_DATA && targetCurve != "")
			{
				LAStringVector assignedCurves = targetCurve.toToken(':');
				targetDF = assignedCurves[0];
			}
			else
			{
				throw LACoreInvalidData("Missing TargetCurve Discount Factors", __FILE__, __LINE__);
			}

			if (curveObj.isTargetCurveFound_)
			{
				curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
				curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(targetDF));
			}
			else
			{
				curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
				curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new LADataString(targetDF));
			}
		}
	}
	curveObj.dataInstance_.getReferencePool().completeDependency();
}

// Calibrate the Yield Curve
void LAUpdateCurveObject::calibrateOISAndSwapCurve( CurveObject & curveObj ) const
{
	if ( curveObj.isForwardFXCurve_ )
	{
		return;
	}

	if ( curveObj.enableCalculation_ || curveObj.isTargetCurveFound_ )
	{
		if (curveObj.isTargetCurveFound_)
		{
			curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new LADataBool(true));
		}
		
		// Load the Curve Model Generator
		LADataProcedure &curveGenerator = dynamic_cast<LADataProcedure &>( curveObj.calibrationData_->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

		// Main Calibration Routine
		// **********************************************
		curveGenerator.calibrateOISAndSwapCurve( curveObj.asOfDate_,
                                                 curveObj.curveCollection_,
                                                 curveObj.curveIndex_ );
		// **********************************************

		curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	}
}

// Set Basis Curve Discount Factors to Basis Curve Object in Object Pool
void LAUpdateCurveObject::calibrateBasisCurve( CurveObject & curveObj ) const
{
	// 1.	Check If Basis Curve
	// *********************************************************************************
	bool isBasis = false;
	if (!curveObj.tenorBasisCurves_.empty() && !curveObj.isTargetCurveFound_ && curveObj.targetCurveType_ != CURVETYPE_FLOATER)
	{
		isBasis = true;
		curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_BASISDFS, new LADataStrings(curveObj.tenorBasisCurves_));
	}

	if (curveObj.tenorBasisStaticDataName_.size() > 1)
	{
		isBasis = true;
		// set reference for basis
		curveObj.tenorBasisStaticDataName_ = curveObj.tenorBasisStaticDataName_.subString(0, curveObj.tenorBasisStaticDataName_.size() - 2);
		curveObj.calibrationData_->getBasisData().convertFromString(curveObj.tenorBasisStaticDataName_);
	}

	// 2.	Main Basis Curve Calibration Routine
	// *********************************************************************************
	if (isBasis)
	{
		bool isBasisEnabled = (!curveObj.enableCalculation_ && !curveObj.isTargetCurveFound_);
		if (isBasisEnabled)
		{
			curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			curveObj.calibrationData_->LAObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new LADataBool(true));
		}
		
        // Main Basis Curve Calibration Routine
        curveObj.calibrationData_->setBasisRates();

		curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		curveObj.calibrationData_->LAObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
	}

	LAString genFloaterName = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if ( genFloaterName != MLIB_NO_DATA && (curveObj.enableCalculation_ || curveObj.targetCurveType_ == CURVETYPE_FLOATER) )
	{
		setUpFloater(curveObj.currencyLowercase_, *curveObj.calibrationData_, genFloaterName);
	}

	// Set Basis Discount Factors df2
	if (curveObj.targetCurveType_ == STD && curveObj.tenorBasisDiscountFactors_ != MLIB_NO_DATA)
	{
		curveObj.calibrationData_->setDF2();
	}

	if (!curveObj.tenorBasisMarketQuotes_.empty())
	{
		map<LAString, map<LAString, double> >::const_iterator it = curveObj.tenorBasisMarketQuotes_.begin();
		while (it != curveObj.tenorBasisMarketQuotes_.end())
		{
			LACurveMarketDataHelpers::restoreSwapRateFromLibor(*curveObj.calibrationData_, it->second, curveObj.currency_, &(it->first));
			++it;
		}
	}

	// 3.	Update Curve Terms & Discount Factors
	// *********************************************************************************
	
	// Flag Curve as Built
	mCurveGenCcyMap[curveObj.currency_] = true;
	
	// Context Key for Curve Data Interface
	LAString contextKey = curveObj.currency_;
	contextKey.toLower();
	LAString curveContext = LACoreDataService::getContext(contextKey + CONTEXT_KEY_SDE_YIELD);

	if ( curveContext != MLIB_NO_DATA )
	{
		LADataDoubleMatrix matrix;
		matrix.convertFromString(curveContext);
		unsigned int rowSize = matrix.get1DSize();
		unsigned int colSize = matrix.getSize(0);

		bool isBasisDiscountFactor = colSize > 2 ? true : false;
		DoubleArray terms(rowSize);
		DoubleArray dfs(rowSize);
		DoubleArray dfs2(rowSize);
		for (unsigned int i = 0; i < rowSize; i++)
		{
			terms[i] = matrix.get(i, 0);
			dfs[i] = matrix.get(i, 1);

			if (isBasisDiscountFactor)
				dfs2[i] = matrix.get(i, 2);
		}
		dynamic_cast<LADataDoubles &>(curveObj.instance_->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<LADataDoubles &>(curveObj.instance_->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		curveObj.instance_->remove(IR_CALIBRATION_DATA_DFS2);
		if (isBasisDiscountFactor)
		{
			curveObj.instance_->add(IR_CALIBRATION_DATA_DFS2, new LADataDoubles(dfs2));
		}
	}
	else
	{ 
		// Update Curve Data for Curve Index Aliases
		// *********************************************************************************************************
		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
		{
			LAStringVector dataoutCurves;
			dataoutCurves.push_back(STD);

			const std::map<LAString, LAString>& curveAliasList = curveObj.calibrationData_->getAssignedCurveMktMap();
			for (std::map<LAString, LAString>::const_iterator it = curveAliasList.begin(); it != curveAliasList.end(); it++)
			{
				if (it->first != STD && std::find(curveObj.listOfCurvesBuilt_.begin(), curveObj.listOfCurvesBuilt_.end(), it->second) != curveObj.listOfCurvesBuilt_.end())
				{
					dataoutCurves.push_back(it->first);
				}
			}
			// floater curve
			if (genFloaterName != MLIB_NO_DATA) dataoutCurves.push_back(genFloaterName);
			dataoutCurve(dataoutCurves, *curveObj.instance_, curveObj.name_);
		}
	}

	// Update Curve Data Map
	// curveObj.calibrationData_->getYieldData().get().storeCurveDataMap( curveObj.curveCollection_.c_str(), curveObj.curveIndex_.c_str() );
	return;
}
	
// Load curve object pool data and calibrate with dataInstance as output
void LAUpdateCurveObject::loadYieldCurveDataAndCalibrate( const LAString &currency, LADataInstance &dataInstance, const LAString & curveID, const LAString & marketName ) const
{
	// 1.	Exit if Curve Already Flagged as Built or if Using Legacy Curves
	if ( useLegacyCurves( currency, dataInstance ) || mCurveGenCcyMap[currency] ) return;

	// 2.	Initialize Curve Object
	CurveObject curveObj( dataInstance, currency, curveID, marketName );

	// 3.	Update Curve Object and Import Property Manager Data
	updateCurveObjectAndImportPropertyManagerData( curveObj );
	
	// 4.	Main Calibration Routine for OIS and Swap Curves
	calibrateOISAndSwapCurve( curveObj );
	
	// 5.	Main Calibration Routine for Basis Curves
	calibrateBasisCurve( curveObj );
}

// 
/*!
	@brief set up curveType daycount

	@param[out] curveCalibrationData
	@param[out] yc
*/
void
LAUpdateCurveObject::setUpCurveTypeDayCount(CurveCalibrationData &curveCalibrationData, LAMathYieldCurve &yc) const
{
	// set daycount
	const map<LAString, LAString> &assignedCurveMktMap = curveCalibrationData.getAssignedCurveMktMap();
	map<LAString, LAString>::const_iterator it_cur = assignedCurveMktMap.begin();
	while (it_cur != assignedCurveMktMap.end())
	{
		LAPriceDataDayCount dc;
		LAPriceDataCalendar cal;
		LAPriceDataSlidingRule sld;
		LAString accessory;
		curveCalibrationData.getForwardConvention(it_cur->first, dc, sld, cal, accessory);
		yc.getDayCount(it_cur->first) = dc;
		curveCalibrationData.getDayCount(it_cur->first) = dc;
		++it_cur;
	}
}

// 
/*!
	@brief generate sde initial curve data when arbfree curve is generated

	@param[in] currency  currency or fx ex.JPY/USD
	@param[out] dataInstance
*/
void
LAUpdateCurveObject::generateInitialValueArbfree(const LAString &currency, LADataInstance &dataInstance) const
{
	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

	LADataBool tmpAttrB;

	LAString isSetCurveID = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	LAObjectPool &objPool = dataInstance.getObjectPool();

	//LAString yieldName = PREFIX_YIELD + getSDEAttrName(currency);	
	LAString yieldName = LAMarketData::getBaseYieldName(currency);

	LAMathYieldCurve *yc = NULL;
	const LAObjectHolder ehyc = objPool.getObject(yieldName);
	if (!ehyc.isDefined())
	{
		yc = new LAMathYieldCurve(&dataInstance);
		objPool.set(yieldName, yc);

	}
	else
	{
		dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		yc = &dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	yc->getName().convertFromString(yieldName);

	LAString curvePropertiesName = "PRO_" + yieldName;
	CurveCalibrationData *curveCalibrationData = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(curvePropertiesName);
	if (!ehycpro.isDefined())
	{
		curveCalibrationData = new CurveCalibrationData(&dataInstance);
		objPool.set(curvePropertiesName, curveCalibrationData);
	}
	else
	{
		//we must not erase the reset method for only curveCalibrationData
		curveCalibrationData = &dynamic_cast<CurveCalibrationData &>(objPool.getObject(curvePropertiesName).get());
	}
	curveCalibrationData->getName().convertFromString(curvePropertiesName);

	LAObject *objectPool = NULL;
	LAString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const LAObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		objectPool = new LAObject();
		objectPool->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);
		objPool.set(yieldDataName, objectPool);
	}
	else
	{
		//we must not erase the reset method for only edata
		objectPool = &objPool.getObject(yieldDataName).get();
	}
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool useTenorBasis = false;
	bool useTenorBasisInstruments = false;
	bool isSpotStarting = false;

	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *curveCalibrationData, *objectPool, useTenorBasis, useTenorBasisInstruments, isSpotStarting, true);
	// set isArbFree
	curveCalibrationData->getIsArbFree().set(true);
	bool isXccyMarkedToMarket = false;
	LAString isXccyMarkedToMarketString = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST).toUpper();
	if (isXccyMarkedToMarketString != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(isXccyMarkedToMarketString);
		isXccyMarkedToMarket = tmpAttrB.get();
	}

	LAStringVector curveNames_6ML;
	LAStringVector curveNames_3ML;
	LAStringVector curveNames_DF;
	LAString curveName_6ML = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MLCURVENAME).toUpper();
	LAString curveName_DF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DFCURVENAME).toUpper();
	LAString curveName_3ML = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MLCURVENAME).toUpper();
	if (curveName_6ML != MLIB_NO_DATA)
	{
		curveNames_6ML = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MLCURVENAME).toToken(':');
	}
	if (curveName_DF != MLIB_NO_DATA)
	{
		curveNames_DF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DFCURVENAME).toToken(':');
	}
	if (curveName_3ML != MLIB_NO_DATA)
	{
		curveNames_3ML = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MLCURVENAME).toToken(':');
	}

	objectPool->remove(IR_CALIBRATION_DATA_6MLCURVENAMES);
	objectPool->remove(IR_CALIBRATION_DATA_DFCURVENAMES);
	objectPool->remove(IR_CALIBRATION_DATA_3MLCURVENAMES);
	objectPool->add(IR_CALIBRATION_DATA_6MLCURVENAMES, new LADataStrings(curveNames_6ML));
	objectPool->add(IR_CALIBRATION_DATA_DFCURVENAMES, new LADataStrings(curveNames_DF));
	objectPool->add(IR_CALIBRATION_DATA_3MLCURVENAMES, new LADataStrings(curveNames_3ML));
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new LADataBool(isXccyMarkedToMarket));
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new LADataBool(isXccyMarkedToMarket));
	curveCalibrationData->setArbFreeCurveName(curveNames_6ML, curveNames_DF, curveNames_3ML);

	// reference data for market pro
	LAString staticDataName;
	LAString tenorBasisStaticDataName;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// >>>>>>>>>>>>> for XLL plus
		// setup context key
	LAString contextKey = currency;
	contextKey.toLower();

	// setup risk info
	bool isrisk = false;
	isrisk |= LACoreDataService::getContext(ARG_KEY_OFFICIALRISK) != MLIB_NO_DATA ? true : false;
	isrisk |= LACoreDataService::getContext(ARG_KEY_FRONTRISK) != MLIB_NO_DATA ? true : false;

	LAString curveContext = LACoreDataService::getContext(contextKey + CONTEXT_KEY_SDE_YIELD);

	if (!isrisk&&curveContext != MLIB_NO_DATA)
	{
		LADataDoubleMatrix matrix;
		matrix.convertFromString(curveContext);
		unsigned int rowSize = matrix.get1DSize();
		unsigned int colSize = matrix.getSize(0);

		bool isBasisDiscountFactor = colSize > 2 ? true : false;
		DoubleArray terms(rowSize);
		DoubleArray dfs(rowSize);
		DoubleArray dfs2(rowSize);
		for (unsigned int i = 0; i < rowSize; i++)
		{
			terms[i] = matrix.get(i, 0);
			dfs[i] = matrix.get(i, 1);

			if (isBasisDiscountFactor)
				dfs2[i] = matrix.get(i, 2);
		}
		dynamic_cast<LADataDoubles &>(objectPool->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<LADataDoubles &>(objectPool->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		if (isBasisDiscountFactor)
		{
			LADataDoubles* attrDF2 = NULL;
			LADataHolder* df2H = &objectPool->getData(IR_CALIBRATION_DATA_DFS2);
			if (!df2H->isDefined())
			{
				attrDF2 = new LADataDoubles();
				objectPool->add(IR_CALIBRATION_DATA_DFS2, attrDF2);
			}
			else
			{
				attrDF2 = dynamic_cast<LADataDoubles*>(&df2H->get());
			}
			attrDF2->set(dfs2);
		}

		return;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool enableCalculation = true;
	LAString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
	if (target != MLIB_NO_DATA) enableCalculation = false;

	map<LAString, map<LAString, double> > tenorBasisMarketQuotes;
	setUpGenCurveData(dataInstance, staticDataName, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// create Xccy Basis information object
	LAString refData_XccyBasis = "";
	if (currency == CURRENCY_USD)
	{
		LAObject *xccyBasis = NULL;
		LAString nameXccyBasis = yieldDataName + "_" + XCCYBASIS;
		const LAObjectHolder ehdata = objPool.getObject(nameXccyBasis);
		if (!ehdata.isDefined())
		{
			xccyBasis = new LAObject();
			objPool.set(nameXccyBasis, xccyBasis);
		}
		else
		{
			objPool.getObject(nameXccyBasis).get().clear();
			xccyBasis = &objPool.getObject(nameXccyBasis).get();
		}

		refData_XccyBasis += nameXccyBasis + ":";
		// set name
		xccyBasis->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameXccyBasis);
		// set data type
		xccyBasis->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BASIS);
		// set curve name
		LAStringVector usd3mlFloaterInfo = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USD3MLFLOATER).toToken(':');
		if (usd3mlFloaterInfo.size() != 2) throw LACoreInvalidData("No XccyBasis information!", __FILE__, __LINE__);
		xccyBasis->add(IR_CALIBRATION_DATA_AGTFORECAST, new LADataString()).convertFromString(usd3mlFloaterInfo[1]);
		// set foreign ccy info
		LAString ccy_floater, fYieldDataName;
		if (isSetCurveID == "TRUE")
		{
			LAObject* fYieldData = &objPool.getObject(usd3mlFloaterInfo[0], ENCHKTYPE_ISDEFINED).get();
			ccy_floater = dynamic_cast<LADataString&> ((fYieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
			fYieldDataName = usd3mlFloaterInfo[0];
		}
		else
		{
			LAString isPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
			if (isPricer == "TRUE")
			{
				LAObject* yieldData = &objPool.getObject(usd3mlFloaterInfo[0], ENCHKTYPE_ISDEFINED).get();
				ccy_floater = dynamic_cast<LADataString&> ((yieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
			}
			else
			{
				ccy_floater = usd3mlFloaterInfo[0];
			}
			fYieldDataName = PREFIX_YIELD + getSDEAttrName(ccy_floater) + "_DATA";
		}

		if (mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_floater) && enableCalculation)
		{
			loadYieldCurveDataAndCalibrate(ccy_floater, dataInstance);
		}
		LAString curveName_floater = usd3mlFloaterInfo[1];
		curveCalibrationData->getForeignYieldData().convertFromString(fYieldDataName);

		if (isSetCurveID != "TRUE")
		{
			LAString baseCcy = currency; LAString domCcy = currency; LAString forCcy = ccy_floater;
			while (1)
			{
				CurveCalibrationData &ycPro_dccy = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(LAMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				CurveCalibrationData &ycPro_fccy = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(LAMarketData::getBaseYieldProName(forCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());

				ycPro_fccy.setAffectingCcy(baseCcy.toUpper());
				const LAString& affectedCcy_fccy = ycPro_fccy.getAffectedCcy();
				if (affectedCcy_fccy == domCcy)
					throw LACoreInvalidData("The affected currency includes in Affecting currencies!", __FILE__, __LINE__);

				ycPro_dccy.setAffectedCcy(forCcy.toUpper());

				domCcy = forCcy;
				forCcy = ycPro_fccy.getAffectedCcy();


				if (!forCcy.isDefined()) break;
			}
		}

		if (isXccyMarkedToMarket)
		{
			LAStringVector baseCcyDFInfo = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASECCYDF).toToken(':');
			if (baseCcyDFInfo.size() != 2) throw LACoreInvalidData("No XccyBasis information!", __FILE__, __LINE__);
			if (usd3mlFloaterInfo[0] != baseCcyDFInfo[0]) throw LACoreInvalidData("Reference currencies are inconsistent!", __FILE__, __LINE__);
			xccyBasis->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new LADataString()).convertFromString(baseCcyDFInfo[1]);
		}

		// set yield curve pro
		refData_XccyBasis = refData_XccyBasis.subString(0, refData_XccyBasis.size() - 2);
		curveCalibrationData->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + XCCYBASIS);
		curveCalibrationData->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + XCCYBASIS, new LADataMultiReference()).convertFromString(refData_XccyBasis);
	}

	tmpCurrency.toLower();
	LAStringVector listOfCurvesBuilt;
	LAString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == MLIB_NO_DATA)
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(listOfCurvesBuilt);
	if (currency != CURRENCY_USD && find(listOfCurvesBuilt.begin(), listOfCurvesBuilt.end(), XCCYBASIS) == listOfCurvesBuilt.end()) throw LACoreInvalidData("XCCYBASIS is not in generate dfs!!", __FILE__, __LINE__);
	if (find(listOfCurvesBuilt.begin(), listOfCurvesBuilt.end(), THREESIXBASIS) == listOfCurvesBuilt.end())
	{
		LAString thisCurveStaticData;
		setUp36BasisDummyData(dataInstance, thisCurveStaticData, asOfDate, tmpCurrency, THREESIXBASIS, yieldDataName, isSpotStarting, *curveCalibrationData);
		// set yield curve pro
		thisCurveStaticData = thisCurveStaticData.subString(0, thisCurveStaticData.size() - 2);
		curveCalibrationData->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + THREESIXBASIS);
		curveCalibrationData->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + THREESIXBASIS, new LADataMultiReference()).convertFromString(thisCurveStaticData);
	}
	// set yield curve pro
	staticDataName = staticDataName.subString(0, staticDataName.size() - 2);
	curveCalibrationData->getMarketData().convertFromString(staticDataName);

	LADataHolder* dh;
	LAStringVector swapCurveDiscountFactors;
	dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) swapCurveDiscountFactors = dynamic_cast<const LADataStrings &>(dh->get()).get();

	LAStringVector basisCurves;
	dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) basisCurves = dynamic_cast<const LADataStrings &>(dh->get()).get();

	LAString tenorBasisDiscountFactors = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (!listOfCurvesBuilt.empty() && listOfCurvesBuilt[0] != MLIB_NO_DATA)
	{
		//LADataStrings &attrGenCurves = dynamic_cast<LADataStrings &>(curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings()).get());		
		for (unsigned int i = 0; i < listOfCurvesBuilt.size(); ++i)
		{
			if (listOfCurvesBuilt[i] == SWAP) continue;

			tmpCurrency.toLower();
			LAString thisCurveStaticData;
			LAString suffixLowerCase = "." + listOfCurvesBuilt[i];
			suffixLowerCase.toLower();
			LAString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffixLowerCase).toUpper();
			if (marketType == MLIB_NO_DATA)
			{
				LAString isBasisStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISBASIS + suffixLowerCase).toUpper();
				LAString isReadFile = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffixLowerCase).toUpper();
				if (isBasisStr == "TRUE") marketType = MARKETTYPE_BASIS;
				else if (isReadFile == "TRUE") marketType = MARKETTYPE_ZERORATE;
				else marketType = MARKETTYPE_SWAP;
			}

			// set up interpolation
			LAString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase).toUpper();
			if (strInter != MLIB_NO_DATA)
			{
				yc->getInterpolation(listOfCurvesBuilt[i]).convertFromString(strInter.toLower());
				curveCalibrationData->getInterpolation(listOfCurvesBuilt[i]).convertFromString(strInter);
				if (tenorBasisDiscountFactors == listOfCurvesBuilt[i])
				{
					yc->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
					curveCalibrationData->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
				}
			}

			if (marketType == MARKETTYPE_BASIS)
			{
				if (!enableCalculation && listOfCurvesBuilt[i] != XCCYBASIS && listOfCurvesBuilt[i] != THREESIXBASIS) continue;

				setUpBasisCurveData(dataInstance, thisCurveStaticData, asOfDate, tmpCurrency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, *curveCalibrationData);

				if (listOfCurvesBuilt[i] != XCCYBASIS && listOfCurvesBuilt[i] != THREESIXBASIS)
				{
					LAString thisMarketName = listOfCurvesBuilt[i];
					LAString aliasCurveNames = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + thisMarketName.toLower());
					if (aliasCurveNames != MLIB_NO_DATA)
					{
						LAStringVector assignedCurves = aliasCurveNames.toToken(':');
						for (size_t i = 0; i < assignedCurves.size(); i++)
						{
							basisCurves.push_back(assignedCurves[i]);
						}
					}
					else
					{
						basisCurves.push_back(listOfCurvesBuilt[i]);
					}
				}
			}
			else if (marketType == MARKETTYPE_ZERORATE)
			{
				if (!enableCalculation && target != listOfCurvesBuilt[i]) continue;

				setUpCurveDataByReadFile(dataInstance, asOfDate, tmpCurrency, listOfCurvesBuilt[i], yieldDataName, *curveCalibrationData);
				continue;
			}
			else if (marketType == MARKETTYPE_SWAP)
			{
				if (!enableCalculation) continue;

				LAString thisMarketName = listOfCurvesBuilt[i];
				LAString aliasCurveNames = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != MLIB_NO_DATA)
				{
					LAStringVector assignedCurves = aliasCurveNames.toToken(':');
					for (size_t i = 0; i < assignedCurves.size(); i++)
					{
						if (swapCurveDiscountFactors.end() == std::find(swapCurveDiscountFactors.begin(), swapCurveDiscountFactors.end(), assignedCurves[i]))
						{
							swapCurveDiscountFactors.push_back(assignedCurves[i]);
						}
					}
				}
				else
				{
					if (swapCurveDiscountFactors.end() == std::find(swapCurveDiscountFactors.begin(), swapCurveDiscountFactors.end(), listOfCurvesBuilt[i]))
					{
						swapCurveDiscountFactors.push_back(listOfCurvesBuilt[i]);
					}
				}

				//attrGenCurves.set(swapCurveDiscountFactors);

				LAString oisCompoundType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffixLowerCase).toUpper();
				if (oisCompoundType.toUpper() == "DAILYCOMPOUNDING")
					setUpGenCurveDataOIS(dataInstance, thisCurveStaticData, asOfDate, tmpCurrency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes);
				else
					setUpGenCurveData(dataInstance, thisCurveStaticData, asOfDate, tmpCurrency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes);
			}
			else
			{
				throw LACoreInvalidData("Market type is not supported!!", __FILE__, __LINE__);
			}

			// set yield curve pro
			thisCurveStaticData = thisCurveStaticData.subString(0, thisCurveStaticData.size() - 2);
			curveCalibrationData->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + listOfCurvesBuilt[i]);
			curveCalibrationData->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + listOfCurvesBuilt[i], new LADataMultiReference()).convertFromString(thisCurveStaticData);
		}
	}

	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(swapCurveDiscountFactors));

	// generate yield data
	curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
		(curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	dataInstance.getReferencePool().completeDependency();
	modelDataObj.calibrateModel(asOfDate);

	// generate swap curve
	if (swapCurveDiscountFactors.size() != 0 && enableCalculation)
	{
		curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
		for (size_t i = 0; i < swapCurveDiscountFactors.size(); i++)
		{
			curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(swapCurveDiscountFactors[i]));
			modelDataObj = dynamic_cast<LADataProcedure &> (curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
			modelDataObj.calibrateModel(asOfDate);
			curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
		}
		curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	}

	//adjust discountfactor
	tmpCurrency.toLower();
	LADataBool tmpAttrDF;
	tmpAttrDF.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF));
	if (tmpAttrDF.get())
	{
		const std::map<LAString, LAString> &assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (!curveCalibrationData->isBasisCurve(it->first) && (enableCalculation || target == it->second))
			{
				LAMarketData::adjustDiscountFactor(*objectPool, it->first);
			}
		}
	}

	// basis
	bool isBasis = false;
	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_BASISDFS);
	if (!basisCurves.empty() && enableCalculation)
	{
		isBasis = true;
		curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_BASISDFS, new LADataStrings(basisCurves));
	}

	curveCalibrationData->LAObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
	if (tenorBasisDiscountFactors != MLIB_NO_DATA)
	{
		isBasis = true;
		curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new LADataString(tenorBasisDiscountFactors));
	}
	if (tenorBasisStaticDataName.size() > 1)
	{
		isBasis = true;
		// set reference for basis
		tenorBasisStaticDataName = tenorBasisStaticDataName.subString(0, tenorBasisStaticDataName.size() - 2);
		curveCalibrationData->getBasisData().convertFromString(tenorBasisStaticDataName);
	}

	if (isBasis)
	{
		curveCalibrationData->setBasisRates();
	}

	// set daycount
	setUpCurveTypeDayCount(*curveCalibrationData, *yc);

	LAString genFloaterName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != MLIB_NO_DATA || target == CURVETYPE_FLOATER) setUpFloater(tmpCurrency, *curveCalibrationData, genFloaterName);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// >>>>>>>>>>>>> for XLL plus
	if (curveContext != MLIB_NO_DATA)
	{
		LADataDoubleMatrix matrix;
		matrix.convertFromString(curveContext);
		unsigned int rowSize = matrix.get1DSize();
		unsigned int colSize = matrix.getSize(0);

		bool isBasisDiscountFactor = colSize > 2 ? true : false;
		DoubleArray terms(rowSize);
		DoubleArray dfs(rowSize);
		DoubleArray dfs2(rowSize);
		for (unsigned int i = 0; i < rowSize; i++)
		{
			terms[i] = matrix.get(i, 0);
			dfs[i] = matrix.get(i, 1);

			if (isBasisDiscountFactor)
				dfs2[i] = matrix.get(i, 2);
		}
		dynamic_cast<LADataDoubles &>(objectPool->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<LADataDoubles &>(objectPool->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		objectPool->remove(IR_CALIBRATION_DATA_DFS2);
		if (isBasisDiscountFactor)
		{
			objectPool->add(IR_CALIBRATION_DATA_DFS2, new LADataDoubles(dfs2));
		}

		return;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
	{
		LAStringVector dataoutCurves;
		dataoutCurves.push_back(STD);
		for (size_t i = 0; i < curveNames_6ML.size(); i++)
		{
			if (curveNames_6ML[i] != STD) dataoutCurves.push_back(curveNames_6ML[i]);
		}
		for (size_t i = 0; i < curveNames_DF.size(); i++)
		{
			if (curveNames_DF[i] != STD) dataoutCurves.push_back(curveNames_DF[i]);
		}
		for (size_t i = 0; i < curveNames_3ML.size(); i++)
		{
			if (curveNames_3ML[i] != STD) dataoutCurves.push_back(curveNames_3ML[i]);
		}

		const std::map<LAString, LAString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second != SWAP && it->second != XCCYBASIS && it->second != THREESIXBASIS) dataoutCurves.push_back(it->first);
		}

		if (genFloaterName != MLIB_NO_DATA) dataoutCurves.push_back(genFloaterName);

		dataoutCurve(dataoutCurves, *objectPool, yieldDataName);
	}
}

// 
/*!
	@brief generate sde correlation data

	@param[in] currency  currency or fx ex.JPY/USD
	@param[in] dataInstance
*/
void
LAUpdateCurveObject::loadCorrelationDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const
{
	LAString sdeName = getSDEAttrName(currency);
	LAString type = getCorTye(currency);
	type.toUpper();

	LAMathCorrelation *corEntity = new LAMathCorrelation(&dataInstance);
	LADataString &corAttrName = corEntity->getName();
	LAString corName = PREFIX_COR + sdeName;
	corAttrName.set(corName);

	if (!isCancelForFunding(currency))
	{
		if (type == INPUT_FACTOR)
		{
			setUpCorFactor(currency, *corEntity, dataInstance);
		}
		else if (type == INPUT_DATA)
		{
			setUpCorData(currency, *corEntity, dataInstance);
		}
		else if (type == INPUT_FUNC)
		{
			setUpCorFunc(currency, *corEntity, dataInstance);
		}
		else
		{
			throw LACoreInvalidData("Volatility input type. only function or data is support", __FILE__, __LINE__);
		}
	}
	// set data as reference
	dataInstance.getDataMaster().setData(corName, DATA_REFERENCE);
	// set object pool
	dataInstance.getObjectPool().remove(corName);
	dataInstance.getObjectPool().set(corName, corEntity);
}

// 
/*!
	@brief get SDE data name

	@param[in] currency
	@return LAString
*/
LAString
LAUpdateCurveObject::getSDEAttrName(const LAString &currency) const
{
	LAString key_ccy = currency;
	return  mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
}

// 
/*!
	@brief get Grid property val

	@param[in] key
	@param[in] curve
	@param[in] grid
	@return LAString
*/
LAString
LAUpdateCurveObject::getGridStaticData(const LAString &key, const LAString &curve, const LAString &grid) const
{
	return etrading::getGridStaticData(mpStaticData, key, curve, grid);
}

// 
/*!
	@brief fuction to set up generate configuration
*/
void
LAUpdateCurveObject::setUpGenerateConfig
(LADataInstance &dataInstance, const LADate &asOfDate, const LAString &currency, LAMathYieldCurve &yc, CurveCalibrationData &curveCalibrationData,
	LAObject &ycData, bool &useTenorBasis, bool &useTenorBasisInstruments, bool &isSpotStarting, bool isArbFree, const LAString& suffixLowerCase, const LAString& epSuffix) const
{
	LAString isSetCurveID = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

	LAString yieldName = yc.getName().get();

	yc.LAObject::remove(CALIBRATION_DATA_ASOFDATE);
	yc.LAObject::add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));

	// set interpolation
	LAString interp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase);
	if (interp == MLIB_NO_DATA)
	{
		interp = FN_SPLINEINTERPOLATION_STR;
	}
	yc.getInterpolation().convertFromString(interp.toLower());

	// set daycount
	LAString dayCount = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT);
	if (dayCount != MLIB_NO_DATA)
	{
		yc.getDayCount().convertFromString(dayCount.toUpper());
	}

	// set freq
	LAString freq = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY);
	if (freq != MLIB_NO_DATA)
	{
		yc.getFrequency().convertFromString(freq.toUpper());
	}

	// set sliding rule
	LAString slidingRule = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE);
	if (slidingRule != MLIB_NO_DATA)
	{
		yc.getSlidingRule().convertFromString(slidingRule.toUpper());
	}

	// set cal 
	LAPriceDataCalendar calY;
	LAString calYStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_CALENDAR);
	if (calYStr != MLIB_NO_DATA)
	{
		calY.convertFromString(calYStr);
		yc.getCalendar() = calY;
	}

	//set basis DF
	LAString tenorBasisDiscountFactors = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (tenorBasisDiscountFactors != MLIB_NO_DATA)
	{
		yc.setBasisCurveType(tenorBasisDiscountFactors);
	}

	// set yield to object pool
	LAObjectPool &objPool = dataInstance.getObjectPool();

	// set yieldpro to object pool
	// set as of date
	curveCalibrationData.getAsOfDate().set(asOfDate);

	// set curve generator
	curveCalibrationData.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);

	// set interpolation
	LAString genInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase);
	if (genInterp == MLIB_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}
	if (suffixLowerCase.size() == 0)
	{
		curveCalibrationData.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		curveCalibrationData.LAObject::remove(CALIBRATION_DATA_INTERPOLATION + epSuffix);
		curveCalibrationData.LAObject::add(CALIBRATION_DATA_INTERPOLATION + epSuffix, new LAPriceDataInterpolation()).convertFromString(genInterp);
	}
	curveCalibrationData.getInterpolation().convertFromString(genInterp.toLower());

	// interpolation for OIS and STD swaps
	LAString genYieldGenInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffixLowerCase);
	if (genYieldGenInterp == MLIB_NO_DATA)
	{
		if (genInterp != MLIB_NO_DATA)
		{
			genYieldGenInterp = genInterp;
		}
		else
		{
			genYieldGenInterp = FN_SPLINEINTERPOLATION_STR;
		}
	}
	if (suffixLowerCase.size() == 0)
	{
		curveCalibrationData.getInterpolation_yg().convertFromString(genYieldGenInterp.toLower());
	}
	else
	{
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix, new LAPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// interpolation for FRA/futures
	LAString genFutureInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FUTURE_INTERPOLATION + suffixLowerCase);
	if (genFutureInterp == MLIB_NO_DATA)
	{
		if (genInterp != MLIB_NO_DATA)
		{
			genFutureInterp = genInterp;
		}
		else
		{
			genFutureInterp = FN_SPLINEINTERPOLATION_STR;
		}
	}
	if (suffixLowerCase.size() == 0)
	{
		curveCalibrationData.getInterpolation_fw().convertFromString(genFutureInterp.toLower());
	}
	else
	{
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONFW + epSuffix);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONFW + epSuffix, new LAPriceDataInterpolation()).convertFromString(genFutureInterp);
	}

	// interpolation for basis
	LAString genBasisInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASIS_INTERPOLATION + suffixLowerCase);
	if (genBasisInterp == MLIB_NO_DATA)
	{
		if (genInterp != MLIB_NO_DATA)
		{
			genBasisInterp = genInterp;
		}
		else
		{
			genBasisInterp = FN_SPLINEINTERPOLATION_STR;
		}
	}
	if (suffixLowerCase.size() == 0)
	{
		curveCalibrationData.getInterpolation_bs().convertFromString(genBasisInterp.toLower());
	}
	else
	{
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONBS + epSuffix);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONBS + epSuffix, new LAPriceDataInterpolation()).convertFromString(genBasisInterp);
	}

	// set basis function
	LAString genBasisFunc = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASISFUNCTION);
	if (genBasisFunc == MLIB_NO_DATA)
	{
		genBasisFunc = FN_BASISFUNC2_STR;
	}
	curveCalibrationData.getBasisFunction().setFunction(genBasisFunc.toLower());

	// set daycount
	LAString genDayCount = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT);
	if (genDayCount != MLIB_NO_DATA)
	{
		curveCalibrationData.getDayCount().convertFromString(genDayCount.toUpper());
	}

	// set freq
	LAString genFreq = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY);
	if (genFreq != MLIB_NO_DATA)
	{
		curveCalibrationData.getFrequency().convertFromString(genFreq.toUpper());
	}

	// set sliding rulue
	LAString genSlidingRule = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE);
	if (genSlidingRule != MLIB_NO_DATA)
	{
		curveCalibrationData.getSlidingRule().convertFromString(genSlidingRule.toUpper());
	}

	// set rate priority
	LAString ratePrio_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_RATEPRIORITY).toUpper();
	if (ratePrio_str != MLIB_NO_DATA)
	{
		curveCalibrationData.getData(PRICING_DATA_RATEPRIORITY).convertFromString(ratePrio_str);
	}

	// set tenor adjust
	LADataBool tmpAttrB;
	LAString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + suffixLowerCase).toUpper();
	if (strSwapTenorAdj == MLIB_NO_DATA)
	{
		if (tmpCurrency == "aud")
		{
			strSwapTenorAdj = "TRUE";
		}
		else
		{
			strSwapTenorAdj = "FALSE";
		}
	}
	tmpAttrB.convertFromString(strSwapTenorAdj);
	useTenorBasisInstruments = tmpAttrB.get();

	//20170531 - Fixed Fatal Error For Win-Server2012
#if (WINVER >= 0x0601)
	LADataHolder *ahTemp;
	ahTemp = &curveCalibrationData.LAObject::getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, NOCHECK);
	if (!ahTemp->isDefined())
	{
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, new LADataBool(useTenorBasisInstruments));
	}
	else
	{
		dynamic_cast<LADataBool&>(ahTemp->get()).set(useTenorBasisInstruments);
	}
#else	
	curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix);
	curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, new LADataBool(useTenorBasisInstruments));
#endif

	if (useTenorBasisInstruments)
	{
		LAString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
		if (tenorSwapName == MLIB_NO_DATA)
		{
			throw LACoreInvalidData("Set tenor swap name!", __FILE__, __LINE__);
		}

		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_TENORSWAPNAME);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_TENORSWAPNAME, new LADataString(tenorSwapName));
	}

	/*LAString yieldDataName = yieldName + "_DATA";
	ycData.remove(CALIBRATION_DATA_NAME);
	ycData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);*/
	LAString yieldDataName = dynamic_cast<const LADataString&> ((ycData.getData(CALIBRATION_DATA_NAME, ISNOTNULL)).get()).get();

	// set asofdate
	ycData.remove(CALIBRATION_DATA_ASOFDATE);
	ycData.add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));

	// set currency
	ycData.remove(IR_CALIBRATION_DATA_CURRENCY);
	ycData.add(IR_CALIBRATION_DATA_CURRENCY, new LADataString(currency));

	// set interpolation
	ycData.remove(CALIBRATION_DATA_INTERPOLATION);
	ycData.add(CALIBRATION_DATA_INTERPOLATION, new LAPriceDataInterpolation()).convertFromString(genInterp.toLower());

	// set yield data
	yc.getYieldData().convertFromString(yieldDataName);
	curveCalibrationData.getYieldData().convertFromString(yieldDataName);

	// set base curve reference
	curveCalibrationData.getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(yieldName);

	// set currency in curve ID for excel pricer 
	const LAString isPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE" && isSetCurveID != "TRUE")
	{
		LAString curveIndex = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
		const LAObjectHolder ehdata = objPool.getObject(curveIndex);
		LAObject *eCurveID = NULL;
		if (!ehdata.isDefined())
		{
			eCurveID = new LAObject();
			objPool.set(curveIndex, eCurveID);
		}
		else
		{
			objPool.getObject(curveIndex).get().reset();
			eCurveID = &objPool.getObject(curveIndex).get();
		}
		eCurveID->add(CALIBRATION_DATA_NAME, new LADataString(curveIndex));
		eCurveID->add(IR_CALIBRATION_DATA_CURRENCY, new LADataString(currency));
	}

	LAString strIsAudExtra = mpStaticData->getStaticData(KEY_SDE_YIELD_ISAUDEXTRA);
	if (strIsAudExtra != MLIB_NO_DATA)
	{
		LADataBool tmpIsAudExtra;
		tmpIsAudExtra.convertFromString(strIsAudExtra);
		useTenorBasis = tmpIsAudExtra.get();
	}

	if (useTenorBasis && useTenorBasisInstruments)
	{
		throw LACoreInvalidData("We can not set AUD extra and swap tenor adjust at a same time!", __FILE__, __LINE__);
	}

	// spotDate use flag
	LAString isSpotUseStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSPOTUSE);
	if (isSpotUseStr == MLIB_NO_DATA)
	{
		isSpotUseStr = "FALSE";
	}
	tmpAttrB.convertFromString(isSpotUseStr);
	isSpotStarting = tmpAttrB.get();

	// set curve exist check
	if (isPricer == "FALSE" || isPricer == MLIB_NO_DATA)
	{
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, new LADataBool(true));
	}

	// set max term of curve
	LAString maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM);
	if (maxTerm == MLIB_NO_DATA)
	{
		maxTerm = mpStaticData->getStaticData(KEY_SIMULATION_TERM_MAX);
	}

	if (maxTerm != MLIB_NO_DATA)
	{
		ycData.remove(IR_CALIBRATION_DATA_MAXTERM);
		ycData.add(IR_CALIBRATION_DATA_MAXTERM, new LADataString(maxTerm));
		LAString maxFreq = LACoreDataService::getContext(CONTEXT_KEY_MAXTERMFREQ);

		if (maxFreq == MLIB_NO_DATA)
		{
			maxFreq = mpStaticData->getStaticData(KEY_SIMULATION_TERM_MAX_FREQ);
		}

		if (maxFreq == MLIB_NO_DATA)
		{
			throw LACoreInvalidData("Max term frequency is needed for curve extrapolation!", __FILE__, __LINE__);
		}

		ycData.remove(IR_CALIBRATION_DATA_MAXTERMFREQ);
		ycData.add(IR_CALIBRATION_DATA_MAXTERMFREQ, new LADataString(maxFreq.toUpper()));
	}
}

// 
/*!
	@brief fuction to set up basis curve data
*/
void
LAUpdateCurveObject::setUpBasisCurveData(LADataInstance &dataInstance, LAString &staticDataName, const LADate &asOfDate, const LAString &curveCurrencyLowerCase,
	const LAString &marketName, const LAString &yieldDataName, bool isSpotStarting, CurveCalibrationData &curveCalibrationData, const LAString& propertyManagerCurveName, const LAString* pMktCurrency) const
{
	LAString mktCurrency = pMktCurrency ? *pMktCurrency : curveCurrencyLowerCase;
	LAString curveMktName = pMktCurrency ? mktCurrency + "_" + marketName : marketName;
	mktCurrency.toLower();
	curveMktName.toUpper();

	LAString isPricerStr = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	const bool isPricer = isPricerStr == "TRUE";
	LAString isSetCurveIDStr = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);
	const bool isSetCurveID = isSetCurveIDStr == "TRUE";
	bool enableCalculation = true;
	LAString yieldGeneratorTarget = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
	if (yieldGeneratorTarget != MLIB_NO_DATA) enableCalculation = false;

	LAObjectPool &objPool = dataInstance.getObjectPool();

	LAString mktSuffix = "." + marketName;
	LAString curveSuffix = pMktCurrency ? "." + mktCurrency + "_" + marketName : "." + marketName;
	mktSuffix.toLower();
	curveSuffix.toLower();

	LAString isLeg1SpreadStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISLEG1SPREAD + mktSuffix).toUpper();
	// get market rate interpolation
	LAString interpStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASIS_INTERPOLATION + mktSuffix).toUpper();
	LAString isTimeInterStr = "TRUE";

	bool isDiscount = true;
	bool isLeg1 = true;
	bool isAgtSpread = true;

	LAString yieldBasisTarget = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_TARGET + curveSuffix).toUpper();
	if (yieldBasisTarget == LEG1FORECAST)
	{
		isDiscount = false;
		isLeg1 = true;
		if (isLeg1SpreadStr == "TRUE")
		{
			isAgtSpread = false;
		}
	}
	else if (yieldBasisTarget == LEG1DISCOUNT)
	{
		isDiscount = true;
		isLeg1 = true;
		if (isLeg1SpreadStr == "TRUE")
		{
			isAgtSpread = false;
		}
	}
	else if (yieldBasisTarget == LEG2FORECAST)
	{
		isDiscount = false;
		isLeg1 = false;
		if (isLeg1SpreadStr != "TRUE")
		{
			isAgtSpread = false;
		}
	}
	else if (yieldBasisTarget == LEG2DISCOUNT)
	{
		isDiscount = true;
		isLeg1 = false;
		if (isLeg1SpreadStr != "TRUE")
		{
			isAgtSpread = false;
		}
	}

	LAString fCurve;
	LAString dCurve;
	LAString ccy_fCurve;
	LAString ccy_dCurve;
	LAString a_fCurve;
	LAString a_dCurve;
	LAString ccy_a_fCurve;
	LAString ccy_a_dCurve;
	// cashflow info
	LAString c_freqStr;
	LAString c_freqcpdStr;
	LAString c_daycStr;
	LAString c_slidingStr;
	LAString c_calStr;
	LADate c_spotDate;
	int c_resetLag;
	// index info
	LAString i_freqStr;
	LAString i_accessaryStr;
	LAString i_daycStr;
	LAString i_slidingStr;
	LAString i_fixcalStr;
	LAString i_paycalStr;
	int i_resetLag;
	// against cashflow info
	LAString a_c_freqStr;
	LAString a_c_freqcpdStr;
	LAString a_c_daycStr;
	LAString a_c_slidingStr;
	LAString a_c_calStr;
	LADate a_c_spotDate;
	int a_c_resetLag;
	// against index info
	LAString a_i_freqStr;
	LAString a_i_accessaryStr;
	LAString a_i_daycStr;
	LAString a_i_slidingStr;
	LAString a_i_fixcalStr;
	LAString a_i_paycalStr;
	int a_i_resetLag;

	LAPriceDataCalendar tmpCal;
	if (isLeg1)
	{
		fCurve = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + curveSuffix);
		dCurve = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + curveSuffix);
		a_fCurve = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + curveSuffix);
		a_dCurve = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + curveSuffix);

		// cashflow info
		c_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCY + mktSuffix).toUpper();
		c_freqcpdStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCYCOMPOUND + mktSuffix).toUpper();
		c_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_DAYCOUNT + mktSuffix).toUpper();
		c_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SLIDINGRULE + mktSuffix).toUpper();
		c_calStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_CALENDAR + mktSuffix);
		c_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SPOTLAG + mktSuffix).getIntValue();
		if (isSpotStarting)
		{
			c_spotDate.setDate(mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SPOTDATE + mktSuffix).getCString());
		}
		else
		{
			tmpCal.convertFromString(c_calStr);
			c_spotDate = tmpCal.getBusinessDay(asOfDate, c_resetLag);
		}
		// index info
		i_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_FREQUENCY + mktSuffix).toUpper();
		i_accessaryStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_ACCESSARY + mktSuffix).toUpper();
		i_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_DAYCOUNT + mktSuffix).toUpper();
		i_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_SLIDINGRULE + mktSuffix).toUpper();
		i_fixcalStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_FIXINGCALENDAR + mktSuffix).toUpper();
		i_paycalStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_PAYMENTCALENDAR + mktSuffix).toUpper();
		i_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_RESETLAG + mktSuffix).getIntValue();
		// against cashflow info
		a_c_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCY + mktSuffix).toUpper();
		a_c_freqcpdStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCYCOMPOUND + mktSuffix).toUpper();
		a_c_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_DAYCOUNT + mktSuffix).toUpper();
		a_c_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SLIDINGRULE + mktSuffix).toUpper();
		a_c_calStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_CALENDAR + mktSuffix);
		a_c_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SPOTLAG + mktSuffix).getIntValue();
		if (isSpotStarting)
		{
			a_c_spotDate.setDate(mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SPOTDATE + mktSuffix).getCString());
		}
		else
		{
			tmpCal.convertFromString(a_c_calStr);
			a_c_spotDate = tmpCal.getBusinessDay(asOfDate, a_c_resetLag);
		}
		// against index info
		a_i_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_FREQUENCY + mktSuffix).toUpper();
		a_i_accessaryStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_ACCESSARY + mktSuffix).toUpper();
		a_i_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_DAYCOUNT + mktSuffix).toUpper();
		a_i_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_SLIDINGRULE + mktSuffix).toUpper();
		a_i_fixcalStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_FIXINGCALENDAR + mktSuffix).toUpper();
		a_i_paycalStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_PAYMENTCALENDAR + mktSuffix).toUpper();
		a_i_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_RESETLAG + mktSuffix).getIntValue();
	}
	else
	{
		fCurve = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + curveSuffix);
		dCurve = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + curveSuffix);
		a_fCurve = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + curveSuffix);
		a_dCurve = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + curveSuffix);
		// cashflow info
		c_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCY + mktSuffix).toUpper();
		c_freqcpdStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCYCOMPOUND + mktSuffix).toUpper();
		c_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_DAYCOUNT + mktSuffix).toUpper();
		c_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SLIDINGRULE + mktSuffix).toUpper();
		c_calStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_CALENDAR + mktSuffix);
		c_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SPOTLAG + mktSuffix).getIntValue();
		if (isSpotStarting)
		{
			c_spotDate.setDate(mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SPOTDATE + mktSuffix).getCString());
		}
		else
		{
			tmpCal.convertFromString(c_calStr);
			c_spotDate = tmpCal.getBusinessDay(asOfDate, c_resetLag);
		}
		// index info
		i_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_FREQUENCY + mktSuffix).toUpper();
		i_accessaryStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_ACCESSARY + mktSuffix).toUpper();
		i_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_DAYCOUNT + mktSuffix).toUpper();
		i_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_SLIDINGRULE + mktSuffix).toUpper();
		i_fixcalStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_FIXINGCALENDAR + mktSuffix).toUpper();
		i_paycalStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_PAYMENTCALENDAR + mktSuffix).toUpper();
		i_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_RESETLAG + mktSuffix).getIntValue();
		// against cashflow info
		a_c_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCY + mktSuffix).toUpper();
		a_c_freqcpdStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCYCOMPOUND + mktSuffix).toUpper();
		a_c_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_DAYCOUNT + mktSuffix).toUpper();
		a_c_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SLIDINGRULE + mktSuffix).toUpper();
		a_c_calStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_CALENDAR + mktSuffix);
		a_c_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SPOTLAG + mktSuffix).getIntValue();
		if (isSpotStarting)
		{
			a_c_spotDate.setDate(mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SPOTDATE + mktSuffix).getCString());
		}
		else
		{
			tmpCal.convertFromString(a_c_calStr);
			a_c_spotDate = tmpCal.getBusinessDay(asOfDate, a_c_resetLag);
		}
		// against index info
		a_i_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_FREQUENCY + mktSuffix).toUpper();
		a_i_accessaryStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_ACCESSARY + mktSuffix).toUpper();
		a_i_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_DAYCOUNT + mktSuffix).toUpper();
		a_i_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_SLIDINGRULE + mktSuffix).toUpper();
		a_i_fixcalStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_FIXINGCALENDAR + mktSuffix).toUpper();
		a_i_paycalStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_PAYMENTCALENDAR + mktSuffix).toUpper();
		a_i_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_RESETLAG + mktSuffix).getIntValue();
	}

	if (isDiscount)
	{
		// When target curve is a discount curve, the base curve is a discount curve but there is
		// also a secondary base curve which is the forecast curve on the same side
		curveCalibrationData.remove(CALIBRATION_DATA_BASISCURVEBASE + LAString("_") + curveMktName);
		curveCalibrationData.LAObject::add(CALIBRATION_DATA_BASISCURVEBASE + LAString("_") + curveMktName, new LADataString(a_dCurve));

		curveCalibrationData.remove(CALIBRATION_DATA_BASISCURVESECONDARYBASE + LAString("_") + curveMktName);
		curveCalibrationData.LAObject::add(CALIBRATION_DATA_BASISCURVESECONDARYBASE + LAString("_") + curveMktName, new LADataString(a_fCurve));

		// Store the forecast curve on the same side as the target discount
		curveCalibrationData.remove(CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE + LAString("_") + curveMktName);
		curveCalibrationData.LAObject::add(CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE + LAString("_") + curveMktName, new LADataString(fCurve));
	}
	else
	{
		curveCalibrationData.remove(CALIBRATION_DATA_BASISCURVEBASE + LAString("_") + curveMktName);
		curveCalibrationData.LAObject::add(CALIBRATION_DATA_BASISCURVEBASE + LAString("_") + curveMktName, new LADataString(a_fCurve));
	}

	LAStringMatrix basisDataMtx;
	// rate is virtual or not
	bool isVirtualRate = false;
	LAString isVirtualRateSr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_ISVIRTUAL + mktSuffix).toUpper();
	if (isVirtualRateSr == "TRUE")
	{
		isVirtualRate = true;
	}
	if (isVirtualRate)
	{
		LAString tmp = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_VIRTUALGRID + mktSuffix).toUpper();
		LAStringVector virtualGrid = tmp.toToken(':');
		tmp = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_VIRTUALRATE + mktSuffix).toUpper();
		LAStringVector virtualRate = tmp.toToken(':');
		// size check
		if (virtualRate.size() != virtualGrid.size())
			throw LACoreInvalidData("virtual grids and virtual rates are not same size!!", __FILE__, __LINE__);
		for (size_t i = 0; i < virtualRate.size(); ++i)
		{
			LAStringVector tmp2;
			tmp2.push_back(virtualGrid[i]);
			tmp2.push_back(virtualRate[i]);
			basisDataMtx.push_back(tmp2);
		}
	}
	else
	{
		// Store Basis Data in Temp Property File Data Object
		// ----------------------------------------------------------------------------------------------------------
		// Property File Line:		CCY.sde.yield.basis.file.3m6mbasis=data/in/CCY_yield_basisswap_3m6mbasis.csv
		// Search String Format:	CCY.sde.yield.basis.file.3m6mbasis
		// Result String Format:	data/in/CCY_yield_basisswap_3m6mbasis.csv

		// 1. A temporary data file name is read from the irProp config file
		LAString basisFileName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + mktSuffix);
		
		// 2. If the temp object file name is missing from the irProperties file then create a local name using the same format
		// I think the name used here is irrelevant, but I keep the same format just in case, since who really knows what this crazy code expects
		if( basisFileName.size() == 0 || basisFileName == "NO_DATA" )
		{
			// Remove the '.' (dot) prefix from the mktSuffix and if suffixLowerCase is ill formed then assume 3m6mbasis
			LAString basisMarket = mktSuffix.size() > 1 ? mktSuffix.subString( 1, mktSuffix.size()-1 ) : "3m6mbasis";
			basisFileName = "data/in/" + mktCurrency + "_yield_basisswap_" + basisMarket + ".csv";
		}

		MAFileAccessor basisFile(LAMarketData::getNumFileName(basisFileName));

		basisFile.readAllData(MARKET_DATA_DELIMITER, basisDataMtx);
		basisFile.close();
	}

	LAStringMatrix adjustValueMtx;
	LAString adjustFileName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + mktSuffix);
	if (adjustFileName != MLIB_NO_DATA)
	{
		MAFileAccessor adjustValueFile(LAMarketData::getNumFileName(adjustFileName));
		adjustValueFile.readAllData(MARKET_DATA_DELIMITER, adjustValueMtx);
		adjustValueFile.close();
	}

	if (adjustValueMtx.size() != 0 && adjustValueMtx[0].size() < 2)
	{
		MLIB_THROW("Invalid Market Data: Tenor Basis data is missing")
	}
	DoubleArray adjustValue_term, adjustValue;
	for (size_t i = 0; i < adjustValueMtx.size(); i++)
	{
		adjustValue_term.push_back(adjustValueMtx[i][0].getDoubleValue());
		adjustValue.push_back(adjustValueMtx[i][1].getDoubleValue());
	}
	// get adjust value interpolation
	LAString interpAdjustStr
		= mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_INTERPOLATION + mktSuffix).toUpper();

	// get curve ccy and name
	LAString tmp_fCurve = fCurve;
	LAString tmp_dCurve = dCurve;
	LAString tmp_a_fCurve = a_fCurve;
	LAString tmp_a_dCurve = a_dCurve;
	convertCurveName(tmp_fCurve, curveCurrencyLowerCase, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(tmp_dCurve, curveCurrencyLowerCase, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(tmp_a_fCurve, curveCurrencyLowerCase, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(tmp_a_dCurve, curveCurrencyLowerCase, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	if (ccy_dCurve != curveCurrencyLowerCase || ccy_fCurve != curveCurrencyLowerCase || ccy_a_dCurve != ccy_a_fCurve)
	{
		throw LACoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);
	}

	// Get curve collection ID
	LAObjectHolder& yData = curveCalibrationData.getYieldData().get();
	LAString ydName = yData.getName();
	if (isSetCurveID)
	{
		ydName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	ydName.toUpper();

	// Allow the two curves of the 'against leg' to come from a different curve collection
	// regardless of currency
	bool useAgainstCurveCollection = false;
	LAStringVector a_fCurves_partitioned = tmp_a_fCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	LAStringVector a_dCurves_partitioned = tmp_a_dCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (a_fCurves_partitioned.size() == 2 && a_dCurves_partitioned.size() == 2)
	{
		LAString curveID1 = a_fCurves_partitioned[0].toUpper();
		LAString curveID2 = a_dCurves_partitioned[0].toUpper();
		if (curveID1 == curveID2)
		{
			if (curveID1 == ydName)
			{
				throw LACoreInvalidData("#Error: Please don't prefix dependent curves with a curve collection name that is the same as the target curve.", __FILE__, __LINE__);
			}

			useAgainstCurveCollection = true;
		}
	}

	// Allow the two curves of the 'target leg' to come from a different curve collection
	curveCalibrationData.remove(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + LAString("_") + curveMktName);
	LAStringVector fCurves_partitioned = tmp_fCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	LAStringVector dCurves_partitioned = tmp_dCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (fCurves_partitioned.size() == 2 && dCurves_partitioned.size() == 2)
	{
		LAString curveID1 = fCurves_partitioned[0].toUpper();
		LAString curveID2 = dCurves_partitioned[0].toUpper();

		if (curveID1 == curveID2)
		{
			if (curveID1 == ydName)
			{
				throw LACoreInvalidData("#Error: Please don't prefix dependent curves with a curve collection name that is the same as the target curve.", __FILE__, __LINE__);
			}

			curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + LAString("_") + curveMktName, new LADataString(fCurves_partitioned[0]));
		}
		else
		{
			throw LACoreInvalidData("#Error: Forecast curve and discount curve of the target leg must have the same curve collection prefix", __FILE__, __LINE__);
		}
	}

	// against currency
	bool isForeignCcyLeg = false;
	LAString fYieldDataName;
	curveCalibrationData.remove(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + LAString("_") + curveMktName);
	if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrencyLowerCase
		|| useAgainstCurveCollection)
	{
		isForeignCcyLeg = true;

		if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrencyLowerCase)
		{
			// Re-set the curveType to be XCCYBASIS
			LAString curveType = XCCYBASIS;
			curveCalibrationData.LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveMktName);
			curveCalibrationData.LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveMktName, new LADataString(curveType.toUpper()));

			LAObjectHolder& yieldData = curveCalibrationData.getYieldData().get();
			yieldData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveMktName);
			yieldData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveMktName, new LADataString(curveType.toUpper()));

			// Also re-set the curveType to be XCCYBASIS within the properties
			mpStaticData->setStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + mktSuffix, XCCYBASIS);
		}

		ccy_a_fCurve.toUpper();
		if (isSetCurveID)
		{
			LAStringVector tmp_ccy_a_fCurve = tmp_a_fCurve.toToken(':');
			if (tmp_ccy_a_fCurve.size() == 2)
			{
				fYieldDataName = tmp_ccy_a_fCurve[0];
			}
			else
			{
				throw LACoreInvalidData("no foreign yield data!", __FILE__, __LINE__);
			}
		}
		else
		{
			fYieldDataName = PREFIX_YIELD + getSDEAttrName(ccy_a_fCurve) + "_DATA";
		}

		if (mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_a_fCurve) && enableCalculation)
		{
			loadYieldCurveDataAndCalibrate(ccy_a_fCurve, dataInstance);
		}
		curveCalibrationData.getForeignYieldData().convertFromString(fYieldDataName);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + LAString("_") + curveMktName, new LADataBool(isForeignCcyLeg));

		if (!isSetCurveID)
		{
			LAString baseCcy = curveCurrencyLowerCase; LAString domCcy = curveCurrencyLowerCase; LAString forCcy = ccy_a_fCurve;
			while (1)
			{
				CurveCalibrationData &ycPro_dccy = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(LAMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				CurveCalibrationData &ycPro_fccy = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(LAMarketData::getBaseYieldProName(forCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());

				ycPro_fccy.setAffectingCcy(baseCcy.toUpper());
				const LAString& affectedCcy_fccy = ycPro_fccy.getAffectedCcy();
				if (affectedCcy_fccy == baseCcy)
					throw LACoreInvalidData("The affected currency includes in Affecting currencies!", __FILE__, __LINE__);

				ycPro_dccy.setAffectedCcy(forCcy.toUpper());

				domCcy = forCcy;
				forCcy = ycPro_fccy.getAffectedCcy();

				if (!forCcy.isDefined()) break;
			}
		}

	}

	// if dummy curve reset spotdate 
	if ((a_fCurve == DUMMY || a_dCurve == DUMMY) && !isSpotStarting)
	{
		tmpCal.convertFromString(c_calStr);
		a_c_spotDate = tmpCal.getBusinessDay(asOfDate, a_c_resetLag);
	}

	const unsigned int basisSize = basisDataMtx.size();
	// use grid
	LAStringVector basisUseGrid;
	LAString tmpBasisUseGrid = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + mktSuffix).toUpper();
	if (tmpBasisUseGrid != MLIB_NO_DATA)
	{
		basisUseGrid = tmpBasisUseGrid.toToken(':');
	}
	bool isSimuEq = false;
	LAString strIsSimuEq = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISSIMULTANEOUSEQ + mktSuffix).toUpper();
	if (strIsSimuEq == "TRUE")
	{
		isSimuEq = true;
	}
	bool isSameGridIndex = false;
	LAString strIsFRNMethod = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISSAMEGRIDINDEX + mktSuffix).toUpper();
	if (strIsFRNMethod == "TRUE")
	{
		isSameGridIndex = true;
	}
	bool isYieldSpreadCalc = true;
	LAString strIsYieldSpreadCalc = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISYIELDSPREADCALC + mktSuffix).toUpper();
	if (strIsYieldSpreadCalc == "FALSE")
	{
		isYieldSpreadCalc = false;
	}
	bool isFWDInter = false;
	LAString strFWDInter = FN_SPLINEINTERPOLATION_STR;
	LAString strIsFWDInter = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISFWDINTER + mktSuffix).toUpper();
	if (strIsFWDInter == "TRUE")
	{
		isFWDInter = true;
		strFWDInter = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDINTERPOLATION + mktSuffix);
	}
	bool isEOMRoll = false;
	LAString strEOMRoll = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISEOMRLL + mktSuffix).toUpper();
	if (strEOMRoll != MLIB_NO_DATA)
	{
		LADataBool tmpIsEOMRoll;
		tmpIsEOMRoll.convertFromString(strEOMRoll);
		isEOMRoll = tmpIsEOMRoll.get();
	}
	if (isEOMRoll)
	{
		LAString strEOMDay = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_EOMDAY + mktSuffix).toUpper();
		if (strEOMDay != MLIB_NO_DATA)
		{
			if (c_spotDate.dayOfMonth() != strEOMDay.getIntValue())
			{
				isEOMRoll = false;
			}
		}
		else
		{
			LAPriceDataCalendar cal;
			cal.convertFromString(c_calStr);
			LADate eomDate = cal.getEOMDay(c_spotDate);
			if (c_spotDate != eomDate)
			{
				isEOMRoll = false;
			}
		}
	}
	bool isFwdFX = false;
	LAString isFwdFXStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX + mktSuffix).toUpper();
	if (isFwdFXStr == "TRUE")
	{
		isFwdFX = true;
	}

	// If spotRateTerm is provided, try to find the Libor instrument with matching tenor and make it one of the basis curve instruments
	LAString spotRateTerm = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_SPOTRATETERM + mktSuffix).toUpper();
	double spotRate = DBL_MAX;
	if (spotRateTerm != MLIB_NO_DATA)
	{
		LADataMultiReference marketRef;

		// User-specified source of the Libor fixing
		LAString fixingSource = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FIXINGSOURCE + mktSuffix);
		if (LAString(fixingSource).toUpper() == ITSELF)
		{
			// Populate cash market instruments to object pool
			// Cash instruments include ON, TN and Libor listOfCurvesBuilt
			spotRate = etrading::populateCashInstrumentsToEntityPool(mpStaticData,
																	 staticDataName,
																	 objPool,
																	 mktCurrency,
																	 marketName,
																	 yieldDataName,
																	 mktSuffix,
																	 curveMktName,
																	 isSpotStarting,
																	 isFwdFX,
																	 asOfDate,
																	 ITSELF,
																	 spotRateTerm);

			MLIB_REQUIRE(spotRate != DBL_MAX, "#Error: Can't locate spot Libor fixing rate that corresponds to '" + spotRateTerm + "'.");
		}
		else
		{
			if (fixingSource == MLIB_NO_DATA)
			{
				marketRef = curveCalibrationData.getMarketData();
			}
			else
			{
				// Use fixingSource to obtain an external yieldCurvePro
				LAString foreignYcProName = etrading::LACurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + fixingSource;
				foreignYcProName.toUpper();

				CurveCalibrationData* foreignYieldCurvePro = NULL;

				const LAObjectHolder tmpHolder = objPool.getObject(foreignYcProName);
				if (tmpHolder.isDefined())
				{
					foreignYieldCurvePro = &dynamic_cast<CurveCalibrationData&>(objPool.getObject(foreignYcProName).get());
				}
				else
				{
					LAString err = "#Error: Could not locate the foreign Yield Curve data using '" + foreignYcProName + "'";
					throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
				}

				marketRef = foreignYieldCurvePro->getMarketData();
			}

			for (unsigned int i = 0; i < marketRef.getSize(); ++i)
			{
				const LADataHolder &dh = marketRef.get(i).getData(IR_CALIBRATION_DATA_TERM, NOCHECK);
				if (dh.isDefined() && !dh.isNull())
				{
					LAString term = dynamic_cast<const LADataString &>(dh.get()).get();
					term.toUpper();
					if (spotRateTerm == term)
					{
						spotRate = dynamic_cast<const LADataDouble &>(marketRef.get(i).getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();

						LAObject *mktData = NULL;
						LAString basicCurveName = yieldDataName + "_LIBOR_" + LAString(0) + "_" + curveMktName;
						const LAObjectHolder ehbasis = objPool.getObject(basicCurveName);
						if (!ehbasis.isDefined())
						{
							mktData = new LAObject();
							objPool.set(basicCurveName, mktData);
						}
						else
						{
							objPool.getObject(basicCurveName).get().clear();
							mktData = &objPool.getObject(basicCurveName).get();
						}

						staticDataName += basicCurveName + ":";

						// set term
						mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));

						// set name
						mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(basicCurveName);

						// set spot rate term
						mktData->add(IR_CALIBRATION_DATA_SPOTRATETERM, new LADataString()).convertFromString(term);

						// set data type
						mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_ZERO);

						// Set frequency
						LAString freq = dynamic_cast<const LADataString&> ((marketRef.get(i).getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
						mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freq);

						// Set rate
						mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(spotRate));

						// Set calendar
						const LAPriceDataCalendar& cal = dynamic_cast<const LAPriceDataCalendar&> ((marketRef.get(i).getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
						mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar(cal));

						// Set sliding rule
						const LAPriceDataSlidingRule& sld = dynamic_cast<const LAPriceDataSlidingRule&> ((marketRef.get(i).getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
						mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule(sld));

						// Set date count
						const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&> ((marketRef.get(i).getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
						mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount(dc));

						// Set Spot date
						const LADate& spotdate = dynamic_cast<const LADataDate&> ((marketRef.get(i).getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
						mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotdate));

						// Set EOM
						const LADataHolder* dh = &(marketRef.get(i).getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							bool isEOM = dynamic_cast<const LADataBool &>(dh->get()).get();
							mktData->add(IR_CALIBRATION_DATA_ISEOMROLL, new LADataBool(isEOM));
						}

						break;
					}
				}
			}
		}
	}

	//get constant for convergence
	double eps = 1.0e-9;
	LAString strEPS = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_EPSILON + mktSuffix);
	if (strEPS.toUpper() != MLIB_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	double grad_eps = 1.0e-15;
	LAString strGEPS = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_GRADIENTEPSILON + mktSuffix);
	if (strGEPS.toUpper() != MLIB_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	double delta = 1.0e-10;
	LAString strDLT = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_DELTA + mktSuffix);
	if (strDLT.toUpper() != MLIB_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	int maxLoop = 1000;
	LAString strMLP = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_MAXLOOP + mktSuffix);
	if (strMLP.toUpper() != MLIB_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}

	// cashflow info
	bool isFXForwardQuotesAsOutright = false;
	bool fwd_isPriceCcy = false;
	LAString fwd_slidingStr;
	LAString fwd_calStr;
	LAString fwd_freqStr;
	LADate fwd_spotDate;
	int fwd_resetLag = 0;
	bool fwd_eom = false;
	LAStringMatrix fwdFXDataMtx;
	unsigned int fwdFXSize = 0;
	LAStringVector fwdFXUseGrid;
	double fwdFXPipsizeFactor = 1.0; // The Inverted Pipsize Denominator i.e. 10,000 = 4 d.p. FX Forward Pipsize Quote

	// for fwd fx	
	if (isFwdFX)
	{
		// getStaticData has been overloaded to search for the key and then an alias if missing
		LAString isFXForwardQuotesAsOutrightString = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISRATIO + mktSuffix, mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISFXOUTRIGHT + mktSuffix).toUpper();
		if (isFXForwardQuotesAsOutrightString == "TRUE") isFXForwardQuotesAsOutright = true;
		// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
		if (!isFXForwardQuotesAsOutright)
		{
			fwdFXPipsizeFactor = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_DENOMINATOR + mktSuffix,
															 mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_PIPSIZE + mktSuffix).getDoubleValue(); // Alias Method: First Parameter Takes Priority
			LAString isPriceCcyStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISPRICECCY + mktSuffix,
																  mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISDOMESTICCURRENCY + mktSuffix).toUpper(); // Alias Method: First Parameter Takes Priority
			if (isPriceCcyStr == "TRUE") fwd_isPriceCcy = true;
		}
		fwd_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FREQUENCY + mktSuffix).toUpper();
		fwd_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_SLIDINGRULE + mktSuffix).toUpper();
		fwd_calStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_CALENDAR + mktSuffix);
		fwd_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_RESETLAG + mktSuffix).getIntValue();
		if (isSpotStarting)
		{
			fwd_spotDate.setDate(mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_SPOTDATE + mktSuffix).getCString());
		}
		else
		{
			tmpCal.convertFromString(fwd_calStr);
			fwd_spotDate = tmpCal.getBusinessDay(asOfDate, fwd_resetLag);
		}
		LAString strEOM = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISEOMROLL + mktSuffix).toUpper();
		if (strEOM == "TRUE") fwd_eom = true;
		if (fwd_eom)
		{
			LAString strEOMDay = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_EOMDAY + mktSuffix).toUpper();
			if (strEOMDay != MLIB_NO_DATA)
			{
				if (fwd_spotDate.dayOfMonth() != strEOMDay.getIntValue())
				{
					fwd_eom = false;
				}
			}
			else
			{
				tmpCal.convertFromString(fwd_calStr);
				LADate eomDate = tmpCal.getEOMDay(fwd_spotDate);
				if (fwd_spotDate != eomDate)
				{
					fwd_eom = false;
				}
			}
		}

		LAString fwdFXFileName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FILE + mktSuffix);
		MAFileAccessor fwdFXFile(LAMarketData::getNumFileName(fwdFXFileName));
		fwdFXFile.readAllData(MARKET_DATA_DELIMITER, fwdFXDataMtx);
		fwdFXFile.close();
		if (fwdFXDataMtx.size() == 0 || fwdFXDataMtx[0].size() < 2)
		{
			MLIB_THROW("Invalid Market Data: FX Forward data is missing")
		}
		fwdFXSize = fwdFXDataMtx.size();

		LAString tmpfwdFXUseGrid = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID + mktSuffix).toUpper();
		if (tmpfwdFXUseGrid != MLIB_NO_DATA)
		{
			fwdFXUseGrid = tmpfwdFXUseGrid.toToken(':');
		}
	}
	if ((basisDataMtx.size() == 0 || basisDataMtx[0].size() < 2) && !isFwdFX)
	{
		MLIB_THROW("Invalid Market Data: Basis data is missing")
	}

	// get fwd basis
	bool isFwdBasis = false;
	LAString isFwdBasisStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDBASIS + mktSuffix);
	if (isFwdBasisStr.toUpper() == "TRUE")
	{
		isFwdBasis = true;
	}

	// optimise performance
	bool optimizePerformance = false;
	LAString optimizePerformanceStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE + mktSuffix);
	if (optimizePerformanceStr.toUpper() == "TRUE")
	{
		optimizePerformance = true;
	}

	// fast jacobian rebuild
	bool fastRebuild = false;
	LAString fastRebuildStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD + mktSuffix);
	if (fastRebuildStr.toUpper() == "TRUE")
	{
		fastRebuild = true;
	}


	LAString fixedRateXccyStartTenor = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FIXEDRATEXCCYSTARTTENOR + mktSuffix);
	if (fixedRateXccyStartTenor == MLIB_NO_DATA || fixedRateXccyStartTenor.size() == 0)
	{
		//default to 1Y
		fixedRateXccyStartTenor = "1Y";
	}

	LADate fixedRateXccyStartDate = etrading::LADateHelpers::getDate(asOfDate, fixedRateXccyStartTenor, true);
	
	double ndfSize = 0.0;
	for (unsigned int j = 0; j < fwdFXSize + basisSize; ++j)
	{
		LAObject *mktData = NULL;
		LAString basicCurveName = yieldDataName + "_BASIS_" + LAString(static_cast<int>(j)) + "_" + curveMktName;
		const LAObjectHolder ehbasis = objPool.getObject(basicCurveName);
		if (!ehbasis.isDefined())
		{
			mktData = new LAObject();
			objPool.set(basicCurveName, mktData);
		}
		else
		{
			objPool.getObject(basicCurveName).get().clear();
			mktData = &objPool.getObject(basicCurveName).get();
		}

		staticDataName += basicCurveName + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(basicCurveName);

		// set forward ratio
		if (j < fwdFXSize)
		{
			LAString fwd_term = fwdFXDataMtx[j][0].toUpper();
			double fwd_fx = fwdFXDataMtx[j][1].getDoubleValue();

			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_FWDFX);
			mktData->add(IR_CALIBRATION_DATA_ISRATIO, new LADataBool(isFXForwardQuotesAsOutright));
			mktData->add(IR_CALIBRATION_DATA_ISFXOUTRIGHT, new LADataBool(isFXForwardQuotesAsOutright)); // alias for ISRATIO
			
			// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
			if (isFXForwardQuotesAsOutright)
			{
				mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(fwd_fx));
			}
			else
			{
				mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(fwd_fx / fwdFXPipsizeFactor));
				mktData->add(IR_CALIBRATION_DATA_ISPRICECCY, new LADataBool(fwd_isPriceCcy));
				mktData->add(IR_CALIBRATION_DATA_ISDOMESTICCURRENCY, new LADataBool(fwd_isPriceCcy)); // alias for isPriceCcy
			}
			mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(fwd_term));
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(fwd_slidingStr);
			mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(fwd_calStr);
			mktData->add(IR_CALIBRATION_DATA_SPOTLAG, new LADataInt(fwd_resetLag));
			mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(fwd_spotDate));
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(fwd_freqStr);
			mktData->add(IR_CALIBRATION_DATA_ISEOMROLL, new LADataBool(fwd_eom));
			if (fwdFXUseGrid.size() != 0 && find(fwdFXUseGrid.begin(), fwdFXUseGrid.end(), fwd_term) == fwdFXUseGrid.end())
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
			}
		}
		else
		{
			// Check if we are pricing a fixed-float xccy swap internally, if yes treat the short end instruments as NDF
			if (fCurve == FIXEDRATE && dCurve == FIXEDRATE)
			{
				const unsigned int j1 = j - fwdFXSize;

				LAString ndfTerm = basisDataMtx[j1][0].toUpper();
				const LADate date = etrading::LADateHelpers::getDate(asOfDate, ndfTerm, true);

				// treat swap with less than 1Y as NDF
				if (date < fixedRateXccyStartDate)
				{
					// set term
					mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(ndfTerm));

					// set rate
					double ndfRate = basisDataMtx[j1][1].getDoubleValue();
					mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(ndfRate / 10000.0));

					mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(NDF);
					if (spotRate != DBL_MAX)
					{
						// set spot rate
						mktData->add(IR_CALIBRATION_DATA_SPOTRATE, new LADataDouble(spotRate));
						mktData->add(IR_CALIBRATION_DATA_SPOTRATETERM, new LADataString(spotRateTerm));
					}
					// cashflow info
					mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(c_calStr);
					mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(c_freqStr);
					mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(c_daycStr);
					mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(c_slidingStr);
					//mktData->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE, new LADataDate(c_spotDate));

					ndfSize++;
					continue;
				}
			}

			// set data type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BASIS);
			const unsigned int j2 = j - fwdFXSize;

			// set term
			LAString term = basisDataMtx[j2][0].toUpper();
			mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));

			// set rate
			double basis = basisDataMtx[j2][1].getDoubleValue();
			mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(basis / 10000.0));

			//set fwd basis
			if (isFwdBasis)
			{
				mktData->add(IR_CALIBRATION_DATA_ISFWDBASIS, new LADataBool(isFwdBasis));
				if (basisDataMtx[j2].size() != 5)
				{
					throw LACoreInvalidData("#Error: Invalid FwdBasis Data: FwdBasis data must have 5 columns when isFwdBasis = TRUE and indicate the forward start- and end-dates.", __FILE__, __LINE__);
				}
				const bool isDate = basisDataMtx[j2][2].toUpper() == "TRUE";
				mktData->add(PRICING_DATA_ISDATE, new LADataBool(isDate));
				if (isDate)
				{
					const LADate startDate = LADate(basisDataMtx[j2][3].getCString());
					mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
					const LADate endDate = LADate(basisDataMtx[j2][4].getCString());
					mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
				}
				else
				{
					const LAString startTerm = basisDataMtx[j2][3].toUpper();
					mktData->add(PRICING_DATA_STARTTERM, new LADataString(startTerm));
					const LAString tenor = basisDataMtx[j2][4].toUpper();
					mktData->add(PRICING_DATA_TENOR, new LADataString(tenor));
				}
			}
			// is eom roll
			mktData->add(IR_CALIBRATION_DATA_ISEOMROLL, new LADataBool(isEOMRoll));
			//grid use
			if (basisUseGrid.size() != 0 && find(basisUseGrid.begin(), basisUseGrid.end(), term) == basisUseGrid.end())
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
			}
		}

		// set optimize performance
		mktData->add(IR_CALIBRATION_DATA_OPTIMIZEPERFORMANCE, new LADataBool(optimizePerformance));
		// set fastrebuild
		mktData->add(IR_CALIBRATION_DATA_FASTREBUILD, new LADataBool(fastRebuild));
		// set simultaneous equation
		mktData->add(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS, new LADataBool(isSimuEq));
		// set epsilon
		mktData->add(IR_CALIBRATION_DATA_EPSILON, new LADataDouble(eps));
		// set gradient epsilon
		mktData->add(IR_CALIBRATION_DATA_GRADIENTEPSILON, new LADataDouble(grad_eps));
		// set delta
		mktData->add(IR_CALIBRATION_DATA_DELTA, new LADataDouble(delta));
		// set maxloop
		mktData->add(IR_CALIBRATION_DATA_MAXLOOP, new LADataInt(maxLoop));
		// set issamegridindex
		mktData->add(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX, new LADataBool(isSameGridIndex));
		// set isyieldspreadcalc
		mktData->add(IR_CALIBRATION_DATA_ISYIELDSPREADCALC, new LADataBool(isYieldSpreadCalc));
		if (spotRate != DBL_MAX)
		{
			// set spot rate
			mktData->add(IR_CALIBRATION_DATA_SPOTRATE, new LADataDouble(spotRate));
			mktData->add(IR_CALIBRATION_DATA_SPOTRATETERM, new LADataString(spotRateTerm));
		}
		// set fwd interpolation
		mktData->add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, new LADataBool(isFWDInter));
		mktData->add(IR_CALIBRATION_DATA_FWDINTERPOLATION, new LAPriceDataInterpolation()).convertFromString(strFWDInter);
		// curve info
		mktData->add(IR_CALIBRATION_DATA_ISDISCOUNT, new LADataBool(isDiscount));
		mktData->add(IR_CALIBRATION_DATA_ISAGTSPREAD, new LADataBool(isAgtSpread));
		mktData->add(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, new LADataBool()).convertFromString(isTimeInterStr);

		mktData->add(IR_CALIBRATION_DATA_FORECAST, new LADataString()).convertFromString(fCurve);
		mktData->add(IR_CALIBRATION_DATA_DISCOUNT, new LADataString()).convertFromString(dCurve);
		mktData->add(IR_CALIBRATION_DATA_AGTCURRENCY, new LADataString()).convertFromString(ccy_a_fCurve);
		mktData->add(IR_CALIBRATION_DATA_AGTFORECAST, new LADataString()).convertFromString(a_fCurve);
		mktData->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new LADataString()).convertFromString(a_dCurve);
		// cashflow info
		mktData->add(IR_CALIBRATION_DATA_CASHLETCALENDAR, new LAPriceDataCalendar()).convertFromString(c_calStr);
		mktData->add(IR_CALIBRATION_DATA_CASHLETFREQUENCY, new LADataString()).convertFromString(c_freqStr);
		// set frequency of compounding
		if (c_freqcpdStr != MLIB_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND, new LADataString()).convertFromString(c_freqcpdStr);
		}
		mktData->add(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, new LAPriceDataDayCount()).convertFromString(c_daycStr);
		mktData->add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(c_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE, new LADataDate(c_spotDate));
		mktData->add(IR_CALIBRATION_DATA_CASHLETRESETLAG, new LADataInt(c_resetLag));
		// index info
		if (isSameGridIndex)
		{
			mktData->add(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(c_calStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, new LAPriceDataCalendar()).convertFromString(c_calStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXFREQUENCY, new LADataString()).convertFromString(SIMPLE);
			mktData->add(IR_CALIBRATION_DATA_INDEXACCESSARY, new LADataString()).convertFromString(getAccFromFreq(c_freqStr));
			mktData->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(c_daycStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(c_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXRESETLAG, new LADataInt(c_resetLag));
		}
		else
		{
			mktData->add(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(i_fixcalStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, new LAPriceDataCalendar()).convertFromString(i_paycalStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXFREQUENCY, new LADataString()).convertFromString(i_freqStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXACCESSARY, new LADataString()).convertFromString(i_accessaryStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(i_daycStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(i_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXRESETLAG, new LADataInt(i_resetLag));
		}
		// against cashflow info
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETCALENDAR, new LAPriceDataCalendar()).convertFromString(a_c_calStr);
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, new LADataString()).convertFromString(a_c_freqStr);
		// set frequency of compounding
		if (a_c_freqcpdStr != MLIB_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND, new LADataString()).convertFromString(a_c_freqcpdStr);
		}
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETDAYCOUNT, new LAPriceDataDayCount()).convertFromString(a_c_daycStr);
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(a_c_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETSPOTDATE, new LADataDate(a_c_spotDate));
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETRESETLAG, new LADataInt(a_c_resetLag));
		// against index info
		if (isSameGridIndex)
		{
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(a_c_calStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, new LAPriceDataCalendar()).convertFromString(a_c_calStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, new LADataString()).convertFromString(SIMPLE);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, new LADataString()).convertFromString(getAccFromFreq(a_c_freqStr));
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(a_c_daycStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(a_c_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, new LADataInt(a_c_resetLag));
		}
		else
		{
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(a_i_fixcalStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, new LAPriceDataCalendar()).convertFromString(a_i_paycalStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, new LADataString()).convertFromString(a_i_freqStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, new LADataString()).convertFromString(a_i_accessaryStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(a_i_daycStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(a_i_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, new LADataInt(a_i_resetLag));
		}
		// set market rate interpolation
		if (interpStr != MLIB_NO_DATA)
		{
			interpStr.toLower();
			LAPriceDataInterpolation interpAtt;
			interpAtt.convertFromString(interpStr);
			mktData->add(CALIBRATION_DATA_INTERPOLATION, new LAPriceDataInterpolation(interpAtt));
		}
		// set adjust value interpolation
		if (interpAdjustStr != MLIB_NO_DATA)
		{
			interpAdjustStr.toLower();
			LAPriceDataInterpolation interpAtt;
			interpAtt.convertFromString(interpAdjustStr);
			mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, new LAPriceDataInterpolation(interpAtt));
			mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new LADataDoubles(adjustValue_term));
			mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUE, new LADataDoubles(adjustValue));
		}
		// set adjust value
		//mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new LADataDoubles(adjustValue_term));
		//mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUE, new LADataDoubles(adjustValue));
	}

	//set foreign curve data
	if (pMktCurrency)
	{
		LAObject& fYieldData = objPool.getObject(fYieldDataName, ENCHKTYPE_ISDEFINED).get();
		fYieldData.remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + marketName);
		fYieldData.add(CALIBRATION_DATA_MARKETDATA + LAString("_") + marketName, new LADataMultiReference()).convertFromString(staticDataName.subString(0, staticDataName.size() - 2));
	}

	LAString aliasCurveNames = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + curveSuffix);
	if (aliasCurveNames != MLIB_NO_DATA)
	{
		LAStringVector assignedCurves = aliasCurveNames.toToken(MULTI_STATIC_DATA_DELIMITER);
		for (size_t i = 0; i < assignedCurves.size(); i++)
		{
			curveCalibrationData.setAssignedCurveMktMap(assignedCurves[i], curveMktName);
		}
	}
	else
	{
		curveCalibrationData.setAssignedCurveMktMap(curveMktName, curveMktName);
	}

	if (interpAdjustStr != MLIB_NO_DATA)
	{
		interpAdjustStr.toLower();
		LAPriceDataInterpolation interpAtt;
		interpAtt.convertFromString(interpAdjustStr);
		if (pMktCurrency)
		{
			LAObject& fYieldData = objPool.getObject(fYieldDataName, ENCHKTYPE_ISDEFINED).get();
			fYieldData.remove(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION);
			fYieldData.remove(IR_CALIBRATION_DATA_ADJUSTVALUETERM);
			fYieldData.remove(IR_CALIBRATION_DATA_ADJUSTVALUE);
			fYieldData.add(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, new LAPriceDataInterpolation(interpAtt));
			fYieldData.add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new LADataDoubles(adjustValue_term));
			fYieldData.add(IR_CALIBRATION_DATA_ADJUSTVALUE, new LADataDoubles(adjustValue));
		}
		else
		{
			LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
			yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION);
			yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUETERM);
			yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUE);
			yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, new LAPriceDataInterpolation(interpAtt));
			yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new LADataDoubles(adjustValue_term));
			yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUE, new LADataDoubles(adjustValue));
		}
	}

	// get fwd swap
	bool areSwapsForwardStarting = false;
	LADataBool tmpAttrB;
	LAString isFwdSwap_str = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + mktSuffix);
	if (isFwdSwap_str != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		areSwapsForwardStarting = tmpAttrB.get();
	}

	//FRA Data
	bool useTenorBasis = false;
	LAString staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	LAString suffix_data = "_" + marketName;

	LAString curveName("");
	if (propertyManagerCurveName != STD)
	{
		curveName = "." + propertyManagerCurveName;
	}
	curveName.toLower();
	LAString curveType = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + curveName);

	// Only process FRAs when building basis curves.
	// In the case of AUD curves, OIS and STD curves would process basis instruments and the
	// processing of FRAs is not allowed in that case.
	if (curveType != "OIS" && curveType != "SWAP")
	{
		etrading::populateFRADataToEntityPool(mpStaticData,
											  curveCalibrationData,
											  staticDataName,
											  objPool,
											  curveCurrencyLowerCase,
											  marketName,
											  yieldDataName,
											  staticDataSuffix,
											  suffix_data,
											  useTenorBasis,
											  isSpotStarting,
											  areSwapsForwardStarting,
											  asOfDate,
											  true);		// isBasisCurve
	}
}

// 
/*!
	@brief fuction to set up 36 basis curve dummy data
*/
void
LAUpdateCurveObject::setUp36BasisDummyData(LADataInstance &dataInstance, LAString &staticDataName, const LADate &asOfDate, const LAString &currency,
	const LAString &marketName, const LAString &yieldDataName, bool isSpotStarting,
	CurveCalibrationData &curveCalibrationData) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	LAString suffixLowerCase = "." + marketName;
	suffixLowerCase.toLower();

	// get swap market
	LAString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE);
	MAFileAccessor swapFile(LAMarketData::getNumFileName(swapFileName));
	LAStringMatrix swapDataMtx;
	swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
	swapFile.close();
	if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2)
	{
		MLIB_THROW("Invalid Market Data: Swap data is missing")
	}
	const int swapSize = swapDataMtx.size();
	LAString daycSStr_float = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT).toUpper();
	LAString interpSStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_INTERPOLATION).toLower();
	LAPriceDataCalendar cal;
	LAString calStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR).toLower();
	cal.convertFromString(calStr);
	LADate spotDate;
	int resetLag;
	if (isSpotStarting)
	{
		spotDate.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SPOTDATE).getCString());
	}
	else
	{
		LAString reseLag_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG);
		resetLag = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG).getIntValue();
		if (reseLag_str == MLIB_NO_DATA)
		{
			throw LACoreInvalidData("Reset Lag is not set !!", __FILE__, __LINE__);
		}
		else
		{
			spotDate = cal.getBusinessDay(asOfDate, resetLag);
		}
	}
	// get sliding
	LAString slidingStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE).toUpper();

	for (int j = 0; j < swapSize; ++j)
	{
		LAString term = swapDataMtx[j][0].toUpper();
		double basis = 0.;

		LAObject *mktData = NULL;
		LAString basicCurveName = yieldDataName + "_" + marketName + "_" + LAString(static_cast<int>(j));
		const LAObjectHolder ehbdummy = objPool.getObject(basicCurveName);
		if (!ehbdummy.isDefined())
		{
			mktData = new LAObject();
			objPool.set(basicCurveName, mktData);
		}
		else
		{
			objPool.getObject(basicCurveName).get().clear();
			mktData = &objPool.getObject(basicCurveName).get();
		}
		staticDataName += basicCurveName + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(basicCurveName);
		// curve info
		mktData->add(IR_CALIBRATION_DATA_ISAGTSPREAD, new LADataBool(false));
		// frequency info
		mktData->add(IR_CALIBRATION_DATA_CASHLETFREQUENCY, new LADataString()).convertFromString(QUARTERLY);
		// against frequency info
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, new LADataString()).convertFromString(SEMI_ANNUAL);
		// index info
		mktData->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycSStr_float);
		// against index info
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycSStr_float);
		// calendar
		mktData->add(IR_CALIBRATION_DATA_CASHLETCALENDAR, new LAPriceDataCalendar(cal));
		// spot date
		mktData->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE, new LADataDate(spotDate));
		// slidingrule
		mktData->add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingStr);
		// market rate interpolation
		LAPriceDataInterpolation interpSAtt;
		interpSAtt.convertFromString(interpSStr);
		mktData->add(CALIBRATION_DATA_INTERPOLATION, new LAPriceDataInterpolation(interpSAtt));
		// set data type
		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BASIS);
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));
		// set rate
		mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(basis / 10000.0));
	}
}

// Set-up Base Curve Data
void LAUpdateCurveObject::setUpGenCurveData(LADataInstance &dataInstance, LAString &staticDataName, const LADate &asOfDate, const LAString &currency,
	const LAString &marketName, const LAString &yieldDataName, bool isSpotStarting, bool useTenorBasis, CurveCalibrationData &curveCalibrationData,
	MarketDataTenorQuotes &tenorBasisMarketQuotes, const LAString& curveName) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAString staticDataSuffix = "";

	if (marketName == SWAP)
	{
		if (curveName != STD && curveName.size() != 0)
		{
			staticDataSuffix = "." + curveName;
			staticDataSuffix.toLower();
		}
	}
	else
	{
		staticDataSuffix = "." + marketName;
		staticDataSuffix.toLower();
	}


	LAString suffix_data = "";
	if (marketName == SWAP)
	{
		if (curveName != STD && curveName.size() != 0)
		{
			suffix_data = "_" + curveName;
		}
	}
	else
	{
		suffix_data = "_" + marketName;
	}

	bool isFwdFX = false;
	LAString isFwdFXStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX + staticDataSuffix);
	if (isFwdFXStr.toUpper() == "TRUE")
	{
		isFwdFX = true;
	}

	// Populate cash market instruments to object pool
	// Cash instruments include ON, TN and Libor listOfCurvesBuilt
	etrading::populateCashInstrumentsToEntityPool(mpStaticData,
												  staticDataName,
												  objPool,
												  currency,
												  marketName,
												  yieldDataName,
												  staticDataSuffix,
												  suffix_data,
												  isSpotStarting,
												  isFwdFX,
												  asOfDate);

	//DF curve name
	LAString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix);
	if (dfCurveName == MLIB_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new LADataString(dfCurveName));
	LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new LADataString(dfCurveName));

	// get fwd swap
	bool areSwapsForwardStarting = false;
	LADataBool tmpAttrB;
	LAString isFwdSwap_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + staticDataSuffix);
	if (isFwdSwap_str != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		areSwapsForwardStarting = tmpAttrB.get();
	}

	// Do we always recalculate the dynamic linear spline join date?
	LAString alwaysCalcJoinDate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE + staticDataSuffix);
	curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	if (alwaysCalcJoinDate != MLIB_NO_DATA && alwaysCalcJoinDate.size() != 0)
	{
		if (alwaysCalcJoinDate.toUpper() == "TRUE")
		{
			curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(true));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(true));
		}
		else
		{
			curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(false));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(false));
		}
	}

	//Set stateVariable
	LAString stateVariableStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_STATEVARIABLE + staticDataSuffix);
	curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_STATEVARIABLE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_STATEVARIABLE + suffix_data);
	if (stateVariableStr != MLIB_NO_DATA && stateVariableStr.size() != 0)
	{
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_STATEVARIABLE + suffix_data, new LADataString(stateVariableStr));
		yldEntity.add(IR_CALIBRATION_DATA_STATEVARIABLE + suffix_data, new LADataString(stateVariableStr));
	}

	// IsFutureUse flag
	LAString tmpCurrency = currency;
	tmpCurrency.toLower();
	bool isFutureUse = false;
	LAString tmpFutureStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + staticDataSuffix);
	if (tmpFutureStr != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(tmpFutureStr);
		isFutureUse = tmpAttrB.get();
	}

	// Hybrid Spline interpolation cut off date
	LAString inputInterpJoinDateStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATIONJOINDATE + staticDataSuffix);
	curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	if (inputInterpJoinDateStr != MLIB_NO_DATA && inputInterpJoinDateStr.size() != 0)
	{
		std::string calKey = isFutureUse ? STATIC_DATA_KEY_YIELD_FUTURE_CALENDAR : STATIC_DATA_KEY_YIELD_FRA_CALENDAR;
		std::string spotDateKey = isFutureUse ? STATIC_DATA_KEY_YIELD_FUTURE_SPOTDATE : STATIC_DATA_KEY_YIELD_FRA_SPOTDATE;
		std::string spotLagKey = isFutureUse ? STATIC_DATA_KEY_YIELD_FUTURE_RESETLAG : STATIC_DATA_KEY_YIELD_FRA_RESETLAG;

		LAPriceDataCalendar cal;
		LAString calStr = mpStaticData->getStaticData(currency + calKey + staticDataSuffix);
		cal.convertFromString(calStr);

		LADate spotDate;
		if (isSpotStarting)
		{
			spotDate.setDate(mpStaticData->getStaticData(currency + spotDateKey + staticDataSuffix).getCString());
		}
		else
		{
			spotDate = cal.getBusinessDay(asOfDate, mpStaticData->getStaticData(tmpCurrency + spotLagKey + staticDataSuffix).getIntValue());
		}

		//Use NO_CHANGE as businessDayAdj
		LADate inputInterpolationJoinDate = etrading::validateDateOrTenor(spotDate, inputInterpJoinDateStr, "NO_CHANGE", calStr, "Invalid joinDate");

		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputInterpolationJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputInterpolationJoinDate));
	}

	//FRA Data
	etrading::populateFRADataToEntityPool(mpStaticData,
										  curveCalibrationData,
										  staticDataName,
										  objPool,
										  currency,
										  marketName,
										  yieldDataName,
										  staticDataSuffix,
										  suffix_data,
										  useTenorBasis,
										  isSpotStarting,
										  areSwapsForwardStarting,
										  asOfDate);

	// future

	/* Handle isFutureUse flag:
	*  Some care is needed here since some currencies (such as AUD) can have TWO swap curves.
	*  If the curveType is SWAP, *and* the curveName is STD, then use the simple data flag stored in curveCalibrationData.
	*  Otherwise, for non-STD curves use the full object pool data IR_CALIBRATION_DATA_ISFUTUREUSE plus suffixLowerCase.
	*  This prevents the settings of two or more swap curves from overwriting each other.
	*/
	if ((marketName == SWAP) && (curveName == STD))
	{
		curveCalibrationData.getIsFutureUse().set(isFutureUse);
	}
	else
	{
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data, new LADataBool(isFutureUse));
	}

	if (isFutureUse)
	{
		LAString futureFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix);
		MAFileAccessor futureFile(LAMarketData::getNumFileName(futureFileName));
		LAStringMatrix futureDataMtx;
		futureFile.readAllData(MARKET_DATA_DELIMITER, futureDataMtx);
		futureFile.close();
		// use grid
		LAStringVector futureUseGrid;
		LAString tmpFutureUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + staticDataSuffix).toUpper();
		if (tmpFutureUseGrid != MLIB_NO_DATA)
		{
			futureUseGrid = tmpFutureUseGrid.toToken(':');
		}
		else
		{
			LAString tmpUseGridNum = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRIDNUM + staticDataSuffix).toUpper();
			if (tmpUseGridNum != MLIB_NO_DATA)
			{
				int useGridNum = tmpUseGridNum.getIntValue();
				if (useGridNum == 0)
				{
					futureUseGrid.push_back(MLIB_NO_DATA);
				}
				else
				{
					for (size_t i = 0; i < (size_t)useGridNum; ++i)
					{
						if (futureDataMtx.size() <= i) break;
						futureUseGrid.push_back(futureDataMtx[i][0]);
					}
				}
			}
		}

		// get cal and calc spot date
		LAPriceDataCalendar calF;
		LAString calFStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_CALENDAR + staticDataSuffix);
		calF.convertFromString(calFStr);
		// get freq
		LAString freqFStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FREQUENCY + staticDataSuffix).toUpper();
		// get daycount
		LAString daycFStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_DAYCOUNT + staticDataSuffix).toUpper();
		// get sliding
		LAPriceDataSlidingRule slidingF;
		LAString slidingFStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_SLIDINGRULE + staticDataSuffix).toUpper();
		slidingF.convertFromString(slidingFStr);
		//spot lag
		LAString spotLagF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_RESETLAG + staticDataSuffix);
		//is convexity adjust precise
		bool isConvAdjPrecise = false;
		LAString isConvAdjPrecise_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_ISCONVADJPRECISE + staticDataSuffix);
		if (isConvAdjPrecise_str != MLIB_NO_DATA)
		{
			tmpAttrB.convertFromString(isConvAdjPrecise_str);
			isConvAdjPrecise = tmpAttrB.get();
		}
		// get mean reversion
		double meanReversion = 0.0;
		LAString meanReversion_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_MEANREVERSION + staticDataSuffix);
		if (meanReversion_str != MLIB_NO_DATA)
		{
			LADataDouble tmpAttrDouble;
			tmpAttrDouble.convertFromString(meanReversion_str);
			meanReversion = tmpAttrDouble.get();
		}

		// get applyTension
		bool applyTensionFutures = false;
		LAString applyTensionFuturesStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_APPLYTENSION + staticDataSuffix).toUpper();
		if (applyTensionFuturesStr != MLIB_NO_DATA)
		{
			LADataBool tmpApplyTensionFutures;
			tmpApplyTensionFutures.convertFromString(applyTensionFuturesStr);
			applyTensionFutures = tmpApplyTensionFutures.get();
		}
		
		// *** CONVEXITY QUOTE TYPE *** 
		// ----------------------------------------------------------

		// get UseConvexAdjustment (ConvexityQuotedAsPrice), legacy parameter replaced by ConvexityQuoteType
		bool useConvexAdjustment = false;
		bool convexityQuotedAsVol = true;
		LAString useConvexAdjustmentStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USECONVEXADJUSTMENT + staticDataSuffix).toUpper();
		if (useConvexAdjustmentStr != MLIB_NO_DATA)
		{
			LADataBool tmpUseConvexAdjustment;
			tmpUseConvexAdjustment.convertFromString(useConvexAdjustmentStr);
			useConvexAdjustment = tmpUseConvexAdjustment.get();
			convexityQuotedAsVol = !useConvexAdjustment;
		}

		// get convexityQuoteType *** Alias for UseConvexAdjustment (convexityQuoteAsPrice) ***
		LAString convexityQuoteTypeStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_CONVEXITYQUOTETYPE + staticDataSuffix).toUpper();
		if (convexityQuoteTypeStr != MLIB_NO_DATA)
		{
			convexityQuoteTypeStr.toUpper();
			MLIB_REQUIRE( convexityQuoteTypeStr == "VOL" || convexityQuoteTypeStr == "PRICE", "Invalid Futures Convexity Quote Type: ConvexityQuoteType must be VOL or PRICE" )
			convexityQuotedAsVol = ( convexityQuoteTypeStr == "VOL" ) ? true : false;

			// TODO: Property Manager does not allow us to clear parameters once set ... hence the below fails ... prioritize the convexityQuoteType parameter for now
			// if ( useConvexAdjustmentStr != MLIB_NO_DATA )
			// {
			// 	MLIB_THROW("Invalid Futures Convexity Parameter: Cannot use ConvexityQuoteType and UseConvexAdjustment (ConvexityQuotedAsPrice) parameters at the same time.")
			// }
		}
		
		// Important Update ConvexityQuoteString for Object Pool Setting
		convexityQuoteTypeStr = convexityQuotedAsVol ? "VOL" : "PRICE";

		// ----------------------------------------------------------

		// Curve controls
		bool smoothShortEnd = true;
		LAString strSmoothShortEnd = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_SMOOTHSHORTEND + staticDataSuffix);
		if (strSmoothShortEnd != MLIB_NO_DATA)
		{
			LADataBool tmpSmoothShortEnd;
			tmpSmoothShortEnd.convertFromString(strSmoothShortEnd);
			smoothShortEnd = tmpSmoothShortEnd.get();
		}

		// get tensionGap
		int tensionGapFutures = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_TENSIONGAP + staticDataSuffix).getIntValue();

		// Default to RATE
		LAString futureSerialCalcType = etrading::toString(etrading::SERIAL_FUTURES_BY_RATE);
		auto dh = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_SERIAL_CALC_TYPE + staticDataSuffix);
		if (dh != MLIB_NO_DATA)
		{
			futureSerialCalcType = dh.getCString();
		}

		const int futureSize = futureDataMtx.size();
		for (int i = 0; i < futureSize; ++i)
		{
			if (futureDataMtx[i].size() != 5 && futureDataMtx[i].size() != 3)
			{
				throw LACoreInvalidData("Future File format is wrong", __FILE__, __LINE__);
			}
			LAString term;
			LADate startDate, endDate;
			double futurePrice(0.0), rate(0.0), futureVol(0.0), convexAdj(0.0);

			if (futureDataMtx[i].size() == 5)
			{
				term = futureDataMtx[i][0].toUpper();
				startDate = LADate(futureDataMtx[i][1].getCString());
				endDate = LADate(futureDataMtx[i][2].getCString());
				futurePrice = futureDataMtx[i][3].getDoubleValue();
				rate = 1.0 - futurePrice * 0.01;

				if (convexityQuotedAsVol)
				{
					// Futures Convexity Quoted as a Volatility
					futureVol = futureDataMtx[i][4].getDoubleValue();
				}
				else
				{
					// Futures Convexity Quoted as a Price
					convexAdj = futureDataMtx[i][4].getDoubleValue();
				}
			}
			else if (futureDataMtx[i].size() == 3)
			{
				term = futureDataMtx[i][0].toUpper();
				startDate = etrading::LADateHelpers::getIMMDateFromTerm(asOfDate, term);
				startDate = etrading::LADateHelpers::getDate(startDate, "0d", slidingF, &calF, true);
				endDate = etrading::LADateHelpers::getDate(startDate, "3M", slidingF, &calF, true);
				int mm = endDate.monthOfYear();
				int yy = endDate.yearOfEra();
				endDate = etrading::LADateHelpers::getIMMDate(yy, mm, true);

				futurePrice = futureDataMtx[i][1].getDoubleValue();
				rate = 1.0 - futurePrice * 0.01;

				if (convexityQuotedAsVol)
				{
					// Futures Convexity Quoted as a Volatility
					futureVol = futureDataMtx[i][2].getDoubleValue();
					
				}
				else
				{
					// Futures Convexity Quoted as a Price
					convexAdj = futureDataMtx[i][2].getDoubleValue();
				}
			}
			else
			{
				throw LACoreInvalidData("Future File format is wrong", __FILE__, __LINE__);
			}

			LAObject *mktData = NULL;
			LAString nameF = yieldDataName + "_FUTURE_" + LAString(i) + suffix_data;
			const LAObjectHolder ehfuture = objPool.getObject(nameF);
			if (!ehfuture.isDefined())
			{
				mktData = new LAObject();
				objPool.set(nameF, mktData);
			}
			else
			{
				objPool.getObject(nameF).get().clear();
				mktData = &objPool.getObject(nameF).get();
			}
			staticDataName += nameF + ":";
			// set name
			mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameF);
			// set term
			mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(term);
			// set calendar
			mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(calFStr);
			// set daycount
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycFStr);
			// set data type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_FUTURE);
			// set slidingrule
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingFStr);
			// set start date
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			// set end date
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
			// set rate
			mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(rate));
			// is convexity adjsut presice
			mktData->add(IR_CALIBRATION_DATA_ISCONVADJPRECISE, new LADataBool(isConvAdjPrecise));
			// set mean reversion
			mktData->add(IR_CALIBRATION_DATA_MEANREVERSION, new LADataDouble(meanReversion));
			// set vol
			mktData->add(PRICING_DATA_FUTUREVOLATILITY, new LADataDouble(futureVol));
			
			// set convexAdj - The numerical convexity value
			mktData->add(PRICING_DATA_CONVEXADJUSTMENT, new LADataDouble(convexAdj));
			
			// Convexity Quoted as Price (useConvexAdj) or as Vol
			// set the replacement convexityQuotedAsVol alias for legacy parameter useConvexAdj, which is deprecated
			mktData->add(PRICING_DATA_CONVEXITYQUOTETYPE, new LADataString(convexityQuoteTypeStr));
			
			// set tension
			mktData->add(IR_CALIBRATION_DATA_APPLYTENSION, new LADataBool(applyTensionFutures));
			// set tension gap
			mktData->add(IR_CALIBRATION_DATA_TENSIONGAP, new LADataInt(tensionGapFutures));
			// use grid
			if (futureUseGrid.size() != 0 && find(futureUseGrid.begin(), futureUseGrid.end(), term) == futureUseGrid.end())
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
			}
			// smooth short end of curve
			mktData->add(IR_CALIBRATION_DATA_SMOOTHSHORTEND, new LADataBool(smoothShortEnd));
			// serial calc type
			mktData->add(IR_CALIBRATION_DATA_FRAFUTURE_SERIAL_CALC_TYPE, new LADataString(futureSerialCalcType));
		}
	}

	// swap
	LAString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	MAFileAccessor swapFile(LAMarketData::getNumFileName(swapFileName));
	LAStringMatrix swapDataMtx;
	if (!isFwdFX)
	{
		swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
		swapFile.close();
	}

	if ((swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2) && !isFwdFX)
	{
		MLIB_THROW("Invalid Market Data: Swap data is missing")
	}

	// get cal and calc spot date
	LAPriceDataCalendar calS;
	LAString calSStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + staticDataSuffix);
	calS.convertFromString(calSStr);

	LADate spotDateS;
	int resetLag;
	if (isSpotStarting)
	{
		spotDateS.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SPOTDATE + staticDataSuffix).getCString());
	}
	else
	{
		LAString resetLag_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG + staticDataSuffix);
		resetLag = resetLag_str.getIntValue();
		if (resetLag_str == MLIB_NO_DATA)
		{
			throw LACoreInvalidData("Reset Lag is not set !!", __FILE__, __LINE__);
		}
		else
		{
			if ( ( marketName == SWAP ) && ( same( currency, "USD" ) ) )
			{
				/*	Rule for calculating the spot date for USD swaps:
					First apply the spot lag using the Libor fixing calendar ( e.g LNB ),
					then apply a further 0D lag with the swap payment calendar ( NYB ) to move onto the
					final business day.
					This is important to get right around US holidays.
				*/
				const LAString liborFixingCalendar = mpStaticData->getStaticData( currency + STATIC_DATA_KEY_YIELD_LIBOR_CALENDAR + staticDataSuffix );

				const int tenorPosition = resetLag_str.findString('D');
				if ( tenorPosition == -1 )
				{
					// Add the 'D' tenor if it is missing
					resetLag_str += 'D';
				}
				const LAString businessDayAdjustment("FOLLOWING"); // Specify this to request a Business-day adjustment
				const LAString rollConvention;
				spotDateS = etrading::getCurveUSDSpotDate( asOfDate, resetLag_str, liborFixingCalendar, calSStr, businessDayAdjustment, rollConvention );
			}
			else
			{
				spotDateS = calS.getBusinessDay(asOfDate, resetLag);
			}
		}
	}
	bool resetFlg = true;
	bool isTimeInterSW = false;
	bool isNRSW = false;
	bool isSimuEQSW = false;
	bool isEOMRollSW = false;
	bool isFWDInter = false;
	LAString strIsTimeInterSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISTIMEINTERPOLATION + staticDataSuffix).toUpper();
	if (strIsTimeInterSW != MLIB_NO_DATA)
	{
		LADataBool tmpIsTimeInterSW;
		tmpIsTimeInterSW.convertFromString(strIsTimeInterSW);
		isTimeInterSW = tmpIsTimeInterSW.get();
	}
	LAString strNRSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISNEWTONRAPHSON + staticDataSuffix).toUpper();
	if (strNRSW != MLIB_NO_DATA)
	{
		LADataBool tmpIsNRSW;
		tmpIsNRSW.convertFromString(strNRSW);
		isNRSW = tmpIsNRSW.get();
	}
	LAString strSimuEQSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISSIMULTANEOUSEQ + staticDataSuffix).toUpper();
	if (strSimuEQSW != MLIB_NO_DATA)
	{
		LADataBool tmpIsSimuEQSW;
		tmpIsSimuEQSW.convertFromString(strSimuEQSW);
		isSimuEQSW = tmpIsSimuEQSW.get();
	}
	LAString strEOMRollSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISEOMRLL + staticDataSuffix).toUpper();
	if (strEOMRollSW != MLIB_NO_DATA)
	{
		LADataBool tmpIsEOMRollSW;
		tmpIsEOMRollSW.convertFromString(strEOMRollSW);
		isEOMRollSW = tmpIsEOMRollSW.get();
	}
	if (isEOMRollSW)
	{
		LAString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_EOMDAY + staticDataSuffix).toUpper();
		if (strEOMDay != MLIB_NO_DATA)
		{
			if (spotDateS.dayOfMonth() != strEOMDay.getIntValue())
			{
				isEOMRollSW = false;
			}
		}
		else
		{
			const LADate eomDate = calS.getEOMDay(spotDateS);
			if (spotDateS != eomDate)
			{
				isEOMRollSW = false;
			}
		}
	}

	LAString strFWDInter = FN_SPLINEINTERPOLATION_STR;
	LAString strIsFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISFWDINTER + staticDataSuffix).toUpper();
	if (strIsFWDInter == "TRUE")
	{
		isFWDInter = true;
		strFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FWDINTERPOLATION + staticDataSuffix);
	}

	LAString optimizeMethod = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_OPTIMIZEMETHOD + staticDataSuffix).toUpper();

	//get constant for convergence
	double eps = 1.0e-9;
	LAString strEPS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_EPSILON + staticDataSuffix);
	if (strEPS.toUpper() != MLIB_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	double grad_eps = 1.0e-15;
	LAString strGEPS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_GRADIENTEPSILON + staticDataSuffix);
	if (strGEPS.toUpper() != MLIB_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	double delta = 1.0e-10;
	LAString strDLT = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DELTA + staticDataSuffix);
	if (strDLT.toUpper() != MLIB_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	int maxLoop = 1000;
	LAString strMLP = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_MAXLOOP + staticDataSuffix);
	if (strMLP.toUpper() != MLIB_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	LAString swapTenor("");
	LAString strSwapTenor = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTENOR + staticDataSuffix);
	if (strSwapTenor.toUpper() != MLIB_NO_DATA)
	{
		swapTenor = strSwapTenor;
	}
	LAString swapType("LIBOR");
	LAString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTYPE + staticDataSuffix);
	if (strSwapType.toUpper() != MLIB_NO_DATA)
	{
		swapType = strSwapType;
	}

	// optimise performance
	bool optimizePerformance = false;
	LAString optimizePerformanceStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE + staticDataSuffix);
	if (optimizePerformanceStr.toUpper() == "TRUE")
	{
		optimizePerformance = true;
	}

	// fast jacobian rebuild
	bool fastRebuild = false;
	LAString fastRebuildStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD + staticDataSuffix);
	if (fastRebuildStr.toUpper() == "TRUE")
	{
		fastRebuild = true;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	LAString isResetSkipStr = mpStaticData->getStaticData(KEY_YIELD_IS_AUD_RESET_SKIP);
	if (isResetSkipStr != MLIB_NO_DATA)
	{
		LADataBool tmpIsResetSkip;
		tmpIsResetSkip.convertFromString(isResetSkipStr);
		resetFlg = !tmpIsResetSkip.get();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const int swapSize = swapDataMtx.size();
	// use grid
	LAStringVector swapUseGrid;
	LAString tmpSwapUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + staticDataSuffix).toUpper();
	if (tmpSwapUseGrid != MLIB_NO_DATA)
	{
		swapUseGrid = tmpSwapUseGrid.toToken(':');
	}
	// set tenor adjust
	bool useTenorBasisInstruments = false;
	LAString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + staticDataSuffix).toUpper();
	if (strSwapTenorAdj != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(strSwapTenorAdj);
		useTenorBasisInstruments = tmpAttrB.get();
	}
	for (int j = 0; j < swapSize; ++j)
	{
		LAString term = swapDataMtx[j][0].toUpper();
		LAString tmpCurrency = currency;
		if (useTenorBasis && tmpCurrency.toUpper() == CURRENCY_AUD &&
			term.findString("Y") == static_cast<int>(term.size() - 1) &&
			term.subString(0, term.size() - 2).getIntValue() > 3 && resetFlg)
		{
			// set yield curve pro
			if (marketName != SWAP)
			{
				curveCalibrationData.LAObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
				curveCalibrationData.LAObject::add(CALIBRATION_DATA_MARKETDATA + suffix_data, new LADataMultiReference()).
					convertFromString(staticDataName.subString(0, staticDataName.size() - 2));
			}
			else
			{
				curveCalibrationData.getMarketData().convertFromString(staticDataName.subString(0, staticDataName.size() - 2));
			}

			// generate yield data
			LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
				(curveCalibrationData.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
			curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(marketName));

			dataInstance.getReferencePool().completeDependency();
			modelDataObj.calibrateModel(asOfDate);

			//reset market data as libor
			LACurveMarketDataHelpers::resetMarketDataUsingLibor(curveCalibrationData, currency, &marketName);
			resetFlg = false;
			if (marketName != SWAP)
			{
				curveCalibrationData.LAObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
			}
		}

		double rate = swapDataMtx[j][1].getDoubleValue();
		// get freq
		LAString freqSStr = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY, staticDataSuffix, term).toUpper();
		LAString freqSStr_Fix = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + staticDataSuffix).toUpper();
		// get frequency of floating leg
		LAString baseFreqSStr_float;
		baseFreqSStr_float = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT + staticDataSuffix).toUpper();
		if (baseFreqSStr_float == MLIB_NO_DATA) baseFreqSStr_float = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + staticDataSuffix).toUpper();
		LAString freqSStr_float = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT, staticDataSuffix, term).toUpper();
		// get frequency of compounding
		LAString freqSStr_cpd = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYCOMPOUND + staticDataSuffix).toUpper();
		// get daycount
		LAString daycSStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT, staticDataSuffix, term).toUpper();
		// get daycount of floating leg
		LAString daycSStr_float = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT, staticDataSuffix, term).toUpper();
		// get sliding
		LAString slidingSStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE, staticDataSuffix, term).toUpper();
		// get market rate interpolation
		LAString interpSStr = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_INTERPOLATION + staticDataSuffix);

		if (useTenorBasis && tmpCurrency.toUpper() == CURRENCY_AUD && resetFlg)
		{
			tenorBasisMarketQuotes[marketName].insert(make_pair(term, rate / 100.0));
		}
		LAObject *mktData = NULL;
		LAString nameS = yieldDataName + "_SWAP_" + LAString(j) + suffix_data;
		const LAObjectHolder ehswap = objPool.getObject(nameS);
		if (!ehswap.isDefined())
		{
			mktData = new LAObject();
			objPool.set(nameS, mktData);
		}
		else
		{
			objPool.getObject(nameS).get().clear();
			mktData = &objPool.getObject(nameS).get();
		}
		staticDataName += nameS + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameS);
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateS));
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTLAG, new LADataInt(resetLag));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(calSStr);
		// set daycount
		mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycSStr);
		// set daycount of floating leg
		if (daycSStr_float != MLIB_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, new LAPriceDataDayCount()).convertFromString(daycSStr_float);
		}
		// set data type
		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_PAR);
		// set slidingrule
		mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingSStr);
		// set fwdbasis
		if (areSwapsForwardStarting)
		{
			mktData->add(PRICING_DATA_ISFWDSWAP, new LADataBool(areSwapsForwardStarting));
			if (swapDataMtx[j].size() != 5)
				throw LACoreInvalidData("FwdSwap File format is wrong", __FILE__, __LINE__);
			const bool isDate = swapDataMtx[j][2].toUpper() == "TRUE";
			mktData->add(PRICING_DATA_ISDATE, new LADataBool(isDate));
			if (isDate)
			{
				const LADate startDate = LADate(swapDataMtx[j][3].getCString());
				mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
				const LADate endDate = LADate(swapDataMtx[j][4].getCString());
				mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
			}
			else
			{
				const LAString startTerm = swapDataMtx[j][3].toUpper();
				mktData->add(PRICING_DATA_STARTTERM, new LADataString(startTerm));
				const LAString tenor = swapDataMtx[j][4].toUpper();
				mktData->add(PRICING_DATA_TENOR, new LADataString(tenor));
			}
		}
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));
		// set rate
		mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(rate / 100.0));
		// time inter
		mktData->add(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONSW, new LADataBool(isTimeInterSW));
		// newton raphson
		mktData->add(IR_CALIBRATION_DATA_ISNEWTONRAPHSONSW, new LADataBool(isNRSW));
		// optimize method
		mktData->add(IR_CALIBRATION_DATA_OPTIMIZEMETHOD, new LADataString(optimizeMethod));
		// Swap tenor
		mktData->remove(IR_CALIBRATION_DATA_SWAPTENOR);
		mktData->add(IR_CALIBRATION_DATA_SWAPTENOR, new LADataString(swapTenor));
		// Swap type
		mktData->remove(IR_CALIBRATION_DATA_SWAPTYPE);
		mktData->add(IR_CALIBRATION_DATA_SWAPTYPE, new LADataString(swapType));
		// iseomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLLSW, new LADataBool(isEOMRollSW));
		// simultaneous equation
		mktData->add(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQSW, new LADataBool(isSimuEQSW));
		// set epsilon
		mktData->add(IR_CALIBRATION_DATA_EPSILON, new LADataDouble(eps));
		// set gradient epsilon
		mktData->add(IR_CALIBRATION_DATA_GRADIENTEPSILON, new LADataDouble(grad_eps));
		// set delta
		mktData->add(IR_CALIBRATION_DATA_DELTA, new LADataDouble(delta));
		// set maxloop
		mktData->add(IR_CALIBRATION_DATA_MAXLOOP, new LADataInt(maxLoop));
		// is frequency change
		mktData->add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, new LADataBool(useTenorBasisInstruments));
		// set optimize performance
		mktData->add(IR_CALIBRATION_DATA_OPTIMIZEPERFORMANCE, new LADataBool(optimizePerformance));
		// set fast rebuild
		mktData->add(IR_CALIBRATION_DATA_FASTREBUILD, new LADataBool(fastRebuild));
		// market rate interpolation
		if (interpSStr != MLIB_NO_DATA)
		{
			LAPriceDataInterpolation interpSAtt;
			interpSAtt.convertFromString(interpSStr);
			mktData->add(CALIBRATION_DATA_INTERPOLATION, new LAPriceDataInterpolation(interpSAtt));
		}
		// set fwd interpolation
		mktData->add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, new LADataBool(isFWDInter));
		mktData->add(IR_CALIBRATION_DATA_FWDINTERPOLATION, new LAPriceDataInterpolation()).convertFromString(strFWDInter);
		//grid use
		bool isUse = false;
		if (swapUseGrid.size() != 0 && find(swapUseGrid.begin(), swapUseGrid.end(), term) == swapUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
		}
		else
		{
			isUse = true;
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
		}
		
		// Check and Set Swap Instrument Fixed Frequency
		//*** Note *** Commented out the fixed frequency constraint, which can prohibit valid short dated swaps under 1 year.
		// if ((!checkFrequency(freqSStr, term) || !checkFrequency(freqSStr_Fix, term)) && isUse)
		// {
		// 		MLIB_THROW("Invalid Swap Instrument: Swap tenor '" +  LAString(term) + "' is inconsistent with the swap instrument fixed frequency")
		// }
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freqSStr);
		mktData->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FIX, new LADataString()).convertFromString(freqSStr_Fix);
		
		// Check and Set the Curve Base Float Frequency
		if (baseFreqSStr_float != MLIB_NO_DATA)
		{
			if( !checkFrequency( baseFreqSStr_float, term ) && isUse )
			{
				MLIB_THROW("Invalid Swap Instrument: Swap tenor '" +  LAString(term) + "' is inconsistent with the curve frequency '" +  LAString(baseFreqSStr_float) + "'")
			}
			mktData->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, new LADataString()).convertFromString(baseFreqSStr_float);
		}
		
		// Check and Set the Swap Instrument Float Frequency
		if (freqSStr_float != MLIB_NO_DATA)
		{
			if (!checkFrequency(freqSStr_float, term) && isUse)
			{
				MLIB_THROW("Invalid Swap Instrument: Swap tenor '" +  LAString(term) + "' is inconsistent with the swap instrument float frequency '" +  LAString(freqSStr_float) + "'")
			}
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, new LADataString()).convertFromString(freqSStr_float);
		}

		// Check and Set the Compounding Frequency
		if (freqSStr_cpd != MLIB_NO_DATA)
		{
			if (!checkFrequency(freqSStr_cpd, term) && isUse)
			{
				MLIB_THROW("Invalid Swap Instrument: Swap tenor '" +  LAString(term) + "' is inconsistent with the swap instrument compound frequency '" +  LAString(freqSStr_cpd) + "'")
			}
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_COMPOUND, new LADataString()).convertFromString(freqSStr_cpd);
		}
	}

	if (staticDataName.size() < 2)
	{
		throw LACoreInvalidData("#Error: Missing Market Data. Market Data is not set !!", __FILE__, __LINE__);
	}

	LAString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix);
	if (aliasCurveNames != MLIB_NO_DATA)
	{
		LAStringVector assignedCurves = aliasCurveNames.toToken(':');
		for (size_t i = 0; i < assignedCurves.size(); i++)
		{
			curveCalibrationData.setAssignedCurveMktMap(assignedCurves[i], marketName);
		}
	}
	else
	{
		if (marketName != SWAP)
		{
			curveCalibrationData.setAssignedCurveMktMap(marketName, marketName);
		}
		else
		{
			curveCalibrationData.setAssignedCurveMktMap(STD, marketName);
		}
	}

	// remove curve generate map
	const LAString isPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<LAString, bool>& gCurveMap = curveCalibrationData.getGCurveGenerateMap();
		const std::map<LAString, LAString>& assignedCurveMktMap = curveCalibrationData.getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second == marketName)
			{
				if (gCurveMap.find(it->first) != gCurveMap.end()) gCurveMap.erase(it->first);
			}
		}
	}
}


// 
/*!
	@brief fuction to set up base curve data
*/
void
LAUpdateCurveObject::setUpGenCurveDataOIS(LADataInstance &dataInstance, LAString &staticDataName, const LADate &asOfDate, const LAString &currency,
	const LAString &marketName, const LAString &yieldDataName, bool isSpotStarting,
	bool useTenorBasis, CurveCalibrationData &curveCalibrationData,
	std::map<LAString, std::map<LAString, double> > &tenorBasisMarketQuotes) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	LAString suffixLowerCase = "." + marketName;
	suffixLowerCase.toLower();

	LAString suffix_data = "_" + marketName;

	LAString yeildGenInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffixLowerCase);
	if (yeildGenInter != MLIB_NO_DATA)
	{
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data, new LAPriceDataInterpolation()).convertFromString(yeildGenInter);
	}

	LAString oisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffixLowerCase);
	MAFileAccessor oisFile(LAMarketData::getNumFileName(oisFileName));
	LAStringMatrix oisDataMtx;
	oisFile.readAllData(MARKET_DATA_DELIMITER, oisDataMtx);

	oisFile.close();

	if (oisDataMtx.size() == 0 || oisDataMtx[0].size() < 2)
	{
		MLIB_THROW("Invalid Market Data: OIS Swap Data is missing")
	}

	// get cal and calc spot date
	LAPriceDataCalendar calOIS;
	LAString calOISStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_CALENDAR + suffixLowerCase);
	calOIS.convertFromString(calOISStr);
	LADate spotDateOIS;
	if (isSpotStarting)
	{
		spotDateOIS.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SPOTDATE + suffixLowerCase).getCString());
	}
	else
	{
		spotDateOIS = calOIS.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_RESETLAG + suffixLowerCase).getIntValue());
	}

	bool isEOMRollOIS = false;
	LAString strEOMRollOIS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_ISEOMRLL + suffixLowerCase).toUpper();
	if (strEOMRollOIS != MLIB_NO_DATA)
	{
		LADataBool tmpIsEOMRollOIS;
		tmpIsEOMRollOIS.convertFromString(strEOMRollOIS);
		isEOMRollOIS = tmpIsEOMRollOIS.get();
	}

	if (isEOMRollOIS)
	{
		LAString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_EOMDAY + suffixLowerCase).toUpper();
		if (strEOMDay != MLIB_NO_DATA)
		{
			if (spotDateOIS.dayOfMonth() != strEOMDay.getIntValue())
			{
				isEOMRollOIS = false;
			}
		}
		else
		{
			const LADate eomDate = calOIS.getEOMDay(spotDateOIS);
			if (spotDateOIS != eomDate)
			{
				isEOMRollOIS = false;
			}
		}
	}

	// Linear Spline interpolation cut off date
	LAString inputInterpJoinDateStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_INTERPOLATIONJOINDATE + suffixLowerCase).toUpper();
	curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);

	yldEntity.remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	if (inputInterpJoinDateStr != MLIB_NO_DATA && inputInterpJoinDateStr.size() != 0)
	{
		//LADate inputLinearSplineJoinDate = etrading::stringToDate(inputInterpJoinDateStr, "Invalid joinDate");
		LAString busDayAdj = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE + suffixLowerCase).toUpper();
		LADate inputInterpolationJoinDate = etrading::validateDateOrTenor(spotDateOIS, inputInterpJoinDateStr, busDayAdj, calOISStr, "Invalid joinDate");

		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputInterpolationJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputInterpolationJoinDate));
	}

	// Various OIS control parameters
	LAString str_shortTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERM + suffixLowerCase);
	LADate shortTermDate;
	if (str_shortTerm != MLIB_NO_DATA)
	{
		shortTermDate = calOIS.getBusinessDay(asOfDate, str_shortTerm.getIntValue());
	}

	LAString shortTermConv = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION + suffixLowerCase).toUpper();
	LAString firstRate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FIRSTRATE + suffixLowerCase).toUpper();

	LAString swapCompoundingMethod("");
	LAString compoundValue = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDMETHOD + suffixLowerCase).toUpper();
	LAString compoundValueAlias = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDINGMETHOD + suffixLowerCase).toUpper();

	if (compoundValue != MLIB_NO_DATA && compoundValueAlias != MLIB_NO_DATA)
	{
		MLIB_REQUIRE(compoundValue == compoundValueAlias, "Invalid CompoundMethod: Cannot specify 'CompoundMethod' as '" + compoundValue + "' the alias for this parameter 'CompoundingMethod' as '" + compoundValueAlias + "'")
			swapCompoundingMethod = compoundValueAlias;
	}
	else
	{
		if (compoundValue != MLIB_NO_DATA)
		{
			swapCompoundingMethod = compoundValue;
		}
		else if (compoundValueAlias != MLIB_NO_DATA)
		{
			swapCompoundingMethod = compoundValueAlias;
		}
	}

	// Validate SwapCompounding Method Convention Parameter
	if (swapCompoundingMethod != MLIB_NO_DATA)
	{
		swapCompoundingMethod.toUpper();
		MLIB_REQUIRE(swapCompoundingMethod == "ARITHMETIC" || swapCompoundingMethod == "GEOMETRIC" || swapCompoundingMethod == "NONE" || swapCompoundingMethod == "",
			"Invalid CompoundingMethod - Input was '" + swapCompoundingMethod + "' but must be 'ARITHMETIC', 'GEOMETRIC' or 'NONE' ")
	}

	// Validate shortTermConv Generate Method Parameter by trying to cast to its enumerated type
	if (shortTermConv != MLIB_NO_DATA)
	{
		shortTermConv.toUpper();
		MLIB_REQUIRE(shortTermConv == "ARITHMETIC" || shortTermConv == "ARITHMETICAVERAGE" || shortTermConv == "NONE" || shortTermConv == "",
			"Invalid ShortTermConvention - Input was '" + shortTermConv + "' but must be 'ARITHMETICAVERAGE' or 'NONE'")
	}

	const size_t oisSize = oisDataMtx.size();

	// Curve controls
	bool smoothShortEnd = false;
	LAString strSmoothShortEnd = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SMOOTHSHORTEND + suffixLowerCase);
	if (strSmoothShortEnd != MLIB_NO_DATA)
	{
		LADataBool tmpSmoothShortEnd;
		tmpSmoothShortEnd.convertFromString(strSmoothShortEnd);
		smoothShortEnd = tmpSmoothShortEnd.get();
	}

	bool shortTermSwapOverrules = false;
	LAString strShortTermSwapOverrules = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMSWAPOVERRULES + suffixLowerCase);
	if (strShortTermSwapOverrules != MLIB_NO_DATA)
	{
		LADataBool tmpShortTermSwapOverrules;
		tmpShortTermSwapOverrules.convertFromString(strShortTermSwapOverrules);
		shortTermSwapOverrules = tmpShortTermSwapOverrules.get();
	}

	// use grid
	LAStringVector oisUseGrid;
	LAString tmpOISUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffixLowerCase).toUpper();
	if (tmpOISUseGrid != MLIB_NO_DATA)
	{
		oisUseGrid = tmpOISUseGrid.toToken(':');
	}

	// doing dual bootstrapping?
	bool isDualBootstrapping = false;
	LAString strIsDualBootstrapping = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_ISDUALBOOTSTRAPPING + suffixLowerCase);
	if (strIsDualBootstrapping != MLIB_NO_DATA)
	{
		LADataBool tmpIsDualBootstrapping;
		tmpIsDualBootstrapping.convertFromString(strIsDualBootstrapping);
		isDualBootstrapping = tmpIsDualBootstrapping.get();
	}

	// Validate LongTerm Convention Parameter
	LAString longTermConvStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffixLowerCase).toUpper();

	etrading::OISLongTermInstrumentsEnum longTermConv = (longTermConvStr != MLIB_NO_DATA) ? etrading::toOISLongTermInstrumentsEnum(longTermConvStr.getCString()) : etrading::NONE_OIS_LONGTERM_INSTRUMENTS;

	LAString longTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffixLowerCase).toUpper();

	LADate date_lt;
	LAStringMatrix lobasisDataMtx, swapDataMtx;

	// LongTermGenerate Methodolgy Defaults are managed within the in calcEffectiveOISRate method, see CurveCalibration.cpp
	LAString longTermGenStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD + suffixLowerCase).toUpper();
	LAString longTermGenArrOisStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD_ARROIS + suffixLowerCase).toUpper();

	etrading::OISLongTermCompoundingEnum longTermGen = (longTermGenStr != MLIB_NO_DATA) ? etrading::toOISLongTermCompoundingEnum(longTermGenStr.getCString()) : etrading::NONE_OIS_LONGTERM_COMPOUNDING;
	etrading::OISLongTermCompoundingEnum longTermGenArrOis = (longTermGenArrOisStr != MLIB_NO_DATA) ? etrading::toOISLongTermCompoundingEnum(longTermGenArrOisStr.getCString()) : etrading::NONE_OIS_LONGTERM_COMPOUNDING;

	if (longTermConv == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
	{
		if (longTerm == MLIB_NO_DATA)
		{
			throw LACoreInvalidData("#Error: LongTerm parameter is needed, when the LongTermConvention 'LOBASIS' is specified.", __FILE__, __LINE__);
		}

		date_lt = etrading::LADateHelpers::getDate(asOfDate, longTerm, true);
		LAString lobasisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + suffixLowerCase);
		MAFileAccessor lobasisFile(LAMarketData::getNumFileName(lobasisFileName));
		lobasisFile.readAllData(MARKET_DATA_DELIMITER, lobasisDataMtx);
		lobasisFile.close();
		if (lobasisDataMtx.size() == 0 || lobasisDataMtx[0].size() < 2)
		{
			MLIB_THROW("Missing Libor-OIS Basis Market Data")
		}
		LAString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffixLowerCase);
		if (swapFileName != MLIB_NO_DATA)
		{
			MAFileAccessor swapFile(LAMarketData::getNumFileName(swapFileName));
			swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
			swapFile.close();
			if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2)
			{
				if (!isDualBootstrapping)
				{
					MLIB_THROW("Invalid Market Data: Missing Swap Market Data")
				}
			}

			if (swapDataMtx.size() != lobasisDataMtx.size())
			{
				if (!isDualBootstrapping)
				{
					MLIB_THROW("Invalid Market Data: Inconsistent number of LOBasis spreads and Libor swaps provided; LOBASIS Instruments: " + MLIB_TO_STRING_FROM_SIZE_T(lobasisDataMtx.size()) +  " vs SWAP Instruments: " + MLIB_TO_STRING_FROM_SIZE_T(swapDataMtx.size()) )
				}
			}
		}
	}

	//get constant for convergence
	double epsilon = 1.0e-9;
	LAString strEPS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_EPSILON + suffixLowerCase);
	if (strEPS.toUpper() != MLIB_NO_DATA)
	{
		epsilon = strEPS.getDoubleValue();
	}
	int maxLoop = 1000;
	LAString strMLP = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_MAXLOOP + suffixLowerCase);
	if (strMLP.toUpper() != MLIB_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}

	// Consolidate the outright OIS swap instruments and basis swap + libor swap instruments
	size_t extraBasisCount = 0;
	size_t longTermStartIndex = 0;
	if (longTermConv == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
	{
		LAString lastOisTerm = oisDataMtx[oisSize - 1][0].toUpper();

		// When the outright swap tenors overlap with the basis tenors, if the max basis tenor is larger
		// than the max outright swap tenor, find those tenors that are larger than the max outright swap
		// tenor and concatenate them to the outright swap tenors
		bool isOverlapped = false;
		for (size_t i = 0; i < lobasisDataMtx.size(); ++i)
		{
			LAString term = lobasisDataMtx[i][0].toUpper();
			if (term == lastOisTerm)
			{
				isOverlapped = true;
				if (i < lobasisDataMtx.size() - 1)
				{
					longTermStartIndex = i + 1;
					extraBasisCount = lobasisDataMtx.size() - longTermStartIndex;
					break;
				}
			}
		}

		// When the outright swap tenors do not overlap with the basis tenors, find out if the 'longTerm' 
		// tenor is present in the basis tenors. If yes, concatenate the outright swap tenors with all basis
		// tenors that are larger than the 'longTerm' tenor.
		if (!isOverlapped)
		{
			longTermStartIndex = 0;
			extraBasisCount = 0;
			for (size_t i = 0; i < lobasisDataMtx.size(); ++i)
			{
				LAString term = lobasisDataMtx[i][0].toUpper();
				const LADate date = etrading::LADateHelpers::getDate(asOfDate, term, true);

				if (date >= date_lt)
				{
					longTermStartIndex = i;
					extraBasisCount = lobasisDataMtx.size() - longTermStartIndex;
					break;
				}
			}
		}
	}

	// *** Convexity Quote Type ***
	// ----------------------------------------------------------------

	// *** replaced by convexityQuotedAsVol***
	// get UseConvexAdjustment - Legacy parameter for ConvexityQuotedAsPrice, replaced by ConvexityQuoteType
	bool useConvexAdjustment = false;
	bool convexityQuotedAsVol = true;
	LAString useConvexAdjustmentStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USECONVEXADJUSTMENT + suffixLowerCase).toUpper();
	if (useConvexAdjustmentStr != MLIB_NO_DATA)
	{
		useConvexAdjustment = (useConvexAdjustmentStr == "TRUE");
		convexityQuotedAsVol = !useConvexAdjustment;
	}

	// get ConvexityQuoteType *** An alias for UseConvexAdjustment***
	LAString convexityQuoteTypeStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_CONVEXITYQUOTETYPE + suffixLowerCase).toUpper();
	if (convexityQuoteTypeStr != MLIB_NO_DATA)
	{
		convexityQuoteTypeStr.toUpper();
		MLIB_REQUIRE( convexityQuoteTypeStr == "VOL" || convexityQuoteTypeStr == "PRICE", "Invalid Futures Convexity Quote Type: ConvexityQuoteType must be VOL or PRICE" )
		convexityQuotedAsVol = ( convexityQuoteTypeStr == "VOL" ) ? true : false;

		// TODO: Property Manager does not allow us to clear parameters once set ... hence the below fails ... prioritize the convexityQuoteType parameter for now
		// if ( useConvexAdjustmentStr != MLIB_NO_DATA )
		// {
		// 	MLIB_THROW("Invalid Futures Convexity Parameter: Cannot use ConvexityQuoteType and UseConvexAdjustment (ConvexityQuotedAsPrice) parameters at the same time.")
		// }
	}

	// Important Update ConvexityQuoteString for Object Pool Setting
	convexityQuoteTypeStr = convexityQuotedAsVol ? "VOL" : "PRICE";

	// ----------------------------------------------------------------

	// get meanReversion for future section
	double meanReversion = 0.0;
	std::string meanReversion_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_MEANREVERSION + suffixLowerCase).toUpper().getCString();
	if (meanReversion_str != MLIB_NO_DATA)
	{
		meanReversion = std::stod(meanReversion_str);
	}

	size_t currentLOBasisInstrument = 0;
	size_t currentSwapInstrument = 0;

	size_t lobasisIndex = 0;
	for (size_t i = 0; i < oisSize + extraBasisCount; ++i)
	{
		LAObject *mktData = NULL;
		LAString nameOIS = yieldDataName + "_OIS_" + LAString((int)i) + "_" + marketName;
		const LAObjectHolder ehois = objPool.getObject(nameOIS);
		if (!ehois.isDefined())
		{
			mktData = new LAObject();
			objPool.set(nameOIS, mktData);
		}
		else
		{
			objPool.getObject(nameOIS).get().clear();
			mktData = &objPool.getObject(nameOIS).get();
		}

		//Only attach the name when it's not already there
		if (staticDataName.findString(nameOIS) < 0)
		{
			staticDataName += nameOIS + ":";
		}

		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameOIS);

		// Always get term tenor from the OIS outright swap table first. If the OIS outright swap table
		// is not long enough, get the subsequent term tenors from the LOBASIS table
		LAString term("");
		double rate = 0.0;
		if (i < oisSize)
		{
			term = oisDataMtx[i][0].toUpper();
			rate = oisDataMtx[i][1].getDoubleValue();
		}
		else
		{
			term = lobasisDataMtx[longTermStartIndex + lobasisIndex][0];
			++lobasisIndex;
		}

		// Allow Central Bank Swap instruments to be specified in the OIS curve for all listOfCurvesBuilt ...
		if (etrading::isCentralBankSwap(term))
		{
			if (oisDataMtx[i].size() != 4)
			{
				throw LACoreInvalidData("#Error: OIS Market Data must contain 4 columns. OIS Market Data column size is incorrect.", __FILE__, __LINE__);
			}
			// boj type
			// ** Currently all Central Bank swaps are labelled as BOJ regardless of currency. **
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BOJ);
			LADate startDate = LADataDate(oisDataMtx[i][2]).get();
			LADate endDate = LADataDate(oisDataMtx[i][3]).get();
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));

			//When the startDate is in the past and endDate is in the future, use Historical data in the FixingTable
			if (startDate < asOfDate && endDate > asOfDate)
			{
				LAString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffixLowerCase);
				etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
			}

		}
		else if (etrading::isFuture(term))
		{
			const int NUM_COLUMNS_FUTURE = 5; // Term, Rate, StartDate, EndDate, VolOrConvAdj
			if (oisDataMtx[i].size() != NUM_COLUMNS_FUTURE)
			{
				throw LACoreInvalidData("#Error: OIS Market Data must contain 4 columns. OIS Market Data column size is incorrect.", __FILE__, __LINE__);
			}

			// ARR Future type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_ARR_FUTURE);

			// set startDate, endDate
			LADate startDate = etrading::stringToDate(oisDataMtx[i][2]);
			LADate endDate = etrading::stringToDate(oisDataMtx[i][3]);
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));

			// Futures Convexity Quote
			double futureVol(0.0), convexAdj(0.0);
			if (convexityQuotedAsVol)
			{
				// Futures Convexity Quoted as a Volatility
				futureVol = oisDataMtx[i][4].getDoubleValue();
			}
			else
			{
				// Futures Convexity Quoted as a Price
				convexAdj = oisDataMtx[i][4].getDoubleValue();
			}
			
			// *** FUTURES CONVEXITY ADJUSTMENTS ***
			// -------------------------------------

			// set convexityQuoteType, which is the alias for legacy parameter useConvexAdj (convexity quoted as price)
			mktData->add(PRICING_DATA_CONVEXITYQUOTETYPE, new LADataString(convexityQuoteTypeStr));
			
			// set convexAdj ... futures convexity adjustment when quoted as a price
			mktData->add(PRICING_DATA_CONVEXADJUSTMENT, new LADataDouble(convexAdj));
			
			// set vol ... futures convexity adjustment when quoted as a volatility
			mktData->add(PRICING_DATA_FUTUREVOLATILITY, new LADataDouble(futureVol));

			// -------------------------------------

			//When the startDate is in the past and endDate is in the future, use Historical data in the FixingTable
			if (startDate < asOfDate && endDate > asOfDate)
			{
				LAString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffixLowerCase);
				etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
			}
		}
		else
		{
			// swap type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_PAR);
		}

		// get freq
		LAString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY, suffixLowerCase, term).toUpper();
		// get daycount
		LAString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT, suffixLowerCase, term).toUpper();
		// get sliding
		LAString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE, suffixLowerCase, term).toUpper();
		// get swap type
		LAString swapType("OIS");
		LAString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SWAPTYPE + suffixLowerCase).toUpper();
		if (strSwapType != MLIB_NO_DATA)
		{
			swapType = strSwapType;
		}
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateOIS));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(calOISStr);
		// set sliding
		mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycOISStr);
		// set daycount
		mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingOISStr);
		// set frequency
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freqOISStr);
		// set short term rate convention
		mktData->add(IR_CALIBRATION_DATA_SHORTTERMCONVENTION, new LADataString()).convertFromString(shortTermConv);
		// set first market
		mktData->add(IR_CALIBRATION_DATA_FIRSTRATE, new LADataString()).convertFromString(firstRate);
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));
		// set swap averaging method
		mktData->add(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, new LADataString()).convertFromString(swapCompoundingMethod);

		auto datatype = dynamic_cast<const LADataString&> ((mktData->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();

		if (datatype == YIELD_TYPE_ARR_FUTURE)
		{
			// 1) transform future price to rate
			rate = 1.0 - rate / 100.0;

			LAPriceDataDayCount dc;
			dc.convertFromString(daycOISStr);

			// 2) add convexity adjustment to the future rate
			rate = etrading::getConvexityAdjustedFutureRate(mktData, rate, asOfDate, meanReversion, dc);

			// 3) set convexity adjusted forward rate
			mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(rate));
		}
		else
		{
			// set rate
			mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(rate / 100.0));
		}

		// iseomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLLOIS, new LADataBool(isEOMRollOIS));
		// set short term date
		if (str_shortTerm != MLIB_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_SHORTTERMDATE, new LADataDate(shortTermDate));
		}
		//grid use
		mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
		// set epsilon
		mktData->add(IR_CALIBRATION_DATA_EPSILON, new LADataDouble(epsilon));
		// set maxloop
		mktData->add(IR_CALIBRATION_DATA_MAXLOOP, new LADataInt(maxLoop));
		// set swap type
		mktData->add(IR_CALIBRATION_DATA_SWAPTYPE, new LADataString(swapType));
		// short term swap overrules
		mktData->add(IR_CALIBRATION_DATA_SHORTTERMSWAPOVERRULES, new LADataBool(shortTermSwapOverrules));
		// smooth short end of curve
		mktData->add(IR_CALIBRATION_DATA_SMOOTHSHORTEND, new LADataBool(smoothShortEnd));

		if (longTermConv == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
		{
			// Allow Central Bank Swap instruments to be specified in the OIS curve for all listOfCurvesBuilt ...
			if (etrading::isCentralBankSwap(term) || etrading::isFuture(term)) 
			{
				continue;
			}

			// Skip tenors whose end date is below the 'longTermTenor'
			const LADate date = etrading::LADateHelpers::getDate(asOfDate, term, true);
			if (date < date_lt)
			{
				continue;
			}

			// Look up on the lobasis table and find the basis instrument with the same term tenor
			while (currentLOBasisInstrument < lobasisDataMtx.size())
			{
				const LAString term_lo = lobasisDataMtx[currentLOBasisInstrument][0].toUpper();
				if (term_lo == term)
				{
					break;
				}
				++currentLOBasisInstrument;
			}

			// Not Found Condition
			if (currentLOBasisInstrument >= lobasisDataMtx.size())
			{
				MLIB_THROW("Unable to imply OIS Outright Swaps from Libor-OIS Basis Instruments: Missing Libor-OIS Basis Swap with tenor " + term + ". Note: Overlapping OIS and Libor-OIS instruments must have identical tenors.");
			}

			// Look up on the libor swap table and find the libor swap instrument with the same term tenor
			while (currentSwapInstrument < swapDataMtx.size())
			{
				const LAString term_s = swapDataMtx[currentSwapInstrument][0].toUpper();
				if (term_s == term)
				{
					break;
				}
				++currentSwapInstrument;
			}

			bool isSwapRateAvailable = true;

			// Not Found Condition
			if (currentSwapInstrument >= swapDataMtx.size())
			{
				isSwapRateAvailable = false;
				currentSwapInstrument = 0;

				if (!isDualBootstrapping)
				{
					MLIB_THROW("Unable to imply OIS Outright Swaps from Libor-OIS Basis Instruments: Missing Libor Swap with tenor'" + term + "' to match the corresponding Libor-OIS Basis Swap");
				}
			}

			//If the basisSwapType column is specified, use it, otherwise default the basisSwapType to the LIBOR
			etrading::OISBasisInstrumentSwapTypeEnum basisSwapType = etrading::LIBOR_OISBASIS_INSTRUMENT_SWAP_TYPE;
			if (lobasisDataMtx.size() > 0 && lobasisDataMtx[currentSwapInstrument].size() > 2)
			{
				basisSwapType = etrading::toOISBasisInstrumentSwapTypeEnum(lobasisDataMtx[currentSwapInstrument][2].getCString());
			}

			// get lobasis data
			double rate_lo = lobasisDataMtx[currentLOBasisInstrument][1].getDoubleValue();
			
			LAString calLOStr(""), freqLOStr(""), daycLOStr(""), slidingLOStr("");
			if (basisSwapType == etrading::OIS_OISBASIS_INSTRUMENT_SWAP_TYPE)
			{
				calLOStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_CALENDAR_ARROIS + suffixLowerCase);
				freqLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY_ARROIS, suffixLowerCase, term).toUpper();
				daycLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT_ARROIS, suffixLowerCase, term).toUpper();
				slidingLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE_ARROIS, suffixLowerCase, term).toUpper();

				// if it's ARR-OIS Spread, ARRrate = OISrate + spread, so spread need to be negative
				rate_lo *= (-1.0);
			}
			else
			{
				calLOStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_CALENDAR + suffixLowerCase);
				freqLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY, suffixLowerCase, term).toUpper();
				daycLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT, suffixLowerCase, term).toUpper();
				slidingLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE, suffixLowerCase, term).toUpper();
			}


			// get swap data, use fixed leg's frequency and day count			
			LAString calSwapStr(""), freqSwapStr(""), daycSwapStr(""), slidingSwapStr(""), swapTenor(""), swapType("LIBOR");
			if (basisSwapType == etrading::OIS_OISBASIS_INSTRUMENT_SWAP_TYPE)
			{
				calSwapStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR_ARROIS + suffixLowerCase);
				freqSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFIX_ARROIS, suffixLowerCase, term).toUpper();
				daycSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFIX_ARROIS, suffixLowerCase, term).toUpper();
				slidingSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE_ARROIS, suffixLowerCase, term).toUpper();
			}
			else
			{
				calSwapStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + suffixLowerCase);
				freqSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFIX, suffixLowerCase, term).toUpper();
				daycSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFIX, suffixLowerCase, term).toUpper();
				slidingSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE, suffixLowerCase, term).toUpper();

				LAString strSwapTenor = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTENOR + suffixLowerCase).toUpper();
				if (strSwapTenor != MLIB_NO_DATA)
				{
					swapTenor = strSwapTenor;
				}

				LAString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTYPE + suffixLowerCase).toUpper();
				if (strSwapType != MLIB_NO_DATA)
				{
					swapType = strSwapType;
				}

			}

			// set long term rate convention
			mktData->add(IR_CALIBRATION_DATA_LONGTERMCONVENTION, new LADataString()).convertFromString(toString(longTermConv));
			mktData->add(IR_CALIBRATION_DATA_LONGTERM, new LADataString()).convertFromString(longTerm);
						
			if (basisSwapType == etrading::OIS_OISBASIS_INSTRUMENT_SWAP_TYPE)
			{
				mktData->add(IR_CALIBRATION_DATA_LONGTERMGENMETHOD, new LADataString()).convertFromString(toString(longTermGenArrOis));
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_LONGTERMGENMETHOD, new LADataString()).convertFromString(toString(longTermGen));
			}

			// set lobasis data
			mktData->add(IR_CALIBRATION_DATA_RATE_LOBASIS, new LADataDouble(rate_lo / 100.0));
			if (calLOStr == MLIB_NO_DATA) calLOStr = calOISStr;
			mktData->add(IR_CALIBRATION_DATA_CALENDAR_LOBASIS, new LAPriceDataCalendar()).convertFromString(calLOStr);
			if (daycLOStr == MLIB_NO_DATA) daycLOStr = daycOISStr;
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_LOBASIS, new LAPriceDataDayCount()).convertFromString(daycLOStr);
			if (slidingLOStr == MLIB_NO_DATA) slidingLOStr = slidingOISStr;
			mktData->add(IR_CALIBRATION_DATA_SLIDINGRULE_LOBASIS, new LAPriceDataSlidingRule()).convertFromString(slidingLOStr);
			if (freqLOStr == MLIB_NO_DATA) freqLOStr = freqOISStr;
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_LOBASIS, new LADataString()).convertFromString(freqLOStr);

			// set swap data
			if (isSwapRateAvailable)
			{
				double rate_s = swapDataMtx[currentSwapInstrument][1].getDoubleValue();
				mktData->add(IR_CALIBRATION_DATA_RATE_SWAP, new LADataDouble(rate_s / 100.0));
			}
			mktData->add(IR_CALIBRATION_DATA_CALENDAR_SWAP, new LAPriceDataCalendar()).convertFromString(calSwapStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_SWAP, new LAPriceDataDayCount()).convertFromString(daycSwapStr);
			mktData->add(IR_CALIBRATION_DATA_SLIDINGRULE_SWAP, new LAPriceDataSlidingRule()).convertFromString(slidingSwapStr);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_SWAP, new LADataString()).convertFromString(freqSwapStr);

			mktData->remove(IR_CALIBRATION_DATA_REFSWAPTENOR);
			mktData->remove(IR_CALIBRATION_DATA_REFSWAPTYPE);
			mktData->add(IR_CALIBRATION_DATA_REFSWAPTENOR, new LADataString()).convertFromString(swapTenor);
			mktData->add(IR_CALIBRATION_DATA_REFSWAPTYPE, new LADataString()).convertFromString(swapType);
		}
	}

	LAStringMatrix fedFundFutureDataMtx;
	LAString fedFundFutureFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + suffixLowerCase);
	if (fedFundFutureFileName != MLIB_NO_DATA)
	{
		MAFileAccessor fedFundFutureFile(LAMarketData::getNumFileName(fedFundFutureFileName));
		fedFundFutureFile.readAllData(MARKET_DATA_DELIMITER, fedFundFutureDataMtx);
		fedFundFutureFile.close();
	}

	const int fedFundFutureSize = fedFundFutureDataMtx.size();
	// use grid
	LAStringVector ffFutureUseGrid;
	LAString tmpFFFutureUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID + suffixLowerCase).toUpper();
	if (tmpFFFutureUseGrid != MLIB_NO_DATA)
	{
		ffFutureUseGrid = tmpFFFutureUseGrid.toToken(':');
	}
	else
	{
		LAString tmpUseGridNum = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRIDNUM + suffixLowerCase).toUpper();
		if (tmpUseGridNum != MLIB_NO_DATA)
		{
			int useGridNum = tmpUseGridNum.getIntValue();
			if (useGridNum == 0)
			{
				ffFutureUseGrid.push_back(MLIB_NO_DATA);
			}
			else
			{
				for (size_t i = 0; i < (size_t)useGridNum; ++i)
				{
					if (fedFundFutureDataMtx.size() <= i) break;
					ffFutureUseGrid.push_back(fedFundFutureDataMtx[i][0]);
				}
			}
		}
	}

	for (int i = 0; i < fedFundFutureSize; ++i)
	{
		LAObject *mktData = NULL;
		LAString nameOIS = yieldDataName + "_OIS_" + LAString((int)oisSize + i) + "_" + marketName;
		const LAObjectHolder ehois = objPool.getObject(nameOIS);
		if (!ehois.isDefined())
		{
			mktData = new LAObject();
			objPool.set(nameOIS, mktData);
		}
		else
		{
			objPool.getObject(nameOIS).get().clear();
			mktData = &objPool.getObject(nameOIS).get();
		}

		//Only attach the name when it's not already there
		if (staticDataName.findString(nameOIS) < 0)
		{
			staticDataName += nameOIS + ":";
		}

		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameOIS);

		LADate startDate, endDate;
		LAString term = fedFundFutureDataMtx[i][0].toUpper();

		if (fedFundFutureDataMtx[i].size() < 2)
		{
			throw LACoreInvalidData("#Error: FF Futures data cannot contain more than 2 columns. FF Future File format is wrong", __FILE__, __LINE__);
		}

		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_FF);
		if (fedFundFutureDataMtx[i].size() == 4)
		{
			startDate = LADataDate(fedFundFutureDataMtx[i][2]).get();
			endDate = LADataDate(fedFundFutureDataMtx[i][3]).get();
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
		}
		else //FF non startdate type
		{
			LAString term = fedFundFutureDataMtx[i][0].toUpper();
			DateVector ffdates = etrading::LADateHelpers::getFFDatesFromTerm(asOfDate, term);
			if (ffdates.size() != 2)
				throw LACoreInvalidData("#Error: FF Dates Data must contain 2 columns. FF dates error", __FILE__, __LINE__);

			startDate = ffdates[0];
			endDate = ffdates[1];
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
		}

		double rate = fedFundFutureDataMtx[i][1].getDoubleValue();
		rate = 100. - rate;

		// get freq
		LAString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY, suffixLowerCase, term).toUpper();
		// get daycount
		LAString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT, suffixLowerCase, term).toUpper();
		// get sliding
		LAString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE, suffixLowerCase, term).toUpper();
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateOIS));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(calOISStr);
		// set sliding
		mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycOISStr);
		// set daycount
		mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingOISStr);
		// set frequency
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freqOISStr);
		// set short term rate convention
		mktData->add(IR_CALIBRATION_DATA_SHORTTERMCONVENTION, new LADataString()).convertFromString(shortTermConv);
		// set first market
		mktData->add(IR_CALIBRATION_DATA_FIRSTRATE, new LADataString()).convertFromString(firstRate);
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));
		// set rate
		mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(rate / 100.0));
		// iseomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLLOIS, new LADataBool(isEOMRollOIS));
		// set short term date
		if (str_shortTerm != MLIB_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_SHORTTERMDATE, new LADataDate(shortTermDate));
		}
		//grid use
		if (ffFutureUseGrid.size() != 0 && find(ffFutureUseGrid.begin(), ffFutureUseGrid.end(), term) == ffFutureUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
		}
		else
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
		}


		if (startDate < asOfDate && dynamic_cast<LADataBool &>(mktData->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, ISNOTNULL).get()).get())
		{
			LAString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffixLowerCase);

			etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
		}

	}

	if (staticDataName.size() < 2)
	{
		throw LACoreInvalidData("Market Data is not set !!", __FILE__, __LINE__);
	}

	//DF curve name
	LAString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffixLowerCase);
	if (dfCurveName == MLIB_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName);
	curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName, new LADataString(dfCurveName));
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName, new LADataString(dfCurveName));

	//const std::map<LAString, LAString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
	LAString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffixLowerCase);
	if (aliasCurveNames != MLIB_NO_DATA)
	{
		LAStringVector assignedCurves = aliasCurveNames.toToken(':');
		for (size_t i = 0; i < assignedCurves.size(); i++)
		{
			curveCalibrationData.setAssignedCurveMktMap(assignedCurves[i], marketName);
		}
	}
	else
	{
		curveCalibrationData.setAssignedCurveMktMap(marketName, marketName);
	}

	// remove curve generate map
	const LAString isPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<LAString, bool>& gCurveMap = curveCalibrationData.getGCurveGenerateMap();
		const std::map<LAString, LAString>& assignedCurveMktMap = curveCalibrationData.getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second == marketName)
			{
				if (gCurveMap.find(it->first) != gCurveMap.end()) gCurveMap.erase(it->first);
			}
		}
	}
}

void
LAUpdateCurveObject::setUpFloater(const LAString &currency, CurveCalibrationData &curveCalibrationData, const LAString &genFloaterName) const
{
	LAStringVector listOfCurvesBuilt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	uppervec(listOfCurvesBuilt);
	if (genFloaterName != MLIB_NO_DATA)
	{
		curveCalibrationData.setAssignedCurveMktMap(genFloaterName, genFloaterName);
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_FLOATERDFS);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_FLOATERDFS, new LADataString(genFloaterName));
		LAString tmpGenFloaterName = genFloaterName;
		tmpGenFloaterName.toLower();
		LAString basisMkt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME);
		if (basisMkt != MLIB_NO_DATA)
		{
			if (std::find(listOfCurvesBuilt.begin(), listOfCurvesBuilt.end(), basisMkt) == listOfCurvesBuilt.end())
			{
				throw LACoreInvalidData("Basis market does not exist!", __FILE__, __LINE__);
			}
			curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_BASISDATA + LAString("_") + tmpGenFloaterName);
			curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_BASISDATA + LAString("_") + tmpGenFloaterName, new LADataString(basisMkt));
		}
		LAString discountName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT);
		LAString forecastName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FORECAST);
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_FORECAST + LAString("_") + tmpGenFloaterName);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_FORECAST + LAString("_") + tmpGenFloaterName, new LADataString(forecastName));
		curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_DISCOUNT + LAString("_") + tmpGenFloaterName);
		curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_DISCOUNT + LAString("_") + tmpGenFloaterName, new LADataString(discountName));

		/*LAString isFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_ISFWDINTER).toUpper();
		if (isFWDInter == "TRUE")
		{
			curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + LAString("_") + tmpGenFloaterName);
			curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + LAString("_") + tmpGenFloaterName, new LADataBool(true));
			LAString fwdInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FWDINTERPOLATION).toLower();
			curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_FWDINTERPOLATION + LAString("_") + tmpGenFloaterName);
			curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_FWDINTERPOLATION + LAString("_") + tmpGenFloaterName, new LAPriceDataInterpolation()).convertFromString(fwdInter);;
		}*/

		curveCalibrationData.setFloater(genFloaterName);
	}
}

void
LAUpdateCurveObject::setUpCurveDataByReadFile(LADataInstance &dataInstance, const LADate& asOfDate, const LAString& currency,
	const LAString& marketName, const LAString& yieldDataName, CurveCalibrationData &curveCalibrationData) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAObjectHolder objHolder = objPool.getObject(yieldDataName, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
		throw LACoreInvalidData("yield Object is not set! LAUpdateCurveObject::setUpCurveDataByReadFile", __FILE__, __LINE__);

	LAObject &objectPool = objHolder.get();

	LAString thisMarketName = marketName;
	LAString suffixLowerCase = "." + thisMarketName.toLower();

	LAString dfFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_DF_FILE + suffixLowerCase);
	MAFileAccessor dfFile(LAMarketData::getNumFileName(dfFileName));
	LAStringMatrix dfDataMtx;
	dfFile.readAllData(MARKET_DATA_DELIMITER, dfDataMtx);
	dfFile.close();

	if ( dfDataMtx.size() == 0 || dfDataMtx[0].size() < 2 )
	{
		MLIB_THROW("Invalid Data: Discount Factor data is missing")
	}

	DoubleArray terms;
	DoubleArray dfs;
	DoubleArray dfs2;

	for (size_t i = 0; i < dfDataMtx.size(); i++)
	{
		if (dfDataMtx[i].size() == 3)
		{
			terms.push_back(dfDataMtx[i][0].getDoubleValue());
			dfs.push_back(dfDataMtx[i][1].getDoubleValue());
			dfs2.push_back(dfDataMtx[i][2].getDoubleValue());
		}
		else
		{
			terms.push_back(dfDataMtx[i][0].getDoubleValue());
			dfs.push_back(dfDataMtx[i][1].getDoubleValue());
		}
	}

	if (!terms.empty() && terms[0] != 0.0)
	{
		terms.insert(terms.begin(), 0.0);
		dfs.insert(dfs.begin(), 1.0);
		if (!dfs2.empty())
			dfs2.insert(dfs2.begin(), 1.0);
	}

	LAString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ZERORATE_ASSIGNEDCURVE + suffixLowerCase);
	if (aliasCurveNames != MLIB_NO_DATA)
	{
		LAStringVector assignedCurves = aliasCurveNames.toToken(':');
		for (size_t i = 0; i < assignedCurves.size(); i++)
		{
			curveCalibrationData.setAssignedCurveMktMap(assignedCurves[i], marketName);

			LAString modCurveName = "_" + assignedCurves[i].toUpper();

			objectPool.remove(CALIBRATION_DATA_TERMS + modCurveName);
			objectPool.add(CALIBRATION_DATA_TERMS + modCurveName, new LADataDoubles(terms));

			objectPool.remove(IR_CALIBRATION_DATA_DFS + modCurveName);
			objectPool.add(IR_CALIBRATION_DATA_DFS + modCurveName, new LADataDoubles(dfs));

			objectPool.remove(IR_CALIBRATION_DATA_ACCESSARY + modCurveName);
			objectPool.add(IR_CALIBRATION_DATA_ACCESSARY + modCurveName, new LADataString("DF"));

			if (!dfs2.empty())
			{
				objectPool.remove(IR_CALIBRATION_DATA_DFS2 + modCurveName);
				objectPool.add(IR_CALIBRATION_DATA_DFS2 + modCurveName, new LADataDoubles(dfs2));
			}
		}
	}
	else
	{
		LAString modCurveName = "_" + thisMarketName.toUpper();

		objectPool.remove(CALIBRATION_DATA_TERMS + modCurveName);
		objectPool.add(CALIBRATION_DATA_TERMS + modCurveName, new LADataDoubles(terms));

		objectPool.remove(IR_CALIBRATION_DATA_DFS + modCurveName);
		objectPool.add(IR_CALIBRATION_DATA_DFS + modCurveName, new LADataDoubles(dfs));

		objectPool.remove(IR_CALIBRATION_DATA_ACCESSARY + modCurveName);
		objectPool.add(IR_CALIBRATION_DATA_ACCESSARY + modCurveName, new LADataString("DF"));

		if (!dfs2.empty())
		{
			objectPool.remove(IR_CALIBRATION_DATA_DFS2 + modCurveName);
			objectPool.add(IR_CALIBRATION_DATA_DFS2 + modCurveName, new LADataDoubles(dfs2));
		}
		curveCalibrationData.setAssignedCurveMktMap(marketName, marketName);
	}

	curveCalibrationData.insertNonRemovableMarket(marketName);
}

void
LAUpdateCurveObject::dataoutCurve(const LAStringVector &curveNames, LAObject &objectPool, const LAString &yieldDataName) const
{
	for (unsigned int i = 0; i < curveNames.size(); i++)
	{
		LAString curveSuffix;
		if (curveNames[i] == STD) curveSuffix = "";
		else curveSuffix = "_" + curveNames[i];
		LAString curveSuffix_file = curveSuffix;
		while (curveSuffix_file.findString("/") != -1)
		{
			curveSuffix_file.remove(curveSuffix_file.findString("/"), 1);
		}
		const LAString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM);
		const LAString dirName = LACoreDataService::getOutputDirectory();
		const LAString fileName = dirName + yieldDataName + curveSuffix_file + fileSuffix + ".csv";

		ifstream fin;
		ofstream fout;
		fin.open(fileName.getCString());

		if (!fin)
		{
			LADataHolder* dh = &objectPool.getData(CALIBRATION_DATA_TERMS + curveSuffix, NOCHECK);
			if (!dh->isDefined() || dh->isNull())
				continue;

			const DoubleArray &terms =
				dynamic_cast<const LADataDoubles &>(objectPool.getData(CALIBRATION_DATA_TERMS + curveSuffix, ISNOTNULL).get()).get();
			const DoubleArray &dfs =
				dynamic_cast<const LADataDoubles &>(objectPool.getData(IR_CALIBRATION_DATA_DFS + curveSuffix, ISNOTNULL).get()).get();

			int size = terms.size();
			if (size != static_cast<int>(dfs.size()))
			{
				throw LACoreInvalidData("Term size and df size must be same !!", __FILE__, __LINE__);
			}
			fout.open(fileName.getCString());

			const LADataHolder &dfsH2 = objectPool.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix, NOCHECK);
			if (!dfsH2.isDefined() || dfsH2.isNull())
			{
				for (int j = 0; j < size; ++j)
				{
					LAString termOStr = LAString(terms[j]);
					LAString dfOStr = LAString(dfs[j]);
					fout << termOStr.getCString() << "," << dfOStr.getCString() << std::endl;
				}
			}
			else
			{
				const DoubleArray &dfs2 = dynamic_cast<const LADataDoubles &>(dfsH2.get()).get();
				for (int j = 0; j < size; ++j)
				{
					LAString termOStr = LAString(terms[j]);
					LAString dfOStr = LAString(dfs[j]);
					LAString df2OStr = LAString(dfs2[j]);
					fout << termOStr.getCString() << "," << dfOStr.getCString() << "," << df2OStr.getCString() << std::endl;
				}

			}
			fout.close();
			fin.close();
		}
		else
		{
			DoubleArray terms;
			DoubleArray dfs;
			DoubleArray dfs2;
			string line;
			while (getline(fin, line))
			{
				const char *c_line = line.c_str();
				LAStringVector lineVec = LAString(c_line).toToken(MARKET_DATA_DELIMITER);
				if (lineVec.size() == 3)
				{
					terms.push_back(lineVec[0].trimLeft().trimRight().getDoubleValue());
					dfs.push_back(lineVec[1].trimLeft().trimRight().getDoubleValue());
					dfs2.push_back(lineVec[2].trimLeft().trimRight().getDoubleValue());
				}
				else
				{
					terms.push_back(lineVec[0].trimLeft().trimRight().getDoubleValue());
					dfs.push_back(lineVec[1].trimLeft().trimRight().getDoubleValue());
				}
			}
			fin.close();
			dynamic_cast<LADataDoubles &>(objectPool.getData(CALIBRATION_DATA_TERMS + curveSuffix, ISNOTNULL).get()).set(terms);
			dynamic_cast<LADataDoubles &>(objectPool.getData(IR_CALIBRATION_DATA_DFS + curveSuffix, ISNOTNULL).get()).set(dfs);
			if (!dfs2.empty() && objectPool.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix).isDefined())
			{
				dynamic_cast<LADataDoubles &>(objectPool.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix, ISNOTNULL).get()).set(dfs2);
			}
		}
	}
}



bool
LAUpdateCurveObject::checkFrequency(const LAString& freq, const LAString& mktRateTerm) const
{
	int span = etrading::LADateHelpers::getPeriodFrequencyInMonths(freq);

	int y, m, d, w;
	etrading::LADateHelpers::termStrtoYMDW(mktRateTerm, y, m, d, w);
	int month_mkt_term = 12 * y + m;

	return (month_mkt_term % span) == 0;
}

///// update for XLL Plus //////////////////////////
#include "LAUpdateModelObject.h"

void
LAUpdateCurveObject::generateInitialValueForPricer(const LAString &currency, LADataInstance &dataInstance) const
{
	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

	LAObjectPool &objPool = dataInstance.getObjectPool();

	LAString yieldName = PREFIX_YIELD + getSDEAttrName(currency);
	LAMathYieldCurve *yc = NULL;
	const LAObjectHolder ehyc = objPool.getObject(yieldName);
	if (!ehyc.isDefined())
	{
		yc = new LAMathYieldCurve(&dataInstance);
		objPool.set(yieldName, yc);

	}
	else
	{
		dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		yc = &dynamic_cast<LAMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	yc->getName().convertFromString(yieldName);

	LAString curvePropertiesName = "PRO_" + yieldName;
	CurveCalibrationData *curveCalibrationData = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(curvePropertiesName);
	if (!ehycpro.isDefined())
	{
		curveCalibrationData = new CurveCalibrationData(&dataInstance);
		objPool.set(curvePropertiesName, curveCalibrationData);
	}
	else
	{
		//we must not erase the reset method for only curveCalibrationData
		curveCalibrationData = &dynamic_cast<CurveCalibrationData &>(objPool.getObject(curvePropertiesName).get());
	}
	curveCalibrationData->getName().convertFromString(curvePropertiesName);

	curveCalibrationData->getIsArbFree().set(false);

	LAObject *objectPool = NULL;
	LAString yieldDataName = yieldName + "_DATA";
	const LAObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		objectPool = new LAObject();
		objPool.set(yieldDataName, objectPool);
	}
	else
	{
		//we must not erase the reset method for only edata
		objectPool = &objPool.getObject(yieldDataName).get();
	}
	objectPool->remove(CALIBRATION_DATA_NAME);
	objectPool->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);

	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool useTenorBasis = false;
	bool useTenorBasisInstruments = false;
	bool isSpotStarting = false;

	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *curveCalibrationData, *objectPool, useTenorBasis, useTenorBasisInstruments, isSpotStarting, false);
	setUpCurveDataByContext(*curveCalibrationData, objectPool, currency, SWAP);


	LAStringVector listOfCurvesBuilt;
	LAString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == MLIB_NO_DATA)
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(listOfCurvesBuilt);
	if (!listOfCurvesBuilt.empty() && listOfCurvesBuilt[0] != MLIB_NO_DATA)
	{
		for (unsigned int i = 0; i < listOfCurvesBuilt.size(); ++i)
		{
			LAString suffixLowerCase = listOfCurvesBuilt[i];
			suffixLowerCase.toLower();
			LAString contextKey = tmpCurrency + CONTEXT_KEY_SDE_YIELD_WITH_MARKET + suffixLowerCase;
			LAString contextWithMarket = LACoreDataService::getContext(contextKey);
			if (contextWithMarket != MLIB_NO_DATA)
			{
				LAString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + suffixLowerCase).toUpper();
				if (marketType == MARKETTYPE_BASIS)
				{
					LAString useYieldSDEIRStr = LACoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
					LACoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD, MLIB_NO_DATA);
					LAString tmpCurveName = listOfCurvesBuilt[i];
					tmpCurveName.toUpper();
					mpStaticData->setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
					UpdateObjectPoolForSDEsAndCurves generator(currency);
					generator.loadModelDataAndCalibrate(currency, dataInstance, true);
					mpStaticData->removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
					LACoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD, useYieldSDEIRStr);
				}
			}
			setUpCurveDataByContext(*curveCalibrationData, objectPool, currency, listOfCurvesBuilt[i]);
		}
	}
}

void
LAUpdateCurveObject::setUpCurveDataByContext(CurveCalibrationData &curveCalibrationData, LAObject *objectPool, const LAString& currency, const LAString& marketName) const
{
	LAString prefix = currency;
	prefix.toLower();
	LAString suffixLowerCase;
	LAString data_suffix;
	if (marketName != SWAP)
	{
		suffixLowerCase = marketName;
		suffixLowerCase.toLower();
		data_suffix = "_" + marketName;
	}

	LAString contextKey = prefix + CONTEXT_KEY_SDE_YIELD + suffixLowerCase;
	LAString curveContext = LACoreDataService::getContext(contextKey);
	if (curveContext == MLIB_NO_DATA)
	{
		LAString msg = "context data for generated dfs don't exist.";
		msg += "(" + contextKey + ")";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	LADataDoubleMatrix matrix;
	matrix.convertFromString(curveContext);
	unsigned int rowSize = matrix.get1DSize();
	unsigned int colSize = matrix.getSize(0);

	bool isBasisDiscountFactor = colSize > 2 ? true : false;
	DoubleArray terms(rowSize);
	DoubleArray dfs(rowSize);
	DoubleArray dfs2(rowSize);
	for (unsigned int i = 0; i < rowSize; i++)
	{
		terms[i] = matrix.get(i, 0);
		dfs[i] = matrix.get(i, 1);

		if (isBasisDiscountFactor)
			dfs2[i] = matrix.get(i, 2);
	}

	objectPool->remove(CALIBRATION_DATA_TERMS + data_suffix);
	objectPool->add(CALIBRATION_DATA_TERMS + data_suffix, new LADataDoubles(terms));

	objectPool->remove(IR_CALIBRATION_DATA_DFS + data_suffix);
	objectPool->add(IR_CALIBRATION_DATA_DFS + data_suffix, new LADataDoubles(dfs));

	if (isBasisDiscountFactor)
	{
		LADataDoubles* attrDF2 = NULL;
		LADataHolder* df2H = &objectPool->getData(IR_CALIBRATION_DATA_DFS2 + data_suffix);
		if (!df2H->isDefined())
		{
			attrDF2 = new LADataDoubles();
			objectPool->remove(IR_CALIBRATION_DATA_DFS2 + data_suffix);
			objectPool->add(IR_CALIBRATION_DATA_DFS2 + data_suffix, attrDF2);
		}
		else
		{
			attrDF2 = dynamic_cast<LADataDoubles*>(&df2H->get());
		}
		attrDF2->set(dfs2);
		return;
	}

	// check DF2 by properties
	//LAString df2name = mpStaticData->getStaticData(prefix + STATIC_DATA_KEY_YIELD_DF2);
	LAString df2name = currency + "BASISDISCOUNT";
	df2name.toUpper();
	if (df2name != MLIB_NO_DATA && df2name == marketName)
	{
		LADataHolder* dh = &(objectPool->getData(CALIBRATION_DATA_TERMS, ISDEFINED));
		const DoubleVector& baseterms = dynamic_cast<const LADataDoubles &>(dh->get()).get();

		DoubleVector calcdf2(baseterms.size(), 1.0);
		LASplineInterpolation attrspline;
		attrspline.set(terms, dfs);
		for (unsigned int i = 0; i < baseterms.size(); i++)
			calcdf2[i] = attrspline.value(baseterms[i]);

		objectPool->remove(IR_CALIBRATION_DATA_DFS2);
		objectPool->add(IR_CALIBRATION_DATA_DFS2, new LADataDoubles(calcdf2));
	}



	// set curve name to yield curve pro
	LAStringVector gCurveNames;
	LADataHolder *dh = &(curveCalibrationData.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		gCurveNames = dynamic_cast<const LADataStrings &>(dh->get()).get();
	}
	gCurveNames.push_back(marketName);
	curveCalibrationData.LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	curveCalibrationData.LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(gCurveNames));
}
////////////////////////////////////////////////////

