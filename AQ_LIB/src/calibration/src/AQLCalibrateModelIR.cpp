#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <fstream>
#include "AQLTime.h"
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLPriceDataManager.h"
#include "AQLDataReference.h"
#include "AQLDataMatrix.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLRatesCurveLinearInterpolation.h"
#include "AQLStaticData.h"
#include "AQLDealUtils.h"
#include "AQLCalibrateModelIR.h"
#include "AQLDataVector.h"
#include "AQLDataProcedure.h"
#include "AQLMathYieldCurve.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLCompoundingFunc.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"
#include "AQLMarketData.h"
#include "AQLPriceArbFreeGenerator.h"
#include "AQLFunctionUtilities.h"
#include "AQLDateCalculations.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS 

#include "AQLCoreDataService.h"
#include "AQLRatesTermStructureSDE.h"
#include "AQLRatesLJTermStructureSDE.h"
#include "AQLMathCorrelation.h"
#include <time.h>

#endif

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
/*!

*/
AQLCalibrateModelIR::AQLCalibrateModelIR(const AQLString &baseCurrency)
: AQLCalibrateModel(), mBaseCurrency(baseCurrency)
{
}

// destructor
/*!

*/
AQLCalibrateModelIR::~AQLCalibrateModelIR(void)
{
}


#ifndef VISUAL_STUDIO_2010_ANALYTICS 
// 
/*!
    @brief set setInterpolationMethod

	@param[in] currency
	@param[out] sde
*/
void
AQLCalibrateModelIR::setInterpolationMethod(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	(void)currency;
	sde.setInterpolationMethod(new AQLRatesCurveLinearInterpolation());
}
#endif 
// 
/*!
    @brief generate sde market data

	IR is set initialvalue correlation volatility

	@param[in] currency 
	@param[in] dataInstance
*/
void
AQLCalibrateModelIR::loadModelDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, const bool isCurve, const bool isModel, const AQLString & curveID, const AQLString & marketName) const
{
	clock_t cstart, cend;
	double time;
	if (isCurve)
	{
	// generate initial curve
cout << "[ Currency = " << currency << " ]" << endl;
cout << "AQLCalibrateModelIR generate curve called.." << endl;
cstart = clock();
		loadYieldCurveDataAndCalibrate(currency, dataInstance);
cend = clock();
time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "AQLCalibrateModelIR generate curve end.." << endl;
cout << "-> time = " << time << endl;
	}

///// update for XLL Plus //////////////////////////
	//if (isOnlyCurve) return;
////////////////////////////////////////////////////
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	if (isModel)
	{
	// generate correlation
cout << "AQLCalibrateModelIR generate correlation called.." << endl;
cstart = clock();
		loadCorrelationDataAndCalibrate(currency, dataInstance);
cend = clock();
time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "AQLCalibrateModelIR generate correlation end.." << endl;
cout << "-> time = " << time << endl;
	// generate volatility
cout << "AQLCalibrateModelIR generate volatility called.." << endl;
cstart = clock();
		loadVolatilityDataAndCalibrate(currency, dataInstance);
cend = clock();
time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "AQLCalibrateModelIR generate volatility end.." << endl;
cout << "-> time = " << time << endl;
	}
#endif
}

// 
/*!
    @brief generate sde initial curve data for fwdfx constant

	@param[in/out] dataInstance
*/
void
AQLCalibrateModelIR::loadFwdFXConstCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant) const
{
	if (!isFwdFXConst(currency))
		return;
	AQLString ccy = currency; ccy.toLower();

///// update for XLL Plus in grid //////////////////
	AQLString useYieldSDEIRStr = AQLCoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
	if (useYieldSDEIRStr.toUpper() == "TRUE")
	{
		if (!mCurveGenCcyMap[currency])
		{
			generateInitialValueForPricer(currency, dataInstance);
			mCurveGenCcyMap[currency] = true;
		}
		return;
	}
////////////////////////////////////////////////////

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
	AQLString suffix = "." + market.toLower();
	AQLString target = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_TARGET + suffix).toUpper();
	if (target == LEG1FORECAST || target == LEG2FORECAST)
	{
		throw AQLCoreInvalidData("fwdfx constant curve must be discount curve!", __FILE__, __LINE__);
	}
	else if (target == LEG1DISCOUNT)
	{
		strFCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + suffix);
		strDCurve =  mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + suffix);
		strA_fCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + suffix);
		strA_dCurve =  mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + suffix);
	}
	else if (target == LEG2DISCOUNT)
	{
		strFCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + suffix);
		strDCurve =  mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT + suffix);
		strA_fCurve = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + suffix);
		strA_dCurve =  mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT + suffix);
	}
	else
	{
		throw AQLCoreInvalidData("target curve is invalid!", __FILE__, __LINE__);
	}
	convertCurveName(strFCurve, ccy, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(strDCurve, ccy, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(strA_fCurve, ccy, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(strA_dCurve, ccy, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	if (ccy_dCurve != ccy || ccy_fCurve != ccy || ccy_a_dCurve != ccy_a_fCurve)
		throw AQLCoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);

	const AQLString &ycProName = AQLMarketData::getBaseYieldProName(ccy);
	AQLMathYieldCurvePro &ycPro = dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(ycProName).get());

	AQLObjectHolder& yData = ycPro.getYieldData().get();
	AQLString ydName = yData.getName();
	if (isSetCurveID)
		ydName = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATECURVEID);

	if (!isPricer && mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_a_fCurve.toUpper())) 
	{
		// generate against leg ccy
		loadYieldCurveDataAndCalibrate(ccy_a_fCurve, dataInstance, isCalcFwdBeforeFwdFXConsant);
	}

	// save colateral ccy and curve
	AQLMathYieldCurvePro* colYCPro = NULL;
	if (!isPricer)
	{
		const AQLString &colYCProName = AQLMarketData::getBaseYieldProName(ccy_a_fCurve);
		colYCPro = &(dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(colYCProName).get()));
		colYCPro->setColAffectingCcy(ccy.toUpper());
		ycPro.setColAffectedCcy(ccy_a_fCurve.toUpper());
	}
	const AQLString &fYCName = AQLMarketData::getBaseYieldName(ccy_a_fCurve);
	AQLMathYieldCurve& fYC = dynamic_cast<AQLMathYieldCurve &>(objPool.getObject(fYCName).get());
	const AQLString &fYDName = fYC.getYieldData().get().getName();
	ycPro.getColYieldData().convertFromString(fYDName);

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

	ycPro.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + market);
	ycPro.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + market, new AQLDataMultiReference()).convertFromString(nameB);

	// save assigned curves
	AQLStringVector assignedCurves;
	AQLString strAssignedCurves = mpStaticData->getStaticData(ccy.toLower() + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffix);
	if (strAssignedCurves != AQ_NO_DATA)
	{
		assignedCurves = strAssignedCurves.toToken(MULTI_STATIC_DATA_DELIMITER);
		for (int i = 0; i<assignedCurves.size(); ++i)
			ycPro.setAssignedCurveMktMap(assignedCurves[i], market);
	}
	else
	{
		assignedCurves.push_back(market);
		ycPro.setAssignedCurveMktMap(market, market);
	}

	if (!isCalcFwdBeforeFwdFXConsant)
	{
		// calc curve
		ycPro.calcFwdFXConstantCurve();
		
		// dataout
		if (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
			dataoutCurve(assignedCurves, yData.get(), ydName);
	}
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
AQLCalibrateModelIR::convertCurveName(const AQLString &propCurve, const AQLString &ccy, AQLString &curveCcy, AQLString &curveName, const bool isPricer, AQLObjectPool& objPool) const
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


// 
/*!
    @brief generate sde initial curve data

	@param[in] currency  currency or fx ex.JPY/USD
	@param[out] dataInstance
*/
void
AQLCalibrateModelIR::loadYieldCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant) const
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
	AQLMathYieldCurvePro *ycPro = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		ycPro = new AQLMathYieldCurvePro(&dataInstance);
		objPool.set(yieldProName, ycPro);
	}
	else
	{
		//we must not erase the reset method for only ycpro
		ycPro = &dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(yieldProName).get());
	}
	ycPro->getName().convertFromString(yieldProName);

	ycPro->getIsArbFree().set(false);

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
	
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *ycPro, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, isArbFree);

	bool isXccyMarkedToMarket = false;
	AQLString isXccyMarkedToMarketString = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST,
														              tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISXCCYMARKEDTOMARKET).toUpper(); // Alias Method: First parameter takes priority
	if (isXccyMarkedToMarketString == "TRUE")
	{
		isXccyMarkedToMarket = true;
	}
	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
	ycPro->AQLObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new AQLDataBool(isXccyMarkedToMarket));

	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET);
	ycPro->AQLObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new AQLDataBool(isXccyMarkedToMarket));

	AQLString fxName = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != AQ_NO_DATA)
		ycPro->getFXEntity().convertFromString(fxName);

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

	bool enableCalculation = true;
	AQLString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET).toUpper();
	if (target != AQ_NO_DATA) enableCalculation = false;

	if (enableCalculation || target == STD) setUpGenCurveData(dataInstance, refData, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
			ycPro->getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(baseYieldName);
		}
	}
