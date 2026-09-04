//
// AQLUpdateObjectPoolForCurves.cpp
// This file used to be called CalibratorIR.cpp and before that AQLCalibrateModelIR.cpp
//
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLUpdateObjectPoolForCurves.h"
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
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"
#include "AQLMarketData.h"
#include "AQLPriceArbFreeGenerator.h"
#include "AQLFunctionUtilities.h"
#include "AQLDateHelpers.h"
#include "AQLDateScheduleHelpers.h"
#include "AQLCurveForwardRateHelpers.h"
#include "ParameterValidation.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS 

#include "AQLCoreDataService.h"
#include "AQLRatesTermStructureSDE.h"
#include "AQLRatesLJTermStructureSDE.h"
#include "AQLMathCorrelation.h"
#include <time.h>

#endif

#include "BasisCurveCalibration.h"
#include "CurveCalibration.h"
#include "AQLCurveMarketDataHelpers.h"
#include "CurveInstruments.h"

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

//! constructor
/*!

*/
AQLUpdateObjectPoolForCurves::AQLUpdateObjectPoolForCurves(const AQLString &baseCurrency)
: AQLObjectPoolBase(), mBaseCurrency(baseCurrency)
{
}

//! destructor
/*!

*/
AQLUpdateObjectPoolForCurves::~AQLUpdateObjectPoolForCurves(void)
{
}


#ifndef VISUAL_STUDIO_2010_ANALYTICS 
//! 
/*!
    @brief set setInterpolationMethod

	@param[in] currency
	@param[out] sde
*/
void
AQLUpdateObjectPoolForCurves::setInterpolationMethod(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	(void)currency;
	sde.setInterpolationMethod(new AQLRatesCurveLinearInterpolation());
}
#endif 

//! 
/*!
    @brief generate sde market data

	IR is set initialvalue correlation volatility

	@param[in] currency 
	@param[in] dataInstance
*/
void
AQLUpdateObjectPoolForCurves::generateSDEMarketData(const AQLString &currency, AQLDataInstance &dataInstance, const bool isCurve, const bool isOthers) const
{
	if (isCurve)
	{
        // generate initial curve 
        generateInitialValue(currency, dataInstance);
	}

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	if (isOthers)
	{
	    // generate correlation
		generateCorrelation(currency, dataInstance);

        // generate volatility
		generateVolatility(currency, dataInstance);
	}
#endif
}

//! 
/*!
    @brief generate sde initial curve data for fwdfx constant

	@param[in/out] dataInstance
*/
void
AQLUpdateObjectPoolForCurves::generateInitialValueFwdFXConst(const AQLString &currency, AQLDataInstance &dataInstance) const
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
	AQLString suffix   = "." + AQLString(market).toLower();
	AQLString epSuffix = AQLString(market).toUpper();
	AQLString basisTarget = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_TARGET + suffix).toUpper();
	if (basisTarget == LEG1FORECAST || basisTarget == LEG2FORECAST)
	{
		throw AQLCoreInvalidData("fwdfx constant curve must be discount curve!", __FILE__, __LINE__);
	}
	else if (basisTarget == LEG1DISCOUNT)
	{
		strFCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + suffix);
		strDCurve =  mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + suffix);
		strA_fCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + suffix);
		strA_dCurve =  mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + suffix);
	}
	else if (basisTarget == LEG2DISCOUNT)
	{
		strFCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + suffix);
		strDCurve =  mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + suffix);
		strA_fCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + suffix);
		strA_dCurve =  mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + suffix);
	}
	else
	{
		throw AQLCoreInvalidData("basisTarget curve is invalid!", __FILE__, __LINE__);
	}
	convertCurveName(strFCurve, ccy, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(strDCurve, ccy, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(strA_fCurve, ccy, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(strA_dCurve, ccy, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	if (ccy_dCurve != ccy || ccy_fCurve != ccy || ccy_a_dCurve != ccy_a_fCurve)
		throw AQLCoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);

	const AQLString &ycProName = AQLMarketData::getBaseYieldProName(ccy);
	BasisCurveCalibration &basisCurveEngine = dynamic_cast<BasisCurveCalibration &>(objPool.getObject(ycProName).get());

	AQLObjectHolder& yData = basisCurveEngine.getYieldData().get();
	AQLString ydName = yData.getName();
	if (isSetCurveID)
		ydName = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATECURVEID);

	if (!isPricer && mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_a_fCurve)) 
	{
		// generate against leg ccy
		generateInitialValue(ccy_a_fCurve, dataInstance);
	}

	// save colateral ccy and curve
	BasisCurveCalibration* colYCPro = NULL;
	if (!isPricer)
	{
		const AQLString &colYCProName = AQLMarketData::getBaseYieldProName(ccy_a_fCurve);
		colYCPro = &(dynamic_cast<BasisCurveCalibration &>(objPool.getObject(colYCProName).get()));
		colYCPro->setColAffectingCcy(ccy.toUpper());
		basisCurveEngine.setColAffectedCcy(ccy_a_fCurve.toUpper());
	}
	const AQLString &fYCName = AQLMarketData::getBaseYieldName(ccy_a_fCurve);
	AQLMathYieldCurve& fYC = dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(fYCName).get());
	const AQLString &fYDName = fYC.getYieldData().get().getName();
	basisCurveEngine.getColYieldData().convertFromString(fYDName);

	// save market data
	AQLObject *mktData = NULL;
	AQLString nameB = ydName +  "_" + market.toUpper() + "_" + AQLString(static_cast<int>(0));
	const AQLObjectHolder ehbasis = objPool.getObject(nameB);
	if (!ehbasis.isDefined())
	{
		mktData = new AQLObject();
		objPool.set(nameB, mktData);
	}
	else
	{
		objPool.getObject(nameB).get().clear();
		mktData = &objPool.getObject(nameB).get();
	}
	mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameB);
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
	
	basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + epSuffix);
	basisCurveEngine.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + epSuffix, new AQLDataMultiReference()).convertFromString(nameB);

	// save assigned curves
	AQLStringVector assignedCurves;
	AQLString strAssignedCurves = mpStaticData->getStaticData(ccy.toLower() + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffix);
	if (strAssignedCurves != AQ_NO_DATA)
	{
		assignedCurves = strAssignedCurves.toToken(MULTI_STATIC_DATA_DELIMITER);
		for (size_t i = 0; i<assignedCurves.size(); ++i)
			basisCurveEngine.setAssignedCurveMktMap(assignedCurves[i], market);
	}
	else
	{
		assignedCurves.push_back(market);
		basisCurveEngine.setAssignedCurveMktMap(market, market);
	}

	// set curve type
	AQLString curveType = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
	if (curveType != AQ_NO_DATA)
	{
		basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix);
		basisCurveEngine.AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix, new AQLDataString(curveType.toUpper()));

		yData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix);
        yData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix, new AQLDataString(curveType.toUpper())); 
	}

	// set interpolation
	AQLString genInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix);
	if (genInterp == AQ_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}	
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + epSuffix);
		basisCurveEngine.AQLObject::add(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genInterp);
	}

	// set yieldgen interpolation
	AQLString genYieldGenInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
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
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation_yg().convertFromString(genYieldGenInterp.toLower());
	}
	else
	{
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + epSuffix);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// calc curve
	//basisCurveEngine.calcFwdFXConstantCurve();
    basisCurveEngine.calcFwdFXConstantCurveUsingMarketName(market);// Calculate the FwdFXConstCurve but allow any FWDFXCONST marketName to be specified
		
	// dataout
	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		dataoutCurve(assignedCurves, yData.get(), ydName);
}

//! 
/*!
    @brief generate sde initial curve data for cheapest-to-deliver curve

	@param[in/out] dataInstance
*/
void AQLUpdateObjectPoolForCurves::generateInitialValueCheapestToDeliver(const AQLString &currency, AQLDataInstance &dataInstance) const
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
		throw AQLCoreInvalidData(err.getCString(),__FILE__,__LINE__);
	}
	BasisCurveCalibration &basisCurveEngine = dynamic_cast<BasisCurveCalibration &>(objHolder.get());

	AQLObjectHolder& yData = basisCurveEngine.getYieldData().get();
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
	AQLString suffix = "." + market;
	suffix.toLower();
	AQLString epSuffix = market;
	epSuffix.toUpper();
		
	// save market data
	AQLObject *mktData = NULL;
	AQLString nameB = ydName +  "_" + epSuffix;
	const AQLObjectHolder ehbasis = objPool.getObject(nameB);
	if (!ehbasis.isDefined())
	{
		mktData = new AQLObject();
		objPool.set(nameB, mktData);
	}
	else
	{
		objPool.getObject(nameB).get().clear();
		mktData = &objPool.getObject(nameB).get();
	}

	// Convention data
	AQLString calendar	 = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CALENDAR + suffix).toUpper();
	AQLString dayCount	 = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT + suffix).toUpper();
	AQLString businessAdj = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE + suffix).toUpper();
	AQLString frequency	 = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY + suffix).toUpper();

	mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(calendar);
	mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(dayCount);
	mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(businessAdj);
	mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(frequency);
	mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_CTD);

	basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + epSuffix);
	basisCurveEngine.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + epSuffix, new AQLDataMultiReference()).convertFromString(nameB);

	// save assigned curves
	AQLStringVector assignedCurves;
	AQLString strAssignedCurves = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffix);
	if (strAssignedCurves != AQ_NO_DATA)
	{
		assignedCurves = strAssignedCurves.toToken(MULTI_STATIC_DATA_DELIMITER);
		for (size_t i = 0; i<assignedCurves.size(); ++i)
			basisCurveEngine.setAssignedCurveMktMap(assignedCurves[i], market);
	}
	else
	{
		assignedCurves.push_back(market);
		basisCurveEngine.setAssignedCurveMktMap(market, market);
	}

	// set curve type
	AQLString curveType = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
	if (curveType != AQ_NO_DATA)
	{
		basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix);
		basisCurveEngine.AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix, new AQLDataString(curveType.toUpper()));

		yData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix);
        yData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + epSuffix, new AQLDataString(curveType.toUpper())); 
	}

	// Get CSA collateral curves
	AQLStringVector csaCurves;
	AQLString csaCurvesTemp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_CTD_COLLATERALCURVES + suffix);
	if (csaCurvesTemp != AQ_NO_DATA)
	{
		csaCurves = csaCurvesTemp.toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		throw AQLCoreInvalidData("#Error: Must provide at least one CSA curve",__FILE__,__LINE__);
	}

	// set interpolation
	AQLString genInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix);
	if (genInterp == AQ_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}	
	if (epSuffix.size() == 0)
	{
		basisCurveEngine.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + epSuffix);
		basisCurveEngine.AQLObject::add(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genInterp);
	}

	// set yieldgen interpolation
	AQLString genYieldGenInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
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
		basisCurveEngine.getInterpolation_yg().convertFromString(genYieldGenInterp.toLower());
	}
	else
	{
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + epSuffix);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// calc curve
	basisCurveEngine.calcCheapestToDeliverCurve(market, csaCurves);

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
AQLUpdateObjectPoolForCurves::convertCurveName(const AQLString &propCurve, const AQLString &ccy, AQLString &curveCcy, AQLString &curveName, const bool isPricer, AQLObjectPool& objPool) const
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
		else if(propCurves.size() == 2)
		{
			if (isPricer)
			{
				AQLObject* yieldData = &(objPool.getObject(propCurves[0], ENCHKTYPE_ISDEFINED).get());
				curveCcy = dynamic_cast<AQLDataString& >((yieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
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
			throw AQLCoreInvalidData("currency and curve name are not set!", __FILE__, __LINE__);
		}
	}
}

/*!
    @brief get Accessory from Frequency

	@param[in] freq : frequency 
	@return accessory
*/
const AQLString
AQLUpdateObjectPoolForCurves::getAccFromFreq(const AQLString &freq_) const
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
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return acc;
}


//! 
/*!
    @brief generate curve data using dual-bootstrapping technique

	@param[in] currency  currency of the curve
	@param[out] dataInstance
*/
void
AQLUpdateObjectPoolForCurves::generateInitialValueDualBootstrap(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

	// Get YieldCurve and BasisCurveCalibration objects (as dataValues)
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
	
	AQLString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		basisCurveEngine = new BasisCurveCalibration(&dataInstance);
		objPool.set(yieldProName, basisCurveEngine);
	}
	else
	{
		//we must not erase the reset method for only basisCurveEngine
		basisCurveEngine = &dynamic_cast<BasisCurveCalibration &>(objPool.getObject(yieldProName).get());
	}
	basisCurveEngine->getName().convertFromString(yieldProName);
	bool isArbFree = false;		// London don't use this flag and hence hardcode it to False
	basisCurveEngine->getIsArbFree().set(isArbFree);

	// Get YieldData object (as an data)
	AQLObject *eData = NULL;
	AQLString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const AQLObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new AQLObject();
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	eData->remove(CALIBRATION_DATA_NAME);
	eData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);
	
	// As of date
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	
	// Main curve name
	bool enableCalculation = true;
	AQLString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
	
	if (target == AQ_NO_DATA) 
	{
		throw AQLCoreInvalidData("#Error: Dual bootstrapping curve name is not detected",__FILE__,__LINE__);
	}
	else
	{
		enableCalculation = false;
	}

	target.toUpper();
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(target));
	
	AQLString ep_suffix	= (target == STD || target == SWAP) ? "" : "_" + target;
	AQLString suffix		= (target == STD || target == SWAP) ? "" : "." + target;
	suffix.toLower();
	
	// OIS curve name and suffix
	AQLString currentCurveName_ois = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_OISCURVENAME + suffix).toUpper();
	if (currentCurveName_ois == AQ_NO_DATA) 
	{
		throw AQLCoreInvalidData("#Error: OIS curve name is not found in performing dual bootstrapping", __FILE__, __LINE__); 
	}

	AQLString suffix_ois = (currentCurveName_ois == STD || currentCurveName_ois == SWAP) ? "" : "." + currentCurveName_ois;
	suffix_ois.toLower();

	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + ep_suffix, new AQLDataString(currentCurveName_ois));

	// Swap curve name and suffix
	AQLString currentCurveName_swap = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME + suffix);
	currentCurveName_swap.toUpper();
	if (currentCurveName_swap == AQ_NO_DATA) 
	{
		throw AQLCoreInvalidData("#Error: Swap curve name is not found in performing dual bootstrapping", __FILE__, __LINE__); 
	}

	AQLString suffix_swap = (currentCurveName_swap == STD || currentCurveName_swap == SWAP) ? "" : "." + currentCurveName_swap;
	suffix_swap.toLower();

	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE + ep_suffix, new AQLDataString(currentCurveName_swap));

	//-----------------------------------------------------------------
	// Extract common curve building parameters

	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	// Fast rebuild	
	AQLString tmpFastRebuild_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_FASTREBUILD + suffix).toUpper();
	bool fastRebuild = true;
	if (tmpFastRebuild_str == "FALSE")
	{
		fastRebuild = false;		
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix, new AQLDataBool(fastRebuild));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix, new AQLDataBool(fastRebuild));

	//get constant for convergence
	double eps = 1.0e-9;
	AQLString strEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_EPSILON + suffix);
	if (strEPS.toUpper() != AQ_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix, new AQLDataDouble(eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix, new AQLDataDouble(eps));

	double grad_eps = 1.0e-15;
	AQLString strGEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_GRADIENTEPSILON + suffix);
	if (strGEPS.toUpper() != AQ_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix, new AQLDataDouble(grad_eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix, new AQLDataDouble(grad_eps));

	double delta = 1.0e-10;
	AQLString strDLT = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_DELTA + suffix);
	if (strDLT.toUpper() != AQ_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix, new AQLDataDouble(delta));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix, new AQLDataDouble(delta));

	int maxLoop = 1000;
	AQLString strMLP = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_MAXLOOP + suffix);
	if (strMLP.toUpper() != AQ_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix, new AQLDataInt(maxLoop));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix, new AQLDataInt(maxLoop));

	//-----------------------------------------------------------------
	// Extract individual curve's configuration parameters

	// Define a number of control modes
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;
	
	// Set up curve Generate Config for the swap curve
	AQLString ep_suffix_swap = (currentCurveName_swap == STD || currentCurveName_swap == SWAP) ? "" : "_" + currentCurveName_swap;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, isArbFree, suffix_swap, ep_suffix_swap);

	// Set up curve Generate Config for the OIS curve
	AQLString ep_suffix_ois = (currentCurveName_ois == STD || currentCurveName_ois == SWAP) ? "" : "_" + currentCurveName_ois;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, isArbFree, suffix_ois, ep_suffix_ois);

	// Support on FX?
	AQLString fxName = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != AQ_NO_DATA)
	{
		basisCurveEngine->getFXEntity().convertFromString(fxName);
	}
		
	// OIS curve configuration
	configureCurve(mpStaticData,
			basisCurveEngine,
			yc,
			dataInstance,
			asOfDate,
			tmpCurrency,
			currentCurveName_ois,
			yieldDataName,
			suffix_ois,
			ep_suffix_ois,
			enableCalculation,
			isSpotUse,
			isAudExtra,
			isSwapTenorAdjust);
	
	const std::map<AQLString, AQLString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
	for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		const AQLString& curveName = it->first;
		basisCurveEngine->setDualBootstrapOISCurveMktMap(curveName, it->second);
	}

	// Store curve type to object pool under current engine name + current curve name
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_ois);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_ois, new AQLDataString("OIS"));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_ois);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_ois, new AQLDataString("OIS"));

	// Swap curve configuration
	configureCurve(mpStaticData,
			basisCurveEngine,
			yc,
			dataInstance,
			asOfDate,
			tmpCurrency,
			currentCurveName_swap,
			yieldDataName,
			suffix_swap,
			ep_suffix_swap,
			enableCalculation,
			isSpotUse,
			isAudExtra,
			isSwapTenorAdjust);

	// Store curve type to object pool under current engine name + current curve name
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_swap);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_swap, new AQLDataString("SWAP"));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_swap);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_swap, new AQLDataString("SWAP"));
		
	for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		const AQLString& curveName = it->first;
		basisCurveEngine->setDualBootstrapSwapCurveMktMap(curveName, it->second);
	}

	mCurveGenCcyMap[currency] = true;

	// Store all curve names
	AQLString allCurveNames = currentCurveName_ois + AQLString(MULTI_STATIC_DATA_DELIMITER) + currentCurveName_swap;
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix, new AQLDataString(allCurveNames));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix, new AQLDataString(allCurveNames));
	
	// Get CurveCalibration out of BasisCurveCalibration
	AQLDataProcedure &proc = dynamic_cast<AQLDataProcedure &>(basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());	
	const AQLCoreProcedure& yieldGenProcedure = proc.getMethod();
	const CurveCalibration& curveCalibrationEngine = dynamic_cast<const CurveCalibration& >(yieldGenProcedure);

	// Peform dual-bootstrapping
	AQLObject* parent = dynamic_cast<AQLObject* >(basisCurveEngine);
    curveCalibrationEngine.dualbootstrap(asOfDate, *parent, AQLDataProcedure());
	
}


