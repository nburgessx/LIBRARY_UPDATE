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
#include "LADataInstance.h"
#include "LAMathCalendar.h"
#include "LAMathCalendarSet.h"
#include "LACoreTemplateType.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataMultiReference.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataCalendar.h"
#include "LAMathDateCalculations.h"
#include "LAMathYieldCurvePro.h"
#include "LAPriceDataDayCount.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAFileAccessor.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"
#include "LARiskConfiguration.h"
#include "LAMathPlainVanillaEntity.h"
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
	static void registCalendar(const LAString &fileName);
	//==============================================================================
	// get key for fx
	static LAString getFXKey(const LAString &ccy1, const LAString &ccy2);
	//==============================================================================
	// get model name
	static LAString getModelName(const LAString &key);
	// get sde name
	static LAString getSDEName(const LAString &key);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//=================================================
	// get asofdate
	static LADate getAsofDate(const LAObjectPool &objPool);
	//=================================================
	// get sde correlation 
	static LAString getTimeGridDayCount(const LAObjectPool &objPool);
#endif
	//=================================================
	// get number attached file name
	static LAString getNumFileName(const LAString &fileName, FileID fileid = MARKETID);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//=================================================
	// get sde correlation 
	static void getSDECorrelation(const LAStringVector &ccys, DoubleMatrix &corData);
#endif
	//==============================================
	// get sde vol correlation
	static void getSDEVolCorrelation(const LAString &ccys, double &corData);
	//==============================================
	// get base volatility name
	static LAString getBaseVolatilityName(const LAString &ccy);
	//==============================================
	// get base correlation name
	static LAString getBaseCorrelationName(const LAString &ccy);
	//==============================================
	// get base yeild name
	static LAString getBaseYieldName(const LAString &ccy);
	//==============================================
	// get base yeildcurvepro name
	static LAString getBaseYieldProName(const LAString &ccy);

	//==============================================
	// reset marketdata use libor
	static void resetMarketDataUseL(LAMathYieldCurvePro &curve, const LAString &ccy, const LAString *pCurveType = 0);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// get path object from object pool
	static LAMathPathEntity *getPathEnitty(LAObjectPool &objPool);
	//==============================================
	// get fx object from object pool
	static LAMathFXEntity *getFXEntity(LAObjectPool &objPool, const LAString &type);
	//==============================================
	// get fx object from object pool
	static void setUpMarket2FXEntity(LAMathFXEntity &fx);
#endif
	//==============================================
	// sort yield curve reference
	static void sortMarketData(LAMathYieldCurvePro &ypro);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// get volatility function file path
	static void getVolFuncFilePath(const LAString &model, const LAString &ccy, const LAString &shift_ccy, const LAString &calcType, SCENARIONUM scenarioNum, LAStringVector &filePath, bool isGrid = false, const LAString *pGridTerm = 0, const LAString *pBaseShift = 0, bool isBase = false);
	//==============================================
	// get volatility input type 
	static LAString getVolInputType(const LAString &model, const LAString &ccy, const LAString &calcType);
#endif
	//==============================================
	// convert to currency(domain ccy and foregin ccy) 
	static void convertToCurrency(const LAString &fx, LAStringVector &ccys);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// correlation use check
	static bool isCorUse(const LAString &model);
#endif
	//==============================================
	// convert to M-lib use term
	static LAString convertToMLibTerm(const LAString &term);
	//==============================================
	// search market grid pos correspond to mlib term
	static bool searchMarketGridPos(const LAStringVector &marketGrid, const LAString &term, unsigned int &pos);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// get volatility val
	static double getVolatilityVal(const LAString &fileName, const LAString &termRow, const LAString &termCol, bool isInter = false);
	//==============================================
	// is calibrate model
	static bool isCalibrateModel(const LAString &model);
	//==============================================
	// get calibdata name
	static LAString getCalibDataName(const LAString &calcType, const LAString &yieldName, int gridPos = -1);
	//==============================================
	// get yielddta name
	static LAString getYieldDataName(LAObjectPool &objPool, const LAString &ycName);
	//==============================================
	// check calibrate end
	static bool isCalibrateEnd(const LAString &calibIDName, LAObjectPool &objPool);
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
	static void adjustDiscountFactor(LAObject& yielddata, const LAString &curveType);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// setup scenario
	static void setUpScenario(MAScenarioParam& param);
