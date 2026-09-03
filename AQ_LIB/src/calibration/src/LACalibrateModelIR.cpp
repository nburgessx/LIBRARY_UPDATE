#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <fstream>
#include "LATime.h"
#include "LADataInstance.h"
#include "LAFunctionManager.h"
#include "LAPriceDataManager.h"
#include "LADataReference.h"
#include "LADataMatrix.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LARatesCurveLinearInterpolation.h"
#include "LAStaticData.h"
#include "LADealUtils.h"
#include "LACalibrateModelIR.h"
#include "LADataVector.h"
#include "LADataProcedure.h"
#include "LAMathYieldCurve.h"
#include "LAPriceYieldGenerator.h"
#include "LACompoundingFunc.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataFunction.h"
#include "LAMarketData.h"
#include "LAPriceArbFreeGenerator.h"
#include "LAFunctionUtilities.h"
#include "LAMathDateCalculations.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS 

#include "LACoreDataService.h"
#include "LARatesTermStructureSDE.h"
#include "LARatesLJTermStructureSDE.h"
#include "LAMathCorrelation.h"
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
LACalibrateModelIR::LACalibrateModelIR(const LAString &baseCurrency)
: LACalibrateModel(), mBaseCurrency(baseCurrency)
{
}

// destructor
/*!

*/
LACalibrateModelIR::~LACalibrateModelIR(void)
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
LACalibrateModelIR::setInterpolationMethod(const LAString &currency, LARatesSDEBase &sde) const
{
	(void)currency;
	sde.setInterpolationMethod(new LARatesCurveLinearInterpolation());
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
LACalibrateModelIR::loadModelDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, const bool isCurve, const bool isModel, const LAString & curveID, const LAString & marketName) const
{
	clock_t cstart, cend;
	double time;
	if (isCurve)
	{
	// generate initial curve
cout << "[ Currency = " << currency << " ]" << endl;
cout << "LACalibrateModelIR generate curve called.." << endl;
cstart = clock();
		loadYieldCurveDataAndCalibrate(currency, dataInstance);
cend = clock();
time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LACalibrateModelIR generate curve end.." << endl;
cout << "-> time = " << time << endl;
	}

///// update for XLL Plus //////////////////////////
	//if (isOnlyCurve) return;
////////////////////////////////////////////////////
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	if (isModel)
	{
	// generate correlation
cout << "LACalibrateModelIR generate correlation called.." << endl;
cstart = clock();
		loadCorrelationDataAndCalibrate(currency, dataInstance);
cend = clock();
time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LACalibrateModelIR generate correlation end.." << endl;
cout << "-> time = " << time << endl;
	// generate volatility
cout << "LACalibrateModelIR generate volatility called.." << endl;
cstart = clock();
		loadVolatilityDataAndCalibrate(currency, dataInstance);
cend = clock();
time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LACalibrateModelIR generate volatility end.." << endl;
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
LACalibrateModelIR::loadFwdFXConstCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant) const
{
	if (!isFwdFXConst(currency))
		return;
	LAString ccy = currency; ccy.toLower();

///// update for XLL Plus in grid //////////////////
	LAString useYieldSDEIRStr = LACoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
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
	LAString suffix = "." + market.toLower();
	LAString target = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_BASIS_TARGET + suffix).toUpper();
	if (target == LEG1FORECAST || target == LEG2FORECAST)
	{
		throw LACoreInvalidData("fwdfx constant curve must be discount curve!", __FILE__, __LINE__);
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
		throw LACoreInvalidData("target curve is invalid!", __FILE__, __LINE__);
	}
	convertCurveName(strFCurve, ccy, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(strDCurve, ccy, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(strA_fCurve, ccy, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(strA_dCurve, ccy, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	if (ccy_dCurve != ccy || ccy_fCurve != ccy || ccy_a_dCurve != ccy_a_fCurve)
		throw LACoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);

	const LAString &ycProName = LAMarketData::getBaseYieldProName(ccy);
	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(ycProName).get());

	LAObjectHolder& yData = ycPro.getYieldData().get();
	LAString ydName = yData.getName();
	if (isSetCurveID)
		ydName = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATECURVEID);

	if (!isPricer && mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_a_fCurve.toUpper())) 
	{
		// generate against leg ccy
		loadYieldCurveDataAndCalibrate(ccy_a_fCurve, dataInstance, isCalcFwdBeforeFwdFXConsant);
	}

	// save colateral ccy and curve
	LAMathYieldCurvePro* colYCPro = NULL;
	if (!isPricer)
	{
		const LAString &colYCProName = LAMarketData::getBaseYieldProName(ccy_a_fCurve);
		colYCPro = &(dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(colYCProName).get()));
		colYCPro->setColAffectingCcy(ccy.toUpper());
		ycPro.setColAffectedCcy(ccy_a_fCurve.toUpper());
	}
	const LAString &fYCName = LAMarketData::getBaseYieldName(ccy_a_fCurve);
	LAMathYieldCurve& fYC = dynamic_cast<LAMathYieldCurve &>(objPool.getObject(fYCName).get());
	const LAString &fYDName = fYC.getYieldData().get().getName();
	ycPro.getColYieldData().convertFromString(fYDName);

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

	ycPro.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + market);
	ycPro.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + market, new LADataMultiReference()).convertFromString(nameB);

	// save assigned curves
	LAStringVector assignedCurves;
	LAString strAssignedCurves = mpStaticData->getStaticData(ccy.toLower() + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + suffix);
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
		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
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
LACalibrateModelIR::convertCurveName(const LAString &propCurve, const LAString &ccy, LAString &curveCcy, LAString &curveName, const bool isPricer, LAObjectPool& objPool) const
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


