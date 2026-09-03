#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreProcedure.h"
#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataMultiReference.h"
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


class AQLObject;
class AQLDataProcedure;
class AQLInterpolationBase;
class AQLPriceDataManager;
class AQLPriceDataCalendar;
class AQLPriceDataDayCount;
class AQLPriceDataSlidingRule;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class to calc IR DiscoutFactor
*/
class AQLPriceYieldGenerator : public AQLCoreProcedure
{
public:
//  LIFECYCLE
    // constructor	
    AQLPriceYieldGenerator();
    // destructor	    
    virtual ~AQLPriceYieldGenerator();
    // Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
   	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	// Return this class type
    virtual function_t          getType() const;
    // calibrate curve
    virtual void                calibrateModel(const AQLDate& basedate, 
                                        AQLObject& object, 
                                        const AQLDataProcedure& att) const;
    // function to calc DiscountFactor
    static void                 calcDiscountFactor(const AQLDate& basedate,
										std::vector<AQLObject*>& data,
										const AQLObjectHolder& objHolder,
										const AQLString& tragetMarketName,
                                        DoubleArray& terms,
										DoubleArray& dfs,
										DoubleMatrix& fwd_termsmtx,
										DoubleArray& fwds,
										double& interpolationJoinDateAsDouble,
										AQLDate& interpolationJoinDate,
										unsigned int& fwd_samegridstart_pos,
										const bool is_f_use,										
										const bool is_fra_use,
										AQLInterpolationBase* pInter,
										AQLInterpolationBase* pInter_yg,
										AQLInterpolationBase* pInter_fw,
										const bool isSwapTenorAdjust,
										AQLInterpolationBase* pInter_ts,
										const AQLStringVector* pRatePriority = 0,
										const AQLString* pDFCurveName = 0);

	static AQLString				changeFRATermFormat(const AQLString& inputTerm);

	static AQLString				deduceRollConvention(const AQLString& freq, bool eom);

	static void					getPaymentDates(const AQLDate& basedate, const AQLDate& sdate, const AQLDate& edate, const AQLString& freq,
									const AQLPriceDataCalendar& cal, const AQLPriceDataSlidingRule& sld, const AQLPriceDataDayCount& dc,
									DateVector& dates, DoubleArray& terms, DoubleArray& terms_interval, bool is_eomroll,
									bool isBackward = false);

	static void					getPaymentDates(const AQLDate& sdate, const AQLDate& edate, const AQLString& freq,
									const AQLPriceDataCalendar& cal, const AQLPriceDataSlidingRule& sld, const AQLPriceDataDayCount& dc,
									DateVector& dates, DoubleArray& terms, DoubleArray& terms_interval, bool is_eomroll, 
									bool isBackward = false);

protected:
    // copy constructor    
    AQLPriceYieldGenerator(const AQLPriceYieldGenerator& p);

	static void                 calcOISDiscountFactor(const AQLDate& basedate,
										std::vector<AQLObject*>& data,
										const AQLObjectHolder& objHolder,
                                        DoubleArray& terms,
										DoubleArray& dfs,
										const bool is_f_use,										
										AQLInterpolationBase* pInter,
										AQLInterpolationBase* pInter_yg,
										AQLInterpolationBase* pInter_fw,
										const AQLString* pDFCurveName = 0);
	
	static double                solveOISRateS(const AQLDate& startdate,
											const AQLDate& enddate,
											const AQLPriceDataDayCount& dc,
											const AQLPriceDataCalendar& cal,
											double market_rate,
											const std::map<AQLDate, double> &onforward_map);

	static double                solveOISRateSAverage(const AQLDate& startdate,
											const AQLDate& enddate,
											const AQLPriceDataDayCount& dc,
											const AQLPriceDataCalendar& cal,
											double market_rate,
											const std::map<AQLDate, double> &onforward_map,
											bool compoundAllDays);

	static double                solveOISRate(const AQLDate& startdate,
											const AQLDate& enddate,
											const AQLPriceDataDayCount& dc,
											const AQLPriceDataCalendar& cal,
											double market_rate);

	static double                solveOISRateAverage(const AQLDate& startdate,
											const AQLDate& enddate,
											const AQLPriceDataDayCount& dc,
											const AQLPriceDataCalendar& cal,
											double market_rate,
											bool compoundAllDays);

	static double                calcSettleRate(const AQLDate& startdate,
											const AQLDate& enddate,
											const AQLPriceDataDayCount& dc,
											const AQLPriceDataCalendar& cal,
											double onforward_rate);

	static double                calcSettleRates(const DateVector& startdates,
											const DateVector& enddates,
											const DoubleVector& onforward_rates,
											const AQLPriceDataDayCount& dc);

	static double                calcSettleRatesA(const DoubleVector& startterms,
											const DoubleVector& endterms,
											const AQLInterpolationBase* pInter,
											const DoubleVector& onrateaccrualterms,
											const DoubleVector& weights);

	static double                calcSettleRateDailyAverage(const AQLDate& startdate,
											const AQLDate& enddate,
											const AQLPriceDataDayCount& dc,
											const AQLPriceDataCalendar& cal,
											double onforward_rate,
											bool compoundAllDays);

