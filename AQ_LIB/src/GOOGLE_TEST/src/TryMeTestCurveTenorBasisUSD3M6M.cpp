// TestCurveTenorBasisUSD3M6M.cpp

#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "tryMeCurveDisplay.h"
#include "tryMeCurveDiscountFactor.h"
#include "tryMeCurveForwardRate.h"

#include <gTest/gTest.h>

using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Curves/TestCurveTenorBasisUSD3M6M/"

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
    extern const char USDYC_OIS[]					= TEST_DIR "USDYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char USDYC_STD[]					= TEST_DIR "USDYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char USDYC_3M6M[]					= TEST_DIR "USDYC_3M6M_tryMeCurveCalibrateBasis_inputs";

    //
    // test call input and reference files
    //
    extern const char curveDisplayInputs[]				= TEST_DIR "USDYC_3M6M_tryMeCurveDisplay_inputs.csv";
    extern const char curveDisplayOutputs_32bit[]		= TEST_DIR "USDYC_3M6M_tryMeCurveDisplay_outputs_32bit.csv";
	extern const char curveDisplayOutputs_64bit[]		= TEST_DIR "USDYC_3M6M_tryMeCurveDisplay_outputs_64bit.csv";

    extern const char CheckDiscountFactorsInputs[]				= TEST_DIR "USDYC_3M6M_tryMeCurveDiscountFactorsFromYearFractions_inputs";
    extern const char CheckDiscountFactorsOutputs_32bit[]		= TEST_DIR "USDYC_3M6M_tryMeCurveDiscountFactorsFromYearFractions_outputs_32bit";
	extern const char CheckDiscountFactorsOutputs_64bit[]		= TEST_DIR "USDYC_3M6M_tryMeCurveDiscountFactorsFromYearFractions_outputs_64bit";

    extern const char CheckForwardRatesInputs[]				= TEST_DIR "USDYC_3M6M_tryMeCurveForwardRatesFromYearFraction_inputs";
    extern const char CheckForwardRatesOutputs_32bit[]		= TEST_DIR "USDYC_3M6M_tryMeCurveForwardRatesFromYearFraction_outputs_32bit";
	extern const char CheckForwardRatesOutputs_64bit[]		= TEST_DIR "USDYC_3M6M_tryMeCurveForwardRatesFromYearFraction_outputs_64bit";
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<TryMeCurveOis, USDYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<TryMeCurveStd, USDYC_STD> CurveStdBound;
    typedef BindFileToClassConstructor<TryMeCurveTenorBasis, USDYC_3M6M> CurveTenorBasisBound;

    //
    // Declare Dependencies
    //

    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;
    typedef Dependency1<CurveTenorBasisBound, CurveStdBuilt> CurveTenorBasisBuilt;

    //
    // SetUp Test Fixture
    //

    class TryMeTestCurveTenorBasisUSD3M6M : public testing::Test, public CurveTenorBasisBuilt
    {
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryMeTestCurveTenorBasisUSD3M6M, SNAPSHOT_CheckMeCurveDisplay )
    {
        const ReadDataFile::Load inputFile( curveDisplayInputs );
        const DoubleArray results
            = validation_api::tryMeCurveDisplay(
                  inputFile["curveCollection"],
                  inputFile["curveIndex"] );
        
		#if defined(GTEST32)
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_32bit, tolerance );
		#else
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_64bit, tolerance );
		#endif
    }


    TEST_F( TryMeTestCurveTenorBasisUSD3M6M, SNAPSHOT_CheckMeCurveDiscountFactorsFromYearFractions )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs );

        const DoubleArray yearFractions = inputFile["yearFractions"];
        DoubleArray results
            = validation_api::tryMeCurveDiscountFactorsFromYearFractions(
                  yearFractions,
                  inputFile["dayCount"],
                  inputFile["curveCollection"],
                  inputFile["curveIndex"] );

        ASSERT_EQ( yearFractions.size(), results.size() )
                << "Results size should match terms test size" << std::endl;
        
		#if defined(GTEST32)
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckDiscountFactorsOutputs_32bit, tolerance );
		#else
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckDiscountFactorsOutputs_64bit, tolerance );
		#endif
    }


    TEST_F( TryMeTestCurveTenorBasisUSD3M6M, SNAPSHOT_CheckMeCurveForwardRatesFromYearFraction )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        const double yearFraction = inputFile["yearFraction"];
        const DateVector fromDateVector = inputFile["fromDates"];

        const DoubleArray results = validation_api::tryMeCurveForwardRatesFromYearFraction( fromDateVector,
                                    yearFraction,
                                    inputFile["dayCount"],
                                    inputFile["curveCollection"],
                                    inputFile["curveIndex"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;
        
		#if defined(GTEST32)
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckForwardRatesOutputs_32bit, tolerance );
		#else
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckForwardRatesOutputs_64bit, tolerance );
		#endif
    }

}