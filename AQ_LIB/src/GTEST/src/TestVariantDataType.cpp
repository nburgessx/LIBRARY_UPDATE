// TestVariantDataType.h


#include "Variant.h"

#include <gTest/gTest.h>


namespace google_test
{
    
    TEST( TestVariantDataType, UNIT_toVariantMatrixFromAQLStringMatrix_NoTranspose )
    {
        const AQLStringMatrix laStringMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        const etrading::VariantMatrix expectedVariantMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        // Call Underlying Function without transposing
        const etrading::VariantMatrix actualVariantMatrix = etrading::toVariantMatrixFromAQLStringMatrix( laStringMatrix, false ); // false = don't transpose

        // Dimension Check - Assume Rectangular Matrix i.e. not jagged
        ASSERT_EQ( actualVariantMatrix.size(), expectedVariantMatrix.size() );
        ASSERT_EQ( actualVariantMatrix[0].size(), expectedVariantMatrix[0].size() );

        // Check Results
        for ( size_t row = 0; row < actualVariantMatrix.size(); ++row )
        {
            for ( size_t col = 0; col < actualVariantMatrix[row].size(); ++col )
            {
                EXPECT_EQ( expectedVariantMatrix[row][col].getValueAsString(), actualVariantMatrix[row][col].getValueAsString() );
            }
        }
    }

    TEST( TestVariantDataType, UNIT_toVariantMatrixFromAQLStringMatrix_WithTranspose )
    {
        const AQLStringMatrix laStringMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        const etrading::VariantMatrix expectedVariantMatrix =
        {
            { "A", "D" },
            { "B", "E" },
            { "C", "F" },
        };

        // Call Underlying Function applying transpose
        const etrading::VariantMatrix actualVariantMatrix = etrading::toVariantMatrixFromAQLStringMatrix( laStringMatrix, true ); // true = apply transpose

        // Dimension Check - Assume Rectangular Matrix i.e. not jagged
        ASSERT_EQ( actualVariantMatrix.size(), expectedVariantMatrix.size() );
        ASSERT_EQ( actualVariantMatrix[0].size(), expectedVariantMatrix[0].size() );

        // Check Results
        for ( size_t row = 0; row < actualVariantMatrix.size(); ++row )
        {
            for ( size_t col = 0; col < actualVariantMatrix[row].size(); ++col )
            {
                EXPECT_EQ( expectedVariantMatrix[row][col].getValueAsString(), actualVariantMatrix[row][col].getValueAsString() );
            }
        }
    }

    TEST( TestVariantDataType, UNIT_toVariantMatrixFromStandardStringMatrix_NoTranspose )
    {
        const StandardStringMatrix standardStringMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        const etrading::VariantMatrix expectedVariantMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        // Call Underlying Function without transposing
        const etrading::VariantMatrix actualVariantMatrix = etrading::toVariantMatrixFromStandardStringMatrix( standardStringMatrix, false ); // false = don't transpose

        // Dimension Check - Assume Rectangular Matrix i.e. not jagged
        ASSERT_EQ( actualVariantMatrix.size(), expectedVariantMatrix.size() );
        ASSERT_EQ( actualVariantMatrix[0].size(), expectedVariantMatrix[0].size() );

        // Check Results
        for ( size_t row = 0; row < actualVariantMatrix.size(); ++row )
        {
            for ( size_t col = 0; col < actualVariantMatrix[row].size(); ++col )
            {
                EXPECT_EQ( expectedVariantMatrix[row][col].getValueAsString(), actualVariantMatrix[row][col].getValueAsString() );
            }
        }
    }

    TEST( TestVariantDataType, UNIT_toVariantMatrixFromStandardStringMatrix_WithTranspose )
    {
        const StandardStringMatrix standardStringMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        const etrading::VariantMatrix expectedVariantMatrix =
        {
            { "A", "D" },
            { "B", "E" },
            { "C", "F" },
        };

        // Call Underlying Function applying transpose
        const etrading::VariantMatrix actualVariantMatrix = etrading::toVariantMatrixFromStandardStringMatrix( standardStringMatrix, true ); // true = apply transpose

        // Dimension Check - Assume Rectangular Matrix i.e. not jagged
        ASSERT_EQ( actualVariantMatrix.size(), expectedVariantMatrix.size() );
        ASSERT_EQ( actualVariantMatrix[0].size(), expectedVariantMatrix[0].size() );

        // Check Results
        for ( size_t row = 0; row < actualVariantMatrix.size(); ++row )
        {
            for ( size_t col = 0; col < actualVariantMatrix[row].size(); ++col )
            {
                EXPECT_EQ( expectedVariantMatrix[row][col].getValueAsString(), actualVariantMatrix[row][col].getValueAsString() );
            }
        }
    }

