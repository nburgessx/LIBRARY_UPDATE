// TestApiTryMeUtilityLVB.cpp

/*
 * @brief			Method to create a fixed bond from a Google Test Input File
 */

// Include: Google Test Library
#include <gTest/gTest.h>

// Other Includes
#include "tryAqToolsLVB.h"
#include <unordered_map>

namespace google_test
{
    TEST( TestLVBHelpers, UNIT_LVB_ConvertToAQLStringMatrix )
    {
        AQLStringMatrix aqStringMatrix 
        {
            {"A", "B", "C"},
            {"D", "E", "F"},
            {"G", "H", "I"}
        };

        StandardStringMatrix standardStringMatrix 
        {
            {"A", "B", "C"},
            {"D", "E", "F"},
            {"G", "H", "I"}
        };

        AQLStringMatrix actualStringMatrix = etrading::convertToAQLStringMatrix( standardStringMatrix );
        
        ASSERT_EQ( actualStringMatrix.size(), aqStringMatrix.size() );
        for ( size_t i = 0; i < actualStringMatrix.size(); ++i )
        {
            ASSERT_EQ( actualStringMatrix[i].size(), aqStringMatrix[i].size() );
            for( size_t j = 0; j < actualStringMatrix[i].size(); ++j )
            {
                EXPECT_EQ( aqStringMatrix[i][j], actualStringMatrix[i][j] );
            }
        }
    }

    TEST( TestLVBHelpers, UNIT_LVB_ConvertToStandardStringMatrix )
    {
        AQLStringMatrix aqStringMatrix 
        {
            {"A", "B", "C"},
            {"D", "E", "F"},
            {"G", "H", "I"}
        };

        StandardStringMatrix standardStringMatrix 
        {
            {"A", "B", "C"},
            {"D", "E", "F"},
            {"G", "H", "I"}
        };

        StandardStringMatrix actualStringMatrix = etrading::convertToStandardStringMatrix( aqStringMatrix );
        
        ASSERT_EQ( actualStringMatrix.size(), standardStringMatrix.size() );
        for ( size_t i = 0; i < actualStringMatrix.size(); ++i )
        {
            ASSERT_EQ( actualStringMatrix[i].size(), standardStringMatrix[i].size() );
            for( size_t j = 0; j < actualStringMatrix[i].size(); ++j )
            {
                EXPECT_EQ( standardStringMatrix[i][j], actualStringMatrix[i][j] );
            }
        }
    }

    TEST( TestLVBMethod, UNIT_LVB_toStandardStringMatrix )
    {
        StandardStringMatrix standardStringMatrix 
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"}
        };

        LabelValueBlock LVB( standardStringMatrix );

        // *** The LVB is an unordered map so the string matrix from the LVB may be in a different order to the original string matrix ***
        StandardStringMatrix actualStringMatrix = LVB.toStandardStringMatrix();

        // String Matrix Dimensions must be the same
        ASSERT_EQ( actualStringMatrix.size(), standardStringMatrix.size() );
        
        // Check the Output Matches LVB
        for ( size_t i = 0; i < actualStringMatrix.size(); ++i )
        {
            // String Matrix must have 2 columns
            ASSERT_EQ( actualStringMatrix[i].size(), 2 );
            
            std::string thisKey         = actualStringMatrix[i][0]; // Column 1
            std::string actualValue     = actualStringMatrix[i][1]; // Column 2
            std::string expectedValue   = LVB.getCompulsoryValueAsString( thisKey );
        
            EXPECT_EQ( expectedValue, actualValue );
        }