#endif

	// set yield curve pro
	if (refData.size())
	{
		refData = refData.subString(0, refData.size() - 2);
		ycPro->getMarketData().convertFromString(refData);
	}

	AQLDataHolder* dh;
	AQLStringVector swapCurves;
	dh = &ycPro->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) swapCurves = dynamic_cast<AQLDataStrings &>(dh->get()).get();
			
	AQLStringVector basisCurves;
	dh = &ycPro->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) basisCurves = dynamic_cast<AQLDataStrings &>(dh->get()).get();

	AQLString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	AQLStringVector genBasisSwapMarket;

	AQLString OISName;
	if (!markets.empty() && markets[0] != AQ_NO_DATA)
	{
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
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
							ycPro->setAssignedCurveMktMap(STD,SWAP);
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
			AQLString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_INTERPOLATION + suffix).toUpper();
			if (strInter != AQ_NO_DATA)
			{
				yc->getInterpolation(markets[i]).convertFromString(strInter.toLower());
				ycPro->getInterpolation(markets[i]).convertFromString(strInter);
				if (mainBDF == markets[i])
				{
					yc->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
					ycPro->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
				}
			}
	
			if ( marketType == MARKETTYPE_BASIS )
			{
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
						setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, tmpMktNames[1], yieldDataName, isSpotUse, *ycPro, &tmpMktNames[0]);
					}
					else
					{
						AQLString* pMktCurrency = 0;
						setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *ycPro, pMktCurrency , isCalcFwdBeforeFwdFXConsant);
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
						setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *ycPro);
					}
					else if (enableCalculation || (target == CURVETYPE_FLOATER && markets[i] == XCCYBASIS))
					{
						setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *ycPro);
					}
					else if (!enableCalculation && isSwapTenorAdjust)
					{
						//only entitysetup
						AQLString tenorswapname = dynamic_cast<const AQLDataString &>(ycPro->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
						if (tenorswapname == markets[i])
						{
							setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *ycPro);
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
					continue;
		
				setUpCurveDataByReadFile(dataInstance, asOfDate, tmpCurrency, markets[i], yieldDataName, *ycPro);
				continue;
			}
			else if( marketType == MARKETTYPE_SWAP )
			{
				if (!enableCalculation && target != markets[i])
					continue;

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
				isoismode.toUpper();
				if (isoismode != AQ_NO_DATA)
				{
					setUpGenCurveDataOIS(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);
					OISName = markets[i];
				}
				else
					setUpGenCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);			
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
				AQLMathYieldCurvePro &fYcPro = dynamic_cast<AQLMathYieldCurvePro &>
						(objPool.getObject(AQLMarketData::getBaseYieldProName(tmpMktNames[0]), ENCHKTYPE_ISDEFINED).get());
				fYcPro.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tmpMktNames[1]);
				fYcPro.AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tmpMktNames[1], new AQLDataMultiReference()).convertFromString(refData_);
			}
			else
			{
				ycPro->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i]);
				ycPro->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i], new AQLDataMultiReference()).convertFromString(refData_);
			}
		}
	}

	//set tenorswap convention
	AQLString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(markets.begin(), markets.end(), tenorSwapName) == markets.end() && tenorSwapName != AQ_NO_DATA)
	{
		AQLString refData_tenor = "";
		setUpBasisCurveData(dataInstance, refData_tenor, asOfDate, tmpCurrency, tenorSwapName, yieldDataName, isSpotUse, *ycPro);
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		ycPro->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName);
		ycPro->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tenorSwapName, new AQLDataMultiReference()).convertFromString(refData_tenor);
	}

	//set libor-ois basis convention
	AQLString suffix_ois = AQLString("." + OISName).toLower();
	const AQLString longTermConv = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffix_ois).toUpper();
	AQLString loBasisName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + suffix_ois).toUpper();
	loBasisName = (loBasisName == AQ_NO_DATA) ? LOBASIS : loBasisName;
	if (find(markets.begin(), markets.end(), loBasisName) == markets.end() && longTermConv == LOBASIS)
	{
		AQLString refData_lob = "";
		setUpLiborOISBasisCurveData(dataInstance, refData_lob, asOfDate, tmpCurrency, loBasisName, OISName, yieldDataName, isSpotUse, *ycPro);
		refData_lob = refData_lob.subString(0, refData_lob.size() - 2);
		ycPro->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + loBasisName);
		ycPro->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + loBasisName, new AQLDataMultiReference()).convertFromString(refData_lob);
	}


	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	ycPro->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(swapCurves));

	if (mainBDF != AQ_NO_DATA)
	{
		ycPro->AQLObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		ycPro->AQLObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new AQLDataString(mainBDF));
	}
	
	// generate yield data
	AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
						(ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	bool isCurveTargetFound = false;
	if (!enableCalculation)
	{
		isCurveTargetFound = (genBasisSwapMarket.end() == std::find(genBasisSwapMarket.begin(),genBasisSwapMarket.end(),
			target) && target != CURVETYPE_FLOATER);
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
				ycPro->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
				ycPro->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(targetDF));
			}
			else
			{
				ycPro->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
				ycPro->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new AQLDataString(targetDF));
			}
		}	
	}
	dataInstance.getReferencePool().completeDependency();

	// get fwdfx flag
	//bool isFwdFX = false;
	//AQLString isFwdFXStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX);
	//if (isFwdFXStr.toUpper() == "TRUE") isFwdFX = true;
	loadFwdFXConstCurveDataAndCalibrate(currency, dataInstance, true);

	//if ((enableCalculation || isCurveTargetFound) && !isFwdFX)
	if ((enableCalculation || isCurveTargetFound) && ((std::find(markets.begin(), markets.end(), SWAP) != markets.end())||(std::find(markets.begin(), markets.end(), OISCURVE) != markets.end())))
	{
		if (isCurveTargetFound)
		{
			ycPro->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			ycPro->AQLObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new AQLDataBool(true));
		}
		modelDataObj.calibrateModel(asOfDate);
		ycPro->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		ycPro->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	}
	//adjust discountfactor
	tmpCurrency.toLower();
	AQLDataBool tmpAttrDF;
	tmpAttrDF.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF));
	if(tmpAttrDF.get())
	{
		const std::map<AQLString, AQLString> &assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (!ycPro->isBasisCurve(it->first) && (enableCalculation || target == it->second))
			{
				AQLMarketData::adjustDiscountFactor(*eData, it->first);
			}
		}
	}

	// basis
	bool isBasis = false;
	if (!basisCurves.empty() && !isCurveTargetFound && target != CURVETYPE_FLOATER)
	{
		isBasis = true;
		ycPro->AQLObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		ycPro->AQLObject::add(IR_CALIBRATION_DATA_BASISDFS, new AQLDataStrings(basisCurves));
	}
	
	if (refBData.size() > 1)
	{
		isBasis = true;
		// set reference for basis
		refBData = refBData.subString(0, refBData.size() - 2);
		ycPro->getBasisData().convertFromString(refBData);
	}

	if (isBasis)
	{
		bool isBasisEnabled = (!enableCalculation && !isCurveTargetFound);
		if (isBasisEnabled)
		{
			ycPro->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			ycPro->AQLObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new AQLDataBool(true));
		}
		ycPro->setBasisRates();
		ycPro->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		ycPro->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
	}

	AQLString genFloaterName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != AQ_NO_DATA && (enableCalculation || target == CURVETYPE_FLOATER)) 
		setUpFloater(tmpCurrency, *ycPro, genFloaterName);

	// set df2
	if (target == STD && mainBDF != AQ_NO_DATA)
	{
		ycPro->setDF2();
	}

	if (!aud_origSwapRate.empty())
	{
		map<AQLString, map<AQLString, double> >::const_iterator it = aud_origSwapRate.begin();
		while (it != aud_origSwapRate.end())
		{
			AQLMarketData::restoreSwapRateFromL(*ycPro, it->second, currency, &(it->first));
			++it;
		}
	}
	// set daycount
	setUpCurveTypeDayCount(*ycPro, *yc);

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

		const std::map<AQLString, AQLString>& assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
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

// 
/*!
    @brief set up curveType daycount

	@param[out] ycPro
	@param[out] yc
*/
void
AQLCalibrateModelIR::setUpCurveTypeDayCount(AQLMathYieldCurvePro &ycPro, AQLMathYieldCurve &yc) const
{
	// set daycount
	const map<AQLString, AQLString> &assignedCurveMktMap = ycPro.getAssignedCurveMktMap();
	map<AQLString, AQLString>::const_iterator it_cur = assignedCurveMktMap.begin();
	while (it_cur != assignedCurveMktMap.end())
	{
		AQLPriceDataDayCount dc;
		AQLPriceDataCalendar cal;
		AQLPriceDataSlidingRule sld;
		AQLString accessory;
		ycPro.getForwardConvention(it_cur->first, dc, sld, cal, accessory);
		yc.getDayCount(it_cur->first) = dc;
		ycPro.getDayCount(it_cur->first) = dc;
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
AQLCalibrateModelIR::generateInitialValueArbfree(const AQLString &currency, AQLDataInstance &dataInstance) const
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
	AQLMathYieldCurvePro *ycPro = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		ycPro = new AQLMathYieldCurvePro(&dataInstance);
		objPool.set(yieldProName, ycPro);
	}
	else
	{
		//we must not erase the reset method for only ycpro
		ycPro = &dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(yieldProName).get());
	}
	ycPro->getName().convertFromString(yieldProName);

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

	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *ycPro, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, true);
	// set isArbFree
	ycPro->getIsArbFree().set(true);
	bool isXccyMarkedToMarket = false;
	AQLString isXccyMarkedToMarketString = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST,
																	  tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISXCCYMARKEDTOMARKET).toUpper(); // Alias Method: First parameter takes priority
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

	eData->remove(IR_CALIBRATION_DATA_6MLCURVENAMES);
	eData->remove(IR_CALIBRATION_DATA_DFCURVENAMES);
	eData->remove(IR_CALIBRATION_DATA_3MLCURVENAMES);	
	eData->add(IR_CALIBRATION_DATA_6MLCURVENAMES, new AQLDataStrings(curveNames_6ML));
	eData->add(IR_CALIBRATION_DATA_DFCURVENAMES, new AQLDataStrings(curveNames_DF));
	eData->add(IR_CALIBRATION_DATA_3MLCURVENAMES, new AQLDataStrings(curveNames_3ML));
	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
	ycPro->AQLObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new AQLDataBool(isXccyMarkedToMarket));
	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET);
	ycPro->AQLObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new AQLDataBool(isXccyMarkedToMarket));
	ycPro->setArbFreeCurveName(curveNames_6ML,curveNames_DF,curveNames_3ML);

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
	setUpGenCurveData(dataInstance, refData, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);

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
			loadYieldCurveDataAndCalibrate(ccy_floater, dataInstance);
		}
		AQLString curveName_floater = usd3mlFloaterInfo[1];
		ycPro->getForeignYieldData().convertFromString(fYieldDataName);

		if (isSetCurveID != "TRUE")
		{
			AQLString baseCcy = currency; AQLString domCcy = currency; AQLString forCcy = ccy_floater;
			for (;;)
			{
				AQLMathYieldCurvePro &ycPro_dccy = dynamic_cast<AQLMathYieldCurvePro &>
							(objPool.getObject(AQLMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				AQLMathYieldCurvePro &ycPro_fccy = dynamic_cast<AQLMathYieldCurvePro &>
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

		if (isXccyMarkedToMarket)
		{
			AQLStringVector baseCcyDFInfo = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_BASECCYDF).toToken(':');
			if (baseCcyDFInfo.size() != 2) throw AQLCoreInvalidData("No XccyBasis information!",__FILE__,__LINE__);
			if (usd3mlFloaterInfo[0] != baseCcyDFInfo[0]) throw AQLCoreInvalidData("Reference currencies are inconsistent!",__FILE__,__LINE__);
			xccyBasis->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new AQLDataString()).convertFromString(baseCcyDFInfo[1]);
		}

		// set yield curve pro
		refData_XccyBasis = refData_XccyBasis.subString(0, refData_XccyBasis.size() - 2);
		ycPro->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + XCCYBASIS);
		ycPro->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + XCCYBASIS, new AQLDataMultiReference()).convertFromString(refData_XccyBasis);
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
		setUp36BasisDummyData(dataInstance, refData_, asOfDate, tmpCurrency, THREESIXBASIS, yieldDataName, isSpotUse, *ycPro);
		// set yield curve pro
		refData_ = refData_.subString(0, refData_.size() - 2);
		ycPro->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + THREESIXBASIS);
		ycPro->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + THREESIXBASIS, new AQLDataMultiReference()).convertFromString(refData_);
	}
	// set yield curve pro
	refData = refData.subString(0, refData.size() - 2);
	ycPro->getMarketData().convertFromString(refData);

	AQLDataHolder* dh;
	AQLStringVector swapCurves;
	dh = &ycPro->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) swapCurves = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

	AQLStringVector basisCurves;
	dh = &ycPro->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) basisCurves = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

	AQLString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (!markets.empty() && markets[0] != AQ_NO_DATA)
	{
		//AQLDataStrings &attrGenCurves = dynamic_cast<AQLDataStrings &>(ycPro->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings()).get());		
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
			AQLString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_INTERPOLATION + suffix).toUpper();
			if (strInter != AQ_NO_DATA)
			{
				yc->getInterpolation(markets[i]).convertFromString(strInter.toLower());
				ycPro->getInterpolation(markets[i]).convertFromString(strInter);
				if (mainBDF == markets[i])
				{
					yc->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
					ycPro->getInterpolation(IR_CALIBRATION_DATA_DFS2).convertFromString(strInter);
				}
			}

			if ( marketType == MARKETTYPE_BASIS )
			{
				if (!enableCalculation && markets[i] != XCCYBASIS && markets[i] != THREESIXBASIS) continue;

				setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, *ycPro);

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

				setUpCurveDataByReadFile( dataInstance, asOfDate, tmpCurrency, markets[i], yieldDataName, *ycPro );
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
				isoismode.toUpper();
				if (isoismode != AQ_NO_DATA)
					setUpGenCurveDataOIS(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);
				else
					setUpGenCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);
			}
			else
			{
				throw AQLCoreInvalidData("Market type is not supported!!", __FILE__, __LINE__); 
			}

			// set yield curve pro
			refData_ = refData_.subString(0, refData_.size() - 2);
			ycPro->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i]);
			ycPro->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + markets[i], new AQLDataMultiReference()).convertFromString(refData_);
		}
	}

	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	ycPro->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(swapCurves));

	// generate yield data
	ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
						(ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	dataInstance.getReferencePool().completeDependency();
	modelDataObj.calibrateModel(asOfDate);
	
	// generate swap curve
	if (swapCurves.size() != 0 && enableCalculation)
	{
		ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
		for (size_t i=0; i<swapCurves.size(); i++)
		{
			ycPro->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(swapCurves[i]));
			modelDataObj = dynamic_cast<AQLDataProcedure &> (ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
			modelDataObj.calibrateModel(asOfDate);
			ycPro->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
		}
		ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	}

	//adjust discountfactor
	tmpCurrency.toLower();
	AQLDataBool tmpAttrDF;
	tmpAttrDF.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF));
	if(tmpAttrDF.get())
	{
		const std::map<AQLString, AQLString> &assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (!ycPro->isBasisCurve(it->first) && (enableCalculation || target == it->second))
			{
				AQLMarketData::adjustDiscountFactor(*eData, it->first);
			}
		}
	}

	// basis
	bool isBasis = false;
	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_BASISDFS);
	if (!basisCurves.empty() && enableCalculation)
	{
		isBasis = true;
		ycPro->AQLObject::add(IR_CALIBRATION_DATA_BASISDFS, new AQLDataStrings(basisCurves));
	}

	ycPro->AQLObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
	if (mainBDF != AQ_NO_DATA)
	{
		isBasis = true;
		ycPro->AQLObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new AQLDataString(mainBDF));
	}
	if (refBData.size() > 1)
	{
		isBasis = true;
		// set reference for basis
		refBData = refBData.subString(0, refBData.size() - 2);
		ycPro->getBasisData().convertFromString(refBData);
	}

	if (isBasis)
	{
		ycPro->setBasisRates();
	}

	// set daycount
	setUpCurveTypeDayCount(*ycPro, *yc);

	AQLString genFloaterName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != AQ_NO_DATA || target == CURVETYPE_FLOATER) setUpFloater(tmpCurrency, *ycPro, genFloaterName);


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

		const std::map<AQLString, AQLString>& assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second != SWAP && it->second != XCCYBASIS && it->second != THREESIXBASIS) dataoutCurves.push_back(it->first);
		}

		if (genFloaterName != AQ_NO_DATA) dataoutCurves.push_back(genFloaterName);

		dataoutCurve(dataoutCurves, *eData,yieldDataName);
	}
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
// 
/*!
    @brief generate sde correlation data

	@param[in] currency  currency or fx ex.JPY/USD
	@param[in] dataInstance
*/
void
AQLCalibrateModelIR::loadCorrelationDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	AQLString sdeName = getSDEAttrName(currency);
	AQLString type = getCorTye(currency);
	type.toUpper();
	
	AQLString corName;
	if (AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST) == "TRUE")
	{
		AQLString tmpCurrency = currency;
		corName = "cor" + tmpCurrency.toUpper() + "ir";
	}
	else
	{
		corName = PREFIX_COR + sdeName;
	}
	AQLMathCorrelation *corEntity;
	bool isCorEntityAlreadyExist(dataInstance.getObjectPool().find(corName));
	if (isCorEntityAlreadyExist)
	{
		corEntity = dynamic_cast<AQLMathCorrelation*>(&dataInstance.getObjectPool().getObject(corName).get());
		if (corEntity == 0)
			throw AQLCoreInvalidData("Corrlation object update failed.", __FILE__, __LINE__); 
		corEntity->reset();
	}
	else
	{
		corEntity = new AQLMathCorrelation(&dataInstance);
		AQLDataString &corAttrName = corEntity->getName();
		corAttrName.set(corName);
	}

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

	if (!isCorEntityAlreadyExist)
	{
		// set data as reference
		dataInstance.getDataMaster().setData(corName, DATA_REFERENCE); 
		// set object pool
		dataInstance.getObjectPool().remove(corName);	
		dataInstance.getObjectPool().set(corName, corEntity);	
	}
}