// 
/*!
    @brief generate sde initial curve data

	@param[in] currency  currency or fx ex.JPY/USD
	@param[out] dataInstance
*/
void
LACalibrateModelIR::loadYieldCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant) const
{
	if (mCurveGenCcyMap[currency]) return;

	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

///// update for XLL Plus in grid //////////////////
	LAString useYieldSDEIRStr = LACoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
	if (useYieldSDEIRStr != AQ_NO_DATA)
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
	if (strIsArb != AQ_NO_DATA)
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
	LAMathYieldCurvePro *ycPro = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		ycPro = new LAMathYieldCurvePro(&dataInstance);
		objPool.set(yieldProName, ycPro);
	}
	else
	{
		//we must not erase the reset method for only ycpro
		ycPro = &dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(yieldProName).get());
	}
	ycPro->getName().convertFromString(yieldProName);

	ycPro->getIsArbFree().set(false);

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
	
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *ycPro, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, isArbFree);

	bool isXccyMarkedToMarket = false;
	LAString isXccyMarkedToMarketString = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST,
														              tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISXCCYMARKEDTOMARKET).toUpper(); // Alias Method: First parameter takes priority
	if (isXccyMarkedToMarketString == "TRUE")
	{
		isXccyMarkedToMarket = true;
	}
	ycPro->LAObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
	ycPro->LAObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new LADataBool(isXccyMarkedToMarket));

	ycPro->LAObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET);
	ycPro->LAObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new LADataBool(isXccyMarkedToMarket));

	LAString fxName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	if (fxName != AQ_NO_DATA)
		ycPro->getFXEntity().convertFromString(fxName);

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
	isrisk |= LACoreDataService::getContext(ARG_KEY_OFFICIALRISK)!=AQ_NO_DATA? true: false;
	isrisk |= LACoreDataService::getContext(ARG_KEY_FRONTRISK)!=AQ_NO_DATA? true: false;

	LAString contextYield = LACoreDataService::getContext(contextKey+CONTEXT_KEY_SDE_YIELD);

	if (!isrisk&&contextYield!=AQ_NO_DATA)
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

	bool enableCalculation = true;
	LAString target = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET).toUpper();
	if (target != AQ_NO_DATA) enableCalculation = false;

	if (enableCalculation || target == STD) setUpGenCurveData(dataInstance, refData, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////


	LAStringVector markets;
	LAString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
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
		LAString basisCurrency = mpStaticData->getStaticData(KEY_SDE_BASIS_BASE_CURRENCY);
		basisCurrency.toUpper();

		//if basisCurrency is empty, not set up basis curve
		if (MADealUtils::getSDECurrencys().size() > 1 && tmpCurrency.toUpper() != basisCurrency && basisCurrency != AQ_NO_DATA)
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

	LADataHolder* dh;
	LAStringVector swapCurves;
	dh = &ycPro->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) swapCurves = dynamic_cast<LADataStrings &>(dh->get()).get();
			
	LAStringVector basisCurves;
	dh = &ycPro->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) basisCurves = dynamic_cast<LADataStrings &>(dh->get()).get();

	LAString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	LAStringVector genBasisSwapMarket;

	LAString OISName;
	if (!markets.empty() && markets[0] != AQ_NO_DATA)
	{
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			if (markets[i] == SWAP)
			{
				if (enableCalculation || target == STD)
				{
					LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE);
					if (tmpAssignedCurves != AQ_NO_DATA)
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
			LAString refData_;
			LAString suffix = "." + markets[i];
			suffix.toLower();
			LAString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffix).toUpper();			
			if (marketType == AQ_NO_DATA)
			{
				LAString isBasisStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISBASIS + suffix).toUpper();
				LAString isReadFile = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffix).toUpper();
				if (isBasisStr == "TRUE") marketType = MARKETTYPE_BASIS;
				else if (isReadFile == "TRUE") marketType = MARKETTYPE_ZERORATE;
				else marketType = MARKETTYPE_SWAP;
			}
			// set up interpolation
			LAString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_INTERPOLATION + suffix).toUpper();
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
				LAString tmpMktName = markets[i];
				genBasisSwapMarket.push_back(markets[i]);
				LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != AQ_NO_DATA && (enableCalculation || target == markets[i]))
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
						setUpBasisCurveData(dataInstance, refData_, asOfDate, tmpCurrency, tmpMktNames[1], yieldDataName, isSpotUse, *ycPro, &tmpMktNames[0]);
					}
					else
					{
						LAString* pMktCurrency = 0;
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
						LAString tenorswapname = dynamic_cast<const LADataString &>(ycPro->getData(IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL).get());
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

				LAString tmpMktName = markets[i];
				LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != AQ_NO_DATA)
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
				throw LACoreInvalidData("Market type is not supported!!", __FILE__, __LINE__); 
			}

			// set yield curve pro
			refData_ = refData_.subString(0, refData_.size() - 2);
			LAStringVector tmpMktNames = markets[i].toToken('_');
			if (tmpMktNames.size() == 2)
			{
				LAMathYieldCurvePro &fYcPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(tmpMktNames[0]), ENCHKTYPE_ISDEFINED).get());
				fYcPro.LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + tmpMktNames[1]);
				fYcPro.LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + tmpMktNames[1], new LADataMultiReference()).convertFromString(refData_);
			}
			else
			{
				ycPro->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i]);
				ycPro->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i], new LADataMultiReference()).convertFromString(refData_);
			}
		}
	}

	//set tenorswap convention
	LAString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
	if (find(markets.begin(), markets.end(), tenorSwapName) == markets.end() && tenorSwapName != AQ_NO_DATA)
	{
		LAString refData_tenor = "";
		setUpBasisCurveData(dataInstance, refData_tenor, asOfDate, tmpCurrency, tenorSwapName, yieldDataName, isSpotUse, *ycPro);
		refData_tenor = refData_tenor.subString(0, refData_tenor.size() - 2);
		ycPro->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName);
		ycPro->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + tenorSwapName, new LADataMultiReference()).convertFromString(refData_tenor);
	}

	//set libor-ois basis convention
	LAString suffix_ois = LAString("." + OISName).toLower();
	const LAString longTermConv = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffix_ois).toUpper();
	LAString loBasisName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + suffix_ois).toUpper();
	loBasisName = (loBasisName == AQ_NO_DATA) ? LOBASIS : loBasisName;
	if (find(markets.begin(), markets.end(), loBasisName) == markets.end() && longTermConv == LOBASIS)
	{
		LAString refData_lob = "";
		setUpLiborOISBasisCurveData(dataInstance, refData_lob, asOfDate, tmpCurrency, loBasisName, OISName, yieldDataName, isSpotUse, *ycPro);
		refData_lob = refData_lob.subString(0, refData_lob.size() - 2);
		ycPro->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + loBasisName);
		ycPro->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + loBasisName, new LADataMultiReference()).convertFromString(refData_lob);
	}


	ycPro->LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	ycPro->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(swapCurves));

	if (mainBDF != AQ_NO_DATA)
	{
		ycPro->LAObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
		ycPro->LAObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new LADataString(mainBDF));
	}
	
	// generate yield data
	LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
						(ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

	ycPro->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	bool isCurveTargetFound = false;
	if (!enableCalculation)
	{
		isCurveTargetFound = (genBasisSwapMarket.end() == std::find(genBasisSwapMarket.begin(),genBasisSwapMarket.end(),
			target) && target != CURVETYPE_FLOATER);
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
			if (tmpAssignedCurves != AQ_NO_DATA && tmpAssignedCurves != "")
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
				ycPro->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
				ycPro->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(targetDF));
			}
			else
			{
				ycPro->LAObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
				ycPro->LAObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new LADataString(targetDF));
			}
		}	
	}
	dataInstance.getReferencePool().completeDependency();

	// get fwdfx flag
	//bool isFwdFX = false;
	//LAString isFwdFXStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX);
	//if (isFwdFXStr.toUpper() == "TRUE") isFwdFX = true;
	loadFwdFXConstCurveDataAndCalibrate(currency, dataInstance, true);

	//if ((enableCalculation || isCurveTargetFound) && !isFwdFX)
	if ((enableCalculation || isCurveTargetFound) && ((std::find(markets.begin(), markets.end(), SWAP) != markets.end())||(std::find(markets.begin(), markets.end(), OISCURVE) != markets.end())))
	{
		if (isCurveTargetFound)
		{
			ycPro->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			ycPro->LAObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new LADataBool(true));
		}
		modelDataObj.calibrateModel(asOfDate);
		ycPro->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		ycPro->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
	}
	//adjust discountfactor
	tmpCurrency.toLower();
	LADataBool tmpAttrDF;
	tmpAttrDF.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF));
	if(tmpAttrDF.get())
	{
		const std::map<LAString, LAString> &assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (!ycPro->isBasisCurve(it->first) && (enableCalculation || target == it->second))
			{
				LAMarketData::adjustDiscountFactor(*eData, it->first);
			}
		}
	}

	// basis
	bool isBasis = false;
	if (!basisCurves.empty() && !isCurveTargetFound && target != CURVETYPE_FLOATER)
	{
		isBasis = true;
		ycPro->LAObject::remove(IR_CALIBRATION_DATA_BASISDFS);
		ycPro->LAObject::add(IR_CALIBRATION_DATA_BASISDFS, new LADataStrings(basisCurves));
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
			ycPro->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
			ycPro->LAObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new LADataBool(true));
		}
		ycPro->setBasisRates();
		ycPro->LAObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
		ycPro->LAObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
	}

	LAString genFloaterName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != AQ_NO_DATA && (enableCalculation || target == CURVETYPE_FLOATER)) 
		setUpFloater(tmpCurrency, *ycPro, genFloaterName);

	// set df2
	if (target == STD && mainBDF != AQ_NO_DATA)
	{
		ycPro->setDF2();
	}

	if (!aud_origSwapRate.empty())
	{
		map<LAString, map<LAString, double> >::const_iterator it = aud_origSwapRate.begin();
		while (it != aud_origSwapRate.end())
		{
			LAMarketData::restoreSwapRateFromL(*ycPro, it->second, currency, &(it->first));
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

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		LAStringVector dataoutCurves;
		dataoutCurves.push_back(STD);	

		const std::map<LAString, LAString>& assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
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
LACalibrateModelIR::setUpCurveTypeDayCount(LAMathYieldCurvePro &ycPro, LAMathYieldCurve &yc) const
{
	// set daycount
	const map<LAString, LAString> &assignedCurveMktMap = ycPro.getAssignedCurveMktMap();
	map<LAString, LAString>::const_iterator it_cur = assignedCurveMktMap.begin();
	while (it_cur != assignedCurveMktMap.end())
	{
		LAPriceDataDayCount dc;
		LAPriceDataCalendar cal;
		LAPriceDataSlidingRule sld;
		LAString accessory;
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
LACalibrateModelIR::generateInitialValueArbfree(const LAString &currency, LADataInstance &dataInstance) const
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
	LAMathYieldCurvePro *ycPro = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		ycPro = new LAMathYieldCurvePro(&dataInstance);
		objPool.set(yieldProName, ycPro);
	}
	else
	{
		//we must not erase the reset method for only ycpro
		ycPro = &dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(yieldProName).get());
	}
	ycPro->getName().convertFromString(yieldProName);

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

	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *ycPro, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, true);
	// set isArbFree
	ycPro->getIsArbFree().set(true);
	bool isXccyMarkedToMarket = false;
	LAString isXccyMarkedToMarketString = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST,
																	  tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISXCCYMARKEDTOMARKET).toUpper(); // Alias Method: First parameter takes priority
	if (isXccyMarkedToMarketString != AQ_NO_DATA)
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
	eData->add(IR_CALIBRATION_DATA_6MLCURVENAMES, new LADataStrings(curveNames_6ML));
	eData->add(IR_CALIBRATION_DATA_DFCURVENAMES, new LADataStrings(curveNames_DF));
	eData->add(IR_CALIBRATION_DATA_3MLCURVENAMES, new LADataStrings(curveNames_3ML));
	ycPro->LAObject::remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
	ycPro->LAObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new LADataBool(isXccyMarkedToMarket));
	ycPro->LAObject::remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET);
	ycPro->LAObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new LADataBool(isXccyMarkedToMarket));
	ycPro->setArbFreeCurveName(curveNames_6ML,curveNames_DF,curveNames_3ML);

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
	isrisk |= LACoreDataService::getContext(ARG_KEY_OFFICIALRISK)!=AQ_NO_DATA? true: false;
	isrisk |= LACoreDataService::getContext(ARG_KEY_FRONTRISK)!=AQ_NO_DATA? true: false;

	LAString contextYield = LACoreDataService::getContext(contextKey+CONTEXT_KEY_SDE_YIELD);

	if (!isrisk&&contextYield!=AQ_NO_DATA)
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
	if (target != AQ_NO_DATA) enableCalculation = false;

	map<LAString, map<LAString, double> > aud_origSwapRate;
	setUpGenCurveData(dataInstance, refData, asOfDate, tmpCurrency, SWAP, yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);

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
			loadYieldCurveDataAndCalibrate(ccy_floater, dataInstance);
		}
		LAString curveName_floater = usd3mlFloaterInfo[1];
		ycPro->getForeignYieldData().convertFromString(fYieldDataName);

		if (isSetCurveID != "TRUE")
		{
			LAString baseCcy = currency; LAString domCcy = currency; LAString forCcy = ccy_floater;
			for (;;)
			{
				LAMathYieldCurvePro &ycPro_dccy = dynamic_cast<LAMathYieldCurvePro &>
							(objPool.getObject(LAMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				LAMathYieldCurvePro &ycPro_fccy = dynamic_cast<LAMathYieldCurvePro &>
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
			if (baseCcyDFInfo.size() != 2) throw LACoreInvalidData("No XccyBasis information!",__FILE__,__LINE__);
			if (usd3mlFloaterInfo[0] != baseCcyDFInfo[0]) throw LACoreInvalidData("Reference currencies are inconsistent!",__FILE__,__LINE__);
			xccyBasis->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new LADataString()).convertFromString(baseCcyDFInfo[1]);
		}

		// set yield curve pro
		refData_XccyBasis = refData_XccyBasis.subString(0, refData_XccyBasis.size() - 2);
		ycPro->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + XCCYBASIS);
		ycPro->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + XCCYBASIS, new LADataMultiReference()).convertFromString(refData_XccyBasis);
	}
	
	tmpCurrency.toLower();
	LAStringVector markets;
	LAString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	if (tmpMarket == AQ_NO_DATA)
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
		setUp36BasisDummyData(dataInstance, refData_, asOfDate, tmpCurrency, THREESIXBASIS, yieldDataName, isSpotUse, *ycPro);
		// set yield curve pro
		refData_ = refData_.subString(0, refData_.size() - 2);
		ycPro->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + THREESIXBASIS);
		ycPro->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + THREESIXBASIS, new LADataMultiReference()).convertFromString(refData_);
	}
	// set yield curve pro
	refData = refData.subString(0, refData.size() - 2);
	ycPro->getMarketData().convertFromString(refData);

	LADataHolder* dh;
	LAStringVector swapCurves;
	dh = &ycPro->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) swapCurves = dynamic_cast<const LADataStrings &>(dh->get()).get();

	LAStringVector basisCurves;
	dh = &ycPro->getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) basisCurves = dynamic_cast<const LADataStrings &>(dh->get()).get();

	LAString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (!markets.empty() && markets[0] != AQ_NO_DATA)
	{
		//LADataStrings &attrGenCurves = dynamic_cast<LADataStrings &>(ycPro->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings()).get());		
		for (unsigned int i = 0; i < markets.size(); ++i)
		{
			if (markets[i] == SWAP) continue;
			
			tmpCurrency.toLower();
			LAString refData_;
			LAString suffix = "." + markets[i];
			suffix.toLower();
			LAString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + suffix).toUpper();
			if (marketType == AQ_NO_DATA)
			{
				LAString isBasisStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISBASIS + suffix).toUpper();
				LAString isReadFile = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISREADFILE + suffix).toUpper();
				if (isBasisStr == "TRUE") marketType = MARKETTYPE_BASIS;
				else if (isReadFile == "TRUE") marketType = MARKETTYPE_ZERORATE;
				else marketType = MARKETTYPE_SWAP;
			}
			// set up interpolation
			LAString strInter = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_INTERPOLATION + suffix).toUpper();
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
					LAString tmpMktName = markets[i];
					LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + "." + tmpMktName.toLower());
					if (tmpAssignedCurves != AQ_NO_DATA)
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

				setUpCurveDataByReadFile( dataInstance, asOfDate, tmpCurrency, markets[i], yieldDataName, *ycPro );
				continue;
			}
			else if( marketType == MARKETTYPE_SWAP )
			{
				if (!enableCalculation) continue;

				LAString tmpMktName = markets[i];
				LAString tmpAssignedCurves = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + "." + tmpMktName.toLower());
				if (tmpAssignedCurves != AQ_NO_DATA)
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
				isoismode.toUpper();
				if (isoismode != AQ_NO_DATA)
					setUpGenCurveDataOIS(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);
				else
					setUpGenCurveData(dataInstance, refData_, asOfDate, tmpCurrency, markets[i], yieldDataName, isSpotUse, isAudExtra, *ycPro, aud_origSwapRate);
			}
			else
			{
				throw LACoreInvalidData("Market type is not supported!!", __FILE__, __LINE__); 
			}

			// set yield curve pro
			refData_ = refData_.subString(0, refData_.size() - 2);
			ycPro->LAObject::remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i]);
			ycPro->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + markets[i], new LADataMultiReference()).convertFromString(refData_);
		}
	}

	ycPro->LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	ycPro->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(swapCurves));

	// generate yield data
	ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
						(ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
	ycPro->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	dataInstance.getReferencePool().completeDependency();
	modelDataObj.calibrateModel(asOfDate);
	
	// generate swap curve
	if (swapCurves.size() != 0 && enableCalculation)
	{
		ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
		for (size_t i=0; i<swapCurves.size(); i++)
		{
			ycPro->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(swapCurves[i]));
			modelDataObj = dynamic_cast<LADataProcedure &> (ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
			modelDataObj.calibrateModel(asOfDate);
			ycPro->LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
		}
		ycPro->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	}

	//adjust discountfactor
	tmpCurrency.toLower();
	LADataBool tmpAttrDF;
	tmpAttrDF.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF));
	if(tmpAttrDF.get())
	{
		const std::map<LAString, LAString> &assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (!ycPro->isBasisCurve(it->first) && (enableCalculation || target == it->second))
			{
				LAMarketData::adjustDiscountFactor(*eData, it->first);
			}
		}
	}

	// basis
	bool isBasis = false;
	ycPro->LAObject::remove(IR_CALIBRATION_DATA_BASISDFS);
	if (!basisCurves.empty() && enableCalculation)
	{
		isBasis = true;
		ycPro->LAObject::add(IR_CALIBRATION_DATA_BASISDFS, new LADataStrings(basisCurves));
	}

	ycPro->LAObject::remove(IR_CALIBRATION_DATA_MAINBASISDF);
	if (mainBDF != AQ_NO_DATA)
	{
		isBasis = true;
		ycPro->LAObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new LADataString(mainBDF));
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

	LAString genFloaterName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS).toUpper();
	if (genFloaterName != AQ_NO_DATA || target == CURVETYPE_FLOATER) setUpFloater(tmpCurrency, *ycPro, genFloaterName);


