#include "InterestRateSwap.h"
#include "AQLDateScheduleHelpers.h"
#include "AQLCurveForwardRateHelpers.h"
#include "CommonConstants.h"
#include "AQLCoreComponentManager.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "ConstantDeclarations.h"
#include "AQLCurvePricingObject.h"
#include "AQLPriceDataSlidingRule.h"

namespace etrading
{
    /* @brief			Constructor
    *  @param [in]		A string matrix that defines an interest swap
    */
    InterestRateSwap::InterestRateSwap( const LabelValueBlock& swapLVB )
        : BaseInstrument( swapLVB ), forecastCurve_( "" ), discountCurve_( "" ), interpolation_( "" )
    {
        const std::string nameCollectionName = "SwapSpecification";

        //-------------------------------------
        // Non leg specific parameters

        // compulsory
        notional_		                        = swapLVB.getCompulsoryValueAsDouble(      IRS_KEY::NOTIONAL,                               nameCollectionName         );
        effectiveDate_	                        = swapLVB.getCompulsoryValueAsAQLString(    IRS_KEY::EFFECTIVE_DATE,                         nameCollectionName         );
        maturityDate_				            = swapLVB.getCompulsoryValueAsAQLString(    IRS_KEY::MATURITY_DATE,                          nameCollectionName         );
        
        AQLString payRec	                        = swapLVB.getCompulsoryValueAsAQLStringFromKeys( etrading::IRS_KEY::PAYER_RECEIVER, etrading::IRS_KEY::PAY_RECEIVE, nameCollectionName );
        isPayerSwap_	                        = validateSwapPayRecFlag(              payRec                                                                           );


        //-------------------------------------
        // Fixed leg parameters

        // Compulsory
        fixedRate_				                = swapLVB.getCompulsoryValueAsDouble(       IRS_KEY::FIXED_RATE,                            nameCollectionName         );
        fixedFrequency_			                = swapLVB.getCompulsoryValueAsAQLString(     IRS_KEY::FIXED_FREQUENCY,                       nameCollectionName         );
        fixedDayCount_			                = swapLVB.getCompulsoryValueAsAQLString(     IRS_KEY::FIXED_DAYCOUNT,                        nameCollectionName         );

        // Optional
        fixedStubType_			                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_STUBTYPE                                                     );
        fixedFirstStubDate_		                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_FIRSTSTUBDATE                                                );
        fixedLastStubDate_		                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_LASTSTUBDATE                                                 );
        fixedPaymentLag_		                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_PAYMENTLAG,                      "0D"                        );
        fixedRollDayString_		                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FIXED_ROLLDAY                                                      );
        // Business Day Adjustments
        // ------------------------
        // Note: We can choose to use a single business day adjustment and calendar for the fixed leg or specify the accrual and payment business
        // day adjustments individually. If the individual business day adjustments and calendars are missing the global one will be used. An error will
        // be thrown if none of these are specified, requesting at the mimimum that we populate the global parameters.

        fixedBusinessDayAdjustment_             = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FIXED_BUSINESSDAYADJUSTMENT                                        );
        fixedCalendar_	                        = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FIXED_CALENDAR                                                     );
        fixedAccrualBusinessDayAdjustment_      = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FIXED_ACCRUALBUSINESSDAYADJUSTMENT,    fixedBusinessDayAdjustment_ );
        fixedAccrualCalendar_	                = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FIXED_ACCRUALCALENDAR,                 fixedCalendar_              );
        fixedPaymentBusinessDayAdjustment_      = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FIXED_PAYMENTBUSINESSDAYADJUSTMENT,    fixedBusinessDayAdjustment_ );
        fixedPaymentCalendar_	                = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FIXED_PAYMENTCALENDAR,                 fixedCalendar_              );

        if ( fixedAccrualBusinessDayAdjustment_.size() == 0 || fixedPaymentBusinessDayAdjustment_.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: 'FixedBusinessDayAdjustment' must be specified.", __FILE__, __LINE__ );
        }

        if ( fixedAccrualCalendar_.size() == 0 || fixedPaymentCalendar_.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: 'FixedCelendar' must be specified.", __FILE__, __LINE__ );
        }

        //-------------------------------------
        // Float leg parameters

        // Compulsory
        floatFrequency_			                = swapLVB.getCompulsoryValueAsAQLString(     IRS_KEY::FLOAT_FREQUENCY,                       nameCollectionName         );
        floatDayCount_			                = swapLVB.getCompulsoryValueAsAQLString(     IRS_KEY::FLOAT_DAYCOUNT,                        nameCollectionName         );


        // Optional
        floatStubType_			                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_STUBTYPE                                                     );
        floatFirstStubDate_		                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_FIRSTSTUBDATE                                                );
        floatLastStubDate_		                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_LASTSTUBDATE                                                 );
        floatPaymentLag_		                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_PAYMENTLAG,                      "0D"                        );
        floatFixingLag_			                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_FIXINGLAG,                       "0D"                        );
        floatFirstFixing_		                = swapLVB.getOptionalValueAsDouble(        IRS_KEY::FLOAT_FIRSTFIXING, std::numeric_limits<double>::quiet_NaN()        );
        floatLastFixing_		                = swapLVB.getOptionalValueAsDouble(        IRS_KEY::FLOAT_LASTFIXING, std::numeric_limits<double>::quiet_NaN()         );
        floatSpread_			                = swapLVB.getOptionalValueAsDouble(        IRS_KEY::FLOAT_SPREAD                                                       );
        floatRollDayString_		                = swapLVB.getOptionalValueAsAQLString(      IRS_KEY::FLOAT_ROLLDAY                                                      );
        // Business Day Adjustments
        // ------------------------
        // Note: We can choose to use a single business day adjustment and calendar for the float leg or specify the fixing, accrual and payment business
        // day adjustments individually. If the individual business day adjustments and calendars are missing the global one will be used. An error will
        // be thrown if none of these are specified, requesting at the mimimum that we populate the global parameters.

        floatBusinessDayAdjustment_             = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT                                        );
        floatCalendar_	                        = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FLOAT_CALENDAR                                                     );
        floatFixingBusinessDayAdjustment_       = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FLOAT_FIXINGBUSINESSDAYADJUSTMENT,     floatBusinessDayAdjustment_ );
        floatFixingCalendar_	                = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FLOAT_FIXINGCALENDAR,                  floatCalendar_              );
        floatAccrualBusinessDayAdjustment_      = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT,    floatBusinessDayAdjustment_ );
        floatAccrualCalendar_	                = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FLOAT_ACCRUALCALENDAR,                 floatCalendar_              );
        floatPaymentBusinessDayAdjustment_      = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FLOAT_PAYMENTBUSINESSDAYADJUSTMENT,    floatBusinessDayAdjustment_ );
        floatPaymentCalendar_	                = swapLVB.getOptionalValueAsAQLString(       IRS_KEY::FLOAT_PAYMENTCALENDAR,                 floatCalendar_              );

        if ( floatFixingBusinessDayAdjustment_.size() == 0 || floatAccrualBusinessDayAdjustment_.size() == 0 || floatPaymentBusinessDayAdjustment_.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: 'FloatBusinessDayAdjustment' must be specified.", __FILE__, __LINE__ );
        }

        if ( floatFixingCalendar_.size() == 0 || floatAccrualCalendar_.size() == 0 || floatPaymentCalendar_.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: 'FloatCalendar' must be specified.", __FILE__, __LINE__ );
        }

        // Generate the Swap Schedule
        // --------------------------
        etrading::validateAndGenerateSwapCashflows( fixedAccrualDates_,
                fixedPaymentDates_,
                floatFixingDates_,
                floatAccrualDates_,
                floatPaymentDates_,
                effectiveDate_,
                maturityDate_,
                fixedFrequency_,
                fixedDayCount_,
                fixedAccrualBusinessDayAdjustment_,
                fixedAccrualCalendar_,
                fixedPaymentBusinessDayAdjustment_,
                fixedPaymentCalendar_,
                fixedFirstStubDate_,
                fixedLastStubDate_,
                fixedRollDayString_,
                fixedPaymentLag_,
                fixedStubType_,
                floatFrequency_,
                floatDayCount_,
                floatFixingBusinessDayAdjustment_,
                floatFixingCalendar_,
                floatAccrualBusinessDayAdjustment_,
                floatAccrualCalendar_,
                floatPaymentBusinessDayAdjustment_,
                floatPaymentCalendar_,
                floatFirstStubDate_,
                floatLastStubDate_,
                floatRollDayString_,
                floatFixingLag_,
                floatFirstFixing_,
                floatLastFixing_,
                floatPaymentLag_,
                floatStubType_ );

        // Populate the Accrual Start and End Dates for the Fixed and Floating Legs
        etrading::validateAndGenerateAccrualStartAndEndDates( fixedAccrualStartDates_, fixedAccrualEndDates_, fixedAccrualDates_ );
        etrading::validateAndGenerateAccrualStartAndEndDates( floatAccrualStartDates_, floatAccrualEndDates_, floatAccrualDates_ );
    }


    /* @brief		Method that sets market data
    */
    void InterestRateSwap::setMarketData( const LabelValueBlock& marketDataLVB )
    {
        curveSet_		= marketDataLVB.getCompulsoryValueAsAQLString( MARKET_KEY::CURVE_COLLECTION, "CurveCollections", false );
        forecastCurve_	= marketDataLVB.getCompulsoryValueAsAQLString( MARKET_KEY::FORECAST_CURVE, "CurveCollections", false );
        discountCurve_	= marketDataLVB.getCompulsoryValueAsAQLString( MARKET_KEY::DISCOUNT_CURVE, "CurveCollections", false );

        if ( forecastCurve_.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: Forecast curve has not been specified for PV", __FILE__, __LINE__ );
        }

        if ( discountCurve_.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: Discount curve has not been specified for PV", __FILE__, __LINE__ );
        }
    }

    /* @brief		Set interpolation for pricing
    *  @param [in]	interpolation	Yield curve interpolation
    */
    void InterestRateSwap::setInterpolation( const AQLString& interpolation )
    {
        interpolation_ = interpolation;
        interpolation_.toUpper();
        if ( interpolation_.size() == 0 )
        {
            interpolation_ = "SPLINE";
        }
    }

}

