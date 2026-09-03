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

#define TEST_DIR "ETrading/Curves/TestMeCurveGroupingJPY/"

namespace
{
	const size_t TEST_COUNT = 2;
    
	// test tolerance
    const double tolerance = 1e-8;

	// Input file names
	extern const char JPYYC_OIS[]						=  TEST_DIR "JPYYC_OIS.csv";	
	extern const char JPYYCLCH_OIS[]					=  TEST_DIR "JPYYCLCH_OIS.csv";	
	extern const char JPYYCTIBOR_OIS[]					=  TEST_DIR "JPYYCTIBOR_OIS.csv";	

	extern const char JPYYC_6M_STD[]						=  TEST_DIR "JPYYC_6M_STD.csv";	
	extern const char JPYYC_6M_STD_displayInputs[]			=  TEST_DIR "JPYYC_6M_STD_DisplayInputs.csv";	
	extern const char JPYYC_6M_STD_displayOutputs_32bit[]	=  TEST_DIR "JPYYC_6M_STD_DisplayOutputs_32bit.csv";	
	extern const char JPYYC_6M_STD_displayOutputs_64bit[]	=  TEST_DIR "JPYYC_6M_STD_DisplayOutputs_64bit.csv";	

	extern const char JPYYC_1M_Basis[]						=  TEST_DIR "JPYYC_1M_Basis.csv";	
	extern const char JPYYC_1M_Basis_displayInputs[]		=  TEST_DIR "JPYYC_1M_Basis_DisplayInputs.csv";	
	extern const char JPYYC_1M_Basis_displayOutputs_32bit[]	=  TEST_DIR "JPYYC_1M_Basis_DisplayOutputs_32bit.csv";	
	extern const char JPYYC_1M_Basis_displayOutputs_64bit[]	=  TEST_DIR "JPYYC_1M_Basis_DisplayOutputs_64bit.csv";	

	extern const char JPYYC_3M_Basis[]						=  TEST_DIR "JPYYC_3M_Basis.csv";	
	extern const char JPYYC_3M_Basis_displayInputs[]		=  TEST_DIR "JPYYC_3M_Basis_DisplayInputs.csv";	
	extern const char JPYYC_3M_Basis_displayOutputs_32bit[]	=  TEST_DIR "JPYYC_3M_Basis_DisplayOutputs_32bit.csv";	
	extern const char JPYYC_3M_Basis_displayOutputs_64bit[]	=  TEST_DIR "JPYYC_3M_Basis_DisplayOutputs_64bit.csv";	

	extern const char JPYYCLCH_6M_STD[]							=  TEST_DIR "JPYYCLCH_6M_STD.csv";	
	extern const char JPYYCLCH_6M_STD_displayInputs[]			=  TEST_DIR "JPYYCLCH_6M_STD_DisplayInputs.csv";	
	extern const char JPYYCLCH_6M_STD_displayOutputs_32bit[]	=  TEST_DIR "JPYYCLCH_6M_STD_DisplayOutputs_32bit.csv";	
	extern const char JPYYCLCH_6M_STD_displayOutputs_64bit[]	=  TEST_DIR "JPYYCLCH_6M_STD_DisplayOutputs_64bit.csv";	

	extern const char JPYYCLCH_3M_Basis[]						=  TEST_DIR "JPYYCLCH_3M_Basis.csv";	
	extern const char JPYYCLCH_3M_Basis_displayInputs[]			=  TEST_DIR "JPYYCLCH_3M_Basis_DisplayInputs.csv";	
	extern const char JPYYCLCH_3M_Basis_displayOutputs_32bit[]	=  TEST_DIR "JPYYCLCH_3M_Basis_DisplayOutputs_32bit.csv";	
	extern const char JPYYCLCH_3M_Basis_displayOutputs_64bit[]	=  TEST_DIR "JPYYCLCH_3M_Basis_DisplayOutputs_64bit.csv";	

	extern const char JPYYCLCH_1M_Basis[]						=  TEST_DIR "JPYYCLCH_1M_Basis.csv";	
	extern const char JPYYCLCH_1M_Basis_displayInputs[]			=  TEST_DIR "JPYYCLCH_1M_Basis_DisplayInputs.csv";	
	extern const char JPYYCLCH_1M_Basis_displayOutputs_32bit[]	=  TEST_DIR "JPYYCLCH_1M_Basis_DisplayOutputs_32bit.csv";	
	extern const char JPYYCLCH_1M_Basis_displayOutputs_64bit[]	=  TEST_DIR "JPYYCLCH_1M_Basis_DisplayOutputs_64bit.csv";	

