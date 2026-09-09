// TestUtilitiesData.cpp
#include <string>

// Includes: This Library
#include <tryAqToolAppend.h>
#include <tryAqToolClean.h>
#include <tryAqToolDataFilter.h>

// Include: Google Test Library
#include <gTest/gTest.h>


TEST( TestUtilitiesData, UNIT_VariantCompartorTests )
{
    // Declare Variant Parameters by Type
    // ----------------------------------

    // EMPTY
    etrading::Variant empty1 = etrading::Variant();
    etrading::Variant empty2 = etrading::Variant();

    // DATE
    etrading::Variant date1( AQLDate("20171002") );
    etrading::Variant date2( AQLDate("20171003") );

    // INTEGER
    etrading::Variant integer1( 1 );
    etrading::Variant integer2( 2 );

    // DOUBLE
    etrading::Variant double1( 1.1 );
    etrading::Variant double2( 2.2 );

    // STRING
    etrading::Variant string1( std::string("abc") );
    etrading::Variant string2( std::string("def") );

    // BOOL
    etrading::Variant bool1( false );
    etrading::Variant bool2( true );

    
    // Comparator Tests
    // ----------------------------------

    // EMPTY
    EXPECT_EQ( false, empty1 < empty2 );
    EXPECT_EQ( false, empty1 > empty2 );
    EXPECT_EQ( true, empty1 == empty2 );
    EXPECT_EQ( true, empty1 <= empty2 );
    EXPECT_EQ( true, empty1 >= empty2 );

    // DATE
    EXPECT_EQ( true, date1 < date2 );
    EXPECT_EQ( false, date1 > date2 );
    EXPECT_EQ( false, date1 == date2 );
    EXPECT_EQ( true, date1 <= date2 );
    EXPECT_EQ( false, date1 >= date2 );

    // INTEGER
    EXPECT_EQ( true, integer1 < integer2 );
    EXPECT_EQ( false, integer1 > integer2 );
    EXPECT_EQ( false, integer1 == integer2 );
    EXPECT_EQ( true, integer1 <= integer2 );
    EXPECT_EQ( false, integer1 >= integer2 );

    // DOUBLE
    EXPECT_EQ( true, double1 < double2 );
    EXPECT_EQ( false, double1 > double2 );
    EXPECT_EQ( false, double1 == double2 );
    EXPECT_EQ( true, double1 <= double2 );
    EXPECT_EQ( false, double1 >= double2 );

    // STRING
    EXPECT_EQ( true, string1 < string2 );
    EXPECT_EQ( false, string1 > string2 );
    EXPECT_EQ( false, string1 == string2 );
    EXPECT_EQ( true, string1 <= string2 );
    EXPECT_EQ( false, string1 >= string2 );

    // BOOL
    EXPECT_EQ( true, bool1 < bool2 );
    EXPECT_EQ( false, bool1 > bool2 );
    EXPECT_EQ( false, bool1 == bool2 );
    EXPECT_EQ( true, bool1 <= bool2 );
    EXPECT_EQ( false, bool1 >= bool2 );
    // ----------------------------------
}


TEST( TestUtilitiesData, UNIT_aqToolsAppendByRow )
{
    // Input Matrix 1
    etrading::VariantVector rowVector1;
    rowVector1.push_back( 1 );
    rowVector1.push_back( 2.2 );
    rowVector1.push_back( std::string("abc") );
    rowVector1.push_back( AQLDate( "20170929") );

    etrading::VariantMatrix matrix1;
    matrix1.push_back( rowVector1 );


    // Input Matrix 2
    etrading::VariantVector rowVector2;
    rowVector2.push_back( 1 );
    rowVector2.push_back( 2.2 );
    rowVector2.push_back( std::string("abc") );
    rowVector2.push_back( AQLDate( "20170929") );

    etrading::VariantMatrix matrix2;
    matrix2.push_back( rowVector2 );


    // Call the Append Function with Append by Row = TRUE
    etrading::VariantMatrix actual = validation::tryAqToolAppend( true, matrix1, matrix2 );


    // Expected Results
    etrading::VariantMatrix expected;
    expected.push_back( rowVector1 );
    expected.push_back( rowVector2 );


    // Check Matrix Sizes are not empty
    ASSERT_GT( actual.size(), 0U );
    ASSERT_GT( expected.size(), 0U );
    
    // Check Actual and Expected Results are the same size
    ASSERT_EQ( expected[0].size(), actual[0].size() );
    ASSERT_EQ( expected.size(), actual.size() );
    
    // Check the Test Matrix Results are Correct
    for ( size_t i = 0; i < actual.size(); ++i )
    {
        for ( size_t j = 0; j < actual[0].size(); ++j )
        {
            EXPECT_EQ( expected[i][j].getValueAsString(), actual[i][j].getValueAsString() );
        }
    }
}


