#include "tryAqOISParRate.h"

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

namespace validation
{

    /* @brief			return a set of expected keys for ois swap par rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqOISParRateLVBKeys()
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

    /* @brief			validation interface for the aqOISParRate method
    *  @param [in]		oisLVB		A label value block defining the ois swap
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
    *  @return			OIS Par Swap Rate
    */
    double tryAqOISParRate( const LabelValueBlock& oisLVB, bool validateKeys )
    {
        VALID_EXCEPTION_START

        etrading::validateKeysForLVB( tryAqOISParRateLVBKeys(), oisLVB.getKeys(), validateKeys );

        const std::string inputLVB = "OISLVB";
        AQLString curveCollection = oisLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB );

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryAqOISParRate_inputs", curveCollection ) );
            file.write( "generatorFunction", "tryAqOISParRate" );
            file.write( "oisLVB", oisLVB );
        }


        //----------------------------------------------------------------------------------
        // Validate non-cash flow related parameters

        AQLString forecastCurveIndex	= oisLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::FORECAST_CURVE, inputLVB );
        AQLString discountCurveIndex = oisLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::DISCOUNT_CURVE, inputLVB );

        //Throw exception if the curve has not been built.
        AQLString staticDataTable = etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

        // Synchronize the OIS pricing interpolation with that used within the OIS Curve calibration routine
        AQLString interpolation = etrading::getCurveInterpolation( curveCollection, staticDataTable );

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

        AQLString floatLegRollDayString      = oisLVB.getOptionalValueAsAQLString( etrading::IRS_KEY::FLOAT_ROLLDAY );
        AQLString compoundingMethod          = oisLVB.getOptionalValueAsAQLString( etrading::PRICING_PARAMS::COMPOUND_METHOD );
        etrading::validateOISParameters( floatLegRollDayString, compoundingMethod, false );

        AQLString floatBusinessDayAdjustment             = oisLVB.getOptionalValueAsAQLString( etrading::IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT                                        );
        AQLString floatCalendar	                        = oisLVB.getOptionalValueAsAQLString( etrading::IRS_KEY::FLOAT_CALENDAR                                                     );
        AQLString floatAccrualBusinessDayAdjustment   = oisLVB.getOptionalValueAsAQLString( etrading::IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT,    floatBusinessDayAdjustment );
        AQLString floatAccrualCalendar	            = oisLVB.getOptionalValueAsAQLString( etrading::IRS_KEY::FLOAT_ACCRUALCALENDAR,                 floatCalendar              );

        AQLString fixedDayCount              = oisLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::FIXED_DAYCOUNT );
        AQLString floatDayCount              = oisLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::FLOAT_DAYCOUNT );

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
            CreateDataFile file( decorateFilename( "tryAqOISParRate_outputs", curveCollection ) );
            file.write( "output", ret, 12 );
        }

        return ret;

        VALID_EXCEPTION_END
    }


}
