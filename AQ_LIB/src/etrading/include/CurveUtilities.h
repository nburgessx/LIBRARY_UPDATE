#pragma once

#include "AQLCoreTemplateType.h"
#include "FixingTable.h"
#include "AQLDataInstance.h"
#include "CoreEnumerations.h"
#include "LabelValueBlock.h"
#include "DataProvider.h"
#include "Volatility.h"

namespace etrading
{

	/* @brief			return the day count used in the core library
	*/
	AQLString getDiscountFactorDayCount();

	bool isBasisFlagForDiscountFactor();

	/* @brief			populate and setting default values
	*  @param [in]		curveCollection	curve collection
	*  @param [inout]	crvIndex		curve index
	*  @param [inout]	interpolation	interpolation
	*/
	void populateDiscountFactorConventions(const AQLString& curveCollection, const AQLString& crvIndex, AQLString& interpolation);

	/* @brief			populate and setting default values
	*  @param [in]		curveCollection	curve collection
	*  @param [inout]	curveIndex		curve index
	*  @param [inout]	interpolation	interpolation
	*  @param [inout]	businessDayAdj	business day adjustment
	*  @param [inout]	calendar		calendar
	*/
	void populateDiscountFactorConventions(const AQLString& curveCollection, const AQLString& curveIndex, AQLString& interpolation, AQLString& businessDayAdj, AQLString& calendar);

	/* @brief			populate and setting default values
	*  @param [in]		curveCollection	Curve collection
	*  @param [inout]	crvIndex		Curve index
	*  @param [in]		fwdInter		User specified fwdInter flag
	*  @param [out]		interpolation	Interpolation
	*  @param [inout]	businessDayAdj	Business day adjustment
	*  @param [inout]	calendar		Calendar
	*  @param [inout]	dayCount		Day count
	*  @param [out]		isStdCurve		True if the curve is STD
	*/
	void populateForwardRateConventions(const AQLString& curveCollection,
		const AQLString& crvIndex,
		const BooleanEnum& fwdInter,
		AQLString& interpolation,
		AQLString& businessDayAdj,
		AQLString& calendar,
		AQLString& dayCount,
		bool& isFwdInter,
		bool& useFwdData);


	// Function to get the curve Daycount as an AQLString
	AQLString getCurveDaycountAsLAString(const AQLString& curveCollection, const AQLString& curveIndex);

	/// Function to get the curve Daycount as a std::string
	std::string getCurveDaycountAsStandardString(const std::string & curveCollection, const std::string & curveIndex);

	/* @brief	Generate a fixing schedule with conventions obtained from the curve.
	*  @param[in]	curveCollection				The curve collection
	*  @param[in]	curveIndex					The curveIndex used to get conventions used to build the schedule
	*  @param[out]	curveTenorEnum				Output: The curve tenor e.g. 3M
	*  @param[out]	asOfDate					Output: The curve asOfDate
	*  @param[out]	curveDayCount				Output: The curve dayCount
	*  @param[out]	schedulePaymentDates		Output: The generated schedule payment dates
	*  @param[out]	scheduleAccrualStartDates	Output: The generated schedule accrual start dates
	*  @param[out]	scheduleAccrualEndDates		Output: The generated schedule accrual end dates
	*  @param[out]	scheduleFixingStartDates	Output:	The generated schedule fixing start dates
	*  @param[out]	scheduleFixingEndDates		Output:	The generated schedule fixing end dates
	*/
	void generateScheduleForZeroDiscountMargin(const std::string & curveCollection,
		const std::string & curveIndex,
		CurveTenorEnum &curveTenorEnum,
		AQLDate &asOfDate,
		std::string &curveDayCount,
		DateVector &schedulePaymentDates,
		DateVector &scheduleAccrualStartDates,
		DateVector &scheduleAccrualEndDates,
		DateVector &scheduleFixingStartDates,
		DateVector &scheduleFixingEndDates);

	/* @brief			Calculates the discount factors from a curve and a spread.
	*					The discount factors are calculated using the zero discount margin approach
	*					described by O'Kane in "Credit Spreads Explained".
	*
	*  @param [in]		paymentDates	A vector of payment dates for which discount factors are required
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		The curve index for which to calculate Discount Factors. Default to OIS
	*  @param [in]		spread			Spread to be added to the curve forward-rate when calculating the discount factors.
	*  @param [in]		fixingTableName An optional fixing table name. This field may be blank if no fixings are required.
	*  @return			A vector of discount factors
	*/
	DoubleVector calculateDiscountFactorsUsingZeroDiscountMarginApproach(const DateVector& paymentDates,
		const std::string& curveCollection,
		const std::string& curveIndex,
		const double& spread,
		const std::string& fixingTableName);


	/* @brief			validation method for meCurveDiscountFactors
	*                   NOTE: Payment dates have already been adjusted for business days and holidays
	*                   so we do not apply the same adjustments again. Hence we do not accept
	*                   a businessDayAdjustment or Calendar as input to this function.
	*  @param [in]		paymentDates	A vector of payment dates for which discount factors are required
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		The curve index for which to calculate Discount Factors. Default to OIS
	*  @param [in]		spread			Spread to be added to the curve forward-rate when calculating the discount factors,
	*									following the zero discount margin approach described by O'Kane in "Credit Spreads Explained".
	*  @param [in]		fixingTableName An optional fixing table name. This field may be blank if no fixings are required.
	*  @return			A vector of discount factors
	*/
	DoubleVector getCurveDiscountFactors(const DateVector& paymentDates,
		const AQLString& curveCollection,
		const AQLString& curveIndex,
		const double& spread = 0.0,
		const std::string& fixingTableName = "");

