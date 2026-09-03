#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include <LADataInstance.h>
#include "LAMatrix.h"
#include <LAMathDefine.h>
#include <LAObject.h>
#include <LAObjectHolder.h>
#include <LAObjectPool.h>
#include <LADataBasics.h>
#include "LAFunctionVector.h"
#include "LANl2sol.h"
#include <LADataProcedure.h>
#include <LAPriceDataCalendar.h>
#include <LAPriceDataSlidingRule.h>
#include <LAPriceDataDayCount.h>
#include <LADataMultiReference.h>
#include <LAPriceDataManager.h>
#include <LADataReference.h>
#include <LADataVector.h>
#include "LASplineInterpolation.h"
#include <LAPriceDataInterpolation.h>
#include "LABasic.h"
#include "LAPriceYieldGenerator.h"
#include "LAMathYieldCurvePro.h"
#include "ConstantDeclarations.h"

using namespace std;

            
// irArbitrageFreeCurveGenerator Function id
#define FN_IRARBFREEGENERATOR     306
// irArbitrageFreeCurveGenerator Function Name
#define FN_IRARBFREEGENERATOR_STR "fn_irarbfreegenerator"


// Market
#ifndef CS
#define CS	"XCCYBasisSwap"
#endif		
#ifndef LS
#define LS	"3MLibor6MLiborBasisSwap"
#endif		
#ifndef FRA3ML
#define FRA3ML	"FRA3MLibor"
#endif		
#ifndef FRA6ML
#define FRA6ML	"FRA6MLibor"
#endif	

// currency
#ifndef CURRENCY_JPY
#define CURRENCY_JPY  "JPY"
#endif
#ifndef CURRENCY_USD
#define CURRENCY_USD  "USD"
#endif
#ifndef CURRENCY_EUR
#define CURRENCY_EUR  "EUR"
#endif
#ifndef CURRENCY_TAR
#define CURRENCY_TAR  "TAR"
#endif
#ifndef CURRENCY_GBP
#define CURRENCY_GBP  "GBP"
#endif
#ifndef CURRENCY_AUD
#define CURRENCY_AUD  "AUD"
#endif
#ifndef CURRENCY_CHF
#define CURRENCY_CHF  "CHF"
#endif
#ifndef CURRENCY_CAD
#define CURRENCY_CAD  "CAD"
#endif
#ifndef CURRENCY_NZD
#define CURRENCY_NZD  "NZD"
#endif

