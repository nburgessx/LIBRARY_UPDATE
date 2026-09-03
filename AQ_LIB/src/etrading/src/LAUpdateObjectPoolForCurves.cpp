//
// LAUpdateObjectPoolForCurves.cpp
// This file used to be called CalibratorIR.cpp and before that LACalibrateModelIR.cpp
//
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAUpdateObjectPoolForCurves.h"
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

#ifndef VISUAL_STUDIO_2010_ANALYTICS 

#include "LACoreDataService.h"
#include "LARatesTermStructureSDE.h"
#include "LARatesLJTermStructureSDE.h"
#include "LAMathCorrelation.h"
#include <time.h>

#endif

#include "BasisCurveCalibration.h"
#include "CurveCalibration.h"
#include "LACurveMarketDataHelpers.h"
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
LAUpdateObjectPoolForCurves::LAUpdateObjectPoolForCurves(const LAString &baseCurrency)
: LAObjectPoolBase(), mBaseCurrency(baseCurrency)
{
}

//! destructor
/*!

*/
LAUpdateObjectPoolForCurves::~LAUpdateObjectPoolForCurves(void)
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
LAUpdateObjectPoolForCurves::setInterpolationMethod(const LAString &currency, LARatesSDEBase &sde) const
{
	(void)currency;
	sde.setInterpolationMethod(new LARatesCurveLinearInterpolation());
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
LAUpdateObjectPoolForCurves::generateSDEMarketData(const LAString &currency, LADataInstance &dataInstance, const bool isCurve, const bool isOthers) const
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
LAUpdateObjectPoolForCurves::generateInitialValueFwdFXConst(const LAString &currency, LADataInstance &dataInstance) const
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
	LAString suffix   = "." + LAString(market).toLower();
	LAString epSuffix = LAString(market).toUpper();
	LAString basisTarget = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_TARGET + suffix).toUpper();
	if (basisTarget == LEG1FORECAST || basisTarget == LEG2FORECAST)
	{
		throw LACoreInvalidData("fwdfx constant curve must be discount curve!", __FILE__, __LINE__);
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
		throw LACoreInvalidData("basisTarget curve is invalid!", __FILE__, __LINE__);
	}
	convertCurveName(strFCurve, ccy, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(strDCurve, ccy, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(strA_fCurve, ccy, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(strA_dCurve, ccy, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	if (ccy_dCurve != ccy || ccy_fCurve != ccy || ccy_a_dCurve != ccy_a_fCurve)
		throw LACoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);

	const LAString &ycProName = LAMarketData::getBaseYieldProName(ccy);
	BasisCurveCalibration &basisCurveEngine = dynamic_cast<BasisCurveCalibration &>(objPool.getObject(ycProName).get());

	LAObjectHolder& yData = basisCurveEngine.getYieldData().get();
	LAString ydName = yData.getName();
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
		const LAString &colYCProName = LAMarketData::getBaseYieldProName(ccy_a_fCurve);
		colYCPro = &(dynamic_cast<BasisCurveCalibration &>(objPool.getObject(colYCProName).get()));
		colYCPro->setColAffectingCcy(ccy.toUpper());
		basisCurveEngine.setColAffectedCcy(ccy_a_fCurve.toUpper());
	}
	const LAString &fYCName = LAMarketData::getBaseYieldName(ccy_a_fCurve);
	LAMathYieldCurve& fYC = dynamic_cast<LAMathYieldCurve &>(objPool.getObject(fYCName).get());
	const LAString &fYDName = fYC.getYieldData().get().getName();
	basisCurveEngine.getColYieldData().convertFromString(fYDName);

	// save market data
	LAObject *mktData = NULL;
	LAString nameB = ydName +  "_" + market.toUpper() + "_" + LAString(static_cast<int>(0));
	const LAObjectHolder ehbasis = objPool.getObject(nameB);
	if (!ehbasis.isDefined())
	{
		mktData = new LAObject();
		objPool.set(nameB, mktData);
	}
	else
	{
		objPool.getObject(nameB).get().clear();
		mktData = &objPool.getObject(nameB).get();
	}
	mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameB);
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
	
	basisCurveEngine.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + epSuffix);
	basisCurveEngine.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + epSuffix, new LADataMultiReference()).convertFromString(nameB);

	// save assigned curves
	LAStringVector assignedCurves;
	LAString strAssignedCurves = mpStaticData->getStaticData(ccy.toLower() + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffix);
	if (strAssignedCurves != MLIB_NO_DATA)
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
	LAString curveType = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
	if (curveType != MLIB_NO_DATA)
	{
		basisCurveEngine.LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix);
		basisCurveEngine.LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix, new LADataString(curveType.toUpper()));

		yData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix);
        yData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix, new LADataString(curveType.toUpper())); 
	}

	// set interpolation
	LAString genInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix);
	if (genInterp == MLIB_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}	
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		basisCurveEngine.LAObject::remove(CALIBRATION_DATA_INTERPOLATION + LAString("_") + epSuffix);
		basisCurveEngine.LAObject::add(CALIBRATION_DATA_INTERPOLATION + LAString("_") + epSuffix, new LAPriceDataInterpolation()).convertFromString(genInterp);
	}

	// set yieldgen interpolation
	LAString genYieldGenInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
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
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation_yg().convertFromString(genYieldGenInterp.toLower());
	}
	else
	{
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + LAString("_") + epSuffix);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + LAString("_") + epSuffix, new LAPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// calc curve
	//basisCurveEngine.calcFwdFXConstantCurve();
    basisCurveEngine.calcFwdFXConstantCurveUsingMarketName(market);// Calculate the FwdFXConstCurve but allow any FWDFXCONST marketName to be specified
		
	// dataout
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
		dataoutCurve(assignedCurves, yData.get(), ydName);
}

//! 
/*!
    @brief generate sde initial curve data for cheapest-to-deliver curve

	@param[in/out] dataInstance
*/
void LAUpdateObjectPoolForCurves::generateInitialValueCheapestToDeliver(const LAString &currency, LADataInstance &dataInstance) const
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
		throw LACoreInvalidData(err.getCString(),__FILE__,__LINE__);
	}
	BasisCurveCalibration &basisCurveEngine = dynamic_cast<BasisCurveCalibration &>(objHolder.get());

	LAObjectHolder& yData = basisCurveEngine.getYieldData().get();
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
	LAString suffix = "." + market;
	suffix.toLower();
	LAString epSuffix = market;
	epSuffix.toUpper();
		
	// save market data
	LAObject *mktData = NULL;
	LAString nameB = ydName +  "_" + epSuffix;
	const LAObjectHolder ehbasis = objPool.getObject(nameB);
	if (!ehbasis.isDefined())
	{
		mktData = new LAObject();
		objPool.set(nameB, mktData);
	}
	else
	{
		objPool.getObject(nameB).get().clear();
		mktData = &objPool.getObject(nameB).get();
	}

	// Convention data
	LAString calendar	 = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CALENDAR + suffix).toUpper();
	LAString dayCount	 = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT + suffix).toUpper();
	LAString businessAdj = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE + suffix).toUpper();
	LAString frequency	 = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY + suffix).toUpper();

	mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(calendar);
	mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(dayCount);
	mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(businessAdj);
	mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(frequency);
	mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_CTD);

	basisCurveEngine.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + epSuffix);
	basisCurveEngine.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + epSuffix, new LADataMultiReference()).convertFromString(nameB);

	// save assigned curves
	LAStringVector assignedCurves;
	LAString strAssignedCurves = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffix);
	if (strAssignedCurves != MLIB_NO_DATA)
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
	LAString curveType = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
	if (curveType != MLIB_NO_DATA)
	{
		basisCurveEngine.LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix);
		basisCurveEngine.LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix, new LADataString(curveType.toUpper()));

		yData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix);
        yData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + epSuffix, new LADataString(curveType.toUpper())); 
	}

	// Get CSA collateral curves
	LAStringVector csaCurves;
	LAString csaCurvesTemp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_CTD_COLLATERALCURVES + suffix);
	if (csaCurvesTemp != MLIB_NO_DATA)
	{
		csaCurves = csaCurvesTemp.toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		throw LACoreInvalidData("#Error: Must provide at least one CSA curve",__FILE__,__LINE__);
	}

	// set interpolation
	LAString genInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix);
	if (genInterp == MLIB_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}	
	if (epSuffix.size() == 0)
	{
		basisCurveEngine.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		basisCurveEngine.LAObject::remove(CALIBRATION_DATA_INTERPOLATION + LAString("_") + epSuffix);
		basisCurveEngine.LAObject::add(CALIBRATION_DATA_INTERPOLATION + LAString("_") + epSuffix, new LAPriceDataInterpolation()).convertFromString(genInterp);
	}

	// set yieldgen interpolation
	LAString genYieldGenInterp = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
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
		basisCurveEngine.getInterpolation_yg().convertFromString(genYieldGenInterp.toLower());
	}
	else
	{
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + LAString("_") + epSuffix);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + LAString("_") + epSuffix, new LAPriceDataInterpolation()).convertFromString(genYieldGenInterp);
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
LAUpdateObjectPoolForCurves::convertCurveName(const LAString &propCurve, const LAString &ccy, LAString &curveCcy, LAString &curveName, const bool isPricer, LAObjectPool& objPool) const
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
		else if(propCurves.size() == 2)
		{
			if (isPricer)
			{
				LAObject* yieldData = &(objPool.getObject(propCurves[0], ENCHKTYPE_ISDEFINED).get());
				curveCcy = dynamic_cast<LADataString& >((yieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
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
LAUpdateObjectPoolForCurves::getAccFromFreq(const LAString &freq_) const
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


//! 
/*!
    @brief generate curve data using dual-bootstrapping technique

	@param[in] currency  currency of the curve
	@param[out] dataInstance
*/
void
LAUpdateObjectPoolForCurves::generateInitialValueDualBootstrap(const LAString &currency, LADataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

	// Get YieldCurve and BasisCurveCalibration objects (as dataValues)
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
	
	LAString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(yieldProName);
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
	LAObject *eData = NULL;
	LAString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const LAObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new LAObject();
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	eData->remove(CALIBRATION_DATA_NAME);
	eData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);
	
	// As of date
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	
	// Main curve name
	bool enableCalculation = true;
	LAString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
	
	if (target == MLIB_NO_DATA) 
	{
		throw LACoreInvalidData("#Error: Dual bootstrapping curve name is not detected",__FILE__,__LINE__);
	}
	else
	{
		enableCalculation = false;
	}

	target.toUpper();
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(target));
	
	LAString ep_suffix	= (target == STD || target == SWAP) ? "" : "_" + target;
	LAString suffix		= (target == STD || target == SWAP) ? "" : "." + target;
	suffix.toLower();
	
	// OIS curve name and suffix
	LAString currentCurveName_ois = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_OISCURVENAME + suffix).toUpper();
	if (currentCurveName_ois == MLIB_NO_DATA) 
	{
		throw LACoreInvalidData("#Error: OIS curve name is not found in performing dual bootstrapping", __FILE__, __LINE__); 
	}

	LAString suffix_ois = (currentCurveName_ois == STD || currentCurveName_ois == SWAP) ? "" : "." + currentCurveName_ois;
	suffix_ois.toLower();

	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + ep_suffix, new LADataString(currentCurveName_ois));

	// Swap curve name and suffix
	LAString currentCurveName_swap = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME + suffix);
	currentCurveName_swap.toUpper();
	if (currentCurveName_swap == MLIB_NO_DATA) 
	{
		throw LACoreInvalidData("#Error: Swap curve name is not found in performing dual bootstrapping", __FILE__, __LINE__); 
	}

	LAString suffix_swap = (currentCurveName_swap == STD || currentCurveName_swap == SWAP) ? "" : "." + currentCurveName_swap;
	suffix_swap.toLower();

	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE + ep_suffix, new LADataString(currentCurveName_swap));

	//-----------------------------------------------------------------
	// Extract common curve building parameters

	LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	// Fast rebuild	
	LAString tmpFastRebuild_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_FASTREBUILD + suffix).toUpper();
	bool fastRebuild = true;
	if (tmpFastRebuild_str == "FALSE")
	{
		fastRebuild = false;		
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix, new LADataBool(fastRebuild));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix, new LADataBool(fastRebuild));

	//get constant for convergence
	double eps = 1.0e-9;
	LAString strEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_EPSILON + suffix);
	if (strEPS.toUpper() != MLIB_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix, new LADataDouble(eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix, new LADataDouble(eps));

	double grad_eps = 1.0e-15;
	LAString strGEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_GRADIENTEPSILON + suffix);
	if (strGEPS.toUpper() != MLIB_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix, new LADataDouble(grad_eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix, new LADataDouble(grad_eps));

	double delta = 1.0e-10;
	LAString strDLT = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_DELTA + suffix);
	if (strDLT.toUpper() != MLIB_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix, new LADataDouble(delta));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix, new LADataDouble(delta));

	int maxLoop = 1000;
	LAString strMLP = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_MAXLOOP + suffix);
	if (strMLP.toUpper() != MLIB_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix, new LADataInt(maxLoop));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix, new LADataInt(maxLoop));

	//-----------------------------------------------------------------
	// Extract individual curve's configuration parameters

	// Define a number of control modes
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;
	
	// Set up curve Generate Config for the swap curve
	LAString ep_suffix_swap = (currentCurveName_swap == STD || currentCurveName_swap == SWAP) ? "" : "_" + currentCurveName_swap;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, isArbFree, suffix_swap, ep_suffix_swap);

	// Set up curve Generate Config for the OIS curve
	LAString ep_suffix_ois = (currentCurveName_ois == STD || currentCurveName_ois == SWAP) ? "" : "_" + currentCurveName_ois;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, isArbFree, suffix_ois, ep_suffix_ois);

	// Support on FX?
	LAString fxName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != MLIB_NO_DATA)
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
	
	const std::map<LAString, LAString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
	for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		const LAString& curveName = it->first;
		basisCurveEngine->setDualBootstrapOISCurveMktMap(curveName, it->second);
	}

	// Store curve type to object pool under current engine name + current curve name
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_ois);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_ois, new LADataString("OIS"));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_ois);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_ois, new LADataString("OIS"));

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
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_swap);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_swap, new LADataString("SWAP"));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_swap);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_swap, new LADataString("SWAP"));
		
	for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
	{
		const LAString& curveName = it->first;
		basisCurveEngine->setDualBootstrapSwapCurveMktMap(curveName, it->second);
	}

	mCurveGenCcyMap[currency] = true;

	// Store all curve names
	LAString allCurveNames = currentCurveName_ois + LAString(MULTI_STATIC_DATA_DELIMITER) + currentCurveName_swap;
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix, new LADataString(allCurveNames));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix, new LADataString(allCurveNames));
	
	// Get CurveCalibration out of BasisCurveCalibration
	LADataProcedure &proc = dynamic_cast<LADataProcedure &>(basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());	
	const LACoreProcedure& yieldGenProcedure = proc.getMethod();
	const CurveCalibration& curveCalibrationEngine = dynamic_cast<const CurveCalibration& >(yieldGenProcedure);

	// Peform dual-bootstrapping
	LAObject* parent = dynamic_cast<LAObject* >(basisCurveEngine);
    curveCalibrationEngine.dualbootstrap(asOfDate, *parent, LADataProcedure());
	
}


