// AUDOisCurveConsistency.cpp

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
#include "DataUtilities.h" // AQ_TO_STRING macros

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
    class AUD_OisCurve_Consistency : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectAUDOIS_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameAUDOIS, etrading::JSON );
        const std::string curveObjectAUD3ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameAUD3ML, etrading::JSON );
        const std::string curveObjectAUD6ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameAUD6ML, etrading::JSON );
    };

    // Declare Test Fixture Class
    class AUD_OisCurve_LoadCurve_x3 : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
    public:

        // Load OIS Curve Multiple Times to test reloading of curves
        std::string curveObjectAUDOIS1_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameAUDOIS, etrading::JSON );
		std::string curveObjectAUDOIS2_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameAUDOIS, etrading::JSON );
		std::string curveObjectAUDOIS3_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameAUDOIS, etrading::JSON );
    };

	// Call Test Fixture Class
    TEST_F( AUD_OisCurve_LoadCurve_x3, CONSISTENCY_AUDOIS_OisSwap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    oisSwapTerms    = curveCalibrationOisSwapTenors( curveObjectAUDOIS3_ );		// Load the curve 3 times and use the 3rd curve
        std::vector<double>         oisSwapParRates = curveCalibrationOisSwapParRates( curveObjectAUDOIS3_ );	// Load the curve 3 times and use the 3rd curve
        AQ_REQUIRE( oisSwapTerms.size() == oisSwapParRates.size(), "Inconsistent Calibration Data: Inconsistent Number of OIS Swap Terms and Par Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-4; // TODO: Not a great tolerance; Need to improve the calibration accuracy
        for ( size_t i = 0; i < oisSwapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "1W", "1M", "2M", "3M", "4M", "5M", "6M", "9M", "1Y", "2Y" };
            auto it = calibrationInstruments.find( oisSwapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "AUDOIS_Swap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "AUD_OIS";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("AUDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180809", oisSwapTerms[i] ); // Effective Date = 20180809
            const double actualResult                    = validation::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = oisSwapParRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }

    // Call Test Fixture Class
    TEST_F( AUD_OisCurve_Consistency, CONSISTENCY_AUDOIS_OisSwap_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    oisSwapTerms    = curveCalibrationOisSwapTenors( curveObjectAUDOIS_ ); 
        std::vector<double>         oisSwapParRates = curveCalibrationOisSwapParRates( curveObjectAUDOIS_ );
        AQ_REQUIRE( oisSwapTerms.size() == oisSwapParRates.size(), "Inconsistent Calibration Data: Inconsistent Number of OIS Swap Terms and Par Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-4; // TODO: Not a great tolerance; Need to improve the calibration accuracy
        for ( size_t i = 0; i < oisSwapTerms.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            std::set<std::string> calibrationInstruments = { "1W", "1M", "2M", "3M", "4M", "5M", "6M", "9M", "1Y", "2Y" };
            auto it = calibrationInstruments.find( oisSwapTerms[i] );
            
            if ( it == calibrationInstruments.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "AUDOIS_Swap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGenerator             = "AUD_OIS";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("AUDYC");

            // Create the Swap & Calculate the Par Rate
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180809", oisSwapTerms[i] ); // Effective Date = 20180809
            const double actualResult                    = validation::tryMeLWOSwapParRate( swapObject, curveLVB );
            const double expectedResult                  = oisSwapParRates[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }
    }
    

    // Call Test Fixture Class
    TEST_F( AUD_OisCurve_Consistency, CONSISTENCY_AUDOIS_LiborOIS_Instrument_Repricing )
    {
        // Extract the Swap Terms & Par Rates from the LWO Curve Object
        std::vector<std::string>    liborOisTerms    = curveCalibrationLiborOisTenors( curveObjectAUDOIS_ ); 
        std::vector<double>         liborOisSpreads  = curveCalibrationLiborOisSpreads( curveObjectAUDOIS_ );
        AQ_REQUIRE( liborOisTerms.size() == liborOisSpreads.size(), "Inconsistent Calibration Data: Inconsistent Number of Libor OIS Terms and Spreads" )

        // Reprice Calibration Instruments
        const double tolerance = 1e-4; // TODO: Not a great tolerance; Need to improve the calibration accuracy
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
            const std::string swapName                  = "AUDOIS_LiborOISSwap" + AQ_TO_STRING_FROM_SIZE_T( i );
            const std::string swapGeneratorQtr          = "AUD_BASIS_LIBOROIS_QUARTERLY";
            const std::string swapGeneratorSemi         = "AUD_BASIS_LIBOROIS_SEMIANNUAL";
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB("AUDYC");

            // Create the Swap & Calculate the Libor-OIS Spread
            // Use the Quarterly Swap Generator for Instruments with Tenor less than or equal to 3Y
            std::string swapGenerator = liborOisTerms[i] == "3Y" ? swapGeneratorQtr : swapGeneratorSemi;
            
            const std::string swapObject                 = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, "20180809", liborOisTerms[i] ); // Effective Date = 20180809
            const double actualResult                    = validation::tryMeLWOSwapSpread( swapObject, curveLVB, etrading::LabelValueBlock(), "Leg1:Float" ) / 10000; // Basis Points
            const double expectedResult                  = liborOisSpreads[i];

            EXPECT_NEAR( actualResult, expectedResult, tolerance );
        }   
    }



}