#pragma once

#include "LabelValueBlock.h"
#include "SwapUtilities.h"
#include "FreeObject.h"
#include "BondEnumerations.h"

#include <string>
#include <vector>

namespace etrading
{

	struct BondFwdReinvestedCoupon
	{
		double couponValue = std::numeric_limits<double>::quiet_NaN(); // paid coupon value, the value has been scaled by 100/notional
		double couponFwdYearFraction = std::numeric_limits<double>::quiet_NaN(); // from paid coupon date to fwdSettleDate
		double couponFwdValue = std::numeric_limits<double>::quiet_NaN(); // paid coupon's forward value at bond fwd settle date 
	};


	/* @brief			Validate Bond Description Label Value Block Keys
	*  @return		    Returns a vector of expected Bond LVB Keys
	*/
	std::vector<std::string> bondDescriptionLVBKeys();

	/* @brief			Validate Bond Label Value Block Keys, which include description LVB and Schedule LVB
	*  @return		    Returns a vector of expected Bond LVB Keys
	*/
	std::vector<std::string> fixedBondLVBKeys();

	/* @brief			Validate Bond Label Value Block Keys, which include description LVB and Schedule LVB
	*  @return		    Returns a vector of expected Bond LVB Keys
	*/
	std::vector<std::string> floatBondLVBKeys();

    /* @brief			Validate Bond Description Label Value Block Keys
	*  @param [in]		keysFromUser		A list of keys from user input
	*  @param [in]		validateKeys		True to do the verification. Default to true
	*  @param [in]		withScheduleKeys	True to include keys from Schedule (Fixed/Float). Default to true
	*/
	void validateBondDescriptionLVBKeys( const LabelValueBlock& bondDescriptionLVB, bool validateKeys=true, const std::string& LVBname = "" );
    
	/* @brief Create the Bond Label Value Block from a LWO Free-Object
	*  @param [in]		freeObject				freeObject
	*  @param [in]		objectName			    objectName
	*  @Return     A Label Value Block containing Bond Schedule Data
	*/
	LabelValueBlock createBondLVBFromFreeObject(const FreeObject& freeObject, const std::string& objectName);


	/* @brief Function to convert the bond accrual frequency string to a year fraction
    *  @param [in]		accrualFrequency  An enum representing the Bond accrual frequency e.g. ANNUAL, SEMI-ANNUAL
	*  @Return          returns accrual year fraction
    */
	double convertBondFrequencyToYearFraction( const FrequencyEnum& accrualFrequency );

    /* @brief   Function to calculate the start index to get the index for the first bond coupon that is not in the past
    *  @param [in]		settlementDate                      The bond settlement date
    *  @param [in]		bondPaymentlDatesForIndexation      A vector of all bond cashflow payment dates, these dates are assumed to be SORTED
	*  @Return          returns the index position of the active cashflow given a vector of dates. Will return the final index position if all cashflows are in the past
    */
    unsigned int getBondActiveCashflowIndex( const LADate& settlementDate, const std::vector< LADate >& bondPaymentlDatesForIndexation );

    /* @brief   Function to calculate the start index to get the index for the first bond coupon that is not in the past
    *  @param [in]		settlementDate                      The bond settlement date
    *  @param [in]		bondPaymentlDatesForIndexation      A vector of all bond cashflow payment dates, these dates are assumed to be SORTED
	*  @Return          returns the index position of the active cashflow given a vector of dates. Will return the final index position if all cashflows are in the past
    */
    unsigned int getBondCashflowIndex( const LADate& searchDate, const std::vector< LADate >& bondPaymentlDatesForIndexation );

    /* @brief   Function to calculate the index of a bond's cashflow relative to the first non-past coupon
    *  @param [in]		firstActiveCashflowIndex            The first active coupon (payment) date index
    *  @param [in]		searchPaymentDate                   The search payment date
    *  @param [in]		bondPaymentlDatesForIndexation      A vector of all bond cashflow payment dates, these dates are assumed to be SORTED
	*  @Return          returns the index of the cashflow. The first future cashflow has index 0, the second future cashflow is 1 etc ....
    */
    unsigned int getBondRelativeCashflowIndex( unsigned int& firstActiveCashflowIndex, const LADate& searchPaymentDate, const std::vector< LADate >& bondPaymentlDatesForIndexation );