// define data
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_FIX
#define IR_CALIBRATION_DATA_DAYCOUNT_FIX			"DayCountFix"			// Data Name of DayCount
#endif
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_FLOAT
#define IR_CALIBRATION_DATA_DAYCOUNT_FLOAT		"DayCountFloat"			// Data Name of DayCount
#endif
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_THREE
#define IR_CALIBRATION_DATA_DAYCOUNT_THREE		"DayCountThree"			// Data Name of DayCount
#endif
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_SIX
#define IR_CALIBRATION_DATA_DAYCOUNT_SIX  		"DayCountSix"			// Data Name of DayCount
#endif
#ifndef IR_CALIBRATION_DATA_FREQUENCY_FIX
#define IR_CALIBRATION_DATA_FREQUENCY_FIX			"FrequencyFix"			// Data Name of DayCount
#endif
#ifndef IR_CALIBRATION_DATA_FREQUENCY_FLOAT
#define IR_CALIBRATION_DATA_FREQUENCY_FLOAT		"FrequencyFloat"		// Data Name of DayCount
#endif
#ifndef	IR_CALIBRATION_DATA_USDTERMS	
#define	IR_CALIBRATION_DATA_USDTERMS	"USDTerms"
#endif		
#ifndef	IR_CALIBRATION_DATA_USDDFS	
#define	IR_CALIBRATION_DATA_USDDFS	"USDDFs"
#endif		
#ifndef IR_CALIBRATION_DATA_USDDFINTERPOLATION
#define IR_CALIBRATION_DATA_USDDFINTERPOLATION			"USDDFInterpolation"	// Data Name of DayCount
#endif
#ifndef	IR_CALIBRATION_DATA_USDTERMS_3ML	
#define	IR_CALIBRATION_DATA_USDTERMS_3ML	"USDTerms_3ML"
#endif		
#ifndef	IR_CALIBRATION_DATA_USDDFS_3ML	
#define	IR_CALIBRATION_DATA_USDDFS_3ML	"USDDFs_3ML"
#endif	
#ifndef IR_CALIBRATION_DATA_USD3MLINTERPOLATION
#define IR_CALIBRATION_DATA_USD3MLINTERPOLATION		"USD3MLInterpolation"	// Data Name of DayCount
#endif
#ifndef	IR_CALIBRATION_DATA_BASECURRENCYDFTERMS	
#define	IR_CALIBRATION_DATA_BASECURRENCYDFTERMS	"BaseCurrencyDFTerms"
#endif		
#ifndef	IR_CALIBRATION_DATA_BASECURRENCYDFS	
#define	IR_CALIBRATION_DATA_BASECURRENCYDFS	"BaseCurrencyDFs"
#endif	
#ifndef IR_CALIBRATION_DATA_BASECURRENCYINTERPOLATION
#define IR_CALIBRATION_DATA_BASECURRENCYINTERPOLATION		"BaseCurrencyInterpolation"		// Data Name of DayCount
#endif
#ifndef	IR_CALIBRATION_DATA_ISFRAUSE	
#define	IR_CALIBRATION_DATA_ISFRAUSE	"IsFRAUse"
#endif		
#ifndef	IR_CALIBRATION_DATA_PARCURVE	
#define	IR_CALIBRATION_DATA_PARCURVE	"ParCurve"
#endif		
#ifndef	IR_CALIBRATION_DATA_ISRENOTIONALADJUST	
#define	IR_CALIBRATION_DATA_ISRENOTIONALADJUST	"IsRenotionalAdjust"
#endif
#ifndef	IR_CALIBRATION_DATA_6MLCURVENAMES	
#define	IR_CALIBRATION_DATA_6MLCURVENAMES	"6MLCurveNames"
#endif		
#ifndef	IR_CALIBRATION_DATA_DFCURVENAMES	
#define	IR_CALIBRATION_DATA_DFCURVENAMES	"DFCurveNames"
#endif
#ifndef	IR_CALIBRATION_DATA_3MLCURVENAMES	
#define	IR_CALIBRATION_DATA_3MLCURVENAMES	"3MLCurveNames"
#endif
#ifndef	IR_CALIBRATION_DATA_FLOATERTERM	
#define	IR_CALIBRATION_DATA_FLOATERTERM	"FloaterTerm"
#endif		
#ifndef	IR_CALIBRATION_DATA_FLOATERPRICE	
#define	IR_CALIBRATION_DATA_FLOATERPRICE	"FloaterPrice"
#endif	
#ifndef	IR_CALIBRATION_DATA_BASEFORECASTCURVENAME	
#define	IR_CALIBRATION_DATA_BASEFORECASTCURVENAME	"BaseForecastCurveName"
#endif		
#ifndef	IR_CALIBRATION_DATA_DISCOUNTCURVENAME	
#define	IR_CALIBRATION_DATA_DISCOUNTCURVENAME	"DiscountCurveName"
#endif		
#ifndef	IR_CALIBRATION_DATA_BASISLEG	
#define	IR_CALIBRATION_DATA_BASISLEG	"BasisLeg"
#endif		
#ifndef	IR_CALIBRATION_DATA_ISINITIALRATEUSE	
#define	IR_CALIBRATION_DATA_ISINITIALRATEUSE	"IsInitialRateUse"
#endif		
#ifndef	IR_CALIBRATION_DATA_INITIALRATE	
#define	IR_CALIBRATION_DATA_INITIALRATE	"InitialRate"
#endif		
#ifndef	IR_CALIBRATION_DATA_BASECURVEFREQUENCY	
#define	IR_CALIBRATION_DATA_BASECURVEFREQUENCY	"BaseCurveFrequency"
#endif		
#ifndef	IR_CALIBRATION_DATA_SETUPCURVEFREQUENCY	
#define	IR_CALIBRATION_DATA_SETUPCURVEFREQUENCY	"SetUpCurveFrequency"
#endif		
#ifndef	IR_CALIBRATION_DATA_BASECURVEDAYCOUNT	
#define	IR_CALIBRATION_DATA_BASECURVEDAYCOUNT	"BaseCurveDayCount"
#endif		
#ifndef	IR_CALIBRATION_DATA_SETUPCURVEDAYCOUNT	
#define	IR_CALIBRATION_DATA_SETUPCURVEDAYCOUNT	"SetUpCurveDayCount"
#endif	
#ifndef	IR_CALIBRATION_DATA_ARBFREECURVETYPE	
#define	IR_CALIBRATION_DATA_ARBFREECURVETYPE	"ArbFreeCurveType"
#endif

#ifndef DATA_TYPE_XCCYBASISFOREIGNLEG
#define DATA_TYPE_XCCYBASISFOREIGNLEG	"ForeignLeg"
#endif
#ifndef XCCYBASIS
#define XCCYBASIS	"XCCYBASIS"
#endif
#ifndef THREESIXBASIS
#define THREESIXBASIS	"3M6MBASIS"
#endif


