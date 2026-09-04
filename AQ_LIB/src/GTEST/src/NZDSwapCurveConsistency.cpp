// NZDSwapCurveConsistency.cpp

/*
 * @brief			NZD Curve Consistency Tests
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "ResultsProcessor.h"
#include "CoreEnumerations.h"
#include "tryAqObjects.h"
#include "tryAqObjSwapsCreation.h"
#include "tryAqObjSwapsPricing.h"
#include "BuildSwapTradeFromGenerator.h"
#include "ExtractCurveCalibrationData.h"
#include "RepriceCalibrationInstruments.h"
#include "DataUtilities.h"

// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***
const std::string TEST_FOLDER = "ETrading/Curves/NZDCurves/";

// Test Files
const std::string fileNameNZDOIS = TEST_FOLDER + "NZD_OIS_CURVE.JSON";
const std::string fileNameNZD3ML = TEST_FOLDER + "NZD_SWAP_3M_CURVE.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class NZD_SwapCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
    public:

        // Load Curves
    const std::string curveObjectNZDOIS_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameNZDOIS, etrading::JSON );    
	const std::string curveObjectNZD3ML_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameNZD3ML, etrading::JSON );
    };


    // Call Test Fixture Class
    TEST_F( NZD_SwapCurve_Consistency, CONSISTENCY_NZD3ML_Swap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::SWAP_MARKETDATA,    // Calibration Instrument: STD Interest Rate Swap
                                       { "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y" }, // Calibration Tenors
                                       curveObjectNZD3ML_,                               // AQObjCurveName
                                       "NZDYC",                                          // CurveCollection
                                       "20200420",                                       // Swap Effective Date
                                       "NZD_3ML",                                        // Swap Generator
                                       1.0e-6 );                                         // Test Tolerance
    }
    
    // Call Test Fixture Class
    TEST_F( NZD_SwapCurve_Consistency, CONSISTENCY_NZD3ML_Swap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the AQObj Curve Object
        std::vector<std::string>    swapTerms   = curveCalibrationSwapTenors( curveObjectNZD3ML_ ); 
        std::vector<double>         parRates    = curveCalibrationSwapParRates( curveObjectNZD3ML_ );
        AQ_REQUIRE( swapTerms.size() == parRates.size(), "Inconsistent Calibration Data: Inconsistent Number of Swap Terms and Par Rates" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < swapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y" };
            auto it = calibrationInstruments.find( swapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "NZD3ML_Swap" + AQ_TO_STRING_FROM_SIZE_T(i);
            const std::string swapGenerator             = "NZD_3ML";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("NZDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20200420", swapTerms[i] ); // Effective Date = 20200420
            const double actualResult                    = validation::tryAqObjSwapsParRate( swapObject, curveLVB );
            const double expectedResult                  = parRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }

}