	/* @brief			Get float forward rate
	*  @param [in]		curveFowardRate   forward rate from the curve
	*  @param [in]		asOfDate	      asOfDate
	*  @param [in]		fixingDate	      fixingDate
	*  @param [in]		paymentDate		  paymentDate
	*  @param [in]		fixingTable		        Fixing table object name
	*  @return			The forward rate
	*/
	FloatRateData getForwardRate(const FloatRateData& curveFowardRate,
		const AQLDate& asOfDate,
		const AQLDate& fixingDate,
		const AQLDate& paymentDate,
		const std::shared_ptr<FixingTable>& fixingTable);

	std::string validateCompoundingMethod(const std::string& compoundingMethod);

	// Find the index for the next cashflow, so that we can ignore cashflows in the past
	size_t getFirstNonpastDateIndex(const DateVector& dates, const AQLDate& asOf);

	/* @brief			Get CurveDiscountFactors
	*  @param [in]		toDates			A single or an array of to-date in YYYYMMDD formate
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		Index of the curve set. Default to OIS
	*  @return			A array of discount factors
	*/
	DoubleVector getCurveDiscountFactors(size_t firstNonpastCashflowIndex,
		const DateVector& toDates,
		const AQLString& curveCollection,
		const AQLString& curveIndex);

	/* @brief			validation method for meCurveDiscountFactors
	*  @param [in]		toDates			A single or an array of to-date in YYYYMMDD formate
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		Index of the curve set. Default to OIS
	*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
	*  @param [in]		calendar		Calendar
	*  @return			A array of discount factors
	*/
	DoubleVector getCurveDiscountFactorsFromBaseDate(const size_t& firstNonpastCashflowIndex,
		const AQLDate& baseDate,
		const DateVector& paymentDates,
		const AQLString& curveCollection,
		const AQLString& curveIndex);

	/* @brief			Get the curve forward rate given a year fraction
	*  @param [in]		fromDate			forward rate start or from date
	*  @param [in]		daycount	        daycount fraction
	*  @param [in]		curveCollection	    Curve collection or set ID
	*  @param [in]		curveIndex		    Index of the curve set.
	*  @param [in]		fwdInter			User input fwdInter flag
	*  @param [in]		businessDayAdjust	The business day adjustment
	*  @return			forward rate
	*/
	DoubleVector getCurveForwardRatesFromYearFraction(const DateVector& fromDates,
		double yearFraction,
		const AQLString& dayCount,
		const AQLString& curveCollection,
		const AQLString& curveIndex,
		const BooleanEnum& fwdInter = NONE_BOOL,
		const BusinessDayAdjustmentEnum& businessDayAdj = NONE_BUSINESS_DAY_ADJ);

	/* @brief			Get the curve forward rate given a set of to- and fromDates
	*  @param [in]		fromDates	    A vector of from dates
	*  @param [in]		toDates			A vector of to dates
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set. Default to STD
	*  @param [in]		fwdInter		User input fwdInter flag
	*  @param [in]		businessDayAdjust	The business day adjustment
	*  @return			The forward rates based on fromDates and toDates
	*/
	DoubleVector getCurveForwardRatesFromForwardDates(const DateVector& fromDates,
		const DateVector& toDates,
		const AQLString& curveCollection,
		const AQLString& curveIndex,
		const BooleanEnum& fwdInter = NONE_BOOL,
		const BusinessDayAdjustmentEnum& businessDayAdj = NONE_BUSINESS_DAY_ADJ);

	/* @brief			Helper function to transform resetRates from double list of FloatRateData list
	*  @param [in]		resetRates	    A vector of resetRates in doubles
	*  @return			The forward rates based on fromDates and toDates
	*/
	std::vector<FloatRateData> toFloatRateDataVector(const DoubleVector& resetRates);

	/* @brief			Get the Fixing type
	*  @param [in]		fixingDate				Fixing start date
	*  @param [in]		accrualStartDate		Accrual start date
	*  @param [in]		accrualEndDate			Accrual end date
	*  @param [in]		fixingBusinessDayAdj	Fixing BusinessDayAdj
	*  @param [in]		fixingCalendar			Fixing Calendar
	*  @param [in]		ToleranceTenor			Fixing date tolerance tenor
	*  @return			Fixing type, i.e. Advance, Arrears, or Abitrary
	*/
	FixingTypeEnum getFixingTypeByFixingDate(const AQLDate& fixingDate,
		const AQLDate& accrualStartDate,
		const AQLDate& accrualEndDate,
		const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
		const std::string& fixingCalendar,
		const std::string& fixingDateToleranceTenor);

	/* @brief			Get the curve forward rate with consideration of convexity adjustment
	*  @param [in]		fromDate				unadjustedForwardRate's date
	*  @param [in]		toDate					adjustedForwardRate's date
	*  @param [in]		valuationDate			Valuation date
	*  @param [in]		dayCount				Day count of the curve
	*  @param [in]		fixingType				Fixing in Advance or in Arrears
	*  @param [in]		volObject				Volatility object
	*  @param [in]		convexityMethod			Convexity method
	*  @param [in]		curveCollection			Curve collection, e.g. EURYC
	*  @param [in]		curveIndex				OIS Curve index
	*  @return			The adjusted forward rate
	*/
	FloatRateData getCurveForwardRateWithConvexityAdjustment(const double& unadjustedForwardRate,
		const AQLDate& fixingDate,
		const AQLDate& fixingEndDate,
		const AQLDate& valuationDate,
		const DayCountEnum& dayCount,
		const FixingTypeEnum& fixingType,
		const VolatilityPtr& volObject,
		const ConvexityMethodEnum& convexityMethod,
		const std::string& curveCollection,
		const std::string& curveIndex);