////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	if (contextYield!=AQ_NO_DATA)
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

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
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

		const std::map<LAString, LAString>& assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
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
LACalibrateModelIR::loadCorrelationDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const
{
	LAString sdeName = getSDEAttrName(currency);
	LAString type = getCorTye(currency);
	type.toUpper();
	
	LAString corName;
	if (LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST) == "TRUE")
	{
		LAString tmpCurrency = currency;
		corName = "cor" + tmpCurrency.toUpper() + "ir";
	}
	else
	{
		corName = PREFIX_COR + sdeName;
	}
	LAMathCorrelation *corEntity;
	bool isCorEntityAlreadyExist(dataInstance.getObjectPool().find(corName));
	if (isCorEntityAlreadyExist)
	{
		corEntity = dynamic_cast<LAMathCorrelation*>(&dataInstance.getObjectPool().getObject(corName).get());
		if (corEntity == 0)
			throw LACoreInvalidData("Corrlation object update failed.", __FILE__, __LINE__); 
		corEntity->reset();
	}
	else
	{
		corEntity = new LAMathCorrelation(&dataInstance);
		LADataString &corAttrName = corEntity->getName();
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
			throw LACoreInvalidData("Volatility input type. only function or data is support", __FILE__, __LINE__); 
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
	@return LAString
*/
LAString
LACalibrateModelIR::getSDEAttrName(const LAString &currency) const
{
	LAString key_ccy = currency;
	return  mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
}

// 
/*!
    @brief get Grid property val

	@param[in] key
	@param[in] grid
	@return LAString
*/
LAString
LACalibrateModelIR::getGridStaticData(const LAString &key, const LAString &grid) const
{
	LAString suffix = "." + grid;
	suffix.toLower();
	LAString val = mpStaticData->getStaticData(key + suffix);
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
LACalibrateModelIR::setUpGenerateConfig
(LADataInstance &dataInstance, const LADate &asOfDate, const LAString &currency, LAMathYieldCurve &yc, LAMathYieldCurvePro &ycPro,
 LAObject &ycData, bool &isAudExtra, bool &isSwapTenorAdjust, bool &isSpotUse, bool isArbFree) const
{
	LAString isSetCurveID = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);

	LAString tmpCurrency = currency;
	tmpCurrency.toLower();

	/*LAString yieldName = PREFIX_YIELD + getSDEAttrName(currency);
	yc->getName().convertFromString(yieldName);*/
	LAString yieldName = yc.getName().get();

	yc.LAObject::remove(CALIBRATION_DATA_ASOFDATE);
	yc.LAObject::add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));

	// set interpolation
	yc.getInterpolation().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_INTERPOLATION).toLower());
	// set daycount
	yc.getDayCount().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DAYCOUNT).toUpper());
	// set freq
	yc.getFrequency().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FREQUENCY).toUpper());
	// set sliding rule
	yc.getSlidingRule().convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SLIDINGRULE).toUpper());
	// set cal 
	LAPriceDataCalendar calY;
	LAString calYStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_CALENDAR);
	calY.convertFromString(calYStr);
	yc.getCalendar() = calY;
	//set basis DF
	LAString mainBDF = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (mainBDF != AQ_NO_DATA) yc.setBasisCurveType(mainBDF);

	// set yield to object pool
	LAObjectPool &objPool = dataInstance.getObjectPool();
	//LAString yieldProName = "PRO_" + yieldName;

	// set yieldpro to object pool
	//// set as of date
	//LAString asOfDateStr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	//LADate asOfDate;
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
	LAString ratePrio_str = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_RATEPRIORITY).toUpper();
	if (ratePrio_str != AQ_NO_DATA) ycPro.getData(PRICING_DATA_RATEPRIORITY).convertFromString(ratePrio_str);
	
	LADataBool tmpAttrB;
	// set tenor adjust
	LAString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST).toUpper();
	if (strSwapTenorAdj != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(strSwapTenorAdj);
		isSwapTenorAdjust = tmpAttrB.get();
	}
	ycPro.getIsSwapTenorAdjust().set(isSwapTenorAdjust);
	if (isSwapTenorAdjust)
	{
		LAString tenorSwapName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toUpper();
		if (tenorSwapName == AQ_NO_DATA) throw LACoreInvalidData("Set tenor swap name!", __FILE__, __LINE__);
		
		ycPro.LAObject::remove(IR_CALIBRATION_DATA_TENORSWAPNAME);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_TENORSWAPNAME, new LADataString(tenorSwapName));
	}

	/*LAString yieldDataName = yieldName + "_DATA";
	ycData.remove(CALIBRATION_DATA_NAME);
	ycData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(yieldDataName);*/
	LAString yieldDataName = dynamic_cast<const LADataString&> ((ycData.getData(CALIBRATION_DATA_NAME, ISNOTNULL)).get()).get();
	// set asofdate
	ycData.remove(CALIBRATION_DATA_ASOFDATE);
	ycData.add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));
	ycData.remove(IR_CALIBRATION_DATA_CURRENCY);
	ycData.add(IR_CALIBRATION_DATA_CURRENCY, new LADataString(currency));
	// set interpolation
	ycData.remove(CALIBRATION_DATA_INTERPOLATION);
	ycData.add(CALIBRATION_DATA_INTERPOLATION, new LAPriceDataInterpolation())
		.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_INTERPOLATION).toLower());

	// set yield data
	yc.getYieldData().convertFromString(yieldDataName);
	ycPro.getYieldData().convertFromString(yieldDataName);
	// set base curve reference
	ycPro.getData(IR_CALIBRATION_DATA_BASEYIELDCURVE, ISDEFINED).convertFromString(yieldName);

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
	if (strIsAudExtra != AQ_NO_DATA)
	{
		LADataBool tmpIsAudExtra;
		tmpIsAudExtra.convertFromString(strIsAudExtra);
		isAudExtra = tmpIsAudExtra.get();
	}

	if (isAudExtra && isSwapTenorAdjust) throw LACoreInvalidData("We can not set AUD extra and swap tenor adjust at a same time!", __FILE__, __LINE__);

	// spotDate use flag
	tmpAttrB.convertFromString(mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSPOTUSE));
	isSpotUse = tmpAttrB.get();

	// set curve exist check
	if (isPricer == "FALSE" || isPricer == AQ_NO_DATA)
	{
		ycPro.LAObject::remove(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, new LADataBool(true));
	}

	// set max term of curve
	LAString maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM);
	if (maxTerm == AQ_NO_DATA)
		maxTerm = mpStaticData->getStaticData(KEY_SIMULATION_TERM_MAX);
	if (maxTerm != AQ_NO_DATA)
	{
		ycData.remove(IR_CALIBRATION_DATA_MAXTERM);
		ycData.add(IR_CALIBRATION_DATA_MAXTERM, new LADataString(maxTerm));
		LAString maxFreq = LACoreDataService::getContext(CONTEXT_KEY_MAXTERMFREQ);
		if (maxFreq == AQ_NO_DATA)
			maxFreq = mpStaticData->getStaticData(KEY_SIMULATION_TERM_MAX_FREQ);
		if (maxFreq == AQ_NO_DATA)
			throw LACoreInvalidData("Max term frequency is needed for curve extrapolation!", __FILE__, __LINE__);
		ycData.remove(IR_CALIBRATION_DATA_MAXTERMFREQ);
		ycData.add(IR_CALIBRATION_DATA_MAXTERMFREQ, new LADataString(maxFreq.toUpper()));
	}

}

