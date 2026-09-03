#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "tryMirGetDF.h"
#include "tryMirGetForwardRate.h"


//#include <gTest/gTest.h>

//using google_test::CurveOis;
//using google_test::BindFileToClassConstructor;
//using google_test::Dependency0;
using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "Vanilla/Curves/TestCurveOisEUR_SmoothShortEnd/"

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
    extern const char DIR_EURYC_OIS_8[]			= TEST_DIR "tryMirSetUpOISCurve_inputs_OIS8";

    extern const char DIR_EURYC_1M[]			= "";
    extern const char DIR_EURYC_3M[]			= "";
    extern const char DIR_EURYC_6M[]			= "";
    extern const char DIR_EURYC_12M[]			= "";

    //
    // Test files for DFs
    //
    extern const char CheckDiscountFactorsInputs_1[]		= TEST_DIR "tryMirGetDF1_inputs_OIS1.csv";
    extern const char CheckDiscountFactorsInputs_2[]		= TEST_DIR "tryMirGetDF1_inputs_OIS2.csv";
    extern const char CheckDiscountFactorsInputs_3[]		= TEST_DIR "tryMirGetDF1_inputs_OIS3.csv";
    extern const char CheckDiscountFactorsInputs_4[]		= TEST_DIR "tryMirGetDF1_inputs_OIS4.csv";
    extern const char CheckDiscountFactorsInputs_5[]		= TEST_DIR "tryMirGetDF1_inputs_OIS5.csv";
    extern const char CheckDiscountFactorsInputs_6[]		= TEST_DIR "tryMirGetDF1_inputs_OIS6.csv";
    extern const char CheckDiscountFactorsInputs_7[]		= TEST_DIR "tryMirGetDF1_inputs_OIS7.csv";
    extern const char CheckDiscountFactorsInputs_8[]		= TEST_DIR "tryMirGetDF1_inputs_OIS8.csv";

    extern const char CheckDiscountFactorsReference_1[]		= TEST_DIR "tryMirGetDF1_outputs_OIS1.csv";
    extern const char CheckDiscountFactorsReference_2[]		= TEST_DIR "tryMirGetDF1_outputs_OIS2.csv";
    extern const char CheckDiscountFactorsReference_3[]		= TEST_DIR "tryMirGetDF1_outputs_OIS3.csv";
    extern const char CheckDiscountFactorsReference_4[]		= TEST_DIR "tryMirGetDF1_outputs_OIS4.csv";
    extern const char CheckDiscountFactorsReference_5[]		= TEST_DIR "tryMirGetDF1_outputs_OIS5.csv";
    extern const char CheckDiscountFactorsReference_6[]		= TEST_DIR "tryMirGetDF1_outputs_OIS6.csv";
    extern const char CheckDiscountFactorsReference_7[]		= TEST_DIR "tryMirGetDF1_outputs_OIS7.csv";
    extern const char CheckDiscountFactorsReference_8[]		= TEST_DIR "tryMirGetDF1_outputs_OIS8.csv";

    extern const char DF_Output_FileNameOnly_1[]		= "tryMirGetDF1_outputs_OIS1.csv";
    extern const char DF_Output_FileNameOnly_2[]		= "tryMirGetDF1_outputs_OIS2.csv";
    extern const char DF_Output_FileNameOnly_3[]		= "tryMirGetDF1_outputs_OIS3.csv";
    extern const char DF_Output_FileNameOnly_4[]		= "tryMirGetDF1_outputs_OIS4.csv";
    extern const char DF_Output_FileNameOnly_5[]		= "tryMirGetDF1_outputs_OIS5.csv";
    extern const char DF_Output_FileNameOnly_6[]		= "tryMirGetDF1_outputs_OIS6.csv";
    extern const char DF_Output_FileNameOnly_7[]		= "tryMirGetDF1_outputs_OIS7.csv";
    extern const char DF_Output_FileNameOnly_8[]		= "tryMirGetDF1_outputs_OIS8.csv";

    //
    // Test files for forward rates
    //
    extern const char CheckForwardRatesInputs_1[]			= TEST_DIR "tryMirGetForwardRate2_inputs_OIS1.csv";
    extern const char CheckForwardRatesInputs_2[]			= TEST_DIR "tryMirGetForwardRate2_inputs_OIS2.csv";
    extern const char CheckForwardRatesInputs_3[]			= TEST_DIR "tryMirGetForwardRate2_inputs_OIS3.csv";
    extern const char CheckForwardRatesInputs_4[]			= TEST_DIR "tryMirGetForwardRate2_inputs_OIS4.csv";
    extern const char CheckForwardRatesInputs_5[]			= TEST_DIR "tryMirGetForwardRate2_inputs_OIS5.csv";
    extern const char CheckForwardRatesInputs_6[]			= TEST_DIR "tryMirGetForwardRate2_inputs_OIS6.csv";
    extern const char CheckForwardRatesInputs_7[]			= TEST_DIR "tryMirGetForwardRate2_inputs_OIS7.csv";
    extern const char CheckForwardRatesInputs_8[]			= TEST_DIR "tryMirGetForwardRate2_inputs_OIS8.csv";

    extern const char CheckForwardRatesReference_1[]		= TEST_DIR "tryMirGetForwardRate2_outputs_OIS1.csv";
    extern const char CheckForwardRatesReference_2[]		= TEST_DIR "tryMirGetForwardRate2_outputs_OIS2.csv";
    extern const char CheckForwardRatesReference_3[]		= TEST_DIR "tryMirGetForwardRate2_outputs_OIS3.csv";
    extern const char CheckForwardRatesReference_4[]		= TEST_DIR "tryMirGetForwardRate2_outputs_OIS4.csv";
    extern const char CheckForwardRatesReference_5[]		= TEST_DIR "tryMirGetForwardRate2_outputs_OIS5.csv";
    extern const char CheckForwardRatesReference_6[]		= TEST_DIR "tryMirGetForwardRate2_outputs_OIS6.csv";
    extern const char CheckForwardRatesReference_7[]		= TEST_DIR "tryMirGetForwardRate2_outputs_OIS7.csv";
    extern const char CheckForwardRatesReference_8[]		= TEST_DIR "tryMirGetForwardRate2_outputs_OIS8.csv";

    extern const char FwdRates_Output_FileNameOnly_1[]		= "tryMirGetForwardRate2_outputs_OIS1.csv";
    extern const char FwdRates_Output_FileNameOnly_2[]		= "tryMirGetForwardRate2_outputs_OIS2.csv";
    extern const char FwdRates_Output_FileNameOnly_3[]		= "tryMirGetForwardRate2_outputs_OIS3.csv";
    extern const char FwdRates_Output_FileNameOnly_4[]		= "tryMirGetForwardRate2_outputs_OIS4.csv";
    extern const char FwdRates_Output_FileNameOnly_5[]		= "tryMirGetForwardRate2_outputs_OIS5.csv";
    extern const char FwdRates_Output_FileNameOnly_6[]		= "tryMirGetForwardRate2_outputs_OIS6.csv";
    extern const char FwdRates_Output_FileNameOnly_7[]		= "tryMirGetForwardRate2_outputs_OIS7.csv";
    extern const char FwdRates_Output_FileNameOnly_8[]		= "tryMirGetForwardRate2_outputs_OIS8.csv";
}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_SmoothShortEnd_1, DIR_EURYC_OIS_1, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
    ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_SmoothShortEnd_2, DIR_EURYC_OIS_2, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
    ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_SmoothShortEnd_3, DIR_EURYC_OIS_3, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
    ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_SmoothShortEnd_4, DIR_EURYC_OIS_4, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
    ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_SmoothShortEnd_5, DIR_EURYC_OIS_5, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
    ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_SmoothShortEnd_6, DIR_EURYC_OIS_6, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
    ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_SmoothShortEnd_7, DIR_EURYC_OIS_7, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );
    ADD_EUR_CURVE_DEPENDENCIES( TestCurveOisEUR_SmoothShortEnd_8, DIR_EURYC_OIS_8, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );

    // --------------------------------------------------------------------------------------
    //
    // Use discount factors to check integrity of yield curves
    //

    TEST_F( TestCurveOisEUR_SmoothShortEnd_1, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs_1 );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation_api::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( DF_Output_FileNameOnly_1 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckDiscountFactorsReference_1 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance )
                        << " Discount Factor for Tenor " << term.at( i ) << " is incorrect ";
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_2, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs_2 );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation_api::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( DF_Output_FileNameOnly_2 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckDiscountFactorsReference_2 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance )
                        << " Discount Factor for Tenor " << term.at( i ) << " is incorrect ";
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_3, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs_3 );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation_api::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( DF_Output_FileNameOnly_3 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckDiscountFactorsReference_3 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance )
                        << " Discount Factor for Tenor " << term.at( i ) << " is incorrect ";
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_4, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs_4 );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation_api::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( DF_Output_FileNameOnly_4 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckDiscountFactorsReference_4 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance )
                        << " Discount Factor for Tenor " << term.at( i ) << " is incorrect ";
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_5, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs_5 );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation_api::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( DF_Output_FileNameOnly_5 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckDiscountFactorsReference_5 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance )
                        << " Discount Factor for Tenor " << term.at( i ) << " is incorrect ";
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_6, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs_6 );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation_api::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( DF_Output_FileNameOnly_6 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckDiscountFactorsReference_6 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance )
                        << " Discount Factor for Tenor " << term.at( i ) << " is incorrect ";
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_7, SNAPSHOT_CheckDiscountFactors )
    {
        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs_7 );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation_api::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( DF_Output_FileNameOnly_7 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckDiscountFactorsReference_7 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance )
                        << " Discount Factor for Tenor " << term.at( i ) << " is incorrect ";
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_8, SNAPSHOT_CheckDiscountFactors )
    {
        // OIS Curve NO.8 have been added in to test a specific bug reported by business where
        // the OIS swap date schedule has been in error under special conditions

        const ReadDataFile::Load inputFile( CheckDiscountFactorsInputs_8 );

        const DoubleArray term = inputFile["terms"];

        DoubleArray results
            = validation_api::tryMirGetDF1(
                  getDataInstance(),
                  term,
                  inputFile["curveID"],
                  inputFile["dayCount"],
                  inputFile["interpolation"],
                  inputFile["isBasisFlag"],
                  inputFile["curveName"] );

        ASSERT_EQ( term.size(), results.size() )
                << "Results size should match terms test size" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( DF_Output_FileNameOnly_8 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckDiscountFactorsReference_8 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance )
                        << " Discount Factor for Tenor " << term.at( i ) << " is incorrect ";
            }
        }
    }

    // --------------------------------------------------------------------------------------
    //
    // Use forward rates to check integrity of yield curves
    //

    TEST_F( TestCurveOisEUR_SmoothShortEnd_1, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs_1 );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate2(
                  getDataInstance(),
                  fromDateVector,
                  toDateVector,
                  inputFile["curveId"],
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"],
                  inputFile["useFwdData"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( FwdRates_Output_FileNameOnly_1 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckForwardRatesReference_1 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance );
            }
        }

    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_2, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs_2 );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate2(
                  getDataInstance(),
                  fromDateVector,
                  toDateVector,
                  inputFile["curveId"],
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"],
                  inputFile["useFwdData"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( FwdRates_Output_FileNameOnly_2 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckForwardRatesReference_2 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance );
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_3, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs_3 );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate2(
                  getDataInstance(),
                  fromDateVector,
                  toDateVector,
                  inputFile["curveId"],
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"],
                  inputFile["useFwdData"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( FwdRates_Output_FileNameOnly_3 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckForwardRatesReference_3 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance );
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_4, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs_4 );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate2(
                  getDataInstance(),
                  fromDateVector,
                  toDateVector,
                  inputFile["curveId"],
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"],
                  inputFile["useFwdData"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( FwdRates_Output_FileNameOnly_4 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckForwardRatesReference_4 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance );
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_5, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs_5 );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate2(
                  getDataInstance(),
                  fromDateVector,
                  toDateVector,
                  inputFile["curveId"],
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"],
                  inputFile["useFwdData"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( FwdRates_Output_FileNameOnly_5 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckForwardRatesReference_5 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance );
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_6, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs_6 );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate2(
                  getDataInstance(),
                  fromDateVector,
                  toDateVector,
                  inputFile["curveId"],
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"],
                  inputFile["useFwdData"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( FwdRates_Output_FileNameOnly_6 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckForwardRatesReference_6 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance );
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_7, SNAPSHOT_CheckForwardRates )
    {
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs_7 );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate2(
                  getDataInstance(),
                  fromDateVector,
                  toDateVector,
                  inputFile["curveId"],
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"],
                  inputFile["useFwdData"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( FwdRates_Output_FileNameOnly_7 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckForwardRatesReference_7 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance );
            }
        }
    }

    TEST_F( TestCurveOisEUR_SmoothShortEnd_8, SNAPSHOT_CheckForwardRates )
    {
        // OIS Curve NO.8 have been added in to test a specific bug reported by business where
        // the OIS swap date schedule has been in error under special conditions
        const ReadDataFile::Load inputFile( CheckForwardRatesInputs_8 );

        DateVector fromDateVector = inputFile["fromDates"];
        DateVector toDateVector = inputFile["toDates"];

        const DoubleArray results
            = validation_api::tryMirGetForwardRate2(
                  getDataInstance(),
                  fromDateVector,
                  toDateVector,
                  inputFile["curveId"],
                  inputFile["frequency"],
                  inputFile["dayCount"],
                  inputFile["slidingRule"],
                  inputFile["calendar"],
                  inputFile["interpolation"],
                  inputFile["curveName"],
                  inputFile["isFwdInterp"],
                  inputFile["useFwdData"] );

        ASSERT_EQ( fromDateVector.size(), results.size() )
                << "Results size should match the number of forward rates requested" << std::endl;

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( FwdRates_Output_FileNameOnly_8 ) );
            file.write( "output", results );
        }
        else
        {
            const ReadDataFile::Load resultFile( CheckForwardRatesReference_8 );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance );
            }
        }
    }
}