	extern const char JPYYCTIBOR_6M_Basis[]							=  TEST_DIR "JPYYCTIBOR_6M_Basis.csv";	
	extern const char JPYYCTIBOR_6M_Basis_displayInputs[]			=  TEST_DIR "JPYYCTIBOR_6M_Basis_DisplayInputs.csv";	
	extern const char JPYYCTIBOR_6M_Basis_displayOutputs_32bit[]	=  TEST_DIR "JPYYCTIBOR_6M_Basis_DisplayOutputs_32bit.csv";	
	extern const char JPYYCTIBOR_6M_Basis_displayOutputs_64bit[]	=  TEST_DIR "JPYYCTIBOR_6M_Basis_DisplayOutputs_64bit.csv";	

	extern const char JPYYCTIBOR_1M_Basis[]							=  TEST_DIR "JPYYCTIBOR_1M_Basis.csv";	
	extern const char JPYYCTIBOR_1M_Basis_displayInputs[]			=  TEST_DIR "JPYYCTIBOR_1M_Basis_DisplayInputs.csv";	
	extern const char JPYYCTIBOR_1M_Basis_displayOutputs_32bit[]	=  TEST_DIR "JPYYCTIBOR_1M_Basis_DisplayOutputs_32bit.csv";	
	extern const char JPYYCTIBOR_1M_Basis_displayOutputs_64bit[]	=  TEST_DIR "JPYYCTIBOR_1M_Basis_DisplayOutputs_64bit.csv";	

