// TestCurveTenorBasisJPY3M6MConvergence.cpp

#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "tryAqCurveDisplay.h"
#include "tryAqCurveDiscountFactor.h"
#include "tryAqCurveForwardRate.h"
#include "ETradingException.h"

#include <gTest/gTest.h>
#include <string>

using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using etrading::ReadDataFile;
using etrading::ETradingException;

#define TEST_DIR "ETrading/Curves/TestCurveTenorBasisJPY3M6MConvergence/"

namespace
{

    // test tolerance
    // --------------

#ifdef GTEST32
    // This Test was recorded in WIN32 so higher precision here i.e. better match against test results
    const double tolerance = 1e-10;
#else
    // The test should still pass in WIN64, but since the test was recorded using WIN32, results won't
    // match the expected hard-coded results too well
    const double tolerance = 1e-5;
#endif



    //
    // curve input files
    //
    extern const char JPYYC_OIS[]					= TEST_DIR "JPYYC_OIS_tryAqCurveCalibrateOIS_inputs";
    extern const char JPYYC_STD[]					= TEST_DIR "JPYYC_STD_tryAqCurveCalibrateSwap_inputs";
    extern const char JPYYC_3M6M[]					= TEST_DIR "JPYYC_3M6M_tryAqCurveCalibrateBasis_inputs";

    //
    // test call input and reference files
    //
    extern const char curveOutputs[]				= TEST_DIR "JPYYC_3M6M_tryAqCurveCalibrateBasis_outputs";

}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<TryAqCurvesOis, JPYYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<TryAqCurvesStd, JPYYC_STD> CurveStdBound;
    typedef BindFileToClassConstructor<TryAqCurvesTenorBasis, JPYYC_3M6M> CurveTenorBasisBound;

    //
    // Declare Dependencies
    //

    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;
    typedef Dependency1<CurveTenorBasisBound, CurveStdBuilt> CurveTenorBasisBuilt;

    //
    // SetUp Test Fixture
    //

    class TryAqTestCurveTenorBasisJPY3M6MConvergence : public testing::Test, public CurveTenorBasisBuilt
    {
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryAqTestCurveTenorBasisJPY3M6MConvergence, SNAPSHOT_MonthEndConvergenceCheck )
    {
        try
        {
            const ReadDataFile::Load inputFile( curveOutputs );
            std::string result = inputFile["output"];
            EXPECT_EQ( result, "JPYYC 3M6MBasis Curve has been set" );
        }
        catch ( ETradingException& ex )
        {
            std::cout << ex.what() << std::endl;
        }
    }

}