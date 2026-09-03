#include "TryMeCurveOis.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "tryMeCurveDiscountFactor.h"
#include "tryMeCurveForwardRate.h"
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
    extern const char CurveOisInputs[]					= TEST_DIR "GBPYC_OIS_tryMeCurveCalibrateOIS_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "GBPYC_OIS_tryMeCurveDiscountFactorsFromYearFractions_inputs.csv";
    extern const char CheckDiscountFactorsOutputs[]	= TEST_DIR "GBPYC_OIS_tryMeCurveDiscountFactorsFromYearFractions_outputs.csv";

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "GBPYC_OIS_tryMeCurveForwardRates_inputs.csv";
    extern const char CheckForwardRatesOutputs[]		= TEST_DIR "GBPYC_OIS_tryMeCurveForwardRates_outputs.csv";
}

namespace google_test
{
    //
    // SetUp Test Fixture
    //

    typedef BindFileToClassConstructor<TryMeCurveOis, CurveOisInputs> CurveOisBound;

    typedef Dependency0<CurveOisBound> CurveOisBuilt;

    class TryMeTestCurveOisGBP : public virtual testing::Test, public CurveOisBuilt
    {
    public:
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryMeTestCurveOisGBP, SNAPSHOT_CheckMeCurveDiscountFactorsFromYearFractions )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs );

        const DoubleArray yearFractions = inputFile["yearFractions"];

        DoubleArray results = validation::tryMeCurveDiscountFactorsFromYearFractions( yearFractions,
                              inputFile["dayCount"],
                              inputFile["curveCollection"],
                              inputFile["curveIndex"] );

        ASSERT_EQ( yearFractions.size(), results.size() )
                << "Results size should match yearFractions test size" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckDiscountFactorsOutputs, tolerance );
    }

    TEST_F( TryMeTestCurveOisGBP, SNAPSHOT_CheckMeCurveForwardRatesFromForwardDates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results = validation::tryMeCurveForwardRatesFromForwardDates( fromDateVector,
                                    toDateVector,
                                    inputFile["curveCollection"],
                                    inputFile["curveIndex"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckForwardRatesOutputs, tolerance );
    }
}

