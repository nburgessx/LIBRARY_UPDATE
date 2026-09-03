/*
 * @brief			Regression tests on the swap delta risks tenor basis curve
 * @Created:		24 June 2016
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
#include "CurveXccyBasis.h"
#include "tryMeProductSwapDelta.h"
#include "BindFileToClassConstructor.h"

#include <gTest/gTest.h>

using google_test::CurveOis;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "Vanilla/Risks/TestRiskTenorBasisCurve/"

namespace
{
    // test tolerance
    const double tolerance = 100;  // Notional of test trades is 100MM and delta through numerical bumping is inherently noisy

    // curve input file
    extern const char TEST1_OIS[]				= TEST_DIR "Test1_Curve_OIS_inputs";
    extern const char TEST1_STD[]				= TEST_DIR "Test1_Curve_STD_inputs";
    extern const char TEST1_3M6MBASIS_SPOT[]	= TEST_DIR "Test1_Curve_3M6MBasisSpot_inputs";
    extern const char TEST1_3M6MBASIS_FWD[]		= TEST_DIR "Test1_Curve_3M6M_inputs";

    extern const char TEST2_OIS[]				= TEST_DIR "Test2_Curve_OIS_inputs";
    extern const char TEST2_STD[]				= TEST_DIR "Test2_Curve_STD_inputs";
    extern const char TEST2_3M6MBASIS_SPOT[]	= TEST_DIR "Test2_Curve_3M6MBasisSpot_inputs";
    extern const char TEST2_3M6MBASIS_FWD[]		= TEST_DIR "Test2_Curve_3M6M_inputs";

    //
    // test call input and reference files
    //
    extern const char TEST1_DELTA_INPUTS[]			= TEST_DIR "Test1_DeltaLadder_inputs.csv";
    extern const char TEST1_DELTA_OUTPUTS_32[]		= TEST_DIR "Test1_DeltaLadder_outputs_32bit.csv";
    extern const char TEST1_DELTA_OUTPUTS_64[]		= TEST_DIR "Test1_DeltaLadder_outputs_64bit.csv";
    extern const char RAW_TEST1_DELTA_OUTPUTS_32[]	= "Test1_DeltaLadder_outputs_32bit.csv";
    extern const char RAW_TEST1_DELTA_OUTPUTS_64[]	= "Test1_DeltaLadder_outputs_64bit.csv";

    extern const char TEST2_DELTA_INPUTS[]			= TEST_DIR "Test2_DeltaLadder_inputs.csv";
    extern const char TEST2_DELTA_OUTPUTS_32[]		= TEST_DIR "Test2_DeltaLadder_outputs_32bit.csv";
    extern const char TEST2_DELTA_OUTPUTS_64[]		= TEST_DIR "Test2_DeltaLadder_outputs_64bit.csv";
    extern const char RAW_TEST2_DELTA_OUTPUTS_32[]	= "Test2_DeltaLadder_outputs_32bit.csv";
    extern const char RAW_TEST2_DELTA_OUTPUTS_64[]	= "Test2_DeltaLadder_outputs_64bit.csv";

    const int TRADE_COUNT = 3;


}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods

    class TestRiskTenorBasisCurve1 : public testing::Test,
        protected virtual BindFileToClassConstructor<CurveOis, TEST1_OIS>,
        protected virtual BindFileToClassConstructor<CurveStd, TEST1_STD>,
        protected virtual BindFileToClassConstructor<CurveTenorBasis, TEST1_3M6MBASIS_SPOT>,
        protected virtual BindFileToClassConstructor<CurveTenorBasis, TEST1_3M6MBASIS_FWD>
    {};

    TEST_F( TestRiskTenorBasisCurve1, SCENARIO_BasisCurve )
    {
        const ReadDataFile::Load inputFile( TEST1_DELTA_INPUTS );

        std::vector<LabelValueBlock> dealInfo;
        for ( size_t i = 0; i < TRADE_COUNT; ++i )
        {
            std::stringstream ss;
            ss << i;
            std::string name = "dealInfo_" + ss.str();
            LAStringMatrix m = inputFile[name.c_str()];
            LabelValueBlock oneDeal( m );
            dealInfo.push_back( oneDeal );
        }

        LAStringVector forecastCurveSet = inputFile["forecastCurveSet"];
        LAString curveCollection = inputFile["curveCollection"];
        LAStringVector pillarNames;
        LAStringVector headers;
        DoubleMatrix deltas;

        validation_api::tryMeProductSwapDelta(
            pillarNames,
            headers,
            deltas,
            getDataInstance(),
            dealInfo,
            forecastCurveSet,
            curveCollection,
            inputFile["bumpSize"],
            inputFile["bumpMode"],
            "LADDER",
            false ); // aggregate risks

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
#ifdef GTEST32
            LAString outputFileName = RAW_TEST1_DELTA_OUTPUTS_32;
#else
            LAString outputFileName = RAW_TEST1_DELTA_OUTPUTS_64;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                DoubleVector deltasPerRow = deltas[i];

                // 12 is the number of decimal points required
                file.write( pillarNames[i], deltasPerRow, 12 );
            }
        }
        else
        {
#ifdef GTEST32
            const ReadDataFile::Load resultFile( TEST1_DELTA_OUTPUTS_32 );
#else
            const ReadDataFile::Load resultFile( TEST1_DELTA_OUTPUTS_64 );
#endif

            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                LAString key	= pillarNames[i];

                DoubleVector delta	= deltas[i];
                DoubleVector ref = resultFile[key];

                for( size_t j = 0; j < delta.size(); ++j )
                {
                    EXPECT_NEAR( delta[j], ref[j], tolerance )
                            << " Delta for pillar point " << key.getCString() << " is incorrect ";
                }
            }
        }
    }



    //
    // Construct depedent curves from curve source files by calling the curve constructor methods

    class TestRiskTenorBasisCurve2 : public testing::Test,
        protected virtual BindFileToClassConstructor<CurveOis, TEST2_OIS>,
        protected virtual BindFileToClassConstructor<CurveStd, TEST2_STD>,
        protected virtual BindFileToClassConstructor<CurveTenorBasis, TEST2_3M6MBASIS_SPOT>,
        protected virtual BindFileToClassConstructor<CurveTenorBasis, TEST2_3M6MBASIS_FWD>
    {};

    TEST_F( TestRiskTenorBasisCurve2, SCENARIO_BasisCurve )
    {
        const ReadDataFile::Load inputFile( TEST2_DELTA_INPUTS );

        std::vector<LabelValueBlock> dealInfo;
        for ( size_t i = 0; i < TRADE_COUNT; ++i )
        {
            std::stringstream ss;
            ss << i;
            std::string name = "dealInfo_" + ss.str();
            LAStringMatrix m = inputFile[name.c_str()];
            LabelValueBlock oneDeal( m );
            dealInfo.push_back( oneDeal );
        }

        LAStringVector forecastCurveSet = inputFile["forecastCurveSet"];
        LAString curveCollection = inputFile["curveCollection"];
        LAStringVector pillarNames;
        LAStringVector headers;
        DoubleMatrix deltas;
        
        // Dummy Xccy FX Spot Rates
        DoubleVector dummyXccyFXSpotRates( TRADE_COUNT, 1.0 );

        validation_api::tryMeProductSwapDelta(
            pillarNames,
            headers,
            deltas,
            getDataInstance(),
            dealInfo,
            forecastCurveSet,
            curveCollection,
            inputFile["bumpSize"],
            inputFile["bumpMode"],
            "LADDER",
            false ); // aggregate risks

        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
#ifdef GTEST32
            LAString outputFileName = RAW_TEST2_DELTA_OUTPUTS_32;
#else
            LAString outputFileName = RAW_TEST2_DELTA_OUTPUTS_64;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                DoubleVector deltasPerRow = deltas[i];

                // 12 is the number of decimal points required
                file.write( pillarNames[i], deltasPerRow, 12 );
            }
        }
        else
        {
#ifdef GTEST32
            const ReadDataFile::Load resultFile( TEST2_DELTA_OUTPUTS_32 );
#else
            const ReadDataFile::Load resultFile( TEST2_DELTA_OUTPUTS_64 );
#endif

            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                LAString key	= pillarNames[i];

                DoubleVector delta	= deltas[i];
                DoubleVector ref = resultFile[key];

                for( size_t j = 0; j < delta.size(); ++j )
                {
                    EXPECT_NEAR( delta[j], ref[j], tolerance )
                            << " Delta for pillar point " << key.getCString() << " is incorrect ";
                }
            }
        }
    }

}