//! 
/*!
@brief generate curve data using dual-bootstrapping technique

@param[in] currency  currency of the curve
@param[out] dataInstance
*/
void LAUpdateObjectPoolForCurves::generateInitialValueGlobalEngineCurves(const LAString &currency, LADataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

	// Get YieldCurve and BasisCurveCalibration objects (as dataValues)
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

	LAString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(yieldProName);
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
	LAObject *eData = NULL;
	LAString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const LAObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new LAObject();
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	eData->remove(CALIBRATION_DATA_NAME);
	eData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);

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
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(engineName));

	LAString ep_suffix = (engineName == STD || engineName == SWAP) ? "" : "_" + engineName;
	LAString suffix = (engineName == STD || engineName == SWAP) ? "" : "." + engineName;
	suffix.toLower();

	//-----------------------------------------------------------------
	// Extract common curve building parameters

	// Fast rebuild	
	LAString tmpFastRebuild_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_FASTREBUILD + suffix).toUpper();
	bool fastRebuild = true;
	if (tmpFastRebuild_str == "FALSE")
	{
		fastRebuild = false;
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix, new LADataBool(fastRebuild));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + ep_suffix, new LADataBool(fastRebuild));

	//get constant for convergence
	double eps = 1.0e-9;
	LAString strEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_EPSILON + suffix);
	if (strEPS.toUpper() != MLIB_NO_DATA)
	{
		eps = strEPS.getDoubleValue();
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix, new LADataDouble(eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + ep_suffix, new LADataDouble(eps));

	double grad_eps = 1.0e-15;
	LAString strGEPS = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_GRADIENTEPSILON + suffix);
	if (strGEPS.toUpper() != MLIB_NO_DATA)
	{
		grad_eps = strGEPS.getDoubleValue();
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix, new LADataDouble(grad_eps));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + ep_suffix, new LADataDouble(grad_eps));

	double delta = 1.0e-10;
	LAString strDLT = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_DELTA + suffix);
	if (strDLT.toUpper() != MLIB_NO_DATA)
	{
		delta = strDLT.getDoubleValue();
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix, new LADataDouble(delta));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + ep_suffix, new LADataDouble(delta));

	int maxLoop = 1000;
	LAString strMLP = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_MAXLOOP + suffix);
	if (strMLP.toUpper() != MLIB_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix, new LADataInt(maxLoop));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + ep_suffix, new LADataInt(maxLoop));

	//-----------------------------------------------------------------------------------------------------------
	// Loop through each curve in the engine and prepare its calibration parameters and data in the object pool

	// Define a number of control modes
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;

	LAString allCurveTypes = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVETYPES + suffix);
	LAStringVector curveTypeVector = allCurveTypes.toToken(MULTI_STATIC_DATA_DELIMITER);

	LAString allCurveNames = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVENAMES + suffix);
	LAStringVector curveNameVector = allCurveNames.toToken(MULTI_STATIC_DATA_DELIMITER);

	if (curveTypeVector.size() != curveNameVector.size())
	{
		MLIB_THROW("Not all the curves in the global curve engine has been given a curve type.");
	}

	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix, new LADataString(allCurveNames));
	yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix);
	yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + ep_suffix, new LADataString(allCurveNames));
	
	for (unsigned int i = 0; i < curveNameVector.size(); ++i)
	{
		// Get single curve's name and suffix
		LAString currentCurveName = curveNameVector[i].toUpper();
		LAString currentCurveType = curveTypeVector[i].toUpper();

		LAString suffix_currentCurve = (currentCurveName == STD || currentCurveName == SWAP) ? "" : "." + currentCurveName;
		suffix_currentCurve.toLower();
		
		LAString ep_suffix_currentCurve = (currentCurveName == STD || currentCurveName == SWAP) ? "" : "_" + currentCurveName;
		ep_suffix_currentCurve.toUpper();

		// Store curve type to object pool under current engine name + current curve name
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_currentCurve);
		basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_currentCurve, new LADataString(currentCurveType));
		yldEntity.remove(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_currentCurve);
		yldEntity.add(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + ep_suffix + ep_suffix_currentCurve, new LADataString(currentCurveType));
				
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

		//const std::map<LAString, LAString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
		//for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		//{
		//	const LAString& curveName = it->first;
		//	basisCurveEngine->setDualBootstrapOISCurveMktMap(curveName, it->second);
		//}
	}

	mCurveGenCcyMap[currency] = true;

	// Get CurveCalibration out of BasisCurveCalibration
	LADataProcedure &proc = dynamic_cast<LADataProcedure &>(basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	const LACoreProcedure& yieldGenProcedure = proc.getMethod();
	const CurveCalibration& curveCalibrationEngine = dynamic_cast<const CurveCalibration& >(yieldGenProcedure);

	// Peform dual-bootstrapping
	LAObject* parent = dynamic_cast<LAObject* >(basisCurveEngine);
    curveCalibrationEngine.buildEngineCurves(asOfDate, *parent, LADataProcedure());

}


/*!
    @brief 
*/
void LAUpdateObjectPoolForCurves::configureCurve(LAStaticData *mpStaticData,
							BasisCurveCalibration *basisCurveEngine,
							LAMathYieldCurve *yc,
							LADataInstance &dataInstance,
							const LADate& asOfDate,
							const LAString& currency,
							const LAString& curveName,
							const LAString& yieldDataName,
							const LAString& suffix,
							const LAString& ep_suffix,
							bool enableCalculation,
							bool isSpotUse,
							bool isAudExtra,
							bool isSwapTenorAdjust) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();	

	LAString currentCurveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
	currentCurveType.toUpper();

	// reference data for market pro
	LAString refData;
	LAString refBData;
	std::map<LAString, std::map<LAString, double> > aud_origSwapRate;

	if (enableCalculation || currentCurveType == SWAP) 
	{
		// Variable 'target' is essentially the name of the curve
		setUpGenCurveData(dataInstance, refData, asOfDate, currency, SWAP, yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate, LAString(curveName).toUpper());
	}

	// Get isRenAdj
	bool isRenAdj = false;	
	LAString tmpRenAdj_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST + suffix).toUpper();
	if (tmpRenAdj_str == "TRUE")
	{
		isRenAdj = true;
	}

	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + ep_suffix, new LADataBool(isRenAdj));

	// Get the list of curve indexes
	LAStringVector markets;
	LAString tmpMarket = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffix).toUpper();
	if (tmpMarket == MLIB_NO_DATA)
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

	LADataHolder* dh;
	LAStringVector swapCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) 
	{
		swapCurves = dynamic_cast<LADataStrings &>(dh->get()).get();
	}
			
	LAStringVector basisCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		basisCurves = dynamic_cast<LADataStrings &>(dh->get()).get();
	}

	LAString mainBDF = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_DF2);
	LAStringVector genBasisSwapMarket;
	if (!markets.empty() && markets[0] != MLIB_NO_DATA)
	{
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			LAString refData_;
			LAString suffix = "." + markets[i];
			suffix.toLower();

			// set curve type			
			LAString curveType;
			LAObjectHolder& yieldData = basisCurveEngine->getYieldData().get();

			if (markets[i] == SWAP)
			{
				curveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE);
				if (curveType != MLIB_NO_DATA)
				{
					basisCurveEngine->LAObject::remove(CALIBRATION_DATA_CURVETYPE);
					basisCurveEngine->LAObject::add(CALIBRATION_DATA_CURVETYPE, new LADataString(curveType.toUpper())); 

					yieldData.remove(CALIBRATION_DATA_CURVETYPE);
					yieldData.add(CALIBRATION_DATA_CURVETYPE, new LADataString(curveType.toUpper())); 
				}
			}
			else
			{
				curveType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
				if (curveType != MLIB_NO_DATA)
				{
					basisCurveEngine->LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + markets[i]);
					basisCurveEngine->LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + markets[i], new LADataString(curveType.toUpper())); 

					yieldData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + markets[i]);
					yieldData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + markets[i], new LADataString(curveType.toUpper())); 
				}
			}

			if (markets[i] == SWAP)
			{
				if (enableCalculation || curveName == STD)
				{
					LAString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE);
					if (tmpAssignedCurves != MLIB_NO_DATA)
					{
						LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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
			
			LAString marketType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffix).toUpper();	
			basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_MARKETTYPE + LAString("_") + markets[i]);
			basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_MARKETTYPE + LAString("_") + markets[i], new LADataString(marketType));

			if (marketType == MLIB_NO_DATA)
			{
				LAString isBasisStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ISBASIS + suffix).toUpper();
				LAString isReadFile = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffix).toUpper();
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
			LAString strInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix).toUpper();
			if (strInter != MLIB_NO_DATA)
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

				LAString tmpMktName = markets[i];
				genBasisSwapMarket.push_back(markets[i]);
				LAString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != MLIB_NO_DATA && (enableCalculation || curveName == markets[i]))
				{
					LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
					for (unsigned int j = 0; j < assignedCurves.size(); j++)
					{
						if (basisCurves.end() == std::find(basisCurves.begin(),basisCurves.end(),assignedCurves[j]))
						{
							basisCurves.push_back(assignedCurves[j]);
						}
					}
					LAStringVector tmpMktNames = markets[i].toToken('_');
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
						LAString tenorswapname = dynamic_cast<const LADataString &>(basisCurveEngine->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
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

				LAString tmpMktName = markets[i];
				LAString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != MLIB_NO_DATA)
				{
					LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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

				LAString isoismode = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix).toUpper();
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
				throw LACoreInvalidData("Market type is not supported!!", __FILE__, __LINE__); 
			}

			// set yield curve pro
			refData_ = refData_.subString(0, refData_.size() - 2);
			LAStringVector tmpMktNames = markets[i].toToken('_');
			if (tmpMktNames.size() == 2)
			{
				BasisCurveCalibration &fYcPro = dynamic_cast<BasisCurveCalibration &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(tmpMktNames[0]), ENCHKTYPE_ISDEFINED).get());
				fYcPro.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + tmpMktNames[1]);
				fYcPro.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + tmpMktNames[1], new LADataMultiReference()).convertFromString(refData_);
			}
			else
			{
				basisCurveEngine->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i]);
				basisCurveEngine->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i], new LADataMultiReference()).convertFromString(refData_);
			}			
		}
	}

	//set tenorswap convention
	LAString tenorSwapName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(markets.begin(), markets.end(), tenorSwapName) == markets.end() && tenorSwapName != MLIB_NO_DATA)
	{
		LAString refData_tenor = "";
		setUpBasisCurveData(dataInstance, refData_tenor, asOfDate, currency, tenorSwapName, yieldDataName, isSpotUse, *basisCurveEngine, curveName);
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		basisCurveEngine->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName);
		basisCurveEngine->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName, new LADataMultiReference()).convertFromString(refData_tenor);
	}

	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(swapCurves));

	if (mainBDF != MLIB_NO_DATA)
	{
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new LADataString(mainBDF));
	}
			
	dataInstance.getReferencePool().completeDependency();
	
	// Whether to generate output forward rates only from swaps or from swaps + futures/FRA
	LAString swapOnly = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY);
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY);
	if (swapOnly != MLIB_NO_DATA)
	{
		if (swapOnly.toUpper() == "TRUE")
		{
			basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new LADataBool(true));
		}
		else
		{
			basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new LADataBool(false));
		}
	}

	if (!aud_origSwapRate.empty())
	{
		map<LAString, map<LAString, double> >::const_iterator it = aud_origSwapRate.begin();
		while (it != aud_origSwapRate.end())
		{
			LACurveMarketDataHelpers::restoreSwapRateFromL(*basisCurveEngine, it->second, currency, &(it->first));
			++it;
		}
	}

	// Data settings that are consistent with the single-curve building mechanism of basis curve
	bool hasNoBasisCurve = false;
	hasNoBasisCurve = genBasisSwapMarket.end() == std::find(genBasisSwapMarket.begin(), genBasisSwapMarket.end(), curveName);
	LAString tmpMktName;
	if (curveName != STD)
	{
		tmpMktName = LAString(".") + curveName;
		tmpMktName.toLower();
	}

	LAString tmpAssignedCurves;
	if (hasNoBasisCurve)
	{
		tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + tmpMktName);
	}
	else
	{
		tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + tmpMktName);
	}

	LAString targetCurve;
	if (tmpAssignedCurves != MLIB_NO_DATA && tmpAssignedCurves != "")
	{
		LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
		targetCurve = assignedCurves[0];
	}
	else
	{
		MLIB_THROW("No target basis curve defined")
	}

	if (!hasNoBasisCurve)
	{
		LAString suffix = LAString("_") + targetCurve;
		suffix.toUpper();
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF + suffix);
		basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_BASISTARGETDF + suffix, new LADataString(targetCurve));
	}
	
	// basis
	bool isBasis = false;
	if (!basisCurves.empty() && !hasNoBasisCurve)
	{
		isBasis = true;
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_BASISDFS, new LADataStrings(basisCurves));
	}

}


