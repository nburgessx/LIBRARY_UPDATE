#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreProcedure.h"
#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADataMultiReference.h"
#include <vector>

            
// irYieldGenerator Function id
#define FN_IRYIELDGENERATOR     305
// irYieldGenerator Function Name
#define FN_IRYIELDGENERATOR_STR "fn_iryieldgenerator"

#ifndef PRICING_DATA_PRICE
#define PRICING_DATA_PRICE			"Price"					//  data name of Price
#endif
#ifndef PRICING_DATA_STARTDATE
#define PRICING_DATA_STARTDATE		"StartDate"				//  data name of StartDate
#endif
#ifndef PRICING_DATA_ENDDATE
#define PRICING_DATA_ENDDATE		"EndDate"				//  data name of EndDate
#endif
#ifndef PRICING_DATA_SPREAD
#define PRICING_DATA_SPREAD			"Spread"				//  data name of Spread
#endif
#ifndef PRICING_DATA_FUTUREVOLATILITY
#define PRICING_DATA_FUTUREVOLATILITY	"FutureVolatility"	// data Name of FutureVolatility
#endif
#ifndef PRICING_DATA_RATEPRIORITY
#define PRICING_DATA_RATEPRIORITY	"RatePriority"	// data Name of rate priority 
#endif
#ifndef PRICING_DATA_FUTUREOLDMETHOD
#define PRICING_DATA_FUTUREOLDMETHOD	"FutureOldMethod"	//  data name of Spread
#endif

#ifndef PRICING_DATA_ISFWDSWAP
#define PRICING_DATA_ISFWDSWAP	 "IsFwdSwap"		//  data name of IsFwdSwap
#endif
#ifndef PRICING_DATA_ISDATE
#define PRICING_DATA_ISDATE	 "IsDate"		//  data name of IsDate
#endif
#ifndef PRICING_DATA_STARTTERM
#define PRICING_DATA_STARTTERM		"StartTerm"				//  data name of Start Term
#endif
#ifndef PRICING_DATA_TENOR
#define PRICING_DATA_TENOR		"Tenor"				//  data name of Tenor
#endif

#ifndef STD
#define STD	"STD"
#endif
#ifndef ITSELF
#define ITSELF	"ITSELF"
#endif


class LAObject;
class LADataProcedure;
class LAInterpolationBase;
class LAPriceDataManager;
class LAPriceDataCalendar;
class LAPriceDataDayCount;
class LAPriceDataSlidingRule;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class to calc IR DiscoutFactor
*/
class LAPriceYieldGenerator : public LACoreProcedure
{
public:
//  LIFECYCLE
    // constructor	
    LAPriceYieldGenerator();
    // destructor	    
    virtual ~LAPriceYieldGenerator();
    // Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
   	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;
	// Return this class type
    virtual function_t          getType() const;
    // calibrate curve
    virtual void                calibrateModel(const LADate& basedate, 
                                        LAObject& object, 
                                        const LADataProcedure& att) const;
    // function to calc DiscountFactor
    static void                 calcDiscountFactor(const LADate& basedate,
										std::vector<LAObject*>& data,
										const LAObjectHolder& objHolder,
										const LAString& tragetMarketName,
                                        DoubleArray& terms,
										DoubleArray& dfs,
										DoubleMatrix& fwd_termsmtx,
										DoubleArray& fwds,
										double& interpolationJoinDateAsDouble,
										LADate& interpolationJoinDate,
										unsigned int& fwd_samegridstart_pos,
										const bool is_f_use,										
										const bool is_fra_use,
										LAInterpolationBase* pInter,
										LAInterpolationBase* pInter_yg,
										LAInterpolationBase* pInter_fw,
										const bool isSwapTenorAdjust,
										LAInterpolationBase* pInter_ts,
										const LAStringVector* pRatePriority = 0,
										const LAString* pDFCurveName = 0);

	static LAString				changeFRATermFormat(const LAString& inputTerm);

	static LAString				deduceRollConvention(const LAString& freq, bool eom);

	static void					getPaymentDates(const LADate& basedate, const LADate& sdate, const LADate& edate, const LAString& freq,
									const LAPriceDataCalendar& cal, const LAPriceDataSlidingRule& sld, const LAPriceDataDayCount& dc,
									DateVector& dates, DoubleArray& terms, DoubleArray& terms_interval, bool is_eomroll,
									bool isBackward = false);

