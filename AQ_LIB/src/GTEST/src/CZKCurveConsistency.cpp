// CZKCurveConsistency.cpp

/*
 * @brief			CZK Curve Consistency Tests
 */

// Include: Google Test Library
#include "InitializeGoogleTest.h"

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

const std::string TEST_FOLDER = "ETrading/Curves/CZKCurves/";

// Test Files
const std::string fileNameCZKOIS = TEST_FOLDER + "CZK_OIS_CURVE.JSON";
const std::string fileNameCZK6M = TEST_FOLDER + "CZK_SWAP_6M_CURVE.JSON";
const std::string fileNameCZK3M = TEST_FOLDER + "CZK_BASIS_3M_CURVE.JSON";
const std::string fileNameCZK1M = TEST_FOLDER + "CZK_BASIS_1M_CURVE.JSON";
const std::string fileNameCZK12M = TEST_FOLDER + "CZK_BASIS_12M_CURVE.JSON";


namespace google_test
{
    // Declare Test Fixture Class
    class CZK_Curve_Consistency : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
    public:

        // Load Curves
        const std::string curveObjectCZKOIS_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameCZKOIS, etrading::JSON );
        const std::string curveObjectCZK6M_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameCZK6M, etrading::JSON );
        const std::string curveObjectCZK3M_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameCZK3M, etrading::JSON );
		const std::string curveObjectCZK1M_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameCZK1M, etrading::JSON );
		const std::string curveObjectCZK12M_ = validation::tryAqObjLoad( etrading::getGoogleTestFolder() + fileNameCZK12M, etrading::JSON );
    };

    // Call Test Fixture Class
    TEST_F( CZK_Curve_Consistency, CONSISTENCY_CZKOIS_OisSwap_Under1Y_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::OIS_MARKETDATA,    // Calibration Instrument: OIS Outright Swaps
                                      // Calibration Tenors  
									  {"1W", "2W", "1M", "2M", "3M", "6M", "9M"},                                 
                                       curveObjectCZKOIS_,                               // AQObjCurveName
                                       "CZKYC",                                          // CurveCollection
                                       "20200903",                                       // Swap Effective Date
                                       "EM_CZK_OIS_UNDER_1Y",                            // Swap Generator
                                       1.0e-6 );                                         // Test Tolerance
    }

	// Call Test Fixture Class
    TEST_F( CZK_Curve_Consistency, CONSISTENCY_CZKOIS_OisSwap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::OIS_MARKETDATA,    // Calibration Instrument: OIS Outright Swaps
                                      // Calibration Tenors 
									  {"1Y", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y", "25Y", "30Y", "40Y", "50Y"},
                                       curveObjectCZKOIS_,                               // AQObjCurveName
                                       "CZKYC",                                          // CurveCollection
                                       "20200903",                                       // Swap Effective Date
                                       "EM_CZK_OIS",                                     // Swap Generator
                                       1.0e-6 );                                         // Test Tolerance
    }

	// Call Test Fixture Class
    TEST_F( CZK_Curve_Consistency, CONSISTENCY_CZK6M_Swap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::SWAP_MARKETDATA,	// Calibration Instrument: OIS Outright Swaps
                                      // Calibration Tenors 
									  {"2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y"},
                                       curveObjectCZK6M_,                               // AQObjCurveName
                                       "CZKYC",                                         // CurveCollection
                                       "20200903",                                      // Swap Effective Date
                                       "EM_CZK_6M",                                     // Swap Generator
                                       1.0e-6 );                                        // Test Tolerance
    }

	// Call Test Fixture Class
    TEST_F( CZK_Curve_Consistency, CONSISTENCY_CZK3M_BasisSwap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::BASIS_SWAP_MARKETDATA,	// Calibration Instrument: OIS Outright Swaps
                                      // Calibration Tenors 
									  {"1Y", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y"},
                                       curveObjectCZK3M_,                               // AQObjCurveName
                                       "CZKYC",                                         // CurveCollection
                                       "20200903",                                      // Swap Effective Date
                                       "EM_CZK_3M",                                     // Swap Generator
                                       1.0e-6 );                                        // Test Tolerance
    }

	// Call Test Fixture Class
    TEST_F( CZK_Curve_Consistency, CONSISTENCY_CZK1M_BasisSwap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::BASIS_SWAP_MARKETDATA,	// Calibration Instrument: OIS Outright Swaps
                                      // Calibration Tenors 
									  {"1Y", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y"},
                                       curveObjectCZK1M_,                               // AQObjCurveName
                                       "CZKYC",                                         // CurveCollection
                                       "20200903",                                      // Swap Effective Date
                                       "EM_CZK_1M",                                     // Swap Generator
                                       1.0e-6 );                                        // Test Tolerance
    }

	// Call Test Fixture Class
    TEST_F( CZK_Curve_Consistency, CONSISTENCY_CZK12M_BasisSwap_Instrument_Repricing_Using_Macro )
    {
        // Function will throw on failure or if no tests are run
        repriceCalibrationInstruments( etrading::CurveMarketDataEnum::BASIS_SWAP_MARKETDATA,	// Calibration Instrument: OIS Outright Swaps
                                      // Calibration Tenors 
									  {"1Y", "2Y", "3Y", "4Y", "5Y", "6Y", "7Y", "8Y", "9Y", "10Y", "12Y", "15Y", "20Y"},
                                       curveObjectCZK12M_,                               // AQObjCurveName
                                       "CZKYC",                                         // CurveCollection
                                       "20200903",                                      // Swap Effective Date
                                       "EM_CZK_12M",                                    // Swap Generator
                                       1.0e-6 );                                        // Test Tolerance
    }
}