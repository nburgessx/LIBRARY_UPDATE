//
// AQLUpdateCurveObject.cpp
//
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLUpdateCurveObject.h"
#include <fstream>
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLPriceDataManager.h"
#include "AQLDataReference.h"
#include "AQLDataMatrix.h"
#include "AQLMathYieldCurve.h"
#include "AQLRatesCurveLinearInterpolation.h"
#include "AQLStaticData.h"
#include "AQLDealUtils.h"
#include "AQLDataVector.h"
#include "AQLDataProcedure.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLMonotoneSplineInterpolation.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"
#include "AQLMarketData.h"
#include "AQLPriceArbFreeGenerator.h"
#include "AQLFunctionUtilities.h"
#include "AQLDateHelpers.h"
#include "AQLDateSchedule.h"
#include "AQLCurveForwardRateHelpers.h"
#include "ParameterValidation.h"
#include "CoreEnumerations.h"

#include "AQLRatesTermStructureSDE.h"
#include "AQLRatesLJTermStructureSDE.h"
#include "AQLMathCorrelation.h"
#include <time.h>

#include "CurveCalibrationData.h"
#include "CurveCalibration.h"
#include "AQLCurveMarketDataHelpers.h"
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
AQLUpdateCurveObject::AQLUpdateCurveObject(const AQLString &baseCurrency)
	: AQLObjectPoolBase(), mBaseCurrency(baseCurrency)
{
}

// destructor
AQLUpdateCurveObject::~AQLUpdateCurveObject(void)
{
}

// 
/*!
	@brief set setInterpolationMethod

	@param[in] currency
	@param[out] sde
*/
void
AQLUpdateCurveObject::setInterpolationMethod(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	(void)currency;
	sde.setInterpolationMethod(new AQLRatesCurveLinearInterpolation());
}

// 
/*!
	@brief generate sde market data

	IR is set initialvalue correlation volatility

	@param[in] currency
	@param[in] dataInstance
*/
void
AQLUpdateCurveObject::loadModelDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, const bool isModel, const bool isModelData, const AQLString & curveIndex, const AQLString & marketName ) const
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
AQLUpdateCurveObject::loadFwdFXConstCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, const AQLString & curveID, const AQLString & marketName) const
{
	if (!isFwdFXConst(currency))
		return;
	AQLString ccy = currency; ccy.toLower();

	AQLObjectPool &objPool = dataInstance.getObjectPool();
	bool isPricer = false;
	AQLString strIsPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (strIsPricer == "TRUE") isPricer = true;
	bool isSetCurveID = false;
	AQLString strIsSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);
	if (strIsSetCurveID == "TRUE") isSetCurveID = true;

	AQLString market = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_FWDFXCONST_USEMARKET);

	// get curve ccy and name
	AQLString strFCurve, strDCurve, strA_fCurve, strA_dCurve;
	AQLString ccy_fCurve, ccy_dCurve, ccy_a_fCurve, ccy_a_dCurve;
	AQLString fCurve, dCurve, a_fCurve, a_dCurve;
	AQLString suffixLowerCase = "." + AQLString(market).toLower();
	AQLString epSuffix = AQLString(market).toUpper();
	AQLString basisTarget = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_TARGET + suffixLowerCase).toUpper();
	if ( basisTarget == LEG1FORECAST || basisTarget == LEG2FORECAST )
{
    AQ_THROW( "fwdfx constant curve must be discount curve!" );
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
		AQ_THROW( "basisTarget curve is invalid!" );
	}
	convertCurveName(strFCurve, ccy, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(strDCurve, ccy, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(strA_fCurve, ccy, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(strA_dCurve, ccy, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	if (ccy_dCurve != ccy || ccy_fCurve != ccy || ccy_a_dCurve != ccy_a_fCurve)
		AQ_THROW( "currency of curve is inconsistent!" );

	const AQLString &ycProName = AQLMarketData::getBaseYieldProName(ccy);
	CurveCalibrationData &curveCalibrationData = dynamic_cast<CurveCalibrationData &>(objPool.getObject(ycProName).get());

	AQLObjectHolder& yData = curveCalibrationData.getYieldData().get();
	AQLString ydName = yData.getName();
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
		const AQLString &colYCProName = AQLMarketData::getBaseYieldProName(ccy_a_fCurve);
		colYCPro = &(dynamic_cast<CurveCalibrationData &>(objPool.getObject(colYCProName).get()));
		colYCPro->setColAffectingCcy(ccy.toUpper());
		curveCalibrationData.setColAffectedCcy(ccy_a_fCurve.toUpper());
	}
	const AQLString &fYCName = AQLMarketData::getBaseYieldName(ccy_a_fCurve);
	AQLMathYieldCurve& fYC = dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(fYCName).get());
	const AQLString &fYDName = fYC.getYieldData().get().getName();
	curveCalibrationData.getColYieldData().convertFromString(fYDName);

	// save market data
	AQLObject *mktData = NULL;
	AQLString basicCurveName = ydName + "_" + market.toUpper() + "_" + AQLString(static_cast<int>(0));
	const AQLObjectHolder ehbasis = objPool.getObject(basicCurveName);
	if (!ehbasis.isDefined())
	{
		mktData = new AQLObject();
		objPool.set(basicCurveName, mktData);
	}
	else
	{
		objPool.getObject(basicCurveName).get().clear();
		mktData = &objPool.getObject(basicCurveName).get();
	}
	mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(basicCurveName);
	mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_FWDFXCONST);
	mktData->add(IR_CALIBRATION_DATA_FORECAST, new AQLDataString()).convertFromString(fCurve);
	mktData->add(IR_CALIBRATION_DATA_DISCOUNT, new AQLDataString()).convertFromString(dCurve);
	mktData->add(IR_CALIBRATION_DATA_AGTFORECAST, new AQLDataString()).convertFromString(a_fCurve);
	mktData->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new AQLDataString()).convertFromString(a_dCurve);

	AQLStringVector tokens = strA_fCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (tokens.size() == 2)
	{
		AQLString againstCurveCollectionID = tokens[0];
		mktData->add(IR_CALIBRATION_DATA_AGTCURVECOLLECTION, new AQLDataString()).convertFromString(againstCurveCollectionID);
	}

	curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + epSuffix);
	curveCalibrationData.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + epSuffix, new AQLDataMultiReference()).convertFromString(basicCurveName);

	// save assigned curves
	AQLStringVector assignedCurves;
	AQLString strAssignedCurves = mpStaticData->getStaticData(ccy.toLower() + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffixLowerCase);
	if (strAssignedCurves != AQ_NO_DATA)
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
	AQLString curveType = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
	if (curveType != AQ_NO_DATA)
	{
		curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix);
		curveCalibrationData.AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix, new AQLDataString(curveType.toUpper()));

		yData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix);
		yData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix, new AQLDataString(curveType.toUpper()));
	}

	// set interpolation
	AQLString genInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase);
	if (genInterp == AQ_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}
	if (suffixLowerCase.size() == 0)
	{
		curveCalibrationData.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + epSuffix);
		curveCalibrationData.AQLObject::add(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genInterp);
	}

	// set yieldgen interpolation
	AQLString genYieldGenInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffixLowerCase);
	if (genYieldGenInterp == AQ_NO_DATA)
	{
		if (genInterp != AQ_NO_DATA)
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
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + epSuffix);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// calc curve
	//curveCalibrationData.calcFwdFXConstantCurve();
	curveCalibrationData.calcFwdFXConstantCurveUsingMarketName(market);// Calculate the FwdFXConstCurve but allow any FWDFXCONST marketName to be specified

	// dataout
	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		dataoutCurve(assignedCurves, yData.get(), ydName);
}

// 
/*!
	@brief generate sde initial curve data for cheapest-to-deliver curve

	@param[in/out] dataInstance
*/
void AQLUpdateCurveObject::loadCheapestToDeliverCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	AQLString ccy = currency;
	ccy.toLower();

	AQLObjectPool &objPool = dataInstance.getObjectPool();

	bool isPricer = false;
	AQLString strIsPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (strIsPricer == "TRUE")
	{
		isPricer = true;
	}

	bool isSetCurveID = false;
	AQLString strIsSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);
	if (strIsSetCurveID == "TRUE")
	{
		isSetCurveID = true;
	}

	// Curve name and yieldCurvePro
	const AQLString &ycProName = AQLMarketData::getBaseYieldProName(ccy);
	AQLObjectHolder objHolder = objPool.getObject(ycProName);
	if (!objHolder.isDefined())
	{
		AQLString err = "#Error: CurveCollection for currency '" + ccy + "' is not defined. Please check your CurveCollection";
		AQ_THROW( err.getCString() );
	}
	CurveCalibrationData &curveCalibrationData = dynamic_cast<CurveCalibrationData &>(objHolder.get());

	AQLObjectHolder& yData = curveCalibrationData.getYieldData().get();
	AQLString ydName = yData.getName();
	if (isSetCurveID)
	{
		ydName = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}

	// set asofdate
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	yData.remove(CALIBRATION_DATA_ASOFDATE);
	yData.add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));

	// set currency
	yData.remove(IR_CALIBRATION_DATA_CURRENCY);
	yData.add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString(currency));

	// Curve name
	AQLString market = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET).toUpper();
	AQLString suffixLowerCase = "." + market;
	suffixLowerCase.toLower();
	AQLString epSuffix = market;
	epSuffix.toUpper();

	// save market data
	AQLObject *mktData = NULL;
	AQLString basicCurveName = ydName + "_" + epSuffix;
	const AQLObjectHolder ehbasis = objPool.getObject(basicCurveName);
	if (!ehbasis.isDefined())
	{
		mktData = new AQLObject();
		objPool.set(basicCurveName, mktData);
	}
	else
	{
		objPool.getObject(basicCurveName).get().clear();
		mktData = &objPool.getObject(basicCurveName).get();
	}

	// Convention data
	AQLString calendar = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CALENDAR + suffixLowerCase).toUpper();
	AQLString dayCount = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT + suffixLowerCase).toUpper();
	AQLString businessAdj = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE + suffixLowerCase).toUpper();
	AQLString frequency = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY + suffixLowerCase).toUpper();

	mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(calendar);
	mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(dayCount);
	mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(businessAdj);
	mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(frequency);
	mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_CTD);

	curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + epSuffix);
	curveCalibrationData.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + epSuffix, new AQLDataMultiReference()).convertFromString(basicCurveName);

	// save assigned curves
	AQLStringVector assignedCurves;
	AQLString strAssignedCurves = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffixLowerCase);
	if (strAssignedCurves != AQ_NO_DATA)
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
	AQLString curveType = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
	if (curveType != AQ_NO_DATA)
	{
		curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix);
		curveCalibrationData.AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix, new AQLDataString(curveType.toUpper()));

		yData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix);
		yData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix, new AQLDataString(curveType.toUpper()));
	}

	// Get CSA collateral curves
	AQLStringVector csaCurves;
	AQLString csaCurvesTemp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_CTD_COLLATERALCURVES + suffixLowerCase);
	if (csaCurvesTemp != AQ_NO_DATA)
	{
		csaCurves = csaCurvesTemp.toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		AQ_THROW( "Must provide at least one CSA curve" );
	}

	// set interpolation
	AQLString genInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase);
	if (genInterp == AQ_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}
	if (epSuffix.size() == 0)
	{
		curveCalibrationData.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + epSuffix);
		curveCalibrationData.AQLObject::add(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genInterp);
	}

	// set yieldgen interpolation
	AQLString genYieldGenInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffixLowerCase);
	if (genYieldGenInterp == AQ_NO_DATA)
	{
		if (genInterp != AQ_NO_DATA)
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
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + epSuffix);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genYieldGenInterp);
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
AQLUpdateCurveObject::convertCurveName(const AQLString &propCurve, const AQLString &ccy, AQLString &curveCcy, AQLString &curveName, const bool isPricer, AQLObjectPool& objPool) const
{
	if (propCurve.findString(DUMMY) >= 0)
	{
		curveName = DUMMY;
	}
	else
	{
		AQLStringVector propCurves = propCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (propCurves.size() == 1)
		{
			curveCcy = ccy;
			curveName = propCurves[0];
		}
		else if (propCurves.size() == 2)
		{
			if (isPricer)
			{
				AQLObject* yieldData = &(objPool.getObject(propCurves[0], ENCHKTYPE_ISDEFINED).get());
				curveCcy = dynamic_cast<AQLDataString&>((yieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
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
			AQ_THROW( "currency and curve name are not set!" );
		}
	}
}

/*!
	@brief get Accessory from Frequency

	@param[in] freq : frequency
	@return accessory
*/
const AQLString
AQLUpdateCurveObject::getAccFromFreq(const AQLString &freq_) const
{
	AQLString freq = freq_;
	freq.toUpper();
	AQLString acc;
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
		AQLString msg = "can't convert this frequency(" + freq_ + ") to accessory.";
		AQ_THROW( msg.getCString() );
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
AQLUpdateCurveObject::loadDualBootstrapCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

	// Get YieldCurve and CurveCalibrationData objects (as dataValues)
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString isSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	AQLString yieldName = AQLMarketData::getBaseYieldName(currency);

	AQLMathYieldCurve *yc = NULL;
	const AQLObjectHolder ehyc = objPool.getObject(yieldName);
	if (!ehyc.isDefined())
	{
		yc = new AQLMathYieldCurve(&dataInstance);
		objPool.set(yieldName, yc);
	}
	else
	{
		dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		yc = &dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	yc->getName().convertFromString(yieldName);

	AQLString curvePropertiesName = "PRO_" + yieldName;
	CurveCalibrationData *curveCalibrationData = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(curvePropertiesName);
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
	bool isArbFree = false;		// this flag is not used; hardcoded to false
	curveCalibrationData->getIsArbFree().set(isArbFree);

	// Get YieldData object (as an data)
	AQLObject *objectPool = NULL;
	AQLString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const AQLObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		objectPool = new AQLObject();
		objPool.set(yieldDataName, objectPool);
	}
	else
	{
		//we must not erase the reset method for only edata
		objectPool = &objPool.getObject(yieldDataName).get();
	}
	objectPool->remove(CALIBRATION_DATA_NAME);
	objectPool->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);

	// As of date
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// Main curve name
	bool enableCalculation = true;
	AQLString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);

	if ( target == AQ_NO_DATA )
{
    AQ_THROW( "Dual bootstrapping curve name is not detected" );
}
	else
	{
		enableCalculation = false;
	}

	target.toUpper();
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(target));

	AQLString suffixCurveObj = (target == STD || target == SWAP) ? "" : "_" + target;
	AQLString suffixLowerCase = (target == STD || target == SWAP) ? "" : "." + target;
	suffixLowerCase.toLower();

	// OIS curve name and suffixLowerCase
	AQLString currentCurveName_ois = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_OISCURVENAME + suffixLowerCase).toUpper();
	AQ_THROW_IF( currentCurveName_ois == AQ_NO_DATA, "OIS curve name is not found in performing dual bootstrapping" );

	AQLString suffix_ois = (currentCurveName_ois == STD || currentCurveName_ois == SWAP) ? "" : "." + currentCurveName_ois;
	suffix_ois.toLower();

	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + suffixCurveObj, new AQLDataString(currentCurveName_ois));

	// Swap curve name and suffixLowerCase
	AQLString currentCurveName_swap = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME + suffixLowerCase);
	currentCurveName_swap.toUpper();
	AQ_THROW_IF( currentCurveName_swap == AQ_NO_DATA, "Swap curve name is not found in performing dual bootstrapping" );

	AQLString suffix_swap = (currentCurveName_swap == STD || currentCurveName_swap == SWAP) ? "" : "." + currentCurveName_swap;
	suffix_swap.toLower();

	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE + suffixCurveObj, new AQLDataString(currentCurveName_swap));

	//-----------------------------------------------------------------
	// Extract common curve building parameters

	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	// Fast rebuild	
	AQLString tmpFastRebuild_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_FASTREBUILD + suffixLowerCase).toUpper();
	bool fastRebuild = true;
	if (tmpFastRebuild_str == "FALSE")
	{
		fastRebuild = false;
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj, new AQLDataBool(fastRebuild));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj, new AQLDataBool(fastRebuild));

	//get constant for convergence
	double eps = 1.0e-9;
	AQLString strEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_EPSILON + suffixLowerCase);
	if (strEPS.toUpper() != AQ_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj, new AQLDataDouble(eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj, new AQLDataDouble(eps));

	double grad_eps = 1.0e-15;
	AQLString strGEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_GRADIENTEPSILON + suffixLowerCase);
	if (strGEPS.toUpper() != AQ_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj, new AQLDataDouble(grad_eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj, new AQLDataDouble(grad_eps));

	double delta = 1.0e-10;
	AQLString strDLT = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_DELTA + suffixLowerCase);
	if (strDLT.toUpper() != AQ_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj, new AQLDataDouble(delta));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj, new AQLDataDouble(delta));

	int maxLoop = 1000;
	AQLString strMLP = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_MAXLOOP + suffixLowerCase);
	if (strMLP.toUpper() != AQ_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj, new AQLDataInt(maxLoop));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj, new AQLDataInt(maxLoop));

	//-----------------------------------------------------------------
	// Extract individual curve's configuration parameters

	// Define a number of control modes
	bool useTenorBasis = false;
	bool useTenorBasisInstruments = false;
	bool isSpotStarting = false;

	// Set up curve Generate Config for the swap curve
	AQLString suffixCurveObj_swap = (currentCurveName_swap == STD || currentCurveName_swap == SWAP) ? "" : "_" + currentCurveName_swap;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *curveCalibrationData, *objectPool, useTenorBasis, useTenorBasisInstruments, isSpotStarting, isArbFree, suffix_swap, suffixCurveObj_swap);

	// Set up curve Generate Config for the OIS curve
	AQLString suffixCurveObj_ois = (currentCurveName_ois == STD || currentCurveName_ois == SWAP) ? "" : "_" + currentCurveName_ois;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *curveCalibrationData, *objectPool, useTenorBasis, useTenorBasisInstruments, isSpotStarting, isArbFree, suffix_ois, suffixCurveObj_ois);

	// Support on FX?
	AQLString fxName = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != AQ_NO_DATA)
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

	const std::map<AQLString, AQLString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
	for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		const AQLString& curveName = it->first;
		curveCalibrationData->setDualBootstrapOISCurveMktMap(curveName, it->second);
	}

	// Store curve type to object pool under current engine name + current curve name
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_ois);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_ois, new AQLDataString("OIS"));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_ois);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_ois, new AQLDataString("OIS"));

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
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_swap);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_swap, new AQLDataString("SWAP"));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_swap);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_swap, new AQLDataString("SWAP"));

	for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		const AQLString& curveName = it->first;
		curveCalibrationData->setDualBootstrapSwapCurveMktMap(curveName, it->second);
	}

	mCurveGenCcyMap[currency] = true;

	// Store all curve names
	AQLString allCurveNames = currentCurveName_ois + AQLString(MULTI_STATIC_DATA_DELIMITER) + currentCurveName_swap;
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj, new AQLDataString(allCurveNames));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj, new AQLDataString(allCurveNames));

	// Get CurveCalibration out of CurveCalibrationData
	AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>(curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	const AQLCoreProcedure& yieldGenProcedure = modelDataObj.getMethod();
	const CurveCalibration& curveCalibrationEngine = dynamic_cast<const CurveCalibration&>(yieldGenProcedure);

	// Peform dual-bootstrapping
	AQLObject* parent = dynamic_cast<AQLObject*>(curveCalibrationData);
	curveCalibrationEngine.dualbootstrap(asOfDate, *parent, AQLDataProcedure());

}


// 
/*!
@brief generate curve data using dual-bootstrapping technique

@param[in] currency  currency of the curve
@param[out] dataInstance
*/
void AQLUpdateCurveObject::loadGlobalCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

	// Get YieldCurve and CurveCalibrationData objects (as dataValues)
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString isSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	AQLString yieldName = AQLMarketData::getBaseYieldName(currency);

	AQLMathYieldCurve *yc = NULL;
	const AQLObjectHolder ehyc = objPool.getObject(yieldName);
	if (!ehyc.isDefined())
	{
		yc = new AQLMathYieldCurve(&dataInstance);
		objPool.set(yieldName, yc);
	}
	else
	{
		dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		yc = &dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	yc->getName().convertFromString(yieldName);

	AQLString curvePropertiesName = "PRO_" + yieldName;
	CurveCalibrationData *curveCalibrationData = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(curvePropertiesName);
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
	bool isArbFree = false;		// this flag is not used; hardcoded to false
	curveCalibrationData->getIsArbFree().set(isArbFree);

	// Get YieldData object (as an data)
	AQLObject *objectPool = NULL;
	AQLString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const AQLObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		objectPool = new AQLObject();
		objPool.set(yieldDataName, objectPool);
	}
	else
	{
		//we must not erase the reset method for only edata
		objectPool = &objPool.getObject(yieldDataName).get();
	}
	objectPool->remove(CALIBRATION_DATA_NAME);
	objectPool->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);

	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	// As of date
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// Main curve name
	bool enableCalculation = true;
	AQLString engineName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);

	if ( engineName == AQ_NO_DATA )
	{
		AQ_THROW( "Name of the global yield curve calibration engine is not detected" );
	}
	else
	{
		enableCalculation = false;
	}

	engineName.toUpper();
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(engineName));

	AQLString suffixCurveObj = (engineName == STD || engineName == SWAP) ? "" : "_" + engineName;
	AQLString suffixLowerCase = (engineName == STD || engineName == SWAP) ? "" : "." + engineName;
	suffixLowerCase.toLower();

	//-----------------------------------------------------------------
	// Extract common curve building parameters

	// Fast rebuild	
	AQLString tmpFastRebuild_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_FASTREBUILD + suffixLowerCase).toUpper();
	bool fastRebuild = true;
	if (tmpFastRebuild_str == "FALSE")
	{
		fastRebuild = false;
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj, new AQLDataBool(fastRebuild));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffixCurveObj, new AQLDataBool(fastRebuild));

	//get constant for convergence
	double eps = 1.0e-9;
	AQLString strEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_EPSILON + suffixLowerCase);
	if (strEPS.toUpper() != AQ_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj, new AQLDataDouble(eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + suffixCurveObj, new AQLDataDouble(eps));

	double grad_eps = 1.0e-15;
	AQLString strGEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_GRADIENTEPSILON + suffixLowerCase);
	if (strGEPS.toUpper() != AQ_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj, new AQLDataDouble(grad_eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + suffixCurveObj, new AQLDataDouble(grad_eps));

	double delta = 1.0e-10;
	AQLString strDLT = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_DELTA + suffixLowerCase);
	if (strDLT.toUpper() != AQ_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj, new AQLDataDouble(delta));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + suffixCurveObj, new AQLDataDouble(delta));

	int maxLoop = 1000;
	AQLString strMLP = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_MAXLOOP + suffixLowerCase);
	if (strMLP.toUpper() != AQ_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj, new AQLDataInt(maxLoop));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + suffixCurveObj, new AQLDataInt(maxLoop));

	//-----------------------------------------------------------------------------------------------------------
	// Loop through each curve in the engine and prepare its calibration parameters and data in the object pool

	// Define a number of control modes
	bool useTenorBasis = false;
	bool useTenorBasisInstruments = false;
	bool isSpotStarting = false;

	AQLString allCurveTypes = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVETYPES + suffixLowerCase);
	AQLStringVector curveTypeVector = allCurveTypes.toToken(MULTI_STATIC_DATA_DELIMITER);

	AQLString allCurveNames = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVENAMES + suffixLowerCase);
	AQLStringVector curveNameVector = allCurveNames.toToken(MULTI_STATIC_DATA_DELIMITER);

	if (curveTypeVector.size() != curveNameVector.size())
	{
		AQ_THROW("Not all the curves in the global curve engine has been given a curve type.");
	}

	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj, new AQLDataString(allCurveNames));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffixCurveObj, new AQLDataString(allCurveNames));

	for (unsigned int i = 0; i < curveNameVector.size(); ++i)
	{
		// Get single curve's name and suffixLowerCase
		AQLString propertyManagerCurveName = curveNameVector[i].toUpper();
		AQLString propertyManagerCurveType = curveTypeVector[i].toUpper();

		AQLString suffix_currentCurve = (propertyManagerCurveName == STD || propertyManagerCurveName == SWAP) ? "" : "." + propertyManagerCurveName;
		suffix_currentCurve.toLower();

		AQLString suffixCurveObj_currentCurve = (propertyManagerCurveName == STD || propertyManagerCurveName == SWAP) ? "" : "_" + propertyManagerCurveName;
		suffixCurveObj_currentCurve.toUpper();

		// Store curve type to object pool under current engine name + current curve name
		curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_currentCurve);
		curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_currentCurve, new AQLDataString(propertyManagerCurveType));
		yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_currentCurve);
		yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffixCurveObj + suffixCurveObj_currentCurve, new AQLDataString(propertyManagerCurveType));

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

		//const std::map<AQLString, AQLString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
		//for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		//{
		//	const AQLString& curveName = it->first;
		//	curveCalibrationData->setDualBootstrapOISCurveMktMap(curveName, it->second);
		//}
	}

	mCurveGenCcyMap[currency] = true;

	// Get CurveCalibration out of CurveCalibrationData
	AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>(curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	const AQLCoreProcedure& yieldGenProcedure = modelDataObj.getMethod();
	const CurveCalibration& curveCalibrationEngine = dynamic_cast<const CurveCalibration&>(yieldGenProcedure);

	// Peform dual-bootstrapping
	AQLObject* parent = dynamic_cast<AQLObject*>(curveCalibrationData);
	curveCalibrationEngine.buildEngineCurves(asOfDate, *parent, AQLDataProcedure());

}


