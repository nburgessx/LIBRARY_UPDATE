// TestTradeEURSwapParRate.cpp

#include "InterestRateSwap.h"
#include "Dependency.h"
#include "LADataInstance.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "LACurveForwardRateHelpers.h"
#include "LACoreError.h"

#include <gTest/gTest.h>


using google_test::InterestRateSwap;
using google_test::CurveOis;
using google_test::CurveStd;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using google_test::Dependency2;
using google_test::Dependency3;
using google_test::Dependency4;

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "Vanilla/Trades/TestTradeEURSwapParRate/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;	// solver tolerance is 1e-9

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
    extern const char getParRateInputs[]	= TEST_DIR "EURYC_tryMirGetParRate4_inputs";
    extern const char getParRateResults[]	= TEST_DIR "EURYC_tryMirGetParRate4_outputs";

}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ADD_EUR_CURVE_DEPENDENCIES( TestTradeEURSwapParRate, EURYC_OIS, EURYC_1M, EURYC_3M, EURYC_6M, EURYC_12M );


    //
    // Call Test Fixture
    //

    TEST_F( TestTradeEURSwapParRate, SNAPSHOT_CheckHardCodedSwapParRateUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; ; ++i )
            {
                // Load the input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getParRateInputs, i ) );

                // Read the input file into the getParRateFunction
                InterestRateSwap irs( getDataInstance(), inputFile );
                const double parRate = irs.parRate();

                CheckTestResultsAndRebaseOnRequest( parRate, TEST_DIR, getParRateResults, tolerance, i );
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

    TEST_F( TestTradeEURSwapParRate, SNAPSHOT_CheckSwapParRateAgainstYieldCurveParRate )
    {
        int i = 0;
        try
        {
            for ( i = 0; ; ++i )
            {
                // Load the input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getParRateInputs, i ) );

                // Read the input file into the getParRateFunction
                InterestRateSwap irs( getDataInstance(), inputFile );
                const double parRate = irs.parRate();

                // Read the Swap Rates from the curve dependency input file
                const double expectedRate = boost::lexical_cast<double>( swapRates()( i, "col1" ) );

                // Compare Results
                EXPECT_NEAR( parRate, expectedRate, tolerance );
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

    TEST_F( TestTradeEURSwapParRate, SNAPSHOT_CheckParSwapPriceIsZero )
    {
        int i = 0;
        try
        {
            for ( i = 0; ; ++i )
            {
                // Load the input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getParRateInputs, i ) );

                // Read the input file into the getParRate and swapPV functions
                InterestRateSwap irs( getDataInstance(), inputFile );
                const double parRate = irs.parRate();
                const double parPV = irs.pv( parRate );

                // Compare Results
                EXPECT_NEAR( parPV, 0, tolerance );
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