//! 
/*!
@brief generate curve data using dual-bootstrapping technique

@param[in] currency  currency of the curve
@param[out] dataInstance
*/
void AQLUpdateObjectPoolForCurves::generateInitialValueGlobalEngineCurves(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

	// Get YieldCurve and BasisCurveCalibration objects (as dataValues)
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

	AQLString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		basisCurveEngine = new BasisCurveCalibration(&dataInstance);
		objPool.set(yieldProName, basisCurveEngine);
	}
	else
	{
		//we must not erase the reset method for only basisCurveEngine
		basisCurveEngine = &dynamic_cast<BasisCurveCalibration &>(objPool.getObject(yieldProName).get());
	}
	basisCurveEngine->getName().convertFromString(yieldProName);
	bool isArbFree = false;		// London don't use this flag and hence hardcode it to False
	basisCurveEngine->getIsArbFree().set(isArbFree);

	// Get YieldData object (as an data)
	AQLObject *eData = NULL;
	AQLString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const AQLObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new AQLObject();
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	eData->remove(CALIBRATION_DATA_NAME);
	eData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);

	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	// As of date
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// Main curve name
	bool enableCalculation = true;
	AQLString engineName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);

	if (engineName == AQ_NO_DATA)
	{
		throw AQLCoreInvalidData("#Error: Name of the global yield curve calibration engine is not detected", __FILE__, __LINE__);
	}
	else
	{
		enableCalculation = false;
	}

	engineName.toUpper();
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(engineName));

	AQLString ep_suffix = (engineName == STD || engineName == SWAP) ? "" : "_" + engineName;
	AQLString suffix = (engineName == STD || engineName == SWAP) ? "" : "." + engineName;
	suffix.toLower();

	//-----------------------------------------------------------------
	// Extract common curve building parameters

	// Fast rebuild	
	AQLString tmpFastRebuild_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_FASTREBUILD + suffix).toUpper();
	bool fastRebuild = true;
	if (tmpFastRebuild_str == "FALSE")
	{
		fastRebuild = false;
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix, new AQLDataBool(fastRebuild));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix, new AQLDataBool(fastRebuild));

	//get constant for convergence
	double eps = 1.0e-9;
	AQLString strEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_EPSILON + suffix);
	if (strEPS.toUpper() != AQ_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix, new AQLDataDouble(eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix, new AQLDataDouble(eps));

	double grad_eps = 1.0e-15;
	AQLString strGEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_GRADIENTEPSILON + suffix);
	if (strGEPS.toUpper() != AQ_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix, new AQLDataDouble(grad_eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix, new AQLDataDouble(grad_eps));

	double delta = 1.0e-10;
	AQLString strDLT = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_DELTA + suffix);
	if (strDLT.toUpper() != AQ_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix, new AQLDataDouble(delta));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix, new AQLDataDouble(delta));

	int maxLoop = 1000;
	AQLString strMLP = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_MAXLOOP + suffix);
	if (strMLP.toUpper() != AQ_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix, new AQLDataInt(maxLoop));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix, new AQLDataInt(maxLoop));

	//-----------------------------------------------------------------------------------------------------------
	// Loop through each curve in the engine and prepare its calibration parameters and data in the object pool

	// Define a number of control modes
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;

	AQLString allCurveTypes = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVETYPES + suffix);
	AQLStringVector curveTypeVector = allCurveTypes.toToken(MULTI_STATIC_DATA_DELIMITER);

	AQLString allCurveNames = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVENAMES + suffix);
	AQLStringVector curveNameVector = allCurveNames.toToken(MULTI_STATIC_DATA_DELIMITER);

	if (curveTypeVector.size() != curveNameVector.size())
	{
		AQ_THROW("Not all the curves in the global curve engine has been given a curve type.");
	}

	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix, new AQLDataString(allCurveNames));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix, new AQLDataString(allCurveNames));
	
	for (unsigned int i = 0; i < curveNameVector.size(); ++i)
	{
		// Get single curve's name and suffix
		AQLString currentCurveName = curveNameVector[i].toUpper();
		AQLString currentCurveType = curveTypeVector[i].toUpper();

		AQLString suffix_currentCurve = (currentCurveName == STD || currentCurveName == SWAP) ? "" : "." + currentCurveName;
		suffix_currentCurve.toLower();
		
		AQLString ep_suffix_currentCurve = (currentCurveName == STD || currentCurveName == SWAP) ? "" : "_" + currentCurveName;
		ep_suffix_currentCurve.toUpper();

		// Store curve type to object pool under current engine name + current curve name
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_currentCurve);
		basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_currentCurve, new AQLDataString(currentCurveType));
		yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_currentCurve);
		yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_currentCurve, new AQLDataString(currentCurveType));
				
		// Set up curve Generate Config for the current curve
		setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, isArbFree, suffix_currentCurve, ep_suffix_currentCurve);

		// Configuration of the current single curve
		configureCurve(mpStaticData,
			basisCurveEngine,
			yc,
			dataInstance,
			asOfDate,
			tmpCurrency,
			currentCurveName,
			yieldDataName,
			suffix_currentCurve,
			ep_suffix_currentCurve,
			enableCalculation,
			isSpotUse,
			isAudExtra,
			isSwapTenorAdjust);

		//const std::map<AQLString, AQLString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
		//for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		//{
		//	const AQLString& curveName = it->first;
		//	basisCurveEngine->setDualBootstrapOISCurveMktMap(curveName, it->second);
		//}
	}

	mCurveGenCcyMap[currency] = true;

	// Get CurveCalibration out of BasisCurveCalibration
	AQLDataProcedure &proc = dynamic_cast<AQLDataProcedure &>(basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	const AQLCoreProcedure& yieldGenProcedure = proc.getMethod();
	const CurveCalibration& curveCalibrationEngine = dynamic_cast<const CurveCalibration& >(yieldGenProcedure);

	// Peform dual-bootstrapping
	AQLObject* parent = dynamic_cast<AQLObject* >(basisCurveEngine);
    curveCalibrationEngine.buildEngineCurves(asOfDate, *parent, AQLDataProcedure());

}


/*!
    @brief 
*/
void AQLUpdateObjectPoolForCurves::configureCurve(AQLStaticData *mpStaticData,
							BasisCurveCalibration *basisCurveEngine,
							AQLMathYieldCurve *yc,
							AQLDataInstance &dataInstance,
							const AQLDate& asOfDate,
							const AQLString& currency,
							const AQLString& curveName,
							const AQLString& yieldDataName,
							const AQLString& suffix,
							const AQLString& ep_suffix,
							bool enableCalculation,
							bool isSpotUse,
							bool isAudExtra,
							bool isSwapTenorAdjust) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();	

	AQLString currentCurveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
	currentCurveType.toUpper();

	// reference data for market pro
	AQLString refData;
	AQLString refBData;
	std::map<AQLString, std::map<AQLString, double> > aud_origSwapRate;

	if (enableCalculation || currentCurveType == SWAP) 
	{
		// Variable 'target' is essentially the name of the curve
		setUpGenCurveData(dataInstance, refData, asOfDate, currency, SWAP, yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate, AQLString(curveName).toUpper());
	}

	// Get isRenAdj
	bool isRenAdj = false;	
	AQLString tmpRenAdj_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST + suffix).toUpper();
	if (tmpRenAdj_str == "TRUE")
	{
		isRenAdj = true;
	}

	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + ep_suffix, new AQLDataBool(isRenAdj));

	// Get the list of curve indexes
	AQLStringVector markets;
	AQLString tmpMarket = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffix).toUpper();
	if (tmpMarket == AQ_NO_DATA)
	{
		markets = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		markets = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffix).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(markets);

	// set yield curve pro
	if (refData.size() && (curveName == STD || curveName == SWAP) )
	{
		refData = refData.subString(0, refData.size() - 2);
		basisCurveEngine->getMarketData().convertFromString(refData);
	}

	AQLDataHolder* dh;
	AQLStringVector swapCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) 
	{
		swapCurves = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}
			
	AQLStringVector basisCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		basisCurves = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}

	AQLString mainBDF = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_DF2);
	AQLStringVector genBasisSwapMarket;
	if (!markets.empty() && markets[0] != AQ_NO_DATA)
	{
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			AQLString refData_;
			AQLString suffix = "." + markets[i];
			suffix.toLower();

			// set curve type			
			AQLString curveType;
			AQLObjectHolder& yieldData = basisCurveEngine->getYieldData().get();

			if (markets[i] == SWAP)
			{
				curveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE);
				if (curveType != AQ_NO_DATA)
				{
					basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_CURVETYPE);
					basisCurveEngine->AQLObject::add(CALIBRATION_DATA_CURVETYPE, new AQLDataString(curveType.toUpper())); 

					yieldData.remove(CALIBRATION_DATA_CURVETYPE);
					yieldData.add(CALIBRATION_DATA_CURVETYPE, new AQLDataString(curveType.toUpper())); 
				}
			}
			else
			{
				curveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
				if (curveType != AQ_NO_DATA)
				{
					basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + markets[i]);
					basisCurveEngine->AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + markets[i], new AQLDataString(curveType.toUpper())); 

					yieldData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + markets[i]);
					yieldData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + markets[i], new AQLDataString(curveType.toUpper())); 
				}
			}

			if (markets[i] == SWAP)
			{
				if (enableCalculation || curveName == STD)
				{
					AQLString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE);
					if (tmpAssignedCurves != AQ_NO_DATA)
					{
						AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
						for (size_t i = 0; i<assignedCurves.size(); i++)
						{
							if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),assignedCurves[i]))
							{
								swapCurves.push_back(assignedCurves[i]);
							}
						}

						if (assignedCurves.end() == std::find(assignedCurves.begin(), assignedCurves.end(), STD))
						{
							basisCurveEngine->setAssignedCurveMktMap(STD,SWAP);
							if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),STD))
							{
								swapCurves.push_back(STD);
							}
						}
					}
					else
					{
						if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),STD))
						{
							swapCurves.push_back(STD);
						}
					}
				}
				continue; 
			}
			
			AQLString marketType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffix).toUpper();	
			basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_MARKETTYPE + AQLString("_") + markets[i]);
			basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_MARKETTYPE + AQLString("_") + markets[i], new AQLDataString(marketType));

			if (marketType == AQ_NO_DATA)
			{
				AQLString isBasisStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ISBASIS + suffix).toUpper();
				AQLString isReadFile = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffix).toUpper();
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
			AQLString strInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix).toUpper();
			if (strInter != AQ_NO_DATA)
			{
				yc->getInterpolation(markets[i]).convertFromString(strInter.toLower());
				basisCurveEngine->getInterpolation(markets[i]).convertFromString(strInter);
				if (mainBDF == markets[i])
				{
					yc->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
					basisCurveEngine->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
				}
			}
	
			if ( marketType == MARKETTYPE_BASIS )
			{
				if (currentCurveType == "OIS")
				{
					continue;
				}

				AQLString tmpMktName = markets[i];
				genBasisSwapMarket.push_back(markets[i]);
				AQLString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != AQ_NO_DATA && (enableCalculation || curveName == markets[i]))
				{
					AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
					for (unsigned int j = 0; j < assignedCurves.size(); j++)
					{
						if (basisCurves.end() == std::find(basisCurves.begin(),basisCurves.end(),assignedCurves[j]))
						{
							basisCurves.push_back(assignedCurves[j]);
						}
					}
					AQLStringVector tmpMktNames = markets[i].toToken('_');
					if (tmpMktNames.size() == 2)
					{
						setUpBasisCurveData(dataInstance, refData_, asOfDate, currency, tmpMktNames[1], yieldDataName, isSpotUse, *basisCurveEngine, curveName, &tmpMktNames[0]);
					}
					else
					{
						setUpBasisCurveData(dataInstance, refData_, asOfDate, currency, markets[i], yieldDataName, isSpotUse, *basisCurveEngine, curveName);
					}
				}
				else
				{
					if (curveName == markets[i])
					{
						if (basisCurves.end() == std::find(basisCurves.begin(),basisCurves.end(),markets[i]))
						{
							basisCurves.push_back(markets[i]);
						}
						setUpBasisCurveData(dataInstance, refData_, asOfDate, currency, markets[i], yieldDataName, isSpotUse, *basisCurveEngine, curveName);
					}
					else if (enableCalculation || (curveName == CURVETYPE_FLOATER && markets[i] == XCCYBASIS))
					{
						setUpBasisCurveData(dataInstance, refData_, asOfDate, currency, markets[i], yieldDataName, isSpotUse, *basisCurveEngine, curveName);
					}
					else if (!enableCalculation && isSwapTenorAdjust)
					{
						//only entitysetup
						AQLString tenorswapname = dynamic_cast<const AQLDataString &>(basisCurveEngine->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
						if (tenorswapname == markets[i])
						{
							setUpBasisCurveData(dataInstance, refData_, asOfDate, currency, markets[i], yieldDataName, isSpotUse, *basisCurveEngine, curveName);
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
			else if ( marketType == MARKETTYPE_ZERORATE )
			{
				if (!enableCalculation && curveName != markets[i])
				{
					continue;
				}
		
				setUpCurveDataByReadFile(dataInstance, asOfDate, currency, markets[i], yieldDataName, *basisCurveEngine);
				continue;
			}
			else if( marketType == MARKETTYPE_SWAP )
			{
				if (!enableCalculation && curveName != markets[i])
				{
					continue;
				}

				AQLString tmpMktName = markets[i];
				AQLString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != AQ_NO_DATA)
				{
					AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
					for (size_t i = 0; i<assignedCurves.size(); i++)
					{
						if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),assignedCurves[i]))
						{
							swapCurves.push_back(assignedCurves[i]);
						}
					}
				}
				else
				{
					if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),markets[i]))
					{
						swapCurves.push_back(markets[i]);
					}
				}

				AQLString isoismode = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix).toUpper();
				if (isoismode.toUpper() == "DAILYCOMPOUNDING")
				{
					setUpGenCurveDataOIS(dataInstance, refData_, asOfDate, currency, markets[i], yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate);
				}
				else
				{
					setUpGenCurveData(dataInstance, refData_, asOfDate, currency, markets[i], yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate, curveName);			
				}
			}
			else
			{
				throw AQLCoreInvalidData("Market type is not supported!!", __FILE__, __LINE__); 
			}

			// set yield curve pro
			refData_ = refData_.subString(0, refData_.size() - 2);
			AQLStringVector tmpMktNames = markets[i].toToken('_');
			if (tmpMktNames.size() == 2)
			{
				BasisCurveCalibration &fYcPro = dynamic_cast<BasisCurveCalibration &>
						(objPool.getObject(AQLMarketData::getBaseYieldProName(tmpMktNames[0]), ENCHKTYPE_ISDEFINED).get());
				fYcPro.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tmpMktNames[1]);
				fYcPro.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tmpMktNames[1], new AQLDataMultiReference()).convertFromString(refData_);
			}
			else
			{
				basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i]);
				basisCurveEngine->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i], new AQLDataMultiReference()).convertFromString(refData_);
			}			
		}
	}

	//set tenorswap convention
	AQLString tenorSwapName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(markets.begin(), markets.end(), tenorSwapName) == markets.end() && tenorSwapName != AQ_NO_DATA)
	{
		AQLString refData_tenor = "";
		setUpBasisCurveData(dataInstance, refData_tenor, asOfDate, currency, tenorSwapName, yieldDataName, isSpotUse, *basisCurveEngine, curveName);
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName);
		basisCurveEngine->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName, new AQLDataMultiReference()).convertFromString(refData_tenor);
	}

	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(swapCurves));

	if (mainBDF != AQ_NO_DATA)
	{
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new AQLDataString(mainBDF));
	}
			
	dataInstance.getReferencePool().completeDependency();
	
	// Whether to generate output forward rates only from swaps or from swaps + futures/FRA
	AQLString swapOnly = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY);
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY);
	if (swapOnly != AQ_NO_DATA)
	{
		if (swapOnly.toUpper() == "TRUE")
		{
			basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new AQLDataBool(true));
		}
		else
		{
			basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new AQLDataBool(false));
		}
	}

	if (!aud_origSwapRate.empty())
	{
		map<AQLString, map<AQLString, double> >::const_iterator it = aud_origSwapRate.begin();
		while (it != aud_origSwapRate.end())
		{
			AQLCurveMarketDataHelpers::restoreSwapRateFromL(*basisCurveEngine, it->second, currency, &(it->first));
			++it;
		}
	}

	// Data settings that are consistent with the single-curve building mechanism of basis curve
	bool hasNoBasisCurve = false;
	hasNoBasisCurve = genBasisSwapMarket.end() == std::find(genBasisSwapMarket.begin(), genBasisSwapMarket.end(), curveName);
	AQLString tmpMktName;
	if (curveName != STD)
	{
		tmpMktName = AQLString(".") + curveName;
		tmpMktName.toLower();
	}

	AQLString tmpAssignedCurves;
	if (hasNoBasisCurve)
	{
		tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + tmpMktName);
	}
	else
	{
		tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + tmpMktName);
	}

	AQLString targetCurve;
	if (tmpAssignedCurves != AQ_NO_DATA && tmpAssignedCurves != "")
	{
		AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
		targetCurve = assignedCurves[0];
	}
	else
	{
		AQ_THROW("No target basis curve defined")
	}

	if (!hasNoBasisCurve)
	{
		AQLString suffix = AQLString("_") + targetCurve;
		suffix.toUpper();
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF + suffix);
		basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF + suffix, new AQLDataString(targetCurve));
	}
	
	// basis
	bool isBasis = false;
	if (!basisCurves.empty() && !hasNoBasisCurve)
	{
		isBasis = true;
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_BASISDFS, new AQLDataStrings(basisCurves));
	}

}


