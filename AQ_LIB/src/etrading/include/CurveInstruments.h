//
//  CurveInstruments.h
//  This file was previously called YieldCurveBuildingUtilities.h
//
#pragma once

#include "LACoreTemplateType.h"
#include "CoreEnumerations.h"
#include <vector>
#include <unordered_map>

using etrading::StateVariableEnum;
using etrading::STATE_VARIABLE_ZERO_RATE;
using etrading::STATE_VARIABLE_ZERO_RATE_TIMES_TIME;
using etrading::STATE_VARIABLE_LOG_DF;
using etrading::STATE_VARIABLE_DF;

// Class Forward Declarations
class LAStaticData;
class CurveCalibrationData;
class LAObjectPool;
class LAInterpolationBase;
class LAPriceDataDayCount;
class LAObject;
class LAPriceDataInterpolation;
class LAPriceDataConvention;
class LAPriceDataSlidingRule;
class LAPriceDataCalendar;

// Type Definitions
// **************************************************************************

// TypeDef: Start- and End-date
typedef std::pair<LADate, LADate> StartAndEndDate;

// TypeDef: Money Market Data Map with Key = Start & End Date, Value = LAObject data
typedef std::map<StartAndEndDate, const LAObject*>	MoneyMarketData;

// **************************************************************************

namespace etrading
{
	// Struct: Interpolation Join Date - Success and JoinDateAsDouble
	struct InterpolationJoinDate
	{
		bool success_;
		double joinDateAsDouble_;
	};

	class ForwardQuotes
	{
		public:

		ForwardQuotes() {};
		~ForwardQuotes() {};

		DateVector		startDates_;
		DateVector		endDates_;
		DoubleVector	marketRates_;
	};

	class DiscountFactors
	{
		public:

		DiscountFactors()
			: paymentDatesAsTerms_(DoubleVector()), discountFactors_ (DoubleVector()), paymentDates_(DateVector()), lastLiborEndDate_(LADate()), lastLiborPosition_(0)
			{};
		
		~DiscountFactors()	{};

		// Legacy Get Accessor for backwards compatibility
		// Non-const as we need to update termsDFMatrix_
		DoubleMatrix termsDFMatrix(); 

		// Legacy Set Accessor for backwards compatibility
		void termsDFMatrix( const DoubleMatrix & termsDFMatrix );

		public :
		
		DoubleVector	paymentDatesAsTerms_;
		DoubleVector	discountFactors_;
		DateVector		paymentDates_;
		LADate			lastLiborEndDate_;
		size_t			lastLiborPosition_;
		
		// Private member since we must keep paymentDatesAsTerms and discountFactors in synch with termsDFMatrix
		// The synchronization is managed by the set and get accesors for this member variable
		private:
		
		DoubleMatrix	termsDFMatrix_;
	};

	/* @brief	This struct is needed for the insertSyntheticTensionPoints function to carry the tension iteration data
	*/
	struct tensionMarketData
	{
		// Default Struct Constructor to intialize the iteration count to zero
		tensionMarketData() : frontForwardRate(0.0),
			frontStartDate(LADate()),
			frontEndDate(LADate()),
			frontStartTerm(0.0),
			frontEndTerm(0.0),
			backForwardRate(0.0),
			backStartDate(LADate()),
			backEndDate(LADate()),
			backStartTerm(0.0),
			backEndTerm(0.0) {}

		double frontForwardRate;
		LADate frontStartDate;
		LADate frontEndDate;
		double frontStartTerm;
		double frontEndTerm;
		double backForwardRate;
		LADate backStartDate;
		LADate backEndDate;
		double backStartTerm;
		double backEndTerm;
	};

	struct ForwardRate
	{
		LADate startDate;
		LADate endDate;
		double fwdRate;
	};

	/*
		@brief insert df, term and date into df data

		@param[out]		dfTerms			Discount factor terms (year fractions)
		@param[out]		dfValues		Discount factor values
		@param[out]		dfDates			Discount factor dates
		@param[in]		df_insert		The discount factor being inserted
		@param[in]		term_insert		Date fraction from spot/as-of date to the discount factor date
		@param[in]		date_insert		Discount factor date
		@param[out]		yields			Yields
		@param[in]		yield_insert	Yield to be insert
		@param[in]		overrideIfDatesClash	Override DF(DiscountFactor)/Yields if dates clashed
	*/
	void insertDFData(DoubleVector& dfTerms, DoubleVector& dfValues, DateVector& dfDates, double df_insert, double term_insert, const LADate& date_insert, DoubleVector& yields, const double yield_insert, const bool overrideIfDatesClash = false);

	// Update dfResults, insert df, term and date info data into the dfResults
	void insertDFData( DiscountFactors & dfResults, double df_insert, double term_insert, const LADate& date_insert, const bool overrideIfDatesClash = false );

	/*
		@brief insert df, term and date into df data

		@param[out]		dfs				Object into which discount factor is to be inserted
		@param[out]		dates			Date vector that correspond to the discount factor vector
		@param[in]		df_in			The discount factor being inserted
		@param[in]		term_in			Date fraction from spot/as-of date to the discount factor date
		@param[in]		date_in			Discount factor date
		@param[in]		overrideIfDatesClash	Override DF(DiscountFactor) if dates clashed
	*/
	void insertDFData(DoubleMatrix& dfs, DateVector& dates, double df_insert, double term_insert, const LADate& date_insert, const bool overrideIfDatesClash = false);

	/*
		@brief Function to insert a ***SINGLE*** additional forward rate into an existing forward rate array in a sorted manner. Used for insertion of tension data points

		@param[out]		fwd_termsmtx		Forward rate matrix where new forward rate is being inserted
		@param[out]		fwds				Dates that correspond to all forward rates
		@param[in]		insertStartTerm		Date fraction from spot/as-of date to start date of the forward rate term
		@param[in]		insertEndTerm		Date fraction from spot/as-of date to end date of the forward rate term
		@param[in]		insertFwdRate		Forward rate being inserted
	*/
	void insertForwardRateData(DoubleMatrix &fwd_termsmtx,
							   DoubleArray &fwds,
							   const double &insertStartTerm,
							   const double &insertEndTerm,
							   const double &insertFwdRate);

	/*
		@brief Function to insert ***MULTIPLE*** additional forward rates into an existing forward rate array in a sorted manner. Used for insertion of tension data points

		@param[out]		fwd_termsmtx		Forward rate matrix where new forward rate is being inserted
		@param[out]		fwds				Dates that correspond to all forward rates
		@param[in]		insertStartTerm		Date fractions from spot/as-of date to start date of the forward rate terms
		@param[in]		insertEndTerm		Date fractions from spot/as-of date to end date of the forward rate terms
		@param[in]		insertFwdRate		Forward rates being inserted
	*/
	void insertForwardRatesData(DoubleMatrix &fwd_termsmtx,
								DoubleArray &fwds,
								const DoubleArray &insesrtStartTerms,
								const DoubleArray &insertEndTerms,
								const DoubleArray &insertFwdRates);


	/* @brief			Insert artificial tension between Future/FRA points
	*  @param [out]		dfResults							dfResults
	*  @param [out]		forwardRatesVector					Forward rates resulted from tension
	*  @param [out]		forwardTermsMatrix					Dates on which forward rates have been computed
	*  @param [out]		discountFactorInterpolationTable	Discount factor interpolation object
	*  @param [in]		stateVariable				        stateVariable
	*  @param [in]		asOfDate							Yield curve asOfDate
	*  @param [in]		spotDate							Yield curve spot date
	*  @param [in]		startDate							Start date of the current Future/FRA
	*  @param [in]		endDate								End date of the current Future/FRA
	*  @param [in]		forwardRate							Forward rate of the current Future/FRA
	*  @param [in]		isFirstDataPoint					Is this the first Future/FRA?
	*  @param [in]		termsToDateDayCount					Date count convention from spot date to current date, usually ACT/365
	*  @param [in]		instrumentDayCount					Date count convention between instrument dates
	*  @param [in]		tensionGap							Gap in nummber of days between artificial forward instruments
	*  @param [in]		instrumentData						Object incapsulating Future/FRA instrument data
	*  @param [in]		cutoffDate							When tension should cease to be applied
	*  @param [in]		implyForwards						Imply forward rates from instruments?

	*/
	void insertSyntheticTensionPoints( DiscountFactors & dfResults,
									   DoubleArray& forwardRatesVector,
									   DoubleMatrix& forwardTermsMatrix,
									   LAInterpolationBase* discountFactorInterpolationTable,
									   const StateVariableEnum& stateVariable,
									   const LADate & asOfDate,
									   const LADate& spotDate,
									   const LADate& startDate,
									   const LADate& endDate,
									   const double& forwardRate,
									   const bool& isFirstDataPoint,
									   const LAPriceDataDayCount& termsToDateDaycount,
									   const LAPriceDataDayCount& instrumentDaycount,
									   const unsigned int& tensionGap,
									   tensionMarketData& instrumentData,
									   const LADate& cutoffDate,
									   bool implyForwards = true);


