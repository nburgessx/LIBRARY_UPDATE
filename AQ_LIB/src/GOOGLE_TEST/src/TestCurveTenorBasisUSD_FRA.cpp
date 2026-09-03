#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "tryMirGetDF.h"


using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "Vanilla/Curves/TestCurveTenorBasisUSD_FRA/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;

    // curve input file
    extern const char DIR_OIS[]			= TEST_DIR "Curve_OIS";
    extern const char DIR_STD[]			= TEST_DIR "Curve_STD";
    extern const char DIR_BASIS_1[]		= TEST_DIR "Curve_TenorBasis_1";
    extern const char DIR_BASIS_2[]		= TEST_DIR "Curve_TenorBasis_2";
    extern const char DIR_BASIS_3[]		= TEST_DIR "Curve_TenorBasis_3";
    extern const char DIR_BASIS_4[]		= TEST_DIR "Curve_TenorBasis_4";
	extern const char DIR_BASIS_5[]		= TEST_DIR "Curve_TenorBasis_5";
	extern const char DIR_BASIS_6[]		= TEST_DIR "Curve_TenorBasis_6";
	extern const char DIR_BASIS_7[]		= TEST_DIR "Curve_TenorBasis_7";

	extern const char DIR_DUMMY[]		= "";
	extern const char DIR_DUMMY2[]		= "";

    //
    // test call input and reference files
    //
    extern const char getDF_inputs[]			= TEST_DIR "getDF_inputs.csv";

    extern const char getDF_outputs_32bit_1[]	= TEST_DIR "getDF_outputs_32bit_1.csv";
    extern const char getDF_outputs_32bit_2[]	= TEST_DIR "getDF_outputs_32bit_2.csv";
	extern const char getDF_outputs_32bit_3[]	= TEST_DIR "getDF_outputs_32bit_3.csv";
	extern const char getDF_outputs_32bit_4[]	= TEST_DIR "getDF_outputs_32bit_4.csv";
	extern const char getDF_outputs_32bit_5[]	= TEST_DIR "getDF_outputs_32bit_5.csv";
	extern const char getDF_outputs_32bit_6[]	= TEST_DIR "getDF_outputs_32bit_6.csv";
	extern const char getDF_outputs_32bit_7[]	= TEST_DIR "getDF_outputs_32bit_7.csv";

	extern const char getDF_outputs_64bit_1[]	= TEST_DIR "getDF_outputs_64bit_1.csv";
    extern const char getDF_outputs_64bit_2[]	= TEST_DIR "getDF_outputs_64bit_2.csv";
	extern const char getDF_outputs_64bit_3[]	= TEST_DIR "getDF_outputs_64bit_3.csv";
	extern const char getDF_outputs_64bit_4[]	= TEST_DIR "getDF_outputs_64bit_4.csv";
	extern const char getDF_outputs_64bit_5[]	= TEST_DIR "getDF_outputs_64bit_5.csv";
	extern const char getDF_outputs_64bit_6[]	= TEST_DIR "getDF_outputs_64bit_6.csv";
	extern const char getDF_outputs_64bit_7[]	= TEST_DIR "getDF_outputs_64bit_7.csv";

}

namespace google_test
{

    // Construct depedent curves from curve source files by calling the curve constructor methods

