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

#define TEST_DIR "ETrading/Curves/TestCurveFwdFxConstEURGBP/"

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
    extern const char GBPYC_OIS[]			= TEST_DIR "GBPYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char GBPYC_STD[]			= TEST_DIR "GBPYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char GBPYC_3M6M[]			= TEST_DIR "GBPYC_3M6M_tryMeCurveCalibrateBasis_inputs";
    extern const char GBPYC_XCCY[]			= TEST_DIR "GBPYC_XCCY_tryMeCurveCalibrateBasis_inputs";
    extern const char EURYC_FWDGBP[]		= TEST_DIR "EURYC_FWDGBP_tryMeCurveCalibrateFXForwards_inputs";

    //
    // test call input and reference files
    //
    extern const char curveDisplayInputs[]				= TEST_DIR "EURYC_EURDF_GBPCSA_tryMeCurveDisplay_inputs";
    extern const char curveDisplayOutputs_32bit[]				= TEST_DIR "EURYC_EURDF_GBPCSA_tryMeCurveDisplay_outputs_32bit";
	extern const char curveDisplayOutputs_64bit[]				= TEST_DIR "EURYC_EURDF_GBPCSA_tryMeCurveDisplay_outputs_64bit";
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
    typedef BindFileToClassConstructor<TryMeCurveOis,			GBPYC_OIS>		SourceGBPYC_OIS;
    typedef BindFileToClassConstructor<TryMeCurveStd,			GBPYC_STD>		SourceGBPYC_STD;
    typedef BindFileToClassConstructor<TryMeCurveTenorBasis,	GBPYC_3M6M>		SourceGBPYC_3M6M;
    typedef BindFileToClassConstructor<TryMeCurveXccyBasis,	GBPYC_XCCY>		SourceGBPYC_XCCY;
    typedef BindFileToClassConstructor<TryMeCurveFwdFxConst,	EURYC_FWDGBP>	SourceEURYC_FWDGBP;

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
    typedef Dependency0<SourceGBPYC_OIS>
    CurveGBPYC_OIS;
    typedef Dependency1<SourceGBPYC_STD, CurveGBPYC_OIS>
    CurveGBPYC_STD;
    typedef Dependency2<SourceGBPYC_3M6M, CurveGBPYC_STD, CurveGBPYC_OIS>
    CurveGBPYC_3M6M;
    typedef Dependency4<SourceGBPYC_XCCY, CurveEURYC_STD, CurveEURYC_OIS, CurveGBPYC_OIS, CurveGBPYC_3M6M>
    CurveGBPYC_XCCY;
    typedef Dependency4<SourceEURYC_FWDGBP, CurveEURYC_OIS, CurveEURYC_XCCY, CurveGBPYC_XCCY, CurveGBPYC_OIS>
    CurveEURYC_FWDGBP;

    //
    // SetUp Test Fixture
    //

    class TryMeTestCurveFwdFxConstEURGBP : public testing::Test, public CurveEURYC_FWDGBP
    {
        void testGraphAtCompileTime()
        {
            // make sure that conversion to test base class is unambiguous
            // static_cast<::google_test::InitializeAQGoogleTest&>(*this);

            // make sure that conversion to source nodes is unambiguous
            auto& s0 = static_cast<SourceUSDYC_OIS&>( *this );
            auto& s1 = static_cast<SourceUSDYC_STD&>( *this );
            auto& s2 = static_cast<SourceEURYC_OIS&>( *this );
            auto& s3 = static_cast<SourceEURYC_STD&>( *this );
            auto& s4 = static_cast<SourceEURYC_3M6M&>( *this );
            auto& s5 = static_cast<SourceEURYC_XCCY&>( *this );
            auto& s6 = static_cast<SourceGBPYC_OIS&>( *this );
            auto& s7 = static_cast<SourceGBPYC_STD&>( *this );
            auto& s8 = static_cast<SourceGBPYC_3M6M&>( *this );
            auto& s9 = static_cast<SourceGBPYC_XCCY&>( *this );
        }

    };

    //
    // Call Test Fixture
    //

    TEST_F( TryMeTestCurveFwdFxConstEURGBP, SNAPSHOT_CheckMeCurveDisplay )
    {

        const ReadDataFile::Load inputFile( curveDisplayInputs );
        const DoubleArray results
            = validation::tryMeCurveDisplay(
                  inputFile["curveCollection"],
                  inputFile["curveIndex"] );

		#if defined(GTEST32)
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_32bit, tolerance );
		#else
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_64bit, tolerance );
		#endif
    }
}