// 
/*!
    @brief fuction to set up basis curve data
*/
void
LACalibrateModelIR::setUpBasisCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &curveCurrency, 
									  const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, LAMathYieldCurvePro &ycPro, const LAString* pMktCurrency, bool isCalcFwdBeforeFwdFXConsant) const
{
	LAString mktCurrency = pMktCurrency ? *pMktCurrency : curveCurrency;
	LAString curveMktName = pMktCurrency ? mktCurrency + "_" + marketName : marketName;
	mktCurrency.toLower();curveMktName.toUpper();

	LAString isPricerStr = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	const bool isPricer = isPricerStr == "TRUE";
	LAString isSetCurveIDStr = LACoreDataService::getContext(CONTEXT_KEY_ISSETCURVEID);
	const bool isSetCurveID = isSetCurveIDStr == "TRUE";
	bool enableCalculation = true;
	LAString yieldGeneratorTarget = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
	if (yieldGeneratorTarget != AQ_NO_DATA) enableCalculation = false;

	LAObjectPool &objPool = dataInstance.getObjectPool();

	LAString mktSuffix = "." + marketName;
	LAString curveSuffix = pMktCurrency ? "." + mktCurrency + "_" + marketName : "." + marketName;
	mktSuffix.toLower();curveSuffix.toLower();
	
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

	bool is2Swap = false;
	LAString is2SwapProp = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_IS2SWAP + curveSuffix).toUpper();
	LAString suffixSwap = "";
	if (is2SwapProp != AQ_NO_DATA)
	{
		LADataBool is2SwapAttr;
		is2SwapAttr.convertFromString(is2SwapProp);
		is2Swap = is2SwapAttr.get();
	}

	LAString fCurve;
	LAString dCurve;
	LAString ccy_fCurve;
	LAString ccy_dCurve;
	LAString a_fCurve;
	LAString a_dCurve;
	LAString ccy_a_fCurve;
	LAString ccy_a_dCurve;
	LAString f_dCurve;
	LAString ccy_f_dCurve;
	// cashflow info
	LAString c_freqStr;
	LAString c_freqcpdStr;
	LAString c_daycStr;
	LAString c_slidingStr;
	LAString c_calStr;
	LADate c_spotDate;
	bool c_isBackward = false;
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
	bool a_c_isBackward = false;
	int a_c_resetLag;
	// against index info
	LAString a_i_freqStr;
	LAString a_i_accessaryStr;
	LAString a_i_daycStr;
	LAString a_i_slidingStr;
	LAString a_i_fixcalStr;
	LAString a_i_paycalStr;
	int a_i_resetLag;
	// fixed cashflow info in 2-swap
	LAString f_freqStr;
	LAString f_daycStr;
	LAString f_slidingStr;
	LAString f_calStr;
	LADate f_spotDate;
	int f_resetLag;
	bool f_eom;

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
		LAString strEOMRoll = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_ISEOMRLL + suffixSwap).toUpper();
		if (strEOMRoll != AQ_NO_DATA)
		{
			LADataBool tmpIsEOMRoll;
			tmpIsEOMRoll.convertFromString(strEOMRoll);
			f_eom = tmpIsEOMRoll.get();
		}
		if (f_eom)
		{
			LAString strEOMDay = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_EOMDAY + suffixSwap).toUpper();
			if (strEOMDay != AQ_NO_DATA)
			{
				if (f_spotDate.dayOfMonth() != strEOMDay.getIntValue())
				{
					f_eom = false;
				}
			}
			else
			{
				LAPriceDataCalendar cal;
				cal.convertFromString(f_calStr);
				LADate eomDate = cal.getEOMDay(f_spotDate);
				if (f_spotDate != eomDate)
				{
					f_eom = false;
				}
			}
		}
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
	if (adjustFileName != AQ_NO_DATA)
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
	LAString tmp_f_dCurve = f_dCurve;
	convertCurveName(tmp_fCurve, curveCurrency, ccy_fCurve, fCurve, isPricer, objPool);
	convertCurveName(tmp_dCurve, curveCurrency, ccy_dCurve, dCurve, isPricer, objPool);
	convertCurveName(tmp_a_fCurve, curveCurrency, ccy_a_fCurve, a_fCurve, isPricer, objPool);
	convertCurveName(tmp_a_dCurve, curveCurrency, ccy_a_dCurve, a_dCurve, isPricer, objPool);
	convertCurveName(tmp_f_dCurve, curveCurrency, ccy_f_dCurve, f_dCurve, isPricer, objPool);
	if (ccy_dCurve != curveCurrency || ccy_fCurve != curveCurrency || ccy_a_dCurve != ccy_a_fCurve)
		throw LACoreInvalidData("currency of curve is inconsistent!", __FILE__, __LINE__);

	// against currency
	bool isForeignCcyLeg = false;
	LAString fYieldDataName;
	ycPro.remove(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + LAString("_") + curveMktName);
	if (a_fCurve != DUMMY && ccy_a_fCurve != curveCurrency) 
	{
		isForeignCcyLeg = true;
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

		if ((mCurveGenCcyMap.end() == mCurveGenCcyMap.find(ccy_a_fCurve) && enableCalculation)||isCalcFwdBeforeFwdFXConsant)
		{
			loadYieldCurveDataAndCalibrate(ccy_a_fCurve, dataInstance);
		}
		ycPro.getForeignYieldData().convertFromString(fYieldDataName);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + LAString("_") + curveMktName, new LADataBool(isForeignCcyLeg));
		
		if (!isSetCurveID)
		{
			LAString baseCcy = curveCurrency; LAString domCcy = curveCurrency; LAString forCcy = ccy_a_fCurve;
			for (;;)
			{
				LAMathYieldCurvePro &ycPro_dccy = dynamic_cast<LAMathYieldCurvePro &>
							(objPool.getObject(LAMarketData::getBaseYieldProName(domCcy.toUpper()), ENCHKTYPE_ISDEFINED).get());
				LAMathYieldCurvePro &ycPro_fccy = dynamic_cast<LAMathYieldCurvePro &>
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
	if (is2Swap && (f_dCurve == DUMMY) && !isSpotUse)
	{
		tmpCal.convertFromString(f_calStr);
		f_spotDate = tmpCal.getBusinessDay(asOfDate, f_resetLag);
	}

	const unsigned int basisSize = basisDataMtx.size();
	// use grid
	LAStringVector basisUseGrid;
	LAString tmpBasisUseGrid = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + mktSuffix).toUpper();
	if (tmpBasisUseGrid != AQ_NO_DATA)
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
	bool isOddTermFRNIndex = false;
	LAString strIsOddTermFRNIndex = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISODDTERMFRNINDEX + mktSuffix).toUpper();
	if (strIsOddTermFRNIndex == "TRUE")
	{
		isOddTermFRNIndex = true;
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
	if (strEOMRoll != AQ_NO_DATA)
	{
		LADataBool tmpIsEOMRoll;
		tmpIsEOMRoll.convertFromString(strEOMRoll);
		isEOMRoll = tmpIsEOMRoll.get();
	}
	if (isEOMRoll)
	{
		LAString strEOMDay = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_EOMDAY + mktSuffix).toUpper();
		if (strEOMDay != AQ_NO_DATA)
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

	LAString spotRateTerm = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_SPOTRATETERM + mktSuffix).toUpper();
	double spotRate = DBL_MAX;
	if (spotRateTerm != AQ_NO_DATA)
	{
		const LADataMultiReference &marketRef = ycPro.getMarketData();
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
					break;
				}
			}
		}		
	}

	// cashflow info
	bool isFXForwardQuotedAsOutright = false;
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
	double fwdFXPipSizeFactor = 1.0; // The inverted pipsize i.e. 10,000 = use 4 d.p. for FX Forward Quotes
	//double spotfx = 1.0;
	bool isFwdFX = false;
	LAString isFwdFXStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX).toUpper();
	if (isFwdFXStr == "TRUE") isFwdFX = true;
	if (isFwdFX && marketName == XCCYBASIS)
	{
		// getStaticData has been overloaded to check for first name and if that fails to look for the alias
		LAString isFXForwardQuotedAsOutrightString = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISRATIO + mktSuffix,
																				 mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISFXOUTRIGHT + mktSuffix).toUpper();
		// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
		if (isFXForwardQuotedAsOutrightString == "TRUE") isFXForwardQuotedAsOutright = true;
		if (!isFXForwardQuotedAsOutright)
		{
			fwdFXPipSizeFactor = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_DENOMINATOR + mktSuffix,
															 mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_PIPSIZE + mktSuffix).getDoubleValue(); // Alias Method: First Parameter Takes Priority
			LAString isPriceCcyStr = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISPRICECCY + mktSuffix, mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISDOMESTICCURRENCY + mktSuffix).toUpper(); // Alias Method: first name takes priority
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
		if (tmpfwdFXUseGrid != AQ_NO_DATA)
		{
			fwdFXUseGrid = tmpfwdFXUseGrid.toToken(':');
		}
	}
	if ((basisDataMtx.size() == 0 || basisDataMtx[0].size() < 2) && !isFwdFX)
	{
		LAString msg = marketName + ", basis file is empty";
		throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}

	if (fwdFXSize + basisSize == 0)
	{
		LAString msg = marketName + ", basis and fwd fx data is empty";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	for (unsigned int j = 0; j < fwdFXSize + basisSize; ++j)
	{
		LAObject *mktData = NULL;
		LAString nameB = yieldDataName +  "_" + curveMktName + "_" + LAString(static_cast<int>(j));
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
			mktData->add(IR_CALIBRATION_DATA_ISRATIO, new LADataBool(isFXForwardQuotedAsOutright));
			mktData->add(IR_CALIBRATION_DATA_ISFXOUTRIGHT, new LADataBool(isFXForwardQuotedAsOutright)); // alias for ISRATIO
			if (isFXForwardQuotedAsOutright)
			{
				mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(fwd_fx));
			}
			else
			{
				mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(fwd_fx / fwdFXPipSizeFactor));
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
			// set data type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BASIS);
			LAString term = basisDataMtx[j - fwdFXSize][0].toUpper();
			double basis = basisDataMtx[j - fwdFXSize][1].getDoubleValue();
			// set term
			mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));
			// set rate
			mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(basis / 10000.0));
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
			// 2swap
			if (is2Swap)
			{
				f_freqStr = getGridStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + suffixSwap, term).toUpper();
				f_daycStr = getGridStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT + suffixSwap, term).toUpper();
				f_slidingStr = getGridStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE + suffixSwap, term).toUpper();
			}
		}

		// set simultaneous equation
		mktData->add(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS, new LADataBool(isSimuEq));
		// set issamegridindex
		mktData->add(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX, new LADataBool(isSameGridIndex));
		// set issamegridindex
		mktData->add(IR_CALIBRATION_DATA_ISODDTERMFRNINDEX, new LADataBool(isOddTermFRNIndex));
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
		if (c_freqcpdStr != AQ_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND, new LADataString()).convertFromString(c_freqcpdStr);
		}
		mktData->add(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, new LAPriceDataDayCount()).convertFromString(c_daycStr);
		mktData->add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(c_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE, new LADataDate(c_spotDate));
		mktData->add(IR_CALIBRATION_DATA_CASHLETRESETLAG, new LADataInt(c_resetLag));
		mktData->add(IR_CALIBRATION_DATA_CASHLETISBACKWARD, new LADataBool(c_isBackward));
		// index info
		mktData->add(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(i_fixcalStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, new LAPriceDataCalendar()).convertFromString(i_paycalStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXFREQUENCY, new LADataString()).convertFromString(i_freqStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXACCESSARY, new LADataString()).convertFromString(i_accessaryStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(i_daycStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(i_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_INDEXRESETLAG, new LADataInt(i_resetLag));
		// against cashflow info
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETCALENDAR, new LAPriceDataCalendar()).convertFromString(a_c_calStr);
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, new LADataString()).convertFromString(a_c_freqStr);
		// set frequency of compounding
		if (a_c_freqcpdStr != AQ_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND, new LADataString()).convertFromString(a_c_freqcpdStr);
		}
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETDAYCOUNT, new LAPriceDataDayCount()).convertFromString(a_c_daycStr);
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(a_c_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETSPOTDATE, new LADataDate(a_c_spotDate));
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETRESETLAG, new LADataInt(a_c_resetLag));
		mktData->add(IR_CALIBRATION_DATA_AGTCASHLETISBACKWARD, new LADataBool(a_c_isBackward));
		// against index info
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(a_i_fixcalStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, new LAPriceDataCalendar()).convertFromString(a_i_paycalStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, new LADataString()).convertFromString(a_i_freqStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, new LADataString()).convertFromString(a_i_accessaryStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(a_i_daycStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(a_i_slidingStr);
		mktData->add(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, new LADataInt(a_i_resetLag));
		// set market rate interpolation
		if (interpStr != AQ_NO_DATA)
		{
			interpStr.toLower();
			LAPriceDataInterpolation interpAtt;
			interpAtt.convertFromString(interpStr);
			mktData->add(CALIBRATION_DATA_INTERPOLATION, new LAPriceDataInterpolation(interpAtt));
		}
		// set adjust value interpolation
		if (interpAdjustStr != AQ_NO_DATA)
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

		if (is2Swap)
		{
			mktData->add(IR_CALIBRATION_DATA_IS2SWAP, new LADataBool(true));
			// fixed cashflow info in 2-swap
			mktData->add(IR_CALIBRATION_DATA_DFCURVENAME, new LADataString()).convertFromString(f_dCurve);
			mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(f_calStr);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(f_freqStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(f_daycStr);
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(f_slidingStr);
			mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(f_spotDate));
			mktData->add(IR_CALIBRATION_DATA_ISEOMROLLSW, new LADataBool(f_eom));
		}
	}

	//set foreign curve data
	if (pMktCurrency)
	{
		LAObject& fYieldData = objPool.getObject(fYieldDataName, ENCHKTYPE_ISDEFINED).get();
		fYieldData.remove(CALIBRATION_DATA_MARKETDATA + LAString("_") + marketName);
		fYieldData.add(CALIBRATION_DATA_MARKETDATA + LAString("_") + marketName, new LADataMultiReference()).convertFromString(refData.subString(0, refData.size() - 2));
	}

	LAString tmpAssignedCurves = mpStaticData->getStaticData(curveCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + curveSuffix);
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		LAStringVector assignedCurves = tmpAssignedCurves.toToken(MULTI_STATIC_DATA_DELIMITER);
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
	LAString optimizationMethod = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_OPTIMIZEMETHOD + mktSuffix);
	ycPro.getOptimizeMethod().set(optimizationMethod);

	// compound function
	LAString compoundMethod = mpStaticData->getStaticData(mktCurrency + STATIC_DATA_KEY_YIELD_BASIS_COMPOUNDMETHOD + mktSuffix).toUpper();
	if (compoundMethod == AQ_NO_DATA || compoundMethod == FLATCOMPOUNDING)
	{
		ycPro.getCompoundingFunction().convertFromString(FN_COMPOUNDING8_STR);
	}
	else if (compoundMethod == DAILYCOMPOUNDING)
	{
		if ((c_freqcpdStr != AQ_NO_DATA && c_freqcpdStr != c_freqStr && c_freqStr != BUSINESS_DAYS && c_freqStr != DAILY)
			|| (a_c_freqcpdStr != AQ_NO_DATA && a_c_freqcpdStr != a_c_freqStr && a_c_freqStr != BUSINESS_DAYS && a_c_freqStr != DAILY ))
		{
			LAString msg = "This compound type is suppoted only for daily fixing.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		ycPro.getCompoundingFunction().convertFromString(FN_COMPOUNDING7_STR);
	}
	else if (compoundMethod == DAILYAVERAGING)
	{
		if ((c_freqcpdStr != AQ_NO_DATA && c_freqcpdStr != c_freqStr && c_freqStr != BUSINESS_DAYS && c_freqStr != DAILY)
			|| (a_c_freqcpdStr != AQ_NO_DATA && a_c_freqcpdStr != a_c_freqStr && a_c_freqStr != BUSINESS_DAYS && a_c_freqStr != DAILY ))
		{
			LAString msg = "This compound type is suppoted only for daily fixing.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		ycPro.getCompoundingFunction().convertFromString(FN_COMPOUNDING10_STR);
	}
	else
	{
		LAString msg = "This compunding type is not supported.";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	if (interpAdjustStr != AQ_NO_DATA)
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
	// set adjust value
	//yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUETERM);
	//yldEntity.remove(IR_CALIBRATION_DATA_ADJUSTVALUE);
	//yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUETERM, new LADataDoubles(adjustValue_term));
	//yldEntity.add(IR_CALIBRATION_DATA_ADJUSTVALUE, new LADataDoubles(adjustValue));
}

// 
/*!
    @brief fuction to set up 36 basis curve dummy data
*/
void
LACalibrateModelIR::setUp36BasisDummyData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
									    const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, 
										LAMathYieldCurvePro &ycPro) const
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
		if (reseLag_str == AQ_NO_DATA)
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

// 
/*!
    @brief fuction to set up base curve data
*/
void 
LACalibrateModelIR::setUpGenCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
									const LAString &marketName, const LAString &yieldDataName, bool isSpotUse,
									bool isAudExtra, LAMathYieldCurvePro &ycPro, 
									std::map<LAString, std::map<LAString, double> > &aud_origSwapRate) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAString staticDataSuffix = "";
	if (marketName != SWAP)
	{
		staticDataSuffix = "." + marketName;
		staticDataSuffix.toLower();
	}
	LAString suffix_data = "";
	if (marketName != SWAP)
	{
		suffix_data = "_" + marketName;
	}

	bool isFwdFX = false;
	LAString isFwdFXStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX + staticDataSuffix);
	if (isFwdFXStr.toUpper() == "TRUE") isFwdFX = true;

	LAString liborFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix);
	MAFileAccessor liborFile(LAMarketData::getNumFileName(liborFileName));
	LAStringMatrix liborDataMtx;
	//if (!isFwdFX)
	//{
		liborFile.readAllData(MARKET_DATA_DELIMITER, liborDataMtx);
		liborFile.close();
	//}

	bool isNoLibor = (liborDataMtx.size() == 0 || liborDataMtx[0].size() < 2);
	if (isNoLibor && !isFwdFX)
		throw LACoreInvalidData("LiborFile is empty", __FILE__,__LINE__);
	double firstVal = 0.0;
	if (!isNoLibor) firstVal = liborDataMtx[0][1].getDoubleValue();

	// create O_N
	LAObject *mktDataO_N = NULL;
	LAString nameO_N = yieldDataName + "_O_N" + suffix_data;
	const LAObjectHolder ehois = objPool.getObject(nameO_N);
	if (!ehois.isDefined())
	{
		mktDataO_N = new LAObject();
		objPool.set(nameO_N, mktDataO_N);
	}
	else
	{
		objPool.getObject(nameO_N).get().clear();
		mktDataO_N = &objPool.getObject(nameO_N).get();
	}

	mktDataO_N->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameO_N);
	// set calendar
	LAString calMStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_CALENDAR + staticDataSuffix);
	mktDataO_N->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calMStr);
	// set spotdate
	if (isSpotUse)
	{
		LADate spotDateM(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_SPOTDATE + staticDataSuffix).getCString());
		mktDataO_N->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateM));
	}
	// set daycount
	LAString daycMStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_DAYCOUNT + staticDataSuffix).toUpper();
	mktDataO_N->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycMStr);
	// set sliding
	LAString slidingMStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_MONEYMARKET_SLIDINGRULE + staticDataSuffix).toUpper();
	mktDataO_N->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingMStr);
	// set data type
	mktDataO_N->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_O_N);
	refData += nameO_N + ":";

	// create T_N
	LAObject *mktDataT_N = NULL;
	LAString nameT_N = yieldDataName + "_T_N" + suffix_data;
	const LAObjectHolder ehtn = objPool.getObject(nameT_N);
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
	LAPriceDataCalendar calL;
	LAString calLStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_CALENDAR + staticDataSuffix);
	calL.convertFromString(calLStr);
	LADate spotDateL;
	if (isSpotUse)
	{
		spotDateL.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_SPOTDATE + staticDataSuffix).getCString());
	}
	else
	{
		spotDateL = calL.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_RESETLAG + staticDataSuffix).getIntValue());
	}

	bool isOnSpotAdj = false;
	LAString strIsOnSpotAdj = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_ISONFORSPOTADJUST + staticDataSuffix).toUpper();
	if (strIsOnSpotAdj != AQ_NO_DATA)
	{
		LADataBool tmpIsOnSpotAdj;
		tmpIsOnSpotAdj.convertFromString(strIsOnSpotAdj);
		isOnSpotAdj = tmpIsOnSpotAdj.get();
	}

	bool onValFlg = false;
	bool tnValFlg = false;

	const int liborSize = liborDataMtx.size();
	// use grid
	LAStringVector liborUseGrid;
	LAString tmpLiborUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix).toUpper();
	if (tmpLiborUseGrid != AQ_NO_DATA)
	{
		liborUseGrid = tmpLiborUseGrid.toToken(':');
	}
	// get eomroll
	bool isEOMRollL = false;
	LAString strEOMRollL = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_ISEOMROLL + staticDataSuffix).toUpper();
	if (strEOMRollL != AQ_NO_DATA)
	{
		LADataBool tmpIsEOMRoll;
		tmpIsEOMRoll.convertFromString(strEOMRollL);
		isEOMRollL = tmpIsEOMRoll.get();
	}
	if (isEOMRollL)
	{
		LAString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_EOMDAY + staticDataSuffix).toUpper();
		if (strEOMDay != AQ_NO_DATA)
		{
			if (spotDateL.dayOfMonth() != strEOMDay.getIntValue())
			{
				isEOMRollL = false;
			}
		}
		else
		{
			LADate eomDate = calL.getEOMDay(spotDateL);
			if (spotDateL != eomDate)
			{
				isEOMRollL = false;
			}
		}
	}
	for (int j = 0; j < liborSize; ++j)
	{
		LAString term = liborDataMtx[j][0].toUpper();
		double rate = liborDataMtx[j][1].getDoubleValue();

		// get freq
		LAString freqLStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_FREQUENCY + staticDataSuffix, term).toUpper();
		// get daycount
		LAString daycLStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_DAYCOUNT + staticDataSuffix, term).toUpper();
		// get sliding
		LAString slidingLStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_SLIDINGRULE + staticDataSuffix, term).toUpper();

		if (term == "ON")
		{
			mktDataO_N->add(CALIBRATION_DATA_RATE,new LADataDouble(rate / 100.0));
			onValFlg = true;
			continue;
		}
		else if (term == "TN")
		{
			mktDataT_N->remove(CALIBRATION_DATA_RATE);
			mktDataT_N->add(CALIBRATION_DATA_RATE,new LADataDouble(rate / 100.0));
			tnValFlg = true;
			continue;
		}

		LAObject *mktData = NULL;
		LAString nameL = yieldDataName + "_LIBOR_" + LAString(j) + suffix_data;
		const LAObjectHolder ehlibor = objPool.getObject(nameL);
		if (!ehlibor.isDefined())
		{
			mktData = new LAObject();
			objPool.set(nameL, mktData);
		}
		else
		{
			objPool.getObject(nameL).get().clear();
			mktData = &objPool.getObject(nameL).get();
		}
		refData += nameL + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameL);
		// set spot date
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateL));
		// set calendar, sliding, daycount
		if (term == "SN")
		{
			mktData->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calMStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycMStr);
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingMStr);
		}
		else
		{
			mktData->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calLStr);
			mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycLStr);
			mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingLStr);
		}
		// is on for spot adjust
		mktData->add(IR_CALIBRATION_DATA_ISONFORSPOTADJUST, new LADataBool(isOnSpotAdj));
		// set data type
		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_ZERO);
		// set frequency
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freqLStr);	
		// set eomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLL, new LADataBool(isEOMRollL));
		// set term
		mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(LAMarketData::convertToMLibTerm(term));
		//grid use
		if (liborUseGrid.size() != 0 && find(liborUseGrid.begin(), liborUseGrid.end(), term) == liborUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
		}
		else
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
		}
		// set rate
		mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(rate / 100.0));
		// set object pool
	}

	// if no data for ON and TN set first data
	if (!onValFlg)
	{
		mktDataO_N->add(CALIBRATION_DATA_RATE,new LADataDouble(firstVal / 100.0));
	}
	if (!tnValFlg)
	{
		mktDataT_N->remove(CALIBRATION_DATA_RATE);
		mktDataT_N->add(CALIBRATION_DATA_RATE,new LADataDouble(firstVal / 100.0));
	}

	//FRA Data
	bool isFRAUse = false;
	LAString isFRAUse_str =  mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	LADataBool tmpAttrB;
	if (isFRAUse_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFRAUse_str);
		isFRAUse = tmpAttrB.get();
	}
	if (marketName != SWAP)
	{        
		ycPro.LAObject::remove(IR_CALIBRATION_DATA_ISFRAUSE + suffix_data);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_ISFRAUSE + suffix_data, new LADataBool(isFRAUse));
	}
	else
	{
		ycPro.getIsFRAUse().set(isFRAUse);
	}
	if (isAudExtra && isFRAUse) throw LACoreInvalidData("We can not set AUD extra and FRA use at a same time!", __FILE__, __LINE__);

	//DF curve name
	LAString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix); 
	if (dfCurveName == AQ_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	ycPro.LAObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	ycPro.LAObject::add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new LADataString(dfCurveName));
	LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + suffix_data, new LADataString(dfCurveName));


	// get fwd swap
	bool isFwdSwap = false;
	LAString isFwdSwap_str = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + staticDataSuffix);
	if (isFwdSwap_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFwdSwap_str);
		isFwdSwap = tmpAttrB.get();
	}

	// Do we always recalculate the dynamic linear spline join date?
	LAString alwaysCalcJoinDate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE + staticDataSuffix);
	ycPro.LAObject::remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data);
	if (alwaysCalcJoinDate != AQ_NO_DATA && alwaysCalcJoinDate.size() != 0)
	{
		if (alwaysCalcJoinDate.toUpper() == "TRUE")
		{
			ycPro.LAObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(true));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(true));
		}
		else
		{
			ycPro.LAObject::add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(false));
			yldEntity.add(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + suffix_data, new LADataBool(false));
		}
	}

	// Linear Spline interpolation cut off date
	LAString inputLinearSplineJoinDateStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATIONJOINDATE + staticDataSuffix);
	ycPro.LAObject::remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	yldEntity.remove(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data);
	if (inputLinearSplineJoinDateStr != AQ_NO_DATA && inputLinearSplineJoinDateStr.size() != 0)
	{
		LADate inputLinearSplineJoinDate(inputLinearSplineJoinDateStr.getCString(), "YYYYMMDD");

		ycPro.LAObject::add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputLinearSplineJoinDate));
		yldEntity.add(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + suffix_data, new LADataDate(inputLinearSplineJoinDate));
	}
	
	if (isFRAUse)
	{
		// use grid
		LAStringVector fra3UseGrid;
		LAString tmpFra3UseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID + staticDataSuffix).toUpper();
		if (tmpFra3UseGrid != AQ_NO_DATA)
		{
			fra3UseGrid = tmpFra3UseGrid.toToken(':');
		}
		LAStringVector fra6UseGrid;
		LAString tmpFra6UseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID + staticDataSuffix).toUpper();
		if (tmpFra6UseGrid != AQ_NO_DATA)
		{
			fra6UseGrid = tmpFra6UseGrid.toToken(':');
		}
		
		// get market rate
		LAString fra3FileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + staticDataSuffix);
		LAString fra6FileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + staticDataSuffix);
		if (fra3FileName == AQ_NO_DATA || fra6FileName == AQ_NO_DATA)
		{
			throw LACoreInvalidData("No FRA File", __FILE__, __LINE__);
		}
		MAFileAccessor fra3File(LAMarketData::getNumFileName(fra3FileName));
		MAFileAccessor fra6File(LAMarketData::getNumFileName(fra6FileName));
		LAStringMatrix fra3DataMtx,fra6DataMtx;
		fra3File.readAllData(MARKET_DATA_DELIMITER, fra3DataMtx);
		fra6File.readAllData(MARKET_DATA_DELIMITER, fra6DataMtx);
		fra3File.close();
		fra6File.close();
		const int fra3Size = fra3DataMtx.size();
		const int fra6Size = fra6DataMtx.size();
		// get cal
		LAString calFRAStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_CALENDAR + staticDataSuffix);
		LAPriceDataCalendar calFRA;
		calFRA.convertFromString(calFRAStr);
		// get spot date
		LADate spotDateFRA;
		if (isSpotUse)
		{
			spotDateFRA.setDate(mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_SPOTDATE + staticDataSuffix).getCString());
		}
		else
		{
			spotDateFRA = calFRA.getBusinessDay(asOfDate, mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_RESETLAG + staticDataSuffix).getIntValue());
		}
		// get daycount
		LAString daycFRAStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_DAYCOUNT + staticDataSuffix).toUpper();
		// get sliding
		LAString slidingFRAStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_SLIDINGRULE + staticDataSuffix).toUpper();
		// get applyTension
        bool applyTensionFRAs = false;
        LAString applyTensionFRAsStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_APPLYTENSION + staticDataSuffix).toUpper();
        if (applyTensionFRAsStr != AQ_NO_DATA)
        {
            LADataBool tmpApplyTensionFRAs;
			tmpApplyTensionFRAs.convertFromString(applyTensionFRAsStr);
			applyTensionFRAs = tmpApplyTensionFRAs.get();
        }
		// get tensionGap
        int tensionGapFRAs = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_TENSIONGAP + staticDataSuffix).getIntValue();
        // get eomroll
		bool isEOMRollFRA = false;
		LAString strEOMRollFRA = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_ISEOMROLL + staticDataSuffix).toUpper();
		if (strEOMRollFRA != AQ_NO_DATA)
		{
			LADataBool tmpIsEOMRoll;
			tmpIsEOMRoll.convertFromString(strEOMRollFRA);
			isEOMRollFRA = tmpIsEOMRoll.get();
		}
		if (isEOMRollFRA)
		{
			LAString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_EOMDAY + staticDataSuffix).toUpper();
			if (strEOMDay != AQ_NO_DATA)
			{
				if (spotDateFRA.dayOfMonth() != strEOMDay.getIntValue())
				{
					isEOMRollFRA = false;
				}
			}
			else
			{
				LADate eomDate = calFRA.getEOMDay(spotDateFRA);
				if (spotDateFRA != eomDate)
				{
					isEOMRollFRA = false;
				}
			}
		}

		for (int i = 0; i < fra3Size; ++i)
		{
			LAString term = fra3DataMtx[i][0].toUpper();
			double rate = fra3DataMtx[i][1].getDoubleValue()/100.0;
			//set FRA3M object
			LAObject *mktDataFRA3M = NULL;
			LAString nameFRA3M = yieldDataName + FRA3M + LAString("_") + LAString(i) + suffix_data;
			const LAObjectHolder ehfra3m = objPool.getObject(nameFRA3M);
			if (!ehfra3m.isDefined())
			{
				mktDataFRA3M = new LAObject();
				objPool.set(nameFRA3M, mktDataFRA3M);
			}
			else
			{
				objPool.getObject(nameFRA3M).get().clear();
				mktDataFRA3M = &objPool.getObject(nameFRA3M).get();
			}
			refData += nameFRA3M + ":";
			mktDataFRA3M->add(CALIBRATION_DATA_NAME, new LADataString(nameFRA3M));
			//set fwd swap
			if (isFwdSwap)
			{
				mktDataFRA3M->add(PRICING_DATA_ISFWDSWAP, new LADataBool(isFwdSwap) );
				if (fra3DataMtx[i].size() != 5)
					throw LACoreInvalidData("FRA3M(for FwdSwap) File format is wrong", __FILE__,__LINE__);
				const bool isDate = fra3DataMtx[i][2].toUpper() == "TRUE";
				mktDataFRA3M->add(PRICING_DATA_ISDATE, new LADataBool(isDate) );
				if (isDate)
				{
					const LADate startDate = LADate(fra3DataMtx[i][3].getCString());
					mktDataFRA3M->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
					const LADate endDate = LADate(fra3DataMtx[i][4].getCString());
					mktDataFRA3M->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
				}
				else
				{
					const LAString startTerm = fra3DataMtx[i][3].toUpper();
					mktDataFRA3M->add(PRICING_DATA_STARTTERM, new LADataString(startTerm));
					const LAString tenor = fra3DataMtx[i][4].toUpper();
					mktDataFRA3M->add(PRICING_DATA_TENOR, new LADataString(tenor));
				}
			}
			mktDataFRA3M->add(IR_CALIBRATION_DATA_TERM, new LADataString(term));
			mktDataFRA3M->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar(calFRA));
			mktDataFRA3M->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingFRAStr);
			mktDataFRA3M->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateFRA));
			mktDataFRA3M->add(CALIBRATION_DATA_RATE, new LADataDouble(rate));
			mktDataFRA3M->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycFRAStr);
			mktDataFRA3M->add(IR_CALIBRATION_DATA_APPLYTENSION, new LADataBool(applyTensionFRAs));
			mktDataFRA3M->add(IR_CALIBRATION_DATA_TENSIONGAP, new LADataInt(tensionGapFRAs));
            mktDataFRA3M->add(IR_CALIBRATION_DATA_ISEOMROLL, new LADataBool(isEOMRollFRA));
			//grid use
			if (fra3UseGrid.size() != 0 && find(fra3UseGrid.begin(), fra3UseGrid.end(), term) == fra3UseGrid.end())
			{
				mktDataFRA3M->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
			}
			else
			{
				mktDataFRA3M->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
			}
			//set yield type
			mktDataFRA3M->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(FRA3M);
		}
		for (int i = 0; i < fra6Size; ++i)
		{
			LAString term = fra6DataMtx[i][0].toUpper();
			double rate = fra6DataMtx[i][1].getDoubleValue()/100.0;
			//set FRA6M object
			LAObject *mktDataFRA6M = NULL;
			LAString nameFRA6M = yieldDataName + FRA6M + LAString("_") + LAString(i) + suffix_data;
			const LAObjectHolder ehfra6m = objPool.getObject(nameFRA6M);
			if (!ehfra6m.isDefined())
			{
				mktDataFRA6M = new LAObject();
				objPool.set(nameFRA6M, mktDataFRA6M);
			}
			else
			{
				objPool.getObject(nameFRA6M).get().clear();
				mktDataFRA6M = &objPool.getObject(nameFRA6M).get();
			}
			refData += nameFRA6M + ":";
			mktDataFRA6M->add(CALIBRATION_DATA_NAME, new LADataString(nameFRA6M));
			//set fwd swap
			if (isFwdSwap)
			{
				mktDataFRA6M->add(PRICING_DATA_ISFWDSWAP, new LADataBool(isFwdSwap) );
				if (fra6DataMtx[i].size() != 5)
					throw LACoreInvalidData("FRA6M(for FwdSwap) File format is wrong", __FILE__,__LINE__);
				const bool isDate = fra6DataMtx[i][2].toUpper() == "TRUE";
				mktDataFRA6M->add(PRICING_DATA_ISDATE, new LADataBool(isDate) );
				if (isDate)
				{
					const LADate startDate = LADate(fra6DataMtx[i][3].getCString());
					mktDataFRA6M->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
					const LADate endDate = LADate(fra6DataMtx[i][4].getCString());
					mktDataFRA6M->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
				}
				else
				{
					const LAString startTerm = fra6DataMtx[i][3].toUpper();
					mktDataFRA6M->add(PRICING_DATA_STARTTERM, new LADataString(startTerm));
					const LAString endTerm = fra6DataMtx[i][4].toUpper();
					mktDataFRA6M->add(PRICING_DATA_TENOR, new LADataString(endTerm));
				}
			}
			mktDataFRA6M->add(IR_CALIBRATION_DATA_TERM, new LADataString(term));
			mktDataFRA6M->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar(calFRA));
			mktDataFRA6M->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingFRAStr);
			mktDataFRA6M->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateFRA));
			mktDataFRA6M->add(CALIBRATION_DATA_RATE, new LADataDouble(rate));
			mktDataFRA6M->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycFRAStr);
			mktDataFRA6M->add(IR_CALIBRATION_DATA_APPLYTENSION, new LADataBool(applyTensionFRAs));
			mktDataFRA6M->add(IR_CALIBRATION_DATA_TENSIONGAP, new LADataInt(tensionGapFRAs));
            mktDataFRA6M->add(IR_CALIBRATION_DATA_ISEOMROLL, new LADataBool(isEOMRollFRA));
			//grid use
			if (fra6UseGrid.size() != 0 && find(fra6UseGrid.begin(), fra6UseGrid.end(), term) == fra6UseGrid.end())
			{
				mktDataFRA6M->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
			}
			else
			{
				mktDataFRA6M->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
			}
			//set yield type
			mktDataFRA6M->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(FRA6M);
		}		
	}

	// future
	LAString tmpCurrency = currency;
	tmpCurrency.toLower();
	bool isFutureUse = false;
	LAString tmpFutureStr = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + staticDataSuffix);
	if (tmpFutureStr != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(tmpFutureStr);
		isFutureUse = tmpAttrB.get();
	}
	if (marketName != SWAP)
	{ 
		ycPro.LAObject::remove(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_ISFUTUREUSE + suffix_data, new LADataBool(isFutureUse));
	}
	else
	{
		ycPro.getIsFutureUse().set(isFutureUse);
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
		if (tmpFutureUseGrid != AQ_NO_DATA)
		{
			futureUseGrid = tmpFutureUseGrid.toToken(':');
		}
		else
		{
			LAString tmpUseGridNum = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRIDNUM + staticDataSuffix).toUpper();
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
		if (isConvAdjPrecise_str != AQ_NO_DATA)
		{
			tmpAttrB.convertFromString(isConvAdjPrecise_str);
			isConvAdjPrecise = tmpAttrB.get();
		}
        // get mean reversion
        double meanReversion = 0.0;
        LAString meanReversion_str = mpStaticData->getStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_MEANREVERSION + staticDataSuffix );
        if ( meanReversion_str != AQ_NO_DATA )
        {
            LADataDouble tmpAttrDouble;
            tmpAttrDouble.convertFromString( meanReversion_str );
            meanReversion = tmpAttrDouble.get();
        }

        // get applyTension
        bool applyTensionFutures = false;
        LAString applyTensionFuturesStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_APPLYTENSION + staticDataSuffix).toUpper();
        if (applyTensionFuturesStr != AQ_NO_DATA)
        {
            LADataBool tmpApplyTensionFutures;
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
				throw LACoreInvalidData("Future File format is wrong", __FILE__,__LINE__);
			}
			LAString term;
			LADate startDate,endDate;
			double futurePrice,rate,futureVol;

			if (futureDataMtx[i].size() == 5)
			{
				term = futureDataMtx[i][0].toUpper();
				startDate = LADate(futureDataMtx[i][1].getCString());
				endDate = LADate(futureDataMtx[i][2].getCString());
				futurePrice = futureDataMtx[i][3].getDoubleValue();
				rate = 1.0 - futurePrice * 0.01;
				futureVol = futureDataMtx[i][4].getDoubleValue();
			}
			else if (futureDataMtx[i].size() == 3)
			{
				term = futureDataMtx[i][0].toUpper();
				startDate = LAMathDateCalculations::getIMMDateFromTerm(asOfDate, term);
				startDate = LAMathDateCalculations::getDate(startDate, "0d", slidingF, &calF, true);
				endDate = LAMathDateCalculations::getDate(startDate, "3M", slidingF, &calF, true);
				int mm = endDate.monthOfYear();
				int yy = endDate.yearOfEra();
				endDate = LAMathDateCalculations::getIMMDate(yy, mm, true);
				
				futurePrice = futureDataMtx[i][1].getDoubleValue();
				rate = 1.0 - futurePrice * 0.01;
				futureVol = futureDataMtx[i][2].getDoubleValue();
			}
			else
				throw LACoreInvalidData("Future File format is wrong", __FILE__,__LINE__);
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
		}
	}
	
	// swap
	LAString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	MAFileAccessor swapFile(LAMarketData::getNumFileName(swapFileName));
	LAStringMatrix swapDataMtx;
	//if (!isFwdFX)
	//{
		swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
		swapFile.close();
	//}

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
		if (resetLag_str == AQ_NO_DATA)
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
	if (strIsTimeInterSW != AQ_NO_DATA)
	{
		LADataBool tmpIsTimeInterSW;
		tmpIsTimeInterSW.convertFromString(strIsTimeInterSW);
		isTimeInterSW = tmpIsTimeInterSW.get();
	}
	LAString strNRSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISNEWTONRAPHSON + staticDataSuffix).toUpper();
	if (strNRSW != AQ_NO_DATA)
	{
		LADataBool tmpIsNRSW;
		tmpIsNRSW.convertFromString(strNRSW);
		isNRSW = tmpIsNRSW.get();
	}
	LAString strSimuEQSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISSIMULTANEOUSEQ + staticDataSuffix).toUpper();
	if (strSimuEQSW != AQ_NO_DATA)
	{
		LADataBool tmpIsSimuEQSW;
		tmpIsSimuEQSW.convertFromString(strSimuEQSW);
		isSimuEQSW = tmpIsSimuEQSW.get();
	}
	LAString strEOMRollSW = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_ISEOMRLL + staticDataSuffix).toUpper();
	if (strEOMRollSW != AQ_NO_DATA)
	{
		LADataBool tmpIsEOMRollSW;
		tmpIsEOMRollSW.convertFromString(strEOMRollSW);
		isEOMRollSW = tmpIsEOMRollSW.get();
	}
	if (isEOMRollSW)
	{
		LAString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_EOMDAY + staticDataSuffix).toUpper();
		if (strEOMDay != AQ_NO_DATA)
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// >>>>>>>>>>>>> for XLL plus
	LAString isResetSkipStr = mpStaticData->getStaticData(KEY_YIELD_IS_AUD_RESET_SKIP);
	if (isResetSkipStr!=AQ_NO_DATA)
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
	if (tmpSwapUseGrid != AQ_NO_DATA)
	{
		swapUseGrid = tmpSwapUseGrid.toToken(':');
	}
	// set tenor adjust
	bool isSwapTenorAdjust = false;
	LAString strSwapTenorAdj = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + staticDataSuffix).toUpper();
	if (strSwapTenorAdj != AQ_NO_DATA)
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
				ycPro.LAObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
				ycPro.LAObject::add(CALIBRATION_DATA_MARKETDATA + suffix_data, new LADataMultiReference()).
					convertFromString(refData.subString(0, refData.size() - 2));
			}
			else
			{
				ycPro.getMarketData().convertFromString(refData.subString(0, refData.size() - 2));
			}
			
			// generate yield data
			LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
								(ycPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			ycPro.LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
			ycPro.LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(marketName));

			dataInstance.getReferencePool().completeDependency();
			modelDataObj.calibrateModel(asOfDate);
			
			//reset market data as libor
			LAMarketData::resetMarketDataUseL(ycPro, currency, &marketName);
			resetFlg = false;
			if (marketName != SWAP)
			{
				ycPro.LAObject::remove(CALIBRATION_DATA_MARKETDATA + suffix_data);
			}
		}

		double rate = swapDataMtx[j][1].getDoubleValue();
		// get freq
		LAString freqSStr = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + staticDataSuffix, term).toUpper();
		LAString freqSStr_Fix = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + staticDataSuffix).toUpper();
		// get frequency of floating leg
		LAString baseFreqSStr_float;
		baseFreqSStr_float = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT + staticDataSuffix).toUpper();
		if (baseFreqSStr_float == AQ_NO_DATA) baseFreqSStr_float = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + staticDataSuffix).toUpper();
		LAString freqSStr_float = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + staticDataSuffix, term).toUpper();
		// get frequency of compounding
		LAString freqSStr_cpd = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYCOMPOUND + staticDataSuffix).toUpper();
		// get daycount
		LAString daycSStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT + staticDataSuffix, term).toUpper();
		// get daycount of floating leg
		LAString daycSStr_float = getGridStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT + staticDataSuffix, term).toUpper();
		// get sliding
		LAString slidingSStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE + staticDataSuffix, term).toUpper();
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
		if (daycSStr_float != AQ_NO_DATA)
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
		// iseomroll
		mktData->add(IR_CALIBRATION_DATA_ISEOMROLLSW, new LADataBool(isEOMRollSW));
		// simultaneous equation
		mktData->add(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQSW, new LADataBool(isSimuEQSW));
		// is frequency change
		mktData->add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, new LADataBool(isSwapTenorAdjust) );
		// market rate interpolation
		if (interpSStr != AQ_NO_DATA)
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
		if (baseFreqSStr_float != AQ_NO_DATA)
		{
			if (!checkFrequency(baseFreqSStr_float, term) && isUse)
				throw LACoreInvalidData("base frequency and term of swap rate are inconsistent!!", __FILE__, __LINE__);
			mktData->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, new LADataString()).convertFromString(baseFreqSStr_float);
		}
		// set frequency of floating leg
		if (freqSStr_float != AQ_NO_DATA)
		{
			if (!checkFrequency(freqSStr_float, term) && isUse)
				throw LACoreInvalidData("frequency float and term of swap rate are inconsistent!!", __FILE__, __LINE__);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, new LADataString()).convertFromString(freqSStr_float);
		}
		// set frequency of compounding
		if (freqSStr_cpd != AQ_NO_DATA)
		{
			if (!checkFrequency(freqSStr_cpd, term) && isUse)
				throw LACoreInvalidData("frequency compound and term of swap rate are inconsistent!!", __FILE__, __LINE__);
			mktData->add(IR_CALIBRATION_DATA_FREQUENCY_COMPOUND, new LADataString()).convertFromString(freqSStr_cpd);
		}
	}

	if (refData.size() < 2)
	{
		throw LACoreInvalidData("Market Data is not set !!", __FILE__, __LINE__); 
	}

	LAString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix);
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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
	const LAString isPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (isPricer == "TRUE")
	{
		std::map<LAString, bool>& gCurveMap = ycPro.getGCurveGenerateMap();
		const std::map<LAString, LAString>& assignedCurveMktMap = ycPro.getAssignedCurveMktMap();
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
LACalibrateModelIR::setUpGenCurveDataOIS(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
									const LAString &marketName, const LAString &yieldDataName, bool isSpotUse,
									bool isAudExtra, LAMathYieldCurvePro &ycPro, 
									std::map<LAString, std::map<LAString, double> > &aud_origSwapRate) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	bool isPricer = false;
	LAString strIsPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (strIsPricer == "TRUE") isPricer = true;

	LAString suffix = "." +  marketName;
	suffix.toLower();

	LAString suffix_data = "_" +  marketName;

	LAString yeildGenInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix);
	if (yeildGenInter != AQ_NO_DATA) 
	{
		ycPro.LAObject::remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_data, new LAPriceDataInterpolation()).convertFromString(yeildGenInter);
	}

	LAString oisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffix);
	MAFileAccessor oisFile(LAMarketData::getNumFileName(oisFileName));
	LAStringMatrix oisDataMtx;
	oisFile.readAllData(MARKET_DATA_DELIMITER, oisDataMtx);

	oisFile.close();

	if (oisDataMtx.size() == 0 || oisDataMtx[0].size() < 2 )
	{
		throw LACoreInvalidData("OisFile is empty", __FILE__,__LINE__);
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
	if (strEOMRollOIS != AQ_NO_DATA)	
	{	
		LADataBool tmpIsEOMRollOIS;		
		tmpIsEOMRollOIS.convertFromString(strEOMRollOIS);		
		isEOMRollOIS = tmpIsEOMRollOIS.get();		
	}			
	if (isEOMRollOIS)			
	{			
		LAString strEOMDay = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_EOMDAY + suffix).toUpper();		
		if (strEOMDay != AQ_NO_DATA)		
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
	LAString str_shortTerm =  mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERM + suffix);
	LADate shortTermDate;
	if (str_shortTerm != AQ_NO_DATA)
	{
		shortTermDate = calOIS.getBusinessDay(asOfDate, str_shortTerm.getIntValue());
	}

	LAString shortTermConv = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION + suffix).toUpper();
	LAString firstRate = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FIRSTRATE + suffix).toUpper();

	const int oisSize = oisDataMtx.size();
	// use grid
	LAStringVector oisUseGrid;
	LAString tmpOISUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffix).toUpper();
	if (tmpOISUseGrid != AQ_NO_DATA)
	{
		oisUseGrid = tmpOISUseGrid.toToken(':');
	}

	const LAString longTermConv = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffix).toUpper();
	LAString longTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffix).toUpper();
	LAString loBasisName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + suffix).toUpper();
	LADate date_lt;
	if (longTermConv == LOBASIS)
	{
		loBasisName == AQ_NO_DATA ? LOBASIS : loBasisName;
		ycPro.LAObject::remove(IR_CALIBRATION_DATA_LOBASISNAME + suffix_data);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_LOBASISNAME + suffix_data, new LADataString(loBasisName));
		date_lt = LAMathDateCalculations::getDate(asOfDate, longTerm, true);
	}

	unsigned int j2 = 0;
	for (int i = 0; i < oisSize; ++i)
	{
		LAObject *mktData = NULL;
		LAString nameOIS = yieldDataName + "_OIS_" + LAString(i) + "_" + marketName;
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

		refData += nameOIS + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameOIS);
		
		LADate startDate, endDate;
		LAString term = oisDataMtx[i][0].toUpper();
		if (term.findString("BOJ") >= 0 || term.findString("EUSF") >= 0)
		{
			if (oisDataMtx[i].size() != 4)
			{
				throw LACoreInvalidData("OisFile format is wrong", __FILE__,__LINE__);
			}
			// boj type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BOJ);
			startDate = LADataDate(oisDataMtx[i][2]).get();
			endDate = LADataDate(oisDataMtx[i][3]).get();
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
		}
		else
		{
			// swap type
			mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_PAR);
		}
		
		double rate = oisDataMtx[i][1].getDoubleValue();

		////get if its compounding or daily averaging
		LAString generatemethod = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix).toUpper();

		////set if its compounding or daily averaging
		mktData->add(IR_CALIBRATION_DATA_GENERATEMETHOD, new LADataString()).convertFromString(generatemethod);	

		// get freq
		LAString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY + suffix, term).toUpper();
		// get daycount
		LAString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT + suffix, term).toUpper();
		// get sliding
		LAString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE + suffix, term).toUpper();
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
		if (str_shortTerm != AQ_NO_DATA)
		{
			mktData->add(IR_CALIBRATION_DATA_SHORTTERMDATE, new LADataDate(shortTermDate));
		}		
		//grid use
		if (oisUseGrid.size() != 0 && find(oisUseGrid.begin(), oisUseGrid.end(), term) == oisUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
		}
		else
		{
			if (longTermConv != LOBASIS)
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
			else
			{
				if (term.findString("BOJ") >= 0 || term.findString("EUSF") >= 0)
					// In short term
					mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
				else
				{
					bool IsInLongTerm = (LAMathDateCalculations::getDate(asOfDate, term, true) >= date_lt);
					if (!IsInLongTerm)
						// In middle term
						mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
					else
						// In long term
						mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
				}
			}
		}

	}

	LAStringMatrix fedFundFutureDataMtx;
	LAString fedFundFutureFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + suffix);
	if (fedFundFutureFileName != AQ_NO_DATA)
	{
		MAFileAccessor fedFundFutureFile(LAMarketData::getNumFileName(fedFundFutureFileName));	
		fedFundFutureFile.readAllData(MARKET_DATA_DELIMITER, fedFundFutureDataMtx);
		fedFundFutureFile.close();
	}

	const int fedFundFutureSize = fedFundFutureDataMtx.size();
	// use grid
	LAStringVector ffFutureUseGrid;
	LAString tmpFFFutureUseGrid = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID + suffix).toUpper();	
	if (tmpFFFutureUseGrid != AQ_NO_DATA)
	{
		ffFutureUseGrid = tmpFFFutureUseGrid.toToken(':');
	}
	else
	{
		LAString tmpUseGridNum = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRIDNUM + suffix).toUpper();
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
		LAObject *mktData = NULL;
		LAString nameOIS = yieldDataName + "_OIS_" + LAString(oisSize + i) + "_" + marketName;
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

		refData += nameOIS + ":";
		// set name
		mktData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(nameOIS);
		
		LADate startDate, endDate;
		LAString term = fedFundFutureDataMtx[i][0].toUpper();

		if (fedFundFutureDataMtx[i].size() < 2)
		{
			throw LACoreInvalidData("FF Future File format is wrong", __FILE__,__LINE__);
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
			DateVector ffdates = LAMathDateCalculations::getFFDatesFromTerm(asOfDate,term);
			if (ffdates.size() != 2)
				throw LACoreInvalidData("FF dates error",__FILE__,__LINE__);

			startDate = ffdates[0];
			endDate = ffdates[1];
			mktData->add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
			mktData->add(PRICING_DATA_ENDDATE, new LADataDate(endDate));
		}
		
		double rate = fedFundFutureDataMtx[i][1].getDoubleValue();
		rate  = 100. - rate;

		// get freq
		LAString freqOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FREQUENCY + suffix, term).toUpper();
		// get daycount
		LAString daycOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT + suffix, term).toUpper();
		// get sliding
		LAString slidingOISStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE + suffix, term).toUpper();
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
		if (str_shortTerm != AQ_NO_DATA)
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
			MAFileAccessor oisHistFile(LAMarketData::getNumFileName(oisHistFileName));
			LAStringMatrix oisHistDataMtx;
			oisHistFile.readAllData(MARKET_DATA_DELIMITER, oisHistDataMtx);
			oisHistFile.close();

			if (oisHistDataMtx.size() == 0 || oisHistDataMtx[0].size() < 2 )
			{
				throw LACoreInvalidData("OIS / ARR fixing data is required and empty", __FILE__,__LINE__);
			}

			DateVector histdates;
			DoubleVector histrates;
			for (unsigned int j = 0; j < oisHistDataMtx.size(); j++)
			{
				histdates.push_back(LADataDate(oisHistDataMtx[j][0]).get());
				histrates.push_back(oisHistDataMtx[j][1].getDoubleValue() * 0.01);
			}
			mktData->add(IR_CALIBRATION_DATA_HISTORICALDATES, new LADataDates(histdates));
			mktData->add(IR_CALIBRATION_DATA_HISTORICALRATES, new LADataDoubles(histrates));
		}
	}

	if (refData.size() < 2)
	{
		throw LACoreInvalidData("Market Data is not set !!", __FILE__, __LINE__); 
	}

	//DF curve name
	LAString dfCurveName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffix); 
	if (dfCurveName == AQ_NO_DATA)
	{
		dfCurveName = ITSELF;
	}
	ycPro.LAObject::remove(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName);
	ycPro.LAObject::add(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName, new LADataString(dfCurveName));
	LAObject& yldEntity = objPool.getObject(yieldDataName, ENCHKTYPE_ISDEFINED).get();
	yldEntity.remove(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName);
	yldEntity.add(IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + marketName, new LADataString(dfCurveName));

	//const std::map<LAString, LAString>& assignedCurveMktMap = ycPro->getAssignedCurveMktMap();
	LAString tmpAssignedCurves = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix);
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
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
		std::map<LAString, bool>& gCurveMap = ycPro.getGCurveGenerateMap();
		const std::map<LAString, LAString>& assignedCurveMktMap = ycPro.getAssignedCurveMktMap();
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
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
LACalibrateModelIR::setUpFloater(const LAString &currency, LAMathYieldCurvePro &ycPro, const LAString &genFloaterName) const
{
	LAStringVector markets = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(MULTI_STATIC_DATA_DELIMITER);
	uppervec(markets);
	if (genFloaterName != AQ_NO_DATA)
	{
		ycPro.setAssignedCurveMktMap(genFloaterName, genFloaterName);
		ycPro.LAObject::remove(IR_CALIBRATION_DATA_FLOATERDFS);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_FLOATERDFS, new LADataString(genFloaterName));
		LAString tmpGenFloaterName = genFloaterName;
		tmpGenFloaterName.toLower();
		LAString basisMkt = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME);
		if (basisMkt != AQ_NO_DATA)
		{
			if (std::find(markets.begin(), markets.end(), basisMkt) == markets.end())
			{
				throw LACoreInvalidData("Basis market does not exist!", __FILE__, __LINE__);
			}
			ycPro.LAObject::remove(IR_CALIBRATION_DATA_BASISDATA + LAString("_") + tmpGenFloaterName);
			ycPro.LAObject::add(IR_CALIBRATION_DATA_BASISDATA + LAString("_") + tmpGenFloaterName, new LADataString(basisMkt));
		}
		LAString discountName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT);
		LAString forecastName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FORECAST);
		ycPro.LAObject::remove(IR_CALIBRATION_DATA_FORECAST + LAString("_") + tmpGenFloaterName);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_FORECAST + LAString("_") + tmpGenFloaterName, new LADataString(forecastName));
		ycPro.LAObject::remove(IR_CALIBRATION_DATA_DISCOUNT + LAString("_") + tmpGenFloaterName);
		ycPro.LAObject::add(IR_CALIBRATION_DATA_DISCOUNT + LAString("_") + tmpGenFloaterName, new LADataString(discountName));

		/*LAString isFWDInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_ISFWDINTER).toUpper();
		if (isFWDInter == "TRUE")
		{
			ycPro.LAObject::remove(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + LAString("_") + tmpGenFloaterName);
			ycPro.LAObject::add(IR_CALIBRATION_DATA_ISFWDINTERPOLATION + LAString("_") + tmpGenFloaterName, new LADataBool(true));
			LAString fwdInter = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_FLOATER_FWDINTERPOLATION).toLower();
			ycPro.LAObject::remove(IR_CALIBRATION_DATA_FWDINTERPOLATION + LAString("_") + tmpGenFloaterName);
			ycPro.LAObject::add(IR_CALIBRATION_DATA_FWDINTERPOLATION + LAString("_") + tmpGenFloaterName, new LAPriceDataInterpolation()).convertFromString(fwdInter);;
		}*/
		
		ycPro.setFloater(genFloaterName);
	}
}

