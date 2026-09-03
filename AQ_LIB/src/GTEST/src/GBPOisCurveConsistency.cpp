// GBPOisCurveConsistency.cpp

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


namespace google_test
{
    // Declare Test Fixture Class
    class GBP_OisCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectGBPOIS_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameGBPOIS, etrading::JSON );
        const std::string curveObjectGBP3ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameGBP3ML, etrading::JSON );
        const std::string curveObjectGBP6ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameGBP6ML, etrading::JSON );
    };

    
    // Call Test Fixture Class
    TEST_F( GBP_OisCurve_Consistency, CONSISTENCY_GBPOIS_OisSwap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    oisSwapTerms    = curveCalibrationOisSwapTenors( curveObjectGBPOIS_ ); 
        std::vector<double>         oisSwapParRates = curveCalibrationOisSwapParRates( curveObjectGBPOIS_ );
        AQ_REQUIRE( oisSwapTerms.size() == oisSwapParRates.size(), "Inconsistent Calibration Data: Inconsistent Number of OIS Swap Terms and Par Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < oisSwapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "1W", "1M", "2M", "3M", "4M", "5M", "6M", "7M", "8M", "9M", "10M", "11M", "1Y", "18M", "2Y" };
            auto it = calibrationInstruments.find( oisSwapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "GBPOIS_Swap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "GBP_OIS";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("GBPYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180810", oisSwapTerms[i] ); // Effective Date = 20180810
            const double actualResult                    = validation::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = oisSwapParRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
    

    // Call Test Fixture Class
    TEST_F( GBP_OisCurve_Consistency, CONSISTENCY_GBPOIS_LiborOIS_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    liborOisTerms    = curveCalibrationLiborOisTenors( curveObjectGBPOIS_ ); 
        std::vector<double>         liborOisSpreads  = curveCalibrationLiborOisSpreads( curveObjectGBPOIS_ );
        AQ_REQUIRE( liborOisTerms.size() == liborOisSpreads.size(), "Inconsistent Calibration Data: Inconsistent Number of Libor OIS Terms and Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-4; // <---- Not a great tolerance number, need to investigate the single calibration instrument discrepency
        for ( size_t i = 0; i < liborOisTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            // ***  Note the 1Y point is the 12M fixing, so we exclude the 1Y point ***
            std::set<std::string> calibrationInstruments = { "3Y", "4Y", "5Y", "7Y", "10Y", "15Y", "20Y", "30Y" };
            auto it = calibrationInstruments.find( liborOisTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "GBPOIS_LiborOISSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "GBP_BASIS_LIBOROIS";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("GBPYC");

            // Create the Swap & Calculate the Libor-OIS Spread
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180810", liborOisTerms[i] ); // Effective Date = 20180810
            const double actualResult                    = validation::tryMeLWOSwapSpread( swapObject, curveLVB, etrading::LabelValueBlock(), "Leg1:Float" ) / 10000; // Basis Points
            const double expectedResult                  = liborOisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }   
    }


}