#endif
// 
/*!
    @brief get SDE data name

	@param[in] currency
	@return AQLString
*/
AQLString
AQLCalibrateModelIR::getSDEAttrName(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	return  mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
}

// 
/*!
    @brief get Grid property val

	@param[in] key
	@param[in] grid
	@return AQLString
*/
AQLString
AQLCalibrateModelIR::getGridStaticData(const AQLString &key, const AQLString &grid) const
{
	AQLString suffix = "." + grid;
	suffix.toLower();
	AQLString val = mpStaticData->getStaticData(key + suffix);
	if (val != AQ_NO_DATA)
	{
		return val;
	}
	else
	{
		return mpStaticData->getStaticData(key);
	}
}

// 
/*!
    @brief fuction to set up generate configuration
*/
void
AQLCalibrateModelIR::setUpGenerateConfig
(AQLDataInstance &dataInstance, const AQLDate &asOfDate, const AQLString &currency, AQLMathYieldCurve &yc, AQLMathYieldCurvePro &ycPro,
 AQLObject &ycData, bool &isAudExtra, bool &isSwapTenorAdjust, bool &isSpotUse, bool isArbFree) const
{
	AQLString isSetCurveID = AQLCoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();

	/*AQLString yieldName = PREFIX_YIELD + getSDEAttrName(currency);
	yc->getName().convertFromString(yieldName);*/
	AQLString yieldName = yc.getName().get();

	yc.AQLObject::remove(CALIBRATION_DATA_ASOFDATE);
	yc.AQLObject::add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));

	// set interpolation
	yc.getInterpolation().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_INTERPOLATION).toLower());
	// set daycount
	yc.getDayCount().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DAYCOUNT).toUpper());
	// set freq
	yc.getFrequency().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FREQUENCY).toUpper());
	// set sliding rule
	yc.getSlidingRule().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SLIDINGRULE).toUpper());
	// set cal 
	AQLPriceDataCalendar calY;
	AQLString calYStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_CALENDAR);
	calY.convertFromString(calYStr);
	yc.getCalendar() = calY;
	//set basis DF
	AQLString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (mainBDF != AQ_NO_DATA) yc.setBasisCurveType(mainBDF);

	// set yield to object pool
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	//AQLString yieldProName = "PRO_" + yieldName;

	// set yieldpro to object pool
	//// set as of date
	//AQLString asOfDateStr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	//AQLDate asOfDate;
	//asOfDate.setDate(asOfDateStr.getCString());
	ycPro.getAsOfDate().set(asOfDate);
	// set curve generator
	ycPro.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
	// set interpolation
	ycPro.getInterpolation().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION).toLower());
	ycPro.getInterpolation_yg().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION).toLower());
	ycPro.getInterpolation_fw().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FUTURE_INTERPOLATION).toLower());
	// linear interpolation for basis
	ycPro.getInterpolation_bs().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASIS_INTERPOLATION).toLower());
	// set basis function
	ycPro.getBasisFunction().setFunction(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_BASISFUNCTION).toLower());
	// set daycount
	ycPro.getDayCount().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT).toUpper());
	// set freq
	ycPro.getFrequency().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY).toUpper());
	// set sliding rulue
	ycPro.getSlidingRule().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE).toUpper());

	// set rate priority
	AQLString ratePrio_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_RATEPRIORITY).toUpper();
	if (ratePrio_str != AQ_NO_DATA) ycPro.getData(PRICING_DATA_RATEPRIORITY).convertFromString(ratePrio_str);
	
	AQLDataBool tmpAttrB;
	// set tenor adjust
	AQLString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST).toUpper();
	if (strSwapTenorAdj != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(strSwapTenorAdj);
		isSwapTenorAdjust = tmpAttrB.get();
	}
	ycPro.getIsSwapTenorAdjust().set(isSwapTenorAdjust);
	if (isSwapTenorAdjust)
	{
		AQLString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
		if (tenorSwapName == AQ_NO_DATA) throw AQLCoreInvalidData("Set tenor swap name!", __FILE__, __LINE__);
		
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_TENORSWAPNAME);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_TENORSWAPNAME, new AQLDataString(tenorSwapName));
	}

	/*AQLString yieldDataName = yieldName + "_DATA";
	ycData.remove(CALIBRATION_DATA_NAME);
	ycData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(yieldDataName);*/
	AQLString yieldDataName = dynamic_cast<const AQLDataString&> ((ycData.getData(CALIBRATION_DATA_NAME, ISNOTNULL)).get()).get();
	// set asofdate
	ycData.remove(CALIBRATION_DATA_ASOFDATE);
	ycData.add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));
	ycData.remove(IR_CALIBRATION_DATA_CURRENCY);
	ycData.add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString(currency));
	// set interpolation
	ycData.remove(CALIBRATION_DATA_INTERPOLATION);
	ycData.add(CALIBRATION_DATA_INTERPOLATION, new AQLPriceDataInterpolation())
		.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_INTERPOLATION).toLower());

	// set yield data
	yc.getYieldData().convertFromString(yieldDataName);
	ycPro.getYieldData().convertFromString(yieldDataName);
	// set base curve reference
	ycPro.getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(yieldName);

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

	if (isAudExtra && isSwapTenorAdjust) throw AQLCoreInvalidData("We can not set AUD extra and swap tenor adjust at a same time!", __FILE__, __LINE__);

	// spotDate use flag
	tmpAttrB.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSPOTUSE));
	isSpotUse = tmpAttrB.get();

	// set curve exist check
	if (isPricer == "FALSE" || isPricer == AQ_NO_DATA)
	{
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, new AQLDataBool(true));
	}

	// set max term of curve
	AQLString maxTerm = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM);
	if (maxTerm == AQ_NO_DATA)
		maxTerm = mpStaticData->getStaticData(KEY_SIMULATION_TERM_MAX);
	if (maxTerm != AQ_NO_DATA)
	{
		ycData.remove(IR_CALIBRATION_DATA_MAXTERM);
		ycData.add(IR_CALIBRATION_DATA_MAXTERM, new AQLDataString(maxTerm));
		AQLString maxFreq = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERMFREQ);
		if (maxFreq == AQ_NO_DATA)
			maxFreq = mpStaticData->getStaticData(KEY_SIMULATION_TERM_MAX_FREQ);
		if (maxFreq == AQ_NO_DATA)
			throw AQLCoreInvalidData("Max term frequency is needed for curve extrapolation!", __FILE__, __LINE__);
		ycData.remove(IR_CALIBRATION_DATA_MAXTERMFREQ);
		ycData.add(IR_CALIBRATION_DATA_MAXTERMFREQ, new AQLDataString(maxFreq.toUpper()));
	}

}