   /* @brief   Function to extract the first future date from a SORTED vector of historic and future cashflow dates using Bond Payment Dates for Indexation
    *  @param [in]		settlementDate                      The bond settlement date
    *  @param [in]		bondCashflowDates                   A vector of all bond cashflow dates, these dates are assumed to be SORTED. These dates can be the accrual start, end, payment or fixing dates
    *  @param [in]		bondPaymentlDatesForIndexation      A vector of all bond payment dates, these dates are our reference dates to determine if a bond coupon should be considered historic or in the future
	*  @Return          returns the first future cashflow date and index. Today is considered in the future. Will return the last date in the dates vector if all dates are historic and in the past
    */
    std::pair<LADate, size_t> getActiveCashflowDateAndIndex( const LADate& settlementDate, const std::vector< LADate >& bondCashflowDates, const std::vector< LADate >& bondPaymentlDatesForIndexation );

    /* @brief			Calculate the bond full periods from couponDate1 to couponDate2
    *  @param [in]		paymentDate1    Payment date one
    *  @param [in]		paymentDate2    Payment date two
    *  @param [in]		bondDaycount   Bond Daycount Basis
    *  @param [in]		bondAccrualFrequency   Bond accrual frequency
    *  @return			Full coupon period days
    */
    double getBondFullCouponPeriodDays(const LADate& paymentDate1, const LADate& paymentDate2, const DayCountEnum & bondDaycount, const FrequencyEnum & bondAccrualFrequency);

    /* @brief			Calculate the bond actual coupon period days from paymentDate1 to paymentDate2
    *  @param [in]		paymentDate1    Payment date one
    *  @param [in]		paymentDate2    Payment date two
    *  @param [in]		bondDaycount    Bond Daycount Basis
    *  @return			Actual coupon period days
    */
    double getBondActualCouponPeriodDays(const LADate& paymentDate1, const LADate& paymentDate2, const DayCountEnum & bondDaycount);

    /* @brief			Check if it is JGBBond with issue date earlier than the market convention change date
    *  @param [in]		bondCalculationType Bond calculation type
    *  @param [in]		issueDateStr        Issue date of the bond
    *  @return			True/false
    */
    bool isBondJGBIssuedPriorToMarketConventionChangeDate(const BondCalculationTypeEnum& bondCalculationType, const LAString& issueDateStr);

    /* @brief			Check if the bond calculation type is JGB (JapaneseGovenmentBond)
    *  @param [in]		bondCalculationType   Bond calculation type
    *  @return			True for JGB bonds
    */
    bool isJapaneseGovenmentBond(const BondCalculationTypeEnum& bondCalculationType);

    /* @brief			Check if the bond calculation type is Italian
    *  @param [in]		bondCalculationType   Bond calculation type
    *  @return			True for Italian bonds
    */
    bool isItalianGovenmentBond(const BondCalculationTypeEnum& bondCalculationType);

    /* @brief			Bond's notinoal exchange at maturity
    *  @param [in]		bondCalculationType     Bond calculation type
    *  @param [in]		effectiveDate           Bond effective date
    *  @param [in]		maturityDate            Bond maturity date
    *  @param [in]		notional                Bond face value
    *  @param [in]		issuePrice              Bond issue price 
    *  @param [in]		taxRate                 Bond tax rate
    *  @param [in]		paymentDates            Bond paymentDates excluding the upfront cashflow if there is one
    *  @return			Bond's notinoal exchange at maturity
    */
    double getBondNotionalExchangeAtMaturity(const BondCalculationTypeEnum& bondCalculationType, const LADate& effectiveDate, const LADate& maturityDate, double notional, double issuePrice, double taxRate, const std::vector<LADate>& paymentDates) ;

