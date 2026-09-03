// TestTradeEURAssetSwapSpread.cpp

#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "AQLCoreError.h"
#include "tryMeProductAssetSwapSpread.h"

#include <gTest/gTest.h>

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/Trades/TestTradeEURAssetSwapSpread/"

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

    const int TEST_COUNT = 11;

    //
    // curve input files
    //
    extern const char EURYC_OIS[]			= TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char EURYC_STD[]			= TEST_DIR "EURYC_STD_tryMeCurveCalibrateSwap_inputs";

    //
    // test call input and reference files
    //
    extern const char getAssetSwapSpreadInputs[]	= TEST_DIR "EURYC_tryMeProductAssetSwapSpread_inputs";
    extern const char getAssetSwapSpreadResults[]	= TEST_DIR "EURYC_tryMeProductAssetSwapSpread_outputs";
}

namespace google_test
{
    //
    // Bind Inputs to Sources
    typedef BindFileToClassConstructor<TryMeCurveOis, EURYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<TryMeCurveStd, EURYC_STD> CurveStdBound;

    //
    // Declare Dependencies
    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;

    //
    // SetUp Test Fixture
    //
    class TryMeTestTradeEURAssetSwapSpread : public virtual testing::Test, public CurveStdBuilt
    {
    };


    //
    // Call Test Fixture
    //

    TEST_F( TryMeTestTradeEURAssetSwapSpread, SNAPSHOT_CheckAssetSwapSpreadUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < TEST_COUNT; ++i )
            {
                // Load the input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getAssetSwapSpreadInputs, i ) );
                double bondPrice = inputFile["bondPrice"];
                AQLStringMatrix assetSwapLVB = inputFile["assetSwapLVB"];
                bool validateKeys = inputFile["validateKeys"];

                double assetSwapSpread = validation::tryMeProductAssetSwapSpread( bondPrice, assetSwapLVB, validateKeys );

                CheckTestResultsAndRebaseOnRequest( assetSwapSpread, TEST_DIR, getAssetSwapSpreadResults, tolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT );
        }
        catch( const AQLCoreError& m )
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
