// TestAQObjInfoBlock.cpp

/*
 * @brief			Method to Test the Utility Methods used to interact with the AQObj InfoBlock
 *                  The InfoBlock is a tuple containing 1) vector of column names 2) vector of column enum types
 *                  e.g. STRING, INTEGER, DOUBLE, VARIANT and a data matrix of type variant.
 * @Created:		20th September 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include <gTest/gTest.h>            // Google Test Library
#include "JSONInfoBlock.h"          // InfoBlock Helpers

namespace google_test
{
    TEST( TestInfoBlock, UNIT_CreateInfoBlock )
    {
        const etrading::VariantMatrix inDataBlock =
        {
            { 1, 2, 3, 4 },
            { 5, 6, 7, 8 }
        };
        
        const etrading::VariantMatrix inDataBlock_Transposed =
        {
            { 1, 5 },   // COL1
            { 2, 6 },   // COL2
            { 3, 7 },   // COL3
            { 4, 8 }    // COL4
        };

        // 1. Generate InfoBlock Tuple
        // ---------------------------------------------------
        etrading::JSONInfoBlockTuple actualInfoBlock = etrading::JSONInfoBlock::createInfoBlock( inDataBlock, true ); // EnforceVariantType = true



        // 2. Expected Result - Create the InfoBlock Tuple by Hand
        // ---------------------------------------------------
        etrading::ColumnNames dataColumnNames = { "COL_1", "COL_2", "COL_3", "COL_4" };
        etrading::ColumnDataTypes dataColumnTypes( dataColumnNames.size(), etrading::VARIANT_VALUE );
        etrading::JSONInfoBlockTuple expectedInfoBlock = std::make_tuple( dataColumnNames, dataColumnTypes, inDataBlock_Transposed ); // InfoBlock Data is Transposed when Created


        // 3. Check Generated and Expected InfoBlocks Match
        // ---------------------------------------------------

        // 3a) Check InfoBlock ColumnNames: 
        // Column Names: get<0>
        ASSERT_EQ( std::get<0>(expectedInfoBlock).size(), std::get<0>(actualInfoBlock).size() );            // Check Dimensions Match
        for( size_t i = 0; i < dataColumnNames.size(); ++i )
        {
            EXPECT_EQ( std::get<0>( expectedInfoBlock )[i], std::get<0>( actualInfoBlock )[i] );
        }

        // 3b) Check InfoBlock ColumnDataTypes
        // Column Data Types: get<1>
        ASSERT_EQ( std::get<1>(expectedInfoBlock).size(), std::get<1>(actualInfoBlock).size() );            // Check Dimensions Match
        for( size_t j = 0; j < dataColumnTypes.size(); ++j )
        {
            EXPECT_EQ( std::get<1>( expectedInfoBlock )[j], std::get<1>( actualInfoBlock )[j] );
        }

        // 3c) Check InfoBlock DataMatrix Dimensions Match
        // Variant Data Matrix: get<2>
        ASSERT_GT( std::get<2>(expectedInfoBlock).size(), size_t(0) );                                      // Access Violation Guard: Ensure Matrix not Empty
        ASSERT_GT( std::get<2>(actualInfoBlock).size(), size_t(0) );                                        // Access Violation Guard: Ensure Matrix not Empty
        
        // Check InfoBlock DataMatrix
        // Variant Data Matrix: get<2>
        ASSERT_EQ( std::get<2>(expectedInfoBlock).size(), std::get<2>(actualInfoBlock).size() );            // Check Variant Matrix Rows
        for( size_t row = 0; row < inDataBlock_Transposed.size(); ++row )
        {
            ASSERT_EQ( std::get<2>(expectedInfoBlock)[row].size(), std::get<2>(actualInfoBlock)[row].size() );      // Check Variant Matrix Cols
            for( size_t col = 0; col < inDataBlock_Transposed[row].size(); ++col )
            {
                EXPECT_EQ( std::get<2>(expectedInfoBlock)[row][col].getValueAsString(), std::get<2>(actualInfoBlock)[row][col].getValueAsString() ); // Variant Type therefore we getValueAsString()
            }
        }
    }


    TEST( TestInfoBlock, UNIT_TestJSONInfoBlockClass )
    {
        const etrading::VariantMatrix inDataBlock =
        {
            { 1, 2, 3, 4 },
            { 5, 6, 7, 8 }
        };
        
        const etrading::VariantMatrix inDataBlock_Transposed =
        {
            { 1, 5 },   // COL1
            { 2, 6 },   // COL2
            { 3, 7 },   // COL3
            { 4, 8 }    // COL4
        };

        // 1. Generate InfoBlock Tuple
        // ---------------------------------------------------
        etrading::JSONInfoBlock actualInfoBlock( inDataBlock );
        

        // 2. Expected Result - Create the InfoBlock Tuple by Hand
        // ---------------------------------------------------
        etrading::ColumnNames dataColumnNames = { "COL_1", "COL_2", "COL_3", "COL_4" };
        etrading::ColumnDataTypes dataColumnTypes( dataColumnNames.size(), etrading::VARIANT_VALUE );
        etrading::JSONInfoBlockTuple expectedInfoBlock = std::make_tuple( dataColumnNames, dataColumnTypes, inDataBlock_Transposed ); // InfoBlock Data is Transposed when Created


        // 3. Check Generated and Expected InfoBlocks Match
        // ---------------------------------------------------

        // 3a) Check InfoBlock ColumnNames: 
        // Column Names: get<0>
        ASSERT_EQ( std::get<0>(expectedInfoBlock).size(), actualInfoBlock.columnNames().size() );            // Check Dimensions Match
        for( size_t i = 0; i < dataColumnNames.size(); ++i )
        {
            EXPECT_EQ( std::get<0>( expectedInfoBlock )[i], actualInfoBlock.columnNames()[i] );
        }

        // 3b) Check InfoBlock ColumnDataTypes
        // Column Data Types: get<1>
        ASSERT_EQ( std::get<1>(expectedInfoBlock).size(), actualInfoBlock.columnDataTypes().size() );       // Check Dimensions Match
        for( size_t j = 0; j < dataColumnTypes.size(); ++j )
        {
            EXPECT_EQ( std::get<1>( expectedInfoBlock )[j], actualInfoBlock.columnDataTypes()[j] );
        }

        // 3c) Check InfoBlock DataMatrix Dimensions Match
        // Variant Data Matrix: get<2>
        ASSERT_GT( std::get<2>(expectedInfoBlock).size(), size_t(0) );                                      // Access Violation Guard: Ensure Matrix not Empty
        ASSERT_GT( actualInfoBlock.jsonFormattedData().size(), size_t(0) );                                 // Access Violation Guard: Ensure Matrix not Empty
        
        // Check InfoBlock DataMatrix
        // Variant Data Matrix: get<2>
        ASSERT_EQ( std::get<2>(expectedInfoBlock).size(), actualInfoBlock.jsonFormattedData().size() );     // Check Variant Matrix Rows
        for( size_t row = 0; row < inDataBlock_Transposed.size(); ++row )
        {
            ASSERT_EQ( std::get<2>(expectedInfoBlock)[row].size(), actualInfoBlock.jsonFormattedData()[row].size() );      // Check Variant Matrix Cols
            for( size_t col = 0; col < inDataBlock_Transposed[row].size(); ++col )
            {
                EXPECT_EQ( std::get<2>(expectedInfoBlock)[row][col].getValueAsString(), actualInfoBlock.jsonFormattedData()[row][col].getValueAsString() ); // Variant Type therefore we getValueAsString()
            }
        }
    }

}