//! 
/*!
    @brief generate sde initial curve data

	@param[in] currency  currency or fx ex.JPY/USD
	@param[out] dataInstance
*/
void
AQLUpdateObjectPoolForCurves::generateInitialValue(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

///// update for XLL Plus in grid //////////////////
	AQLString useYieldSDEIRStr = AQLCoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
	if (useYieldSDEIRStr != AQ_NO_DATA)
	{
		AQLDataBool tmpAttrBool;
		tmpAttrBool.convertFromString(useYieldSDEIRStr);
		if (tmpAttrBool.get())
		{
			generateInitialValueForPricer(currency, dataInstance);
			mCurveGenCcyMap[currency] = true;
			return;
		}
	}
////////////////////////////////////////////////////

	// arbfree generate flag
	bool isArbFree = false;
	AQLString strIsArb = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);
	if (strIsArb != AQ_NO_DATA)
	{
		AQLDataBool tmpAttrBool;
		tmpAttrBool.convertFromString(strIsArb);
		isArbFree = tmpAttrBool.get();
	}
	if (isArbFree)
	{
		generateInitialValueArbfree(currency, dataInstance);
		mCurveGenCcyMap[currency] = true;
		return;
	}

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
	
	AQLString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		basisCurveEngine = new BasisCurveCalibration(&dataInstance);
		objPool.set(yieldProName, basisCurveEngine);
	}
	else
	{
		//we must not erase the reset method for only basisCurveEngine
		basisCurveEngine = &dynamic_cast<BasisCurveCalibration &>(objPool.getObject(yieldProName).get());
	}
	basisCurveEngine->getName().convertFromString(yieldProName);

	basisCurveEngine->getIsArbFree().set(false);

	AQLObject *eData = NULL;
	AQLString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const AQLObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new AQLObject();
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	eData->remove(CALIBRATION_DATA_NAME);
	eData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);
	
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;
	
	bool enableCalculation = true;
	AQLString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET).toUpper();
	if (target != AQ_NO_DATA) 
	{
		enableCalculation = false;
	}

	AQLString suffix = (target == STD || target == SWAP) ? "" : "." + target;
	suffix.toLower();

	AQLString ep_suffix = (target == STD || target == SWAP) ? "" : "_" + target;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, isArbFree, suffix, ep_suffix);

	AQLString fxName = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != AQ_NO_DATA)
	{
		basisCurveEngine->getFXEntity().convertFromString(fxName);
	}

	// reference data for market pro
	AQLString refData;
	AQLString refBData;
	map<AQLString, map<AQLString, double> > aud_origSwapRate;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	// setup context key
	AQLString contextKey = currency;
	contextKey.toLower();

	// setup risk info
	bool isrisk=false;
	isrisk |= AQLCoreDataService::getContext(ARG_KEY_OFFICIALRISK)!=AQ_NO_DATA? true: false;
	isrisk |= AQLCoreDataService::getContext(ARG_KEY_FRONTRISK)!=AQ_NO_DATA? true: false;

	AQLString contextYield = AQLCoreDataService::getContext(contextKey+CONTEXT_KEY_SDE_YIELD);

	if (!isrisk&&contextYield!=AQ_NO_DATA)
	{
		AQLDataDoubleMatrix matrix;
		matrix.convertFromString(contextYield);
		unsigned int rowSize = matrix.get1DSize();
		unsigned int colSize = matrix.getSize(0);

		bool isDF2 = colSize>2? true: false;
		DoubleArray terms(rowSize);
		DoubleArray dfs(rowSize);
		DoubleArray dfs2(rowSize);
		for(unsigned int i=0; i<rowSize; i++)
		{
			terms[i] = matrix.get(i,0);
			dfs[i] = matrix.get(i,1);

			if (isDF2)
				dfs2[i] = matrix.get(i,2);
		}
		dynamic_cast<AQLDataDoubles &>(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<AQLDataDoubles &>(eData->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		if (isDF2)
		{
			AQLDataDoubles* attrDF2 = NULL;
			AQLDataHolder* df2H = &eData->getData(IR_CALIBRATION_DATA_DFS2);
			if (!df2H->isDefined())
			{
				attrDF2 = new AQLDataDoubles();
				eData->remove(IR_CALIBRATION_DATA_DFS2);
				eData->add(IR_CALIBRATION_DATA_DFS2,attrDF2);
			}
			else
			{
				attrDF2 = dynamic_cast<AQLDataDoubles*>(&df2H->get());
			}
			attrDF2->set(dfs2);
		}

		return;
	}

	// Current curve type
	// This is the type of the curve currently being constructed. To be distinguished from
	// the curve type within the for loop below
	AQLString currentCurveName("");
	if (target != STD)
	{
		currentCurveName = "." + target;
		currentCurveName.toLower();
	}	
	AQLString currentCurveType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + currentCurveName);
	currentCurveType.toUpper();

	if (enableCalculation || currentCurveType == SWAP) 
	{
		// Variable 'target' is essentially the name of the curve
		setUpGenCurveData(dataInstance, refData, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate, AQLString(target).toUpper());
	}
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //set isRenAdj
    bool isRenAdj = false;	
	AQLString tmpRenAdj_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST + suffix).toUpper();
	if (tmpRenAdj_str == "TRUE")
	{
		isRenAdj = true;
    }

	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + ep_suffix);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + ep_suffix, new AQLDataBool(isRenAdj));

	AQLStringVector markets;
	AQLString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == AQ_NO_DATA)
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(markets);

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	if (markets.empty() || markets[0] == AQ_NO_DATA)
	{
		// old type (exo etc..)
		AQLString basisCurrency = mpStaticData->getStaticData(KEY_SDE_BASIS_BASE_CURRENCY);
		basisCurrency.toUpper();

		//if basisCurrency is empty, not set up basis curve
		if (AQLDealUtils::getSDECurrencys().size() > 1 && tmpCurrency.toUpper() != basisCurrency && basisCurrency != AQ_NO_DATA)
		{
			tmpCurrency.toLower();
			// if not base currency set basis curve
			AQLString basisFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASISSWAP_FILE);
			AQLFileAccessor basisFile(AQLMarketData::getNumFileName(basisFileName));
			AQLStringMatrix basisDataMtx;
			basisFile.readAllData(MARKET_DATA_DELIMITER, basisDataMtx);
			basisFile.close();

			// get cal and calc spot date
			AQLPriceDataCalendar calB;
			AQLString calBStr =  mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_CALENDAR);
			calB.convertFromString(calBStr);
			AQLDate spotDateB;
			if (isSpotUse)
			{
				spotDateB.setDate(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_SPOTDATE).getCString());
			}
			else
			{
				spotDateB = calB.getBusinessDay(asOfDate, mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_RESETLAG).getIntValue());
			}
			// get freq
			AQLString freqBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY).toUpper();
			// get daycount
			AQLString daycBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT).toUpper();
			// get sliding
			AQLString slidingBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE).toUpper();
			// get base swap info
			// get freq
			AQLString freqBBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASEFREQUENCY).toUpper();
			// get daycount
			AQLString daycBBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASEDAYCOUNT).toUpper();
			// get sliding
			AQLString slidingBBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASESLIDINGRULE).toUpper();
			// get calendar
			AQLString calBBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASECALENDAR);
			// baseyield name
			AQLString basecur = currency;
			AQLString baseYieldName = PREFIX_YIELD + getSDEAttrName(basecur.toUpper());
			
			
			const int basisSize = basisDataMtx.size();
			for (int i = 0; i < basisSize; ++i)
			{
				AQLString term = basisDataMtx[i][0].toUpper();
				double basis = basisDataMtx[i][1].getDoubleValue();

				AQLObject *mktData = new AQLObject();
				AQLString nameB = yieldDataName + "_BASIS_" + AQLString(i);
				refData += nameB + ":";
				refBData += nameB + ":";
				// set name
				mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameB);
				// set spot date
				mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateB));
				// set calendar
				mktData->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calBStr);
				// set daycount
				mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycBStr);
				// set data type
				mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BASIS);
				// set frequency
				mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freqBStr);
				// set slidingrule
				mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingBStr);
				// set term
				mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));
				// set rate
				mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(basis / 10000.0));
				// set base basis info
				// freq
				mktData->add(IR_CALIBRATION_DATA_FREQUENCYBASE, new AQLDataString).convertFromString(freqBBStr);
				// calender
				mktData->add(IR_CALIBRATION_DATA_CALENDARBASE, new AQLPriceDataCalendar()).convertFromString(calBBStr);
				// daycount
				mktData->add(IR_CALIBRATION_DATA_DAYCOUNTBASE, new AQLPriceDataDayCount()).convertFromString(daycBBStr);
				// sliding
				mktData->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE, new AQLPriceDataSlidingRule()).convertFromString(slidingBBStr);

				// set object pool
				objPool.set(nameB, mktData);
			}
			// set base curve reference
			basisCurveEngine->getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(baseYieldName);
		}
	}
#endif

	// set yield curve pro
	if (refData.size() && (target == STD || target == SWAP) )
	{
		refData = refData.subString(0, refData.size() - 2);
		basisCurveEngine->getMarketData().convertFromString(refData);
	}

	AQLDataHolder* dh;
	AQLStringVector swapCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) 
	{
		swapCurves = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}
			
	AQLStringVector basisCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		basisCurves = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}

	AQLString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	AQLStringVector genBasisSwapMarket;
	if (!markets.empty() && markets[0] != AQ_NO_DATA)
	{
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			tmpCurrency.toLower();
			AQLString refData_;
			AQLString suffix = "." + markets[i];
			suffix.toLower();

            // set curve type			
			AQLString curveType;
            AQLObjectHolder& yieldData = basisCurveEngine->getYieldData().get();

			if (markets[i] == SWAP)
			{
				curveType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE);
				if (curveType != AQ_NO_DATA)
				{
					basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_CURVETYPE);
					basisCurveEngine->AQLObject::add(CALIBRATION_DATA_CURVETYPE, new AQLDataString(curveType.toUpper())); 

                    yieldData.remove(CALIBRATION_DATA_CURVETYPE);
                    yieldData.add(CALIBRATION_DATA_CURVETYPE, new AQLDataString(curveType.toUpper())); 
				}
			}
			else
			{
				curveType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
				if (curveType != AQ_NO_DATA)
				{
					basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + markets[i]);
					basisCurveEngine->AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + markets[i], new AQLDataString(curveType.toUpper())); 

                    yieldData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + markets[i]);
                    yieldData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + markets[i], new AQLDataString(curveType.toUpper())); 
				}
			}

			if (markets[i] == SWAP)
			{
				if (enableCalculation || target == STD)
				{
					AQLString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE);
					if (tmpAssignedCurves != AQ_NO_DATA)
					{
						AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
						for (size_t i = 0; i<assignedCurves.size(); i++)
						{
							if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),assignedCurves[i]))
							{
								swapCurves.push_back(assignedCurves[i]);
							}
						}

						if (assignedCurves.end() == std::find(assignedCurves.begin(), assignedCurves.end(), STD))
						{
							basisCurveEngine->setAssignedCurveMktMap(STD,SWAP);
							if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),STD))
							{
								swapCurves.push_back(STD);
							}
						}
					}
					else
					{
						if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),STD))
						{
							swapCurves.push_back(STD);
						}
					}
				}
				continue; 
			}
			
			AQLString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffix).toUpper();	
			basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_MARKETTYPE + AQLString("_") + markets[i]);
			basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_MARKETTYPE + AQLString("_") + markets[i], new AQLDataString(marketType));

			if (marketType == AQ_NO_DATA)
			{
				AQLString isBasisStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISBASIS + suffix).toUpper();
				AQLString isReadFile = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffix).toUpper();
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
			AQLString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix).toUpper();
			if (strInter != AQ_NO_DATA)
			{
				yc->getInterpolation(markets[i]).convertFromString(strInter.toLower());
				basisCurveEngine->getInterpolation(markets[i]).convertFromString(strInter);
				if (mainBDF == markets[i])
				{
					yc->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
					basisCurveEngine->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
				}
			}
	
			if ( marketType == MARKETTYPE_BASIS )
			{
				if (currentCurveType == "OIS")
				{
					continue;
				}

				AQLString tmpMktName = markets[i];
				genBasisSwapMarket.push_back(markets[i]);
				AQLString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != AQ_NO_DATA && (enableCalculation || target == markets[i]))
				{
					AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
					for (unsigned int j = 0; j < assignedCurves.size(); j++)
					{
						if (basisCurves.end() == std::find(basisCurves.begin(),basisCurves.end(),assignedCurves[j]))
						{
							basisCurves.push_back(assignedCurves[j]);
						}
					}
					AQLStringVector tmpMktNames = markets[i].toToken('_');
					if (tmpMktNames.size() == 2)
					{
						setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, tmpMktNames[1], yieldDataName, isSpotUse, *basisCurveEngine, target, &tmpMktNames[0]);
					}
					else
					{
						setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *basisCurveEngine, target);
					}
				}
				else
				{
					if (target == markets[i])
					{
						if (basisCurves.end() == std::find(basisCurves.begin(),basisCurves.end(),markets[i]))
						{
							basisCurves.push_back(markets[i]);
						}
						setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *basisCurveEngine, target);
					}
					else if (enableCalculation || (target == CURVETYPE_FLOATER && markets[i] == XCCYBASIS))
					{
						setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *basisCurveEngine, target);
					}
					else if (!enableCalculation && isSwapTenorAdjust)
					{
						//only entitysetup
						AQLString tenorswapname = dynamic_cast<const AQLDataString &>(basisCurveEngine->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
						if (tenorswapname == markets[i])
						{
							setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *basisCurveEngine, target);
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
			else if ( marketType == MARKETTYPE_ZERORATE )
			{
				if (!enableCalculation && target != markets[i])
				{
					continue;
				}
		
				setUpCurveDataByReadFile(dataInstance, asOfDate, tmpCurrency, markets[i], yieldDataName, *basisCurveEngine);
				continue;
			}
			else if( marketType == MARKETTYPE_SWAP )
			{
				if (!enableCalculation && target != markets[i])
				{
					continue;
				}

				AQLString tmpMktName = markets[i];
				AQLString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != AQ_NO_DATA)
				{
					AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
					for (size_t i = 0; i<assignedCurves.size(); i++)
					{
						if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),assignedCurves[i]))
						{
							swapCurves.push_back(assignedCurves[i]);
						}
					}
				}
				else
				{
					if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),markets[i]))
					{
						swapCurves.push_back(markets[i]);
					}
				}

				AQLString isoismode = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix).toUpper();
				if (isoismode.toUpper() == "DAILYCOMPOUNDING")
				{
					setUpGenCurveDataOIS(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate);
				}
				else
				{
					setUpGenCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate, target);			
				}
			}
			else
			{
				throw AQLCoreInvalidData("Market type is not supported!!", __FILE__, __LINE__); 
			}

			// set yield curve pro
			refData_ = refData_.subString(0, refData_.size() - 2);
			AQLStringVector tmpMktNames = markets[i].toToken('_');
			if (tmpMktNames.size() == 2)
			{
				BasisCurveCalibration &fYcPro = dynamic_cast<BasisCurveCalibration &>
						(objPool.getObject(AQLMarketData::getBaseYieldProName(tmpMktNames[0]), ENCHKTYPE_ISDEFINED).get());
				fYcPro.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tmpMktNames[1]);
				fYcPro.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tmpMktNames[1], new AQLDataMultiReference()).convertFromString(refData_);
			}
			else
			{
				basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i]);
				basisCurveEngine->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i], new AQLDataMultiReference()).convertFromString(refData_);
			}			
		}
	}

	//set tenorswap convention
	AQLString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(markets.begin(), markets.end(), tenorSwapName) == markets.end() && tenorSwapName != AQ_NO_DATA)
	{
		AQLString refData_tenor = "";
		setUpBasisCurveData(dataInstance, refData_tenor, asOfDate, tmpCurrency, tenorSwapName, yieldDataName, isSpotUse, *basisCurveEngine, target);
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName);
		basisCurveEngine->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName, new AQLDataMultiReference()).convertFromString(refData_tenor);
	}

	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(swapCurves));

	if (mainBDF != AQ_NO_DATA)
	{
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new AQLDataString(mainBDF));
	}
	
	// generate yield data
	AQLDataProcedure &proc = dynamic_cast<AQLDataProcedure &>(basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

//	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	bool isCurveTargetFound = false;
	if (!enableCalculation)
	{
		isCurveTargetFound = (genBasisSwapMarket.end() == std::find(genBasisSwapMarket.begin(),genBasisSwapMarket.end(), target) 
							    && target != CURVETYPE_FLOATER);
		if (target != CURVETYPE_FLOATER)
		{
			AQLString tmpMktName;
			if (target != STD) 
			{
				tmpMktName = AQLString(".") + target; 
				tmpMktName.toLower();
			}

			AQLString tmpAssignedCurves;
			if (isCurveTargetFound)
			{
				tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + tmpMktName);
			}
			else
			{
				tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + tmpMktName);
			}

			AQLString targetDF;
			if (tmpAssignedCurves != AQ_NO_DATA && tmpAssignedCurves != "")
			{
				AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
				targetDF = assignedCurves[0];
			}
			else
			{
				throw AQLCoreInvalidData("No target df!", __FILE__, __LINE__); 
			}

			if (isCurveTargetFound)
			{
				basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
				basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(targetDF));
			}
			else
			{
				basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
				basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new AQLDataString(targetDF));
			}
		}	
	}
	dataInstance.getReferencePool().completeDependency();

	// get fwdfx flag
	bool isFwdFX = false;
	AQLString isFwdFXStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX);
	if (isFwdFXStr.toUpper() == "TRUE") 
	{
		isFwdFX = true;
	}

	// Whether to generate output forward rates only from swaps or from swaps + futures/FRA
	AQLString swapOnly = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY);
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY);
	if (swapOnly != AQ_NO_DATA)
	{
		if (swapOnly.toUpper() == "TRUE")
		{
			basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new AQLDataBool(true));
		}
		else
		{
			basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new AQLDataBool(false));
		}
	}
	
	if ((enableCalculation || isCurveTargetFound) && !isFwdFX)
	{
		if (isCurveTargetFound)
		{
			basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new AQLDataBool(true));
		}
		proc.estimate(asOfDate);
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	}
	
	// basis
	bool isBasis = false;
	if (!basisCurves.empty() && !isCurveTargetFound && target != CURVETYPE_FLOATER)
	{
		isBasis = true;
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_BASISDFS, new AQLDataStrings(basisCurves));
	}
	
	if (refBData.size() > 1)
	{
		isBasis = true;
		// set reference for basis
		refBData = refBData.subString(0, refBData.size() - 2);
		basisCurveEngine->getBasisData().convertFromString(refBData);
	}

	if (isBasis)
	{
		bool isBasisEnabled = (!enableCalculation && !isCurveTargetFound);
		if (isBasisEnabled)
		{
			basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new AQLDataBool(true));
		}
		basisCurveEngine->setBasisRates();
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
	}

	AQLString genFloaterName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != AQ_NO_DATA && 
		(enableCalculation || target == CURVETYPE_FLOATER)) 
	{
		setUpFloater(tmpCurrency, *basisCurveEngine, genFloaterName);
	}

	// set df2
	if (target == STD && mainBDF != AQ_NO_DATA)
	{
		basisCurveEngine->setDF2();
	}

	if (!aud_origSwapRate.empty())
	{
		map<AQLString, map<AQLString, double> >::const_iterator it = aud_origSwapRate.begin();
		while (it != aud_origSwapRate.end())
		{
			AQLCurveMarketDataHelpers::restoreSwapRateFromL(*basisCurveEngine, it->second, currency, &(it->first));
			++it;
		}
	}
	// set daycount
