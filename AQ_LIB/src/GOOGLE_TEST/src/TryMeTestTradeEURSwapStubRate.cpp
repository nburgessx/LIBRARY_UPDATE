#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"
#include "tryMeProductSwapStubRate.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/Trades/TestTradeEURSwapStubRate/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;	// solver tolerance is 1e-9

    const int TEST1_COUNT = 13;
    const int TEST2_COUNT = 11;
    const int TEST3_COUNT = 5;
    const int TEST4_COUNT = 5;

    //
    // Curve input files.
    //
    extern const char Test1_DIR_EURYC_OIS[]			= TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char Test1_DIR_EURYC_1M[]			= TEST_DIR "EURYC_1M3M_tryMeCurveCalibrateBasis_inputs";
    extern const char Test1_DIR_EURYC_3M[]			= TEST_DIR "EURYC_3M6M_tryMeCurveCalibrateBasis_inputs";
    extern const char Test1_DIR_EURYC_6M[]			= TEST_DIR "EURYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char Test1_DIR_EURYC_12M[]			= TEST_DIR "EURYC_3M12MBasis_tryMeCurveCalibrateBasis_inputs";

    extern const char Test2_DIR_EURYC_OIS[]			= TEST_DIR "OIS_inputs";
    extern const char Test2_DIR_EURYC_1M[]			= TEST_DIR "1M_inputs";
    extern const char Test2_DIR_EURYC_3M[]			= TEST_DIR "3M_inputs";
    extern const char Test2_DIR_EURYC_6M[]			= TEST_DIR "SwapCurve_inputs";
    extern const char Test2_DIR_EURYC_12M[]			= TEST_DIR "12M_inputs";

    extern const char Test3_DIR_EURYC_OIS[]			= TEST_DIR "Test3_EURYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char Test3_DIR_EURYC_1M[]			= TEST_DIR "Test3_EURYC_1M3M_tryMeCurveCalibrateBasis_inputs";
    extern const char Test3_DIR_EURYC_3M[]			= TEST_DIR "Test3_EURYC_3M6M_tryMeCurveCalibrateBasis_inputs";
    extern const char Test3_DIR_EURYC_6M[]			= TEST_DIR "Test3_EURYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char Test3_DIR_EURYC_12M[]			= TEST_DIR "Test3_EURYC_6M12MBasis_tryMeCurveCalibrateBasis_inputs";

    extern const char Test4_DIR_EURYC_OIS[]			= TEST_DIR "Test4_EURYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char Test4_DIR_EURYC_1M[]			= TEST_DIR "Test4_EURYC_1M3M_tryMeCurveCalibrateBasis_inputs";
    extern const char Test4_DIR_EURYC_3M[]			= TEST_DIR "Test4_EURYC_3M6M_tryMeCurveCalibrateBasis_inputs";
    extern const char Test4_DIR_EURYC_6M[]			= TEST_DIR "Test4_EURYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char Test4_DIR_EURYC_12M[]			= TEST_DIR "Test4_EURYC_6M12MBasis_tryMeCurveCalibrateBasis_inputs";
    
    //
    // test call input and reference files
    //
    extern const char Test1_getStubRateInputs[]		= TEST_DIR "EURYC_tryMeProductSwapStubRate_inputs";
    extern const char Test1_getStubRateOutputs[]	= TEST_DIR "EURYC_tryMeProductSwapStubRate_outputs";

    extern const char Test2_getStubRateInputs[]		= TEST_DIR "Test2_MeProductSwapStubRate_inputs";
    extern const char Test2_getStubRateOutputs[]	= TEST_DIR "Test2_MeProductSwapStubRate_outputs";

    extern const char Test3_getStubRateInputs[]		= TEST_DIR "Test3_EURYC_tryMeProductSwapStubRate_inputs";
    extern const char Test3_getStubRateOutputs[]	= TEST_DIR "Test3_EURYC_tryMeProductSwapStubRate_outputs";

    extern const char Test4_getStubRateInputs[]		= TEST_DIR "Test4_EURYC_tryMeProductSwapStubRate_inputs";
    extern const char Test4_getStubRateOutputs[]	= TEST_DIR "Test4_EURYC_tryMeProductSwapStubRate_outputs";
}


namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ME_BUILD_EUR_CURVE( TryMeTestTradeEURSwapStubRate, Test1_DIR_EURYC_OIS, Test1_DIR_EURYC_1M, Test1_DIR_EURYC_3M, Test1_DIR_EURYC_6M, Test1_DIR_EURYC_12M );

    //
    // Call Test Fixture
    //
    TEST_F( TryMeTestTradeEURSwapStubRate, SNAPSHOT_CheckHardCodedSwapStubRateUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < TEST1_COUNT; ++i )
            {

                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( Test1_getStubRateInputs, i ) );

                LAStringVector curveIndices = inputFile["curveIndices"];
                LAStringVector curveTenors = inputFile["curveTenors"];
                DoubleVector tenorCurveFixings = inputFile["tenorCurveFixings"];
                LAStringMatrix swapLVB = inputFile["swapLVB"];

                double stubRate = validation::tryMeProductSwapStubRate( swapLVB, curveIndices, curveTenors, tenorCurveFixings, true );

                CheckTestResultsAndRebaseOnRequest( stubRate, TEST_DIR, Test1_getStubRateOutputs, tolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST1_COUNT );
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
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ME_BUILD_EUR_CURVE( TryMeTestTradeEURSwapStubRate2, Test2_DIR_EURYC_OIS, Test2_DIR_EURYC_1M, Test2_DIR_EURYC_3M, Test2_DIR_EURYC_6M, Test2_DIR_EURYC_12M );

    //
    // Call Test Fixture
    //
    TEST_F( TryMeTestTradeEURSwapStubRate2, SNAPSHOT_CheckHardCodedSwapStubRateUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < TEST2_COUNT; ++i )
            {

                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( Test2_getStubRateInputs, i ) );

                LAStringVector curveIndices = inputFile["curveIndices"];
                LAStringVector curveTenors = inputFile["curveTenors"];
                DoubleVector tenorCurveFixings = inputFile["tenorCurveFixings"];
                LAStringMatrix swapLVB = inputFile["swapLVB"];

                double stubRate = validation::tryMeProductSwapStubRate( swapLVB, curveIndices, curveTenors, tenorCurveFixings, false );

                CheckTestResultsAndRebaseOnRequest( stubRate, TEST_DIR, Test2_getStubRateOutputs, tolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST2_COUNT );
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
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ME_BUILD_EUR_CURVE( TryMeTestTradeEURSwapStubRate3, Test3_DIR_EURYC_OIS, Test3_DIR_EURYC_1M, Test3_DIR_EURYC_3M, Test3_DIR_EURYC_6M, Test3_DIR_EURYC_12M );

    //
    // Call Test Fixture
    //
    TEST_F( TryMeTestTradeEURSwapStubRate3, CONSISTENCY_CheckStubRatesMatchFixingInputs )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < TEST3_COUNT; ++i )
            {

                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( Test3_getStubRateInputs, i ) );

                LAStringVector curveIndices = inputFile["curveIndices"];
                LAStringVector curveTenors = inputFile["curveTenors"];
                DoubleVector tenorCurveFixings = inputFile["tenorCurveFixings"];
                LAStringMatrix swapLVB = inputFile["swapLVB"];

                double stubRate = validation::tryMeProductSwapStubRate( swapLVB, curveIndices, curveTenors, tenorCurveFixings, false );

                CheckTestResultsAndRebaseOnRequest( stubRate, TEST_DIR, Test3_getStubRateOutputs, tolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST3_COUNT );
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
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    
    ME_BUILD_EUR_CURVE( TryMeTestTradeEURSwapStubRate4, Test4_DIR_EURYC_OIS, Test4_DIR_EURYC_1M, Test4_DIR_EURYC_3M, Test4_DIR_EURYC_6M, Test4_DIR_EURYC_12M );
    
    //
    // Call Test Fixture
    //
    TEST_F( TryMeTestTradeEURSwapStubRate4, SNAPSHOT_CheckStubRateInterpolationOnFixings )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < TEST4_COUNT; ++i )
            {

                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( Test4_getStubRateInputs, i ) );

                LAStringVector curveIndices = inputFile["curveIndices"];
                LAStringVector curveTenors = inputFile["curveTenors"];
                DoubleVector tenorCurveFixings = inputFile["tenorCurveFixings"];
                LAStringMatrix swapLVB = inputFile["swapLVB"];

                double stubRate = validation::tryMeProductSwapStubRate( swapLVB, curveIndices, curveTenors, tenorCurveFixings, false );

                CheckTestResultsAndRebaseOnRequest( stubRate, TEST_DIR, Test4_getStubRateOutputs, tolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST4_COUNT );
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