	static double                calcSettleRateDailyAverage(const DateVector& startdates,
											const DateVector& enddates,
											const DoubleVector& onforward_rates,
											const AQLPriceDataDayCount& dc,
											const AQLPriceDataCalendar& cal,
											bool compoundAllDays);

	static void					insertDFData(DoubleMatrix& dfs, 
											 DateVector& dates, 
											 double df_insert, 
											 double term_insert, 
											 const AQLDate date_insert);
	
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

	static double				calcFloatSide(AQLInterpolationBase &inter, 
										 AQLInterpolationBase &df_inter, 
										 const double df_spot,
										 const double term_spot,
										 const DoubleArray &term_grid,
										 const int cpd_times);

	static void                 updateImpliedForwardRates(const AQLInterpolationBase &inter,
		                                 const DoubleArray &grid_swap,
										 const DoubleArray &tau_swap,
										 DoubleMatrix &fwd_termsmtx, 
										 DoubleArray &fwds);

	static double				getAverageRateFromHistRates(const AQLDate& startdate,
												const AQLDate& enddate,
												const AQLPriceDataDayCount& dc,
												const AQLPriceDataCalendar& cal,
												double market_rate,
												AQLObject* data,
												const AQLDate& basedate,
												bool istodayrateexist,
												double todayffrate);

	static void					setCurveConvention(AQLObjectHolder& objHolder,
												   std::vector<AQLObject*>& mktData,
												   const AQLString& curveName);

	static void                 setAddtionalCalibGridForTenorBasis(const AQLDataMultiReference& mr_mktdata,
		                                                           const AQLDataMultiReference& mr_mktdata_ts,
													               AQLString& addtionalCalibGrid);

	//  This struct is needed for the insertSyntheticTensionPoints function to carry the tension iteration data
    struct tensionMarketData
    {
        // Default Struct Constructor to intialize the iteration count to zero
        tensionMarketData() :   frontForwardRate( 0.0 ),
                                frontStartDate( AQLDate() ),
                                frontEndDate( AQLDate() ),
                                frontStartTerm( 0.0 ),
                                frontEndTerm( 0.0 ),
                                backForwardRate( 0.0 ),
                                backStartDate( AQLDate() ),
                                backEndDate( AQLDate() ),
                                backStartTerm( 0.0 ),
                                backEndTerm( 0.0 )  {}

        double      frontForwardRate;
        AQLDate      frontStartDate;
        AQLDate      frontEndDate;
        double      frontStartTerm;
        double      frontEndTerm;
        double      backForwardRate;
        AQLDate      backStartDate;
        AQLDate      backEndDate;
        double      backStartTerm;
        double      backEndTerm;
    };

	// Function to insert synthetic discount factors and forward data points to simulate and apply tension
    static void insertSyntheticTensionPoints(           DateVector &            discountFactorDatesVector,              // [Output]
                                                        DoubleMatrix &          discountFactorMatrix,                   // [Output]
                                                        DoubleArray &           forwardRatesVector,                     // [Output]
                                                        DoubleMatrix &          forwardTermsMatrix,                     // [Output]
                                                        AQLInterpolationBase *   discountFactorInterpolationTable,       // [Input]
                                                const   AQLDate &                spotDate,                               // [Input]
                                                const   AQLDate &                startDate,                              // [Input]
                                                const   AQLDate &                endDate,                                // [Input]
                                                const   double &                forwardRate,                            // [Input]
                                                const   bool &                  isFirstDataPoint,                       // [Input]
                                                const   AQLPriceDataDayCount &        termsToDateDaycount,                    // [Input]
                                                const   AQLPriceDataDayCount &        instrumentDaycount,                     // [Input]
                                                const   unsigned int &          tensionGap,                             // [Input]
                                                        tensionMarketData &     instrumentData,                         // [Input]      ( tensionMarketData struct used here )
                                                const   AQLDate &                cutOffDate = AQLDate() );                // [Input]


private:
	// calibrate DiscountFactor
    void                estimateSwapDFCurve(const AQLDate& basedate, 
											AQLObject& object, 
											const AQLDataProcedure& att,
											const AQLString& dfCurveName) const;

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
	static void calculateFraDates(AQLDate& startDate,
		AQLDate& endDate,
		const AQLDate& spotDate,
		const AQLObject* data_fra,
		const AQLString& refRateTerm,
		const AQLString& baseFreq,
		const AQLPriceDataDayCount& dc);

	/*!
		@brief Determine the join date used by linear spline interpolation
		@param[in] lastFuture	Last future calibration instrument
		@param[in] firstSwap	First swap calibration instrument
		@param[in] spotDate		Spot date
		@param[in] is_fwdswap	Is forward starting swap used?
		@return	   Linear spline join date
	*/
	static AQLDate determineLinearSplineInterpolationJoinDate(const AQLObject* lastFuture,
													  const AQLObject* firstSwap,
													  const AQLDate& spotDate,
													  bool is_fwdswap);
};