/*!
	@brief
*/
void AQLUpdateCurveObject::configureCurve(AQLStaticData *mpStaticData,
	CurveCalibrationData *curveCalibrationData,
	AQLMathYieldCurve *yc,
	AQLDataInstance &dataInstance,
	const AQLDate& asOfDate,
	const AQLString& currency,
	const AQLString& curveName,
	const AQLString& yieldDataName,
	const AQLString& suffixLowerCase,
	const AQLString& suffixCurveObj,
	bool enableCalculation,
	bool isSpotStarting,
	bool useTenorBasis,
	bool useTenorBasisInstruments) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	AQLString propertyManagerCurveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
	propertyManagerCurveType.toUpper();

	// reference data for market pro
	AQLString staticDataName;
	AQLString tenorBasisStaticDataName;
	std::map<AQLString, std::map<AQLString, double> > tenorBasisMarketQuotes;

	if (enableCalculation || propertyManagerCurveType == SWAP)
	{
		// Variable 'target' is essentially the name of the curve
		setUpGenCurveData(dataInstance, staticDataName, asOfDate, currency, SWAP, yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes, AQLString(curveName).toUpper());
	}

	// Get isXccyMarkedToMarket
	bool isXccyMarkedToMarket = false;
	AQLString isXccyMarkedToMarketString = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST + suffixLowerCase).toUpper();
	if (isXccyMarkedToMarketString == "TRUE")
	{
		isXccyMarkedToMarket = true;
	}

	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffixCurveObj, new AQLDataBool(isXccyMarkedToMarket));

	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET + suffixCurveObj);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET + suffixCurveObj, new AQLDataBool(isXccyMarkedToMarket));

	// Get the list of curve indexes
	AQLStringVector listOfCurvesBuilt;
	AQLString tmpMarket = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffixLowerCase).toUpper();
	if (tmpMarket == AQ_NO_DATA)
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

	AQLDataHolder* dh;
	AQLStringVector swapCurveDiscountFactors;
	dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		swapCurveDiscountFactors = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}

	AQLStringVector basisCurves;
	dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		basisCurves = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}

	AQLString tenorBasisDiscountFactors = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_DF2);
	AQLStringVector tenorBasisMarkets;
	if (!listOfCurvesBuilt.empty() && listOfCurvesBuilt[0] != AQ_NO_DATA)
	{
		for (unsigned int i = 0; i < listOfCurvesBuilt.size(); ++i)
		{
			AQLString thisCurveStaticData;
			AQLString suffixLowerCase = "." + listOfCurvesBuilt[i];
			suffixLowerCase.toLower();

			// set curve type			
			AQLString curveType;
			AQLObjectHolder& yieldData = curveCalibrationData->getYieldData().get();

			if (listOfCurvesBuilt[i] == SWAP)
			{
				curveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE);
				if (curveType != AQ_NO_DATA)
				{
					curveCalibrationData->AQLObject::remove(CALIBRATION_DATA_CURVETYPE);
					curveCalibrationData->AQLObject::add(CALIBRATION_DATA_CURVETYPE, new AQLDataString(curveType.toUpper()));

					yieldData.remove(CALIBRATION_DATA_CURVETYPE);
					yieldData.add(CALIBRATION_DATA_CURVETYPE, new AQLDataString(curveType.toUpper()));
				}
			}
			else
			{
				curveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
				if (curveType != AQ_NO_DATA)
				{
					curveCalibrationData->AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + listOfCurvesBuilt[i]);
					curveCalibrationData->AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + listOfCurvesBuilt[i], new AQLDataString(curveType.toUpper()));

					yieldData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + listOfCurvesBuilt[i]);
					yieldData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + listOfCurvesBuilt[i], new AQLDataString(curveType.toUpper()));
				}
			}

			if (listOfCurvesBuilt[i] == SWAP)
			{
				if (enableCalculation || curveName == STD)
				{
					AQLString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE);
					if (aliasCurveNames != AQ_NO_DATA)
					{
						AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
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

			AQLString marketType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffixLowerCase).toUpper();
			curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_MARKETTYPE + AQLString("_") + listOfCurvesBuilt[i]);
			curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_MARKETTYPE + AQLString("_") + listOfCurvesBuilt[i], new AQLDataString(marketType));

			if (marketType == AQ_NO_DATA)
			{
				AQLString isBasisStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ISBASIS + suffixLowerCase).toUpper();
				AQLString isReadFile = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffixLowerCase).toUpper();
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
			AQLString strInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase).toUpper();
			if (strInter != AQ_NO_DATA)
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

				AQLString thisMarketName = listOfCurvesBuilt[i];
				tenorBasisMarkets.push_back(listOfCurvesBuilt[i]);
				AQLString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != AQ_NO_DATA && (enableCalculation || curveName == listOfCurvesBuilt[i]))
				{
					AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
					for (unsigned int j = 0; j < assignedCurves.size(); j++)
					{
						if (basisCurves.end() == std::find(basisCurves.begin(), basisCurves.end(), assignedCurves[j]))
						{
							basisCurves.push_back(assignedCurves[j]);
						}
					}
					AQLStringVector marketNames = listOfCurvesBuilt[i].toToken('_');
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
						AQLString tenorswapname = dynamic_cast<const AQLDataString &>(curveCalibrationData->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
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

				AQLString thisMarketName = listOfCurvesBuilt[i];
				AQLString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != AQ_NO_DATA)
				{
					AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
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

				AQLString oisCompoundType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffixLowerCase).toUpper();
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
				AQ_THROW( "Market type is not supported!!" );
			}

			// set yield curve pro
			thisCurveStaticData = thisCurveStaticData.subString(0, thisCurveStaticData.size() - 2);
			AQLStringVector marketNames = listOfCurvesBuilt[i].toToken('_');
			if (marketNames.size() == 2)
			{
				CurveCalibrationData &fYcPro = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(AQLMarketData::getBaseYieldProName(marketNames[0]), ENCHKTYPE_ISDEFINED).get());
				fYcPro.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + marketNames[1]);
				fYcPro.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + marketNames[1], new AQLDataMultiReference()).convertFromString(thisCurveStaticData);
			}
			else
			{
				curveCalibrationData->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + listOfCurvesBuilt[i]);
				curveCalibrationData->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + listOfCurvesBuilt[i], new AQLDataMultiReference()).convertFromString(thisCurveStaticData);
			}
		}
	}

	//set tenorswap convention
	AQLString tenorSwapName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(listOfCurvesBuilt.begin(), listOfCurvesBuilt.end(), tenorSwapName) == listOfCurvesBuilt.end() && tenorSwapName != AQ_NO_DATA)
	{
		AQLString refData_tenor = "";
		setUpBasisCurveData(dataInstance, refData_tenor, asOfDate, currency, tenorSwapName, yieldDataName, isSpotStarting, *curveCalibrationData, curveName);
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		curveCalibrationData->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName);
		curveCalibrationData->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName, new AQLDataMultiReference()).convertFromString(refData_tenor);
	}

	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(swapCurveDiscountFactors));

	if (tenorBasisDiscountFactors != AQ_NO_DATA)
	{
		curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new AQLDataString(tenorBasisDiscountFactors));
	}

	dataInstance.getReferencePool().completeDependency();

	// Whether to generate output forward rates only from swaps or from swaps + futures/FRA
	AQLString prioritizeSwaps = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY);
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY);
	if (prioritizeSwaps != AQ_NO_DATA)
	{
		if (prioritizeSwaps.toUpper() == "TRUE")
		{
			curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new AQLDataBool(true));
		}
		else
		{
			curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new AQLDataBool(false));
		}
	}

	if (!tenorBasisMarketQuotes.empty())
	{
		map<AQLString, map<AQLString, double> >::const_iterator it = tenorBasisMarketQuotes.begin();
		while (it != tenorBasisMarketQuotes.end())
		{
			AQLCurveMarketDataHelpers::restoreSwapRateFromLibor(*curveCalibrationData, it->second, currency, &(it->first));
			++it;
		}
	}

	// Data settings that are consistent with the single-curve building mechanism of basis curve
	bool hasNoBasisCurve = false;
	hasNoBasisCurve = tenorBasisMarkets.end() == std::find(tenorBasisMarkets.begin(), tenorBasisMarkets.end(), curveName);
	AQLString thisMarketName;
	if (curveName != STD)
	{
		thisMarketName = AQLString(".") + curveName;
		thisMarketName.toLower();
	}

	AQLString aliasCurveNames;
	if (hasNoBasisCurve)
	{
		aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + thisMarketName);
	}
	else
	{
		aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + thisMarketName);
	}

	AQLString targetCurve;
	if (aliasCurveNames != AQ_NO_DATA && aliasCurveNames != "")
	{
		AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
		targetCurve = assignedCurves[0];
	}
	else
	{
		AQ_THROW("No target basis curve defined")
	}

	if (!hasNoBasisCurve)
	{
		AQLString suffixLowerCase = AQLString("_") + targetCurve;
		suffixLowerCase.toUpper();
		curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF + suffixLowerCase);
		curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF + suffixLowerCase, new AQLDataString(targetCurve));
	}

	// basis
	bool isBasis = false;
	if (!basisCurves.empty() && !hasNoBasisCurve)
	{
		isBasis = true;
		curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_BASISDFS, new AQLDataStrings(basisCurves));
	}

}


// ********************************************************************************************************
// **************** HELPER METHODS FOR 'loadModelDataAndCalibrate' ROUTINE ********************************

// Method to Initialize and Check if Legacy Curves are to be Used, if boolean result is true must exit calibration
bool AQLUpdateCurveObject::useLegacyCurves( const AQLString &currency, AQLDataInstance &dataInstance ) const
{
	// Result Flag
	bool exitFlag = false;

	// Currency Names
	AQLString curveCurrencyLowerCase = currency;
	curveCurrencyLowerCase.toLower();

	// Required for Legacy Method to Initialize the Curve for a Stand Alone Pricing Tool
	// ***********************************************************************************************
	AQLString useLegacyYieldModel = AQLCoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
	if (useLegacyYieldModel != AQ_NO_DATA)
	{
		AQLDataBool useYieldModelBool;
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
	AQLString isArbitrageFreeString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_ISARBFREE);
	if (isArbitrageFreeString != AQ_NO_DATA)
	{
		AQLDataBool useArbFreeModelBool;
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
void AQLUpdateCurveObject::updateCurveObjectAndImportPropertyManagerData( CurveObject & curveObj ) const
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
void AQLUpdateCurveObject::initializeCurveObject( CurveObject & curveObj ) const
{
	// Currency Names
	AQLString curveCurrencyLowerCase = curveObj.currency_;
	curveCurrencyLowerCase.toLower();
	
	// Object Pool
	AQLObjectPool & objPool = curveObj.dataInstance_.getObjectPool();

	AQLString yieldName = AQLMarketData::getBaseYieldName(curveObj.currency_);
	AQLString isSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	curveObj.mathObj_ = NULL;
	const AQLObjectHolder yieldCurveObjHolder = objPool.getObject(yieldName);
	if (!yieldCurveObjHolder.isDefined())
	{
		curveObj.mathObj_ = new AQLMathYieldCurve(&curveObj.dataInstance_);
		objPool.set(yieldName, curveObj.mathObj_);
	}
	else
	{
		dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		curveObj.mathObj_ = &dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	curveObj.mathObj_->getName().convertFromString(yieldName);

	AQLString curvePropertiesName = "PRO_" + yieldName;
	curveObj.calibrationData_ = NULL;
	const AQLObjectHolder yieldCurvePropertiesObj = objPool.getObject(curvePropertiesName);
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
	const AQLObjectHolder ehdata = objPool.getObject(curveObj.name_);
	if (!ehdata.isDefined())
	{
		curveObj.instance_ = new AQLObject();
		objPool.set(curveObj.name_, curveObj.instance_);
	}
	else
	{
		//we must not erase the reset method for only edata
		curveObj.instance_ = &objPool.getObject(curveObj.name_).get();
	}
	curveObj.instance_->remove(CALIBRATION_DATA_NAME);
	curveObj.instance_->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(curveObj.name_);


	// Enable Curve Calculations Only if Target Curve Type Specified
	// **************************************************************************************
	curveObj.enableCalculation_ = true;
	curveObj.targetCurveType_ = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET).toUpper();
	if (curveObj.targetCurveType_ != AQ_NO_DATA)
	{
		curveObj.enableCalculation_ = false;
	}
}

// Method to Initialise the Curve Object using Property Manager Config Data
void AQLUpdateCurveObject::updateCurveObjectPropertyMangerData( CurveObject & curveObj ) const
{
	AQLString suffixLowerCase = (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP) ? "" : "." + curveObj.targetCurveType_;
	suffixLowerCase.toLower();

	AQLString suffixCurveObj = (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP) ? "" : "_" + curveObj.targetCurveType_;
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

	AQLString fxName = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != AQ_NO_DATA)
	{
		curveObj.calibrationData_->getFXEntity().convertFromString(fxName);
	}
}

// Method to Initialize Risk Data
void AQLUpdateCurveObject::updateCurveObjectRiskData( CurveObject & curveObj ) const
{
	// Context Key for Curve Data Interface
	AQLString contextKey = curveObj.currency_;
	contextKey.toLower();
	AQLString curveContext = AQLCoreDataService::getContext(contextKey + CONTEXT_KEY_SDE_YIELD);

	// Do we have the Risk Results?
	bool doesRiskDataExist = false;
	
	// Use Bitwise OR here if LHS or RHS is true the result is true
	doesRiskDataExist |= AQLCoreDataService::getContext(ARG_KEY_OFFICIALRISK) != AQ_NO_DATA ? true : false;
	doesRiskDataExist |= AQLCoreDataService::getContext(ARG_KEY_FRONTRISK)	 != AQ_NO_DATA ? true : false;
		
	// Update Risk if we have the Risk Results and they are Missing from the Curve Context / Data Instance
	if ( !doesRiskDataExist && curveContext != AQ_NO_DATA )
	{
		AQLDataDoubleMatrix matrix;
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
		dynamic_cast<AQLDataDoubles &>(curveObj.instance_->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<AQLDataDoubles &>(curveObj.instance_->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		if (isBasisDiscountFactor)
		{
			AQLDataDoubles* attrDF2 = NULL;
			AQLDataHolder* df2H = &curveObj.instance_->getData(IR_CALIBRATION_DATA_DFS2);
			if (!df2H->isDefined())
			{
				attrDF2 = new AQLDataDoubles();
				curveObj.instance_->remove(IR_CALIBRATION_DATA_DFS2);
				curveObj.instance_->add(IR_CALIBRATION_DATA_DFS2, attrDF2);
			}
			else
			{
				attrDF2 = dynamic_cast<AQLDataDoubles*>(&df2H->get());
			}
			attrDF2->set(dfs2);
		}
	}
}

// Method to Set-Up Curve Market Data
void AQLUpdateCurveObject::updateCurveObjectMarketData( CurveObject & curveObj ) const
{
	// CURVE BUILDING
	// Rebuild curves if the curve index is missing from the useMarkets or listOfCurvesBuilt list
	
	// *** Important Note ***
	// The ir.Properties file hard codes the listOfCurvesBuilt list in some instances to indicate which curves *** NOT *** to build

	// Object Pool
	AQLObjectPool & objPool = curveObj.dataInstance_.getObjectPool();

	// set curve properties
	if (curveObj.staticDataName_.size() > 0 && (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP))
	{
		curveObj.staticDataName_ = curveObj.staticDataName_.subString(0, curveObj.staticDataName_.size() - 2);
		curveObj.calibrationData_->getMarketData().convertFromString(curveObj.staticDataName_);
	}

	AQLDataHolder* dh;
	dh = &curveObj.calibrationData_->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		curveObj.swapCurveDiscountFactors_ = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}

	dh = &curveObj.calibrationData_->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		curveObj.tenorBasisCurves_ = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}

	curveObj.tenorBasisDiscountFactors_ = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_DF2);
	if (!curveObj.listOfCurvesBuilt_.empty() && curveObj.listOfCurvesBuilt_[0] != AQ_NO_DATA)
	{
		for (unsigned int i = 0; i < curveObj.listOfCurvesBuilt_.size(); ++i)
		{
			AQLString thisCurveStaticData;
			AQLString suffixLowerCase = "." + curveObj.listOfCurvesBuilt_[i];
			suffixLowerCase.toLower();

			// set curve type			
			AQLString curveType;
			AQLObjectHolder& yieldData = curveObj.calibrationData_->getYieldData().get();

			if (curveObj.listOfCurvesBuilt_[i] == SWAP)
			{
				curveType = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE);
				if (curveType != AQ_NO_DATA)
				{
					curveObj.calibrationData_->AQLObject::remove(CALIBRATION_DATA_CURVETYPE);
					curveObj.calibrationData_->AQLObject::add(CALIBRATION_DATA_CURVETYPE, new AQLDataString(curveType.toUpper()));

					yieldData.remove(CALIBRATION_DATA_CURVETYPE);
					yieldData.add(CALIBRATION_DATA_CURVETYPE, new AQLDataString(curveType.toUpper()));
				}
			}
			else
			{
				curveType = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffixLowerCase);
				if (curveType != AQ_NO_DATA)
				{
					curveObj.calibrationData_->AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveObj.listOfCurvesBuilt_[i]);
					curveObj.calibrationData_->AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveObj.listOfCurvesBuilt_[i], new AQLDataString(curveType.toUpper()));

					yieldData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveObj.listOfCurvesBuilt_[i]);
					yieldData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveObj.listOfCurvesBuilt_[i], new AQLDataString(curveType.toUpper()));
				}
			}

			if (curveObj.listOfCurvesBuilt_[i] == SWAP)
			{
				if (curveObj.enableCalculation_ || curveObj.targetCurveType_ == STD)
				{
					AQLString aliasCurveNames = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE);
					if (aliasCurveNames != AQ_NO_DATA)
					{
						AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
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

			AQLString marketType = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffixLowerCase).toUpper();
			curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_MARKETTYPE + AQLString("_") + curveObj.listOfCurvesBuilt_[i]);
			curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_MARKETTYPE + AQLString("_") + curveObj.listOfCurvesBuilt_[i], new AQLDataString(marketType));

			if (marketType == AQ_NO_DATA)
			{
				AQLString isBasisStr = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ISBASIS + suffixLowerCase).toUpper();
				AQLString isReadFile = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ISREADFILE + suffixLowerCase).toUpper();
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
			AQLString strInter = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase).toUpper();
			if (strInter != AQ_NO_DATA)
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

				AQLString thisMarketName = curveObj.listOfCurvesBuilt_[i];
				curveObj.tenorBasisMarkets_.push_back(curveObj.listOfCurvesBuilt_[i]);
				AQLString  aliasCurveNames = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != AQ_NO_DATA && (curveObj.enableCalculation_ || curveObj.targetCurveType_ == curveObj.listOfCurvesBuilt_[i]))
				{
					AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
					for (unsigned int j = 0; j < assignedCurves.size(); j++)
					{
						if (curveObj.tenorBasisCurves_.end() == std::find(curveObj.tenorBasisCurves_.begin(), curveObj.tenorBasisCurves_.end(), assignedCurves[j]))
						{
							curveObj.tenorBasisCurves_.push_back(assignedCurves[j]);
						}
					}
					AQLStringVector marketNames = curveObj.listOfCurvesBuilt_[i].toToken('_');
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
						AQLString tenorswapname = dynamic_cast<const AQLDataString &>(curveObj.calibrationData_->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
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

				AQLString thisMarketName = curveObj.listOfCurvesBuilt_[i];
				AQLString aliasCurveNames = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != AQ_NO_DATA)
				{
					AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
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

				AQLString oisCompoundType = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffixLowerCase).toUpper();
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
				AQ_THROW( "Invalid Market Data Type" );
			}

			// set yield curve properties
			thisCurveStaticData = thisCurveStaticData.subString(0, thisCurveStaticData.size() - 2);
			AQLStringVector marketNames = curveObj.listOfCurvesBuilt_[i].toToken('_');
			if (marketNames.size() == 2)
			{
				CurveCalibrationData &yieldCurveProperties = dynamic_cast<CurveCalibrationData &> (objPool.getObject(AQLMarketData::getBaseYieldProName(marketNames[0]), ENCHKTYPE_ISDEFINED).get());
				yieldCurveProperties.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + marketNames[1]);
				yieldCurveProperties.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + marketNames[1], new AQLDataMultiReference()).convertFromString(thisCurveStaticData);
			}
			else
			{
				curveObj.calibrationData_->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + curveObj.listOfCurvesBuilt_[i]);
				curveObj.calibrationData_->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + curveObj.listOfCurvesBuilt_[i], new AQLDataMultiReference()).convertFromString(thisCurveStaticData);
			}
		}
	}
}

// Method to Update the Curve Object with Swap Curve Instruments and Cashflows
void AQLUpdateCurveObject::updateCurveObjectSwapCurveInstrumentsAndCashflows( CurveObject & curveObj ) const
{
	// Current curve type is the type of the curve currently being constructed
	// To be distinguished from the curve type within the for loop below
	AQLString propertyManagerCurveName("");
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
						   AQLString(curveObj.targetCurveType_).toUpper() );
	}
}

// Method to Set-Up Tenor Basis Curve Data
void AQLUpdateCurveObject::updateCurveObjectBasisCurveInstrumentParameters( CurveObject & curveObj ) const
{
	// Currency
	AQLString curveCurrencyLowerCase = curveObj.currency_;
	curveCurrencyLowerCase.toLower();

	// Object Pool
	AQLObjectPool & objPool = curveObj.dataInstance_.getObjectPool();

	// Set-Up Tenor-Basis Curve
	if ( curveObj.listOfCurvesBuilt_.empty() || curveObj.listOfCurvesBuilt_[0] == AQ_NO_DATA)
	{
		AQLString basisCurrency = mpStaticData->getStaticData(KEY_SDE_BASIS_BASE_CURRENCY);
		basisCurrency.toUpper();

		// Set-Up Basis Curve if Basis Currency Provided
		if ( ( AQLDealUtils::getSDECurrencys().size() > 1 )
			 && (curveCurrencyLowerCase.toUpper() != basisCurrency )
			 && (basisCurrency != AQ_NO_DATA) )
		{
			curveCurrencyLowerCase.toLower();
			// if not base currency set basis curve
			AQLString basisFileName = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASISSWAP_FILE);
			AQLFileAccessor basisFile(AQLMarketData::getNumFileName(basisFileName));
			AQLStringMatrix basisDataMtx;
			basisFile.readAllData(MARKET_DATA_DELIMITER, basisDataMtx);
			basisFile.close();

			// get cal and calc spot date
			AQLPriceDataCalendar tenorBasisCalendar;
			AQLString tenorBasisCalendarString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_CALENDAR);
			tenorBasisCalendar.convertFromString(tenorBasisCalendarString);
			AQLDate tenorBasisSpotDate;
			if (curveObj.isSpotStarting_)
			{
				tenorBasisSpotDate.setDate(mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_SPOTDATE).getCString());
			}
			else
			{
				tenorBasisSpotDate = tenorBasisCalendar.getBusinessDay(curveObj.asOfDate_, mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_RESETLAG).getIntValue());
			}
			// get freq
			AQLString tenorBasisFrequencyString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY).toUpper();
			// get daycount
			AQLString tenorBasisDaycountString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT).toUpper();
			// get sliding
			AQLString tenorBasisBusDayAdjString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE).toUpper();
			// get freq
			AQLString tenorBasisIndexFrequencyString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_BASEFREQUENCY).toUpper();
			// get daycount
			AQLString tenorBasisIndexDaycountString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_BASEDAYCOUNT).toUpper();
			// get sliding
			AQLString tenorBasisIndexBusDayAdjString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_BASESLIDINGRULE).toUpper();
			// get calendar
			AQLString tenorBasisIndexCalendarString = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_BASECALENDAR);
			// baseyield name
			AQLString baseCurrency = curveObj.currency_;
			AQLString baseYieldName = PREFIX_YIELD + getSDEAttrName(baseCurrency.toUpper());

			const int basisSize = basisDataMtx.size();
			for (int i = 0; i < basisSize; ++i)
			{
				AQLString term = basisDataMtx[i][0].toUpper();
				double basis = basisDataMtx[i][1].getDoubleValue();

				AQLObject *mktData = new AQLObject();
				AQLString basicCurveName = curveObj.name_ + "_BASIS_" + AQLString(i);
				curveObj.staticDataName_ += basicCurveName + ":";
				curveObj.tenorBasisStaticDataName_ += basicCurveName + ":";
				
				// set name
				mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(basicCurveName);
				// set spot date
				mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(tenorBasisSpotDate));
				// set calendar
				mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(tenorBasisCalendarString);
				// set daycount
				mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(tenorBasisDaycountString);
				// set data type
				mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BASIS);
				// set frequency
				mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(tenorBasisFrequencyString);
				// set slidingrule
				mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(tenorBasisBusDayAdjString);
				// set term
				mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));
				// set rate
				mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(basis / 10000.0) );
				// set base basis info
				// freq
				mktData->add(IR_CALIBRATION_DATA_FREQUENCYBASE, new AQLDataString).convertFromString(tenorBasisIndexFrequencyString);
				// calender
				mktData->add(IR_CALIBRATION_DATA_CALENDARBASE, new AQLPriceDataCalendar()).convertFromString(tenorBasisIndexCalendarString);
				// daycount
				mktData->add(IR_CALIBRATION_DATA_DAYCOUNTBASE, new AQLPriceDataDayCount()).convertFromString(tenorBasisIndexDaycountString);
				// sliding
				mktData->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE, new AQLPriceDataSlidingRule()).convertFromString(tenorBasisIndexBusDayAdjString);

				// set object pool
				objPool.set(basicCurveName, mktData);
			}
			// set base curve reference
			curveObj.calibrationData_->getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(baseYieldName);
		}
	}
}