//! 
/*!
    @brief generate sde initial curve data

	@param[in] currency  currency or fx ex.JPY/USD
	@param[out] dataInstance
*/
void
LAUpdateObjectPoolForCurves::generateInitialValue(const LAString &currency, LADataInstance &dataInstance) const
{
	if (mCurveGenCcyMap[currency]) return;

	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

///// update for XLL Plus in grid //////////////////
	LAString useYieldSDEIRStr = LACoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
	if (useYieldSDEIRStr != MLIB_NO_DATA)
	{
		LADataBool tmpAttrBool;
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
	LAString strIsArb = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);
	if (strIsArb != MLIB_NO_DATA)
	{
		LADataBool tmpAttrBool;
		tmpAttrBool.convertFromString(strIsArb);
		isArbFree = tmpAttrBool.get();
	}
	if (isArbFree)
	{
		generateInitialValueArbfree(currency, dataInstance);
		mCurveGenCcyMap[currency] = true;
		return;
	}

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
	
	LAString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(yieldProName);
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

	LAObject *eData = NULL;
	LAString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const LAObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new LAObject();
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	eData->remove(CALIBRATION_DATA_NAME);
	eData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);
	
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;
	
	bool enableCalculation = true;
	LAString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET).toUpper();
	if (target != MLIB_NO_DATA) 
	{
		enableCalculation = false;
	}

	LAString suffix = (target == STD || target == SWAP) ? "" : "." + target;
	suffix.toLower();

	LAString ep_suffix = (target == STD || target == SWAP) ? "" : "_" + target;
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, isArbFree, suffix, ep_suffix);

	LAString fxName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != MLIB_NO_DATA)
	{
		basisCurveEngine->getFXEntity().convertFromString(fxName);
	}

	// reference data for market pro
	LAString refData;
	LAString refBData;
	map<LAString, map<LAString, double> > aud_origSwapRate;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	// setup context key
	LAString contextKey = currency;
	contextKey.toLower();

	// setup risk info
	bool isrisk=false;
	isrisk |= LACoreDataService::getContext(ARG_KEY_OFFICIALRISK)!=MLIB_NO_DATA? true: false;
	isrisk |= LACoreDataService::getContext(ARG_KEY_FRONTRISK)!=MLIB_NO_DATA? true: false;

	LAString contextYield = LACoreDataService::getContext(contextKey+CONTEXT_KEY_SDE_YIELD);

	if (!isrisk&&contextYield!=MLIB_NO_DATA)
	{
		LADataDoubleMatrix matrix;
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
		dynamic_cast<LADataDoubles &>(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<LADataDoubles &>(eData->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		if (isDF2)
		{
			LADataDoubles* attrDF2 = NULL;
			LADataHolder* df2H = &eData->getData(IR_CALIBRATION_DATA_DFS2);
			if (!df2H->isDefined())
			{
				attrDF2 = new LADataDoubles();
				eData->remove(IR_CALIBRATION_DATA_DFS2);
				eData->add(IR_CALIBRATION_DATA_DFS2,attrDF2);
			}
			else
			{
				attrDF2 = dynamic_cast<LADataDoubles*>(&df2H->get());
			}
			attrDF2->set(dfs2);
		}

		return;
	}

	// Current curve type
	// This is the type of the curve currently being constructed. To be distinguished from
	// the curve type within the for loop below
	LAString currentCurveName("");
	if (target != STD)
	{
		currentCurveName = "." + target;
		currentCurveName.toLower();
	}	
	LAString currentCurveType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + currentCurveName);
	currentCurveType.toUpper();

	if (enableCalculation || currentCurveType == SWAP) 
	{
		// Variable 'target' is essentially the name of the curve
		setUpGenCurveData(dataInstance, refData, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate, LAString(target).toUpper());
	}
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //set isRenAdj
    bool isRenAdj = false;	
	LAString tmpRenAdj_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST + suffix).toUpper();
	if (tmpRenAdj_str == "TRUE")
	{
		isRenAdj = true;
    }

	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + ep_suffix);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + ep_suffix, new LADataBool(isRenAdj));

	LAStringVector markets;
	LAString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == MLIB_NO_DATA)
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(markets);

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	if (markets.empty() || markets[0] == MLIB_NO_DATA)
	{
		// old type (exo etc..)
		LAString basisCurrency = mpStaticData->getStaticData(KEY_SDE_BASIS_BASE_CURRENCY);
		basisCurrency.toUpper();

		//if basisCurrency is empty, not set up basis curve
		if (MADealUtils::getSDECurrencys().size() > 1 && tmpCurrency.toUpper() != basisCurrency && basisCurrency != MLIB_NO_DATA)
		{
			tmpCurrency.toLower();
			// if not base currency set basis curve
			LAString basisFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASISSWAP_FILE);
			MAFileAccessor basisFile(LAMarketData::getNumFileName(basisFileName));
			LAStringMatrix basisDataMtx;
			basisFile.readAllData(MARKET_DATA_DELIMITER, basisDataMtx);
			basisFile.close();

			// get cal and calc spot date
			LAPriceDataCalendar calB;
			LAString calBStr =  mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_CALENDAR);
			calB.convertFromString(calBStr);
			LADate spotDateB;
			if (isSpotUse)
			{
				spotDateB.setDate(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_SPOTDATE).getCString());
			}
			else
			{
				spotDateB = calB.getBusinessDay(asOfDate, mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_RESETLAG).getIntValue());
			}
			// get freq
			LAString freqBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY).toUpper();
			// get daycount
			LAString daycBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT).toUpper();
			// get sliding
			LAString slidingBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE).toUpper();
			// get base swap info
			// get freq
			LAString freqBBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASEFREQUENCY).toUpper();
			// get daycount
			LAString daycBBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASEDAYCOUNT).toUpper();
			// get sliding
			LAString slidingBBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASESLIDINGRULE).toUpper();
			// get calendar
			LAString calBBStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASECALENDAR);
			// baseyield name
			LAString basecur = currency;
			LAString baseYieldName = PREFIX_YIELD + getSDEAttrName(basecur.toUpper());
			
			
			const int basisSize = basisDataMtx.size();
			for (int i = 0; i < basisSize; ++i)
			{
				LAString term = basisDataMtx[i][0].toUpper();
				double basis = basisDataMtx[i][1].getDoubleValue();

				LAObject *mktData = new LAObject();
				LAString nameB = yieldDataName + "_BASIS_" + LAString(i);
				refData += nameB + ":";
				refBData += nameB + ":";
				// set name
				mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameB);
				// set spot date
				mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateB));
				// set calendar
				mktData->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calBStr);
				// set daycount
				mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycBStr);
				// set data type
				mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BASIS);
				// set frequency
				mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freqBStr);
				// set slidingrule
				mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingBStr);
				// set term
				mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));
				// set rate
				mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(basis / 10000.0));
				// set base basis info
				// freq
				mktData->add(IR_CALIBRATION_DATA_FREQUENCYBASE, new LADataString).convertFromString(freqBBStr);
				// calender
				mktData->add(IR_CALIBRATION_DATA_CALENDARBASE, new LAPriceDataCalendar()).convertFromString(calBBStr);
				// daycount
				mktData->add(IR_CALIBRATION_DATA_DAYCOUNTBASE, new LAPriceDataDayCount()).convertFromString(daycBBStr);
				// sliding
				mktData->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE, new LAPriceDataSlidingRule()).convertFromString(slidingBBStr);

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

	LADataHolder* dh;
	LAStringVector swapCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) 
	{
		swapCurves = dynamic_cast<LADataStrings &>(dh->get()).get();
	}
			
	LAStringVector basisCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		basisCurves = dynamic_cast<LADataStrings &>(dh->get()).get();
	}

	LAString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	LAStringVector genBasisSwapMarket;
	if (!markets.empty() && markets[0] != MLIB_NO_DATA)
	{
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			tmpCurrency.toLower();
			LAString refData_;
			LAString suffix = "." + markets[i];
			suffix.toLower();

            // set curve type			
			LAString curveType;
            LAObjectHolder& yieldData = basisCurveEngine->getYieldData().get();

			if (markets[i] == SWAP)
			{
				curveType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE);
				if (curveType != MLIB_NO_DATA)
				{
					basisCurveEngine->LAObject::remove(CALIBRATION_DATA_CURVETYPE);
					basisCurveEngine->LAObject::add(CALIBRATION_DATA_CURVETYPE, new LADataString(curveType.toUpper())); 

                    yieldData.remove(CALIBRATION_DATA_CURVETYPE);
                    yieldData.add(CALIBRATION_DATA_CURVETYPE, new LADataString(curveType.toUpper())); 
				}
			}
			else
			{
				curveType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix);
				if (curveType != MLIB_NO_DATA)
				{
					basisCurveEngine->LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + markets[i]);
					basisCurveEngine->LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + markets[i], new LADataString(curveType.toUpper())); 

                    yieldData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + markets[i]);
                    yieldData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + markets[i], new LADataString(curveType.toUpper())); 
				}
			}

			if (markets[i] == SWAP)
			{
				if (enableCalculation || target == STD)
				{
					LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE);
					if (tmpAssignedCurves != MLIB_NO_DATA)
					{
						LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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
			
			LAString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffix).toUpper();	
			basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_MARKETTYPE + LAString("_") + markets[i]);
			basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_MARKETTYPE + LAString("_") + markets[i], new LADataString(marketType));

			if (marketType == MLIB_NO_DATA)
			{
				LAString isBasisStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISBASIS + suffix).toUpper();
				LAString isReadFile = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffix).toUpper();
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
			LAString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix).toUpper();
			if (strInter != MLIB_NO_DATA)
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

				LAString tmpMktName = markets[i];
				genBasisSwapMarket.push_back(markets[i]);
				LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != MLIB_NO_DATA && (enableCalculation || target == markets[i]))
				{
					LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
					for (unsigned int j = 0; j < assignedCurves.size(); j++)
					{
						if (basisCurves.end() == std::find(basisCurves.begin(),basisCurves.end(),assignedCurves[j]))
						{
							basisCurves.push_back(assignedCurves[j]);
						}
					}
					LAStringVector tmpMktNames = markets[i].toToken('_');
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
						LAString tenorswapname = dynamic_cast<const LADataString &>(basisCurveEngine->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
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

				LAString tmpMktName = markets[i];
				LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != MLIB_NO_DATA)
				{
					LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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

				LAString isoismode = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix).toUpper();
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
				throw LACoreInvalidData("Market type is not supported!!", __FILE__, __LINE__); 
			}

			// set yield curve pro
			refData_ = refData_.subString(0, refData_.size() - 2);
			LAStringVector tmpMktNames = markets[i].toToken('_');
			if (tmpMktNames.size() == 2)
			{
				BasisCurveCalibration &fYcPro = dynamic_cast<BasisCurveCalibration &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(tmpMktNames[0]), ENCHKTYPE_ISDEFINED).get());
				fYcPro.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + tmpMktNames[1]);
				fYcPro.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + tmpMktNames[1], new LADataMultiReference()).convertFromString(refData_);
			}
			else
			{
				basisCurveEngine->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i]);
				basisCurveEngine->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i], new LADataMultiReference()).convertFromString(refData_);
			}			
		}
	}

	//set tenorswap convention
	LAString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(markets.begin(), markets.end(), tenorSwapName) == markets.end() && tenorSwapName != MLIB_NO_DATA)
	{
		LAString refData_tenor = "";
		setUpBasisCurveData(dataInstance, refData_tenor, asOfDate, tmpCurrency, tenorSwapName, yieldDataName, isSpotUse, *basisCurveEngine, target);
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		basisCurveEngine->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName);
		basisCurveEngine->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName, new LADataMultiReference()).convertFromString(refData_tenor);
	}

	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(swapCurves));

	if (mainBDF != MLIB_NO_DATA)
	{
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new LADataString(mainBDF));
	}
	
	// generate yield data
	LADataProcedure &proc = dynamic_cast<LADataProcedure &>(basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

//	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	bool isCurveTargetFound = false;
	if (!enableCalculation)
	{
		isCurveTargetFound = (genBasisSwapMarket.end() == std::find(genBasisSwapMarket.begin(),genBasisSwapMarket.end(), target) 
							    && target != CURVETYPE_FLOATER);
		if (target != CURVETYPE_FLOATER)
		{
			LAString tmpMktName;
			if (target != STD) 
			{
				tmpMktName = LAString(".") + target; 
				tmpMktName.toLower();
			}

			LAString tmpAssignedCurves;
			if (isCurveTargetFound)
			{
				tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + tmpMktName);
			}
			else
			{
				tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + tmpMktName);
			}

			LAString targetDF;
			if (tmpAssignedCurves != MLIB_NO_DATA && tmpAssignedCurves != "")
			{
				LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
				targetDF = assignedCurves[0];
			}
			else
			{
				throw LACoreInvalidData("No target df!", __FILE__, __LINE__); 
			}

			if (isCurveTargetFound)
			{
				basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
				basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(targetDF));
			}
			else
			{
				basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
				basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new LADataString(targetDF));
			}
		}	
	}
	dataInstance.getReferencePool().completeDependency();

	// get fwdfx flag
	bool isFwdFX = false;
	LAString isFwdFXStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX);
	if (isFwdFXStr.toUpper() == "TRUE") 
	{
		isFwdFX = true;
	}

	// Whether to generate output forward rates only from swaps or from swaps + futures/FRA
	LAString swapOnly = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY);
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY);
	if (swapOnly != MLIB_NO_DATA)
	{
		if (swapOnly.toUpper() == "TRUE")
		{
			basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new LADataBool(true));
		}
		else
		{
			basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY, new LADataBool(false));
		}
	}
	
	if ((enableCalculation || isCurveTargetFound) && !isFwdFX)
	{
		if (isCurveTargetFound)
		{
			basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new LADataBool(true));
		}
		proc.estimate(asOfDate);
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	}
	
	// basis
	bool isBasis = false;
	if (!basisCurves.empty() && !isCurveTargetFound && target != CURVETYPE_FLOATER)
	{
		isBasis = true;
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_BASISDFS, new LADataStrings(basisCurves));
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
			basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new LADataBool(true));
		}
		basisCurveEngine->setBasisRates();
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
	}

	LAString genFloaterName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != MLIB_NO_DATA && 
		(enableCalculation || target == CURVETYPE_FLOATER)) 
	{
		setUpFloater(tmpCurrency, *basisCurveEngine, genFloaterName);
	}

	// set df2
	if (target == STD && mainBDF != MLIB_NO_DATA)
	{
		basisCurveEngine->setDF2();
	}

	if (!aud_origSwapRate.empty())
	{
		map<LAString, map<LAString, double> >::const_iterator it = aud_origSwapRate.begin();
		while (it != aud_origSwapRate.end())
		{
			LACurveMarketDataHelpers::restoreSwapRateFromL(*basisCurveEngine, it->second, currency, &(it->first));
			++it;
		}
	}
	// set daycount
//	setUpCurveTypeDayCount(*basisCurveEngine, *yc);

	mCurveGenCcyMap[currency] = true;