//	setUpCurveTypeDayCount(*basisCurveEngine, *yc);

	mCurveGenCcyMap[currency] = true;
////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	if (contextYield!=AQ_NO_DATA)
	{
		AQLDataDoubleMatrix matrix;
		matrix.convertFromString(contextYield);
		unsigned int rowSize = matrix.get1DSize();
		unsigned int colSize = matrix.getSize(0);

		bool isDF2 = colSize>2? true: false;
		DoubleArray terms(rowSize);
		DoubleArray dfs(rowSize);
		DoubleArray dfs2(rowSize);
		for(unsigned int i=0; i<rowSize; i++)
		{
			terms[i] = matrix.get(i,0);
			dfs[i] = matrix.get(i,1);

			if (isDF2)
				dfs2[i] = matrix.get(i,2);
		}
		dynamic_cast<AQLDataDoubles &>(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<AQLDataDoubles &>(eData->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		eData->remove(IR_CALIBRATION_DATA_DFS2);
		if (isDF2)
		{
			eData->add(IR_CALIBRATION_DATA_DFS2,new AQLDataDoubles(dfs2));
		}

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		AQLStringVector dataoutCurves;
		dataoutCurves.push_back(STD);	

		const std::map<AQLString, AQLString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
            if (it->first != STD && std::find(markets.begin(), markets.end(), it->second) != markets.end()) {
                dataoutCurves.push_back(it->first);
            }
		}
		// floater curve
		if (genFloaterName != AQ_NO_DATA) dataoutCurves.push_back(genFloaterName);
		dataoutCurve(dataoutCurves, *eData, yieldDataName);
	}
}

//! 
/*!
    @brief set up curveType daycount

	@param[out] basisCurveEngine
	@param[out] yc
*/
void
AQLUpdateObjectPoolForCurves::setUpCurveTypeDayCount(BasisCurveCalibration &basisCurveEngine, AQLMathYieldCurve &yc) const
{
	// set daycount
	const map<AQLString, AQLString> &assignedCurveMktMap = basisCurveEngine.getAssignedCurveMktMap();
	map<AQLString, AQLString>::const_iterator it_cur = assignedCurveMktMap.begin();
	while (it_cur != assignedCurveMktMap.end())
	{
		AQLPriceDataDayCount dc;
		AQLPriceDataCalendar cal;
		AQLPriceDataSlidingRule sld;
		AQLString accessory;
		basisCurveEngine.getForwardConvention(it_cur->first, dc, sld, cal, accessory);
		yc.getDayCount(it_cur->first) = dc;
		basisCurveEngine.getDayCount(it_cur->first) = dc;
		++it_cur;
	}
}

//! 
/*!
    @brief generate sde initial curve data when arbfree curve is generated

	@param[in] currency  currency or fx ex.JPY/USD
	@param[out] dataInstance
*/
void
AQLUpdateObjectPoolForCurves::generateInitialValueArbfree(const AQLString &currency, AQLDataInstance &dataInstance) const
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
	
	AQLString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		basisCurveEngine = new BasisCurveCalibration(&dataInstance);
		objPool.set(yieldProName, basisCurveEngine);
	}
	else
	{
		//we must not erase the reset method for only basisCurveEngine
		basisCurveEngine = &dynamic_cast<BasisCurveCalibration &>(objPool.getObject(yieldProName).get());
	}
	basisCurveEngine->getName().convertFromString(yieldProName);

	AQLObject *eData = NULL;
	AQLString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const AQLObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new AQLObject();
		eData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;

	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, true);
	// set isArbFree
	basisCurveEngine->getIsArbFree().set(true);
	bool isRenAdj = false;
	AQLString tmpRenAdj_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST).toUpper();
	if (tmpRenAdj_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(tmpRenAdj_str);
		isRenAdj = tmpAttrB.get();
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

	eData->remove(IR_CALIBRATION_DATA_6MLCURVENAMES);
	eData->remove(IR_CALIBRATION_DATA_DFCURVENAMES);
	eData->remove(IR_CALIBRATION_DATA_3MLCURVENAMES);	
	eData->add(IR_CALIBRATION_DATA_6MLCURVENAMES, new AQLDataStrings(curveNames_6ML));
	eData->add(IR_CALIBRATION_DATA_DFCURVENAMES, new AQLDataStrings(curveNames_DF));
	eData->add(IR_CALIBRATION_DATA_3MLCURVENAMES, new AQLDataStrings(curveNames_3ML));
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new AQLDataBool(isRenAdj));
	basisCurveEngine->setArbFreeCurveName(curveNames_6ML,curveNames_DF,curveNames_3ML);

	// reference data for market pro
	AQLString refData;
	AQLString refBData;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	// setup context key
	AQLString contextKey = currency;
	contextKey.toLower();

	// setup risk info
	bool isrisk=false;
	isrisk |= AQLCoreDataService::getContext(ARG_KEY_OFFICIALRISK)!=AQ_NO_DATA? true: false;
	isrisk |= AQLCoreDataService::getContext(ARG_KEY_FRONTRISK)!=AQ_NO_DATA? true: false;

	AQLString contextYield = AQLCoreDataService::getContext(contextKey+CONTEXT_KEY_SDE_YIELD);

	if (!isrisk&&contextYield!=AQ_NO_DATA)
	{
		AQLDataDoubleMatrix matrix;
		matrix.convertFromString(contextYield);
		unsigned int rowSize = matrix.get1DSize();
		unsigned int colSize = matrix.getSize(0);

		bool isDF2 = colSize>2? true: false;
		DoubleArray terms(rowSize);
		DoubleArray dfs(rowSize);
		DoubleArray dfs2(rowSize);
		for(unsigned int i=0; i<rowSize; i++)
		{
			terms[i] = matrix.get(i,0);
			dfs[i] = matrix.get(i,1);

			if (isDF2)
				dfs2[i] = matrix.get(i,2);
		}
		dynamic_cast<AQLDataDoubles &>(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<AQLDataDoubles &>(eData->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		if (isDF2)
		{
			AQLDataDoubles* attrDF2 = NULL;
			AQLDataHolder* df2H = &eData->getData(IR_CALIBRATION_DATA_DFS2);
			if (!df2H->isDefined())
			{
				attrDF2 = new AQLDataDoubles();
				eData->add(IR_CALIBRATION_DATA_DFS2,attrDF2);
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

	map<AQLString, map<AQLString, double> > aud_origSwapRate;
	setUpGenCurveData(dataInstance, refData, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate);

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
		if (usd3mlFloaterInfo.size() != 2) throw AQLCoreInvalidData("No XccyBasis information!",__FILE__,__LINE__);
		xccyBasis->add(IR_CALIBRATION_DATA_AGTFORECAST, new AQLDataString()).convertFromString(usd3mlFloaterInfo[1]);
		// set foreign ccy info
		AQLString ccy_floater, fYieldDataName;
		if (isSetCurveID == "TRUE")
		{
			AQLObject* fYieldData = &objPool.getObject(usd3mlFloaterInfo[0], ENCHKTYPE_ISDEFINED).get();
			ccy_floater = dynamic_cast<AQLDataString& > ((fYieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
			fYieldDataName = usd3mlFloaterInfo[0];
		}
		else
		{
			AQLString isPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
			if (isPricer == "TRUE")
			{
				AQLObject* yieldData = &objPool.getObject(usd3mlFloaterInfo[0], ENCHKTYPE_ISDEFINED).get();
				ccy_floater = dynamic_cast<AQLDataString& > ((yieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
			}
			else
			{
				ccy_floater = usd3mlFloaterInfo[0];
			}
			fYieldDataName = PREFIX_YIELD + getSDEAttrName(ccy_floater) + "_DATA";
		}

		if (mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_floater) && enableCalculation) 
		{
			generateInitialValue(ccy_floater, dataInstance);
		}
		AQLString curveName_floater = usd3mlFloaterInfo[1];
		basisCurveEngine->getForeignYieldData().convertFromString(fYieldDataName);

		if (isSetCurveID != "TRUE")
		{
			AQLString baseCcy = currency; AQLString domCcy = currency; AQLString forCcy = ccy_floater;
			while (1)
			{
				BasisCurveCalibration &ycPro_dccy = dynamic_cast<BasisCurveCalibration &>
							(objPool.getObject(AQLMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				BasisCurveCalibration &ycPro_fccy = dynamic_cast<BasisCurveCalibration &>
							(objPool.getObject(AQLMarketData::getBaseYieldProName(forCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());

				ycPro_fccy.setAffectingCcy(baseCcy.toUpper());
				const AQLString& affectedCcy_fccy = ycPro_fccy.getAffectedCcy();
				if (affectedCcy_fccy == domCcy) 
					throw AQLCoreInvalidData("The affected currency includes in Affecting currencies!", __FILE__, __LINE__);

				ycPro_dccy.setAffectedCcy(forCcy.toUpper());
				
				domCcy = forCcy;
				forCcy = ycPro_fccy.getAffectedCcy();


				if (!forCcy.isDefined()) break;
			}
		}

		if (isRenAdj)
		{
			AQLStringVector baseCcyDFInfo = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASECCYDF).toToken(':');
			if (baseCcyDFInfo.size() != 2) throw AQLCoreInvalidData("No XccyBasis information!",__FILE__,__LINE__);
			if (usd3mlFloaterInfo[0] != baseCcyDFInfo[0]) throw AQLCoreInvalidData("Reference currencies are inconsistent!",__FILE__,__LINE__);
			xccyBasis->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new AQLDataString()).convertFromString(baseCcyDFInfo[1]);
		}

		// set yield curve pro
		refData_XccyBasis = refData_XccyBasis.subString(0, refData_XccyBasis.size() - 2);
		basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + XCCYBASIS);
		basisCurveEngine->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + XCCYBASIS, new AQLDataMultiReference()).convertFromString(refData_XccyBasis);
	}
	
	tmpCurrency.toLower();
	AQLStringVector markets;
	AQLString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == AQ_NO_DATA)
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(markets);
	if (currency != CURRENCY_USD && find(markets.begin(), markets.end(), XCCYBASIS) == markets.end()) throw AQLCoreInvalidData("XCCYBASIS is not in generate dfs!!", __FILE__, __LINE__); 
	if (find(markets.begin(), markets.end(), THREESIXBASIS) == markets.end())
	{
		AQLString refData_;
		setUp36BasisDummyData(dataInstance, refData_, asOfDate, tmpCurrency, THREESIXBASIS, yieldDataName, isSpotUse, *basisCurveEngine);
		// set yield curve pro
		refData_ = refData_.subString(0, refData_.size() - 2);
		basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + THREESIXBASIS);
		basisCurveEngine->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + THREESIXBASIS, new AQLDataMultiReference()).convertFromString(refData_);
	}
	// set yield curve pro
	refData = refData.subString(0, refData.size() - 2);
	basisCurveEngine->getMarketData().convertFromString(refData);

	AQLDataHolder* dh;
	AQLStringVector swapCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) swapCurves = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

	AQLStringVector basisCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) basisCurves = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

	AQLString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (!markets.empty() && markets[0] != AQ_NO_DATA)
	{
		//AQLDataStrings &attrGenCurves = dynamic_cast<AQLDataStrings &>(basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings()).get());		
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			if (markets[i] == SWAP) continue;
			
			tmpCurrency.toLower();
			AQLString refData_;
			AQLString suffix = "." + markets[i];
			suffix.toLower();
			AQLString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffix).toUpper();
			if (marketType == AQ_NO_DATA)
			{
				AQLString isBasisStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISBASIS + suffix).toUpper();
				AQLString isReadFile = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffix).toUpper();
				if (isBasisStr == "TRUE") marketType = MARKETTYPE_BASIS;
				else if (isReadFile == "TRUE") marketType = MARKETTYPE_ZERORATE;
				else marketType = MARKETTYPE_SWAP;
			}

			// set up interpolation
			AQLString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix).toUpper();
			if (strInter != AQ_NO_DATA)
			{
				yc->getInterpolation(markets[i]).convertFromString(strInter.toLower());
				basisCurveEngine->getInterpolation(markets[i]).convertFromString(strInter);
				if (mainBDF == markets[i])
				{
					yc->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
					basisCurveEngine->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
				}
			}

			if ( marketType == MARKETTYPE_BASIS )
			{
				if (!enableCalculation && markets[i] != XCCYBASIS && markets[i] != THREESIXBASIS) continue;

				setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *basisCurveEngine);

				if (markets[i] != XCCYBASIS && markets[i] != THREESIXBASIS)
				{
					AQLString tmpMktName = markets[i];
					AQLString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + tmpMktName.toLower());
					if (tmpAssignedCurves != AQ_NO_DATA)
					{
						AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
						for (size_t i = 0; i<assignedCurves.size(); i++)
						{
							basisCurves.push_back(assignedCurves[i]);
						}
					}
					else
					{
						basisCurves.push_back(markets[i]);
					}
				}
			}
			else if ( marketType == MARKETTYPE_ZERORATE )
			{
				if (!enableCalculation && target != markets[i]) continue;

				setUpCurveDataByReadFile( dataInstance, asOfDate, tmpCurrency, markets[i], yieldDataName, *basisCurveEngine );
				continue;
			}
			else if( marketType == MARKETTYPE_SWAP )
			{
				if (!enableCalculation) continue;

				AQLString tmpMktName = markets[i];
				AQLString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != AQ_NO_DATA)
				{
					AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
					for (size_t i = 0; i<assignedCurves.size(); i++)
					{
						if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),assignedCurves[i]))
						{
							swapCurves.push_back(assignedCurves[i]);
						}
					}
				}
				else
				{
					if (swapCurves.end() == std::find(swapCurves.begin(),swapCurves.end(),markets[i]))
					{
						swapCurves.push_back(markets[i]);
					}
				}

				//attrGenCurves.set(swapCurves);

				AQLString isoismode = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix).toUpper();
				if (isoismode.toUpper() == "DAILYCOMPOUNDING")
					setUpGenCurveDataOIS(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate);
				else
					setUpGenCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate);
			}
			else
			{
				throw AQLCoreInvalidData("Market type is not supported!!", __FILE__, __LINE__); 
			}

			// set yield curve pro
			refData_ = refData_.subString(0, refData_.size() - 2);
			basisCurveEngine->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i]);
			basisCurveEngine->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i], new AQLDataMultiReference()).convertFromString(refData_);
		}
	}

	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(swapCurves));

	// generate yield data
	basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	AQLDataProcedure &proc = dynamic_cast<AQLDataProcedure &>
						(basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	dataInstance.getReferencePool().completeDependency();
	proc.estimate(asOfDate);
	
	// generate swap curve
	if (swapCurves.size() != 0 && enableCalculation)
	{
		basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
		for (size_t i=0; i<swapCurves.size(); i++)
		{
			basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(swapCurves[i]));
			proc = dynamic_cast<AQLDataProcedure &> (basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
			proc.estimate(asOfDate);
			basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
		}
		basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	}

	//adjust discountfactor
	tmpCurrency.toLower();
	AQLDataBool tmpAttrDF;
	tmpAttrDF.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF));
	if(tmpAttrDF.get())
	{
		const std::map<AQLString, AQLString> &assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (!basisCurveEngine->isBasisCurve(it->first) && (enableCalculation || target == it->second))
			{
				AQLMarketData::adjustDiscountFactor(*eData, it->first);
			}
		}
	}

	// basis
	bool isBasis = false;
	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_BASISDFS);
	if (!basisCurves.empty() && enableCalculation)
	{
		isBasis = true;
		basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_BASISDFS, new AQLDataStrings(basisCurves));
	}

	basisCurveEngine->AQLObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
	if (mainBDF != AQ_NO_DATA)
	{
		isBasis = true;
		basisCurveEngine->AQLObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new AQLDataString(mainBDF));
	}
	if (refBData.size() > 1)
	{
		isBasis = true;
		// set reference for basis
		refBData = refBData.subString(0, refBData.size() - 2);
		basisCurveEngine->getBasisData().convertFromString(refBData);
	}

	if (isBasis)
	{
		basisCurveEngine->setBasisRates();
	}

	// set daycount
	setUpCurveTypeDayCount(*basisCurveEngine, *yc);

	AQLString genFloaterName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != AQ_NO_DATA || target == CURVETYPE_FLOATER) setUpFloater(tmpCurrency, *basisCurveEngine, genFloaterName);


////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	if (contextYield!=AQ_NO_DATA)
	{
		AQLDataDoubleMatrix matrix;
		matrix.convertFromString(contextYield);
		unsigned int rowSize = matrix.get1DSize();
		unsigned int colSize = matrix.getSize(0);

		bool isDF2 = colSize>2? true: false;
		DoubleArray terms(rowSize);
		DoubleArray dfs(rowSize);
		DoubleArray dfs2(rowSize);
		for(unsigned int i=0; i<rowSize; i++)
		{
			terms[i] = matrix.get(i,0);
			dfs[i] = matrix.get(i,1);

			if (isDF2)
				dfs2[i] = matrix.get(i,2);
		}
		dynamic_cast<AQLDataDoubles &>(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<AQLDataDoubles &>(eData->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		eData->remove(IR_CALIBRATION_DATA_DFS2);
		if (isDF2)
		{
			eData->add(IR_CALIBRATION_DATA_DFS2,new AQLDataDoubles(dfs2));
		}

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		AQLStringVector dataoutCurves;
		dataoutCurves.push_back(STD);	
		for (size_t i=0; i<curveNames_6ML.size(); i++)
		{
			if(curveNames_6ML[i] != STD) dataoutCurves.push_back(curveNames_6ML[i]);
		}
		for (size_t i=0; i<curveNames_DF.size(); i++)
		{
			if(curveNames_DF[i] != STD) dataoutCurves.push_back(curveNames_DF[i]);
		}
		for (size_t i=0; i<curveNames_3ML.size(); i++)
		{
			if(curveNames_3ML[i] != STD) dataoutCurves.push_back(curveNames_3ML[i]);
		}

		const std::map<AQLString, AQLString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second != SWAP && it->second != XCCYBASIS && it->second != THREESIXBASIS) dataoutCurves.push_back(it->first);
		}

		if (genFloaterName != AQ_NO_DATA) dataoutCurves.push_back(genFloaterName);

		dataoutCurve(dataoutCurves, *eData,yieldDataName);
	}
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
//! 
/*!
    @brief generate sde correlation data

	@param[in] currency  currency or fx ex.JPY/USD
	@param[in] dataInstance
*/
void
AQLUpdateObjectPoolForCurves::generateCorrelation(const AQLString &currency, AQLDataInstance &dataInstance) const
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
			throw AQLCoreInvalidData("Volatility input type. only function or data is support", __FILE__, __LINE__); 
		}
	}
	// set data as reference
	dataInstance.getDataMaster().setData(corName, DATA_REFERENCE); 
	// set object pool
	dataInstance.getObjectPool().remove(corName);	
	dataInstance.getObjectPool().set(corName, corEntity);	
}