	/* @brief			Group all input FRA market data into a file to be used later in yield curve calibration
	*  @param [in]		fraFileName		Name of FRA market data file
	*  @param [in]		fraRates		FRA market quotes
	*  @param [in]		areSwapsForwardStarting		Will FRA instruments have user-specified start and end dates?
	*  @param [inout]	useGrid_FRA		A string used for market data validation
	*  @return			File carrying FRA market data
	*/
	LAString buildFRAMarketDataFile(const LAString& fraFileName,
									const LAStringMatrix& fraRates,
									bool areSwapsForwardStarting,
									LAString& useGrid_FRA);


	/* @brief			Populate FRA data into object pool
	*  @param [inout]	mpStaticData		Property manager object
	*  @param [inout]	curveCalibrationData				CurveCalibrationData object
	*  @param [inout]	refData				Reference data
	*  @param [inout]	objPool					Reference object to the object pool
	*  @param [in]		currency			Curve currency
	*  @param [in]		marketName			Market name of curve
	*  @param [in]		yieldDataName		Yield data name
	*  @param [in]		staticDataSuffix			Suffix used with property manager
	*  @param [in]		suffix_data			Suffix used with object pool
	*  @param [in]		isAudExtra			Australian curve flag
	*  @param [in]		isSpotUse			Use use given spot date?
	*  @param [in]		areSwapsForwardStarting			Instruments have user specified start dates?
	*  @param [in]		asOfDate			As of date of curve
	*  @param [in]		isBasisCurve		Is the current curve a basis curve?
	*/
	void populateFRADataToEntityPool(LAStaticData * mpStaticData,
									 CurveCalibrationData &curveCalibrationData,
									 LAString& refData,
									 LAObjectPool& objPool,
									 const LAString& currency,
									 const LAString& marketName,
									 const LAString& yieldDataName,
									 const LAString& staticDataSuffix,
									 const LAString& suffix_data,
									 bool isAudExtra,
									 bool isSpotUse,
									 bool areSwapsForwardStarting,
									 const LADate &asOfDate,
									 bool isBasisCurve = false);

	/*
		@brief change FRA term from "X" format to "M" format
		@param[in] term in "X" format
		@return term in "M" format
	*/
	LAString changeFRATermFormat(const LAString& inputTerm);


	/*
		@brief Bootstrap yield curve using FRA instruments producing discount factors and forward rates

		@param[out] dfResults				dfResults
		@param[out] fwd						Forward rates being generated
		@param[out] fwd_termsmtx			Forward rate dates
		@param[in] data_fra					FRA instruments
		@param[in] data_libor				Libor instruments
		@param[in] pInter_fw				Interpolator specific to FRAs/Futures
		@param[in] stateVariable			StateVariable
		@param[in] is_fwdswap				Using forward starting instruments?
		@param[in] frequency				Base frequency
		@param[in] liborIndexTerm			Libor Index Term
		@param[in] asOfDate					curve AsOfDate
		@param[in] spotdate					Spot date of the curve
		@param[in] spotDateLibor			Spot date of the Libor instruments
		@param[in] firstSwapDate			First swap maturity date used for tension cutoff
		@param[in] implyForwards				Imply forward rates from instruments?
	*/
	void bootstrapFRAs( DiscountFactors & dfResults,
						DoubleArray& fwds,
						DoubleMatrix& fwd_termsmtx,
						const std::vector<LAObject*>& data_fra,
						const std::vector<LAObject*>&  data_libor,
						LAInterpolationBase* pInter_fw,
						const StateVariableEnum& stateVariable,
						bool is_fwdswap,
						const LAString& frequency,
						const LAString& liborIndexTerm,
						const LADate & asOfDate,
						const LADate& spotdate,
						const LADate& spotDateLibor,
						const LADate& firstSwapDate,
						bool implyForwards,
					    const double linearSplineJoinDate = 0.0,
					    unsigned int lastLiborPosition = 0,
					    const LADate& lastLiborEndDate = LADate() );
	
	/*
		@brief Bootstrap yield curve using Libor instruments producing discount factors and forward rates

		@param[out] fwd					Forward rates being generated
		@param[out] fwd_termsmtx		Forward rate dates
		@param[in] data_libor			Libor instruments
		@param[in] data_moneymarket		Market market instruments
		@param[in] is_fra_use			Are FRAs used?
		@param[in] is_f_use				Are Futures used?
		@param[in] baseFreq				Base frequency
		@param[in] spotDateSwap			Spot rate of the swap instruments
		@param[in] spotDateLibor		Spot rate of the Libor instruments
		@param[in] liborIndexTerm		Libor Index Term
		@param[in] implyForwards		Imply forward rates from instruments?

		@return the index in the Libor vector that points to the current Libor rate
	*/
	DiscountFactors bootstrapLibors( DoubleArray& fwds,
									 DoubleMatrix& fwd_termsmtx,
									 const std::vector<LAObject*> & data_libor,
									 const MoneyMarketData & data_moneymarket,
									 bool is_fra_use,
									 bool is_f_use,
									 const LADate& spotDateSwap,
									 const LADate& spotDateLibor,
									 const LAString& liborIndexTerm,
									 bool implyForwards = true );
	
	/*
		@brief Calculate start and end dates of a FRA instrument

		@param[out] startDate		FRA start date
		@param[out] endDate			FRA end date
		@param[in]  spotDate		Spot date of curve
		@param[in]  data_fra		FRA instrument
		@param[in]	liborIndexTerm		The tenor of the curve
		@param[in]	baseFreq		The frequency of the curve
		@param[in]	dc				FRA day count
	*/
	void calculateFraDates(LADate& startDate,
						   LADate& endDate,
						   const LADate& spotDate,
						   const LAObject* data_fra,
						   const LAString& liborIndexTerm,
						   const LAString& baseFreq,
						   const LAPriceDataDayCount& dc);

	/*
		@brief Populate start and end dates of a FRA instrument

		@param[out] startDate		FRA start date
		@param[out] endDate			FRA end date
		@param[in]  spotDate		Spot date of curve
		@param[in]  data_fra		FRA instrument
		@param[in]	liborIndexTerm	The tenor of the curve
		@param[in]	baseFreq		The frequency of the curve
		@param[in]	dc				FRA day count
	*/
	void populateFraDates(LADate& startDate,
						  LADate& endDate,
						  const LADate& spotDate,
						  const LAObject* data_fra,
						  const LAString& liborIndexTerm,
						  const LAString& roll_conv,
						  const LAPriceDataSlidingRule& sld,
						  const LAPriceDataCalendar& cal,
						  const LAPriceDataDayCount& dc);

	/*
		@brief Get YieldGen interpolation method for a curve from object pool

		@param[inout]	objHolder			Object holder object representing CurveCalibrationData
		@param[in]		suffix		Name of the curve concerned
		@return			Curve's interpolation
	*/
	LAPriceDataInterpolation getYieldGenInterpolationByCurveName(LAObjectHolder& objHolder, const LAString& suffix);

