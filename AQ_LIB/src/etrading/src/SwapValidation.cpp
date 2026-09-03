#include "SwapValidation.h"
#include "ParameterValidation.h"

#include "LADateScheduleHelpers.h"
#include "LACurveForwardRateHelpers.h"
#include "LAPriceDataCalendar.h"
#include <cctype>
#include "LAMarketData.h"
#include "LAPriceDataInterpolation.h"
#include "CommonConstants.h"
#include <utility>
#include "LAStaticData.h"
#include <cmath>
#include <boost/algorithm/string.hpp>

namespace etrading
{

    /* @brief				validate OIS specific parameters
    * @param [in,out]		slidingRule			Sliding rule required by the underlying compounding method
    * @param [in,out]		compoundingMethod	OIS compounding method
    * @param [in]			eomRoll				Do we do EOM rolling?
    * @output				A boolean indicating the fixed leg direction
    */
    void validateOISParameters( LAString& slidingRule, LAString& compoundingMethod, bool eomRoll )
    {
		if (compoundingMethod.size() != 0)
		{
			if ( boost::iequals( compoundingMethod.getCString(), "Arithmetic" ) )
			{
				compoundingMethod = "AVERAGE";
			}
			else if ( boost::iequals( compoundingMethod.getCString(), "Geometric" ) )
			{
				compoundingMethod = "NORMAL";
			}
			else if ( !( boost::iequals( compoundingMethod.getCString(), "FLAT" ) || boost::iequals( compoundingMethod.getCString(), "SIMPLE" ) ) )
			{
				throw LACoreInvalidData( "#Error: Invalid compounding method, 'CompMethod' should either be 'Arithmetic', 'Geometric', 'FLAT', or 'SIMPLE'", __FILE__, __LINE__ );
			}
		}

        if ( eomRoll )
        {
            slidingRule = "EOM";
        }
    }

    /* @brief			Validate maturity string and convert it to LADate. As a market convention, maturity date is NOT adjusted for holiday, so businessAdjustment and calendar are not required.
    * @param [in]		effectiveDate		The start date
    * @param [in]		maturityDateStr	    Maturity Date in string format, can be a date or tenor
    * @output			maturity date in LADate format
    */
    LADate validateMaturityDate( const LADate& effectiveDate, const LAString& maturityDateStr )
    {
        LADate maturityDate;
        bool isMaturityInDateFormat = true;
        try
        {
            maturityDate = stringToDate( maturityDateStr, "#Error: Invalid 'MaturityDate'." );
        }
        catch ( LACoreError& )
        {
            isMaturityInDateFormat = false;
        }

        // Check if Maturity End Date is a Tenor or Date and Convert to a Date, if required
        if( !isMaturityInDateFormat )
        {
            // *** As a market convention, maturity date is NOT adjusted for holiday
            AQ_REQUIRE( LADateScheduleHelpers::isValidDate( effectiveDate ), "Invalid Date: Unable to convert MaturityTenor to a date" )
            maturityDate = LADateScheduleHelpers::getDate( effectiveDate,
                                                         maturityDateStr,		// maturity is Tenor
                                                         LAString(),         // BusinessDayAdjustment is NO_CHANGE
                                                         LAString());       // Calendar is NONE
        }
        
        AQ_REQUIRE( LADateScheduleHelpers::isValidDate( maturityDate ), "Invalid Maturity Date " + maturityDateStr )
        return maturityDate;
    }

    /* @brief			Validate maturity string and convert it to LADate, given the effective date as a string
    * @param [in]		effectiveDateStr		The start date as a string
    * @param [in]		maturityDateStr 	    Maturity Date in string format, can be a date or tenor
    * @output			maturity date in LADate format
    */
    LADate validateMaturityDate( const LAString& effectiveDateStr, const LAString& maturityDateStr )
    {
        // Convert the Effective Date from a string to a date
        LADate effectiveDate = stringToDate( effectiveDateStr, "#Error: Invalid 'EffectiveDate'." );

        // Convert the Maturity Date from a String to a Date
        LADate maturityDate = validateMaturityDate( effectiveDate, maturityDateStr );
        return maturityDate;
    }

    /* @brief			Validate maturity string and convert it to LAString, given the effective date as a string
    * @param [in]		effectiveDateStr	The start date as a string
    * @param [in]		maturityDateStr		Maturity Date in string format, can be a date or tenor
    * @output			maturity date in LADate format
    */
    LAString validateMaturityDateString( const LAString& effectiveDateStr, const LAString& maturityDateStr )
    {
        // Convert the Maturity Date from a String to a Date
        LAString maturityDate = validateMaturityDate( effectiveDateStr, maturityDateStr ).stringWithFormat( "YYYYMMDD" );
        return maturityDate;
    }

	/* @brief			Validate if maturity string is in date format or tenor format
    * @param [in]		maturtiyStr			Maturity in string format
    * @output			TRUE for tenor format
    */
	bool isMaturityDateTenor(const LAString& maturityStr)
	{
		bool isMaturityInDateFormat = true;
        try
        {
            LADate maturityDate = stringToDate( maturityStr, "#Error: Invalid 'MaturityDate'." );
        }
        catch ( LACoreError& )
        {
            isMaturityInDateFormat = false;
        }

		return !isMaturityInDateFormat;
	}

