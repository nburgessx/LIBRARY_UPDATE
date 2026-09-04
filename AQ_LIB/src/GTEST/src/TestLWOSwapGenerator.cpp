// TestLWOSwapGenerator.cpp

#include "Dependency.h"
#include "ReadDataFile.h"
#include "AQLCoreError.h"
#include "InitializeETrading.h"
#include "tryAqObjectsSwapCreation.h"

#include <gTest/gTest.h>
using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Trades/TestTradeEURSwapGenerator/"

namespace
{
    //test tolerance
    const double tolerance = 1e-8;
}

namespace google_test
{
    
    std::string createSwapFromGenerator( std::string tradePrefix )
    {
        // FilePath
        const std::string filePath = TEST_DIR + tradePrefix + "_tryMeLWOSwapCreateFromGenerator_inputs.csv";
        
        // Load File
        const ReadDataFile::Load inputFile( filePath.c_str() );
        
        // Read Inputs
        std::string swapName                                = inputFile["swapName"];
        std::string swapGeneratorName                       = inputFile["swapGeneratorName"];
        AQLStringMatrix expressionLVB                          = inputFile["expressionLVB"];
        AQLStringMatrix swapPropertiesLVB                      = inputFile["swapPropertiesLVB"];
        bool isXccySwap                                     = inputFile["isXccySwap"];
        bool validateKeys                                   = inputFile["validateKeys"];
        
        // Create Swap
        std::string swapNameResult = validation::tryAqObjectsSwapCreateFromGenerator( swapName,
                                                                                      swapGeneratorName,
                                                                                      etrading::LabelValueBlock(expressionLVB),
                                                                                      etrading::LabelValueBlock(swapPropertiesLVB),
                                                                                      isXccySwap,
                                                                                      validateKeys );
        return swapNameResult;
    }


    std::string createSwapFromGeneratorRepeat2( std::string tradePrefix )
    {
        // FilePath
        const std::string filePath = TEST_DIR + tradePrefix + "_tryMeLWOSwapCreateFromGenerator_inputs.csv";
        
        // Load File
        const ReadDataFile::Load inputFile( filePath.c_str() );
        
        // Read Inputs
        std::string swapName                                = inputFile["swapName"];
        std::string swapGeneratorName                       = inputFile["swapGeneratorName"];
        AQLStringMatrix expressionLVB                          = inputFile["expressionLVB"];
        AQLStringMatrix swapPropertiesLVB                      = inputFile["swapPropertiesLVB"];
        bool isXccySwap                                     = inputFile["isXccySwap"];
        bool validateKeys                                   = inputFile["validateKeys"];
        
        // Create Swap
        std::string swapNameResult = validation::tryAqObjectsSwapCreateFromGenerator( swapName,
                                                                                      swapGeneratorName,
                                                                                      etrading::LabelValueBlock(expressionLVB),
                                                                                      etrading::LabelValueBlock(swapPropertiesLVB),
                                                                                      isXccySwap,
                                                                                      validateKeys );

        swapNameResult = validation::tryAqObjectsSwapCreateFromGenerator( swapName,
                                                                          swapGeneratorName,
                                                                          etrading::LabelValueBlock(expressionLVB),
                                                                          etrading::LabelValueBlock(swapPropertiesLVB),
                                                                          isXccySwap,
                                                                          validateKeys );
        return swapNameResult;
    }

    // Declare test creates the fixture class constructor and initializes AlgoQuantLib calling the start-up routine
    DECLARE_TEST_FIXTURE(TestLWOSwapGenerator);

