#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace etrading
{
    /* @brief				validate OIS specific parameters
    * @param [in,out]		slidingRule			Sliding rule required by the underlying compounding method
    * @param [in,out]		compoundingMethod	OIS compounding method
    * @param [in]			eomRoll				Do we do EOM rolling?
    * @output				A boolean indicating the fixed leg direction
    */
    void validateOISParameters( LAString& slidingRule, LAString& compoundingMethod, bool eomRoll );

    /* @brief			Validate maturity string and convert it to LADate. As a market convention, maturity date is NOT adjusted for holiday, so businessAdjustment and calendar are not required.
    * @param [in]		effectiveDate		The start date
    * @param [in]		maturityDateStr		Maturity Date in string format, can be a date or tenor
    * @output			maturity date in LADate format
    */
    LADate validateMaturityDate( const LADate& effectiveDate, const LAString& maturityDateStr );		

    /* @brief			Validate maturity string and convert it to LADate, given the effective date as a string
    * @param [in]		effectiveDateStr	The start date as a string
    * @param [in]		maturityDateStr		Maturity Date in string format, can be a date or tenor
    * @output			maturity date in LADate format
    */
    LADate validateMaturityDate( const LAString& effectiveDateStr, const LAString& maturityDateStr );		

    /* @brief			Validate maturity string and convert it to LAString, given the effective date as a string
    * @param [in]		effectiveDateStr	The start date as a string
    * @param [in]		maturityDateStr		Maturity Date in string format, can be a date or tenor
    * @output			maturity date in LAString format
    */
    LAString validateMaturityDateString( const LAString& effectiveDateStr, const LAString& maturityDateStr );

    /* @brief			validate the payRec flag for swap
    * @param [in]		payRec		A string representing the fixed leg direction
    * @output			A boolean indicating the fixed leg direction
    */
    bool validateSwapPayRecFlag( const LAString& payRec );

	/* @brief			Indicator for payer or receiver
    * @param [in]		payRec		A string representing the fixed leg direction
    * @output			Return -1 for payer, 1 for receiver
    */
	int getIndicatorFromPayRec(const PayReceiveEnum& payRec );

    /* @brief			Validate if a Swap rollDay is a day of the month or a roll convention i.e. IMM, EOM.
    *                   The function will populate and synchronize the multiple parameters which support
    *                   the chosen roll day or convention
    *
    *  @param [in]		rollDayString       A string representing the roll day or convention
    *  @param [in]		startDate           The Swap accrual start date ( Effective Date )
    *  @param [in]		endDate             The Swap accrual end date ( Maturity Date )
    *  @param [out]		rollDay             An integer *** pointer *** representing a day of the month; 1-31
    *  @param [out]     rollConvention      A string *** pointer *** representing the roll convention; IMM or EOM
    *  @param [out]     isEndOfMonthRoll    Are coupons rolling on the end of the month? True or False
    *  @param [out]     isStartRoll         Are we rolling forwards from the start? True = coupons roll from the start date i.e. ShortEnd stub, False = coupons roll from the end date i.e. ShortStart stub
    */
    void validateAndPopulateRollDayConventions( const LAString& rollDayString, const LADate& startDate, const LADate& endDate, int** rollDay, LAString** rollConvention, bool& isEndOfMonthRoll, bool& isStartRoll );

    /* @brief			Check if float fixing should be use
    *  @param [in]		floatFixing	 Float leg's first/last fixing
    *  @return			True if the fixing is not zero
    */
    bool useFloatFixing( double floatFixing );


	/* @brief			Round to the zero decimal based on currency 
    *  @param [in]		val	        Value 
    *  @param [in]		currency	Currency
    *  @return			rounded value
    */
    double roundToNearest(double val, const CCY& currency);

    /* @brief			Round the value to the given decimal  
    *  @param [in]		val	        Value 
    *  @param [in]		decimal     decimal
    *  @return			rounded value
    */
    double roundToDecimal(double val, int decimal);

    /* @brief			Truncate the value to the given decimal  
    *  @param [in]		val	        Value 
    *  @param [in]		decimal	    decimal
    *  @return			truncated value
    */
    double truncateToDecimal(double val, int decimal);

    /* @brief			Validate two swap legs' currecies matching the isXccySwap flag
    * @param [in]		isXccySwap  True to indicate xccy swap
    * @param [in]		leg1Currency  Leg1 currency
    * @param [in]		leg2Currency  Leg2 currency
    */
    void validateSwapCurrency(bool isXccySwap, const CCY& leg1Currency, const CCY& leg2Currency);

    /* @brief			Validate two swap legs' currecies matching the isXccySwap flag
    * @param [in]		isXccySwap  True to indicate xccy swap
    * @param [in]		leg1LVB  Leg1 label value block
    * @param [in]		leg2LVB  Leg2 label value block
    */
    void validateSwapCurrency(bool isXccySwap, const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB);

	/* @brief			Validate if maturity string is in date format or tenor format
    * @param [in]		maturtiyStr			Maturity in string format
    * @output			TRUE for tenor format
    */
	bool isMaturityDateTenor(const LAString& maturityStr);
}