// 
/*!
    @brief fuction to set up basis curve data
*/
void
AQLCalibrateModelIR::setUpBasisCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &curveCurrency, 
									  const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, AQLMathYieldCurvePro &ycPro, const AQLString* pMktCurrency, bool isCalcFwdBeforeFwdFXConsant) const
{
	AQLString mktCurrency = pMktCurrency ? *pMktCurrency : curveCurrency;
	AQLString curveMktName = pMktCurrency ? mktCurrency + "_" + marketName : marketName;
	mktCurrency.toLower();curveMktName.toUpper();

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
	mktSuffix.toLower();curveSuffix.toLower();
	
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

	bool is2Swap = false;
	AQLString is2SwapProp = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_IS2SWAP + curveSuffix).toUpper();
	AQLString suffixSwap = "";
	if (is2SwapProp != AQ_NO_DATA)
	{
		AQLDataBool is2SwapAttr;
		is2SwapAttr.convertFromString(is2SwapProp);
		is2Swap = is2SwapAttr.get();
	}

	AQLString fCurve;
	AQLString dCurve;
	AQLString ccy_fCurve;
	AQLString ccy_dCurve;
	AQLString a_fCurve;
	AQLString a_dCurve;
	AQLString ccy_a_fCurve;
	AQLString ccy_a_dCurve;
	AQLString f_dCurve;
	AQLString ccy_f_dCurve;
	// cashflow info
	AQLString c_freqStr;
	AQLString c_freqcpdStr;
	AQLString c_daycStr;
	AQLString c_slidingStr;
	AQLString c_calStr;
	AQLDate c_spotDate;
	bool c_isBackward = false;
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
	bool a_c_isBackward = false;
	int a_c_resetLag;
	// against index info
	AQLString a_i_freqStr;
	AQLString a_i_accessaryStr;
	AQLString a_i_daycStr;
	AQLString a_i_slidingStr;
	AQLString a_i_fixcalStr;
	AQLString a_i_paycalStr;
	int a_i_resetLag;
	// fixed cashflow info in 2-swap
	AQLString f_freqStr;
	AQLString f_daycStr;
	AQLString f_slidingStr;
	AQLString f_calStr;
	AQLDate f_spotDate;
	int f_resetLag;
	bool f_eom;

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
		if (mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_ISBACKWARD + mktSuffix).toUpper() == "TRUE")
		{
			c_isBackward = true;
		}		
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
		if (mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_ISBACKWARD + mktSuffix).toUpper() == "TRUE")
		{
			a_c_isBackward = true;
		}
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
		if (mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_ISBACKWARD + mktSuffix).toUpper() == "TRUE")
		{
			c_isBackward = true;
		}
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
		if (mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_ISBACKWARD + mktSuffix).toUpper() == "TRUE")
		{
			a_c_isBackward = true;
		}
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
	if (is2Swap)
	{
		f_dCurve = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffixSwap);
		if (f_dCurve == AQ_NO_DATA)
		{
			f_dCurve = dCurve;
		}
		f_calStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + suffixSwap);
		if (isSpotUse)
		{
			f_spotDate.setDate(mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_SPOTDATE + suffixSwap).getCString());
		}
		else
		{
			f_resetLag = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_RESETLAG + suffixSwap).getIntValue();
			tmpCal.convertFromString(f_calStr);
			f_spotDate = tmpCal.getBusinessDay(asOfDate, f_resetLag);
		}
		AQLString strEOMRoll = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_ISEOMRLL + suffixSwap).toUpper();
		if (strEOMRoll != AQ_NO_DATA)
		{
			AQLDataBool tmpIsEOMRoll;
			tmpIsEOMRoll.convertFromString(strEOMRoll);
			f_eom = tmpIsEOMRoll.get();
		}
		if (f_eom)
		{
			AQLString strEOMDay = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_EOMDAY + suffixSwap).toUpper();
			if (strEOMDay != AQ_NO_DATA)
			{
				if (f_spotDate.dayOfMonth() != strEOMDay.getIntValue())
				{
					f_eom = false;
				}
			}
			else
			{
				AQLPriceDataCalendar cal;
				cal.convertFromString(f_calStr);
				AQLDate eomDate = cal.getEOMDay(f_spotDate);
				if (f_spotDate != eomDate)
				{
					f_eom = false;
				}
			}
		}
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
	AQLString tmp_f_dCurve = f_dCurve;
	convertCurveName(tmp_fCurve, curveCurrency, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(tmp_dCurve, curveCurrency, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(tmp_a_fCurve, curveCurrency, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(tmp_a_dCurve, curveCurrency, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	convertCurveName(tmp_f_dCurve, curveCurrency, ccy_f_dCurve, f_dCurve, isPricer, objPool);
	if (ccy_dCurve != curveCurrency || ccy_fCurve != curveCurrency || ccy_a_dCurve != ccy_a_fCurve)
		throw AQLCoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);

	// against currency
	bool isForeignCcyLeg = false;
	AQLString fYieldDataName;
	ycPro.remove(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + AQLString("_") + curveMktName);
	if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrency) 
	{
		isForeignCcyLeg = true;
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

		if ((mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_a_fCurve) && enableCalculation)||isCalcFwdBeforeFwdFXConsant)
		{
			loadYieldCurveDataAndCalibrate(ccy_a_fCurve, dataInstance);
		}
		ycPro.getForeignYieldData().convertFromString(fYieldDataName);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + AQLString("_") + curveMktName, new AQLDataBool(isForeignCcyLeg));
		
		if (!isSetCurveID)
		{
			AQLString baseCcy = curveCurrency; AQLString domCcy = curveCurrency; AQLString forCcy = ccy_a_fCurve;
			for (;;)
			{
				AQLMathYieldCurvePro &ycPro_dccy = dynamic_cast<AQLMathYieldCurvePro &>
							(objPool.getObject(AQLMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				AQLMathYieldCurvePro &ycPro_fccy = dynamic_cast<AQLMathYieldCurvePro &>
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
	if (is2Swap && (f_dCurve == DUMMY) && !isSpotUse)
	{
		tmpCal.convertFromString(f_calStr);
		f_spotDate = tmpCal.getBusinessDay(asOfDate, f_resetLag);
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
	bool isOddTermFRNIndex = false;
	AQLString strIsOddTermFRNIndex = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISODDTERMFRNINDEX + mktSuffix).toUpper();
	if (strIsOddTermFRNIndex == "TRUE")
	{
		isOddTermFRNIndex = true;
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

	AQLString spotRateTerm = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_SPOTRATETERM + mktSuffix).toUpper();
	double spotRate = DBL_MAX;
	if (spotRateTerm != AQ_NO_DATA)
	{
		const AQLDataMultiReference &marketRef = ycPro.getMarketData();
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
					break;
				}
			}
		}		
	}

	// cashflow info
	bool isFXForwardQuotedAsOutright = false;
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
	double fwdFXPipSizeFactor = 1.0; // The inverted pipsize i.e. 10,000 = use 4 d.p. for FX Forward Quotes
	//double spotfx = 1.0;
	bool isFwdFX = false;
	AQLString isFwdFXStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX).toUpper();
	if (isFwdFXStr == "TRUE") isFwdFX = true;
	if (isFwdFX && marketName == XCCYBASIS)
	{
		// getStaticData has been overloaded to check for first name and if that fails to look for the alias
		AQLString isFXForwardQuotedAsOutrightString = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISRATIO + mktSuffix,
																				 mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISFXOUTRIGHT + mktSuffix).toUpper();
		// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
		if (isFXForwardQuotedAsOutrightString == "TRUE") isFXForwardQuotedAsOutright = true;
		if (!isFXForwardQuotedAsOutright)
		{
			fwdFXPipSizeFactor = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_DENOMINATOR + mktSuffix,
															 mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_PIPSIZE + mktSuffix).getDoubleValue(); // Alias Method: First Parameter Takes Priority
			AQLString isPriceCcyStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISPRICECCY + mktSuffix, mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISDOMESTICCURRENCY + mktSuffix).toUpper(); // Alias Method: first name takes priority
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

	if (fwdFXSize + basisSize == 0)
	{
		AQLString msg = marketName + ", basis and fwd fx data is empty";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	for (unsigned int j = 0; j < fwdFXSize + basisSize; ++j)
	{
		AQLObject *mktData = NULL;
		AQLString nameB = yieldDataName +  "_" + curveMktName + "_" + AQLString(static_cast<int>(j));
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
			mktData->add(IR_CALIBRATION_DATA_ISRATIO, new AQLDataBool(isFXForwardQuotedAsOutright));
			mktData->add(IR_CALIBRATION_DATA_ISFXOUTRIGHT, new AQLDataBool(isFXForwardQuotedAsOutright)); // alias for ISRATIO
			if (isFXForwardQuotedAsOutright)
			{
				mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(fwd_fx));
			}
			else
			{
				mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(fwd_fx / fwdFXPipSizeFactor));
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
			// set data type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BASIS);
			AQLString term = basisDataMtx[j - fwdFXSize][0].toUpper();
			double basis = basisDataMtx[j - fwdFXSize][1].getDoubleValue();
			// set term
			mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));
			// set rate
			mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(basis / 10000.0));
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
			// 2swap
			if (is2Swap)
			{
				f_freqStr = getGridStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + suffixSwap, term).toUpper();
				f_daycStr = getGridStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT + suffixSwap, term).toUpper();
				f_slidingStr = getGridStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE + suffixSwap, term).toUpper();
			}
		}

		// set simultaneous equation
		mktData->add(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS, new AQLDataBool(isSimuEq));
		// set issamegridindex
		mktData->add(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX, new AQLDataBool(isSameGridIndex));
		// set issamegridindex
		mktData->add(IR_CALIBRATION_DATA_ISODDTERMFRNINDEX, new AQLDataBool(isOddTermFRNIndex));
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
		mktData->add(IR_CALIBRATION_DATA_CASHLETISBACKWARD, new AQLDataBool(c_isBackward));
		// index info
		mktData->add(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(i_fixcalStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, new AQLPriceDataCalendar()).convertFromString(i_paycalStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXFREQUENCY, new AQLDataString()).convertFromString(i_freqStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXACCESSARY, new AQLDataString()).convertFromString(i_accessaryStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(i_daycStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(i_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXRESETLAG, new AQLDataInt(i_resetLag));
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
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETISBACKWARD, new AQLDataBool(a_c_isBackward));
		// against index info
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(a_i_fixcalStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, new AQLPriceDataCalendar()).convertFromString(a_i_paycalStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, new AQLDataString()).convertFromString(a_i_freqStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, new AQLDataString()).convertFromString(a_i_accessaryStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(a_i_daycStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(a_i_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, new AQLDataInt(a_i_resetLag));
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

		if (is2Swap)
		{
			mktData->add(IR_CALIBRATION_DATA_IS2SWAP, new AQLDataBool(true));
			// fixed cashflow info in 2-swap
			mktData->add(IR_CALIBRATION_DATA_DFCURVENAME, new AQLDataString()).convertFromString(f_dCurve);
			mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(f_calStr);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(f_freqStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(f_daycStr);
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(f_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(f_spotDate));
			mktData->add(IR_CALIBRATION_DATA_ISEOMROLLSW, new AQLDataBool(f_eom));
		}
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
			ycPro.setAssignedCurveMktMap(assignedCurves[i], curveMktName);
		}
	}
	else
	{
		ycPro.setAssignedCurveMktMap(curveMktName,curveMktName);
	}

	// optimization method
	AQLString optimizationMethod = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_OPTIMIZEMETHOD + mktSuffix);
	ycPro.getOptimizeMethod().set(optimizationMethod);

	// compound function
	AQLString compoundMethod = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_COMPOUNDMETHOD + mktSuffix).toUpper();
	if (compoundMethod == AQ_NO_DATA || compoundMethod == FLATCOMPOUNDING)
	{
		ycPro.getCompoundingFunction().convertFromString(FN_COMPOUNDING8_STR);
	}
	else if (compoundMethod == DAILYCOMPOUNDING)
	{
		if ((c_freqcpdStr != AQ_NO_DATA && c_freqcpdStr != c_freqStr && c_freqStr != BUSINESS_DAYS && c_freqStr != DAILY)
			|| (a_c_freqcpdStr != AQ_NO_DATA && a_c_freqcpdStr != a_c_freqStr && a_c_freqStr != BUSINESS_DAYS && a_c_freqStr != DAILY ))
		{
			AQLString msg = "This compound type is suppoted only for daily fixing.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		ycPro.getCompoundingFunction().convertFromString(FN_COMPOUNDING7_STR);
	}
	else if (compoundMethod == DAILYAVERAGING)
	{
		if ((c_freqcpdStr != AQ_NO_DATA && c_freqcpdStr != c_freqStr && c_freqStr != BUSINESS_DAYS && c_freqStr != DAILY)
			|| (a_c_freqcpdStr != AQ_NO_DATA && a_c_freqcpdStr != a_c_freqStr && a_c_freqStr != BUSINESS_DAYS && a_c_freqStr != DAILY ))
		{
			AQLString msg = "This compound type is suppoted only for daily fixing.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		ycPro.getCompoundingFunction().convertFromString(FN_COMPOUNDING10_STR);
	}
	else
	{
		AQLString msg = "This compunding type is not supported.";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
	// set adjust value
	//yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUETERM);
	//yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUE);
	//yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new AQLDataDoubles(adjustValue_term));
	//yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUE, new AQLDataDoubles(adjustValue));
}

// 
/*!
    @brief fuction to set up 36 basis curve dummy data
*/
void
AQLCalibrateModelIR::setUp36BasisDummyData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
									    const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, 
										AQLMathYieldCurvePro &ycPro) const
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

// 
/*!
    @brief fuction to set up base curve data
*/
void 
AQLCalibrateModelIR::setUpGenCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
									const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse,
									bool isAudExtra, AQLMathYieldCurvePro &ycPro, 
									std::map<AQLString, std::map<AQLString, double> > &aud_origSwapRate) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString staticDataSuffix = "";
	if (marketName != SWAP)
	{
		staticDataSuffix = "." + marketName;
		staticDataSuffix.toLower();
	}
	AQLString suffix_data = "";
	if (marketName != SWAP)
	{
		suffix_data = "_" + marketName;
	}

	bool isFwdFX = false;
	AQLString isFwdFXStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX + staticDataSuffix);
	if (isFwdFXStr.toUpper() == "TRUE") isFwdFX = true;

	AQLString liborFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix);
	AQLFileAccessor liborFile(AQLMarketData::getNumFileName(liborFileName));
	AQLStringMatrix liborDataMtx;
	//if (!isFwdFX)
	//{
		liborFile.readAllData(MARKET_DATA_DELIMITER, liborDataMtx);
		liborFile.close();
	//}

	bool isNoLibor = (liborDataMtx.size() == 0 || liborDataMtx[0].size() < 2);
	if (isNoLibor && !isFwdFX)
		throw AQLCoreInvalidData("LiborFile is empty", __FILE__,__LINE__);
	double firstVal = 0.0;
	if (!isNoLibor) firstVal = liborDataMtx[0][1].getDoubleValue();

	// create O_N
	AQLObject *mktDataO_N = NULL;
	AQLString nameO_N = yieldDataName + "_O_N" + suffix_data;
	const AQLObjectHolder ehois = objPool.getObject(nameO_N);
	if (!ehois.isDefined())
	{
		mktDataO_N = new AQLObject();
		objPool.set(nameO_N, mktDataO_N);
	}
	else
	{
		objPool.getObject(nameO_N).get().clear();
		mktDataO_N = &objPool.getObject(nameO_N).get();
	}

	mktDataO_N->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameO_N);
	// set calendar
	AQLString calMStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_CALENDAR + staticDataSuffix);
	mktDataO_N->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calMStr);
	// set spotdate
	if (isSpotUse)
	{
		AQLDate spotDateM(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_SPOTDATE + staticDataSuffix).getCString());
		mktDataO_N->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateM));
	}
	// set daycount
	AQLString daycMStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_DAYCOUNT + staticDataSuffix).toUpper();
	mktDataO_N->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycMStr);
	// set sliding
	AQLString slidingMStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_SLIDINGRULE + staticDataSuffix).toUpper();
	mktDataO_N->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingMStr);
	// set data type
	mktDataO_N->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_O_N);
	refData += nameO_N + ":";

	// create T_N
	AQLObject *mktDataT_N = NULL;
	AQLString nameT_N = yieldDataName + "_T_N" + suffix_data;
	const AQLObjectHolder ehtn = objPool.getObject(nameT_N);
	if (!ehtn.isDefined())
	{
		mktDataT_N = mktDataO_N->clone();
		objPool.set(nameT_N, mktDataT_N);
	}
	else
	{
		mktDataT_N = &objPool.getObject(nameT_N).get();
	}
