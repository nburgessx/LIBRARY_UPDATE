#include "TryAqCurvesOis.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "tryAqCurvesDiscountFactor.h"
#include "tryAqCurvesForwardRate.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

using google_test::CurveOis;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Curves/TestCurveOisGBP/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;

    // curve input file
    extern const char CurveOisInputs[]					= TEST_DIR "GBPYC_OIS_tryAqCurvesCalibrateOIS_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "GBPYC_OIS_tryAqCurvesDiscountFactorsFromYearFractions_inputs.csv";
    extern const char CheckDiscountFactorsOutputs[]	= TEST_DIR "GBPYC_OIS_tryAqCurvesDiscountFactorsFromYearFractions_outputs.csv";

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "GBPYC_OIS_tryAqCurvesForwardRates_inputs.csv";
    extern const char CheckForwardRatesOutputs[]		= TEST_DIR "GBPYC_OIS_tryAqCurvesForwardRates_outputs.csv";
}

namespace google_test
{
    //
    // SetUp Test Fixture
    //

    typedef BindFileToClassConstructor<TryAqCurvesOis, CurveOisInputs> CurveOisBound;

    typedef Dependency0<CurveOisBound> CurveOisBuilt;

    class TryAqTestCurveOisGBP : public virtual testing::Test, public CurveOisBuilt
    {
    public:
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryAqTestCurveOisGBP, SNAPSHOT_CheckMeCurveDiscountFactorsFromYearFractions )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs );

        const DoubleArray yearFractions = inputFile["yearFractions"];

        DoubleArray results = validation::tryAqCurvesDiscountFactorsFromYearFractions( yearFractions,
                              inputFile["dayCount"],
                              inputFile["curveCollection"],
                              inputFile["curveIndex"] );

        ASSERT_EQ( yearFractions.size(), results.size() )
                << "Results size should match yearFractions test size" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckDiscountFactorsOutputs, tolerance );
    }

    TEST_F( TryAqTestCurveOisGBP, SNAPSHOT_CheckMeCurveForwardRatesFromForwardDates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results = validation::tryAqCurvesForwardRatesFromForwardDates( fromDateVector,
                                    toDateVector,
                                    inputFile["curveCollection"],
                                    inputFile["curveIndex"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckForwardRatesOutputs, tolerance );
    }
}

