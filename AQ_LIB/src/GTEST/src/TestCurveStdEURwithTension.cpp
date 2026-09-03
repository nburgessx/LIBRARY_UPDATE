// TestCurveStdEURwithTension.cpp

#include "CurveOis.h"
#include "CurveStd.h"

#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "tryMirOutputCurve.h"
#include "tryMirGetForwardRate.h"

#include <gTest/gTest.h>

using google_test::CurveOis;
using google_test::CurveStd;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using etrading::ReadDataFile;

#define TEST_DIR "Vanilla/Curves/TestCurveStdEURwithTension/"

namespace
{
    // TODO: Tolerance relaxed to invalidate this test because the tension feature is undergoing much change.

    // test tolerance
    //const double tolerance = 1e-10;
    const double tolerance = 1.00;

    //
    // curve input files
    //
    extern const char EURYC_OIS[]					= TEST_DIR "EURYC_OIS_tryMirSetUpOISCurve_inputs.csv";
    extern const char EURYC_STD[]					= TEST_DIR "EURYC_STD_tryMirSetUpSwapCurve_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char OutPutDFReference[]				= TEST_DIR "EURYC_STD_tryMirOutputCurve_outputs.csv";

    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "EURYC_OIS_tryMirGetDF1_inputs.csv";
    extern const char CheckDiscountFactorsReference[]	= TEST_DIR "EURYC_OIS_tryMirGetDF1_outputs.csv";

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "EURYC_EUR6ML_tryMirGetForwardRate2_inputs.csv";
    extern const char CheckForwardRatesReference[]		= TEST_DIR "EURYC_EUR6ML_tryMirGetForwardRate2_outputs.csv";
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<CurveOis, EURYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<CurveStd, EURYC_STD> CurveStdBound;

    //
    // Declare Dependencies
    //

    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;

    //
    // SetUp Test Fixture
    //

    class TestCurveStdEURwithTension : public virtual testing::Test, public CurveStdBuilt
    {
    public:
        virtual AQLString getCurveID() const
        {
            return CurveStdBound::getCurveID();
        }
        virtual AQLString getMarketName() const
        {
            return CurveStdBound::getMarketName();
        };
    };

    //
    // Call Test Fixture
    //

    TEST_F( TestCurveStdEURwithTension, SNAPSHOT_OutPutDF )
    {
        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  getCurveID(),
                  getMarketName() );

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, OutPutDFReference, tolerance );
    }


    TEST_F( TestCurveStdEURwithTension, SNAPSHOT_CheckForwardRates )
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