//#endif
	mktDataT_N->getData(CALIBRATION_DATA_NAME, ISNOTNULL).convertFromString(nameT_N);
	// set data type
	mktDataT_N->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).convertFromString(YIELD_TYPE_T_N);
	refData += nameT_N + ":";

	// get cal and calc spot date
	AQLPriceDataCalendar calL;
	AQLString calLStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_CALENDAR + staticDataSuffix);
	calL.convertFromString(calLStr);
	AQLDate spotDateL;
	if (isSpotUse)
	{
		spotDateL.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_SPOTDATE + staticDataSuffix).getCString());
	}
	else
	{
		spotDateL = calL.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_RESETLAG + staticDataSuffix).getIntValue());
	}

	bool isOnSpotAdj = false;
	AQLString strIsOnSpotAdj = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_ISONFORSPOTADJUST + staticDataSuffix).toUpper();
	if (strIsOnSpotAdj != AQ_NO_DATA)
	{
		AQLDataBool tmpIsOnSpotAdj;
		tmpIsOnSpotAdj.convertFromString(strIsOnSpotAdj);
		isOnSpotAdj = tmpIsOnSpotAdj.get();
	}

	bool onValFlg = false;
	bool tnValFlg = false;

	const int liborSize = liborDataMtx.size();
	// use grid
	AQLStringVector liborUseGrid;
	AQLString tmpLiborUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix).toUpper();
	if (tmpLiborUseGrid != AQ_NO_DATA)
	{
		liborUseGrid = tmpLiborUseGrid.toToken(':');
	}
	// get eomroll
	bool isEOMRollL = false;
	AQLString strEOMRollL = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_ISEOMROLL + staticDataSuffix).toUpper();
	if (strEOMRollL != AQ_NO_DATA)
	{
		AQLDataBool tmpIsEOMRoll;
		tmpIsEOMRoll.convertFromString(strEOMRollL);
		isEOMRollL = tmpIsEOMRoll.get();
	}
	if (isEOMRollL)
	{
		AQLString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_EOMDAY + staticDataSuffix).toUpper();
		if (strEOMDay != AQ_NO_DATA)
		{
			if (spotDateL.dayOfMonth() != strEOMDay.getIntValue())
			{
				isEOMRollL = false;
			}
		}
		else
		{
			AQLDate eomDate = calL.getEOMDay(spotDateL);
			if (spotDateL != eomDate)
			{
				isEOMRollL = false;
			}
		}
	}
	for (int j = 0; j < liborSize; ++j)
	{
		AQLString term = liborDataMtx[j][0].toUpper();
		double rate = liborDataMtx[j][1].getDoubleValue();

		// get freq
		AQLString freqLStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_FREQUENCY + staticDataSuffix, term).toUpper();
		// get daycount
		AQLString daycLStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_DAYCOUNT + staticDataSuffix, term).toUpper();
		// get sliding
		AQLString slidingLStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_SLIDINGRULE + staticDataSuffix, term).toUpper();

		if (term == "ON")
		{
			mktDataO_N->add(CALIBRATION_DATA_RATE,new AQLDataDouble(rate / 100.0));
			onValFlg = true;
			continue;
		}
		else if (term == "TN")
		{
			mktDataT_N->remove(CALIBRATION_DATA_RATE);
			mktDataT_N->add(CALIBRATION_DATA_RATE,new AQLDataDouble(rate / 100.0));
			tnValFlg = true;
			continue;
		}

		AQLObject *mktData = NULL;
		AQLString nameL = yieldDataName + "_LIBOR_" + AQLString(j) + suffix_data;
		const AQLObjectHolder ehlibor = objPool.getObject(nameL);
		if (!ehlibor.isDefined())
		{
			mktData = new AQLObject();
			objPool.set(nameL, mktData);
		}
		else
		{
			objPool.getObject(nameL).get().clear();
			mktData = &objPool.getObject(nameL).get();
		}
		refData += nameL + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameL);
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateL));
		// set calendar, sliding, daycount
		if (term == "SN")
		{
			mktData->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calMStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycMStr);
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingMStr);
		}
		else
		{
			mktData->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calLStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycLStr);
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingLStr);
		}
		// is on for spot adjust
		mktData->add(IR_CALIBRATION_DATA_ISONFORSPOTADJUST, new AQLDataBool(isOnSpotAdj));
		// set data type
		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_ZERO);
		// set frequency
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freqLStr);	
		// set eomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLL, new AQLDataBool(isEOMRollL));
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(AQLMarketData::aqlConvertToTerm(term));
		//grid use
		if (liborUseGrid.size() != 0 && find(liborUseGrid.begin(), liborUseGrid.end(), term) == liborUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
		}
		else
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
		}
		// set rate
		mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate / 100.0));
		// set object pool
	}

	// if no data for ON and TN set first data
	if (!onValFlg)
	{
		mktDataO_N->add(CALIBRATION_DATA_RATE,new AQLDataDouble(firstVal / 100.0));
	}
	if (!tnValFlg)
	{
		mktDataT_N->remove(CALIBRATION_DATA_RATE);
		mktDataT_N->add(CALIBRATION_DATA_RATE,new AQLDataDouble(firstVal / 100.0));
	}

	//FRA Data
	bool isFRAUse = false;
	AQLString isFRAUse_str =  mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	AQLDataBool tmpAttrB;
	if (isFRAUse_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFRAUse_str);
		isFRAUse = tmpAttrB.get();
	}
	if (marketName != SWAP)
	{        
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_ISFRAUSE + suffix_data);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_ISFRAUSE + suffix_data, new AQLDataBool(isFRAUse));
	}
	else
	{
		ycPro.getIsFRAUse().set(isFRAUse);
	}
	if (isAudExtra && isFRAUse) throw AQLCoreInvalidData("We can not set AUD extra and FRA use at a same time!", __FILE__, __LINE__);

	//DF curve name
	AQLString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix); 
	if (dfCurveName == AQ_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	ycPro.AQLObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	ycPro.AQLObject::add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new AQLDataString(dfCurveName));
	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new AQLDataString(dfCurveName));


	// get fwd swap
	bool isFwdSwap = false;
	AQLString isFwdSwap_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + staticDataSuffix);
	if (isFwdSwap_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		isFwdSwap = tmpAttrB.get();
	}

	// Do we always recalculate the dynamic linear spline join date?
	AQLString alwaysCalcJoinDate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE + staticDataSuffix);
	ycPro.AQLObject::remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	if (alwaysCalcJoinDate != AQ_NO_DATA && alwaysCalcJoinDate.size() != 0)
	{
		if (alwaysCalcJoinDate.toUpper() == "TRUE")
		{
			ycPro.AQLObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(true));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(true));
		}
		else
		{
			ycPro.AQLObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(false));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new AQLDataBool(false));
		}
	}

	// Linear Spline interpolation cut off date
	AQLString inputLinearSplineJoinDateStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATIONJOINDATE + staticDataSuffix);
	ycPro.AQLObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	if (inputLinearSplineJoinDateStr != AQ_NO_DATA && inputLinearSplineJoinDateStr.size() != 0)
	{
		AQLDate inputLinearSplineJoinDate(inputLinearSplineJoinDateStr.getCString(), "YYYYMMDD");

		ycPro.AQLObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputLinearSplineJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new AQLDataDate(inputLinearSplineJoinDate));
	}
	
	if (isFRAUse)
	{
		// use grid
		AQLStringVector fra3UseGrid;
		AQLString tmpFra3UseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID + staticDataSuffix).toUpper();
		if (tmpFra3UseGrid != AQ_NO_DATA)
		{
			fra3UseGrid = tmpFra3UseGrid.toToken(':');
		}
		AQLStringVector fra6UseGrid;
		AQLString tmpFra6UseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID + staticDataSuffix).toUpper();
		if (tmpFra6UseGrid != AQ_NO_DATA)
		{
			fra6UseGrid = tmpFra6UseGrid.toToken(':');
		}
		
		// get market rate
		AQLString fra3FileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + staticDataSuffix);
		AQLString fra6FileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + staticDataSuffix);
		if (fra3FileName == AQ_NO_DATA || fra6FileName == AQ_NO_DATA)
		{
			throw AQLCoreInvalidData("No FRA File", __FILE__, __LINE__);
		}
		AQLFileAccessor fra3File(AQLMarketData::getNumFileName(fra3FileName));
		AQLFileAccessor fra6File(AQLMarketData::getNumFileName(fra6FileName));
		AQLStringMatrix fra3DataMtx,fra6DataMtx;
		fra3File.readAllData(MARKET_DATA_DELIMITER, fra3DataMtx);
		fra6File.readAllData(MARKET_DATA_DELIMITER, fra6DataMtx);
		fra3File.close();
		fra6File.close();
		const int fra3Size = fra3DataMtx.size();
		const int fra6Size = fra6DataMtx.size();
		// get cal
		AQLString calFRAStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_CALENDAR + staticDataSuffix);
		AQLPriceDataCalendar calFRA;
		calFRA.convertFromString(calFRAStr);
		// get spot date
		AQLDate spotDateFRA;
		if (isSpotUse)
		{
			spotDateFRA.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_SPOTDATE + staticDataSuffix).getCString());
		}
		else
		{
			spotDateFRA = calFRA.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_RESETLAG + staticDataSuffix).getIntValue());
		}
		// get daycount
		AQLString daycFRAStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_DAYCOUNT + staticDataSuffix).toUpper();
		// get sliding
		AQLString slidingFRAStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_SLIDINGRULE + staticDataSuffix).toUpper();
		// get applyTension
        bool applyTensionFRAs = false;
        AQLString applyTensionFRAsStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_APPLYTENSION + staticDataSuffix).toUpper();
        if (applyTensionFRAsStr != AQ_NO_DATA)
        {
            AQLDataBool tmpApplyTensionFRAs;
			tmpApplyTensionFRAs.convertFromString(applyTensionFRAsStr);
			applyTensionFRAs = tmpApplyTensionFRAs.get();
        }
		// get tensionGap
        int tensionGapFRAs = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_TENSIONGAP + staticDataSuffix).getIntValue();
        // get eomroll
		bool isEOMRollFRA = false;
		AQLString strEOMRollFRA = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_ISEOMROLL + staticDataSuffix).toUpper();
		if (strEOMRollFRA != AQ_NO_DATA)
		{
			AQLDataBool tmpIsEOMRoll;
			tmpIsEOMRoll.convertFromString(strEOMRollFRA);
			isEOMRollFRA = tmpIsEOMRoll.get();
		}
		if (isEOMRollFRA)
		{
			AQLString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_EOMDAY + staticDataSuffix).toUpper();
			if (strEOMDay != AQ_NO_DATA)
			{
				if (spotDateFRA.dayOfMonth() != strEOMDay.getIntValue())
				{
					isEOMRollFRA = false;
				}
			}
			else
			{
				AQLDate eomDate = calFRA.getEOMDay(spotDateFRA);
				if (spotDateFRA != eomDate)
				{
					isEOMRollFRA = false;
				}
			}
		}

		for (int i = 0; i < fra3Size; ++i)
		{
			AQLString term = fra3DataMtx[i][0].toUpper();
			double rate = fra3DataMtx[i][1].getDoubleValue()/100.0;
			//set FRA3M object
			AQLObject *mktDataFRA3M = NULL;
			AQLString nameFRA3M = yieldDataName + FRA3M + AQLString("_") + AQLString(i) + suffix_data;
			const AQLObjectHolder ehfra3m = objPool.getObject(nameFRA3M);
			if (!ehfra3m.isDefined())
			{
				mktDataFRA3M = new AQLObject();
				objPool.set(nameFRA3M, mktDataFRA3M);
			}
			else
			{
				objPool.getObject(nameFRA3M).get().clear();
				mktDataFRA3M = &objPool.getObject(nameFRA3M).get();
			}
			refData += nameFRA3M + ":";
			mktDataFRA3M->add(CALIBRATION_DATA_NAME, new AQLDataString(nameFRA3M));
			//set fwd swap
			if (isFwdSwap)
			{
				mktDataFRA3M->add(PRICING_DATA_ISFWDSWAP, new AQLDataBool(isFwdSwap) );
				if (fra3DataMtx[i].size() != 5)
					throw AQLCoreInvalidData("FRA3M(for FwdSwap) File format is wrong", __FILE__,__LINE__);
				const bool isDate = fra3DataMtx[i][2].toUpper() == "TRUE";
				mktDataFRA3M->add(PRICING_DATA_ISDATE, new AQLDataBool(isDate) );
				if (isDate)
				{
					const AQLDate startDate = AQLDate(fra3DataMtx[i][3].getCString());
					mktDataFRA3M->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
					const AQLDate endDate = AQLDate(fra3DataMtx[i][4].getCString());
					mktDataFRA3M->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
				}
				else
				{
					const AQLString startTerm = fra3DataMtx[i][3].toUpper();
					mktDataFRA3M->add(PRICING_DATA_STARTTERM, new AQLDataString(startTerm));
					const AQLString tenor = fra3DataMtx[i][4].toUpper();
					mktDataFRA3M->add(PRICING_DATA_TENOR, new AQLDataString(tenor));
				}
			}
			mktDataFRA3M->add(IR_CALIBRATION_DATA_TERM, new AQLDataString(term));
			mktDataFRA3M->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(calFRA));
			mktDataFRA3M->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingFRAStr);
			mktDataFRA3M->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateFRA));
			mktDataFRA3M->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate));
			mktDataFRA3M->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycFRAStr);
			mktDataFRA3M->add(IR_CALIBRATION_DATA_APPLYTENSION, new AQLDataBool(applyTensionFRAs));
			mktDataFRA3M->add(IR_CALIBRATION_DATA_TENSIONGAP, new AQLDataInt(tensionGapFRAs));
            mktDataFRA3M->add(IR_CALIBRATION_DATA_ISEOMROLL, new AQLDataBool(isEOMRollFRA));
			//grid use
			if (fra3UseGrid.size() != 0 && find(fra3UseGrid.begin(), fra3UseGrid.end(), term) == fra3UseGrid.end())
			{
				mktDataFRA3M->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
			}
			else
			{
				mktDataFRA3M->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
			}
			//set yield type
			mktDataFRA3M->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(FRA3M);
		}
		for (int i = 0; i < fra6Size; ++i)
		{
			AQLString term = fra6DataMtx[i][0].toUpper();
			double rate = fra6DataMtx[i][1].getDoubleValue()/100.0;
			//set FRA6M object
			AQLObject *mktDataFRA6M = NULL;
			AQLString nameFRA6M = yieldDataName + FRA6M + AQLString("_") + AQLString(i) + suffix_data;
			const AQLObjectHolder ehfra6m = objPool.getObject(nameFRA6M);
			if (!ehfra6m.isDefined())
			{
				mktDataFRA6M = new AQLObject();
				objPool.set(nameFRA6M, mktDataFRA6M);
			}
			else
			{
				objPool.getObject(nameFRA6M).get().clear();
				mktDataFRA6M = &objPool.getObject(nameFRA6M).get();
			}
			refData += nameFRA6M + ":";
			mktDataFRA6M->add(CALIBRATION_DATA_NAME, new AQLDataString(nameFRA6M));
			//set fwd swap
			if (isFwdSwap)
			{
				mktDataFRA6M->add(PRICING_DATA_ISFWDSWAP, new AQLDataBool(isFwdSwap) );
				if (fra6DataMtx[i].size() != 5)
					throw AQLCoreInvalidData("FRA6M(for FwdSwap) File format is wrong", __FILE__,__LINE__);
				const bool isDate = fra6DataMtx[i][2].toUpper() == "TRUE";
				mktDataFRA6M->add(PRICING_DATA_ISDATE, new AQLDataBool(isDate) );
				if (isDate)
				{
					const AQLDate startDate = AQLDate(fra6DataMtx[i][3].getCString());
					mktDataFRA6M->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
					const AQLDate endDate = AQLDate(fra6DataMtx[i][4].getCString());
					mktDataFRA6M->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
				}
				else
				{
					const AQLString startTerm = fra6DataMtx[i][3].toUpper();
					mktDataFRA6M->add(PRICING_DATA_STARTTERM, new AQLDataString(startTerm));
					const AQLString endTerm = fra6DataMtx[i][4].toUpper();
					mktDataFRA6M->add(PRICING_DATA_TENOR, new AQLDataString(endTerm));
				}
			}
			mktDataFRA6M->add(IR_CALIBRATION_DATA_TERM, new AQLDataString(term));
			mktDataFRA6M->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(calFRA));
			mktDataFRA6M->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingFRAStr);
			mktDataFRA6M->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateFRA));
			mktDataFRA6M->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate));
			mktDataFRA6M->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycFRAStr);
			mktDataFRA6M->add(IR_CALIBRATION_DATA_APPLYTENSION, new AQLDataBool(applyTensionFRAs));
			mktDataFRA6M->add(IR_CALIBRATION_DATA_TENSIONGAP, new AQLDataInt(tensionGapFRAs));
            mktDataFRA6M->add(IR_CALIBRATION_DATA_ISEOMROLL, new AQLDataBool(isEOMRollFRA));
			//grid use
			if (fra6UseGrid.size() != 0 && find(fra6UseGrid.begin(), fra6UseGrid.end(), term) == fra6UseGrid.end())
			{
				mktDataFRA6M->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
			}
			else
			{
				mktDataFRA6M->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
			}
			//set yield type
			mktDataFRA6M->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(FRA6M);
		}		
	}

	// future
	AQLString tmpCurrency = currency;
	tmpCurrency.toLower();
	bool isFutureUse = false;
	AQLString tmpFutureStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + staticDataSuffix);
	if (tmpFutureStr != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(tmpFutureStr);
		isFutureUse = tmpAttrB.get();
	}
	if (marketName != SWAP)
	{ 
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data, new AQLDataBool(isFutureUse));
	}
	else
	{
		ycPro.getIsFutureUse().set(isFutureUse);
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
					for (int i = 0; i < useGridNum; ++i)
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
			double futurePrice,rate,futureVol;

			if (futureDataMtx[i].size() == 5)
			{
				term = futureDataMtx[i][0].toUpper();
				startDate = AQLDate(futureDataMtx[i][1].getCString());
				endDate = AQLDate(futureDataMtx[i][2].getCString());
				futurePrice = futureDataMtx[i][3].getDoubleValue();
				rate = 1.0 - futurePrice * 0.01;
				futureVol = futureDataMtx[i][4].getDoubleValue();
			}
			else if (futureDataMtx[i].size() == 3)
			{
				term = futureDataMtx[i][0].toUpper();
				startDate = AQLDateCalculations::getIMMDateFromTerm(asOfDate, term);
				startDate = AQLDateCalculations::getDate(startDate, "0d", slidingF, &calF, true);
				endDate = AQLDateCalculations::getDate(startDate, "3M", slidingF, &calF, true);
				int mm = endDate.monthOfYear();
				int yy = endDate.yearOfEra();
				endDate = AQLDateCalculations::getIMMDate(yy, mm, true);
				
				futurePrice = futureDataMtx[i][1].getDoubleValue();
				rate = 1.0 - futurePrice * 0.01;
				futureVol = futureDataMtx[i][2].getDoubleValue();
			}
			else
				throw AQLCoreInvalidData("Future File format is wrong", __FILE__,__LINE__);
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
		}
	}
	
	// swap
	AQLString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	AQLFileAccessor swapFile(AQLMarketData::getNumFileName(swapFileName));
	AQLStringMatrix swapDataMtx;
	//if (!isFwdFX)
	//{
		swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
		swapFile.close();
	//}

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
				ycPro.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
				ycPro.AQLObject::add(CALIBRATION_DATA_MARKETDATA + suffix_data, new AQLDataMultiReference()).
					convertFromString(refData.subString(0, refData.size() - 2));
			}
			else
			{
				ycPro.getMarketData().convertFromString(refData.subString(0, refData.size() - 2));
			}
			
			// generate yield data
			AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
								(ycPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			ycPro.AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
			ycPro.AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(marketName));

			dataInstance.getReferencePool().completeDependency();
			modelDataObj.calibrateModel(asOfDate);
			
			//reset market data as libor
			AQLMarketData::resetMarketDataUseL(ycPro, currency, &marketName);
			resetFlg = false;
			if (marketName != SWAP)
			{
				ycPro.AQLObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
			}
		}

		double rate = swapDataMtx[j][1].getDoubleValue();
		// get freq
		AQLString freqSStr = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + staticDataSuffix, term).toUpper();
		AQLString freqSStr_Fix = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + staticDataSuffix).toUpper();
		// get frequency of floating leg
		AQLString baseFreqSStr_float;
		baseFreqSStr_float = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT + staticDataSuffix).toUpper();
		if (baseFreqSStr_float == AQ_NO_DATA) baseFreqSStr_float = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + staticDataSuffix).toUpper();
		AQLString freqSStr_float = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + staticDataSuffix, term).toUpper();
		// get frequency of compounding
		AQLString freqSStr_cpd = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYCOMPOUND + staticDataSuffix).toUpper();
		// get daycount
		AQLString daycSStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT + staticDataSuffix, term).toUpper();
		// get daycount of floating leg
		AQLString daycSStr_float = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT + staticDataSuffix, term).toUpper();
		// get sliding
		AQLString slidingSStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE + staticDataSuffix, term).toUpper();
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
		// iseomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLLSW, new AQLDataBool(isEOMRollSW));
		// simultaneous equation
		mktData->add(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQSW, new AQLDataBool(isSimuEQSW));
		// is frequency change
		mktData->add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, new AQLDataBool(isSwapTenorAdjust) );
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
		throw AQLCoreInvalidData("Market Data is not set !!", __FILE__, __LINE__); 
	}

	AQLString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix);
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
		for (size_t i = 0; i<assignedCurves.size(); i++)
		{
			ycPro.setAssignedCurveMktMap(assignedCurves[i],marketName);
		}
	}
	else
	{
		if (marketName != SWAP) 
		{
			ycPro.setAssignedCurveMktMap(marketName,marketName);
		}
		else
		{
			ycPro.setAssignedCurveMktMap(STD,marketName);
		}
	}

	// remove curve generate map
	const AQLString isPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<AQLString, bool>& gCurveMap = ycPro.getGCurveGenerateMap();
		const std::map<AQLString, AQLString>& assignedCurveMktMap = ycPro.getAssignedCurveMktMap();
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
AQLCalibrateModelIR::setUpGenCurveDataOIS(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
									const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse,
									bool isAudExtra, AQLMathYieldCurvePro &ycPro, 
									std::map<AQLString, std::map<AQLString, double> > &aud_origSwapRate) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	bool isPricer = false;
	AQLString strIsPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (strIsPricer == "TRUE") isPricer = true;

	AQLString suffix = "." +  marketName;
	suffix.toLower();

	AQLString suffix_data = "_" +  marketName;

	AQLString yeildGenInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
	if (yeildGenInter != AQ_NO_DATA) 
	{
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data, new AQLPriceDataInterpolation()).convertFromString(yeildGenInter);
	}

	AQLString oisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffix);
	AQLFileAccessor oisFile(AQLMarketData::getNumFileName(oisFileName));
	AQLStringMatrix oisDataMtx;
	oisFile.readAllData(MARKET_DATA_DELIMITER, oisDataMtx);

	oisFile.close();

	if (oisDataMtx.size() == 0 || oisDataMtx[0].size() < 2 )
	{
		throw AQLCoreInvalidData("OisFile is empty", __FILE__,__LINE__);
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
	AQLString str_shortTerm =  mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERM + suffix);
	AQLDate shortTermDate;
	if (str_shortTerm != AQ_NO_DATA)
	{
		shortTermDate = calOIS.getBusinessDay(asOfDate, str_shortTerm.getIntValue());
	}

	AQLString shortTermConv = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION + suffix).toUpper();
	AQLString firstRate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FIRSTRATE + suffix).toUpper();

	const int oisSize = oisDataMtx.size();
	// use grid
	AQLStringVector oisUseGrid;
	AQLString tmpOISUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffix).toUpper();
	if (tmpOISUseGrid != AQ_NO_DATA)
	{
		oisUseGrid = tmpOISUseGrid.toToken(':');
	}

	const AQLString longTermConv = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffix).toUpper();
	AQLString longTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffix).toUpper();
	AQLString loBasisName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + suffix).toUpper();
	AQLDate date_lt;
	if (longTermConv == LOBASIS)
	{
		loBasisName == AQ_NO_DATA ? LOBASIS : loBasisName;
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_LOBASISNAME + suffix_data);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_LOBASISNAME + suffix_data, new AQLDataString(loBasisName));
		date_lt = AQLDateCalculations::getDate(asOfDate, longTerm, true);
	}

	unsigned int j2 = 0;
	for (int i = 0; i < oisSize; ++i)
	{
		AQLObject *mktData = NULL;
		AQLString nameOIS = yieldDataName + "_OIS_" + AQLString(i) + "_" + marketName;
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

		refData += nameOIS + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameOIS);
		
		AQLDate startDate, endDate;
		AQLString term = oisDataMtx[i][0].toUpper();
		if (term.findString("BOJ") >= 0 || term.findString("EUSF") >= 0)
		{
			if (oisDataMtx[i].size() != 4)
			{
				throw AQLCoreInvalidData("OisFile format is wrong", __FILE__,__LINE__);
			}
			// boj type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BOJ);
			startDate = AQLDataDate(oisDataMtx[i][2]).get();
			endDate = AQLDataDate(oisDataMtx[i][3]).get();
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
		}
		else
		{
			// swap type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_PAR);
		}
		
		double rate = oisDataMtx[i][1].getDoubleValue();

		////get if its compounding or daily averaging
		AQLString generatemethod = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix).toUpper();

		////set if its compounding or daily averaging
		mktData->add(IR_CALIBRATION_DATA_GENERATEMETHOD, new AQLDataString()).convertFromString(generatemethod);	

		// get freq
		AQLString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY + suffix, term).toUpper();
		// get daycount
		AQLString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT + suffix, term).toUpper();
		// get sliding
		AQLString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE + suffix, term).toUpper();
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
		if (oisUseGrid.size() != 0 && find(oisUseGrid.begin(), oisUseGrid.end(), term) == oisUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
		}
		else
		{
			if (longTermConv != LOBASIS)
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
			else
			{
				if (term.findString("BOJ") >= 0 || term.findString("EUSF") >= 0)
					// In short term
					mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
				else
				{
					bool IsInLongTerm = (AQLDateCalculations::getDate(asOfDate, term, true) >= date_lt);
					if (!IsInLongTerm)
						// In middle term
						mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
					else
						// In long term
						mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
				}
			}
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
				for (int i = 0; i < useGridNum; ++i)
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
		AQLString nameOIS = yieldDataName + "_OIS_" + AQLString(oisSize + i) + "_" + marketName;
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

		refData += nameOIS + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(nameOIS);
		
		AQLDate startDate, endDate;
		AQLString term = fedFundFutureDataMtx[i][0].toUpper();

		if (fedFundFutureDataMtx[i].size() < 2)
		{
			throw AQLCoreInvalidData("FF Future File format is wrong", __FILE__,__LINE__);
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
			DateVector ffdates = AQLDateCalculations::getFFDatesFromTerm(asOfDate,term);
			if (ffdates.size() != 2)
				throw AQLCoreInvalidData("FF dates error",__FILE__,__LINE__);

			startDate = ffdates[0];
			endDate = ffdates[1];
			mktData->add(PRICING_DATA_STARTDATE, new AQLDataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new AQLDataDate(endDate));
		}
		
		double rate = fedFundFutureDataMtx[i][1].getDoubleValue();
		rate  = 100. - rate;

		// get freq
		AQLString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY + suffix, term).toUpper();
		// get daycount
		AQLString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT + suffix, term).toUpper();
		// get sliding
		AQLString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE + suffix, term).toUpper();
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
			AQLFileAccessor oisHistFile(AQLMarketData::getNumFileName(oisHistFileName));
			AQLStringMatrix oisHistDataMtx;
			oisHistFile.readAllData(MARKET_DATA_DELIMITER, oisHistDataMtx);
			oisHistFile.close();

			if (oisHistDataMtx.size() == 0 || oisHistDataMtx[0].size() < 2 )
			{
				throw AQLCoreInvalidData("OIS / ARR fixing data is required and empty", __FILE__,__LINE__);
			}

			DateVector histdates;
			DoubleVector histrates;
			for (unsigned int j = 0; j < oisHistDataMtx.size(); j++)
			{
				histdates.push_back(AQLDataDate(oisHistDataMtx[j][0]).get());
				histrates.push_back(oisHistDataMtx[j][1].getDoubleValue() * 0.01);
			}
			mktData->add(IR_CALIBRATION_DATA_HISTORICALDATES, new AQLDataDates(histdates));
			mktData->add(IR_CALIBRATION_DATA_HISTORICALRATES, new AQLDataDoubles(histrates));
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
	ycPro.AQLObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName);
	ycPro.AQLObject::add(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName, new AQLDataString(dfCurveName));
	AQLObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + marketName, new AQLDataString(dfCurveName));

	//const std::map<AQLString, AQLString>& assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
	AQLString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix);
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		AQLStringVector assignedCurves = tmpAssignedCurves.toToken(':');
		for (size_t i = 0; i<assignedCurves.size(); i++)
		{
			ycPro.setAssignedCurveMktMap(assignedCurves[i],marketName);
		}
	}
	else
	{
		ycPro.setAssignedCurveMktMap(marketName,marketName);
	}

	// remove curve generate map
	if (isPricer)
	{
		std::map<AQLString, bool>& gCurveMap = ycPro.getGCurveGenerateMap();
		const std::map<AQLString, AQLString>& assignedCurveMktMap = ycPro.getAssignedCurveMktMap();
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second == marketName)
			{
				if (gCurveMap.find(it->first) != gCurveMap.end()) gCurveMap.erase(it->first);
			}
		}
		//if (gCurveMap.find(marketName) != gCurveMap.end()) gCurveMap.erase(marketName);
	}
}

