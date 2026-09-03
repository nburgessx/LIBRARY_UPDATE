// TestCurveStdEUR.cpp

#include "CurveOis.h"
#include "CurveStd.h"

#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "tryMirOutputCurve.h"
#include "tryMirGetDF.h"
#include "tryMirGetForwardRate.h"

#include <gTest/gTest.h>

using google_test::CurveOis;
using google_test::CurveStd;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using etrading::ReadDataFile;

#define TEST_DIR "Vanilla/Curves/TestCurveStdEUR/"

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
    extern const char EURYC_OIS[]					= TEST_DIR "EURYC_OIS_tryMirSetUpOISCurve_inputs.csv";
    extern const char EURYC_STD[]					= TEST_DIR "EURYC_STD_tryMirSetUpSwapCurve_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char OutPutDFReference[]				= TEST_DIR "EURYC_STD_tryMirOutputCurve_outputs.csv";

    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "EURYC_OIS_tryMirGetDF1_inputs.csv";
    extern const char CheckDiscountFactorsReference[]	= TEST_DIR "EURYC_OIS_tryMirGetDF1_outputs.csv";

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "EURYC_EURDF_tryMirGetForwardRate1_inputs.csv";
    extern const char CheckForwardRatesReference[]		= TEST_DIR "EURYC_EURDF_tryMirGetForwardRate1_outputs.csv";
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

    class TestCurveStdEUR : public virtual testing::Test, public CurveStdBuilt
    {
    public:
        virtual LAString getCurveID() const
        {
            return CurveStdBound::getCurveID();
        }
        virtual LAString getMarketName() const
        {
            return CurveStdBound::getMarketName();
        };
    };

    //
    // Call Test Fixture
    //

    TEST_F( TestCurveStdEUR, SNAPSHOT_OutPutDF )
    {
        const DoubleArray results
            = validation_api::tryMirOutputCurve(
                  getDataInstance(),
                  getCurveID(),
                  getMarketName() );

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            LAStringVector v = LAString( OutPutDFReference ).toToken( '/' );
            LAString outputFileName = v.back();
            etrading::CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( OutPutDFReference );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance );
            }
        }
    }

    TEST_F( TestCurveStdEUR, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation_api::tryMirGetDF1(
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

    TEST_F( TestCurveStdEUR, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        const double term = inputFile["term"];

        const DateVector fromDateVector = inputFile["fromDateVec"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate1(
                  getDataInstance(),
                  fromDateVector,
                  term,					// pass native argument to avoid overloading ambiguity
                  getCurveID(),
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckForwardRatesReference, tolerance );
    }

}
