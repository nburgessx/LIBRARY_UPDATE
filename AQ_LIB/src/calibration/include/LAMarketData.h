#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


#include <algorithm>
#include "AQLDataInstance.h"
#include "AQLMathCalendar.h"
#include "AQLMathCalendarSet.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataCalendar.h"
#include "AQLMathDateCalculations.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLPriceDataDayCount.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAFileAccessor.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
#include "AQLMathPathEntity.h"
#include "AQLMathFXEntity.h"
#include "LARiskConfiguration.h"
#include "AQLMathPlainVanillaEntity.h"
#endif


#ifndef VISUAL_STUDIO_2010_ANALYTICS 
struct MAIndexData;
#endif
struct MAScenarioParam;
struct MASwapConvention;

enum FileID
{
	FILENUM,		   
    MARKETID,		   
    MLIBID,		    
    PROPERTIESID
};

//===================== Class Declare LAMarketData==================================
/*! 
    @brief  Market data util class

   
*/
class LAMarketData
{
public:
	//==============================================================================
	// regist calender file
	static void registCalendar(const AQLString &fileName);
	//==============================================================================
	// get key for fx
	static AQLString getFXKey(const AQLString &ccy1, const AQLString &ccy2);
	//==============================================================================
	// get model name
	static AQLString getModelName(const AQLString &key);
	// get sde name
	static AQLString getSDEName(const AQLString &key);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//=================================================
	// get asofdate
	static AQLDate getAsofDate(const AQLObjectPool &objPool);
	//=================================================
	// get sde correlation 
	static AQLString getTimeGridDayCount(const AQLObjectPool &objPool);
#endif
	//=================================================
	// get number attached file name
	static AQLString getNumFileName(const AQLString &fileName, FileID fileid = MARKETID);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//=================================================
	// get sde correlation 
	static void getSDECorrelation(const AQLStringVector &ccys, DoubleMatrix &corData);
#endif
	//==============================================
	// get sde vol correlation
	static void getSDEVolCorrelation(const AQLString &ccys, double &corData);
	//==============================================
	// get base volatility name
	static AQLString getBaseVolatilityName(const AQLString &ccy);
	//==============================================
	// get base correlation name
	static AQLString getBaseCorrelationName(const AQLString &ccy);
	//==============================================
	// get base yeild name
	static AQLString getBaseYieldName(const AQLString &ccy);
	//==============================================
	// get base yeildcurvepro name
	static AQLString getBaseYieldProName(const AQLString &ccy);

	//==============================================
	// reset marketdata use libor
	static void resetMarketDataUseL(AQLMathYieldCurvePro &curve, const AQLString &ccy, const AQLString *pCurveType = 0);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// get path object from object pool
	static AQLMathPathEntity *getPathEnitty(AQLObjectPool &objPool);
	//==============================================
	// get fx object from object pool
	static AQLMathFXEntity *getFXEntity(AQLObjectPool &objPool, const AQLString &type);
	//==============================================
	// get fx object from object pool
	static void setUpMarket2FXEntity(AQLMathFXEntity &fx);
#endif
	//==============================================
	// sort yield curve reference
	static void sortMarketData(AQLMathYieldCurvePro &ypro);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// get volatility function file path
	static void getVolFuncFilePath(const AQLString &model, const AQLString &ccy, const AQLString &shift_ccy, const AQLString &calcType, SCENARIONUM scenarioNum, AQLStringVector &filePath, bool isGrid = false, const AQLString *pGridTerm = 0, const AQLString *pBaseShift = 0, bool isBase = false);
	//==============================================
	// get volatility input type 
	static AQLString getVolInputType(const AQLString &model, const AQLString &ccy, const AQLString &calcType);
#endif
	//==============================================
	// convert to currency(domain ccy and foregin ccy) 
	static void convertToCurrency(const AQLString &fx, AQLStringVector &ccys);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// correlation use check
	static bool isCorUse(const AQLString &model);
#endif
	//==============================================
	// convert to M-lib use term
	static AQLString convertToMLibTerm(const AQLString &term);
	//==============================================
	// search market grid pos correspond to AlgoQuantLib term
	static bool searchMarketGridPos(const AQLStringVector &marketGrid, const AQLString &term, unsigned int &pos);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// get volatility val
	static double getVolatilityVal(const AQLString &fileName, const AQLString &termRow, const AQLString &termCol, bool isInter = false);
	//==============================================
	// is calibrate model
	static bool isCalibrateModel(const AQLString &model);
	//==============================================
	// get calibdata name
	static AQLString getCalibDataName(const AQLString &calcType, const AQLString &yieldName, int gridPos = -1);
	//==============================================
	// get yielddta name
	static AQLString getYieldDataName(AQLObjectPool &objPool, const AQLString &ycName);
	//==============================================
	// check calibrate end
	static bool isCalibrateEnd(const AQLString &calibIDName, AQLObjectPool &objPool);
#endif
	//==============================================
	// convert fx volatility calib format to market format
	static void convFXVolCalib2Market(const DoubleVector &atm, const DoubleVector &dh25, const DoubleVector &dh10, const DoubleVector &dl25, const DoubleVector &dl10,
										DoubleVector &bf25, DoubleVector &bf10, DoubleVector &rr25, DoubleVector &rr10);
	//==============================================
	// convert fx volatility market format to calib format
	static void convFXVolMarket2Calib(const DoubleVector &atm, const DoubleVector &bf25, const DoubleVector &bf10,  const DoubleVector &rr25, const DoubleVector &rr10,
										DoubleVector &dh25, DoubleVector &dh10, DoubleVector &dl25, DoubleVector &dl10);
	//==============================================
	// adjust df if df increase for time t
	static void adjustDiscountFactor(AQLObject& yielddata, const AQLString &curveType);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// setup scenario
	static void setUpScenario(MAScenarioParam& param);
#endif
	//==============================================
	// restor swap rate 
	static void restoreSwapRateFromL(AQLMathYieldCurvePro &curve, const std::map<AQLString, double> &sRateMap, const AQLString &ccy, const AQLString *pCurveType = 0);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// get plainvanilla object from object pool
	static AQLMathPlainVanillaEntity *getPlainVanillaEntity(AQLObjectPool &objPool);
	//==============================================
	// get maturity term from object pool
	static double getMaturityTermFromPlainVanillaEntity(AQLObject& trade, const AQLDate& asOfDate);
	//==============================================
	//get getBaseCurrencyOfFXPair
	static AQLString getBaseCurrencyOfFXPair(const AQLString& fx);
#endif
	//==============================================
	//get getCalendarTime
	static double getCalendarTime(const AQLDate& asOfDate, AQLString strTerm);
	//==============================================
	//get getFutureVolFromSwaption
	static double getFutureVolFromSwaption(const AQLString &ccy, const AQLDate &expityDate);
	//==============================================
	//get propVal
	static AQLString getStaticDataValue(const LAStaticData &staticData, const AQLString &ccy, const AQLString &key, const bool is_include_key = true);
	//==============================================
	// getFrequency from accessory
	static AQLString getFrequency(const AQLString &accessory);
	//==============================================
	// getFrequency from accessory
	static void shiftGridMarketVol(const MAScenarioParam &param, int gridPos, const AQLStringVector *capTermVec, const AQLStringVector *capTenorVec, DoubleVector *capVol, const AQLStringVector *swapOptionMatVec, const AQLStringVector *swapTenorVec, DoubleMatrix *swaptionVolMtx);
	//==============================================
	// get calibproperty
	static AQLString getCalibStaticDataValue(const LAStaticData* pCalibStaticData, const AQLString &key, const AQLString &grid);
	//==============================================
	// get swap calc convention
	static MASwapConvention getSwapConvention(AQLObjectPool &objPool, const AQLString& optionMaturity, const AQLString& tenor, const AQLString& curveName, const MAScenarioParam& param);
	//==============================================
	// get swaption volatility file name
	AQLString
	static getVolFileName(const AQLString& model, const AQLString& currency, const AQLString& underlying);


private:
friend class LACoreDataService;
	// constructor
	LAMarketData(void);
	// destructor
	~LAMarketData(void);
	// copy constructor
	LAMarketData(const LAMarketData &rhs);
	LAMarketData &operator=(const LAMarketData &rhs);

