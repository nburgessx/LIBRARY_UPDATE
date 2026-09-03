// TestTradeEURAssetSwapSpread.cpp

#include "CurveOis.h"
#include "CurveStd.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "CurveTenorBasis.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "LACoreError.h"
#include "tryMirAssetSwapSpread.h"

#include <gTest/gTest.h>

using google_test::CurveOis;
using google_test::CurveStd;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "Vanilla/Trades/TestTradeEURAssetSwapSpread/"

namespace
{
#ifdef GTEST32
    // This Test was recorded in WIN32 so higher precision here i.e. better match against test results

    const double tolerance = 1e-6; // result in basis points

#else
    // The test should still pass in WIN64, but since the test was recorded using WIN32, results won't
    // match the expected hard-coded results too well

    const double tolerance = 1e-3; // result in basis points

#endif

    const int minTests = 17;

    //
    // curve input files
    //
    extern const char EURYC_OIS[]			= TEST_DIR "EURYC_OIS_tryMirSetUpOISCurve_inputs";
    extern const char EURYC_1M[]			= "";
    extern const char EURYC_3M[]			= "";
    extern const char EURYC_6M[]			= TEST_DIR "EURYC_STD_tryMirSetUpSwapCurve_inputs";
    extern const char EURYC_12M[]			= "";

    //
    // test call input and reference files
    //
    extern const char getAssetSwapSpreadInputs[]	= TEST_DIR "EURYC_tryMirAssetSwapSpread_inputs";
    extern const char getAssetSwapSpreadResults[]	= TEST_DIR "EURYC_tryMirAssetSwapSpread_outputs";
}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ADD_EUR_CURVE_DEPENDENCIES( TestTradeEURAssetSwapSpread, EURYC_OIS, EURYC_1M, EURYC_3M, EURYC_6M, EURYC_12M );


    //
    // Call Test Fixture
    //

    TEST_F( TestTradeEURAssetSwapSpread, SNAPSHOT_CheckAssetSwapSpreadsAgainstSnapshot )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < minTests; ++i )
            {
                // Load the input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getAssetSwapSpreadInputs, i ) );

                // Call the asset swap spread function
                const double assetSwapSpread = validation::tryMirAssetSwapSpread( getDataInstance(),
                                               inputFile["bondPrice"],
                                               inputFile["effectDt"],
                                               inputFile["maturity"],
                                               inputFile["curveID"],
                                               inputFile["xRt"],
                                               inputFile["xFreq"],
                                               inputFile["xDayCount"],
                                               inputFile["xRollCnv"],
                                               inputFile["xCalendar"],
                                               inputFile["xFirstStub"],
                                               inputFile["xLastStub"],
                                               inputFile["xRollDay"],
                                               inputFile["xPayLag"],
                                               inputFile["xStub"],
                                               inputFile["tFreq"],
                                               inputFile["tDayCount"],
                                               inputFile["tRollCnv"],
                                               inputFile["tCalendar"],
                                               inputFile["tFirstStub"],
                                               inputFile["tLastStub"],
                                               inputFile["tRollDay"],
                                               inputFile["tFixLag"],
                                               inputFile["tFirstFix"],
                                               inputFile["tLastFix"],
                                               inputFile["tPayLag"],
                                               inputFile["tStub"],
                                               inputFile["interpolation"],
                                               inputFile["forecastCurve"],
                                               inputFile["discountCurve"],
                                               inputFile["interpFwds"],
                                               inputFile["eomRoll"],
                                               inputFile["issueDt"],
                                               inputFile["isCleanPrice"] );

                if ( etrading::CreateDataFile::rebaseResultsEnabled() )
                {
                    // Record outputs and rebase test outputs
                    etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
                    LAStringVector v = LAString( getAssetSwapSpreadResults ).toToken( '/' );
                    LAString outputFileName = v.back();
                    etrading::CreateDataFile file( CreateDataFile::makeFilename( outputFileName, i ) );
                    file.write( "output", assetSwapSpread );
                }
                else
                {
                    // Read the results file
                    const ReadDataFile::Load resultFile( CreateDataFile::makeFilename( getAssetSwapSpreadResults, i ) );
                    const double expectedRate = resultFile["output"];

                    // Compare
                    EXPECT_NEAR( assetSwapSpread, expectedRate, tolerance );
                }
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, minTests );
        }
        catch( const LACoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }
}
