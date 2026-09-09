// TestAQObjSwapEURVariableNotionalSwaps.cpp

#include "tryAqSwapObjectSchedule.h"
#include "tryAqSwapObjectCreation.h"
#include "tryAqSwapObjectPricing.h"


#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"


using etrading::ReadDataFile;
using etrading::CreateDataFile;
#include <gTest/gTest.h>

using google_test::CurveOis;
using google_test::CurveStd;
using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using google_test::Dependency2;
using google_test::Dependency3;
using google_test::Dependency4;

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/AQObjects/TestAQObjSwapVariableNotionalSwaps/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;	// solver tolerance is 1e-9

    //
    // curve input files
    //
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryAqCurveCalibrateOIS_inputs";
    extern const char EURYC_1M[]			    = TEST_DIR "EURYC_1M3M_tryAqCurveCalibrateBasis_inputs";
    extern const char EURYC_3M[]			    = TEST_DIR "EURYC_3M6M_tryAqCurveCalibrateBasis_inputs";
    extern const char EURYC_6M[]			    = TEST_DIR "EURYC_STD_tryAqCurveCalibrateSwap_inputs";
    extern const char EURYC_12M[]			    = TEST_DIR "EURYC_6M12MBasis_tryAqCurveCalibrateBasis_inputs";

    //
    // test call input and reference files
    //
    extern const char fixedSchedulePath[]	    = TEST_DIR "EUR_FIXEDSCHEDULE"; // Test files require a suffix. Format :=  EUR_FIXEDSCHEDULE + '_tryAqSwapObjectScheduleCreateBespoke_inputs/outputs' + Index
    extern const char floatSchedulePath[]       = TEST_DIR "EUR_FLOATSCHEDULE"; // Test files require a suffix. Format :=  EUR_FLOATSCHEDULE + '_tryAqSwapObjectScheduleCreateBespoke_inputs/outputs' + Index
    extern const char vnsPath[]                 = TEST_DIR "EUR_VNS";           // Test files require a suffix. Format :=  EUR_VNS + '_tryAqSwapObjectCreateFromSchedule_inputs/outputs' + Index
}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_EUR_CURVE( TestAQObjSwapEURVariableNotionalSwaps, EURYC_OIS, EURYC_1M, EURYC_3M, EURYC_6M, EURYC_12M );


    //
    // Call Test Fixture
    //

    TEST_F( TestAQObjSwapEURVariableNotionalSwaps, UNIT_CheckVariableNotionalSwapScheduleCreation )
    {
        try
        {
            // Load the input files : i.e. the fixed and floating schedules
            AQLString fixedScheduleInputFilePath = etrading::decorateFilename( fixedSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
            AQLString floatScheduleInputFilePath = etrading::decorateFilename( floatSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
            
            const ReadDataFile::Load fixedScheduleInputFile( fixedScheduleInputFilePath );
            const ReadDataFile::Load floatScheduleInputFile( floatScheduleInputFilePath );


            // Recreate the Fixed Schedule
            std::string     fixedScheduleName       = fixedScheduleInputFile["scheduleName"];
            AQLStringMatrix    fixedScheduleProperties = fixedScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    fixedScheduleDates      = fixedScheduleInputFile["bespokeScheduleLVB"];
	        std::string     fixedScheduleResult     = validation::tryAqSwapObjectScheduleCreateBespoke( fixedScheduleName, fixedScheduleProperties, fixedScheduleDates );

            // Recreate the Float Schedule
            std::string     floatScheduleName       = floatScheduleInputFile["scheduleName"];
            AQLStringMatrix    floatScheduleProperties = floatScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    floatScheduleDates      = floatScheduleInputFile["bespokeScheduleLVB"];
	        std::string     floatScheduleResult     = validation::tryAqSwapObjectScheduleCreateBespoke( floatScheduleName, floatScheduleProperties, floatScheduleDates );

            // Load the Saved Schedules
            AQLString fixedScheduleOutputFilePath = etrading::decorateFilename( fixedSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_outputs.csv" );
            AQLString floatScheduleOutputFilePath = etrading::decorateFilename( floatSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_outputs.csv" );
                
            const ReadDataFile::Load fixedScheduleOutputFile( fixedScheduleOutputFilePath );
            const ReadDataFile::Load floatScheduleOutputFile( floatScheduleOutputFilePath );

            // Check Schedules Load OK
            std::string fixedExpectedResult = fixedScheduleOutputFile["output"];
            std::string floatExpectedResult = floatScheduleOutputFile["output"];

            // Compare the local result with the output file result
            EXPECT_EQ( fixedScheduleResult, fixedExpectedResult );
            EXPECT_EQ( floatScheduleResult, floatExpectedResult );
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


    TEST_F( TestAQObjSwapEURVariableNotionalSwaps, UNIT_CheckVariableNotionalSwapCreation )
    {
        try
        {
            // 1. Load the fixed and float Schedules
            AQLString fixedScheduleInputFilePath = etrading::decorateFilename( fixedSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
            AQLString floatScheduleInputFilePath = etrading::decorateFilename( floatSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
                
            const ReadDataFile::Load fixedScheduleInputFile( fixedScheduleInputFilePath );
            const ReadDataFile::Load floatScheduleInputFile( floatScheduleInputFilePath );
                
            // Recreate the Fixed Schedule
            std::string     fixedScheduleName       = fixedScheduleInputFile["scheduleName"];
            AQLStringMatrix    fixedScheduleProperties = fixedScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    fixedScheduleDates      = fixedScheduleInputFile["bespokeScheduleLVB"];
	        std::string     loadFixedSchedule       = validation::tryAqSwapObjectScheduleCreateBespoke( fixedScheduleName, fixedScheduleProperties, fixedScheduleDates );

            // Recreate the Float Schedule
            std::string     floatScheduleName       = floatScheduleInputFile["scheduleName"];
            AQLStringMatrix    floatScheduleProperties = floatScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    floatScheduleDates      = floatScheduleInputFile["bespokeScheduleLVB"];
	        std::string     loadFloatSchedule       = validation::tryAqSwapObjectScheduleCreateBespoke( floatScheduleName, floatScheduleProperties, floatScheduleDates );



            // 2. Load the Variable Notional Swap, which requires the fixed and float schedules to be loaded first
            AQLString vnsInputFilePath = etrading::decorateFilename( vnsPath, "", "tryAqSwapObjectCreateFromSchedule_inputs.csv" );
            const ReadDataFile::Load vnsInputFile( vnsInputFilePath );
                
            // Recreate the Variable Notional Swap
            std::string     vnsName                 = vnsInputFile["swapName"];
            std::string     vnsFixedScheduleName    = vnsInputFile["schedule1Name"];
            std::string     vnsFloatScheduleName    = vnsInputFile["schedule2Name"];
            AQLStringMatrix    vnsFixedScheduleParams  = vnsInputFile["leg1LVB"];
            AQLStringMatrix    vnsFloatScheduleParams  = vnsInputFile["leg2LVB"];
            bool            validateKeys            = vnsInputFile["validateKeys"];
	            
            std::string     vnsResult = validation::tryAqSwapObjectCreateFromSchedule( vnsName,
                                                                                        vnsFixedScheduleName,
                                                                                        vnsFloatScheduleName,
                                                                                        vnsFixedScheduleParams,
                                                                                        vnsFloatScheduleParams,
                                                                                        AQLStringMatrix(), 
                                                                                        false, 
                                                                                        validateKeys );


            // 3. Load the Expected Result
            AQLString vnsOutputFilePath = etrading::decorateFilename( vnsPath, "", "tryAqSwapObjectCreateFromSchedule_outputs.csv" );
            const ReadDataFile::Load vnsOutputFile( vnsOutputFilePath );

            // Get the expected result
            std::string vnsExpectedResult = vnsOutputFile["output"];
                
            // Compare the local result with the output file result
            EXPECT_EQ( vnsResult, vnsExpectedResult );
                
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


    TEST_F( TestAQObjSwapEURVariableNotionalSwaps, SNAPSHOT_CheckVariableNotionalSwapPV )
    {
        try
        {
            // 1. Load the fixed and float Schedules
            AQLString fixedScheduleInputFilePath = etrading::decorateFilename( fixedSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
            AQLString floatScheduleInputFilePath = etrading::decorateFilename( floatSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
                
            const ReadDataFile::Load fixedScheduleInputFile( fixedScheduleInputFilePath );
            const ReadDataFile::Load floatScheduleInputFile( floatScheduleInputFilePath );
                
            // Recreate the Fixed Schedule
            std::string     fixedScheduleName       = fixedScheduleInputFile["scheduleName"];
            AQLStringMatrix    fixedScheduleProperties = fixedScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    fixedScheduleDates      = fixedScheduleInputFile["bespokeScheduleLVB"];
	        std::string     loadFixedSchedule       = validation::tryAqSwapObjectScheduleCreateBespoke( fixedScheduleName, fixedScheduleProperties, fixedScheduleDates );

            // Recreate the Float Schedule
            std::string     floatScheduleName       = floatScheduleInputFile["scheduleName"];
            AQLStringMatrix    floatScheduleProperties = floatScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    floatScheduleDates      = floatScheduleInputFile["bespokeScheduleLVB"];
	        std::string     loadFloatSchedule       = validation::tryAqSwapObjectScheduleCreateBespoke( floatScheduleName, floatScheduleProperties, floatScheduleDates );



            // 2. Load the Variable Notional Swap, which requires the fixed and float schedules to be loaded first
            AQLString vnsInputFilePath = etrading::decorateFilename( vnsPath, "", "tryAqSwapObjectCreateFromSchedule_inputs.csv" );
            const ReadDataFile::Load vnsInputFile( vnsInputFilePath );
                
            // Recreate the Variable Notional Swap
            std::string     vnsName                 = vnsInputFile["swapName"];
            std::string     vnsFixedScheduleName    = vnsInputFile["schedule1Name"];
            std::string     vnsFloatScheduleName    = vnsInputFile["schedule2Name"];
            AQLStringMatrix    vnsFixedScheduleParams  = vnsInputFile["leg1LVB"];
            AQLStringMatrix    vnsFloatScheduleParams  = vnsInputFile["leg2LVB"];
            bool            validateKeys            = vnsInputFile["validateKeys"];
	            
            std::string     loadVNS = validation::tryAqSwapObjectCreateFromSchedule( vnsName,
                                                                                      vnsFixedScheduleName,
                                                                                      vnsFloatScheduleName,
                                                                                      vnsFixedScheduleParams,
                                                                                      vnsFloatScheduleParams,
                                                                                      AQLStringMatrix(), false, 
                                                                                      validateKeys );


            // 3. Get the Swap PV
            AQLString vnsPVInputFilePath = etrading::decorateFilename( vnsPath, "", "tryAqSwapObjectPV_inputs.csv" );
            const ReadDataFile::Load vnsPVInputFile( vnsPVInputFilePath );

            // Get the PV Parameters
            std::string     vnsPVName   = vnsPVInputFile["swapName"];
			etrading::LabelValueBlock        vnsPVCurve  = etrading::fromStringToLVB(vnsPVInputFile["curveCollection"]);

            // Price the Variable Notional Swap
            double          vnsPVResult = validation::tryAqSwapObjectPV( vnsPVName, vnsPVCurve );



            // 4. Load the Expected Result and Compare
            const double tolerance = 0.01;  // Notional of trades is 10MM
			#if defined(GTEST32)
            CheckTestResultsAndRebaseOnRequest( vnsPVResult, TEST_DIR, vnsPath, "_tryAqSwapObjectPV_outputs_32bit", tolerance );
			#else
			CheckTestResultsAndRebaseOnRequest( vnsPVResult, TEST_DIR, vnsPath, "_tryAqSwapObjectPV_outputs_64bit", tolerance );
			#endif
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
        
    

    TEST_F( TestAQObjSwapEURVariableNotionalSwaps, SNAPSHOT_CheckVariableNotionalSwapParRate )
    {
        try
        {
            // 1. Load the fixed and float Schedules
            AQLString fixedScheduleInputFilePath = etrading::decorateFilename( fixedSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
            AQLString floatScheduleInputFilePath = etrading::decorateFilename( floatSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
                
            const ReadDataFile::Load fixedScheduleInputFile( fixedScheduleInputFilePath );
            const ReadDataFile::Load floatScheduleInputFile( floatScheduleInputFilePath );
                
            // Recreate the Fixed Schedule
            std::string     fixedScheduleName       = fixedScheduleInputFile["scheduleName"];
            AQLStringMatrix    fixedScheduleProperties = fixedScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    fixedScheduleDates      = fixedScheduleInputFile["bespokeScheduleLVB"];
	        std::string     loadFixedSchedule       = validation::tryAqSwapObjectScheduleCreateBespoke( fixedScheduleName, fixedScheduleProperties, fixedScheduleDates );

            // Recreate the Float Schedule
            std::string     floatScheduleName       = floatScheduleInputFile["scheduleName"];
            AQLStringMatrix    floatScheduleProperties = floatScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    floatScheduleDates      = floatScheduleInputFile["bespokeScheduleLVB"];
	        std::string     loadFloatSchedule       = validation::tryAqSwapObjectScheduleCreateBespoke( floatScheduleName, floatScheduleProperties, floatScheduleDates );



            // 2. Load the Variable Notional Swap, which requires the fixed and float schedules to be loaded first
            AQLString vnsInputFilePath = etrading::decorateFilename( vnsPath, "", "tryAqSwapObjectCreateFromSchedule_inputs.csv" );
            const ReadDataFile::Load vnsInputFile( vnsInputFilePath );
                
            // Recreate the Variable Notional Swap
            std::string     vnsName                 = vnsInputFile["swapName"];
            std::string     vnsFixedScheduleName    = vnsInputFile["schedule1Name"];
            std::string     vnsFloatScheduleName    = vnsInputFile["schedule2Name"];
            AQLStringMatrix    vnsFixedScheduleParams  = vnsInputFile["leg1LVB"];
            AQLStringMatrix    vnsFloatScheduleParams  = vnsInputFile["leg2LVB"];
            bool            validateKeys            = vnsInputFile["validateKeys"];
	            
            std::string     loadVNS = validation::tryAqSwapObjectCreateFromSchedule( vnsName,
                                                                                      vnsFixedScheduleName,
                                                                                      vnsFloatScheduleName,
                                                                                      vnsFixedScheduleParams,
                                                                                      vnsFloatScheduleParams,
                                                                                      AQLStringMatrix(), 
                                                                                      false,
                                                                                      validateKeys );


            // 3. Get the Swap Par Rate
            AQLString vnsParRateInputFilePath = etrading::decorateFilename( vnsPath, "", "tryAqSwapObjectParRate_inputs.csv" );
            const ReadDataFile::Load vnsParRateInputFile( vnsParRateInputFilePath );

            // Get the PV Parameters
            std::string     vnsParRateName   = vnsParRateInputFile["swapName"];
			etrading::LabelValueBlock        vnsParRateCurve  = etrading::fromStringToLVB(vnsParRateInputFile["curveCollection"]);

            // Price the Variable Notional Swap
            double          vnsParRateResult = validation::tryAqSwapObjectParRate( vnsParRateName, vnsParRateCurve );



            // 4. Load the Expected Result and Compare
            const double tolerance = 1e-008;
            CheckTestResultsAndRebaseOnRequest( vnsParRateResult, TEST_DIR, vnsPath, "_tryAqSwapObjectParRate_outputs", tolerance );
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



    TEST_F( TestAQObjSwapEURVariableNotionalSwaps, SNAPSHOT_CheckVariableNotionalSwapPV01 )
    {
        try
        {
            // 1. Load the fixed and float Schedules
            AQLString fixedScheduleInputFilePath = etrading::decorateFilename( fixedSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
            AQLString floatScheduleInputFilePath = etrading::decorateFilename( floatSchedulePath, "", "tryAqSwapObjectScheduleCreateBespoke_inputs.csv" );
                
            const ReadDataFile::Load fixedScheduleInputFile( fixedScheduleInputFilePath );
            const ReadDataFile::Load floatScheduleInputFile( floatScheduleInputFilePath );
                
            // Recreate the Fixed Schedule
            std::string     fixedScheduleName       = fixedScheduleInputFile["scheduleName"];
            AQLStringMatrix    fixedScheduleProperties = fixedScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    fixedScheduleDates      = fixedScheduleInputFile["bespokeScheduleLVB"];
	        std::string     loadFixedSchedule       = validation::tryAqSwapObjectScheduleCreateBespoke( fixedScheduleName, fixedScheduleProperties, fixedScheduleDates );

            // Recreate the Float Schedule
            std::string     floatScheduleName       = floatScheduleInputFile["scheduleName"];
            AQLStringMatrix    floatScheduleProperties = floatScheduleInputFile["bespokeScheduleProperties"];
            AQLStringMatrix    floatScheduleDates      = floatScheduleInputFile["bespokeScheduleLVB"];
	        std::string     loadFloatSchedule       = validation::tryAqSwapObjectScheduleCreateBespoke( floatScheduleName, floatScheduleProperties, floatScheduleDates );



            // 2. Load the Variable Notional Swap, which requires the fixed and float schedules to be loaded first
            AQLString vnsInputFilePath = etrading::decorateFilename( vnsPath, "", "tryAqSwapObjectCreateFromSchedule_inputs.csv" );
            const ReadDataFile::Load vnsInputFile( vnsInputFilePath );
                
            // Recreate the Variable Notional Swap
            std::string     vnsName                 = vnsInputFile["swapName"];
            std::string     vnsFixedScheduleName    = vnsInputFile["schedule1Name"];
            std::string     vnsFloatScheduleName    = vnsInputFile["schedule2Name"];
            AQLStringMatrix    vnsFixedScheduleParams  = vnsInputFile["leg1LVB"];
            AQLStringMatrix    vnsFloatScheduleParams  = vnsInputFile["leg2LVB"];
            bool            validateKeys            = vnsInputFile["validateKeys"];
	            
            std::string     loadVNS = validation::tryAqSwapObjectCreateFromSchedule( vnsName,
                                                                                      vnsFixedScheduleName,
                                                                                      vnsFloatScheduleName,
                                                                                      vnsFixedScheduleParams,
                                                                                      vnsFloatScheduleParams,
                                                                                      AQLStringMatrix(), 
                                                                                      false,
                                                                                      validateKeys );


            // 3. Get the Swap PV01
            AQLString vnsPV01InputFilePath = etrading::decorateFilename( vnsPath, "", "tryAqSwapObjectPV01_inputs.csv" );
            const ReadDataFile::Load vnsPV01InputFile( vnsPV01InputFilePath );

            // Get the PV Parameters
            std::string     vnsPV01Name   = vnsPV01InputFile["swapName"];
			etrading::LabelValueBlock        vnsPV01Curve  = etrading::fromStringToLVB(vnsPV01InputFile["curveCollection"]);

            // Price the Variable Notional Swap
            double          vnsPV01Result = validation::tryAqSwapObjectPV01( vnsPV01Name, vnsPV01Curve );



            // 4. Load the Expected Result and Compare
            const double tolerance = 1e-008;
            CheckTestResultsAndRebaseOnRequest( vnsPV01Result, TEST_DIR, vnsPath, "_tryAqSwapObjectPV01_outputs", tolerance );
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
}