TEST( TestUtilitiesData, UNIT_aqToolsAppendByColumn )
{
    // Input Matrix 1
    etrading::VariantVector rowVector1;
    rowVector1.push_back( 1 );
    rowVector1.push_back( 2.2 );
    rowVector1.push_back( std::string("abc") );
    rowVector1.push_back( AQLDate("20170929") );

    etrading::VariantMatrix matrix1;
    matrix1.push_back( rowVector1 );


    // Input Matrix 2
    etrading::VariantVector rowVector2;
    rowVector2.push_back( 1 );
    rowVector2.push_back( 2.2 );
    rowVector2.push_back( std::string("abc") );
    rowVector2.push_back( AQLDate("20170929") );

    etrading::VariantMatrix matrix2;
    matrix2.push_back( rowVector2 );


    // Call the Append Function with Append by Row = FALSE
    etrading::VariantMatrix actual = validation::tryAqToolAppend( false, matrix1, matrix2 );


    // Expected Results
    etrading::VariantMatrix expected;
    
    etrading::VariantVector outputVector;
    outputVector.push_back( 1 );
    outputVector.push_back( 2.2 );
    outputVector.push_back( std::string("abc") );
    outputVector.push_back( AQLDate("20170929") );
    outputVector.push_back( 1 );
    outputVector.push_back( 2.2 );
    outputVector.push_back( std::string("abc") );
    outputVector.push_back( AQLDate("20170929") );

    expected.push_back( outputVector );


    // Check Matrix Sizes are not empty
    ASSERT_GT( actual.size(), 0U );
    ASSERT_GT( expected.size(), 0U );
    
    // Check Actual and Expected Results are the same size
    ASSERT_EQ( expected[0].size(), actual[0].size() );
    ASSERT_EQ( expected.size(), actual.size() );
    
    // Check the Test Matrix Results are Correct
    for ( size_t i = 0; i < actual.size(); ++i )
    {
        for ( size_t j = 0; j < actual[0].size(); ++j )
        {
            EXPECT_EQ( expected[i][j].getValueAsString(), actual[i][j].getValueAsString() );
        }
    }
}


TEST( TestUtilitiesData, UNIT_aqToolsDataFilterByRow )
{
    // Input Vector
    etrading::VariantVector input;
    input.push_back( 1 );
    input.push_back( 1 );
    input.push_back( 2.2 );
    input.push_back( 2.2 );
    input.push_back( std::string("abc") );
    input.push_back( std::string("abc") );
    input.push_back( AQLDate("20170929") );
    input.push_back( AQLDate("20170929") );

    // Call the Data Filter function which removes duplicates
    etrading::VariantMatrix actual = validation::tryAqToolDataFilter( input, false ); // displayByRow = false

    // Expected Output Matrix
    etrading::VariantVector rowVector;
    rowVector.push_back( 1 );
    rowVector.push_back( 2.2 );
    rowVector.push_back( std::string("abc") );
    rowVector.push_back( AQLDate("20170929") );

    etrading::VariantMatrix expected;
    expected.push_back( rowVector );
    
    // Check Matrix Sizes are not empty
    ASSERT_GT( actual.size(), 0U );
    ASSERT_GT( expected.size(), 0U );
    
    // Check Actual and Expected Results are the same size
    ASSERT_EQ( expected[0].size(), actual[0].size() );
    ASSERT_EQ( expected.size(), actual.size() );
    
    // Check the Test Matrix Results are Correct
    for ( size_t i = 0; i < actual.size(); ++i )
    {
        for ( size_t j = 0; j < actual[0].size(); ++j )
        {
            EXPECT_EQ( expected[i][j].getValueAsString(), actual[i][j].getValueAsString() );
        }
    }

}