#endif
//! 
/*!
    @brief get SDE data name

	@param[in] currency
	@return AQLString
*/
AQLString
AQLUpdateObjectPoolForCurves::getSDEAttrName(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	return  mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
}

//! 
/*!
    @brief get Grid property val

	@param[in] key
	@param[in] curve
	@param[in] grid
	@return AQLString
*/
AQLString
AQLUpdateObjectPoolForCurves::getGridStaticData(const AQLString &key, const AQLString &curve, const AQLString &grid) const
{
	return etrading::getGridStaticData(mpStaticData, key, curve, grid);
}

//! 
/*!
    @brief fuction to set up generate configuration
*/
void
AQLUpdateObjectPoolForCurves::setUpGenerateConfig
(AQLDataInstance &dataInstance, const AQLDate &asOfDate, const AQLString &currency, AQLMathYieldCurve &yc, BasisCurveCalibration &basisCurveEngine,
 AQLObject &ycData, bool &isAudExtra, bool &isSwapTenorAdjust, bool &isSpotUse, bool isArbFree, const AQLString& suffix, const AQLString& epSuffix) const
{
	AQLString isSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

	AQLString yieldName = yc.getName().get();

	yc.AQLObject::remove(CALIBRATION_DATA_ASOFDATE);
	yc.AQLObject::add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));

	// set interpolation
	AQLString interp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix);
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
	AQLString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (mainBDF != AQ_NO_DATA) 
	{
		yc.setBasisCurveType(mainBDF);
	}

	// set yield to object pool
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	// set yieldpro to object pool
	// set as of date
	basisCurveEngine.getAsOfDate().set(asOfDate);
	
	// set curve generator
	basisCurveEngine.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
	
	// set interpolation
	AQLString genInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix);
	if (genInterp == AQ_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}	
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_INTERPOLATION + epSuffix);
		basisCurveEngine.AQLObject::add(CALIBRATION_DATA_INTERPOLATION + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genInterp);
	}
	basisCurveEngine.getInterpolation().convertFromString(genInterp.toLower());

	// interpolation for OIS and STD swaps
	AQLString genYieldGenInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
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
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation_yg().convertFromString(genYieldGenInterp.toLower());
	}
	else
	{
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// interpolation for FRA/futures
	AQLString genFutureInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FUTURE_INTERPOLATION + suffix);
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
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation_fw().convertFromString(genFutureInterp.toLower());
	}
	else
	{
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONFW + epSuffix);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONFW + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genFutureInterp);
	}

	// interpolation for basis
	AQLString genBasisInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASIS_INTERPOLATION + suffix);
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
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation_bs().convertFromString(genBasisInterp.toLower());
	}
	else
	{
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONBS + epSuffix);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONBS + epSuffix, new AQLPriceDataInterpolation()).convertFromString(genBasisInterp);
	}
	
	// set basis function
	AQLString genBasisFunc = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASISFUNCTION);
	if (genBasisFunc == AQ_NO_DATA)
	{
		genBasisFunc = FN_BASISFUNC2_STR;
	}
	basisCurveEngine.getBasisFunction().setFunction(genBasisFunc.toLower());
	
	// set daycount
	AQLString genDayCount = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT);
	if (genDayCount != AQ_NO_DATA)
	{
		basisCurveEngine.getDayCount().convertFromString(genDayCount.toUpper());
	}
	
	// set freq
	AQLString genFreq = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY);
	if (genFreq != AQ_NO_DATA)
	{
		basisCurveEngine.getFrequency().convertFromString(genFreq.toUpper());
	}
	
	// set sliding rulue
	AQLString genSlidingRule = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE);
	if (genSlidingRule != AQ_NO_DATA)
	{
		basisCurveEngine.getSlidingRule().convertFromString(genSlidingRule.toUpper());
	}

	// set rate priority
	AQLString ratePrio_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_RATEPRIORITY).toUpper();
	if (ratePrio_str != AQ_NO_DATA) 
	{		
		basisCurveEngine.getData(PRICING_DATA_RATEPRIORITY).convertFromString(ratePrio_str);	
	}
	
	// set tenor adjust
	AQLDataBool tmpAttrB;
	AQLString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + suffix).toUpper();
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
	isSwapTenorAdjust = tmpAttrB.get();
	
	//20170531 - Fixed Fatal Error For Win-Server2012
	#if (WINVER >= 0x0601)
		AQLDataHolder *ahTemp;
		ahTemp = &basisCurveEngine.AQLObject::getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, NOCHECK);
		if (!ahTemp->isDefined())
		{
			basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, new AQLDataBool(isSwapTenorAdjust));
		}
		else
		{
			dynamic_cast<AQLDataBool&>(ahTemp->get()).set(isSwapTenorAdjust);
		}
	#else	
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, new AQLDataBool(isSwapTenorAdjust));
	#endif
	
	if (isSwapTenorAdjust)
	{
		AQLString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
		if (tenorSwapName == AQ_NO_DATA) 
		{
			throw AQLCoreInvalidData("Set tenor swap name!", __FILE__, __LINE__);
		}
		
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_TENORSWAPNAME);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_TENORSWAPNAME, new AQLDataString(tenorSwapName));
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
	basisCurveEngine.getYieldData().convertFromString(yieldDataName);
	
	// set base curve reference
	basisCurveEngine.getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(yieldName);

	// set currency in curve ID for excel pricer 
	const AQLString isPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE" && isSetCurveID != "TRUE")
	{
		AQLString curveID = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
		const AQLObjectHolder ehdata = objPool.getObject(curveID);
		AQLObject *eCurveID = NULL;
		if (!ehdata.isDefined())
		{
			eCurveID = new AQLObject();
			objPool.set(curveID, eCurveID);
		}
		else
		{
			objPool.getObject(curveID).get().reset();
			eCurveID = &objPool.getObject(curveID).get();
		}
		eCurveID->add(CALIBRATION_DATA_NAME, new AQLDataString(curveID));
		eCurveID->add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString(currency));
	}

	AQLString strIsAudExtra = mpStaticData->getStaticData(KEY_SDE_YIELD_ISAUDEXTRA);
	if (strIsAudExtra != AQ_NO_DATA)
	{
		AQLDataBool tmpIsAudExtra;
		tmpIsAudExtra.convertFromString(strIsAudExtra);
		isAudExtra = tmpIsAudExtra.get();
	}

	if (isAudExtra && isSwapTenorAdjust) 
	{
		throw AQLCoreInvalidData("We can not set AUD extra and swap tenor adjust at a same time!", __FILE__, __LINE__);
	}

	// spotDate use flag
	AQLString isSpotUseStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSPOTUSE);
	if (isSpotUseStr == AQ_NO_DATA)
	{
		isSpotUseStr = "FALSE";
	}
	tmpAttrB.convertFromString(isSpotUseStr);
	isSpotUse = tmpAttrB.get();

	// set curve exist check
	if (isPricer == "FALSE" || isPricer == AQ_NO_DATA)
	{
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, new AQLDataBool(true));
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
		
		if (maxFreq == AQ_NO_DATA)
		{
			throw AQLCoreInvalidData("Max term frequency is needed for curve extrapolation!", __FILE__, __LINE__);
		}
		
		ycData.remove(IR_CALIBRATION_DATA_MAXTERMFREQ);
		ycData.add(IR_CALIBRATION_DATA_MAXTERMFREQ, new AQLDataString(maxFreq.toUpper()));
	}
}

//! 
/*!
    @brief fuction to set up basis curve data
*/
void
AQLUpdateObjectPoolForCurves::setUpBasisCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &curveCurrency, 
									  const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, BasisCurveCalibration &basisCurveEngine, const AQLString& currentCurveName, const AQLString* pMktCurrency) const
{
	AQLString mktCurrency = pMktCurrency ? *pMktCurrency : curveCurrency;
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

	AQLString yieldBasisTarget = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_TARGET + curveSuffix).toUpper();
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
		fCurve = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + curveSuffix);
		dCurve =  mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + curveSuffix);
		a_fCurve = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + curveSuffix);
		a_dCurve =  mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + curveSuffix);
		
		// cashflow info
		c_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCY + mktSuffix).toUpper();
		c_freqcpdStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCYCOMPOUND + mktSuffix).toUpper();
		c_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_DAYCOUNT + mktSuffix).toUpper();
		c_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SLIDINGRULE + mktSuffix).toUpper();
		c_calStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_CALENDAR + mktSuffix);
		c_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SPOTLAG + mktSuffix).getIntValue();
		if (isSpotUse)
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
		if (isSpotUse)
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
		fCurve = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + curveSuffix);
		dCurve =  mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + curveSuffix);
		a_fCurve = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + curveSuffix);
		a_dCurve =  mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + curveSuffix);
		// cashflow info
		c_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCY + mktSuffix).toUpper();
		c_freqcpdStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCYCOMPOUND + mktSuffix).toUpper();
		c_daycStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_DAYCOUNT + mktSuffix).toUpper();
		c_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SLIDINGRULE + mktSuffix).toUpper();
		c_calStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_CALENDAR + mktSuffix);
		c_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SPOTLAG + mktSuffix).getIntValue();
		if (isSpotUse)
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
		if (isSpotUse)
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
		basisCurveEngine.remove(CALIBRATION_DATA_BASISCURVEBASE + AQLString("_") + curveMktName);
		basisCurveEngine.AQLObject::add(CALIBRATION_DATA_BASISCURVEBASE + AQLString("_") + curveMktName, new AQLDataString(a_dCurve));

		basisCurveEngine.remove(CALIBRATION_DATA_BASISCURVESECONDARYBASE + AQLString("_") + curveMktName);
		basisCurveEngine.AQLObject::add(CALIBRATION_DATA_BASISCURVESECONDARYBASE + AQLString("_") + curveMktName, new AQLDataString(a_fCurve));

		// Store the forecast curve on the same side as the target discount
		basisCurveEngine.remove(CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE + AQLString("_") + curveMktName);
		basisCurveEngine.AQLObject::add(CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE + AQLString("_") + curveMktName, new AQLDataString(fCurve));
	}
	else
	{
		basisCurveEngine.remove(CALIBRATION_DATA_BASISCURVEBASE + AQLString("_") + curveMktName);
		basisCurveEngine.AQLObject::add(CALIBRATION_DATA_BASISCURVEBASE + AQLString("_") + curveMktName, new AQLDataString(a_fCurve));
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
			throw AQLCoreInvalidData("virtual grids and virtual rates are not same size!!", __FILE__, __LINE__);
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
		AQLString basisFileName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + mktSuffix);
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

	if (adjustValueMtx.size() != 0 && adjustValueMtx[0].size() < 2 )
	{
		AQLString msg = marketName + ", basis adjust file is empty";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}
	DoubleArray adjustValue_term,adjustValue;
	for (size_t i=0; i<adjustValueMtx.size(); i++)
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
	convertCurveName(tmp_fCurve, curveCurrency, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(tmp_dCurve, curveCurrency, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(tmp_a_fCurve, curveCurrency, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(tmp_a_dCurve, curveCurrency, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	if (ccy_dCurve != curveCurrency || ccy_fCurve != curveCurrency || ccy_a_dCurve != ccy_a_fCurve)
	{
		throw AQLCoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);
	}

	// Get curve collection ID
	AQLObjectHolder& yData = basisCurveEngine.getYieldData().get();
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
			if (curveID1 == ydName)
			{
				throw AQLCoreInvalidData("#Error: Please don't prefix dependent curves with a curve collection name that is the same as the target curve.", __FILE__, __LINE__);
			}

			useAgainstCurveCollection = true;
		}
	}

	// Allow the two curves of the 'target leg' to come from a different curve collection
	basisCurveEngine.remove(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + AQLString("_") + curveMktName);
	AQLStringVector fCurves_partitioned = tmp_fCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	AQLStringVector dCurves_partitioned = tmp_dCurve.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (fCurves_partitioned.size() == 2 && dCurves_partitioned.size() == 2)
	{
		AQLString curveID1 = fCurves_partitioned[0].toUpper();
		AQLString curveID2 = dCurves_partitioned[0].toUpper();
				
		if (curveID1 == curveID2)
		{
			if (curveID1 == ydName)
			{
				throw AQLCoreInvalidData("#Error: Please don't prefix dependent curves with a curve collection name that is the same as the target curve.", __FILE__, __LINE__);
			}

			basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + AQLString("_") + curveMktName, new AQLDataString(fCurves_partitioned[0]));
		}
		else
		{
			throw AQLCoreInvalidData("#Error: Forecast curve and discount curve of the target leg must have the same curve collection prefix", __FILE__, __LINE__);
		}
	}

	// against currency
	bool isForeignCcyLeg = false;
	AQLString fYieldDataName;
	basisCurveEngine.remove(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + AQLString("_") + curveMktName);
	if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrency		
		|| useAgainstCurveCollection) 
	{
		isForeignCcyLeg = true;

		if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrency)
		{
			// Re-set the curveType to be XCCYBASIS
			AQLString curveType = XCCYBASIS;
			basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveMktName);
			basisCurveEngine.AQLObject::add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveMktName, new AQLDataString(curveType.toUpper())); 

			AQLObjectHolder& yieldData = basisCurveEngine.getYieldData().get();
            yieldData.remove(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveMktName);
            yieldData.add(CALIBRATION_DATA_CURVETYPE + AQLString("_") + curveMktName, new AQLDataString(curveType.toUpper())); 

			// Also re-set the curveType to be XCCYBASIS within the properties
			mpStaticData->setStaticData( mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + mktSuffix, XCCYBASIS );
		}

		ccy_a_fCurve.toUpper();
		if (isSetCurveID)
		{
			AQLStringVector tmp_ccy_a_fCurve = tmp_a_fCurve.toToken(':');
			if(tmp_ccy_a_fCurve.size() == 2)
			{
				fYieldDataName = tmp_ccy_a_fCurve[0];
			}
			else
			{
				throw AQLCoreInvalidData("no foreign yield data!", __FILE__, __LINE__);
			}
		}
		else
		{
			fYieldDataName = PREFIX_YIELD + getSDEAttrName(ccy_a_fCurve) + "_DATA";
		}

		if (mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_a_fCurve) && enableCalculation) 
		{
			generateInitialValue(ccy_a_fCurve, dataInstance);
		}
		basisCurveEngine.getForeignYieldData().convertFromString(fYieldDataName);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + AQLString("_") + curveMktName, new AQLDataBool(isForeignCcyLeg));
		
		if (!isSetCurveID)
		{
			AQLString baseCcy = curveCurrency; AQLString domCcy = curveCurrency; AQLString forCcy = ccy_a_fCurve;
			while (1)
			{
				BasisCurveCalibration &ycPro_dccy = dynamic_cast<BasisCurveCalibration &>
							(objPool.getObject(AQLMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				BasisCurveCalibration &ycPro_fccy = dynamic_cast<BasisCurveCalibration &>
							(objPool.getObject(AQLMarketData::getBaseYieldProName(forCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());

				ycPro_fccy.setAffectingCcy(baseCcy.toUpper());
				const AQLString& affectedCcy_fccy = ycPro_fccy.getAffectedCcy();
				if (affectedCcy_fccy == baseCcy) 
					throw AQLCoreInvalidData("The affected currency includes in Affecting currencies!", __FILE__, __LINE__);

				ycPro_dccy.setAffectedCcy(forCcy.toUpper());
				
				domCcy = forCcy;
				forCcy = ycPro_fccy.getAffectedCcy();

				if (!forCcy.isDefined()) break;
			}
		}

	}

	// if dummy curve reset spotdate 
	if ((a_fCurve == DUMMY || a_dCurve == DUMMY) && !isSpotUse)
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
			// Cash instruments include ON, TN and Libor markets
			spotRate = etrading::populateCashInstrumentsToEntityPool(mpStaticData,
												                     refData,
												                     objPool,
												                     mktCurrency,
												                     marketName,
												                     yieldDataName,
												                     mktSuffix,
												                     curveMktName,
												                     isSpotUse,
												                     isFwdFX,
												                     asOfDate,
												                     ITSELF,
												                     spotRateTerm);			

			AQ_REQUIRE( spotRate != DBL_MAX,  "#Error: Can't locate spot Libor fixing rate that corresponds to '" + spotRateTerm + "'.");
		}
		else
		{
			if (fixingSource == AQ_NO_DATA)
			{
				marketRef = basisCurveEngine.getMarketData();
			}
			else 
			{
				// Use fixingSource to obtain an external yieldCurvePro
				AQLString foreignYcProName = etrading::AQLCurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + fixingSource;
				foreignYcProName.toUpper();

				BasisCurveCalibration* foreignYieldCurvePro = NULL;

				const AQLObjectHolder tmpHolder = objPool.getObject( foreignYcProName );
				if( tmpHolder.isDefined() )
				{
					foreignYieldCurvePro = &dynamic_cast<BasisCurveCalibration&>( objPool.getObject( foreignYcProName ).get() );
				}
				else
				{
					AQLString err = "#Error: Could not locate the foreign Yield Curve data using '" + foreignYcProName + "'";
					throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__); 
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
						AQLString nameB = yieldDataName + "_LIBOR_" + AQLString(0) + "_" + curveMktName;
						const AQLObjectHolder ehbasis = objPool.getObject(nameB);
						if (!ehbasis.isDefined())
						{
							mktData = new AQLObject();
							objPool.set(nameB, mktData);
						}
						else
						{
							objPool.getObject(nameB).get().clear();
							mktData = &objPool.getObject(nameB).get();
						}

						refData += nameB + ":";
					
						// set term
						mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));

						// set name
						mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameB);

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
						const AQLPriceDataCalendar& cal  = dynamic_cast<const AQLPriceDataCalendar&> ((marketRef.get(i).getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
						mktData->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar(cal));

						// Set sliding rule
						const AQLPriceDataSlidingRule& sld  = dynamic_cast<const AQLPriceDataSlidingRule&> ((marketRef.get(i).getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
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
	bool fwd_isRatio = false;
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
	double fwd_denominator = 1.0;

	// for fwd fx	
	if (isFwdFX)
	{
		AQLString isRatioStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISRATIO + mktSuffix).toUpper();
		if (isRatioStr == "TRUE") fwd_isRatio = true;
		if (!fwd_isRatio)
		{
			fwd_denominator = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_DENOMINATOR + mktSuffix).getDoubleValue();
			AQLString isPriceCcyStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISPRICECCY + mktSuffix).toUpper();
			if (isPriceCcyStr == "TRUE") fwd_isPriceCcy = true;
		}
		fwd_freqStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FREQUENCY + mktSuffix).toUpper();
		fwd_slidingStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_SLIDINGRULE + mktSuffix).toUpper();
		fwd_calStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_CALENDAR + mktSuffix);
		fwd_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_RESETLAG + mktSuffix).getIntValue();
		if (isSpotUse)
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
			AQLString msg = marketName + ", forward fx file is empty";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
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
		AQLString msg = marketName + ", basis file is empty";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
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

	const AQLDate date1Y = etrading::AQLDateHelpers::getDate(asOfDate, "1Y", true);
	double ndfSize = 0.0;
	for (unsigned int j = 0; j < fwdFXSize + basisSize; ++j)
	{
		AQLObject *mktData = NULL;		
		AQLString nameB = yieldDataName + "_BASIS_" + AQLString(static_cast<int>(j)) + "_" + curveMktName;
		const AQLObjectHolder ehbasis = objPool.getObject(nameB);
		if (!ehbasis.isDefined())
		{
			mktData = new AQLObject();
			objPool.set(nameB, mktData);
		}
		else
		{
			objPool.getObject(nameB).get().clear();
			mktData = &objPool.getObject(nameB).get();
		}

		refData += nameB + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameB);

		// set forward ratio
		if (j < fwdFXSize)
		{
			AQLString fwd_term = fwdFXDataMtx[j][0].toUpper();
			double fwd_fx = fwdFXDataMtx[j][1].getDoubleValue();

			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_FWDFX);
			mktData->add(IR_CALIBRATION_DATA_ISRATIO, new AQLDataBool(fwd_isRatio));
			if (fwd_isRatio)
				mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(fwd_fx));
			else
			{
				mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(fwd_fx / fwd_denominator));
				mktData->add(IR_CALIBRATION_DATA_ISPRICECCY, new AQLDataBool(fwd_isPriceCcy));
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
				AQLString ndfTerm = basisDataMtx[j][0].toUpper();
				const AQLDate date = etrading::AQLDateHelpers::getDate(asOfDate, ndfTerm, true);
				
				if (date <= date1Y)
				{
					// set term
					mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(ndfTerm));

					// set rate
					double ndfRate = basisDataMtx[j][1].getDoubleValue();
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
				mktData->add(IR_CALIBRATION_DATA_ISFWDBASIS, new AQLDataBool(isFwdBasis) );
				if (basisDataMtx[j2].size() != 5)
					throw AQLCoreInvalidData("FwdBasis File format is wrong", __FILE__,__LINE__);
				const bool isDate = basisDataMtx[j2][2].toUpper() == "TRUE";
				mktData->add(PRICING_DATA_ISDATE, new AQLDataBool(isDate) );
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
		fYieldData.add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + marketName, new AQLDataMultiReference()).convertFromString(refData.subString(0, refData.size() - 2));
	}

	AQLString tmpAssignedCurves = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + curveSuffix);
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = tmpAssignedCurves.toToken(MULTI_STATIC_DATA_DELIMITER);
		for (size_t i = 0; i<assignedCurves.size(); i++)
		{
			basisCurveEngine.setAssignedCurveMktMap(assignedCurves[i], curveMktName);
		}
	}
	else
	{
		basisCurveEngine.setAssignedCurveMktMap(curveMktName,curveMktName);
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
	bool isFwdSwap = false;
	AQLDataBool tmpAttrB;
	AQLString isFwdSwap_str = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + mktSuffix);
	if (isFwdSwap_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		isFwdSwap = tmpAttrB.get();
	}

	//FRA Data
	bool isAudExtra = false;
	AQLString staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	AQLString suffix_data = "_" + marketName;

	AQLString curveName("");
	if (currentCurveName != STD)
	{
		curveName = "." + currentCurveName;
	}
	curveName.toLower();
	AQLString curveType = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + curveName);

	// Only process FRAs when building basis curves.
	// In the case of AUD curves, OIS and STD curves would process basis instruments and the
	// processing of FRAs is not allowed in that case.
	if (curveType != "OIS" && curveType != "SWAP")
	{
		etrading::populateFRADataToEntityPool(mpStaticData,
											basisCurveEngine,
											refData,
											objPool,
											curveCurrency,
											marketName,
											yieldDataName,
											staticDataSuffix,
											suffix_data,
											isAudExtra,
											isSpotUse,
											isFwdSwap,
											asOfDate,
											true);		// isBasisCurve
	}
}

