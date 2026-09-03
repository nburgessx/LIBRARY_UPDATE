#include "Dependency.h"
#include "ReadDataFile.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "tryMirGetDF.h"
#include "tryMirGetParRate.h"
#include "YieldCurveUtil.h"
#include "LADateScheduleHelpers.h"
#include "InitializeETrading.h"
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

			AQLString forecastCurveFile = AQLString("STD_") + AQLString(static_cast<int>(i + 1));
			AQLString discountCurveFile = AQLString("OIS_") + AQLString(static_cast<int>(i + 1));
						
			AQLString forecastFileDir = TEST_STD_DIR + forecastCurveFile + AQLString(".csv");
			AQLString discountFileDir = TEST_STD_DIR + discountCurveFile + AQLString(".csv");

			SET_UP_STD_CURVE(discountFileDir, forecastFileDir);
			
			const ReadDataFile::Load STDinputFile( forecastFileDir );

			// Calculate forward rates
			
			AQLString inputFileName = GetForwardRate2_inputs + AQLString("_") + AQLString(static_cast<int>(i + 1));
									
			const ReadDataFile::Load testInputFile( inputFileName );
			
			DateVector fromDateVector	= testInputFile["fromDates"];
			DateVector toDateVector		= testInputFile["toDates"];
			
			const DoubleArray results
				= validation::tryMirGetForwardRate2(
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
			AQLString outputFileName = GetForwardRate2_outputs_32bit + AQLString("_") + AQLString(static_cast<int>(i + 1));
			CheckTestResultsAndRebaseOnRequest( results, TEST_STD_DIR, outputFileName, tolerance );
			#else
			AQLString outputFileName = GetForwardRate2_outputs_64bit + AQLString("_") + AQLString(static_cast<int>(i + 1));
			CheckTestResultsAndRebaseOnRequest( results, TEST_STD_DIR, outputFileName, tolerance );
			#endif
		}
    }

	TEST_F( LinearSplineTests, SNAPSHOT_CheckBasisCurveForwardRates )
    {
		for (size_t i = 0; i < BASIS_TEST_COUNT; ++i)
		{
			// Build yield curves of the current test case

			AQLString swapCurveFile	= AQLString("STD_") + AQLString(static_cast<int>(i + 1));
			AQLString oisCurveFile	= AQLString("OIS_") + AQLString(static_cast<int>(i + 1));
			AQLString basisCurveFile = AQLString("Basis_") + AQLString(static_cast<int>(i + 1));
						
			AQLString swapFileDir  = TEST_BASIS_DIR + swapCurveFile + AQLString(".csv");
			AQLString oisFileDir	  = TEST_BASIS_DIR + oisCurveFile + AQLString(".csv");
			AQLString basisFileDir = TEST_BASIS_DIR + basisCurveFile + AQLString(".csv");

			SET_UP_TENOR_BASIS_CURVE_1(oisFileDir, swapFileDir, basisFileDir);
			
			const ReadDataFile::Load STDinputFile( basisFileDir );

			// Calculate forward rates
			
			AQLString inputFileName = BasisCurve_GetForwardRate2_inputs + AQLString("_") + AQLString(static_cast<int>(i + 1));
									
			const ReadDataFile::Load testInputFile( inputFileName );
			
			DateVector fromDateVector	= testInputFile["fromDates"];
			DateVector toDateVector		= testInputFile["toDates"];
			
			const DoubleArray results
				= validation::tryMirGetForwardRate2(
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
			AQLString outputFileName = BasisCurve_GetForwardRate2_outputs_32bit + AQLString("_") + AQLString(static_cast<int>(i + 1));
			CheckTestResultsAndRebaseOnRequest( results, TEST_BASIS_DIR, outputFileName, tolerance );
			#else
			AQLString outputFileName = BasisCurve_GetForwardRate2_outputs_64bit + AQLString("_") + AQLString(static_cast<int>(i + 1));
			CheckTestResultsAndRebaseOnRequest( results, TEST_BASIS_DIR, outputFileName, tolerance );
			#endif
		}
    }
}
