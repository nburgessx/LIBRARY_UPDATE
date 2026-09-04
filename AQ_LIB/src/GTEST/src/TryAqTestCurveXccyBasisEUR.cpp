#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "TryAqCurvesXccyBasis.h"

#include "tryAqCurvesDisplay.h"
#include "ResultsProcessor.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"

#include <gTest/gTest.h>

using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using google_test::Dependency2;
using google_test::Dependency3;
using google_test::Dependency4;

using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Curves/TestCurveXccyBasisEUR/"

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

    //
    // test call input and reference files
    //
    extern const char curveDisplayInputs[]				= TEST_DIR "EURYC_XCCY_tryMeCurveDisplay_inputs.csv";
    extern const char curveDisplayOutputs_32bit[]		= TEST_DIR "EURYC_XCCY_tryMeCurveDisplay_outputs_32bit.csv";
	extern const char curveDisplayOutputs_64bit[]		= TEST_DIR "EURYC_XCCY_tryMeCurveDisplay_outputs_64bit.csv";

}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<TryAqCurvesOis,			USDYC_OIS>		SourceUSDYC_OIS;
    typedef BindFileToClassConstructor<TryAqCurvesStd,			USDYC_STD>		SourceUSDYC_STD;
    typedef BindFileToClassConstructor<TryAqCurvesOis,			EURYC_OIS>		SourceEURYC_OIS;
    typedef BindFileToClassConstructor<TryAqCurvesStd,			EURYC_STD>		SourceEURYC_STD;
    typedef BindFileToClassConstructor<TryAqCurvesTenorBasis,	EURYC_3M6M>		SourceEURYC_3M6M;
    typedef BindFileToClassConstructor<TryAqCurvesXccyBasis,	EURYC_XCCY>		SourceEURYC_XCCY;

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

    //
    // SetUp Test Fixture
    //

    class TryAqTestCurveXccyBasisEUR: public testing::Test, public CurveEURYC_XCCY
    {
        void testGraphAtCompileTime()
        {
            // make sure that conversion to test base class is unambiguous
            auto& g0 = static_cast<google_test::InitializeGoogleTest&>( *this );

            // make sure that conversion to source nodes is unambiguous
            auto& s0 = static_cast<SourceUSDYC_OIS&>( *this );
            auto& s1 = static_cast<SourceUSDYC_STD&>( *this );
            auto& s2 = static_cast<SourceEURYC_OIS&>( *this );
            auto& s3 = static_cast<SourceEURYC_STD&>( *this );
            auto& s4 = static_cast<SourceEURYC_3M6M&>( *this );
            auto& s5 = static_cast<SourceEURYC_XCCY&>( *this );

            // make sure that conversion to source nodes is unambiguous
            auto& c0 = static_cast<CurveUSDYC_OIS&>( *this );
            auto& c1 = static_cast<CurveUSDYC_STD&>( *this );
            auto& c2 = static_cast<CurveEURYC_OIS&>( *this );
            auto& c3 = static_cast<CurveEURYC_STD&>( *this );
            auto& c4 = static_cast<CurveEURYC_3M6M&>( *this );
            auto& c5 = static_cast<CurveEURYC_XCCY&>( *this );
        }
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryAqTestCurveXccyBasisEUR, SNAPSHOT_CheckMeCurveDisplay )
    {
        const ReadDataFile::Load inputFile( curveDisplayInputs );
        const DoubleArray results
            = validation::tryAqCurvesDisplay(
                  inputFile["curveCollection"],
                  inputFile["curveIndex"] );
        
		#if defined(GTEST32)
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_32bit, tolerance );
		#else
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_64bit, tolerance );
		#endif
    }
}

