#include "InterestRateSwap.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "tryMirSwapStubRate.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "Vanilla/Trades/TestTradeEURSwapStubRate/"
#define TEST_DIR2 "Vanilla/Trades/TestTradeEURSwapStubRateUsingTenor/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;	// solver tolerance is 1e-9

    const int TEST_COUNT = 13;

    //
    // Curve input files.
    //
    extern const char DIR_EURYC_OIS[]			        = TEST_DIR "EURYC_OIS_tryMirSetUpOISCurve_inputs";
    extern const char DIR_EURYC_1M[]			        = TEST_DIR "EURYC_1M3M_tryMirSetUpBasisSwapCurve_inputs";
    extern const char DIR_EURYC_3M[]			        = TEST_DIR "EURYC_3M6M_tryMirSetUpBasisSwapCurve_inputs";
    extern const char DIR_EURYC_6M[]			        = TEST_DIR "EURYC_STD_tryMirSetUpSwapCurve_inputs";
    extern const char DIR_EURYC_12M[]			        = TEST_DIR "EURYC_3M12MBasis_tryMirSetUpBasisSwapCurve_inputs";

    //
    // test call input and reference files
    //
    
    // Stub Tests using Date Maturity String
    extern const char getStubRateInputs[]               = TEST_DIR "EURYC_tryMirSwapStubRate_inputs";
    extern const char getStubRateOutputs[]	            = TEST_DIR "EURYC_tryMirSwapStubRate_outputs";

    // Stub Tests using Tenor Maturity String
    extern const char getStubRateInputsUsingTenor[]     = TEST_DIR2 "EURYC_tryMirSwapStubRate_inputs";
    extern const char getStubRateOutputsUsingTenor[]    = TEST_DIR2 "EURYC_tryMirSwapStubRate_outputs";
}


namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ADD_EUR_CURVE_DEPENDENCIES( TestTradeEURSwapStubRate, DIR_EURYC_OIS, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );

    //
    // Call Test Fixture
    //
    TEST_F( TestTradeEURSwapStubRate, SNAPSHOT_CheckHardCodedSwapStubRateUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < TEST_COUNT; ++i )
            {

                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getStubRateInputs, i ) );

                const double stubRate = validation::tryMirSwapStubRate(
                                            getDataInstance(),
                                            inputFile["effDt"],
                                            inputFile["mat"],
                                            inputFile["freq"],
                                            inputFile["dayCt"],
                                            inputFile["rollCnv"],
                                            inputFile["cal"],
                                            inputFile["rollDay"],
                                            inputFile["fixLag"],
                                            inputFile["eomRoll"],
                                            inputFile["interpolation"],
                                            inputFile["firstStub"],
                                            inputFile["lastStub"],
                                            inputFile["stub"],
                                            inputFile["crvID"],
                                            inputFile["curveNames"],
                                            inputFile["curveTenors"],
                                            inputFile["tenorCurveFixings"],
                                            inputFile["useCurveName"],
                                            inputFile["toleranceTenor"],
                                            inputFile["isFwdInterp"],
                                            inputFile["useFwdData"] );

                CheckTestResultsAndRebaseOnRequest( stubRate, TEST_DIR, getStubRateOutputs, tolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, TEST_COUNT );
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
    // Call Test Fixture
    //
    TEST_F( TestTradeEURSwapStubRate, SNAPSHOT_CheckHardCodedSwapStubRateUnchangedUsingTenorMaturities)
    {
        int i = 0;
        try
        {
            for ( i = 0; i < TEST_COUNT; ++i )
            {

                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getStubRateInputsUsingTenor, i ) );

                const double stubRate = validation::tryMirSwapStubRate( getDataInstance(),
                                                                            inputFile["effDt"],
                                                                            inputFile["mat"],
                                                                            inputFile["freq"],
                                                                            inputFile["dayCt"],
                                                                            inputFile["rollCnv"],
                                                                            inputFile["cal"],
                                                                            inputFile["rollDay"],
                                                                            inputFile["fixLag"],
                                                                            inputFile["eomRoll"],
                                                                            inputFile["interpolation"],
                                                                            inputFile["firstStub"],
                                                                            inputFile["lastStub"],
                                                                            inputFile["stub"],
                                                                            inputFile["crvID"],
                                                                            inputFile["curveNames"],
                                                                            inputFile["curveTenors"],
                                                                            inputFile["tenorCurveFixings"],
                                                                            inputFile["useCurveName"],
                                                                            inputFile["toleranceTenor"],
                                                                            inputFile["isFwdInterp"],
                                                                            inputFile["useFwdData"] );

                CheckTestResultsAndRebaseOnRequest( stubRate, TEST_DIR2, getStubRateOutputsUsingTenor, tolerance, i );
            }
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
