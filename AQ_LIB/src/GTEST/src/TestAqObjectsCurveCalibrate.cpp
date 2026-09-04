// Include: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "ResultsProcessor.h"
#include "CurveValidation.h"
#include "tryAqObjects.h"
#include "tryAqCurvesDiscountFactor.h"

// Define Test Folder Path
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***
#define TEST_DIR_BRL "ETrading/AQObjects/TestAQOCurveCalibrate/BRL/"
#define TEST_DIR_CNH "ETrading/AQObjects/TestAQOCurveCalibrate/CNH/"
using etrading::ReadDataFile;

namespace
{
    extern const char fileNameBRLUSDOIS[]   = TEST_DIR_BRL "USD_OIS_CURVE.JSON";
    extern const char fileNameBRLUSD3ML[]   = TEST_DIR_BRL "USD_SWAP_3M_CURVE.JSON";
    extern const char fileNameBRLXCCY[]     = TEST_DIR_BRL "BRL_XCCYBASIS_CURVE.JSON";
    extern const char inputsBRL[]           = TEST_DIR_BRL "BRLYC_BRLDF_tryAqCurvesDiscountFactorsFromTenors_inputs.csv";
    extern const char outputsBRL[]          = TEST_DIR_BRL "BRLYC_BRLDF_tryAqCurvesDiscountFactorsFromTenors_outputs.csv";

    extern const char fileNameCNHUSDOIS[]   = TEST_DIR_CNH "USD_OIS_CURVE.JSON";
    extern const char fileNameCNHUSD3ML[]   = TEST_DIR_CNH "USD_SWAP_3M_CURVE.JSON";
    extern const char fileNameCNHXCCY[]     = TEST_DIR_CNH "CNH_XCCYBASIS_CURVE.JSON";
    extern const char inputsCNH[]           = TEST_DIR_CNH "CNHYC_CNHDF_tryAqCurvesDiscountFactorsFromTenors_inputs.csv";
    extern const char outputsCNH[]          = TEST_DIR_CNH "CNHYC_CNHDF_tryAqCurvesDiscountFactorsFromTenors_outputs.csv";
}

namespace google_test
{
    
    // Declare Test Fixture Class
    DECLARE_TEST_FIXTURE( TestAqObjectsCurveCalibrate )

    // Call Test Fixture Class
    TEST_F( TestAqObjectsCurveCalibrate, SNAPSHOT_BRL_CalibrateCurve_and_CheckDiscountFactors )
    {
        // Load Curves
        const std::string loadUSDOIS     = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameBRLUSDOIS,  etrading::JSON );
        const std::string loadUSD3ML     = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameBRLUSD3ML,  etrading::JSON );
        const std::string loadBRLXCCY    = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameBRLXCCY,    etrading::JSON );

        // Calculate Actual Discount Factors
        const ReadDataFile::Load inputFile( inputsBRL );
        std::vector<double> discountFactorResults = validation::tryAqCurvesDiscountFactorsFromTenors( inputFile["tenors"],
                                                                                                         inputFile["businessDayAdj"],
                                                                                                         inputFile["calendar"], 
                                                                                                         inputFile["curveCollection"],
                                                                                                         inputFile["curveIndex"] );
        // Compare Results with Stored Snapshot Results - Allow Rebasing
        const double tolerance = 1e-6;
        CheckTestResultsAndRebaseOnRequest( discountFactorResults, TEST_DIR_BRL, outputsBRL, tolerance );
        
    }

    TEST_F( TestAqObjectsCurveCalibrate, SNAPSHOT_CNH_CalibrateCurve_and_CheckDiscountFactors )
    {
        // Load Curves
        const std::string loadUSDOIS     = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameCNHUSDOIS,  etrading::JSON );
        const std::string loadUSD3ML     = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameCNHUSD3ML,  etrading::JSON );
        const std::string loadCNHXCCY    = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameCNHXCCY,    etrading::JSON );

        // Calculate Actual Discount Factors
        const ReadDataFile::Load inputFile( inputsCNH );
        std::vector<double> discountFactorResults = validation::tryAqCurvesDiscountFactorsFromTenors( inputFile["tenors"],
                                                                                                         inputFile["businessDayAdj"],
                                                                                                         inputFile["calendar"], 
                                                                                                         inputFile["curveCollection"],
                                                                                                         inputFile["curveIndex"] );
        // Compare Results with Stored Snapshot Results - Allow Rebasing
        const double tolerance = 1e-6;
        CheckTestResultsAndRebaseOnRequest( discountFactorResults, TEST_DIR_CNH, outputsCNH, tolerance );
    }
}