// Method to Update the Curve Object with Xccy and FX Forward Curve Parameters
void AQLUpdateCurveObject::updateCurveObjectXccyAndFXForwardParameters( CurveObject & curveObj ) const
{
	AQLString suffixLowerCase = (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP) ? "" : "." + curveObj.targetCurveType_;
	suffixLowerCase.toLower();

	AQLString suffixCurveObj = (curveObj.targetCurveType_ == STD || curveObj.targetCurveType_ == SWAP) ? "" : "_" + curveObj.targetCurveType_;

	// set isXccyMarkedToMarket
	bool isXccyMarkedToMarket = false;
	AQLString isXccyMarkedToMarketString = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST + suffixLowerCase).toUpper();
	if (isXccyMarkedToMarketString == "TRUE")
	{
		isXccyMarkedToMarket = true;
	}

	// RENOTIONAL ADJUST HAS ALIAS XCCY MARKED TO MARKET
	curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffixCurveObj);
	curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffixCurveObj, new AQLDataBool(isXccyMarkedToMarket));

	curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET + suffixCurveObj);
	curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET + suffixCurveObj, new AQLDataBool(isXccyMarkedToMarket));

	// IsForwardFX
	curveObj.isForwardFXCurve_ = false;
	AQLString isFwdFXStr = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX);
	if (isFwdFXStr.toUpper() == "TRUE")
	{
		curveObj.isForwardFXCurve_ = true;
	}
}

// Method to Update Curve Data with List of Curves Built
void AQLUpdateCurveObject::updateCurveObjectListOfCurvesBuilt( CurveObject & curveObj ) const
{
	// Property Manager: 'useMarkets' / 'listOfCurvesBuilt' parameter
	// The useMarkets parameter controls which curves indices to calibrate. If a curve index / market is missing the property
	// manager code will try to build any missing curve index. Therefore adding a market to the list, tells this code *NOT* to 
	// calibrate it since the code will think it exists already. 

	AQLString existingMarketsString = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (existingMarketsString == AQ_NO_DATA)
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
void AQLUpdateCurveObject::updateCurveObjectBasisCurveInstrumentCashflows( CurveObject & curveObj ) const
{
	// Set tenorswap convention
	AQLString tenorSwapName = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(curveObj.listOfCurvesBuilt_.begin(), curveObj.listOfCurvesBuilt_.end(), tenorSwapName) == curveObj.listOfCurvesBuilt_.end() && tenorSwapName != AQ_NO_DATA)
	{
		AQLString refData_tenor = "";

		// Set-Up Basis Curve Instruments and Cashflows
		setUpBasisCurveData( curveObj.dataInstance_, refData_tenor, curveObj.asOfDate_, curveObj.currencyLowercase_, tenorSwapName, curveObj.name_, curveObj.isSpotStarting_, *curveObj.calibrationData_, curveObj.targetCurveType_);
		
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		curveObj.calibrationData_->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName);
		curveObj.calibrationData_->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName, new AQLDataMultiReference()).convertFromString(refData_tenor);
	}

	curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(curveObj.swapCurveDiscountFactors_));

	if (curveObj.tenorBasisDiscountFactors_ != AQ_NO_DATA)
	{
		curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new AQLDataString(curveObj.tenorBasisDiscountFactors_));
	}
}

// Method to Update the Curve Object Intrument Priority
void AQLUpdateCurveObject::updateCurveObjectInstrumentPriority( CurveObject & curveObj ) const
{
	// Update parameters that control whether to prioritise swap calibration instruments
	// Whether to generate output forward rates only from swaps or from swaps + futures/FRA
	AQLString prioritizeSwaps = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY);
	curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY);
	if (prioritizeSwaps != AQ_NO_DATA)
	{
		if (prioritizeSwaps.toUpper() == "TRUE")
		{
			curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new AQLDataBool(true));
		}
		else
		{
			curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new AQLDataBool(false));
		}
	}
}

// Update the Curve Object with the Target Curve Discount Factors
void AQLUpdateCurveObject::updateCurveObjectTargetDiscountFactors( CurveObject & curveObj ) const
{
	if (!curveObj.enableCalculation_)
	{
		curveObj.isTargetCurveFound_ = ( curveObj.targetCurveType_ != CURVETYPE_FLOATER ) && ( curveObj.tenorBasisMarkets_.end() == std::find(curveObj.tenorBasisMarkets_.begin(), curveObj.tenorBasisMarkets_.end(), curveObj.targetCurveType_) );
	
		if (curveObj.targetCurveType_ != CURVETYPE_FLOATER)
		{
			AQLString curveMktName;
			if (curveObj.targetCurveType_ != STD)
			{
				curveMktName = AQLString(".") + curveObj.targetCurveType_;
				curveMktName.toLower();
			}

			AQLString targetCurve;
			if (curveObj.isTargetCurveFound_)
			{
				targetCurve = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + curveMktName);
			}
			else
			{
				targetCurve = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + curveMktName);
			}

			AQLString targetDF;
			if (targetCurve != AQ_NO_DATA && targetCurve != "")
			{
				AQLStringVector assignedCurves = targetCurve.toToken(':');
				targetDF = assignedCurves[0];
			}
			else
			{
				AQ_THROW( "Missing TargetCurve Discount Factors" );
			}

			if (curveObj.isTargetCurveFound_)
			{
				curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
				curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(targetDF));
			}
			else
			{
				curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
				curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new AQLDataString(targetDF));
			}
		}
	}
	curveObj.dataInstance_.getReferencePool().completeDependency();
}

// Calibrate the Yield Curve
void AQLUpdateCurveObject::calibrateOISAndSwapCurve( CurveObject & curveObj ) const
{
	if ( curveObj.isForwardFXCurve_ )
	{
		return;
	}

	if ( curveObj.enableCalculation_ || curveObj.isTargetCurveFound_ )
	{
		if (curveObj.isTargetCurveFound_)
		{
			curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new AQLDataBool(true));
		}
		
		// Load the Curve Model Generator
		AQLDataProcedure &curveGenerator = dynamic_cast<AQLDataProcedure &>( curveObj.calibrationData_->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

		// Main Calibration Routine
		// **********************************************
		curveGenerator.calibrateOISAndSwapCurve( curveObj.asOfDate_,
                                                 curveObj.curveCollection_,
                                                 curveObj.curveIndex_ );
		// **********************************************

		curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	}
}

// Set Basis Curve Discount Factors to Basis Curve Object in Object Pool
void AQLUpdateCurveObject::calibrateBasisCurve( CurveObject & curveObj ) const
{
	// 1.	Check If Basis Curve
	// *********************************************************************************
	bool isBasis = false;
	if (!curveObj.tenorBasisCurves_.empty() && !curveObj.isTargetCurveFound_ && curveObj.targetCurveType_ != CURVETYPE_FLOATER)
	{
		isBasis = true;
		curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_BASISDFS, new AQLDataStrings(curveObj.tenorBasisCurves_));
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
			curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			curveObj.calibrationData_->AQLObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new AQLDataBool(true));
		}
		
        // Main Basis Curve Calibration Routine
        curveObj.calibrationData_->setBasisRates();

		curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		curveObj.calibrationData_->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
	}

	AQLString genFloaterName = mpStaticData->getStaticData(curveObj.currencyLowercase_ + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if ( genFloaterName != AQ_NO_DATA && (curveObj.enableCalculation_ || curveObj.targetCurveType_ == CURVETYPE_FLOATER) )
	{
		setUpFloater(curveObj.currencyLowercase_, *curveObj.calibrationData_, genFloaterName);
	}

	// Set Basis Discount Factors df2
	if (curveObj.targetCurveType_ == STD && curveObj.tenorBasisDiscountFactors_ != AQ_NO_DATA)
	{
		curveObj.calibrationData_->setDF2();
	}

	if (!curveObj.tenorBasisMarketQuotes_.empty())
	{
		map<AQLString, map<AQLString, double> >::const_iterator it = curveObj.tenorBasisMarketQuotes_.begin();
		while (it != curveObj.tenorBasisMarketQuotes_.end())
		{
			AQLCurveMarketDataHelpers::restoreSwapRateFromLibor(*curveObj.calibrationData_, it->second, curveObj.currency_, &(it->first));
			++it;
		}
	}

	// 3.	Update Curve Terms & Discount Factors
	// *********************************************************************************
	
	// Flag Curve as Built
	mCurveGenCcyMap[curveObj.currency_] = true;
	
	// Context Key for Curve Data Interface
	AQLString contextKey = curveObj.currency_;
	contextKey.toLower();
	AQLString curveContext = AQLCoreDataService::getContext(contextKey + CONTEXT_KEY_SDE_YIELD);

	if ( curveContext != AQ_NO_DATA )
	{
		AQLDataDoubleMatrix matrix;
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
		dynamic_cast<AQLDataDoubles &>(curveObj.instance_->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<AQLDataDoubles &>(curveObj.instance_->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		curveObj.instance_->remove(IR_CALIBRATION_DATA_DFS2);
		if (isBasisDiscountFactor)
		{
			curveObj.instance_->add(IR_CALIBRATION_DATA_DFS2, new AQLDataDoubles(dfs2));
		}
	}
	else
	{ 
		// Update Curve Data for Curve Index Aliases
		// *********************************************************************************************************
		if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			AQLStringVector dataoutCurves;
			dataoutCurves.push_back(STD);

			const std::map<AQLString, AQLString>& curveAliasList = curveObj.calibrationData_->getAssignedCurveMktMap();
			for (std::map<AQLString, AQLString>::const_iterator it = curveAliasList.begin(); it != curveAliasList.end(); it++)
			{
				if (it->first != STD && std::find(curveObj.listOfCurvesBuilt_.begin(), curveObj.listOfCurvesBuilt_.end(), it->second) != curveObj.listOfCurvesBuilt_.end())
				{
					dataoutCurves.push_back(it->first);
				}
			}
			// floater curve
			if (genFloaterName != AQ_NO_DATA) dataoutCurves.push_back(genFloaterName);
			dataoutCurve(dataoutCurves, *curveObj.instance_, curveObj.name_);
		}
	}

	// Update Curve Data Map
	// curveObj.calibrationData_->getYieldData().get().storeCurveDataMap( curveObj.curveCollection_.c_str(), curveObj.curveIndex_.c_str() );
	return;
}
	
// Load curve object pool data and calibrate with dataInstance as output
void AQLUpdateCurveObject::loadYieldCurveDataAndCalibrate( const AQLString &currency, AQLDataInstance &dataInstance, const AQLString & curveID, const AQLString & marketName ) const
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
AQLUpdateCurveObject::setUpCurveTypeDayCount(CurveCalibrationData &curveCalibrationData, AQLMathYieldCurve &yc) const
{
	// set daycount
	const map<AQLString, AQLString> &assignedCurveMktMap = curveCalibrationData.getAssignedCurveMktMap();
	map<AQLString, AQLString>::const_iterator it_cur = assignedCurveMktMap.begin();
	while (it_cur != assignedCurveMktMap.end())
	{
		AQLPriceDataDayCount dc;
		AQLPriceDataCalendar cal;
		AQLPriceDataSlidingRule sld;
		AQLString accessory;
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
AQLUpdateCurveObject::generateInitialValueArbfree(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

	AQLDataBool tmpAttrB;

	AQLString isSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	AQLObjectPool &objPool = dataInstance.getObjectPool();

	//AQLString yieldName = PREFIX_YIELD + getSDEAttrName(currency);	
	AQLString yieldName = AQLMarketData::getBaseYieldName(currency);

	AQLMathYieldCurve *yc = NULL;
	const AQLObjectHolder ehyc = objPool.getObject(yieldName);
	if (!ehyc.isDefined())
	{
		yc = new AQLMathYieldCurve(&dataInstance);
		objPool.set(yieldName, yc);

	}
	else
	{
		dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		yc = &dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	yc->getName().convertFromString(yieldName);

	AQLString curvePropertiesName = "PRO_" + yieldName;
	CurveCalibrationData *curveCalibrationData = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(curvePropertiesName);
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

	AQLObject *objectPool = NULL;
	AQLString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const AQLObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		objectPool = new AQLObject();
		objectPool->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);
		objPool.set(yieldDataName, objectPool);
	}
	else
	{
		//we must not erase the reset method for only edata
		objectPool = &objPool.getObject(yieldDataName).get();
	}
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool useTenorBasis = false;
	bool useTenorBasisInstruments = false;
	bool isSpotStarting = false;

	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *curveCalibrationData, *objectPool, useTenorBasis, useTenorBasisInstruments, isSpotStarting, true);
	// set isArbFree
	curveCalibrationData->getIsArbFree().set(true);
	bool isXccyMarkedToMarket = false;
	AQLString isXccyMarkedToMarketString = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST).toUpper();
	if (isXccyMarkedToMarketString != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isXccyMarkedToMarketString);
		isXccyMarkedToMarket = tmpAttrB.get();
	}

	AQLStringVector curveNames_6ML;
	AQLStringVector curveNames_3ML;
	AQLStringVector curveNames_DF;
	AQLString curveName_6ML = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MLCURVENAME).toUpper();
	AQLString curveName_DF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DFCURVENAME).toUpper();
	AQLString curveName_3ML = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MLCURVENAME).toUpper();
	if (curveName_6ML != AQ_NO_DATA)
	{
		curveNames_6ML = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MLCURVENAME).toToken(':');
	}
	if (curveName_DF != AQ_NO_DATA)
	{
		curveNames_DF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DFCURVENAME).toToken(':');
	}
	if (curveName_3ML != AQ_NO_DATA)
	{
		curveNames_3ML = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MLCURVENAME).toToken(':');
	}

	objectPool->remove(IR_CALIBRATION_DATA_6MLCURVENAMES);
	objectPool->remove(IR_CALIBRATION_DATA_DFCURVENAMES);
	objectPool->remove(IR_CALIBRATION_DATA_3MLCURVENAMES);
	objectPool->add(IR_CALIBRATION_DATA_6MLCURVENAMES, new AQLDataStrings(curveNames_6ML));
	objectPool->add(IR_CALIBRATION_DATA_DFCURVENAMES, new AQLDataStrings(curveNames_DF));
	objectPool->add(IR_CALIBRATION_DATA_3MLCURVENAMES, new AQLDataStrings(curveNames_3ML));
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new AQLDataBool(isXccyMarkedToMarket));
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new AQLDataBool(isXccyMarkedToMarket));
	curveCalibrationData->setArbFreeCurveName(curveNames_6ML, curveNames_DF, curveNames_3ML);

	// reference data for market pro
	AQLString staticDataName;
	AQLString tenorBasisStaticDataName;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// >>>>>>>>>>>>> for XLL plus
		// setup context key
	AQLString contextKey = currency;
	contextKey.toLower();

	// setup risk info
	bool isrisk = false;
	isrisk |= AQLCoreDataService::getContext(ARG_KEY_OFFICIALRISK) != AQ_NO_DATA ? true : false;
	isrisk |= AQLCoreDataService::getContext(ARG_KEY_FRONTRISK) != AQ_NO_DATA ? true : false;

	AQLString curveContext = AQLCoreDataService::getContext(contextKey + CONTEXT_KEY_SDE_YIELD);

	if (!isrisk&&curveContext != AQ_NO_DATA)
	{
		AQLDataDoubleMatrix matrix;
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
		dynamic_cast<AQLDataDoubles &>(objectPool->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<AQLDataDoubles &>(objectPool->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		if (isBasisDiscountFactor)
		{
			AQLDataDoubles* attrDF2 = NULL;
			AQLDataHolder* df2H = &objectPool->getData(IR_CALIBRATION_DATA_DFS2);
			if (!df2H->isDefined())
			{
				attrDF2 = new AQLDataDoubles();
				objectPool->add(IR_CALIBRATION_DATA_DFS2, attrDF2);
			}
			else
			{
				attrDF2 = dynamic_cast<AQLDataDoubles*>(&df2H->get());
			}
			attrDF2->set(dfs2);
		}

		return;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool enableCalculation = true;
	AQLString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
	if (target != AQ_NO_DATA) enableCalculation = false;

	map<AQLString, map<AQLString, double> > tenorBasisMarketQuotes;
	setUpGenCurveData(dataInstance, staticDataName, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// create Xccy Basis information object
	AQLString refData_XccyBasis = "";
	if (currency == CURRENCY_USD)
	{
		AQLObject *xccyBasis = NULL;
		AQLString nameXccyBasis = yieldDataName + "_" + XCCYBASIS;
		const AQLObjectHolder ehdata = objPool.getObject(nameXccyBasis);
		if (!ehdata.isDefined())
		{
			xccyBasis = new AQLObject();
			objPool.set(nameXccyBasis, xccyBasis);
		}
		else
		{
			objPool.getObject(nameXccyBasis).get().clear();
			xccyBasis = &objPool.getObject(nameXccyBasis).get();
		}

		refData_XccyBasis += nameXccyBasis + ":";
		// set name
		xccyBasis->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameXccyBasis);
		// set data type
		xccyBasis->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BASIS);
		// set curve name
		AQLStringVector usd3mlFloaterInfo = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USD3MLFLOATER).toToken(':');
		if (usd3mlFloaterInfo.size() != 2) AQ_THROW( "No XccyBasis information!" );
		xccyBasis->add(IR_CALIBRATION_DATA_AGTFORECAST, new AQLDataString()).convertFromString(usd3mlFloaterInfo[1]);
		// set foreign ccy info
		AQLString ccy_floater, fYieldDataName;
		if (isSetCurveID == "TRUE")
		{
			AQLObject* fYieldData = &objPool.getObject(usd3mlFloaterInfo[0], ENCHKTYPE_ISDEFINED).get();
			ccy_floater = dynamic_cast<AQLDataString&> ((fYieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
			fYieldDataName = usd3mlFloaterInfo[0];
		}
		else
		{
			AQLString isPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
			if (isPricer == "TRUE")
			{
				AQLObject* yieldData = &objPool.getObject(usd3mlFloaterInfo[0], ENCHKTYPE_ISDEFINED).get();
				ccy_floater = dynamic_cast<AQLDataString&> ((yieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
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
		AQLString curveName_floater = usd3mlFloaterInfo[1];
		curveCalibrationData->getForeignYieldData().convertFromString(fYieldDataName);

		if (isSetCurveID != "TRUE")
		{
			AQLString baseCcy = currency; AQLString domCcy = currency; AQLString forCcy = ccy_floater;
			while (1)
			{
				CurveCalibrationData &ycPro_dccy = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(AQLMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				CurveCalibrationData &ycPro_fccy = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(AQLMarketData::getBaseYieldProName(forCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());

				ycPro_fccy.setAffectingCcy(baseCcy.toUpper());
				const AQLString& affectedCcy_fccy = ycPro_fccy.getAffectedCcy();
				if (affectedCcy_fccy == domCcy)
					AQ_THROW( "The affected currency includes in Affecting currencies!" );

				ycPro_dccy.setAffectedCcy(forCcy.toUpper());

				domCcy = forCcy;
				forCcy = ycPro_fccy.getAffectedCcy();


				if (!forCcy.isDefined()) break;
			}
		}

		if (isXccyMarkedToMarket)
		{
			AQLStringVector baseCcyDFInfo = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASECCYDF).toToken(':');
			if (baseCcyDFInfo.size() != 2) AQ_THROW( "No XccyBasis information!" );
			if (usd3mlFloaterInfo[0] != baseCcyDFInfo[0]) AQ_THROW( "Reference currencies are inconsistent!" );
			xccyBasis->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new AQLDataString()).convertFromString(baseCcyDFInfo[1]);
		}

		// set yield curve pro
		refData_XccyBasis = refData_XccyBasis.subString(0, refData_XccyBasis.size() - 2);
		curveCalibrationData->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + XCCYBASIS);
		curveCalibrationData->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + XCCYBASIS, new AQLDataMultiReference()).convertFromString(refData_XccyBasis);
	}

	tmpCurrency.toLower();
	AQLStringVector listOfCurvesBuilt;
	AQLString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == AQ_NO_DATA)
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(listOfCurvesBuilt);
	if (currency != CURRENCY_USD && find(listOfCurvesBuilt.begin(), listOfCurvesBuilt.end(), XCCYBASIS) == listOfCurvesBuilt.end()) AQ_THROW( "XCCYBASIS is not in generate dfs!!" );
	if (find(listOfCurvesBuilt.begin(), listOfCurvesBuilt.end(), THREESIXBASIS) == listOfCurvesBuilt.end())
	{
		AQLString thisCurveStaticData;
		setUp36BasisDummyData(dataInstance, thisCurveStaticData, asOfDate, tmpCurrency, THREESIXBASIS, yieldDataName, isSpotStarting, *curveCalibrationData);
		// set yield curve pro
		thisCurveStaticData = thisCurveStaticData.subString(0, thisCurveStaticData.size() - 2);
		curveCalibrationData->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + THREESIXBASIS);
		curveCalibrationData->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + THREESIXBASIS, new AQLDataMultiReference()).convertFromString(thisCurveStaticData);
	}
	// set yield curve pro
	staticDataName = staticDataName.subString(0, staticDataName.size() - 2);
	curveCalibrationData->getMarketData().convertFromString(staticDataName);

	AQLDataHolder* dh;
	AQLStringVector swapCurveDiscountFactors;
	dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) swapCurveDiscountFactors = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

	AQLStringVector basisCurves;
	dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) basisCurves = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

	AQLString tenorBasisDiscountFactors = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (!listOfCurvesBuilt.empty() && listOfCurvesBuilt[0] != AQ_NO_DATA)
	{
		//AQLDataStrings &attrGenCurves = dynamic_cast<AQLDataStrings &>(curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings()).get());		
		for (unsigned int i = 0; i < listOfCurvesBuilt.size(); ++i)
		{
			if (listOfCurvesBuilt[i] == SWAP) continue;

			tmpCurrency.toLower();
			AQLString thisCurveStaticData;
			AQLString suffixLowerCase = "." + listOfCurvesBuilt[i];
			suffixLowerCase.toLower();
			AQLString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffixLowerCase).toUpper();
			if (marketType == AQ_NO_DATA)
			{
				AQLString isBasisStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISBASIS + suffixLowerCase).toUpper();
				AQLString isReadFile = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffixLowerCase).toUpper();
				if (isBasisStr == "TRUE") marketType = MARKETTYPE_BASIS;
				else if (isReadFile == "TRUE") marketType = MARKETTYPE_ZERORATE;
				else marketType = MARKETTYPE_SWAP;
			}

			// set up interpolation
			AQLString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase).toUpper();
			if (strInter != AQ_NO_DATA)
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
					AQLString thisMarketName = listOfCurvesBuilt[i];
					AQLString aliasCurveNames = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + thisMarketName.toLower());
					if (aliasCurveNames != AQ_NO_DATA)
					{
						AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
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

				AQLString thisMarketName = listOfCurvesBuilt[i];
				AQLString aliasCurveNames = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + thisMarketName.toLower());
				if (aliasCurveNames != AQ_NO_DATA)
				{
					AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
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

				AQLString oisCompoundType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffixLowerCase).toUpper();
				if (oisCompoundType.toUpper() == "DAILYCOMPOUNDING")
					setUpGenCurveDataOIS(dataInstance, thisCurveStaticData, asOfDate, tmpCurrency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes);
				else
					setUpGenCurveData(dataInstance, thisCurveStaticData, asOfDate, tmpCurrency, listOfCurvesBuilt[i], yieldDataName, isSpotStarting, useTenorBasis, *curveCalibrationData, tenorBasisMarketQuotes);
			}
			else
			{
				AQ_THROW( "Market type is not supported!!" );
			}

			// set yield curve pro
			thisCurveStaticData = thisCurveStaticData.subString(0, thisCurveStaticData.size() - 2);
			curveCalibrationData->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + listOfCurvesBuilt[i]);
			curveCalibrationData->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + listOfCurvesBuilt[i], new AQLDataMultiReference()).convertFromString(thisCurveStaticData);
		}
	}

	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(swapCurveDiscountFactors));

	// generate yield data
	curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
		(curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	dataInstance.getReferencePool().completeDependency();
	modelDataObj.calibrateModel(asOfDate);

	// generate swap curve
	if (swapCurveDiscountFactors.size() != 0 && enableCalculation)
	{
		curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
		for (size_t i = 0; i < swapCurveDiscountFactors.size(); i++)
		{
			curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(swapCurveDiscountFactors[i]));
			modelDataObj = dynamic_cast<AQLDataProcedure &> (curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
			modelDataObj.calibrateModel(asOfDate);
			curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
		}
		curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	}

	//adjust discountfactor
	tmpCurrency.toLower();
	AQLDataBool tmpAttrDF;
	tmpAttrDF.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF));
	if (tmpAttrDF.get())
	{
		const std::map<AQLString, AQLString> &assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (!curveCalibrationData->isBasisCurve(it->first) && (enableCalculation || target == it->second))
			{
				AQLMarketData::adjustDiscountFactor(*objectPool, it->first);
			}
		}
	}

	// basis
	bool isBasis = false;
	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_BASISDFS);
	if (!basisCurves.empty() && enableCalculation)
	{
		isBasis = true;
		curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_BASISDFS, new AQLDataStrings(basisCurves));
	}

	curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
	if (tenorBasisDiscountFactors != AQ_NO_DATA)
	{
		isBasis = true;
		curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new AQLDataString(tenorBasisDiscountFactors));
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

	AQLString genFloaterName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != AQ_NO_DATA || target == CURVETYPE_FLOATER) setUpFloater(tmpCurrency, *curveCalibrationData, genFloaterName);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// >>>>>>>>>>>>> for XLL plus
	if (curveContext != AQ_NO_DATA)
	{
		AQLDataDoubleMatrix matrix;
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
		dynamic_cast<AQLDataDoubles &>(objectPool->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<AQLDataDoubles &>(objectPool->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		objectPool->remove(IR_CALIBRATION_DATA_DFS2);
		if (isBasisDiscountFactor)
		{
			objectPool->add(IR_CALIBRATION_DATA_DFS2, new AQLDataDoubles(dfs2));
		}

		return;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		AQLStringVector dataoutCurves;
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

		const std::map<AQLString, AQLString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second != SWAP && it->second != XCCYBASIS && it->second != THREESIXBASIS) dataoutCurves.push_back(it->first);
		}

		if (genFloaterName != AQ_NO_DATA) dataoutCurves.push_back(genFloaterName);

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
AQLUpdateCurveObject::loadCorrelationDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	AQLString sdeName = getSDEAttrName(currency);
	AQLString type = getCorTye(currency);
	type.toUpper();

	AQLMathCorrelation *corEntity = new AQLMathCorrelation(&dataInstance);
	AQLDataString &corAttrName = corEntity->getName();
	AQLString corName = PREFIX_COR + sdeName;
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
			AQ_THROW( "Volatility input type. only function or data is support" );
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
	@return AQLString
*/
AQLString
AQLUpdateCurveObject::getSDEAttrName(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	return  mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
}

// 
/*!
	@brief get Grid property val

	@param[in] key
	@param[in] curve
	@param[in] grid
	@return AQLString
*/
AQLString
AQLUpdateCurveObject::getGridStaticData(const AQLString &key, const AQLString &curve, const AQLString &grid) const
{
	return etrading::getGridStaticData(mpStaticData, key, curve, grid);
}

// 
/*!
	@brief fuction to set up generate configuration
*/
void
AQLUpdateCurveObject::setUpGenerateConfig
(AQLDataInstance &dataInstance, const AQLDate &asOfDate, const AQLString &currency, AQLMathYieldCurve &yc, CurveCalibrationData &curveCalibrationData,
	AQLObject &ycData, bool &useTenorBasis, bool &useTenorBasisInstruments, bool &isSpotStarting, bool isArbFree, const AQLString& suffixLowerCase, const AQLString& epSuffix) const
{
	AQLString isSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

	AQLString yieldName = yc.getName().get();

	yc.AQLObject::remove(CALIBRATION_DATA_ASOFDATE);
	yc.AQLObject::add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));

	// set interpolation
	AQLString interp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase);
	if (interp == AQ_NO_DATA)
	{
		interp = FN_SPLINEINTERPOLATION_STR;
	}
	yc.getInterpolation().convertFromString(interp.toLower());

	// set daycount
	AQLString dayCount = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT);
	if (dayCount != AQ_NO_DATA)
	{
		yc.getDayCount().convertFromString(dayCount.toUpper());
	}

	// set freq
	AQLString freq = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY);
	if (freq != AQ_NO_DATA)
	{
		yc.getFrequency().convertFromString(freq.toUpper());
	}

	// set sliding rule
	AQLString slidingRule = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE);
	if (slidingRule != AQ_NO_DATA)
	{
		yc.getSlidingRule().convertFromString(slidingRule.toUpper());
	}

	// set cal 
	AQLPriceDataCalendar calY;
	AQLString calYStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_CALENDAR);
	if (calYStr != AQ_NO_DATA)
	{
		calY.convertFromString(calYStr);
		yc.getCalendar() = calY;
	}

	//set basis DF
	AQLString tenorBasisDiscountFactors = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (tenorBasisDiscountFactors != AQ_NO_DATA)
	{
		yc.setBasisCurveType(tenorBasisDiscountFactors);
	}

	// set yield to object pool
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	// set yieldpro to object pool
	// set as of date
	curveCalibrationData.getAsOfDate().set(asOfDate);

	// set curve generator
	curveCalibrationData.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);

	// set interpolation
	AQLString genInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffixLowerCase);
	if (genInterp == AQ_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}
	if (suffixLowerCase.size() == 0)
	{
		curveCalibrationData.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_INTERPOLATION + epSuffix);
		curveCalibrationData.AQLObject::add(CALIBRATION_DATA_INTERPOLATION + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genInterp);
	}
	curveCalibrationData.getInterpolation().convertFromString(genInterp.toLower());

	// interpolation for OIS and STD swaps
	AQLString genYieldGenInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffixLowerCase);
	if (genYieldGenInterp == AQ_NO_DATA)
	{
		if (genInterp != AQ_NO_DATA)
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
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// interpolation for FRA/futures
	AQLString genFutureInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FUTURE_INTERPOLATION + suffixLowerCase);
	if (genFutureInterp == AQ_NO_DATA)
	{
		if (genInterp != AQ_NO_DATA)
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
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONFW + epSuffix);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONFW + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genFutureInterp);
	}

	// interpolation for basis
	AQLString genBasisInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASIS_INTERPOLATION + suffixLowerCase);
	if (genBasisInterp == AQ_NO_DATA)
	{
		if (genInterp != AQ_NO_DATA)
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
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONBS + epSuffix);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONBS + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genBasisInterp);
	}

	// set basis function
	AQLString genBasisFunc = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASISFUNCTION);
	if (genBasisFunc == AQ_NO_DATA)
	{
		genBasisFunc = FN_BASISFUNC2_STR;
	}
	curveCalibrationData.getBasisFunction().setFunction(genBasisFunc.toLower());

	// set daycount
	AQLString genDayCount = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT);
	if (genDayCount != AQ_NO_DATA)
	{
		curveCalibrationData.getDayCount().convertFromString(genDayCount.toUpper());
	}

	// set freq
	AQLString genFreq = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY);
	if (genFreq != AQ_NO_DATA)
	{
		curveCalibrationData.getFrequency().convertFromString(genFreq.toUpper());
	}

	// set sliding rulue
	AQLString genSlidingRule = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE);
	if (genSlidingRule != AQ_NO_DATA)
	{
		curveCalibrationData.getSlidingRule().convertFromString(genSlidingRule.toUpper());
	}

	// set rate priority
	AQLString ratePrio_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_RATEPRIORITY).toUpper();
	if (ratePrio_str != AQ_NO_DATA)
	{
		curveCalibrationData.getData(PRICING_DATA_RATEPRIORITY).convertFromString(ratePrio_str);
	}

	// set tenor adjust
	AQLDataBool tmpAttrB;
	AQLString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + suffixLowerCase).toUpper();
	if (strSwapTenorAdj == AQ_NO_DATA)
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
	AQLDataHolder *ahTemp;
	ahTemp = &curveCalibrationData.AQLObject::getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, NOCHECK);
	if (!ahTemp->isDefined())
	{
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, new AQLDataBool(useTenorBasisInstruments));
	}
	else
	{
		dynamic_cast<AQLDataBool&>(ahTemp->get()).set(useTenorBasisInstruments);
	}