////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	if (contextYield!=MLIB_NO_DATA)
	{
		LADataDoubleMatrix matrix;
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
		dynamic_cast<LADataDoubles &>(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<LADataDoubles &>(eData->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		eData->remove(IR_CALIBRATION_DATA_DFS2);
		if (isDF2)
		{
			eData->add(IR_CALIBRATION_DATA_DFS2,new LADataDoubles(dfs2));
		}

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
	{
		LAStringVector dataoutCurves;
		dataoutCurves.push_back(STD);	

		const std::map<LAString, LAString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
            if (it->first != STD && std::find(markets.begin(), markets.end(), it->second) != markets.end()) {
                dataoutCurves.push_back(it->first);
            }
		}
		// floater curve
		if (genFloaterName != MLIB_NO_DATA) dataoutCurves.push_back(genFloaterName);
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
LAUpdateObjectPoolForCurves::setUpCurveTypeDayCount(BasisCurveCalibration &basisCurveEngine, LAMathYieldCurve &yc) const
{
	// set daycount
	const map<LAString, LAString> &assignedCurveMktMap = basisCurveEngine.getAssignedCurveMktMap();
	map<LAString, LAString>::const_iterator it_cur = assignedCurveMktMap.begin();
	while (it_cur != assignedCurveMktMap.end())
	{
		LAPriceDataDayCount dc;
		LAPriceDataCalendar cal;
		LAPriceDataSlidingRule sld;
		LAString accessory;
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
LAUpdateObjectPoolForCurves::generateInitialValueArbfree(const LAString &currency, LADataInstance &dataInstance) const
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
	
	LAString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(yieldProName);
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

	LAObject *eData = NULL;
	LAString yieldDataName = yieldName + "_DATA";
	if (isSetCurveID == "TRUE")
	{
		yieldDataName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
	}
	const LAObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new LAObject();
		eData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;

	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, true);
	// set isArbFree
	basisCurveEngine->getIsArbFree().set(true);
	bool isRenAdj = false;
	LAString tmpRenAdj_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST).toUpper();
	if (tmpRenAdj_str != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(tmpRenAdj_str);
		isRenAdj = tmpAttrB.get();
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

	eData->remove(IR_CALIBRATION_DATA_6MLCURVENAMES);
	eData->remove(IR_CALIBRATION_DATA_DFCURVENAMES);
	eData->remove(IR_CALIBRATION_DATA_3MLCURVENAMES);	
	eData->add(IR_CALIBRATION_DATA_6MLCURVENAMES, new LADataStrings(curveNames_6ML));
	eData->add(IR_CALIBRATION_DATA_DFCURVENAMES, new LADataStrings(curveNames_DF));
	eData->add(IR_CALIBRATION_DATA_3MLCURVENAMES, new LADataStrings(curveNames_3ML));
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new LADataBool(isRenAdj));
	basisCurveEngine->setArbFreeCurveName(curveNames_6ML,curveNames_DF,curveNames_3ML);

	// reference data for market pro
	LAString refData;
	LAString refBData;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	// setup context key
	LAString contextKey = currency;
	contextKey.toLower();

	// setup risk info
	bool isrisk=false;
	isrisk |= LACoreDataService::getContext(ARG_KEY_OFFICIALRISK)!=MLIB_NO_DATA? true: false;
	isrisk |= LACoreDataService::getContext(ARG_KEY_FRONTRISK)!=MLIB_NO_DATA? true: false;

	LAString contextYield = LACoreDataService::getContext(contextKey+CONTEXT_KEY_SDE_YIELD);

	if (!isrisk&&contextYield!=MLIB_NO_DATA)
	{
		LADataDoubleMatrix matrix;
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
		dynamic_cast<LADataDoubles &>(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<LADataDoubles &>(eData->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		if (isDF2)
		{
			LADataDoubles* attrDF2 = NULL;
			LADataHolder* df2H = &eData->getData(IR_CALIBRATION_DATA_DFS2);
			if (!df2H->isDefined())
			{
				attrDF2 = new LADataDoubles();
				eData->add(IR_CALIBRATION_DATA_DFS2,attrDF2);
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

	map<LAString, map<LAString, double> > aud_origSwapRate;
	setUpGenCurveData(dataInstance, refData, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate);

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
		if (usd3mlFloaterInfo.size() != 2) throw LACoreInvalidData("No XccyBasis information!",__FILE__,__LINE__);
		xccyBasis->add(IR_CALIBRATION_DATA_AGTFORECAST, new LADataString()).convertFromString(usd3mlFloaterInfo[1]);
		// set foreign ccy info
		LAString ccy_floater, fYieldDataName;
		if (isSetCurveID == "TRUE")
		{
			LAObject* fYieldData = &objPool.getObject(usd3mlFloaterInfo[0], ENCHKTYPE_ISDEFINED).get();
			ccy_floater = dynamic_cast<LADataString& > ((fYieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
			fYieldDataName = usd3mlFloaterInfo[0];
		}
		else
		{
			LAString isPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
			if (isPricer == "TRUE")
			{
				LAObject* yieldData = &objPool.getObject(usd3mlFloaterInfo[0], ENCHKTYPE_ISDEFINED).get();
				ccy_floater = dynamic_cast<LADataString& > ((yieldData->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
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
		LAString curveName_floater = usd3mlFloaterInfo[1];
		basisCurveEngine->getForeignYieldData().convertFromString(fYieldDataName);

		if (isSetCurveID != "TRUE")
		{
			LAString baseCcy = currency; LAString domCcy = currency; LAString forCcy = ccy_floater;
			while (1)
			{
				BasisCurveCalibration &ycPro_dccy = dynamic_cast<BasisCurveCalibration &>
							(objPool.getObject(LAMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				BasisCurveCalibration &ycPro_fccy = dynamic_cast<BasisCurveCalibration &>
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

		if (isRenAdj)
		{
			LAStringVector baseCcyDFInfo = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASECCYDF).toToken(':');
			if (baseCcyDFInfo.size() != 2) throw LACoreInvalidData("No XccyBasis information!",__FILE__,__LINE__);
			if (usd3mlFloaterInfo[0] != baseCcyDFInfo[0]) throw LACoreInvalidData("Reference currencies are inconsistent!",__FILE__,__LINE__);
			xccyBasis->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new LADataString()).convertFromString(baseCcyDFInfo[1]);
		}

		// set yield curve pro
		refData_XccyBasis = refData_XccyBasis.subString(0, refData_XccyBasis.size() - 2);
		basisCurveEngine->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + XCCYBASIS);
		basisCurveEngine->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + XCCYBASIS, new LADataMultiReference()).convertFromString(refData_XccyBasis);
	}
	
	tmpCurrency.toLower();
	LAStringVector markets;
	LAString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == MLIB_NO_DATA)
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(markets);
	if (currency != CURRENCY_USD && find(markets.begin(), markets.end(), XCCYBASIS) == markets.end()) throw LACoreInvalidData("XCCYBASIS is not in generate dfs!!", __FILE__, __LINE__); 
	if (find(markets.begin(), markets.end(), THREESIXBASIS) == markets.end())
	{
		LAString refData_;
		setUp36BasisDummyData(dataInstance, refData_, asOfDate, tmpCurrency, THREESIXBASIS, yieldDataName, isSpotUse, *basisCurveEngine);
		// set yield curve pro
		refData_ = refData_.subString(0, refData_.size() - 2);
		basisCurveEngine->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + THREESIXBASIS);
		basisCurveEngine->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + THREESIXBASIS, new LADataMultiReference()).convertFromString(refData_);
	}
	// set yield curve pro
	refData = refData.subString(0, refData.size() - 2);
	basisCurveEngine->getMarketData().convertFromString(refData);

	LADataHolder* dh;
	LAStringVector swapCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) swapCurves = dynamic_cast<const LADataStrings &>(dh->get()).get();

	LAStringVector basisCurves;
	dh = &basisCurveEngine->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) basisCurves = dynamic_cast<const LADataStrings &>(dh->get()).get();

	LAString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (!markets.empty() && markets[0] != MLIB_NO_DATA)
	{
		//LADataStrings &attrGenCurves = dynamic_cast<LADataStrings &>(basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings()).get());		
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			if (markets[i] == SWAP) continue;
			
			tmpCurrency.toLower();
			LAString refData_;
			LAString suffix = "." + markets[i];
			suffix.toLower();
			LAString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffix).toUpper();
			if (marketType == MLIB_NO_DATA)
			{
				LAString isBasisStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISBASIS + suffix).toUpper();
				LAString isReadFile = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffix).toUpper();
				if (isBasisStr == "TRUE") marketType = MARKETTYPE_BASIS;
				else if (isReadFile == "TRUE") marketType = MARKETTYPE_ZERORATE;
				else marketType = MARKETTYPE_SWAP;
			}

			// set up interpolation
			LAString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix).toUpper();
			if (strInter != MLIB_NO_DATA)
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
					LAString tmpMktName = markets[i];
					LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + tmpMktName.toLower());
					if (tmpAssignedCurves != MLIB_NO_DATA)
					{
						LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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

				LAString tmpMktName = markets[i];
				LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != MLIB_NO_DATA)
				{
					LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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

				LAString isoismode = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix).toUpper();
				if (isoismode.toUpper() == "DAILYCOMPOUNDING")
					setUpGenCurveDataOIS(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate);
				else
					setUpGenCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *basisCurveEngine, aud_origSwapRate);
			}
			else
			{
				throw LACoreInvalidData("Market type is not supported!!", __FILE__, __LINE__); 
			}

			// set yield curve pro
			refData_ = refData_.subString(0, refData_.size() - 2);
			basisCurveEngine->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i]);
			basisCurveEngine->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i], new LADataMultiReference()).convertFromString(refData_);
		}
	}

	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(swapCurves));

	// generate yield data
	basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	LADataProcedure &proc = dynamic_cast<LADataProcedure &>
						(basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	dataInstance.getReferencePool().completeDependency();
	proc.estimate(asOfDate);
	
	// generate swap curve
	if (swapCurves.size() != 0 && enableCalculation)
	{
		basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
		for (size_t i=0; i<swapCurves.size(); i++)
		{
			basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(swapCurves[i]));
			proc = dynamic_cast<LADataProcedure &> (basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
			proc.estimate(asOfDate);
			basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
		}
		basisCurveEngine->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	}

	//adjust discountfactor
	tmpCurrency.toLower();
	LADataBool tmpAttrDF;
	tmpAttrDF.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF));
	if(tmpAttrDF.get())
	{
		const std::map<LAString, LAString> &assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (!basisCurveEngine->isBasisCurve(it->first) && (enableCalculation || target == it->second))
			{
				LAMarketData::adjustDiscountFactor(*eData, it->first);
			}
		}
	}

	// basis
	bool isBasis = false;
	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_BASISDFS);
	if (!basisCurves.empty() && enableCalculation)
	{
		isBasis = true;
		basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_BASISDFS, new LADataStrings(basisCurves));
	}

	basisCurveEngine->LAObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
	if (mainBDF != MLIB_NO_DATA)
	{
		isBasis = true;
		basisCurveEngine->LAObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new LADataString(mainBDF));
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

	LAString genFloaterName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != MLIB_NO_DATA || target == CURVETYPE_FLOATER) setUpFloater(tmpCurrency, *basisCurveEngine, genFloaterName);


////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	if (contextYield!=MLIB_NO_DATA)
	{
		LADataDoubleMatrix matrix;
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
		dynamic_cast<LADataDoubles &>(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED).get()).set(terms);
		dynamic_cast<LADataDoubles &>(eData->getData(IR_CALIBRATION_DATA_DFS, ISDEFINED).get()).set(dfs);
		eData->remove(IR_CALIBRATION_DATA_DFS2);
		if (isDF2)
		{
			eData->add(IR_CALIBRATION_DATA_DFS2,new LADataDoubles(dfs2));
		}

		return;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
	{
		LAStringVector dataoutCurves;
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

		const std::map<LAString, LAString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second != SWAP && it->second != XCCYBASIS && it->second != THREESIXBASIS) dataoutCurves.push_back(it->first);
		}

		if (genFloaterName != MLIB_NO_DATA) dataoutCurves.push_back(genFloaterName);

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
LAUpdateObjectPoolForCurves::generateCorrelation(const LAString &currency, LADataInstance &dataInstance) const
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

#endif
//! 
/*!
    @brief get SDE data name

	@param[in] currency
	@return LAString
*/
LAString
LAUpdateObjectPoolForCurves::getSDEAttrName(const LAString &currency) const
{
	LAString key_ccy = currency;
	return  mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
}

//! 
/*!
    @brief get Grid property val

	@param[in] key
	@param[in] curve
	@param[in] grid
	@return LAString
*/
LAString
LAUpdateObjectPoolForCurves::getGridStaticData(const LAString &key, const LAString &curve, const LAString &grid) const
{
	return etrading::getGridStaticData(mpStaticData, key, curve, grid);
}