//! 
/*!
    @brief fuction to set up 36 basis curve dummy data
*/
void
AQLUpdateObjectPoolForCurves::setUp36BasisDummyData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
									    const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, 
										BasisCurveCalibration &basisCurveEngine) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	AQLString suffix = "." + marketName;
	suffix.toLower();

	// get swap market
	AQLString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE);
	AQLFileAccessor swapFile(AQLMarketData::getNumFileName(swapFileName));
	AQLStringMatrix swapDataMtx;
	swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
	swapFile.close();
	if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2 )
	{
		AQLString msg = "swap file is empty";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}
	const int swapSize = swapDataMtx.size();
	AQLString daycSStr_float = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT).toUpper();
	AQLString interpSStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_INTERPOLATION).toLower();
	AQLPriceDataCalendar cal;
	AQLString calStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR).toLower();
	cal.convertFromString(calStr);
	AQLDate spotDate;
	int resetLag;
	if (isSpotUse)
	{
		spotDate.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SPOTDATE).getCString());
	}
	else
	{
		AQLString reseLag_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG);
		resetLag = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG).getIntValue();
		if (reseLag_str == AQ_NO_DATA)
		{
			throw AQLCoreInvalidData("Reset Lag is not set !!", __FILE__, __LINE__); 
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
		AQLString nameB = yieldDataName +  "_" + marketName + "_" + AQLString(static_cast<int>(j));
		const AQLObjectHolder ehbdummy = objPool.getObject(nameB);
		if (!ehbdummy.isDefined())
		{
			mktData = new AQLObject();
			objPool.set(nameB, mktData);
		}
		else
		{
			objPool.getObject(nameB).get().clear();
			mktData = &objPool.getObject(nameB).get();
		}
		refData += nameB + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameB);
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
		mktData->add(IR_CALIBRATION_DATA_CASHLETCALENDAR , new AQLPriceDataCalendar(cal));
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

//! 
/*!
    @brief fuction to set up base curve data
*/
void 
AQLUpdateObjectPoolForCurves::setUpGenCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
									const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse,
									bool isAudExtra, BasisCurveCalibration &basisCurveEngine, 
									std::map<AQLString, std::map<AQLString, double> > &aud_origSwapRate, const AQLString& curveName) const
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
	// Cash instruments include ON, TN and Libor markets
	etrading::populateCashInstrumentsToEntityPool(mpStaticData,
										refData,
										objPool,
										currency,
										marketName,
										yieldDataName,
										staticDataSuffix,
										suffix_data,
										isSpotUse,
										isFwdFX,
										asOfDate);

	//DF curve name
	AQLString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix); 
	if (dfCurveName == AQ_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new AQLDataString(dfCurveName));
	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new AQLDataString(dfCurveName));

	// get fwd swap
	bool isFwdSwap = false;
	AQLDataBool tmpAttrB;
	AQLString isFwdSwap_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + staticDataSuffix);
	if (isFwdSwap_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		isFwdSwap = tmpAttrB.get();
	}

	// Do we always recalculate the dynamic linear spline join date?
	AQLString alwaysCalcJoinDate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE + staticDataSuffix);
	basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);	
	if (alwaysCalcJoinDate != AQ_NO_DATA && alwaysCalcJoinDate.size() != 0)
	{
		if (alwaysCalcJoinDate.toUpper() == "TRUE")
		{
			basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(true));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(true));
		}
		else
		{
			basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(false));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(false));
		}
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
	basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
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
		if (isSpotUse)
		{
			spotDate.setDate(mpStaticData->getStaticData(currency + spotDateKey + staticDataSuffix).getCString());
		}
		else
		{
			spotDate = cal.getBusinessDay(asOfDate, mpStaticData->getStaticData(tmpCurrency + spotLagKey + staticDataSuffix).getIntValue());
		}

		//Use NO_CHANGE as businessDayAdj
		AQLDate inputInterpolationJoinDate = etrading::validateDateOrTenor(spotDate, inputInterpJoinDateStr, "NO_CHANGE", calStr, "Invalid joinDate");

		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputInterpolationJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputInterpolationJoinDate));
	}
	
	//FRA Data
	etrading::populateFRADataToEntityPool(mpStaticData,
										basisCurveEngine,
										refData,
										objPool,
										currency,
										marketName,
										yieldDataName,
										staticDataSuffix,
										suffix_data,
										isAudExtra,
										isSpotUse,
										isFwdSwap,
										asOfDate);

	// future

	/* Handle isFutureUse flag:
	*  Some care is needed here since some currencies (such as AUD) can have TWO swap curves.
	*  If the curveType is SWAP, *and* the curveName is STD, then use the simple data flag stored in basisCurveEngine.
	*  Otherwise, for non-STD curves use the full object pool data IR_CALIBRATION_DATA_ISFUTUREUSE plus suffix.
	*  This prevents the settings of two or more swap curves from overwriting each other.
	*/
	if ( (marketName == SWAP) && (curveName == STD) )
	{
		basisCurveEngine.getIsFutureUse().set(isFutureUse);
	}
	else
	{
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data, new AQLDataBool(isFutureUse));
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
        AQLString meanReversion_str = mpStaticData->getStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_MEANREVERSION + staticDataSuffix );
        if ( meanReversion_str != AQ_NO_DATA )
        {
            AQLDataDouble tmpAttrDouble;
            tmpAttrDouble.convertFromString( meanReversion_str );
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

		// get UseConvexAdjustment
		bool useConvexAdjustment = false;
		AQLString useConvexAdjustmentStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USECONVEXADJUSTMENT + staticDataSuffix).toUpper();
        if (useConvexAdjustmentStr != AQ_NO_DATA)
        {
            AQLDataBool tmpUseConvexAdjustment;
			tmpUseConvexAdjustment.convertFromString(useConvexAdjustmentStr);
			useConvexAdjustment = tmpUseConvexAdjustment.get();
        }

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
						
        const int futureSize = futureDataMtx.size();
		for (int i = 0; i < futureSize; ++i)
		{
			if (futureDataMtx[i].size() != 5 && futureDataMtx[i].size() != 3)
			{
				throw AQLCoreInvalidData("Future File format is wrong", __FILE__,__LINE__);
			}
			AQLString term;
			AQLDate startDate,endDate;
			double futurePrice(0.0), rate(0.0), futureVol(0.0), convexAdj(0.0);

			if (futureDataMtx[i].size() == 5)
			{
				term = futureDataMtx[i][0].toUpper();
				startDate = AQLDate(futureDataMtx[i][1].getCString());
				endDate = AQLDate(futureDataMtx[i][2].getCString());
				futurePrice = futureDataMtx[i][3].getDoubleValue();
				rate = 1.0 - futurePrice * 0.01;

				if (useConvexAdjustment)
				{
					convexAdj = futureDataMtx[i][4].getDoubleValue();
				}
				else
				{
					futureVol = futureDataMtx[i][4].getDoubleValue();
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
				
				if (useConvexAdjustment)
				{
					convexAdj = futureDataMtx[i][2].getDoubleValue();
				}
				else
				{
					futureVol = futureDataMtx[i][2].getDoubleValue();
				}
			}
			else
			{
				throw AQLCoreInvalidData("Future File format is wrong", __FILE__,__LINE__);
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
			refData += nameF + ":";
			// set name
			mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameF);
			// set term
			mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(term);
			// set calendar
			mktData->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calFStr);
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
			// set convexAdj
			mktData->add(PRICING_DATA_CONVEXADJUSTMENT, new AQLDataDouble(convexAdj));
			// set useConvexAdj
			mktData->add(PRICING_DATA_USECONVEXADJUSTMENT, new AQLDataBool(useConvexAdjustment));
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
		AQLString msg = "swap file is empty";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}

	// get cal and calc spot date
	AQLPriceDataCalendar calS;
	AQLString calSStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + staticDataSuffix);
	calS.convertFromString(calSStr);

	AQLDate spotDateS;
	int resetLag;
	if (isSpotUse)
	{
		spotDateS.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SPOTDATE + staticDataSuffix).getCString());
	}
	else
	{
		AQLString resetLag_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG + staticDataSuffix);
		resetLag = resetLag_str.getIntValue();
		if (resetLag_str == AQ_NO_DATA)
		{
			throw AQLCoreInvalidData("Reset Lag is not set !!", __FILE__, __LINE__); 
		}
		else
		{
			spotDateS = calS.getBusinessDay(asOfDate, resetLag);
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
	if (isResetSkipStr!=AQ_NO_DATA)
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
	bool isSwapTenorAdjust = false;
	AQLString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + staticDataSuffix).toUpper();
	if (strSwapTenorAdj != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(strSwapTenorAdj);
		isSwapTenorAdjust = tmpAttrB.get();
	}
	for (int j = 0; j < swapSize; ++j)
	{
		AQLString term = swapDataMtx[j][0].toUpper();
		AQLString tmpCurrency = currency;
		if (isAudExtra && tmpCurrency.toUpper() == CURRENCY_AUD &&
			term.findString("Y") == static_cast<int>(term.size() - 1) &&
				term.subString(0, term.size() - 2).getIntValue() > 3 && resetFlg)
		{
			// set yield curve pro
			if (marketName != SWAP)
			{
				basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
				basisCurveEngine.AQLObject::add(CALIBRATION_DATA_MARKETDATA + suffix_data, new AQLDataMultiReference()).
					convertFromString(refData.subString(0, refData.size() - 2));
			}
			else
			{
				basisCurveEngine.getMarketData().convertFromString(refData.subString(0, refData.size() - 2));
			}
			
			// generate yield data
			AQLDataProcedure &proc = dynamic_cast<AQLDataProcedure &>
								(basisCurveEngine.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
			basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(marketName));

			dataInstance.getReferencePool().completeDependency();
			proc.estimate(asOfDate);
			
			//reset market data as libor
			AQLCurveMarketDataHelpers::resetMarketDataUseL(basisCurveEngine, currency, &marketName);
			resetFlg = false;
			if (marketName != SWAP)
			{
				basisCurveEngine.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
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

		if (isAudExtra && tmpCurrency.toUpper() == CURRENCY_AUD && resetFlg)
		{
			aud_origSwapRate[marketName].insert(make_pair(term, rate / 100.0));
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
		refData += nameS + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameS);
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateS));
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTLAG, new AQLDataInt(resetLag));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calSStr);
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
		if (isFwdSwap)
		{
			mktData->add(PRICING_DATA_ISFWDSWAP, new AQLDataBool(isFwdSwap) );
			if (swapDataMtx[j].size() != 5)
				throw AQLCoreInvalidData("FwdSwap File format is wrong", __FILE__,__LINE__);
			const bool isDate = swapDataMtx[j][2].toUpper() == "TRUE";
			mktData->add(PRICING_DATA_ISDATE, new AQLDataBool(isDate) );
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
		mktData->add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, new AQLDataBool(isSwapTenorAdjust) );
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
		// set frequency
		if ((!checkFrequency(freqSStr, term) || !checkFrequency(freqSStr_Fix, term)) && isUse)
				throw AQLCoreInvalidData("frequency fix and term of swap rate are inconsistent!!", __FILE__, __LINE__);
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freqSStr);
		mktData->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FIX, new AQLDataString()).convertFromString(freqSStr_Fix);
		// set base frequency
		if (baseFreqSStr_float != AQ_NO_DATA)
		{
			if (!checkFrequency(baseFreqSStr_float, term) && isUse)
				throw AQLCoreInvalidData("base frequency and term of swap rate are inconsistent!!", __FILE__, __LINE__);
			mktData->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, new AQLDataString()).convertFromString(baseFreqSStr_float);
		}
		// set frequency of floating leg
		if (freqSStr_float != AQ_NO_DATA)
		{
			if (!checkFrequency(freqSStr_float, term) && isUse)
				throw AQLCoreInvalidData("frequency float and term of swap rate are inconsistent!!", __FILE__, __LINE__);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, new AQLDataString()).convertFromString(freqSStr_float);
		}
		// set frequency of compounding
		if (freqSStr_cpd != AQ_NO_DATA)
		{
			if (!checkFrequency(freqSStr_cpd, term) && isUse)
				throw AQLCoreInvalidData("frequency compound and term of swap rate are inconsistent!!", __FILE__, __LINE__);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_COMPOUND, new AQLDataString()).convertFromString(freqSStr_cpd);
		}
	}

	if (refData.size() < 2)
	{
        throw AQLCoreInvalidData("#Error: Missing Market Data. Market Data is not set !!", __FILE__, __LINE__); 
	}

	AQLString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix);
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
		for (size_t i = 0; i<assignedCurves.size(); i++)
		{
			basisCurveEngine.setAssignedCurveMktMap(assignedCurves[i],marketName);
		}
	}
	else
	{
		if (marketName != SWAP) 
		{
			basisCurveEngine.setAssignedCurveMktMap(marketName,marketName);
		}
		else
		{
			basisCurveEngine.setAssignedCurveMktMap(STD,marketName);
		}
	}

	// remove curve generate map
	const AQLString isPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<AQLString, bool>& gCurveMap = basisCurveEngine.getGCurveGenerateMap();
		const std::map<AQLString, AQLString>& assignedCurveMktMap = basisCurveEngine.getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second == marketName)
			{
				if (gCurveMap.find(it->first) != gCurveMap.end()) gCurveMap.erase(it->first);
			}
		}
	}
}


