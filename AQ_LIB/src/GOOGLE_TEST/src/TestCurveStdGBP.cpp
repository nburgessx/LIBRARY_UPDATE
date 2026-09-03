// TestCurveStdGBP.cpp

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

#define TEST_DIR "Vanilla/Curves/TestCurveStdGBP/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;

    //
    // curve input files
    //
    extern const char GBPYC_OIS[]					= TEST_DIR "GBPYC_OIS_tryMirSetUpOISCurve_inputs.csv";
    extern const char GBPYC_STD[]					= TEST_DIR "GBPYC_STD_tryMirSetUpSwapCurve_inputs.csv";

    //
    // test call input and reference files
    //
    extern const char OutPutDFReference_32bit[]				= TEST_DIR "GBPYC_STD_tryMirOutputCurve_outputs_32bit.csv";
	extern const char OutPutDFReference_64bit[]				= TEST_DIR "GBPYC_STD_tryMirOutputCurve_outputs_64bit.csv";

    extern const char CheckDiscountFactorsInputs[]		= TEST_DIR "GBPYC_OIS_tryMirGetDF1_inputs.csv";
    extern const char CheckDiscountFactorsReference[]	= TEST_DIR "GBPYC_OIS_tryMirGetDF1_outputs.csv";

    extern const char CheckForwardRatesInputs[]			= TEST_DIR "GBPYC_GBPDF_tryMirGetForwardRate1_inputs.csv";
    extern const char CheckForwardRatesReference[]		= TEST_DIR "GBPYC_GBPDF_tryMirGetForwardRate1_outputs.csv";
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<CurveOis, GBPYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<CurveStd, GBPYC_STD> CurveStdBound;

    //
    // Declare Dependencies
    //

    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;

    //
    // SetUp Test Fixture
    //

    class TestCurveStdGBP : public virtual testing::Test, public CurveStdBuilt
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

    TEST_F( TestCurveStdGBP, SNAPSHOT_OutPutDF )
    {
        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  getCurveID(),
                  getMarketName() );

		#if defined(GTEST32)
			CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, OutPutDFReference_32bit, tolerance );
		#else
			CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, OutPutDFReference_64bit, tolerance );
		#endif
    }

    TEST_F( TestCurveStdGBP, SNAPSHOT_CheckDiscountFactors )
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

    TEST_F( TestCurveStdGBP, SNAPSHOT_CheckForwardRates )
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
