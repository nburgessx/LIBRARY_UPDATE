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
    //        const LAString actualResult = tryMirReplay( EUROisCurveInputs );
    //
    //        // Load the Input Curve and Read the curveID and marketName
    //        const ReadDataFile::Load inputFile( EUROisCurveInputs );
    //        const LAString curveID = inputFile["curveID"];
    //        const LAString marketName = inputFile["marketName"];

    //        // Expected Result
    //        const LAString expectedResult = curveID + " " + marketName + " Curve has been set";
    //
    //        // Compare
    //        EXPECT_EQ( actualResult, expectedResult );
    //    }
    //    catch( const ReadDataFile::LoadError& )
    //    {
    //        std::cout << "#Error: Unable to Load Test File";
    //        ASSERT_FALSE( true );
    // }
    //    catch( const LACoreError & m )
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
            const LAString oisResult = tryMirReplay( EUROisCurveInputs );
            const LAString actualResult = tryMirReplay( EURStdCurveInputs );

            // Load the Input Curve and Read the curveID and marketName
            const ReadDataFile::Load inputFile( EURStdCurveInputs );
            const LAString curveID = inputFile["curveID"];
            const LAString marketName = inputFile["marketName"];

            // Expected Result
            const LAString expectedResult = curveID + " " + marketName + " Curve has been set";

            // Compare
            EXPECT_EQ( actualResult, expectedResult );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout << "#Error: Unable to Load Test File";
            ASSERT_FALSE( true );
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


    //TEST_F( TestCurveReplay, CheckReplayTenorBasisCurves )
    //{
    //    try
    //    {
    //        // Actual Result from Replay the OIS Curve
    //        const LAString oisResult = tryMirReplay( EUROisCurveInputs );
    //        const LAString stdResult = tryMirReplay( EURStdCurveInputs );
    //        const LAString actualResult = tryMirReplay( EURTenorBasisCurveInputs );
    //
    //        // Load the Input Curve and Read the curveID and marketName
    //        const ReadDataFile::Load inputFile( EURTenorBasisCurveInputs );
    //        const LAString curveID = inputFile["curveID"];
    //        const LAString marketName = inputFile["marketName"];

    //        // Expected Result
    //        const LAString expectedResult = curveID + " " + marketName + " Curve has been set";
    //
    //        // Compare
    //        EXPECT_EQ( actualResult, expectedResult );
    //    }
    //    catch( const ReadDataFile::LoadError& )
    //    {
    //        std::cout << "#Error: Unable to Load Test File";
    //        ASSERT_FALSE( true );
    // }
    //    catch( const LACoreError & m )
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
    //        const LAString usdOisResult         = tryMirReplay( USDOisCurveInputs );
    //        const LAString usdStdResult         = tryMirReplay( USDStdCurveInputs );

    //        const LAString eurOisResult         = tryMirReplay( EUROisCurveInputs );
    //        const LAString eurStdResult         = tryMirReplay( EURStdCurveInputs );
    //        const LAString eurTenorBasisResult  = tryMirReplay( EURTenorBasisCurveInputs );
    //
    //        const LAString actualResult         = tryMirReplay( EURXccyBasisCurveInputs );
    //
    //        // Load the Input Curve and Read the curveID and marketName
    //        const ReadDataFile::Load inputFile( EURXccyBasisCurveInputs );
    //        const LAString curveID = inputFile["curveID"];
    //        const LAString marketName = inputFile["marketName"];

    //        // Expected Result
    //        const LAString expectedResult = curveID + " " + marketName + " Curve has been set";
    //
    //        // Compare
    //        EXPECT_EQ( actualResult, expectedResult );
    //    }
    //    catch( const ReadDataFile::LoadError& )
    //    {
    //        std::cout << "#Error: Unable to Load Test File";
    //        ASSERT_FALSE( true );
    // }
    //    catch( const LACoreError & m )
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
    //        const LAString usdOisResult         = tryMirReplay( USDOisCurveInputs );
    //        const LAString usdStdResult         = tryMirReplay( USDStdCurveInputs );

    //        const LAString gbpOisResult         = tryMirReplay( GBPOisCurveInputs );
    //        const LAString gbpStdResult         = tryMirReplay( GBPStdCurveInputs );
    //        const LAString gbpTenorBasisResult  = tryMirReplay( GBPTenorBasisCurveInputs );
    //        const LAString gbpXccyBasisResult   = tryMirReplay( GBPXccyBasisCurveInputs );

    //        const LAString eurOisResult         = tryMirReplay( EUROisCurveInputs );
    //        const LAString eurStdResult         = tryMirReplay( EURStdCurveInputs );
    //        const LAString eurTenorBasisResult  = tryMirReplay( EURTenorBasisCurveInputs );
    //        const LAString eurXccyBasisResult   = tryMirReplay( EURXccyBasisCurveInputs );
    //
    //        const LAString actualResult         = tryMirReplay( EURFXFwdGbpCurveInputs );

    //        // Load the Input Curve and Read the curveID and marketName
    //        const ReadDataFile::Load inputFile( EURFXFwdGbpCurveInputs );
    //        const LAString curveID = inputFile["curveID"];
    //        const LAString marketName = inputFile["marketName"];

    //        // Expected Result
    //        const LAString expectedResult = curveID + " " + marketName + " Curve has been set";
    //
    //        // Compare
    //        EXPECT_EQ( actualResult, expectedResult );
    //    }
    //    catch( const ReadDataFile::LoadError& )
    //    {
    //        std::cout << "#Error: Unable to Load Test File";
    //        ASSERT_FALSE( true );
    // }
    //    catch( const LACoreError & m )
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
