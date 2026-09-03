#include "tryMeProductSwapDV01.h"

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

    /* @brief			return a set of expected keys for swap DV01 label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapDV01LVBKeys()
    {
        const std::string arr[] =
        {
            etrading::MARKET_KEY::CURVE_COLLECTION
            , etrading::MARKET_KEY::FORECAST_CURVE
            , etrading::MARKET_KEY::DISCOUNT_CURVE
            , etrading::IRS_KEY::NOTIONAL
            , etrading::IRS_KEY::PAY_RECEIVE
            , etrading::IRS_KEY::PAYER_RECEIVER
            , etrading::IRS_KEY::FIXED_RATE
            , etrading::IRS_KEY::FLOAT_SPREAD
        };

        std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
        std::vector<std::string> commonKeys = etrading::getSwapCommonLVBKeys();
        expectedKeys.insert( expectedKeys.end(), commonKeys.begin(), commonKeys.end() );

        return expectedKeys;
    }

    /* @brief			validation interface for the meProductSwapDV01 method
    *  @param [in]		swapLVB			A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to TRUE
    *  @return			Swap DV01
    */
    double tryMeProductSwapDV01( const LabelValueBlock& swapLVB, bool validateKeys )
    {
        VALID_EXCEPTION_START

        etrading::validateKeysForLVB( tryMeProductSwapDV01LVBKeys(), swapLVB.getKeys(), validateKeys );

        const std::string inputLVB = "SwapLVB";
        LAString curveCollection = swapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB );

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductSwapDV01_inputs", curveCollection ) );
            file.write( "generatorFunction", "tryMeProductSwapDV01" );
            file.write( "swapLVB", swapLVB );
        }


        //----------------------------------------------------------------------------------
        // Validate non-cash flow related parameters
        LAString forecastCurveIndex	= swapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::FORECAST_CURVE, inputLVB );
        LAString discountCurveIndex = swapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::DISCOUNT_CURVE, inputLVB );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, discountCurveIndex );
        LAString forecastCurvestaticDataTable = etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

        // Synchronize the OIS pricing interpolation with that used within the OIS Curve calibration routine
        LAString interpolation = etrading::getCurveInterpolation( curveCollection, forecastCurvestaticDataTable );

        //----------------------------------------------------------------------------------
        // Validate swap trade specific parameters

        // Payer or Receiver Swap i.e. Pay or Receive Fixed Coupons
        LAString payRec	= swapLVB.getCompulsoryValueAsLAStringFromKeys( etrading::IRS_KEY::PAYER_RECEIVER, etrading::IRS_KEY::PAY_RECEIVE , inputLVB );
        bool isFixedRatePayerSwap = etrading::validateSwapPayRecFlag( payRec );

        //----------------------------------------------------------------------------------
        // Validate cash flow related parameters and generate cash flows

        DateVector fixedAccrualDates;
        DateVector fixedPaymentDates;
        DateVector floatAccrualDates;
        DateVector floatFixingDates;
        DateVector floatPaymentDates;
        etrading::validateAndGenerateSwapCashflows( swapLVB, inputLVB, fixedAccrualDates, fixedPaymentDates, floatFixingDates, floatAccrualDates, floatPaymentDates );

        //----------------------------------------------------------------------------------
        // Get the Swap DV01

        // Float Leg Fixing Parameters
        double floatFirstFix = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_FIRSTFIXING, std::numeric_limits<double>::quiet_NaN() );
        double floatLastFix	= swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_LASTFIXING, std::numeric_limits<double>::quiet_NaN() );
        bool useFirstFixing = etrading::useFloatFixing( floatFirstFix );
        bool useLastFixing = etrading::useFloatFixing( floatLastFix );

		const std::string fwdInter = swapLVB.getOptionalValueAsLAString( etrading::IRS_KEY::IS_FWD_INTER, "").getCString();

        // Get the Swap DV01
        bool isFwdInter = etrading::getfwdInterInfo( curveCollection, forecastCurvestaticDataTable, etrading::toBooleanEnum(fwdInter)  ).isFwdInter;

        double notional = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::NOTIONAL );
        double fixedRate = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FIXED_RATE );
        double floatSpread = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_SPREAD );
        LAString fixedDayCount = swapLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::FIXED_DAYCOUNT );
        LAString floatDayCount = swapLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::FLOAT_DAYCOUNT );

        double ret = CurveInstrumentPricing::getSwapDV01( isFixedRatePayerSwap,
                     notional,
                     fixedAccrualDates,
                     fixedPaymentDates,
                     floatFixingDates,
                     floatAccrualDates,
                     floatPaymentDates,
                     etrading::getDataInstance(),
                     curveCollection,
                     fixedRate,
                     fixedDayCount,
                     floatSpread,
                     floatDayCount,
                     interpolation,
                     forecastCurveIndex,
                     discountCurveIndex,
                     isFwdInter,	// isFwdInter. An obsolete parameter for OIS pricing
                     useFirstFixing,
                     floatFirstFix,
                     useLastFixing,
                     floatLastFix );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductSwapDV01_outputs", curveCollection ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}
