#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"

#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "tryMeCurveDisplay.h"
#include "tryMeCurveDiscountFactor.h"
#include "tryMeCurveForwardRate.h"

#include <gTest/gTest.h>

using google_test::TryMeCurveOis;
using google_test::TryMeCurveStd;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Curves/TestCurveStdGBP/"

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
    extern const char GBPYC_OIS[]					= TEST_DIR "GBPYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char GBPYC_STD[]					= TEST_DIR "GBPYC_STD_tryMeCurveCalibrateSwap_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char curveDisplayInputs[]				= TEST_DIR "GBPYC_STD_tryMeCurveDisplay_inputs.csv";
    extern const char curveDisplayOutputs[]				= TEST_DIR "GBPYC_STD_tryMeCurveDisplay_outputs.csv";

    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "GBPYC_STD_tryMeCurveDiscountFactorsFromYearFractions_inputs.csv";
    extern const char CheckDiscountFactorsOutputs[]	    = TEST_DIR "GBPYC_STD_tryMeCurveDiscountFactorsFromYearFractions_outputs.csv";

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "GBPYC_STD_tryMeCurveForwardRatesFromYearFraction_inputs.csv";
    extern const char CheckForwardRatesOutputs[]		= TEST_DIR "GBPYC_STD_tryMeCurveForwardRatesFromYearFraction_outputs.csv";
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<TryMeCurveOis, GBPYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<TryMeCurveStd, GBPYC_STD> CurveStdBound;

    //
    // Declare Dependencies
    //

    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;

    //
    // SetUp Test Fixture
    //

    class TryMeTestCurveStdGBP : public virtual testing::Test, public CurveStdBuilt
    {
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryMeTestCurveStdGBP, SNAPSHOT_CheckMeCurveDisplay )
    {

        const ReadDataFile::Load inputFile( curveDisplayInputs );

        const DoubleArray results
            = validation::tryMeCurveDisplay(
                  inputFile["curveCollection"],
                  inputFile["curveIndex"] );

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs, tolerance );
    }

    TEST_F( TryMeTestCurveStdGBP, SNAPSHOT_CheckMeCurveDiscountFactorsFromYearFractions )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs );

        const DoubleArray yearFractions = inputFile["yearFractions"];

        DoubleArray results = validation::tryMeCurveDiscountFactorsFromYearFractions( yearFractions,
                              inputFile["dayCount"],
                              inputFile["curveCollection"],
                              inputFile["curveIndex"] );

        ASSERT_EQ( yearFractions.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckDiscountFactorsOutputs, tolerance );
    }

    TEST_F( TryMeTestCurveStdGBP, SNAPSHOT_CheckMeCurveForwardRatesFromYearFraction )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        const double yearFraction = inputFile["yearFraction"];

        const DateVector fromDateVector = inputFile["fromDates"];

        const DoubleArray results = validation::tryMeCurveForwardRatesFromYearFraction( fromDateVector,
                                    yearFraction,
                                    inputFile["dayCount"],
                                    inputFile["curveCollection"],
                                    inputFile["curveIndex"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckForwardRatesOutputs, tolerance );
    }

}
