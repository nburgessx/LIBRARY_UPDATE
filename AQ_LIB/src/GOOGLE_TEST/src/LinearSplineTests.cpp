/*
 * @brief			Tests that examine the performance of linear spline interpolation
 * @Created:		17 May 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "Dependency.h"
#include "ReadDataFile.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "tryMirGetDF.h"
#include "tryMirGetParRate.h"
#include "YieldCurveUtil.h"
#include "LADateScheduleHelpers.h"
#include "InitializeMLibETrading.h"
#include "tryMeUtilitySetup.h"
#include "tryMirGetForwardRate.h"
#include "ResultsProcessor.h"
#include <sstream>

using etrading::ReadDataFile;

#define TEST_STD_DIR "Vanilla/Interpolations/LinearSplineTests/STD/"
#define TEST_BASIS_DIR "Vanilla/Interpolations/LinearSplineTests/Basis/"


namespace
{
	size_t STD_TEST_COUNT = 17;
	size_t BASIS_TEST_COUNT = 8;

    // test tolerance
    const double tolerance = 1e-8;

    //
    // test call input and reference files
    //

    extern const char GetForwardRate2_inputs[]				= TEST_STD_DIR "MirGetForwardRate2_inputs";
    extern const char GetForwardRate2_outputs_32bit[]		= TEST_STD_DIR "MirGetForwardRate2_outputs_32bit";
	extern const char GetForwardRate2_outputs_64bit[]		= TEST_STD_DIR "MirGetForwardRate2_outputs_64bit";

	extern const char BasisCurve_GetForwardRate2_inputs[]			= TEST_BASIS_DIR "MirGetForwardRate2_inputs";
    extern const char BasisCurve_GetForwardRate2_outputs_32bit[]	= TEST_BASIS_DIR "MirGetForwardRate2_outputs_32bit";
	extern const char BasisCurve_GetForwardRate2_outputs_64bit[]	= TEST_BASIS_DIR "MirGetForwardRate2_outputs_64bit";
}

namespace google_test
{
    DECLARE_TEST_FIXTURE(LinearSplineTests);

    TEST_F( LinearSplineTests, SNAPSHOT_CheckSwapCurveForwardRates )
    {
		for (size_t i = 0; i < STD_TEST_COUNT; ++i)
		{
			// Build yield curves of the current test case

			LAString forecastCurveFile = LAString("STD_") + LAString(static_cast<int>(i + 1));
			LAString discountCurveFile = LAString("OIS_") + LAString(static_cast<int>(i + 1));
						
			LAString forecastFileDir = TEST_STD_DIR + forecastCurveFile + LAString(".csv");
			LAString discountFileDir = TEST_STD_DIR + discountCurveFile + LAString(".csv");

			SET_UP_STD_CURVE(discountFileDir, forecastFileDir);
			
			const ReadDataFile::Load STDinputFile( forecastFileDir );

			// Calculate forward rates
			
			LAString inputFileName = GetForwardRate2_inputs + LAString("_") + LAString(static_cast<int>(i + 1));
									
			const ReadDataFile::Load testInputFile( inputFileName );
			
			DateVector fromDateVector	= testInputFile["fromDates"];
			DateVector toDateVector		= testInputFile["toDates"];
			
			const DoubleArray results
				= validation_api::tryMirGetForwardRate2(
					  getDataInstance(),
					  fromDateVector,
					  toDateVector,
					  etrading::getCurveID( STDinputFile ),
					  testInputFile["frequency"],
					  testInputFile["dayCount"],
					  testInputFile["slidingRule"],
					  testInputFile["calendar"],
					  testInputFile["interpolation"],
					  testInputFile["curveName"],
					  testInputFile["isFwdInterp"],
					  testInputFile["useFwdData"] );

			// Validate results
			ASSERT_EQ( fromDateVector.size(), results.size() )
					<< "Results size should match the number of forward rates requested" << std::endl;

			#if defined(GTEST32)
			LAString outputFileName = GetForwardRate2_outputs_32bit + LAString("_") + LAString(static_cast<int>(i + 1));
			CheckTestResultsAndRebaseOnRequest( results, TEST_STD_DIR, outputFileName, tolerance );
			#else
			LAString outputFileName = GetForwardRate2_outputs_64bit + LAString("_") + LAString(static_cast<int>(i + 1));
			CheckTestResultsAndRebaseOnRequest( results, TEST_STD_DIR, outputFileName, tolerance );
			#endif
		}
    }

	TEST_F( LinearSplineTests, SNAPSHOT_CheckBasisCurveForwardRates )
    {
		for (size_t i = 0; i < BASIS_TEST_COUNT; ++i)
		{
			// Build yield curves of the current test case

			LAString swapCurveFile	= LAString("STD_") + LAString(static_cast<int>(i + 1));
			LAString oisCurveFile	= LAString("OIS_") + LAString(static_cast<int>(i + 1));
			LAString basisCurveFile = LAString("Basis_") + LAString(static_cast<int>(i + 1));
						
			LAString swapFileDir  = TEST_BASIS_DIR + swapCurveFile + LAString(".csv");
			LAString oisFileDir	  = TEST_BASIS_DIR + oisCurveFile + LAString(".csv");
			LAString basisFileDir = TEST_BASIS_DIR + basisCurveFile + LAString(".csv");

			SET_UP_TENOR_BASIS_CURVE_1(oisFileDir, swapFileDir, basisFileDir);
			
			const ReadDataFile::Load STDinputFile( basisFileDir );

			// Calculate forward rates
			
			LAString inputFileName = BasisCurve_GetForwardRate2_inputs + LAString("_") + LAString(static_cast<int>(i + 1));
									
			const ReadDataFile::Load testInputFile( inputFileName );
			
			DateVector fromDateVector	= testInputFile["fromDates"];
			DateVector toDateVector		= testInputFile["toDates"];
			
			const DoubleArray results
				= validation_api::tryMirGetForwardRate2(
					  getDataInstance(),
					  fromDateVector,
					  toDateVector,
					  etrading::getCurveID( STDinputFile ),
					  testInputFile["frequency"],
					  testInputFile["dayCount"],
					  testInputFile["slidingRule"],
					  testInputFile["calendar"],
					  testInputFile["interpolation"],
					  testInputFile["curveName"],
					  testInputFile["isFwdInterp"],
					  testInputFile["useFwdData"] );

			// Validate results
			ASSERT_EQ( fromDateVector.size(), results.size() )
					<< "Results size should match the number of forward rates requested" << std::endl;

			#if defined(GTEST32)
			LAString outputFileName = BasisCurve_GetForwardRate2_outputs_32bit + LAString("_") + LAString(static_cast<int>(i + 1));
			CheckTestResultsAndRebaseOnRequest( results, TEST_BASIS_DIR, outputFileName, tolerance );
			#else
			LAString outputFileName = BasisCurve_GetForwardRate2_outputs_64bit + LAString("_") + LAString(static_cast<int>(i + 1));
			CheckTestResultsAndRebaseOnRequest( results, TEST_BASIS_DIR, outputFileName, tolerance );
			#endif
		}
    }
}
