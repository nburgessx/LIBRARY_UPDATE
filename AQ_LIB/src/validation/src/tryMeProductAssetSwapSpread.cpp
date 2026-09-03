#include "tryMeProductAssetSwapSpread.h"

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

    /* @brief			return a set of expected keys for asset swap spread label value block
    *  @return			expected keys
    */
    std::vector<std::string> tryMeProductAssetSwapSpreadLVBKeys()
    {
        const std::string arr[] =
        {
            etrading::MARKET_KEY::CURVE_COLLECTION
            , etrading::MARKET_KEY::FORECAST_CURVE
            , etrading::MARKET_KEY::DISCOUNT_CURVE
            , etrading::IRS_KEY::FIXED_RATE
            , etrading::IRS_KEY::EFFECTIVE_DATE
            , etrading::ASSET_SWAP_KEY::IS_CLEAN_PRICE
            , etrading::ASSET_SWAP_KEY::ISSUE_DATE
        };

        std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
        std::vector<std::string> commonKeys = etrading::getSwapCommonLVBKeys();
        expectedKeys.insert( expectedKeys.end(), commonKeys.begin(), commonKeys.end() );

        return expectedKeys;
    }

    /* @brief			Validate the Fixed/Float Frequency for Accrued Interest Start Date Calculation
    * @param [in]		frequency	Fixed/Float Frequencey
    * @param [in]		err			If err is not empty, throw it
    */
    void validateAssetSwapFrequency( const AQLString& frequency, const AQLString& err )
    {
        AQLString accruedInterestTerm = AQLString();
        AQLString freq = AQLString( frequency ).toUpper();

        if ( freq == "ANNUAL" )
        {
            accruedInterestTerm = "-12M";
        }
        else if ( freq == "SEMI-ANNUAL" )
        {
            accruedInterestTerm = "-6M";
        }
        else if ( freq == "QUARTERLY" )
        {
            accruedInterestTerm = "-3M";
        }
        else if ( freq == "MONTHLY" )
        {
            accruedInterestTerm = "-1M";
        }
        else if ( freq == "WEEKLY" )
        {
            accruedInterestTerm = "-1W";
        }
        else
        {
            throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
        }
    }

    /* @brief			validation interface for the meProductAssetSwapSpread method
    *  @param [in]		bondPrice	        Bond Price, can be clean or dirty, but must specify in the assetSwapLVB
    *  @param [in]		assetSwapLVB		A label value block defining an asset swap
    *  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
    *  @return			Asset Swap Spread
    */
    double tryMeProductAssetSwapSpread( double bondPrice, const LabelValueBlock& assetSwapLVB, bool validateKeys)
    {
        VALID_EXCEPTION_START

        etrading::validateKeysForLVB( tryMeProductAssetSwapSpreadLVBKeys(), assetSwapLVB.getKeys(), validateKeys );

        const std::string inputLVB = "AssetSwapLVB";
        AQLString curveCollection = assetSwapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB );

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductAssetSwapSpread_inputs", curveCollection ) );
            file.write( "generatorFunction", "tryMeProductAssetSwapSpread" );
            file.write( "bondPrice", bondPrice );
            file.write( "assetSwapLVB", assetSwapLVB );
            file.write( "validateKeys", validateKeys );
        }

        //----------------------------------------------------------------------------------
        // Validate non-cash flow related parameters
        AQLString forecastCurveIndex	= assetSwapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::FORECAST_CURVE, inputLVB );
        AQLString discountCurveIndex = assetSwapLVB.getCompulsoryValueAsLAString( etrading::MARKET_KEY::DISCOUNT_CURVE, inputLVB );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, discountCurveIndex );
        AQLString forecastCurveStaticDataTable = etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

        // Synchronize the Asset Swap pricing interpolation with that used within the Forecast Curve calibration routine
        AQLString interpolation = etrading::getCurveInterpolation( curveCollection, forecastCurveStaticDataTable );

        //----------------------------------------------------------------------------------
        // Validate cash flow related parameters and generate cash flows
        DateVector fixedAccrualDates;
        DateVector fixedPaymentDates;
        DateVector floatAccrualDates;
        DateVector floatFixingDates;
        DateVector floatPaymentDates;

        const AQLString fixingInAdvance = "ADVANCE";
        const bool isAssetSwap = true;

        etrading::validateAndGenerateSwapCashflows( assetSwapLVB, inputLVB, fixedAccrualDates, fixedPaymentDates, floatFixingDates, floatAccrualDates, floatPaymentDates, fixingInAdvance, isAssetSwap );

        if ( fixedAccrualDates.size() < 1 || floatAccrualDates.size() < 1 )
        {
            throw AQLCoreInvalidData( "#Error: Swap schedule error. There must be at least 1 fixed and float coupon.", __FILE__, __LINE__ );
        }

        AQLString fixedFreq = assetSwapLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::FIXED_FREQUENCY );
        AQLString floatFreq = assetSwapLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::FLOAT_FREQUENCY );
        validateAssetSwapFrequency( fixedFreq, "#Error: Invalid 'FixedFrequency'. Available frequencies are Annual, Semi-Annual, Quarterly, Monthly and Weekly." );
        validateAssetSwapFrequency( floatFreq, "#Error: Invalid 'FloatFrequency'. Available frequencies are Annual, Semi-Annual, Quarterly, Monthly and Weekly." );

        double fixedRate = assetSwapLVB.getCompulsoryValueAsDouble( etrading::IRS_KEY::FIXED_RATE );

        AQLString fixedDayCount = assetSwapLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::FIXED_DAYCOUNT );
        AQLString floatDayCount = assetSwapLVB.getCompulsoryValueAsLAString( etrading::IRS_KEY::FLOAT_DAYCOUNT );

        double floatFirstFix = assetSwapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_FIRSTFIXING, std::numeric_limits<double>::quiet_NaN() );
        double floatLastFix	= assetSwapLVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_LASTFIXING, std::numeric_limits<double>::quiet_NaN() );

        // Float Leg Fixing Parameters
        bool useFirstFixing = etrading::useFloatFixing( floatFirstFix );
        bool useLastFixing = etrading::useFloatFixing( floatLastFix );

		const std::string fwdInter = assetSwapLVB.getOptionalValueAsLAString( etrading::IRS_KEY::IS_FWD_INTER, "").getCString();

        bool isFwdInter = etrading::getfwdInterInfo( curveCollection, forecastCurveStaticDataTable, etrading::toBooleanEnum(fwdInter) ).isFwdInter;

        // Asset Swap Parameters
        const bool isCleanPrice     = assetSwapLVB.getCompulsoryValueAsBool( etrading::ASSET_SWAP_KEY::IS_CLEAN_PRICE );
        const AQLDate issueDate      = assetSwapLVB.getCompulsoryValueAsDate( etrading::ASSET_SWAP_KEY::ISSUE_DATE );
        const AQLDate settlementDate = assetSwapLVB.getCompulsoryValueAsDate( etrading::IRS_KEY::EFFECTIVE_DATE );


        double ret = CurveInstrumentPricing::getAssetSwapSpread( bondPrice,
                     fixedAccrualDates,
                     fixedPaymentDates,
                     floatFixingDates,
                     floatAccrualDates,
                     floatPaymentDates,
                     etrading::getDataInstance(),
                     curveCollection,
                     fixedRate,
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
                     isCleanPrice,
                     settlementDate );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file1( decorateFilename( "tryMeProductAssetSwapSpread_outputs", curveCollection ) );
            file1.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }



}