    TEST( TestVariantDataType, UNIT_toAQLStringMatrixFromVariantMatrix_NoTranspose )
    {
        const etrading::VariantMatrix variantMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        const AQLStringMatrix expectedMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        // Call Underlying Function without transposing
        const AQLStringMatrix actualMatrix = etrading::toAQLStringMatrixFromVariantMatrix( variantMatrix, false ); // false = don't transpose

        // Dimension Check - Assume Rectangular Matrix i.e. not jagged
        ASSERT_EQ( actualMatrix.size(), expectedMatrix.size() );
        ASSERT_EQ( actualMatrix[0].size(), expectedMatrix[0].size() );

        // Check Results
        for ( size_t row = 0; row < actualMatrix.size(); ++row )
        {
            for ( size_t col = 0; col < actualMatrix[row].size(); ++col )
            {
                EXPECT_EQ( expectedMatrix[row][col], actualMatrix[row][col] );
            }
        }
    }

    TEST( TestVariantDataType, UNIT_toAQLStringMatrixFromVariantMatrix_WithTranspose )
    {
        const etrading::VariantMatrix variantMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        const AQLStringMatrix expectedMatrix =
        {
            { "A", "D" },
            { "B", "E" },
            { "C", "F" },
        };

        // Call Underlying Function applying transpose
        const AQLStringMatrix actualMatrix = etrading::toAQLStringMatrixFromVariantMatrix( variantMatrix, true ); // true = apply transpose

        // Dimension Check - Assume Rectangular Matrix i.e. not jagged
        ASSERT_EQ( actualMatrix.size(), expectedMatrix.size() );
        ASSERT_EQ( actualMatrix[0].size(), expectedMatrix[0].size() );

        // Check Results
        for ( size_t row = 0; row < actualMatrix.size(); ++row )
        {
            for ( size_t col = 0; col < actualMatrix[row].size(); ++col )
            {
                EXPECT_EQ( expectedMatrix[row][col], actualMatrix[row][col] );
            }
        }
    }

    TEST( TestVariantDataType, UNIT_toStandardStringMatrixFromVariantMatrix_NoTranspose )
    {
        const etrading::VariantMatrix variantMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        const StandardStringMatrix expectedMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        // Call Underlying Function without transposing
        const StandardStringMatrix actualMatrix = etrading::toStandardStringMatrixFromVariantMatrix( variantMatrix, false ); // false = don't transpose

        // Dimension Check - Assume Rectangular Matrix i.e. not jagged
        ASSERT_EQ( actualMatrix.size(), expectedMatrix.size() );
        ASSERT_EQ( actualMatrix[0].size(), expectedMatrix[0].size() );

        // Check Results
        for ( size_t row = 0; row < actualMatrix.size(); ++row )
        {
            for ( size_t col = 0; col < actualMatrix[row].size(); ++col )
            {
                EXPECT_EQ( expectedMatrix[row][col], actualMatrix[row][col] );
            }
        }
    }

    TEST( TestVariantDataType, UNIT_toStandardStringMatrixFromVariantMatrix_WithTranspose )
    {
        const etrading::VariantMatrix variantMatrix =
        {
            { "A", "B", "C" },
            { "D", "E", "F" },
        };

        const StandardStringMatrix expectedMatrix =
        {
            { "A", "D" },
            { "B", "E" },
            { "C", "F" },
        };

        // Call Underlying Function applying transpose
        const StandardStringMatrix actualMatrix = etrading::toStandardStringMatrixFromVariantMatrix( variantMatrix, true ); // true = apply transpose

        // Dimension Check - Assume Rectangular Matrix i.e. not jagged
        ASSERT_EQ( actualMatrix.size(), expectedMatrix.size() );
        ASSERT_EQ( actualMatrix[0].size(), expectedMatrix[0].size() );

        // Check Results
        for ( size_t row = 0; row < actualMatrix.size(); ++row )
        {
            for ( size_t col = 0; col < actualMatrix[row].size(); ++col )
            {
                EXPECT_EQ( expectedMatrix[row][col], actualMatrix[row][col] );
            }
        }
    }

}