	extern const char JPYYCTIBOR_3M_Basis[]							=  TEST_DIR "JPYYCTIBOR_3M_Basis.csv";	
	extern const char JPYYCTIBOR_3M_Basis_displayInputs[]			=  TEST_DIR "JPYYCTIBOR_3M_Basis_DisplayInputs.csv";	
	extern const char JPYYCTIBOR_3M_Basis_displayOutputs_32bit[]	=  TEST_DIR "JPYYCTIBOR_3M_Basis_DisplayOutputs_32bit.csv";	
	extern const char JPYYCTIBOR_3M_Basis_displayOutputs_64bit[]	=  TEST_DIR "JPYYCTIBOR_3M_Basis_DisplayOutputs_64bit.csv";	
		
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestMeCurveGroupingJPY);

    //
    // Call Test Fixture
    //

    TEST_F( TestMeCurveGroupingJPY, SNAPSHOT_CheckMeCurveDisplay )
    {
		// JPYYC OIS, JPYYCLCH_OIS and JPYYCTIBOR_OIS
		AQLString inputDir = JPYYC_OIS;
		setUpMeOISCurve(inputDir);

		inputDir = JPYYCLCH_OIS;
		setUpMeOISCurve(inputDir);

		inputDir = JPYYC_OIS;
		setUpMeOISCurve(JPYYCTIBOR_OIS);

		// JPYYC 6M
		inputDir = JPYYC_6M_STD;
		setUpMeSTDCurve(inputDir);

		inputDir = JPYYC_6M_STD_displayInputs;
		const ReadDataFile::Load inputFile0( inputDir );
		DoubleArray results
			= validation::tryMeCurveDisplay(
					inputFile0["curveCollection"],
					inputFile0["curveIndex"] );

		#ifdef GTEST32
		AQLString outputDir = JPYYC_6M_STD_displayOutputs_32bit;
		#else
		AQLString outputDir = JPYYC_6M_STD_displayOutputs_64bit;
		#endif
			
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputDir, tolerance );

		// JPYYC 1M
		inputDir = JPYYC_1M_Basis;
		setUpMeTenorBasisCurve(inputDir);

		inputDir = JPYYC_1M_Basis_displayInputs;
		const ReadDataFile::Load inputFile1( inputDir );
		results
			= validation::tryMeCurveDisplay(
					inputFile1["curveCollection"],
					inputFile1["curveIndex"] );

		#ifdef GTEST32
		outputDir = JPYYC_1M_Basis_displayOutputs_32bit;
		#else
		outputDir = JPYYC_1M_Basis_displayOutputs_64bit;
		#endif
			
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputDir, tolerance );

		// JPYYC 3M
		inputDir = JPYYC_3M_Basis;
		setUpMeTenorBasisCurve(inputDir);

		inputDir = JPYYC_3M_Basis_displayInputs;
		const ReadDataFile::Load inputFile2( inputDir );
		results
			= validation::tryMeCurveDisplay(
					inputFile2["curveCollection"],
					inputFile2["curveIndex"] );

		#ifdef GTEST32
		outputDir = JPYYC_3M_Basis_displayOutputs_32bit;
		#else
		outputDir = JPYYC_3M_Basis_displayOutputs_64bit;
		#endif
			
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputDir, tolerance );

		// JPYYC LCH 6M
		inputDir = JPYYCLCH_6M_STD;
		setUpMeSTDCurve(inputDir);

		inputDir = JPYYCLCH_6M_STD_displayInputs;
		const ReadDataFile::Load inputFile3( inputDir );
		results
			= validation::tryMeCurveDisplay(
					inputFile3["curveCollection"],
					inputFile3["curveIndex"] );

		#ifdef GTEST32
		outputDir = JPYYCLCH_6M_STD_displayOutputs_32bit;
		#else
		outputDir = JPYYCLCH_6M_STD_displayOutputs_64bit;
		#endif
			
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputDir, tolerance );

		// JPYYC LCH 3M
		inputDir = JPYYCLCH_3M_Basis;
		setUpMeTenorBasisCurve(inputDir);

		inputDir = JPYYCLCH_3M_Basis_displayInputs;
		const ReadDataFile::Load inputFile4( inputDir );
		results
			= validation::tryMeCurveDisplay(
					inputFile4["curveCollection"],
					inputFile4["curveIndex"] );

		#ifdef GTEST32
		outputDir = JPYYCLCH_3M_Basis_displayOutputs_32bit;
		#else
		outputDir = JPYYCLCH_3M_Basis_displayOutputs_64bit;
		#endif
			
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputDir, tolerance );

		// JPYYC LCH 1M
		inputDir = JPYYCLCH_1M_Basis;
		setUpMeTenorBasisCurve(inputDir);

		inputDir = JPYYCLCH_1M_Basis_displayInputs;
		const ReadDataFile::Load inputFile5( inputDir );
		results
			= validation::tryMeCurveDisplay(
					inputFile5["curveCollection"],
					inputFile5["curveIndex"] );

		#ifdef GTEST32
		outputDir = JPYYCLCH_1M_Basis_displayOutputs_32bit;
		#else
		outputDir = JPYYCLCH_1M_Basis_displayOutputs_64bit;
		#endif
			
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputDir, tolerance );

		// JPYYC TIBOR 6M
		inputDir = JPYYCTIBOR_6M_Basis;
		setUpMeTenorBasisCurve(inputDir);

		inputDir = JPYYCTIBOR_6M_Basis_displayInputs;
		const ReadDataFile::Load inputFile6( inputDir );
		results
			= validation::tryMeCurveDisplay(
					inputFile6["curveCollection"],
					inputFile6["curveIndex"] );

		#ifdef GTEST32
		outputDir = JPYYCTIBOR_6M_Basis_displayOutputs_32bit;
		#else
		outputDir = JPYYCTIBOR_6M_Basis_displayOutputs_64bit;
		#endif
			
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputDir, tolerance );

		// JPYYC TIBOR 1M
		inputDir = JPYYCTIBOR_1M_Basis;
		setUpMeTenorBasisCurve(inputDir);

		inputDir = JPYYCTIBOR_1M_Basis_displayInputs;
		const ReadDataFile::Load inputFile7( inputDir );
		results
			= validation::tryMeCurveDisplay(
					inputFile7["curveCollection"],
					inputFile7["curveIndex"] );

		#ifdef GTEST32
		outputDir = JPYYCTIBOR_1M_Basis_displayOutputs_32bit;
		#else
		outputDir = JPYYCTIBOR_1M_Basis_displayOutputs_64bit;
		#endif
			
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputDir, tolerance );

		// JPYYC TIBOR 3M
		inputDir = JPYYCTIBOR_3M_Basis;
		setUpMeTenorBasisCurve(inputDir);

		inputDir = JPYYCTIBOR_3M_Basis_displayInputs;
		const ReadDataFile::Load inputFile8( inputDir );
		results
			= validation::tryMeCurveDisplay(
					inputFile8["curveCollection"],
					inputFile8["curveIndex"] );

		#ifdef GTEST32
		outputDir = JPYYCTIBOR_3M_Basis_displayOutputs_32bit;
		#else
		outputDir = JPYYCTIBOR_3M_Basis_displayOutputs_64bit;
		#endif
			
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputDir, tolerance );
		
    }

}