class LAObject;
class LADataProcedure;
class LAPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class to generate DF Curve, 6m Libor Curve and 3m Libor Curve at the same time
*/
class LAPriceArbFreeGenerator : public LACoreProcedure
{
public:
//  LIFECYCLE
    // constructor	
    LAPriceArbFreeGenerator();
    // destructor	    
    virtual ~LAPriceArbFreeGenerator();
    // Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
   	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;
	// Return this class type
    virtual function_t          getType() const;
	// generate curve
    virtual void                calibrateModel(	const LADate& basedate, LAObject& inst, 
											const LADataProcedure& att ) const;
	
	static void setForecastCurve(	LADataInstance* dataInstance,
									const LAString& setUpCurveName );

    static void generateCdtDFCurve(	LADataInstance* dataInstance,
                                    const LAString& arbFreeCurveID,
                                    const LAString& forecastCurveID,
                                    const LAString& ctdCurveID,
                                    const LADate& spotDate,
                                    LAPriceDataCalendar& cal, 
                                    LAPriceDataSlidingRule& sld, 
                                    LAString& freq, 
                                    LAPriceDataDayCount& dc,
                                    double spread );

	static	void generateSchedule(	const LADate& asOfDate,
							const LADate& spotDate,
							const LADate& endDate,
							unsigned int span,
							const LAPriceDataCalendar* pCal,
							const LAPriceDataSlidingRule& sld,
							DateVector& dates,
							DoubleArray& terms,
							bool isEOMRoll );

protected:
	// copy constructor    
    LAPriceArbFreeGenerator(const LAPriceArbFreeGenerator& p);

	//static void generateAUDCurve(	const LADate& basedate,
	//								const vector<LAObject*>& mktData,
	//								DoubleArray& dfTerms,
	//								DoubleArray& dfCurve,
	//								DoubleArray& threeMLTerms_Rate,
	//								DoubleArray& threeMLRate,
	//								DoubleArray& threeMLTerms_DF,
	//								DoubleArray& threeMLDF,
	//								DoubleArray& sixMLTerms_Rate,
	//								DoubleArray& sixMLRate,
	//								DoubleArray& sixMLTerms_DF,
	//								DoubleArray& sixMLDF,
	//								bool isFRAUse,
	//								bool isRenAdj,
	//								LAInterpolationBase* pInter_DF );

	static void generateUSDCurve(	const LADate& basedate,
									const vector<LAObject*>& mktData,
									const LAObject* fYieldData,
									DoubleArray& dfTerms,
									DoubleArray& dfCurve,
									DoubleMatrix& threeMLTermsMtx_Rate,
									DoubleArray& threeMLRate,
									DoubleArray& threeMLTerms_DF,
									DoubleArray& threeMLDF,
									DoubleMatrix& sixMLTermsMtx_Rate,
									DoubleArray& sixMLRate,
									DoubleArray& sixMLTerms_DF,
									DoubleArray& sixMLDF,
									unsigned int& threeMLSPos,
									unsigned int& sixMLSPos,
									bool isFRAUse,
									bool isFutureUse, 
									bool isRenAdj,
									LAInterpolationBase* pInter_DF );

	static void generateCurve(	const LADate& basedate,
								const vector<LAObject*>& mktData,
								const LAObject* fYieldData,
								DoubleArray& dfTerms,
								DoubleArray& dfCurve,
								DoubleMatrix& threeMLTermsMtx_Rate,
								DoubleArray& threeMLRate,
								DoubleArray& threeMLTerms_DF,
								DoubleArray& threeMLDF,
								DoubleMatrix& sixMLTermsMtx_Rate,
								DoubleArray& sixMLRate,
								DoubleArray& sixMLTerms_DF,
								DoubleArray& sixMLDF,
								unsigned int& threeMLSPos,
								unsigned int& sixMLSPos,
								bool isFRAUse,
								bool isFutureUse, 
								bool isRenAdj,
								LAInterpolationBase* pInter_yg );

	static	void insertRate(	double rate, 
								DoubleMatrix& termsMtx, 
								DoubleArray& rates, 
								DoubleArray& df_terms, 
								DoubleArray& dfs,
								double staTerm, 
								double endTerm, 
								const LADate& staDate, 
								const LADate& endDate, 
								const LAPriceDataDayCount& dc );

	class MoneyMarket
	{
		public:
			MoneyMarket( vector<const LAObject* > mktData );
			virtual ~MoneyMarket();

			LAStringVector terms;
			DoubleArray rates;
			LAPriceDataDayCount dc;
			LAPriceDataSlidingRule sld;
			const LAPriceDataCalendar* pCal;
	};

	class LiborMarket
	{
		public:
			LiborMarket( vector<const LAObject* > mktData );
			virtual ~LiborMarket();
		