void 
AQLCalibrateModelIR::setUpFloater(const AQLString &currency, AQLMathYieldCurvePro &ycPro, const AQLString &genFloaterName) const
{
	AQLStringVector markets = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	uppervec(markets);
	if (genFloaterName != AQ_NO_DATA)
	{
		ycPro.setAssignedCurveMktMap(genFloaterName, genFloaterName);
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_FLOATERDFS);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_FLOATERDFS, new AQLDataString(genFloaterName));
		AQLString tmpGenFloaterName = genFloaterName;
		tmpGenFloaterName.toLower();
		AQLString basisMkt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME);
		if (basisMkt != AQ_NO_DATA)
		{
			if (std::find(markets.begin(), markets.end(), basisMkt) == markets.end())
			{
				throw AQLCoreInvalidData("Basis market does not exist!", __FILE__, __LINE__);
			}
			ycPro.AQLObject::remove(IR_CALIBRATION_DATA_BASISDATA + AQLString("_") + tmpGenFloaterName);
			ycPro.AQLObject::add(IR_CALIBRATION_DATA_BASISDATA + AQLString("_") + tmpGenFloaterName, new AQLDataString(basisMkt));
		}
		AQLString discountName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT);
		AQLString forecastName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FORECAST);
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_FORECAST + AQLString("_") + tmpGenFloaterName);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_FORECAST + AQLString("_") + tmpGenFloaterName, new AQLDataString(forecastName));
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_DISCOUNT + AQLString("_") + tmpGenFloaterName);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_DISCOUNT + AQLString("_") + tmpGenFloaterName, new AQLDataString(discountName));

		/*AQLString isFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_ISFWDINTER).toUpper();
		if (isFWDInter == "TRUE")
		{
			ycPro.AQLObject::remove(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + AQLString("_") + tmpGenFloaterName);
			ycPro.AQLObject::add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + AQLString("_") + tmpGenFloaterName, new AQLDataBool(true));
			AQLString fwdInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FWDINTERPOLATION).toLower();
			ycPro.AQLObject::remove(IR_CALIBRATION_DATA_FWDINTERPOLATION + AQLString("_") + tmpGenFloaterName);
			ycPro.AQLObject::add(IR_CALIBRATION_DATA_FWDINTERPOLATION + AQLString("_") + tmpGenFloaterName, new AQLPriceDataInterpolation()).convertFromString(fwdInter);;
		}*/
		
		ycPro.setFloater(genFloaterName);
	}
}