    /* @brief			Convert yield between frequencies
    *  @param [in]		fromYield     Input yield
    *  @param [in]		fromFreq      Frequency of input yield
    *  @param [in]		toFreq        The target yield frequency
    *  @return			yield of target frequency
    */
    double convertYield(double fromYield, const FrequencyEnum&  fromFreq, const FrequencyEnum& toFreq);

	double roundBondAccruedInterestFactor(const double& origAccruedInterestFactor, const BondCalculationTypeEnum& calculationType, const LADate& settlementDate);

    /* @brief			Function to format a raw bond price according to its quote convention
    *  @param [in]		rawBondPrice      Frequency of input yield
    *  @param [in]		quoteConvention   The target yield frequency
    *  @return			Formatted Bond Price
    */
    double formatBondPrice( const double& rawBondPrice, const BondQuoteConventionEnum& quoteConvention );

	void populateBondYieldParameters(BondYieldParameters& bondYieldParameters,
									const FrequencyEnum& couponFrequency,
									const FrequencyEnum& yieldFrequency,
									const BondCalculationTypeEnum& calculationType,
									const DayCountEnum&	accrualDayCount,
									const std::string& accrualCalendar,
									const std::string& exDividendTenor);

	bool excludeCouponInterest(const LADate& settlementDate, const LADate& exDividendDate);

	/* @brief			Calculate Fwd Bond's sum of coupon future value
	*  @param [in]		reinvestedCoupons		Coupons between settleDate and forwardSettleDate
	*  @return			Sum of paid coupon future values
	*/
	double calculateReinvestedCouponsFwdValue(const std::vector< BondFwdReinvestedCoupon >& reinvestedCoupons);

	/* @brief			Calculate Bond's forward dirtyPrice at forward settle date, from repo rate
	*  @param [in]		dirtyPrice			Bond dirty price at settle date
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		repoRate			Bond repo rate
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @param [in]		reinvestedCouponsFwdValue	Sum of paid voupons between settleDate and forwardSettleDate
	*  @return			Bond forward dirty price
	*/
	double calculateForwardDirtyPrice(const double& dirtyPrice, const LADate& settleDate, const LADate& forwardSettleDate, const double& repoRate, const DayCountEnum& repoDayCount, const double& reinvestedCouponsFwdValue);

	/* @brief			Calculate Bond's implied repo rate from bond's forward dirty price at settle date
	*  @param [in]		dirtyPrice			Bond dirty price at settle date
	*  @param [in]		forwardDirtyPrice	Bond's forward dirtyPrice at forward settle date
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @param [in]		reinvestedCoupons		Coupons between settleDate and forwardSettleDate
	*  @return			Bond implied repo rate
	*/
	double calculateRepoRate(const double& dirtyPrice, const double& forwardDirtyPrice, const LADate& settleDate, const LADate& forwardSettleDate, const DayCountEnum& repoDayCount, const std::vector< BondFwdReinvestedCoupon >& reinvestedCoupons);

	/* @brief			Get Bond's implied forward price from quoted future price
	*  @param [in]		futurePrice			Bond future dirty/price at future settle date
	*  @param [in]		conversionFactor	Bond's conversion factor
	*  @return			Bond forward dirty/clean price
	*/
	double toImpliedForwardCleanPrice(const double& futurePrice, const double& conversionFactor);

	/* @brief			Get the first full coupon days of the long start stub
	*  @param [in]		firstActivePaymentDate			Coupon payment date of the first active (non-past) cashflow
	*  @param [in]		firstPriorVirtualPaymentDate    The date prior to the first active coupon date by one full coupon period
	*  @param [in]		secondPriorVirtualPaymentDate   The date prior to the first active coupon date by two full coupon period
	*  @param [in]		bondCalculationType             Bond calculation type
	*  @param [in]		dayCount						Bond Daycount
	*  @param [in]		frequency						Bond frequency
	*  @return			First full coupon days of the long start stub
	*/
	double getLongStartStubFirstFullCouponDays(const LADate& firstActivePaymentDate, const LADate& firstPriorVirtualPaymentDate, const LADate& secondPriorVirtualPaymentDate, const BondCalculationTypeEnum& bondCalculationType, const DayCountEnum& dayCount, const FrequencyEnum& frequency);

}