	/* @brief			Get the curve forward rate
	*  @param [in]		fromDates	    A vector of from dates
	*  @param [in]		toDates			A vector of to dates
	*  @param [in]		valuationDate	Valuation date
	*  @param [in]		dayCount		Day count of the curve
	*  @param [in]		fixingType		Fixing in Advance or in Arrears
	*  @param [in]		volObject		Volatility object
	*  @param [in]		convexityMethod			Convexity method
	*  @param [in]		curveCollection	Curve collection, e.g. EURYC
	*  @param [in]		curveIndex		OIS Curve index
	*  @return			The forward rates based on fromDates and toDates
	*/
	std::vector<FloatRateData> getCurveForwardRatesWithConvexityAdjustment(const DoubleVector& unadjustedForwardRates,
		const DateVector& fromDates,
		const DateVector& toDates,
		const AQLDate& valuationDate,
		const DayCountEnum& dayCount,
		const FixingTypeEnum& fixingType,
		const VolatilityPtr& volObject,
		const ConvexityMethodEnum& convexityMethod,
		const std::string& curveCollection,
		const std::string& curveIndex);

	/* @brief			Get CurveFixingEndDate, which uses the curve frequency, useful when implying forward rates from discount factors
	*  @param [in]		fixingDates	            A vector of fixing Dates
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		        Index of the curve set. Default to STD
	*  @param [in]		businessDayAdjust	The business day adjustment
	*  @param [in]		calendar		        Calendar
	*  @param [in]		fwdInter				User input fwdInter flag
	*  @return			The forward rates based on the fixing dates
	*/
	DateVector getCurveFixingEndDates(const DateVector& fixingDates,
		const AQLString& curveCollection,
		const AQLString& curveIndex,
		const BusinessDayAdjustmentEnum& businessDayAdj = NONE_BUSINESS_DAY_ADJ,
		const AQLString& calendar = "",
		const BooleanEnum& fwdInter = NONE_BOOL,
		const CurveTenorEnum& curveTenorOverride = NONE_CURVE_TENOR);

	/* @brief			Get CurveFixingEndDateAsTerms, which uses the curve frequency, useful when implying forward rates from discount factors
	*  @param [in]		fixingDates	            A vector of fixing Dates
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		        Index of the curve set. Default to STD
	*  @param [in]		businessDayAdjust	The business day adjustment
	*  @param [in]		calendar		        Calendar
	*  @param [in]		fwdInter				User input fwdInter flag
	*  @return			The forward rates based on the fixing dates
	*/
	DoubleVector getCurveFixingEndDatesAsTerms(const DateVector& fixingDates,
		const AQLString& curveCollection,
		const AQLString& curveIndex,
		const BusinessDayAdjustmentEnum& businessDayAdj = NONE_BUSINESS_DAY_ADJ,
		const AQLString& calendar = "",
		const BooleanEnum& fwdInter = NONE_BOOL,
		const CurveTenorEnum& curveTenorOverride = NONE_CURVE_TENOR);

	/* @brief			Get CurveForwardRates, which uses the curve frequency
	*  @param [in]		fixingDates	            A vector of fixing Dates
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		        Index of the curve set. Default to STD
	*  @param [in]		businessDayAdjust	The business day adjustment
	*  @param [in]		calendar		        Calendar
	*  @param [in]		fwdInter				User input fwdInter flag
	*  @return			The forward rates based on the fixing dates
	*/
	DoubleVector getCurveForwardRates( const DateVector& fixingDates,
									   const AQLString& curveCollection,
									   const AQLString& curveIndex,
									   const BusinessDayAdjustmentEnum& businessDayAdj = NONE_BUSINESS_DAY_ADJ,
									   const AQLString& calendar = "",
									   const BooleanEnum& fwdInter = NONE_BOOL,
									   const CurveTenorEnum& curveTenorOverride = NONE_CURVE_TENOR);


	/* @brief			Generates a date schedule and uses the date schedule to calculate a vector of forward rates for a set of curveIndices.
	*  @param [out]		fixingDates			The fixing dates on which the forwardRates are calculated. Generated from schedule parameters
	*  @param [out]		forwardRates		A matrix containing the calculated forward rates for each curve, for each fixing date in the generated schedule
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndices		The curveIndices to use when calculating forward rates
	*  @param [in]		startDate			An optional string specifying the first date in the schedule. If empty, the curveCollection asOf date is used.
	*  @param [in]		maturity			A tenor describing the length of the date schedule, for example 10Y
	*  @param [in]		businessDayAdjust	The businessday adjustment, for example MODFOLLOWING
	*  @param [in]		calendar			The holiday calendar to use when rolling out the date schedule
	*  @param [in]		rollConvention		The roll convention to use when generating the date schedule, for example IMM
	*  @param [in]		frequency			The frquency of points in the date schedule, for example MONTHLY
	*  @param [in]		fwdInters			User input's fwdInter flags
	*/
	void getForwardRatesForCurveIndices(DateVector& fixingDates,
									    DoubleMatrix& forwardRates,
									    const AQLString& curveCollection,
									    const AQLStringVector& curveIndices,
									    const AQLString& startDate,
									    const AQLString& maturity,
									    const BusinessDayAdjustmentEnum& businessDayAdjust,
									    const AQLString& calendar,
									    const AQLString& rollConvention,
									    const AQLString& frequency,
									    const AQLStringVector& fwdInters = AQLStringVector());

