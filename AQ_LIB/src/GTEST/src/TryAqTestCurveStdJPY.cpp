#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"

#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "tryAqCurveDisplay.h"
#include "tryAqCurveDiscountFactor.h"
#include "tryAqCurveForwardRate.h"

#include <gTest/gTest.h>

using google_test::TryAqCurvesOis;
using google_test::TryAqCurvesStd;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Curves/TestCurveStdJPY/"

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
    const double tolerance = 1e-5;
#endif


    //
    // curve input files
    //
    extern const char JPYYC_OIS[]					= TEST_DIR "JPYYC_OIS_tryAqCurveCalibrateOIS_inputs.csv";
    extern const char JPYYC_STD[]					= TEST_DIR "JPYYC_STD_tryAqCurveCalibrateSwap_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char curveDisplayInputs[]				= TEST_DIR "JPYYC_STD_tryAqCurveDisplay_inputs.csv";
    extern const char curveDisplayOutputs[]				= TEST_DIR "JPYYC_STD_tryAqCurveDisplay_outputs.csv";

    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "JPYYC_STD_tryAqCurveDiscountFactorsFromYearFractions_inputs.csv";
    extern const char CheckDiscountFactorsOutputs[]	= TEST_DIR "JPYYC_STD_tryAqCurveDiscountFactorsFromYearFractions_outputs.csv";

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "JPYYC_STD_tryAqCurveForwardRatesFromYearFraction_inputs.csv";
    extern const char CheckForwardRatesOutputs[]		= TEST_DIR "JPYYC_STD_tryAqCurveForwardRatesFromYearFraction_outputs.csv";
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<TryAqCurvesOis, JPYYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<TryAqCurvesStd, JPYYC_STD> CurveStdBound;

    //
    // Declare Dependencies
    //

    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;

    //
    // SetUp Test Fixture
    //

    class TryAqTestCurveStdJPY : public virtual testing::Test, public CurveStdBuilt
    {
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryAqTestCurveStdJPY, SNAPSHOT_CheckMeCurveDisplay )
    {

        const ReadDataFile::Load inputFile( curveDisplayInputs );

        const DoubleArray results
            = validation::tryAqCurveDisplay(
                  inputFile["curveCollection"],
                  inputFile["curveIndex"] );

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs, tolerance );
    }

    TEST_F( TryAqTestCurveStdJPY, SNAPSHOT_CheckMeCurveDiscountFactorsFromYearFractions )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs );

        const DoubleArray yearFractions = inputFile["yearFractions"];

        DoubleArray results = validation::tryAqCurveDiscountFactorsFromYearFractions( yearFractions,
                              inputFile["dayCount"],
                              inputFile["curveCollection"],
                              inputFile["curveIndex"] );

        ASSERT_EQ( yearFractions.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckDiscountFactorsOutputs, tolerance );
    }

    TEST_F( TryAqTestCurveStdJPY, SNAPSHOT_CheckMeCurveForwardRatesFromYearFraction )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        const double yearFraction = inputFile["yearFraction"];

        const DateVector fromDateVector = inputFile["fromDates"];

        const DoubleArray results = validation::tryAqCurveForwardRatesFromYearFraction( fromDateVector,
                                    yearFraction,
                                    inputFile["dayCount"],
                                    inputFile["curveCollection"],
                                    inputFile["curveIndex"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckForwardRatesOutputs, tolerance );
    }

}