			double threeMLibor;
			double sixMLibor;
			LAPriceDataDayCount dc;
			LAPriceDataSlidingRule sld;
			const LAPriceDataCalendar* pCal;
			bool isEOMRoll;
			LAString roll_conv;
			LADate spotDate;
	};

	class SwapMarket
	{
		public:
			SwapMarket( vector<const LAObject* > mktData, vector<const LAObject* > tenorSwapMktData, const LiborMarket& libMkt, 
				const LADate& basedate );
			SwapMarket(const SwapMarket& v);
			virtual ~SwapMarket();
		
			LAStringVector mktTerms_str;
			DoubleVector mktTerms;
			DoubleVector mktRates;
			LADate spotDate;
			LADate startDate;
			LADate endDate;
			LAString endTerm;
			LAPriceDataSlidingRule sld;
			const LAPriceDataCalendar* pCal;
			LAString freq_Float;
			LAString freq_Fix;
			LAString freq_Float_Pay;
			LAPriceDataDayCount dc_Float;
			LAPriceDataDayCount dc_Fix;
			LAInterpolationBase* pInter;
			map<int, LAString > map_freq_Fix;
			bool isEOMRoll;
			LAString roll_conv;
			LAString optimizeMethod;
			bool isSwapTenorAdjust;
			DateVector dateGrid_3MRoll;
			DateVector dateGrid_6MRoll;
			DateVector dateGrid_12MRoll;
			DoubleArray termGrid_3MRoll;
			DoubleArray termGrid_6MRoll;
			DoubleArray termGrid_12MRoll;
	};

	class XCCYBasisMarket
	{
		public:
			XCCYBasisMarket( vector<const LAObject* > mktData, const LAObject* fYieldData, const SwapMarket& swapMkt, bool& isRenAdj, 
				bool isUSD, const LADate& basedate );
			XCCYBasisMarket(const XCCYBasisMarket& v);
			virtual ~XCCYBasisMarket();
		
			LAStringVector mktTerms_str;
			DoubleVector mktTerms;
			DoubleVector mktRates;
			LADate spotDate;
			LADate endDate;
			LAPriceDataDayCount dc;
			LAPriceDataSlidingRule sld;
			const LAPriceDataCalendar* pCal;
			LAInterpolationBase* pInter;
			LAInterpolationBase* pInter_usd;
			LAInterpolationBase* pInter_usd_3ML;
			LAInterpolationBase* pInter_fPrices;
			LAInterpolationBase* pInter_baseccydf;
			LAInterpolationBase* pInter_adjust;
			bool isEOMRoll;
			LAString roll_conv;
			DateVector dateGrid_3MRoll;
			DoubleArray termGrid_3MRoll;
	};

	// 3M6M Basis Market
	class LiborBasisMarket
	{
		public:
			LiborBasisMarket( vector<const LAObject* > mktData, const SwapMarket& swapMkt, const LADate& basedate );
			LiborBasisMarket(const LiborBasisMarket& v);
			virtual ~LiborBasisMarket();
		
			LAStringVector mktTerms_str;
			DoubleVector mktTerms;
			DoubleVector mktRates;
			LAPriceDataDayCount dc_3L;
			LAPriceDataDayCount dc_6L;
			LAPriceDataSlidingRule sld;
			const LAPriceDataCalendar* pCal;
			LADate spotDate;
			LADate endDate;
			LAInterpolationBase* pInter;
			bool isEOMRoll;
			LAString roll_conv;
			DateVector dateGrid_3MRoll;
			DateVector dateGrid_6MRoll;
			DoubleArray termGrid_3MRoll;
			DoubleArray termGrid_6MRoll;
			LAString freq_3L_pay;
	};

	class FRAMarket
	{
		public:
			FRAMarket( vector<const LAObject* > mktData, bool isFRAUse );
			virtual ~FRAMarket();
		
			LADate spotDate;
			LAPriceDataDayCount dc;
			const LAPriceDataCalendar* pCal;
			LAPriceDataSlidingRule sld;
			std::map<LAString, double > map_term_rate;
			bool isEOMRoll;
			LAString roll_conv;
	};
	
	class FutureMarket
	{
		public:
			FutureMarket( vector<const LAObject* > mktData, const SwapMarket& swapMkt, bool isFutureUse, const LADate& basedate );
			virtual ~FutureMarket();
		
			LAPriceDataDayCount dc;
			DateVector startDates;
			DateVector endDates;
			DoubleVector mktRates;
	};