TEST( TestUtilitiesData, UNIT_aqToolsDataFilterByColumn )
{
    // Input Vector
    etrading::VariantVector input;
    input.push_back( 1 );
    input.push_back( 1 );
    input.push_back( 2.2 );
    input.push_back( 2.2 );
    input.push_back( std::string("abc") );
    input.push_back( std::string("abc") );
    input.push_back( AQLDate("20170929") );
    input.push_back( AQLDate("20170929") );

    // Call the Data Filter function which removes duplicates
    etrading::VariantMatrix actual = validation::tryAqToolDataFilter( input, true ); // displayByRow = true

    // Expected Output Matrix
    etrading::VariantMatrix expected;
    etrading::VariantVector rowVector1;
    etrading::VariantVector rowVector2;
    etrading::VariantVector rowVector3;
    etrading::VariantVector rowVector4;

    rowVector1.push_back( 1 );
    expected.push_back( rowVector1 );

    rowVector2.push_back( 2.2 );
    expected.push_back( rowVector2 );

    rowVector3.push_back( std::string("abc") );
    expected.push_back( rowVector3 );

    rowVector4.push_back( AQLDate("20170929") );
    expected.push_back( rowVector4 );
    
    // Check Matrix Sizes are not empty
    ASSERT_GT( actual.size(), 0U );
    ASSERT_GT( expected.size(), 0U );
    
    // Check Actual and Expected Results are the same size
    ASSERT_EQ( expected[0].size(), actual[0].size() );
    ASSERT_EQ( expected.size(), actual.size() );
    
    // Check the Test Matrix Results are Correct
    for ( size_t i = 0; i < actual.size(); ++i )
    {
        for ( size_t j = 0; j < actual[0].size(); ++j )
        {
            EXPECT_EQ( expected[i][j].getValueAsString(), actual[i][j].getValueAsString() );
        }
    }

}


TEST( TestUtilitiesData, UNIT_aqToolsCleanByRow )
{
    // Input Matrix
    etrading::VariantVector rowVector1;
    rowVector1.push_back( 1 );
    rowVector1.push_back( 1 );

    etrading::VariantVector rowVector2;
    rowVector2.push_back( 2.2 );
    rowVector2.push_back( 2.2 );

    etrading::VariantVector rowVector3;
    rowVector3.push_back( std::string("abc") );
    rowVector3.push_back( std::string("abc") );

    etrading::VariantVector rowVector4;
    rowVector4.push_back( AQLDate("20170929") );
    rowVector4.push_back( AQLDate("20170929") );

    etrading::VariantVector rowVector5;
    rowVector5.push_back( etrading::Variant() );     // Variant should be treated as blank input of type EMPTY
    rowVector5.push_back( etrading::Variant() );     // Variant should be treated as blank input of type EMPTY

    etrading::VariantVector rowVector6;
    rowVector6.push_back( std::string("#Error") );   // '#' prefix should be treated as an error
    rowVector6.push_back( std::string("#Error") );   // '#' prefix should be treated as an error

    etrading::VariantVector rowVector7;
    rowVector7.push_back( std::string("[Error]") );  // '[' prefix should be treated as an error
    rowVector7.push_back( std::string("[Error]") );  // '[' prefix should be treated as an error


    etrading::VariantMatrix inputMatrix;
    inputMatrix.push_back( rowVector1 );
    inputMatrix.push_back( rowVector2 );
    inputMatrix.push_back( rowVector3 );
    inputMatrix.push_back( rowVector4 );
    inputMatrix.push_back( rowVector5 );
    inputMatrix.push_back( rowVector6 );
    inputMatrix.push_back( rowVector7 );


    // Call the Clean Function with Clean by Row = TRUE
    etrading::VariantMatrix actual = validation::tryAqToolClean( inputMatrix, true );


    // Expected Results
    etrading::VariantVector outputVector1;
    outputVector1.push_back( 1 );
    outputVector1.push_back( 1 );

    etrading::VariantVector outputVector2;
    outputVector2.push_back( 2.2 );
    outputVector2.push_back( 2.2 );

    etrading::VariantVector outputVector3;
    outputVector3.push_back( std::string("abc") );
    outputVector3.push_back( std::string("abc") );

    etrading::VariantVector outputVector4;
    outputVector4.push_back( AQLDate("20170929") );
    outputVector4.push_back( AQLDate("20170929") );


    etrading::VariantMatrix expected;
    expected.push_back( outputVector1 );
    expected.push_back( outputVector2 );
    expected.push_back( outputVector3 );
    expected.push_back( outputVector4 );


    // Check Matrix Sizes are not empty
    ASSERT_GT( actual.size(), 0U );
    ASSERT_GT( expected.size(), 0U );
    
    // Check Actual and Expected Results are the same size
    ASSERT_EQ( expected[0].size(), actual[0].size() );
    ASSERT_EQ( expected.size(), actual.size() );
    
    // Check the Test Matrix Results are Correct
    for ( size_t i = 0; i < actual.size(); ++i )
    {
        for ( size_t j = 0; j < actual[0].size(); ++j )
        {
            EXPECT_EQ( expected[i][j].getValueAsString(), actual[i][j].getValueAsString() );
        }
    }
}


