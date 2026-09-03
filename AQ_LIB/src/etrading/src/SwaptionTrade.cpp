/*
* @brief			Class the defines the Swaption Trade
* @Created:			18 July 2018
* @Author:			Ian Castleton
* @Department:		Quant Research & Analytics
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

#include "SwaptionTrade.h"

#include "ExceptionMacros.h"
#include "CoreEnumerations.h"
#include "SwapUtilities.h"
#include "ScheduleValidation.h"

namespace etrading
{
	// Main constructor
	SwaptionTrade::SwaptionTrade( const std::string& objectName, const LabelValueBlock& dealLVB ) : Option(objectName)
	{
        // Validate Object Type
        const std::string inputLVB = "swaptionLVB";
        CachedObjectEnum objectType = toCachedObjectEnum( dealLVB.getCompulsoryValueAsString( OBJECT_KEY::OBJECT_TYPE, inputLVB ) );
        MLIB_REQUIRE( objectType == OPTION, "Invalid Object Type - 'OPTION' type required" )
        
        // Validate Swaption Style: European, Bermudan, American
		optionStyle_    = toOptionStyleEnum(dealLVB.getCompulsoryValueAsString(OPTION_KEYS::OPTION_STYLE, inputLVB));
        MLIB_REQUIRE( optionStyle_ == EUROPEAN_OPTION, "Invalid Option Stlye, Only EUROPEAN swaptions supported" )
        

        // Parse the input parameter block
		// ===============================
        inputParameters_ = dealLVB;
        

		const std::string inputCurrency = dealLVB.getCompulsoryValueAsString( IRS_KEY::CURRENCY, inputLVB );

		currency_           = toCCYEnum( inputCurrency);
		valuationCurrency_  = toCCYEnum( dealLVB.getOptionalValueAsString( IRS_KEY::VALUATION_CURRENCY, inputCurrency ) );
		
        longShort_          = toLongShortPositionEnum( dealLVB. getOptionalValueAsString( SWAPTION_KEYS::LONG_SHORT, "LONG" ) );
        payerReceiver_      = toPayerReceiverSwaptionEnum( dealLVB.getCompulsoryValueAsString( SWAPTION_KEYS::PAYER_RECEIVER, inputLVB ) );
		notional_           = dealLVB.getCompulsoryValueAsDouble( IRS_KEY::NOTIONAL );
		leverage_           = dealLVB.getOptionalValueAsDouble( IRS_KEY::LEVERAGE, 1.0 );
		strike_             = dealLVB.getCompulsoryValueAsDoubleFromKeys( IRS_KEY::STRIKE_RATE, OPTION_KEYS::STRIKE, inputLVB );
        
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( notional_ ), "Invalid Notional - Trade Notional cannot be negative or zero")

        tenorDescription_   = dealLVB.getOptionalValueAsString( SWAPTION_KEYS::TENOR_DESCRIPTION, "" );
		optionDayCount_     = toDayCountEnum( dealLVB.getOptionalValueAsString( OPTION_KEYS::DAYCOUNT, "" ) );

        // Swaption settlement: PriceCash ParYieldCash, BilateralPhysical, ClearedPhysical
		settlementType_     = toSettlementTypeEnum( dealLVB.getCompulsoryValueAsString(SWAPTION_KEYS::SETTLEMENT_TYPE, inputLVB ) );

        // Option Premium and Fees
        // =======================================================================
        fee_                    = dealLVB.getOptionalValueAsDouble( SWAPTION_KEYS::FEE, 0.0 );
        feeDate_                = dealLVB.getOptionalValueAsDate( SWAPTION_KEYS::FEE_DATE, LADate() );
        feePayReceive_          = toPayReceiveEnum( dealLVB.getOptionalValueAsString( SWAPTION_KEYS::FEE_PAY_RECEIVE, toString( etrading::NONE_PAYRECEIVE_ENUM ) ) );
        feePayReceiveIndicator_ = 0.0;

        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( fee_ ), "Fees cannot be negative - Use the FeePayReceive parameter to specify if the fee is to be paid or received.")
    
        if ( !MLIB_IS_EQUAL_ZERO( fee_ ) ) 
        {
            MLIB_REQUIRE( feeDate_ != LADate(), "Fee Date required - Must provide a feeDate when specifying fee payments")

            if ( feePayReceive_ == etrading::PAY_PAYRECEIVE_ENUM )
            {
                feePayReceiveIndicator_ = -1.0;
            }
            else if ( feePayReceive_ == etrading::RECEIVE_PAYRECEIVE_ENUM )
            {
                feePayReceiveIndicator_ = 1.0;
            }
            else
            {
                MLIB_THROW( "Invalid or missing FeePayReceive parameter - FeePayReceive must be set to Pay or Receive" )
            }
        }


        // Option Expiry and Underlying Swap Dates Can be in Date or Tenor Format
        // ======================================================================
        
        // Unadjusted expiry Date - Effective Date cannot be a tenor
        std::string effectiveDateString     = dealLVB.getCompulsoryValueAsString( OPTION_KEYS::EFFECTIVE_DATE, inputLVB );
        effectiveDate_                      = stringToDate( effectiveDateString.c_str(), "Invalid Effective Date" );

        std::string optionExpiryString      = dealLVB.getCompulsoryValueAsString( OPTION_KEYS::EXPIRY, inputLVB );
        std::string swapStartString         = dealLVB.getCompulsoryValueAsString( SWAPTION_KEYS::SWAP_START, inputLVB );
        std::string swapEndString           = dealLVB.getCompulsoryValueAsString( SWAPTION_KEYS::SWAP_END, inputLVB );
        
        bool isOptionExpiryTenor            = isDateTenor( optionExpiryString );
        bool isSwapStartTenor               = isDateTenor( swapStartString );
        bool isSwapEndTenor                 = isDateTenor( swapEndString );
        
        // Default Values for busDayAdj_ and calendar_
        busDayAdj_                          = NONE_BUSINESS_DAY_ADJ;
        calendar_                           = "";

        if ( isOptionExpiryTenor || isSwapStartTenor || isSwapEndTenor )
        {
            // We require the business day adjustment and calendar paramters to convert to dates from tenor strings
            busDayAdj_  = toBusinessDayAdjustmentEnum( dealLVB.getCompulsoryValueAsString( SWAPTION_KEYS::BUSINESSDAYADJUSTMENT, inputLVB ) );
            calendar_   = dealLVB.getCompulsoryValueAsString( SWAPTION_KEYS::CALENDAR, inputLVB );
        }
        else
        {
            // BusinessDayAdjustment and Calendar information are optional if providing the Swaption Expiry, Swap Start and Swap End dates explicitly
            busDayAdj_  = toBusinessDayAdjustmentEnum( dealLVB.getOptionalValueAsString( SWAPTION_KEYS::BUSINESSDAYADJUSTMENT, "NONE" ) );
            calendar_   = dealLVB.getOptionalValueAsString( SWAPTION_KEYS::CALENDAR, "" );
        }
        
        // Option Expiry Date or Tenor
		if ( isOptionExpiryTenor )
        {
            adjustedOptionExpiryDate_       = validateDateOrTenor( effectiveDate_, optionExpiryString.c_str(), toString( busDayAdj_ ).c_str(), calendar_.c_str() );
        }
        else
        {
            adjustedOptionExpiryDate_       = stringToDate( optionExpiryString.c_str(), "Invalid Option Expiry Date" );
        }
        
        // Apply the Option Notification Day Lag to the Option Expiry
        notificationDays_       = dealLVB.getOptionalValueAsString( SWAPTION_KEYS::NOTIFICATION_DAYS, "0D" );
        if ( notificationDays_ != "0D" )
        {
            // The notification tenor string should be negative, add the -ve prefix
            MLIB_REQUIRE( isDateTenor( notificationDays_ ), "Invalid Notification Days: Must specify a tenorString e.g. 2D" )
            
            LAString notificationTenorString = LAString("-") + LAString( notificationDays_.c_str() );
            adjustedOptionExpiryDate_        = validateDateOrTenor( adjustedOptionExpiryDate_, notificationTenorString, toString( busDayAdj_ ).c_str(), calendar_.c_str()  );
        }

        // Swap Start Date or Tenor
        if ( isSwapStartTenor )
        {
            swapStartDate_      = validateDateOrTenor( effectiveDate_, swapStartString.c_str(), toString( busDayAdj_ ).c_str(), calendar_.c_str() );
        }
        else
        {
            swapStartDate_      = stringToDate( swapStartString.c_str(), "Invalid Swap Start Date" );
        }

        // Swap End Date or Tenor
        if ( isSwapEndTenor )
        {
            swapMaturityDate_   = validateDateOrTenor( swapStartDate_, swapEndString.c_str(), toString( busDayAdj_ ).c_str(), calendar_.c_str() );
        }
        else
        {
            swapMaturityDate_   = stringToDate( swapEndString.c_str(), "Invalid Swap End Date" );
        }

        MLIB_REQUIRE( swapStartDate_ >= adjustedOptionExpiryDate_, "Invalid Swap Start Date: The swap start date cannot be before the option expiry date" )


		// Underlying Swap parameters
		// ===========================
        swapGeneratorString_ = dealLVB.getCompulsoryValueAsString( SWAPTION_KEYS::SWAP_GENERATOR, inputLVB );
        
		// Create the swapExpressionLVB
        // Note we use SwapLVB from above, which is a copy of dealLVB. We need to modify the swapStart if it is quoted as a tenor,
        // since the underlying swap only accepts swap start dates and not tenor strings
		const LabelValueBlock& swapExpressionLVB = setupSwapExpressionLVBforSwaption( swapStartDate_, dealLVB );

		// Create the underlying swap
		LabelValueBlock swapPropertiesLVB;
		bool isXccySwap = false;
		underlyingSwap_ = createSwapFromGenerator( GENERATOR_COMPONENTS::KEY_SWAPS, swapGeneratorString_, swapExpressionLVB, swapPropertiesLVB, isXccySwap );
		MLIB_REQUIRE( underlyingSwap_ != nullptr, "Unable to build swap from generator." );

	}

	// Copy constructor
	SwaptionTrade::SwaptionTrade(const SwaptionTrade& rhs) : Option(rhs),
                                                             optionStyle_( rhs.optionStyle_ ),
                                                             longShort_( rhs.longShort_ ),
															 payerReceiver_( rhs.payerReceiver_ ),
															 settlementType_( rhs.settlementType_ ),
															 strike_( rhs.strike_ ),
                                                             effectiveDate_(rhs.effectiveDate_),
															 adjustedOptionExpiryDate_( rhs.adjustedOptionExpiryDate_ ),
                                                             tenorDescription_( rhs.tenorDescription_ ),
                                                             notificationDays_( rhs.notificationDays_ ),
															 optionDayCount_( rhs.optionDayCount_ ),
															 currency_( rhs.currency_ ),
															 valuationCurrency_( rhs.valuationCurrency_ ),
															 notional_( rhs.notional_ ),
															 leverage_( rhs.leverage_ ),
                                                             swapGeneratorString_( rhs.swapGeneratorString_ ),
															 underlyingSwap_( rhs.underlyingSwap_ ),
															 swapStartDate_( rhs.swapStartDate_ ),
															 swapMaturityDate_( rhs.swapMaturityDate_ ),
                                                             busDayAdj_( rhs.busDayAdj_ ),
                                                             calendar_( rhs.calendar_ ),
                                                             daycount_( rhs.daycount_ ),
                                                             feePayReceive_( rhs.feePayReceive_ ),
                                                             feePayReceiveIndicator_( rhs.feePayReceiveIndicator_ ),
                                                             fee_( rhs.fee_ ),
                                                             feeDate_ ( rhs.feeDate_ )
	{}

	std::shared_ptr<Option> SwaptionTrade::clone()
	{
		SwaptionTrade temp(*this);
		return std::make_shared<SwaptionTrade>(temp);
	}

		
	/* @brief Populates a default swap expression label value block, used for building the Swap instrument underlier for the swaption
	* @param [in]   asOfDate		The asOf / valuation date
    * @param [in]   swapStartDate   The Swap Start date. We explicitly provide the date, because the underlying swap cannot imply the start date if provided as a tenor, since it is the base / reference date.
    * @param [in]   deaLVB	        The deal LVB used for building the Swap
	*/
	LabelValueBlock SwaptionTrade::setupSwapExpressionLVBforSwaption( const LADate& swapStartDate, const LabelValueBlock& dealLVB ) const
	{
		// Set up the Swap Expression LVB used for repricing swap calibration instruments
		LAStringVector keys; 
		LAStringVector values;
		const std::string inputLVB = "dealLVB";

		// Standardized trade keys because we are only interested in calculating par rate and annuity 
	    keys.push_back( IRS_KEY::PAY_RECEIVE.c_str() );					values.push_back( LAString( "PAY" ) );
        keys.push_back( IRS_KEY::NOTIONAL.c_str() );					values.push_back( dealLVB.getCompulsoryValueAsLAString(IRS_KEY::NOTIONAL ) );
		keys.push_back( IRS_KEY::EFFECTIVE_DATE.c_str() );				values.push_back( swapStartDate.stringWithFormat() );
        keys.push_back( IRS_KEY::MATURITY_DATE.c_str() );				values.push_back( dealLVB.getCompulsoryValueAsLAString( SWAPTION_KEYS::SWAP_END, inputLVB ) );
		keys.push_back( SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1.c_str() );	values.push_back( LAString( "0.0" ) );
		keys.push_back( SWAP_EXPRESSION_KEY::RATE_OR_SPREAD2.c_str() );	values.push_back( LAString( "0.0" ) );

		LabelValueBlock swapExpressionLVB = etrading::populateLabelValueBlock( keys, values );
        return swapExpressionLVB;			
	}


}
