#include "tryMeProductSwapPV01.h"
#include "AQLCurveForwardRateHelpers.h"
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

    /* @brief			return a set of expected keys for swap pv01 label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductSwapPV01LVBKeys()
    {
        const std::string arr[] =
        {
            etrading::MARKET_KEY::CURVE_COLLECTION
            , etrading::MARKET_KEY::FORECAST_CURVE
            , etrading::MARKET_KEY::DISCOUNT_CURVE
            , etrading::IRS_KEY::NOTIONAL
        };

        std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
        std::vector<std::string> commonKeys = etrading::getSwapCommonLVBKeys();
        expectedKeys.insert( expectedKeys.end(), commonKeys.begin(), commonKeys.end() );

        return expectedKeys;
    }

    /* @brief			validation interface for the meProductSwapPV01 method
    *  @param [in]		swapLVB		A label value block defining the swap
    *  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
    *  @return			Swap PV01
    */
    double tryMeProductSwapPV01( const LabelValueBlock& swapLVB, bool validateKeys )
    {
        VALID_EXCEPTION_START

        etrading::validateKeysForLVB( tryMeProductSwapPV01LVBKeys(), swapLVB.getKeys(), validateKeys );

        const std::string inputLVB = "SwapLVB";
        AQLString curveCollection = swapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB );

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductSwapPV01_inputs", curveCollection ) );
            file.write( "generatorFunction", "tryMeProductSwapPV01" );
            file.write( "swapLVB", swapLVB );
        }

        AQLString forecastCurveIndex	= swapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::FORECAST_CURVE, inputLVB );
        AQLString discountCurveIndex = swapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::DISCOUNT_CURVE, inputLVB );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, discountCurveIndex );
        AQLString staticDataTable = etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

        // Synchronize the OIS pricing interpolation with that used within the OIS Curve calibration routine
        AQLString interpolation = etrading::getCurveInterpolation( curveCollection, staticDataTable );

        // Generate Swap Schedule with validation
        DateVector fixedAccrualDates;
        DateVector fixedPaymentDates;
        DateVector floatAccrualDates;
        DateVector floatFixingDates;
        DateVector floatPaymentDates;
        etrading::validateAndGenerateSwapCashflows( swapLVB, inputLVB, fixedAccrualDates, fixedPaymentDates, floatFixingDates, floatAccrualDates, floatPaymentDates );

        AQLString payRec	= swapLVB.getCompulsoryValueAsLAStringFromKeys( etrading::IRS_KEY::PAYER_RECEIVER, etrading::IRS_KEY::PAY_RECEIVE , inputLVB );
        bool isFixedRatePayerSwap = etrading::validateSwapPayRecFlag( payRec );

        double notional = swapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::NOTIONAL );
        AQLString fixedDayCount = swapLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::FIXED_DAYCOUNT );

        // Get the Swap PV01
        double ret = etrading::AQLCurveForwardRateHelpers::getSwapPV01( isFixedRatePayerSwap,
                                                                  notional,
                                                                  fixedAccrualDates,
                                                                  fixedPaymentDates,
                                                                  etrading::getDataInstance(),
                                                                  curveCollection,
                                                                  fixedDayCount,
                                                                  interpolation,
                                                                  forecastCurveIndex,
                                                                  discountCurveIndex );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductSwapPV01_outputs", curveCollection ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }
}

