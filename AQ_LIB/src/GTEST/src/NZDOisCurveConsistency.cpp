// NZDOisCurveConsistency.cpp

/*
 * @brief			NZD Curve Consistency Tests
 * @Created:		16th April 2020
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
#include "DataUtilities.h" // AQ_TO_STRING macros

// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***

const std::string TEST_FOLDER = "ETrading/Curves/NZDCurves/";

// Test Files
const std::string fileNameNZDOIS = TEST_FOLDER + "NZD_OIS_CURVE.JSON";
const std::string fileNameNZD3ML = TEST_FOLDER + "NZD_SWAP_3M_CURVE.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class NZD_OisCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectNZDOIS_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameNZDOIS, etrading::JSON );    
		const std::string curveObjectNZD3ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameNZD3ML, etrading::JSON );
    };

    // Call Test Fixture Class
    TEST_F( NZD_OisCurve_Consistency, CONSISTENCY_NZDOIS_OisSwap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::OIS_MARKETDATA,    // Calibration Instrument: OIS Outright Swaps
                                       {"1D", "1M", "2M", "3M", "4M", "5M", "6M", "7M", "8M", "9M", "1Y", "18M", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y"}, // Calibration Tenors                                  
                                       curveObjectNZDOIS_,                               // LWOCurveName
                                       "NZDYC",                                          // CurveCollection
                                       "20200420",                                       // Swap Effective Date
                                       "NZD_OIS",                                        // Swap Generator
                                       1.0e-6 );                                         // Test Tolerance
    }

    // Call Test Fixture Class
    TEST_F( NZD_OisCurve_Consistency, CONSISTENCY_NZDOIS_OisSwap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    oisSwapTerms    = curveCalibrationOisSwapTenors( curveObjectNZDOIS_ ); 
        std::vector<double>         oisSwapParRates = curveCalibrationOisSwapParRates( curveObjectNZDOIS_ );
        AQ_REQUIRE( oisSwapTerms.size() == oisSwapParRates.size(), "Inconsistent Calibration Data: Inconsistent Number of OIS Swap Terms and Par Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < oisSwapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "1D", "1M", "2M", "3M", "4M", "5M", "6M", "7M", "8M", "9M", "1Y", "18M", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y" };
            auto it = calibrationInstruments.find( oisSwapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "NZDOIS_Swap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "NZD_OIS";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("NZDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20200420", oisSwapTerms[i] ); // Effective Date = 20200420
            const double actualResult                    = validation::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = oisSwapParRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }

}