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

#define TEST_DIR "Vanilla/Risks/TestRiskSwapDeltaLadder/"

namespace
{
    // test tolerance
    const double tolerance = 1.5e-3;  // Notional of test trades is 100MM and delta through bumping is inherently noisy.

    // curve input file
    extern const char DIR_EURYC_OIS[]			= TEST_DIR "EURYC_OIS_tryMirSetUpOISCurve_inputs";
    extern const char DIR_EURYC_3M[]			= TEST_DIR "EURYC_3M6M_tryMirSetUpBasisSwapCurve_inputs";
    extern const char DIR_EURYC_6M[]			= TEST_DIR "EURYC_STD_tryMirSetUpSwapCurve_inputs";
    extern const char DIR_EURYC_USDCSA[]		= TEST_DIR "EURYC_EURDFUSDCSA_tryMirSetUpBasisSwapCurve_inputs";
    extern const char DIR_USDYC_OIS[]			= TEST_DIR "USDYC_OIS_tryMirSetUpOISCurve_inputs";
    extern const char DIR_USDYC_STD[]			= TEST_DIR "USDYC_STD_tryMirSetUpSwapCurve_inputs";

    //
    // test call input and reference files
    //
    extern const char inputs[]		    = TEST_DIR "tryMeSwapDelta_inputs.csv";
    extern const char raw_outputs_32[]	= "tryMeSwapDelta_outputs.csv";
    extern const char raw_outputs_64[]	= "tryMeSwapDelta_outputs_64bit.csv";
    extern const char outputs_32[]		= TEST_DIR "tryMeSwapDelta_outputs.csv";
    extern const char outputs_64[]		= TEST_DIR "tryMeSwapDelta_outputs_64bit.csv";


    const int TRADE_COUNT = 4;

}

namespace google_test
{

    // Construct depedent curves from curve source files by calling the curve constructor methods

    class TestRiskSwapDeltaLadder : public testing::Test,
        protected virtual BindFileToClassConstructor<CurveOis, DIR_EURYC_OIS>,
        protected virtual BindFileToClassConstructor<CurveStd, DIR_EURYC_6M>,
        protected virtual BindFileToClassConstructor<CurveTenorBasis, DIR_EURYC_3M>,
        protected virtual BindFileToClassConstructor<CurveOis, DIR_USDYC_OIS>,
        protected virtual BindFileToClassConstructor<CurveStd, DIR_USDYC_STD>,
        protected virtual BindFileToClassConstructor<CurveXccyBasis, DIR_EURYC_USDCSA>
    {};

    // --------------------------------------------------------------------------------------
    //
    // Use discount factors to check integrity of yield curves
    //

    TEST_F( TestRiskSwapDeltaLadder, SCENARIO_Portfolio )
    {
        const ReadDataFile::Load inputFile( inputs );

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

        validation_api::tryMeProductSwapDelta( pillarNames,
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
            LAString outputFileName = raw_outputs_32;
#else
            LAString outputFileName = raw_outputs_64;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( pillarNames[i], deltas[i], 12 );
            }
        }
        else
        {
            // Carry out actual test and peform result comparison
#ifdef GTEST32
            const ReadDataFile::Load resultFile( outputs_32 );
#else
            const ReadDataFile::Load resultFile( outputs_64 );
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
