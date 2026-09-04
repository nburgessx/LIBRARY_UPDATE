// EURBasisCurveConsistency.cpp

/*
 * @brief			EUR Curve Consistency Tests
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
#include "DataUtilities.h" // AQ_TO_STRING macros

// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***

const std::string TEST_FOLDER = "ETrading/Curves/EURCurves/";

// Test Files
const std::string fileNameEUROIS = TEST_FOLDER + "EUR_OIS_CURVE.JSON";
const std::string fileNameEUR3ML = TEST_FOLDER + "EUR_SWAP_3M_CURVE.JSON";
const std::string fileNameEUR6ML = TEST_FOLDER + "EUR_SWAP_6M_CURVE.JSON";
const std::string fileNameEUR1ML = TEST_FOLDER + "EUR_BASIS_1M_CURVE.JSON";
const std::string fileNameEUR12ML = TEST_FOLDER + "EUR_BASIS_12M_CURVE.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class EUR_BasisCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectEUROIS_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameEUROIS, etrading::JSON );
        const std::string curveObjectEUR3ML_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameEUR3ML, etrading::JSON );
        const std::string curveObjectEUR6ML_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameEUR6ML, etrading::JSON );
        const std::string curveObjectEUR1ML_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameEUR1ML, etrading::JSON );
        const std::string curveObjectEUR12ML_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameEUR12ML, etrading::JSON );
    };

    // Call Test Fixture Class
    TEST_F( EUR_BasisCurve_Consistency, CONSISTENCY_EUR1ML_TenorBasis_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::BASIS_SWAP_MARKETDATA,    // Calibration Instrument: Tenor Basis Swap
                                       { "1Y", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" }, // Calibration Tenors                                  
                                       curveObjectEUR1ML_,                               // AQObjCurveName
                                       "EURYC",                                          // CurveCollection
                                       "20180814",                                       // Swap Effective Date
                                       "EUR_BASIS_1X3",                                  // Swap Generator
                                       1.0e-6 );                                         // Test Tolerance
    }

    // Call Test Fixture Class
    TEST_F( EUR_BasisCurve_Consistency, CONSISTENCY_EUR1ML_TenorBasis_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the AQObj Curve Object
        std::vector<std::string>    basisTerms   = curveCalibrationBasisSwapTenors( curveObjectEUR1ML_ ); 
        std::vector<double>         basisSpreads = curveCalibrationBasisSwapSpreads( curveObjectEUR1ML_ );
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
            const std::string swapName                  = "EUR1ML_BasisSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "EUR_BASIS_1X3";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("EURYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", basisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation::tryAqObjSwapsSpread( swapObject, curveLVB ) / 10000; // Basis Points
            const double expectedResult                  = basisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
    
    // Call Test Fixture Class
    TEST_F( EUR_BasisCurve_Consistency, CONSISTENCY_EUR12ML_TenorBasis_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::BASIS_SWAP_MARKETDATA,    // Calibration Instrument: Tenor Basis Swap
                                       { "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" }, // Calibration Tenors                                  
                                       curveObjectEUR12ML_,                              // AQObjCurveName
                                       "EURYC",                                          // CurveCollection
                                       "20180814",                                       // Swap Effective Date
                                       "EUR_BASIS_6X12",                                 // Swap Generator
                                       1.0e-6 );                                         // Test Tolerance
    }

    // Call Test Fixture Class
    TEST_F( EUR_BasisCurve_Consistency, CONSISTENCY_EUR12ML_TenorBasis_Instrument_Repricing )
    {
         // Extract the Swap Terms & Par Rates from the AQObj Curve Object
        std::vector<std::string>    basisTerms   = curveCalibrationBasisSwapTenors( curveObjectEUR12ML_ ); 
        std::vector<double>         basisSpreads = curveCalibrationBasisSwapSpreads( curveObjectEUR12ML_ );
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
            std::set<std::string> calibrationInstruments = { "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" };
            auto it = calibrationInstruments.find( basisTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "EUR12ML_BasisSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "EUR_BASIS_6X12";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("EURYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", basisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation::tryAqObjSwapsSpread( swapObject, curveLVB ) / 10000; // Basis Points
            const double expectedResult                  = basisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }   
    }


}