    // Call Test Fixture - We use a fixture so the fixture class contructor can initialize / start-up the library to load the swap generator
    TEST_F( TestLWOSwapGenerator, UNIT_CreateSwapUsingGenerator )
    {
        try
        {
            std::string actual1    = createSwapFromGenerator("EUR_SWAP2Y_ASDATE");
            std::string actual2    = createSwapFromGenerator("EUR_SWAP3Y_ASDATE");
            std::string actual3    = createSwapFromGenerator("EUR_SWAP4Y_ASDATE");
            std::string actual4    = createSwapFromGenerator("EUR_SWAP5Y_ASDATE");
            std::string actual5    = createSwapFromGenerator("EUR_SWAP6Y_ASDATE");
            std::string actual6    = createSwapFromGenerator("EUR_SWAP7Y_ASDATE");
            std::string actual7    = createSwapFromGenerator("EUR_SWAP8Y_ASDATE");
            std::string actual8    = createSwapFromGenerator("EUR_SWAP9Y_ASDATE");
            std::string actual9    = createSwapFromGenerator("EUR_SWAP10Y_ASDATE");
            std::string actual10   = createSwapFromGenerator("EUR_SWAP11Y_ASDATE");
            std::string actual11   = createSwapFromGenerator("EUR_SWAP12Y_ASDATE");
            std::string actual12   = createSwapFromGenerator("EUR_SWAP15Y_ASDATE");
            std::string actual13   = createSwapFromGenerator("EUR_SWAP20Y_ASDATE");
            std::string actual14   = createSwapFromGenerator("EUR_SWAP25Y_ASDATE");
            std::string actual15   = createSwapFromGenerator("EUR_SWAP30Y_ASDATE");
            
            std::string expected1  = "EUR_SWAP2Y_ASDATE";
            std::string expected2  = "EUR_SWAP3Y_ASDATE";
            std::string expected3  = "EUR_SWAP4Y_ASDATE";
            std::string expected4  = "EUR_SWAP5Y_ASDATE";
            std::string expected5  = "EUR_SWAP6Y_ASDATE";
            std::string expected6  = "EUR_SWAP7Y_ASDATE";
            std::string expected7  = "EUR_SWAP8Y_ASDATE";
            std::string expected8  = "EUR_SWAP9Y_ASDATE";
            std::string expected9  = "EUR_SWAP10Y_ASDATE";
            std::string expected10 = "EUR_SWAP11Y_ASDATE";
            std::string expected11 = "EUR_SWAP12Y_ASDATE";
            std::string expected12 = "EUR_SWAP15Y_ASDATE";
            std::string expected13 = "EUR_SWAP20Y_ASDATE";
            std::string expected14 = "EUR_SWAP25Y_ASDATE";
            std::string expected15 = "EUR_SWAP30Y_ASDATE";

            // When the swap is created, if successful, the swapName is returned
            EXPECT_EQ(expected1,  actual1);
            EXPECT_EQ(expected2,  actual2);
            EXPECT_EQ(expected3,  actual3);
            EXPECT_EQ(expected4,  actual4);
            EXPECT_EQ(expected5,  actual5);
            EXPECT_EQ(expected6,  actual6);
            EXPECT_EQ(expected7,  actual7);
            EXPECT_EQ(expected8,  actual8);
            EXPECT_EQ(expected9,  actual9);
            EXPECT_EQ(expected10, actual10);
            EXPECT_EQ(expected11, actual11);
            EXPECT_EQ(expected12, actual12);
            EXPECT_EQ(expected13, actual13);
            EXPECT_EQ(expected14, actual14);
            EXPECT_EQ(expected15, actual15);
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "Test Error: TestLWOSwapGenerator.UNIT_CreateSwapUsingGenerator. Unable to read test input/output file(s).";
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

    // Call Test Fixture - We use a fixture so the fixture class contructor can initialize / start-up the library to load the swap generator
    // We create the swap twice to test the sub function which checks if the swap already exists
    TEST_F( TestLWOSwapGenerator, UNIT_CreateSwapUsingGenerator_Repeat2 )
    {
        try
        {
            std::string actual1    = createSwapFromGeneratorRepeat2("EUR_SWAP2Y_ASDATE");
            std::string actual2    = createSwapFromGeneratorRepeat2("EUR_SWAP3Y_ASDATE");
            std::string actual3    = createSwapFromGeneratorRepeat2("EUR_SWAP4Y_ASDATE");
            std::string actual4    = createSwapFromGeneratorRepeat2("EUR_SWAP5Y_ASDATE");
            std::string actual5    = createSwapFromGeneratorRepeat2("EUR_SWAP6Y_ASDATE");
            std::string actual6    = createSwapFromGeneratorRepeat2("EUR_SWAP7Y_ASDATE");
            std::string actual7    = createSwapFromGeneratorRepeat2("EUR_SWAP8Y_ASDATE");
            std::string actual8    = createSwapFromGeneratorRepeat2("EUR_SWAP9Y_ASDATE");
            std::string actual9    = createSwapFromGeneratorRepeat2("EUR_SWAP10Y_ASDATE");
            std::string actual10   = createSwapFromGeneratorRepeat2("EUR_SWAP11Y_ASDATE");
            std::string actual11   = createSwapFromGeneratorRepeat2("EUR_SWAP12Y_ASDATE");
            std::string actual12   = createSwapFromGeneratorRepeat2("EUR_SWAP15Y_ASDATE");
            std::string actual13   = createSwapFromGeneratorRepeat2("EUR_SWAP20Y_ASDATE");
            std::string actual14   = createSwapFromGeneratorRepeat2("EUR_SWAP25Y_ASDATE");
            std::string actual15   = createSwapFromGeneratorRepeat2("EUR_SWAP30Y_ASDATE");
            
            std::string expected1  = "EUR_SWAP2Y_ASDATE";
            std::string expected2  = "EUR_SWAP3Y_ASDATE";
            std::string expected3  = "EUR_SWAP4Y_ASDATE";
            std::string expected4  = "EUR_SWAP5Y_ASDATE";
            std::string expected5  = "EUR_SWAP6Y_ASDATE";
            std::string expected6  = "EUR_SWAP7Y_ASDATE";
            std::string expected7  = "EUR_SWAP8Y_ASDATE";
            std::string expected8  = "EUR_SWAP9Y_ASDATE";
            std::string expected9  = "EUR_SWAP10Y_ASDATE";
            std::string expected10 = "EUR_SWAP11Y_ASDATE";
            std::string expected11 = "EUR_SWAP12Y_ASDATE";
            std::string expected12 = "EUR_SWAP15Y_ASDATE";
            std::string expected13 = "EUR_SWAP20Y_ASDATE";
            std::string expected14 = "EUR_SWAP25Y_ASDATE";
            std::string expected15 = "EUR_SWAP30Y_ASDATE";

            // When the swap is created, if successful, the swapName is returned
            EXPECT_EQ(expected1,  actual1);
            EXPECT_EQ(expected2,  actual2);
            EXPECT_EQ(expected3,  actual3);
            EXPECT_EQ(expected4,  actual4);
            EXPECT_EQ(expected5,  actual5);
            EXPECT_EQ(expected6,  actual6);
            EXPECT_EQ(expected7,  actual7);
            EXPECT_EQ(expected8,  actual8);
            EXPECT_EQ(expected9,  actual9);
            EXPECT_EQ(expected10, actual10);
            EXPECT_EQ(expected11, actual11);
            EXPECT_EQ(expected12, actual12);
            EXPECT_EQ(expected13, actual13);
            EXPECT_EQ(expected14, actual14);
            EXPECT_EQ(expected15, actual15);
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "Test Error: TestLWOSwapGenerator.UNIT_CreateSwapUsingGenerator_Repeat2. Unable to read test input/output file(s).";
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

    // Call Test Fixture - We use a fixture so the fixture class contructor can initialize / start-up the library to load the swap generator
    TEST_F( TestLWOSwapGenerator, UNIT_CreateSwapUsingGeneratorWithMaturityAsTenorString )
    {
        try
        {
            std::string actual1    = createSwapFromGenerator("EUR_SWAP2Y");
            std::string actual2    = createSwapFromGenerator("EUR_SWAP3Y");
            std::string actual3    = createSwapFromGenerator("EUR_SWAP4Y");
            std::string actual4    = createSwapFromGenerator("EUR_SWAP5Y");
            std::string actual5    = createSwapFromGenerator("EUR_SWAP6Y");
            std::string actual6    = createSwapFromGenerator("EUR_SWAP7Y");
            std::string actual7    = createSwapFromGenerator("EUR_SWAP8Y");
            std::string actual8    = createSwapFromGenerator("EUR_SWAP9Y");
            std::string actual9    = createSwapFromGenerator("EUR_SWAP10Y");
            std::string actual10   = createSwapFromGenerator("EUR_SWAP11Y");
            std::string actual11   = createSwapFromGenerator("EUR_SWAP12Y");
            std::string actual12   = createSwapFromGenerator("EUR_SWAP15Y");
            std::string actual13   = createSwapFromGenerator("EUR_SWAP20Y");
            std::string actual14   = createSwapFromGenerator("EUR_SWAP25Y");
            std::string actual15   = createSwapFromGenerator("EUR_SWAP30Y");
            
            std::string expected1  = "EUR_SWAP2Y";
            std::string expected2  = "EUR_SWAP3Y";
            std::string expected3  = "EUR_SWAP4Y";
            std::string expected4  = "EUR_SWAP5Y";
            std::string expected5  = "EUR_SWAP6Y";
            std::string expected6  = "EUR_SWAP7Y";
            std::string expected7  = "EUR_SWAP8Y";
            std::string expected8  = "EUR_SWAP9Y";
            std::string expected9  = "EUR_SWAP10Y";
            std::string expected10 = "EUR_SWAP11Y";
            std::string expected11 = "EUR_SWAP12Y";
            std::string expected12 = "EUR_SWAP15Y";
            std::string expected13 = "EUR_SWAP20Y";
            std::string expected14 = "EUR_SWAP25Y";
            std::string expected15 = "EUR_SWAP30Y";

            // When the swap is created, if successful, the swapName is returned
            EXPECT_EQ(expected1,  actual1);
            EXPECT_EQ(expected2,  actual2);
            EXPECT_EQ(expected3,  actual3);
            EXPECT_EQ(expected4,  actual4);
            EXPECT_EQ(expected5,  actual5);
            EXPECT_EQ(expected6,  actual6);
            EXPECT_EQ(expected7,  actual7);
            EXPECT_EQ(expected8,  actual8);
            EXPECT_EQ(expected9,  actual9);
            EXPECT_EQ(expected10, actual10);
            EXPECT_EQ(expected11, actual11);
            EXPECT_EQ(expected12, actual12);
            EXPECT_EQ(expected13, actual13);
            EXPECT_EQ(expected14, actual14);
            EXPECT_EQ(expected15, actual15);
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "Test Error: TestLWOSwapGenerator.UNIT_CreateSwapUsingGeneratorWithMaturityAsTenorString. Unable to read test input/output file(s).";
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

    // Call Test Fixture - We use a fixture so the fixture class contructor can initialize / start-up the library to load the swap generator
    // We create the swap twice to test the sub function which checks if the swap already exists
    TEST_F( TestLWOSwapGenerator, UNIT_CreateSwapUsingGeneratorWithMaturityAsTenorString_Repeat2 )
    {
        try
        {
            std::string actual1    = createSwapFromGeneratorRepeat2("EUR_SWAP2Y");
            std::string actual2    = createSwapFromGeneratorRepeat2("EUR_SWAP3Y");
            std::string actual3    = createSwapFromGeneratorRepeat2("EUR_SWAP4Y");
            std::string actual4    = createSwapFromGeneratorRepeat2("EUR_SWAP5Y");
            std::string actual5    = createSwapFromGeneratorRepeat2("EUR_SWAP6Y");
            std::string actual6    = createSwapFromGeneratorRepeat2("EUR_SWAP7Y");
            std::string actual7    = createSwapFromGeneratorRepeat2("EUR_SWAP8Y");
            std::string actual8    = createSwapFromGeneratorRepeat2("EUR_SWAP9Y");
            std::string actual9    = createSwapFromGeneratorRepeat2("EUR_SWAP10Y");
            std::string actual10   = createSwapFromGeneratorRepeat2("EUR_SWAP11Y");
            std::string actual11   = createSwapFromGeneratorRepeat2("EUR_SWAP12Y");
            std::string actual12   = createSwapFromGeneratorRepeat2("EUR_SWAP15Y");
            std::string actual13   = createSwapFromGeneratorRepeat2("EUR_SWAP20Y");
            std::string actual14   = createSwapFromGeneratorRepeat2("EUR_SWAP25Y");
            std::string actual15   = createSwapFromGeneratorRepeat2("EUR_SWAP30Y");
            
            std::string expected1  = "EUR_SWAP2Y";
            std::string expected2  = "EUR_SWAP3Y";
            std::string expected3  = "EUR_SWAP4Y";
            std::string expected4  = "EUR_SWAP5Y";
            std::string expected5  = "EUR_SWAP6Y";
            std::string expected6  = "EUR_SWAP7Y";
            std::string expected7  = "EUR_SWAP8Y";
            std::string expected8  = "EUR_SWAP9Y";
            std::string expected9  = "EUR_SWAP10Y";
            std::string expected10 = "EUR_SWAP11Y";
            std::string expected11 = "EUR_SWAP12Y";
            std::string expected12 = "EUR_SWAP15Y";
            std::string expected13 = "EUR_SWAP20Y";
            std::string expected14 = "EUR_SWAP25Y";
            std::string expected15 = "EUR_SWAP30Y";

            // When the swap is created, if successful, the swapName is returned
            EXPECT_EQ(expected1,  actual1);
            EXPECT_EQ(expected2,  actual2);
            EXPECT_EQ(expected3,  actual3);
            EXPECT_EQ(expected4,  actual4);
            EXPECT_EQ(expected5,  actual5);
            EXPECT_EQ(expected6,  actual6);
            EXPECT_EQ(expected7,  actual7);
            EXPECT_EQ(expected8,  actual8);
            EXPECT_EQ(expected9,  actual9);
            EXPECT_EQ(expected10, actual10);
            EXPECT_EQ(expected11, actual11);
            EXPECT_EQ(expected12, actual12);
            EXPECT_EQ(expected13, actual13);
            EXPECT_EQ(expected14, actual14);
            EXPECT_EQ(expected15, actual15);
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "Test Error: TestLWOSwapGenerator.UNIT_CreateSwapUsingGeneratorWithMaturityAsTenorString_Repeat2. Unable to read test input/output file(s).";
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
}