void
LACalibrateModelIR::setUpCurveDataByReadFile( LADataInstance &dataInstance, const LADate& asOfDate, const LAString& currency, 
										    const LAString& marketName, const LAString& yieldDataName, LAMathYieldCurvePro &ycPro ) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAObjectHolder objHolder = objPool.getObject(yieldDataName, ENCHKTYPE_NOCHECK );
	if (!objHolder.isDefined() )
		throw LACoreInvalidData("yield Object is not set! LACalibrateModelIR::setUpCurveDataByReadFile", __FILE__, __LINE__ );
	
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
	if (tmpAssignedCurves != AQ_NO_DATA)
	{
		LAStringVector assignedCurves = tmpAssignedCurves.toToken(':');
		for (size_t i = 0; i<assignedCurves.size(); i++)
		{
			ycPro.setAssignedCurveMktMap(assignedCurves[i],marketName);

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
		ycPro.setAssignedCurveMktMap(marketName,marketName);
	}

	ycPro.insertNonRemovableMarket(marketName);
}

void 
LACalibrateModelIR::dataoutCurve(const LAStringVector &curveNames, LAObject &eData, const LAString &yieldDataName) const
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
LACalibrateModelIR::checkFrequency(const LAString& freq, const LAString& mktRateTerm) const
{
	int span = LAMathDateCalculations::getPeriodFrequencyInMonths(freq);

	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(mktRateTerm, y, m, d, w);
	int moth_mkt_term = 12 * y + m;

	return (moth_mkt_term % span) == 0;
}

