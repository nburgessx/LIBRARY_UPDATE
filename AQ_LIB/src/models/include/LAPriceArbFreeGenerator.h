#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include <AQLDataInstance.h>
#include "AQLMatrix.h"
#include <AQLMathDefine.h>
#include <AQLObject.h>
#include <AQLObjectHolder.h>
#include <AQLObjectPool.h>
#include <AQLDataBasics.h>
#include "AQLFunctionVector.h"
#include "AQLNl2sol.h"
#include <AQLDataProcedure.h>
#include <AQLPriceDataCalendar.h>
#include <AQLPriceDataSlidingRule.h>
#include <AQLPriceDataDayCount.h>
#include <AQLDataMultiReference.h>
#include <AQLPriceDataManager.h>
#include <AQLDataReference.h>
#include <AQLDataVector.h>
#include "AQLSplineInterpolation.h"
#include <AQLPriceDataInterpolation.h>
#include "AQLBasic.h"
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


class AQLObject;
class AQLDataProcedure;
class AQLPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class to generate DF Curve, 6m Libor Curve and 3m Libor Curve at the same time
*/
class LAPriceArbFreeGenerator : public AQLCoreProcedure
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
    virtual AQLCoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
   	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	// Return this class type
    virtual function_t          getType() const;
	// generate curve
    virtual void                calibrateModel(	const AQLDate& basedate, AQLObject& inst, 
											const AQLDataProcedure& att ) const;
	
	static void setForecastCurve(	AQLDataInstance* dataInstance,
									const AQLString& setUpCurveName );

    static void generateCdtDFCurve(	AQLDataInstance* dataInstance,
                                    const AQLString& arbFreeCurveID,
                                    const AQLString& forecastCurveID,
                                    const AQLString& ctdCurveID,
                                    const AQLDate& spotDate,
                                    AQLPriceDataCalendar& cal, 
                                    AQLPriceDataSlidingRule& sld, 
                                    AQLString& freq, 
                                    AQLPriceDataDayCount& dc,
                                    double spread );

	static	void generateSchedule(	const AQLDate& asOfDate,
							const AQLDate& spotDate,
							const AQLDate& endDate,
							unsigned int span,
							const AQLPriceDataCalendar* pCal,
							const AQLPriceDataSlidingRule& sld,
							DateVector& dates,
							DoubleArray& terms,
							bool isEOMRoll );

