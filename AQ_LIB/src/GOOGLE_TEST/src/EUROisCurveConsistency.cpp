// EUROisCurveConsistency.cpp

/*
 * @brief			EUR Curve Consistency Tests
 * @Created:		8th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		MHI London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
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
#include "DataUtilities.h" // MLIB_TO_STRING macros

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
    class EUR_OisCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectEUROIS_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUROIS, etrading::JSON );
        const std::string curveObjectEUR3ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR3ML, etrading::JSON );
        const std::string curveObjectEUR6ML_ = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR6ML, etrading::JSON );
    };

    // Call Test Fixture Class
    TEST_F( EUR_OisCurve_Consistency, CONSISTENCY_EUROIS_OisSwap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::OIS_MARKETDATA,    // Calibration Instrument: OIS Outright Swaps
                                       {"12M", "18M", "2Y"},                             // Calibration Tenors                                  
                                       curveObjectEUROIS_,                               // LWOCurveName
                                       "EURYC",                                          // CurveCollection
                                       "20180814",                                       // Swap Effective Date
                                       "EUR_OIS",                                        // Swap Generator
                                       1.0e-6 );                                         // Test Tolerance
    }

    // Call Test Fixture Class
    TEST_F( EUR_OisCurve_Consistency, CONSISTENCY_EUROIS_OisSwap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    oisSwapTerms    = curveCalibrationOisSwapTenors( curveObjectEUROIS_ ); 
        std::vector<double>         oisSwapParRates = curveCalibrationOisSwapParRates( curveObjectEUROIS_ );
        MLIB_REQUIRE( oisSwapTerms.size() == oisSwapParRates.size(), "Inconsistent Calibration Data: Inconsistent Number of OIS Swap Terms and Par Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-6;
        for ( size_t i = 0; i < oisSwapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "12M", "18M", "2Y" };
            auto it = calibrationInstruments.find( oisSwapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "EUROIS_Swap" + MLIB_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "EUR_OIS";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("EURYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", oisSwapTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = oisSwapParRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
    
    // Call Test Fixture Class
    TEST_F( EUR_OisCurve_Consistency, CONSISTENCY_EUROIS_LiborOIS_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::LIBOR_OIS_BASISSPREAD_MARKETDATA,    // Calibration Instrument: LIBOR-OIS Basis Swaps
                                       { "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "11Y", "12Y", "15Y", "20Y", "25Y", "30Y", "40Y", "50Y" },  // Calibration Tenors                                  
                                       curveObjectEUROIS_,                               // LWOCurveName
                                       "EURYC",                                          // CurveCollection
                                       "20180814",                                       // Swap Effective Date
                                       "EUR_BASIS_LIBOROIS",                             // Swap Generator
                                       1.0e-5 );                                         // Test Tolerance
    }

    // Call Test Fixture Class
    TEST_F( EUR_OisCurve_Consistency, CONSISTENCY_EUROIS_LiborOIS_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    liborOisTerms    = curveCalibrationLiborOisTenors( curveObjectEUROIS_ ); 
        std::vector<double>         liborOisSpreads  = curveCalibrationLiborOisSpreads( curveObjectEUROIS_ );
        MLIB_REQUIRE( liborOisTerms.size() == liborOisSpreads.size(), "Inconsistent Calibration Data: Inconsistent Number of Libor OIS Terms and Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-5;
        for ( size_t i = 0; i < liborOisTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            // ***  Note the 1Y point is the 12M fixing, so we exclude the 1Y point ***
            std::set<std::string> calibrationInstruments = { "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "11Y", "12Y", "15Y", "20Y", "25Y", "30Y", "40Y", "50Y" };
            auto it = calibrationInstruments.find( liborOisTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "EUROIS_LiborOISSwap" + MLIB_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "EUR_BASIS_LIBOROIS";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("EURYC");

            // Create the Swap & Calculate the Libor-OIS Spread
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180814", liborOisTerms[i] ); // Effective Date = 20180814
            const double actualResult                    = validation_api::tryMeLWOSwapSpread( swapObject, curveLVB, etrading::LabelValueBlock(), "Leg1:Float" ) / 10000; // Basis Points
            const double expectedResult                  = liborOisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }   
    }


}