	static void calcCurve_Semi_Semi(	const LADate& basedate,
										DoubleArray& dfTerms,
										DoubleArray& dfCurve,
										DoubleMatrix& threeMLTermsMtx_Rate,
										DoubleArray& threeMLRate,
										DoubleArray& threeMLTerms_DF,
										DoubleArray& threeMLDF,
										DoubleMatrix& sixMLTermsMtx_Rate,
										DoubleArray& sixMLRate,
										DoubleArray& sixMLTerms_DF,
										DoubleArray& sixMLDF,
										bool isFRAUse,
										bool isRenAdj, 
										const LiborMarket& libMkt,
										const SwapMarket& swapMkt,
										const XCCYBasisMarket& xccyBasisMkt,
										const LiborBasisMarket& libBasisMkt,
										const FRAMarket& fra3MLMkt,
										const FRAMarket& fra6MLMkt,
										const DoubleArray& swapRateGrid,
										const DoubleArray& currBasisGrid,
										const DoubleArray& threeSixBasisGrid,
										const DateVector& dateGrid_3MRoll,
										const DoubleArray& termGrid_3MRoll,
										const DateVector& dateGrid_6MRoll,
										const DoubleArray& termGrid_6MRoll,
										double& PV_Swap,
										double& PV_3L6L_6L,
										double& PV_3L6L_3L,
										double& PV_Curr,
										double& ANN_Swap_6M,
										double& ANN_Swap_3M,
										double& ANN_3L6L_6L,
										double& ANN_3L6L_3L,
										double& ANN_Curr,
										double& I,
										double spotAdjust,
										double spotTerm,
										unsigned int pos );

	static void calcCurve_Quar_Quar(	const LADate& basedate,
										DoubleArray& dfTerms,
										DoubleArray& dfCurve,
										DoubleMatrix& threeMLTermsMtx_Rate,
										DoubleArray& threeMLRate,
										DoubleArray& threeMLTerms_DF,
										DoubleArray& threeMLDF,
										DoubleMatrix& sixMLTermsMtx_Rate,
										DoubleArray& sixMLRate,
										DoubleArray& sixMLTerms_DF,
										DoubleArray& sixMLDF,
										bool isFRAUse,
										bool isRenAdj, 
										const LiborMarket& libMkt,
										const SwapMarket& swapMkt,
										const XCCYBasisMarket& xccyBasisMkt,
										const LiborBasisMarket& libBasisMkt,
										const FRAMarket& fra3MLMkt,
										const FRAMarket& fra6MLMkt,
										const DoubleArray& swapRateGrid,
										const DoubleArray& currBasisGrid,
										const DoubleArray& threeSixBasisGrid,
										const DateVector& dateGrid_3MRoll,
										const DoubleArray& termGrid_3MRoll,
										const DateVector& dateGrid_6MRoll,
										const DoubleArray& termGrid_6MRoll,
										double& PV_Swap,
										double& PV_3L6L_6L,
										double& PV_3L6L_3L,
										double& PV_Curr,
										double& ANN_Swap_6M,
										double& ANN_Swap_3M,
										double& ANN_3L6L_6L,
										double& ANN_3L6L_3L,
										double& ANN_Curr,
										double& I,
										double spotAdjust,
										double spotTerm,
										unsigned int pos );

	static void calcCurve_Quar_Semi(	const LADate& basedate,
										DoubleArray& dfTerms,
										DoubleArray& dfCurve,
										DoubleMatrix& threeMLTermsMtx_Rate,
										DoubleArray& threeMLRate,
										DoubleArray& threeMLTerms_DF,
										DoubleArray& threeMLDF,
										DoubleMatrix& sixMLTermsMtx_Rate,
										DoubleArray& sixMLRate,
										DoubleArray& sixMLTerms_DF,
										DoubleArray& sixMLDF,
										bool isFRAUse,
										bool isRenAdj, 
										const LiborMarket& libMkt,
										const SwapMarket& swapMkt,
										const XCCYBasisMarket& xccyBasisMkt,
										const LiborBasisMarket& libBasisMkt,
										const FRAMarket& fra3MLMkt,
										const FRAMarket& fra6MLMkt,
										const DoubleArray& swapRateGrid,
										const DoubleArray& currBasisGrid,
										const DoubleArray& threeSixBasisGrid,
										const DateVector& dateGrid_3MRoll,
										const DoubleArray& termGrid_3MRoll,
										const DateVector& dateGrid_6MRoll,
										const DoubleArray& termGrid_6MRoll,
										double& PV_Swap,
										double& PV_3L6L_6L,
										double& PV_3L6L_3L,
										double& PV_Curr,
										double& ANN_Swap_6M,
										double& ANN_Swap_3M,
										double& ANN_3L6L_6L,
										double& ANN_3L6L_3L,
										double& ANN_Curr,
										double& I,
										double spotAdjust,
										double spotTerm,
										unsigned int pos );