	/*
		@brief calc float side pv on spot date

		@param[in] fwdRate_Interpolation_ZeroTime : Forward Rate Index Interpolation using ZeroRateTimesTime state variable
		@param[in] stateVariable :					stateVariable
		@param[in] discFactor_Interpolation :		Discount Factor Interpolation using Discount Factor state variable
		@param[in] spotDiscountFactor :						spotdate DF
		@param[in] spotDateAsTerms :						spotdate term
		@param[in] terms_grid :						grid
		@param[in] asOfDate							curve AsOfDate
		@param[in] accrualDaycount					The accrual daycount basis to use e.g. ACT/360
		@param[in] compoundFreq						The compounding frequency, defaults to SIMPLE
		@param[in] cpd_times :						compounding times (optionally)
		@param[in] term_start :						start term (might be fwd starting, optionally)

		@return floatside pv
	*/
	double calcFloatPV( LAInterpolationBase &fwdRate_Interpolation,
						const StateVariableEnum& stateVariable,
						LAInterpolationBase &discFactor_Interpolation,
						const double spotDiscountFactor,
						const double spotDateAsTerms,
						const DoubleArray &term_grid,
						const LADate & asOfDate,
						const DayCountEnum & accrualDaycount,
						const CompoundingFrequencyEnum & compoundFreq = SIMPLE_COMPOUNDING,
						const int cpd_times = 1,
						const double term_start = 0.0,
						const DoubleVector& floatAccrualPeriods = DoubleVector(),
						const DoubleVector& fixingStarts = DoubleVector(),
						const DoubleVector& fixingEnds = DoubleVector(),
						const DoubleVector &fixingTaus = DoubleVector() );

	/*
		@brief *** NEWTON-RAPHSON SOLVER ***
		Solve for a smooth short end of the STD curve mitigating/eliminating Libor/Future(FRA) clash
		We solve for the level of startDF and endDF that allows the libor instruments to be correctly repriced

		@param[inout] startDF					start discount factor of the first future/FRA
		@param[inout] endDF						end discount factor of the first future/FRA
		@param[in] startDate					Start date of the first future/FRA instrument
		@param[in] endDate						End date of the first future/FRA instrument
		@param[in] spotDateLibor					Libor spot date
		@param[in] spotDateSwap					Swap spot date
		@param[in] liborDate					End date of libor fixing
		@param[in] data_libor					The libor instrument object
		@param[in] cashDepositInterpolation		Interpolator used to smooth the front end
		@param[in] stateVariable			    stateVariable
		@param[in] dfResults					DF container of the STD curve
		@param[in] forwardRate					Forward rate of the first future/FRA instrument
		@param[in] forwardConv					Convention object of the first future/FRA instrument
	*/
	void solveSmoothSTDShortEnd(double& startDF,
								double& endDF,
								const LADate& startDate,
								const LADate& endDate,
								const LADate& spotDateLibor,
								const LADate& spotDateSwap,
								const LADate& liborDate,
								const LAObject* data_libor,
								const std::unique_ptr<LAInterpolationBase>& cashDepositInterpolation,
								const StateVariableEnum& stateVariable,
								const DiscountFactors & dfResults,
								const double forwardRate,
								const LAPriceDataConvention& forwardConv);

	class SwapPaymentSchedule
	{
		public:

		// Default constructor/destructor
		SwapPaymentSchedule() {};
		~SwapPaymentSchedule() {};
		
		// Main Constructor
		SwapPaymentSchedule( const LADate & startDate,
							 const LADate & endDate,
							 const LAString & frequency,
							 const LAPriceDataCalendar & calendar,
							 const LAPriceDataSlidingRule & businessDayAdjustment,
							 const LAPriceDataDayCount & daycount,
							 const LADate & forwardStartingSpotDate );

		// Accessors
		DateVector	paymentDates()			const	{ return paymentDates_; };
		DoubleArray paymentDatesAsTerms()	const	{ return paymentDatesAsTerms_; };
		DoubleArray paymentDateIntervals()	const	{ return paymentDateIntervals_; };
		bool		rollEndOfMonth()		const	{ return rollEndOfMonth_; };

		protected:

		// Inputs
		LADate startDate_;
		LADate endDate_;
		LAString frequency_;
		const LAPriceDataCalendar * calendar_;
		const LAPriceDataSlidingRule * businessDayAdjustment_;
		const LAPriceDataDayCount * daycount_;
		const LADate * forwardStartingSpotDate_;

		// Outputs
		DateVector paymentDates_;
		DoubleArray paymentDatesAsTerms_;
		DoubleArray paymentDateIntervals_;
		bool rollEndOfMonth_;
		
	};

	/*!
		@brief calculate swap payment dates, terms and intervals
		@param[in] startDate
		@param[in] endDate
		@param[in] frequency
		@param[in] calendar
		@param[in] busDayAdj Business Day Adjustment
		@param[in] daycount
		@param[out] cashflowPaymentDates
		@param[out] cashflowPaymentDatesAsTerms  (daycount = ACT/365)
		@param[out] accuralPeriods (accrual period between payment Dates, which is incorrect - assumes no fixing or pay lag)
		@param[out] isEOMRoll roll end-of-month
		@param[in] spotDateSwap ( swap spot date, optionally)
	*/
	void updateAccrualPeriodsAndPaymentDates( const LADate& startDate,
							                  const LADate& endDate,
							                  const LAString& frequency,
							                  const LAPriceDataCalendar& calendar,
							                  const LAPriceDataSlidingRule& busDayAdj,
							                  const LAPriceDataDayCount& daycount,
							                  DateVector& cashflowPaymentDates,
							                  DoubleArray& cashflowPaymentDatesAsTerms,
							                  DoubleArray& accuralPeriods,
							                  bool isEOMRoll,
							                  const LADate* spotDateSwap = 0 );

	/*!
		@brief Determine the join date used by linear spline interpolation
		@param[in] lastFuture	Last future calibration instrument
		@param[in] firstSwap	First swap calibration instrument
		@param[in] spotDate		Spot date
		@param[in] is_fwdswap	Is forward starting swap used?
		@return	   Linear spline join date
	*/
	LADate determineLinearSplineInterpolationJoinDate(const LAObject* lastFuture,
													  const LAObject* firstSwap,
													  const LADate& spotDate,
													  bool is_fwdswap);

	/*!
		@brief Obtains the ith term point ( eg '15Y' ) from the input market data, and checks for duplicate terms
		@param[in] i			The index into the market data
		@param[in] data_swap	The market data object
		@returns				The term string
	*/
	const LAString& getMaturityAsTermString(unsigned int i, const std::vector<LAObject*> & data_item);

	/* @brief	Populate cash instrument data into object pool.
	*			Cash market instruments include ON, TN and Libor market data.
	*
	*  @param [inout]	mpStaticData		Property manager object
	*  @param [inout]	curveCalibrationData	CurveCalibrationData object
	*  @param [inout]	refData				Reference data
	*  @param [inout]	objPool				Reference object to the object pool
	*  @param [in]		currency			Curve currency
	*  @param [in]		marketName			Market name of curve
	*  @param [in]		yieldDataName		Yield data name
	*  @param [in]		staticDataSuffix	Suffix used with property manager
	*  @param [in]		suffix_data			Suffix used with object pool
	*  @param [in]		isSpotUse			Use use given spot date?
	*  @param [in]		isFwdFX				Is using Fx forwards (for the short end)?
	*  @param [in]		asOfDate			As of date of curve
	*  @param [in]		fixingSource		Source of libor fixing
	*  @return			Libor rate that matches spot rate term (if provided)
	*/
	double populateCashInstrumentsToEntityPool(LAStaticData * mpStaticData,
											   LAString& refData,
											   LAObjectPool& objPool,
											   const LAString& currency,
											   const LAString& marketName,
											   const LAString& yieldDataName,
											   const LAString& staticDataSuffix,
											   const LAString& suffix_data,
											   bool isSpotUse,
											   bool isFwdFX,
											   const LADate &asOfDate,
											   const LAString& fixingSource = "",
											   const LAString& spotRateTerm = "");

	/*! @brief get Grid property val

		@param[in] key
		@param[in] curve
		@param[in] grid

		@return Property data value
	*/
	LAString getGridStaticData(LAStaticData * mpStaticData, const LAString &key, const LAString &curve, const LAString &grid);