	static void					getPaymentDates(const LADate& sdate, const LADate& edate, const LAString& freq,
									const LAPriceDataCalendar& cal, const LAPriceDataSlidingRule& sld, const LAPriceDataDayCount& dc,
									DateVector& dates, DoubleArray& terms, DoubleArray& terms_interval, bool is_eomroll, 
									bool isBackward = false);

protected:
    // copy constructor    
    LAPriceYieldGenerator(const LAPriceYieldGenerator& p);

	static void                 calcOISDiscountFactor(const LADate& basedate,
										std::vector<LAObject*>& data,
										const LAObjectHolder& objHolder,
                                        DoubleArray& terms,
										DoubleArray& dfs,
										const bool is_f_use,										
										LAInterpolationBase* pInter,
										LAInterpolationBase* pInter_yg,
										LAInterpolationBase* pInter_fw,
										const LAString* pDFCurveName = 0);
	
	static double                solveOISRateS(const LADate& startdate,
											const LADate& enddate,
											const LAPriceDataDayCount& dc,
											const LAPriceDataCalendar& cal,
											double market_rate,
											const std::map<LADate, double> &onforward_map);

	static double                solveOISRateSAverage(const LADate& startdate,
											const LADate& enddate,
											const LAPriceDataDayCount& dc,
											const LAPriceDataCalendar& cal,
											double market_rate,
											const std::map<LADate, double> &onforward_map,
											bool compoundAllDays);

	static double                solveOISRate(const LADate& startdate,
											const LADate& enddate,
											const LAPriceDataDayCount& dc,
											const LAPriceDataCalendar& cal,
											double market_rate);

	static double                solveOISRateAverage(const LADate& startdate,
											const LADate& enddate,
											const LAPriceDataDayCount& dc,
											const LAPriceDataCalendar& cal,
											double market_rate,
											bool compoundAllDays);

	static double                calcSettleRate(const LADate& startdate,
											const LADate& enddate,
											const LAPriceDataDayCount& dc,
											const LAPriceDataCalendar& cal,
											double onforward_rate);

	static double                calcSettleRates(const DateVector& startdates,
											const DateVector& enddates,
											const DoubleVector& onforward_rates,
											const LAPriceDataDayCount& dc);

	static double                calcSettleRatesA(const DoubleVector& startterms,
											const DoubleVector& endterms,
											const LAInterpolationBase* pInter,
											const DoubleVector& onrateaccrualterms,
											const DoubleVector& weights);

	static double                calcSettleRateDailyAverage(const LADate& startdate,
											const LADate& enddate,
											const LAPriceDataDayCount& dc,
											const LAPriceDataCalendar& cal,
											double onforward_rate,
											bool compoundAllDays);

	static double                calcSettleRateDailyAverage(const DateVector& startdates,
											const DateVector& enddates,
											const DoubleVector& onforward_rates,
											const LAPriceDataDayCount& dc,
											const LAPriceDataCalendar& cal,
											bool compoundAllDays);

	static void					insertDFData(DoubleMatrix& dfs, 
											 DateVector& dates, 
											 double df_insert, 
											 double term_insert, 
											 const LADate date_insert);
	
	static void                 insertForwardRateData( DoubleMatrix &fwd_termsmtx,
                                                       DoubleArray &fwds,
                                                       const double &insesrtStartTerm,
                                                       const double &insertEndTerm,
                                                       const double &insertFwdRate );

    static void                 insertForwardRatesData( DoubleMatrix &fwd_termsmtx,
                                                        DoubleArray &fwds,
                                                        const DoubleArray &insertStartTerm,
                                                        const DoubleArray &insertEndTerms,
                                                        const DoubleArray &insertFwdRates );

	static double				calcFloatSide(LAInterpolationBase &inter, 
										 LAInterpolationBase &df_inter, 
										 const double df_spot,
										 const double term_spot,
										 const DoubleArray &term_grid,
										 const int cpd_times);

	static void                 updateImpliedForwardRates(const LAInterpolationBase &inter,
		                                 const DoubleArray &grid_swap,
										 const DoubleArray &tau_swap,
										 DoubleMatrix &fwd_termsmtx, 
										 DoubleArray &fwds);