    ADD_USD_CURVE_DEPENDENCIES( TestCurveTenorBasisUSD_FRA_1, DIR_OIS, DIR_DUMMY, DIR_STD, DIR_BASIS_1, DIR_DUMMY2);
	ADD_USD_CURVE_DEPENDENCIES( TestCurveTenorBasisUSD_FRA_2, DIR_OIS, DIR_DUMMY, DIR_STD, DIR_BASIS_2, DIR_DUMMY2);
	ADD_USD_CURVE_DEPENDENCIES( TestCurveTenorBasisUSD_FRA_3, DIR_OIS, DIR_DUMMY, DIR_STD, DIR_BASIS_3, DIR_DUMMY2);
	ADD_USD_CURVE_DEPENDENCIES( TestCurveTenorBasisUSD_FRA_4, DIR_OIS, DIR_DUMMY, DIR_STD, DIR_BASIS_4, DIR_DUMMY2);
	ADD_USD_CURVE_DEPENDENCIES( TestCurveTenorBasisUSD_FRA_5, DIR_OIS, DIR_DUMMY, DIR_STD, DIR_BASIS_5, DIR_DUMMY2);
	ADD_USD_CURVE_DEPENDENCIES( TestCurveTenorBasisUSD_FRA_6, DIR_OIS, DIR_DUMMY, DIR_STD, DIR_BASIS_6, DIR_DUMMY2);
	ADD_USD_CURVE_DEPENDENCIES( TestCurveTenorBasisUSD_FRA_7, DIR_OIS, DIR_DUMMY, DIR_STD, DIR_BASIS_7, DIR_DUMMY2);

	// Test 1
	// Tenor basis curve description: IsFRAUse is TRUE; No 6M basis swap is present; IsLeg1Spread is FALSE;
    TEST_F( TestCurveTenorBasisUSD_FRA_1, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( getDF_inputs );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

#ifdef GTEST32
        LAString outputFileName = getDF_outputs_32bit_1;
#else
        LAString outputFileName = getDF_outputs_64bit_1;
#endif
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputFileName, tolerance );
    }

	// Test 2 
	// Tenor basis curve description: IsFRAUse is TRUE; 6M basis swap is present; IsLeg1Spread is FALSE;
    TEST_F( TestCurveTenorBasisUSD_FRA_2, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( getDF_inputs );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

#ifdef GTEST32
        LAString outputFileName = getDF_outputs_32bit_2;
#else
        LAString outputFileName = getDF_outputs_64bit_2;
#endif
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputFileName, tolerance );
    }

	// Test 3
	// Tenor basis curve description: IsFRAUse is FALSE; No 6M basis swap is present; 6M Libor is provided; IsLeg1Spread is FALSE;
    TEST_F( TestCurveTenorBasisUSD_FRA_3, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( getDF_inputs );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

#ifdef GTEST32
        LAString outputFileName = getDF_outputs_32bit_3;
#else
        LAString outputFileName = getDF_outputs_64bit_3;
#endif
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputFileName, tolerance );
    }

	// Test 4
	// Tenor basis curve description: IsFRAUse is FALSE; 6M basis swap is present; 6M Libor is provided; IsLeg1Spread is FALSE;
    TEST_F( TestCurveTenorBasisUSD_FRA_4, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( getDF_inputs );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

#ifdef GTEST32
        LAString outputFileName = getDF_outputs_32bit_4;
#else
        LAString outputFileName = getDF_outputs_64bit_4;
#endif
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputFileName, tolerance );
    }

	// Test 5
	// Tenor basis curve description: IsFRAUse is TRUE; 6M basis swap is present; Only 1x7 FRA given;
    TEST_F( TestCurveTenorBasisUSD_FRA_5, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( getDF_inputs );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

#ifdef GTEST32
        LAString outputFileName = getDF_outputs_32bit_5;
#else
        LAString outputFileName = getDF_outputs_64bit_5;
#endif
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputFileName, tolerance );
    }

	// Test 6
	// Tenor basis curve description: IsFRAUse is TRUE; IsLeg1Spread is TRUE, using negative basis spreads;
    TEST_F( TestCurveTenorBasisUSD_FRA_6, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( getDF_inputs );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

#ifdef GTEST32
        LAString outputFileName = getDF_outputs_32bit_6;
#else
        LAString outputFileName = getDF_outputs_64bit_6;
#endif
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputFileName, tolerance );
    }

	// Test 7
	// Tenor basis curve description: IsFRAUse is TRUE; 6M basis swap is present; ApplyTenison = TRUE
    TEST_F( TestCurveTenorBasisUSD_FRA_7, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( getDF_inputs );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

#ifdef GTEST32
        LAString outputFileName = getDF_outputs_32bit_7;
#else
        LAString outputFileName = getDF_outputs_64bit_7;
#endif
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, outputFileName, tolerance );
    }

}