	/*! @brief Price the PVs of a group of OIS swaps (either outright or Libor-OIS basis swap)

		@param[out] allPVs				Calculated PVs of all swaps
		@param[in]  pInter_yield		Interpolator carrying OIS state variable
		@param[in]  df_inter			Interpolator carrying external discount rates
		@param[in]  size_calcs			Number of swap cash flows that need to be calculated for all calibration swaps
		@param[in]  size_calcs_s		Number of Libor swap cash flows that need to be calculated for all calibration swaps
		@param[in]  calced_sizes		Number of OIS swap cash flows that can be re-used
		@param[in]  calced_sizes_s		Number of Libor swap cash flows that can be re-used
		@param[in]  terms_grids			Terms from spot date to all OIS swap dates
		@param[in]  terms_grids_s		Terms from spot date to all Libor swap dates
		@param[in]  terms_intervals		Accrual terms between all OIS swap accrual dates
		@param[in]  terms_intervals_s	Accrual terms between all Libor swap accrual dates
		@param[in]  marketRate			Market OIS par rate
		@param[in]  marketRate_s		Market Libor swap par rate
		@param[in]  fixingStartDates	OIS swap fixing start dates
		@param[in]  fixingEndDates		OIS swap fixing end dates
		@param[in]  fixingTaus			Fixing terms between all OIS swap fixing dates
		@param[in]  term_strs			Maturity term of all calibration swaps
		@param[in]  is_selfdf			Boolean indicating self-discounting or not
		@param[in]  spotterm			Year fraction from as-of date to spot date
		@param[in]  d_spotdf			Discount factor from spot date to as-of date
		@param[in]  spotdate			Spot date
		@param[in]  swapCompoundingMethods	Compounding method for OIS swaps
		@param[in]  longTermConv		Indication of calibration swap type for the long end of OIS curve
		@param[in]  longTermGen			Generation method for long end calibration swaps
		@param[in]  bOISSwapCalcReset	Calculation reset condition for OIS swaps
		@param[in]  bFullSigmaReset		Cash flow reset condition for OIS swaps
		@param[in]  bLiborSwapCalcReset	Calculation reset condition for Libor legs
		@param[inout]  startterms		Terms from spot to start date of each OIS cash flow
		@param[inout]  endterms			Terms from spot to end date of each OIS cash flow
	*/
	void priceOISSwaps( DoubleVector& allPVs,
						LAInterpolationBase* pInter_yield,
						LAInterpolationBase *df_inter,
						const std::vector<size_t>& size_calcs,
						const std::vector<size_t>& size_calcs_s,
						const std::vector<size_t>& calced_sizes,
						const std::vector<size_t>& calced_sizes_s,
						const DoubleMatrix& terms_grids,
						const DoubleMatrix& terms_grids_s,
						const DoubleMatrix& terms_intervals,
						const DoubleMatrix& terms_intervals_s,
						const DoubleVector& marketRates,
						const DoubleVector& marketRates_s,
						const std::vector<DateVector>& fixingStartDates,
						const std::vector<DateVector>& fixingEndDates,
						const DoubleMatrix& fixingTaus,
						const LAStringVector& term_strs,
						bool is_selfdf,
						double d_spotdf,
						double spotterm,
						const LADate& spotdate,
						const std::vector<etrading::OISCompoundingEnum> & swapCompoundingMethods,
						const std::vector<etrading::OISLongTermInstrumentsEnum> & longTermConvs,
						const LAStringVector& longTermGens,
						const std::vector<const LAPriceDataCalendar*>& cals,
						const std::vector<const LAPriceDataDayCount*>& dateCounts,
						const std::vector<bool>& bOISSwapCalcReset,
						const std::vector<bool>& bFullSigmaReset,
						const std::vector<bool>& bLiborSwapCalcReset,
						std::vector<DoubleMatrix>& startterms,
						std::vector<DoubleMatrix>& endterms );

	/*! @brief Price the PV of a single OIS swap (either outright or Libor-OIS basis swap)

	@param[in]      oisParRate			                    Market OIS par rate
	@param[in]      liborOisParSpread		                Market Libor-OIS par spread
	@param[in]      liborParRate		                    Market Libor swap par rate
	@param[in]      pInter_yield		                    Interpolator carrying OIS state variable
	@param[in]      df_inter			                    Interpolator carrying external discount rates
	@param[inout]   oisFloatLegPV_WithoutSpread	            PV of the CURRENT OIS Float leg *** with no OIS Spread ***
	@param[inout]   oisFloatLegAnnuity				        Annuity of the current OIS Float Leg
	@param[inout]   oisFixedLegAnnuity				        Annuity of the current OIS Fixed Leg
	@param[inout]   liborFloatLegAnnuity			        Annuity of the Libor Float leg of the current LIBOR-OIS swap
	@param[in]      previousOisFloatLegPV_WithoutSpread	    PV of the PREVIOUS OIS Float leg - For optimization purposes *** with no OIS Spread ***
	@param[in]      previousOisFloatLegAnnuity				Annuity of the previous OIS Float Leg
	@param[in]      previousOisFixedLegAnnuity				Annuity of the previous OIS Fixed Leg
	@param[in]      previousLiborFloatLegAnnuity            Annuity of the Libor Float leg of the previous LIBOR-OIS swap
	@param[in]      size_calc			                    Number of OIS swap cash flows that need to be calculated for the current swap
	@param[in]      calced_size			                    Number of OIS swap cash flows that can be re-used
	@param[in]      size_calc_s			                    Number of Libor swap cash flows that need to be calculated for the current swap
	@param[in]      calced_size_s		                    Number of Libor swap cash flows that can be re-used
	@param[in]      terms_grids			                    Terms from spot date to all OIS swap dates
	@param[in]      terms_intervals		                    Accrual terms between all OIS swap accrual dates
	@param[in]      terms_grids_s		                    Terms from spot date to all Libor swap dates
	@param[in]      terms_intervals_s	                    Accrual terms between all Libor swap accrual dates
	@param[in]      fixingTaus			                    Fixing terms between all OIS swap fixing dates
	@param[in]      fixingStartDates	                    OIS swap fixing start dates
	@param[in]      fixingEndDates		                    OIS swap fixing end dates
	@param[in]      is_selfdf			                    Boolean indicating self-discounting or not
	@param[in]      spotterm			                    Year fraction from as-of date to spot date
	@param[in]      d_spotdf			                    Discount factor from spot date to as-of date
	@param[in]      spotdate			                    Spot date
	@param[in]      swapAveragingMethodEnum	                Compounding method for OIS swap
	@param[in]      longTermConvEnum	                    Indication of calibration swap type for the long end of OIS curve
	@param[in]      longTermGen			                    Generation method for long end calibration swaps
	@param[in]      dateCount			                    OIS swap date count
	@param[in]      cal					                    OIS swap calendar
	@param[inout]   startterms		                        Terms from spot to start date of each OIS cash flow
	@param[inout]   endterms			                    Terms from spot to end date of each OIS cash flow

	@return PV of the OIS swap (either an outright or a Libor-OIS basis swap)
	*/
	double priceSingleOISSwapPV(double oisParRate,
								double liborOisParSpread,
								double liborParRate,
								LAInterpolationBase* pInter_yield,
								LAInterpolationBase* df_inter,
								double& oisFloatLegPV_WithoutSpread,
								double& oisFloatLegAnnuity,
								double& oisFixedLegAnnuity,
								double& liborFloatLegAnnuity,
								double previousOisFloatLegPV_WithoutSpread,
								double previousOisFloatLegAnnuity,
								double previousOisFixedLegAnnuity,
								double previousLiborFloatLegAnnuity,
								size_t size_calc,
								size_t calced_size,
								size_t size_calc_s,
								size_t calced_size_s,
								const DoubleVector& terms_grids,
								const DoubleVector& terms_intervals,
								const DoubleVector& terms_grids_s,
								const DoubleVector& terms_intervals_s,
								const DoubleVector& fixingTaus,
								const DateVector& fixingStartDates,
								const DateVector& fixingEndDates,
								bool is_selfdf,
								double spotterm,
								double d_spotdf,
								const LADate& spotdate,
								const etrading::OISCompoundingEnum& swapAveragingMethodEnum,
								const etrading::OISLongTermInstrumentsEnum& longTermConvEnum,
								const LAString& longTermGen,
								const LAPriceDataDayCount* dayCount,
								const LAPriceDataCalendar* cal,
								DoubleMatrix& startterms,
								DoubleMatrix& endterms);