#else	
	curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix);
	curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, new AQLDataBool(useTenorBasisInstruments));
#endif

	if (useTenorBasisInstruments)
	{
		AQLString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
		AQ_THROW_IF( tenorSwapName == AQ_NO_DATA, "Set tenor swap name!" );

		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_TENORSWAPNAME);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_TENORSWAPNAME, new AQLDataString(tenorSwapName));
	}

	/*AQLString yieldDataName = yieldName + "_DATA";
	ycData.remove(CALIBRATION_DATA_NAME);
	ycData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);*/
	AQLString yieldDataName = dynamic_cast<const AQLDataString&> ((ycData.getData(CALIBRATION_DATA_NAME, ISNOTNULL)).get()).get();

	// set asofdate
	ycData.remove(CALIBRATION_DATA_ASOFDATE);
	ycData.add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));

	// set currency
	ycData.remove(IR_CALIBRATION_DATA_CURRENCY);
	ycData.add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString(currency));

	// set interpolation
	ycData.remove(CALIBRATION_DATA_INTERPOLATION);
	ycData.add(CALIBRATION_DATA_INTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(genInterp.toLower());

	// set yield data
	yc.getYieldData().convertFromString(yieldDataName);
	curveCalibrationData.getYieldData().convertFromString(yieldDataName);

	// set base curve reference
	curveCalibrationData.getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(yieldName);

	// set currency in curve ID for excel pricer 
	const AQLString isPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE" && isSetCurveID != "TRUE")
	{
		AQLString curveIndex = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
		const AQLObjectHolder ehdata = objPool.getObject(curveIndex);
		AQLObject *eCurveID = NULL;
		if (!ehdata.isDefined())
		{
			eCurveID = new AQLObject();
			objPool.set(curveIndex, eCurveID);
		}
		else
		{
			objPool.getObject(curveIndex).get().reset();
			eCurveID = &objPool.getObject(curveIndex).get();
		}
		eCurveID->add(CALIBRATION_DATA_NAME, new AQLDataString(curveIndex));
		eCurveID->add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString(currency));
	}

	AQLString strIsAudExtra = mpStaticData->getStaticData(KEY_SDE_YIELD_ISAUDEXTRA);
	if (strIsAudExtra != AQ_NO_DATA)
	{
		AQLDataBool tmpIsAudExtra;
		tmpIsAudExtra.convertFromString(strIsAudExtra);
		useTenorBasis = tmpIsAudExtra.get();
	}

	AQ_THROW_IF( useTenorBasis && useTenorBasisInstruments, "We can not set AUD extra and swap tenor adjust at a same time!" );

	// spotDate use flag
	AQLString isSpotUseStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSPOTUSE);
	if (isSpotUseStr == AQ_NO_DATA)
	{
		isSpotUseStr = "FALSE";
	}
	tmpAttrB.convertFromString(isSpotUseStr);
	isSpotStarting = tmpAttrB.get();

	// set curve exist check
	if (isPricer == "FALSE" || isPricer == AQ_NO_DATA)
	{
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, new AQLDataBool(true));
	}

	// set max term of curve
	AQLString maxTerm = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM);
	if (maxTerm == AQ_NO_DATA)
	{
		maxTerm = mpStaticData->getStaticData(KEY_SIMULATION_TERM_MAX);
	}

	if (maxTerm != AQ_NO_DATA)
	{
		ycData.remove(IR_CALIBRATION_DATA_MAXTERM);
		ycData.add(IR_CALIBRATION_DATA_MAXTERM, new AQLDataString(maxTerm));
		AQLString maxFreq = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERMFREQ);

		if (maxFreq == AQ_NO_DATA)
		{
			maxFreq = mpStaticData->getStaticData(KEY_SIMULATION_TERM_MAX_FREQ);
		}

		AQ_THROW_IF( maxFreq == AQ_NO_DATA, "Max term frequency is needed for curve extrapolation!" );

		ycData.remove(IR_CALIBRATION_DATA_MAXTERMFREQ);
		ycData.add(IR_CALIBRATION_DATA_MAXTERMFREQ, new AQLDataString(maxFreq.toUpper()));
	}
}

