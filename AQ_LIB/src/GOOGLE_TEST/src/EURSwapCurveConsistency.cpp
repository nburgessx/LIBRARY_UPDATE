// EURSwapCurveConsistency.cpp

/*
 * @brief			EUR Curve Consistency Tests
 * @Created:		8th August 2018
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
#include "ResultsProcessor.h"
#include "CoreEnumerations.h"
#include "tryMeLWO.h"
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"
#include "BuildSwapTradeFromGenerator.h"
#include "ExtractCurveCalibrationData.h"
#include "RepriceCalibrationInstruments.h"
#include "DataUtilities.h"

// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***

const std::string TEST_FOLDER = "ETrading/Curves/EURCurves/";

// Test Files
const std::string fileNameEUROIS = TEST_FOLDER + "EUR_OIS_CURVE.JSON";
const std::string fileNameEUR3ML = TEST_FOLDER + "EUR_SWAP_3M_CURVE.JSON";
const std::string fileNameEUR6ML = TEST_FOLDER + "EUR_SWAP_6M_CURVE.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class EUR_SwapCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectEUROIS_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUROIS, etrading::JSON );
        const std::string curveObjectEUR3ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR3ML, etrading::JSON );
        const std::string curveObjectEUR6ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR6ML, etrading::JSON );
    };


    // Call Test Fixture Class
    TEST_F( EUR_SwapCurve_Consistency, CONSISTENCY_EUR3ML_Swap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::SWAP_MARKETDATA,    // Calibration Instrument: STD Interest Rate Swap
                                       { "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "11Y", "12Y", "15Y", "20Y", "25Y", "30Y", "40Y", "50Y" }, // Calibration Tenors                                  
                                       curveObjectEUR3ML_,                               // LWOCurveName
                                       "EURYC",                                          // CurveCollection
                                       "20180814",                                       // Swap Effective Date
                                       "EUR_3ML",                                        // Swap Generator
                                       1.0e-6 );                                         // Test Tolerance
    }
    
    // Call Test Fixture Class
    TEST_F( EUR_SwapCurve_Consistency, CONSISTENCY_EUR3ML_Swap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    swapTerms   = curveCalibrationSwapTenors( curveObjectEUR3ML_ ); 
        std::vector<double>         parRates    = curveCalibrationSwapParRates( curveObjectEUR3ML_ );
        MLIB_REQUIRE( swapTerms.size() == parRates.size(), "Inconsistent Calibration Data: Inconsistent Number of Swap Terms and Par Rates" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < swapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "11Y", "12Y", "15Y", "20Y", "25Y", "30Y", "40Y", "50Y" };
            auto it = calibrationInstruments.find( swapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "EUR3ML_Swap" + MLIB_TO_STRING_FROM_SIZE_T(i);
            const std::string swapGenerator             = "EUR_3ML";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("EURYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", swapTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = parRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
    
    // Call Test Fixture Class
    TEST_F( EUR_SwapCurve_Consistency, CONSISTENCY_EUR6ML_Swap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::SWAP_MARKETDATA,    // Calibration Instrument: STD Interest Rate Swap
                                       { "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "11Y", "12Y", "15Y", "20Y", "25Y", "30Y", "40Y", "50Y" }, // Calibration Tenors                                  
                                       curveObjectEUR6ML_,                               // LWOCurveName
                                       "EURYC",                                          // CurveCollection
                                       "20180814",                                       // Swap Effective Date
                                       "EUR_6ML",                                        // Swap Generator
                                       1.0e-6 );                                         // Test Tolerance
    }

    // Call Test Fixture Class
    TEST_F( EUR_SwapCurve_Consistency, CONSISTENCY_EUR6ML_Swap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    swapTerms   = curveCalibrationSwapTenors( curveObjectEUR6ML_ ); 
        std::vector<double>         parRates    = curveCalibrationSwapParRates( curveObjectEUR6ML_ );
        MLIB_REQUIRE( swapTerms.size() == parRates.size(), "Inconsistent Calibration Data: Inconsistent Number of Swap Terms and Par Rates" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < swapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "11Y", "12Y", "15Y", "20Y", "25Y", "30Y", "40Y", "50Y" };
            auto it = calibrationInstruments.find( swapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "EUR6ML_Swap" + MLIB_TO_STRING_FROM_SIZE_T(i);
            const std::string swapGenerator             = "EUR_6ML";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("EURYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", swapTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = parRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
}