	/*! @brief Optimise Interpolation Join Date

		@param[in] interp						Interpolation objects carrying logDF (the state variable of swap curve), usually inter_yg
		@param[in] futuresStartDateTerms		Date terms of all futures start dates
		@param[in] futuresEndDateTerms			Date terms of all futures end dates
		@param[in] futuresRates					Equivalent forward rates of all futures contracts
		@param[in] interpDatesAsTerms			Date terms that correspond to all logDFs in 'interp'
		@param[in] interpValues					All logDFs in 'interp'
		@param[in] useNewtonRaphsonMinimizer	Default (TRUE): Use the NewtonRaphson Minimizer (TRUE), much faster or search for solution (FALSE)
		@param[in] searchIntervalDays			Default (1 Day): Controls the search inverval days
		@return a pair of boolean indicating success and the refined join date
	*/
	InterpolationJoinDate optimizeInterpolationJoinDate( std::unique_ptr<LAInterpolationBase>& interp,
														 const DoubleArray& futuresStartDateTerms,
														 const DoubleArray& futuresEndDateTerms,
														 const DoubleVector& futuresRates,
														 const DoubleArray& interpDatesAsTerms,
														 const DoubleArray& interpValues,
														 const bool useNewtonRaphsonMinimizer = true,
														 const size_t searchIntervalDays = 1 );

	/*!
		@brief get date for moneymarket except O_N and T_N
		@param[in] basedate basedate
		@param[in] termstr string of such as "2D_1D" or "2D_1W"
		@param[in] cal calendar
		@param[in] srule holiday sliding rule
		@param[in,out] start start date
		@param[in,out] end end date
	*/
	void getMoneyMarketDates(const LADate& basedate, const LAString& termstr, const LAPriceDataCalendar& cal, const LAPriceDataSlidingRule& srule, LADate& start, LADate& end);

	// Initialise yield curve state variables prior to solving
	void initialiseStateVariablesForSolving(DoubleArray& stateVariable_rates,
										    DoubleArray&stateVariable_grid,
										    const LAObjectHolder& objHolder,
										    const StateVariableEnum& stateVariable,
										    const DoubleArray& rates,
										    const DoubleArray& grids,
										    const bool& fastRebuildRequested,
										    const LAString& targetSuffix,
										    size_t dataSize);


	// Insert daily zero rates for the  Central Bank Swap/ARR future, when there are swaps before ECB/Futures
	void insertDailyZeroRatesForCentralBankFromShortTermSwaps(DoubleVector& grid,
															  DoubleVector& yields,
															  DateVector& dates,
															  const LADate& fromDate,
															  const LADate& spotdate,
															  const LADate& shortterm_date,
															  const double lastSwapRate,
															  const LADate& firstCBSStartDate,
															  const double firstCBSRate,
															  const std::map<LADate, std::pair<LADate, double>>& mpcSwapRates,
															  const double initialDF,
															  const LAPriceDataDayCount& dc_act365,
															  const LAPriceDataCalendar& cal,
															  const LAPriceDataConvention& conv);

	// Insert daily zero rates for the  Central Bank Swap/ARR future, when there is no short end swap
	void insertDailyZeroRatesForCentralBank(DoubleVector& grid,
											DoubleVector& yields,
											DateVector& dates,
											const LADate& fromDate,
											const LADate& spotdate,
											const LADate& shortterm_date,
											const std::map<LADate, std::pair<LADate, double>>& mpcSwapRates,
											const double initialDF,
											const LAPriceDataDayCount& dc_act365,
											const LAPriceDataCalendar& cal,
											const LAPriceDataConvention& conv,
											const double lastShortSwapTerm = std::numeric_limits<double>::quiet_NaN());

	// Insert zero rates for the Central Bank Swap/ARR future, by interpolation on rateTime or fwdRates.
	void insertZeroRatesForFutures( DoubleVector& grid,
									DoubleVector& yields,
									DateVector& dates,
									const LADate& spotdate,
									const std::unique_ptr<LAInterpolationBase>& pInter_yield,
									const std::map<LADate, std::pair<LADate, double>>& mpcSwapRates, // key as startDate, values as endDate and fwdRate
									const LAPriceDataDayCount& dc_act365,
									const LAPriceDataConvention& conv,
									const LAPriceDataCalendar& cal,
									const bool interpOnFwdRate );

	// Helper function to insert daily zeroRates based on a constant fwd rate 
	void insertDailyZeroRatesByConstantFwdRate(DoubleVector& grid,
											   DoubleVector& yields,
											   DateVector& dates,
											   const LADate& spotdate,
											   const LADate& startDate,
											   const LADate& endDate,
											   const double fwdRateToUse,
											   const double initialDF,
											   const LAPriceDataDayCount& dc_act365,
											   const LAPriceDataCalendar& cal,
											   const LAPriceDataConvention& conv);
		
	// Interpolate on rateTime for the Central Bank Swap, so that the curve section can have step fwd rates
	void insertZeroRatesWithStepFwdRates(DoubleVector& grid,
										 DoubleVector& yields,
										 DateVector& dates,
										 const LADate& spotdate,
										 const std::unique_ptr<LAInterpolationBase>& pInter_yield,
										 const std::map<LADate, std::pair<LADate, double>>& futureRates, // key as startDate, values as endDate and fwdRate
										 const LAPriceDataDayCount& dc_act365,
										 const LAPriceDataConvention& conv,
										 const LAPriceDataCalendar& cal);

	// Interpolate on fwd rate directly, for ARR future, so that the curve section can have linear fwd rates
	void insertZeroRatesWithLinearFwdRates(DoubleVector& grid,
										   DoubleVector& yields,
										   DateVector& dates,
										   const LADate& spotdate,
										   const std::unique_ptr<LAInterpolationBase>& pInter_yield,
										   const std::map<LADate, std::pair<LADate, double>>& futureRates, // key as startDate, values as endDate and fwdRate
										   const LAPriceDataDayCount& dc_act365,
										   const LAPriceDataConvention& conv,
										   const LAPriceDataCalendar& cal);

	// Helper function to get the DF from zero rate
	double getOISdiscountFactor(const double term, const std::unordered_map<double, double>& termYieldMap, const std::unique_ptr<LAInterpolationBase>& pInter_yield);

	// Helper function to populate the input fixings to the market date object
	void populateHistoricalDataToMarketData(LAObject *mktData, const LAString& oisHistFileName);

	// Helper function to get the convexity adjusted rate for future instruments
	double getConvexityAdjustedFutureRate(LAObject *mktData, const double futureRate, const LADate& asOfDate, const double meanReversion, const LAPriceDataDayCount& dc);

	/*
	@brief Helper function to check if it's a Central bank swap based on the term string
	*/
	bool isCentralBankSwap(const LAString& term);

	/*
	@brief Helper function to check if it's a Future based on the term string
	*/
	bool isFuture(const LAString& term);

	/*
	@brief Helper function to check if it's a Central bank swap based on the term string
	*/
	OISMidTermInstrumentsEnum getOISMidTermInstrumentsEnum(const LAString& term);

	/*
	@brief Helper function to get the default OIS compounding method, based on the instrument type
	*/
	OISCompoundingEnum getDefaultOISCompounding(const OISMidTermInstrumentsEnum& instrumentType, const LAString& userInputShortTermConvStr);

	//curveMarketName is the unique staticDataTable, which is different from a curveIndex
	CurveTypeEnum getCurveTypeEnum(const LAObject& yieldData, const LAString& curveMarketName);

