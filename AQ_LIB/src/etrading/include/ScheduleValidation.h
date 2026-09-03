/*
 * @brief			schedule validation methods used within validation_api
 * @Created:		29th June 2016
 * @Author:			Nicholas Burgess
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include <boost/lexical_cast.hpp>
#include "LabelValueBlock.h"
#include "CoreEnumerations.h"
#include "ScheduleParameters.h"
#include "EnrichedSchedule.h"

using etrading::LabelValueBlock;

namespace etrading
{
    /* @brief			Function to check if a date string is a date; used to distinguish between a date string and a tenor string
    * @param [in]		dateString	            Date in string format, can be a date or tenor
    * @output			Boolean to confirm if the date string is a date
    */
    bool isDate( const std::string& dateString );

    /* @brief			Function to check if a date string is a date tenor string; used to distinguish between a date string and a tenor string
    * @param [in]		dateString	            Date in string format, can be a date or tenor
    * @output			Boolean to confirm if the date string is a date tenor string
    */
    bool isDateTenor( const std::string& dateString );

    /* @brief			Validate a date string and convert it to LADate. Note as a market convention, maturity dates are NOT adjusted for holiday, so businessAdjustment and calendar are not required.
    * @param [in]		asOfDate		        The start date
    * @param [in]		dateString	            Date in string format, can be a date or tenor
    * @param [in]		businessDayAdjustment	The business day adjustment, will default to NO_CHANGE i.e. unadjusted
    * @param [in]		calendar	            Calendar string for business day adjustments, defaults to blank
    * @output			Date in LADate format
    */
    LADate validateDateOrTenor( const LADate& asOfDate, const LAString& dateOrTenor, const LAString businessDayAdjustment = LAString(), const LAString calendar = LAString() );

    /* @brief			helper method that validates swap cash flow related params and generate actual cash flows
    *  @param [in]		swapLVB			    A label value block of the swap label value block
    *  @param [in]		inputLVB			Name of the label value block
    *  @param [out]		fixedAccrualDates	Accrual dates schedule on fixed leg
    *  @param [out]		fixedPaymentDates	Payment dates schedule on fixed leg
    *  @param [out]		floatFixingDates	Fixing dates schedule on floating leg
    *  @param [out]		floatAccrualDates	Accrual dates schedule on floating leg
    *  @param [out]		floatPaymentDates	Payment dates schedule on floating leg
    *  @param [in]		isAssetSwap	        Flag to indicate if the swap is an asset swap
    */
    void validateAndGenerateSwapCashflows( const LabelValueBlock&	swapLVB,
                                           const std::string		inputLVB,
                                           DateVector&				fixedAccrualDates,
                                           DateVector&				fixedPaymentDates,
                                           DateVector&				floatFixingDates,
                                           DateVector&				floatAccrualDates,
                                           DateVector&				floatPaymentDates,
                                           const LAString&			fixingAdvanceOrArrears = LAString( "advance" ),
                                           const bool&              isAssetSwap = false );


    /* @brief			helper method that validates swap cash flow related params and generate actual cash flows
    *  @param [out]		fixedAccrualDates		                        Accrual dates schedule on fixed leg
    *  @param [out]		fixedPaymentDates		                        Payment dates schedule on fixed leg
    *  @param [out]		floatFixingDates		                        Fixing dates schedule on floating leg
    *  @param [out]		floatAccrualDates		                        Accrual dates schedule on floating leg
    *  @param [out]		floatPaymentDates		                        Payment dates schedule on floating leg
    *  @param [in]		effectDateString	                            The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturityDateString                              Swap maturity date or tenor
    *  @param [in]		fixedLegFreq		                            Fixed leg frequency
    *  @param [in]		fixedLegDayCount		                        Fixed leg day count convention
    *  @param [in]		fixedLegAccrualBusinessDayAdjustment		    Fixed leg accrual business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegAccrualCalendar	                        Fixed leg accrual calendar
    *  @param [in]		fixedLegPaymentBusinessDayAdjustment            Fixed leg payment business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegPaymentCalendar                         Fixed leg payment calendar
    *  @param [in]		fixedLegFirstStubDate	                        Fixed leg First stub Date
    *  @param [in]		fixedLegLastStubDate	                        Fixed leg Last stub Date
    *  @param [in]		fixedLegRollDayString                           Fixed leg rolling date (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
    *  @param [in]		fixedLegPayLag			                        Fixed leg payment date lag
    *  @param [in]		fixedLegStubType		                        Fixed leg Stub type
    *  @param [in]		floatLegFreq			                        Floating leg frequency
    *  @param [in]		floatLegDayCount		                        Floating leg day count convention
    *  @param [in]		fixedLegFixingBusinessDayAdjustment		        Fixed leg fixing business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegFixingCalendar	                        Fixed leg fixing calendar
    *  @param [in]		fixedLegAccrualBusinessDayAdjustment            Fixed leg accrual business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegAccrualCalendar                         Fixed leg accrual calendar
    *  @param [in]		fixedLegPaymentBusinessDayAdjustment		    Fixed leg payment business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegPaymentCalendar	                        Fixed leg payment calendar
    *  @param [in]		floatLegFirstStub		                        Float leg Front stub Date
    *  @param [in]		floatLegLastStub		                        Float leg Last stub Date
    *  @param [in]		floatLegRollDayString                           Floating leg rolling day (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
    *  @param [in]		floatLegFixLag			                        Floating leg fixing day lag
    *  @param [in]		floatLegFistFix		                            Floating leg first fixing
    *  @param [in]		floatLegLastFix		                            Floating leg last fixing
    *  @param [in]		floatLegPayLag			                        Floating leg payment date lag
    *  @param [in]		floatLegStubType		                        Floating leg stub type
    *  @param [in]		fixedLegPaymentFreq		                        Fixed leg payment frequency
    *  @param [in]		floatLegPaymentFreq		                        Floating leg payment frequency
    *  @param [in]		floatLegFixingAdvanceOrArrears		            Floating leg fixingAdvanceOrArrears
    *  @param [in]		isAssetSwap	                                    Flag to indicate if the swap is an asset swap
    *  @param [in]		issueDate	                                    Asset Swap Bond Issue Date
    */
    void validateAndGenerateSwapCashflows( DateVector&          fixedAccrualDates,
                                           DateVector&          fixedPaymentDates,
                                           DateVector&          floatFixingDates,
                                           DateVector&          floatAccrualDates,
                                           DateVector&          floatPaymentDates,
                                           const LAString&      effectiveDateString,
                                           const LAString&      maturityDateString,
                                           const LAString&      fixedLegFreq,
                                           const LAString&      fixedLegDayCount,
                                           const LAString&      fixedLegAccrualBusinessDayAdjustment,
                                           const LAString&      fixedLegAccrualCalendar,
                                           const LAString&      fixedLegPaymentBusinessDayAdjustment,
                                           const LAString&      fixedLegPaymentCalendar,
                                           const LAString&      fixedLegFirstStubDate,
                                           const LAString&      fixedLegLastStubDate,
                                           const LAString&      fixedLegRollDayString,
                                           const LAString&      fixedLegPayLag,
                                           const LAString&      fixedLegStubType,
                                           const LAString&      floatLegFreq,
                                           const LAString&      floatLegDayCount,
                                           const LAString&      floatLegFixingBusinessDayAdjustment,
                                           const LAString&      floatLegFixingCalendar,
                                           const LAString&      floatLegAccrualBusinessDayAdjustment,
                                           const LAString&      floatLegAccrualCalendar,
                                           const LAString&      floatLegPaymentBusinessDayAdjustment,
                                           const LAString&      floatLegPaymentCalendar,
                                           const LAString&      floatLegFirstStubDate,
                                           const LAString&      floatLegLastStubDate,
                                           const LAString&      floatLegRollDayString,
                                           const LAString&      floatLegFixLag,
                                           double               floatLegFirstFix,
                                           double               floatlegLastFix,
                                           const LAString&      floatLegPayLag,
                                           const LAString&      floatLegStubType,
                                           const LAString&		fixedLegPaymentFreq = LAString(),
                                           const LAString&		floatLegPaymentFreq = LAString(),
                                           const LAString&		fixingAdvanceOrArrears = LAString( "advance" ),
                                           const bool           isAssetSwap = false,
                                           LADate               issueDate = LADate() );

    /* @brief			helper method that validates fixed leg cash flow related params and generate actual cash flows
    *  @param [out]		fixedAccrualDates		                        Accrual dates schedule on fixed leg
    *  @param [out]		fixedPaymentDates		                        Payment dates schedule on fixed leg
    *  @param [in]		effectDateString	                            The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturityDateString                              Swap maturity date or tenor
    *  @param [in]		fixedLegFreq		                            Fixed leg frequency
    *  @param [in]		fixedLegDayCount		                        Fixed leg day count convention
    *  @param [in]		fixedLegAccrualBusinessDayAdjustment		    Fixed leg accrual business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegAccrualCalendar	                        Fixed leg accrual calendar
    *  @param [in]		fixedLegPaymentBusinessDayAdjustment            Fixed leg payment business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegPaymentCalendar                         Fixed leg payment calendar
    *  @param [in]		fixedLegFirstStubDate	                        Fixed leg First stub Date
    *  @param [in]		fixedLegLastStubDate	                        Fixed leg Last stub Date
    *  @param [in]		fixedLegRollDayString                           Fixed leg rolling date (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
    *  @param [in]		fixedLegPayLag			                        Fixed leg payment date lag
    *  @param [in]		fixedLegStubType		                        Fixed leg Stub type
    *  @param [in]		fixedLegPaymentFreq		                        Fixed leg payment frequency
    *  @param [in]		isAssetSwap	                                    Flag to indicate if the swap is an asset swap
    *  @param [in]		issueDate	                                    Asset Swap Bond Issue Date
    */
    void validateAndGenerateFixedLegCashflows( DateVector&          fixedAccrualDates,
            DateVector&          fixedPaymentDates,
            const LAString&      effectiveDateString,
            const LAString&      maturityDateString,
            const LAString&      fixedLegFreq,
            const LAString&      fixedLegDayCount,
            const LAString&      fixedLegAccrualBusinessDayAdjustment,
            const LAString&      fixedLegAccrualCalendar,
            const LAString&      fixedLegPaymentBusinessDayAdjustment,
            const LAString&      fixedLegPaymentCalendar,
            const LAString&      fixedLegFirstStubDate,
            const LAString&      fixedLegLastStubDate,
            const LAString&      fixedLegRollDayString,
            const LAString&      fixedLegPayLag,
            const LAString&      fixedLegStubType,
            const LAString&      fixedLegPaymentFreq,
            const bool           isAssetSwap = false,
            LADate               issueDate = LADate() );

    /* @brief			helper method that validates float leg cash flow related params and generate actual cash flows
    *  @param [out]		floatFixingDates		                        Fixing dates schedule on floating leg
    *  @param [out]		floatAccrualDates		                        Accrual dates schedule on floating leg
    *  @param [out]		floatPaymentDates		                        Payment dates schedule on floating leg
    *  @param [in]		effectDateString	                            The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturityDateString                              Swap maturity date or tenor
    *  @param [in]		floatLegFreq			                        Floating leg frequency
    *  @param [in]		floatLegDayCount		                        Floating leg day count convention
    *  @param [in]		fixedLegFixingBusinessDayAdjustment		        Fixed leg fixing business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegFixingCalendar	                        Fixed leg fixing calendar
    *  @param [in]		fixedLegAccrualBusinessDayAdjustment            Fixed leg accrual business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegAccrualCalendar                         Fixed leg accrual calendar
    *  @param [in]		fixedLegPaymentBusinessDayAdjustment		    Fixed leg payment business day adjustment e.g. Modified Following
    *  @param [in]		fixedLegPaymentCalendar	                        Fixed leg payment calendar
    *  @param [in]		floatLegFirstStub		                        Float leg Front stub Date
    *  @param [in]		floatLegLastStub		                        Float leg Last stub Date
    *  @param [in]		floatLegRollDayString                           Floating leg rolling day (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
    *  @param [in]		floatLegFixLag			                        Floating leg fixing day lag
    *  @param [in]		floatLegFistFix		                            Floating leg first fixing
    *  @param [in]		floatLegLastFix		                            Floating leg last fixing
    *  @param [in]		floatLegPayLag			                        Floating leg payment date lag
    *  @param [in]		floatLegStubType		                        Floating leg stub type
    *  @param [in]		floatLegPaymentFreq		                        Floating leg payment frequency
    *  @param [in]		floatLegFixingAdvanceOrArrears		            Floating leg fixingAdvanceOrArrears
    *  @param [in]      removeExtraDay									True to remove the extra fixing date and payment date. Default to False for backward compatibility, as all the core functions expect fixing dates & payment dates having the same size as accrual days
    */
    void validateAndGenerateFloatLegCashflows( DateVector&          floatFixingDates,
            DateVector&          floatAccrualDates,
            DateVector&          floatPaymentDates,
            const LAString&      effectiveDateString,
            const LAString&      maturityDateString,
            const LAString&      floatLegFreq,
            const LAString&      floatLegDayCount,
            const LAString&      floatLegFixingBusinessDayAdjustment,
            const LAString&      floatLegFixingCalendar,
            const LAString&      floatLegAccrualBusinessDayAdjustment,
            const LAString&      floatLegAccrualCalendar,
            const LAString&      floatLegPaymentBusinessDayAdjustment,
            const LAString&      floatLegPaymentCalendar,
            const LAString&      floatLegFirstStubDate,
            const LAString&      floatLegLastStubDate,
            const LAString&      floatLegRollDayString,
            const LAString&      floatLegFixLag,
            double               floatLegFirstFix,
            double               floatlegLastFix,
            const LAString&      floatLegPayLag,
            const LAString&      floatLegStubType,
            const LAString&		floatLegPaymentFreq = LAString(),
            const LAString&		floatLegFixingAdvanceOrArrears = LAString( "advance" ),
            bool				    removeExtraDay = false );

    /* @brief			helper method that validates and generates accrual start and end dates from a single combined vector of accrual dates
    *  @param [out]		accrualStartDates		    Accrual Start Dates
    *  @param [out]		accrualEndDates		        Accrual End Dates
    *  @param [in]		combinedAccrualDates        Combined Accrual Dates
    */
    void validateAndGenerateAccrualStartAndEndDates( DateVector&          accrualStartDates,
            DateVector&          accrualEndDates,
            const DateVector&    combinedAccrualDates );

    /* @brief			Get isStartRoll value and populate stub dates from stubType
    * @param [in]		stubType		Stub type
    * @param [in]		firstStubDtPtr	Pointer to first stub date
    * @param [in]		lastStubDtPtr	Pointer to last stub date
    * @param [in]		startDate		Start date
    * @param [in]		endDate			End date
    * @param [in]		frequency		Frequency
    * @output			isStartRoll		True when it's ShortEnd (SE) or LongEnd (LE), otherwise False
    */
    bool isStartRollAndPopulateStubDatesFromStubType( const StubTypeEnum& stubType,
            const LADate* firstStubDtPtr,
            const LADate* lastStubDtPtr,
            const LADate& startDate,
            const LADate& endDate,
            const LAString& frequency );

    /* @brief			Check the input dates are in ascending order
    *  @param [in]		dates	                   Input dates
    *  @param [in]		allowEqual		           True to allow equal
    *  @param [in]		dateName                   Input date names, for error message
    */
    void checkDatesInAscendingOrder( const DateVector& dates, bool allowEqual, const LAString& dateName );


	/* @brief			Helper method that generates accrual dates and payment dates
	*					Note: In this version, the effectiveDate and maturityDate are provided as actual dates.
	*  @param [out]		accrualDates		                   Accrual dates schedule
	*  @param [out]		paymentDates		                   Payment dates schedule
	*  @param [in]		effectDate	                           Effective start date of the swap, ie, base date + spot date
	*  @param [in]		maturityDate                           Maturity date
	*  @param [in]		accrualFreq		                       AccrualFreq frequency
	*  @param [in]		accrualBusinessDayAdjustment		   Accrual business day adjustment e.g. Modified Following
	*  @param [in]		accrualCalendar	                       Accrual calendar
	*  @param [in]		paymentFreq		                       Payment frequency
	*  @param [in]		paymentBusinessDayAdjustment           Payment business day adjustment e.g. Modified Following
	*  @param [in]		paymentCalendar                        Payment calendar
	*  @param [in]		paymentLag			                   Payment lag
	*  @param [in]		stubType		                       Stub type
	*  @param [in]		firstStubDate	                       First stub Date
	*  @param [in]		lastStubDate	                       Last stub Date
	*  @param [in]		rollDayString                          Rolling date (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
	*/
	
	void validateAndGenerateAccrualAndPaymentSchedules( DateVector&          accrualDates,
														DateVector&          paymentDates,
														const LADate&        effectiveDate,
														const LADate&        maturityDate,
														const LAString&      accrualFreq,
														const LAString&      accrualBusinessDayAdjustment,
														const LAString&      accrualCalendar,
														const LAString&      paymentFreq,
														const LAString&      paymentBusinessDayAdjustment,
														const LAString&      paymentCalendar,
														const LAString&      paymentLag,
														const LAString&      stubType,
														const LAString&      firstStubDate,
														const LAString&      lastStubDate,
														const LAString&      rollDayString,
														const LAString&		 fixingAdvanceOrArrears );
														

    	/* @brief			Helper method that generates accrual dates and payment dates
		*					Note: This version accepts the effectiveDate and PaymentDate as strings.
        *  @param [out]		accrualDates		                   Accrual dates schedule
        *  @param [out]		paymentDates		                   Payment dates schedule
        *  @param [in]		effectDateString	                   Effective start date of the swap, ie, base date + spot date
        *  @param [in]		maturityDateString                     Maturity date or tenor
        *  @param [in]		accrualFreq		                       AccrualFreq frequency
    	*  @param [in]		accrualBusinessDayAdjustment		   Accrual business day adjustment e.g. Modified Following
        *  @param [in]		accrualCalendar	                       Accrual calendar
    	*  @param [in]		paymentFreq		                       Payment frequency
        *  @param [in]		paymentBusinessDayAdjustment           Payment business day adjustment e.g. Modified Following
        *  @param [in]		paymentCalendar                        Payment calendar
        *  @param [in]		paymentLag			                   Payment lag
        *  @param [in]		stubType		                       Stub type
    	*  @param [in]		firstStubDate	                       First stub Date
        *  @param [in]		lastStubDate	                       Last stub Date
        *  @param [in]		rollDayString                          Rolling date (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
        */
    void validateAndGenerateAccrualAndPaymentSchedules( DateVector&          accrualDates,
                                                        DateVector&          paymentDates,
                                                        const LAString&      effectiveDateString,
                                                        const LAString&      maturityDateString,
                                                        const LAString&      accrualFreq,
                                                        const LAString&      accrualBusinessDayAdjustment,
                                                        const LAString&      accrualCalendar,
                                                        const LAString&      paymentFreq,
                                                        const LAString&      paymentBusinessDayAdjustment,
                                                        const LAString&      paymentCalendar,
                                                        const LAString&      paymentLag,
                                                        const LAString&      stubType,
                                                        const LAString&      firstStubDate,
                                                        const LAString&      lastStubDate,
                                                        const LAString&      rollDayString,
                                                        const LAString&		 fixingAdvanceOrArrears );


    /* @brief		Validates and genertate fixing schedule
    * @param [in]	accrualDates					Accrual schedue
    * @param [in]	fixingBusinessDayAdjustment     Fixing business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    * @param [in]	fixingCalendar					Fixing calendar
    * @param [in]	fixingLag						Fixing lag
    * @param [in]	fixingAdvanceOrArrears			Flag to indicate the fixing is advance or arrears
    * @return the fixing dates
    */
    DateVector validateAndGenerateFixingSchedule( const DateVector&    accrualDates,
            const LAString&      fixingBusinessDayAdjustment,
            const LAString&      fixingCalendar,
            const LAString&      fixingLag,
            const LAString&		 fixingAdvanceOrArrears,
			const bool			 includeLastExtraFixingDate = false);

    /* @brief			Transform the accrual start and end dates to a single combined vector of accrual dates
    *  @param [in]		accrualStartDates		    Accrual Start Dates
    *  @param [in]		accrualEndDates		        Accrual End Dates
    *  @return 		combinedAccrualDates        Combined Accrual Dates
    */
    std::vector<LADate> combineAccrualStartAndEndDates( const std::vector<LADate>& accrualStartDates, const std::vector<LADate>& accrualEndDates );

    /* @brief Transform dates from LADate format to double format
    *  @param [in] dateVec dates in LADate format
    *  @Return     dates in double format
    */
    DoubleVector fromDateToDoubleVector(const DateVector& dateVec );

    /* @brief Transform dates from LADate format to double format
    * 
    *  @param [in] date date in LADate format
    *  @Return     date in double format
    */
	double fromLADateToDouble(const LADate& date);

    /* @brief Transform dates from double format to LADate format
    *  @param [in] doubleVec dates in double format
    *  @Return     dates in LADate format
    */
    DateVector fromDoubleToDateVector(const DoubleVector& doubleVec );

    /* @brief Transpose a double matrix
    *  @param [in] input                The input matrix
    *  @Return     a transposed matrix
    */
    DoubleMatrix transpose( const DoubleMatrix& input );

	/* @brief Calculate days between start/end dates 
    * 
    *  @param [in] accrualStart		Accrual start date
    *  @param [in] accrualEnd		Accrual end date
    *  @Return     days
    */
	int getDays(const LADate& accrualStart, const LADate& accrualEnd);

	/* @brief			Number of long Feb (feb29) between fromDate and toDate
	*  @param [in]		fromDate	    From Date
	*  @param [in]		toDate			To Date
	*  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
	*  @return			Number of long Feb (feb29)
	*/
	int numberOfLongFeb(const LADate& fromDate, const LADate& toDate, bool includeLast);

    /* @brief			Year fraction between fromDate and toDate
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		dayCount		Day count convention
    *  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
    *  @return			Year fraction between fromDate and toDate
    */
    double getYearFraction( const LADate& fromDate, const LADate& toDate, const DayCountEnum& dayCount, bool includeLast = false);

	/* @brief Transpose a matrix of elements
    *  @param [in] input                The input matrix
    *  @Return     a transpose matrix
    */
    template<typename T>
	std::vector< std::vector<T> > transpose(const std::vector< std::vector<T> >& input );

	/* @brief Transpose a matrix of elements
	*  @param [in] input                The input matrix
	*  @param [in] padIfInputIsRagged   If the input is a ragged matrix with different number of columns, and this flag set to true, pad the matrix to form a rectangular matrix.
	*  @Return     a transpose matrix
	*/
	template<typename T>
	std::vector< std::vector<T> > transpose( const std::vector< std::vector<T> >& input, const bool padIfInputIsRagged );

    /* @brief			Helper method to get the RollConvenction string from rollDayString
    *  @param [in]		startDate	               Start date
    *  @param [in]		endDate                    end Date
    *  @param [in]		rollDayString              Rolling date (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
    */
    LAString getRollConvection(const LADate& startDate,
                               const LADate& endDate,
                               const LAString& rollDayString);

	/* @brief			Helper method to get the RollConvenction string from rollDayString
	*  @param [in]		rollDayString              Rolling date (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
	*/
	LAString getRollConvection(const LAString& rollDayString);


    /* @brief			Calculate an unadjusted date from a tenor
	* @param [in]		startDate		        start date
    * @param [in]		tenorAdjustment			Tenor Adjustment
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
    * @output			Returns the adjusted date
	*/
    LADate getUnadjustedDateFromTenor( const LADate& startDate,
                                       const LAString& tenorAdjustment,
                                       const LAString& rollConvention );

    /* @brief			Calculate an unadjusted date from a tenor
	* @param [in]		startDate		        start date
    * @param [in]		tenorAdjustment			Tenor Adjustment
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
    * @param [in]		useRollConvention	    boolean to toggle if the roll convention should be used
	* @output			Returns the adjusted date
	*/
    LADate getUnadjustedDateFromTenor( const LADate& startDate,
                                       const LAString& tenorAdjustment,
                                       const LAString& rollConvention,
                                       const bool useRollConvention );

    /* @brief			Calculate an adjusted date from an unadjusted date
	* @param [in]		unadjustedDate		    Unadjusted date
    * @param [in]		tenorAdjustment			Tenor Adjustment
    * @param [in]		busDayAdj			    Swap business date adjustment convention
	* @param [in]		calendar			    Swap calendar
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
	* @output			Returns the adjusted date
	*/
    LADate getAdjustedDate( const LADate& unadjustedDate,
                            const LAString& tenorAdjustment,
                            const LAString& busDayAdj,
                            const LAString& calendar,
                            const LAString& rollConvention );

    /* @brief			Calculate an adjusted date from an unadjusted date
	* @param [in]		unadjustedDate		    Unadjusted date
    * @param [in]		tenorAdjustment			Tenor Adjustment
    * @param [in]		busDayAdj			    Swap business date adjustment convention
	* @param [in]		calendar			    Swap calendar
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
    * @param [in]		useRollConvention	    boolean to toggle if the roll convention should be used
	* @output			Returns the adjusted date
	*/
    LADate getAdjustedDate( const LADate& unadjustedDate,
                            const LAString& tenorAdjustment,
                            const LAString& busDayAdj,
                            const LAString& calendar,
                            const LAString& rollConvention,
                            const bool useRollConvention );

	/* @brief			Validate if swap has regular date schedule without stub coupons.
	* @param [in]		swapStart			    Swap start date
	* @param [in]		swapMaturity            Swap end date. Tenors are typically adjusted and end dates are not
	* @param [in]		isMaturityAdjusted      Swap end date business day adjusted. Maturities derived from Tenors are adjusted, whereas explicit maturity dates are unadjusted
	* @param [in]		frequency			    Swap floating frequency
	* @param [in]		busDayAdj			    Swap business date adjustment convention
	* @param [in]		calendar			    Swap calendar
	* @param [in]		rollDay				    Roll day
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
	* @output			Returns TRUE if the swap schedule is regular (with no stub) and FALSE otherwise
	*/
	bool isRegularSwapSchedule( const LADate& swapStart,
		                        const LADate& swapMaturity,
		                        bool isMaturityAdjusted, // Explicit dates are unadjusted for holidays, whereas Tenors are adjusted
		                        const LAString& frequency,
		                        const LAString& busDayAdj,
		                        const LAString& calendar,
		                        int rollDay,
		                        const LAString& rollConvention = "" );

	
	/* @brief			Populate schedule outputs based on the schedule input
	*  @param [in]		schParams	               Schedule input
	*  @param [in]		checkStub	               True to check if the schedule has stub. 
	*  @Return			Schedule output
	*/
	EnrichedSchedule populateScheduleDates(const ScheduleParameters& schParams, const bool& checkStub);

	/* @brief			Check if the schedule has stub
	*  @param [in]		schParams	               Schedule input
	*  @Return			True to indicate the schedule has stub
	*/
	bool checkScheduleHasStub(const ScheduleParameters& schParams);

	bool isFrontStub(const StubTypeEnum& stubType);

	DateVector calculateAndPopulateFixingEndDates(const DateVector& accrualEndDates, const std::string& fixingAdvanceOrArrears, const BusinessDayAdjustmentEnum& fixingbusinessDayAdj, const std::string& fixingCalendar, const std::string& fixLag);
	
}
