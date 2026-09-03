// TestCurveStdUSD.cpp

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

#define TEST_DIR "Vanilla/Curves/TestCurveStdUSD_NonOverridingFwds/"

namespace
{
    // test tolerance
    const double tolerance = 1e-9;

    //
    // curve input files
    //
    extern const char USDYC_OIS[]					= TEST_DIR "USDYC_OIS_tryMirSetUpOISCurve_inputs.csv";
    extern const char USDYC_STD[]					= TEST_DIR "USDYC_STD_tryMirSetUpSwapCurve_inputs.csv";

    //
    // test call input and reference files
    //
#if defined(GTEST32)
    extern const char OutPutDFReference[]				= TEST_DIR "USDYC_STD_tryMirOutputCurve_outputs_x86.csv";
#else
    extern const char OutPutDFReference[]				= TEST_DIR "USDYC_STD_tryMirOutputCurve_outputs_x64.csv";
#endif

    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "USDYC_OIS_tryMirGetDF1_inputs.csv";
#if defined(GTEST32)
    extern const char CheckDiscountFactorsReference[]	= TEST_DIR "USDYC_OIS_tryMirGetDF1_outputs_x86.csv";
#else
	extern const char CheckDiscountFactorsReference[]	= TEST_DIR "USDYC_OIS_tryMirGetDF1_outputs_x64.csv";
#endif

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "USDYC_USDDF_tryMirGetForwardRate1_inputs.csv";
#if defined(GTEST32)
    extern const char CheckForwardRatesReference[]		= TEST_DIR "USDYC_USDDF_tryMirGetForwardRate1_outputs_x86.csv";
#else
	extern const char CheckForwardRatesReference[]		= TEST_DIR "USDYC_USDDF_tryMirGetForwardRate1_outputs_x64.csv";
#endif
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<CurveOis, USDYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<CurveStd, USDYC_STD> CurveStdBound;

    //
    // Declare Dependencies
    //

    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;

    //
    // SetUp Test Fixture
    //

    class TestCurveStdUSD_NonOverridingFwds : public virtual testing::Test, public CurveStdBuilt
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

    TEST_F( TestCurveStdUSD_NonOverridingFwds, SNAPSHOT_OutPutDF )
    {
        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  getCurveID(),
                  getMarketName() );

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, OutPutDFReference, tolerance );
    }

    TEST_F( TestCurveStdUSD_NonOverridingFwds, SNAPSHOT_CheckDiscountFactors )
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

    TEST_F( TestCurveStdUSD_NonOverridingFwds, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        const double term = inputFile["term"];

        const DateVector fromDateVector = inputFile["fromDateVec"];

        const DoubleArray results
            = validation::tryMirGetForwardRate1(
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