TEST( TestUtilitiesData, UNIT_aqToolsCleanByColumn )
{
    // Input Matrix
    etrading::VariantVector rowVector;
    rowVector.push_back( 1 );
    rowVector.push_back( 2.2 );
    rowVector.push_back( std::string("abc") );
    rowVector.push_back( AQLDate( "20170929") );
    rowVector.push_back( etrading::Variant() );     // Variant should be treated as blank input of type EMPTY
    rowVector.push_back( std::string("#Error") );   // '#' prefix should be treated as an error
    rowVector.push_back( std::string("[Error]") );  // '[' prefix should be treated as an error

    etrading::VariantMatrix inputMatrix;
    inputMatrix.push_back( rowVector );


    // Call the Clean Function with Clean by Row = FALSE
    etrading::VariantMatrix actual = validation::tryAqToolClean( inputMatrix, false );


    // Expected Results
    etrading::VariantVector resultsVector;
    resultsVector.push_back( 1 );
    resultsVector.push_back( 2.2 );
    resultsVector.push_back( std::string("abc") );
    resultsVector.push_back( AQLDate( "20170929") );

    etrading::VariantMatrix expected;
    expected.push_back( resultsVector );


    // Check Matrix Sizes are not empty
    ASSERT_GT( actual.size(), 0U );
    ASSERT_GT( expected.size(), 0U );
    
    // Check Actual and Expected Results are the same size
    ASSERT_EQ( expected[0].size(), actual[0].size() );
    ASSERT_EQ( expected.size(), actual.size() );
    
    // Check the Test Matrix Results are Correct
    for ( size_t i = 0; i < actual.size(); ++i )
    {
        for ( size_t j = 0; j < actual[0].size(); ++j )
        {
            EXPECT_EQ( expected[i][j].getValueAsString(), actual[i][j].getValueAsString() );
        }
    }
}

TEST(TestDataTrimmingMethods, UNIT_TrimStandardStringVector)
{
	StandardStringVector dirtyString = { "", "1", "", "2", "", "3", "" };
	StandardStringVector expectedResults = { "1", "2", "3" };
	StandardStringVector actualResults = validation::trimStandardStringVector( dirtyString );
	
	ASSERT_EQ( actualResults.size(), expectedResults.size() );
	for( size_t i = 0; i < expectedResults.size(); ++i )
	{
		EXPECT_EQ( expectedResults[i], actualResults[i] );
	}
}

TEST(TestDataTrimmingMethods, UNIT_TrimAQLStringVector)
{
	AQLStringVector dirtyString = { "", "1", "", "2", "", "3", "" };
	AQLStringVector expectedResults = { "1", "2", "3" };
	AQLStringVector actualResults = validation::trimAQLStringVector(dirtyString);
	
	ASSERT_EQ( actualResults.size(), expectedResults.size() );
	for ( size_t i = 0; i < expectedResults.size(); ++i )
	{
		EXPECT_EQ( expectedResults[i], actualResults[i] );
	}
}

TEST(TestDataTrimmingMethods, UNIT_TrimStandardStringMatrix)
{
	StandardStringMatrix dirtyString =
    {
        { "1", "2", "3" }, 
        { "", "", "" },
        { "4", "5", "6" },
        { "", "", "" }
    };
	StandardStringMatrix expectedResults =
    {
        { "1", "2", "3" }, 
        { "4", "5", "6" }
    };
	StandardStringMatrix actualResults = validation::trimStandardStringMatrix( dirtyString );
	
	ASSERT_EQ( actualResults.size(), expectedResults.size() );

	for( size_t i = 0; i < expectedResults.size(); ++i )
	{
        for (size_t j = 0; j < expectedResults[i].size(); ++j )
        {
		    EXPECT_EQ( expectedResults[i][j], actualResults[i][j] );
        }
	}
}

TEST(TestDataTrimmingMethods, UNIT_TrimAQLStringMatrix)
{
	AQLStringMatrix dirtyString =
    {
        { "1", "2", "3" }, 
        { "", "", "" },
        { "4", "5", "6" },
        { "", "", "" }
    };
	AQLStringMatrix expectedResults =
    {
        { "1", "2", "3" }, 
        { "4", "5", "6" }
    };
	AQLStringMatrix actualResults = validation::trimAQLStringMatrix( dirtyString );
	
	ASSERT_EQ( actualResults.size(), expectedResults.size() );

	for( size_t i = 0; i < expectedResults.size(); ++i )
	{
        for (size_t j = 0; j < expectedResults[i].size(); ++j )
        {
		    EXPECT_EQ( expectedResults[i][j], actualResults[i][j] );
        }
	}
}