///// update for XLL Plus //////////////////////////
#include "LACalibrateModelIRVanilla.h"

void
LACalibrateModelIR::generateInitialValueForPricer(const LAString &currency, LADataInstance &dataInstance) const
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
	LAMathYieldCurvePro *ycPro = NULL;
	const LAObjectHolder ehycpro = objPool.getObject(yieldProName);
	if (!ehycpro.isDefined())
	{
		ycPro = new LAMathYieldCurvePro(&dataInstance);
		objPool.set(yieldProName, ycPro);
	}
	else
	{
		//we must not erase the reset method for only ycpro
		ycPro = &dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(yieldProName).get());
	}
	ycPro->getName().convertFromString(yieldProName);

	ycPro->getIsArbFree().set(false);

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
	
	setUpGenerateConfig(dataInstance, asOfDate, currency, *yc, *ycPro, *eData, isAudExtra, isSwapTenorAdjust, isSpotUse, false);
	setUpCurveDataByContext(*ycPro,*yc,eData,currency,SWAP);


	LAStringVector markets;
	LAString tmpMarket = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
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
			LAString suffix = markets[i];
			suffix.toLower();
			LAString contextKey = tmpCurrency+CONTEXT_KEY_SDE_YIELD_WITH_MARKET+suffix;
			LAString contextWithMarket = LACoreDataService::getContext(contextKey);
			if (contextWithMarket!=AQ_NO_DATA)
			{
				LAString marketType = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + suffix).toUpper();
				if (marketType==MARKETTYPE_BASIS)
				{
					LAString useYieldSDEIRStr = LACoreDataService::getContext(CONTEXT_KEY_USE_SDE_YIELD);
					LACoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD,AQ_NO_DATA);
					LAString tmpCurveName = markets[i];
					tmpCurveName.toUpper();
					mpStaticData->setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
					LACalibrateModelIRVanilla generator(currency);
					generator.loadModelDataAndCalibrate(currency, dataInstance, true);
					mpStaticData->removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET);
					LACoreDataService::setContext(CONTEXT_KEY_USE_SDE_YIELD,useYieldSDEIRStr);
				}
			}
			setUpCurveDataByContext(*ycPro,*yc,eData,currency,markets[i]);
		}
	}
}

