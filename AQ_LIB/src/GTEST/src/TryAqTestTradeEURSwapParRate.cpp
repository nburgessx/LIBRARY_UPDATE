#include "Dependency.h"

#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "tryAqSwapsParRate.h"
#include "tryAqSwapsPV.h"

#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLCoreError.h"

#include <gTest/gTest.h>

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/Trades/TestTradeEURSwapParRate/"

namespace
{
    //test tolerance
    //--------------
#ifdef GTEST32
    // This Test was recorded in WIN32 so higher precision here i.e. better match against test results
    const double tolerance = 1e-8;
#else
    // The test should still pass in WIN64, but since the test was recorded using WIN32, results won't
    // match the expected hard-coded results too well
    const double tolerance = 1e-2;
#endif

    const int TEST_COUNT = 20;

    //
    // curve input files
    //
    extern const char EURYC_OIS[]			= TEST_DIR "EURYC_OIS_tryAqCurvesCalibrateOIS_inputs";
    extern const char EURYC_STD[]			= TEST_DIR "EURYC_STD_tryAqCurvesCalibrateSwap_inputs";

    //
    // test call input and reference files
    //
    extern const char getParRateInputs[]	= TEST_DIR "EURYC_tryAqSwapsParRate_inputs";
    extern const char getParRateResults[]	= TEST_DIR "EURYC_tryAqSwapsParRate_outputs";
    

    extern const char getPVInputs[]			= TEST_DIR "EURYC_tryAqSwapsPV_inputs";
#ifdef GTEST32
    extern const char getPVResults[]		= TEST_DIR "EURYC_tryAqSwapsPV_x86_outputs";
#else
    extern const char getPVResults[]		= TEST_DIR "EURYC_tryAqSwapsPV_x64_outputs";
#endif


}

namespace google_test
{
    //
    // Bind Inputs to Sources
    typedef BindFileToClassConstructor<TryAqCurvesOis, EURYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<TryAqCurvesStd, EURYC_STD> CurveStdBound;

    //
    // Declare Dependencies
    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;

    //
    // SetUp Test Fixture
    //
    class TryAqTestTradeEURSwapParRate : public virtual testing::Test, public CurveStdBuilt
    {
    };


    //
    // Call Test Fixture
    //
    TEST_F( TryAqTestTradeEURSwapParRate, SNAPSHOT_CheckSwapParRate )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < TEST_COUNT; ++i )
            {
                // Load the input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getParRateInputs, i ) );

                AQLStringMatrix swapLVB = inputFile["swapLVB"];
                double parRate = validation::tryAqSwapsParRate( swapLVB, true );

                CheckTestResultsAndRebaseOnRequest( parRate, TEST_DIR, getParRateResults, tolerance, i );
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


    TEST_F( TryAqTestTradeEURSwapParRate, SNAPSHOT_CheckSwapPV )
    {
        int i = 0;
        try
        {
			const double tolerancePV = 1.0e-5; // Notional of test trades is 1MM
            for ( i = 0; i < TEST_COUNT; ++i )
            {
                // Load the input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getPVInputs, i ) );

                // Read the input file into the getParRate and swapPV functions
                AQLStringMatrix swapLVB = inputFile["swapLVB"];
                double pv = validation::tryAqSwapsPV( swapLVB, true );

                CheckTestResultsAndRebaseOnRequest( pv, TEST_DIR, getPVResults, tolerancePV, i );
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