    /* @brief			validate the payRec flag for swap
    * @param [in]		payRec		A string representing the fixed leg direction
    * @output			A boolean indicating the fixed leg direction
    */
    bool validateSwapPayRecFlag( const LAString& payRec )
    {
        bool isFixedRatePayerSwap = true;

        LAString payerReceiver = LAString( payRec ).toUpper();

        if ( payerReceiver == "PAYER" || payerReceiver == "PAY" || payerReceiver == "P" )
        {
            isFixedRatePayerSwap = true;
        }
        else if ( payerReceiver == "RECEIVER" || payerReceiver == "RECEIVE" || payerReceiver == "REC" || payerReceiver == "R" )
        {
            isFixedRatePayerSwap = false;
        }
        else
        {
            AQ_THROW("Must specify if the Swap is a Payer (PAY) or Receiver (REC).")
        }

        return isFixedRatePayerSwap;
    }

	/* @brief			Indicator for payer or receiver
    * @param [in]		payRec		A string representing the fixed leg direction
    * @output			Return -1 for payer, 1 for receiver
    */
	int getIndicatorFromPayRec(const PayReceiveEnum& payRec )
	{
		if( payRec == PAY_PAYRECEIVE_ENUM)
        {
            return -1;
        }
		else if( payRec == RECEIVE_PAYRECEIVE_ENUM)
        {
            return 1;
        }
		else
		{
            AQ_THROW("Must specify if the Swap is a Payer (PAY) or Receiver (REC).")
		}
	}

    /* @brief			Validate if a Swap rollDay is a day of the month or a roll convention i.e. IMM, EOM.
    *                   The function will populate and synchronize the multiple parameters which support
    *                   the chosen roll day or convention
    *
    *  @param [in]		rollDayString       A string representing the roll day or convention, can be a day of the month or a convention; IMM, EOM or NULL
    *  @param [in]		startDate           The Swap accrual start date ( Effective Date )
    *  @param [in]		endDate             The Swap accrual end date ( Maturity Date )
    *  @param [out]		rollDay             An integer *** pointer *** representing a day of the month; 1-31
    *  @param [out]     rollConvention      A string *** pointer *** representing the roll convention; IMM, EOM
    *  @param [out]     isEndOfMonthRoll    Are coupons rolling on the end of the month? True or False
    *  @param [out]     isStartRoll         Are we rolling forwards from the start? True = coupons roll from the start date i.e. ShortEnd stub, False = coupons roll from the end date i.e. ShortStart stub
    */
    void validateAndPopulateRollDayConventions( const LAString& rollDayStr, const LADate& startDate, const LADate& endDate, int** rollDay, LAString** rollConvention, bool& isEndOfMonthRoll, bool& isStartRoll )
    {
        LAString rollDayString = rollDayStr;
        rollDayString.toUpper();

        if ( rollDayString.size() == 0 )
        {
            // If rollDayString is Empty or Null
            // ---------------------------------
            // Do nothing, leave values as already set in their default state
            rollDayString = LAString( "NORMAL" );
        }

        if ( isValid< int >( rollDayString.getCString() ) || isValid< double >( rollDayString.getCString() ) )
        {
            // If rollDayString is Numeric
            // ---------------------------

            // Convert the rollDayString to a rollDayInteger
            std::string::size_type size_t_alias;
            ** rollDay = std::stoi( rollDayString.getCString(), &size_t_alias );

            // Check the rollDayInteger is Valid
            if ( **rollDay < 1 || **rollDay > 31 )
            {
                LAString msg = "#Error: The rollDay must be a day of the month i.e. a number from 1 to 31 or a convention e.g. 'IMM' or 'EOM'.";
                throw LACoreInvalidData( msg.getCString(), __FILE__, __LINE__ );
            }

            // Update Roll Conventions
            *rollConvention         = nullptr;              // Not Used
            isEndOfMonthRoll        = false;                // Not Used
            isStartRoll             = false;                // Not Used
        }
        else
        {
            // If rollDayString is Non-Numeric
            // -------------------------------

            if ( rollDayString == LAString( "IMM" ) )
            {
                // Roll Day is ignored when the roll convention is IMM
                *rollDay            = nullptr; 				// Not Used
                **rollConvention    = LAString( "IMM" );    // IMM Roll
                isEndOfMonthRoll    = false;                // Not Rolling on End of Month
                isStartRoll         = false;                // ShortStart Stub
            }
            //else if ( rollDayString == LAString( "START" ) )
            //{
            //    // "START" This means use the start date for the rollDay and roll forwards i.e. short end stub
            //    **rollDay           = startDate.dayOfMonth();
            //    ** rollConvention    = LAString( "START" ); // Roll Forwards from the Start Date i.e. ShortEnd Stub
            //    isEndOfMonthRoll    = false;                // Not Rolling on End of Month
            //    isStartRoll         = true;                 // ShortEnd Stub
            //}
            //else if ( rollDayString == LAString( "END" ) )
            //{
            //    // "END" This means use the end date for the rollDay and roll backwards i.e. short start stub
            //    // Please do not confuse with EOM i.e. End of month roll
            //    **rollDay           = endDate.dayOfMonth();
            //    ** rollConvention    = LAString( "END" );   // Roll Backwards from the End Date i.e. ShortStart Stub
            //    isEndOfMonthRoll    = false;                // Not Rolling on End of Month
            //    isStartRoll         = false;                // ShortStart Stub
            //}
            else if ( rollDayString == LAString( "NORMAL" ) )
            {
                // NORMAL: Do nothing ... Kept for backwards compaitbility purposes
                // Normal indicates that the analytics should imply the correct roll day and other stub settings
                *rollDay            = nullptr;
                *rollConvention     = nullptr;              // Normal Roll i.e. do nothing
                isEndOfMonthRoll    = false;                // Not Rolling on End of Month
                isStartRoll         = false;                // ShortStart Stub
            }
            else if ( rollDayString == LAString( "EOM" ) )
            {
                *rollDay            = nullptr;              // Not Used
                **rollConvention    = LAString( "EOM" );    // End of Month Roll
                isEndOfMonthRoll    = true;                 // Rolling on End of Month
                isStartRoll         = false;                // ShortStart Stub
            }
            else
            {
                // Possible choices of roll convention are: IMM and EOM (End-Of-Month)
                LAString msg = "#Error: The rollDay must be a day of the month i.e. a  from 1 to 31 or a convention e.g. 'IMM' or 'EOM'.";
                throw LACoreInvalidData( msg.getCString(), __FILE__, __LINE__ );
            }
        }
    }

