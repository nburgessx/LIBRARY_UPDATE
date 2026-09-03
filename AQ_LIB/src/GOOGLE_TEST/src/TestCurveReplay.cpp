// TestCurveReplay.cpp

/*
* @File:			This file tests the ability to reload yield curves from the recorded csv log files
* @Created:			18 April 2016
* @Author:			Nicholas Burgess
* @Department:		Quant Developement Team, AlgoQuantHub London
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

// Includes: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "tryMirReplay.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"


// Test Directory Folder
#define TEST_DIR "Vanilla/Curves/TestCurveReplay/"


// Test Filenames
const char USDOisCurveInputs[]           = TEST_DIR "USDYC_OIS_tryMirSetUpOISCurve_inputs.csv";
const char USDStdCurveInputs[]           = TEST_DIR "USDYC_STD_tryMirSetUpSwapCurve_inputs.csv";

const char GBPOisCurveInputs[]           = TEST_DIR "GBPYC_OIS_tryMirSetUpOISCurve_inputs.csv";
const char GBPStdCurveInputs[]           = TEST_DIR "GBPYC_STD_tryMirSetUpSwapCurve_inputs.csv";
const char GBPTenorBasisCurveInputs[]    = TEST_DIR "GBPYC_3M6M_tryMirSetUpBasisSwapCurve_inputs.csv";
const char GBPXccyBasisCurveInputs[]     = TEST_DIR "GBPYC_XCCY_tryMirSetUpBasisSwapCurve_inputs.csv";

const char EUROisCurveInputs[]           = TEST_DIR "EURYC_OIS_tryMirSetUpOISCurve_inputs.csv";
const char EURStdCurveInputs[]           = TEST_DIR "EURYC_STD_tryMirSetUpSwapCurve_inputs.csv";
const char EURTenorBasisCurveInputs[]    = TEST_DIR "EURYC_3M6M_tryMirSetUpBasisSwapCurve_inputs.csv";
const char EURXccyBasisCurveInputs[]     = TEST_DIR "EURYC_XCCY_tryMirSetUpBasisSwapCurve_inputs.csv";

const char EURFXFwdGbpCurveInputs[]      = TEST_DIR "EURYC_FWDGBP_tryMirSetUpFwdFXConstantCurve_inputs.csv";


namespace google_test
{
    using etrading::ReadDataFile;
    using validation::tryMirReplay;

	DECLARE_TEST_FIXTURE(TestCurveReplay);

    //TEST_F( TestCurveReplay, CheckReplayOisCurves )
    //{
    //    try
    //    {
    //        // Actual Result from Replay the OIS Curve
    //        const AQLString actualResult = tryMirReplay( EUROisCurveInputs );
    //
    //        // Load the Input Curve and Read the curveID and marketName
    //        const ReadDataFile::Load inputFile( EUROisCurveInputs );
    //        const AQLString curveID = inputFile["curveID"];
    //        const AQLString marketName = inputFile["marketName"];

    //        // Expected Result
    //        const AQLString expectedResult = curveID + " " + marketName + " Curve has been set";
    //
    //        // Compare
    //        EXPECT_EQ( actualResult, expectedResult );
    //    }
    //    catch( const ReadDataFile::LoadError& )
    //    {
    //        std::cout << "#Error: Unable to Load Test File";
    //        ASSERT_FALSE( true );
    // }
    //    catch( const AQLCoreError & m )
    //    {
    //        std::cout <<  m.getMsg();
    //        ASSERT_FALSE( true );
    //    }
    //    catch( const std::exception & e )
    //    {
    //        std::cout << e.what();
    //        ASSERT_FALSE( true );
    //    }
    //}

    TEST_F( TestCurveReplay, UNIT_CheckReplayStdCurves )
    {
        try
        {
            // Actual Result from Replay the OIS Curve
            const AQLString oisResult = tryMirReplay( EUROisCurveInputs );
            const AQLString actualResult = tryMirReplay( EURStdCurveInputs );

            // Load the Input Curve and Read the curveID and marketName
            const ReadDataFile::Load inputFile( EURStdCurveInputs );
            const AQLString curveID = inputFile["curveID"];
            const AQLString marketName = inputFile["marketName"];

            // Expected Result
            const AQLString expectedResult = curveID + " " + marketName + " Curve has been set";

            // Compare
            EXPECT_EQ( actualResult, expectedResult );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout << "#Error: Unable to Load Test File";
            ASSERT_FALSE( true );
        }
        catch( const AQLCoreError& m )
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


    //TEST_F( TestCurveReplay, CheckReplayTenorBasisCurves )
    //{
    //    try
    //    {
    //        // Actual Result from Replay the OIS Curve
    //        const AQLString oisResult = tryMirReplay( EUROisCurveInputs );
    //        const AQLString stdResult = tryMirReplay( EURStdCurveInputs );
    //        const AQLString actualResult = tryMirReplay( EURTenorBasisCurveInputs );
    //
    //        // Load the Input Curve and Read the curveID and marketName
    //        const ReadDataFile::Load inputFile( EURTenorBasisCurveInputs );
    //        const AQLString curveID = inputFile["curveID"];
    //        const AQLString marketName = inputFile["marketName"];

    //        // Expected Result
    //        const AQLString expectedResult = curveID + " " + marketName + " Curve has been set";
    //
    //        // Compare
    //        EXPECT_EQ( actualResult, expectedResult );
    //    }
    //    catch( const ReadDataFile::LoadError& )
    //    {
    //        std::cout << "#Error: Unable to Load Test File";
    //        ASSERT_FALSE( true );
    // }
    //    catch( const AQLCoreError & m )
    //    {
    //        std::cout <<  m.getMsg();
    //        ASSERT_FALSE( true );
    //    }
    //    catch( const std::exception & e )
    //    {
    //        std::cout << e.what();
    //        ASSERT_FALSE( true );
    //    }
    //}


    //TEST_F( TestCurveReplay, CheckReplayXccyBasisCurves )
    //{
    //    try
    //    {
    //        // Actual Result from Replay the OIS Curve
    //        const AQLString usdOisResult         = tryMirReplay( USDOisCurveInputs );
    //        const AQLString usdStdResult         = tryMirReplay( USDStdCurveInputs );

    //        const AQLString eurOisResult         = tryMirReplay( EUROisCurveInputs );
    //        const AQLString eurStdResult         = tryMirReplay( EURStdCurveInputs );
    //        const AQLString eurTenorBasisResult  = tryMirReplay( EURTenorBasisCurveInputs );
    //
    //        const AQLString actualResult         = tryMirReplay( EURXccyBasisCurveInputs );
    //
    //        // Load the Input Curve and Read the curveID and marketName
    //        const ReadDataFile::Load inputFile( EURXccyBasisCurveInputs );
    //        const AQLString curveID = inputFile["curveID"];
    //        const AQLString marketName = inputFile["marketName"];

    //        // Expected Result
    //        const AQLString expectedResult = curveID + " " + marketName + " Curve has been set";
    //
    //        // Compare
    //        EXPECT_EQ( actualResult, expectedResult );
    //    }
    //    catch( const ReadDataFile::LoadError& )
    //    {
    //        std::cout << "#Error: Unable to Load Test File";
    //        ASSERT_FALSE( true );
    // }
    //    catch( const AQLCoreError & m )
    //    {
    //        std::cout <<  m.getMsg();
    //        ASSERT_FALSE( true );
    //    }
    //    catch( const std::exception & e )
    //    {
    //        std::cout << e.what();
    //        ASSERT_FALSE( true );
    //    }
    //}


    //TEST_F( TestCurveReplay, CheckReplayFXForwardCurves )
    //{
    //    try
    //    {
    //        // Actual Result from Replay the OIS Curve
    //        const AQLString usdOisResult         = tryMirReplay( USDOisCurveInputs );
    //        const AQLString usdStdResult         = tryMirReplay( USDStdCurveInputs );

    //        const AQLString gbpOisResult         = tryMirReplay( GBPOisCurveInputs );
    //        const AQLString gbpStdResult         = tryMirReplay( GBPStdCurveInputs );
    //        const AQLString gbpTenorBasisResult  = tryMirReplay( GBPTenorBasisCurveInputs );
    //        const AQLString gbpXccyBasisResult   = tryMirReplay( GBPXccyBasisCurveInputs );

    //        const AQLString eurOisResult         = tryMirReplay( EUROisCurveInputs );
    //        const AQLString eurStdResult         = tryMirReplay( EURStdCurveInputs );
    //        const AQLString eurTenorBasisResult  = tryMirReplay( EURTenorBasisCurveInputs );
    //        const AQLString eurXccyBasisResult   = tryMirReplay( EURXccyBasisCurveInputs );
    //
    //        const AQLString actualResult         = tryMirReplay( EURFXFwdGbpCurveInputs );

    //        // Load the Input Curve and Read the curveID and marketName
    //        const ReadDataFile::Load inputFile( EURFXFwdGbpCurveInputs );
    //        const AQLString curveID = inputFile["curveID"];
    //        const AQLString marketName = inputFile["marketName"];

    //        // Expected Result
    //        const AQLString expectedResult = curveID + " " + marketName + " Curve has been set";
    //
    //        // Compare
    //        EXPECT_EQ( actualResult, expectedResult );
    //    }
    //    catch( const ReadDataFile::LoadError& )
    //    {
    //        std::cout << "#Error: Unable to Load Test File";
    //        ASSERT_FALSE( true );
    // }
    //    catch( const AQLCoreError & m )
    //    {
    //        std::cout <<  m.getMsg();
    //        ASSERT_FALSE( true );
    //    }
    //    catch( const std::exception & e )
    //    {
    //        std::cout << e.what();
    //        ASSERT_FALSE( true );
    //    }
    //}

}