// 
/*!
	@brief fuction to set up basis curve data
*/
void
AQLUpdateCurveObject::setUpBasisCurveData(AQLDataInstance &dataInstance, AQLString &staticDataName, const AQLDate &asOfDate, const AQLString &curveCurrencyLowerCase,
	const AQLString &marketName, const AQLString &yieldDataName, bool isSpotStarting, CurveCalibrationData &curveCalibrationData, const AQLString& propertyManagerCurveName, const AQLString* pMktCurrency) const
{
	AQLString mktCurrency = pMktCurrency ? *pMktCurrency : curveCurrencyLowerCase;
	AQLString curveMktName = pMktCurrency ? mktCurrency + "_" + marketName : marketName;
	mktCurrency.toLower();
	curveMktName.toUpper();

	AQLString isPricerStr = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	const bool isPricer = isPricerStr == "TRUE";
	AQLString isSetCurveIDStr = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);
	const bool isSetCurveID = isSetCurveIDStr == "TRUE";
	bool enableCalculation = true;
	AQLString yieldGeneratorTarget = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
	if (yieldGeneratorTarget != AQ_NO_DATA) enableCalculation = false;

	AQLObjectPool &objPool = dataInstance.getObjectPool();

	AQLString mktSuffix = "." + marketName;
	AQLString curveSuffix = pMktCurrency ? "." + mktCurrency + "_" + marketName : "." + marketName;
	mktSuffix.toLower();
	curveSuffix.toLower();

	AQLString isLeg1SpreadStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISLEG1SPREAD + mktSuffix).toUpper();
	// get market rate interpolation
	AQLString interpStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASIS_INTERPOLATION + mktSuffix).toUpper();
	AQLString isTimeInterStr = "TRUE";

	bool isDiscount = true;
	bool isLeg1 = true;
	bool isAgtSpread = true;

	AQLString yieldBasisTarget = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_TARGET + curveSuffix).toUpper();
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

	AQLString fCurve;
	AQLString dCurve;
	AQLString ccy_fCurve;
	AQLString ccy_dCurve;
	AQLString a_fCurve;
	AQLString a_dCurve;
	AQLString ccy_a_fCurve;
	AQLString ccy_a_dCurve;
	// cashflow info
	AQLString c_freqStr;
	AQLString c_freqcpdStr;
	AQLString c_daycStr;
	AQLString c_slidingStr;
	AQLString c_calStr;
	AQLDate c_spotDate;
	int c_resetLag;
	// index info
	AQLString i_freqStr;
	AQLString i_accessaryStr;
	AQLString i_daycStr;
	AQLString i_slidingStr;
	AQLString i_fixcalStr;
	AQLString i_paycalStr;
	int i_resetLag;
	// against cashflow info
	AQLString a_c_freqStr;
	AQLString a_c_freqcpdStr;
	AQLString a_c_daycStr;
	AQLString a_c_slidingStr;
	AQLString a_c_calStr;
	AQLDate a_c_spotDate;
	int a_c_resetLag;
	// against index info
	AQLString a_i_freqStr;
	AQLString a_i_accessaryStr;
	AQLString a_i_daycStr;
	AQLString a_i_slidingStr;
	AQLString a_i_fixcalStr;
	AQLString a_i_paycalStr;
	int a_i_resetLag;

	AQLPriceDataCalendar tmpCal;
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
		curveCalibrationData.remove(CALIBRATION_DATA_BASISCURVEBASE + AQLString("_") + curveMktName);
		curveCalibrationData.AQLObject::add(CALIBRATION_DATA_BASISCURVEBASE + AQLString("_") + curveMktName, new AQLDataString(a_dCurve));

		curveCalibrationData.remove(CALIBRATION_DATA_BASISCURVESECONDARYBASE + AQLString("_") + curveMktName);
		curveCalibrationData.AQLObject::add(CALIBRATION_DATA_BASISCURVESECONDARYBASE + AQLString("_") + curveMktName, new AQLDataString(a_fCurve));

		// Store the forecast curve on the same side as the target discount
		curveCalibrationData.remove(CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE + AQLString("_") + curveMktName);
		curveCalibrationData.AQLObject::add(CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE + AQLString("_") + curveMktName, new AQLDataString(fCurve));
	}
	else
	{
		curveCalibrationData.remove(CALIBRATION_DATA_BASISCURVEBASE + AQLString("_") + curveMktName);
		curveCalibrationData.AQLObject::add(CALIBRATION_DATA_BASISCURVEBASE + AQLString("_") + curveMktName, new AQLDataString(a_fCurve));
	}

	AQLStringMatrix basisDataMtx;
	// rate is virtual or not
	bool isVirtualRate = false;
	AQLString isVirtualRateSr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_ISVIRTUAL + mktSuffix).toUpper();
	if (isVirtualRateSr == "TRUE")
	{
		isVirtualRate = true;
	}
	if (isVirtualRate)
	{
		AQLString tmp = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_VIRTUALGRID + mktSuffix).toUpper();
		AQLStringVector virtualGrid = tmp.toToken(':');
		tmp = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_VIRTUALRATE + mktSuffix).toUpper();
		AQLStringVector virtualRate = tmp.toToken(':');
		// size check
		if (virtualRate.size() != virtualGrid.size())
			AQ_THROW( "virtual grids and virtual rates are not same size!!" );
		for (size_t i = 0; i < virtualRate.size(); ++i)
		{
			AQLStringVector tmp2;
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
		AQLString basisFileName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + mktSuffix);
		
		// 2. If the temp object file name is missing from the irProperties file then create a local name using the same format
		// I think the name used here is irrelevant, but I keep the same format just in case, since who really knows what this crazy code expects
		if( basisFileName.size() == 0 || basisFileName == "NO_DATA" )
		{
			// Remove the '.' (dot) prefix from the mktSuffix and if suffixLowerCase is ill formed then assume 3m6mbasis
			AQLString basisMarket = mktSuffix.size() > 1 ? mktSuffix.subString( 1, mktSuffix.size()-1 ) : "3m6mbasis";
			basisFileName = "data/in/" + mktCurrency + "_yield_basisswap_" + basisMarket + ".csv";
		}

		AQLFileAccessor basisFile(AQLMarketData::getNumFileName(basisFileName));

		basisFile.readAllData(MARKET_DATA_DELIMITER, basisDataMtx);
		basisFile.close();
	}

	AQLStringMatrix adjustValueMtx;
	AQLString adjustFileName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + mktSuffix);
	if (adjustFileName != AQ_NO_DATA)
	{
		AQLFileAccessor adjustValueFile(AQLMarketData::getNumFileName(adjustFileName));
		adjustValueFile.readAllData(MARKET_DATA_DELIMITER, adjustValueMtx);
		adjustValueFile.close();
	}

	if (adjustValueMtx.size() != 0 && adjustValueMtx[0].size() < 2)
	{
		AQ_THROW("Invalid Market Data: Tenor Basis data is missing")
	}
	DoubleArray adjustValue_term, adjustValue;
	for (size_t i = 0; i < adjustValueMtx.size(); i++)
	{
		adjustValue_term.push_back(adjustValueMtx[i][0].getDoubleValue());
		adjustValue.push_back(adjustValueMtx[i][1].getDoubleValue());
	}
	// get adjust value interpolation
	AQLString interpAdjustStr
		= mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_INTERPOLATION + mktSuffix).toUpper();

	// get curve ccy and name
	AQLString tmp_fCurve = fCurve;
	AQLString tmp_dCurve = dCurve;
	AQLString tmp_a_fCurve = a_fCurve;
	AQLString tmp_a_dCurve = a_dCurve;
	convertCurveName(tmp_fCurve, curveCurrencyLowerCase, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(tmp_dCurve, curveCurrencyLowerCase, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(tmp_a_fCurve, curveCurrencyLowerCase, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(tmp_a_dCurve, curveCurrencyLowerCase, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	AQ_THROW_IF( ccy_dCurve != curveCurrencyLowerCase || ccy_fCurve != curveCurrencyLowerCase || ccy_a_dCurve != ccy_a_fCurve, "currency of curve is inconsistent!" );

	// Get curve collection ID
	AQLObjectHolder& yData = curveCalibrationData.getYieldData().get();
	AQLString ydName = yData.getName();
	if (isSetCurveID)
	{
		ydName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	ydName.toUpper();

	// Allow the two curves of the 'against leg' to come from a different curve collection
	// regardless of currency
	bool useAgainstCurveCollection = false;
	AQLStringVector a_fCurves_partitioned = tmp_a_fCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	AQLStringVector a_dCurves_partitioned = tmp_a_dCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (a_fCurves_partitioned.size() == 2 && a_dCurves_partitioned.size() == 2)
	{
		AQLString curveID1 = a_fCurves_partitioned[0].toUpper();
		AQLString curveID2 = a_dCurves_partitioned[0].toUpper();
		if (curveID1 == curveID2)
		{
			AQ_THROW_IF( curveID1 == ydName, "Please don't prefix dependent curves with a curve collection name that is the same as the target curve." );

			useAgainstCurveCollection = true;
		}
	}

	// Allow the two curves of the 'target leg' to come from a different curve collection
	curveCalibrationData.remove(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + AQLString("_") + curveMktName);
	AQLStringVector fCurves_partitioned = tmp_fCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	AQLStringVector dCurves_partitioned = tmp_dCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (fCurves_partitioned.size() == 2 && dCurves_partitioned.size() == 2)
	{
		AQLString curveID1 = fCurves_partitioned[0].toUpper();
		AQLString curveID2 = dCurves_partitioned[0].toUpper();

		if (curveID1 == curveID2)
		{
			AQ_THROW_IF( curveID1 == ydName, "Please don't prefix dependent curves with a curve collection name that is the same as the target curve." );

			curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + AQLString("_") + curveMktName, new AQLDataString(fCurves_partitioned[0]));
		}
		else
		{
			AQ_THROW( "Forecast curve and discount curve of the target leg must have the same curve collection prefix" );
		}
	}

	// against currency
	bool isForeignCcyLeg = false;
	AQLString fYieldDataName;
	curveCalibrationData.remove(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + AQLString("_") + curveMktName);
	if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrencyLowerCase
		|| useAgainstCurveCollection)
	{
		isForeignCcyLeg = true;

		if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrencyLowerCase)
		{
			// Re-set the curveType to be XCCYBASIS
			AQLString curveType = XCCYBASIS;
			curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveMktName);
			curveCalibrationData.AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveMktName, new AQLDataString(curveType.toUpper()));

			AQLObjectHolder& yieldData = curveCalibrationData.getYieldData().get();
			yieldData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveMktName);
			yieldData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveMktName, new AQLDataString(curveType.toUpper()));

			// Also re-set the curveType to be XCCYBASIS within the properties
			mpStaticData->setStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + mktSuffix, XCCYBASIS);
		}

		ccy_a_fCurve.toUpper();
		if (isSetCurveID)
		{
			AQLStringVector tmp_ccy_a_fCurve = tmp_a_fCurve.toToken(':');
			if (tmp_ccy_a_fCurve.size() == 2)
			{
				fYieldDataName = tmp_ccy_a_fCurve[0];
			}
			else
			{
				AQ_THROW( "no foreign yield data!" );
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
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + AQLString("_") + curveMktName, new AQLDataBool(isForeignCcyLeg));

		if (!isSetCurveID)
		{
			AQLString baseCcy = curveCurrencyLowerCase; AQLString domCcy = curveCurrencyLowerCase; AQLString forCcy = ccy_a_fCurve;
			while (1)
			{
				CurveCalibrationData &ycPro_dccy = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(AQLMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				CurveCalibrationData &ycPro_fccy = dynamic_cast<CurveCalibrationData &>
					(objPool.getObject(AQLMarketData::getBaseYieldProName(forCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());

				ycPro_fccy.setAffectingCcy(baseCcy.toUpper());
				const AQLString& affectedCcy_fccy = ycPro_fccy.getAffectedCcy();
				if (affectedCcy_fccy == baseCcy)
					AQ_THROW( "The affected currency includes in Affecting currencies!" );

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
	AQLStringVector basisUseGrid;
	AQLString tmpBasisUseGrid = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + mktSuffix).toUpper();
	if (tmpBasisUseGrid != AQ_NO_DATA)
	{
		basisUseGrid = tmpBasisUseGrid.toToken(':');
	}
	bool isSimuEq = false;
	AQLString strIsSimuEq = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISSIMULTANEOUSEQ + mktSuffix).toUpper();
	if (strIsSimuEq == "TRUE")
	{
		isSimuEq = true;
	}
	bool isSameGridIndex = false;
	AQLString strIsFRNMethod = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISSAMEGRIDINDEX + mktSuffix).toUpper();
	if (strIsFRNMethod == "TRUE")
	{
		isSameGridIndex = true;
	}
	bool isYieldSpreadCalc = true;
	AQLString strIsYieldSpreadCalc = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISYIELDSPREADCALC + mktSuffix).toUpper();
	if (strIsYieldSpreadCalc == "FALSE")
	{
		isYieldSpreadCalc = false;
	}
	bool isFWDInter = false;
	AQLString strFWDInter = FN_SPLINEINTERPOLATION_STR;
	AQLString strIsFWDInter = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISFWDINTER + mktSuffix).toUpper();
	if (strIsFWDInter == "TRUE")
	{
		isFWDInter = true;
		strFWDInter = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDINTERPOLATION + mktSuffix);
	}
	bool isEOMRoll = false;
	AQLString strEOMRoll = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISEOMRLL + mktSuffix).toUpper();
	if (strEOMRoll != AQ_NO_DATA)
	{
		AQLDataBool tmpIsEOMRoll;
		tmpIsEOMRoll.convertFromString(strEOMRoll);
		isEOMRoll = tmpIsEOMRoll.get();
	}
	if (isEOMRoll)
	{
		AQLString strEOMDay = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_EOMDAY + mktSuffix).toUpper();
		if (strEOMDay != AQ_NO_DATA)
		{
			if (c_spotDate.dayOfMonth() != strEOMDay.getIntValue())
			{
				isEOMRoll = false;
			}
		}
		else
		{
			AQLPriceDataCalendar cal;
			cal.convertFromString(c_calStr);
			AQLDate eomDate = cal.getEOMDay(c_spotDate);
			if (c_spotDate != eomDate)
			{
				isEOMRoll = false;
			}
		}
	}
	bool isFwdFX = false;
	AQLString isFwdFXStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX + mktSuffix).toUpper();
	if (isFwdFXStr == "TRUE")
	{
		isFwdFX = true;
	}

	// If spotRateTerm is provided, try to find the Libor instrument with matching tenor and make it one of the basis curve instruments
	AQLString spotRateTerm = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_SPOTRATETERM + mktSuffix).toUpper();
	double spotRate = DBL_MAX;
	if (spotRateTerm != AQ_NO_DATA)
	{
		AQLDataMultiReference marketRef;

		// User-specified source of the Libor fixing
		AQLString fixingSource = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FIXINGSOURCE + mktSuffix);
		if (AQLString(fixingSource).toUpper() == ITSELF)
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

			AQ_REQUIRE(spotRate != DBL_MAX, "#Error: Can't locate spot Libor fixing rate that corresponds to '" + spotRateTerm + "'.");
		}
		else
		{
			if (fixingSource == AQ_NO_DATA)
			{
				marketRef = curveCalibrationData.getMarketData();
			}
			else
			{
				// Use fixingSource to obtain an external yieldCurvePro
				AQLString foreignYcProName = etrading::AQLCurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + fixingSource;
				foreignYcProName.toUpper();

				CurveCalibrationData* foreignYieldCurvePro = NULL;

				const AQLObjectHolder tmpHolder = objPool.getObject(foreignYcProName);
				if (tmpHolder.isDefined())
				{
					foreignYieldCurvePro = &dynamic_cast<CurveCalibrationData&>(objPool.getObject(foreignYcProName).get());
				}
				else
				{
					AQLString err = "#Error: Could not locate the foreign Yield Curve data using '" + foreignYcProName + "'";
					AQ_THROW( err.getCString() );
				}

				marketRef = foreignYieldCurvePro->getMarketData();
			}

			for (unsigned int i = 0; i < marketRef.getSize(); ++i)
			{
				const AQLDataHolder &dh = marketRef.get(i).getData(IR_CALIBRATION_DATA_TERM, NOCHECK);
				if (dh.isDefined() && !dh.isNull())
				{
					AQLString term = dynamic_cast<const AQLDataString &>(dh.get()).get();
					term.toUpper();
					if (spotRateTerm == term)
					{
						spotRate = dynamic_cast<const AQLDataDouble &>(marketRef.get(i).getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();

						AQLObject *mktData = NULL;
						AQLString basicCurveName = yieldDataName + "_LIBOR_" + AQLString(0) + "_" + curveMktName;
						const AQLObjectHolder ehbasis = objPool.getObject(basicCurveName);
						if (!ehbasis.isDefined())
						{
							mktData = new AQLObject();
							objPool.set(basicCurveName, mktData);
						}
						else
						{
							objPool.getObject(basicCurveName).get().clear();
							mktData = &objPool.getObject(basicCurveName).get();
						}

						staticDataName += basicCurveName + ":";

						// set term
						mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));

						// set name
						mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(basicCurveName);

						// set spot rate term
						mktData->add(IR_CALIBRATION_DATA_SPOTRATETERM, new AQLDataString()).convertFromString(term);

						// set data type
						mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_ZERO);

						// Set frequency
						AQLString freq = dynamic_cast<const AQLDataString&> ((marketRef.get(i).getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
						mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freq);

						// Set rate
						mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(spotRate));

						// Set calendar
						const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&> ((marketRef.get(i).getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
						mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(cal));

						// Set sliding rule
						const AQLPriceDataSlidingRule& sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((marketRef.get(i).getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
						mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule(sld));

						// Set date count
						const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&> ((marketRef.get(i).getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
						mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount(dc));

						// Set Spot date
						const AQLDate& spotdate = dynamic_cast<const AQLDataDate&> ((marketRef.get(i).getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
						mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotdate));

						// Set EOM
						const AQLDataHolder* dh = &(marketRef.get(i).getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							bool isEOM = dynamic_cast<const AQLDataBool &>(dh->get()).get();
							mktData->add(IR_CALIBRATION_DATA_ISEOMROLL, new AQLDataBool(isEOM));
						}

						break;
					}
				}
			}
		}
	}

	//get constant for convergence
	double eps = 1.0e-9;
	AQLString strEPS = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_EPSILON + mktSuffix);
	if (strEPS.toUpper() != AQ_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	double grad_eps = 1.0e-15;
	AQLString strGEPS = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_GRADIENTEPSILON + mktSuffix);
	if (strGEPS.toUpper() != AQ_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	double delta = 1.0e-10;
	AQLString strDLT = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_DELTA + mktSuffix);
	if (strDLT.toUpper() != AQ_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	int maxLoop = 1000;
	AQLString strMLP = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_MAXLOOP + mktSuffix);
	if (strMLP.toUpper() != AQ_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}

	// cashflow info
	bool isFXForwardQuotesAsOutright = false;
	bool fwd_isPriceCcy = false;
	AQLString fwd_slidingStr;
	AQLString fwd_calStr;
	AQLString fwd_freqStr;
	AQLDate fwd_spotDate;
	int fwd_resetLag = 0;
	bool fwd_eom = false;
	AQLStringMatrix fwdFXDataMtx;
	unsigned int fwdFXSize = 0;
	AQLStringVector fwdFXUseGrid;
	double fwdFXPipsizeFactor = 1.0; // The Inverted Pipsize Denominator i.e. 10,000 = 4 d.p. FX Forward Pipsize Quote

	// for fwd fx	
	if (isFwdFX)
	{
		// getStaticData has been overloaded to search for the key and then an alias if missing
		AQLString isFXForwardQuotesAsOutrightString = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISRATIO + mktSuffix, mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISFXOUTRIGHT + mktSuffix).toUpper();
		if (isFXForwardQuotesAsOutrightString == "TRUE") isFXForwardQuotesAsOutright = true;
		// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
		if (!isFXForwardQuotesAsOutright)
		{
			fwdFXPipsizeFactor = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_DENOMINATOR + mktSuffix,
															 mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_PIPSIZE + mktSuffix).getDoubleValue(); // Alias Method: First Parameter Takes Priority
			AQLString isPriceCcyStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISPRICECCY + mktSuffix,
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
		AQLString strEOM = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISEOMROLL + mktSuffix).toUpper();
		if (strEOM == "TRUE") fwd_eom = true;
		if (fwd_eom)
		{
			AQLString strEOMDay = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_EOMDAY + mktSuffix).toUpper();
			if (strEOMDay != AQ_NO_DATA)
			{
				if (fwd_spotDate.dayOfMonth() != strEOMDay.getIntValue())
				{
					fwd_eom = false;
				}
			}
			else
			{
				tmpCal.convertFromString(fwd_calStr);
				AQLDate eomDate = tmpCal.getEOMDay(fwd_spotDate);
				if (fwd_spotDate != eomDate)
				{
					fwd_eom = false;
				}
			}
		}

		AQLString fwdFXFileName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FILE + mktSuffix);
		AQLFileAccessor fwdFXFile(AQLMarketData::getNumFileName(fwdFXFileName));
		fwdFXFile.readAllData(MARKET_DATA_DELIMITER, fwdFXDataMtx);
		fwdFXFile.close();
		if (fwdFXDataMtx.size() == 0 || fwdFXDataMtx[0].size() < 2)
		{
			AQ_THROW("Invalid Market Data: FX Forward data is missing")
		}
		fwdFXSize = fwdFXDataMtx.size();

		AQLString tmpfwdFXUseGrid = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID + mktSuffix).toUpper();
		if (tmpfwdFXUseGrid != AQ_NO_DATA)
		{
			fwdFXUseGrid = tmpfwdFXUseGrid.toToken(':');
		}
	}
	if ((basisDataMtx.size() == 0 || basisDataMtx[0].size() < 2) && !isFwdFX)
	{
		AQ_THROW("Invalid Market Data: Basis data is missing")
	}

	// get fwd basis
	bool isFwdBasis = false;
	AQLString isFwdBasisStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDBASIS + mktSuffix);
	if (isFwdBasisStr.toUpper() == "TRUE")
	{
		isFwdBasis = true;
	}

	// optimise performance
	bool optimizePerformance = false;
	AQLString optimizePerformanceStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE + mktSuffix);
	if (optimizePerformanceStr.toUpper() == "TRUE")
	{
		optimizePerformance = true;
	}

	// fast jacobian rebuild
	bool fastRebuild = false;
	AQLString fastRebuildStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD + mktSuffix);
	if (fastRebuildStr.toUpper() == "TRUE")
	{
		fastRebuild = true;
	}


	AQLString fixedRateXccyStartTenor = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FIXEDRATEXCCYSTARTTENOR + mktSuffix);
	if (fixedRateXccyStartTenor == AQ_NO_DATA || fixedRateXccyStartTenor.size() == 0)
	{
		//default to 1Y
		fixedRateXccyStartTenor = "1Y";
	}

	AQLDate fixedRateXccyStartDate = etrading::AQLDateHelpers::getDate(asOfDate, fixedRateXccyStartTenor, true);
	
	double ndfSize = 0.0;
	for (unsigned int j = 0; j < fwdFXSize + basisSize; ++j)
	{
		AQLObject *mktData = NULL;
		AQLString basicCurveName = yieldDataName + "_BASIS_" + AQLString(static_cast<int>(j)) + "_" + curveMktName;
		const AQLObjectHolder ehbasis = objPool.getObject(basicCurveName);
		if (!ehbasis.isDefined())
		{
			mktData = new AQLObject();
			objPool.set(basicCurveName, mktData);
		}
		else
		{
			objPool.getObject(basicCurveName).get().clear();
			mktData = &objPool.getObject(basicCurveName).get();
		}

		staticDataName += basicCurveName + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(basicCurveName);

		// set forward ratio
		if (j < fwdFXSize)
		{
			AQLString fwd_term = fwdFXDataMtx[j][0].toUpper();
			double fwd_fx = fwdFXDataMtx[j][1].getDoubleValue();

			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_FWDFX);
			mktData->add(IR_CALIBRATION_DATA_ISRATIO, new AQLDataBool(isFXForwardQuotesAsOutright));
			mktData->add(IR_CALIBRATION_DATA_ISFXOUTRIGHT, new AQLDataBool(isFXForwardQuotesAsOutright)); // alias for ISRATIO
			
			// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
			if (isFXForwardQuotesAsOutright)
			{
				mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(fwd_fx));
			}
			else
			{
				mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(fwd_fx / fwdFXPipsizeFactor));
				mktData->add(IR_CALIBRATION_DATA_ISPRICECCY, new AQLDataBool(fwd_isPriceCcy));
				mktData->add(IR_CALIBRATION_DATA_ISDOMESTICCURRENCY, new AQLDataBool(fwd_isPriceCcy)); // alias for isPriceCcy
			}
			mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(fwd_term));
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(fwd_slidingStr);
			mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(fwd_calStr);
			mktData->add(IR_CALIBRATION_DATA_SPOTLAG, new AQLDataInt(fwd_resetLag));
			mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(fwd_spotDate));
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(fwd_freqStr);
			mktData->add(IR_CALIBRATION_DATA_ISEOMROLL, new AQLDataBool(fwd_eom));
			if (fwdFXUseGrid.size() != 0 && find(fwdFXUseGrid.begin(), fwdFXUseGrid.end(), fwd_term) == fwdFXUseGrid.end())
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
			}
		}
		else
		{
			// Check if we are pricing a fixed-float xccy swap internally, if yes treat the short end instruments as NDF
			if (fCurve == FIXEDRATE && dCurve == FIXEDRATE)
			{
				const unsigned int j1 = j - fwdFXSize;

				AQLString ndfTerm = basisDataMtx[j1][0].toUpper();
				const AQLDate date = etrading::AQLDateHelpers::getDate(asOfDate, ndfTerm, true);

				// treat swap with less than 1Y as NDF
				if (date < fixedRateXccyStartDate)
				{
					// set term
					mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(ndfTerm));

					// set rate
					double ndfRate = basisDataMtx[j1][1].getDoubleValue();
					mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(ndfRate / 10000.0));

					mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(NDF);
					if (spotRate != DBL_MAX)
					{
						// set spot rate
						mktData->add(IR_CALIBRATION_DATA_SPOTRATE, new AQLDataDouble(spotRate));
						mktData->add(IR_CALIBRATION_DATA_SPOTRATETERM, new AQLDataString(spotRateTerm));
					}
					// cashflow info
					mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(c_calStr);
					mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(c_freqStr);
					mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(c_daycStr);
					mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(c_slidingStr);
					//mktData->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE, new AQLDataDate(c_spotDate));

					ndfSize++;
					continue;
				}
			}

			// set data type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BASIS);
			const unsigned int j2 = j - fwdFXSize;

			// set term
			AQLString term = basisDataMtx[j2][0].toUpper();
			mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));

			// set rate
			double basis = basisDataMtx[j2][1].getDoubleValue();
			mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(basis / 10000.0));

			//set fwd basis
			if (isFwdBasis)
			{
				mktData->add(IR_CALIBRATION_DATA_ISFWDBASIS, new AQLDataBool(isFwdBasis));
				AQ_THROW_IF( basisDataMtx[j2].size() != 5, "Invalid FwdBasis Data: FwdBasis data must have 5 columns when isFwdBasis = TRUE and indicate the forward start- and end-dates." );
				const bool isDate = basisDataMtx[j2][2].toUpper() == "TRUE";
				mktData->add(PRICING_DATA_ISDATE, new AQLDataBool(isDate));
				if (isDate)
				{
					const AQLDate startDate = AQLDate(basisDataMtx[j2][3].getCString());
					mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
					const AQLDate endDate = AQLDate(basisDataMtx[j2][4].getCString());
					mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
				}
				else
				{
					const AQLString startTerm = basisDataMtx[j2][3].toUpper();
					mktData->add(PRICING_DATA_STARTTERM, new AQLDataString(startTerm));
					const AQLString tenor = basisDataMtx[j2][4].toUpper();
					mktData->add(PRICING_DATA_TENOR, new AQLDataString(tenor));
				}
			}
			// is eom roll
			mktData->add(IR_CALIBRATION_DATA_ISEOMROLL, new AQLDataBool(isEOMRoll));
			//grid use
			if (basisUseGrid.size() != 0 && find(basisUseGrid.begin(), basisUseGrid.end(), term) == basisUseGrid.end())
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
			}
		}

		// set optimize performance
		mktData->add(IR_CALIBRATION_DATA_OPTIMIZEPERFORMANCE, new AQLDataBool(optimizePerformance));
		// set fastrebuild
		mktData->add(IR_CALIBRATION_DATA_FASTREBUILD, new AQLDataBool(fastRebuild));
		// set simultaneous equation
		mktData->add(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS, new AQLDataBool(isSimuEq));
		// set epsilon
		mktData->add(IR_CALIBRATION_DATA_EPSILON, new AQLDataDouble(eps));
		// set gradient epsilon
		mktData->add(IR_CALIBRATION_DATA_GRADIENTEPSILON, new AQLDataDouble(grad_eps));
		// set delta
		mktData->add(IR_CALIBRATION_DATA_DELTA, new AQLDataDouble(delta));
		// set maxloop
		mktData->add(IR_CALIBRATION_DATA_MAXLOOP, new AQLDataInt(maxLoop));
		// set issamegridindex
		mktData->add(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX, new AQLDataBool(isSameGridIndex));
		// set isyieldspreadcalc
		mktData->add(IR_CALIBRATION_DATA_ISYIELDSPREADCALC, new AQLDataBool(isYieldSpreadCalc));
		if (spotRate != DBL_MAX)
		{
			// set spot rate
			mktData->add(IR_CALIBRATION_DATA_SPOTRATE, new AQLDataDouble(spotRate));
			mktData->add(IR_CALIBRATION_DATA_SPOTRATETERM, new AQLDataString(spotRateTerm));
		}
		// set fwd interpolation
		mktData->add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, new AQLDataBool(isFWDInter));
		mktData->add(IR_CALIBRATION_DATA_FWDINTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(strFWDInter);
		// curve info
		mktData->add(IR_CALIBRATION_DATA_ISDISCOUNT, new AQLDataBool(isDiscount));
		mktData->add(IR_CALIBRATION_DATA_ISAGTSPREAD, new AQLDataBool(isAgtSpread));
		mktData->add(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, new AQLDataBool()).convertFromString(isTimeInterStr);

		mktData->add(IR_CALIBRATION_DATA_FORECAST, new AQLDataString()).convertFromString(fCurve);
		mktData->add(IR_CALIBRATION_DATA_DISCOUNT, new AQLDataString()).convertFromString(dCurve);
		mktData->add(IR_CALIBRATION_DATA_AGTCURRENCY, new AQLDataString()).convertFromString(ccy_a_fCurve);
		mktData->add(IR_CALIBRATION_DATA_AGTFORECAST, new AQLDataString()).convertFromString(a_fCurve);
		mktData->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new AQLDataString()).convertFromString(a_dCurve);
		// cashflow info
		mktData->add(IR_CALIBRATION_DATA_CASHLETCALENDAR, new AQLPriceDataCalendar()).convertFromString(c_calStr);
		mktData->add(IR_CALIBRATION_DATA_CASHLETFREQUENCY, new AQLDataString()).convertFromString(c_freqStr);
		// set frequency of compounding
		if (c_freqcpdStr != AQ_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND, new AQLDataString()).convertFromString(c_freqcpdStr);
		}
		mktData->add(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(c_daycStr);
		mktData->add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(c_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE, new AQLDataDate(c_spotDate));
		mktData->add(IR_CALIBRATION_DATA_CASHLETRESETLAG, new AQLDataInt(c_resetLag));
		// index info
		if (isSameGridIndex)
		{
			mktData->add(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(c_calStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, new AQLPriceDataCalendar()).convertFromString(c_calStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXFREQUENCY, new AQLDataString()).convertFromString(SIMPLE);
			mktData->add(IR_CALIBRATION_DATA_INDEXACCESSARY, new AQLDataString()).convertFromString(getAccFromFreq(c_freqStr));
			mktData->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(c_daycStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(c_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXRESETLAG, new AQLDataInt(c_resetLag));
		}
		else
		{
			mktData->add(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(i_fixcalStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, new AQLPriceDataCalendar()).convertFromString(i_paycalStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXFREQUENCY, new AQLDataString()).convertFromString(i_freqStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXACCESSARY, new AQLDataString()).convertFromString(i_accessaryStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(i_daycStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(i_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_INDEXRESETLAG, new AQLDataInt(i_resetLag));
		}
		// against cashflow info
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETCALENDAR, new AQLPriceDataCalendar()).convertFromString(a_c_calStr);
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, new AQLDataString()).convertFromString(a_c_freqStr);
		// set frequency of compounding
		if (a_c_freqcpdStr != AQ_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND, new AQLDataString()).convertFromString(a_c_freqcpdStr);
		}
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(a_c_daycStr);
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(a_c_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETSPOTDATE, new AQLDataDate(a_c_spotDate));
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETRESETLAG, new AQLDataInt(a_c_resetLag));
		// against index info
		if (isSameGridIndex)
		{
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(a_c_calStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, new AQLPriceDataCalendar()).convertFromString(a_c_calStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, new AQLDataString()).convertFromString(SIMPLE);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, new AQLDataString()).convertFromString(getAccFromFreq(a_c_freqStr));
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(a_c_daycStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(a_c_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, new AQLDataInt(a_c_resetLag));
		}
		else
		{
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(a_i_fixcalStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, new AQLPriceDataCalendar()).convertFromString(a_i_paycalStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, new AQLDataString()).convertFromString(a_i_freqStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, new AQLDataString()).convertFromString(a_i_accessaryStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(a_i_daycStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(a_i_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, new AQLDataInt(a_i_resetLag));
		}
		// set market rate interpolation
		if (interpStr != AQ_NO_DATA)
		{
			interpStr.toLower();
			AQLPriceDataInterpolation interpAtt;
			interpAtt.convertFromString(interpStr);
			mktData->add(CALIBRATION_DATA_INTERPOLATION, new AQLPriceDataInterpolation(interpAtt));
		}
		// set adjust value interpolation
		if (interpAdjustStr != AQ_NO_DATA)
		{
			interpAdjustStr.toLower();
			AQLPriceDataInterpolation interpAtt;
			interpAtt.convertFromString(interpAdjustStr);
			mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, new AQLPriceDataInterpolation(interpAtt));
			mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new AQLDataDoubles(adjustValue_term));
			mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUE, new AQLDataDoubles(adjustValue));
		}
		// set adjust value
		//mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new AQLDataDoubles(adjustValue_term));
		//mktData->add(IR_CALIBRATION_DATA_ADJUSTVALUE, new AQLDataDoubles(adjustValue));
	}

	//set foreign curve data
	if (pMktCurrency)
	{
		AQLObject& fYieldData = objPool.getObject(fYieldDataName, ENCHKTYPE_ISDEFINED).get();
		fYieldData.remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + marketName);
		fYieldData.add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + marketName, new AQLDataMultiReference()).convertFromString(staticDataName.subString(0, staticDataName.size() - 2));
	}

	AQLString aliasCurveNames = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + curveSuffix);
	if (aliasCurveNames != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = aliasCurveNames.toToken(MULTI_STATIC_DATA_DELIMITER);
		for (size_t i = 0; i < assignedCurves.size(); i++)
		{
			curveCalibrationData.setAssignedCurveMktMap(assignedCurves[i], curveMktName);
		}
	}
	else
	{
		curveCalibrationData.setAssignedCurveMktMap(curveMktName, curveMktName);
	}

	if (interpAdjustStr != AQ_NO_DATA)
	{
		interpAdjustStr.toLower();
		AQLPriceDataInterpolation interpAtt;
		interpAtt.convertFromString(interpAdjustStr);
		if (pMktCurrency)
		{
			AQLObject& fYieldData = objPool.getObject(fYieldDataName, ENCHKTYPE_ISDEFINED).get();
			fYieldData.remove(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION);
			fYieldData.remove(IR_CALIBRATION_DATA_ADJUSTVALUETERM);
			fYieldData.remove(IR_CALIBRATION_DATA_ADJUSTVALUE);
			fYieldData.add(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, new AQLPriceDataInterpolation(interpAtt));
			fYieldData.add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new AQLDataDoubles(adjustValue_term));
			fYieldData.add(IR_CALIBRATION_DATA_ADJUSTVALUE, new AQLDataDoubles(adjustValue));
		}
		else
		{
			AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
			yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION);
			yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUETERM);
			yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUE);
			yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, new AQLPriceDataInterpolation(interpAtt));
			yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new AQLDataDoubles(adjustValue_term));
			yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUE, new AQLDataDoubles(adjustValue));
		}
	}

	// get fwd swap
	bool areSwapsForwardStarting = false;
	AQLDataBool tmpAttrB;
	AQLString isFwdSwap_str = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + mktSuffix);
	if (isFwdSwap_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		areSwapsForwardStarting = tmpAttrB.get();
	}

	//FRA Data
	bool useTenorBasis = false;
	AQLString staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	AQLString suffix_data = "_" + marketName;

	AQLString curveName("");
	if (propertyManagerCurveName != STD)
	{
		curveName = "." + propertyManagerCurveName;
	}
	curveName.toLower();
	AQLString curveType = mpStaticData->getStaticData(curveCurrencyLowerCase + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + curveName);

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
AQLUpdateCurveObject::setUp36BasisDummyData(AQLDataInstance &dataInstance, AQLString &staticDataName, const AQLDate &asOfDate, const AQLString &currency,
	const AQLString &marketName, const AQLString &yieldDataName, bool isSpotStarting,
	CurveCalibrationData &curveCalibrationData) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	AQLString suffixLowerCase = "." + marketName;
	suffixLowerCase.toLower();

	// get swap market
	AQLString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE);
	AQLFileAccessor swapFile(AQLMarketData::getNumFileName(swapFileName));
	AQLStringMatrix swapDataMtx;
	swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
	swapFile.close();
	if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2)
	{
		AQ_THROW("Invalid Market Data: Swap data is missing")
	}
	const int swapSize = swapDataMtx.size();
	AQLString daycSStr_float = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT).toUpper();
	AQLString interpSStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_INTERPOLATION).toLower();
	AQLPriceDataCalendar cal;
	AQLString calStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR).toLower();
	cal.convertFromString(calStr);
	AQLDate spotDate;
	int resetLag;
	if (isSpotStarting)
	{
		spotDate.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SPOTDATE).getCString());
	}
	else
	{
		AQLString reseLag_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG);
		resetLag = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG).getIntValue();
		if ( reseLag_str == AQ_NO_DATA )
		{
			AQ_THROW( "Reset Lag is not set !!" );
		}
		else
		{
			spotDate = cal.getBusinessDay(asOfDate, resetLag);
		}
	}
	// get sliding
	AQLString slidingStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE).toUpper();

	for (int j = 0; j < swapSize; ++j)
	{
		AQLString term = swapDataMtx[j][0].toUpper();
		double basis = 0.;

		AQLObject *mktData = NULL;
		AQLString basicCurveName = yieldDataName + "_" + marketName + "_" + AQLString(static_cast<int>(j));
		const AQLObjectHolder ehbdummy = objPool.getObject(basicCurveName);
		if (!ehbdummy.isDefined())
		{
			mktData = new AQLObject();
			objPool.set(basicCurveName, mktData);
		}
		else
		{
			objPool.getObject(basicCurveName).get().clear();
			mktData = &objPool.getObject(basicCurveName).get();
		}
		staticDataName += basicCurveName + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(basicCurveName);
		// curve info
		mktData->add(IR_CALIBRATION_DATA_ISAGTSPREAD, new AQLDataBool(false));
		// frequency info
		mktData->add(IR_CALIBRATION_DATA_CASHLETFREQUENCY, new AQLDataString()).convertFromString(QUARTERLY);
		// against frequency info
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, new AQLDataString()).convertFromString(SEMI_ANNUAL);
		// index info
		mktData->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycSStr_float);
		// against index info
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycSStr_float);
		// calendar
		mktData->add(IR_CALIBRATION_DATA_CASHLETCALENDAR, new AQLPriceDataCalendar(cal));
		// spot date
		mktData->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE, new AQLDataDate(spotDate));
		// slidingrule
		mktData->add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingStr);
		// market rate interpolation
		AQLPriceDataInterpolation interpSAtt;
		interpSAtt.convertFromString(interpSStr);
		mktData->add(CALIBRATION_DATA_INTERPOLATION, new AQLPriceDataInterpolation(interpSAtt));
		// set data type
		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BASIS);
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));
		// set rate
		mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(basis / 10000.0));
	}
}