        // Check the Input Matches LVB
        for ( size_t i = 0; i < standardStringMatrix.size(); ++i )
        {
            // String Matrix must have 2 columns
            ASSERT_EQ( standardStringMatrix[i].size(), 2 );
            
            std::string thisKey         = standardStringMatrix[i][0]; // Column 1
            std::string actualValue     = standardStringMatrix[i][1]; // Column 2
            std::string expectedValue   = LVB.getCompulsoryValueAsString( thisKey );
        
            EXPECT_EQ( expectedValue, actualValue );
        }
    }

    TEST( TestLVBMethod, UNIT_LVB_toAQLStringMatrix )
    {
        AQLStringMatrix aqStringMatrix 
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"}
        };

        LabelValueBlock LVB( aqStringMatrix );

        // *** The LVB is an unordered map so the string matrix from the LVB may be in a different order to the original string matrix ***
        AQLStringMatrix actualStringMatrix = LVB.toAQLStringMatrix();

        // String Matrix Dimensions must be the same
        ASSERT_EQ( actualStringMatrix.size(), aqStringMatrix.size() );
        
        // Check the Output Matches LVB
        for ( size_t i = 0; i < actualStringMatrix.size(); ++i )
        {
            // String Matrix must have 2 columns
            ASSERT_EQ( actualStringMatrix[i].size(), 2 );
            
            AQLString thisKey         = actualStringMatrix[i][0]; // Column 1
            AQLString actualValue     = actualStringMatrix[i][1]; // Column 2
            AQLString expectedValue   = LVB.getCompulsoryValueAsAQLString( thisKey.c_str() );
        
            EXPECT_EQ( expectedValue, actualValue );
        }

        // Check the Input Matches LVB
        for ( size_t i = 0; i < aqStringMatrix.size(); ++i )
        {
            // String Matrix must have 2 columns
            ASSERT_EQ( aqStringMatrix[i].size(), 2 );
            
            AQLString thisKey         = aqStringMatrix[i][0]; // Column 1
            AQLString actualValue     = aqStringMatrix[i][1]; // Column 2
            AQLString expectedValue   = LVB.getCompulsoryValueAsAQLString( thisKey.c_str() );
        
            EXPECT_EQ( expectedValue, actualValue );
        }
    }

    TEST( TestLVBMethod, UNIT_LVB_combineAQLStringMatrices )
    {
        AQLStringMatrix matrix1
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"}
        };

        AQLStringMatrix matrix2
        {
            {"KEY4", "VALUE4"},
            {"KEY5", "VALUE5"},
            {"KEY6", "VALUE6"}
        };

        AQLStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"},
            {"KEY4", "VALUE4"},
            {"KEY5", "VALUE5"},
            {"KEY6", "VALUE6"}
        };

        AQLStringMatrix actualMatrix = etrading::combineAQLStringMatrices( matrix1, matrix2 );

        ASSERT_GT( actualMatrix.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), actualMatrix.size() );
        ASSERT_EQ( expectedMatrix[0].size(), actualMatrix[0].size() );

        for( size_t i = 0; i < actualMatrix.size(); ++i )
        {
            for( size_t j = 0; j < actualMatrix[i].size(); ++j )
            {
                EXPECT_EQ( expectedMatrix[i][j], actualMatrix[i][j] );
            }
        }
    }

    TEST( TestLVBMethod, UNIT_LVB_combineStandardStringMatrices )
    {
        StandardStringMatrix matrix1
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"}
        };

        StandardStringMatrix matrix2
        {
            {"KEY4", "VALUE4"},
            {"KEY5", "VALUE5"},
            {"KEY6", "VALUE6"}
        };

        StandardStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"},
            {"KEY4", "VALUE4"},
            {"KEY5", "VALUE5"},
            {"KEY6", "VALUE6"}
        };

        StandardStringMatrix actualMatrix = etrading::combineStandardStringMatrices( matrix1, matrix2 );

        ASSERT_GT( actualMatrix.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), actualMatrix.size() );
        ASSERT_EQ( expectedMatrix[0].size(), actualMatrix[0].size() );

        for( size_t i = 0; i < actualMatrix.size(); ++i )
        {
            for( size_t j = 0; j < actualMatrix[i].size(); ++j )
            {
                EXPECT_EQ( expectedMatrix[i][j], actualMatrix[i][j] );
            }
        }
    }

    TEST( TestLVBMethod, UNIT_LVB_searchAQLStringMatrix )
    {
        AQLStringMatrix searchMatrix 
        {
            {"KEY1",  "VALUE1"},
            {"KEY2",  "VALUE2"},
            {"KEY3",  "VALUE3"},
            {"KEY4",  "VALUE4"},
            {"KEY5",  "VALUE5"},
            {"KEY6",  "VALUE6"},
            {"KEY7",  "VALUE7"},
            {"KEY8",  "VALUE8"},
            {"KEY9",  "VALUE9"},
            {"KEY10", "VALUE10"}
        };

        AQLString actualValue;
        AQLString expectedValue;

        actualValue     = etrading::searchAQLStringMatrix( "KEY1", searchMatrix );
        expectedValue   = "VALUE1";
        EXPECT_EQ( expectedValue, actualValue );

        actualValue     = etrading::searchAQLStringMatrix( "KEY5", searchMatrix );
        expectedValue   = "VALUE5";
        EXPECT_EQ( expectedValue, actualValue );

        actualValue     = etrading::searchAQLStringMatrix( "KEY10", searchMatrix );
        expectedValue   = "VALUE10";
        EXPECT_EQ( expectedValue, actualValue );
    }

    TEST( TestLVBMethod, UNIT_LVB_searchAQLStringMatrix_ViaLVBLookupMethods )
    {
        AQLStringMatrix searchMatrix 
        {
            {"KEY1",  "VALUE1"},
            {"KEY2",  "VALUE2"},
            {"KEY3",  "VALUE3"},
            {"KEY4",  "VALUE4"},
            {"KEY5",  "VALUE5"},
            {"KEY6",  "VALUE6"},
            {"KEY7",  "VALUE7"},
            {"KEY8",  "VALUE8"},
            {"KEY9",  "VALUE9"},
            {"KEY10", "VALUE10"}
        };

        LabelValueBlock LVB( searchMatrix );

        AQLString actualValue;
        AQLString expectedValue;

        actualValue     = LVB.getCompulsoryValueAsAQLString( "KEY1", "SEARCH MATRIX" );
        expectedValue   = "VALUE1";
        EXPECT_EQ( expectedValue, actualValue );

        actualValue     = LVB.getCompulsoryValueAsAQLString( "KEY5", "SEARCH MATRIX" );
        expectedValue   = "VALUE5";
        EXPECT_EQ( expectedValue, actualValue );

        actualValue     = LVB.getCompulsoryValueAsAQLString( "KEY10", "SEARCH MATRIX" );
        expectedValue   = "VALUE10";
        EXPECT_EQ( expectedValue, actualValue );
    }

    TEST( TestLVBMethod, UNIT_LVB_searchStandardStringMatrix )
    {
        StandardStringMatrix searchMatrix 
        {
            {"KEY1",  "VALUE1"},
            {"KEY2",  "VALUE2"},
            {"KEY3",  "VALUE3"},
            {"KEY4",  "VALUE4"},
            {"KEY5",  "VALUE5"},
            {"KEY6",  "VALUE6"},
            {"KEY7",  "VALUE7"},
            {"KEY8",  "VALUE8"},
            {"KEY9",  "VALUE9"},
            {"KEY10", "VALUE10"}
        };

        StandardString actualValue;
        StandardString expectedValue;

        actualValue     = etrading::searchStandardStringMatrix( "KEY1", searchMatrix );
        expectedValue   = "VALUE1";
        EXPECT_EQ( expectedValue, actualValue );

        actualValue     = etrading::searchStandardStringMatrix( "KEY5", searchMatrix );
        expectedValue   = "VALUE5";
        EXPECT_EQ( expectedValue, actualValue );

        actualValue     = etrading::searchStandardStringMatrix( "KEY10", searchMatrix );
        expectedValue   = "VALUE10";
        EXPECT_EQ( expectedValue, actualValue );
    }

    TEST( TestLVBMethod, UNIT_LVB_searchStandardStringMatrix_ViaLVBLookupMethods )
    {
        StandardStringMatrix searchMatrix 
        {
            {"KEY1",  "VALUE1"},
            {"KEY2",  "VALUE2"},
            {"KEY3",  "VALUE3"},
            {"KEY4",  "VALUE4"},
            {"KEY5",  "VALUE5"},
            {"KEY6",  "VALUE6"},
            {"KEY7",  "VALUE7"},
            {"KEY8",  "VALUE8"},
            {"KEY9",  "VALUE9"},
            {"KEY10", "VALUE10"}
        };

        LabelValueBlock LVB( searchMatrix );

        AQLString actualValue;
        AQLString expectedValue;

        actualValue     = LVB.getCompulsoryValueAsAQLString( "KEY1", "SEARCH MATRIX" );
        expectedValue   = "VALUE1";
        EXPECT_EQ( expectedValue, actualValue );

        actualValue     = LVB.getCompulsoryValueAsAQLString( "KEY5", "SEARCH MATRIX" );
        expectedValue   = "VALUE5";
        EXPECT_EQ( expectedValue, actualValue );

        actualValue     = LVB.getCompulsoryValueAsAQLString( "KEY10", "SEARCH MATRIX" );
        expectedValue   = "VALUE10";
        EXPECT_EQ( expectedValue, actualValue );
    }

    TEST( TestLVBConstructors, UNIT_LVB_AppendLVB )
    {
        AQLStringMatrix aqStringMatrix1
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        AQLStringMatrix aqStringMatrix2
        {
            {"KEY3", "VALUE3"},
            {"KEY4", "VALUE4"}
        };

        AQLStringMatrix expectedStringMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"},
            {"KEY4", "VALUE4"}
        };

        // Create LVBs
        LabelValueBlock LVB1( aqStringMatrix1 );
        LabelValueBlock LVB2( aqStringMatrix2 );
        
        LabelValueBlock actualLVB( LVB1, LVB2 );
        LabelValueBlock expectedLVB( expectedStringMatrix );

        // Check for Size Match
        EXPECT_EQ( expectedLVB.size(), actualLVB.size() );
        
        // Check expected LVB size same as it's matrix size
        EXPECT_EQ( expectedLVB.size(), expectedStringMatrix.size() );

        for ( size_t i = 0; i < expectedLVB.size(); ++i )
        {
            AQLString key            = expectedStringMatrix[i][0];
            AQLString actualValue    = actualLVB.getCompulsoryValueAsAQLString( key.c_str(), "Actual LVB" );
            AQLString expectedValue  = expectedLVB.getCompulsoryValueAsAQLString( key.c_str(), "Expected LVB" );
            EXPECT_EQ( expectedValue, actualValue );
        }
    }

    TEST( TestLVBConstructors, UNIT_LVB_AppendLVB_CamelCaseKeys )
    {
        AQLStringMatrix aqStringMatrix1
        {
            {"CamelCaseKey1", "Value1"},
            {"CamelCaseKey2", "Value2"}
        };

        AQLStringMatrix aqStringMatrix2
        {
            {"CamelCaseKey3", "Value3"},
            {"CamelCaseKey4", "Value4"}
        };

        AQLStringMatrix expectedStringMatrix
        {
            {"CamelCaseKey1", "Value1"},
            {"CamelCaseKey2", "Value2"},
            {"CamelCaseKey3", "Value3"},
            {"CamelCaseKey4", "Value4"}
        };

        // Create LVBs
        LabelValueBlock LVB1( aqStringMatrix1, false );  // Make All Keys Upper Case = false
        LabelValueBlock LVB2( aqStringMatrix2 , false );  // Make All Keys Upper Case = false
        
        LabelValueBlock actualLVB( LVB1, LVB2, false ); // Make All Keys Upper Case = false
        LabelValueBlock expectedLVB( expectedStringMatrix, false ); // Make All Keys Uppercase = false

        // Check for Size Match
        EXPECT_EQ( expectedLVB.size(), actualLVB.size() );
        
        // Check expected LVB size same as it's matrix size
        EXPECT_EQ( expectedLVB.size(), expectedStringMatrix.size() );

        for ( size_t i = 0; i < expectedLVB.size(); ++i )
        {
            AQLString key            = expectedStringMatrix[i][0];
            AQLString actualValue    = actualLVB.getCompulsoryValueAsAQLString( key.c_str(), "Actual LVB" );
            AQLString expectedValue  = expectedLVB.getCompulsoryValueAsAQLString( key.c_str(), "Expected LVB" );
            EXPECT_EQ( expectedValue, actualValue );
        }
    }

    TEST( TestLVBConstructors, UNIT_CreateLVB_SingleKeyValue_UsingAQLString )
    {
        AQLString key    = "KEY1";
        AQLString value  = "VALUE1";

        AQLStringMatrix expectedStringMatrix
        {
            {"KEY1", "VALUE1"}
        };

        // Create LVBs
        LabelValueBlock actualLVB( key, value );
        LabelValueBlock expectedLVB( expectedStringMatrix );

        // Check for Size Match
        EXPECT_EQ( expectedLVB.size(), actualLVB.size() );
        
        // Check expected LVB size same as it's matrix size
        EXPECT_EQ( expectedLVB.size(), expectedStringMatrix.size() );

        AQLString actualValue    = actualLVB.getCompulsoryValueAsAQLString( key.c_str(), "Actual LVB" );
        AQLString expectedValue  = expectedLVB.getCompulsoryValueAsAQLString( key.c_str(), "Expected LVB" );
        EXPECT_EQ( expectedValue, actualValue );
    }

    TEST( TestLVBConstructors, UNIT_CreateLVB_SingleKeyValue_UsingAQLString_CamelCaseKey )
    {
        AQLString key    = "CamelCaseKey1";
        AQLString value  = "Value1";

        AQLStringMatrix expectedStringMatrix
        {
            {"CamelCaseKey1", "Value1"}
        };

        // Create LVBs
        LabelValueBlock actualLVB( key, value, false ); // Make All Keys Uppercase = false
        LabelValueBlock expectedLVB( expectedStringMatrix, false ); // Make All Keys Uppercase = false

        // Check for Size Match
        EXPECT_EQ( expectedLVB.size(), actualLVB.size() );
        
        // Check expected LVB size same as it's matrix size
        EXPECT_EQ( expectedLVB.size(), expectedStringMatrix.size() );

        AQLString actualValue    = actualLVB.getCompulsoryValueAsAQLString( key.c_str(), "Actual LVB" );
        AQLString expectedValue  = expectedLVB.getCompulsoryValueAsAQLString( key.c_str(), "Expected LVB" );
        EXPECT_EQ( expectedValue, actualValue );
    }

    TEST( TestLVBConstructors, UNIT_CreateLVB_SingleKeyValue_UsingStandardString )
    {
        StandardString key    = "KEY1";
        StandardString value  = "VALUE1";

        StandardStringMatrix expectedStringMatrix
        {
            {"KEY1", "VALUE1"}
        };

        // Create LVBs
        LabelValueBlock actualLVB( key, value );
        LabelValueBlock expectedLVB( expectedStringMatrix );

        // Check for Size Match
        EXPECT_EQ( expectedLVB.size(), actualLVB.size() );
        
        // Check expected LVB size same as it's matrix size
        EXPECT_EQ( expectedLVB.size(), expectedStringMatrix.size() );

        AQLString actualValue    = actualLVB.getCompulsoryValueAsString( key, "Actual LVB" );
        AQLString expectedValue  = expectedLVB.getCompulsoryValueAsAQLString( key, "Expected LVB" );
        EXPECT_EQ( expectedValue, actualValue );
    }

    TEST( TestLVBConstructors, UNIT_CreateLVB_SingleKeyValue_UsingStandardString_CamelCaseKey )
    {
        StandardString key    = "CamelCaseKey1";
        StandardString value  = "Value1";

        StandardStringMatrix expectedStringMatrix
        {
            {"CamelCaseKey1", "Value1"}
        };

        // Create LVBs
        LabelValueBlock actualLVB( key, value, false ); // Make All Keys Uppercase = false
        LabelValueBlock expectedLVB( expectedStringMatrix, false ); // Make All Keys Uppercase = false

        // Check for Size Match
        EXPECT_EQ( expectedLVB.size(), actualLVB.size() );
        
        // Check expected LVB size same as it's matrix size
        EXPECT_EQ( expectedLVB.size(), expectedStringMatrix.size() );

        AQLString actualValue    = actualLVB.getCompulsoryValueAsString( key, "Actual LVB" );
        AQLString expectedValue  = expectedLVB.getCompulsoryValueAsAQLString( key, "Expected LVB" );
        EXPECT_EQ( expectedValue, actualValue );
    }

    TEST( TestLVBConstructors, UNIT_CreateLVB_MultipleKeysValues_UsingAQLString )
    {
        AQLStringVector keys    = { "KEY1", "KEY2" };
        AQLStringVector values  = { "VALUE1", "VALUE2" };

        AQLStringMatrix expectedStringMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        // Create LVBs
        LabelValueBlock actualLVB( keys, values );
        LabelValueBlock expectedLVB( expectedStringMatrix );

        // Check for Size Match
        EXPECT_EQ( expectedLVB.size(), actualLVB.size() );
        
        // Check expected LVB size same as it's matrix size
        EXPECT_EQ( expectedLVB.size(), expectedStringMatrix.size() );

        for( size_t i = 0; i < actualLVB.size(); ++i )
        {
            AQLString actualValue    = actualLVB.getCompulsoryValueAsAQLString( keys[i].c_str(), "Actual LVB" );
            AQLString expectedValue  = expectedLVB.getCompulsoryValueAsAQLString( keys[i].c_str(), "Expected LVB" );
            EXPECT_EQ( expectedValue, actualValue );
        }
    }

    TEST( TestLVBConstructors, UNIT_CreateLVB_MultipleKeysValues_UsingStandardString )
    {
        StandardStringVector keys    = { "KEY1", "KEY2" };
        StandardStringVector values  = { "VALUE1", "VALUE2" };

        StandardStringMatrix expectedStringMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        // Create LVBs
        LabelValueBlock actualLVB( keys, values );
        LabelValueBlock expectedLVB( expectedStringMatrix );

        // Check for Size Match
        EXPECT_EQ( expectedLVB.size(), actualLVB.size() );
        
        // Check expected LVB size same as it's matrix size
        EXPECT_EQ( expectedLVB.size(), expectedStringMatrix.size() );

        for( size_t i = 0; i < actualLVB.size(); ++i )
        {
            AQLString actualValue    = actualLVB.getCompulsoryValueAsString( keys[i], "Actual LVB" );
            AQLString expectedValue  = expectedLVB.getCompulsoryValueAsString( keys[i], "Expected LVB" );
            EXPECT_EQ( expectedValue, actualValue );
        }
    }

    TEST( TestLVBConstructors, UNIT_CreateLVB_fromAQLStringMatrix )
    {
        AQLStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        // Test LVB Constructor
        LabelValueBlock LVB( expectedMatrix );
        AQLStringMatrix actualMatrix = LVB.toAQLStringMatrix();

        ASSERT_GT( actualMatrix.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), actualMatrix.size() );
        ASSERT_EQ( expectedMatrix[0].size(), actualMatrix[0].size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            AQLString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            AQLString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsAQLString( expectedKey.c_str() ) );
        }
    }

    TEST( TestLVBConstructors, UNIT_CreateLVB_fromStandardStringMatrix )
    {
        StandardStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        // Test LVB Constructor
        LabelValueBlock LVB( expectedMatrix );
        StandardStringMatrix actualMatrix = LVB.toStandardStringMatrix();

        ASSERT_GT( actualMatrix.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), actualMatrix.size() );
        ASSERT_EQ( expectedMatrix[0].size(), actualMatrix[0].size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            StandardString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            StandardString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsString( expectedKey ) );
        }
    }
    
    TEST( TestLVBConstructors, UNIT_CreateLVB_fromAQLStringKeyValue )
    {
        AQLStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"}
        };

        AQLString key    = "KEY1";
        AQLString value  = "VALUE1";
        
        // Test LVB Constructor
        LabelValueBlock LVB( key, value );
        AQLStringMatrix actualMatrix = LVB.toAQLStringMatrix();

        ASSERT_GT( actualMatrix.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), actualMatrix.size() );
        ASSERT_EQ( expectedMatrix[0].size(), actualMatrix[0].size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            AQLString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            AQLString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsAQLString( expectedKey.c_str() ) );
        }
    }
    
    TEST( TestLVBConstructors, UNIT_CreateLVB_fromStandardStringKeyValue )
    {
        StandardStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"}
        };

        StandardString key    = "KEY1";
        StandardString value  = "VALUE1";
        
        // Test LVB Constructor
        LabelValueBlock LVB( key, value );
        StandardStringMatrix actualMatrix = LVB.toStandardStringMatrix();

        ASSERT_GT( actualMatrix.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), actualMatrix.size() );
        ASSERT_EQ( expectedMatrix[0].size(), actualMatrix[0].size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            StandardString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            StandardString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsString( expectedKey ) );
        }
    }
    
    TEST( TestLVBConstructors, UNIT_CreateLVB_fromAQLStringMatrixKeysValues )
    {
        AQLStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        AQLStringVector keys     = { "KEY1", "KEY2" };
        AQLStringVector values   = { "VALUE1", "VALUE2" };
        
        // Test LVB Constructor
        LabelValueBlock LVB( keys, values );
        AQLStringMatrix actualMatrix = LVB.toAQLStringMatrix();

        ASSERT_GT( actualMatrix.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), actualMatrix.size() );
        ASSERT_EQ( expectedMatrix[0].size(), actualMatrix[0].size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            AQLString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            AQLString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsAQLString( expectedKey.c_str() ) );
        }
    }
    
    TEST( TestLVBConstructors, UNIT_CreateLVB_fromStandardStringMatrixKeysValues )
    {
        StandardStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        StandardStringVector keys     = { "KEY1", "KEY2" };
        StandardStringVector values   = { "VALUE1", "VALUE2" };
        
        // Test LVB Constructor
        LabelValueBlock LVB( keys, values );
        StandardStringMatrix actualMatrix = LVB.toStandardStringMatrix();

        ASSERT_GT( actualMatrix.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), actualMatrix.size() );
        ASSERT_EQ( expectedMatrix[0].size(), actualMatrix[0].size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            StandardString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            StandardString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsString( expectedKey ) );
        }
    }
    
    TEST( TestLVBConstructors, UNIT_CreateLVB_FromLVBs )
    {
        StandardStringMatrix originalMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        StandardStringMatrix appendMatrix
        {
            {"KEY3", "VALUE3"},
            {"KEY4", "VALUE4"}
        };

        StandardStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"},
            {"KEY4", "VALUE4"}
        };

        // Test LVB Constructor
        LabelValueBlock LVB1( originalMatrix );
        LabelValueBlock LVB2( appendMatrix );
        LabelValueBlock LVBresults( LVB1, LVB2 );

        ASSERT_GT( LVBresults.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), LVBresults.size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            StandardString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            StandardString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVBresults.getCompulsoryValueAsString( expectedKey ) );
        }
    }
    
    TEST( TestLVBConstructors, UNIT_CreateLVB_OriginalMatrix_AppendAQLStringKeyValue )
    {
        AQLStringMatrix originalMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        AQLString appendKey    = "KEY3";
        AQLString appendValue  = "VALUE3";

        AQLStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"}
        };

        // Test LVB Constructor
        LabelValueBlock LVB( originalMatrix, appendKey, appendValue );

        ASSERT_GT( LVB.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), LVB.size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            AQLString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            AQLString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsAQLString( expectedKey.c_str() ) );
        }
    }
    
    TEST( TestLVBConstructors, UNIT_CreateLVB_OriginalMatrix_AppendAQLStringKeysValues )
    {
        AQLStringMatrix originalMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        AQLStringVector appendKeys    = { "KEY3", "KEY4" };
        AQLStringVector appendValues  = { "VALUE3", "VALUE4" };

        AQLStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"},
            {"KEY4", "VALUE4"},
        };

        // Test LVB Constructor
        LabelValueBlock LVB( originalMatrix, appendKeys, appendValues );
        
        ASSERT_GT( LVB.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), LVB.size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            AQLString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            AQLString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsAQLString( expectedKey.c_str() ) );
        }
    }
    
    TEST( TestLVBConstructors, UNIT_CreateLVB_OriginalMatrix_AppendStandardStringKeyValue )
    {
        StandardStringMatrix originalMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        StandardString appendKey    = "KEY3";
        StandardString appendValue  = "VALUE3";

        StandardStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"}
        };

        // Test LVB Constructor
        LabelValueBlock LVB( originalMatrix, appendKey, appendValue );

        ASSERT_GT( LVB.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), LVB.size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            StandardString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            StandardString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsString( expectedKey ) );
        }
    }
    
    TEST( TestLVBConstructors, UNIT_CreateLVB_OriginalMatrix_AppendStandardStringKeysValues )
    {
        StandardStringMatrix originalMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"}
        };

        StandardStringVector appendKeys    = { "KEY3", "KEY4" };
        StandardStringVector appendValues  = { "VALUE3", "VALUE4" };

        StandardStringMatrix expectedMatrix
        {
            {"KEY1", "VALUE1"},
            {"KEY2", "VALUE2"},
            {"KEY3", "VALUE3"},
            {"KEY4", "VALUE4"},
        };

        // Test LVB Constructor
        LabelValueBlock LVB( originalMatrix, appendKeys, appendValues );

        ASSERT_GT( LVB.size(), size_t(0) );
        ASSERT_EQ( expectedMatrix.size(), LVB.size() );

        for ( size_t i = 0; i < expectedMatrix.size(); ++i )
        {
            StandardString expectedKey    = expectedMatrix[i][0]; // Column 1 Key
            StandardString expectedValue  = expectedMatrix[i][1]; // Column 2 Value
            EXPECT_EQ( expectedValue, LVB.getCompulsoryValueAsString( expectedKey ) );
        }
    }
    
    
    TEST( TryAqToolsLVB, UNIT_TestLVBCreate_2D )
    {
        const STDStringVector keys =   {"label1","label2","label3"};
        const STDStringVector values = {"value1","value2","value3"};
        
        STDStringMatrix LVB = validation::tryAqToolsLVBCreate( keys, values );

        // Validate LVB Dimensions
        ASSERT_EQ( LVB.size(), 3 );     // 3 Rows
        ASSERT_EQ( LVB[0].size(), 2 );  // 2 Columns

        // Test Labels in Column 0
        EXPECT_EQ( "label1", LVB[0][0] );
        EXPECT_EQ( "label2", LVB[1][0] );
        EXPECT_EQ( "label3", LVB[2][0] );
        
        // Test Values in Column 1
        EXPECT_EQ( "value1", LVB[0][1] );
        EXPECT_EQ( "value2", LVB[1][1] );
        EXPECT_EQ( "value3", LVB[2][1] );
    }

    TEST( TryAqToolsLVB, UNIT_TestLVBCreate_3D )
    {
        const STDStringVector keys =    {"label1","label2","label3"};
        const STDStringVector values1 = {"value11","value12","value13"};
        const STDStringVector values2 = {"value21","value22","value23"};
        
        STDStringMatrix LVB = validation::tryAqToolsLVBCreate( keys, values1, values2 );

        // Validate LVB Dimensions
        ASSERT_EQ( LVB.size(), 3 );     // 3 Rows
        ASSERT_EQ( LVB[0].size(), 3 );  // 3 Columns

        // Test Original Labels in Column 0
        EXPECT_EQ( "label1", LVB[0][0] );
        EXPECT_EQ( "label2", LVB[1][0] );
        EXPECT_EQ( "label3", LVB[2][0] );
        
        // Test Original Values in Column 1
        EXPECT_EQ( "value11", LVB[0][1] );
        EXPECT_EQ( "value12", LVB[1][1] );
        EXPECT_EQ( "value13", LVB[2][1] );

        // Test Original Values in Column 2
        EXPECT_EQ( "value21", LVB[0][2] );
        EXPECT_EQ( "value22", LVB[1][2] );
        EXPECT_EQ( "value23", LVB[2][2] );
    }

    TEST( TryAqToolsLVB, UNIT_TestLVBadd_2D )
    {
        const STDStringVector keys =   {"label1","label2","label3"};
        const STDStringVector values = {"value1","value2","value3"};
        
        STDStringMatrix LVB = validation::tryAqToolsLVBCreate( keys, values );

        // Validate LVB Dimensions
        ASSERT_EQ( LVB.size(), 3 );     // 3 Rows
        ASSERT_EQ( LVB[0].size(), 2 );  // 2 Columns

        // Test Original Labels in Column 0
        EXPECT_EQ( "label1", LVB[0][0] );
        EXPECT_EQ( "label2", LVB[1][0] );
        EXPECT_EQ( "label3", LVB[2][0] );
        
        // Test Original Values in Column 1
        EXPECT_EQ( "value1", LVB[0][1] );
        EXPECT_EQ( "value2", LVB[1][1] );
        EXPECT_EQ( "value3", LVB[2][1] );

        // Add some extra key value pairs
        validation::tryAqToolsLVBAdd( LVB, "label4", "value4" );
        validation::tryAqToolsLVBAdd( LVB, "label5", "value5" );
        validation::tryAqToolsLVBAdd( LVB, "label6", "value6" );

        // Validate NEW LVB Dimensions
        ASSERT_EQ( LVB.size(), 6 );     // 3 Rows
        ASSERT_EQ( LVB[0].size(), 2 );  // 2 Columns

        // Test Original Labels in Column 0
        EXPECT_EQ( "label4", LVB[3][0] );
        EXPECT_EQ( "label5", LVB[4][0] );
        EXPECT_EQ( "label6", LVB[5][0] );
        
        // Test Original Values in Column 1
        EXPECT_EQ( "value4", LVB[3][1] );
        EXPECT_EQ( "value5", LVB[4][1] );
        EXPECT_EQ( "value6", LVB[5][1] );
    }

    TEST( TryAqToolsLVB, UNIT_TestLVBadd_3D )
    {
        const STDStringVector keys =    {"label1","label2","label3"};
        const STDStringVector values1 = {"value11","value12","value13"};
        const STDStringVector values2 = {"value21","value22","value23"};
        
        STDStringMatrix LVB = validation::tryAqToolsLVBCreate( keys, values1, values2 );

        // Validate LVB Dimensions
        ASSERT_EQ( LVB.size(), 3 );     // 3 Rows
        ASSERT_EQ( LVB[0].size(), 3 );  // 3 Columns

        // Test Original Labels in Column 0
        EXPECT_EQ( "label1", LVB[0][0] );
        EXPECT_EQ( "label2", LVB[1][0] );
        EXPECT_EQ( "label3", LVB[2][0] );
        
        // Test Original Values in Column 1
        EXPECT_EQ( "value11", LVB[0][1] );
        EXPECT_EQ( "value12", LVB[1][1] );
        EXPECT_EQ( "value13", LVB[2][1] );

        // Test Original Values in Column 2
        EXPECT_EQ( "value21", LVB[0][2] );
        EXPECT_EQ( "value22", LVB[1][2] );
        EXPECT_EQ( "value23", LVB[2][2] );

        // Add some extra key value pairs
        validation::tryAqToolsLVBAdd( LVB, "label4", "value14", "value24" );
        validation::tryAqToolsLVBAdd( LVB, "label5", "value15", "value25" );
        validation::tryAqToolsLVBAdd( LVB, "label6", "value16", "value26" );
        
        // Validate NEW LVB Dimensions
        ASSERT_EQ( LVB.size(), 6 );     // 3 Rows
        ASSERT_EQ( LVB[0].size(), 3 );  // 2 Columns

        // Test Original Labels in Column 0
        EXPECT_EQ( "label4", LVB[3][0] );
        EXPECT_EQ( "label5", LVB[4][0] );
        EXPECT_EQ( "label6", LVB[5][0] );
        
        // Test Original Values in Column 1
        EXPECT_EQ( "value14", LVB[3][1] );
        EXPECT_EQ( "value15", LVB[4][1] );
        EXPECT_EQ( "value16", LVB[5][1] );

        // Test Original Values in Column 2
        EXPECT_EQ( "value24", LVB[3][2] );
        EXPECT_EQ( "value25", LVB[4][2] );
        EXPECT_EQ( "value26", LVB[5][2] );
    }
}