void
AQLCalibrateModelIR::setUpCurveDataByReadFile( AQLDataInstance &dataInstance, const AQLDate& asOfDate, const AQLString& currency, 
										    const AQLString& marketName, const AQLString& yieldDataName, AQLMathYieldCurvePro &ycPro ) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLObjectHolder objHolder = objPool.getObject(yieldDataName, ENCHKTYPE_NOCHECK );
	if (!objHolder.isDefined() )
		throw AQLCoreInvalidData("yield Object is not set! AQLCalibrateModelIR::setUpCurveDataByReadFile", __FILE__, __LINE__ );
	
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
			ycPro.setAssignedCurveMktMap(assignedCurves[i],marketName);

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
		ycPro.setAssignedCurveMktMap(marketName,marketName);
	}

	ycPro.insertNonRemovableMarket(marketName);
}

void 
AQLCalibrateModelIR::dataoutCurve(const AQLStringVector &curveNames, AQLObject &eData, const AQLString &yieldDataName) const
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
AQLCalibrateModelIR::checkFrequency(const AQLString& freq, const AQLString& mktRateTerm) const
{
	int span = AQLDateCalculations::getPeriodFrequencyInMonths(freq);

	int y, m, d, w;
	AQLDateCalculations::termStrtoYMDW(mktRateTerm, y, m, d, w);
	int moth_mkt_term = 12 * y + m;

	return (moth_mkt_term % span) == 0;
}

///// update for XLL Plus //////////////////////////
#include "AQLCalibrateModelIRVanilla.h"

void
AQLCalibrateModelIR::generateInitialValueForPricer(const AQLString &currency, AQLDataInstance &dataInstance) const
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
	AQLMathYieldCurvePro *ycPro = NULL;
	const AQLObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		ycPro = new AQLMathYieldCurvePro(&dataInstance);
		objPool.set(yieldProName, ycPro);
	}
	else
	{
		//we must not erase the reset method for only ycpro
		ycPro = &dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(yieldProName).get());
	}
	ycPro->getName().convertFromString(yieldProName);

	ycPro->getIsArbFree().set(false);

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
	
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *ycPro, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, false);
	setUpCurveDataByContext(*ycPro,*yc,eData,currency,SWAP);


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
					AQLCalibrateModelIRVanilla generator(currency);
					generator.loadModelDataAndCalibrate(currency, dataInstance, true);
					mpStaticData->removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
					AQLCoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD,useYieldSDEIRStr);
				}
			}
			setUpCurveDataByContext(*ycPro,*yc,eData,currency,markets[i]);
		}
	}
}

void 
AQLCalibrateModelIR::setUpCurveDataByContext(AQLMathYieldCurvePro &ycPro, AQLMathYieldCurve &yc, AQLObject *eData, const AQLString& currency, const AQLString& marketName ) const
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

	// set yield data
	AQLString contextAttrKey = prefix+CONTEXT_KEY_SDE_YIELD_DATA+suffix;
	AQLString contextYieldAttr = AQLCoreDataService::getContext(contextAttrKey);
	if (contextYieldAttr != AQ_NO_DATA)
	{
		AQLDataStringMatrix attrStringMatrx;
		attrStringMatrx.convertFromString(contextYieldAttr);

		const AQLStringMatrix& strMatrix = attrStringMatrx.get();
		if (strMatrix.size() != 0 && strMatrix[0].size() == 2)
		{
			for (size_t i = 0; i < strMatrix.size(); ++i)
			{
				if (strMatrix[i][0] == CALIBRATION_DATA_CALENDAR) 
				{
					eData->remove(CALIBRATION_DATA_CALENDAR + data_suffix);
					eData->add(CALIBRATION_DATA_CALENDAR + data_suffix, new AQLPriceDataCalendar()).convertFromString(strMatrix[i][1]);
				}
				else if (strMatrix[i][0] == CALIBRATION_DATA_SLIDINGRULE) 
				{
					eData->remove(CALIBRATION_DATA_SLIDINGRULE + data_suffix);
					eData->add(CALIBRATION_DATA_SLIDINGRULE + data_suffix, new AQLPriceDataSlidingRule()).convertFromString(strMatrix[i][1]);
				}
				else if (strMatrix[i][0] == IR_CALIBRATION_DATA_DAYCOUNT) 
				{
					eData->remove(IR_CALIBRATION_DATA_DAYCOUNT + data_suffix);
					eData->add(IR_CALIBRATION_DATA_DAYCOUNT + data_suffix, new AQLPriceDataDayCount()).convertFromString(strMatrix[i][1]);
				}
				else if (strMatrix[i][0] == IR_CALIBRATION_DATA_ACCESSARY) 
				{
					eData->remove(IR_CALIBRATION_DATA_ACCESSARY + data_suffix);
					eData->add(IR_CALIBRATION_DATA_ACCESSARY + data_suffix, new AQLDataString(strMatrix[i][1]));
				}
				else if (strMatrix[i][0] == IR_CALIBRATION_DATA_FREQUENCY) 
				{
					eData->remove(IR_CALIBRATION_DATA_FREQUENCY + data_suffix);
					eData->add(IR_CALIBRATION_DATA_FREQUENCY + data_suffix, new AQLDataString(strMatrix[i][1]));
				}
				else if (strMatrix[i][0] == CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE)
				{
					eData->remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + data_suffix);
					eData->add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + data_suffix, new AQLDataDouble(strMatrix[i][1].getDoubleValue()));
				}
			}
		}
	}

	// set yield interpolation
	AQLString contextInterKey = prefix+CONTEXT_KEY_SDE_YIELD_INTER+suffix;
	AQLString contextYieldInter = AQLCoreDataService::getContext(contextInterKey);
	if (contextYieldInter != AQ_NO_DATA)
	{
		yc.getInterpolation().convertFromString(contextYieldInter);
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
	AQLDataHolder *dh = &(ycPro.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		gCurveNames = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
	}
	gCurveNames.push_back(marketName);
	ycPro.AQLObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	ycPro.AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(gCurveNames));
}
////////////////////////////////////////////////////

