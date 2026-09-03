#include "TryMeCurveCTD.h"
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveFwdFxConst.h"
#include "TryMeCurveXccyBasis.h"
#include "TryMeCurveTenorBasis.h"

#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "tryMeCurveDisplay.h"

#include <gTest/gTest.h>

using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Curves/TestMeCurveCheapestToDeliver_USD_SEK/"

namespace
{
	const size_t TEST_COUNT = 2;
    
	// test tolerance
    const double tolerance = 1e-8;


    //
    // curve input files
    //    
	extern const char EUR_OIS[]			=  "_EUR_OIS";	
	extern const char EUR_6M_STD[]		=  "_EUR_6M_STD";
	extern const char EUR_3M_Basis[]	=  "_EUR_3M_Basis";
	extern const char EURDF_USDCSA[]	=  "_EURDF_USDCSA";
	extern const char EURDF_SEKCSA[]	=  "_EURDF_SEKCSA";

	extern const char USD_OIS[]			=  "_USD_OIS";
	extern const char USD_3M_STD[]		=  "_USD_3M_STD";

	extern const char SEK_3M_STD[]		=  "_SEK_3M_STD";
	extern const char SEKDF_USDCSA[]	=  "_SEKDF_USDCSA";
	
	extern const char Daily_CTD_Curve[]			= "_Daily_CTD_Curve";
	extern const char Quarterly_CTD_Curve[]		= "_Quarterly_CTD_Curve";

    //
    // test call input and reference files
    //
    extern const char curveDisplayInputs_Daily[]			= "_Daily_CTD_Display_Inputs";
    extern const char curveDisplayOutputs_Daily_32bit[]		= "_Daily_CTD_Display_Outputs_32bit";
	extern const char curveDisplayOutputs_Daily_64bit[]		= "_Daily_CTD_Display_Outputs_64bit";

	extern const char curveDisplayInputs_Quarterly[]		= "_Quarterly_CTD_Display_Inputs";
    extern const char curveDisplayOutputs_Quarterly_32bit[]	= "_Quarterly_CTD_Display_Outputs_32bit";
	extern const char curveDisplayOutputs_Quarterly_64bit[]	= "_Quarterly_CTD_Display_Outputs_64bit";
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestMeCurveCheapestToDeliver_USD_SEK);

    //
    // Call Test Fixture
    //

    TEST_F( TestMeCurveCheapestToDeliver_USD_SEK, SNAPSHOT_CheckMeCurveDisplay )
    {
		for (size_t i = 0; i < TEST_COUNT; ++i)
		{
			//----------------------------------------------------------------------------------------
			// Build yield curves of the current test case

			// Build dependent EUR, USD and SEK curves
			LAString EUR_OIS_Dir		= TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + EUR_OIS + LAString(".csv");
			setUpMeOISCurve(EUR_OIS_Dir);

			LAString EUR_6M_STD_Dir		= TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + EUR_6M_STD + LAString(".csv");
			setUpMeSTDCurve(EUR_6M_STD_Dir);

			LAString EUR_3M_Basis_Dir	= TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + EUR_3M_Basis + LAString(".csv");
			setUpMeTenorBasisCurve(EUR_3M_Basis_Dir);

			LAString USD_OIS_Dir		= TEST_DIR +  LAString("Test") + LAString(static_cast<int>(i + 1)) + USD_OIS + LAString(".csv");
			setUpMeOISCurve(USD_OIS_Dir);

			LAString USD_3M_STD_Dir		= TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + USD_3M_STD + LAString(".csv");
			setUpMeSTDCurve(USD_3M_STD_Dir);

			LAString SEK_3M_STD_Dir		= TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + SEK_3M_STD + LAString(".csv");
			setUpMeSTDCurve(SEK_3M_STD_Dir);

			LAString SEKDF_USDCSA_Dir	= TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + SEKDF_USDCSA + LAString(".csv");
			setUpMeXccyBasisCurve(SEKDF_USDCSA_Dir);

			// Build EURDF_USDCSA and EURDF_SEKCSA
			LAString EURDF_USDCSA_Dir	= TEST_DIR +  LAString("Test") + LAString(static_cast<int>(i + 1)) + EURDF_USDCSA + LAString(".csv");
			setUpMeXccyBasisCurve(EURDF_USDCSA_Dir);

			LAString EURDF_SEKCSA_Dir	= TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + EURDF_SEKCSA + LAString(".csv");
			setUpMeFwdFxConstCurveCurve(EURDF_SEKCSA_Dir);

			// Build cheapest to deliver curve that has a daily frequency
			LAString CTD_Curve_Dir		= TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + Daily_CTD_Curve + LAString(".csv");
			setUpMeCTDCurve(CTD_Curve_Dir);
									
			LAString inputDir = TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + curveDisplayInputs_Daily + LAString(".csv");
			const ReadDataFile::Load inputFile1( inputDir );
			const DoubleArray results1
				= validation::tryMeCurveDisplay(
					  inputFile1["curveCollection"],
					  inputFile1["curveIndex"] );

			#ifdef GTEST32
			LAString outputDir = TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + curveDisplayOutputs_Daily_32bit + LAString(".csv");
			#else
			LAString outputDir = TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + curveDisplayOutputs_Daily_64bit + LAString(".csv");
			#endif
			
			CheckTestResultsAndRebaseOnRequest( results1, TEST_DIR, outputDir, tolerance );

			// Build cheapest to deliver curve that has a quarterly frequency
			CTD_Curve_Dir		= TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + Quarterly_CTD_Curve + LAString(".csv");
			setUpMeCTDCurve(CTD_Curve_Dir);
									
			inputDir = TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + curveDisplayInputs_Quarterly + LAString(".csv");
			const ReadDataFile::Load inputFile2( inputDir );
			const DoubleArray results2
				= validation::tryMeCurveDisplay(
					  inputFile2["curveCollection"],
					  inputFile2["curveIndex"] );

			#ifdef GTEST32
			outputDir = TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + curveDisplayOutputs_Quarterly_32bit + LAString(".csv");
			#else
			outputDir = TEST_DIR + LAString("Test") + LAString(static_cast<int>(i + 1)) + curveDisplayOutputs_Quarterly_64bit + LAString(".csv");
			#endif
			
			CheckTestResultsAndRebaseOnRequest( results2, TEST_DIR, outputDir, tolerance );

		}
    }

}
