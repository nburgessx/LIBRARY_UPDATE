// TestLWOSwapEUROvernightIndexedSwaps.cpp

#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"

#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "TryMeCurveTenorBasis.h"


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

#define TEST_DIR "ETrading/LWObjects/TestLWOSwapOvernightIndexedSwaps/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;	// solver tolerance is 1e-9

    const int minTests = 18;

    //
    // curve input files
    //
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char EURYC_1M[]			    = "";
    extern const char EURYC_3M[]			    = "";
    extern const char EURYC_6M[]			    = "";
    extern const char EURYC_12M[]			    = "";

    //
    // test call input and reference files
    //
    extern const char oisInputs[]	            = TEST_DIR "EUROIS"; // Test files require a suffix. Format :=  Basename + Index + '_tryMeLWOSwapCreateFromLegLVBs_inputs'

#if defined(GTEST32)
    extern const char oisOutputs[]	    = TEST_DIR "x86_EUROIS"; // Test files require a suffix. Format :=  Basename + Index + '_tryMeLWOSwapCreateFromLegLVBs_outputs'
#else
	extern const char oisOutputs[]	    = TEST_DIR "x64_EUROIS"; // Test files require a suffix. Format :=  Basename + Index + '_tryMeLWOSwapCreateFromLegLVBs_outputs'
