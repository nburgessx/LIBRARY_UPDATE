// TestCurveOisUSD.cpp

#include "CurveOis.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "tryMirGetDF.h"
#include "tryMirGetForwardRate.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

using google_test::CurveOis;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using etrading::ReadDataFile;

#define TEST_DIR "Vanilla/Curves/TestCurveOisUSD/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;

    // curve input file
    extern const char CurveOisInputs[]					= TEST_DIR "USDYC_OIS_tryMirSetUpOISCurve_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "USDYC_OIS_tryMirGetDF1_inputs.csv";
    extern const char CheckDiscountFactorsReference[]	= TEST_DIR "USDYC_OIS_tryMirGetDF1_outputs.csv";

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "USDYC_USDDF_tryMirGetForwardRate2_inputs.csv";
    extern const char CheckForwardRatesReference[]		= TEST_DIR "USDYC_USDDF_tryMirGetForwardRate2_outputs.csv";
}

namespace google_test
{

    //
    // SetUp Test Fixture
    //

    typedef BindFileToClassConstructor<CurveOis, CurveOisInputs> CurveOisBound;

    typedef Dependency0<CurveOisBound> CurveOisBuilt;

    class TestCurveOisUSD : public virtual testing::Test, public CurveOisBuilt
    {
    public:
    };

    //
    // Call Test Fixture
    //

    TEST_F( TestCurveOisUSD, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  getCurveID(),
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckDiscountFactorsReference, tolerance );
    }

    TEST_F( TestCurveOisUSD, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results
            = validation::tryMirGetForwardRate2(
                  getDataInstance(),
                  fromDateVector,
                  toDateVector,
                  getCurveID(),
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"],
                  inputFile["useFwdData"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckForwardRatesReference, tolerance );

    }
}