	/*
	@brief Calculate forward rates given a list of calibrated discount factors of the curve

	@param[in] inter
	@param[in] stateVariable
	@param[in] grid_swap
	@param[in] tau_swap
	@param[in] generateForwardsFromSwapsOnly
	@param[in] asOfDate
	@param[in] accrualDaycount
	@param[out] fwd_termsmtx
	@param[out] fwds
	*/
	void updateImpliedForwardRates(const LAInterpolationBase &inter, const StateVariableEnum& stateVariable, const DoubleArray &fixingStarts, const DoubleArray &fixingEnds, const DoubleArray &tau_swap, bool generateForwardsFromSwapsOnly, const LADate & asOfDate, const DayCountEnum & accrualDaycount, DoubleMatrix &fwd_termsmtx, DoubleArray &fwds);

	/*
	@brief calc forwardrates for swap grid

	@param[in] inter			ARR Curve's interpolator
	@param[in] baseDate			AsOfDate of the curve
	@param[in] terms			year fractions from asOfDate
	@param[in] dfs				discount factors
	@param[in] cal				calendar
	@param[in] dc_act365		Day count for discount factor
	@param[in] dc				Day count for trade
	@param[out] fwd_termsmtx    From/to terms of forward rate table
	@param[out] fwds			Forward rates of forward rate table
	*/
	void populateARRCurveForwardRateTable(const LAInterpolationBase &inter,
										  const LADate& baseDate,
										  const DoubleVector& terms,
										  const DoubleVector& dfs,
										  const LAPriceDataCalendar& cal,
										  const LAPriceDataDayCount& dc_act365,
										  const LAPriceDataDayCount& dc,
										  DoubleMatrix& fwd_termsmtx,
										  DoubleArray& fwds);

	etrading::StateVariableEnum getInterpolationStateVariable(const CurveTypeEnum& curveTypeEnum);

	// Calculate the Swap Curve Spot Discount Factor and update money market discount factors and dates
	double getSpotDFandUpdateMoneyMarket(DoubleMatrix& df_moneymarket, DateVector& df_moneymarket_date, const std::map<std::pair<LADate, LADate>, const LAObject*>&  data_moneymarket, const LAPriceDataDayCount& dc_act365, const LADate& spotDateSwap);

	/*
	@brief Function to evaluate the curve state variable requested, which can be a Zero Rate, Zero Rate times Time, Log DF or DF

	@param[in] zeroRate		        zero rate
	@param[in] accrualPeriod		accrualPeriod
	@param[in] stateVariableType    state variable: STATE_VARIABLE_ZERO_RATE, STATE_VARIABLE_ZERO_RATE_TIMES_TIME, STATE_VARIABLE_LOG_DF, STATE_VARIABLE_DF
	@return    returns the state variable requested, which can be a Zero Rate, Zero Rate times Time, Log DF or DF
	*/
	double getStateVariableValue(const double& zeroRate, const double& accrualPeriod, const StateVariableEnum& stateVariableType);

	/*
	@brief Function to evaluate the curve state variable requested, which can be STATE_VARIABLE_ZERO_RATE_TIMES_TIME, STATE_VARIABLE_DF

	@param[in] discountFactor		discountFactor
	@param[in] stateVariableType    stateVariableType
	@return    returns the state variable requested
	*/
	double getStateVariableValueFromDF(const double& discountFactor, const StateVariableEnum& stateVariableType);

	/*
	@brief Function to evaluate the curve state variable requested, which can be STATE_VARIABLE_ZERO_RATE_TIMES_TIME, STATE_VARIABLE_DF

	@param[in] rateTime		rateTime
	@param[in] stateVariableType    stateVariableType
	@return    returns the state variable requested
	*/
	double getStateVariableValueFromRateTime(const double& rateTime, const StateVariableEnum& stateVariableType);

	/*
	@brief Function to evaluate the curve state variable requested, which can be a Zero Rate, Zero Rate times Time, Log DF or DF

	@param[in] accrualTerm			The accrual term, not the date as a term
	@param[in] stateVariableValue   stateVariableValue
	@param[in] stateVariableType    state variable: STATE_VARIABLE_ZERO_RATE,
													STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
													STATE_VARIABLE_LOG_DF,
													STATE_VARIABLE_DF

	@return    returns the state variable requested, which can be a Zero Rate, Zero Rate times Time, Log DF or DF
	*/
	double getZeroRateFromStateVariable( const double& accrualPeriod,
										 const double& stateVariableValue,
										 const StateVariableEnum& stateVariableType );

	/*
	@brief Function to interpolate for single discount factor applying the appropriate state variable conversion

	@param[in] paymentDateAsTerm	Payment Date as Term
	@param[in] interpolator		    interpolator class, this should already be set
	@param[in] stateVariableType	state variable: STATE_VARIABLE_ZERO_RATE,
													STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
													STATE_VARIABLE_LOG_DF,
													STATE_VARIABLE_DF
	@param[in]  asOfDate			curve AsOfDate
	@param[in]  accrualDaycount		The accrual daycount basis to use e.g. ACT/360
	@param[in]  compoundFreq		The compounding frequency, defaults to SIMPLE
	@return    returns a single adiscount factor applying the appropriate state variable conversion
	*/
	double getInterpolatedDiscountfactor(const LAInterpolationBase& interpolator,
										  const double& paymentDateAsTerm,
										  const StateVariableEnum& stateVariableType,
									      const LADate & asOfDate,
									      const DayCountEnum & accrualDaycount,
									      const CompoundingFrequencyEnum & compoundFreq = SIMPLE_COMPOUNDING );

	/*
	@brief Function to interpolate for a single forward rate applying the appropriate state variable conversion

	@param[in] fixingStartTerm		Fixing Start Date as Term
	@param[in] fixingEndTerm		Fixing End Date as Term
	@param[in] interpolator		    interpolator class, this should already be set
	@param[in] stateVariableType	state variable: STATE_VARIABLE_ZERO_RATE,
													STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
													STATE_VARIABLE_LOG_DF,
													STATE_VARIABLE_DF
	@param[in]  asOfDate			curve AsOfDate
	@param[in]  accrualDaycount		The accrual daycount basis to use e.g. ACT/360
	@param[in]  compoundFreq		The compounding frequency, defaults to SIMPLE
	@return    returns a single forward rate applying the appropriate state variable conversion
	*/
	double getInterpolatedForwardRate( const double& fixingStartTerm,
									   const double& fixingEndTerm,
									   const LAInterpolationBase& interpolator,
									   const StateVariableEnum& stateVariableType,
									   const LADate & asOfDate,
									   const DayCountEnum & accrualDaycount,
									   const CompoundingFrequencyEnum & compoundFreq = SIMPLE_COMPOUNDING );

	// Method to get a discount factor given the state variable
	double getdiscountFactor( const double& paymentDateAsTerm,
							  const LAInterpolationBase & interpolator,
							  const StateVariableEnum& stateVariableType,
							  const LADate & asOfDate,
							  const DayCountEnum & accrualDaycount,
							  const CompoundingFrequencyEnum & compoundFreq = SIMPLE_COMPOUNDING );

	// Method to get a forward rate given the state variable
	double getForwardRate( const double& fixingStartDateAsTerm,
						   const double& fixingEndDateAsTerm,
						   const LAInterpolationBase & interpolator,
						   const StateVariableEnum& stateVariableType,
						   const LADate & asOfDate,
						   const DayCountEnum & accrualDaycount,
						   const CompoundingFrequencyEnum & compoundFreq = SIMPLE_COMPOUNDING );

	/*
	@brief Function to interpolate for single zero rate applying the appropriate state variable conversion

	@param[in] dateInTermFormat		dateInTermFormat
	@param[in] interpolator		    interpolator class, this should already be set
	@param[in] stateVariableType	state variable: STATE_VARIABLE_ZERO_RATE,
													STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
													STATE_VARIABLE_LOG_DF,
													STATE_VARIABLE_DF
	@param[in] asOfDate					curve AsOfDate
	@param[in] accrualDaycount			The accrual daycount basis to use e.g. ACT/360
	@param[in] compoundFreq				The compounding frequency, defaults to SIMPLE

	@return    returns a single zero rate applying the appropriate state variable conversion
	*/
	double getInterpolatedZeroRate( const double& dateInTermFormat,
								    const LAInterpolationBase* interpolator,
									const StateVariableEnum& stateVariableType,
									const LADate & asOfDate,
									const DayCountEnum & accrualDaycount,
									const CompoundingFrequencyEnum & compoundFreq = SIMPLE_COMPOUNDING );