	static void calcCurve_Semi_Quar(	const LADate& basedate,
										DoubleArray& dfTerms,
										DoubleArray& dfCurve,
										DoubleMatrix& threeMLTermsMtx_Rate,
										DoubleArray& threeMLRate,
										DoubleArray& threeMLTerms_DF,
										DoubleArray& threeMLDF,
										DoubleMatrix& sixMLTermsMtx_Rate,
										DoubleArray& sixMLRate,
										DoubleArray& sixMLTerms_DF,
										DoubleArray& sixMLDF,
										bool isFRAUse,
										bool isRenAdj, 
										const LiborMarket& libMkt,
										const SwapMarket& swapMkt,
										const XCCYBasisMarket& xccyBasisMkt,
										const LiborBasisMarket& libBasisMkt,
										const FRAMarket& fra3MLMkt,
										const FRAMarket& fra6MLMkt,
										const DoubleArray& swapRateGrid,
										const DoubleArray& currBasisGrid,
										const DoubleArray& threeSixBasisGrid,
										const DateVector& dateGrid_3MRoll,
										const DoubleArray& termGrid_3MRoll,
										const DateVector& dateGrid_6MRoll,
										const DoubleArray& termGrid_6MRoll,
										double& PV_Swap,
										double& PV_3L6L_6L,
										double& PV_3L6L_3L,
										double& PV_Curr,
										double& ANN_Swap_6M,
										double& ANN_Swap_3M,
										double& ANN_3L6L_6L,
										double& ANN_3L6L_3L,
										double& ANN_Curr,
										double& I,
										double spotAdjust,
										double spotTerm,
										unsigned int pos );

	static void calcCurve_Annu_Semi(	const LADate& basedate,
										DoubleArray& dfTerms,
										DoubleArray& dfCurve,
										DoubleMatrix& threeMLTermsMtx_Rate,
										DoubleArray& threeMLRate,
										DoubleArray& threeMLTerms_DF,
										DoubleArray& threeMLDF,
										DoubleMatrix& sixMLTermsMtx_Rate,
										DoubleArray& sixMLRate,
										DoubleArray& sixMLTerms_DF,
										DoubleArray& sixMLDF,
										bool isFRAUse,
										bool isRenAdj, 
										const LiborMarket& libMkt,
										const SwapMarket& swapMkt,
										const XCCYBasisMarket& xccyBasisMkt,
										const LiborBasisMarket& libBasisMkt,
										const FRAMarket& fra3MLMkt,
										const FRAMarket& fra6MLMkt,
										const DoubleArray& swapRateGrid,
										const DoubleArray& currBasisGrid,
										const DoubleArray& threeSixBasisGrid,
										const DateVector& dateGrid_3MRoll,
										const DoubleArray& termGrid_3MRoll,
										const DateVector& dateGrid_6MRoll,
										const DoubleArray& termGrid_6MRoll,
										double& PV_Swap,
										double& PV_3L6L_6L,
										double& PV_3L6L_3L,
										double& PV_Curr,
										double& ANN_Swap,
										double& ANN_3L6L_6L,
										double& ANN_3L6L_3L,
										double& ANN_Curr,
										double& I,
										double spotAdjust,
										double spotTerm,
										unsigned int pos );

	static void calcCurve_Annu_Quar(	const LADate& basedate,
										DoubleArray& dfTerms,
										DoubleArray& dfCurve,
										DoubleMatrix& threeMLTermsMtx_Rate,
										DoubleArray& threeMLRate,
										DoubleArray& threeMLTerms_DF,
										DoubleArray& threeMLDF,
										DoubleMatrix& sixMLTermsMtx_Rate,
										DoubleArray& sixMLRate,
										DoubleArray& sixMLTerms_DF,
										DoubleArray& sixMLDF,
										bool isFRAUse,
										bool isRenAdj, 
										const LiborMarket& libMkt,
										const SwapMarket& swapMkt,
										const XCCYBasisMarket& xccyBasisMkt,
										const LiborBasisMarket& libBasisMkt,
										const FRAMarket& fra3MLMkt,
										const FRAMarket& fra6MLMkt,
										const DoubleArray& swapRateGrid,
										const DoubleArray& currBasisGrid,
										const DoubleArray& threeSixBasisGrid,
										const DateVector& dateGrid_3MRoll,
										const DoubleArray& termGrid_3MRoll,
										const DateVector& dateGrid_6MRoll,
										const DoubleArray& termGrid_6MRoll,
										double& PV_Swap,
										double& PV_3L6L_6L,
										double& PV_3L6L_3L,
										double& PV_Curr,
										double& ANN_Swap,
										double& ANN_3L6L_6L,
										double& ANN_3L6L_3L,
										double& ANN_Curr,
										double& I,
										double spotAdjust,
										double spotTerm,
										unsigned int pos );