void
AQLCalibrateModelIR::setUpLiborOISBasisCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
							                  const AQLString &marketName, const AQLString &marketNameOIS, const AQLString &yieldDataName, bool isSpotUse, AQLMathYieldCurvePro &ycPro) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	bool isPricer = false;
	AQLString strIsPricer = AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (strIsPricer == "TRUE") isPricer = true;

	AQLString suffix_ois = "." +  marketNameOIS;
	suffix_ois.toLower();

	AQLString longTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffix_ois).toUpper();
	AQLDate date_lt;
	AQLString suffix_lo;
	unsigned int LoBasisLiborLegNo;
	unsigned int LoBasisOISLegNo;

	AQLStringVector loBasisUseGrid;
	AQLStringMatrix lobasisDataMtx;
	if (longTerm == AQ_NO_DATA)
	{
		throw AQLCoreInvalidData("LongTerm property is needed", __FILE__,__LINE__);
	}

	suffix_lo = AQLString("." + marketName).toLower();
	date_lt = AQLDateCalculations::getDate(asOfDate, longTerm, true);
	AQLString lobasisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FILE + suffix_lo);
	AQLFileAccessor lobasisFile(AQLMarketData::getNumFileName(lobasisFileName));
	lobasisFile.readAllData(MARKET_DATA_DELIMITER, lobasisDataMtx);
	lobasisFile.close();
	if (lobasisDataMtx.size() == 0 || lobasisDataMtx[0].size() < 2 )
	{
		throw AQLCoreInvalidData("LOBasisFile is empty", __FILE__,__LINE__);
	}

	const AQLString isLeg1Type = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_TYPE + suffix_lo).toUpper();
	const AQLString isLeg2Type = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_TYPE + suffix_lo).toUpper();
	AQLString liborForecastInfoStr;
	if (isLeg1Type == "OIS" && isLeg2Type == "LIBOR")
	{
		LoBasisLiborLegNo = 2;
		LoBasisOISLegNo = 1;
		
		liborForecastInfoStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST + suffix_lo);
	}
	else if (isLeg1Type == "LIBOR" && isLeg2Type == "OIS")
	{
		LoBasisLiborLegNo = 1;
		LoBasisOISLegNo = 2;
		
		liborForecastInfoStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST + suffix_lo);
	}
	else
	{
		throw AQLCoreInvalidData("Combination of index Types are not consistent with Libor-OIS Basis swap.", __FILE__,__LINE__);
	}

	// When there does not exist forecast information in ir.properteis, 
	// we will contine the curve generation with the assumation that base curve will be set as forecast curve for libor leg.
	// This is temporal implementation for backward capability.
	// Ideally, The error should be thrown when we cannot find forecast information.
	if (liborForecastInfoStr == AQ_NO_DATA)
	{
		liborForecastInfoStr = STD;
	}

	bool isLeg1Spread = (mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ISLEG1SPREAD + suffix_lo).toUpper() == "TRUE");
	bool isSpreadOnOISLeg = ((isLeg1Spread && LoBasisOISLegNo == 1) || (!isLeg1Spread && LoBasisOISLegNo == 2));
	if (!isSpreadOnOISLeg)
	{
		throw AQLCoreInvalidData("Curve generation is implemented only for the case spread of Libor-OIS Basis is on OIS leg.", __FILE__,__LINE__);			
	}
	// use grid
	AQLString tmp = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + suffix_lo).toUpper();
	if (tmp != AQ_NO_DATA)
	{
		loBasisUseGrid = tmp.toToken(':');
	}

	// generate method, calendar and spot date
	AQLString longTermGen;
	AQLPriceDataCalendar calLOBasis;
	AQLDate spotDateLOBasis;
	if (LoBasisOISLegNo == 1)
	{
		longTermGen = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_GENERATEMETHOD + suffix_lo).toUpper();
		calLOBasis = (mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_CALENDAR + suffix_lo).toToken(MULTI_STATIC_DATA_DELIMITER));
		if (isSpotUse)
			spotDateLOBasis.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SPOTDATE + suffix_lo).getCString());
		else
			spotDateLOBasis = calLOBasis.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SPOTLAG + suffix_lo).getIntValue());
	}
	else
	{
		longTermGen = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_GENERATEMETHOD + suffix_lo).toUpper();
		calLOBasis = (mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_CALENDAR + suffix_lo).toToken(MULTI_STATIC_DATA_DELIMITER));
		if (isSpotUse)
			spotDateLOBasis.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SPOTDATE + suffix_lo).getCString());
		else
			spotDateLOBasis = calLOBasis.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SPOTLAG + suffix_lo).getIntValue());
	}

	if (longTermGen == AQ_NO_DATA)
	{
		longTermGen = AQLString("DAILYAVERAGING");
	}


	// set falg if affect base and ois curve (for risk calculation)
	const AQLString attrSuffix_lo("_" + marketName);
	ycPro.AQLObject::remove(IR_CALIBRATION_DATA_ISAFFECTINGBASECURVE + attrSuffix_lo);
	ycPro.AQLObject::add(IR_CALIBRATION_DATA_ISAFFECTINGBASECURVE + attrSuffix_lo, new AQLDataBool(true));

	// get swap market data 
	AQLString suffix_s("");
	AQLString attrSuffix_s("");
	AQLStringMatrix swapDataMtx;
	if (isPricer)
	{
		suffix_s = suffix_ois;
		AQLString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix_s);
		AQLFileAccessor swapFile(AQLMarketData::getNumFileName(swapFileName));
		swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
		swapFile.close();

		if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2 )
		{
			throw AQLCoreInvalidData("SwapFile is empty", __FILE__,__LINE__);
		}
	}
	else
	{
		std::vector<AQLString> forcastInfo(2);
		convertCurveName(liborForecastInfoStr, currency, forcastInfo[0], forcastInfo[1], isPricer, objPool);
		AQLString equivSwapMarket(ycPro.getMarketForCurve(forcastInfo[1]));
		if (equivSwapMarket == IR_NO_DATA)
		{
			throw AQLCoreInvalidData("There does not exist swap market infomration for converting a libor floating leg in Libor-OIS swap.", __FILE__,__LINE__);
		}
		if (equivSwapMarket != SWAP)
		{
			suffix_s = "." + equivSwapMarket;
			suffix_s.toLower();
			attrSuffix_s = "_" + equivSwapMarket;
		}
	}

	for (int i = 0; i < lobasisDataMtx.size(); ++i)
	{
		// get term & check
		AQLString term = lobasisDataMtx[i][0];
		const AQLDate date = AQLDateCalculations::getDate(asOfDate, term, true);
		bool isLongTerm = true;
		if (date < date_lt)
		{
			// In short or middle term
			isLongTerm = false;
		}

		// create object
		AQLObject *mktData = NULL;
		AQLString tmpCurrency = currency;
		tmpCurrency.toUpper();
		AQLString nameLOBasis = yieldDataName + "_" + marketName + "_" + AQLString(i);
		const AQLObjectHolder ehLOBasis = objPool.getObject(nameLOBasis);
		if (!ehLOBasis.isDefined())
		{
			mktData = new AQLObject();
			objPool.set(nameLOBasis, mktData);
		}
		else
		{
			objPool.getObject(nameLOBasis).get().clear();
			mktData = &objPool.getObject(nameLOBasis).get();
		}

		refData += nameLOBasis + ":";

		// get lobasis data
		double rate_lo = lobasisDataMtx[i][1].getDoubleValue();
		AQLString freqLOStr;
		AQLString daycLOStr;
		AQLString slidingLOStr;
		if (LoBasisOISLegNo == 1)
		{
			freqLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCY + suffix_lo, term).toUpper();
			daycLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_DAYCOUNT + suffix_lo, term).toUpper();
			slidingLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SLIDINGRULE + suffix_lo, term).toUpper();
		}
		else
		{
			freqLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCY + suffix_lo, term).toUpper();
			daycLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_DAYCOUNT + suffix_lo, term).toUpper();
			slidingLOStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SLIDINGRULE + suffix_lo, term).toUpper();
		}
			
		// check if libor leg can be converted to fixed leg
		const AQLString freqSwapBaseStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + suffix_s, term).toUpper();
		AQLString freqSwapFloatStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + suffix_s, term).toUpper();
		if (freqSwapFloatStr == AQ_NO_DATA) freqSwapFloatStr = freqSwapBaseStr;

		const AQLString daycSwapBaseStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT + suffix_s, term).toUpper();
		AQLString daycSwapFloatStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT + suffix_s, term).toUpper();
		if (daycSwapFloatStr == AQ_NO_DATA) daycSwapFloatStr = daycSwapBaseStr;

		AQLString freqLOLiborStr;
		AQLString daycLOLiborStr;
		if (LoBasisLiborLegNo == 1)
		{
			freqLOLiborStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCY + suffix_lo, term).toUpper();
			daycLOLiborStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_DAYCOUNT + suffix_lo, term).toUpper();
		}
		else
		{
			freqLOLiborStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCY + suffix_lo, term).toUpper();
			daycLOLiborStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_DAYCOUNT + suffix_lo, term).toUpper();
		}
		if (freqSwapFloatStr != freqLOLiborStr || daycSwapFloatStr != daycLOLiborStr)
		{
			throw AQLCoreInvalidData("Libor leg convension of Libor-OIS Basis is not consistent with IRS convention", __FILE__,__LINE__);
		}

		// set long term rate convention
		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BASIS);
		mktData->add(IR_CALIBRATION_DATA_TERM, new AQLDataString()).convertFromString(term);
		mktData->add(IR_CALIBRATION_DATA_GENERATEMETHOD, new AQLDataString()).convertFromString(longTermGen);
		// set lobasis data
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDateLOBasis));
		mktData->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate_lo / 10000.0));
		mktData->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(calLOBasis));
		mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycLOStr);
		mktData->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingLOStr);
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freqLOStr);	
		//grid use
		if (loBasisUseGrid.size() != 0 && find(loBasisUseGrid.begin(), loBasisUseGrid.end(), term) == loBasisUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
			continue;
		}
		else
		{
			if (!isLongTerm)
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(false));
				continue;
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool(true));
			}
		}
		// set swap data
		if (isPricer)
		{
			unsigned int j2 = 0;
			while (j2 < swapDataMtx.size())
			{
				const AQLString term_s = swapDataMtx[j2][0].toUpper();
				if (term_s == term)
				{
					break;
				}
				++j2;
			}
			if (j2 >= swapDataMtx.size())
			{
				throw AQLCoreInvalidData("can't find the LIBOR Swap rate which is consistent with OIS rate", __FILE__,__LINE__);
			}

			// get swap data
			double rate_s = swapDataMtx[j2][1].getDoubleValue();
			AQLString calSwapStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + suffix_s);
			
			AQLString freqSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFIX + suffix_s, term).toUpper();
			if (freqSwapStr == AQ_NO_DATA) freqSwapStr = freqSwapBaseStr;
			
			const AQLString daycSwapBaseStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT + suffix_s, term).toUpper();
			AQLString daycSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFIX + suffix_s, term).toUpper();
			if (daycSwapStr == AQ_NO_DATA) daycSwapStr = daycSwapBaseStr;

			AQLString slidingSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE + suffix_s, term).toUpper();


			// create object
			AQLObject *mktDataIRS = NULL;
			AQLString nameIRSForLOBasis = yieldDataName + "_IRS_FOR_LOBASIS_" + marketName + "_" + AQLString(i);
			const AQLObjectHolder& ehIRSForLOBasis = objPool.getObject(nameIRSForLOBasis);
			if (!ehIRSForLOBasis.isDefined())
			{
				mktDataIRS = new AQLObject();
				objPool.set(nameIRSForLOBasis, mktDataIRS);
			}
			else
			{
				objPool.getObject(nameIRSForLOBasis).get().clear();
				mktDataIRS = &objPool.getObject(nameIRSForLOBasis).get();
			}

			mktDataIRS->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate_s / 100.0));
			mktDataIRS->add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calSwapStr);
			mktDataIRS->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(daycSwapStr);
			mktDataIRS->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingSwapStr);
			mktDataIRS->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freqSwapStr);

			mktData->add(CALIBRATION_DATA_MARKETDATA_EXT, new AQLDataReference()).convertFromString(nameIRSForLOBasis);

		}
		else
		{
			// We will create a refrence to base market data below.
			// We must finish the setup of base market data (IRS data) before invoking this function.
			const AQLDataMultiReference& mktDataRefIRS = dynamic_cast<const AQLDataMultiReference &>(ycPro.getData(CALIBRATION_DATA_MARKETDATA + attrSuffix_s, ISNOTNULL).get());
			if (mktDataRefIRS.getSize() == 0)
			{
				throw AQLCoreInvalidData("There not exist IRS market data in yield curve pro object.", __FILE__,__LINE__);
			}

			// Search position of target grid
			unsigned int j2 = 0;
			while (j2 < mktDataRefIRS.getSize())
			{
				const AQLObject* mktDataIRS = &mktDataRefIRS.get(j2).get(); 
				const AQLString& data_type_s = dynamic_cast<const AQLDataString &>(mktDataIRS->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get());
				if (data_type_s == PAR)
				{
					const AQLString& term_s = dynamic_cast<const AQLDataString &>(mktDataIRS->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get());
					if (term_s == term)
					{
						break;
					}
				}
				++j2;
			}
			if (j2 >= mktDataRefIRS.getSize())
			{
				throw AQLCoreInvalidData("can't find the LIBOR Swap rate which is consistent with OIS rate", __FILE__,__LINE__);
			}

			// Create reference
			AQLObjectHolder& ehIRSForLOBasis = mktDataRefIRS.get(j2);
			mktData->add(CALIBRATION_DATA_MARKETDATA_EXT, new AQLDataReference(&ehIRSForLOBasis));
		}
	}
}