	/* @brief			Generates a date schedule and uses the date schedule to calculate a vector of discount factors for a set of curveIndices.
	*  @param [out]		paymentDates		The payment dates on which the discount factors are calculated. Generated from schedule parameters.
	*  @param [out]		discountFactors		A matrix containing the calculated discount factors for each curve, for each fixing date in the generated schedule
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndices		The curveIndices to use when calculating discount factors
	*  @param [in]		startDate			An optional string specifying the first date in the schedule. If empty, the curveCollection asOf date is used.
	*  @param [in]		maturity			A tenor describing the length of the date schedule, for example 10Y
	*  @param [in]		businessDayAdjust	The businessday adjustment, for example MODFOLLOWING
	*  @param [in]		calendar			The holiday calendar to use when rolling out the date schedule
	*  @param [in]		rollConvention		The roll convention to use when generating the date schedule, for example IMM
	*  @param [in]		frequency			The frquency of points in the date schedule, for example MONTHLY
	*/
	void getDiscountFactorsForCurveIndices(DateVector& paymentDates,
										   DoubleMatrix& discountFactors,
										   const AQLString& curveCollection,
										   const AQLStringVector& curveIndices,
										   const AQLString& startDate,
										   const AQLString& maturity,
										   const BusinessDayAdjustmentEnum& businessDayAdjust,
										   const AQLString& calendar,
										   const AQLString& rollConvention,
										   const AQLString& frequency);


	/* @brief			Calculates discount factors for a curve.
	*					If a spread has been provided, performs the zero-discount margin calculation
	*					described by O'Kane in "Credit Spreads Explained".
	*  @param [in]		paymentDates	An array of payment dates
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		The curve index for which to calculate Discount Factors.
	*  @param [in]		spread			Spread to be added to the curve forward-rate when calculating the discount factors,
	*									following the zero discount margin approach described by O'Kane in "Credit Spreads Explained".
	*  @param [in]		fixingTableName An optional fixing table name. This field may be blank if no fixings are required.    *  @return			A array of discount factors
	*/
	DoubleVector getDiscountFactorsForCurveIndexWithSpread(const DateVector& paymentDates,
														   const std::string& curveCollection,
														   const std::string& curveIndex,
														   const double& spread,
														   const std::string& fixingTableName);

	/* @brief			Get the curve indics and curve tenors for stub rate function where the returned are order by curve tenors
	*  @param [in]		curveCollection	ID of the yield curve
	*  @return			curve indics and curve tenors
	*/
	std::pair<AQLStringVector, AQLStringVector> getStubRateCurveIndicesTenors(const AQLString& curveCollection);

	/* @brief			Get the matching curveIndex (by staticDataTable) from the curveIndices
	*  @param [in]		stubCurveIndex Stub Curve Index specified by the user
	*  @param [in]		curveIndices     Indcies for the stub curves
	*  @return			the matching curveIndex from curveIndices
	*/
	AQLString getMatchingCurveIndex(const AQLString& curveCollection, const AQLString& stubCurveIndex, const AQLStringVector& curveIndices);

	/* @brief			Calculate toDate based on startDates and tenor
	*  @param [in]		startDate			The from date
	*  @param [in]		tenor				Tenor added to the from date
	*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
	*  @param [in]		calendar			Calendar
	*  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
	*  @return			A vector of end dates
	*/
	AQLDate getSingleDateFromTenor(const AQLDate& startDate,
								  const AQLString & tenor,                    // TODO: Convert to StandardString
								  const AQLString & businessDayAdj,           // TODO: Convert to StandardString
								  const AQLString & calendar,                 // TODO: Convert to StandardString
								  const AQLString & rollConvention = "");     // TODO: Convert to StandardString

/* @brief			Calculate toDate based on startDates and tenor
*  @param [in]		startDates			A vector of from dates
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			A vector of end dates
*/
	DateVector getDateFromTenor(const DateVector& startDates,
								const AQLString& tenor,              // TODO: Convert to StandardString
								const AQLString& businessDayAdj,     // TODO: Convert to StandardString
								const AQLString& calendar,           // TODO: Convert to StandardString
								const AQLString& rollConvention);	// TODO: Convert to StandardString

    /* @brief			Method to get the curve spot date by applying multiple date shifts
    *  @param [in]		asOfDate		        The curve asOfDate
    *  @param [in]		fixingLag			    Tenor added to the from date
    *  @param [in]		fixingCalendar		    Fixing Calendar
    *  @param [in]		fixingBusDayAdj		    Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		paymentLag			    Tenor added to the from date
    *  @param [in]		paymentCalendar		    Fixing Calendar
    *  @param [in]		paymentBusDayAdj        Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The curve spot date
    */
	AQLDate getShiftedSpotDate(const AQLDate& asOfDate,
						      const AQLString& fixingLag,			// TODO: Convert to StandardString
						      const AQLString& fixingCalendar,		// TODO: Convert to StandardString
                              const AQLString& fixingBusDayAdj,		// TODO: Convert to StandardString
                              const AQLString& paymentLag,			// TODO: Convert to StandardString
						      const AQLString& paymentCalendar,		// TODO: Convert to StandardString
                              const AQLString& paymentBusDayAdj);	// TODO: Convert to StandardString