//! 
/*!
    @brief fuction to set up base curve data
*/
void 
AQLUpdateObjectPoolForCurves::setUpGenCurveDataOIS(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
									const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse,
									bool isAudExtra, BasisCurveCalibration &basisCurveEngine, 
									std::map<AQLString, std::map<AQLString, double> > &aud_origSwapRate) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	AQLString suffix = "." +  marketName;
	suffix.toLower();

	AQLString suffix_data = "_" +  marketName;

	AQLString yeildGenInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
	if (yeildGenInter != AQ_NO_DATA) 
	{
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data, new AQLPriceDataInterpolation()).convertFromString(yeildGenInter);
	}

	AQLString oisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffix);
	AQLFileAccessor oisFile(AQLMarketData::getNumFileName(oisFileName));
	AQLStringMatrix oisDataMtx;
	oisFile.readAllData(MARKET_DATA_DELIMITER, oisDataMtx);

	oisFile.close();

	if (oisDataMtx.size() == 0 || oisDataMtx[0].size() < 2 )
	{
		throw AQLCoreInvalidData("Error: OIS Swap Data is missing. OisFile is empty", __FILE__,__LINE__);
	}

	// get cal and calc spot date
	AQLPriceDataCalendar calOIS;
	AQLString calOISStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_CALENDAR + suffix);
	calOIS.convertFromString(calOISStr);
	AQLDate spotDateOIS;
	if (isSpotUse)
	{
		spotDateOIS.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SPOTDATE + suffix).getCString());
	}
	else
	{
		spotDateOIS = calOIS.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_RESETLAG + suffix).getIntValue());
	}

	bool isEOMRollOIS = false;
	AQLString strEOMRollOIS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_ISEOMRLL + suffix).toUpper();	
	if (strEOMRollOIS != AQ_NO_DATA)	
	{	
		AQLDataBool tmpIsEOMRollOIS;		
		tmpIsEOMRollOIS.convertFromString(strEOMRollOIS);		
		isEOMRollOIS = tmpIsEOMRollOIS.get();		
	}			
	
	if (isEOMRollOIS)			
	{			
		AQLString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_EOMDAY + suffix).toUpper();		
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
	AQLString inputInterpJoinDateStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_INTERPOLATIONJOINDATE + suffix).toUpper();
	basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);

	yldEntity.remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	if (inputInterpJoinDateStr != AQ_NO_DATA && inputInterpJoinDateStr.size() != 0)
	{
		//AQLDate inputLinearSplineJoinDate = etrading::validateAndConvertStringToDate(inputInterpJoinDateStr, "Invalid joinDate");
		AQLString busDayAdj = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE + suffix).toUpper();
		AQLDate inputInterpolationJoinDate = etrading::validateDateOrTenor(spotDateOIS, inputInterpJoinDateStr, busDayAdj, calOISStr, "Invalid joinDate");

		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputInterpolationJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputInterpolationJoinDate));
	}
	
	// Various OIS control parameters
	AQLString str_shortTerm =  mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERM + suffix);
	AQLDate shortTermDate;
	if (str_shortTerm != AQ_NO_DATA)
	{
		shortTermDate = calOIS.getBusinessDay(asOfDate, str_shortTerm.getIntValue());
	}

	AQLString shortTermConv			= mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION + suffix).toUpper();
	AQLString firstRate				= mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FIRSTRATE + suffix).toUpper();

	AQLString swapCompoundingMethod("");
	AQLString compoundValue		    = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDMETHOD + suffix).toUpper();
	AQLString compoundValueAlias     = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDINGMETHOD + suffix).toUpper();
	
    if (compoundValue != AQ_NO_DATA && compoundValueAlias != AQ_NO_DATA)
	{
		AQ_REQUIRE( compoundValue == compoundValueAlias, "Invalid CompoundMethod: Cannot specify 'CompoundMethod' as '" + compoundValue + "' the alias for this parameter 'CompoundingMethod' as '" + compoundValueAlias + "'" )
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
    if ( swapCompoundingMethod != AQ_NO_DATA )
    {
        swapCompoundingMethod.toUpper();
        AQ_REQUIRE( swapCompoundingMethod == "ARITHMETIC" || swapCompoundingMethod == "GEOMETRIC" || swapCompoundingMethod == "NONE" || swapCompoundingMethod == "",
                      "Invalid CompoundingMethod - Input was '" + swapCompoundingMethod + "' but must be 'ARITHMETIC', 'GEOMETRIC' or 'NONE' " )
    }
    
    // Validate shortTermConv Generate Method Parameter by trying to cast to its enumerated type
    if ( shortTermConv != AQ_NO_DATA )
    {
        shortTermConv.toUpper();
        AQ_REQUIRE( shortTermConv == "ARITHMETIC" || shortTermConv == "ARITHMETICAVERAGE" || shortTermConv == "NONE" || shortTermConv == "",
                      "Invalid ShortTermConvention - Input was '" + shortTermConv + "' but must be 'ARITHMETICAVERAGE' or 'NONE'" )
    }

	const size_t oisSize = oisDataMtx.size();

	// Curve controls
	bool smoothShortEnd = false;
	 AQLString strSmoothShortEnd = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SMOOTHSHORTEND + suffix);
	 if (strSmoothShortEnd != AQ_NO_DATA)
	 {
		AQLDataBool tmpSmoothShortEnd;		
		tmpSmoothShortEnd.convertFromString(strSmoothShortEnd);		
		smoothShortEnd = tmpSmoothShortEnd.get();	
	 }

	 bool shortTermSwapOverrules = false;
	 AQLString strShortTermSwapOverrules = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMSWAPOVERRULES + suffix);
	 if (strShortTermSwapOverrules != AQ_NO_DATA)
	 {
		 AQLDataBool tmpShortTermSwapOverrules;
		 tmpShortTermSwapOverrules.convertFromString(strShortTermSwapOverrules);
		 shortTermSwapOverrules = tmpShortTermSwapOverrules.get();
	 }

	// use grid
	AQLStringVector oisUseGrid;
	AQLString tmpOISUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffix).toUpper();
	if (tmpOISUseGrid != AQ_NO_DATA)
	{
		oisUseGrid = tmpOISUseGrid.toToken(':');
	}

	// doing dual bootstrapping?
	bool isDualBootstrapping = false;
	AQLString strIsDualBootstrapping = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_ISDUALBOOTSTRAPPING + suffix);
	if (strIsDualBootstrapping != AQ_NO_DATA)
	{
		AQLDataBool tmpIsDualBootstrapping;
		tmpIsDualBootstrapping.convertFromString(strIsDualBootstrapping);
		isDualBootstrapping = tmpIsDualBootstrapping.get();
	}

	AQLString longTermConv = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffix).toUpper();
	AQLString longTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffix).toUpper();
	AQLString longTermGen = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD + suffix).toUpper();
	
	AQLDate date_lt;
	AQLStringMatrix lobasisDataMtx, swapDataMtx;
	
    // Validate LongTerm Convention Parameter
    if ( longTermConv != AQ_NO_DATA )
    {
        longTermConv.toUpper();
        AQ_REQUIRE( longTermConv == "LOBASIS" || longTermConv == "NONE" || longTermConv == "",
                      "Invalid LongTermConvetion - Input was '" + longTermConv + "' but must be 'LOBASIS' to calibrate to Libor-OIS Instruments or 'NONE' to calibrate to OIS Outrights only" )
    }
    
    // Validate LongTerm Generate Method Parameter
    if ( longTermGen != AQ_NO_DATA )
    {
        longTermGen.toUpper();
        AQ_REQUIRE( longTermGen == "DAILYAVERAGING" || longTermGen == "NONE" || longTermGen == "", 
                      "Invalid LongTerm.GenerateMethod - Input was '" + longTermGen + "' but must be 'DAILYAVERAGING' or 'NONE'" )
    }

    if (longTermConv == "LOBASIS")
	{
		if (longTerm == AQ_NO_DATA)
		{
            throw AQLCoreInvalidData("#Error: LongTerm parameter is needed, when the LongTermConvention 'LOBASIS' is specified.", __FILE__,__LINE__);
		}
		if (longTermGen == AQ_NO_DATA)
		{
            // LongTermGenerate Methodolgy Defaults are managed within the in calcEffectiveOISRate method, see CurveCalibration.cpp
			longTermGen = AQLString("NONE");
		}
		date_lt = etrading::AQLDateHelpers::getDate(asOfDate, longTerm, true);
		AQLString lobasisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + suffix);
		AQLFileAccessor lobasisFile(AQLMarketData::getNumFileName(lobasisFileName));
		lobasisFile.readAllData(MARKET_DATA_DELIMITER, lobasisDataMtx);
		lobasisFile.close();
		if (lobasisDataMtx.size() == 0 || lobasisDataMtx[0].size() < 2 )
		{
            throw AQLCoreInvalidData("#Error: Missing Libor-OIS Basis Market Data. LOBasisFile is empty", __FILE__,__LINE__);
		}
		AQLString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix);
		if (swapFileName != AQ_NO_DATA)
		{
			AQLFileAccessor swapFile(AQLMarketData::getNumFileName(swapFileName));
			swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
			swapFile.close();
			if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2)
			{
				if (!isDualBootstrapping)
				{
					throw AQLCoreInvalidData("#Error: Missing Libor-OIS Swap Market Data. SwapFile is empty", __FILE__, __LINE__);
				}
			}

			if (swapDataMtx.size() != lobasisDataMtx.size())
			{
				if (!isDualBootstrapping)
				{
					throw AQLCoreInvalidData("#Error: Not the same number of LOBasis spreads and Libor swaps are provided ", __FILE__, __LINE__);
				}
			}
		}
	}

	//get constant for convergence
	double epsilon = 1.0e-9;
	AQLString strEPS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_EPSILON + suffix);
	if (strEPS.toUpper() != AQ_NO_DATA)
	{
		epsilon = strEPS.getDoubleValue();
	}
	int maxLoop = 1000;
	AQLString strMLP = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_MAXLOOP + suffix);
	if (strMLP.toUpper() != AQ_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}

	// Consolidate the outright OIS swap instruments and basis swap + libor swap instruments
	size_t extraBasisCount = 0;
	size_t longTermStartIndex = 0;
	if (longTermConv == "LOBASIS")
	{
		AQLString lastOisTerm = oisDataMtx[oisSize-1][0].toUpper();

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

	// get UseConvexAdjustment for future section
	bool useConvexAdjustment = false;
	std::string useConvexAdjustmentStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USECONVEXADJUSTMENT + suffix).toUpper().getCString();
	if (useConvexAdjustmentStr != AQ_NO_DATA)
	{
		useConvexAdjustment = (useConvexAdjustmentStr == "TRUE");
	}

	// get meanReversion for future section
	double meanReversion = 0.0;
	std::string meanReversion_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_MEANREVERSION + suffix).toUpper().getCString();
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
		AQLString nameOIS = yieldDataName + "_OIS_" + AQLString( (int)i ) + "_" + marketName;
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
		if (refData.findString(nameOIS) < 0)
		{
			refData += nameOIS + ":";
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

        // Allow Central Bank Swap instruments to be specified in the OIS curve for all markets ...
        if (etrading::isCentralBankSwap(term))     
		{
			if (oisDataMtx[i].size() != 4)
			{
                throw AQLCoreInvalidData("#Error: OIS Market Data must contain 4 columns. OIS Market Data column size is incorrect.", __FILE__,__LINE__);
			}
			// boj type
			// ** Currently all Central Bank swaps are labelled as BOJ regardless of currency. **
			mktData->add(IR_CALIBRATION_DATA_DATATYPE,   new AQLDataString()).convertFromString(YIELD_TYPE_BOJ);
			AQLDate startDate = AQLDataDate(oisDataMtx[i][2]).get();
			AQLDate endDate = AQLDataDate(oisDataMtx[i][3]).get();
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));

			//When the startDate is in the past and endDate is in the future, use Historical data in the FixingTable
			if (startDate < asOfDate && endDate > asOfDate)
			{
				AQLString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix);
				etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
			}

		}
		else if (etrading::isARRFuture(term))
		{
			const int NUM_COLUMNS_FUTURE = 5; // Term, Rate, StartDate, EndDate, VolOrConvAdj
			if (oisDataMtx[i].size() != NUM_COLUMNS_FUTURE)
			{
				throw AQLCoreInvalidData("#Error: OIS Market Data must contain 4 columns. OIS Market Data column size is incorrect.", __FILE__, __LINE__);
			}

			// ARR Future type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_ARR_FUTURE);

			// set startDate, endDate
			AQLDate startDate = etrading::validateAndConvertStringToDate(oisDataMtx[i][2]);
			AQLDate endDate = etrading::validateAndConvertStringToDate(oisDataMtx[i][3]);
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));

			double futureVol(0.0), convexAdj(0.0);

			// Populate the actual convexityAdjustment value or Volatility
			if (useConvexAdjustment) 
			{
				convexAdj = oisDataMtx[i][4].getDoubleValue();
			}
			else
			{
				futureVol = oisDataMtx[i][4].getDoubleValue();
			}

			// set useConvexAdj
			mktData->add(PRICING_DATA_USECONVEXADJUSTMENT, new AQLDataBool(useConvexAdjustment));
			// set convexAdj
			mktData->add(PRICING_DATA_CONVEXADJUSTMENT, new AQLDataDouble(convexAdj));
			// set vol
			mktData->add(PRICING_DATA_FUTUREVOLATILITY, new AQLDataDouble(futureVol));

			//When the startDate is in the past and endDate is in the future, use Historical data in the FixingTable
			if (startDate < asOfDate && endDate > asOfDate)
			{
				AQLString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix);
				etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
			}
		}
		else
		{
			// swap type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_PAR);
		}				

		// get freq
		AQLString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY, suffix, term).toUpper();
		// get daycount
		AQLString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT, suffix, term).toUpper();
		// get sliding
		AQLString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE, suffix, term).toUpper();
		// get swap type
		AQLString swapType("OIS");
		AQLString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SWAPTYPE + suffix).toUpper();
		if (strSwapType != AQ_NO_DATA)
		{
			swapType = strSwapType;
		}
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateOIS));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calOISStr);
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
			rate = 1.0 - rate/100.0;

			double convexAdj = etrading::getFutureConvexityAdjustment(mktData, asOfDate, meanReversion);

			// 2) add convexity adjustment to the future rate
			rate -= convexAdj;
			
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

		if (longTermConv == "LOBASIS")
		{
            // Allow Central Bank Swap instruments to be specified in the OIS curve for all markets ...
            if( term.findString("BOJ")      >= 0        // Bank of Japan
             || term.findString("EUSF")     >= 0        // European Central Bank (Bloomberg Ticker)
             || term.findString("ECB")      >= 0        // European Central Bank
             || term.findString("BOE")      >= 0        // Bank of England
             || term.findString("CB")       >= 0        // *** GENERIC *** Central Bank Swaps
             || term.findString("MPC")       >= 0 )     // *** GENERIC *** Monetary Policy Committee Swaps
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
                AQ_THROW( "Unable to imply OIS Outright Swaps from Libor-OIS Basis Instruments: Missing Libor-OIS Basis Swap with tenor " + term + ". Note: Overlapping OIS and Libor-OIS instruments must have identical tenors." );
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
                    AQ_THROW( "Unable to imply OIS Outright Swaps from Libor-OIS Basis Instruments: Missing Libor Swap with tenor'" + term + "' to match the corresponding Libor-OIS Basis Swap" );
				}
			}

			// get lobasis data
			double rate_lo = lobasisDataMtx[currentLOBasisInstrument][1].getDoubleValue();
			AQLString calLOStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_CALENDAR + suffix);
			AQLString freqLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY, suffix, term).toUpper();
			AQLString daycLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT, suffix, term).toUpper();
			AQLString slidingLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE, suffix, term).toUpper();
			
			// get swap data			
			AQLString calSwapStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + suffix);
			AQLString freqSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFIX, suffix, term).toUpper();
			AQLString daycSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFIX, suffix, term).toUpper();
			AQLString slidingSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE, suffix, term).toUpper();
			AQLString swapTenor("");
			AQLString strSwapTenor = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTENOR + suffix).toUpper();
			if (strSwapTenor != AQ_NO_DATA)
			{
				swapTenor = strSwapTenor;
			}
			AQLString swapType("LIBOR");
			AQLString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTYPE + suffix).toUpper();
			if (strSwapType != AQ_NO_DATA)
			{
				swapType = strSwapType;
			}
			
			// set long term rate convention
			mktData->add(IR_CALIBRATION_DATA_LONGTERMCONVENTION,	new AQLDataString()).convertFromString(longTermConv);
			mktData->add(IR_CALIBRATION_DATA_LONGTERM,			new AQLDataString()).convertFromString(longTerm);
			mktData->add(IR_CALIBRATION_DATA_LONGTERMGENMETHOD,	new AQLDataString()).convertFromString(longTermGen);
			
			// set lobasis data
			mktData->add(IR_CALIBRATION_DATA_RATE_LOBASIS,		new AQLDataDouble(rate_lo / 100.0));
			if (calLOStr == AQ_NO_DATA) calLOStr = calOISStr;
			mktData->add(IR_CALIBRATION_DATA_CALENDAR_LOBASIS ,	new AQLPriceDataCalendar()).convertFromString(calLOStr);
			if (daycLOStr == AQ_NO_DATA) daycLOStr = daycOISStr;
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_LOBASIS,	new AQLPriceDataDayCount()).convertFromString(daycLOStr);
			if (slidingLOStr == AQ_NO_DATA) slidingLOStr = slidingOISStr;
			mktData->add(IR_CALIBRATION_DATA_SLIDINGRULE_LOBASIS, new AQLPriceDataSlidingRule()).convertFromString(slidingLOStr);
			if (freqLOStr == AQ_NO_DATA) freqLOStr = freqOISStr;
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_LOBASIS,	new AQLDataString()).convertFromString(freqLOStr);	
			
			// set swap data
			if (isSwapRateAvailable)
			{
				double rate_s = swapDataMtx[currentSwapInstrument][1].getDoubleValue();
				mktData->add(IR_CALIBRATION_DATA_RATE_SWAP,		new AQLDataDouble(rate_s / 100.0));
			}
			mktData->add(IR_CALIBRATION_DATA_CALENDAR_SWAP ,	new AQLPriceDataCalendar()).convertFromString(calSwapStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_SWAP,	new AQLPriceDataDayCount()).convertFromString(daycSwapStr);
			mktData->add(IR_CALIBRATION_DATA_SLIDINGRULE_SWAP,new AQLPriceDataSlidingRule()).convertFromString(slidingSwapStr);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_SWAP,	new AQLDataString()).convertFromString(freqSwapStr);	
			
			mktData->remove(IR_CALIBRATION_DATA_REFSWAPTENOR);
			mktData->remove(IR_CALIBRATION_DATA_REFSWAPTYPE);
			mktData->add(IR_CALIBRATION_DATA_REFSWAPTENOR,	new AQLDataString()).convertFromString(swapTenor);	
			mktData->add(IR_CALIBRATION_DATA_REFSWAPTYPE,		new AQLDataString()).convertFromString(swapType);	
		}
	}

	AQLStringMatrix fedFundFutureDataMtx;
	AQLString fedFundFutureFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + suffix);
	if (fedFundFutureFileName != AQ_NO_DATA)
	{
		AQLFileAccessor fedFundFutureFile(AQLMarketData::getNumFileName(fedFundFutureFileName));	
		fedFundFutureFile.readAllData(MARKET_DATA_DELIMITER, fedFundFutureDataMtx);
		fedFundFutureFile.close();
	}

	const int fedFundFutureSize = fedFundFutureDataMtx.size();
	// use grid
	AQLStringVector ffFutureUseGrid;
	AQLString tmpFFFutureUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID + suffix).toUpper();	
	if (tmpFFFutureUseGrid != AQ_NO_DATA)
	{
		ffFutureUseGrid = tmpFFFutureUseGrid.toToken(':');
	}
	else
	{
		AQLString tmpUseGridNum = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRIDNUM + suffix).toUpper();
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
		if (refData.findString(nameOIS) < 0)
		{
			refData += nameOIS + ":";
		}
		
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameOIS);
		
		AQLDate startDate, endDate;
		AQLString term = fedFundFutureDataMtx[i][0].toUpper();

		if (fedFundFutureDataMtx[i].size() < 2)
		{
            throw AQLCoreInvalidData("#Error: FF Futures data cannot contain more than 2 columns. FF Future File format is wrong", __FILE__,__LINE__);
		}

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
			DateVector ffdates = etrading::AQLDateHelpers::getFFDatesFromTerm(asOfDate,term);
			if (ffdates.size() != 2)
                throw AQLCoreInvalidData("#Error: FF Dates Data must contain 2 columns. FF dates error",__FILE__,__LINE__);

			startDate = ffdates[0];
			endDate = ffdates[1];
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
		}
		
		double rate = fedFundFutureDataMtx[i][1].getDoubleValue();
		rate  = 100. - rate;

		// get freq
		AQLString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY, suffix, term).toUpper();
		// get daycount
		AQLString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT, suffix, term).toUpper();
		// get sliding
		AQLString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE, suffix, term).toUpper();
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateOIS));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calOISStr);
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
			AQLString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix);

			etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
		}

	}

	if (refData.size() < 2)
	{
		throw AQLCoreInvalidData("Market Data is not set !!", __FILE__, __LINE__); 
	}

	//DF curve name
	AQLString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffix); 
	if (dfCurveName == AQ_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName);
	basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName, new AQLDataString(dfCurveName));
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName, new AQLDataString(dfCurveName));

	//const std::map<AQLString, AQLString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
	AQLString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix);
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
		for (size_t i = 0; i<assignedCurves.size(); i++)
		{
			basisCurveEngine.setAssignedCurveMktMap(assignedCurves[i],marketName);
		}
	}
	else
	{
		basisCurveEngine.setAssignedCurveMktMap(marketName,marketName);
	}

	// remove curve generate map
	const AQLString isPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<AQLString, bool>& gCurveMap = basisCurveEngine.getGCurveGenerateMap();
		const std::map<AQLString, AQLString>& assignedCurveMktMap = basisCurveEngine.getAssignedCurveMktMap();
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
AQLUpdateObjectPoolForCurves::setUpFloater(const AQLString &currency, BasisCurveCalibration &basisCurveEngine, const AQLString &genFloaterName) const
{
	AQLStringVector markets = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	uppervec(markets);
	if (genFloaterName != AQ_NO_DATA)
	{
		basisCurveEngine.setAssignedCurveMktMap(genFloaterName, genFloaterName);
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_FLOATERDFS);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_FLOATERDFS, new AQLDataString(genFloaterName));
		AQLString tmpGenFloaterName = genFloaterName;
		tmpGenFloaterName.toLower();
		AQLString basisMkt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME);
		if (basisMkt != AQ_NO_DATA)
		{
			if (std::find(markets.begin(), markets.end(), basisMkt) == markets.end())
			{
				throw AQLCoreInvalidData("Basis market does not exist!", __FILE__, __LINE__);
			}
			basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_BASISDATA + AQLString("_") + tmpGenFloaterName);
			basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_BASISDATA + AQLString("_") + tmpGenFloaterName, new AQLDataString(basisMkt));
		}
		AQLString discountName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT);
		AQLString forecastName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FORECAST);
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_FORECAST + AQLString("_") + tmpGenFloaterName);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_FORECAST + AQLString("_") + tmpGenFloaterName, new AQLDataString(forecastName));
		basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_DISCOUNT + AQLString("_") + tmpGenFloaterName);
		basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_DISCOUNT + AQLString("_") + tmpGenFloaterName, new AQLDataString(discountName));

		/*AQLString isFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_ISFWDINTER).toUpper();
		if (isFWDInter == "TRUE")
		{
			basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + AQLString("_") + tmpGenFloaterName);
			basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + AQLString("_") + tmpGenFloaterName, new AQLDataBool(true));
			AQLString fwdInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FWDINTERPOLATION).toLower();
			basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_FWDINTERPOLATION + AQLString("_") + tmpGenFloaterName);
			basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_FWDINTERPOLATION + AQLString("_") + tmpGenFloaterName, new AQLPriceDataInterpolation()).convertFromString(fwdInter);;
		}*/
		
		basisCurveEngine.setFloater(genFloaterName);
	}
}