	bool isContiguousFraFuture(const LADate& spotDate, const LADate& startDate, const LAString& liborIndexTerm, const bool isFuture);

	/*
		@brief Insert DFs for Serial Fra/Futures by interpolating existing DFs and fwdRates

		@param[out] dfResults				dfResults
		@param[out] fwd_termsmtx			Matrix of fwd rate table (start terms and end terms)
		@param[out] fwds					A list of fwd rates
		@param[in]	serialFraFutures		A list of serial fra/futures
		@param[in]  asOfDate				curve AsOfDate
		@param[in]	spotdate				Spot date of the curve
		@param[in]	pInter_fw				Future/Fra interpolator
		@param[in]	liborIndexTerm			Reference rate term
		@param[in]	roll_conv				Roll convention
		@param[in]	sld						Sliding rule
		@param[in]	cal						Calendar
		@param[in]	dc						Trade day count
		@param[in]	dc_act365				dc_act365 day count
		@param[in]	stateVariable			State variable for future/fra
		@param[in]	populateFwdTable		True to populate fwd table
		@param[in]	dayIntervalTolerance	Tolerance between current future's endDate and next future's startDate, Default value is 7 days
	*/
	void insertDFs_SerialFuturesByDF( DiscountFactors & dfResults,
									  DoubleMatrix& fwd_termsmtx,
									  DoubleArray& fwds,
									  const std::vector<ForwardRate>& serialFraFutures,
									  const LADate & asOfDate,
									  const LADate& spotdate,
									  const LAInterpolationBase& pInter_fw,
									  const LAString& liborIndexTerm,
									  const LAString& roll_conv,
									  const LAPriceDataSlidingRule& sld,
									  const LAPriceDataCalendar& cal,
									  const LAPriceDataDayCount& dc,
									  const LAPriceDataDayCount& dc_act365,
									  const StateVariableEnum& stateVariable,
									  const bool populateFwdTable,
									  const int dayIntervalTolerance = 7);

	/*
		@brief Insert DFs for Futures with Serials by adjusting the quoted fwd rates

		@param[out] dfResults					dfResults
		@param[out] fwd_termsmtx				Matrix of fwd rate table (start terms and end terms)
		@param[out] fwds						A list of fwd rates
		@param[in]	futureStartDates			Future start dates
		@param[in]	futureEndDates				Future end dates
		@param[in]	futureRates					Future rates
		@param[in]  asOfDate					curve AsOfDate
		@param[in]	spotDateSwap				Swap spot date
		@param[in]	spotDateLibor				Libor spot date
		@param[in]	pInter_fw					Future/Fra interpolator
		@param[in]	dc							Trade day count
		@param[in]	dc_act365					dc_act365 day count
		@param[in]	stateVariableFutureFra		State variable for future/fra
		@param[in]	includeSwapsBeforeMPCSwaps	True to call the includeSwapsBeforeMPCSwaps method
		@param[in]	data_libor					Libor data
		@param[in]	liborDate					Libor date
		@param[in]	populateFwdTable			True to populate fwd table
		@param[in]	dayIntervalTolerance		Tolerance between current future's endDate and next future's startDate
	*/
	void insertDFs_SerialFuturesByRate( DiscountFactors & dfResults,
										DoubleMatrix& fwd_termsmtx,
										DoubleArray& fwds,
										const DateVector& futureStartDates,
										const DateVector& futureEndDates,
										const DoubleVector& futureRates,
										const LADate & asOfDate,
										const LADate& spotDateLibor,
										const LADate& spotDateSwap,
										const LAInterpolationBase& pInter_fw,
										const LAPriceDataDayCount& dc,
										const LAPriceDataDayCount& dc_act365,
										const StateVariableEnum& stateVariableFutureFra,
										const bool includeSwapsBeforeMPCSwaps,
										const LAObject* data_libor,
										const LADate& liborDate,
										const bool populateFwdTable = true,
										const int dayIntervalTolerance = 7 );


	/*
	@brief insertDFs_ContiguousFutures

	@param[out] dfResults						Discount Factor Results
	@param[out] fwd_termsmtx					Matrix of fwd rate table (start terms and end terms)
	@param[out] fwds							A list of fwd rates
	@param[in]	data_libor						libor instrument data
	@param[in]	data_fraFuture					Fra/Future instrument data
	@param[in]	fraFutureStartDates				Fra/Future start dates
	@param[in]	fraFutureEndDates				Fra/Future end dates
	@param[in]	fraFutureRates					Fra/Future rates
	@param[in]  asOfDate						curve AsOfDate
	@param[in]	spotDateSwap					Swap Spot Date
	@param[in]	spotDateLibor					Libor Spot Date
	@param[in]	pInter_fw						Future/Fra interpolator
	@param[in]	liborIndexTerm					Reference rate term
	@param[in]	roll_conv						Roll convention
	@param[in]	dc								Trade day count
	@param[in]	dc_act365						dc_act365 day count
	@param[in]	stateVariable					State variable for future/fra
	@param[in]	interpolationJoinDateAsDouble	interpolationJoinDateAsDouble
	@param[in]	liborDate						liborDate
	@param[in]	liborPos						liborPos
	@param[in]	firstSwapDate					Date of the first swap instrument after FRA/Future
	@param[in]	includeSwapsBeforeMPCSwaps		includeSwapsBeforeMPCSwaps
	@param[in]	applyTension					applyTension
	@param[in]	tensionGap						tensionGap
	@param[in]	isFuture						True to indicate it's a future
	@param[in]	populateFwdTable				True to populate fwd table
	*/
	void insertDFs_ContiguousFutures( DiscountFactors & dfResults,
									  DoubleMatrix& fwd_termsmtx,
									  DoubleArray& fwds,
									  const std::vector<LAObject*>&  data_libor,
									  const std::vector<LAObject*>& data_fraFuture,
									  const DateVector& fraFutureStartDates,
									  const DateVector& fraFutureEndDates,
									  const DoubleVector& fraFutureRates,
									  const LADate & asOfDate,
									  const LADate& spotDateSwap,
									  const LADate& spotDateLibor,
									  const LAInterpolationBase& pInter_fw,
									  const LAString& liborIndexTerm,
									  const LAString& roll_conv,
									  const LAPriceDataDayCount& dc,
									  const LAPriceDataDayCount& dc_act365,
									  const StateVariableEnum& stateVariable,
									  const double interpolationJoinDateAsDouble,
									  const LADate& liborDate,
									  const int liborPos,
									  const LADate& firstSwapDate,
									  const bool includeSwapsBeforeMPCSwaps,
									  const bool applyTension,
									  const int tensionGap,
									  const bool isFuture,
									  const bool populateFwdTable = true );

	/*
	@brief  get rollConvention (LUNAR, EOM, NORMAL)
	@param[in]	freq			Frequency
	@param[in]	isEOMRoll	    End of the Month flag
	*/
	LAString getRollConv(const LAString& freq, const bool isEOMRoll);