// Set-up Base Curve Data
void AQLUpdateCurveObject::setUpGenCurveData(AQLDataInstance &dataInstance, AQLString &staticDataName, const AQLDate &asOfDate, const AQLString &currency,
	const AQLString &marketName, const AQLString &yieldDataName, bool isSpotStarting, bool useTenorBasis, CurveCalibrationData &curveCalibrationData,
	MarketDataTenorQuotes &tenorBasisMarketQuotes, const AQLString& curveName) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString staticDataSuffix = "";

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


	AQLString suffix_data = "";
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
	AQLString isFwdFXStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX + staticDataSuffix);
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
	AQLString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix);
	if (dfCurveName == AQ_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new AQLDataString(dfCurveName));
	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new AQLDataString(dfCurveName));

	// get fwd swap
	bool areSwapsForwardStarting = false;
	AQLDataBool tmpAttrB;
	AQLString isFwdSwap_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + staticDataSuffix);
	if (isFwdSwap_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		areSwapsForwardStarting = tmpAttrB.get();
	}

	// Do we always recalculate the dynamic linear spline join date?
	AQLString alwaysCalcJoinDate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE + staticDataSuffix);
	curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	if (alwaysCalcJoinDate != AQ_NO_DATA && alwaysCalcJoinDate.size() != 0)
	{
		if (alwaysCalcJoinDate.toUpper() == "TRUE")
		{
			curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(true));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(true));
		}
		else
		{
			curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(false));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(false));
		}
	}

	//Set stateVariable
	AQLString stateVariableStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_STATEVARIABLE + staticDataSuffix);
	curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_STATEVARIABLE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_STATEVARIABLE + suffix_data);
	if (stateVariableStr != AQ_NO_DATA && stateVariableStr.size() != 0)
	{
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_STATEVARIABLE + suffix_data, new AQLDataString(stateVariableStr));
		yldEntity.add(IR_CALIBRATION_DATA_STATEVARIABLE + suffix_data, new AQLDataString(stateVariableStr));
	}

	// IsFutureUse flag
	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();
	bool isFutureUse = false;
	AQLString tmpFutureStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + staticDataSuffix);
	if (tmpFutureStr != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(tmpFutureStr);
		isFutureUse = tmpAttrB.get();
	}

	// Hybrid Spline interpolation cut off date
	AQLString inputInterpJoinDateStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATIONJOINDATE + staticDataSuffix);
	curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	if (inputInterpJoinDateStr != AQ_NO_DATA && inputInterpJoinDateStr.size() != 0)
	{
		std::string calKey = isFutureUse ? STATIC_DATA_KEY_YIELD_FUTURE_CALENDAR : STATIC_DATA_KEY_YIELD_FRA_CALENDAR;
		std::string spotDateKey = isFutureUse ? STATIC_DATA_KEY_YIELD_FUTURE_SPOTDATE : STATIC_DATA_KEY_YIELD_FRA_SPOTDATE;
		std::string spotLagKey = isFutureUse ? STATIC_DATA_KEY_YIELD_FUTURE_RESETLAG : STATIC_DATA_KEY_YIELD_FRA_RESETLAG;

		AQLPriceDataCalendar cal;
		AQLString calStr = mpStaticData->getStaticData(currency + calKey + staticDataSuffix);
		cal.convertFromString(calStr);

		AQLDate spotDate;
		if (isSpotStarting)
		{
			spotDate.setDate(mpStaticData->getStaticData(currency + spotDateKey + staticDataSuffix).getCString());
		}
		else
		{
			spotDate = cal.getBusinessDay(asOfDate, mpStaticData->getStaticData(tmpCurrency + spotLagKey + staticDataSuffix).getIntValue());
		}

		//Use NO_CHANGE as businessDayAdj
		AQLDate inputInterpolationJoinDate = etrading::validateDateOrTenor(spotDate, inputInterpJoinDateStr, "NO_CHANGE", calStr, "Invalid joinDate");

		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputInterpolationJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputInterpolationJoinDate));
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
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data, new AQLDataBool(isFutureUse));
	}

	if (isFutureUse)
	{
		AQLString futureFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix);
		AQLFileAccessor futureFile(AQLMarketData::getNumFileName(futureFileName));
		AQLStringMatrix futureDataMtx;
		futureFile.readAllData(MARKET_DATA_DELIMITER, futureDataMtx);
		futureFile.close();
		// use grid
		AQLStringVector futureUseGrid;
		AQLString tmpFutureUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + staticDataSuffix).toUpper();
		if (tmpFutureUseGrid != AQ_NO_DATA)
		{
			futureUseGrid = tmpFutureUseGrid.toToken(':');
		}
		else
		{
			AQLString tmpUseGridNum = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRIDNUM + staticDataSuffix).toUpper();
			if (tmpUseGridNum != AQ_NO_DATA)
			{
				int useGridNum = tmpUseGridNum.getIntValue();
				if (useGridNum == 0)
				{
					futureUseGrid.push_back(AQ_NO_DATA);
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
		AQLPriceDataCalendar calF;
		AQLString calFStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_CALENDAR + staticDataSuffix);
		calF.convertFromString(calFStr);
		// get freq
		AQLString freqFStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FREQUENCY + staticDataSuffix).toUpper();
		// get daycount
		AQLString daycFStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_DAYCOUNT + staticDataSuffix).toUpper();
		// get sliding
		AQLPriceDataSlidingRule slidingF;
		AQLString slidingFStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_SLIDINGRULE + staticDataSuffix).toUpper();
		slidingF.convertFromString(slidingFStr);
		//spot lag
		AQLString spotLagF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_RESETLAG + staticDataSuffix);
		//is convexity adjust precise
		bool isConvAdjPrecise = false;
		AQLString isConvAdjPrecise_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_ISCONVADJPRECISE + staticDataSuffix);
		if (isConvAdjPrecise_str != AQ_NO_DATA)
		{
			tmpAttrB.convertFromString(isConvAdjPrecise_str);
			isConvAdjPrecise = tmpAttrB.get();
		}
		// get mean reversion
		double meanReversion = 0.0;
		AQLString meanReversion_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_MEANREVERSION + staticDataSuffix);
		if (meanReversion_str != AQ_NO_DATA)
		{
			AQLDataDouble tmpAttrDouble;
			tmpAttrDouble.convertFromString(meanReversion_str);
			meanReversion = tmpAttrDouble.get();
		}

		// get applyTension
		bool applyTensionFutures = false;
		AQLString applyTensionFuturesStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_APPLYTENSION + staticDataSuffix).toUpper();
		if (applyTensionFuturesStr != AQ_NO_DATA)
		{
			AQLDataBool tmpApplyTensionFutures;
			tmpApplyTensionFutures.convertFromString(applyTensionFuturesStr);
			applyTensionFutures = tmpApplyTensionFutures.get();
		}
		
		// *** CONVEXITY QUOTE TYPE *** 
		// ----------------------------------------------------------

		// get UseConvexAdjustment (ConvexityQuotedAsPrice), legacy parameter replaced by ConvexityQuoteType
		bool useConvexAdjustment = false;
		bool convexityQuotedAsVol = true;
		AQLString useConvexAdjustmentStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USECONVEXADJUSTMENT + staticDataSuffix).toUpper();
		if (useConvexAdjustmentStr != AQ_NO_DATA)
		{
			AQLDataBool tmpUseConvexAdjustment;
			tmpUseConvexAdjustment.convertFromString(useConvexAdjustmentStr);
			useConvexAdjustment = tmpUseConvexAdjustment.get();
			convexityQuotedAsVol = !useConvexAdjustment;
		}

		// get convexityQuoteType *** Alias for UseConvexAdjustment (convexityQuoteAsPrice) ***
		AQLString convexityQuoteTypeStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_CONVEXITYQUOTETYPE + staticDataSuffix).toUpper();
		if (convexityQuoteTypeStr != AQ_NO_DATA)
		{
			convexityQuoteTypeStr.toUpper();
			AQ_REQUIRE( convexityQuoteTypeStr == "VOL" || convexityQuoteTypeStr == "PRICE", "Invalid Futures Convexity Quote Type: ConvexityQuoteType must be VOL or PRICE" )
			convexityQuotedAsVol = ( convexityQuoteTypeStr == "VOL" ) ? true : false;

			// TODO: Property Manager does not allow us to clear parameters once set ... hence the below fails ... prioritize the convexityQuoteType parameter for now
			// if ( useConvexAdjustmentStr != AQ_NO_DATA )
			// {
			// 	AQ_THROW("Invalid Futures Convexity Parameter: Cannot use ConvexityQuoteType and UseConvexAdjustment (ConvexityQuotedAsPrice) parameters at the same time.")
			// }
		}
		
		// Important Update ConvexityQuoteString for Object Pool Setting
		convexityQuoteTypeStr = convexityQuotedAsVol ? "VOL" : "PRICE";

		// ----------------------------------------------------------

		// Curve controls
		bool smoothShortEnd = true;
		AQLString strSmoothShortEnd = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_SMOOTHSHORTEND + staticDataSuffix);
		if (strSmoothShortEnd != AQ_NO_DATA)
		{
			AQLDataBool tmpSmoothShortEnd;
			tmpSmoothShortEnd.convertFromString(strSmoothShortEnd);
			smoothShortEnd = tmpSmoothShortEnd.get();
		}

		// get tensionGap
		int tensionGapFutures = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_TENSIONGAP + staticDataSuffix).getIntValue();

		// Default to RATE
		AQLString futureSerialCalcType = etrading::toString(etrading::SERIAL_FUTURES_BY_RATE);
		auto dh = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_SERIAL_CALC_TYPE + staticDataSuffix);
		if (dh != AQ_NO_DATA)
		{
			futureSerialCalcType = dh.getCString();
		}

		const int futureSize = futureDataMtx.size();
		for (int i = 0; i < futureSize; ++i)
		{
			AQ_THROW_IF( futureDataMtx[i].size() != 5 && futureDataMtx[i].size() != 3, "Future File format is wrong" );
			AQLString term;
			AQLDate startDate, endDate;
			double futurePrice(0.0), rate(0.0), futureVol(0.0), convexAdj(0.0);

			if (futureDataMtx[i].size() == 5)
			{
				term = futureDataMtx[i][0].toUpper();
				startDate = AQLDate(futureDataMtx[i][1].getCString());
				endDate = AQLDate(futureDataMtx[i][2].getCString());
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
				startDate = etrading::AQLDateHelpers::getIMMDateFromTerm(asOfDate, term);
				startDate = etrading::AQLDateHelpers::getDate(startDate, "0d", slidingF, &calF, true);
				endDate = etrading::AQLDateHelpers::getDate(startDate, "3M", slidingF, &calF, true);
				int mm = endDate.monthOfYear();
				int yy = endDate.yearOfEra();
				endDate = etrading::AQLDateHelpers::getIMMDate(yy, mm, true);

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
				AQ_THROW( "Future File format is wrong" );
			}

			AQLObject *mktData = NULL;
			AQLString nameF = yieldDataName + "_FUTURE_" + AQLString(i) + suffix_data;
			const AQLObjectHolder ehfuture = objPool.getObject(nameF);
			if (!ehfuture.isDefined())
			{
				mktData = new AQLObject();
				objPool.set(nameF, mktData);
			}
			else
			{
				objPool.getObject(nameF).get().clear();
				mktData = &objPool.getObject(nameF).get();
			}
			staticDataName += nameF + ":";
			// set name
			mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameF);
			// set term
			mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(term);
			// set calendar
			mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(calFStr);
			// set daycount
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycFStr);
			// set data type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_FUTURE);
			// set slidingrule
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingFStr);
			// set start date
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			// set end date
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
			// set rate
			mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate));
			// is convexity adjsut presice
			mktData->add(IR_CALIBRATION_DATA_ISCONVADJPRECISE, new AQLDataBool(isConvAdjPrecise));
			// set mean reversion
			mktData->add(IR_CALIBRATION_DATA_MEANREVERSION, new AQLDataDouble(meanReversion));
			// set vol
			mktData->add(PRICING_DATA_FUTUREVOLATILITY, new AQLDataDouble(futureVol));
			
			// set convexAdj - The numerical convexity value
			mktData->add(PRICING_DATA_CONVEXADJUSTMENT, new AQLDataDouble(convexAdj));
			
			// Convexity Quoted as Price (useConvexAdj) or as Vol
			// set the replacement convexityQuotedAsVol alias for legacy parameter useConvexAdj, which is deprecated
			mktData->add(PRICING_DATA_CONVEXITYQUOTETYPE, new AQLDataString(convexityQuoteTypeStr));
			
			// set tension
			mktData->add(IR_CALIBRATION_DATA_APPLYTENSION, new AQLDataBool(applyTensionFutures));
			// set tension gap
			mktData->add(IR_CALIBRATION_DATA_TENSIONGAP, new AQLDataInt(tensionGapFutures));
			// use grid
			if (futureUseGrid.size() != 0 && find(futureUseGrid.begin(), futureUseGrid.end(), term) == futureUseGrid.end())
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
			}
			// smooth short end of curve
			mktData->add(IR_CALIBRATION_DATA_SMOOTHSHORTEND, new AQLDataBool(smoothShortEnd));
			// serial calc type
			mktData->add(IR_CALIBRATION_DATA_FRAFUTURE_SERIAL_CALC_TYPE, new AQLDataString(futureSerialCalcType));
		}
	}

	// swap
	AQLString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	AQLFileAccessor swapFile(AQLMarketData::getNumFileName(swapFileName));
	AQLStringMatrix swapDataMtx;
	if (!isFwdFX)
	{
		swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
		swapFile.close();
	}

	if ((swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2) && !isFwdFX)
	{
		AQ_THROW("Invalid Market Data: Swap data is missing")
	}

	// get cal and calc spot date
	AQLPriceDataCalendar calS;
	AQLString calSStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + staticDataSuffix);
	calS.convertFromString(calSStr);

	AQLDate spotDateS;
	int resetLag;
	if (isSpotStarting)
	{
		spotDateS.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SPOTDATE + staticDataSuffix).getCString());
	}
	else
	{
		AQLString resetLag_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG + staticDataSuffix);
		resetLag = resetLag_str.getIntValue();
		if ( resetLag_str == AQ_NO_DATA )
		{
			AQ_THROW( "Reset Lag is not set !!" );
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
				const AQLString liborFixingCalendar = mpStaticData->getStaticData( currency + STATIC_DATA_KEY_YIELD_LIBOR_CALENDAR + staticDataSuffix );

				const int tenorPosition = resetLag_str.findString('D');
				if ( tenorPosition == -1 )
				{
					// Add the 'D' tenor if it is missing
					resetLag_str += 'D';
				}
				const AQLString businessDayAdjustment("FOLLOWING"); // Specify this to request a Business-day adjustment
				const AQLString rollConvention;
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
	AQLString strIsTimeInterSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISTIMEINTERPOLATION + staticDataSuffix).toUpper();
	if (strIsTimeInterSW != AQ_NO_DATA)
	{
		AQLDataBool tmpIsTimeInterSW;
		tmpIsTimeInterSW.convertFromString(strIsTimeInterSW);
		isTimeInterSW = tmpIsTimeInterSW.get();
	}
	AQLString strNRSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISNEWTONRAPHSON + staticDataSuffix).toUpper();
	if (strNRSW != AQ_NO_DATA)
	{
		AQLDataBool tmpIsNRSW;
		tmpIsNRSW.convertFromString(strNRSW);
		isNRSW = tmpIsNRSW.get();
	}
	AQLString strSimuEQSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISSIMULTANEOUSEQ + staticDataSuffix).toUpper();
	if (strSimuEQSW != AQ_NO_DATA)
	{
		AQLDataBool tmpIsSimuEQSW;
		tmpIsSimuEQSW.convertFromString(strSimuEQSW);
		isSimuEQSW = tmpIsSimuEQSW.get();
	}
	AQLString strEOMRollSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISEOMRLL + staticDataSuffix).toUpper();
	if (strEOMRollSW != AQ_NO_DATA)
	{
		AQLDataBool tmpIsEOMRollSW;
		tmpIsEOMRollSW.convertFromString(strEOMRollSW);
		isEOMRollSW = tmpIsEOMRollSW.get();
	}
	if (isEOMRollSW)
	{
		AQLString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_EOMDAY + staticDataSuffix).toUpper();
		if (strEOMDay != AQ_NO_DATA)
		{
			if (spotDateS.dayOfMonth() != strEOMDay.getIntValue())
			{
				isEOMRollSW = false;
			}
		}
		else
		{
			const AQLDate eomDate = calS.getEOMDay(spotDateS);
			if (spotDateS != eomDate)
			{
				isEOMRollSW = false;
			}
		}
	}

	AQLString strFWDInter = FN_SPLINEINTERPOLATION_STR;
	AQLString strIsFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISFWDINTER + staticDataSuffix).toUpper();
	if (strIsFWDInter == "TRUE")
	{
		isFWDInter = true;
		strFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FWDINTERPOLATION + staticDataSuffix);
	}

	AQLString optimizeMethod = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_OPTIMIZEMETHOD + staticDataSuffix).toUpper();

	//get constant for convergence
	double eps = 1.0e-9;
	AQLString strEPS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_EPSILON + staticDataSuffix);
	if (strEPS.toUpper() != AQ_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	double grad_eps = 1.0e-15;
	AQLString strGEPS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_GRADIENTEPSILON + staticDataSuffix);
	if (strGEPS.toUpper() != AQ_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	double delta = 1.0e-10;
	AQLString strDLT = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DELTA + staticDataSuffix);
	if (strDLT.toUpper() != AQ_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	int maxLoop = 1000;
	AQLString strMLP = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_MAXLOOP + staticDataSuffix);
	if (strMLP.toUpper() != AQ_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	AQLString swapTenor("");
	AQLString strSwapTenor = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTENOR + staticDataSuffix);
	if (strSwapTenor.toUpper() != AQ_NO_DATA)
	{
		swapTenor = strSwapTenor;
	}
	AQLString swapType("LIBOR");
	AQLString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTYPE + staticDataSuffix);
	if (strSwapType.toUpper() != AQ_NO_DATA)
	{
		swapType = strSwapType;
	}

	// optimise performance
	bool optimizePerformance = false;
	AQLString optimizePerformanceStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE + staticDataSuffix);
	if (optimizePerformanceStr.toUpper() == "TRUE")
	{
		optimizePerformance = true;
	}

	// fast jacobian rebuild
	bool fastRebuild = false;
	AQLString fastRebuildStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD + staticDataSuffix);
	if (fastRebuildStr.toUpper() == "TRUE")
	{
		fastRebuild = true;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	AQLString isResetSkipStr = mpStaticData->getStaticData(KEY_YIELD_IS_AUD_RESET_SKIP);
	if (isResetSkipStr != AQ_NO_DATA)
	{
		AQLDataBool tmpIsResetSkip;
		tmpIsResetSkip.convertFromString(isResetSkipStr);
		resetFlg = !tmpIsResetSkip.get();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const int swapSize = swapDataMtx.size();
	// use grid
	AQLStringVector swapUseGrid;
	AQLString tmpSwapUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + staticDataSuffix).toUpper();
	if (tmpSwapUseGrid != AQ_NO_DATA)
	{
		swapUseGrid = tmpSwapUseGrid.toToken(':');
	}
	// set tenor adjust
	bool useTenorBasisInstruments = false;
	AQLString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + staticDataSuffix).toUpper();
	if (strSwapTenorAdj != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(strSwapTenorAdj);
		useTenorBasisInstruments = tmpAttrB.get();
	}
	for (int j = 0; j < swapSize; ++j)
	{
		AQLString term = swapDataMtx[j][0].toUpper();
		AQLString tmpCurrency = currency;
		if (useTenorBasis && tmpCurrency.toUpper() == CURRENCY_AUD &&
			term.findString("Y") == static_cast<int>(term.size() - 1) &&
			term.subString(0, term.size() - 2).getIntValue() > 3 && resetFlg)
		{
			// set yield curve pro
			if (marketName != SWAP)
			{
				curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
				curveCalibrationData.AQLObject::add(CALIBRATION_DATA_MARKETDATA + suffix_data, new AQLDataMultiReference()).
					convertFromString(staticDataName.subString(0, staticDataName.size() - 2));
			}
			else
			{
				curveCalibrationData.getMarketData().convertFromString(staticDataName.subString(0, staticDataName.size() - 2));
			}

			// generate yield data
			AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
				(curveCalibrationData.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
			curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(marketName));

			dataInstance.getReferencePool().completeDependency();
			modelDataObj.calibrateModel(asOfDate);

			//reset market data as libor
			AQLCurveMarketDataHelpers::resetMarketDataUsingLibor(curveCalibrationData, currency, &marketName);
			resetFlg = false;
			if (marketName != SWAP)
			{
				curveCalibrationData.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
			}
		}

		double rate = swapDataMtx[j][1].getDoubleValue();
		// get freq
		AQLString freqSStr = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY, staticDataSuffix, term).toUpper();
		AQLString freqSStr_Fix = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + staticDataSuffix).toUpper();
		// get frequency of floating leg
		AQLString baseFreqSStr_float;
		baseFreqSStr_float = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT + staticDataSuffix).toUpper();
		if (baseFreqSStr_float == AQ_NO_DATA) baseFreqSStr_float = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + staticDataSuffix).toUpper();
		AQLString freqSStr_float = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT, staticDataSuffix, term).toUpper();
		// get frequency of compounding
		AQLString freqSStr_cpd = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYCOMPOUND + staticDataSuffix).toUpper();
		// get daycount
		AQLString daycSStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT, staticDataSuffix, term).toUpper();
		// get daycount of floating leg
		AQLString daycSStr_float = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT, staticDataSuffix, term).toUpper();
		// get sliding
		AQLString slidingSStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE, staticDataSuffix, term).toUpper();
		// get market rate interpolation
		AQLString interpSStr = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_INTERPOLATION + staticDataSuffix);

		if (useTenorBasis && tmpCurrency.toUpper() == CURRENCY_AUD && resetFlg)
		{
			tenorBasisMarketQuotes[marketName].insert(make_pair(term, rate / 100.0));
		}
		AQLObject *mktData = NULL;
		AQLString nameS = yieldDataName + "_SWAP_" + AQLString(j) + suffix_data;
		const AQLObjectHolder ehswap = objPool.getObject(nameS);
		if (!ehswap.isDefined())
		{
			mktData = new AQLObject();
			objPool.set(nameS, mktData);
		}
		else
		{
			objPool.getObject(nameS).get().clear();
			mktData = &objPool.getObject(nameS).get();
		}
		staticDataName += nameS + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameS);
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateS));
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTLAG, new AQLDataInt(resetLag));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(calSStr);
		// set daycount
		mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycSStr);
		// set daycount of floating leg
		if (daycSStr_float != AQ_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, new AQLPriceDataDayCount()).convertFromString(daycSStr_float);
		}
		// set data type
		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_PAR);
		// set slidingrule
		mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingSStr);
		// set fwdbasis
		if (areSwapsForwardStarting)
		{
			mktData->add(PRICING_DATA_ISFWDSWAP, new AQLDataBool(areSwapsForwardStarting));
			if (swapDataMtx[j].size() != 5)
				AQ_THROW( "FwdSwap File format is wrong" );
			const bool isDate = swapDataMtx[j][2].toUpper() == "TRUE";
			mktData->add(PRICING_DATA_ISDATE, new AQLDataBool(isDate));
			if (isDate)
			{
				const AQLDate startDate = AQLDate(swapDataMtx[j][3].getCString());
				mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
				const AQLDate endDate = AQLDate(swapDataMtx[j][4].getCString());
				mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
			}
			else
			{
				const AQLString startTerm = swapDataMtx[j][3].toUpper();
				mktData->add(PRICING_DATA_STARTTERM, new AQLDataString(startTerm));
				const AQLString tenor = swapDataMtx[j][4].toUpper();
				mktData->add(PRICING_DATA_TENOR, new AQLDataString(tenor));
			}
		}
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));
		// set rate
		mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate / 100.0));
		// time inter
		mktData->add(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONSW, new AQLDataBool(isTimeInterSW));
		// newton raphson
		mktData->add(IR_CALIBRATION_DATA_ISNEWTONRAPHSONSW, new AQLDataBool(isNRSW));
		// optimize method
		mktData->add(IR_CALIBRATION_DATA_OPTIMIZEMETHOD, new AQLDataString(optimizeMethod));
		// Swap tenor
		mktData->remove(IR_CALIBRATION_DATA_SWAPTENOR);
		mktData->add(IR_CALIBRATION_DATA_SWAPTENOR, new AQLDataString(swapTenor));
		// Swap type
		mktData->remove(IR_CALIBRATION_DATA_SWAPTYPE);
		mktData->add(IR_CALIBRATION_DATA_SWAPTYPE, new AQLDataString(swapType));
		// iseomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLLSW, new AQLDataBool(isEOMRollSW));
		// simultaneous equation
		mktData->add(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQSW, new AQLDataBool(isSimuEQSW));
		// set epsilon
		mktData->add(IR_CALIBRATION_DATA_EPSILON, new AQLDataDouble(eps));
		// set gradient epsilon
		mktData->add(IR_CALIBRATION_DATA_GRADIENTEPSILON, new AQLDataDouble(grad_eps));
		// set delta
		mktData->add(IR_CALIBRATION_DATA_DELTA, new AQLDataDouble(delta));
		// set maxloop
		mktData->add(IR_CALIBRATION_DATA_MAXLOOP, new AQLDataInt(maxLoop));
		// is frequency change
		mktData->add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, new AQLDataBool(useTenorBasisInstruments));
		// set optimize performance
		mktData->add(IR_CALIBRATION_DATA_OPTIMIZEPERFORMANCE, new AQLDataBool(optimizePerformance));
		// set fast rebuild
		mktData->add(IR_CALIBRATION_DATA_FASTREBUILD, new AQLDataBool(fastRebuild));
		// market rate interpolation
		if (interpSStr != AQ_NO_DATA)
		{
			AQLPriceDataInterpolation interpSAtt;
			interpSAtt.convertFromString(interpSStr);
			mktData->add(CALIBRATION_DATA_INTERPOLATION, new AQLPriceDataInterpolation(interpSAtt));
		}
		// set fwd interpolation
		mktData->add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, new AQLDataBool(isFWDInter));
		mktData->add(IR_CALIBRATION_DATA_FWDINTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(strFWDInter);
		//grid use
		bool isUse = false;
		if (swapUseGrid.size() != 0 && find(swapUseGrid.begin(), swapUseGrid.end(), term) == swapUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
		}
		else
		{
			isUse = true;
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
		}
		
		// Check and Set Swap Instrument Fixed Frequency
		//*** Note *** Commented out the fixed frequency constraint, which can prohibit valid short dated swaps under 1 year.
		// if ((!checkFrequency(freqSStr, term) || !checkFrequency(freqSStr_Fix, term)) && isUse)
		// {
		// 		AQ_THROW("Invalid Swap Instrument: Swap tenor '" +  AQLString(term) + "' is inconsistent with the swap instrument fixed frequency")
		// }
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freqSStr);
		mktData->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FIX, new AQLDataString()).convertFromString(freqSStr_Fix);
		
		// Check and Set the Curve Base Float Frequency
		if (baseFreqSStr_float != AQ_NO_DATA)
		{
			if( !checkFrequency( baseFreqSStr_float, term ) && isUse )
			{
				AQ_THROW("Invalid Swap Instrument: Swap tenor '" +  AQLString(term) + "' is inconsistent with the curve frequency '" +  AQLString(baseFreqSStr_float) + "'")
			}
			mktData->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, new AQLDataString()).convertFromString(baseFreqSStr_float);
		}
		
		// Check and Set the Swap Instrument Float Frequency
		if (freqSStr_float != AQ_NO_DATA)
		{
			if (!checkFrequency(freqSStr_float, term) && isUse)
			{
				AQ_THROW("Invalid Swap Instrument: Swap tenor '" +  AQLString(term) + "' is inconsistent with the swap instrument float frequency '" +  AQLString(freqSStr_float) + "'")
			}
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, new AQLDataString()).convertFromString(freqSStr_float);
		}

		// Check and Set the Compounding Frequency
		if (freqSStr_cpd != AQ_NO_DATA)
		{
			if (!checkFrequency(freqSStr_cpd, term) && isUse)
			{
				AQ_THROW("Invalid Swap Instrument: Swap tenor '" +  AQLString(term) + "' is inconsistent with the swap instrument compound frequency '" +  AQLString(freqSStr_cpd) + "'")
			}
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_COMPOUND, new AQLDataString()).convertFromString(freqSStr_cpd);
		}
	}

	AQ_THROW_IF( staticDataName.size() < 2, "Missing Market Data. Market Data is not set !!" );

	AQLString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix);
	if (aliasCurveNames != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
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
	const AQLString isPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<AQLString, bool>& gCurveMap = curveCalibrationData.getGCurveGenerateMap();
		const std::map<AQLString, AQLString>& assignedCurveMktMap = curveCalibrationData.getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
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
AQLUpdateCurveObject::setUpGenCurveDataOIS(AQLDataInstance &dataInstance, AQLString &staticDataName, const AQLDate &asOfDate, const AQLString &currency,
	const AQLString &marketName, const AQLString &yieldDataName, bool isSpotStarting,
	bool useTenorBasis, CurveCalibrationData &curveCalibrationData,
	std::map<AQLString, std::map<AQLString, double> > &tenorBasisMarketQuotes) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	AQLString suffixLowerCase = "." + marketName;
	suffixLowerCase.toLower();

	AQLString suffix_data = "_" + marketName;

	AQLString yeildGenInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffixLowerCase);
	if (yeildGenInter != AQ_NO_DATA)
	{
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data, new AQLPriceDataInterpolation()).convertFromString(yeildGenInter);
	}

	AQLString oisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffixLowerCase);
	AQLFileAccessor oisFile(AQLMarketData::getNumFileName(oisFileName));
	AQLStringMatrix oisDataMtx;
	oisFile.readAllData(MARKET_DATA_DELIMITER, oisDataMtx);

	oisFile.close();

	if (oisDataMtx.size() == 0 || oisDataMtx[0].size() < 2)
	{
		AQ_THROW("Invalid Market Data: OIS Swap Data is missing")
	}

	// get cal and calc spot date
	AQLPriceDataCalendar calOIS;
	AQLString calOISStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_CALENDAR + suffixLowerCase);
	calOIS.convertFromString(calOISStr);
	AQLDate spotDateOIS;
	if (isSpotStarting)
	{
		spotDateOIS.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SPOTDATE + suffixLowerCase).getCString());
	}
	else
	{
		spotDateOIS = calOIS.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_RESETLAG + suffixLowerCase).getIntValue());
	}

	bool isEOMRollOIS = false;
	AQLString strEOMRollOIS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_ISEOMRLL + suffixLowerCase).toUpper();
	if (strEOMRollOIS != AQ_NO_DATA)
	{
		AQLDataBool tmpIsEOMRollOIS;
		tmpIsEOMRollOIS.convertFromString(strEOMRollOIS);
		isEOMRollOIS = tmpIsEOMRollOIS.get();
	}

	if (isEOMRollOIS)
	{
		AQLString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_EOMDAY + suffixLowerCase).toUpper();
		if (strEOMDay != AQ_NO_DATA)
		{
			if (spotDateOIS.dayOfMonth() != strEOMDay.getIntValue())
			{
				isEOMRollOIS = false;
			}
		}
		else
		{
			const AQLDate eomDate = calOIS.getEOMDay(spotDateOIS);
			if (spotDateOIS != eomDate)
			{
				isEOMRollOIS = false;
			}
		}
	}

	// Linear Spline interpolation cut off date
	AQLString inputInterpJoinDateStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_INTERPOLATIONJOINDATE + suffixLowerCase).toUpper();
	curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);

	yldEntity.remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	if (inputInterpJoinDateStr != AQ_NO_DATA && inputInterpJoinDateStr.size() != 0)
	{
		//AQLDate inputLinearSplineJoinDate = etrading::stringToDate(inputInterpJoinDateStr, "Invalid joinDate");
		AQLString busDayAdj = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE + suffixLowerCase).toUpper();
		AQLDate inputInterpolationJoinDate = etrading::validateDateOrTenor(spotDateOIS, inputInterpJoinDateStr, busDayAdj, calOISStr, "Invalid joinDate");

		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputInterpolationJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputInterpolationJoinDate));
	}

	// Various OIS control parameters
	AQLString str_shortTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERM + suffixLowerCase);
	AQLDate shortTermDate;
	if (str_shortTerm != AQ_NO_DATA)
	{
		shortTermDate = calOIS.getBusinessDay(asOfDate, str_shortTerm.getIntValue());
	}

	AQLString shortTermConv = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION + suffixLowerCase).toUpper();
	AQLString firstRate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FIRSTRATE + suffixLowerCase).toUpper();

	AQLString swapCompoundingMethod("");
	AQLString compoundValue = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDMETHOD + suffixLowerCase).toUpper();
	AQLString compoundValueAlias = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDINGMETHOD + suffixLowerCase).toUpper();

	if (compoundValue != AQ_NO_DATA && compoundValueAlias != AQ_NO_DATA)
	{
		AQ_REQUIRE(compoundValue == compoundValueAlias, "Invalid CompoundMethod: Cannot specify 'CompoundMethod' as '" + compoundValue + "' the alias for this parameter 'CompoundingMethod' as '" + compoundValueAlias + "'")
			swapCompoundingMethod = compoundValueAlias;
	}
	else
	{
		if (compoundValue != AQ_NO_DATA)
		{
			swapCompoundingMethod = compoundValue;
		}
		else if (compoundValueAlias != AQ_NO_DATA)
		{
			swapCompoundingMethod = compoundValueAlias;
		}
	}

	// Validate SwapCompounding Method Convention Parameter
	if (swapCompoundingMethod != AQ_NO_DATA)
	{
		swapCompoundingMethod.toUpper();
		AQ_REQUIRE(swapCompoundingMethod == "ARITHMETIC" || swapCompoundingMethod == "GEOMETRIC" || swapCompoundingMethod == "NONE" || swapCompoundingMethod == "",
			"Invalid CompoundingMethod - Input was '" + swapCompoundingMethod + "' but must be 'ARITHMETIC', 'GEOMETRIC' or 'NONE' ")
	}

	// Validate shortTermConv Generate Method Parameter by trying to cast to its enumerated type
	if (shortTermConv != AQ_NO_DATA)
	{
		shortTermConv.toUpper();
		AQ_REQUIRE(shortTermConv == "ARITHMETIC" || shortTermConv == "ARITHMETICAVERAGE" || shortTermConv == "NONE" || shortTermConv == "",
			"Invalid ShortTermConvention - Input was '" + shortTermConv + "' but must be 'ARITHMETICAVERAGE' or 'NONE'")
	}

	const size_t oisSize = oisDataMtx.size();

	// Curve controls
	bool smoothShortEnd = false;
	AQLString strSmoothShortEnd = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SMOOTHSHORTEND + suffixLowerCase);
	if (strSmoothShortEnd != AQ_NO_DATA)
	{
		AQLDataBool tmpSmoothShortEnd;
		tmpSmoothShortEnd.convertFromString(strSmoothShortEnd);
		smoothShortEnd = tmpSmoothShortEnd.get();
	}

	bool shortTermSwapOverrules = false;
	AQLString strShortTermSwapOverrules = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMSWAPOVERRULES + suffixLowerCase);
	if (strShortTermSwapOverrules != AQ_NO_DATA)
	{
		AQLDataBool tmpShortTermSwapOverrules;
		tmpShortTermSwapOverrules.convertFromString(strShortTermSwapOverrules);
		shortTermSwapOverrules = tmpShortTermSwapOverrules.get();
	}

	// use grid
	AQLStringVector oisUseGrid;
	AQLString tmpOISUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffixLowerCase).toUpper();
	if (tmpOISUseGrid != AQ_NO_DATA)
	{
		oisUseGrid = tmpOISUseGrid.toToken(':');
	}

	// doing dual bootstrapping?
	bool isDualBootstrapping = false;
	AQLString strIsDualBootstrapping = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_ISDUALBOOTSTRAPPING + suffixLowerCase);
	if (strIsDualBootstrapping != AQ_NO_DATA)
	{
		AQLDataBool tmpIsDualBootstrapping;
		tmpIsDualBootstrapping.convertFromString(strIsDualBootstrapping);
		isDualBootstrapping = tmpIsDualBootstrapping.get();
	}

	// Validate LongTerm Convention Parameter
	AQLString longTermConvStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffixLowerCase).toUpper();

	etrading::OISLongTermInstrumentsEnum longTermConv = (longTermConvStr != AQ_NO_DATA) ? etrading::toOISLongTermInstrumentsEnum(longTermConvStr.getCString()) : etrading::NONE_OIS_LONGTERM_INSTRUMENTS;

	AQLString longTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffixLowerCase).toUpper();

	AQLDate date_lt;
	AQLStringMatrix lobasisDataMtx, swapDataMtx;

	// LongTermGenerate Methodolgy Defaults are managed within the in calcEffectiveOISRate method, see CurveCalibration.cpp
	AQLString longTermGenStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD + suffixLowerCase).toUpper();
	AQLString longTermGenArrOisStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD_ARROIS + suffixLowerCase).toUpper();

	etrading::OISLongTermCompoundingEnum longTermGen = (longTermGenStr != AQ_NO_DATA) ? etrading::toOISLongTermCompoundingEnum(longTermGenStr.getCString()) : etrading::NONE_OIS_LONGTERM_COMPOUNDING;
	etrading::OISLongTermCompoundingEnum longTermGenArrOis = (longTermGenArrOisStr != AQ_NO_DATA) ? etrading::toOISLongTermCompoundingEnum(longTermGenArrOisStr.getCString()) : etrading::NONE_OIS_LONGTERM_COMPOUNDING;

	if (longTermConv == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
	{
		AQ_THROW_IF( longTerm == AQ_NO_DATA, "LongTerm parameter is needed, when the LongTermConvention 'LOBASIS' is specified." );

		date_lt = etrading::AQLDateHelpers::getDate(asOfDate, longTerm, true);
		AQLString lobasisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + suffixLowerCase);
		AQLFileAccessor lobasisFile(AQLMarketData::getNumFileName(lobasisFileName));
		lobasisFile.readAllData(MARKET_DATA_DELIMITER, lobasisDataMtx);
		lobasisFile.close();
		if (lobasisDataMtx.size() == 0 || lobasisDataMtx[0].size() < 2)
		{
			AQ_THROW("Missing Libor-OIS Basis Market Data")
		}
		AQLString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffixLowerCase);
		if (swapFileName != AQ_NO_DATA)
		{
			AQLFileAccessor swapFile(AQLMarketData::getNumFileName(swapFileName));
			swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
			swapFile.close();
			if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2)
			{
				if (!isDualBootstrapping)
				{
					AQ_THROW("Invalid Market Data: Missing Swap Market Data")
				}
			}

			if (swapDataMtx.size() != lobasisDataMtx.size())
			{
				if (!isDualBootstrapping)
				{
					AQ_THROW("Invalid Market Data: Inconsistent number of LOBasis spreads and Libor swaps provided; LOBASIS Instruments: " + AQ_TO_STRING_FROM_SIZE_T(lobasisDataMtx.size()) +  " vs SWAP Instruments: " + AQ_TO_STRING_FROM_SIZE_T(swapDataMtx.size()) )
				}
			}
		}
	}

	//get constant for convergence
	double epsilon = 1.0e-9;
	AQLString strEPS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_EPSILON + suffixLowerCase);
	if (strEPS.toUpper() != AQ_NO_DATA)
	{
		epsilon = strEPS.getDoubleValue();
	}
	int maxLoop = 1000;
	AQLString strMLP = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_MAXLOOP + suffixLowerCase);
	if (strMLP.toUpper() != AQ_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}

	// Consolidate the outright OIS swap instruments and basis swap + libor swap instruments
	size_t extraBasisCount = 0;
	size_t longTermStartIndex = 0;
	if (longTermConv == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
	{
		AQLString lastOisTerm = oisDataMtx[oisSize - 1][0].toUpper();

		// When the outright swap tenors overlap with the basis tenors, if the max basis tenor is larger
		// than the max outright swap tenor, find those tenors that are larger than the max outright swap
		// tenor and concatenate them to the outright swap tenors
		bool isOverlapped = false;
		for (size_t i = 0; i < lobasisDataMtx.size(); ++i)
		{
			AQLString term = lobasisDataMtx[i][0].toUpper();
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
				AQLString term = lobasisDataMtx[i][0].toUpper();
				const AQLDate date = etrading::AQLDateHelpers::getDate(asOfDate, term, true);

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
	AQLString useConvexAdjustmentStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USECONVEXADJUSTMENT + suffixLowerCase).toUpper();
	if (useConvexAdjustmentStr != AQ_NO_DATA)
	{
		useConvexAdjustment = (useConvexAdjustmentStr == "TRUE");
		convexityQuotedAsVol = !useConvexAdjustment;
	}

	// get ConvexityQuoteType *** An alias for UseConvexAdjustment***
	AQLString convexityQuoteTypeStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_CONVEXITYQUOTETYPE + suffixLowerCase).toUpper();
	if (convexityQuoteTypeStr != AQ_NO_DATA)
	{
		convexityQuoteTypeStr.toUpper();
		AQ_REQUIRE( convexityQuoteTypeStr == "VOL" || convexityQuoteTypeStr == "PRICE", "Invalid Futures Convexity Quote Type: ConvexityQuoteType must be VOL or PRICE" )
		convexityQuotedAsVol = ( convexityQuoteTypeStr == "VOL" ) ? true : false;

		// TODO: Property Manager does not allow us to clear parameters once set ... hence the below fails ... prioritize the convexityQuoteType parameter for now
		// if ( useConvexAdjustmentStr != AQ_NO_DATA )
		// {
		// 	AQ_THROW("Invalid Futures Convexity Parameter: Cannot use ConvexityQuoteType and UseConvexAdjustment (ConvexityQuotedAsPrice) parameters at the same time.")
		// }
	}

	// Important Update ConvexityQuoteString for Object Pool Setting
	convexityQuoteTypeStr = convexityQuotedAsVol ? "VOL" : "PRICE";

	// ----------------------------------------------------------------

	// get meanReversion for future section
	double meanReversion = 0.0;
	std::string meanReversion_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_MEANREVERSION + suffixLowerCase).toUpper().getCString();
	if (meanReversion_str != AQ_NO_DATA)
	{
		meanReversion = std::stod(meanReversion_str);
	}

	size_t currentLOBasisInstrument = 0;
	size_t currentSwapInstrument = 0;

	size_t lobasisIndex = 0;
	for (size_t i = 0; i < oisSize + extraBasisCount; ++i)
	{
		AQLObject *mktData = NULL;
		AQLString nameOIS = yieldDataName + "_OIS_" + AQLString((int)i) + "_" + marketName;
		const AQLObjectHolder ehois = objPool.getObject(nameOIS);
		if (!ehois.isDefined())
		{
			mktData = new AQLObject();
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
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameOIS);

		// Always get term tenor from the OIS outright swap table first. If the OIS outright swap table
		// is not long enough, get the subsequent term tenors from the LOBASIS table
		AQLString term("");
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
			AQ_THROW_IF( oisDataMtx[i].size() != 4, "OIS Market Data must contain 4 columns. OIS Market Data column size is incorrect." );
			// boj type
			// ** Currently all Central Bank swaps are labelled as BOJ regardless of currency. **
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BOJ);
			AQLDate startDate = AQLDataDate(oisDataMtx[i][2]).get();
			AQLDate endDate = AQLDataDate(oisDataMtx[i][3]).get();
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));

			//When the startDate is in the past and endDate is in the future, use Historical data in the FixingTable
			if (startDate < asOfDate && endDate > asOfDate)
			{
				AQLString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffixLowerCase);
				etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
			}

		}
		else if (etrading::isFuture(term))
		{
			const int NUM_COLUMNS_FUTURE = 5; // Term, Rate, StartDate, EndDate, VolOrConvAdj
			AQ_THROW_IF( oisDataMtx[i].size() != NUM_COLUMNS_FUTURE, "OIS Market Data must contain 4 columns. OIS Market Data column size is incorrect." );

			// ARR Future type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_ARR_FUTURE);

			// set startDate, endDate
			AQLDate startDate = etrading::stringToDate(oisDataMtx[i][2]);
			AQLDate endDate = etrading::stringToDate(oisDataMtx[i][3]);
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));

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
			mktData->add(PRICING_DATA_CONVEXITYQUOTETYPE, new AQLDataString(convexityQuoteTypeStr));
			
			// set convexAdj ... futures convexity adjustment when quoted as a price
			mktData->add(PRICING_DATA_CONVEXADJUSTMENT, new AQLDataDouble(convexAdj));
			
			// set vol ... futures convexity adjustment when quoted as a volatility
			mktData->add(PRICING_DATA_FUTUREVOLATILITY, new AQLDataDouble(futureVol));

			// -------------------------------------

			//When the startDate is in the past and endDate is in the future, use Historical data in the FixingTable
			if (startDate < asOfDate && endDate > asOfDate)
			{
				AQLString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffixLowerCase);
				etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
			}
		}
		else
		{
			// swap type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_PAR);
		}

		// get freq
		AQLString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY, suffixLowerCase, term).toUpper();
		// get daycount
		AQLString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT, suffixLowerCase, term).toUpper();
		// get sliding
		AQLString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE, suffixLowerCase, term).toUpper();
		// get swap type
		AQLString swapType("OIS");
		AQLString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SWAPTYPE + suffixLowerCase).toUpper();
		if (strSwapType != AQ_NO_DATA)
		{
			swapType = strSwapType;
		}
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateOIS));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(calOISStr);
		// set sliding
		mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycOISStr);
		// set daycount
		mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingOISStr);
		// set frequency
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freqOISStr);
		// set short term rate convention
		mktData->add(IR_CALIBRATION_DATA_SHORTTERMCONVENTION, new AQLDataString()).convertFromString(shortTermConv);
		// set first market
		mktData->add(IR_CALIBRATION_DATA_FIRSTRATE, new AQLDataString()).convertFromString(firstRate);
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));
		// set swap averaging method
		mktData->add(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, new AQLDataString()).convertFromString(swapCompoundingMethod);

		auto datatype = dynamic_cast<const AQLDataString&> ((mktData->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();

		if (datatype == YIELD_TYPE_ARR_FUTURE)
		{
			// 1) transform future price to rate
			rate = 1.0 - rate / 100.0;

			AQLPriceDataDayCount dc;
			dc.convertFromString(daycOISStr);

			// 2) add convexity adjustment to the future rate
			rate = etrading::getConvexityAdjustedFutureRate(mktData, rate, asOfDate, meanReversion, dc);

			// 3) set convexity adjusted forward rate
			mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate));
		}
		else
		{
			// set rate
			mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate / 100.0));
		}

		// iseomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLLOIS, new AQLDataBool(isEOMRollOIS));
		// set short term date
		if (str_shortTerm != AQ_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_SHORTTERMDATE, new AQLDataDate(shortTermDate));
		}
		//grid use
		mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
		// set epsilon
		mktData->add(IR_CALIBRATION_DATA_EPSILON, new AQLDataDouble(epsilon));
		// set maxloop
		mktData->add(IR_CALIBRATION_DATA_MAXLOOP, new AQLDataInt(maxLoop));
		// set swap type
		mktData->add(IR_CALIBRATION_DATA_SWAPTYPE, new AQLDataString(swapType));
		// short term swap overrules
		mktData->add(IR_CALIBRATION_DATA_SHORTTERMSWAPOVERRULES, new AQLDataBool(shortTermSwapOverrules));
		// smooth short end of curve
		mktData->add(IR_CALIBRATION_DATA_SMOOTHSHORTEND, new AQLDataBool(smoothShortEnd));

		if (longTermConv == etrading::LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
		{
			// Allow Central Bank Swap instruments to be specified in the OIS curve for all listOfCurvesBuilt ...
			if (etrading::isCentralBankSwap(term) || etrading::isFuture(term)) 
			{
				continue;
			}

			// Skip tenors whose end date is below the 'longTermTenor'
			const AQLDate date = etrading::AQLDateHelpers::getDate(asOfDate, term, true);
			if (date < date_lt)
			{
				continue;
			}

			// Look up on the lobasis table and find the basis instrument with the same term tenor
			while (currentLOBasisInstrument < lobasisDataMtx.size())
			{
				const AQLString term_lo = lobasisDataMtx[currentLOBasisInstrument][0].toUpper();
				if (term_lo == term)
				{
					break;
				}
				++currentLOBasisInstrument;
			}

			// Not Found Condition
			if (currentLOBasisInstrument >= lobasisDataMtx.size())
			{
				AQ_THROW("Unable to imply OIS Outright Swaps from Libor-OIS Basis Instruments: Missing Libor-OIS Basis Swap with tenor " + term + ". Note: Overlapping OIS and Libor-OIS instruments must have identical tenors.");
			}

			// Look up on the libor swap table and find the libor swap instrument with the same term tenor
			while (currentSwapInstrument < swapDataMtx.size())
			{
				const AQLString term_s = swapDataMtx[currentSwapInstrument][0].toUpper();
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
					AQ_THROW("Unable to imply OIS Outright Swaps from Libor-OIS Basis Instruments: Missing Libor Swap with tenor'" + term + "' to match the corresponding Libor-OIS Basis Swap");
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
			
			AQLString calLOStr(""), freqLOStr(""), daycLOStr(""), slidingLOStr("");
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
			AQLString calSwapStr(""), freqSwapStr(""), daycSwapStr(""), slidingSwapStr(""), swapTenor(""), swapType("LIBOR");
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

				AQLString strSwapTenor = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTENOR + suffixLowerCase).toUpper();
				if (strSwapTenor != AQ_NO_DATA)
				{
					swapTenor = strSwapTenor;
				}

				AQLString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTYPE + suffixLowerCase).toUpper();
				if (strSwapType != AQ_NO_DATA)
				{
					swapType = strSwapType;
				}

			}

			// set long term rate convention
			mktData->add(IR_CALIBRATION_DATA_LONGTERMCONVENTION, new AQLDataString()).convertFromString(toString(longTermConv));
			mktData->add(IR_CALIBRATION_DATA_LONGTERM, new AQLDataString()).convertFromString(longTerm);
						
			if (basisSwapType == etrading::OIS_OISBASIS_INSTRUMENT_SWAP_TYPE)
			{
				mktData->add(IR_CALIBRATION_DATA_LONGTERMGENMETHOD, new AQLDataString()).convertFromString(toString(longTermGenArrOis));
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_LONGTERMGENMETHOD, new AQLDataString()).convertFromString(toString(longTermGen));
			}

			// set lobasis data
			mktData->add(IR_CALIBRATION_DATA_RATE_LOBASIS, new AQLDataDouble(rate_lo / 100.0));
			if (calLOStr == AQ_NO_DATA) calLOStr = calOISStr;
			mktData->add(IR_CALIBRATION_DATA_CALENDAR_LOBASIS, new AQLPriceDataCalendar()).convertFromString(calLOStr);
			if (daycLOStr == AQ_NO_DATA) daycLOStr = daycOISStr;
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_LOBASIS, new AQLPriceDataDayCount()).convertFromString(daycLOStr);
			if (slidingLOStr == AQ_NO_DATA) slidingLOStr = slidingOISStr;
			mktData->add(IR_CALIBRATION_DATA_SLIDINGRULE_LOBASIS, new AQLPriceDataSlidingRule()).convertFromString(slidingLOStr);
			if (freqLOStr == AQ_NO_DATA) freqLOStr = freqOISStr;
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_LOBASIS, new AQLDataString()).convertFromString(freqLOStr);

			// set swap data
			if (isSwapRateAvailable)
			{
				double rate_s = swapDataMtx[currentSwapInstrument][1].getDoubleValue();
				mktData->add(IR_CALIBRATION_DATA_RATE_SWAP, new AQLDataDouble(rate_s / 100.0));
			}
			mktData->add(IR_CALIBRATION_DATA_CALENDAR_SWAP, new AQLPriceDataCalendar()).convertFromString(calSwapStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_SWAP, new AQLPriceDataDayCount()).convertFromString(daycSwapStr);
			mktData->add(IR_CALIBRATION_DATA_SLIDINGRULE_SWAP, new AQLPriceDataSlidingRule()).convertFromString(slidingSwapStr);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_SWAP, new AQLDataString()).convertFromString(freqSwapStr);

			mktData->remove(IR_CALIBRATION_DATA_REFSWAPTENOR);
			mktData->remove(IR_CALIBRATION_DATA_REFSWAPTYPE);
			mktData->add(IR_CALIBRATION_DATA_REFSWAPTENOR, new AQLDataString()).convertFromString(swapTenor);
			mktData->add(IR_CALIBRATION_DATA_REFSWAPTYPE, new AQLDataString()).convertFromString(swapType);
		}
	}

	AQLStringMatrix fedFundFutureDataMtx;
	AQLString fedFundFutureFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + suffixLowerCase);
	if (fedFundFutureFileName != AQ_NO_DATA)
	{
		AQLFileAccessor fedFundFutureFile(AQLMarketData::getNumFileName(fedFundFutureFileName));
		fedFundFutureFile.readAllData(MARKET_DATA_DELIMITER, fedFundFutureDataMtx);
		fedFundFutureFile.close();
	}

	const int fedFundFutureSize = fedFundFutureDataMtx.size();
	// use grid
	AQLStringVector ffFutureUseGrid;
	AQLString tmpFFFutureUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID + suffixLowerCase).toUpper();
	if (tmpFFFutureUseGrid != AQ_NO_DATA)
	{
		ffFutureUseGrid = tmpFFFutureUseGrid.toToken(':');
	}
	else
	{
		AQLString tmpUseGridNum = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRIDNUM + suffixLowerCase).toUpper();
		if (tmpUseGridNum != AQ_NO_DATA)
		{
			int useGridNum = tmpUseGridNum.getIntValue();
			if (useGridNum == 0)
			{
				ffFutureUseGrid.push_back(AQ_NO_DATA);
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
		AQLObject *mktData = NULL;
		AQLString nameOIS = yieldDataName + "_OIS_" + AQLString((int)oisSize + i) + "_" + marketName;
		const AQLObjectHolder ehois = objPool.getObject(nameOIS);
		if (!ehois.isDefined())
		{
			mktData = new AQLObject();
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
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameOIS);

		AQLDate startDate, endDate;
		AQLString term = fedFundFutureDataMtx[i][0].toUpper();

		AQ_THROW_IF( fedFundFutureDataMtx[i].size() < 2, "FF Futures data cannot contain more than 2 columns. FF Future File format is wrong" );

		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_FF);
		if (fedFundFutureDataMtx[i].size() == 4)
		{
			startDate = AQLDataDate(fedFundFutureDataMtx[i][2]).get();
			endDate = AQLDataDate(fedFundFutureDataMtx[i][3]).get();
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
		}
		else //FF non startdate type
		{
			AQLString term = fedFundFutureDataMtx[i][0].toUpper();
			DateVector ffdates = etrading::AQLDateHelpers::getFFDatesFromTerm(asOfDate, term);
			if (ffdates.size() != 2)
				AQ_THROW( "FF Dates Data must contain 2 columns. FF dates error" );

			startDate = ffdates[0];
			endDate = ffdates[1];
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
		}

		double rate = fedFundFutureDataMtx[i][1].getDoubleValue();
		rate = 100. - rate;

		// get freq
		AQLString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY, suffixLowerCase, term).toUpper();
		// get daycount
		AQLString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT, suffixLowerCase, term).toUpper();
		// get sliding
		AQLString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE, suffixLowerCase, term).toUpper();
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateOIS));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(calOISStr);
		// set sliding
		mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycOISStr);
		// set daycount
		mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingOISStr);
		// set frequency
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freqOISStr);
		// set short term rate convention
		mktData->add(IR_CALIBRATION_DATA_SHORTTERMCONVENTION, new AQLDataString()).convertFromString(shortTermConv);
		// set first market
		mktData->add(IR_CALIBRATION_DATA_FIRSTRATE, new AQLDataString()).convertFromString(firstRate);
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));
		// set rate
		mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate / 100.0));
		// iseomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLLOIS, new AQLDataBool(isEOMRollOIS));
		// set short term date
		if (str_shortTerm != AQ_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_SHORTTERMDATE, new AQLDataDate(shortTermDate));
		}
		//grid use
		if (ffFutureUseGrid.size() != 0 && find(ffFutureUseGrid.begin(), ffFutureUseGrid.end(), term) == ffFutureUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
		}
		else
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
		}


		if (startDate < asOfDate && dynamic_cast<AQLDataBool &>(mktData->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, ISNOTNULL).get()).get())
		{
			AQLString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffixLowerCase);

			etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
		}

	}

	AQ_THROW_IF( staticDataName.size() < 2, "Market Data is not set !!" );

	//DF curve name
	AQLString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffixLowerCase);
	if (dfCurveName == AQ_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName);
	curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName, new AQLDataString(dfCurveName));
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName, new AQLDataString(dfCurveName));

	//const std::map<AQLString, AQLString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
	AQLString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffixLowerCase);
	if (aliasCurveNames != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
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
	const AQLString isPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<AQLString, bool>& gCurveMap = curveCalibrationData.getGCurveGenerateMap();
		const std::map<AQLString, AQLString>& assignedCurveMktMap = curveCalibrationData.getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second == marketName)
			{
				if (gCurveMap.find(it->first) != gCurveMap.end()) gCurveMap.erase(it->first);
			}
		}
	}
}