    /* @brief			Calculate the curve spot date based on the curve asOfDate, spot lag, fixing calendar and payment calendar
    *  @param [in]		curveAsOfDate		The curve asOfDate
    *  @param [in]		spotLag				Tenor added to the from date
    *  @param [in]		fixingCalendar		Fixing Calendar
    *  @param [in]		paymentCalendar		Payment Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The curve spot date
    */
	AQLDate getCurveUSDSpotDate(const AQLDate& curveAsOfDate,
							   const AQLString& spotLag,				// TODO: Convert to StandardString
							   const AQLString& fixingCalendar,	    // TODO: Convert to StandardString
							   const AQLString& paymentCalendar,		// TODO: Convert to StandardString
							   const AQLString& businessDayAdj,		// TODO: Convert to StandardString
							   const AQLString& rollConvention);		// TODO: Convert to StandardString

    /* @brief			Calculate toDate based on startDate and yearFraction
    *  @param [in]		startDate			Start date
    *  @param [in]		yearFraction		Year fraction
    *  @param [in]		dayCount			Day count convention
    *  @return			The end date derived from the FromDate and given year fraction
    */
	AQLDate getDateFromYearFraction(const AQLDate& startDate,
		                           const double& yearFraction,
		                           const DayCountEnum& dayCount);

	//This function is the same as etrading::AQLCurveForwardRateHelpers::compound() function, but making use of the fixing table
	double compound(AQLDataInstance* dataInstance,
					const AQLString& curveID,
					const AQLString& forecastCurveName,
					const AQLDate& start_date,
					const AQLDate& end_date,
					const double spread,
					const AQLString& frequency_,
					const bool is_start_roll,
					const AQLString& roll_convention_,
					const AQLString& calendar_,
					const AQLString& sliding_rule_,
					const AQLString& day_count_,
					const AQLString& interpolation_,
					const AQLString& compound_type_,
					const bool isFwdInter,
					const std::shared_ptr<FixingTable>& fixingTable,  //fixing table 
					bool annualized,                                 //true to return annualized rate, default to true
					const AQLDate* first_odd = NULL,
					const AQLDate* last_odd = NULL,
					const bool usePDay = false);

	/* @brief			Calculate compound rate with fixing table
	*  @param [in]		startDates			A vector of start dates
	*  @param [in]		endDates			A vector of end dates
	*  @param [in]		curveCollection		ID of the curve set
	*  @param [in]		forecastCurveIndex	Forecasting curve index
	*  @param [in]		frequency			Frequency
	*  @param [in]		spread				Spread in basis points.
	*  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
	*  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
	*  @param [in]		calendar			Calendar
	*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
	*  @param [in]		dayCount			Day count convention
	*  @param [in]		interpolation		Interpolation method
	*  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
	*  @param [in]		firstStubDate		End date of the front stub period
	*  @param [in]		lastStubDate		Start date of the end stub period
	*  @param [in]		fwdInter			User input fwdInter flag
	*  @param [in]		fixingTable		    Fixing table object
	*  @param [in]		annualized  		True to return annualized rate. Default to true
	*  @return			interest rates calculated based on the startDates, endDates, and compound type
	*/
	DoubleVector calculateCompoundRateWithFixingTable(const DateVector& startDates,
													  const DateVector& endDates,
													  const AQLString& curveCollection,
													  const AQLString& forecastCurveIndex,
													  const AQLString& frequency,
													  double spread,
													  const StubTypeEnum& stubType,
													  const AQLString& rollDayInput,
													  const AQLString& calendar,
													  const BusinessDayAdjustmentEnum& businessDayAdj,
													  const DayCountEnum& dayCount,
													  const AQLString& interpolation,
													  const AQLString& compoundType,
													  const AQLString& firstStubDate,
													  const AQLString& lastStubDate,
													  const BooleanEnum& fwdInter,
													  const std::shared_ptr<FixingTable>& fixingTable,
													  bool annualized = true);

	void fixingTableExistenceValidation(const std::shared_ptr<FixingTable>& fixingTable, const AQLDate& asOfDate, const DateVector& fixingDates, const DateVector& paymentDates);


