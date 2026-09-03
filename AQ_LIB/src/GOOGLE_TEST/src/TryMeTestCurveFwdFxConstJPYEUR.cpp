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

#define TEST_DIR "ETrading/Curves/TestCurveFwdFxConstJPYEUR/"

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
    extern const char JPYYC_OIS[]			= TEST_DIR "JPYYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char JPYYC_STD[]			= TEST_DIR "JPYYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char JPYYC_3M6M[]			= TEST_DIR "JPYYC_3M6M_tryMeCurveCalibrateBasis_inputs";
    extern const char JPYYC_XCCY[]			= TEST_DIR "JPYYC_XCCY_tryMeCurveCalibrateBasis_inputs";
    extern const char EURYC_OIS[]			= TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char EURYC_STD[]			= TEST_DIR "EURYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char EURYC_3M6M[]			= TEST_DIR "EURYC_3M6M_tryMeCurveCalibrateBasis_inputs";
    extern const char EURYC_XCCY[]			= TEST_DIR "EURYC_XCCY_tryMeCurveCalibrateBasis_inputs";
    extern const char JPYYC_FWDEUR[]		= TEST_DIR "JPYYC_FWDEUR_tryMeCurveCalibrateFXForwards_inputs";

    //
    // test call input and reference files
    //
    extern const char curveDisplayInputs[]				= TEST_DIR "JPYYC_JPYDF_EURCSA_tryMeCurveDisplay_inputs";
    extern const char curveDisplayOutputs_32bit[]		= TEST_DIR "JPYYC_JPYDF_EURCSA_tryMeCurveDisplay_outputs_32bit";
	extern const char curveDisplayOutputs_64bit[]		= TEST_DIR "JPYYC_JPYDF_EURCSA_tryMeCurveDisplay_outputs_64bit";
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<TryMeCurveOis,			USDYC_OIS>		SourceUSDYC_OIS;
    typedef BindFileToClassConstructor<TryMeCurveStd,			USDYC_STD>		SourceUSDYC_STD;
    typedef BindFileToClassConstructor<TryMeCurveOis,			JPYYC_OIS>		SourceJPYYC_OIS;
    typedef BindFileToClassConstructor<TryMeCurveStd,			JPYYC_STD>		SourceJPYYC_STD;
    typedef BindFileToClassConstructor<TryMeCurveTenorBasis,	JPYYC_3M6M>		SourceJPYYC_3M6M;
    typedef BindFileToClassConstructor<TryMeCurveXccyBasis,	JPYYC_XCCY>		SourceJPYYC_XCCY;
    typedef BindFileToClassConstructor<TryMeCurveOis,			EURYC_OIS>		SourceEURYC_OIS;
    typedef BindFileToClassConstructor<TryMeCurveStd,			EURYC_STD>		SourceEURYC_STD;
    typedef BindFileToClassConstructor<TryMeCurveTenorBasis,	EURYC_3M6M>		SourceEURYC_3M6M;
    typedef BindFileToClassConstructor<TryMeCurveXccyBasis,	EURYC_XCCY>		SourceEURYC_XCCY;
    typedef BindFileToClassConstructor<TryMeCurveFwdFxConst,	JPYYC_FWDEUR>	SourceJPYYC_FWDEUR;

    //
    // Declare Dependencies
    //

    typedef Dependency0<SourceUSDYC_OIS>
    CurveUSDYC_OIS;
    typedef Dependency1<SourceUSDYC_STD, CurveUSDYC_OIS>
    CurveUSDYC_STD;
    typedef Dependency0<SourceJPYYC_OIS>
    CurveJPYYC_OIS;
    typedef Dependency1<SourceJPYYC_STD, CurveJPYYC_OIS>
    CurveJPYYC_STD;
    typedef Dependency2<SourceJPYYC_3M6M, CurveJPYYC_STD, CurveJPYYC_OIS>
    CurveJPYYC_3M6M;
    typedef Dependency4<SourceJPYYC_XCCY, CurveUSDYC_STD, CurveUSDYC_OIS, CurveJPYYC_OIS, CurveJPYYC_3M6M>
    CurveJPYYC_XCCY;
    typedef Dependency0<SourceEURYC_OIS>
    CurveEURYC_OIS;
    typedef Dependency1<SourceEURYC_STD, CurveEURYC_OIS>
    CurveEURYC_STD;
    typedef Dependency2<SourceEURYC_3M6M, CurveEURYC_STD, CurveEURYC_OIS>
    CurveEURYC_3M6M;
    typedef Dependency4<SourceEURYC_XCCY, CurveJPYYC_STD, CurveJPYYC_OIS, CurveEURYC_OIS, CurveEURYC_3M6M>
    CurveEURYC_XCCY;
    typedef Dependency4<SourceJPYYC_FWDEUR, CurveJPYYC_OIS, CurveJPYYC_XCCY, CurveEURYC_XCCY, CurveEURYC_OIS>
    CurveJPYYC_FWDEUR;

    //
    // SetUp Test Fixture
    //

    class TryMeTestCurveFwdFxConstJPYEUR : public testing::Test, public CurveJPYYC_FWDEUR
    {
        void testGraphAtCompileTime()
        {
            // make sure that conversion to test base class is unambiguous
            auto& g0 = static_cast<google_test::InitializeMLibGoogleTest&>( *this );

            // make sure that conversion to source nodes is unambiguous
            auto& s0 = static_cast<SourceUSDYC_OIS&>( *this );
            auto& s1 = static_cast<SourceUSDYC_STD&>( *this );
            auto& s2 = static_cast<SourceEURYC_OIS&>( *this );
            auto& s3 = static_cast<SourceEURYC_STD&>( *this );
            auto& s4 = static_cast<SourceEURYC_3M6M&>( *this );
            auto& s5 = static_cast<SourceEURYC_XCCY&>( *this );
            auto& s6 = static_cast<SourceJPYYC_OIS&>( *this );
            auto& s7 = static_cast<SourceJPYYC_STD&>( *this );
            auto& s8 = static_cast<SourceJPYYC_3M6M&>( *this );
            auto& s9 = static_cast<SourceJPYYC_XCCY&>( *this );
        }
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryMeTestCurveFwdFxConstJPYEUR, SNAPSHOT_CheckMeCurveDisplay )
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