	static void calcCurve_NewtonRaphson(	const LADate&  basedate, 
											DoubleArray& dfTerms, 
											DoubleArray& dfCurve, 
											DoubleArray& threeMLTerms_DF, 
											DoubleArray& threeMLDF, 
											DoubleArray& sixMLTerms_DF, 
											DoubleArray& sixMLDF, 
											bool isFRAUse, 
											bool isRenAdj, 
											const LiborMarket& libMkt, 
											const SwapMarket& swapMkt, 
											const XCCYBasisMarket& xccyBasisMkt, 
											const LiborBasisMarket& libBasisMkt,
											const DateVector& dateGrid_3MRoll,
											const DoubleArray& termGrid_3MRoll,
											const DateVector& dateGrid_6MRoll,
											const DoubleArray& termGrid_6MRoll,
											const DateVector& dateGrid_12MRoll,
											const DoubleArray& termGrid_12MRoll,
											LAInterpolationBase* pInter_yg,
											bool isUSD );

	static void calcCurve_NL2SOL(	const LADate&  basedate, 
									DoubleArray& dfTerms, 
									DoubleArray& dfCurve, 
									DoubleMatrix& threeMLTermsMtx_Rate,
									DoubleArray& threeMLRate,
									DoubleArray& threeMLTerms_DF, 
									DoubleArray& threeMLDF, 
									DoubleMatrix& sixMLTermsMtx_Rate,
									DoubleArray& sixMLRate,
									DoubleArray& sixMLTerms_DF, 
									DoubleArray& sixMLDF, 
									bool isFRAUse, 
									bool isRenAdj, 
									const LiborMarket& libMkt, 
									const SwapMarket& swapMkt, 
									const XCCYBasisMarket& xccyBasisMkt, 
									const LiborBasisMarket& libBasisMkt,
									LAInterpolationBase* pInter_yg,
									bool isUSD );

	static DoubleVector getSwapValue(	const LiborMarket& libMkt, 
										const SwapMarket& swapMkt, 
										const DateVector& dateGrid_3MRoll,
										const DoubleArray& termGrid_3MRoll,
										const DateVector& dateGrid_6MRoll,
										const DoubleArray& termGrid_6MRoll,
										const DateVector& dateGrid_12MRoll,
										const DoubleArray& termGrid_12MRoll,
										LAInterpolationBase* pInter_3ML,
										LAInterpolationBase* pInter_6ML,
										LAInterpolationBase* pInter_DF );

	static DoubleVector getXccyBasisValue(	const LADate&  basedate, 
											bool isRenAdj, 
											const LiborMarket& libMkt, 
											const SwapMarket& swapMkt, 
											const XCCYBasisMarket& xccyBasisMkt,
											const DateVector& dateGrid_3MRoll,
											const DoubleArray& termGrid_3MRoll,
											LAInterpolationBase* pInter_3ML,
											LAInterpolationBase* pInter_6ML,
											LAInterpolationBase* pInter_DF,
											bool isUSD );

	static DoubleVector getLibBasisValue(	const LiborMarket& libMkt, 
											const SwapMarket& swapMkt, 
											const LiborBasisMarket& libBasisMkt,
											const DateVector& dateGrid_3MRoll,
											const DoubleArray& termGrid_3MRoll,
											const DateVector& dateGrid_6MRoll,
											const DoubleArray& termGrid_6MRoll,
											LAInterpolationBase* pInter_3ML,
											LAInterpolationBase* pInter_6ML,
											LAInterpolationBase* pInter_DF );

	static void baseDateAdjust(	const LADate& basedate,
								const LADate& spotDate,
								const MoneyMarket& monMkt,
								DoubleArray& dfTerms,
								DoubleArray& dfCurve,
								DoubleMatrix& threeMLTermsMtx_Rate,
								DoubleArray& threeMLRate,
								DoubleArray& threeMLTerms_DF,
								DoubleArray& threeMLDF,
								DoubleMatrix& sixMLTermsMtx_Rate,
								DoubleArray& sixMLRate,
								DoubleArray& sixMLTerms_DF,
								DoubleArray& sixMLDF );

	static void liborDateAdjust(	const LADate& basedate,
									const LiborMarket& libMkt, 
									const SwapMarket& swapMkt,
									DoubleMatrix& threeMLTermsMtx_Rate,
									DoubleArray& threeMLRate,
									DoubleArray& threeMLTerms_DF,
									DoubleArray& threeMLDF,
									DoubleMatrix& sixMLTermsMtx_Rate,
									DoubleArray& sixMLRate,
									DoubleArray& sixMLTerms_DF,
									DoubleArray& sixMLDF,
									LAInterpolationBase* pInter );

