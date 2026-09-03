// JPYLCHCurveConsistency.cpp

/*
 * @brief			JPY Curve Consistency Tests
 * @Created:		25th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "CoreEnumerations.h"
#include "tryMeLWO.h"
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"
#include "BuildSwapTradeFromGenerator.h"
#include "ExtractCurveCalibrationData.h"
#include "RepriceCalibrationInstruments.h"

// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***

const std::string TEST_FOLDER = "ETrading/Curves/JPYCurves/";

// Test Files
const std::string fileNameJPYOIS = TEST_FOLDER + "JPY_LCH_OIS_CURVE.JSON";
const std::string fileNameJPY6ML = TEST_FOLDER + "JPY_LCH_SWAP_6M_CURVE.JSON";
const std::string fileNameJPY3ML = TEST_FOLDER + "JPY_LCH_BASIS_3M_CURVE.JSON";
const std::string fileNameJPY1ML = TEST_FOLDER + "JPY_LCH_BASIS_1M_CURVE.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class JPYLCH_Curve_Consistency : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        // Load Curves - Must Build Curves in the Correct Order
        const std::string curveObjectJPYOIS_        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameJPYOIS, etrading::JSON );
        const std::string curveObjectJPYStd6ML_     = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameJPY6ML, etrading::JSON );
        const std::string curveObjectJPYBasis3ML_   = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameJPY3ML, etrading::JSON );
        const std::string curveObjectJPYBasis1ML_   = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameJPY1ML, etrading::JSON );
    };

    // OIS OUTRIGHT SWAPS
    TEST_F( JPYLCH_Curve_Consistency, CONSISTENCY_JPYLCH_OIS_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::OIS_MARKETDATA,    // Calibration Instrument: OIS Outright Swaps
                                       {"1W", "1M", "2M", "3M", "4M", "5M", "6M", "9M", "1Y", "18M", "2Y" }, // Calibration Tenors
                                       curveObjectJPYOIS_,        // LWOCurveName - Extract Calibration Data from this curve object
                                       "JPYYC_LCH",               // CurveCollection - For Repricing
                                       "20180829",                // Swap Effective Date
                                       "JPY_OIS",                 // Swap Generator
                                       1.0e-6 );                  // Test Tolerance
    }

    //// LIBOR-OIS BASIS SWAPS
    //TEST_F( JPYLCH_Curve_Consistency, CONSISTENCY_JPYLCH_LIBOROIS_Instrument_Repricing_Using_Macro )
    //{
    //    // Function will throw on failure or if no tests are run
    //    repriceCalibrationInstruments( etrading::CurveMarketDataEnum::LIBOR_OIS_BASISSPREAD_MARKETDATA,    // Calibration Instrument: LIBOR-OIS Basis Swaps
    //                                   { "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y", "25Y", "40Y" },  // Calibration Tenors
    //                                   curveObjectJPYOIS_,        // LWOCurveName - Extract Calibration Data from this curve object
    //                                   "JPYYC_LCH",               // CurveCollection - For Repricing
    //                                   "20180829",                // Swap Effective Date
    //                                   "JPY_BASIS_LIBOROIS",      // Swap Generator
    //                                   1.0e-5 );                  // Test Tolerance
    //}

    // STD INTEREST RATE SWAPS
    TEST_F( JPYLCH_Curve_Consistency, CONSISTENCY_JPYLCH_SWAP6ML_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::SWAP_MARKETDATA,    // Calibration Instrument: STD Interest Rate Swap
                                       { "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "11Y", "12Y", "15Y", "20Y", "25Y", "30Y", "40Y", "50Y" }, // Calibration Tenors                                  
                                       curveObjectJPYStd6ML_,      // LWOCurveName - Extract Calibration Data from this curve object
                                       "JPYYC_LCH",                // CurveCollection - For Repricing
                                       "20180829",                 // Swap Effective Date
                                       "JPY_6ML",                  // Swap Generator
                                       1.0e-6 );                   // Test Tolerance
    }

    // TENOR BASIS SWAPS 3ML
    TEST_F( JPYLCH_Curve_Consistency, CONSISTENCY_JPYLCH_BASIS3ML_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::BASIS_SWAP_MARKETDATA,    // Calibration Instrument: Tenor Basis Swap
                                       { "1Y", "18M", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" }, // Calibration Tenors                                  
                                       curveObjectJPYBasis3ML_,     // LWOCurveName - Extract Calibration Data from this curve object
                                       "JPYYC_LCH",                 // CurveCollection - For Repricing
                                       "20180829",                  // Swap Effective Date
                                       "JPY_BASIS_3X6",             // Swap Generator
                                       1.0e-6 );                    // Test Tolerance
    }

    // TENOR BASIS SWAPS 1ML
    TEST_F( JPYLCH_Curve_Consistency, CONSISTENCY_JPYLCH_BASIS1ML_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::BASIS_SWAP_MARKETDATA,    // Calibration Instrument: Tenor Basis Swap
                                       { "1Y", "18M", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" }, // Calibration Tenors                                  
                                       curveObjectJPYBasis1ML_,      // LWOCurveName - Extract Calibration Data from this curve object
                                       "JPYYC_LCH",                  // CurveCollection - For Repricing
                                       "20180829",                   // Swap Effective Date
                                       "JPY_BASIS_1X6",              // Swap Generator
                                       1.0e-6 );                     // Test Tolerance
    }
}