	/* @brief			Method to interpolate an interpolation function
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
	double interpolate(const DoubleVector& xValues,
		const DoubleVector& yValues,
		const double xPoint,
		const InterpolationEnum& interpolationMethod,
		const double joinXValue = 0.0);

	// Interpolate a vector of xPoints
	DoubleVector interpolate(const DoubleVector& xValues,
		const DoubleVector& yValues,
		const DoubleVector xPoints,
		const InterpolationEnum& interpolationMethod,
		const double joinXValue = 0.0);

	/* @brief			Method to differentiate an interpolation function at a single point
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
	double differentiate(const DoubleVector& xValues,
						 const DoubleVector& yValues,
						 const double xPoint,
						 const InterpolationEnum& interpolationMethod,
						 const double joinXValue = 0.0);

	/* @brief			Method to differentiate an interpolation function on a vector of single points
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
	DoubleVector differentiate(const DoubleVector& xValues,
							   const DoubleVector& yValues,
							   const DoubleVector xPoints,
							   const InterpolationEnum& interpolationMethod,
							   const double joinXValue = 0.0);

	/* @brief			Method to differentiate an interpolation function discreteley given a start and end point
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
	double differentiate(const DoubleVector& xValues,
						 const DoubleVector& yValues,
						 const double fromXPoint,
						 const double toXPoint,
						 const InterpolationEnum& interpolationMethod,
						 const AQLDate & asOfDate,
						 const DayCountEnum & accrualDaycount,
						 const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING,
						 const double joinXValue = 0.0);

	/* @brief			Method to differentiate an interpolation function discreteley given a vector of ranges
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
	DoubleVector differentiate(const DoubleVector& xValues,
							   const DoubleVector& yValues,
							   const DoubleVector fromXPoints,
							   const DoubleVector toXPoints,
							   const InterpolationEnum& interpolationMethod,
							   const AQLDate & asOfDate,
							   const DayCountEnum & accrualDaycount,
							   const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING,
							   const double joinXValue = 0.0);

	/* @brief			Method to integrate an interpolation function
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
	double integrate(const DoubleVector& xValues,
					 const DoubleVector& yValues,
					 const double lowerBound,
					 const double upperBound,
					 const InterpolationEnum& interpolationMethod,
					 const AQLDate & asOfDate,
					 const DayCountEnum & accrualDaycount,
					 const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING,
					 const double joinXValue = 0.0);

	// Integrate a vector of xPoints
	DoubleVector integrate(const DoubleVector& xValues,
						   const DoubleVector& yValues,
						   const DoubleVector lowerBounds,
						   const DoubleVector upperBounds,
						   const InterpolationEnum& interpolationMethod,
						   const AQLDate & asOfDate,
						   const DayCountEnum & accrualDaycount,
						   const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING,
						   const double joinXValue = 0.0);

	/* @brief			Calculate the discount factors from baseDate to targetDates
	*  @param [in]		baseDate	    The date the discount factor is based on
	*  @param [in]		targetDates		A single or an array of to-date
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		The curve index for which to calculate Discount Factors. Default to OIS
	*  @return			A array of discount factors
	*/
	DoubleVector getCurveDiscountFactors(const AQLDate& baseDate,
		const DateVector& targetDates,
		const AQLString& curveCollection,
		const AQLString& curveIndex);

	/* @brief			Calculate the discount factors from FromDates to ToDates
	*  @param [in]		fromDates	    A single or an array of From-date
	*  @param [in]		toDates			A single or an array of To-date
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		The curve index for which to calculate Discount Factors. Default to OIS
	*  @return			A array of discount factors
	*/
	DoubleVector getCurveDiscountFactors(const DateVector& fromDates, const DateVector& toDates, const AQLString& curveCollection, const AQLString& curveIndex);


	/* @brief			Converts a given set of yearFractions to ACT/365 yearFractions for Discount factor functions
	*  @param [in]		startDate	    start date
	*  @param [in]		yearFractions	A list of year fractions
	*  @param [in]		dayCount		Day count
	*  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
	*  @return			Year fraction based on ACT365
	*/
	void convertToYearFractionsACT365(const AQLDate& startDate, DoubleVector& yearFractions, const AQLString& dayCount, const bool& includeLast = true);

	/* @brief			Get curve discount factor based on fromDate and year fractions
	*  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
	*  @param [in]	    yearFractions		An array of year fraction yearFractions
	*  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
	*  @param [in]		curveCollection		Curve set ID
	*  @param [in]		curveIndex			Index of the curve set. Default to OIS
	*  @return			A array of discount factors
	*/
	DoubleVector getCurveDiscountFactorsForwardStartingFromYearFractions(const DateVector& fromDates,
		const DoubleVector& yearFractions,
		const AQLString& dayCount,
		const AQLString& curveCollection,
		const AQLString& curveIndex);

	/* @brief			Get discount factors from asOfDate and yearFractions
   *  @param [in]      yearFractions	An array of yearFractions by year count
   *  @param [in]		dayCount		Day count
   *  @param [in]		curveCollection	Curve set ID
   *  @param [in]		curveIndex		Index of the curve set. Default to OIS
   *  @return			An array of discount factor
   */
	DoubleVector getCurveDiscountFactorsFromYearFractions(const DoubleVector& yearFractions,
		const AQLString& dayCount,
		const AQLString& curveCollection,
		const AQLString& curveIndex);

	/* @brief			populate and setting default values
	*  @param [in]		curveCollection	curve collection
	*  @param [inout]	curveIndex		curve index
	*  @param [in]		fwdInter		User specified fwdInter flag
	*  @param [inout]	interpolation	interpolation
	*  @param [inout]	businessDayAdj	business day adjustment
	*  @param [inout]	dayCount		day count
	*/
	void populateZeroRateConventions(const AQLString& curveCollection,
		AQLString& curveIndex,
		const BooleanEnum& fwdInter,
		AQLString& interpolation,
		AQLString& dayCount,
		AQLString& frequency,
		bool& isFwdInter,
		AQLString& businessDayAdj);

	/* @brief			Zero rate over the yearFractions from the curve's asof date
	*  @param [in]		yearFractions	A  list of given year fractions
	*  @param [in]		curveCollection	Curve collection id
	*  @param [in]		curveIndex		Curve index name. Default to STD
	*  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
	*  @param [in]		dayCount		Day count convention. Default to ACT/365
	*  @return			Zero rate
	*/
	DoubleVector getCurveZeroRatesFromYearFractions(const DoubleVector& yearFractions,
		const AQLString& curveCollection,
		const AQLString& curveIndex,
		const AQLString& frequency = "",
		const AQLString& dayCount = "",
		const BooleanEnum& fwdInter = NONE_BOOL);


	/* @brief			Get the curve forward rate given curveAsOfDate and a set of toDates
	*  @param [in]		toDates			A vector of to dates
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set. Default to STD
	*  @param [in]		fwdInter		User input fwdInter flag
	*  @return			The forward rates based on fromDates and toDates
	*/
	DoubleVector getCurveForwardRatesFromAsOfDate(const DateVector& toDates,
												  const AQLString& curveCollection,
												  const AQLString& curveIndex,
												  const BooleanEnum& fwdInter = NONE_BOOL);