void
AQLUpdateCurveObject::setUpFloater(const AQLString &currency, CurveCalibrationData &curveCalibrationData, const AQLString &genFloaterName) const
{
	AQLStringVector listOfCurvesBuilt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	uppervec(listOfCurvesBuilt);
	if (genFloaterName != AQ_NO_DATA)
	{
		curveCalibrationData.setAssignedCurveMktMap(genFloaterName, genFloaterName);
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_FLOATERDFS);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_FLOATERDFS, new AQLDataString(genFloaterName));
		AQLString tmpGenFloaterName = genFloaterName;
		tmpGenFloaterName.toLower();
		AQLString basisMkt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME);
		if (basisMkt != AQ_NO_DATA)
		{
			AQ_THROW_IF( std::find(listOfCurvesBuilt.begin(), listOfCurvesBuilt.end(), basisMkt) == listOfCurvesBuilt.end(), "Basis market does not exist!" );
			curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_BASISDATA + AQLString("_") + tmpGenFloaterName);
			curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_BASISDATA + AQLString("_") + tmpGenFloaterName, new AQLDataString(basisMkt));
		}
		AQLString discountName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT);
		AQLString forecastName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FORECAST);
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_FORECAST + AQLString("_") + tmpGenFloaterName);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_FORECAST + AQLString("_") + tmpGenFloaterName, new AQLDataString(forecastName));
		curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_DISCOUNT + AQLString("_") + tmpGenFloaterName);
		curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_DISCOUNT + AQLString("_") + tmpGenFloaterName, new AQLDataString(discountName));

		/*AQLString isFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_ISFWDINTER).toUpper();
		if (isFWDInter == "TRUE")
		{
			curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + AQLString("_") + tmpGenFloaterName);
			curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + AQLString("_") + tmpGenFloaterName, new AQLDataBool(true));
			AQLString fwdInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FWDINTERPOLATION).toLower();
			curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_FWDINTERPOLATION + AQLString("_") + tmpGenFloaterName);
			curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_FWDINTERPOLATION + AQLString("_") + tmpGenFloaterName, new AQLPriceDataInterpolation()).convertFromString(fwdInter);;
		}*/

		curveCalibrationData.setFloater(genFloaterName);
	}
}