void
AQLUpdateObjectPoolForCurves::setUpCurveDataByReadFile( AQLDataInstance &dataInstance, const AQLDate& asOfDate, const AQLString& currency, 
										    const AQLString& marketName, const AQLString& yieldDataName, BasisCurveCalibration &basisCurveEngine ) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLObjectHolder objHolder = objPool.getObject(yieldDataName, ENCHKTYPE_NOCHECK );
	if (!objHolder.isDefined() )
		throw AQLCoreInvalidData("yield Object is not set! AQLUpdateObjectPoolForCurves::setUpCurveDataByReadFile", __FILE__, __LINE__ );
	
	AQLObject &eData = objHolder.get();

	AQLString tmpMktName = marketName;
	AQLString suffix = "." + tmpMktName.toLower();

	AQLString dfFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_DF_FILE + suffix );
	AQLFileAccessor dfFile( AQLMarketData::getNumFileName(dfFileName));
	AQLStringMatrix dfDataMtx;
	dfFile.readAllData(MARKET_DATA_DELIMITER, dfDataMtx);
	dfFile.close();

	if (dfDataMtx.size() == 0 || dfDataMtx[0].size() < 2 )
		throw AQLCoreInvalidData("dfFile is empty", __FILE__,__LINE__);

	DoubleArray terms;
	DoubleArray dfs;
	DoubleArray dfs2;

	for (size_t i = 0; i < dfDataMtx.size(); i++)
	{
		if (dfDataMtx[i].size() == 3)
		{
			terms.push_back( dfDataMtx[i][0].getDoubleValue());
		    dfs.push_back( dfDataMtx[i][1].getDoubleValue());
			dfs2.push_back( dfDataMtx[i][2].getDoubleValue());
		}
		else
		{
			terms.push_back( dfDataMtx[i][0].getDoubleValue());
			dfs.push_back( dfDataMtx[i][1].getDoubleValue());
		}
	}

	if (!terms.empty() && terms[0] != 0.0)
	{
		terms.insert(terms.begin(),0.0);
		dfs.insert(dfs.begin(),1.0);
		if (!dfs2.empty())
			dfs2.insert(dfs2.begin(),1.0);
	}
	
	AQLString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ZERORATE_ASSIGNEDCURVE + suffix);
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
		for (size_t i = 0; i<assignedCurves.size(); i++)
		{
			basisCurveEngine.setAssignedCurveMktMap(assignedCurves[i],marketName);

			AQLString modCurveName = "_" + assignedCurves[i].toUpper();

			eData.remove(CALIBRATION_DATA_TERMS + modCurveName);
			eData.add(CALIBRATION_DATA_TERMS + modCurveName, new AQLDataDoubles(terms));
			
			eData.remove(IR_CALIBRATION_DATA_DFS + modCurveName);
			eData.add(IR_CALIBRATION_DATA_DFS + modCurveName, new AQLDataDoubles(dfs));

			eData.remove(IR_CALIBRATION_DATA_ACCESSARY + modCurveName);
			eData.add(IR_CALIBRATION_DATA_ACCESSARY + modCurveName, new AQLDataString("DF"));

			if (!dfs2.empty())
			{
				eData.remove( IR_CALIBRATION_DATA_DFS2 + modCurveName); 
				eData.add( IR_CALIBRATION_DATA_DFS2 + modCurveName, new AQLDataDoubles(dfs2));
			}
		}
	}
	else
	{
		AQLString modCurveName = "_" + tmpMktName.toUpper();

		eData.remove(CALIBRATION_DATA_TERMS + modCurveName);
		eData.add(CALIBRATION_DATA_TERMS + modCurveName, new AQLDataDoubles(terms));
		
		eData.remove(IR_CALIBRATION_DATA_DFS + modCurveName);
		eData.add(IR_CALIBRATION_DATA_DFS + modCurveName, new AQLDataDoubles(dfs));

		eData.remove(IR_CALIBRATION_DATA_ACCESSARY + modCurveName);
		eData.add(IR_CALIBRATION_DATA_ACCESSARY + modCurveName, new AQLDataString("DF"));

		if (!dfs2.empty())
		{
			eData.remove( IR_CALIBRATION_DATA_DFS2 + modCurveName); 
			eData.add( IR_CALIBRATION_DATA_DFS2 + modCurveName, new AQLDataDoubles(dfs2));
		}
		basisCurveEngine.setAssignedCurveMktMap(marketName,marketName);
	}

	basisCurveEngine.insertNonRemovableMarket(marketName);
}

void 
AQLUpdateObjectPoolForCurves::dataoutCurve(const AQLStringVector &curveNames, AQLObject &eData, const AQLString &yieldDataName) const
{
	for(unsigned int i = 0; i < curveNames.size(); i++)
	{
		AQLString curveSuffix;
		if(curveNames[i] == STD) curveSuffix = "";
		else curveSuffix = "_" + curveNames[i];
		AQLString curveSuffix_file = curveSuffix;
		while(curveSuffix_file.findString("/") != -1)
		{
			curveSuffix_file.remove(curveSuffix_file.findString("/"),1);
		}
		const AQLString fileSuffix = AQLCoreDataService::getContext(ARG_KEY_FILENUM);
		const AQLString dirName = AQLCoreDataService::getOutputDirectory(); 
		const AQLString fileName  = dirName + yieldDataName + curveSuffix_file + fileSuffix + ".csv";

		ifstream fin;
		ofstream fout;
		fin.open(fileName.getCString());

		if (!fin)
		{
			AQLDataHolder* dh = &eData.getData(CALIBRATION_DATA_TERMS + curveSuffix, NOCHECK);
			if (!dh->isDefined() || dh->isNull())
				continue;

			const DoubleArray &terms = 
				dynamic_cast<const AQLDataDoubles &>(eData.getData(CALIBRATION_DATA_TERMS + curveSuffix, ISNOTNULL).get()).get();
			const DoubleArray &dfs = 
				dynamic_cast<const AQLDataDoubles &>(eData.getData(IR_CALIBRATION_DATA_DFS + curveSuffix, ISNOTNULL).get()).get();

			int size = terms.size();
			if (size != static_cast<int>(dfs.size()))
			{
				throw AQLCoreInvalidData("Term size and df size must be same !!", __FILE__, __LINE__);
			}
			fout.open(fileName.getCString());

			const AQLDataHolder &dfsH2 = eData.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix, NOCHECK);
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
			dynamic_cast<AQLDataDoubles &>(eData.getData(CALIBRATION_DATA_TERMS + curveSuffix, ISNOTNULL).get()).set(terms);
			dynamic_cast<AQLDataDoubles &>(eData.getData(IR_CALIBRATION_DATA_DFS + curveSuffix, ISNOTNULL).get()).set(dfs);
			if (!dfs2.empty() && eData.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix).isDefined())
			{
				dynamic_cast<AQLDataDoubles &>(eData.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix, ISNOTNULL).get()).set(dfs2);
			}
		}
	}
}



bool 
AQLUpdateObjectPoolForCurves::checkFrequency(const AQLString& freq, const AQLString& mktRateTerm) const
{
	int span = etrading::AQLDateHelpers::getOnePeriodOfFrequency(freq);

	int y, m, d, w;
    etrading::AQLDateHelpers::termStrtoYMDW(mktRateTerm, y, m, d, w);
	int moth_mkt_term = 12 * y + m;

	return (moth_mkt_term % span) == 0;
}

///// update for XLL Plus //////////////////////////
#include "AQLUpdateObjectPoolForCurvesAndModels.h"

void
AQLUpdateObjectPoolForCurves::generateInitialValueForPricer(const AQLString &currency, AQLDataInstance &dataInstance) const
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
	
	AQLString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		basisCurveEngine = new BasisCurveCalibration(&dataInstance);
		objPool.set(yieldProName, basisCurveEngine);
	}
	else
	{
		//we must not erase the reset method for only basisCurveEngine
		basisCurveEngine = &dynamic_cast<BasisCurveCalibration &>(objPool.getObject(yieldProName).get());
	}
	basisCurveEngine->getName().convertFromString(yieldProName);

	basisCurveEngine->getIsArbFree().set(false);

	AQLObject *eData = NULL;
	AQLString yieldDataName = yieldName + "_DATA";
	const AQLObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new AQLObject();
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	eData->remove(CALIBRATION_DATA_NAME);
	eData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);

	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;
	
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, false);
	setUpCurveDataByContext(*basisCurveEngine,eData,currency,SWAP);


	AQLStringVector markets;
	AQLString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == AQ_NO_DATA)
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(markets);
	if (!markets.empty() && markets[0] != AQ_NO_DATA)
	{
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			AQLString suffix = markets[i];
			suffix.toLower();
			AQLString contextKey = tmpCurrency+CONTEXT_KEY_SDE_YIELD_WITH_MARKET+suffix;
			AQLString contextWithMarket = AQLCoreDataService::getContext(contextKey);
			if (contextWithMarket!=AQ_NO_DATA)
			{
				AQLString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + suffix).toUpper();
				if (marketType==MARKETTYPE_BASIS)
				{
					AQLString useYieldSDEIRStr = AQLCoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
					AQLCoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD,AQ_NO_DATA);
					AQLString tmpCurveName = markets[i];
					tmpCurveName.toUpper();
					mpStaticData->setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
					UpdateObjectPoolForSDEsAndCurves generator(currency);
					generator.generateSDEMarketData(currency, dataInstance, true);
					mpStaticData->removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
					AQLCoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD,useYieldSDEIRStr);
				}
			}
			setUpCurveDataByContext(*basisCurveEngine,eData,currency,markets[i]);
		}
	}
}

void 
AQLUpdateObjectPoolForCurves::setUpCurveDataByContext(BasisCurveCalibration &basisCurveEngine, AQLObject *eData, const AQLString& currency, const AQLString& marketName ) const
{
	AQLString prefix = currency;
	prefix.toLower();
	AQLString suffix;
	AQLString data_suffix;
	if (marketName!=SWAP)
	{
		suffix = marketName;
		suffix.toLower();
		data_suffix = "_" + marketName;
	}

	AQLString contextKey = prefix+CONTEXT_KEY_SDE_YIELD+suffix;
	AQLString contextYield = AQLCoreDataService::getContext(contextKey);
	if (contextYield==AQ_NO_DATA)
	{
		AQLString msg = "context data for generated dfs don't exist.";
		msg += "(" + contextKey + ")";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	AQLDataDoubleMatrix matrix;
	matrix.convertFromString(contextYield);
	unsigned int rowSize = matrix.get1DSize();
	unsigned int colSize = matrix.getSize(0);

	bool isDF2 = colSize>2? true: false;
	DoubleArray terms(rowSize);
	DoubleArray dfs(rowSize);
	DoubleArray dfs2(rowSize);
	for(unsigned int i=0; i<rowSize; i++)
	{
		terms[i] = matrix.get(i,0);
		dfs[i] = matrix.get(i,1);

		if (isDF2)
			dfs2[i] = matrix.get(i,2);
	}

	eData->remove(CALIBRATION_DATA_TERMS + data_suffix);
	eData->add(CALIBRATION_DATA_TERMS + data_suffix, new AQLDataDoubles(terms));

	eData->remove(IR_CALIBRATION_DATA_DFS + data_suffix);
	eData->add(IR_CALIBRATION_DATA_DFS + data_suffix, new AQLDataDoubles(dfs));

	if (isDF2)
	{
		AQLDataDoubles* attrDF2 = NULL;
		AQLDataHolder* df2H = &eData->getData(IR_CALIBRATION_DATA_DFS2 + data_suffix);
		if (!df2H->isDefined())
		{
			attrDF2 = new AQLDataDoubles();
			eData->remove(IR_CALIBRATION_DATA_DFS2 + data_suffix);
			eData->add(IR_CALIBRATION_DATA_DFS2 + data_suffix,attrDF2);
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
		AQLDataHolder* dh = &(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED));
		const DoubleVector& baseterms = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();

		DoubleVector calcdf2(baseterms.size() ,1.0);
		AQLSplineInterpolation attrspline;
		attrspline.set(terms, dfs);
		for (unsigned int i = 0; i < baseterms.size(); i++)
			calcdf2[i] = attrspline.value(baseterms[i]);

		eData->remove(IR_CALIBRATION_DATA_DFS2);
		eData->add(IR_CALIBRATION_DATA_DFS2, new AQLDataDoubles(calcdf2));
	}



	// set curve name to yield curve pro
	AQLStringVector gCurveNames;
	AQLDataHolder *dh = &(basisCurveEngine.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		gCurveNames = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
	}
	gCurveNames.push_back(marketName);
	basisCurveEngine.AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	basisCurveEngine.AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(gCurveNames));
}
////////////////////////////////////////////////////