//! 
/*!
    @brief fuction to set up generate configuration
*/
void
LAUpdateObjectPoolForCurves::setUpGenerateConfig
(LADataInstance &dataInstance, const LADate &asOfDate, const LAString &currency, LAMathYieldCurve &yc, BasisCurveCalibration &basisCurveEngine,
 LAObject &ycData, bool &isAudExtra, bool &isSwapTenorAdjust, bool &isSpotUse, bool isArbFree, const LAString& suffix, const LAString& epSuffix) const
{
	LAString isSetCurveID = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

	LAString yieldName = yc.getName().get();

	yc.LAObject::remove(CALIBRATION_DATA_ASOFDATE);
	yc.LAObject::add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));

	// set interpolation
	LAString interp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix);
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
	LAString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (mainBDF != MLIB_NO_DATA) 
	{
		yc.setBasisCurveType(mainBDF);
	}

	// set yield to object pool
	LAObjectPool &objPool = dataInstance.getObjectPool();

	// set yieldpro to object pool
	// set as of date
	basisCurveEngine.getAsOfDate().set(asOfDate);
	
	// set curve generator
	basisCurveEngine.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
	
	// set interpolation
	LAString genInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION + suffix);
	if (genInterp == MLIB_NO_DATA)
	{
		genInterp = FN_SPLINEINTERPOLATION_STR;
	}	
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation().convertFromString(genInterp.toLower());
	}
	else
	{
		basisCurveEngine.LAObject::remove(CALIBRATION_DATA_INTERPOLATION + epSuffix);
		basisCurveEngine.LAObject::add(CALIBRATION_DATA_INTERPOLATION + epSuffix, new LAPriceDataInterpolation()).convertFromString(genInterp);
	}
	basisCurveEngine.getInterpolation().convertFromString(genInterp.toLower());

	// interpolation for OIS and STD swaps
	LAString genYieldGenInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
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
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation_yg().convertFromString(genYieldGenInterp.toLower());
	}
	else
	{
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix, new LAPriceDataInterpolation()).convertFromString(genYieldGenInterp);
	}

	// interpolation for FRA/futures
	LAString genFutureInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FUTURE_INTERPOLATION + suffix);
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
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation_fw().convertFromString(genFutureInterp.toLower());
	}
	else
	{
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONFW + epSuffix);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONFW + epSuffix, new LAPriceDataInterpolation()).convertFromString(genFutureInterp);
	}

	// interpolation for basis
	LAString genBasisInterp = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASIS_INTERPOLATION + suffix);
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
	if (suffix.size() == 0)
	{
		basisCurveEngine.getInterpolation_bs().convertFromString(genBasisInterp.toLower());
	}
	else
	{
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONBS + epSuffix);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONBS + epSuffix, new LAPriceDataInterpolation()).convertFromString(genBasisInterp);
	}
	
	// set basis function
	LAString genBasisFunc = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASISFUNCTION);
	if (genBasisFunc == MLIB_NO_DATA)
	{
		genBasisFunc = FN_BASISFUNC2_STR;
	}
	basisCurveEngine.getBasisFunction().setFunction(genBasisFunc.toLower());
	
	// set daycount
	LAString genDayCount = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT);
	if (genDayCount != MLIB_NO_DATA)
	{
		basisCurveEngine.getDayCount().convertFromString(genDayCount.toUpper());
	}
	
	// set freq
	LAString genFreq = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY);
	if (genFreq != MLIB_NO_DATA)
	{
		basisCurveEngine.getFrequency().convertFromString(genFreq.toUpper());
	}
	
	// set sliding rulue
	LAString genSlidingRule = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE);
	if (genSlidingRule != MLIB_NO_DATA)
	{
		basisCurveEngine.getSlidingRule().convertFromString(genSlidingRule.toUpper());
	}

	// set rate priority
	LAString ratePrio_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_RATEPRIORITY).toUpper();
	if (ratePrio_str != MLIB_NO_DATA) 
	{		
		basisCurveEngine.getData(PRICING_DATA_RATEPRIORITY).convertFromString(ratePrio_str);	
	}
	
	// set tenor adjust
	LADataBool tmpAttrB;
	LAString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + suffix).toUpper();
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
	isSwapTenorAdjust = tmpAttrB.get();
	
	//20170531 - Fixed Fatal Error For Win-Server2012
	#if (WINVER >= 0x0601)
		LADataHolder *ahTemp;
		ahTemp = &basisCurveEngine.LAObject::getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, NOCHECK);
		if (!ahTemp->isDefined())
		{
			basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, new LADataBool(isSwapTenorAdjust));
		}
		else
		{
			dynamic_cast<LADataBool&>(ahTemp->get()).set(isSwapTenorAdjust);
		}
	#else	
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + epSuffix, new LADataBool(isSwapTenorAdjust));
	#endif
	
	if (isSwapTenorAdjust)
	{
		LAString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
		if (tenorSwapName == MLIB_NO_DATA) 
		{
			throw LACoreInvalidData("Set tenor swap name!", __FILE__, __LINE__);
		}
		
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_TENORSWAPNAME);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_TENORSWAPNAME, new LADataString(tenorSwapName));
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
	basisCurveEngine.getYieldData().convertFromString(yieldDataName);
	
	// set base curve reference
	basisCurveEngine.getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(yieldName);

	// set currency in curve ID for excel pricer 
	const LAString isPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE" && isSetCurveID != "TRUE")
	{
		LAString curveID = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID);
		const LAObjectHolder ehdata = objPool.getObject(curveID);
		LAObject *eCurveID = NULL;
		if (!ehdata.isDefined())
		{
			eCurveID = new LAObject();
			objPool.set(curveID, eCurveID);
		}
		else
		{
			objPool.getObject(curveID).get().reset();
			eCurveID = &objPool.getObject(curveID).get();
		}
		eCurveID->add(CALIBRATION_DATA_NAME, new LADataString(curveID));
		eCurveID->add(IR_CALIBRATION_DATA_CURRENCY, new LADataString(currency));
	}

	LAString strIsAudExtra = mpStaticData->getStaticData(KEY_SDE_YIELD_ISAUDEXTRA);
	if (strIsAudExtra != MLIB_NO_DATA)
	{
		LADataBool tmpIsAudExtra;
		tmpIsAudExtra.convertFromString(strIsAudExtra);
		isAudExtra = tmpIsAudExtra.get();
	}

	if (isAudExtra && isSwapTenorAdjust) 
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
	isSpotUse = tmpAttrB.get();

	// set curve exist check
	if (isPricer == "FALSE" || isPricer == MLIB_NO_DATA)
	{
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, new LADataBool(true));
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

//! 
/*!
    @brief fuction to set up basis curve data
*/
void
LAUpdateObjectPoolForCurves::setUpBasisCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &curveCurrency, 
									  const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, BasisCurveCalibration &basisCurveEngine, const LAString& currentCurveName, const LAString* pMktCurrency) const
{
	LAString mktCurrency = pMktCurrency ? *pMktCurrency : curveCurrency;
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

	LAString yieldBasisTarget = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_TARGET + curveSuffix).toUpper();
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
		basisCurveEngine.remove(CALIBRATION_DATA_BASISCURVEBASE + LAString("_") + curveMktName);
		basisCurveEngine.LAObject::add(CALIBRATION_DATA_BASISCURVEBASE + LAString("_") + curveMktName, new LADataString(a_dCurve));

		basisCurveEngine.remove(CALIBRATION_DATA_BASISCURVESECONDARYBASE + LAString("_") + curveMktName);
		basisCurveEngine.LAObject::add(CALIBRATION_DATA_BASISCURVESECONDARYBASE + LAString("_") + curveMktName, new LADataString(a_fCurve));

		// Store the forecast curve on the same side as the target discount
		basisCurveEngine.remove(CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE + LAString("_") + curveMktName);
		basisCurveEngine.LAObject::add(CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE + LAString("_") + curveMktName, new LADataString(fCurve));
	}
	else
	{
		basisCurveEngine.remove(CALIBRATION_DATA_BASISCURVEBASE + LAString("_") + curveMktName);
		basisCurveEngine.LAObject::add(CALIBRATION_DATA_BASISCURVEBASE + LAString("_") + curveMktName, new LADataString(a_fCurve));
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
		LAString basisFileName = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + mktSuffix);
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

	if (adjustValueMtx.size() != 0 && adjustValueMtx[0].size() < 2 )
	{
		LAString msg = marketName + ", basis adjust file is empty";
		throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}
	DoubleArray adjustValue_term,adjustValue;
	for (size_t i=0; i<adjustValueMtx.size(); i++)
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
	convertCurveName(tmp_fCurve, curveCurrency, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(tmp_dCurve, curveCurrency, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(tmp_a_fCurve, curveCurrency, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(tmp_a_dCurve, curveCurrency, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	if (ccy_dCurve != curveCurrency || ccy_fCurve != curveCurrency || ccy_a_dCurve != ccy_a_fCurve)
	{
		throw LACoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);
	}

	// Get curve collection ID
	LAObjectHolder& yData = basisCurveEngine.getYieldData().get();
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
	basisCurveEngine.remove(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + LAString("_") + curveMktName);
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

			basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + LAString("_") + curveMktName, new LADataString(fCurves_partitioned[0]));
		}
		else
		{
			throw LACoreInvalidData("#Error: Forecast curve and discount curve of the target leg must have the same curve collection prefix", __FILE__, __LINE__);
		}
	}

	// against currency
	bool isForeignCcyLeg = false;
	LAString fYieldDataName;
	basisCurveEngine.remove(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + LAString("_") + curveMktName);
	if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrency		
		|| useAgainstCurveCollection) 
	{
		isForeignCcyLeg = true;

		if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrency)
		{
			// Re-set the curveType to be XCCYBASIS
			LAString curveType = XCCYBASIS;
			basisCurveEngine.LAObject::remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveMktName);
			basisCurveEngine.LAObject::add(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveMktName, new LADataString(curveType.toUpper())); 

			LAObjectHolder& yieldData = basisCurveEngine.getYieldData().get();
            yieldData.remove(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveMktName);
            yieldData.add(CALIBRATION_DATA_CURVETYPE + LAString("_") + curveMktName, new LADataString(curveType.toUpper())); 

			// Also re-set the curveType to be XCCYBASIS within the properties
			mpStaticData->setStaticData( mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + mktSuffix, XCCYBASIS );
		}

		ccy_a_fCurve.toUpper();
		if (isSetCurveID)
		{
			LAStringVector tmp_ccy_a_fCurve = tmp_a_fCurve.toToken(':');
			if(tmp_ccy_a_fCurve.size() == 2)
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
			generateInitialValue(ccy_a_fCurve, dataInstance);
		}
		basisCurveEngine.getForeignYieldData().convertFromString(fYieldDataName);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + LAString("_") + curveMktName, new LADataBool(isForeignCcyLeg));
		
		if (!isSetCurveID)
		{
			LAString baseCcy = curveCurrency; LAString domCcy = curveCurrency; LAString forCcy = ccy_a_fCurve;
			while (1)
			{
				BasisCurveCalibration &ycPro_dccy = dynamic_cast<BasisCurveCalibration &>
							(objPool.getObject(LAMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				BasisCurveCalibration &ycPro_fccy = dynamic_cast<BasisCurveCalibration &>
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
	if ((a_fCurve == DUMMY || a_dCurve == DUMMY) && !isSpotUse)
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

			MLIB_REQUIRE( spotRate != DBL_MAX,  "#Error: Can't locate spot Libor fixing rate that corresponds to '" + spotRateTerm + "'.");
		}
		else
		{
			if (fixingSource == MLIB_NO_DATA)
			{
				marketRef = basisCurveEngine.getMarketData();
			}
			else 
			{
				// Use fixingSource to obtain an external yieldCurvePro
				LAString foreignYcProName = etrading::LACurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + fixingSource;
				foreignYcProName.toUpper();

				BasisCurveCalibration* foreignYieldCurvePro = NULL;

				const LAObjectHolder tmpHolder = objPool.getObject( foreignYcProName );
				if( tmpHolder.isDefined() )
				{
					foreignYieldCurvePro = &dynamic_cast<BasisCurveCalibration&>( objPool.getObject( foreignYcProName ).get() );
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
						LAString nameB = yieldDataName + "_LIBOR_" + LAString(0) + "_" + curveMktName;
						const LAObjectHolder ehbasis = objPool.getObject(nameB);
						if (!ehbasis.isDefined())
						{
							mktData = new LAObject();
							objPool.set(nameB, mktData);
						}
						else
						{
							objPool.getObject(nameB).get().clear();
							mktData = &objPool.getObject(nameB).get();
						}

						refData += nameB + ":";
					
						// set term
						mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));

						// set name
						mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameB);

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
						const LAPriceDataCalendar& cal  = dynamic_cast<const LAPriceDataCalendar&> ((marketRef.get(i).getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
						mktData->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar(cal));

						// Set sliding rule
						const LAPriceDataSlidingRule& sld  = dynamic_cast<const LAPriceDataSlidingRule&> ((marketRef.get(i).getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
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
	bool fwd_isRatio = false;
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
	double fwd_denominator = 1.0;

	// for fwd fx	
	if (isFwdFX)
	{
		LAString isRatioStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISRATIO + mktSuffix).toUpper();
		if (isRatioStr == "TRUE") fwd_isRatio = true;
		if (!fwd_isRatio)
		{
			fwd_denominator = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_DENOMINATOR + mktSuffix).getDoubleValue();
			LAString isPriceCcyStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISPRICECCY + mktSuffix).toUpper();
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
			LAString msg = marketName + ", forward fx file is empty";
			throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
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
		LAString msg = marketName + ", basis file is empty";
		throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
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

	const LADate date1Y = etrading::LADateHelpers::getDate(asOfDate, "1Y", true);
	double ndfSize = 0.0;
	for (unsigned int j = 0; j < fwdFXSize + basisSize; ++j)
	{
		LAObject *mktData = NULL;		
		LAString nameB = yieldDataName + "_BASIS_" + LAString(static_cast<int>(j)) + "_" + curveMktName;
		const LAObjectHolder ehbasis = objPool.getObject(nameB);
		if (!ehbasis.isDefined())
		{
			mktData = new LAObject();
			objPool.set(nameB, mktData);
		}
		else
		{
			objPool.getObject(nameB).get().clear();
			mktData = &objPool.getObject(nameB).get();
		}

		refData += nameB + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameB);

		// set forward ratio
		if (j < fwdFXSize)
		{
			LAString fwd_term = fwdFXDataMtx[j][0].toUpper();
			double fwd_fx = fwdFXDataMtx[j][1].getDoubleValue();

			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_FWDFX);
			mktData->add(IR_CALIBRATION_DATA_ISRATIO, new LADataBool(fwd_isRatio));
			if (fwd_isRatio)
				mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(fwd_fx));
			else
			{
				mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(fwd_fx / fwd_denominator));
				mktData->add(IR_CALIBRATION_DATA_ISPRICECCY, new LADataBool(fwd_isPriceCcy));
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
				LAString ndfTerm = basisDataMtx[j][0].toUpper();
				const LADate date = etrading::LADateHelpers::getDate(asOfDate, ndfTerm, true);
				
				if (date <= date1Y)
				{
					// set term
					mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(ndfTerm));

					// set rate
					double ndfRate = basisDataMtx[j][1].getDoubleValue();
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
				mktData->add(IR_CALIBRATION_DATA_ISFWDBASIS, new LADataBool(isFwdBasis) );
				if (basisDataMtx[j2].size() != 5)
					throw LACoreInvalidData("FwdBasis File format is wrong", __FILE__,__LINE__);
				const bool isDate = basisDataMtx[j2][2].toUpper() == "TRUE";
				mktData->add(PRICING_DATA_ISDATE, new LADataBool(isDate) );
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
		fYieldData.add(CALIBRATION_DATA_MARKETDATA + LAString("_") + marketName, new LADataMultiReference()).convertFromString(refData.subString(0, refData.size() - 2));
	}

	LAString tmpAssignedCurves = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + curveSuffix);
	if (tmpAssignedCurves != MLIB_NO_DATA)
	{
		LAStringVector assignedCurves = tmpAssignedCurves.toToken(MULTI_STATIC_DATA_DELIMITER);
		for (size_t i = 0; i<assignedCurves.size(); i++)
		{
			basisCurveEngine.setAssignedCurveMktMap(assignedCurves[i], curveMktName);
		}
	}
	else
	{
		basisCurveEngine.setAssignedCurveMktMap(curveMktName,curveMktName);
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
	bool isFwdSwap = false;
	LADataBool tmpAttrB;
	LAString isFwdSwap_str = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + mktSuffix);
	if (isFwdSwap_str != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		isFwdSwap = tmpAttrB.get();
	}

	//FRA Data
	bool isAudExtra = false;
	LAString staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	LAString suffix_data = "_" + marketName;

	LAString curveName("");
	if (currentCurveName != STD)
	{
		curveName = "." + currentCurveName;
	}
	curveName.toLower();
	LAString curveType = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + curveName);

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
LAUpdateObjectPoolForCurves::setUp36BasisDummyData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
									    const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, 
										BasisCurveCalibration &basisCurveEngine) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	LAString suffix = "." + marketName;
	suffix.toLower();

	// get swap market
	LAString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE);
	MAFileAccessor swapFile(LAMarketData::getNumFileName(swapFileName));
	LAStringMatrix swapDataMtx;
	swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
	swapFile.close();
	if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2 )
	{
		LAString msg = "swap file is empty";
		throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}
	const int swapSize = swapDataMtx.size();
	LAString daycSStr_float = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT).toUpper();
	LAString interpSStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_INTERPOLATION).toLower();
	LAPriceDataCalendar cal;
	LAString calStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR).toLower();
	cal.convertFromString(calStr);
	LADate spotDate;
	int resetLag;
	if (isSpotUse)
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
		LAString nameB = yieldDataName +  "_" + marketName + "_" + LAString(static_cast<int>(j));
		const LAObjectHolder ehbdummy = objPool.getObject(nameB);
		if (!ehbdummy.isDefined())
		{
			mktData = new LAObject();
			objPool.set(nameB, mktData);
		}
		else
		{
			objPool.getObject(nameB).get().clear();
			mktData = &objPool.getObject(nameB).get();
		}
		refData += nameB + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameB);
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
		mktData->add(IR_CALIBRATION_DATA_CASHLETCALENDAR , new LAPriceDataCalendar(cal));
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

