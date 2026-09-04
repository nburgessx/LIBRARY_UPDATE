// TestLWOSwapEURTenorBasis.cpp

#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"

#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "tryAqSwapStubRate.h"

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

#define TEST_DIR "ETrading/LWObjects/TestLWOSwapTenorBasis/"

namespace
{
    // test tolerance
    const double tolerance = 1e-8;	// solver tolerance is 1e-9

    const int minTests = 16;

    //
    // curve input files
    //
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryMeCurveCalibrateOIS_inputs";
    extern const char EURYC_1M[]			    = TEST_DIR "EURYC_1M3M_tryMeCurveCalibrateBasis_inputs";
    extern const char EURYC_3M[]			    = TEST_DIR "EURYC_3M6M_tryMeCurveCalibrateBasis_inputs";
    extern const char EURYC_6M[]			    = TEST_DIR "EURYC_STD_tryMeCurveCalibrateSwap_inputs";
    extern const char EURYC_12M[]			    = TEST_DIR "EURYC_6M12MBasis_tryMeCurveCalibrateBasis_inputs";

    //
    // test call input and reference files
    //
    extern const char tenorBasisSwapInputs[]	= TEST_DIR "EUR3X6_"; // Test files require a suffix. Format :=  Basename + Index + '_tryMeLWOSwapCreateFromLegLVBs_inputs'
    extern const char tenorBasisSwapOutputs[]	= TEST_DIR "EUR3X6_"; // Test files require a suffix. Format :=  Basename + Index + '_tryMeLWOSwapCreateFromLegLVBs_outputs'

}

namespace google_test
{
    //
    // Construct depedent curves from curve source files by calling the curve constructor methods
    //
    // All tenor curve name MUST be defined for the ADD_EUR_CURVE_DEPENDENCIES macro to work.
    // If any curve is not in use and not defined, simply assign a "" to the curve name.
    //
    AQL_BUILD_EUR_CURVE( TestLWOSwapEURTenorBasis, EURYC_OIS, EURYC_1M, EURYC_3M, EURYC_6M, EURYC_12M );


    //
    // Call Test Fixture
    //