	/* @brief			Get the short rate(s) at fromDate(s), we use overnight forward rate as an approximation
	*  @param [in]		fromDates	    A vector of from dates
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set
	*  @param [in]		fwdInter		User input fwdInter flag
	*  @return			The short rates based on fromDates
	*/
	DoubleVector getCurveShortRates(const DateVector& fromDates, const std::string& curveCollection, const std::string& curveIndex, const BooleanEnum& fwdInter = NONE_BOOL);

	/* @brief			Retrieve the hybrid interpolation join date from a swap curve
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set
	*  @return			The join date used in hybrid interpolation of a swap curve
	*/
	AQLDate getHybridInterpolationJoinDate(const AQLString& curveCollection, const AQLString& curveIndex);

	/* @brief			Retrieve the hybrid interpolation join date as a double from a swap curve
	*  IMPORTANT NOTE:	Join Dates are Relative to the Curve's Swap Calibration Spot Dates NOT the curve asOfDate
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set
	*  @return			The join date used in hybrid interpolation of a swap curve
	*/
	double getHybridInterpolationJoinDateAsDouble(const AQLString& curveCollection, const AQLString& curveIndex);

	/* @brief			Retrieve the linear spline join date from a swap curve
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set
	*  @return			The join date used in linear spline interpolation of a swap curve
	*/
	AQLDate getLinearSplineJoinDate(const AQLString& curveCollection, const AQLString& curveIndex);

	/* @brief			Retrieve the linear spline join date as a double from a swap curve
	*  IMPORTANT NOTE:	Join Dates are Relative to the Curve's Swap Calibration Spot Dates NOT the curve asOfDate
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set
	*  @return			The join date used in linear spline interpolation of a swap curve
	*/
	double getLinearSplineJoinDateAsDouble(const AQLString& curveCollection, const AQLString& curveIndex);

	/* @brief			Calculate the EuroDollar Futures Convexity Adjustment
	* @param [in]		curveAsOfDate       The yield curve as of or valuation date
	* @param [in]		futuresStartDate    The futures start date
	* @param [in]		futuresEndDate      The futures end date
	* @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
	* @param [in]		volatility          The Hull-White 1F Volatility Parameter
	* @param [out]		Returns the EuroDollar Futures Convexity Adjustment
	*/
	double getCurveEuroDollarConvexityAdjustment(const AQLDate& curveAsOfDate, const AQLDate& futuresStartDate, const AQLDate& futuresEndDate, const double& meanReversion, const double& volatility);

	/* @brief			Future Price from FRA rate based on the given convexity adjustment
	* @param [in]		fraRate				The given fra rate
	* @param [in]		convexityAdjustment The given convexity adjustment between Future rate and Fra rate
	* @param [out]		Returns Future Price
	*/
	double fromFraRateToFuturePrice(const double& fraRate, const double& convexityAdjustment);

	/* @brief			FRA rate from Future Price based on the given convexity adjustment
	* @param [in]		futurePrice			The given future price
	* @param [in]		convexityAdjustment The given convexity adjustment between Future rate and Fra rate
	* @param [out]		Returns Fra Rate
	*/
	double fromFuturePriceToFraRate(const double& futurePrice, const double& convexityAdjustment);

	/* @brief			Check if a curve was calibrated using global curve engine
	* @param [in]		curveCollection			Name of the curve collection
	* @param [in]		curveName				Curve name
	* @return			Boolean - was curve built from curve engine
	*/
	bool isCurveBuiltFromCurveEngine(const AQLString& curveCollection, const AQLString& curveName);

	/* @brief Display the yield curve engine jacobian matrix
	* @param [out] matrix							Matrix being returned and displayed
	* @param [out] labelMatrix						Matrix that has label for each jacobian matrix element
	* @param [in] curveEngineObject					The name of the dual-bootstrapped object
	* @param [in] curveCollection					Curve collection name
	* @param [in] displayLabels						Display labels of the matrix
	* @param [in] displayInverseMatrix				Display inverse Jacobian matrix or not
	*/
	void displayCurveEngineJacobian(DoubleMatrix& matrix,
									AQLStringMatrix& labelMatrix,
									const AQLString& curveEngineObject,
									const AQLString& curveCollection,
									bool displayLabels,
									bool displayInverseMatrix);

	/* @brief Display the jacobian matrix of one yield curve
	* @param [out] matrix							Matrix being returned and displayed
	* @param [in] curveCollection					Curve collection name
	* @param [in] curveName							Name of yield curve
	* @param [in] displayInverseMatrix				Display inverse Jacobian matrix or not
	*/
	void displayCurveJacobian(DoubleMatrix& matrix,
							  const AQLString& curveCollection,
							  const AQLString& curveName,
							  bool displayInverseMatrix);

	/* @brief			Get ValuationDate from the valuationSettingsLVB map, default by using curve's asOfDate
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		modelName			The model or curve collection name
	* @output			The matched valuationDate
	*/
	AQLDate getValuationDate(const LabelValueBlock& valuationSettingsLVB, const AQLString& modelName);