//! 
/*!
    @brief fuction to set up base curve data
*/
void 
LAUpdateObjectPoolForCurves::setUpGenCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
									const LAString &marketName, const LAString &yieldDataName, bool isSpotUse,
									bool isAudExtra, BasisCurveCalibration &basisCurveEngine, 
									std::map<LAString, std::map<LAString, double> > &aud_origSwapRate, const LAString& curveName) const
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
	LAString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix); 
	if (dfCurveName == MLIB_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new LADataString(dfCurveName));
	LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new LADataString(dfCurveName));

	// get fwd swap
	bool isFwdSwap = false;
	LADataBool tmpAttrB;
	LAString isFwdSwap_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + staticDataSuffix);
	if (isFwdSwap_str != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		isFwdSwap = tmpAttrB.get();
	}

	// Do we always recalculate the dynamic linear spline join date?
	LAString alwaysCalcJoinDate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE + staticDataSuffix);
	basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);	
	if (alwaysCalcJoinDate != MLIB_NO_DATA && alwaysCalcJoinDate.size() != 0)
	{
		if (alwaysCalcJoinDate.toUpper() == "TRUE")
		{
			basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(true));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(true));
		}
		else
		{
			basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(false));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(false));
		}
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
	basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
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
		if (isSpotUse)
		{
			spotDate.setDate(mpStaticData->getStaticData(currency + spotDateKey + staticDataSuffix).getCString());
		}
		else
		{
			spotDate = cal.getBusinessDay(asOfDate, mpStaticData->getStaticData(tmpCurrency + spotLagKey + staticDataSuffix).getIntValue());
		}

		//Use NO_CHANGE as businessDayAdj
		LADate inputInterpolationJoinDate = etrading::validateDateOrTenor(spotDate, inputInterpJoinDateStr, "NO_CHANGE", calStr, "Invalid joinDate");

		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputInterpolationJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputInterpolationJoinDate));
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
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data, new LADataBool(isFutureUse));
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
        LAString meanReversion_str = mpStaticData->getStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_MEANREVERSION + staticDataSuffix );
        if ( meanReversion_str != MLIB_NO_DATA )
        {
            LADataDouble tmpAttrDouble;
            tmpAttrDouble.convertFromString( meanReversion_str );
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

		// get UseConvexAdjustment
		bool useConvexAdjustment = false;
		LAString useConvexAdjustmentStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USECONVEXADJUSTMENT + staticDataSuffix).toUpper();
        if (useConvexAdjustmentStr != MLIB_NO_DATA)
        {
            LADataBool tmpUseConvexAdjustment;
			tmpUseConvexAdjustment.convertFromString(useConvexAdjustmentStr);
			useConvexAdjustment = tmpUseConvexAdjustment.get();
        }

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
						
        const int futureSize = futureDataMtx.size();
		for (int i = 0; i < futureSize; ++i)
		{
			if (futureDataMtx[i].size() != 5 && futureDataMtx[i].size() != 3)
			{
				throw LACoreInvalidData("Future File format is wrong", __FILE__,__LINE__);
			}
			LAString term;
			LADate startDate,endDate;
			double futurePrice(0.0), rate(0.0), futureVol(0.0), convexAdj(0.0);

			if (futureDataMtx[i].size() == 5)
			{
				term = futureDataMtx[i][0].toUpper();
				startDate = LADate(futureDataMtx[i][1].getCString());
				endDate = LADate(futureDataMtx[i][2].getCString());
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
				startDate = etrading::LADateHelpers::getIMMDateFromTerm(asOfDate, term);
				startDate = etrading::LADateHelpers::getDate(startDate, "0d", slidingF, &calF, true);
				endDate = etrading::LADateHelpers::getDate(startDate, "3M", slidingF, &calF, true);
				int mm = endDate.monthOfYear();
				int yy = endDate.yearOfEra();
				endDate = etrading::LADateHelpers::getIMMDate(yy, mm, true);
				
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
				throw LACoreInvalidData("Future File format is wrong", __FILE__,__LINE__);
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
			refData += nameF + ":";
			// set name
			mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameF);
			// set term
			mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(term);
			// set calendar
			mktData->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calFStr);
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
			// set convexAdj
			mktData->add(PRICING_DATA_CONVEXADJUSTMENT, new LADataDouble(convexAdj));
			// set useConvexAdj
			mktData->add(PRICING_DATA_USECONVEXADJUSTMENT, new LADataBool(useConvexAdjustment));
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
		LAString msg = "swap file is empty";
		throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}

	// get cal and calc spot date
	LAPriceDataCalendar calS;
	LAString calSStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + staticDataSuffix);
	calS.convertFromString(calSStr);

	LADate spotDateS;
	int resetLag;
	if (isSpotUse)
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
			spotDateS = calS.getBusinessDay(asOfDate, resetLag);
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
	if (isResetSkipStr!=MLIB_NO_DATA)
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
	bool isSwapTenorAdjust = false;
	LAString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + staticDataSuffix).toUpper();
	if (strSwapTenorAdj != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(strSwapTenorAdj);
		isSwapTenorAdjust = tmpAttrB.get();
	}
	for (int j = 0; j < swapSize; ++j)
	{
		LAString term = swapDataMtx[j][0].toUpper();
		LAString tmpCurrency = currency;
		if (isAudExtra && tmpCurrency.toUpper() == CURRENCY_AUD &&
			term.findString("Y") == static_cast<int>(term.size() - 1) &&
				term.subString(0, term.size() - 2).getIntValue() > 3 && resetFlg)
		{
			// set yield curve pro
			if (marketName != SWAP)
			{
				basisCurveEngine.LAObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
				basisCurveEngine.LAObject::add(CALIBRATION_DATA_MARKETDATA + suffix_data, new LADataMultiReference()).
					convertFromString(refData.subString(0, refData.size() - 2));
			}
			else
			{
				basisCurveEngine.getMarketData().convertFromString(refData.subString(0, refData.size() - 2));
			}
			
			// generate yield data
			LADataProcedure &proc = dynamic_cast<LADataProcedure &>
								(basisCurveEngine.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
			basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(marketName));

			dataInstance.getReferencePool().completeDependency();
			proc.estimate(asOfDate);
			
			//reset market data as libor
			LACurveMarketDataHelpers::resetMarketDataUseL(basisCurveEngine, currency, &marketName);
			resetFlg = false;
			if (marketName != SWAP)
			{
				basisCurveEngine.LAObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
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

		if (isAudExtra && tmpCurrency.toUpper() == CURRENCY_AUD && resetFlg)
		{
			aud_origSwapRate[marketName].insert(make_pair(term, rate / 100.0));
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
		refData += nameS + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameS);
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateS));
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTLAG, new LADataInt(resetLag));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calSStr);
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
		if (isFwdSwap)
		{
			mktData->add(PRICING_DATA_ISFWDSWAP, new LADataBool(isFwdSwap) );
			if (swapDataMtx[j].size() != 5)
				throw LACoreInvalidData("FwdSwap File format is wrong", __FILE__,__LINE__);
			const bool isDate = swapDataMtx[j][2].toUpper() == "TRUE";
			mktData->add(PRICING_DATA_ISDATE, new LADataBool(isDate) );
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
		mktData->add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, new LADataBool(isSwapTenorAdjust) );
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
		// set frequency
		if ((!checkFrequency(freqSStr, term) || !checkFrequency(freqSStr_Fix, term)) && isUse)
				throw LACoreInvalidData("frequency fix and term of swap rate are inconsistent!!", __FILE__, __LINE__);
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freqSStr);
		mktData->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FIX, new LADataString()).convertFromString(freqSStr_Fix);
		// set base frequency
		if (baseFreqSStr_float != MLIB_NO_DATA)
		{
			if (!checkFrequency(baseFreqSStr_float, term) && isUse)
				throw LACoreInvalidData("base frequency and term of swap rate are inconsistent!!", __FILE__, __LINE__);
			mktData->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, new LADataString()).convertFromString(baseFreqSStr_float);
		}
		// set frequency of floating leg
		if (freqSStr_float != MLIB_NO_DATA)
		{
			if (!checkFrequency(freqSStr_float, term) && isUse)
				throw LACoreInvalidData("frequency float and term of swap rate are inconsistent!!", __FILE__, __LINE__);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, new LADataString()).convertFromString(freqSStr_float);
		}
		// set frequency of compounding
		if (freqSStr_cpd != MLIB_NO_DATA)
		{
			if (!checkFrequency(freqSStr_cpd, term) && isUse)
				throw LACoreInvalidData("frequency compound and term of swap rate are inconsistent!!", __FILE__, __LINE__);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_COMPOUND, new LADataString()).convertFromString(freqSStr_cpd);
		}
	}

	if (refData.size() < 2)
	{
        throw LACoreInvalidData("#Error: Missing Market Data. Market Data is not set !!", __FILE__, __LINE__); 
	}

	LAString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix);
	if (tmpAssignedCurves != MLIB_NO_DATA)
	{
		LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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
	const LAString isPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<LAString, bool>& gCurveMap = basisCurveEngine.getGCurveGenerateMap();
		const std::map<LAString, LAString>& assignedCurveMktMap = basisCurveEngine.getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
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
LAUpdateObjectPoolForCurves::setUpGenCurveDataOIS(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
									const LAString &marketName, const LAString &yieldDataName, bool isSpotUse,
									bool isAudExtra, BasisCurveCalibration &basisCurveEngine, 
									std::map<LAString, std::map<LAString, double> > &aud_origSwapRate) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();

	LAString suffix = "." +  marketName;
	suffix.toLower();

	LAString suffix_data = "_" +  marketName;

	LAString yeildGenInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
	if (yeildGenInter != MLIB_NO_DATA) 
	{
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data, new LAPriceDataInterpolation()).convertFromString(yeildGenInter);
	}

	LAString oisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffix);
	MAFileAccessor oisFile(LAMarketData::getNumFileName(oisFileName));
	LAStringMatrix oisDataMtx;
	oisFile.readAllData(MARKET_DATA_DELIMITER, oisDataMtx);

	oisFile.close();

	if (oisDataMtx.size() == 0 || oisDataMtx[0].size() < 2 )
	{
		throw LACoreInvalidData("Error: OIS Swap Data is missing. OisFile is empty", __FILE__,__LINE__);
	}

	// get cal and calc spot date
	LAPriceDataCalendar calOIS;
	LAString calOISStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_CALENDAR + suffix);
	calOIS.convertFromString(calOISStr);
	LADate spotDateOIS;
	if (isSpotUse)
	{
		spotDateOIS.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SPOTDATE + suffix).getCString());
	}
	else
	{
		spotDateOIS = calOIS.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_RESETLAG + suffix).getIntValue());
	}

	bool isEOMRollOIS = false;
	LAString strEOMRollOIS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_ISEOMRLL + suffix).toUpper();	
	if (strEOMRollOIS != MLIB_NO_DATA)	
	{	
		LADataBool tmpIsEOMRollOIS;		
		tmpIsEOMRollOIS.convertFromString(strEOMRollOIS);		
		isEOMRollOIS = tmpIsEOMRollOIS.get();		
	}			
	
	if (isEOMRollOIS)			
	{			
		LAString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_EOMDAY + suffix).toUpper();		
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
	LAString inputInterpJoinDateStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_INTERPOLATIONJOINDATE + suffix).toUpper();
	basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);

	yldEntity.remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	if (inputInterpJoinDateStr != MLIB_NO_DATA && inputInterpJoinDateStr.size() != 0)
	{
		//LADate inputLinearSplineJoinDate = etrading::validateAndConvertStringToDate(inputInterpJoinDateStr, "Invalid joinDate");
		LAString busDayAdj = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE + suffix).toUpper();
		LADate inputInterpolationJoinDate = etrading::validateDateOrTenor(spotDateOIS, inputInterpJoinDateStr, busDayAdj, calOISStr, "Invalid joinDate");

		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputInterpolationJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputInterpolationJoinDate));
	}
	
	// Various OIS control parameters
	LAString str_shortTerm =  mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERM + suffix);
	LADate shortTermDate;
	if (str_shortTerm != MLIB_NO_DATA)
	{
		shortTermDate = calOIS.getBusinessDay(asOfDate, str_shortTerm.getIntValue());
	}

	LAString shortTermConv			= mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION + suffix).toUpper();
	LAString firstRate				= mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FIRSTRATE + suffix).toUpper();

	LAString swapCompoundingMethod("");
	LAString compoundValue		    = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDMETHOD + suffix).toUpper();
	LAString compoundValueAlias     = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDINGMETHOD + suffix).toUpper();
	
    if (compoundValue != MLIB_NO_DATA && compoundValueAlias != MLIB_NO_DATA)
	{
		MLIB_REQUIRE( compoundValue == compoundValueAlias, "Invalid CompoundMethod: Cannot specify 'CompoundMethod' as '" + compoundValue + "' the alias for this parameter 'CompoundingMethod' as '" + compoundValueAlias + "'" )
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
    if ( swapCompoundingMethod != MLIB_NO_DATA )
    {
        swapCompoundingMethod.toUpper();
        MLIB_REQUIRE( swapCompoundingMethod == "ARITHMETIC" || swapCompoundingMethod == "GEOMETRIC" || swapCompoundingMethod == "NONE" || swapCompoundingMethod == "",
                      "Invalid CompoundingMethod - Input was '" + swapCompoundingMethod + "' but must be 'ARITHMETIC', 'GEOMETRIC' or 'NONE' " )
    }
    
    // Validate shortTermConv Generate Method Parameter by trying to cast to its enumerated type
    if ( shortTermConv != MLIB_NO_DATA )
    {
        shortTermConv.toUpper();
        MLIB_REQUIRE( shortTermConv == "ARITHMETIC" || shortTermConv == "ARITHMETICAVERAGE" || shortTermConv == "NONE" || shortTermConv == "",
                      "Invalid ShortTermConvention - Input was '" + shortTermConv + "' but must be 'ARITHMETICAVERAGE' or 'NONE'" )
    }

	const size_t oisSize = oisDataMtx.size();

	// Curve controls
	bool smoothShortEnd = false;
	 LAString strSmoothShortEnd = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SMOOTHSHORTEND + suffix);
	 if (strSmoothShortEnd != MLIB_NO_DATA)
	 {
		LADataBool tmpSmoothShortEnd;		
		tmpSmoothShortEnd.convertFromString(strSmoothShortEnd);		
		smoothShortEnd = tmpSmoothShortEnd.get();	
	 }

	 bool shortTermSwapOverrules = false;
	 LAString strShortTermSwapOverrules = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMSWAPOVERRULES + suffix);
	 if (strShortTermSwapOverrules != MLIB_NO_DATA)
	 {
		 LADataBool tmpShortTermSwapOverrules;
		 tmpShortTermSwapOverrules.convertFromString(strShortTermSwapOverrules);
		 shortTermSwapOverrules = tmpShortTermSwapOverrules.get();
	 }

	// use grid
	LAStringVector oisUseGrid;
	LAString tmpOISUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffix).toUpper();
	if (tmpOISUseGrid != MLIB_NO_DATA)
	{
		oisUseGrid = tmpOISUseGrid.toToken(':');
	}

	// doing dual bootstrapping?
	bool isDualBootstrapping = false;
	LAString strIsDualBootstrapping = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_ISDUALBOOTSTRAPPING + suffix);
	if (strIsDualBootstrapping != MLIB_NO_DATA)
	{
		LADataBool tmpIsDualBootstrapping;
		tmpIsDualBootstrapping.convertFromString(strIsDualBootstrapping);
		isDualBootstrapping = tmpIsDualBootstrapping.get();
	}

	LAString longTermConv = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffix).toUpper();
	LAString longTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffix).toUpper();
	LAString longTermGen = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD + suffix).toUpper();
	
	LADate date_lt;
	LAStringMatrix lobasisDataMtx, swapDataMtx;
	
    // Validate LongTerm Convention Parameter
    if ( longTermConv != MLIB_NO_DATA )
    {
        longTermConv.toUpper();
        MLIB_REQUIRE( longTermConv == "LOBASIS" || longTermConv == "NONE" || longTermConv == "",
                      "Invalid LongTermConvetion - Input was '" + longTermConv + "' but must be 'LOBASIS' to calibrate to Libor-OIS Instruments or 'NONE' to calibrate to OIS Outrights only" )
    }
    
    // Validate LongTerm Generate Method Parameter
    if ( longTermGen != MLIB_NO_DATA )
    {
        longTermGen.toUpper();
        MLIB_REQUIRE( longTermGen == "DAILYAVERAGING" || longTermGen == "NONE" || longTermGen == "", 
                      "Invalid LongTerm.GenerateMethod - Input was '" + longTermGen + "' but must be 'DAILYAVERAGING' or 'NONE'" )
    }

    if (longTermConv == "LOBASIS")
	{
		if (longTerm == MLIB_NO_DATA)
		{
            throw LACoreInvalidData("#Error: LongTerm parameter is needed, when the LongTermConvention 'LOBASIS' is specified.", __FILE__,__LINE__);
		}
		if (longTermGen == MLIB_NO_DATA)
		{
            // LongTermGenerate Methodolgy Defaults are managed within the in calcEffectiveOISRate method, see CurveCalibration.cpp
			longTermGen = LAString("NONE");
		}
		date_lt = etrading::LADateHelpers::getDate(asOfDate, longTerm, true);
		LAString lobasisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + suffix);
		MAFileAccessor lobasisFile(LAMarketData::getNumFileName(lobasisFileName));
		lobasisFile.readAllData(MARKET_DATA_DELIMITER, lobasisDataMtx);
		lobasisFile.close();
		if (lobasisDataMtx.size() == 0 || lobasisDataMtx[0].size() < 2 )
		{
            throw LACoreInvalidData("#Error: Missing Libor-OIS Basis Market Data. LOBasisFile is empty", __FILE__,__LINE__);
		}
		LAString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix);
		if (swapFileName != MLIB_NO_DATA)
		{
			MAFileAccessor swapFile(LAMarketData::getNumFileName(swapFileName));
			swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
			swapFile.close();
			if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2)
			{
				if (!isDualBootstrapping)
				{
					throw LACoreInvalidData("#Error: Missing Libor-OIS Swap Market Data. SwapFile is empty", __FILE__, __LINE__);
				}
			}

			if (swapDataMtx.size() != lobasisDataMtx.size())
			{
				if (!isDualBootstrapping)
				{
					throw LACoreInvalidData("#Error: Not the same number of LOBasis spreads and Libor swaps are provided ", __FILE__, __LINE__);
				}
			}
		}
	}

	//get constant for convergence
	double epsilon = 1.0e-9;
	LAString strEPS = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_EPSILON + suffix);
	if (strEPS.toUpper() != MLIB_NO_DATA)
	{
		epsilon = strEPS.getDoubleValue();
	}
	int maxLoop = 1000;
	LAString strMLP = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_MAXLOOP + suffix);
	if (strMLP.toUpper() != MLIB_NO_DATA)
	{
		maxLoop = strMLP.getIntValue();
	}

	// Consolidate the outright OIS swap instruments and basis swap + libor swap instruments
	size_t extraBasisCount = 0;
	size_t longTermStartIndex = 0;
	if (longTermConv == "LOBASIS")
	{
		LAString lastOisTerm = oisDataMtx[oisSize-1][0].toUpper();

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

	// get UseConvexAdjustment for future section
	bool useConvexAdjustment = false;
	std::string useConvexAdjustmentStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USECONVEXADJUSTMENT + suffix).toUpper().getCString();
	if (useConvexAdjustmentStr != MLIB_NO_DATA)
	{
		useConvexAdjustment = (useConvexAdjustmentStr == "TRUE");
	}

	// get meanReversion for future section
	double meanReversion = 0.0;
	std::string meanReversion_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_MEANREVERSION + suffix).toUpper().getCString();
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
		LAString nameOIS = yieldDataName + "_OIS_" + LAString( (int)i ) + "_" + marketName;
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
		if (refData.findString(nameOIS) < 0)
		{
			refData += nameOIS + ":";
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

        // Allow Central Bank Swap instruments to be specified in the OIS curve for all markets ...
        if (etrading::isCentralBankSwap(term))     
		{
			if (oisDataMtx[i].size() != 4)
			{
                throw LACoreInvalidData("#Error: OIS Market Data must contain 4 columns. OIS Market Data column size is incorrect.", __FILE__,__LINE__);
			}
			// boj type
			// ** Currently all Central Bank swaps are labelled as BOJ regardless of currency. **
			mktData->add(IR_CALIBRATION_DATA_DATATYPE,   new LADataString()).convertFromString(YIELD_TYPE_BOJ);
			LADate startDate = LADataDate(oisDataMtx[i][2]).get();
			LADate endDate = LADataDate(oisDataMtx[i][3]).get();
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));

			//When the startDate is in the past and endDate is in the future, use Historical data in the FixingTable
			if (startDate < asOfDate && endDate > asOfDate)
			{
				LAString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix);
				etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
			}

		}
		else if (etrading::isARRFuture(term))
		{
			const int NUM_COLUMNS_FUTURE = 5; // Term, Rate, StartDate, EndDate, VolOrConvAdj
			if (oisDataMtx[i].size() != NUM_COLUMNS_FUTURE)
			{
				throw LACoreInvalidData("#Error: OIS Market Data must contain 4 columns. OIS Market Data column size is incorrect.", __FILE__, __LINE__);
			}

			// ARR Future type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_ARR_FUTURE);

			// set startDate, endDate
			LADate startDate = etrading::validateAndConvertStringToDate(oisDataMtx[i][2]);
			LADate endDate = etrading::validateAndConvertStringToDate(oisDataMtx[i][3]);
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));

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
			mktData->add(PRICING_DATA_USECONVEXADJUSTMENT, new LADataBool(useConvexAdjustment));
			// set convexAdj
			mktData->add(PRICING_DATA_CONVEXADJUSTMENT, new LADataDouble(convexAdj));
			// set vol
			mktData->add(PRICING_DATA_FUTUREVOLATILITY, new LADataDouble(futureVol));

			//When the startDate is in the past and endDate is in the future, use Historical data in the FixingTable
			if (startDate < asOfDate && endDate > asOfDate)
			{
				LAString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix);
				etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
			}
		}
		else
		{
			// swap type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_PAR);
		}				

		// get freq
		LAString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY, suffix, term).toUpper();
		// get daycount
		LAString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT, suffix, term).toUpper();
		// get sliding
		LAString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE, suffix, term).toUpper();
		// get swap type
		LAString swapType("OIS");
		LAString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SWAPTYPE + suffix).toUpper();
		if (strSwapType != MLIB_NO_DATA)
		{
			swapType = strSwapType;
		}
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateOIS));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calOISStr);
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
			rate = 1.0 - rate/100.0;

			double convexAdj = etrading::getFutureConvexityAdjustment(mktData, asOfDate, meanReversion);

			// 2) add convexity adjustment to the future rate
			rate -= convexAdj;
			
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
                MLIB_THROW( "Unable to imply OIS Outright Swaps from Libor-OIS Basis Instruments: Missing Libor-OIS Basis Swap with tenor " + term + ". Note: Overlapping OIS and Libor-OIS instruments must have identical tenors." );
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
                    MLIB_THROW( "Unable to imply OIS Outright Swaps from Libor-OIS Basis Instruments: Missing Libor Swap with tenor'" + term + "' to match the corresponding Libor-OIS Basis Swap" );
				}
			}

			// get lobasis data
			double rate_lo = lobasisDataMtx[currentLOBasisInstrument][1].getDoubleValue();
			LAString calLOStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_CALENDAR + suffix);
			LAString freqLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY, suffix, term).toUpper();
			LAString daycLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT, suffix, term).toUpper();
			LAString slidingLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE, suffix, term).toUpper();
			
			// get swap data			
			LAString calSwapStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + suffix);
			LAString freqSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFIX, suffix, term).toUpper();
			LAString daycSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFIX, suffix, term).toUpper();
			LAString slidingSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE, suffix, term).toUpper();
			LAString swapTenor("");
			LAString strSwapTenor = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTENOR + suffix).toUpper();
			if (strSwapTenor != MLIB_NO_DATA)
			{
				swapTenor = strSwapTenor;
			}
			LAString swapType("LIBOR");
			LAString strSwapType = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SWAPTYPE + suffix).toUpper();
			if (strSwapType != MLIB_NO_DATA)
			{
				swapType = strSwapType;
			}
			
			// set long term rate convention
			mktData->add(IR_CALIBRATION_DATA_LONGTERMCONVENTION,	new LADataString()).convertFromString(longTermConv);
			mktData->add(IR_CALIBRATION_DATA_LONGTERM,			new LADataString()).convertFromString(longTerm);
			mktData->add(IR_CALIBRATION_DATA_LONGTERMGENMETHOD,	new LADataString()).convertFromString(longTermGen);
			
			// set lobasis data
			mktData->add(IR_CALIBRATION_DATA_RATE_LOBASIS,		new LADataDouble(rate_lo / 100.0));
			if (calLOStr == MLIB_NO_DATA) calLOStr = calOISStr;
			mktData->add(IR_CALIBRATION_DATA_CALENDAR_LOBASIS ,	new LAPriceDataCalendar()).convertFromString(calLOStr);
			if (daycLOStr == MLIB_NO_DATA) daycLOStr = daycOISStr;
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_LOBASIS,	new LAPriceDataDayCount()).convertFromString(daycLOStr);
			if (slidingLOStr == MLIB_NO_DATA) slidingLOStr = slidingOISStr;
			mktData->add(IR_CALIBRATION_DATA_SLIDINGRULE_LOBASIS, new LAPriceDataSlidingRule()).convertFromString(slidingLOStr);
			if (freqLOStr == MLIB_NO_DATA) freqLOStr = freqOISStr;
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_LOBASIS,	new LADataString()).convertFromString(freqLOStr);	
			
			// set swap data
			if (isSwapRateAvailable)
			{
				double rate_s = swapDataMtx[currentSwapInstrument][1].getDoubleValue();
				mktData->add(IR_CALIBRATION_DATA_RATE_SWAP,		new LADataDouble(rate_s / 100.0));
			}
			mktData->add(IR_CALIBRATION_DATA_CALENDAR_SWAP ,	new LAPriceDataCalendar()).convertFromString(calSwapStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT_SWAP,	new LAPriceDataDayCount()).convertFromString(daycSwapStr);
			mktData->add(IR_CALIBRATION_DATA_SLIDINGRULE_SWAP,new LAPriceDataSlidingRule()).convertFromString(slidingSwapStr);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_SWAP,	new LADataString()).convertFromString(freqSwapStr);	
			
			mktData->remove(IR_CALIBRATION_DATA_REFSWAPTENOR);
			mktData->remove(IR_CALIBRATION_DATA_REFSWAPTYPE);
			mktData->add(IR_CALIBRATION_DATA_REFSWAPTENOR,	new LADataString()).convertFromString(swapTenor);	
			mktData->add(IR_CALIBRATION_DATA_REFSWAPTYPE,		new LADataString()).convertFromString(swapType);	
		}
	}

	LAStringMatrix fedFundFutureDataMtx;
	LAString fedFundFutureFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + suffix);
	if (fedFundFutureFileName != MLIB_NO_DATA)
	{
		MAFileAccessor fedFundFutureFile(LAMarketData::getNumFileName(fedFundFutureFileName));	
		fedFundFutureFile.readAllData(MARKET_DATA_DELIMITER, fedFundFutureDataMtx);
		fedFundFutureFile.close();
	}

	const int fedFundFutureSize = fedFundFutureDataMtx.size();
	// use grid
	LAStringVector ffFutureUseGrid;
	LAString tmpFFFutureUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID + suffix).toUpper();	
	if (tmpFFFutureUseGrid != MLIB_NO_DATA)
	{
		ffFutureUseGrid = tmpFFFutureUseGrid.toToken(':');
	}
	else
	{
		LAString tmpUseGridNum = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRIDNUM + suffix).toUpper();
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
		if (refData.findString(nameOIS) < 0)
		{
			refData += nameOIS + ":";
		}
		
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameOIS);
		
		LADate startDate, endDate;
		LAString term = fedFundFutureDataMtx[i][0].toUpper();

		if (fedFundFutureDataMtx[i].size() < 2)
		{
            throw LACoreInvalidData("#Error: FF Futures data cannot contain more than 2 columns. FF Future File format is wrong", __FILE__,__LINE__);
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
			DateVector ffdates = etrading::LADateHelpers::getFFDatesFromTerm(asOfDate,term);
			if (ffdates.size() != 2)
                throw LACoreInvalidData("#Error: FF Dates Data must contain 2 columns. FF dates error",__FILE__,__LINE__);

			startDate = ffdates[0];
			endDate = ffdates[1];
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
		}
		
		double rate = fedFundFutureDataMtx[i][1].getDoubleValue();
		rate  = 100. - rate;

		// get freq
		LAString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY, suffix, term).toUpper();
		// get daycount
		LAString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT, suffix, term).toUpper();
		// get sliding
		LAString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE, suffix, term).toUpper();
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateOIS));
		// set calendar
		mktData->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calOISStr);
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
			LAString oisHistFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix);

			etrading::populateHistoricalDataToMarketData(mktData, oisHistFileName);
		}

	}

	if (refData.size() < 2)
	{
		throw LACoreInvalidData("Market Data is not set !!", __FILE__, __LINE__); 
	}

	//DF curve name
	LAString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffix); 
	if (dfCurveName == MLIB_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName);
	basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName, new LADataString(dfCurveName));
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName, new LADataString(dfCurveName));

	//const std::map<LAString, LAString>& assignedCurveMktMap = basisCurveEngine->getAssignedCurveMktMap();
	LAString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix);
	if (tmpAssignedCurves != MLIB_NO_DATA)
	{
		LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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
	const LAString isPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<LAString, bool>& gCurveMap = basisCurveEngine.getGCurveGenerateMap();
		const std::map<LAString, LAString>& assignedCurveMktMap = basisCurveEngine.getAssignedCurveMktMap();
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
LAUpdateObjectPoolForCurves::setUpFloater(const LAString &currency, BasisCurveCalibration &basisCurveEngine, const LAString &genFloaterName) const
{
	LAStringVector markets = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	uppervec(markets);
	if (genFloaterName != MLIB_NO_DATA)
	{
		basisCurveEngine.setAssignedCurveMktMap(genFloaterName, genFloaterName);
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_FLOATERDFS);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_FLOATERDFS, new LADataString(genFloaterName));
		LAString tmpGenFloaterName = genFloaterName;
		tmpGenFloaterName.toLower();
		LAString basisMkt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME);
		if (basisMkt != MLIB_NO_DATA)
		{
			if (std::find(markets.begin(), markets.end(), basisMkt) == markets.end())
			{
				throw LACoreInvalidData("Basis market does not exist!", __FILE__, __LINE__);
			}
			basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_BASISDATA + LAString("_") + tmpGenFloaterName);
			basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_BASISDATA + LAString("_") + tmpGenFloaterName, new LADataString(basisMkt));
		}
		LAString discountName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT);
		LAString forecastName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FORECAST);
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_FORECAST + LAString("_") + tmpGenFloaterName);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_FORECAST + LAString("_") + tmpGenFloaterName, new LADataString(forecastName));
		basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_DISCOUNT + LAString("_") + tmpGenFloaterName);
		basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_DISCOUNT + LAString("_") + tmpGenFloaterName, new LADataString(discountName));

		/*LAString isFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_ISFWDINTER).toUpper();
		if (isFWDInter == "TRUE")
		{
			basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + LAString("_") + tmpGenFloaterName);
			basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + LAString("_") + tmpGenFloaterName, new LADataBool(true));
			LAString fwdInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FWDINTERPOLATION).toLower();
			basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_FWDINTERPOLATION + LAString("_") + tmpGenFloaterName);
			basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_FWDINTERPOLATION + LAString("_") + tmpGenFloaterName, new LAPriceDataInterpolation()).convertFromString(fwdInter);;
		}*/
		
		basisCurveEngine.setFloater(genFloaterName);
	}
}

