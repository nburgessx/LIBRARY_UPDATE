/*
 * @brief			Regression tests on calibrating AUD OIS curves
 * @Created:		05 Jan 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "tryMeCurveDiscountFactor.h"
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"
#include "tryMeCurveDiscountFactor.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;
using google_test::CurveOis;
using google_test::CurveStd;
using google_test::BindFileToClassConstructor;

#define TEST_DIR "ETrading/Curves/TestCurveOisAUD/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;

    // curve input file
    extern const char DIR_OIS_1[]	= TEST_DIR "AUD_OIS_Inputs_1.csv";
	extern const char DIR_OIS_2[]	= TEST_DIR "AUD_OIS_Inputs_2.csv";
	extern const char DIR_DUMMY_1[]	= "";
	extern const char DIR_DUMMY_2[]	= "";
	extern const char DIR_DUMMY_3[]	= "";
	extern const char DIR_DUMMY_4[]	= "";

    //
    // test call input and reference files
    //
    extern const char CheckDF_inputs[]		= TEST_DIR "CheckDF_inputs.csv";
	
	extern const char CheckDF_outputs_1[]		= TEST_DIR "CheckDF_outputs_1.csv";
	extern const char CheckDF_outputs_2[]		= TEST_DIR "CheckDF_outputs_2.csv";

}

namespace google_test
{
    // Construct depedent curves from curve source files by calling the curve constructor methods

    ME_BUILD_EUR_CURVE( TestMeCurveOisAUD_1, DIR_OIS_1, DIR_DUMMY_1, DIR_DUMMY_2, DIR_DUMMY_3, DIR_DUMMY_4);
	ME_BUILD_EUR_CURVE( TestMeCurveOisAUD_2, DIR_OIS_2, DIR_DUMMY_1, DIR_DUMMY_2, DIR_DUMMY_3, DIR_DUMMY_4);

    
	// Test 1
	// Long term tenor is 1Y. 
	// 1Y, 2Y and 3Y use BBSW 3M vs OIS basis
	// Other longer tenors use BBSW 6M vs OIS basis
	// SmoothShortend = FALSE
    TEST_F( TestMeCurveOisAUD_1, SNAPSHOT_CheckDiscountFactors )
    {
		const ReadDataFile::Load inputFile( CheckDF_inputs );

        const DoubleArray yearFractions = inputFile["yearFractions"];

        DoubleArray results = validation_api::tryMeCurveDiscountFactorsFromYearFractions( yearFractions,
                              inputFile["dayCount"],
                              inputFile["curveCollection"],
                              inputFile["curveIndex"] );

        ASSERT_EQ( yearFractions.size(), results.size() )
                << "Results size should match yearFractions test size" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckDF_outputs_1, tolerance );
    }

	// Test 2
	// Long term tenor is 1Y. 
	// 1Y, 2Y and 3Y use BBSW 3M vs OIS basis
	// Other longer tenors use BBSW 6M vs OIS basis
	// SmoothShortend = TRUE
	TEST_F( TestMeCurveOisAUD_2, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDF_inputs );

        const DoubleArray yearFractions = inputFile["yearFractions"];

        DoubleArray results = validation_api::tryMeCurveDiscountFactorsFromYearFractions( yearFractions,
                              inputFile["dayCount"],
                              inputFile["curveCollection"],
                              inputFile["curveIndex"] );

        ASSERT_EQ( yearFractions.size(), results.size() )
                << "Results size should match yearFractions test size" << std::endl;

        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, CheckDF_outputs_2, tolerance );
    }

    
}

