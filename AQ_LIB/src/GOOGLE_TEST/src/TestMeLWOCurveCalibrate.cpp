/*
 * @brief			Tests to validate the LWO curve calibration for CNH and BRL Curves
 * @Created:		29th May 2018
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
#include "CurveValidation.h"
#include "tryMeLWO.h"
#include "tryMeCurveDiscountFactor.h"

// Define Test Folder Path
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***
#define TEST_DIR_BRL "ETrading/LWObjects/TestLWOCurveCalibrate/BRL/"
#define TEST_DIR_CNH "ETrading/LWObjects/TestLWOCurveCalibrate/CNH/"
using etrading::ReadDataFile;

namespace
{
    extern const char fileNameBRLUSDOIS[]   = TEST_DIR_BRL "USD_OIS_CURVE.JSON";
    extern const char fileNameBRLUSD3ML[]   = TEST_DIR_BRL "USD_SWAP_3M_CURVE.JSON";
    extern const char fileNameBRLXCCY[]     = TEST_DIR_BRL "BRL_XCCYBASIS_CURVE.JSON";
    extern const char inputsBRL[]           = TEST_DIR_BRL "BRLYC_BRLDF_tryMeCurveDiscountFactorsFromTenors_inputs.csv";
    extern const char outputsBRL[]          = TEST_DIR_BRL "BRLYC_BRLDF_tryMeCurveDiscountFactorsFromTenors_outputs.csv";

    extern const char fileNameCNHUSDOIS[]   = TEST_DIR_CNH "USD_OIS_CURVE.JSON";
    extern const char fileNameCNHUSD3ML[]   = TEST_DIR_CNH "USD_SWAP_3M_CURVE.JSON";
    extern const char fileNameCNHXCCY[]     = TEST_DIR_CNH "CNH_XCCYBASIS_CURVE.JSON";
    extern const char inputsCNH[]           = TEST_DIR_CNH "CNHYC_CNHDF_tryMeCurveDiscountFactorsFromTenors_inputs.csv";
    extern const char outputsCNH[]          = TEST_DIR_CNH "CNHYC_CNHDF_tryMeCurveDiscountFactorsFromTenors_outputs.csv";
}

namespace google_test
{
    
    // Declare Test Fixture Class
    DECLARE_TEST_FIXTURE( TestMeLWOCurveCalibrate )

    // Call Test Fixture Class
    TEST_F( TestMeLWOCurveCalibrate, SNAPSHOT_BRL_CalibrateCurve_and_CheckDiscountFactors )
    {
        // Load Curves
        const std::string loadUSDOIS     = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameBRLUSDOIS,  etrading::JSON );
        const std::string loadUSD3ML     = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameBRLUSD3ML,  etrading::JSON );
        const std::string loadBRLXCCY    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameBRLXCCY,    etrading::JSON );

        // Calculate Actual Discount Factors
        const ReadDataFile::Load inputFile( inputsBRL );
        std::vector<double> discountFactorResults = validation_api::tryMeCurveDiscountFactorsFromTenors( inputFile["tenors"],
                                                                                                         inputFile["businessDayAdj"],
                                                                                                         inputFile["calendar"], 
                                                                                                         inputFile["curveCollection"],
                                                                                                         inputFile["curveIndex"] );
        // Compare Results with Stored Snapshot Results - Allow Rebasing
        const double tolerance = 1e-6;
        CheckTestResultsAndRebaseOnRequest( discountFactorResults, TEST_DIR_BRL, outputsBRL, tolerance );
        
    }

    TEST_F( TestMeLWOCurveCalibrate, SNAPSHOT_CNH_CalibrateCurve_and_CheckDiscountFactors )
    {
        // Load Curves
        const std::string loadUSDOIS     = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameCNHUSDOIS,  etrading::JSON );
        const std::string loadUSD3ML     = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameCNHUSD3ML,  etrading::JSON );
        const std::string loadCNHXCCY    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameCNHXCCY,    etrading::JSON );

        // Calculate Actual Discount Factors
        const ReadDataFile::Load inputFile( inputsCNH );
        std::vector<double> discountFactorResults = validation_api::tryMeCurveDiscountFactorsFromTenors( inputFile["tenors"],
                                                                                                         inputFile["businessDayAdj"],
                                                                                                         inputFile["calendar"], 
                                                                                                         inputFile["curveCollection"],
                                                                                                         inputFile["curveIndex"] );
        // Compare Results with Stored Snapshot Results - Allow Rebasing
        const double tolerance = 1e-6;
        CheckTestResultsAndRebaseOnRequest( discountFactorResults, TEST_DIR_CNH, outputsCNH, tolerance );
    }
}