void
LAUpdateObjectPoolForCurves::setUpCurveDataByReadFile( LADataInstance &dataInstance, const LADate& asOfDate, const LAString& currency, 
										    const LAString& marketName, const LAString& yieldDataName, BasisCurveCalibration &basisCurveEngine ) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAObjectHolder objHolder = objPool.getObject(yieldDataName, ENCHKTYPE_NOCHECK );
	if (!objHolder.isDefined() )
		throw LACoreInvalidData("yield Object is not set! LAUpdateObjectPoolForCurves::setUpCurveDataByReadFile", __FILE__, __LINE__ );
	
	LAObject &eData = objHolder.get();

	LAString tmpMktName = marketName;
	LAString suffix = "." + tmpMktName.toLower();

	LAString dfFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_DF_FILE + suffix );
	MAFileAccessor dfFile( LAMarketData::getNumFileName(dfFileName));
	LAStringMatrix dfDataMtx;
	dfFile.readAllData(MARKET_DATA_DELIMITER, dfDataMtx);
	dfFile.close();

	if (dfDataMtx.size() == 0 || dfDataMtx[0].size() < 2 )
		throw LACoreInvalidData("dfFile is empty", __FILE__,__LINE__);

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
	
	LAString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ZERORATE_ASSIGNEDCURVE + suffix);
	if (tmpAssignedCurves != MLIB_NO_DATA)
	{
		LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
		for (size_t i = 0; i<assignedCurves.size(); i++)
		{
			basisCurveEngine.setAssignedCurveMktMap(assignedCurves[i],marketName);

			LAString modCurveName = "_" + assignedCurves[i].toUpper();

			eData.remove(CALIBRATION_DATA_TERMS + modCurveName);
			eData.add(CALIBRATION_DATA_TERMS + modCurveName, new LADataDoubles(terms));
			
			eData.remove(IR_CALIBRATION_DATA_DFS + modCurveName);
			eData.add(IR_CALIBRATION_DATA_DFS + modCurveName, new LADataDoubles(dfs));

			eData.remove(IR_CALIBRATION_DATA_ACCESSARY + modCurveName);
			eData.add(IR_CALIBRATION_DATA_ACCESSARY + modCurveName, new LADataString("DF"));

			if (!dfs2.empty())
			{
				eData.remove( IR_CALIBRATION_DATA_DFS2 + modCurveName); 
				eData.add( IR_CALIBRATION_DATA_DFS2 + modCurveName, new LADataDoubles(dfs2));
			}
		}
	}
	else
	{
		LAString modCurveName = "_" + tmpMktName.toUpper();

		eData.remove(CALIBRATION_DATA_TERMS + modCurveName);
		eData.add(CALIBRATION_DATA_TERMS + modCurveName, new LADataDoubles(terms));
		
		eData.remove(IR_CALIBRATION_DATA_DFS + modCurveName);
		eData.add(IR_CALIBRATION_DATA_DFS + modCurveName, new LADataDoubles(dfs));

		eData.remove(IR_CALIBRATION_DATA_ACCESSARY + modCurveName);
		eData.add(IR_CALIBRATION_DATA_ACCESSARY + modCurveName, new LADataString("DF"));

		if (!dfs2.empty())
		{
			eData.remove( IR_CALIBRATION_DATA_DFS2 + modCurveName); 
			eData.add( IR_CALIBRATION_DATA_DFS2 + modCurveName, new LADataDoubles(dfs2));
		}
		basisCurveEngine.setAssignedCurveMktMap(marketName,marketName);
	}

	basisCurveEngine.insertNonRemovableMarket(marketName);
}

