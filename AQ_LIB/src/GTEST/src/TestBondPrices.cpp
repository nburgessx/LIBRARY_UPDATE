// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeGoogleTest.h"

#include "BondYields.h"
#include "BondFactory.h"
#include "ObjectUtilities.h"
#include "CreateFixedBond.h"
#include "tryAqObjectsBond.h"

#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/LWObjects/LWOBonds/BondPricing/"

namespace
{
    // Test Bond Input File(s)
    extern const std::string bondInputsFile1            = TEST_DIR "tryMeLWOBondCreate_inputs.csv";
	extern const std::string bondInputsFileParallel     = TEST_DIR "tryMeLWOBondCreate_inputs_parallel.csv";
    
    extern const std::string dirtyPriceInputs           = TEST_DIR "tryMeLWOBondDirtyPrice_inputs.csv";
    extern const std::string dirtyPriceOutputs          = TEST_DIR "tryMeLWOBondDirtyPrice_outputs.csv";
    
    extern const std::string accruedInterestInputs      = TEST_DIR "tryMeLWOBondAccruedInterest_inputs.csv";
    extern const std::string accruedInterestOutputs     = TEST_DIR "tryMeLWOBondAccruedInterest_outputs.csv";
    
    extern const std::string cleanPriceInputs           = TEST_DIR "tryMeLWOBondCleanPrice_inputs.csv";
    extern const std::string cleanPriceOutputs          = TEST_DIR "tryMeLWOBondCleanPrice_outputs.csv";

    extern const std::string yieldInputs                = TEST_DIR "tryMeLWOBondYield_inputs.csv";
    extern const std::string yieldOutputs               = TEST_DIR "tryMeLWOBondYield_outputs.csv";

	extern const std::string yieldInputsParallel        = TEST_DIR "tryMeLWOBondYield_inputs_parallel.csv";
    extern const std::string yieldOutputsParallel       = TEST_DIR "tryMeLWOBondYield_outputs_parallel.csv";

	extern const std::string dv01Inputs                 = TEST_DIR "tryMeLWOBondDV01_inputs.csv";
    extern const std::string dv01Outputs                = TEST_DIR "tryMeLWOBondDV01_outputs.csv";

	extern const std::string modifiedDurationInputs     = TEST_DIR "tryMeLWOBondModifiedDuration_inputs.csv";
    extern const std::string modifiedDurationOutputs    = TEST_DIR "tryMeLWOBondModifiedDuration_outputs.csv";
}

namespace google_test
{
    class TestBondFactory : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
        public:

