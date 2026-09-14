#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "RollConventionValidation.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "CommonConstants.h"
#include "DateUtilities.h"
#include "BondEnumerations.h"
#include "Variant.h"
#include "ExceptionMacros.h"

// LA Includes
#include "AQLStaticData.h"
#include "AQLDateHelpers.h"
#include "AQLDateScheduleHelpers.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataInterpolation.h"

#include <cctype>
#include <utility>
#include <boost/date_time/gregorian/greg_date.hpp>

namespace etrading
{
    /* @brief			Function to check if a date string is a date; used to distinguish between a date string and a tenor string
    * @param [in]		dateString	            Date in string format, can be a date or tenor
    * @output			Boolean to confirm if the date string is a date
    */
    bool isDate( const std::string& dateString )
    {
        bool isDateFormat = true;

        try
        {
            auto result = stringToDate( dateString.c_str(), "#Error: Invalid Date String" );
        }
        catch ( AQLCoreInvalidData& )
        {
            isDateFormat = false;
        }

        return isDateFormat;
    }
    
    /* @brief			Function to check if a date string is a date tenor string; used to distinguish between a date string and a tenor string
    * @param [in]		dateString	            Date in string format, can be a date or tenor
    * @output			Boolean to confirm if the date string is a date tenor string
    */
    bool isDateTenor( const std::string& dateString )
    {
        // This is a primative test: We only check that date string can't be a date by checking string size against the minimum size of AQLDate.
        // We also check if the final character of the string is consistent with a tenor string ending with (D)ay, (W)eek, (M)onth, (Y)ear.

        const unsigned int dateStringSize = dateString.size();
        AQ_REQUIRE( dateStringSize > 0, "Invalid Date: Empty Date Parameter")

        bool isTenorFormat = false;

        // (D)ay, (W)eek, (M)onth, (Y)ear
        std::set<char> tenorSuffix = { 'D', 'W', 'M', 'Y' };
        auto iter = tenorSuffix.find( dateString[ dateStringSize-1 ] );
        
        if ( iter != tenorSuffix.end() )
        {
            isTenorFormat = true;
        }

        return isTenorFormat;
    }