	/*
	@brief Bootstrap Futures

	@param[out] dfResults						Discount Factor Results
	@param[out] fwd_termsmtx					Matrix of fwd rate table (start terms and end terms)
	@param[out] fwds							A list of fwd rates
	@param[in]	futuresType						futuresType
	@param[in]	serialFraFutures				Serial fra/futures for the method when SerialCalcType is DF
	@param[in]	data_libor						libor instrument data
	@param[in]	data_fraFuture					Fra/Future instrument data
	@param[in]	fraFutureRates					Fra/Future rates
	@param[in]	fraFutureStartDates				Fra/Future start dates
	@param[in]	fraFutureEndDates				Fra/Future end dates
	@param[in]  asOfDate						curve AsOfDate
	@param[in]	spotDateSwap					Swap Spot Date
	@param[in]	spotDateLibor					Libor Spot Date
	@param[in]	pInter_fw						Future/Fra interpolator
	@param[in]	liborIndexTerm					Libor Index Term
	@param[in]	roll_conv						Roll convention
	@param[in]	dc								Trade day count
	@param[in]	dc_act365						dc_act365 day count
	@param[in]	stateVariable					State variable for future/fra
	@param[in]	interpolationJoinDateAsDouble	interpolationJoinDateAsDouble
	@param[in]	liborDate						liborDate
	@param[in]	liborPos						liborPos
	@param[in]	includeSwapsBeforeMPCSwaps		includeSwapsBeforeMPCSwaps
	@param[in]	applyTension					applyTension
	@param[in]	tensionGap						tensionGap
	@param[in]	isFuture						True to indicate it's a future
	@param[in]	populateFwdTable				True to populate fwd table
	*/
	void bootstrapFuturesOrFRAs( DiscountFactors & dfResults,
								  DoubleMatrix& fwd_termsmtx,
								  DoubleArray& fwds,
								  const etrading::FuturesTypeEnum& futuresType,
								  const std::vector<ForwardRate>& serialFraFutures,
								  const std::vector<LAObject*>&  data_libor,
								  const std::vector<LAObject*>& data_fraFuture,
								  const DateVector& fraFutureStartDates,
								  const DateVector& fraFutureEndDates,
								  const DoubleVector& fraFutureRates,
								  const LADate & asOfDate,
								  const LADate& spotDateSwap,
								  const LADate& spotDateLibor,
								  const LAInterpolationBase& pInter_fw,
								  const LAString& liborIndexTerm,
								  const LAString& roll_conv,
								  const LAPriceDataSlidingRule& sld,
								  const LAPriceDataCalendar& cal,
								  const LAPriceDataDayCount& dc,
								  const LAPriceDataDayCount& dc_act365,
								  const StateVariableEnum& stateVariable,
								  const double interpolationJoinDateAsDouble,
								  const LADate& liborDate,
								  const int liborPos,
								  const LADate& firstSwapDate,
								  const bool includeSwapsBeforeMPCSwaps,
								  const bool applyTension,
								  const int tensionGap,
								  const bool isFuture,
								  const bool populateFwdTable = true );
	
	/*
	@brief Populate start/end dates and rates for Fra/Futures

	@param[out]	fraFutureStartDates		Start dates of Fra/Futures
	@param[out]	fraFutureEndDates		End dates of Fra/Futures
	@param[out]	fraFutureRates			Rates of Fra/Futures
	@param[out]	serialFraFutures		Serial fra/futures for the method when SerialCalcType is DF
	@param[out]	allContiguous			True to indicate all the Fra/Futures are in Contiguous months
	@param[in]	futuresType				futuresType
	@param[in]	liborIndexTerm			Reference rate term
	@param[in]	spotDate				Spot date of the curve
	@param[in]	startDate				StartDate of Fra/Future
	@param[in]	endDate					EndDate of Fra/Future
	@param[in]	fwdRate					Forward rate of Fra/Future
	@param[in]	isFuture				True to indicate it's a future
	*/
	void populateFraFutureData(DateVector& fraFutureStartDates,
							   DateVector& fraFutureEndDates,
							   DoubleVector& fraFutureRates,
							   std::vector<ForwardRate>& serialFraFutures,
							   bool& allContiguous,
							   const etrading::FuturesTypeEnum& futuresType,
							   const LAString& liborIndexTerm,
							   const LADate& spotDate,
							   const LADate& startDate,
							   const LADate& endDate,
							   const double fwdRate,
							   const bool isFuture);

	// Method to populate MarketQuotes 
	void updateMarketQuotes( ForwardQuotes& marketQuotes,
							 bool& areAllForwardsContiguous,
							 const etrading::FuturesTypeEnum& futuresType,
							 const LAString& liborIndexTerm,
							 const LADate& spotDate,
							 const LADate& startDate,
							 const LADate& endDate,
							 const double fwdRate,
							 const bool isFuture );

	/*
	@brief Helper function to populate Central bank swap rates and future rates

	@param[inout]	mpcSwapRates	Central bank/FF swap rates
	@param[inout]	futureRates		future rates
	@param[in]	instrumentType	OISMidTermInstrumentType
	@param[in]	startDate		StartDate of Fra/Future
	@param[in]	endDate			EndDate of Fra/Future
	@param[in]	fwdRate			Forward rate of Fra/Future
	*/
	void populateOISMidInstrumentRate(std::map<LADate, std::pair<LADate, double>>& mpcSwapRates, 
									std::map<LADate, std::pair<LADate, double>>& futureRates,
									const OISMidTermInstrumentsEnum& instrumentType,
									const LADate& startDate,
									const LADate& endDate,
									const double fwdRate);

	/*
	@brief Insert daily zero rates for the from last short term swap to start date of first mid term instrument (which can be Central Bank Swap/FF/future)
	*/
	void insertZeroRatesFromShortTermSwapToMidInstrument(DoubleVector& grid,
														DoubleVector& yields,
														std::vector<LADate>& dates,
														const LADate& lastShortTermSwapDate,
														const double lastShortTermSwapRate,
														const std::map<LADate, std::pair<LADate, double>>& mpcSwapRates,
														const std::map<LADate, std::pair<LADate, double>>& futureRates,
														const LADate& spotdate,
														const LAPriceDataDayCount& dc_act365,
														const LAPriceDataCalendar& cal,
														const LAPriceDataConvention& conv);

	/*
	@brief Populate Zero Rates for OIS Mid term instruments (CentralBanks, FedFunds, Futures)
	*/
	void insertZeroRatesForOISMidInstruments(DoubleVector& grid,
											DoubleVector& yields,
											DateVector& dates,
											const LADate& spotdate,
											const LADate& centralBankFromDate,
											const LADate& centralBankShortTermDate,
											const double initialDF,
											const std::map<LADate, std::pair<LADate, double>>& mpcSwapRates,
											const std::map<LADate, std::pair<LADate, double>>& futureRates,
											const std::unique_ptr<LAInterpolationBase>& pInter_yield,
											const LAPriceDataDayCount& dc_act365,
											const LAPriceDataCalendar& cal,
											const LAPriceDataConvention& conv,
											const bool isARRCurve,
											const double lastShortSwapTerm = std::numeric_limits<double>::quiet_NaN());

	/*
	@brief Insert DF to the DiscountFactor table for fixing start date and fixing end dates

	@param[inout] df_2
	@param[inout] df_2_date
	@param[in] inter
	@param[in] stateVariable
	@param[in] fixingStartTerms
	@param[in] fixingEndTerms
	@param[in] fixingStartDates
	@param[in] fixingEndDates
	@param[in] generateForwardsFromSwapsOnly
	@param[in] asOfDate
	@param[in] accrualDaycount
	*/
	void insertDFsForFixingStartEnds(DoubleMatrix& df_2,
									DateVector& df_2_date,
									const LAInterpolationBase &inter,
									const StateVariableEnum& stateVariable,
									const DoubleArray &fixingStartTerms,
									const DoubleArray &fixingEndTerms,
									const DateVector &fixingStartDates,
									const DateVector &fixingEndDates,
									bool generateForwardsFromSwapsOnly,
									const LADate & asOfDate,
									const DayCountEnum & accrualDaycount);
	/*
	@brief Insert DF to the DiscountFactor table for fixing start date and fixing end dates

	@param[out] dfResults
	@param[in] inter
	@param[in] stateVariable
	@param[in] fixingStartTerms
	@param[in] fixingEndTerms
	@param[in] fixingStartDates
	@param[in] fixingEndDates
	@param[in] generateForwardsFromSwapsOnly
	@param[in] asOfDate
	@param[in] accrualDaycount
	*/
	void insertDFsForFixingStartEnds( DiscountFactors & dfResults,
									  const LAInterpolationBase &inter,
									  const StateVariableEnum& stateVariable,
									  const DoubleArray &fixingStartTerms,
									  const DoubleArray &fixingEndTerms,
									  const DateVector &fixingStartDates,
									  const DateVector &fixingEndDates,
									  bool generateForwardsFromSwapsOnly,
									  const LADate & asOfDate,
									  const DayCountEnum & accrualDaycount );


	/*
	@brief Remove duplicated terms based on the specified tolerance,  tolerance default to 1 day.
	*/
	void removeDuplicatedTerms(DoubleVector& terms, const double TERM_TOLERANCE = 1.0/365.0);

}