void 
LAUpdateObjectPoolForCurves::dataoutCurve(const LAStringVector &curveNames, LAObject &eData, const LAString &yieldDataName) const
{
	for(unsigned int i = 0; i < curveNames.size(); i++)
	{
		LAString curveSuffix;
		if(curveNames[i] == STD) curveSuffix = "";
		else curveSuffix = "_" + curveNames[i];
		LAString curveSuffix_file = curveSuffix;
		while(curveSuffix_file.findString("/") != -1)
		{
			curveSuffix_file.remove(curveSuffix_file.findString("/"),1);
		}
		const LAString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM);
		const LAString dirName = LACoreDataService::getOutputDirectory(); 
		const LAString fileName  = dirName + yieldDataName + curveSuffix_file + fileSuffix + ".csv";

		ifstream fin;
		ofstream fout;
		fin.open(fileName.getCString());

		if (!fin)
		{
			LADataHolder* dh = &eData.getData(CALIBRATION_DATA_TERMS + curveSuffix, NOCHECK);
			if (!dh->isDefined() || dh->isNull())
				continue;

			const DoubleArray &terms = 
				dynamic_cast<const LADataDoubles &>(eData.getData(CALIBRATION_DATA_TERMS + curveSuffix, ISNOTNULL).get()).get();
			const DoubleArray &dfs = 
				dynamic_cast<const LADataDoubles &>(eData.getData(IR_CALIBRATION_DATA_DFS + curveSuffix, ISNOTNULL).get()).get();

			int size = terms.size();
			if (size != static_cast<int>(dfs.size()))
			{
				throw LACoreInvalidData("Term size and df size must be same !!", __FILE__, __LINE__);
			}
			fout.open(fileName.getCString());

			const LADataHolder &dfsH2 = eData.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix, NOCHECK);
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
			dynamic_cast<LADataDoubles &>(eData.getData(CALIBRATION_DATA_TERMS + curveSuffix, ISNOTNULL).get()).set(terms);
			dynamic_cast<LADataDoubles &>(eData.getData(IR_CALIBRATION_DATA_DFS + curveSuffix, ISNOTNULL).get()).set(dfs);
			if (!dfs2.empty() && eData.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix).isDefined())
			{
				dynamic_cast<LADataDoubles &>(eData.getData(IR_CALIBRATION_DATA_DFS2 + curveSuffix, ISNOTNULL).get()).set(dfs2);
			}
		}
	}
}



bool 
LAUpdateObjectPoolForCurves::checkFrequency(const LAString& freq, const LAString& mktRateTerm) const
{
	int span = etrading::LADateHelpers::getOnePeriodOfFrequency(freq);

	int y, m, d, w;
    etrading::LADateHelpers::termStrtoYMDW(mktRateTerm, y, m, d, w);
	int moth_mkt_term = 12 * y + m;

	return (moth_mkt_term % span) == 0;
}

///// update for XLL Plus //////////////////////////
#include "LAUpdateObjectPoolForCurvesAndModels.h"

void
LAUpdateObjectPoolForCurves::generateInitialValueForPricer(const LAString &currency, LADataInstance &dataInstance) const
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
	
	LAString yieldProName = "PRO_" + yieldName;
	BasisCurveCalibration *basisCurveEngine = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(yieldProName);
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

	LAObject *eData = NULL;
	LAString yieldDataName = yieldName + "_DATA";
	const LAObjectHolder ehdata = objPool.getObject(yieldDataName);
	if (!ehdata.isDefined())
	{
		eData = new LAObject();
		objPool.set(yieldDataName, eData);
	}
	else
	{
		//we must not erase the reset method for only edata
		eData = &objPool.getObject(yieldDataName).get();
	}
	eData->remove(CALIBRATION_DATA_NAME);
	eData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);

	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	bool isAudExtra = false;
	bool isSwapTenorAdjust = false;
	bool isSpotUse = false;
	
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *basisCurveEngine, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, false);
	setUpCurveDataByContext(*basisCurveEngine,eData,currency,SWAP);


	LAStringVector markets;
	LAString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == MLIB_NO_DATA)
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEDFS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	else
	{
		markets = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	}
	uppervec(markets);
	if (!markets.empty() && markets[0] != MLIB_NO_DATA)
	{
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			LAString suffix = markets[i];
			suffix.toLower();
			LAString contextKey = tmpCurrency+CONTEXT_KEY_SDE_YIELD_WITH_MARKET+suffix;
			LAString contextWithMarket = LACoreDataService::getContext(contextKey);
			if (contextWithMarket!=MLIB_NO_DATA)
			{
				LAString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + suffix).toUpper();
				if (marketType==MARKETTYPE_BASIS)
				{
					LAString useYieldSDEIRStr = LACoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
					LACoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD,MLIB_NO_DATA);
					LAString tmpCurveName = markets[i];
					tmpCurveName.toUpper();
					mpStaticData->setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
					UpdateObjectPoolForSDEsAndCurves generator(currency);
					generator.generateSDEMarketData(currency, dataInstance, true);
					mpStaticData->removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
					LACoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD,useYieldSDEIRStr);
				}
			}
			setUpCurveDataByContext(*basisCurveEngine,eData,currency,markets[i]);
		}
	}
}

void 
LAUpdateObjectPoolForCurves::setUpCurveDataByContext(BasisCurveCalibration &basisCurveEngine, LAObject *eData, const LAString& currency, const LAString& marketName ) const
{
	LAString prefix = currency;
	prefix.toLower();
	LAString suffix;
	LAString data_suffix;
	if (marketName!=SWAP)
	{
		suffix = marketName;
		suffix.toLower();
		data_suffix = "_" + marketName;
	}

	LAString contextKey = prefix+CONTEXT_KEY_SDE_YIELD+suffix;
	LAString contextYield = LACoreDataService::getContext(contextKey);
	if (contextYield==MLIB_NO_DATA)
	{
		LAString msg = "context data for generated dfs don't exist.";
		msg += "(" + contextKey + ")";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	LADataDoubleMatrix matrix;
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
	eData->add(CALIBRATION_DATA_TERMS + data_suffix, new LADataDoubles(terms));

	eData->remove(IR_CALIBRATION_DATA_DFS + data_suffix);
	eData->add(IR_CALIBRATION_DATA_DFS + data_suffix, new LADataDoubles(dfs));

	if (isDF2)
	{
		LADataDoubles* attrDF2 = NULL;
		LADataHolder* df2H = &eData->getData(IR_CALIBRATION_DATA_DFS2 + data_suffix);
		if (!df2H->isDefined())
		{
			attrDF2 = new LADataDoubles();
			eData->remove(IR_CALIBRATION_DATA_DFS2 + data_suffix);
			eData->add(IR_CALIBRATION_DATA_DFS2 + data_suffix,attrDF2);
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
		LADataHolder* dh = &(eData->getData(CALIBRATION_DATA_TERMS, ISDEFINED));
		const DoubleVector& baseterms = dynamic_cast<const LADataDoubles &>(dh->get()).get();

		DoubleVector calcdf2(baseterms.size() ,1.0);
		LASplineInterpolation attrspline;
		attrspline.set(terms, dfs);
		for (unsigned int i = 0; i < baseterms.size(); i++)
			calcdf2[i] = attrspline.value(baseterms[i]);

		eData->remove(IR_CALIBRATION_DATA_DFS2);
		eData->add(IR_CALIBRATION_DATA_DFS2, new LADataDoubles(calcdf2));
	}



	// set curve name to yield curve pro
	LAStringVector gCurveNames;
	LADataHolder *dh = &(basisCurveEngine.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		gCurveNames = dynamic_cast<const LADataStrings &>(dh->get()).get();
	}
	gCurveNames.push_back(marketName);
	basisCurveEngine.LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	basisCurveEngine.LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(gCurveNames));
}
////////////////////////////////////////////////////