void
AQLUpdateCurveObject::setUpCurveDataByReadFile(AQLDataInstance &dataInstance, const AQLDate& asOfDate, const AQLString& currency,
	const AQLString& marketName, const AQLString& yieldDataName, CurveCalibrationData &curveCalibrationData) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLObjectHolder objHolder = objPool.getObject(yieldDataName, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
		AQ_THROW( "yield Object is not set! AQLUpdateCurveObject::setUpCurveDataByReadFile" );

	AQLObject &objectPool = objHolder.get();

	AQLString thisMarketName = marketName;
	AQLString suffixLowerCase = "." + thisMarketName.toLower();

	AQLString dfFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_DF_FILE + suffixLowerCase);
	AQLFileAccessor dfFile(AQLMarketData::getNumFileName(dfFileName));
	AQLStringMatrix dfDataMtx;
	dfFile.readAllData(MARKET_DATA_DELIMITER, dfDataMtx);
	dfFile.close();

	if ( dfDataMtx.size() == 0 || dfDataMtx[0].size() < 2 )
	{
		AQ_THROW("Invalid Data: Discount Factor data is missing")
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

	AQLString aliasCurveNames = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ZERORATE_ASSIGNEDCURVE + suffixLowerCase);
	if (aliasCurveNames != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = aliasCurveNames.toToken(':');
		for (size_t i = 0; i < assignedCurves.size(); i++)
		{
			curveCalibrationData.setAssignedCurveMktMap(assignedCurves[i], marketName);

			AQLString modCurveName = "_" + assignedCurves[i].toUpper();

			objectPool.remove(CALIBRATION_DATA_TERMS + modCurveName);
			objectPool.add(CALIBRATION_DATA_TERMS + modCurveName, new AQLDataDoubles(terms));

			objectPool.remove(IR_CALIBRATION_DATA_DFS + modCurveName);
			objectPool.add(IR_CALIBRATION_DATA_DFS + modCurveName, new AQLDataDoubles(dfs));

			objectPool.remove(IR_CALIBRATION_DATA_ACCESSARY + modCurveName);
			objectPool.add(IR_CALIBRATION_DATA_ACCESSARY + modCurveName, new AQLDataString("DF"));

			if (!dfs2.empty())
			{
				objectPool.remove(IR_CALIBRATION_DATA_DFS2 + modCurveName);
				objectPool.add(IR_CALIBRATION_DATA_DFS2 + modCurveName, new AQLDataDoubles(dfs2));
			}
		}
	}
	else
	{
		AQLString modCurveName = "_" + thisMarketName.toUpper();

		objectPool.remove(CALIBRATION_DATA_TERMS + modCurveName);
		objectPool.add(CALIBRATION_DATA_TERMS + modCurveName, new AQLDataDoubles(terms));

		objectPool.remove(IR_CALIBRATION_DATA_DFS + modCurveName);
		objectPool.add(IR_CALIBRATION_DATA_DFS + modCurveName, new AQLDataDoubles(dfs));

		objectPool.remove(IR_CALIBRATION_DATA_ACCESSARY + modCurveName);
		objectPool.add(IR_CALIBRATION_DATA_ACCESSARY + modCurveName, new AQLDataString("DF"));

		if (!dfs2.empty())
		{
			objectPool.remove(IR_CALIBRATION_DATA_DFS2 + modCurveName);
			objectPool.add(IR_CALIBRATION_DATA_DFS2 + modCurveName, new AQLDataDoubles(dfs2));
		}
		curveCalibrationData.setAssignedCurveMktMap(marketName, marketName);
	}

	curveCalibrationData.insertNonRemovableMarket(marketName);
}

void
AQLUpdateCurveObject::dataoutCurve(const AQLStringVector &curveNames, AQLObject &objectPool, const AQLString &yieldDataName) const
{
	for (unsigned int i = 0; i < curveNames.size(); i++)
	{
		AQLString curveSuffix;
		if (curveNames[i] == STD) curveSuffix = "";
		else curveSuffix = "_" + curveNames[i];
		AQLString curveSuffix_file = curveSuffix;
		while (curveSuffix_file.findString("/") != -1)
		{
			curveSuffix_file.remove(curveSuffix_file.findString("/"), 1);
		}
		const AQLString fileSuffix = AQLCoreDataService::getContext(ARG_KEY_FILENUM);
		const AQLString dirName = AQLCoreDataService::getOutputDirectory();
		const AQLString fileName = dirName + yieldDataName + curveSuffix_file + fileSuffix + ".csv";

		ifstream fin;
		ofstream fout;
		fin.open(fileName.getCString());

		if (!fin)
		{
			AQLDataHolder* dh = &objectPool.getData(CALIBRATION_DATA_TERMS + curveSuffix, NOCHECK);
			if (!dh->isDefined() || dh->isNull())
				continue;

			const DoubleArray &terms =
				dynamic_cast<const AQLDataDoubles &>(objectPool.getData(CALIBRATION_DATA_TERMS + curveSuffix, ISNOTNULL).get()).get();
			const DoubleArray &dfs =
				dynamic_cast<const AQLDataDoubles &>(objectPool.getData(IR_CALIBRATION_DATA_DFS + curveSuffix, ISNOTNULL).get()).get();

			int size = terms.size();
			AQ_THROW_IF( size != static_cast<int>(dfs.size()), "Term size and df size must be same !!" );
			fout.open(fileName.getCString());

			const AQLDataHolder &dfsH2 = objectPool.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix, NOCHECK);
			if (!dfsH2.isDefined() || dfsH2.isNull())
			{
				for (int j = 0; j < size; ++j)
				{
					AQLString termOStr = AQLString(terms[j]);
					AQLString dfOStr = AQLString(dfs[j]);
					fout << termOStr.getCString() << "," << dfOStr.getCString() << std::endl;
				}
			}
			else
			{
				const DoubleArray &dfs2 = dynamic_cast<const AQLDataDoubles &>(dfsH2.get()).get();
				for (int j = 0; j < size; ++j)
				{
					AQLString termOStr = AQLString(terms[j]);
					AQLString dfOStr = AQLString(dfs[j]);
					AQLString df2OStr = AQLString(dfs2[j]);
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
				AQLStringVector lineVec = AQLString(c_line).toToken(MARKET_DATA_DELIMITER);
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
			dynamic_cast<AQLDataDoubles &>(objectPool.getData(CALIBRATION_DATA_TERMS + curveSuffix, ISNOTNULL).get()).set(terms);
			dynamic_cast<AQLDataDoubles &>(objectPool.getData(IR_CALIBRATION_DATA_DFS + curveSuffix, ISNOTNULL).get()).set(dfs);
			if (!dfs2.empty() && objectPool.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix).isDefined())
			{
				dynamic_cast<AQLDataDoubles &>(objectPool.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix, ISNOTNULL).get()).set(dfs2);
			}
		}
	}
}



bool
AQLUpdateCurveObject::checkFrequency(const AQLString& freq, const AQLString& mktRateTerm) const
{
	int span = etrading::AQLDateHelpers::getPeriodFrequencyInMonths(freq);

	int y, m, d, w;
	etrading::AQLDateHelpers::termStrtoYMDW(mktRateTerm, y, m, d, w);
	int month_mkt_term = 12 * y + m;

	return (month_mkt_term % span) == 0;
}

///// update for XLL Plus //////////////////////////
#include "AQLUpdateModelObject.h"

void
AQLUpdateCurveObject::generateInitialValueForPricer(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

	AQLObjectPool &objPool = dataInstance.getObjectPool();

	AQLString yieldName = PREFIX_YIELD + getSDEAttrName(currency);
	AQLMathYieldCurve *yc = NULL;
	const AQLObjectHolder ehyc = objPool.getObject(yieldName);
	if (!ehyc.isDefined())
	{
		yc = new AQLMathYieldCurve(&dataInstance);
		objPool.set(yieldName, yc);

	}
	else
	{
		dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get()).reset();
		yc = &dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(yieldName).get());
	}
	yc->getName().convertFromString(yieldName);

	AQLString curvePropertiesName = "PRO_" + yieldName;
	CurveCalibrationData *curveCalibrationData = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(curvePropertiesName);
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

	AQLObject *objectPool = NULL;
	AQLString yieldDataName = yieldName + "_DATA";
	const AQLObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		objectPool = new AQLObject();
		objPool.set(yieldDataName, objectPool);
	}
	else
	{
		//we must not erase the reset method for only edata
		objectPool = &objPool.getObject(yieldDataName).get();
	}
	objectPool->remove(CALIBRATION_DATA_NAME);
	objectPool->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);

	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool useTenorBasis = false;
	bool useTenorBasisInstruments = false;
	bool isSpotStarting = false;

	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *curveCalibrationData, *objectPool, useTenorBasis, useTenorBasisInstruments, isSpotStarting, false);
	setUpCurveDataByContext(*curveCalibrationData, objectPool, currency, SWAP);


	AQLStringVector listOfCurvesBuilt;
	AQLString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == AQ_NO_DATA)
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		listOfCurvesBuilt = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(listOfCurvesBuilt);
	if (!listOfCurvesBuilt.empty() && listOfCurvesBuilt[0] != AQ_NO_DATA)
	{
		for (unsigned int i = 0; i < listOfCurvesBuilt.size(); ++i)
		{
			AQLString suffixLowerCase = listOfCurvesBuilt[i];
			suffixLowerCase.toLower();
			AQLString contextKey = tmpCurrency + CONTEXT_KEY_SDE_YIELD_WITH_MARKET + suffixLowerCase;
			AQLString contextWithMarket = AQLCoreDataService::getContext(contextKey);
			if (contextWithMarket != AQ_NO_DATA)
			{
				AQLString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + suffixLowerCase).toUpper();
				if (marketType == MARKETTYPE_BASIS)
				{
					AQLString useYieldSDEIRStr = AQLCoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
					AQLCoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD, AQ_NO_DATA);
					AQLString tmpCurveName = listOfCurvesBuilt[i];
					tmpCurveName.toUpper();
					mpStaticData->setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
					UpdateObjectPoolForSDEsAndCurves generator(currency);
					generator.loadModelDataAndCalibrate(currency, dataInstance, true);
					mpStaticData->removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
					AQLCoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD, useYieldSDEIRStr);
				}
			}
			setUpCurveDataByContext(*curveCalibrationData, objectPool, currency, listOfCurvesBuilt[i]);
		}
	}
}

void
AQLUpdateCurveObject::setUpCurveDataByContext(CurveCalibrationData &curveCalibrationData, AQLObject *objectPool, const AQLString& currency, const AQLString& marketName) const
{
	AQLString prefix = currency;
	prefix.toLower();
	AQLString suffixLowerCase;
	AQLString data_suffix;
	if (marketName != SWAP)
	{
		suffixLowerCase = marketName;
		suffixLowerCase.toLower();
		data_suffix = "_" + marketName;
	}

	AQLString contextKey = prefix + CONTEXT_KEY_SDE_YIELD + suffixLowerCase;
	AQLString curveContext = AQLCoreDataService::getContext(contextKey);
	if (curveContext == AQ_NO_DATA)
	{
		AQLString msg = "context data for generated dfs don't exist.";
		msg += "(" + contextKey + ")";
		AQ_THROW( msg.getCString() );
	}

	AQLDataDoubleMatrix matrix;
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
	objectPool->add(CALIBRATION_DATA_TERMS + data_suffix, new AQLDataDoubles(terms));

	objectPool->remove(IR_CALIBRATION_DATA_DFS + data_suffix);
	objectPool->add(IR_CALIBRATION_DATA_DFS + data_suffix, new AQLDataDoubles(dfs));

	if (isBasisDiscountFactor)
	{
		AQLDataDoubles* attrDF2 = NULL;
		AQLDataHolder* df2H = &objectPool->getData(IR_CALIBRATION_DATA_DFS2 + data_suffix);
		if (!df2H->isDefined())
		{
			attrDF2 = new AQLDataDoubles();
			objectPool->remove(IR_CALIBRATION_DATA_DFS2 + data_suffix);
			objectPool->add(IR_CALIBRATION_DATA_DFS2 + data_suffix, attrDF2);
		}
		else
		{
			attrDF2 = dynamic_cast<AQLDataDoubles*>(&df2H->get());
		}
		attrDF2->set(dfs2);
		return;
	}

	// check DF2 by properties
	//AQLString df2name = mpStaticData->getStaticData(prefix + STATIC_DATA_KEY_YIELD_DF2);
	AQLString df2name = currency + "BASISDISCOUNT";
	df2name.toUpper();
	if (df2name != AQ_NO_DATA && df2name == marketName)
	{
		AQLDataHolder* dh = &(objectPool->getData(CALIBRATION_DATA_TERMS, ISDEFINED));
		const DoubleVector& baseterms = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();

		DoubleVector calcdf2(baseterms.size(), 1.0);
		AQLSplineInterpolation attrspline;
		attrspline.set(terms, dfs);
		for (unsigned int i = 0; i < baseterms.size(); i++)
			calcdf2[i] = attrspline.value(baseterms[i]);

		objectPool->remove(IR_CALIBRATION_DATA_DFS2);
		objectPool->add(IR_CALIBRATION_DATA_DFS2, new AQLDataDoubles(calcdf2));
	}



	// set curve name to yield curve pro
	AQLStringVector gCurveNames;
	AQLDataHolder *dh = &(curveCalibrationData.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		gCurveNames = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
	}
	gCurveNames.push_back(marketName);
	curveCalibrationData.AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	curveCalibrationData.AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(gCurveNames));
}
////////////////////////////////////////////////////

