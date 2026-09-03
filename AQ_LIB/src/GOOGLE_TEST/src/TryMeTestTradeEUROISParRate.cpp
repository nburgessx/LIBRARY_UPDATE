#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "TryMeCurveOis.h"
#include "tryMeProductOISParRate.h"
#include "tryMeProductOISPV.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/Trades/TestTradeEUROISParRate/"

namespace
{
    // test tolerance
    // --------------

#ifdef GTEST32
    // This Test was recorded in WIN32 so higher precision here i.e. better match against test results
    const double parRateTolerance = 1e-7;
	const double pvTolerance = 1e-5;
#else
    // The test should still pass in WIN64, but since the test was recorded using WIN32, results won't
    // match the expected hard-coded results too well
    const double parRateTolerance = 1e-5;
	const double pvTolerance = 1e-2;
#endif

    const int PARRATE_TEST_COUNT = 25;
    const int PV_TEST_COUNT		 = 25;

    //
    // Curve input files.
    //
    extern const char DIR_EURYC_OIS[]			= TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs";

    //
    // test call input and reference files
    //
    extern const char getParRateInputs[]     = TEST_DIR "EURYC_tryMeProductOISParRate_inputs";
    extern const char getParRateOutputs[]	 = TEST_DIR "EURYC_tryMeProductOISParRate_outputs";

    extern const char getPVInputs[]			 = TEST_DIR "EURYC_tryMeProductOISPV_inputs";
    extern const char getPVOutputs[]		 = TEST_DIR "EURYC_tryMeProductOISPV_outputs";
}


namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    typedef BindFileToClassConstructor<TryMeCurveOis, DIR_EURYC_OIS> CurveOisBound;
    typedef Dependency0<CurveOisBound> CurveOisBuilt;

    class TryMeTestTradeEUROISParRate : public virtual testing::Test, public CurveOisBuilt
    {
    };


    //
    // Call Test Fixture for OIS Swap par rate
    //
    TEST_F( TryMeTestTradeEUROISParRate, SNAPSHOT_CheckHardCodedOISParRateUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < PARRATE_TEST_COUNT; ++i )
            {
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getParRateInputs, i ) );

                LAStringMatrix oisLVB = inputFile["oisLVB"];
                double parRate = validation::tryMeProductOISParRate( oisLVB, true );

                if ( etrading::CreateDataFile::rebaseResultsEnabled() )
                {
                    // Record outputs and rebase test outputs
                    etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
                    LAStringVector v = LAString( getParRateOutputs ).toToken( '/' );
                    LAString outputFileName = v.back();
                    etrading::CreateDataFile file( CreateDataFile::makeFilename( outputFileName, i ) );
                    file.write( "output", parRate );
                }
                else
                {
                    const ReadDataFile::Load resultFile( CreateDataFile::makeFilename( getParRateOutputs, i ) );
                    const double expectedResult = resultFile["output"];

                    EXPECT_NEAR( expectedResult, parRate, parRateTolerance );
                }
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, PARRATE_TEST_COUNT );
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

    //
    // Call Test Fixture for OIS Swap pv
    //
    TEST_F( TryMeTestTradeEUROISParRate, SNAPSHOT_CheckHardCodedOISPVUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < PV_TEST_COUNT; ++i )
            {
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getPVInputs, i ) );

                LAStringMatrix oisLVB = inputFile["oisLVB"];

                double pv = validation::tryMeProductOISPV( oisLVB, true );

                CheckTestResultsAndRebaseOnRequest( pv, TEST_DIR, getPVOutputs, pvTolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, PV_TEST_COUNT );
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
