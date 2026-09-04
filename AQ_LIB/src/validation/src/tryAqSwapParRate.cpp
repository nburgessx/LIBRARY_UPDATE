#include "tryAqSwapParRate.h"

#include "CurveInstrumentPricing.h"

#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "AQLDateScheduleHelpers.h"
#include "CommonConstants.h"

namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			return a set of expected keys for swap par rate label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryAqSwapParRateLVBKeys()
    {
        const std::string arr[] =
        {
            etrading::MARKET_KEY::CURVE_COLLECTION
            , etrading::MARKET_KEY::FORECAST_CURVE
            , etrading::MARKET_KEY::DISCOUNT_CURVE
            , etrading::IRS_KEY::FLOAT_SPREAD
        };

        std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
        std::vector<std::string> commonKeys = etrading::getSwapCommonLVBKeys();
        expectedKeys.insert( expectedKeys.end(), commonKeys.begin(), commonKeys.end() );

        return expectedKeys;
    }

    /* @brief			validation interface for the aqSwapParRate method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Par Swap Rate
    */
    double tryAqSwapParRate( const LabelValueBlock& swapLVB, bool validateKeys )
    {
        VALID_EXCEPTION_START

        etrading::validateKeysForLVB( tryAqSwapParRateLVBKeys(), swapLVB.getKeys(), validateKeys );

        const std::string inputLVB = "SwapLVB";
        AQLString curveCollection = swapLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductSwapParRate_inputs", curveCollection ) );
            file.write( "generatorFunction", "tryMeProductSwapParRate" );
            file.write( "swapLVB", swapLVB );
        }

        AQLString forecastCurveIndex	= swapLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::FORECAST_CURVE, inputLVB );
        AQLString discountCurveIndex = swapLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::DISCOUNT_CURVE, inputLVB );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, discountCurveIndex );
        AQLString forecastCurveStaticDataTable = etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

        // Synchronize the OIS pricing interpolation with that used within the OIS Curve calibration routine
        AQLString interpolation = etrading::getCurveInterpolation( curveCollection, forecastCurveStaticDataTable );

        DateVector fixedAccrualDates;
        DateVector fixedPaymentDates;
        DateVector floatAccrualDates;
        DateVector floatFixingDates;
        DateVector floatPaymentDates;
        etrading::validateAndGenerateSwapCashflows( swapLVB, inputLVB, fixedAccrualDates, fixedPaymentDates, floatFixingDates, floatAccrualDates, floatPaymentDates );

        double floatSpread = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_SPREAD );
        AQLString fixedDayCount = swapLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::FIXED_DAYCOUNT );
        AQLString floatDayCount = swapLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::FLOAT_DAYCOUNT );

        // Float Leg Fixing Parameters
        double floatFirstFix = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_FIRSTFIXING, std::numeric_limits<double>::quiet_NaN() );
        double floatLastFix	= swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_LASTFIXING, std::numeric_limits<double>::quiet_NaN() );
        bool useFirstFixing = etrading::useFloatFixing( floatFirstFix );
        bool useLastFixing = etrading::useFloatFixing( floatLastFix );

		const std::string fwdInter = swapLVB.getOptionalValueAsAQLString( etrading::IRS_KEY::IS_FWD_INTER, "").getCString();

        //----------------------------------------------------------------------------------
        // Get the Swap Par Rates using the full swap definition for both legs of the swap

        etrading::FwdInterInfo info = etrading::getfwdInterInfo( curveCollection, forecastCurveStaticDataTable, etrading::toBooleanEnum(fwdInter)  );
        bool isFwdInter = info.isFwdInter;
        bool useFwdData = info.useFwdData;

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
                     isFwdInter,
                     useFirstFixing,
                     floatFirstFix,
                     useLastFixing,
                     floatLastFix,
                     floatSpread,
                     useFwdData
                                                     );
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductSwapParRate_outputs", curveCollection ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}

