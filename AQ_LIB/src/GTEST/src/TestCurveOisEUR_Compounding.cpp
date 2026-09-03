#include "CurveOis.h"
#include "CurveStd.h"

#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "tryMirOutputCurve.h"

#include <gTest/gTest.h>

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "Vanilla/Curves/TestCurveOisEUR_Compounding/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;

    // curve input file
    extern const char DIR_EURYC_OIS_1[]			= TEST_DIR "tryMirSetUpOISCurve_inputs_OIS1";
    extern const char DIR_EURYC_OIS_2[]			= TEST_DIR "tryMirSetUpOISCurve_inputs_OIS2";
    extern const char DIR_EURYC_OIS_3[]			= TEST_DIR "tryMirSetUpOISCurve_inputs_OIS3";
    extern const char DIR_EURYC_OIS_4[]			= TEST_DIR "tryMirSetUpOISCurve_inputs_OIS4";
    extern const char DIR_EURYC_OIS_5[]			= TEST_DIR "tryMirSetUpOISCurve_inputs_OIS5";
    extern const char DIR_EURYC_OIS_6[]			= TEST_DIR "tryMirSetUpOISCurve_inputs_OIS6";
	extern const char DIR_EURYC_OIS_7[]			= TEST_DIR "tryMirSetUpOISCurve_inputs_OIS7";

    extern const char DIR_EURYC_1M[]			= "";
    extern const char DIR_EURYC_3M[]			= "";
    extern const char DIR_EURYC_6M[]			= "";
    extern const char DIR_EURYC_12M[]			= "";

    //
    // Test files for internal DFs of curves
    //
    extern const char CheckCurveOutputs_Outputs_1[]		= TEST_DIR "CheckCurveOutputs_Outputs_1.csv";
	extern const char CheckCurveOutputs_Outputs_2[]		= TEST_DIR "CheckCurveOutputs_Outputs_2.csv";
	extern const char CheckCurveOutputs_Outputs_3[]		= TEST_DIR "CheckCurveOutputs_Outputs_3.csv";
	extern const char CheckCurveOutputs_Outputs_4[]		= TEST_DIR "CheckCurveOutputs_Outputs_4.csv";
	extern const char CheckCurveOutputs_Outputs_5[]		= TEST_DIR "CheckCurveOutputs_Outputs_5.csv";
	extern const char CheckCurveOutputs_Outputs_6[]		= TEST_DIR "CheckCurveOutputs_Outputs_6.csv";
	extern const char CheckCurveOutputs_Outputs_7[]		= TEST_DIR "CheckCurveOutputs_Outputs_7.csv";
	
}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_Compounding_1, DIR_EURYC_OIS_1, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
	ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_Compounding_2, DIR_EURYC_OIS_2, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
	ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_Compounding_3, DIR_EURYC_OIS_3, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
	ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_Compounding_4, DIR_EURYC_OIS_4, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
	ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_Compounding_5, DIR_EURYC_OIS_5, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
	ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_Compounding_6, DIR_EURYC_OIS_6, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
	ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_Compounding_7, DIR_EURYC_OIS_7, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
	

    
    // --------------------------------------------------------------------------------------
    //
    // Use discount factors to check integrity of yield curves
    //

	// 1
    TEST_F( TestCurveOisEUR_Compounding_1, SNAPSHOT_CheckCurveOutputs )
    {
		const ReadDataFile::Load inputFile( DIR_EURYC_OIS_1 );
		AQLString curveID = inputFile["curveID"];
		AQLString market = inputFile["marketName"];

        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  curveID,
                  market );

        CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, CheckCurveOutputs_Outputs_1, tolerance);
	}

	// 2
	TEST_F( TestCurveOisEUR_Compounding_2, SNAPSHOT_CheckCurveOutputs )
    {
		const ReadDataFile::Load inputFile( DIR_EURYC_OIS_2 );
		AQLString curveID = inputFile["curveID"];
		AQLString market = inputFile["marketName"];

        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  curveID,
                  market );

        CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, CheckCurveOutputs_Outputs_2, tolerance);
	}

	// 3
	TEST_F( TestCurveOisEUR_Compounding_3, SNAPSHOT_CheckCurveOutputs )
    {
		const ReadDataFile::Load inputFile( DIR_EURYC_OIS_3 );
		AQLString curveID = inputFile["curveID"];
		AQLString market = inputFile["marketName"];

        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  curveID,
                  market );

        CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, CheckCurveOutputs_Outputs_3, tolerance);
	}

	// 4
	TEST_F( TestCurveOisEUR_Compounding_4, SNAPSHOT_CheckCurveOutputs )
    {
		const ReadDataFile::Load inputFile( DIR_EURYC_OIS_4 );
		AQLString curveID = inputFile["curveID"];
		AQLString market = inputFile["marketName"];

        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  curveID,
                  market );

        CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, CheckCurveOutputs_Outputs_4, tolerance);
	}

	// 5
	TEST_F( TestCurveOisEUR_Compounding_5, SNAPSHOT_CheckCurveOutputs )
    {
		const ReadDataFile::Load inputFile( DIR_EURYC_OIS_5 );
		AQLString curveID = inputFile["curveID"];
		AQLString market = inputFile["marketName"];

        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  curveID,
                  market );

        CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, CheckCurveOutputs_Outputs_5, tolerance);
	}

	// 6
	TEST_F( TestCurveOisEUR_Compounding_6, SNAPSHOT_CheckCurveOutputs )
    {
		const ReadDataFile::Load inputFile( DIR_EURYC_OIS_6 );
		AQLString curveID = inputFile["curveID"];
		AQLString market = inputFile["marketName"];

        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  curveID,
                  market );

        CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, CheckCurveOutputs_Outputs_6, tolerance);
	}

	// 7
	TEST_F( TestCurveOisEUR_Compounding_7, SNAPSHOT_CheckCurveOutputs )
    {
		const ReadDataFile::Load inputFile( DIR_EURYC_OIS_7 );
		AQLString curveID = inputFile["curveID"];
		AQLString market = inputFile["marketName"];

        const DoubleArray results
            = validation::tryMirOutputCurve(
                  getDataInstance(),
                  curveID,
                  market );

        CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, CheckCurveOutputs_Outputs_7, tolerance);
	}

}