void 
LACalibrateModelIR::setUpCurveDataByContext(LAMathYieldCurvePro &ycPro, LAMathYieldCurve &yc, LAObject *eData, const LAString& currency, const LAString& marketName ) const
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
	if (contextYield==AQ_NO_DATA)
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

	// set yield data
	LAString contextAttrKey = prefix+CONTEXT_KEY_SDE_YIELD_DATA+suffix;
	LAString contextYieldAttr = LACoreDataService::getContext(contextAttrKey);
	if (contextYieldAttr != AQ_NO_DATA)
	{
		LADataStringMatrix attrStringMatrx;
		attrStringMatrx.convertFromString(contextYieldAttr);

		const LAStringMatrix& strMatrix = attrStringMatrx.get();
		if (strMatrix.size() != 0 && strMatrix[0].size() == 2)
		{
			for (size_t i = 0; i < strMatrix.size(); ++i)
			{
				if (strMatrix[i][0] == CALIBRATION_DATA_CALENDAR) 
				{
					eData->remove(CALIBRATION_DATA_CALENDAR + data_suffix);
					eData->add(CALIBRATION_DATA_CALENDAR + data_suffix, new LAPriceDataCalendar()).convertFromString(strMatrix[i][1]);
				}
				else if (strMatrix[i][0] == CALIBRATION_DATA_SLIDINGRULE) 
				{
					eData->remove(CALIBRATION_DATA_SLIDINGRULE + data_suffix);
					eData->add(CALIBRATION_DATA_SLIDINGRULE + data_suffix, new LAPriceDataSlidingRule()).convertFromString(strMatrix[i][1]);
				}
				else if (strMatrix[i][0] == IR_CALIBRATION_DATA_DAYCOUNT) 
				{
					eData->remove(IR_CALIBRATION_DATA_DAYCOUNT + data_suffix);
					eData->add(IR_CALIBRATION_DATA_DAYCOUNT + data_suffix, new LAPriceDataDayCount()).convertFromString(strMatrix[i][1]);
				}
				else if (strMatrix[i][0] == IR_CALIBRATION_DATA_ACCESSARY) 
				{
					eData->remove(IR_CALIBRATION_DATA_ACCESSARY + data_suffix);
					eData->add(IR_CALIBRATION_DATA_ACCESSARY + data_suffix, new LADataString(strMatrix[i][1]));
				}
				else if (strMatrix[i][0] == IR_CALIBRATION_DATA_FREQUENCY) 
				{
					eData->remove(IR_CALIBRATION_DATA_FREQUENCY + data_suffix);
					eData->add(IR_CALIBRATION_DATA_FREQUENCY + data_suffix, new LADataString(strMatrix[i][1]));
				}
				else if (strMatrix[i][0] == CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE)
				{
					eData->remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + data_suffix);
					eData->add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + data_suffix, new LADataDouble(strMatrix[i][1].getDoubleValue()));
				}
			}
		}
	}

	// set yield interpolation
	LAString contextInterKey = prefix+CONTEXT_KEY_SDE_YIELD_INTER+suffix;
	LAString contextYieldInter = LACoreDataService::getContext(contextInterKey);
	if (contextYieldInter != AQ_NO_DATA)
	{
		yc.getInterpolation().convertFromString(contextYieldInter);
	}


	// check DF2 by properties
	//LAString df2name = mpStaticData->getStaticData(prefix + STATIC_DATA_KEY_YIELD_DF2);
	LAString df2name = currency + "BASISDISCOUNT";
	df2name.toUpper();
	if (df2name != AQ_NO_DATA && df2name == marketName)
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
	LADataHolder *dh = &(ycPro.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		gCurveNames = dynamic_cast<const LADataStrings &>(dh->get()).get();
	}
	gCurveNames.push_back(marketName);
	ycPro.LAObject::remove(IR_CALIBRATION_DATA_GENERATEDFS);
	ycPro.LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(gCurveNames));
}
////////////////////////////////////////////////////