#endif
	//==============================================
	// restor swap rate 
	static void restoreSwapRateFromL(LAMathYieldCurvePro &curve, const std::map<LAString, double> &sRateMap, const LAString &ccy, const LAString *pCurveType = 0);
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// get plainvanilla object from object pool
	static LAMathPlainVanillaEntity *getPlainVanillaEntity(LAObjectPool &objPool);
	//==============================================
	// get maturity term from object pool
	static double getMaturityTermFromPlainVanillaEntity(LAObject& trade, const LADate& asOfDate);
	//==============================================
	//get getBaseCurrencyOfFXPair
	static LAString getBaseCurrencyOfFXPair(const LAString& fx);
#endif
	//==============================================
	//get getCalendarTime
	static double getCalendarTime(const LADate& asOfDate, LAString strTerm);
	//==============================================
	//get getFutureVolFromSwaption
	static double getFutureVolFromSwaption(const LAString &ccy, const LADate &expityDate);
	//==============================================
	//get propVal
	static LAString getStaticDataValue(const LAStaticData &staticData, const LAString &ccy, const LAString &key, const bool is_include_key = true);
	//==============================================
	// getFrequency from accessory
	static LAString getFrequency(const LAString &accessory);
	//==============================================
	// getFrequency from accessory
	static void shiftGridMarketVol(const MAScenarioParam &param, int gridPos, const LAStringVector *capTermVec, const LAStringVector *capTenorVec, DoubleVector *capVol, const LAStringVector *swapOptionMatVec, const LAStringVector *swapTenorVec, DoubleMatrix *swaptionVolMtx);
	//==============================================
	// get calibproperty
	static LAString getCalibStaticDataValue(const LAStaticData* pCalibStaticData, const LAString &key, const LAString &grid);
	//==============================================
	// get swap calc convention
	static MASwapConvention getSwapConvention(LAObjectPool &objPool, const LAString& optionMaturity, const LAString& tenor, const LAString& curveName, const MAScenarioParam& param);
	//==============================================
	// get swaption volatility file name
	LAString
	static getVolFileName(const LAString& model, const LAString& currency, const LAString& underlying);


private:
friend class LACoreDataService;
	// constructor
	LAMarketData(void);
	// destructor
	~LAMarketData(void);
	// copy constructor
	LAMarketData(const LAMarketData &rhs);
	LAMarketData &operator=(const LAMarketData &rhs);

	static std::map<LAString, MAIndexData> mIndexMapLibor; // libor index map
	static std::map<LAString, MAIndexData> mIndexMapSwap; // swap index map
	static std::map<LAString, MAIndexData> mIndexMapBasis; // basis index map
	static std::map<LAString, int> mCorrelationMap; // currency correlation map
	static LAString mCalFileName; // calendar file name

#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif
};

class MAComp_StrTerm
{
public:
	/*!
		@brief compare term
		@param[in] _Left one LAString
		@param[in] _Right another LAString
		@return true when right argument date > left argument date
	*/
	bool operator()(const LAString &lterm, const LAString &rterm) const
	{
		LADate ldate, rdate;

		LADate asof;
		asof.setSystemDate();
		LAString t_lterm = lterm;
		LAString t_rterm = rterm;
		t_lterm.toUpper();
		t_rterm.toUpper();
		
		if (t_lterm.findString("ED") != -1 || t_lterm.findString("FF") != -1) 
		{
			ldate = LAMathDateCalculations::getIMMDateFromTerm(asof, t_lterm);
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
			if (t_lterm.findString("X") != -1) t_lterm = LAPriceYieldGenerator::changeFRATermFormat(t_lterm);

			ldate = LAMathDateCalculations::getDate(asof, t_lterm, true);
		}
		
		// right
		if (t_rterm.findString("ED") != -1 || t_rterm.findString("FF") != -1) 
		{
			rdate = LAMathDateCalculations::getIMMDateFromTerm(asof, t_rterm);
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
			if (t_rterm.findString("X") != -1) t_rterm = LAPriceYieldGenerator::changeFRATermFormat(t_rterm);

			rdate = LAMathDateCalculations::getDate(asof, t_rterm, true);
		}

		return ldate < rdate;
	};
};

struct MASwapConvention
{
	LAString frequency, spotLag;
	LAPriceDataDayCount daycount;
	LAPriceDataSlidingRule slidingRule;
	LAPriceDataCalendar paymentCalendar, fixingCalendar;
	LAString curveID, forecastCurveName, discountCurveName;
};