	static double				getAverageRateFromHistRates(const LADate& startdate,
												const LADate& enddate,
												const LAPriceDataDayCount& dc,
												const LAPriceDataCalendar& cal,
												double market_rate,
												LAObject* data,
												const LADate& basedate,
												bool istodayrateexist,
												double todayffrate);

	static void					setCurveConvention(LAObjectHolder& objHolder,
												   std::vector<LAObject*>& mktData,
												   const LAString& curveName);

	static void                 setAddtionalCalibGridForTenorBasis(const LADataMultiReference& mr_mktdata,
		                                                           const LADataMultiReference& mr_mktdata_ts,
													               LAString& addtionalCalibGrid);

	//  This struct is needed for the insertSyntheticTensionPoints function to carry the tension iteration data
    struct tensionMarketData
    {
        // Default Struct Constructor to intialize the iteration count to zero
        tensionMarketData() :   frontForwardRate( 0.0 ),
                                frontStartDate( LADate() ),
                                frontEndDate( LADate() ),
                                frontStartTerm( 0.0 ),
                                frontEndTerm( 0.0 ),
                                backForwardRate( 0.0 ),
                                backStartDate( LADate() ),
                                backEndDate( LADate() ),
                                backStartTerm( 0.0 ),
                                backEndTerm( 0.0 )  {}

        double      frontForwardRate;
        LADate      frontStartDate;
        LADate      frontEndDate;
        double      frontStartTerm;
        double      frontEndTerm;
        double      backForwardRate;
        LADate      backStartDate;
        LADate      backEndDate;
        double      backStartTerm;
        double      backEndTerm;
    };

	// Function to insert synthetic discount factors and forward data points to simulate and apply tension
    static void insertSyntheticTensionPoints(           DateVector &            discountFactorDatesVector,              // [Output]
                                                        DoubleMatrix &          discountFactorMatrix,                   // [Output]
                                                        DoubleArray &           forwardRatesVector,                     // [Output]
                                                        DoubleMatrix &          forwardTermsMatrix,                     // [Output]
                                                        LAInterpolationBase *   discountFactorInterpolationTable,       // [Input]
                                                const   LADate &                spotDate,                               // [Input]
                                                const   LADate &                startDate,                              // [Input]
                                                const   LADate &                endDate,                                // [Input]
                                                const   double &                forwardRate,                            // [Input]
                                                const   bool &                  isFirstDataPoint,                       // [Input]
                                                const   LAPriceDataDayCount &        termsToDateDaycount,                    // [Input]
                                                const   LAPriceDataDayCount &        instrumentDaycount,                     // [Input]
                                                const   unsigned int &          tensionGap,                             // [Input]
                                                        tensionMarketData &     instrumentData,                         // [Input]      ( tensionMarketData struct used here )
                                                const   LADate &                cutOffDate = LADate() );                // [Input]


private:
	// calibrate DiscountFactor
    void                estimateSwapDFCurve(const LADate& basedate, 
											LAObject& object, 
											const LADataProcedure& att,
											const LAString& dfCurveName) const;

	/*
	@brief Calculate start and end dates of a FRA instrument

	@param[out] startDate		FRA start date
	@param[out] endDate			FRA end date
	@param[in]  spotDate		Spot date of curve
	@param[in]  data_fra		FRA instrument
	@param[in]	refRateTerm		The tenor of the curve
	@param[in]	baseFreq		The frequency of the curve
	@param[in]	dc				FRA day count
	*/
	static void calculateFraDates(LADate& startDate,
		LADate& endDate,
		const LADate& spotDate,
		const LAObject* data_fra,
		const LAString& refRateTerm,
		const LAString& baseFreq,
		const LAPriceDataDayCount& dc);

	/*!
		@brief Determine the join date used by linear spline interpolation
		@param[in] lastFuture	Last future calibration instrument
		@param[in] firstSwap	First swap calibration instrument
		@param[in] spotDate		Spot date
		@param[in] is_fwdswap	Is forward starting swap used?
		@return	   Linear spline join date
	*/
	static LADate determineLinearSplineInterpolationJoinDate(const LAObject* lastFuture,
													  const LAObject* firstSwap,
													  const LADate& spotDate,
													  bool is_fwdswap);
};