void
LACalibrateModelIR::setUpLiborOISBasisCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
							                  const LAString &marketName, const LAString &marketNameOIS, const LAString &yieldDataName, bool isSpotUse, LAMathYieldCurvePro &ycPro) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	bool isPricer = false;
	LAString strIsPricer = LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST);
	if (strIsPricer == "TRUE") isPricer = true;

	LAString suffix_ois = "." +  marketNameOIS;
	suffix_ois.toLower();

	LAString longTerm = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffix_ois).toUpper();
	LADate date_lt;
	LAString suffix_lo;
	unsigned int LoBasisLiborLegNo;
	unsigned int LoBasisOISLegNo;

	LAStringVector loBasisUseGrid;
	LAStringMatrix lobasisDataMtx;
	if (longTerm == AQ_NO_DATA)
	{
		throw LACoreInvalidData("LongTerm property is needed", __FILE__,__LINE__);
	}

	suffix_lo = LAString("." + marketName).toLower();
	date_lt = LAMathDateCalculations::getDate(asOfDate, longTerm, true);
	LAString lobasisFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FILE + suffix_lo);
	MAFileAccessor lobasisFile(LAMarketData::getNumFileName(lobasisFileName));
	lobasisFile.readAllData(MARKET_DATA_DELIMITER, lobasisDataMtx);
	lobasisFile.close();
	if (lobasisDataMtx.size() == 0 || lobasisDataMtx[0].size() < 2 )
	{
		throw LACoreInvalidData("LOBasisFile is empty", __FILE__,__LINE__);
	}

	const LAString isLeg1Type = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_TYPE + suffix_lo).toUpper();
	const LAString isLeg2Type = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_TYPE + suffix_lo).toUpper();
	LAString liborForecastInfoStr;
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
		throw LACoreInvalidData("Combination of index Types are not consistent with Libor-OIS Basis swap.", __FILE__,__LINE__);
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
		throw LACoreInvalidData("Curve generation is implemented only for the case spread of Libor-OIS Basis is on OIS leg.", __FILE__,__LINE__);			
	}
	// use grid
	LAString tmp = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + suffix_lo).toUpper();
	if (tmp != AQ_NO_DATA)
	{
		loBasisUseGrid = tmp.toToken(':');
	}

	// generate method, calendar and spot date
	LAString longTermGen;
	LAPriceDataCalendar calLOBasis;
	LADate spotDateLOBasis;
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
		longTermGen = LAString("DAILYAVERAGING");
	}


	// set falg if affect base and ois curve (for risk calculation)
	const LAString attrSuffix_lo("_" + marketName);
	ycPro.LAObject::remove(IR_CALIBRATION_DATA_ISAFFECTINGBASECURVE + attrSuffix_lo);
	ycPro.LAObject::add(IR_CALIBRATION_DATA_ISAFFECTINGBASECURVE + attrSuffix_lo, new LADataBool(true));

	// get swap market data 
	LAString suffix_s("");
	LAString attrSuffix_s("");
	LAStringMatrix swapDataMtx;
	if (isPricer)
	{
		suffix_s = suffix_ois;
		LAString swapFileName = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix_s);
		MAFileAccessor swapFile(LAMarketData::getNumFileName(swapFileName));
		swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
		swapFile.close();

		if (swapDataMtx.size() == 0 || swapDataMtx[0].size() < 2 )
		{
			throw LACoreInvalidData("SwapFile is empty", __FILE__,__LINE__);
		}
	}
	else
	{
		std::vector<LAString> forcastInfo(2);
		convertCurveName(liborForecastInfoStr, currency, forcastInfo[0], forcastInfo[1], isPricer, objPool);
		LAString equivSwapMarket(ycPro.getMarketForCurve(forcastInfo[1]));
		if (equivSwapMarket == IR_NO_DATA)
		{
			throw LACoreInvalidData("There does not exist swap market infomration for converting a libor floating leg in Libor-OIS swap.", __FILE__,__LINE__);
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
		LAString term = lobasisDataMtx[i][0];
		const LADate date = LAMathDateCalculations::getDate(asOfDate, term, true);
		bool isLongTerm = true;
		if (date < date_lt)
		{
			// In short or middle term
			isLongTerm = false;
		}

		// create object
		LAObject *mktData = NULL;
		LAString tmpCurrency = currency;
		tmpCurrency.toUpper();
		LAString nameLOBasis = yieldDataName + "_" + marketName + "_" + LAString(i);
		const LAObjectHolder ehLOBasis = objPool.getObject(nameLOBasis);
		if (!ehLOBasis.isDefined())
		{
			mktData = new LAObject();
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
		LAString freqLOStr;
		LAString daycLOStr;
		LAString slidingLOStr;
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
		const LAString freqSwapBaseStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY + suffix_s, term).toUpper();
		LAString freqSwapFloatStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + suffix_s, term).toUpper();
		if (freqSwapFloatStr == AQ_NO_DATA) freqSwapFloatStr = freqSwapBaseStr;

		const LAString daycSwapBaseStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT + suffix_s, term).toUpper();
		LAString daycSwapFloatStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT + suffix_s, term).toUpper();
		if (daycSwapFloatStr == AQ_NO_DATA) daycSwapFloatStr = daycSwapBaseStr;

		LAString freqLOLiborStr;
		LAString daycLOLiborStr;
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
			throw LACoreInvalidData("Libor leg convension of Libor-OIS Basis is not consistent with IRS convention", __FILE__,__LINE__);
		}

		// set long term rate convention
		mktData->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BASIS);
		mktData->add(IR_CALIBRATION_DATA_TERM, new LADataString()).convertFromString(term);
		mktData->add(IR_CALIBRATION_DATA_GENERATEMETHOD, new LADataString()).convertFromString(longTermGen);
		// set lobasis data
		mktData->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDateLOBasis));
		mktData->add(CALIBRATION_DATA_RATE, new LADataDouble(rate_lo / 10000.0));
		mktData->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar(calLOBasis));
		mktData->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycLOStr);
		mktData->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingLOStr);
		mktData->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freqLOStr);	
		//grid use
		if (loBasisUseGrid.size() != 0 && find(loBasisUseGrid.begin(), loBasisUseGrid.end(), term) == loBasisUseGrid.end())
		{
			mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
			continue;
		}
		else
		{
			if (!isLongTerm)
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(false));
				continue;
			}
			else
			{
				mktData->add(IR_CALIBRATION_DATA_GRIDUSEFLAG, new LADataBool(true));
			}
		}
		// set swap data
		if (isPricer)
		{
			unsigned int j2 = 0;
			while (j2 < swapDataMtx.size())
			{
				const LAString term_s = swapDataMtx[j2][0].toUpper();
				if (term_s == term)
				{
					break;
				}
				++j2;
			}
			if (j2 >= swapDataMtx.size())
			{
				throw LACoreInvalidData("can't find the LIBOR Swap rate which is consistent with OIS rate", __FILE__,__LINE__);
			}

			// get swap data
			double rate_s = swapDataMtx[j2][1].getDoubleValue();
			LAString calSwapStr = mpStaticData->getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_CALENDAR + suffix_s);
			
			LAString freqSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFIX + suffix_s, term).toUpper();
			if (freqSwapStr == AQ_NO_DATA) freqSwapStr = freqSwapBaseStr;
			
			const LAString daycSwapBaseStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT + suffix_s, term).toUpper();
			LAString daycSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFIX + suffix_s, term).toUpper();
			if (daycSwapStr == AQ_NO_DATA) daycSwapStr = daycSwapBaseStr;

			LAString slidingSwapStr = getGridStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE + suffix_s, term).toUpper();


			// create object
			LAObject *mktDataIRS = NULL;
			LAString nameIRSForLOBasis = yieldDataName + "_IRS_FOR_LOBASIS_" + marketName + "_" + LAString(i);
			const LAObjectHolder& ehIRSForLOBasis = objPool.getObject(nameIRSForLOBasis);
			if (!ehIRSForLOBasis.isDefined())
			{
				mktDataIRS = new LAObject();
				objPool.set(nameIRSForLOBasis, mktDataIRS);
			}
			else
			{
				objPool.getObject(nameIRSForLOBasis).get().clear();
				mktDataIRS = &objPool.getObject(nameIRSForLOBasis).get();
			}

			mktDataIRS->add(CALIBRATION_DATA_RATE, new LADataDouble(rate_s / 100.0));
			mktDataIRS->add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calSwapStr);
			mktDataIRS->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(daycSwapStr);
			mktDataIRS->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingSwapStr);
			mktDataIRS->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freqSwapStr);

			mktData->add(CALIBRATION_DATA_MARKETDATA_EXT, new LADataReference()).convertFromString(nameIRSForLOBasis);

		}
		else
		{
			// We will create a refrence to base market data below.
			// We must finish the setup of base market data (IRS data) before invoking this function.
			const LADataMultiReference& mktDataRefIRS = dynamic_cast<const LADataMultiReference &>(ycPro.getData(CALIBRATION_DATA_MARKETDATA + attrSuffix_s, ISNOTNULL).get());
			if (mktDataRefIRS.getSize() == 0)
			{
				throw LACoreInvalidData("There not exist IRS market data in yield curve pro object.", __FILE__,__LINE__);
			}

			// Search position of target grid
			unsigned int j2 = 0;
			while (j2 < mktDataRefIRS.getSize())
			{
				const LAObject* mktDataIRS = &mktDataRefIRS.get(j2).get(); 
				const LAString& data_type_s = dynamic_cast<const LADataString &>(mktDataIRS->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get());
				if (data_type_s == PAR)
				{
					const LAString& term_s = dynamic_cast<const LADataString &>(mktDataIRS->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get());
					if (term_s == term)
					{
						break;
					}
				}
				++j2;
			}
			if (j2 >= mktDataRefIRS.getSize())
			{
				throw LACoreInvalidData("can't find the LIBOR Swap rate which is consistent with OIS rate", __FILE__,__LINE__);
			}

			// Create reference
			LAObjectHolder& ehIRSForLOBasis = mktDataRefIRS.get(j2);
			mktData->add(CALIBRATION_DATA_MARKETDATA_EXT, new LADataReference(&ehIRSForLOBasis));
		}
	}
}