        static etrading::BondPtr createBond( const std::string& inputFile )
        {
            // Build the Bond Object
            CreateFixedBond fixedBond;
            etrading::BondPtr myBondPtr = fixedBond.createFixedBondFromInputFile( inputFile );
            return myBondPtr;
        }
    };

    // Inherit the Create Bond Object Framework from TestBondFactory
    class TestBondPrices : public TestBondFactory {};


    TEST_F( TestBondPrices, UNIT_tryMeLWOBondDirtyPrice )
    {
        try
        {
            // Create the Bond
            etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );

            // Register the Bond in the LWO Cache
            etrading::registerToCache< etrading::Bond >( myBondPtr );

            // Load the Input and Output Files
            const ReadDataFile::Load tradeInputFile( dirtyPriceInputs.c_str() );

            // Read the Trade Inputs and Create the Swap
            const std::string bondObjectName                = myBondPtr->getBondObjectName(); 
            const std::vector< AQLDate > settlementDates     = tradeInputFile["settlementDates"];
            const std::vector< double > yields              = tradeInputFile["yields"];

            // Calculate the Actual Results
            std::vector< double > actualResults = validation::tryAqObjectsBondDirtyPrice( bondObjectName, settlementDates, yields );

            // Compare Results
            const double tolerance = 0.000000001;
            CheckTestResultsAndRebaseOnRequest( actualResults, TEST_DIR, AQLString( dirtyPriceOutputs.c_str() ), tolerance );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "#Error: Unable to open the Google Test File";
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


    TEST_F( TestBondPrices, UNIT_tryMeLWOBondCleanPrice )
    {
        try
        {
            // Create the Bond
            etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );

            // Register the Bond in the LWO Cache
            etrading::registerToCache< etrading::Bond >( myBondPtr );

            // Load the Input and Output Files
            const ReadDataFile::Load tradeInputFile( cleanPriceInputs.c_str() );

            // Read the Trade Inputs and Create the Swap
            const std::string bondObjectName                = myBondPtr->getBondObjectName(); 
            const std::vector< AQLDate > settlementDates     = tradeInputFile["settlementDates"];
            const std::vector< double > yields              = tradeInputFile["yields"];

            // Calculate the Actual Results
            std::vector< double > actualResults = validation::tryAqObjectsBondCleanPrice( bondObjectName, settlementDates, yields );

            // Compare Results
            const double tolerance = 0.000000001;
            CheckTestResultsAndRebaseOnRequest( actualResults, TEST_DIR, AQLString( cleanPriceOutputs.c_str() ), tolerance );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "#Error: Unable to open the Google Test File";
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

    TEST_F( TestBondPrices, UNIT_tryMeLWOBondAccruedInterest )
    {
        try
        {
            // Create the Bond
            etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );

            // Register the Bond in the LWO Cache
            etrading::registerToCache< etrading::Bond >( myBondPtr );

            // Load the Input and Output Files
            const ReadDataFile::Load tradeInputFile( accruedInterestInputs.c_str() );

            // Read the Trade Inputs and Create the Swap
            const std::string bondObjectName                = myBondPtr->getBondObjectName(); 
            const std::vector< AQLDate > settlementDates     = tradeInputFile["settlementDates"];

            // Calculate the Actual Results
            std::vector< double > actualResults = validation::tryAqObjectsBondAccruedInterest( bondObjectName, settlementDates );

            // Compare Results
            const double tolerance = 0.000000001;
            CheckTestResultsAndRebaseOnRequest( actualResults, TEST_DIR, AQLString( accruedInterestOutputs.c_str() ), tolerance );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "#Error: Unable to open the Google Test File";
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

    TEST_F( TestBondPrices, UNIT_tryMeLWOBondYield )
    {
        try
        {
            // Create the Bond
            etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );

            // Register the Bond in the LWO Cache
            etrading::registerToCache< etrading::Bond >( myBondPtr );

            // Load the Input and Output Files
            const ReadDataFile::Load tradeInputFile( yieldInputs.c_str() );

            // Read the Trade Inputs and Create the Swap
            const std::string bondObjectName                = myBondPtr->getBondObjectName(); 
            const std::vector< AQLDate > settlementDates     = tradeInputFile["settlementDates"];
            const std::vector< double > dirtyPrices         = tradeInputFile["prices"];

            // Calculate the Actual Results
            std::vector< double > actualResults = validation::tryAqObjectsBondYield( bondObjectName, settlementDates, dirtyPrices );

            // Compare Results
            const double tolerance = 0.000000001;
            CheckTestResultsAndRebaseOnRequest( actualResults, TEST_DIR, AQLString( yieldOutputs.c_str() ), tolerance );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "#Error: Unable to open the Google Test File";
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


	void runYieldTest(bool runInParallel )
	{
		try
        {
            // Create the Bond
            etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFileParallel );
			
            // Register the Bond in the LWO Cache
            etrading::registerToCache< etrading::Bond >( myBondPtr );

            // Load the Input and Output Files
            const ReadDataFile::Load tradeInputFile( yieldInputsParallel.c_str() );

            // Read the Trade Inputs and Create the Swap
            const std::string bondObjectName                = myBondPtr->getBondObjectName(); 
            const std::vector< AQLDate > settlementDates     = tradeInputFile["settlementDates"];
            const std::vector< double > dirtyPrices         = tradeInputFile["prices"];

            // Calculate the Actual Results
			std::string yieldCalculationType = "";
			std::vector< double > actualResults;

			// Run the test in a tight loop in an attempt to provoke trouble
			// A loop of 5 or more will usually uncover concurrency problems
			const int numIterations = 200;
			for (int i=0; i<numIterations; i++)
			{
				actualResults = validation::tryAqObjectsBondYield( bondObjectName, settlementDates, dirtyPrices, yieldCalculationType, runInParallel );
			}

            // Compare Results
            const double tolerance = 0.000000001;
            CheckTestResultsAndRebaseOnRequest( actualResults, TEST_DIR, AQLString( yieldOutputsParallel.c_str() ), tolerance );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "#Error: Unable to open the Google Test File";
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

	TEST_F( TestBondPrices, SNAPSHOT_tryMeLWOBondYield_Sequential )
    {
		runYieldTest( false /* run sequentially */ );
	}

    TEST_F( TestBondPrices, SNAPSHOT_tryMeLWOBondYield_Parallel )
    {
		runYieldTest( true /* run in parallel */ );
    }

	TEST_F( TestBondPrices, UNIT_tryMeLWOBondDV01Numerical )
    {
        try
        {
            // Create the Bond
            etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );

            // Register the Bond in the LWO Cache
            etrading::registerToCache< etrading::Bond >( myBondPtr );

            // Load the Input and Output Files
            const ReadDataFile::Load tradeInputFile( dv01Inputs.c_str() );

            // Read the Trade Inputs and Create the Swap
            const std::string bondObjectName                = myBondPtr->getBondObjectName(); 
            const std::vector< AQLDate > settlementDates     = tradeInputFile["settlementDates"];
            const std::vector< double > yields              = tradeInputFile["yields"];

            // Calculate the DV01 numerically
			double bumpSize = 0.01; // in bps
			AQLString bumpMode = "CENTRAL";
            std::vector< double > actualResults = validation::tryAqObjectsBondDV01Numerical( bondObjectName, settlementDates, yields,bumpSize, bumpMode );

            // Compare Results
            const double tolerance = 0.000001; // Use wider tolerance on Numerical DV01. Allows us to use the same output file for win32 and x64.
            CheckTestResultsAndRebaseOnRequest( actualResults, TEST_DIR, AQLString( dv01Outputs.c_str() ), tolerance );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "#Error: Unable to open the Google Test File";
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

	TEST_F( TestBondPrices, UNIT_tryMeLWOBondDV01 )
    {
        try
        {
            // Create the Bond
            etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );

            // Register the Bond in the LWO Cache
            etrading::registerToCache< etrading::Bond >( myBondPtr );

            // Load the Input and Output Files
            const ReadDataFile::Load tradeInputFile( dv01Inputs.c_str() );

            // Read the Trade Inputs and Create the Swap
            const std::string bondObjectName                = myBondPtr->getBondObjectName(); 
            const std::vector< AQLDate > settlementDates     = tradeInputFile["settlementDates"];
            const std::vector< double > yields              = tradeInputFile["yields"];

            // Calculate the Analytic DV01
            std::vector< double > actualResults = validation::tryAqObjectsBondDV01( bondObjectName, settlementDates, yields );

            // Compare Results
            const double tolerance = 0.000000001;
            CheckTestResultsAndRebaseOnRequest( actualResults, TEST_DIR, AQLString( dv01Outputs.c_str() ), tolerance );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "#Error: Unable to open the Google Test File";
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

		TEST_F( TestBondPrices, UNIT_tryMeLWOBondModifiedDuration )
    {
        try
        {
            // Create the Bond
            etrading::BondPtr myBondPtr = TestBondFactory::createBond( bondInputsFile1 );

            // Register the Bond in the LWO Cache
            etrading::registerToCache< etrading::Bond >( myBondPtr );

            // Load the Input and Output Files
            const ReadDataFile::Load tradeInputFile( dv01Inputs.c_str() );

            // Read the Trade Inputs and Create the Swap
            const std::string bondObjectName                = myBondPtr->getBondObjectName(); 
            const std::vector< AQLDate > settlementDates     = tradeInputFile["settlementDates"];
            const std::vector< double > yields              = tradeInputFile["yields"];
       
            // Calculate the Analytic DV01
            std::vector< double > actualResults = validation::tryAqObjectsBondModifiedDuration( bondObjectName, settlementDates, yields );

            // Compare Results
            const double tolerance = 0.000000001;
            CheckTestResultsAndRebaseOnRequest( actualResults, TEST_DIR, AQLString( modifiedDurationOutputs.c_str() ), tolerance );
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "#Error: Unable to open the Google Test File";
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

};