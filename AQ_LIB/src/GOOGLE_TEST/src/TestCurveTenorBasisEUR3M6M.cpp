// TestCurveTenorBasisEUR3M6M.cpp

#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "tryMirOutputCurve.h"
#include "tryMirGetDF.h"
#include "tryMirGetForwardRate.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

using google_test::CurveOis;
using google_test::CurveStd;
using google_test::CurveTenorBasis;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using etrading::ReadDataFile;

#define TEST_DIR "Vanilla/Curves/TestCurveTenorBasisEUR3M6M/"

namespace
{

    // test tolerance
    // --------------

#ifdef GTEST32
    // This Test was recorded in WIN32 so higher precision here i.e. better match against test results
    const double tolerance = 1e-7;
#else
    // The test should still pass in WIN64, but since the test was recorded using WIN32, results won't
    // match the expected hard-coded results too well
    const double tolerance = 1e-5;
#endif



    //
    // curve input files
    //
    extern const char EURYC_OIS[]					= TEST_DIR "EURYC_OIS_tryMirSetUpOISCurve_inputs";
    extern const char EURYC_STD[]					= TEST_DIR "EURYC_STD_tryMirSetUpSwapCurve_inputs";
    extern const char EURYC_3M6M[]					= TEST_DIR "EURYC_3M6M_tryMirSetUpBasisSwapCurve_inputs";

    //
    // test call input and reference files
    //
    extern const char OutPutDFReference_32bit[]				= TEST_DIR "EURYC_3M6M_tryMirOutputCurve_outputs_32bit";
	extern const char OutPutDFReference_64bit[]				= TEST_DIR "EURYC_3M6M_tryMirOutputCurve_outputs_64bit";

    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "EURYC_OIS_tryMirGetDF1_inputs";
    extern const char CheckDiscountFactorsReference[]	= TEST_DIR "EURYC_OIS_tryMirGetDF1_outputs";

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "EURYC_EURDF_tryMirGetForwardRate1_inputs";
    extern const char CheckForwardRatesReference[]		= TEST_DIR "EURYC_EURDF_tryMirGetForwardRate1_outputs";
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<CurveOis, EURYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<CurveStd, EURYC_STD> CurveStdBound;
    typedef BindFileToClassConstructor<CurveTenorBasis, EURYC_3M6M> CurveTenorBasisBound;

    //
    // Declare Dependencies
    //

    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;
    typedef Dependency1<CurveTenorBasisBound, CurveStdBuilt> CurveTenorBasisBuilt;

    //
    // SetUp Test Fixture
    //

    class TestCurveTenorBasisEUR3M6M : public testing::Test, public CurveTenorBasisBuilt
    {
    public:
        //
        // disambiguate accessors
        //
        virtual LAString getCurveID() const
        {
            return CurveTenorBasis::getCurveID();
        }
        virtual LAString getMarketName() const
        {
            return CurveTenorBasis::getMarketName();
        }
    };

    //
    // Call Test Fixture
    //

    TEST_F( TestCurveTenorBasisEUR3M6M, SNAPSHOT_OutPutDF )
    {
        const DoubleArray results
            = validation_api::tryMirOutputCurve(
                  getDataInstance(),
                  getCurveID(),
                  getMarketName() );

		#if defined(GTEST32)
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, OutPutDFReference_32bit, tolerance );
		#else
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, OutPutDFReference_64bit, tolerance );
		#endif
    }

    TEST_F( TestCurveTenorBasisEUR3M6M, SNAPSHOT_CheckDiscountFactors )
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

    TEST_F( TestCurveTenorBasisEUR3M6M, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        const DateVector fromDateVector = inputFile["fromDateVec"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate1(
                  getDataInstance(),
                  fromDateVector,
                  inputFile["term"],
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