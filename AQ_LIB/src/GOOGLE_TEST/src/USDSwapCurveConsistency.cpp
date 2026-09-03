// USDSwapCurveConsistency.cpp

/*
 * @brief			USD Curve Consistency Tests
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
#include "DataUtilities.h" // AQ_TO_STRING macros

// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***

const std::string TEST_FOLDER = "ETrading/Curves/USDCurves/";

// Test Files
const std::string fileNameUSDOIS = TEST_FOLDER + "USD_OIS_CURVE.JSON";
const std::string fileNameUSD3ML = TEST_FOLDER + "USD_SWAP_3M_CURVE.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class USD_SwapCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectUSDOIS_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string curveObjectUSD3ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
    };

    
    // Call Test Fixture Class
    TEST_F( USD_SwapCurve_Consistency, CONSISTENCY_USD3ML_Swap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    swapTerms   = curveCalibrationSwapTenors( curveObjectUSD3ML_ ); 
        std::vector<double>         parRates    = curveCalibrationSwapParRates( curveObjectUSD3ML_ );
        AQ_REQUIRE( swapTerms.size() == parRates.size(), "Inconsistent Calibration Data: Inconsistent Number of Swap Terms and Par Rates" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < swapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "3Y", "4Y", "5Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" };
            auto it = calibrationInstruments.find( swapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "USD3ML_Swap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "USD_3ML";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("USDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", swapTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = parRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
        
}