	/* @brief			Get ValuationDate from the valuationSettingsLVB map, default by using curve's asOfDate
	* @param [in]		forwardRate					Forward rate fixed in advance
	* @param [in]		fixingDate					Fixing date for forward rate fixed in advance
	* @param [in]		inArrearFixingDate  		Fixing date for forward rate fixed in arrears
	* @param [in]		valuationDate  				Valuation date
	* @param [in]		dayCount  					Fixing daycount
	* @param [in]		volatility  				Volatility input to the convexity formula
	* @param [in]		volatilityType  			Volatility type
	* @output			Convexity for libor in arrear
	*/
	double liborInArrearConvexity(const double& forwardRate, const AQLDate& fixingDate, const AQLDate& inArrearFixingDate, const AQLDate& valuationDate, const DayCountEnum& dayCount, const double& volatility, const VolatilityTypeEnum& volatilityType);


	/* @brief			Helper function to populate fixing start and end dates from fixingDates
	* @param [out]		fixingStartDates		Fixing start date
	* @param [out]		fixingEndDates			Fixing end date based on the curve tenor; when if there is front/end stub, it will be overrided by end date
	* @param [in]		allFixingDates  		All fixing dates calculated from accrued date and fixLag
	* @param [in]		curveFrequency  		Curve Frequency, e.g. Daily, Quartely, Semi-Annual, Annual
	* @param [in]		fixingBusinessDayAdj  	Fixing business day adjustment
	* @param [in]		fixingCalendar  		Fixing calendar
	* @param [in]		rollConvention  		rollConvention
	* @param [in]		isRegularSwap  		    True for swap with no stub, false for swap with front/end stub
	* @param [in]		isFrontStubType  		True for front stub, False for end stub
	*/
	void populateFixingStartEndDates(DateVector& fixingStartDates,
									DateVector& fixingEndDates,
									const DateVector& allFixingDates,
									const FrequencyEnum& curveFrequency,
									const AQLString& fixingBusinessDayAdj,
									const AQLString& fixingCalendar,
									const AQLString& rollConvention,
									const bool isRegularSwap,
									const bool isFrontStubType);


	/* Function to calculate the accrual period or term given the forward rate start- and end-dates in year fraction terms
	* @param [in]		fixingStartTerm				fixing start date as a curve term (i.e. using hard-coded ACT/365)
	* @param [in]		fixingEndTerm  				fixing end date as a curve term (i.e. using hard-coded ACT/365)
	* @param [in]		asOfDate  					curve as of date
	* @param [in]		accrualDaycount				instrument accrual daycount basis
	* @param [in]		compoundFrequency			compounding frequency enum - defaults to SIMPLE COMPOUNDING
	* @output			The forward rate accrual period
	*/
	double accrualPeriod( const double & fixingStartTerm,
						  const double & fixingEndTerm,
						  const AQLDate & asOfDate,
						  const DayCountEnum & accrualDaycount,
						  const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING );

	/* Function to calculate the accrual period or term given the forward rate start- and end-dates in year fraction terms
	* @param [in]		fixingStartDate				fixing start date
	* @param [in]		fixingEndDate  				fixing end date
	* @param [in]		asOfDate  					curve as of date
	* @param [in]		accrualDaycount				instrument accrual daycount basis
	* @param [in]		compoundFrequency			compounding frequency enum - defaults to SIMPLE COMPOUNDING
	* @output			The forward rate accrual period
	*/
	double accrualPeriod( const AQLDate & fixingStartDate,
						  const AQLDate & fixingEndDate,
						  const AQLDate & asOfDate,
						  const DayCountEnum & accrualDaycount,
						  const CompoundingFrequencyEnum & compoundFrequency= SIMPLE_COMPOUNDING );

	/* Function to calculate the accrual daycount adjustment from a curve daycount of ACT/365 to the daycount basis specified
	* @param [in]		fixingStartTerm				fixing start date as a curve term (i.e. using hard-coded ACT/365)
	* @param [in]		fixingEndTerm  				fixing end date as a curve term (i.e. using hard-coded ACT/365)
	* @param [in]		asOfDate  					curve as of date
	* @param [in]		accrualDaycount				instrument accrual daycount basis
	* @param [in]		compoundFrequency			compounding frequency enum - defaults to SIMPLE COMPOUNDING
	* @output			The accrual daycount adjustment to convert a year fraction to the correct daycount basis
	*/
	double daycountConversionFactor( const double & fixingStartTerm,
									 const double & fixingEndTerm,
									 const AQLDate & asOfDate,
									 const DayCountEnum & accrualDaycount,
									 const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING );

	/* Function to calculate the accrual daycount adjustment from a curve daycount of ACT/365 to the daycount basis specified given the forward rate start- and end-dates in year fraction terms
	* @param [in]		fixingStartDate				fixing start date
	* @param [in]		fixingEndDate  				fixing end date
	* @param [in]		accrualPeriodAct365			The date year fraction between the fixing Start- and End Dates in the curve year faction of ACT/365
	* @param [in]		asOfDate  					curve as of date
	* @param [in]		accrualDaycount				instrument accrual daycount basis
	* @param [in]		compoundFrequency			compounding frequency enum - defaults to SIMPLE COMPOUNDING
	* @output			The accrual daycount adjustment to convert a year fraction to the correct daycount basis
	*/
	double daycountConversionFactor( const AQLDate & fixingStartDate,
									 const AQLDate & fixingEndDate,
									 const double & accrualPeriodAct365,
									 const AQLDate & asOfDate,
									 const DayCountEnum & accrualDaycount,
									 const CompoundingFrequencyEnum & compoundFrequency= SIMPLE_COMPOUNDING );

	// oisHistoricalRates can be either: 1) fixingTable object handle name, or 2) a column of fixingDates and a column of fixintRates
	AQLStringMatrix retrieveFixingTableMatrix(const AQLStringMatrix& oisHistoricalRates);
}