    /* @brief			Check if float fixing should be use
    *  @param [in]		floatFixing	 Float leg's first/last fixing
    *  @return			True if the fixing is not zero
    */
    bool useFloatFixing( double floatFixing )
    {
        if ( boost::math::isnan( floatFixing ) )
        {
            return false;
        }
        return true;
    }

    /* @brief			Helper function to round to zero decimal 
    *  @param [in]		val	        Value 
    *  @return			rounded value
    */
    double round( double val )
    {
        double newVal = ( val < 0 ) ? ceil(val - 0.5) : floor(val + 0.5);
        return newVal;
    }

	/* @brief			Round to the zero decimal based on currency 
    *  @param [in]		val	        Value 
    *  @param [in]		currency	Currency
    *  @return			rounded value
    */
    double roundToNearest(double val, const CCY& currency)
    {
        if ( currency != JPY )
        {
            return val;
        }

        double newVal =  round(val);
        return newVal;
    }

    /* @brief			Round the value to the given decimal  
    *  @param [in]		val	        Value 
    *  @param [in]		decimal     decimal
    *  @return			rounded value
    */
    double roundToDecimal(double val, int decimal)
    {
        double factor = pow (10.0, decimal);
        double newVal = round( val * factor ) / factor;
        return newVal;
    }

    /* @brief			Truncate the value to the given decimal  
    *  @param [in]		val	        Value 
    *  @param [in]		decimal		decimal
    *  @return			truncated value
    */
    double truncateToDecimal(double val, int decimal)
    {
        double factor = pow (10.0, decimal);
        double newVal = floor( val * factor ) / factor;
        return newVal;
    }

    /* @brief			Validate two swap legs' currecies matching the isXccySwap flag
    * @param [in]		isXccySwap  True to indicate xccy swap
    * @param [in]		leg1Currency  Leg1 currency
    * @param [in]		leg2Currency  Leg2 currency
    */
    void validateSwapCurrency(bool isXccySwap, const CCY& leg1Currency, const CCY& leg2Currency)
    {
        bool sameCurrency = ( leg1Currency == leg2Currency );

        if (!isXccySwap && !sameCurrency)
        {
            throw LACoreInvalidData( "#Error: For Single Currency Swap, two legs should have same Currency", __FILE__, __LINE__ );
        }
        if (isXccySwap && sameCurrency)
        {
            throw LACoreInvalidData( "#Error: For Cross Currency Swap, two legs should have different Currencies", __FILE__, __LINE__ );
        }
    }

    /* @brief			Validate two swap legs' currecies matching the isXccySwap flag
    * @param [in]		isXccySwap  True to indicate xccy swap
    * @param [in]		leg1LVB  Leg1 label value block
    * @param [in]		leg2LVB  Leg2 label value block
    */
    void validateSwapCurrency(bool isXccySwap, const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB)
    {
        auto leg1Currency =   toCCYEnum( leg1LVB.getOptionalValueAsLAString( etrading::IRS_KEY::CURRENCY ).getCString() );
        auto leg2Currency =   toCCYEnum( leg2LVB.getOptionalValueAsLAString( etrading::IRS_KEY::CURRENCY ).getCString() );

        validateSwapCurrency(isXccySwap, leg1Currency, leg2Currency);
    }

}