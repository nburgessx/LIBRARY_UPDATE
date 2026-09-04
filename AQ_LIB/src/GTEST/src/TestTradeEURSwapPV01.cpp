// TestTradeEURSwapParRate.cpp

#include "InterestRateSwap.h"
#include "Dependency.h"
#include "AQLDataInstance.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLCoreError.h"

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

#define TEST_DIR "Vanilla/Trades/TestTradeEURSwapPV01/"

namespace
{
    // test tolerance
    const double tolerance = 1e-2;	// This is a delta calculation so the tolerance cannot be so tight

    const int minTests = 5;

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
    extern const char getPV01Inputs[]		= TEST_DIR "EURYC_tryMirSwapPV01_inputs";
    extern const char getPV01Results[]		= TEST_DIR "EURYC_tryMirSwapPV01_outputs";

}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ADD_EUR_CURVE_DEPENDENCIES( TestTradeEURSwapPV01, EURYC_OIS, EURYC_1M, EURYC_3M, EURYC_6M, EURYC_12M );


    //
    // Call Test Fixture
    //

    TEST_F( TestTradeEURSwapPV01, SNAPSHOT_CheckHardCodedSwapPV01Unchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; ; ++i )
            {
                // Load the input file
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getPV01Inputs, i ) );

                // Read the input file into the getParRateFunction
                InterestRateSwap irs( getDataInstance(), inputFile );
                const double pv01 = irs.pv01();

                CheckTestResultsAndRebaseOnRequest( pv01, TEST_DIR, getPV01Results, tolerance, i );
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
