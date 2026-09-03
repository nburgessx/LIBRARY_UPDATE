// USDBasisCurveConsistency.cpp

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
const std::string fileNameUSD6ML = TEST_FOLDER + "USD_BASIS_6M_CURVE.JSON";
const std::string fileNameUSD1ML = TEST_FOLDER + "USD_BASIS_1M_CURVE.JSON";
const std::string fileNameUSD12ML = TEST_FOLDER + "USD_BASIS_12M_CURVE.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class USD_BasisCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectUSDOIS_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string curveObjectUSD3ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string curveObjectUSD6ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );
        const std::string curveObjectUSD1ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD1ML, etrading::JSON );
        const std::string curveObjectUSD12ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD12ML, etrading::JSON );
    };

    

    // Call Test Fixture Class
    TEST_F( USD_BasisCurve_Consistency, CONSISTENCY_USD6ML_TenorBasis_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    basisTerms   = curveCalibrationBasisSwapTenors( curveObjectUSD6ML_ ); 
        std::vector<double>         basisSpreads = curveCalibrationBasisSwapSpreads( curveObjectUSD6ML_ );
        AQ_REQUIRE( basisTerms.size() == basisSpreads.size(), "Inconsistent Calibration Data: Inconsistent Number of Basis Swap Terms and Par Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < basisTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "1Y", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" };
            auto it = calibrationInstruments.find( basisTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "USD6ML_BasisSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "USD_BASIS_3X6";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("USDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", basisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapSpread( swapObject, curveLVB ) / 10000; // Basis Points
            const double expectedResult                  = basisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }


    // Call Test Fixture Class
    TEST_F( USD_BasisCurve_Consistency, CONSISTENCY_USD1ML_TenorBasis_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    basisTerms   = curveCalibrationBasisSwapTenors( curveObjectUSD1ML_ ); 
        std::vector<double>         basisSpreads = curveCalibrationBasisSwapSpreads( curveObjectUSD1ML_ );
        AQ_REQUIRE( basisTerms.size() == basisSpreads.size(), "Inconsistent Calibration Data: Inconsistent Number of Basis Swap Terms and Par Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < basisTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "1Y", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" };
            auto it = calibrationInstruments.find( basisTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "USD1ML_BasisSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "USD_BASIS_1X3";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("USDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", basisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapSpread( swapObject, curveLVB ) / 10000; // Basis Points
            const double expectedResult                  = basisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
    

    // Call Test Fixture Class
    TEST_F( USD_BasisCurve_Consistency, CONSISTENCY_USD12ML_TenorBasis_Instrument_Repricing )
    {
         // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    basisTerms   = curveCalibrationBasisSwapTenors( curveObjectUSD12ML_ ); 
        std::vector<double>         basisSpreads = curveCalibrationBasisSwapSpreads( curveObjectUSD12ML_ );
        AQ_REQUIRE( basisTerms.size() == basisSpreads.size(), "Inconsistent Calibration Data: Inconsistent Number of Basis Swap Terms and Par Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < basisTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            // ***  Note the 1Y point is the 12M fixing, so we exclude the 1Y point ***
            std::set<std::string> calibrationInstruments = { "2Y", "3Y", "4Y", "5Y", "7Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" };
            auto it = calibrationInstruments.find( basisTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "USD12ML_BasisSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "USD_BASIS_3X12";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("USDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", basisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapSpread( swapObject, curveLVB ) / 10000; // Basis Points
            const double expectedResult                  = basisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }   
    }


}