	static void insertFRA(	const LADate& basedate,
							const FRAMarket& fra3LMkt,
							const FRAMarket& fra6LMkt,
							DoubleArray& dfTerms,
							DoubleArray& dfCurve,
							DoubleMatrix& threeMLTermsMtx_Rate,
							DoubleArray& threeMLRate,
							DoubleArray& threeMLTerms_DF,
							DoubleArray& threeMLDF,
							DoubleMatrix& sixMLTermsMtx_Rate,
							DoubleArray& sixMLRate,
							DoubleArray& sixMLTerms_DF,
							DoubleArray& sixMLDF,
							unsigned int& threeMLSPos,
							unsigned int& sixMLSPos,
							LAInterpolationBase* pInter );

	static void insertFuture(	const LADate& basedate,
								const FutureMarket& futureMkt,
								const SwapMarket& swapMkt,
								DoubleMatrix& threeMLTermsMtx_Rate,
								DoubleArray& threeMLRate,
								DoubleArray& threeMLTerms_DF,
								DoubleArray& threeMLDF,
								unsigned int& threeMLSPos,
								LAInterpolationBase* pInter );

	static bool insertToVector( double in1,
								double in2,
								DoubleArray& array1, 
								DoubleArray& array2,
								unsigned int& pos);

	static bool getPositionOfVector( double target,
									 DoubleArray& vec, 
									 double error,
									 unsigned int& pos);

	static double getSpotAdjust(	const LADate& basedate,
									const LADate& spotDate,
									const MoneyMarket& monMkt );

	static LAString getSwapFixFrequency(unsigned int month, const SwapMarket& swapMkt);

	static void	setCurveConvention(	LAObjectHolder& objHolder,
								std::vector<LAObject*>& mktData,
								const LAStringVector& curveNames_3ML,
								const LAStringVector& curveNames_6ML);

	// class for generating arb free curve by NL2Sol optimize mothod
	class LAMathAFCurveCalibrator : public LAFunctionVector
	{
	public:
		// constructor
		LAMathAFCurveCalibrator(
			const LADate& basedate_,
			const DoubleArray& threeMLTerms_DF_, 
			const DoubleArray& threeMLDF_, 
			const DoubleArray& sixMLTerms_DF_, 
			const DoubleArray& sixMLDF_, 
			const DoubleArray& dfTerms_, 
			const DoubleArray& dfCurve_, 
			bool isRenAdj_, 
			const LiborMarket& libMkt_, 
			const SwapMarket& swapMkt_, 
			const XCCYBasisMarket& xccyBasisMkt_, 
			const LiborBasisMarket& libBasisMkt_, 
			LAInterpolationBase* pInter_3ML_, 
			LAInterpolationBase* pInter_6ML_, 
			LAInterpolationBase* pInter_DF_, 
			const size_t& shortTermSize_3ML_, 
			const size_t& shortTermSize_6ML_, 
			const size_t& shortTermSize_DF_, 
			bool isUSD_);
		// destructor
		virtual ~LAMathAFCurveCalibrator(){}

		virtual unsigned long lengthOfArgumentVector( ) { return swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size() + xccyBasisMkt.mktTerms.size(); }

		virtual unsigned long lengthOfFunctionVector( ) { return swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size() + xccyBasisMkt.mktTerms.size(); }

		virtual unsigned long maximumNumberOfIterations() { return 150; }

		virtual void operator()(DoubleArray& f, const DoubleArray& x);

		virtual bool constraintsAreViolated(const DoubleArray& x);

	private:
		LADate basedate;

		DoubleArray threeMLTerms_DF; 
		DoubleArray threeMLDF;
		DoubleArray sixMLTerms_DF; 
		DoubleArray sixMLDF;
		DoubleArray dfTerms;
		DoubleArray dfCurve;

		bool isRenAdj;

		LiborMarket libMkt; 
		SwapMarket swapMkt; 
		XCCYBasisMarket xccyBasisMkt;
		LiborBasisMarket libBasisMkt; 

		DateVector dateGrid_3MRoll;
		DoubleArray termGrid_3MRoll;
		DateVector dateGrid_6MRoll;
		DoubleArray termGrid_6MRoll;
		DateVector dateGrid_12MRoll;
		DoubleArray termGrid_12MRoll;

		LAInterpolationBase* pInter_3ML;
		LAInterpolationBase* pInter_6ML;
		LAInterpolationBase* pInter_DF;

		size_t shortTermSize_3ML;
		size_t shortTermSize_6ML;
		size_t shortTermSize_DF;		

		bool isUSD;
	};
};