    /* @brief			Validate a date string and convert it to AQLDate. Note as a market convention, maturity dates are NOT adjusted for holiday, so businessAdjustment and calendar are not required.
    * @param [in]		asOfDate		        The start date
    * @param [in]		dateOrTenor	            Date in string format, can be a date or tenor
    * @param [in]		businessDayAdjustment	The business day adjustment, will default to NO_CHANGE i.e. unadjusted
    * @param [in]		calendar	            Calendar string for business day adjustments, defaults to blank
    * @output			Date in AQLDate format
    */
    AQLDate validateDateOrTenor( const AQLDate& asOfDate, const AQLString& dateOrTenor, const AQLString businessDayAdjustment, const AQLString calendar )
    {
        AQLDate resultDate;
        bool isInDateFormat = true;

        try
        {
            resultDate = stringToDate( dateOrTenor, "#Error: Invalid Date String" );
        }
        catch ( AQLCoreInvalidData& )
        {
            isInDateFormat = false;
        }

        // Convert Date Tenor String to a Date
        if( !isInDateFormat )
        {
            // DateString is in Tenor Format
            AQ_REQUIRE( AQLDateScheduleHelpers::isValidDate( asOfDate ), "Invalid Date: Unable to convert DateTenor to a date" )
            resultDate = AQLDateScheduleHelpers::getDate( asOfDate, dateOrTenor, businessDayAdjustment, calendar );
        }

        AQ_REQUIRE( AQLDateScheduleHelpers::isValidDate( resultDate ), "Invalid Date " + dateOrTenor )
        return resultDate;
    }

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
										   const AQLString&			fixingAdvanceOrArrears,
                                           const bool&              isAssetSwap )
    {
        AQLDate issueDate = AQLDate();
        if ( isAssetSwap )
        {
            issueDate = swapLVB.getCompulsoryValueAsDate( ASSET_SWAP_KEY::ISSUE_DATE, inputLVB );
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Fixed Leg Parameters
        /////////////////////////////////////////////////////////////////////////////////////
        AQLString fixedLegFreq							= swapLVB.getCompulsoryValueAsAQLString(    IRS_KEY::FIXED_FREQUENCY,                       inputLVB                     );
        AQLString fixedDayCount			                = swapLVB.getCompulsoryValueAsAQLString(    IRS_KEY::FIXED_DAYCOUNT,                        inputLVB                     );
        AQLString fixedBusinessDayAdjustment             = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_BUSINESSDAYADJUSTMENT                                         );
        AQLString fixedCalendar	                        = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_CALENDAR                                                      );
        AQLString fixedLegAccrualBusinessDayAdjustment   = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_ACCRUALBUSINESSDAYADJUSTMENT,    fixedBusinessDayAdjustment   );
        AQLString fixedLegAccrualCalendar	            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_ACCRUALCALENDAR,                 fixedCalendar                );
        AQLString fixedLegPaymentBusinessDayAdjustment   = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_PAYMENTBUSINESSDAYADJUSTMENT,    fixedBusinessDayAdjustment   );
        AQLString fixedLegPaymentCalendar	            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_PAYMENTCALENDAR,                 fixedCalendar                );
        AQLString fixedLegPaymentFreq					= swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_PAYMENTFREQUENCY,                fixedLegFreq                 );
        AQLString fixedLegFirstStubDate		            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_FIRSTSTUBDATE                                                 );
        AQLString fixedLegLastStubDate		            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_LASTSTUBDATE                                                  );
        AQLString fixedLegRollDayString		            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_ROLLDAY                                                       );
        AQLString fixedLegPayLag							= swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_PAYMENTLAG,                      "0D"                         );
        AQLString fixedLegStubType			            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_STUBTYPE                                                      );

        validateStringEmptiness( fixedLegAccrualBusinessDayAdjustment,  "#Error: Fixed leg 'Accrual Business Day Adjustment' must be specified." );
        validateStringEmptiness( fixedLegPaymentBusinessDayAdjustment, "#Error: Fixed leg 'Payment Business Day Adjustment' must be specified." );
        validateStringEmptiness( fixedLegAccrualCalendar,               "#Error: Fixed leg 'Accrual Calendar' must be specified." );
        validateStringEmptiness( fixedLegPaymentCalendar,               "#Error: Fixed leg 'Payment Calendar' must be specified." );

        AQLDate effectiveDate	         = swapLVB.getCompulsoryValueAsDate( IRS_KEY::EFFECTIVE_DATE, inputLVB );
        AQLString maturityDateString      = swapLVB.getCompulsoryValueAsAQLString( IRS_KEY::MATURITY_DATE, inputLVB );
        AQLDate maturityDate	             = validateMaturityDate( effectiveDate, maturityDateString);

        AQ_THROW_IF( maturityDate < effectiveDate, "The swap maturity date cannot be before the swap start date" );


        AQLString fixedPayLag( fixedLegPayLag );
        if ( fixedPayLag.size() == 0 )
        {
            fixedPayLag = AQLString( "0D" );
        }

        // Set-Up First and Last Stub Parameters
        AQLDate* fixedAccrualFirstOddDate    = NULL;
        AQLDate* fixedAccrualLastOddDate     = NULL;
        AQLDate  fixedAccrualTempFirst;
        AQLDate  fixedAccrualTempLast;

        if( fixedLegFirstStubDate.size() != 0 && fixedLegFirstStubDate != "0")
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( fixedLegStubType.size() != 0 && AQLString( fixedLegStubType ).toUpper() != "NONE", "Fixed Leg cannot have both the StubType and First- or LastStubDate specified." );

            fixedAccrualTempFirst = stringToDate( fixedLegFirstStubDate, "#Error: Invalid fixed leg 'FirstStubDate'." );
            fixedAccrualFirstOddDate = &fixedAccrualTempFirst;
        }

        if( fixedLegLastStubDate.size() != 0 && fixedLegLastStubDate != "0")
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( fixedLegStubType.size() != 0 && ( AQLString( fixedLegStubType ).toUpper() ) != "NONE", "Fixed Leg cannot have both the StubType and First- or LastStubDate specified." );

            fixedAccrualTempLast = stringToDate( fixedLegLastStubDate, "#Error: Invalid fixed leg 'LastStubDate'." );
            fixedAccrualLastOddDate = & fixedAccrualTempLast;
        }

        // Get the fixed leg stub type i.e. ShortStart, LongStart, Short End or Long End
        AQLString* fixedStubType = NULL;
        if( fixedLegStubType.size() != 0 )
        {
            fixedStubType = const_cast<AQLString*>( &fixedLegStubType );
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Float Leg Parameters
        /////////////////////////////////////////////////////////////////////////////////////
        AQLString floatLegFreq			                = swapLVB.getCompulsoryValueAsAQLString(    IRS_KEY::FLOAT_FREQUENCY,                       inputLVB                     );
        AQLString floatLegDayCount			            = swapLVB.getCompulsoryValueAsAQLString(    IRS_KEY::FLOAT_DAYCOUNT,                        inputLVB                     );
        AQLString floatBusinessDayAdjustment             = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT                                         );
        AQLString floatCalendar	                        = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_CALENDAR                                                      );
        AQLString floatLegFixingBusinessDayAdjustment    = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_FIXINGBUSINESSDAYADJUSTMENT,     floatBusinessDayAdjustment   );
        AQLString floatLegFixingCalendar	                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_FIXINGCALENDAR,                  floatCalendar                );
        AQLString floatLegAccrualBusinessDayAdjustment   = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT,    floatBusinessDayAdjustment   );
        AQLString floatLegAccrualCalendar	            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_ACCRUALCALENDAR,                 floatCalendar                );
        AQLString floatLegPaymentBusinessDayAdjustment   = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_PAYMENTBUSINESSDAYADJUSTMENT,    floatBusinessDayAdjustment   );
        AQLString floatLegPaymentCalendar	            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_PAYMENTCALENDAR,                 floatCalendar                );
        AQLString floatLegPaymentFreq					= swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_PAYMENTFREQUENCY,                floatLegFreq                 );
 

        validateStringEmptiness( floatLegFixingBusinessDayAdjustment,	"#Error: Float leg 'Fixing Business Day Adjustment' must be specified." );
        validateStringEmptiness( floatLegFixingCalendar,                "#Error: Float leg 'Fixing Calendar' must be specified." );
        validateStringEmptiness( floatLegAccrualBusinessDayAdjustment,  "#Error: Float leg 'Accrual Business Day Adjustment' must be specified." );
        validateStringEmptiness( floatLegPaymentBusinessDayAdjustment,  "#Error: Float leg 'Payment Business Day Adjustment' must be specified." );
        validateStringEmptiness( floatLegAccrualCalendar,               "#Error: Float leg 'Accrual Calendar' must be specified." );
        validateStringEmptiness( floatLegPaymentCalendar,               "#Error: Float leg 'Payment Calendar' must be specified." );

        AQLString floatLegFirstStubDate		            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_FIRSTSTUBDATE                                                );
        AQLString floatLegLastStubDate		            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_LASTSTUBDATE                                                 );
        AQLString floatLegRollDayString		            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_ROLLDAY                                                      );
        AQLString floatLegFixLag			                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_FIXINGLAG,                       "0D"                        );
        AQLString floatLegPayLag							= swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_PAYMENTLAG,                      "0D"                        );
        AQLString floatLegStubType			            = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_STUBTYPE                                                     );

        // Set-Up First and Last Stub Parameters
        AQLDate* floatAccrualFirstOddDate    = NULL;
        AQLDate* floatAccrualLastOddDate     = NULL;

        AQLDate floatAccrualTempFirst;
        AQLDate floatAccrualTempLast;

        if( floatLegFirstStubDate.size() != 0 && floatLegFirstStubDate != "0")
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( floatLegStubType.size() != 0 && ( AQLString( floatLegStubType ).toUpper() ) != "NONE", "Float Leg cannot have both the StubType and First- or LastStubDate specified." );

            floatAccrualTempFirst = stringToDate( floatLegFirstStubDate, "#Error: Invalid floating leg 'FirstStubDate'." );
            floatAccrualFirstOddDate    = & floatAccrualTempFirst;
        }

        if( floatLegLastStubDate.size() != 0 && floatLegLastStubDate != "0")
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( floatLegStubType.size() != 0 && ( AQLString( floatLegStubType ).toUpper() ) != "NONE", "Float Leg cannot have both the StubType and First- or LastStubDate specified." );

            floatAccrualTempLast = stringToDate( floatLegLastStubDate, "#Error: Invalid floating leg 'LastStubDate'." );
            floatAccrualLastOddDate     = & floatAccrualTempLast;
        }

        // Get the float leg stub type i.e. ShortStart, LongStart, Short End or Long End
        AQLString* floatStubType = NULL;
        if( floatLegStubType.size() != 0  )
        {
            floatStubType = const_cast<AQLString*>( &floatLegStubType );
        }

        //
        // Generate the roll day conventions and ensure output roll parameters are initialized
        // -----------------------------------------------------------------------------------
        //

        // Fixed Leg Coupon Roll Conventions
        // ---------------------------------
        int* fixedLegRollDayPtr             = nullptr;
        AQLString* fixedLegRollConventionPtr = nullptr;

        int fixedLegRollDay                 = 0;
        AQLString fixedLegRollConvention     = AQLString( "" );
        bool fixedLegIsEOMRoll              = false;
        bool fixedLegIsStartRoll            = false;

        // Note: We check for RollDayString = AQLString("0") for backwards compatibility
        if ( fixedLegRollDayString != AQLString( "0" )  && fixedLegRollDayString.size() != 0 )
        {
            //// Generate Fixed Leg Coupon Roll Conventions if the rollDayString populated
            fixedLegRollDayPtr              = & fixedLegRollDay;
            fixedLegRollConventionPtr       = & fixedLegRollConvention;

            if ( isAssetSwap )
            {
                // *** Important Note *** : For asset swaps we set the effective or start date as the issue date to accumulate a full first coupon on the fixed bond leg
                validateAndPopulateRollDayConventions( fixedLegRollDayString, issueDate, maturityDate, &fixedLegRollDayPtr, &fixedLegRollConventionPtr, fixedLegIsEOMRoll, fixedLegIsStartRoll );
            }
            else
            {
                validateAndPopulateRollDayConventions( fixedLegRollDayString, effectiveDate, maturityDate, &fixedLegRollDayPtr, &fixedLegRollConventionPtr, fixedLegIsEOMRoll, fixedLegIsStartRoll );
            }
        }

        // Float Leg Coupon Roll Conventions
        // ---------------------------------
        int* floatLegRollDayPtr             = nullptr;
        AQLString* floatLegRollConventionPtr = nullptr;

        int floatLegRollDay                 = 0;
        AQLString floatLegRollConvention     = AQLString( "" );
        bool floatLegIsEOMRoll              = false;
        bool floatLegIsStartRoll            = false;

        // Note: We check for RollDayString = AQLString("0") for backwards compatibility
        if ( floatLegRollDayString != AQLString( "0" ) && floatLegRollDayString.size() != 0 )
        {
            //// Generate Fixed Leg Coupon Roll Conventions if the rollDayString populated
            floatLegRollDayPtr              = & floatLegRollDay;
            floatLegRollConventionPtr       = & floatLegRollConvention;

            validateAndPopulateRollDayConventions( floatLegRollDayString, effectiveDate, maturityDate, &floatLegRollDayPtr, &floatLegRollConventionPtr, floatLegIsEOMRoll, floatLegIsStartRoll );
        }

        //
        // Generate the swap schedule. This function populates the following schedule placeholders
        // ---------------------------------------------------------------------------------------
        //

        // Asset Swap Schedule
        etrading::AQLCurveForwardRateHelpers::generateSwapSchedule( effectiveDate,
                                                              maturityDate,
                                                              fixedLegFreq,
                                                              fixedLegAccrualBusinessDayAdjustment,
                                                              fixedLegAccrualCalendar,
                                                              fixedLegPaymentBusinessDayAdjustment,
                                                              fixedLegPaymentCalendar,
                                                              fixedPayLag,
                                                              fixedStubType,
                                                              fixedAccrualFirstOddDate,                   // Fixed Leg FirstStub
                                                              fixedAccrualLastOddDate,                    // Fixed Leg LastStub
                                                              fixedLegRollDayPtr,                         // ( POINTER ) Fixed Leg Roll Day integer 1-31 for day of month
                                                              fixedLegIsStartRoll,                        // Fixed Leg Start Roll Convention: True = Roll from Start Date, False = Roll from End Date
                                                              fixedLegIsEOMRoll,                          // Fixed Leg EOM Roll Convention: True = Roll on End-Of-Month, False = Do not roll on End-of-Month
                                                              fixedLegRollConventionPtr,                  // ( POINTER ) Fixed Leg Generic Roll Convention: IMM, EOM, Start, End or Null
                                                              floatLegFreq,
                                                              floatLegFixingBusinessDayAdjustment,
                                                              floatLegFixingCalendar,
                                                              floatLegFixLag,
                                                              floatLegAccrualBusinessDayAdjustment,
                                                              floatLegAccrualCalendar,
                                                              floatLegPaymentBusinessDayAdjustment,
                                                              floatLegPaymentCalendar,
                                                              floatLegPayLag,
                                                              floatStubType,
                                                              floatAccrualFirstOddDate,                   // Float Leg FirstStub
                                                              floatAccrualLastOddDate,                    // Float Leg LastStub
                                                              floatLegRollDayPtr,                         // ( POINTER ) Float Leg Roll Day integer 1-31 for day of month
                                                              floatLegIsStartRoll,                        // Float Leg Start Roll Convention: True = Roll from Start Date, False = Roll from End Date
                                                              floatLegIsEOMRoll,                          // Float Leg EOM Roll Convention: True = Roll on End-Of-Month, False = Do not roll on End-of-Month
                                                              floatLegRollConventionPtr,                  // ( POINTER ) Float Leg Generic Roll Convention: IMM, EOM, Start, End or Null
                                                              fixedAccrualDates,
                                                              fixedPaymentDates,
                                                              floatFixingDates,
                                                              floatAccrualDates,
                                                              floatPaymentDates,
			                                                  fixedLegPaymentFreq,
			                                                  floatLegPaymentFreq,
			                                                  fixingAdvanceOrArrears,
                                                              isAssetSwap,                                
                                                              issueDate );                                // Issue Date of the underlying bond of the asset swap
        
    }

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
                                           const AQLString&      effectiveDateString,
                                           const AQLString&      maturityDateString,
                                           const AQLString&      fixedLegFreq,
                                           const AQLString&      fixedLegDayCount,
                                           const AQLString&      fixedLegAccrualBusinessDayAdjustment,
                                           const AQLString&      fixedLegAccrualCalendar,
                                           const AQLString&      fixedLegPaymentBusinessDayAdjustment,
                                           const AQLString&      fixedLegPaymentCalendar,
                                           const AQLString&      fixedLegFirstStubDate,
                                           const AQLString&      fixedLegLastStubDate,
                                           const AQLString&      fixedLegRollDayString,
                                           const AQLString&      fixedLegPayLag,
                                           const AQLString&      fixedLegStubType,
                                           const AQLString&      floatLegFreq,
                                           const AQLString&      floatLegDayCount,
                                           const AQLString&      floatLegFixingBusinessDayAdjustment,
                                           const AQLString&      floatLegFixingCalendar,
                                           const AQLString&      floatLegAccrualBusinessDayAdjustment,
                                           const AQLString&      floatLegAccrualCalendar,
                                           const AQLString&      floatLegPaymentBusinessDayAdjustment,
                                           const AQLString&      floatLegPaymentCalendar,
                                           const AQLString&      floatLegFirstStubDate,
                                           const AQLString&      floatLegLastStubDate,
                                           const AQLString&      floatLegRollDayString,
                                           const AQLString&      floatLegFixLag,
                                           double               floatLegFirstFix,
                                           double               floatlegLastFix,
                                           const AQLString&      floatLegPayLag,
                                           const AQLString&      floatLegStubType,
										   const AQLString&		fixedLegPaymentFreq,
										   const AQLString&		floatLegPaymentFreq,
										   const AQLString&		floatLegFixingAdvanceOrArrears,
                                           const bool           isAssetSwap,
                                           AQLDate               issueDate )
    {

        // Set the Asset Swap Issue Date to the effective date if not populated
        if ( issueDate == AQLDate() )
        {
            issueDate = stringToDate( effectiveDateString, "#Error: Invalid 'EffectiveDate'" );
        }

        validateAndGenerateFixedLegCashflows( fixedAccrualDates,
                                              fixedPaymentDates,
                                              effectiveDateString,
                                              maturityDateString,
                                              fixedLegFreq,
                                              fixedLegDayCount,
                                              fixedLegAccrualBusinessDayAdjustment,
                                              fixedLegAccrualCalendar,
                                              fixedLegPaymentBusinessDayAdjustment,
                                              fixedLegPaymentCalendar,
                                              fixedLegFirstStubDate,
                                              fixedLegLastStubDate,
                                              fixedLegRollDayString,
                                              fixedLegPayLag,
                                              fixedLegStubType,
											  fixedLegPaymentFreq,
                                              isAssetSwap,
                                              issueDate ); // Regular Swaps start from the Effective date, however Asset Swaps have a full first fixed coupon and start from the bond issue date);

        validateAndGenerateFloatLegCashflows( floatFixingDates,
                                              floatAccrualDates,
                                              floatPaymentDates,
                                              effectiveDateString,
                                              maturityDateString,
                                              floatLegFreq,
                                              floatLegDayCount,
                                              floatLegFixingBusinessDayAdjustment,
                                              floatLegFixingCalendar,
                                              floatLegAccrualBusinessDayAdjustment,
                                              floatLegAccrualCalendar,
                                              floatLegPaymentBusinessDayAdjustment,
                                              floatLegPaymentCalendar,
                                              floatLegFirstStubDate,
                                              floatLegLastStubDate,
                                              floatLegRollDayString,
                                              floatLegFixLag,
                                              floatLegFirstFix,
                                              floatlegLastFix,
                                              floatLegPayLag,
                                              floatLegStubType,
											  floatLegPaymentFreq,
											  floatLegFixingAdvanceOrArrears);
    }

    /* @brief			helper method that validates Fixed Leg cash flow related params and generate actual cash flows
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
    */
    void validateAndGenerateFixedLegCashflows( DateVector&          fixedAccrualDates,
                                               DateVector&          fixedPaymentDates,
                                               const AQLString&      effectiveDateString,
                                               const AQLString&      maturityDateString,
                                               const AQLString&      fixedLegFreq,
                                               const AQLString&      fixedLegDayCount,
                                               const AQLString&      fixedLegAccrualBusinessDayAdjustment,
                                               const AQLString&      fixedLegAccrualCalendar,
                                               const AQLString&      fixedLegPaymentBusinessDayAdjustment,
                                               const AQLString&      fixedLegPaymentCalendar,
                                               const AQLString&      fixedLegFirstStubDate,
                                               const AQLString&      fixedLegLastStubDate,
                                               const AQLString&      fixedLegRollDayString,
                                               const AQLString&      fixedLegPayLag,
                                               const AQLString&      fixedLegStubType,
			                                   const AQLString&      fixedLegPaymentFreq,
                                               const bool           isAssetSwap,
                                               AQLDate               issueDate )
    {
        /////////////////////////////////////////////////////////////////////////////////////
        // Fixed Leg Parameters
        /////////////////////////////////////////////////////////////////////////////////////

        validateStringEmptiness(    fixedLegFreq,		                    "#Error: Fixed leg 'Frequency' must be specified." );
        validateStringEmptiness(    fixedLegDayCount,	                    "#Error: Fixed leg 'DayCount' must be specified." );
        validateStringEmptiness(    fixedLegAccrualBusinessDayAdjustment,	"#Error: Fixed leg 'Accrual BusDayAdjustment' must be specified." );
        validateStringEmptiness(    fixedLegAccrualCalendar,	            "#Error: Fixed leg 'Accrual Calendar' must be specified." );
        validateStringEmptiness(    fixedLegPaymentBusinessDayAdjustment,	"#Error: Fixed leg 'Payment BusDayAdjustment' must be specified." );
        validateStringEmptiness(    fixedLegPaymentCalendar,	            "#Error: Fixed leg 'Payment Calendar' must be specified." );


        AQLString fixedPayLag( fixedLegPayLag );
        if ( fixedPayLag.size() == 0 )
        {
            fixedPayLag = AQLString( "0D" );
        }

        // Set-Up First and Last Stub Parameters
        AQLDate* fixedAccrualFirstOddDate    = nullptr;
        AQLDate* fixedAccrualLastOddDate     = nullptr;
        AQLDate  fixedAccrualTempFirst;
        AQLDate  fixedAccrualTempLast;

        if( fixedLegFirstStubDate.size() != 0 && fixedLegFirstStubDate != "0")
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( fixedLegStubType.size() != 0 && AQLString( fixedLegStubType ).toUpper() != "NONE", "Fixed Leg cannot have both the StubType and First- or LastStubDate specified." );

            fixedAccrualTempFirst = stringToDate( fixedLegFirstStubDate, "#Error: Invalid fixed leg 'FirstStubDate'." );
            fixedAccrualFirstOddDate = &fixedAccrualTempFirst;
        }

        if( fixedLegLastStubDate.size() != 0 && fixedLegLastStubDate != "0")
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( fixedLegStubType.size() != 0 && ( AQLString( fixedLegStubType ).toUpper() ) != "NONE", "Fixed Leg cannot have both the StubType and First- or LastStubDate specified." );

            fixedAccrualTempLast = stringToDate( fixedLegLastStubDate, "#Error: Invalid fixed leg 'LastStubDate'." );
            fixedAccrualLastOddDate = & fixedAccrualTempLast;
        }

        // Get the fixed leg stub type i.e. ShortStart, LongStart, Short End or Long End
        AQLString* fixedStubType = nullptr;
        if( fixedLegStubType.size() != 0 )
        {
            fixedStubType = const_cast<AQLString*>( &fixedLegStubType );
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generic Parameters
        /////////////////////////////////////////////////////////////////////////////////////

        AQLDate effectiveDate    = stringToDate( effectiveDateString, "#Error: Invalid 'EffectiveDate'" );
        AQLDate maturityDate	    = validateMaturityDate( effectiveDate, maturityDateString);

        AQ_THROW_IF( maturityDate < effectiveDate, "The swap maturity date cannot be before the swap start date" );

        // For Asset Swaps if the issue Date is not provided use the effectiveDate
        if ( isAssetSwap && issueDate == AQLDate() )
        {
            issueDate = effectiveDate;
        }

        //
        // Generate the roll day conventions and ensure output roll parameters are initialized
        // -----------------------------------------------------------------------------------
        //

        // Fixed Leg Coupon Roll Conventions
        // ---------------------------------
        int* fixedLegRollDayPtr             = nullptr;
        AQLString* fixedLegRollConventionPtr = nullptr;

        int fixedLegRollDay                 = 0;
        AQLString fixedLegRollConvention     = AQLString( "" );
        bool fixedLegIsEOMRoll              = false;
        bool fixedLegIsStartRoll            = false;

        // Note: We check for RollDayString = AQLString("0") for backwards compatibility
        if ( fixedLegRollDayString != AQLString( "0" )  && fixedLegRollDayString.size() != 0 )
        {
            //// Generate Fixed Leg Coupon Roll Conventions if the rollDayString populated
            fixedLegRollDayPtr              = & fixedLegRollDay;
            fixedLegRollConventionPtr       = & fixedLegRollConvention;

            validateAndPopulateRollDayConventions( fixedLegRollDayString, ( isAssetSwap ? issueDate : effectiveDate ) , maturityDate, &fixedLegRollDayPtr, &fixedLegRollConventionPtr, fixedLegIsEOMRoll, fixedLegIsStartRoll );
        }

        //
        // Generate the swap schedule. This function populates the following schedule placeholders
        // ---------------------------------------------------------------------------------------
        //
        etrading::AQLCurveForwardRateHelpers::generateFixedLegSchedule( isAssetSwap ? issueDate : effectiveDate,
                                                                  maturityDate,
                                                                  fixedLegFreq,
                                                                  fixedLegAccrualBusinessDayAdjustment,
                                                                  fixedLegAccrualCalendar,
                                                                  fixedLegPaymentBusinessDayAdjustment,
                                                                  fixedLegPaymentCalendar,
                                                                  fixedPayLag,
                                                                  fixedStubType,
                                                                  fixedAccrualFirstOddDate,                   // Fixed Leg FirstStub
                                                                  fixedAccrualLastOddDate,                    // Fixed Leg LastStub
                                                                  fixedLegRollDayPtr,                         // ( POINTER ) Fixed Leg Roll Day integer 1-31 for day of month
                                                                  fixedLegIsStartRoll,                        // Fixed Leg Start Roll Convention: True = Roll from Start Date, False = Roll from End Date
                                                                  fixedLegIsEOMRoll,                          // Fixed Leg EOM Roll Convention: True = Roll on End-Of-Month, False = Do not roll on End-of-Month
                                                                  fixedLegRollConventionPtr,                  // ( POINTER ) Fixed Leg Generic Roll Convention: IMM, EOM, Start, End or Null
                                                                  fixedAccrualDates,
                                                                  fixedPaymentDates,
				                                                  fixedLegPaymentFreq );

        //
        // Validate Fixed Leg Accrual & Payment Dates
        //
        int nFixedAccrualDates = fixedAccrualDates.size() - 1;
        for( int i = 1; i < nFixedAccrualDates; i++ )
        {
            AQ_THROW_IF( fixedAccrualDates[i - 1] >= fixedAccrualDates[i], "Invalid Dates Generated. Fixed Leg Accrual Dates must be in ascending order." );
        }

        int nFixedPaymentDates = fixedPaymentDates.size() - 1;
        for( int i = 1; i < nFixedPaymentDates; i++ )
        {
            AQ_THROW_IF( fixedPaymentDates[i - 1] > fixedPaymentDates[i], "Invalid Dates Generated. Fixed Leg Payment Dates must be in ascending order." );
        }
    }

    /* @brief			helper method that validates Float Leg cash flow related params and generate actual cash flows
    *  @param [out]		floatFixingDates		                        Fixing dates schedule on floating leg
    *  @param [out]		floatAccrualDates		                        Accrual dates schedule on floating leg
    *  @param [out]		floatPaymentDates		                        Payment dates schedule on floating leg
    *  @param [in]		effectDateString	                            The effective start date of the swap, ie, base date + spot date
    *  @param [in]		maturityDateString                              Swap maturity date or tenor
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
            const AQLString&      effectiveDateString,
            const AQLString&      maturityDateString,
            const AQLString&      floatLegFreq,
            const AQLString&      floatLegDayCount,
            const AQLString&      floatLegFixingBusinessDayAdjustment,
            const AQLString&      floatLegFixingCalendar,
            const AQLString&      floatLegAccrualBusinessDayAdjustment,
            const AQLString&      floatLegAccrualCalendar,
            const AQLString&      floatLegPaymentBusinessDayAdjustment,
            const AQLString&      floatLegPaymentCalendar,
            const AQLString&      floatLegFirstStubDate,
            const AQLString&      floatLegLastStubDate,
            const AQLString&      floatLegRollDayString,
            const AQLString&      floatLegFixLag,
            double               floatLegFirstFix,
            double               floatlegLastFix,
            const AQLString&      floatLegPayLag,
            const AQLString&      floatLegStubType,
			const AQLString&		 floatLegPaymentFreq,
			const AQLString&		 floatLegFixingAdvanceOrArrears,
			bool				 removeExtraDay)
    {
        /////////////////////////////////////////////////////////////////////////////////////
        // Float Leg Parameters
        /////////////////////////////////////////////////////////////////////////////////////

        validateStringEmptiness( floatLegFreq,		                    "#Error: Float leg 'Frequency' must be specified." );
        validateStringEmptiness( floatLegDayCount,	                    "#Error: Float leg 'DayCount' must be specified." );
        validateStringEmptiness( floatLegFixingBusinessDayAdjustment,	"#Error: Float leg 'Fixing Business Day Adjustment' must be specified." );
        validateStringEmptiness( floatLegPaymentCalendar,               "#Error: Float leg 'Fixing Calendar' must be specified." );
        validateStringEmptiness( floatLegAccrualBusinessDayAdjustment,	"#Error: Float leg 'Accrual Business Day Adjustment' must be specified." );
        validateStringEmptiness( floatLegPaymentCalendar,               "#Error: Float leg 'Accrual Calendar' must be specified." );
        validateStringEmptiness( floatLegPaymentBusinessDayAdjustment,	"#Error: Float leg 'Payment Business Day Adjustment' must be specified." );
        validateStringEmptiness( floatLegPaymentCalendar,               "#Error: Float leg 'Payment Calendar' must be specified." );

        // Convert FixingLag to AQLString
        AQLString floatFixingLag( floatLegFixLag );
        if( floatLegFixLag.size() == 0 )
        {
            floatFixingLag = AQLString( "0D" );
        }

        // Convert Floating Pay Lag to AQLString
        AQLString floatPayLag( floatLegPayLag );
        if( floatLegPayLag.size() == 0 )
        {
            floatPayLag = AQLString( "0D" );
        }

        // Set-Up First and Last Stub Parameters
        AQLDate* floatAccrualFirstOddDate    = nullptr;
        AQLDate* floatAccrualLastOddDate     = nullptr;

        AQLDate floatAccrualTempFirst;
        AQLDate floatAccrualTempLast;

        if( floatLegFirstStubDate.size() != 0 && floatLegFirstStubDate != "0")
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( floatLegStubType.size() != 0 && ( AQLString( floatLegStubType ).toUpper() ) != "NONE", "Float Leg cannot have both the StubType and First- or LastStubDate specified." );

            floatAccrualTempFirst = stringToDate( floatLegFirstStubDate, "#Error: Invalid floating leg 'FirstStubDate'." );
            floatAccrualFirstOddDate    = & floatAccrualTempFirst;
        }

        if( floatLegLastStubDate.size() != 0 && floatLegLastStubDate != "0")
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( floatLegStubType.size() != 0 && ( AQLString( floatLegStubType ).toUpper() ) != "NONE", "Float Leg cannot have both the StubType and First- or LastStubDate specified." );

            floatAccrualTempLast = stringToDate( floatLegLastStubDate, "#Error: Invalid floating leg 'LastStubDate'." );
            floatAccrualLastOddDate     = & floatAccrualTempLast;
        }

        // Get the float leg stub type i.e. ShortStart, LongStart, Short End or Long End
        AQLString* floatStubType = nullptr;
        if( floatLegStubType.size() != 0  )
        {
            floatStubType = const_cast<AQLString*>( &floatLegStubType );
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generic Parameters
        /////////////////////////////////////////////////////////////////////////////////////

        AQLDate effectiveDate    = stringToDate( effectiveDateString, "#Error: Invalid 'EffectiveDate'" );
        AQLDate maturityDate	    = validateMaturityDate( effectiveDate, maturityDateString);

        AQ_THROW_IF( maturityDate < effectiveDate, "The swap maturity date cannot be before the swap start date" );

        //
        // Generate the roll day conventions and ensure output roll parameters are initialized
        // -----------------------------------------------------------------------------------
        //

        // Float Leg Coupon Roll Conventions
        // ---------------------------------
        int* floatLegRollDayPtr             = nullptr;
        AQLString* floatLegRollConventionPtr = nullptr;

        int floatLegRollDay                 = 0;
        AQLString floatLegRollConvention     = AQLString( "" );
        bool floatLegIsEOMRoll              = false;
        bool floatLegIsStartRoll            = false;

        // Note: We check for RollDayString = AQLString("0") for backwards compatibility
        if ( floatLegRollDayString != AQLString( "0" ) && floatLegRollDayString.size() != 0 )
        {
            //// Generate Fixed Leg Coupon Roll Conventions if the rollDayString populated
            floatLegRollDayPtr              = & floatLegRollDay;
            floatLegRollConventionPtr       = & floatLegRollConvention;

            validateAndPopulateRollDayConventions( floatLegRollDayString, effectiveDate, maturityDate, &floatLegRollDayPtr, &floatLegRollConventionPtr, floatLegIsEOMRoll, floatLegIsStartRoll );
        }

        //
        // Generate the swap schedule. This function populates the following schedule placeholders
        // ---------------------------------------------------------------------------------------
        //
        etrading::AQLCurveForwardRateHelpers::generateFloatLegSchedule( effectiveDate,
                                                                  maturityDate,
                                                                  floatLegFreq,
                                                                  floatLegFixingBusinessDayAdjustment,
                                                                  floatLegFixingCalendar,
                                                                  floatFixingLag,
                                                                  floatLegAccrualBusinessDayAdjustment,
                                                                  floatLegAccrualCalendar,
                                                                  floatLegPaymentBusinessDayAdjustment,
                                                                  floatLegPaymentCalendar,
                                                                  floatPayLag,
                                                                  floatStubType,
                                                                  floatAccrualFirstOddDate,         // Float Leg FirstStub
                                                                  floatAccrualLastOddDate,          // Float Leg LastStub
                                                                  floatLegRollDayPtr,               // ( POINTER ) Float Leg Roll Day integer 1-31 for day of month
                                                                  floatLegIsStartRoll,              // Float Leg Start Roll Convention: True = Roll from Start Date, False = Roll from End Date
                                                                  floatLegIsEOMRoll,                // Float Leg EOM Roll Convention: True = Roll on End-Of-Month, False = Do not roll on End-of-Month
                                                                  floatLegRollConventionPtr,        // ( POINTER ) Float Leg Generic Roll Convention: IMM, EOM, Start, End or Null
                                                                  floatFixingDates,
                                                                  floatAccrualDates,
                                                                  floatPaymentDates,
				                                                  floatLegPaymentFreq,
				                                                  floatLegFixingAdvanceOrArrears,
				                                                  removeExtraDay);

        //
        // Validate Floating Leg Accrual, Fixing & Payment Dates
        //
        int nFloatAccrualDates = floatAccrualDates.size() - 1;
        for( int i = 1; i < nFloatAccrualDates; i++ )
        {
            AQ_THROW_IF( floatAccrualDates[i - 1] >= floatAccrualDates[i], "Invalid Dates Generated. Floating Leg Accrual Dates must be in ascending order" );
        }

        int nFloatFixingDates = floatFixingDates.size() - 1;
        for( int i = 1; i < nFloatFixingDates; i++ )
        {
            AQ_THROW_IF( floatFixingDates[i - 1] > floatFixingDates[i], "Invalid Dates Generated. Floating Leg Fixing Dates must be in ascending order" );
        }

        int nFloatPaymentDates = floatPaymentDates.size() - 1;
        for( int i = 1; i < nFloatPaymentDates; i++ )
        {
            AQ_THROW_IF( floatPaymentDates[i - 1] > floatPaymentDates[i], "Invalid Dates Generated. Floating Leg Payment Dates must be in ascending order" );
        }
    }

    /* @brief			helper method that validates and generates accrual start and end dates from a single combined vector of accrual dates
    *  @param [out]		accrualStartDates		    Accrual Start Dates
    *  @param [out]		accrualEndDates		        Accrual End Dates
    *  @param [in]		combinedAccrualDates        Combined Accrual Dates
    */
    void validateAndGenerateAccrualStartAndEndDates( DateVector&          accrualStartDates,
													DateVector&          accrualEndDates,
													const DateVector&    combinedAccrualDates )
    {
        etrading::AQLCurveForwardRateHelpers::validateAndGenerateAccrualStartAndEndDates(accrualStartDates, accrualEndDates, combinedAccrualDates);

		AQ_THROW_IF( accrualStartDates.size() != accrualEndDates.size(), "Invalid Input, the accrualStartDates and accrualEndDates must have the same size." );

    }

    
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
            const AQLDate* firstStubDtPtr,
            const AQLDate* lastStubDtPtr,
            const AQLDate& startDate,
            const AQLDate& endDate,
            const AQLString& frequency )
    {

        bool isStartRoll = false; // default to false, the same as ShortStart

        if (stubType != NONE_STUBTYPE )
        {

            AQLString term = etrading::fromFrequencyToTerm( frequency );

            AQ_THROW_IF( firstStubDtPtr != nullptr || lastStubDtPtr != nullptr, "Must not specifiy 'StubType' with 'FirstStubDate' or 'LastStubDate'." );

            if ( stubType == SHORT_START_STUBTYPE )
            {
                isStartRoll = false;
                firstStubDtPtr = nullptr;
                lastStubDtPtr = nullptr;
            }
            else if ( stubType == LONG_START_STUBTYPE )
            {
                isStartRoll = false;
                AQLDate pfoddTemp = AQLDateScheduleHelpers::firstStubDateFromStubType( startDate, endDate, term );
                firstStubDtPtr = &pfoddTemp;
                lastStubDtPtr = nullptr;
            }
            else if ( stubType == SHORT_END_STUBTYPE )
            {
                isStartRoll = true;
                firstStubDtPtr = nullptr;
                lastStubDtPtr = nullptr;
            }
            else if ( stubType == LONG_END_STUBTYPE )
            {
                isStartRoll = true;
                AQLDate pfoddTemp = AQLDateScheduleHelpers::lastStubDateFromStubType( startDate, endDate, term );
                firstStubDtPtr = nullptr;
                lastStubDtPtr = &pfoddTemp;
            }
            else
            {
                AQ_THROW( "Stub Type must be None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE)." );
            }
        }

        return isStartRoll;

    }

		/* @brief			Check the input dates are in ascending order
    *  @param [in]		dates	                   Input dates
    *  @param [in]		allowEqual		           True to allow equal
    *  @param [in]		dateName                   Input date names, for error message
    */
	void checkDatesInAscendingOrder(const DateVector& dates, bool allowEqual, const AQLString& dateName)
	{
 	   int nDates = dates.size() - 1;
	   for( int i = 1; i < nDates; i++ )
        {
	       std::ostringstream  ss;
			if (dates[i - 1] > dates[i] || (!allowEqual && dates[i - 1] == dates[i] ))
            {
				ss << "#Error: Invalid " << dateName << " Generated. Dates must be in ascending order.";
				AQ_THROW( ss.str().c_str() );
			}
        }
	}
	
	/* @brief			Helper method that generates accrual dates and payment dates
	*					Note: In this version, the effectiveDate and maturityDate are provided as actual dates.
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
    void validateAndGenerateAccrualAndPaymentSchedules( DateVector &                        accrualDates,
													    DateVector &                        paymentDates,
														const AQLDate&        				effectiveDate,
														const AQLDate&        				maturityDate,
													    const AQLString &                    accrualFreq,
													    const AQLString &                    accrualBusinessDayAdjustment,
													    const AQLString &                    accrualCalendar,
													    const AQLString &                    paymentFreq,
													    const AQLString &                    paymentBusinessDayAdjustment,
													    const AQLString &                    paymentCalendar,
													    const AQLString &                    paymentLag,
													    const AQLString &                    stubType,
													    const AQLString &                    firstStubDate,
													    const AQLString &                    lastStubDate,
													    const AQLString &                    rollDayString,
													    const AQLString &		            fixingAdvanceOrArrears )
    {
        /////////////////////////////////////////////////////////////////////////////////////
        //  Leg Parameters
        /////////////////////////////////////////////////////////////////////////////////////

        validateStringEmptiness(    accrualFreq,		            "#Error: 'Accrual Frequency' must be specified." );
        validateStringEmptiness(    accrualBusinessDayAdjustment,	"#Error: 'Accrual BusDayAdjustment' must be specified." );
        validateStringEmptiness(    accrualCalendar,	            "#Error: 'Accrual Calendar' must be specified." );
        validateStringEmptiness(    paymentFreq,		            "#Error: 'Payment Frequency' must be specified." );
        validateStringEmptiness(    paymentBusinessDayAdjustment,	"#Error: 'Payment BusDayAdjustment' must be specified." );
        validateStringEmptiness(    paymentCalendar,	            "#Error: 'Payment Calendar' must be specified." );


        AQ_THROW_IF( maturityDate < effectiveDate, "The swap maturity date cannot be before the swap start date" );

        AQLString payLag(paymentLag);
        if ( payLag.size() == 0 )
        {
            payLag = AQLString( "0D" );
        }

        // Set-Up First and Last Stub Parameters
        AQLDate* accrualFirstOddDate    = nullptr;
        AQLDate* accrualLastOddDate     = nullptr;
        AQLDate  accrualTempFirst;
        AQLDate  accrualTempLast;

        if( firstStubDate.size() != 0 )
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( stubType.size() != 0 && AQLString( stubType ).toUpper() != "NONE", "Cannot have both the StubType and First- or LastStubDate specified." );

            accrualTempFirst = stringToDate( firstStubDate, "#Error: Invalid 'FirstStubDate'." );
            accrualFirstOddDate = &accrualTempFirst;
        }

        if( lastStubDate.size() != 0 )
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            AQ_THROW_IF( stubType.size() != 0 && ( AQLString( stubType ).toUpper() ) != "NONE", "Cannot have both the StubType and First- or LastStubDate specified." );

            accrualTempLast = stringToDate( lastStubDate, "#Error: Invalid 'LastStubDate'." );
            accrualLastOddDate = & accrualTempLast;
        }

        // Get the  leg stub type i.e. ShortStart, LongStart, Short End or Long End
        AQLString* stubT = nullptr;
        if( stubType.size() != 0 )
        {
            stubT = const_cast<AQLString*>( &stubType );
        }

        //
        // Generate the roll day conventions and ensure output roll parameters are initialized
        // -----------------------------------------------------------------------------------
        //

        //  Coupon Roll Conventions
        // ---------------------------------
        int* rollDayPtr             = nullptr;
        AQLString* rollConventionPtr = nullptr;

        int rollDay                 = 0;
        AQLString rollConvention     = AQLString( "" );
        bool isEOMRoll              = false;
        bool isStartRoll            = false;

        // Note: We check for RollDayString = AQLString("0") for backwards compatibility
        if ( rollDayString != AQLString( "0" )  && rollDayString.size() != 0 )
        {
            //// Generate  Leg Coupon Roll Conventions if the rollDayString populated
            rollDayPtr              = & rollDay;
            rollConventionPtr       = & rollConvention;

            validateAndPopulateRollDayConventions( rollDayString, effectiveDate, maturityDate, &rollDayPtr, &rollConventionPtr, isEOMRoll, isStartRoll );
        }

        //
        // Generate accrual schedule and payment schedule. 
        // ---------------------------------------------------------------------------------------
        //
		etrading::AQLCurveForwardRateHelpers::generateAccrualAndPaymentSchedule( accrualDates,
												                                paymentDates,
												                                effectiveDate,
												                                maturityDate,
												                                accrualFreq,
												                                accrualBusinessDayAdjustment,
												                                accrualCalendar,
												                                paymentFreq,
												                                paymentBusinessDayAdjustment,
												                                paymentCalendar,
												                                payLag,
												                                stubT,
												                                accrualFirstOddDate,             // FirstStub
												                                accrualLastOddDate,              // LastStub
												                                rollDayPtr,                      // ( POINTER )  Roll Day integer 1-31 for day of month
												                                isStartRoll,                     //  Start Roll Convention: True = Roll from Start Date, False = Roll from End Date
												                                isEOMRoll,                       //  EOM Roll Convention: True = Roll on End-Of-Month, False = Do not roll on End-of-Month
												                                rollConventionPtr,               // ( POINTER ) Generic Roll Convention: IMM, EOM, Start, End or Null
												                                fixingAdvanceOrArrears,
												                                true);                           // removeExtraPaymentDay

		checkDatesInAscendingOrder(accrualDates, false, "Accrual Dates");
		checkDatesInAscendingOrder(paymentDates, true, "Payment Dates");

	}


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
    void validateAndGenerateAccrualAndPaymentSchedules( DateVector &                        accrualDates,
													    DateVector &                        paymentDates,
													    const AQLString &                    effectiveDateString,
													    const AQLString &                    maturityDateString,
													    const AQLString &                    accrualFreq,
													    const AQLString &                    accrualBusinessDayAdjustment,
													    const AQLString &                    accrualCalendar,
													    const AQLString &                    paymentFreq,
													    const AQLString &                    paymentBusinessDayAdjustment,
													    const AQLString &                    paymentCalendar,
													    const AQLString &                    paymentLag,
													    const AQLString &                    stubType,
													    const AQLString &                    firstStubDate,
													    const AQLString &                    lastStubDate,
													    const AQLString &                    rollDayString,
													    const AQLString &		            fixingAdvanceOrArrears )
    {
		AQLDate effectiveDate = stringToDate(effectiveDateString, "#Error: Invalid 'EffectiveDate'");
		AQLDate maturityDate = validateMaturityDate(effectiveDate, maturityDateString);

		validateAndGenerateAccrualAndPaymentSchedules(  accrualDates,
														paymentDates,
														effectiveDate,
														maturityDate,
														accrualFreq,
														accrualBusinessDayAdjustment,
														accrualCalendar,
														paymentFreq,
														paymentBusinessDayAdjustment,
														paymentCalendar,
														paymentLag,
														stubType,
														firstStubDate,
														lastStubDate,
														rollDayString,
														fixingAdvanceOrArrears );

	}


	/* @brief		Validates and genertate fixing schedule
    * @param [out]	fixingDates		                Fixing schedule
	* @param [in]	accrualDates					Accrual schedue
	* @param [in]	fixingBusinessDayAdjustment     Fixing business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
	* @param [in]	fixingCalendar					Fixing calendar 
	* @param [in]	fixingLag						Fixing lag
	* @param [in]	fixingAdvanceOrArrears			Flag to indicate the fixing is advance or arrears
    * @return the fixing dates 
    */
    DateVector validateAndGenerateFixingSchedule(const DateVector&  accrualDates,
											    const AQLString&     fixingBusinessDayAdjustment,
											    const AQLString&     fixingCalendar,
											    const AQLString&     fixingLag,
											    const AQLString&		fixingAdvanceOrArrears,
												const bool          includeLastExtraFixingDate)
    {
  
        validateStringEmptiness( fixingBusinessDayAdjustment,	"#Error: 'Fixing Business Day Adjustment' must be specified." );
        validateStringEmptiness( fixingCalendar,	"#Error: 'Fixing Calendar' must be specified." );
    
        AQLString fixLag( fixingLag );
        if( fixLag.size() == 0 )
        {
            fixLag = AQLString( "0D" );
        }

        //
        // Generate accrual schedule and payment schedule. 
        //
        DateVector fixingDates = etrading::AQLCurveForwardRateHelpers::getFixingSchedule( accrualDates,
																						 fixingBusinessDayAdjustment,
																						 fixingCalendar,
																						 fixLag,
																						 fixingAdvanceOrArrears,
																						 !includeLastExtraFixingDate);

		checkDatesInAscendingOrder(fixingDates, true, "Fixing Dates");

		return fixingDates;
    }

	/* @brief			Transform the accrual start and end dates to a single combined vector of accrual dates
	*  @param [in]		accrualStartDates		    Accrual Start Dates
	*  @param [in]		accrualEndDates		        Accrual End Dates
	*  @return 		combinedAccrualDates        Combined Accrual Dates
	*/
	std::vector<AQLDate> combineAccrualStartAndEndDates(const std::vector<AQLDate>& accrualStartDates, const std::vector<AQLDate>& accrualEndDates)
	{
		AQ_THROW_IF( accrualStartDates.size() == 0 || accrualEndDates.size() == 0, "Accrual Start Dates and End Dates cannot be empty" );
		std::vector<AQLDate> accrualDates(accrualStartDates);
		accrualDates.push_back(accrualEndDates.back());
		return accrualDates;
	}

	/* @brief Transform dates from AQLDate format to double format
    * 
    *  @param [in] dateVec dates in AQLDate format
    *  @Return     date in double format
    */
	DoubleVector fromDateToDoubleVector(const DateVector& dateVec)
	{
		DoubleVector dVec;
		for(size_t i=0;i<dateVec.size(); ++i)
		{
			dVec.push_back((double)(AQLDateScheduleHelpers::getExcelDate(dateVec[i])));
		}
		return dVec;
	}

    /* @brief Transform dates from AQLDate format to double format, if date is NaN, return NaN
    * 
    *  @param [in] date date in AQLDate format
    *  @Return     date in double format
    */
	double fromAQLDateToDouble(const AQLDate& date)
	{
		if (date == AQLDate())
		{
			return std::numeric_limits<double>::quiet_NaN();
		}
        return ((double)(AQLDateScheduleHelpers::getExcelDate(date)));
	}

    /* @brief Transform dates from double format to AQLDate format
    *  @param [in] doubleVec dates in double format
    *  @Return     dates in AQLDate format
    */
	DateVector fromDoubleToDateVector(const DoubleVector& doubleVec)
	{
		DateVector dVec;
		for(size_t i=0;i<doubleVec.size(); ++i)
		{
			dVec.push_back( AQLDateScheduleHelpers::getAQLDate( (int)( doubleVec[i] ) ) );
		}
		return dVec;
	}

	/* @brief Transpose a double matrix
    *  @param [in] input                The input matrix
    *  @Return     a transposed matrix
    */
	DoubleMatrix transpose( const DoubleMatrix& input )
	{
		size_t rowSize = input.size();
        AQ_THROW_IF( rowSize == 0, "Data Validation - input matrix is empty." );

        // Only allow Rectangular Matrices, we do not support jagged matrices here
        size_t columnSize = input[0].size();
        for (size_t i = 1; i < rowSize; ++i) // Start from base 1 not 0
        {
            AQ_REQUIRE( columnSize == input[i].size(), "#Error: Data Validation - only rectangular matrices supported. Data rows and columns must be of the same size." );
        }
        
        DoubleMatrix output( columnSize, DoubleVector( rowSize, 0.0 ) );

        // Transpose Data
		for (size_t i = 0; i < rowSize; ++i)
		{
            for (size_t j = 0; j < columnSize; ++j)
			{
				output[j][i] = input[i][j];
			}
		}
		return output;
	}

	/* @brief Calculate days between start/end dates 
    * 
    *  @param [in] accrualStart		Accrual start date
    *  @param [in] accrualEnd		Accrual end date
    *  @Return     days
    */
	int getDays(const AQLDate& accrualStart, const AQLDate& accrualEnd)
	{
        int days = accrualStart.intervalDays(accrualEnd);
        return days;
	}

	/* @brief			Number of long Feb (feb29) between fromDate and toDate
	*  @param [in]		fromDate	    From Date
	*  @param [in]		toDate			To Date
	*  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
	*  @return			Number of long Feb (feb29)
	*/
	int numberOfLongFeb(const AQLDate& fromDate, const AQLDate& toDate, bool includeLast)
	{

		int years = toDate.yearOfEra() - fromDate.yearOfEra();

		AQLDate curYearFeb29 = fromDate;
		curYearFeb29.setMonth(2);

		//Count how many date is 29thFeb between fromDate and toDate
		int numOfLongFeb = 0;

		AQLDate curDate = fromDate;

		for (int i = 0; i <= years; i++)
		{
			if (curDate.isLeapYear())
			{
				auto curYear = curDate.yearOfEra();
				curYearFeb29.setYear(curYear);
				curYearFeb29.setDay(29);

				if (curYearFeb29 >= fromDate &&  curYearFeb29 <= toDate)
				{
					numOfLongFeb++;
				}
			}

			//next year
			curDate.addYears(1);
		}

		//includeLast true: include last date exclude first date 
		bool fromDateFeb29 = (fromDate.monthOfYear() == 2 && fromDate.dayOfMonth() == 29);
		if (includeLast  && fromDateFeb29) numOfLongFeb--;

		//includeLast false: exclue last date include first date 
		bool toDateFeb29 = (toDate.monthOfYear() == 2 && toDate.dayOfMonth() == 29);
		if (!includeLast && toDateFeb29) numOfLongFeb--;

		return numOfLongFeb;
	}

    /* @brief			Year fraction between fromDate and toDate
    *  @param [in]		fromDate	    From Date
    *  @param [in]		toDate			To Date
    *  @param [in]		dayCount		Day count convention
    *  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
    *  @return			Year fraction between fromDate and toDate
    */
    double getYearFraction( const AQLDate& fromDate, const AQLDate& toDate, const DayCountEnum& dayCount, bool includeLast )
    {
        double tao = 0.0;

        if (dayCount == ACT_365_FJ_DAYCOUNT)
        {
            //If February 29 is not in the period then actual number of days between dates is used. Else actual number of days minus 1 is used. Day count basis = 365. 

			int numOfLongFeb = numberOfLongFeb(fromDate, toDate, includeLast);

            auto totalDays = fromDate.intervalDays(toDate);

            tao = (totalDays- numOfLongFeb) * 1.0 / 365;
        } 
        else if (dayCount == E30_360_DAYCOUNT) 
		{
            /*  30E/360, 30/360 ISMA 
            	Start date:	M1/D1/Y1, End date:	M2/D2/Y2
            	Day count	= (Y2-Y1)*360+(M2-M1)*30+(D2-D1) 
            Convention: 
            	if D1=31 then D1=30
            	if D2=31 then D2=30
            */
			int d1 = (fromDate.dayOfMonth() == 31) ? 30 : fromDate.dayOfMonth();
			int d2 = (toDate.dayOfMonth() == 31) ? 30 : toDate.dayOfMonth();

			auto days =  (toDate.yearOfEra() - fromDate.yearOfEra()) * 360 + (toDate.monthOfYear() - fromDate.monthOfYear()) * 30 + (d2 - d1);

            tao = days / 360.0;
			
		}
        else if (dayCount == E30_360_ISDA_DAYCOUNT) 
		{
            /*  30E/360 ISDA, 30/360 German
            	Start date:	M1/D1/Y1, End date:	M2/D2/Y2
            	Day count	= (Y2-Y1)*360+(M2-M1)*30+(D2-D1) 
            Convention: 
            	if D1=31 then D1=30
            	if D2=31 then D2=30
            	if D1 is the last day of February then D1=30 
            	if D2 is the last day of February then D2=30 
            */
			int d1 = (fromDate.dayOfMonth() == 31) ? 30 : fromDate.dayOfMonth();
			int d2 = (toDate.dayOfMonth() == 31) ? 30 : toDate.dayOfMonth();

			if( (fromDate.monthOfYear() == 2) && (d1 == 29 || (!fromDate.isLeapYear() && d1 == 28))) d1 = 30;
			if( (toDate.monthOfYear() == 2) && (d2 == 29 || (!toDate.isLeapYear() && d2 == 28))) d2 = 30;
	
			auto days =  (toDate.yearOfEra() - fromDate.yearOfEra()) * 360 + (toDate.monthOfYear() - fromDate.monthOfYear()) * 30 + (d2 - d1);

            tao = days / 360.0;
		}
        else
        {
			AQLString dayCountStr( toString( dayCount ).c_str() );
            tao = AQLDateScheduleHelpers::getTerm( fromDate, toDate, dayCountStr, includeLast );
        }
        return tao;
    }

	/* @brief Transpose a matrix of elements
    *  @param [in] input The input matrix
    *  @Return     a transpose matrix
    */
	template<typename T>
	std::vector< std::vector<T> > transpose( const std::vector< std::vector<T> >& input )
	{
		size_t rowSize = input.size();
        AQ_THROW_IF( rowSize == 0, "Data Validation - input matrix is empty." );
        
        // Only allow Rectangular Matrices, we do not support jagged matrices here
        size_t columnSize = input[0].size();
        for (size_t i = 1; i < rowSize; ++i) // Start from base 1 not 0
        {
            AQ_REQUIRE( columnSize == input[i].size(), "#Error: Data Validation - only rectangular matrices supported. Data rows and columns must be of the same size." );
        }
        std::vector< std::vector<T> > output( columnSize, std::vector<T>( rowSize ) );

        // Transpose Data
		for (size_t i = 0; i < rowSize; ++i)
		{
            for (size_t j = 0; j < columnSize; ++j)
			{
				output[j][i] = input[i][j];
			}
		}
		return output;
	};

	/* @brief Transpose a matrix of elements
	*  @param [in] input                The input matrix
	*  @param [in] padIfInputIsRagged   If the input is a ragged matrix with different number of columns, and this flag set to true, pad the matrix to form a rectangular matrix.
	*  @Return     a transpose matrix
	*/
	template<typename T>
	std::vector< std::vector<T> > transpose(const std::vector< std::vector<T> >& input, const bool padIfInputIsRagged )
	{
		size_t rowSize = input.size();
		AQ_THROW_IF( rowSize == 0, "Data Validation - input matrix is empty." );

		// Determine the number of columns in each row
		size_t maxColumnSize = input[0].size();
		for (size_t i = 1; i < rowSize; ++i) // Start from base 1 not 0
		{
			size_t curColumnSize = input[i].size();
			if (curColumnSize != maxColumnSize)
			{
				if ( padIfInputIsRagged )
				{
					if ( curColumnSize > maxColumnSize )
					{
						maxColumnSize = curColumnSize;
					}
				}
				else
				{
					AQ_THROW( "Data Validation - only rectangular matrices supported. Data rows and columns must be of the same size." );
				}
			}
		}

		std::vector< std::vector<T> > output(maxColumnSize, std::vector<T>(rowSize));

		// Transpose Data
		for (size_t i = 0; i < rowSize; ++i)
		{
			for (size_t j = 0; j < maxColumnSize; ++j)
			{
				if (j < input[i].size())
				{
					output[j][i] = input[i][j];
				}
			}
		}
		return output;
	};


	template AQLStringMatrix transpose<AQLString>( const AQLStringMatrix& );
	template std::vector<std::vector<std::string>> transpose<std::string>( const std::vector<std::vector<std::string>>& );
	template std::vector<std::vector<double>> transpose<double>( const std::vector<std::vector<double>>& );
	template std::vector<std::vector<Variant>> transpose<Variant>( const std::vector<std::vector<Variant>>& );
	template std::vector<std::vector<Variant>> transpose<Variant>(const std::vector<std::vector<Variant>>&, const bool padIfNeccesary );
      
    /* @brief			Helper method to get the RollConvenction string from rollDayString
    *  @param [in]		startDate	               Start date
    *  @param [in]		endDate                    end Date
    *  @param [in]		rollDayString              Rolling date (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
    */
    AQLString getRollConvection(const AQLDate& startDate,
                               const AQLDate& endDate,
                               const AQLString& rollDayString)
    {
        int* rollDayPtr             = nullptr;
        AQLString* rollConventionPtr = nullptr;

        int rollDay                 = 0;
        AQLString rollConvention     = AQLString( "" );
        bool isEOMRoll              = false;
        bool isStartRoll            = false;

        // Note: We check for RollDayString = AQLString("0") for backwards compatibility
        if ( rollDayString != AQLString( "0" )  && rollDayString.size() != 0 )
        {
            //// Generate  Leg Coupon Roll Conventions if the rollDayString populated
            rollDayPtr              = & rollDay;
            rollConventionPtr       = & rollConvention;

            validateAndPopulateRollDayConventions( rollDayString, startDate, endDate, &rollDayPtr, &rollConventionPtr, isEOMRoll, isStartRoll );
        }

        AQLString rollConv = (rollConventionPtr == nullptr) ? "" : *rollConventionPtr;

        return rollConv;

    }

	/* @brief			Helper method to get the RollConvenction string from rollDayString
	*  @param [in]		rollDayString              Rolling date (This can be an integer for the day of the month or a string for IMM, EOM, Start or End roll conventions)
	*/
	AQLString getRollConvection(const AQLString& rollDayString)
	{
		int* rollDayPtr = nullptr;
		AQLString* rollConventionPtr = nullptr;

		int rollDay = 0;
		AQLString rollConvention = AQLString("");
		bool isEOMRoll = false;
		bool isStartRoll = false;

		// Note: We check for RollDayString = AQLString("0") for backwards compatibility
		if (rollDayString != AQLString("0") && rollDayString.size() != 0)
		{
			//// Generate  Leg Coupon Roll Conventions if the rollDayString populated
			rollDayPtr = &rollDay;
			rollConventionPtr = &rollConvention;

			// The StartDate/EndDate are actually not used in the method, pass in dummy for backward compatibility
			validateAndPopulateRollDayConventions(rollDayString, AQLDate(), AQLDate(), &rollDayPtr, &rollConventionPtr, isEOMRoll, isStartRoll);
		}

		AQLString rollConv = (rollConventionPtr == nullptr) ? "" : *rollConventionPtr;

		return rollConv;

	}

    /* @brief			Calculate an unadjusted date from tenor
	* @param [in]		startDate		    Unadjusted date
    * @param [in]		tenorAdjustment			Tenor Adjustment
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
	* @output			Returns the adjusted date
	*/
    AQLDate getUnadjustedDateFromTenor( const AQLDate& startDate,
                                       const AQLString& tenorAdjustment,
                                       const AQLString& busDayAdj,
                                       const AQLString& calendar,
                                       const AQLString& rollConvention )
    {
        return getAdjustedDate( startDate, tenorAdjustment, "NO_CHANGE", "", rollConvention ); // "" = NO CALENDAR
    }

    /* @brief			Calculate an unadjusted date from a tenor
	* @param [in]		startDate		        start date
    * @param [in]		tenorAdjustment			Tenor Adjustment
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
    * @param [in]		useRollConvention	    boolean to toggle if the roll convention should be used
	* @output			Returns the adjusted date
	*/
    AQLDate getUnadjustedDateFromTenor( const AQLDate& startDate,
                                       const AQLString& tenorAdjustment,
                                       const AQLString& rollConvention,
                                       const bool useRollConvention )
    {
        return getAdjustedDate( startDate, tenorAdjustment, "NO_CHANGE", "", rollConvention, useRollConvention ); // "" = NO CALENDAR
    }

    /* @brief			Calculate an adjusted date from an unadjusted date
	* @param [in]		unadjustedDate		    Unadjusted date
    * @param [in]		tenorAdjustment			Tenor Adjustment
    * @param [in]		busDayAdj			    Swap business date adjustment convention
	* @param [in]		calendar			    Swap calendar
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
	* @output			Returns the adjusted date
	*/
    AQLDate getAdjustedDate( const AQLDate& unadjustedDate,
                            const AQLString& tenorAdjustment,
                            const AQLString& busDayAdj,
                            const AQLString& calendar,
                            const AQLString& rollConvention )
    {
        bool useRollConvention = true;
        if (  rollConvention.size() == 0 || rollConvention == AQLString("0") || rollConvention == AQLString("NORMAL") )
        {
            useRollConvention = false;
        }

        return getAdjustedDate( unadjustedDate, tenorAdjustment, busDayAdj, calendar, rollConvention, useRollConvention );
    }

    /* @brief			Calculate an adjusted date from an unadjusted date
	* @param [in]		unadjustedDate		    Unadjusted date
    * @param [in]		tenorAdjustment			Tenor Adjustment
	* @param [in]		busDayAdj			    Swap business date adjustment convention
	* @param [in]		calendar			    Swap calendar
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
    * @param [in]		useRollConvention	    boolean to toggle if the roll convention should be used
	* @output			Returns the adjusted date
	*/
    AQLDate getAdjustedDate( const AQLDate& unadjustedDate,
                            const AQLString& tenorAdjustment,
                            const AQLString& busDayAdj,
                            const AQLString& calendar,
                            const AQLString& rollConvention,
                            const bool useRollConvention )
    {
        if ( !useRollConvention )
		{
			AQLDate adjustedDate = AQLDateScheduleHelpers::getDate(unadjustedDate, tenorAdjustment, busDayAdj, calendar);
            return adjustedDate;
		}
		else
        {
            DateVector inputs, outputs;
		    inputs.push_back(unadjustedDate);
            outputs = AQLDateScheduleHelpers::getMultiDate( inputs, tenorAdjustment, busDayAdj, calendar, &rollConvention );
            AQLDate adjustedDate = outputs[0];
            return adjustedDate;
        }
    }
    

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
	bool isRegularSwapSchedule( const AQLDate& swapStart,
		                        const AQLDate& swapMaturity,
		                        bool isMaturityAdjusted,
		                        const AQLString& frequency,
		                        const AQLString& busDayAdj,
		                        const AQLString& calendar,
		                        int rollDay,
		                        const AQLString& rollConvention )
	{
        AQLDate unAdjustedSwapStartDate      = swapStart;
        AQLDate unAdjustedSwapEndDate        = swapStart;
		AQLDate adjustedSwapEndInput         = swapMaturity;

        AQLString NO_CHANGE("NO_CHANGE");
        AQLString NO_CALENDAR("");
        AQLString NO_ROLLCONVENTION("");
		
		const AQLString liborTenor = etrading::fromFrequencyToTerm(frequency);
		AQ_THROW_IF( liborTenor == AQLString(), "Invalid Stub Rate or Unknown Libor Tenor in Float Schedule" )

		// If Libor Tenor is 1D or 1W then such a rate is a stub rate, since we don't build 1D or 1W curves
		// Note: OIS rates are Annualized Compounded 1D rates not 1D
		if ( liborTenor == "1D" || liborTenor == "1W" )
		{
			return false;
		}

        bool useRollConvention = true;
        if ( rollConvention.size() == 0 || rollConvention == AQLString("0") || rollConvention == AQLString("NORMAL") )
        {
            useRollConvention = false;
        }

        AQ_REQUIRE( ( useRollConvention && rollDay == 0 ) || !useRollConvention, "Invalid Swap Schedule - Inconsistent roll day and roll convention settings" )
        AQ_REQUIRE( rollDay >=0, "Invalid Swap Schedule - RollDay cannot be negative" )
        
        // All Maturity Dates must be adjusted to compare with an adjusted regular swap end date.
        if ( !isMaturityAdjusted )
		{
			// When maturity date is given as a tenor, the calculated swap end date is always an adjusted date
			// If the given swap end is not already adjusted, it gets adjusted here
            // We adjust for holidays, but be careful not to reapply the roll convention e.g. IMM, EOM
			adjustedSwapEndInput = getAdjustedDate( swapMaturity, "0D", busDayAdj, calendar, NO_ROLLCONVENTION, false ); // false = no roll convention
		}

		// Firstly, test if the maturity lands on the spot Libor tenor.
		// Important note: When maturity is equal to or shorter than the Libor tenor, rollConvention is NOT applied. This means 'IMM' is not used in that case.
		AQLDate liborEndDate = getAdjustedDate( swapStart, liborTenor, busDayAdj, calendar, rollConvention, useRollConvention );
        
        // Swap end date shorter than Libor tenor date so definitely a stub swap
        if ( adjustedSwapEndInput < liborEndDate)
		{
			return false;	
		}

        // For a Regular Normal Roll we can check the Swap End Date Only
        // This does not apply to IMM and EOM roll conventions however - which need start and end date checks
		if (adjustedSwapEndInput == liborEndDate && !useRollConvention )
		{
			return true;	// a regular swap with NO stub
		}

		 // FRONT STUB CHECK
        // ----------------
		
		if ( useRollConvention )
		{
			// Start Dates should always be quoted as adjusted dates
			// Check the Start Date falls on the correct adjusted start date - the roll convention is needed for IMM and EOM checking
			AQLDate swapStart_RollConv = getAdjustedDate( swapStart, "0D", busDayAdj, calendar, rollConvention, useRollConvention );
            
			if ( swapStart_RollConv != swapStart )
			{
				// If the swap start is not on the roll day then we have a front stub i.e. irregular stub
				return false;
			}
		}

        // SEARCH FOR END STUB - Interate and search for regular end stub date (which has no stub)
        // -------------------

		// Secondly, Iterate over each coupon date until we reach a standard maturity tenor that is equal to or longer than actual swap mmaturity date
		// The front stub check above ensures that swap start is adjusted
        AQLDate regularSwapEndWithNoStub = swapStart;
        AQLDate rollDate = swapStart;
		size_t maxInterations = 40000; // While loop guard - max for 100Y of daily coupons
        size_t thisIteration = 0;
		while ( regularSwapEndWithNoStub < adjustedSwapEndInput && thisIteration < maxInterations )
		{
			++thisIteration;

			// Update the Unadjusted start date
			unAdjustedSwapStartDate = unAdjustedSwapEndDate;
            
            // Get the Unadjusted End Date
            unAdjustedSwapEndDate   = getUnadjustedDateFromTenor( rollDate, liborTenor, rollConvention, useRollConvention );
			rollDate				= unAdjustedSwapEndDate;

            // Calculate the adjusted regular swap end, which has no stub.
            regularSwapEndWithNoStub = getAdjustedDate( unAdjustedSwapEndDate, "0D", busDayAdj, calendar, rollConvention, useRollConvention );
        }
        
        // SEARCH END
        // ----------

		// Determine if swap has stub or not
		if( regularSwapEndWithNoStub == adjustedSwapEndInput )
		{
			return true;
		}
		
        // All regular swap tests failed, so return false
        return false;
	}


	/* @brief			Populate schedule outputs from the schedule input
	*  @param [in]		schParams	               Schedule input
	*  @param [in]		checkStub	               True to check if the schedule has stub
	*  @Return			Schedule output
	*/
	EnrichedSchedule populateScheduleDates(const ScheduleParameters& schParams, const bool& checkStub)
	{

		//Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates 
		DateVector accrualDates;
		DateVector paymentDates;
		validateAndGenerateAccrualAndPaymentSchedules(accrualDates,
														paymentDates,
														schParams.adjustedAccrualStartDate().c_str(), //Adjusted effective date is used. 
														schParams.accrualEndDateOrTenor().c_str(),
														toString(schParams.accrualFrequency()).c_str(),
														toString(schParams.accrualbusinessDayAdj()).c_str(),
														schParams.accrualCalendar().c_str(),
														toString(schParams.paymentFrequency()).c_str(),
														toString(schParams.paymentbusinessDayAdj()).c_str(),
														schParams.paymentCalendar().c_str(),
														schParams.payLag().c_str(),
														toString(schParams.stubType()).c_str(),
														schParams.firstStub().c_str(),
														schParams.lastStub().c_str(),
														schParams.rollDayInput().c_str(),
														schParams.fixingAdvanceOrArrears().c_str());

		size_t expectedSize = accrualDates.size() - 1;

		DateVector accrualStartDates;
		accrualStartDates.reserve(expectedSize);

		DateVector accrualEndDates;
		accrualEndDates.reserve(expectedSize);

		validateAndGenerateAccrualStartAndEndDates(accrualStartDates, accrualEndDates, accrualDates);

		double tmpYearFraction = 0.0;

		//int days = 0;

		DoubleVector accrualYearFractions(expectedSize);

		for (size_t i = 0; i < expectedSize; ++i)
		{
			AQLDate accrualStart = accrualStartDates[i];
			AQLDate accrualEnd = accrualEndDates[i];

			// Important Note: If compound interest is being applied then we accrue interest to the
			// payment date ( with no payment lag ) and not the accrual end date.
			bool DONT_INCLUDE_LAST_PAYMENT_DATE = false;
			tmpYearFraction = getYearFraction(accrualStart, accrualEnd, schParams.accrualDaycount(), DONT_INCLUDE_LAST_PAYMENT_DATE);
			
			accrualYearFractions[i] = tmpYearFraction;

			//// Days between accrual start and end dates, not business days
			//days = getDays(accrualStart, accrualEnd);
			//schOutput.accrualDays_.push_back(days);
		}

		bool hasFixing = (schParams.fixingbusinessDayAdj() != NONE_BUSINESS_DAY_ADJ && schParams.fixingCalendar().size() != 0);

		bool isIrregularStub = false;

		DateVector fixingDates;
		DateVector fixingEndDates;

		if (hasFixing)
		{
			fixingDates.reserve(expectedSize);
			fixingEndDates.reserve(expectedSize);

			// 1) If firstFixLag is the not specified, or firstFixLag is the same as fixLag, use the fixLag to calculate fixingDates
			if (schParams.firstFixLag().size() == 0 || same(schParams.fixLag(), schParams.firstFixLag()))
			{
				fixingDates = validateAndGenerateFixingSchedule(accrualDates,
					toString(schParams.fixingbusinessDayAdj()).c_str(),
					schParams.fixingCalendar().c_str(),
					schParams.fixLag().c_str(),
					schParams.fixingAdvanceOrArrears().c_str());
			}
			// 2) If the firstFixLag is specified, use the firstFixLag to calculate the first cashflow's fixingDate, and use fixLag to calculate the rest cashflows
			else
			{

				// First cashflow: accrualDates come in pairs ie a start date and an end date
				if (accrualDates.size() >= 2)
				{

					//fixingDate of the first cashflow, based on the firstFixingLag
					fixingDates = validateAndGenerateFixingSchedule(boost::assign::list_of(accrualDates.at(0))(accrualDates.at(1)),
						toString(schParams.fixingbusinessDayAdj()).c_str(),
						schParams.fixingCalendar().c_str(),
						schParams.firstFixLag().c_str(),
						schParams.fixingAdvanceOrArrears().c_str());

					//excluding the first cashflow's accrualStartDate & accrualEndDate
					accrualDates.erase(accrualDates.begin(), accrualDates.begin() + 1);
				}

				// For the rest cashflows: accrualDates come in pairs ie a start date and an end date
				if (accrualDates.size() >= 2)
				{
					//fixingDates for the remaining cashflows
					auto fixingDates2 = validateAndGenerateFixingSchedule(accrualDates,
						toString(schParams.fixingbusinessDayAdj()).c_str(),
						schParams.fixingCalendar().c_str(),
						schParams.fixLag().c_str(),
						schParams.fixingAdvanceOrArrears().c_str());

					fixingDates.insert(fixingDates.end(), fixingDates2.begin(), fixingDates2.end());
				}

			}

			//Check if the schedule has stub or not
			if (checkStub)
			{
				isIrregularStub = checkScheduleHasStub(schParams);
			}
		
			fixingEndDates = calculateAndPopulateFixingEndDates(accrualEndDates, schParams.fixingAdvanceOrArrears(), schParams.fixingbusinessDayAdj(), schParams.fixingCalendar(), schParams.fixLag());
		}

		//TODO: if it's variable schedule, the spreads are different
		DoubleVector spreads(expectedSize, schParams.spread());

		// Populate the Results Struct
		EnrichedSchedule schOutput(fixingDates, fixingEndDates, paymentDates, accrualStartDates, accrualEndDates, accrualYearFractions, isIrregularStub, spreads);

		return schOutput;
	}

	/* @brief			Check if the schedule has stub
	*  @param [in]		schParams	               Schedule input
	*  @Return			True to indicate the schedule has stub
	*/
	bool checkScheduleHasStub(const ScheduleParameters& schParams)
	{
		// Initialize to false
		bool isIrregularStub = false;

		AQ_REQUIRE(!schParams.accrualStartDate().empty(), "ScheduleParameters Error - No accrualStartDate is provided.")
		AQ_REQUIRE(!schParams.accrualEndDateOrTenor().empty(), "ScheduleParameters Error - No accrualEndDateOrTenor is provided.")

		if (schParams.firstStub().size() != 0 || schParams.lastStub().size() != 0)
		{
			isIrregularStub = true;

			return isIrregularStub;
		}

		//When checking stub, the swap start date should be un-adjusted effective date (i.e. input accrualStartDate_).
		AQLDate swapStart = etrading::stringToDate(schParams.accrualStartDate().c_str(), "#Error: Invalid 'EffectiveDate'.");

		AQLDate unadjustSwapEnd = validateMaturityDate(swapStart, schParams.accrualEndDateOrTenor().c_str());	//getUnadjustedMaturityDate();

		AQLString frequency = getFrequencyString(getFrequencyTenor(schParams.accrualFrequency()));

		// is maturity date given as a tenor?
		bool isMaturityDateTenor = etrading::isMaturityDateTenor(schParams.accrualEndDateOrTenor().c_str());

		// get rollDay
		int* rollDayPtr = nullptr;
		int  rollDay = 0;
		AQLString* rollConventionPtr = nullptr;
		AQLString rollConvention = AQLString("");
		if (same(schParams.rollDayInput(), "0") && schParams.rollDayInput().size() != 0)
		{
			// Generate Fixed Leg Coupon Roll Conventions if the rollDayString is not empty or set to zero
			rollDayPtr = &rollDay;
			rollConventionPtr = &rollConvention;

			bool temp;
			etrading::validateAndPopulateRollDayConventions(schParams.rollDayInput().c_str(), swapStart, unadjustSwapEnd, &rollDayPtr, &rollConventionPtr, temp, temp);
		}

		// is it regular schedule?
		AQLString rollConv = getRollConvection(swapStart, unadjustSwapEnd, schParams.rollDayInput().c_str());
		if (rollDayPtr != nullptr)
		{
			rollDay = *rollDayPtr;
		}

		// Note: Irregular stub, pay attention to the not operator !!!
		isIrregularStub = !isRegularSwapSchedule(swapStart, unadjustSwapEnd, isMaturityDateTenor, frequency, toString(schParams.accrualbusinessDayAdj()).c_str(), schParams.accrualCalendar().c_str(), rollDay, rollConv);

		return isIrregularStub;

	}

	/* @brief			Populate paymentFreqEnum and paymentFreq 
	*  @param [out]		paymentFreqEnum	           Payment Frequency Enum, this is used to distinguish with paymentFreq for zero coupon swap
	*  @param [inout]	paymentFreq	               Payment Frequency
	*  @param [in]		accrualFreq	               Accrual Frequency
	*  @Return			True to indicate the schedule has stub
	*/
	void handlePaymentFrequencies(FrequencyEnum& paymentFreqEnum, FrequencyEnum& paymentFreq, const FrequencyEnum& accrualFreq)
	{
		// The leg can pay AT_MATURITY (in this case the coupon calculation follows the zero coupon swap logic), or pay at other frequencey (in this case the coupon calculation follows normal logic).
		paymentFreqEnum = paymentFreq;

		//Set paymentFreq: If the paymentFreq is 'AT_MATURITY', change it to be the same as accrual frequency, because the 'AT MATURITY' frequency will not impact the schedule, it only impact the coupon calculation
		if (paymentFreq == AT_MATURITY_FREQUENCY)
		{
			paymentFreq = accrualFreq;
		}
	}

	bool isFrontStub(const StubTypeEnum& stubType) 
	{
		if (stubType == NONE_STUBTYPE || stubType == SHORT_START_STUBTYPE || stubType == LONG_START_STUBTYPE)
		{
			return true;
		}
		else if (stubType == SHORT_END_STUBTYPE || stubType == LONG_END_STUBTYPE)
		{
			return false;
		}
		else
		{
			AQ_THROW( "Stub Type must be None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE)." );
		}
	}

	DateVector calculateAndPopulateFixingEndDates(const DateVector& accrualEndDates, const std::string& fixingAdvanceOrArrears, const BusinessDayAdjustmentEnum& fixingbusinessDayAdj, const std::string& fixingCalendar, const std::string& fixLag)
	{
		DateVector accrualDates = accrualEndDates;
		accrualDates.push_back(accrualEndDates.back());

		DateVector fixingEndDates;
		fixingEndDates.reserve(accrualEndDates.size());

		AQ_THROW_IF( !etrading::AQLCurveForwardRateHelpers::isFixingInAdvance(fixingAdvanceOrArrears.c_str()), "For OIS Swap, fixing in arrears is not supported." );

		fixingEndDates = validateAndGenerateFixingSchedule(accrualDates,
			toString(fixingbusinessDayAdj).c_str(),
			fixingCalendar.c_str(),
			fixLag.c_str(),
			"advance");

		return fixingEndDates;

	}





}

