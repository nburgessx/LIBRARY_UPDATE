// TestCurveTenorBasisGBP3M6M.cpp

#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "tryAqCurveDisplay.h"
#include "tryAqCurveDiscountFactor.h"
#include "tryAqCurveForwardRate.h"

#include <gTest/gTest.h>

using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Curves/TestCurveTenorBasisGBP3M6M/"

namespace
{

    // test tolerance
    // --------------

#ifdef GTEST32
    // This Test was recorded in WIN32 so higher precision here i.e. better match against test results
    const double tolerance = 1e-10;
#else
    // The test should still pass in WIN64, but since the test was recorded using WIN32, results won't
    // match the expected hard-coded results too well
    const double tolerance = 1e-5;
#endif



    //
    // curve input files
    //
    extern const char GBPYC_OIS[]					= TEST_DIR "GBPYC_OIS_tryAqCurveCalibrateOIS_inputs";
    extern const char GBPYC_STD[]					= TEST_DIR "GBPYC_STD_tryAqCurveCalibrateSwap_inputs";
    extern const char GBPYC_3M6M[]					= TEST_DIR "GBPYC_3M6M_tryAqCurveCalibrateBasis_inputs";

    //
    // test call input and reference files
    //
    extern const char curveDisplayInputs[]				= TEST_DIR "GBPYC_3M6M_tryAqCurveDisplay_inputs.csv";
    extern const char curveDisplayOutputs_32bit[]		= TEST_DIR "GBPYC_3M6M_tryAqCurveDisplay_outputs_32bit.csv";
	extern const char curveDisplayOutputs_64bit[]		= TEST_DIR "GBPYC_3M6M_tryAqCurveDisplay_outputs_64bit.csv";

    extern const char CheckDiscountFactorsInputs[]				= TEST_DIR "GBPYC_3M6M_tryAqCurveDiscountFactorsFromYearFractions_inputs";
    extern const char CheckDiscountFactorsOutputs_32bit[]		= TEST_DIR "GBPYC_3M6M_tryAqCurveDiscountFactorsFromYearFractions_outputs_32bit";
	extern const char CheckDiscountFactorsOutputs_64bit[]		= TEST_DIR "GBPYC_3M6M_tryAqCurveDiscountFactorsFromYearFractions_outputs_64bit";

    extern const char CheckForwardRatesInputs[]				= TEST_DIR "GBPYC_3M6M_tryAqCurveForwardRatesFromYearFraction_inputs";
    extern const char CheckForwardRatesOutputs_32bit[]		= TEST_DIR "GBPYC_3M6M_tryAqCurveForwardRatesFromYearFraction_outputs_32bit";
	extern const char CheckForwardRatesOutputs_64bit[]		= TEST_DIR "GBPYC_3M6M_tryAqCurveForwardRatesFromYearFraction_outputs_64bit";
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<TryAqCurvesOis, GBPYC_OIS> CurveOisBound;
    typedef BindFileToClassConstructor<TryAqCurvesStd, GBPYC_STD> CurveStdBound;
    typedef BindFileToClassConstructor<TryAqCurvesTenorBasis, GBPYC_3M6M> CurveTenorBasisBound;

    //
    // Declare Dependencies
    //

    typedef Dependency0<CurveOisBound> CurveOisBuilt;
    typedef Dependency1<CurveStdBound, CurveOisBuilt> CurveStdBuilt;
    typedef Dependency1<CurveTenorBasisBound, CurveStdBuilt> CurveTenorBasisBuilt;

    //
    // SetUp Test Fixture
    //

    class TryAqTestCurveTenorBasisGBP3M6M : public testing::Test, public CurveTenorBasisBuilt
    {
    };

    //
    // Call Test Fixture
    //

    TEST_F( TryAqTestCurveTenorBasisGBP3M6M, SNAPSHOT_CheckMeCurveDisplay )
    {
        const ReadDataFile::Load inputFile( curveDisplayInputs );
        const DoubleArray results
            = validation::tryAqCurveDisplay(
                  inputFile["curveCollection"],
                  inputFile["curveIndex"] );
        
		#if defined(GTEST32)
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_32bit, tolerance );
		#else
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, curveDisplayOutputs_64bit, tolerance );
		#endif
    }


    TEST_F( TryAqTestCurveTenorBasisGBP3M6M, SNAPSHOT_CheckMeCurveDiscountFactorsFromYearFractions )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs );

        const DoubleArray yearFractions = inputFile["yearFractions"];
        DoubleArray results
            = validation::tryAqCurveDiscountFactorsFromYearFractions(
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


    TEST_F( TryAqTestCurveTenorBasisGBP3M6M, SNAPSHOT_CheckMeCurveForwardRatesFromYearFraction )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs );

        const double yearFraction = inputFile["yearFraction"];
        const DateVector fromDateVector = inputFile["fromDates"];

        const DoubleArray results = validation::tryAqCurveForwardRatesFromYearFraction( fromDateVector,
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