#endif
}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_EUR_CURVE( TestLWOSwapEUROvernightIndexedSwaps, EURYC_OIS, EURYC_1M, EURYC_3M, EURYC_6M, EURYC_12M );


    //
    // Call Test Fixture
    //

    TEST_F( TestLWOSwapEUROvernightIndexedSwaps, UNIT_CreateOvernightIndexedSwaps )
    {
        int i = 0;
        try
        {
            for ( i = 1; ; ++i )
            {
                // Load the input file
                AQLString inputFilename = CreateDataFile::makeFilename( oisInputs, "_tryMeLWOSwapCreateFromLegLVBs_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load inputFile( inputFilename );

                // Read the input file into the tryMeLWOSwapCreateFromLegLVBs
                std::string swapName            = inputFile["swapName"];
                AQLStringMatrix leg1LVB            = inputFile["leg1LVB"];
                AQLStringMatrix leg2LVB            = inputFile["leg2LVB"];
                bool validateKeys               = inputFile["validateKeys"];
	            std::string localResult         = validation::tryMeLWOSwapCreateFromLegLVBs( swapName, leg1LVB, leg2LVB, AQLStringMatrix(), false, validateKeys );

                // Load the output file and the result                   
                AQLString outputFilename = CreateDataFile::makeFilename( oisInputs, "_tryMeLWOSwapCreateFromLegLVBs_outputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load outputFile( outputFilename );
                
                std::string outputFileResult = outputFile["output"];

                // Compare the local result with the output file result
                EXPECT_EQ( localResult, outputFileResult );
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GT( i, minTests );
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


    TEST_F( TestLWOSwapEUROvernightIndexedSwaps, SNAPSHOT_CheckOisPV )
    {
        int i = 0;
        try
        {
            for ( i = 1; ; ++i )
            {
                // 1. Create the Swap
                AQLString createSwapFilename = CreateDataFile::makeFilename( oisInputs, "_tryMeLWOSwapCreateFromLegLVBs_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load createSwapFile( createSwapFilename );

                    // Read the input file into the tryMeLWOSwapCreateFromLegLVBs
                    std::string swapName            = createSwapFile["swapName"];
                    AQLStringMatrix leg1LVB            = createSwapFile["leg1LVB"];
                    AQLStringMatrix leg2LVB            = createSwapFile["leg2LVB"];
                    bool validateKeys               = createSwapFile["validateKeys"];
	                std::string localResult         = validation::tryMeLWOSwapCreateFromLegLVBs( swapName, leg1LVB, leg2LVB, AQLStringMatrix(), false, validateKeys );
                
                // 2. Price the Swap
                 AQLString SwapPVFilename = CreateDataFile::makeFilename( oisInputs, "_tryMeLWOSwapPV_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load swapPVFile( SwapPVFilename );

                    // Read the input file into the tryMeLWOSwapPV
                    std::string swapObjectName      = swapPVFile["swapName"];

					double localSwapPV              = validation::tryMeLWOSwapPV( swapObjectName, etrading::fromStringToLVB("EURYC"));
                
                // 3. Load the Tenor Basis Spread Results File and Compare against the Local Spread                   
                const double tolerance = 1e-006;  // Notional of trades is 1MM
                CheckTestResultsAndRebaseOnRequest( localSwapPV, TEST_DIR, oisOutputs, "_tryMeLWOSwapPV_outputs", tolerance, i ); // Append the Index and Suffix to test file name
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GT( i, minTests );
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


    TEST_F( TestLWOSwapEUROvernightIndexedSwaps, SNAPSHOT_CheckOisParRate )
    {
        int i = 0;
        try
        {
            for ( i = 1; ; ++i )
            {
                // 1. Create the Swap
                AQLString createSwapFilename = CreateDataFile::makeFilename( oisInputs, "_tryMeLWOSwapCreateFromLegLVBs_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load createSwapFile( createSwapFilename );

                    // Read the input file into the tryMeLWOSwapCreateFromLegLVBs
                    std::string swapName            = createSwapFile["swapName"];
                    AQLStringMatrix leg1LVB            = createSwapFile["leg1LVB"];
                    AQLStringMatrix leg2LVB            = createSwapFile["leg2LVB"];
                    bool validateKeys               = createSwapFile["validateKeys"];
	                std::string localResult         = validation::tryMeLWOSwapCreateFromLegLVBs( swapName, leg1LVB, leg2LVB, AQLStringMatrix(), false, validateKeys );
                
                // 2. Get the Swap Par Rate
                 AQLString SwapParRateFilename = CreateDataFile::makeFilename( oisInputs, "_tryMeLWOSwapParRate_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load swapParRateFile( SwapParRateFilename );

                    // Read the input file into the tryMeLWOSwapParRate
                    std::string swapObjectName      = swapParRateFile["swapName"];
                    double localSwapParRate         = validation::tryMeLWOSwapParRate( swapObjectName, etrading::fromStringToLVB("EURYC") );
                
                // 3. Load the Tenor Basis Spread Results File and Compare against the Local Spread                   
                const double tolerance = 1e-008;
                CheckTestResultsAndRebaseOnRequest( localSwapParRate, TEST_DIR, oisOutputs, "_tryMeLWOSwapParRate_outputs", tolerance, i ); // Append the Index and Suffix to test file name
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GT( i, minTests );
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


    TEST_F( TestLWOSwapEUROvernightIndexedSwaps, SNAPSHOT_CheckOisPV01 )
    {
        int i = 0;
        try
        {
            for ( i = 1; ; ++i )
            {
                // 1. Create the Swap
                AQLString createSwapFilename = CreateDataFile::makeFilename( oisInputs, "_tryMeLWOSwapCreateFromLegLVBs_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load createSwapFile( createSwapFilename );

                    // Read the input file into the tryMeLWOSwapCreateFromLegLVBs
                    std::string swapName            = createSwapFile["swapName"];
                    AQLStringMatrix leg1LVB            = createSwapFile["leg1LVB"];
                    AQLStringMatrix leg2LVB            = createSwapFile["leg2LVB"];
                    bool validateKeys               = createSwapFile["validateKeys"];
	                std::string localResult         = validation::tryMeLWOSwapCreateFromLegLVBs( swapName, leg1LVB, leg2LVB, AQLStringMatrix(), false, validateKeys );
                
                // 2. Price the Swap
                 AQLString SwapPV01Filename = CreateDataFile::makeFilename( oisInputs, "_tryMeLWOSwapPV01_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load swapPV01File( SwapPV01Filename );

                    // Read the input file into the tryMeLWOSwapPV01
                    std::string swapObjectName      = swapPV01File["swapName"];
                    double localSwapPV01            = validation::tryMeLWOSwapPV01( swapObjectName, etrading::fromStringToLVB("EURYC") );
                
                // 3. Load the Tenor Basis Spread Results File and Compare against the Local Spread                   
                const double tolerance = 1e-008;
                CheckTestResultsAndRebaseOnRequest( localSwapPV01, TEST_DIR, oisOutputs, "_tryMeLWOSwapPV01_outputs", tolerance, i ); // Append the Index and Suffix to test file name
            }
        }
        catch( const ReadDataFile::LoadError& )
        {
            EXPECT_GT( i, minTests );
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

