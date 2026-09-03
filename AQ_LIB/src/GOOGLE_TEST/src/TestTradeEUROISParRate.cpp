/*
* @File:			This file tests the par rate and PV methods for OIS swaps
* @Created:			09 March 2016
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
#include "tryMirOISParRate.h"
#include "tryMirOISSwapPV.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "Vanilla/Trades/TestTradeEUROISParRate/"

namespace
{
    // test tolerance
    const double tolerance = 1e-7;

    const int PARRATE_TEST_COUNT = 22;
    const int PV_TEST_COUNT		 = 22;

    //
    // Curve input files.
    //
    extern const char DIR_EURYC_OIS[]			= TEST_DIR "EURYC_OIS_tryMirSetUpOISCurve_inputs";
    extern const char DIR_EURYC_1M[]			= "";
    extern const char DIR_EURYC_3M[]			= "";
    extern const char DIR_EURYC_6M[]			= "";
    extern const char DIR_EURYC_12M[]			= "";

    //
    // test call input and reference files
    //
    extern const char getParRateInputs[]     = TEST_DIR "EURYC_tryMirOISParRate_inputs";
    extern const char getParRateOutputs[]	 = TEST_DIR "EURYC_tryMirOISParRate_outputs";

    extern const char getPVInputs[]			 = TEST_DIR "EURYC_tryMirOISSwapPV_inputs";
    extern const char getPVOutputs[]		 = TEST_DIR "EURYC_tryMirOISSwapPV_outputs";
}


namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    ADD_EUR_CURVE_DEPENDENCIES( TestTradeEUROISParRate, DIR_EURYC_OIS, DIR_EURYC_1M, DIR_EURYC_3M, DIR_EURYC_6M, DIR_EURYC_12M );


    //
    // Call Test Fixture for OIS Swap par rate
    //
    TEST_F( TestTradeEUROISParRate, SNAPSHOT_CheckHardCodedOISParRateUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; PARRATE_TEST_COUNT; ++i )
            {
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getParRateInputs, i ) );

                double parRate = validation_api::tryMirOISParRate( getDataInstance(),
                                 inputFile["effectDt"],
                                 inputFile["maturity"],
                                 inputFile["curveID"],
                                 inputFile["xFreq"],
                                 inputFile["xDayCount"],
                                 inputFile["xRollCnv"],
                                 inputFile["xCalendar"],
                                 inputFile["xFirstStub"],
                                 inputFile["xLastStub"],
                                 inputFile["xRollDay"],
                                 inputFile["xPayLag"],
                                 inputFile["xStub"],
                                 inputFile["tFreq"],
                                 inputFile["tDayCount"],
                                 inputFile["tRollCnv"],
                                 inputFile["tCalendar"],
                                 inputFile["tFirstStub"],
                                 inputFile["tLastStub"],
                                 inputFile["tRollDay"],
                                 inputFile["tFixLag"],
                                 inputFile["tFirstFix"],
                                 inputFile["tLastFix"],
                                 inputFile["tPayLag"],
                                 inputFile["tStub"],
                                 inputFile["interpolation"],
                                 inputFile["forecastCurve"],
                                 inputFile["discountCurve"],
                                 inputFile["eomRoll"],
                                 inputFile["tSpd"],
                                 inputFile["oisCompoundingType"] );

                CheckTestResultsAndRebaseOnRequest( parRate, TEST_DIR, getParRateOutputs, tolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, PARRATE_TEST_COUNT );
        }
        catch( const LACoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }

    //
    // Call Test Fixture for OIS Swap PV
    //
    TEST_F( TestTradeEUROISParRate, SNAPSHOT_CheckHardCodedOISPVUnchanged )
    {
        int i = 0;
        try
        {
            for ( i = 0; i < PV_TEST_COUNT; ++i )
            {
                const ReadDataFile::Load inputFile( CreateDataFile::makeFilename( getPVInputs, i ) );

                // Read the input file into the getParRateFunction
                const double pv = validation_api::tryMirOISSwapPV( getDataInstance(),
                                  inputFile["notional"],
                                  inputFile["payRec"],
                                  inputFile["effectDt"],
                                  inputFile["maturity"],
                                  inputFile["curveID"],
                                  inputFile["xRt"],
                                  inputFile["xFreq"],
                                  inputFile["xDayCount"],
                                  inputFile["xRollCnv"],
                                  inputFile["xCalendar"],
                                  inputFile["xFirstStub"],
                                  inputFile["xLastStub"],
                                  inputFile["xRollDay"],
                                  inputFile["xPayLag"],
                                  inputFile["xStub"],
                                  inputFile["tFreq"],
                                  inputFile["tDayCount"],
                                  inputFile["tRollCnv"],
                                  inputFile["tCalendar"],
                                  inputFile["tFirstStub"],
                                  inputFile["tLastStub"],
                                  inputFile["tRollDay"],
                                  inputFile["tFixLag"],
                                  inputFile["tFirstFix"],
                                  inputFile["tLastFix"],
                                  inputFile["tPayLag"],
                                  inputFile["tStub"],
                                  inputFile["interpolation"],
                                  inputFile["forecastCurve"],
                                  inputFile["discountCurve"],
                                  inputFile["eomRoll"],
                                  inputFile["tSpd"],
                                  inputFile["compMethod"] );

                CheckTestResultsAndRebaseOnRequest( pv, TEST_DIR, getPVOutputs, tolerance, i );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GE( i, PV_TEST_COUNT );
        }
        catch( const LACoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }

}
