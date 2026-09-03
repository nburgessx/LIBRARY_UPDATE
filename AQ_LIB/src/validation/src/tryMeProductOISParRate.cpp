/*
 * @brief			validation interface for tryMeProductOISParRate
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "tryMeProductOISParRate.h"

#include "CurveInstrumentPricing.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{

    /* @brief			return a set of expected keys for ois swap par rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductOISParRateLVBKeys()
    {
        const std::string arr[] =
        {
            etrading::MARKET_KEY::CURVE_COLLECTION
            , etrading::MARKET_KEY::FORECAST_CURVE
            , etrading::MARKET_KEY::DISCOUNT_CURVE
            , etrading::IRS_KEY::FLOAT_SPREAD
            , etrading::PRICING_PARAMS::COMPOUND_METHOD
        };

        std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
        std::vector<std::string> commonKeys = etrading::getSwapCommonLVBKeys();
        expectedKeys.insert( expectedKeys.end(), commonKeys.begin(), commonKeys.end() );

        return expectedKeys;
    }

    /* @brief			validation interface for the meProductOISParRate method
    *  @param [in]		oisLVB		A label value block defining the ois swap
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
    *  @return			OIS Par Swap Rate
    */
    double tryMeProductOISParRate( const LabelValueBlock& oisLVB, bool validateKeys )
    {
        VALID_EXCEPTION_START

        etrading::validateKeysForLVB( tryMeProductOISParRateLVBKeys(), oisLVB.getKeys(), validateKeys );

        const std::string inputLVB = "OISLVB";
        LAString curveCollection = oisLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB );

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductOISParRate_inputs", curveCollection ) );
            file.write( "generatorFunction", "tryMeProductOISParRate" );
            file.write( "oisLVB", oisLVB );
        }


        //----------------------------------------------------------------------------------
        // Validate non-cash flow related parameters

        LAString forecastCurveIndex	= oisLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::FORECAST_CURVE, inputLVB );
        LAString discountCurveIndex = oisLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::DISCOUNT_CURVE, inputLVB );

        //Throw exception if the curve has not been built.
        LAString staticDataTable = etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

        // Synchronize the OIS pricing interpolation with that used within the OIS Curve calibration routine
        LAString interpolation = etrading::getCurveInterpolation( curveCollection, staticDataTable );

        //----------------------------------------------------------------------------------
        // Validate cash flow related parameters and generate cash flows
        DateVector fixedAccrualDates;
        DateVector fixedPaymentDates;
        DateVector floatAccrualDates;
        DateVector floatFixingDates;
        DateVector floatPaymentDates;
        etrading::validateAndGenerateSwapCashflows( oisLVB, inputLVB, fixedAccrualDates, fixedPaymentDates, floatFixingDates, floatAccrualDates, floatPaymentDates );

        //----------------------------------------------------------------------------------
        // Validate OIS specific parameters

        LAString floatLegRollDayString      = oisLVB.getOptionalValueAsLAString( etrading::IRS_KEY::FLOAT_ROLLDAY );
        LAString compoundingMethod          = oisLVB.getOptionalValueAsLAString( etrading::PRICING_PARAMS::COMPOUND_METHOD );
        etrading::validateOISParameters( floatLegRollDayString, compoundingMethod, false );

        LAString floatBusinessDayAdjustment             = oisLVB.getOptionalValueAsLAString( etrading::IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT                                        );
        LAString floatCalendar	                        = oisLVB.getOptionalValueAsLAString( etrading::IRS_KEY::FLOAT_CALENDAR                                                     );
        LAString floatAccrualBusinessDayAdjustment   = oisLVB.getOptionalValueAsLAString( etrading::IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT,    floatBusinessDayAdjustment );
        LAString floatAccrualCalendar	            = oisLVB.getOptionalValueAsLAString( etrading::IRS_KEY::FLOAT_ACCRUALCALENDAR,                 floatCalendar              );

        LAString fixedDayCount              = oisLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::FIXED_DAYCOUNT );
        LAString floatDayCount              = oisLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::FLOAT_DAYCOUNT );

        double floatSpread                  = oisLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_SPREAD );

        double floatFirstFix                = oisLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_FIRSTFIXING, std::numeric_limits<double>::quiet_NaN() );
        double floatLastFix	                = oisLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_LASTFIXING, std::numeric_limits<double>::quiet_NaN() );
        bool useFirstFixing                 = etrading::useFloatFixing( floatFirstFix );
        bool useLastFixing                  = etrading::useFloatFixing( floatLastFix );

        //----------------------------------------------------------------------------------
        // Get the Swap Par Rates using the full swap definition for both legs of the swap

        double ret = CurveInstrumentPricing::getParRate( fixedAccrualDates,
                     fixedPaymentDates,
                     floatFixingDates,
                     floatAccrualDates,
                     floatPaymentDates,
                     etrading::getDataInstance(),
                     curveCollection,
                     fixedDayCount,
                     floatDayCount,
                     interpolation,
                     forecastCurveIndex,
                     discountCurveIndex,
                     false,		// Optional: interpFwds. This input is obsolete to OIS pricing
                     useFirstFixing,
                     floatFirstFix,
                     useLastFixing,
                     floatLastFix,
                     floatSpread,
                     false,		// Optional: useFwdData. This input is obsolete to OIS pricing
                     true,		 // isOIS
                     compoundingMethod,
                     floatAccrualCalendar,
                     floatAccrualBusinessDayAdjustment,
                     floatLegRollDayString );
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductOISParRate_outputs", curveCollection ) );
            file.write( "output", ret, 12 );
        }

        return ret;

        VALID_EXCEPTION_END
    }


}