    TEST_F( TestLWOSwapEURTenorBasis, UNIT_CreateTenorBasisSwapUsingLegLabelValueBlocks )
    {
        int i = 0;
        try
        {
            for ( i = 1; ; ++i )
            {
                // Load the input file
                AQLString inputFilename = CreateDataFile::makeFilename( tenorBasisSwapInputs, "_tryMeLWOSwapCreateFromLegLVBs_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load inputFile( inputFilename );

                // Read the input file into the tryMeLWOSwapCreateFromLegLVBs
                std::string swapName            = inputFile["swapName"];
                AQLStringMatrix leg1LVB            = inputFile["leg1LVB"];
                AQLStringMatrix leg2LVB            = inputFile["leg2LVB"];
                bool validateKeys               = inputFile["validateKeys"];
	            std::string localResult         = validation::tryMeLWOSwapCreateFromLegLVBs( swapName, leg1LVB, leg2LVB, AQLStringMatrix(), false, validateKeys );

                // Load the output file and the result                   
                AQLString outputFilename = CreateDataFile::makeFilename( tenorBasisSwapOutputs, "_tryMeLWOSwapCreateFromLegLVBs_outputs", i ); // Append the Index and Suffix to test file name
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


    TEST_F( TestLWOSwapEURTenorBasis, SNAPSHOT_CheckTenorBasisPV )
    {
        int i = 0;
        try
        {
            for ( i = 1; ; ++i )
            {
                // 1. Create the Swap
                AQLString createSwapFilename = CreateDataFile::makeFilename( tenorBasisSwapInputs, "_tryMeLWOSwapCreateFromLegLVBs_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load createSwapFile( createSwapFilename );

                    // Read the input file into the tryMeLWOSwapCreateFromLegLVBs
                    std::string swapName            = createSwapFile["swapName"];
                    AQLStringMatrix leg1LVB            = createSwapFile["leg1LVB"];
                    AQLStringMatrix leg2LVB            = createSwapFile["leg2LVB"];
                    bool validateKeys               = createSwapFile["validateKeys"];
	                std::string localResult         = validation::tryMeLWOSwapCreateFromLegLVBs( swapName, leg1LVB, leg2LVB, AQLStringMatrix(), false, validateKeys );
                
                // 2. Price the Swap
                 AQLString SwapPVFilename = CreateDataFile::makeFilename( tenorBasisSwapInputs, "_tryMeLWOSwapPV_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load swapPVFile( SwapPVFilename );

                    // Read the input file into the tryMeLWOSwapPV
                    std::string swapObjectName      = swapPVFile["swapName"];
                    double localSwapPV              = validation::tryMeLWOSwapPV( swapObjectName, etrading::fromStringToLVB("EURYC") );
                
                // 3. Load the Tenor Basis Spread Results File and Compare against the Local Spread                   
                const double tolerance = 1e-02;  // Notional of test trades is 1MM
    
                #if defined(GTEST32)
                        CheckTestResultsAndRebaseOnRequest( localSwapPV, TEST_DIR, tenorBasisSwapOutputs, "_tryMeLWOSwapPV_outputsX86", tolerance, i ); // Append the Index and Suffix to test file name
                #else
                        CheckTestResultsAndRebaseOnRequest( localSwapPV, TEST_DIR, tenorBasisSwapOutputs, "_tryMeLWOSwapPV_outputsX64", tolerance, i ); // Append the Index and Suffix to test file name
                #endif
                
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


    TEST_F( TestLWOSwapEURTenorBasis, SNAPSHOT_CheckTenorBasisSpread )
    {
        int i = 0;
        try
        {
            for ( i = 1; ; ++i )
            {
                // 1. Create the Swap
                AQLString createSwapFilename = CreateDataFile::makeFilename( tenorBasisSwapInputs, "_tryMeLWOSwapCreateFromLegLVBs_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load createSwapFile( createSwapFilename );

                    // Read the input file into the tryMeLWOSwapCreateFromLegLVBs
                    std::string swapName            = createSwapFile["swapName"];
                    AQLStringMatrix leg1LVB            = createSwapFile["leg1LVB"];
                    AQLStringMatrix leg2LVB            = createSwapFile["leg2LVB"];
                    bool validateKeys               = createSwapFile["validateKeys"];
	                std::string localResult         = validation::tryMeLWOSwapCreateFromLegLVBs( swapName, leg1LVB, leg2LVB, AQLStringMatrix(), false, validateKeys );
                
                // 2. Price the Swap
                 AQLString SwapPVFilename = CreateDataFile::makeFilename( tenorBasisSwapInputs, "_tryMeLWOSwapSpread_inputs", i ); // Append the Index and Suffix to test file name
                const ReadDataFile::Load swapPVFile( SwapPVFilename );

                    // Read the input file into the tryMeLWOSwapSpread
                    std::string swapObjectName      = swapPVFile["swapName"];
                    double localBasisSpread         = validation::tryMeLWOSwapSpread( swapObjectName, etrading::fromStringToLVB("EURYC"));
                
                // 3. Load the Swap Spread Results File and Compare against the Local spread
                const double tolerance = 1e-007;

                #if defined(GTEST32)
                        CheckTestResultsAndRebaseOnRequest( localBasisSpread, TEST_DIR, tenorBasisSwapOutputs, "_tryMeLWOSwapSpread_outputsX86", tolerance, i ); // Append the Index and Suffix to test file name
                #else
                        CheckTestResultsAndRebaseOnRequest( localBasisSpread, TEST_DIR, tenorBasisSwapOutputs, "_tryMeLWOSwapSpread_outputsX64", tolerance, i ); // Append the Index and Suffix to test file name
                #endif
                
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