	static std::map<AQLString, MAIndexData> mIndexMapLibor; // libor index map
	static std::map<AQLString, MAIndexData> mIndexMapSwap; // swap index map
	static std::map<AQLString, MAIndexData> mIndexMapBasis; // basis index map
	static std::map<AQLString, int> mCorrelationMap; // currency correlation map
	static AQLString mCalFileName; // calendar file name

#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif
};

class MAComp_StrTerm
{
public:
	/*!
		@brief compare term
		@param[in] _Left one AQLString
		@param[in] _Right another AQLString
		@return true when right argument date > left argument date
	*/
	bool operator()(const AQLString &lterm, const AQLString &rterm) const
	{
		AQLDate ldate, rdate;

		AQLDate asof;
		asof.setSystemDate();
		AQLString t_lterm = lterm;
		AQLString t_rterm = rterm;
		t_lterm.toUpper();
		t_rterm.toUpper();
		
		if (t_lterm.findString("ED") != -1 || t_lterm.findString("FF") != -1) 
		{
			ldate = AQLMathDateCalculations::getIMMDateFromTerm(asof, t_lterm);
		}
		else
		{
			// left
			if (t_lterm == "ON")
			{
				t_lterm =  "1D";
			}
			else if (t_lterm == "TN")
			{
				t_lterm = "2D";
			}
			else if (t_lterm == "SN")
			{
				t_lterm = "3D";
			}
			else
			{
				t_lterm = LAMarketData::convertToMLibTerm(t_lterm);
			}
			// fra
			if (t_lterm.findString("X") != -1) t_lterm = AQLPriceYieldGenerator::changeFRATermFormat(t_lterm);

			ldate = AQLMathDateCalculations::getDate(asof, t_lterm, true);
		}
		
		// right
		if (t_rterm.findString("ED") != -1 || t_rterm.findString("FF") != -1) 
		{
			rdate = AQLMathDateCalculations::getIMMDateFromTerm(asof, t_rterm);
		}
		else
		{
			if (t_rterm == "ON")
			{
				t_rterm =  "1D";
			}
			else if (t_rterm == "TN")
			{
				t_rterm = "2D";
			}
			else if (t_rterm == "SN")
			{
				t_rterm = "3D";
			}
			else
			{
				t_rterm = LAMarketData::convertToMLibTerm(t_rterm);
			}
			// fra
			if (t_rterm.findString("X") != -1) t_rterm = AQLPriceYieldGenerator::changeFRATermFormat(t_rterm);

			rdate = AQLMathDateCalculations::getDate(asof, t_rterm, true);
		}

		return ldate < rdate;
	};
};

struct MASwapConvention
{
	AQLString frequency, spotLag;
	AQLPriceDataDayCount daycount;
	AQLPriceDataSlidingRule slidingRule;
	AQLPriceDataCalendar paymentCalendar, fixingCalendar;
	AQLString curveID, forecastCurveName, discountCurveName;
};
