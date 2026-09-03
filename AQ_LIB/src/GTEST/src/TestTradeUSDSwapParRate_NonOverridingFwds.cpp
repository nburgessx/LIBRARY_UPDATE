// TestTradeUSDSwapParRate.cpp

#include "InterestRateSwap.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "CurveTenorBasis.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"

#include <gTest/gTest.h>

using google_test::InterestRateSwap;
using google_test::CurveOis;
using google_test::CurveStd;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "Vanilla/Trades/TestTradeUSDSwapParRate_NonOverridingFwds/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;	// solver tolerance is 1e-9

    const int minTests = 10;

    //
    // curve input files
    //
    extern const char USDYC_OIS[]			= TEST_DIR "USDYC_OIS_tryMirSetUpOISCurve_inputs";
    extern const char USDYC_1M[]			= "";
    extern const char USDYC_3M[]			= TEST_DIR "USDYC_STD_tryMirSetUpSwapCurve_inputs";
    extern const char USDYC_6M[]			= "";
    extern const char USDYC_12M[]			= "";


    //
    // test call input and reference files
    //
    extern const char getParRateInputs[]	= TEST_DIR "USDYC_tryMirGetParRate4_inputs";
    extern const char getParRateOutputs[]	= TEST_DIR "USDYC_tryMirGetParRate4_outputs";
}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_USD_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ADD_USD_CURVE_DEPENDENCIES( TestTradeUSDSwapParRate_NonOverridingFwds, USDYC_OIS, USDYC_1M, USDYC_3M, USDYC_6M, USDYC_12M );


    //
    // Call Test Fixture
    //

    TEST_F( TestTradeUSDSwapParRate_NonOverridingFwds, SNAPSHOT_CheckHardCodedSwapParRateUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; ; ++i )
            {
                // Load the input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getParRateInputs, i ) );

                // Read input file into the getParRate function
                InterestRateSwap irs( getDataInstance(), inputFile );

                const double parRate = irs.parRate();

                CheckTestResultsAndRebaseOnRequest( parRate, TEST_DIR, getParRateOutputs, tolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, minTests );
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
	    
    TEST_F( TestTradeUSDSwapParRate_NonOverridingFwds, SNAPSHOT_CheckParSwapPriceIsZero )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < minTests ; ++i )
            {
                // Load input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getParRateInputs, i ) );

                // Read input file into the getParRate and swapPV functions
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