protected:
	// copy constructor    
    LAPriceArbFreeGenerator(const LAPriceArbFreeGenerator& p);

	//static void generateAUDCurve(	const AQLDate& basedate,
	//								const vector<AQLObject*>& mktData,
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
	//								AQLInterpolationBase* pInter_DF );

	static void generateUSDCurve(	const AQLDate& basedate,
									const vector<AQLObject*>& mktData,
									const AQLObject* fYieldData,
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
									AQLInterpolationBase* pInter_DF );

	static void generateCurve(	const AQLDate& basedate,
								const vector<AQLObject*>& mktData,
								const AQLObject* fYieldData,
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
								AQLInterpolationBase* pInter_yg );

	static	void insertRate(	double rate, 
								DoubleMatrix& termsMtx, 
								DoubleArray& rates, 
								DoubleArray& df_terms, 
								DoubleArray& dfs,
								double staTerm, 
								double endTerm, 
								const AQLDate& staDate, 
								const AQLDate& endDate, 
								const AQLPriceDataDayCount& dc );

	class MoneyMarket
	{
		public:
			MoneyMarket( vector<const AQLObject* > mktData );
			virtual ~MoneyMarket();

			AQLStringVector terms;
			DoubleArray rates;
			AQLPriceDataDayCount dc;
			AQLPriceDataSlidingRule sld;
			const AQLPriceDataCalendar* pCal;
	};

	class LiborMarket
	{
		public:
			LiborMarket( vector<const AQLObject* > mktData );
			virtual ~LiborMarket();
		
			double threeMLibor;
			double sixMLibor;
			AQLPriceDataDayCount dc;
			AQLPriceDataSlidingRule sld;
			const AQLPriceDataCalendar* pCal;
			bool isEOMRoll;
			AQLString roll_conv;
			AQLDate spotDate;
	};

	class SwapMarket
	{
		public:
			SwapMarket( vector<const AQLObject* > mktData, vector<const AQLObject* > tenorSwapMktData, const LiborMarket& libMkt, 
				const AQLDate& basedate );
			SwapMarket(const SwapMarket& v);
			virtual ~SwapMarket();
		
			AQLStringVector mktTerms_str;
			DoubleVector mktTerms;
			DoubleVector mktRates;
			AQLDate spotDate;
			AQLDate startDate;
			AQLDate endDate;
			AQLString endTerm;
			AQLPriceDataSlidingRule sld;
			const AQLPriceDataCalendar* pCal;
			AQLString freq_Float;
			AQLString freq_Fix;
			AQLString freq_Float_Pay;
			AQLPriceDataDayCount dc_Float;
			AQLPriceDataDayCount dc_Fix;
			AQLInterpolationBase* pInter;
			map<int, AQLString > map_freq_Fix;
			bool isEOMRoll;
			AQLString roll_conv;
			AQLString optimizeMethod;
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
			XCCYBasisMarket( vector<const AQLObject* > mktData, const AQLObject* fYieldData, const SwapMarket& swapMkt, bool& isRenAdj, 
				bool isUSD, const AQLDate& basedate );
			XCCYBasisMarket(const XCCYBasisMarket& v);
			virtual ~XCCYBasisMarket();
		
			AQLStringVector mktTerms_str;
			DoubleVector mktTerms;
			DoubleVector mktRates;
			AQLDate spotDate;
			AQLDate endDate;
			AQLPriceDataDayCount dc;
			AQLPriceDataSlidingRule sld;
			const AQLPriceDataCalendar* pCal;
			AQLInterpolationBase* pInter;
			AQLInterpolationBase* pInter_usd;
			AQLInterpolationBase* pInter_usd_3ML;
			AQLInterpolationBase* pInter_fPrices;
			AQLInterpolationBase* pInter_baseccydf;
			AQLInterpolationBase* pInter_adjust;
			bool isEOMRoll;
			AQLString roll_conv;
			DateVector dateGrid_3MRoll;
			DoubleArray termGrid_3MRoll;
	};

	// 3M6M Basis Market
	class LiborBasisMarket
	{
		public:
			LiborBasisMarket( vector<const AQLObject* > mktData, const SwapMarket& swapMkt, const AQLDate& basedate );
			LiborBasisMarket(const LiborBasisMarket& v);
			virtual ~LiborBasisMarket();
		
			AQLStringVector mktTerms_str;
			DoubleVector mktTerms;
			DoubleVector mktRates;
			AQLPriceDataDayCount dc_3L;
			AQLPriceDataDayCount dc_6L;
			AQLPriceDataSlidingRule sld;
			const AQLPriceDataCalendar* pCal;
			AQLDate spotDate;
			AQLDate endDate;
			AQLInterpolationBase* pInter;
			bool isEOMRoll;
			AQLString roll_conv;
			DateVector dateGrid_3MRoll;
			DateVector dateGrid_6MRoll;
			DoubleArray termGrid_3MRoll;
			DoubleArray termGrid_6MRoll;
			AQLString freq_3L_pay;
	};

	class FRAMarket
	{
		public:
			FRAMarket( vector<const AQLObject* > mktData, bool isFRAUse );
			virtual ~FRAMarket();
		
			AQLDate spotDate;
			AQLPriceDataDayCount dc;
			const AQLPriceDataCalendar* pCal;
			AQLPriceDataSlidingRule sld;
			std::map<AQLString, double > map_term_rate;
			bool isEOMRoll;
			AQLString roll_conv;
	};
	
	class FutureMarket
	{
		public:
			FutureMarket( vector<const AQLObject* > mktData, const SwapMarket& swapMkt, bool isFutureUse, const AQLDate& basedate );
			virtual ~FutureMarket();
		
			AQLPriceDataDayCount dc;
			DateVector startDates;
			DateVector endDates;
			DoubleVector mktRates;
	};

	static void calcCurve_Semi_Semi(	const AQLDate& basedate,
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

	static void calcCurve_Quar_Quar(	const AQLDate& basedate,
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

	static void calcCurve_Quar_Semi(	const AQLDate& basedate,
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

	static void calcCurve_Semi_Quar(	const AQLDate& basedate,
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

	static void calcCurve_Annu_Semi(	const AQLDate& basedate,
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

	static void calcCurve_Annu_Quar(	const AQLDate& basedate,
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

	static void calcCurve_NewtonRaphson(	const AQLDate&  basedate, 
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
											AQLInterpolationBase* pInter_yg,
											bool isUSD );

	static void calcCurve_NL2SOL(	const AQLDate&  basedate, 
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
									AQLInterpolationBase* pInter_yg,
									bool isUSD );

	static DoubleVector getSwapValue(	const LiborMarket& libMkt, 
										const SwapMarket& swapMkt, 
										const DateVector& dateGrid_3MRoll,
										const DoubleArray& termGrid_3MRoll,
										const DateVector& dateGrid_6MRoll,
										const DoubleArray& termGrid_6MRoll,
										const DateVector& dateGrid_12MRoll,
										const DoubleArray& termGrid_12MRoll,
										AQLInterpolationBase* pInter_3ML,
										AQLInterpolationBase* pInter_6ML,
										AQLInterpolationBase* pInter_DF );

	static DoubleVector getXccyBasisValue(	const AQLDate&  basedate, 
											bool isRenAdj, 
											const LiborMarket& libMkt, 
											const SwapMarket& swapMkt, 
											const XCCYBasisMarket& xccyBasisMkt,
											const DateVector& dateGrid_3MRoll,
											const DoubleArray& termGrid_3MRoll,
											AQLInterpolationBase* pInter_3ML,
											AQLInterpolationBase* pInter_6ML,
											AQLInterpolationBase* pInter_DF,
											bool isUSD );

	static DoubleVector getLibBasisValue(	const LiborMarket& libMkt, 
											const SwapMarket& swapMkt, 
											const LiborBasisMarket& libBasisMkt,
											const DateVector& dateGrid_3MRoll,
											const DoubleArray& termGrid_3MRoll,
											const DateVector& dateGrid_6MRoll,
											const DoubleArray& termGrid_6MRoll,
											AQLInterpolationBase* pInter_3ML,
											AQLInterpolationBase* pInter_6ML,
											AQLInterpolationBase* pInter_DF );

	static void baseDateAdjust(	const AQLDate& basedate,
								const AQLDate& spotDate,
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

	static void liborDateAdjust(	const AQLDate& basedate,
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
									AQLInterpolationBase* pInter );

	static void insertFRA(	const AQLDate& basedate,
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
							AQLInterpolationBase* pInter );

	static void insertFuture(	const AQLDate& basedate,
								const FutureMarket& futureMkt,
								const SwapMarket& swapMkt,
								DoubleMatrix& threeMLTermsMtx_Rate,
								DoubleArray& threeMLRate,
								DoubleArray& threeMLTerms_DF,
								DoubleArray& threeMLDF,
								unsigned int& threeMLSPos,
								AQLInterpolationBase* pInter );

	static bool insertToVector( double in1,
								double in2,
								DoubleArray& array1, 
								DoubleArray& array2,
								unsigned int& pos);

	static bool getPositionOfVector( double target,
									 DoubleArray& vec, 
									 double error,
									 unsigned int& pos);

	static double getSpotAdjust(	const AQLDate& basedate,
									const AQLDate& spotDate,
									const MoneyMarket& monMkt );

	static AQLString getSwapFixFrequency(unsigned int month, const SwapMarket& swapMkt);

	static void	setCurveConvention(	AQLObjectHolder& objHolder,
								std::vector<AQLObject*>& mktData,
								const AQLStringVector& curveNames_3ML,
								const AQLStringVector& curveNames_6ML);

	// class for generating arb free curve by NL2Sol optimize mothod
	class LAMathAFCurveCalibrator : public AQLFunctionVector
	{
	public:
		// constructor
		LAMathAFCurveCalibrator(
			const AQLDate& basedate_,
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
			AQLInterpolationBase* pInter_3ML_, 
			AQLInterpolationBase* pInter_6ML_, 
			AQLInterpolationBase* pInter_DF_, 
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
		AQLDate basedate;

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

		AQLInterpolationBase* pInter_3ML;
		AQLInterpolationBase* pInter_6ML;
		AQLInterpolationBase* pInter_DF;

		size_t shortTermSize_3ML;
		size_t shortTermSize_6ML;
		size_t shortTermSize_DF;		

		bool isUSD;
	};
};
