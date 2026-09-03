// GBPBasisCurveConsistency.cpp

/*
 * @brief			GBP Curve Consistency Tests
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

const std::string TEST_FOLDER = "ETrading/Curves/GBPCurves/";

// Test Files
const std::string fileNameGBPOIS = TEST_FOLDER + "GBP_OIS_CURVE.JSON";
const std::string fileNameGBP3ML = TEST_FOLDER + "GBP_SWAP_3M_CURVE.JSON";
const std::string fileNameGBP6ML = TEST_FOLDER + "GBP_BASIS_6M_CURVE.JSON";
const std::string fileNameGBP1ML = TEST_FOLDER + "GBP_BASIS_1M_CURVE.JSON";
const std::string fileNameGBP12ML = TEST_FOLDER + "GBP_BASIS_12M_CURVE.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class GBP_BasisCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectGBPOIS_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameGBPOIS, etrading::JSON );
        const std::string curveObjectGBP3ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameGBP3ML, etrading::JSON );
        const std::string curveObjectGBP6ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameGBP6ML, etrading::JSON );
        const std::string curveObjectGBP1ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameGBP1ML, etrading::JSON );
        const std::string curveObjectGBP12ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameGBP12ML, etrading::JSON );
    };

    

    // Call Test Fixture Class
    TEST_F( GBP_BasisCurve_Consistency, CONSISTENCY_GBP6ML_TenorBasis_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    basisTerms   = curveCalibrationBasisSwapTenors( curveObjectGBP6ML_ ); 
        std::vector<double>         basisSpreads = curveCalibrationBasisSwapSpreads( curveObjectGBP6ML_ );
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
            const std::string swapName                  = "GBP6ML_BasisSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "GBP_BASIS_3X6";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("GBPYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180810", basisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation::tryMeLWOSwapSpread( swapObject, curveLVB ) / 10000; // Basis Points
            const double expectedResult                  = basisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }


    // Call Test Fixture Class
    TEST_F( GBP_BasisCurve_Consistency, CONSISTENCY_GBP1ML_TenorBasis_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    basisTerms   = curveCalibrationBasisSwapTenors( curveObjectGBP1ML_ ); 
        std::vector<double>         basisSpreads = curveCalibrationBasisSwapSpreads( curveObjectGBP1ML_ );
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
            const std::string swapName                  = "GBP1ML_BasisSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "GBP_BASIS_1X3";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("GBPYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180810", basisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation::tryMeLWOSwapSpread( swapObject, curveLVB ) / 10000; // Basis Points
            const double expectedResult                  = basisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
    

    // Call Test Fixture Class
    TEST_F( GBP_BasisCurve_Consistency, CONSISTENCY_GBP12ML_TenorBasis_Instrument_Repricing )
    {
         // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    basisTerms   = curveCalibrationBasisSwapTenors( curveObjectGBP12ML_ ); 
        std::vector<double>         basisSpreads = curveCalibrationBasisSwapSpreads( curveObjectGBP12ML_ );
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
            const std::string swapName                  = "GBP12ML_BasisSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "GBP_BASIS_6X12";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("GBPYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180810", basisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation::tryMeLWOSwapSpread( swapObject, curveLVB ) / 10000; // Basis Points
            const double expectedResult                  = basisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }   
    }


}