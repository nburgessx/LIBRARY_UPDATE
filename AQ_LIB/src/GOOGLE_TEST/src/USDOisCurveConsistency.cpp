// USDOisCurveConsistency.cpp

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


namespace google_test
{
    // Declare Test Fixture Class
    class USD_OisCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectUSDOIS_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string curveObjectUSD3ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string curveObjectUSD6ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );
    };

    
    // Call Test Fixture Class
    TEST_F( USD_OisCurve_Consistency, CONSISTENCY_USDOIS_OisSwap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    oisSwapTerms    = curveCalibrationOisSwapTenors( curveObjectUSDOIS_ ); 
        std::vector<double>         oisSwapParRates = curveCalibrationOisSwapParRates( curveObjectUSDOIS_ );
        AQ_REQUIRE( oisSwapTerms.size() == oisSwapParRates.size(), "Inconsistent Calibration Data: Inconsistent Number of OIS Swap Terms and Par Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < oisSwapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments - Note we use Libor-OIS Swaps and not OIS Outrights from and including 1Y
            std::set<std::string> calibrationInstruments = { "1W", "2W", "3W", "1M", "2M", "3M", "4M", "5M", "6M", "9M"};
            auto it = calibrationInstruments.find( oisSwapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "USDOIS_Swap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "USD_OIS";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("USDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", oisSwapTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = oisSwapParRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
    

    // Call Test Fixture Class
    TEST_F( USD_OisCurve_Consistency, CONSISTENCY_USDOIS_LiborOIS_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    liborOisTerms    = curveCalibrationLiborOisTenors( curveObjectUSDOIS_ ); 
        std::vector<double>         liborOisSpreads  = curveCalibrationLiborOisSpreads( curveObjectUSDOIS_ );
        AQ_REQUIRE( liborOisTerms.size() == liborOisSpreads.size(), "Inconsistent Calibration Data: Inconsistent Number of Libor OIS Terms and Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-3; // TODO: Not a great tolerance; Need to improve the calibration accuarcy
        for ( size_t i = 0; i < liborOisTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            // ***  Note the 1Y point is the 12M fixing, so we exclude the 1Y point ***
            std::set<std::string> calibrationInstruments = { "3Y", "4Y", "5Y", "7Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y" };
            auto it = calibrationInstruments.find( liborOisTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "USDOIS_LiborOISSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "USD_BASIS_LIBOROIS";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("USDYC");

            // Create the Swap & Calculate the Libor-OIS Spread
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", liborOisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapSpread( swapObject, curveLVB, etrading::LabelValueBlock(), "Leg1:Float" ) / 10000; // Basis Points
            const double expectedResult                  = liborOisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }   
    }


}