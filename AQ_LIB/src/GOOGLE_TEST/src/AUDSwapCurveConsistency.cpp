// AUDSwapCurveConsistency.cpp

/*
 * @brief			AUD Curve Consistency Tests
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
#include "DataUtilities.h" // MLIB_TO_STRING macros

// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***

const std::string TEST_FOLDER = "ETrading/Curves/AUDCurves/";

// Test Files
const std::string fileNameAUDOIS = TEST_FOLDER + "AUDOIS.JSON";
const std::string fileNameAUD3ML = TEST_FOLDER + "AUD3ML.JSON";
const std::string fileNameAUD6ML = TEST_FOLDER + "AUD6ML.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class AUD_SwapCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectAUDOIS_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameAUDOIS, etrading::JSON );
        const std::string curveObjectAUD3ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameAUD3ML, etrading::JSON );
        const std::string curveObjectAUD6ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameAUD6ML, etrading::JSON );
    };

    
    // Call Test Fixture Class
    TEST_F( AUD_SwapCurve_Consistency, CONSISTENCY_AUD3ML_Swap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    swapTerms   = curveCalibrationSwapTenors( curveObjectAUD3ML_ ); 
        std::vector<double>         parRates    = curveCalibrationSwapParRates( curveObjectAUD3ML_ );
        MLIB_REQUIRE( swapTerms.size() == parRates.size(), "Inconsistent Calibration Data: Inconsistent Number of Swap Terms and Par Rates" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < swapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "2Y", "3Y" };
            auto it = calibrationInstruments.find( swapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "AUD3ML_Swap" + MLIB_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "AUD_3ML";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("AUDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180809", swapTerms[i] ); // Effective Date = 20180809
            const double actualResult                    = validation_api::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = parRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
    

    // Call Test Fixture Class
    TEST_F( AUD_SwapCurve_Consistency, CONSISTENCY_AUD6ML_Swap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    swapTerms   = curveCalibrationSwapTenors( curveObjectAUD6ML_ ); 
        std::vector<double>         parRates    = curveCalibrationSwapParRates( curveObjectAUD6ML_ );
        MLIB_REQUIRE( swapTerms.size() == parRates.size(), "Inconsistent Calibration Data: Inconsistent Number of Swap Terms and Par Rates" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < swapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "15Y", "20Y", "25Y", "30Y" };
            auto it = calibrationInstruments.find( swapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "AUD6ML_Swap" + MLIB_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "AUD_6ML";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("AUDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180809", swapTerms[i] ); // Effective Date = 20180809
            const double actualResult                    = validation_api::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = parRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
}