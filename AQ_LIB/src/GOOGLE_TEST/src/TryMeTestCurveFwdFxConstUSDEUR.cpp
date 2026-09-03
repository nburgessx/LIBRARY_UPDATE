#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"
#include "TryMeCurveXccyBasis.h"
#include "TryMeCurveFwdFxConst.h"
#include "tryMeCurveDisplay.h"
#include "ResultsProcessor.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"

#include <gTest/gTest.h>

using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Curves/TestCurveFwdFxConstUSDEUR/"

namespace
{

    // test tolerance
    // --------------

#ifdef GTEST32
    // This Test was recorded in WIN32 so higher precision here i.e. better match against test results
    const double tolerance = 1e-8;
#else
    // The test should still pass in WIN64, but since the test was recorded using WIN32, results won't
    // match the expected hard-coded results too well
    const double tolerance = 1e-3;
#endif



    //
    // curve input files
    //
    extern const char USDYC_OIS[]			= TEST_DIR "USDYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char USDYC_STD[]			= TEST_DIR "USDYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char EURYC_OIS[]			= TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char EURYC_STD[]			= TEST_DIR "EURYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char EURYC_3M6M[]			= TEST_DIR "EURYC_3M6M_tryMeCurveCalibrateBasis_inputs";
    extern const char EURYC_XCCY[]			= TEST_DIR "EURYC_XCCY_tryMeCurveCalibrateBasis_inputs";
    extern const char USDYC_FWDEUR[]		= TEST_DIR "USDYC_FWDEUR_tryMeCurveCalibrateFXForwards_inputs";

    //
    // test call input and reference files
    //
    extern const char curveDisplayInputs[]				= TEST_DIR "USDYC_USDDF_EURCSA_tryMeCurveDisplay_inputs";
    extern const char curveDisplayOutputs_32bit[]				= TEST_DIR "USDYC_USDDF_EURCSA_tryMeCurveDisplay_outputs_32bit";
	extern const char curveDisplayOutputs_64bit[]				= TEST_DIR "USDYC_USDDF_EURCSA_tryMeCurveDisplay_outputs_64bit";
}

namespace google_test
{
    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<TryMeCurveOis,			USDYC_OIS>		SourceUSDYC_OIS;
    typedef BindFileToClassConstructor<TryMeCurveStd,			USDYC_STD>		SourceUSDYC_STD;
    typedef BindFileToClassConstructor<TryMeCurveOis,			EURYC_OIS>		SourceEURYC_OIS;
    typedef BindFileToClassConstructor<TryMeCurveStd,			EURYC_STD>		SourceEURYC_STD;
    typedef BindFileToClassConstructor<TryMeCurveTenorBasis,	EURYC_3M6M>		SourceEURYC_3M6M;
    typedef BindFileToClassConstructor<TryMeCurveXccyBasis,	EURYC_XCCY>		SourceEURYC_XCCY;
    typedef BindFileToClassConstructor<TryMeCurveFwdFxConst,	USDYC_FWDEUR>	SourceUSDYC_FWDEUR;

    //
    // Declare Dependencies
    //

    typedef Dependency0<SourceUSDYC_OIS>
    CurveUSDYC_OIS;
    typedef Dependency1<SourceUSDYC_STD, CurveUSDYC_OIS>
    CurveUSDYC_STD;
    typedef Dependency0<SourceEURYC_OIS>
    CurveEURYC_OIS;
    typedef Dependency1<SourceEURYC_STD, CurveEURYC_OIS>
    CurveEURYC_STD;
    typedef Dependency2<SourceEURYC_3M6M, CurveEURYC_STD, CurveEURYC_OIS>
    CurveEURYC_3M6M;
    typedef Dependency4<SourceEURYC_XCCY, CurveUSDYC_STD, CurveUSDYC_OIS, CurveEURYC_OIS, CurveEURYC_3M6M>
    CurveEURYC_XCCY;
    typedef Dependency3<SourceUSDYC_FWDEUR, CurveUSDYC_OIS, CurveEURYC_XCCY, CurveEURYC_OIS>
    CurveUSDYC_FWDEUR;

    //
    // SetUp Test Fixture
    //

    class TryMeTestCurveFwdFxConstUSDEUR : public testing::Test, public CurveUSDYC_FWDEUR
    {
        void testGraphAtCompileTime()
        {
            // make sure that conversion to InitializeMLibGoogleTest base class is unambiguous
            auto& g0 = static_cast<google_test::InitializeMLibGoogleTest&>( *this );

            // make sure that conversion to source nodes is unambiguous
            auto& s0 = static_cast<SourceUSDYC_OIS&>( *this );
            auto& s1 = static_cast<SourceUSDYC_STD&>( *this );
            auto& s2 = static_cast<SourceEURYC_OIS&>( *this );
            auto& s3 = static_cast<SourceEURYC_STD&>( *this );
            auto& s4 = static_cast<SourceEURYC_3M6M&>( *this );
            auto& s5 = static_cast<SourceEURYC_XCCY&>( *this );
        }
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryMeTestCurveFwdFxConstUSDEUR, SNAPSHOT_CheckMeCurveDisplay )
    {
        const ReadDataFile::Load inputFile( curveDisplayInputs );
        const DoubleArray results
            = validation_api::tryMeCurveDisplay(
                  inputFile["curveCollection"],
                  inputFile["curveIndex"] );

        #if defined(GTEST32)
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_32bit, tolerance );
		#else
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_64bit, tolerance );
		#endif
    }
}