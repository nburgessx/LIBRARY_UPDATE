// Include: Google Test Library
#include <gTest/gTest.h>


#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <boost/assign.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>

#include "TypeName.h"
#include "Variant.h"
#include "ETradingException.h"


#include "DateUtilities.h"
#include "ContainerUtilities.h"
#include "StatisticsUtilities.h"
#include "ContainerUtilities.h"
#include "UserUtilities.h"
#include "StatisticsUtilities.h"
#include "FileUtilities.h"
#include "ObjectUtilities.h"
#include "EnvironmentPool.h"
#include "DataUtilities.h"
#include "AQLString.h"
#include "TestHelperUtilities.h"    // GTEST_WARNING
#include "OMPThreadManager.h"       // OMP THREAD MACROS

using namespace etrading;

namespace google_test
{
    // Function to copy string using copy constructor, passsing by value
    size_t sizeAQLString( AQLString s )
    {
        AQLString newString(s);
        return newString.size();
    }

    TEST( TestAQLString, UNIT_Test_For_ThreadSafety )
    {
        try
        {
            // Bert is Ian Castleton's Alter-Ego and a very scary man!
            AQLString myString("BERT");
            size_t totalStringSize = 0;
        
            // OMP loop counters must be of type int and not size_t
            // Use reduction to share access to totalStringSize variable
            // #pragma omp parallel for reduction (+:totalStringSize) num_threads( omp_get_max_threads() ) 
            #pragma AQ_PARALLEL_FOR_WITH_REDUCTION_ADD( totalStringSize )
            for ( int i = 0; i < 100000; ++i )
            {
                totalStringSize += sizeAQLString( myString );
            }

            // If we reach here our test has passed.
            std::cout << "Congratulations! AQLString is Thread Safe! - Test Result: Total Test String Size: " << totalStringSize << std::endl;
        }
        catch(...)
        {
            GTEST_WARNING("AQLString Type is not thread safe")
        }
    }

    TEST( TestAQLString, UNIT_Test_StandardString_Constructor )
    {
        AQLString expectedString("BERT");

        std::string standardString("BERT");
        AQLString actualString( standardString );
        
        EXPECT_EQ( expectedString, actualString);
    }

    TEST( TestAQLString, UNIT_Test_CharString_Method )
    {
        std::string myStandardString("BERT");
        const char* charArrayFromStandardString = myStandardString.c_str();
        const std::string expectedString = charArrayFromStandardString;

        AQLString myAQLString("BERT");
        const char* charArrayFromAQLString = myAQLString.c_str();
        const std::string actualString = charArrayFromAQLString;

        EXPECT_EQ( expectedString, actualString);
    }

    TEST( TestAQLString, UNIT_Test_AQLString_StandardString_Operators )
    {
        // Comparator: AQLString vs StandardString
        AQLString myAQLString1("BERT");
        std::string myStandardString1("BERT");
        bool comparator = ( myAQLString1 == myStandardString1 );
        EXPECT_EQ( true, comparator );

        // Assignment: AQLString assigned from StandardString
        std::string myStandardString2("ERNIE");
        AQLString myAQLString2 = myStandardString2;
        EXPECT_EQ( myAQLString2, myStandardString2 );

        // Addition / Concatination of Mixed String Types i.e. AQLString + StandardString
        AQLString expectedAQLString2("BERTERNIE");
        AQLString actualAQLString2 = myAQLString1 + myStandardString2;
        EXPECT_EQ( expectedAQLString2, actualAQLString2 );
    }


	TEST( TestDataUtilitiesMacro, UNIT_AQ_TO_STRING_ALL_TYPES )
	{
		const float floatVarible	= float(1.123456789);
		const double doubleVarible	= double(1.123456789);

		std::string result1 = AQ_TO_STRING_FROM_SIZE_T( 1 );
		std::string result2 = AQ_TO_STRING_FROM_LONG( 2 );
		std::string result3 = AQ_TO_STRING_FROM_INT( 3 );
		std::string result4 = AQ_TO_STRING_FROM_FLOAT( floatVarible );
		std::string result5 = AQ_TO_STRING_FROM_DOUBLE( doubleVarible );
		
		std::string result6 = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( floatVarible, 3 );		// to 3 d.p.
		
		std::string result7 = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( doubleVarible, 3 );	// to 3 d.p.

		std::string result8 = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION(floatVarible, 6);		// to 6 d.p.

		std::string result9 = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( doubleVarible, 6 );	// to 6 d.p.

		EXPECT_EQ( "1",			result1 );
		EXPECT_EQ( "2",			result2 );
		EXPECT_EQ( "3",			result3 );
		EXPECT_EQ( "1.123457",	result4 );	// rounded to 6 d.p. (rounded-up in this case)
		EXPECT_EQ( "1.123457",	result5 );	// rounded to 6 d.p. (rounded-up in this case)
		EXPECT_EQ( "1.123",		result6 );
		EXPECT_EQ( "1.123",		result7 );
		EXPECT_EQ( "1.123457",  result8 );	// rounded to 6 d.p. (rounded-up in this case)
		EXPECT_EQ( "1.123457",  result9 );  // rounded to 6 d.p. (rounded-up in this case)
	}

	TEST(TestDataUtilitiesMacro, UNIT_AQ_TO_STRING_FROM_SIZE_T)
	{
		std::string result1  = AQ_TO_STRING_FROM_SIZE_T(1);
		std::string result2  = AQ_TO_STRING_FROM_SIZE_T(2);
		std::string result3  = AQ_TO_STRING_FROM_SIZE_T(3);
		std::string result4  = AQ_TO_STRING_FROM_SIZE_T(4);
		std::string result5  = AQ_TO_STRING_FROM_SIZE_T(5);
		std::string result6  = AQ_TO_STRING_FROM_SIZE_T(6);
		std::string result7  = AQ_TO_STRING_FROM_SIZE_T(7);
		std::string result8  = AQ_TO_STRING_FROM_SIZE_T(8);
		std::string result9  = AQ_TO_STRING_FROM_SIZE_T(9);
		std::string result10 = AQ_TO_STRING_FROM_SIZE_T(10);
		
		EXPECT_EQ("1", result1);
		EXPECT_EQ("2", result2);
		EXPECT_EQ("3", result3);
		EXPECT_EQ("4", result4);
		EXPECT_EQ("5", result5);
		EXPECT_EQ("6", result6);
		EXPECT_EQ("7", result7);
		EXPECT_EQ("8", result8);
		EXPECT_EQ("9", result9);
		EXPECT_EQ("10", result10);
	}

	TEST(TestDataUtilitiesMacro, UNIT_AQ_TO_STRING_FROM_LONG)
	{
		std::string result1  = AQ_TO_STRING_FROM_LONG(1);
		std::string result2  = AQ_TO_STRING_FROM_LONG(2);
		std::string result3  = AQ_TO_STRING_FROM_LONG(3);
		std::string result4  = AQ_TO_STRING_FROM_LONG(4);
		std::string result5  = AQ_TO_STRING_FROM_LONG(5);
		std::string result6  = AQ_TO_STRING_FROM_LONG(6);
		std::string result7  = AQ_TO_STRING_FROM_LONG(7);
		std::string result8  = AQ_TO_STRING_FROM_LONG(8);
		std::string result9  = AQ_TO_STRING_FROM_LONG(9);
		std::string result10 = AQ_TO_STRING_FROM_LONG(10);

		EXPECT_EQ("1", result1);
		EXPECT_EQ("2", result2);
		EXPECT_EQ("3", result3);
		EXPECT_EQ("4", result4);
		EXPECT_EQ("5", result5);
		EXPECT_EQ("6", result6);
		EXPECT_EQ("7", result7);
		EXPECT_EQ("8", result8);
		EXPECT_EQ("9", result9);
		EXPECT_EQ("10", result10);
	}

	TEST(TestDataUtilitiesMacro, UNIT_AQ_TO_STRING_FROM_INT)
	{
		std::string result1  = AQ_TO_STRING_FROM_INT(1);
		std::string result2  = AQ_TO_STRING_FROM_INT(2);
		std::string result3  = AQ_TO_STRING_FROM_INT(3);
		std::string result4  = AQ_TO_STRING_FROM_INT(4);
		std::string result5  = AQ_TO_STRING_FROM_INT(5);
		std::string result6  = AQ_TO_STRING_FROM_INT(6);
		std::string result7  = AQ_TO_STRING_FROM_INT(7);
		std::string result8  = AQ_TO_STRING_FROM_INT(8);
		std::string result9  = AQ_TO_STRING_FROM_INT(9);
		std::string result10 = AQ_TO_STRING_FROM_INT(10);

		EXPECT_EQ("1", result1);
		EXPECT_EQ("2", result2);
		EXPECT_EQ("3", result3);
		EXPECT_EQ("4", result4);
		EXPECT_EQ("5", result5);
		EXPECT_EQ("6", result6);
		EXPECT_EQ("7", result7);
		EXPECT_EQ("8", result8);
		EXPECT_EQ("9", result9);
		EXPECT_EQ("10", result10);
	}

	TEST(TestDataUtilitiesMacro, UNIT_AQ_TO_STRING_FROM_FLOAT)
	{
		std::string result1  = AQ_TO_STRING_FROM_FLOAT( float(1.123456789) );
		std::string result2  = AQ_TO_STRING_FROM_FLOAT( float(2.123456789) );
		std::string result3  = AQ_TO_STRING_FROM_FLOAT( float(3.123456789) );
		std::string result4  = AQ_TO_STRING_FROM_FLOAT( float(4.123456789) );
		std::string result5  = AQ_TO_STRING_FROM_FLOAT( float(5.123456789) );
		std::string result6  = AQ_TO_STRING_FROM_FLOAT( float(6.123456789) );
		std::string result7  = AQ_TO_STRING_FROM_FLOAT( float(7.123456789) );
		std::string result8  = AQ_TO_STRING_FROM_FLOAT( float(8.123456789) );
		std::string result9  = AQ_TO_STRING_FROM_FLOAT( float(9.123456789) );
		std::string result10 = AQ_TO_STRING_FROM_FLOAT( float(0.123456789) );

		EXPECT_EQ("1.123457", result1);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("2.123457", result2);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("3.123457", result3);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("4.123457", result4);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("5.123457", result5);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("6.123457", result6);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("7.123457", result7);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("8.123457", result8);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("9.123457", result9);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("0.123457", result10);	// to 6 d.p. by default, rounded-up in this case
	}

	TEST(TestDataUtilitiesMacro, UNIT_AQ_TO_STRING_FROM_DOUBLE)
	{
		std::string result1  = AQ_TO_STRING_FROM_DOUBLE( double(1.123456789) );
		std::string result2  = AQ_TO_STRING_FROM_DOUBLE( double(2.123456789) );
		std::string result3  = AQ_TO_STRING_FROM_DOUBLE( double(3.123456789) );
		std::string result4  = AQ_TO_STRING_FROM_DOUBLE( double(4.123456789) );
		std::string result5  = AQ_TO_STRING_FROM_DOUBLE( double(5.123456789) );
		std::string result6  = AQ_TO_STRING_FROM_DOUBLE( double(6.123456789) );
		std::string result7  = AQ_TO_STRING_FROM_DOUBLE( double(7.123456789) );
		std::string result8  = AQ_TO_STRING_FROM_DOUBLE( double(8.123456789) );
		std::string result9  = AQ_TO_STRING_FROM_DOUBLE( double(9.123456789) );
		std::string result10 = AQ_TO_STRING_FROM_DOUBLE( double(0.123456789) );

		EXPECT_EQ("1.123457", result1);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("2.123457", result2);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("3.123457", result3);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("4.123457", result4);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("5.123457", result5);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("6.123457", result6);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("7.123457", result7);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("8.123457", result8);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("9.123457", result9);		// to 6 d.p. by default, rounded-up in this case
		EXPECT_EQ("0.123457", result10);	// to 6 d.p. by default, rounded-up in this case
	}

	TEST(TestDataUtilitiesMacro, UNIT_AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION)
	{
		std::string result1  = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(1.123456789), 1 );
		std::string result2  = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(2.123456789), 2 );
		std::string result3  = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(3.123456789), 3 );
		std::string result4  = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(4.123456789), 4 );
		std::string result5  = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(5.123456789), 5 );
		std::string result6  = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(6.123456789), 0 );
		std::string result7  = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(7.123456789), 1 );
		std::string result8  = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(8.123456789), 2 );
		std::string result9  = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(9.123456789), 3 );
		std::string result10 = AQ_TO_STRING_FROM_FLOAT_WITH_PRECISION( float(0.123456789), 4 );

		EXPECT_EQ("1.1", result1);
		EXPECT_EQ("2.12", result2);
		EXPECT_EQ("3.123", result3);
		EXPECT_EQ("4.1235", result4);		// rounded-up
		EXPECT_EQ("5.12346", result5);		// rounded-up
		EXPECT_EQ("6", result6);
		EXPECT_EQ("7.1", result7);
		EXPECT_EQ("8.12", result8);
		EXPECT_EQ("9.123", result9);
		EXPECT_EQ("0.1235", result10);		// rounded-up
	}

	TEST(TestDataUtilitiesMacro, UNIT_AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION)
	{
		std::string result1  = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(1.123456789), 0 );
		std::string result2  = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(2.123456789), 1 );
		std::string result3  = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(3.123456789), 2 );
		std::string result4  = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(4.123456789), 3 );
		std::string result5  = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(5.123456789), 4 );
		std::string result6  = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(6.123456789), 5 );
		std::string result7  = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(7.123456789), 6 );
		std::string result8  = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(8.123456789), 7 );
		std::string result9  = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(9.123456789), 8 );
		std::string result10 = AQ_TO_STRING_FROM_DOUBLE_WITH_PRECISION( double(0.123456789), 9 );

		EXPECT_EQ("1", result1);
		EXPECT_EQ("2.1", result2);
		EXPECT_EQ("3.12", result3);
		EXPECT_EQ("4.123", result4);
		EXPECT_EQ("5.1235", result5);			// rounded-up
		EXPECT_EQ("6.12346", result6);			// rounded-up
		EXPECT_EQ("7.123457", result7);			// rounded-up
		EXPECT_EQ("8.1234568", result8);		// rounded-up
		EXPECT_EQ("9.12345679", result9);		// rounded-up
		EXPECT_EQ("0.123456789", result10);		// rounded-up
	}

    TEST( TestUtilities, UNIT_DateUtilities )
    {
        EXPECT_TRUE(canBeTenorString("ON"));
        EXPECT_TRUE(canBeTenorString("12M13D"));
        EXPECT_TRUE(canBeTenorString("69Y130D"));
        EXPECT_TRUE(canBeTenorString("169Y1230M2D"));
        EXPECT_FALSE(canBeTenorString("what"));
        EXPECT_FALSE(canBeTenorString(""));
        EXPECT_FALSE(canBeTenorString("4dd"));
        
        auto observedFromHere1 = tenorStringToYMDW( "ON" );
        EXPECT_EQ( std::get<0>( observedFromHere1 ), 0 );
        EXPECT_EQ( std::get<1>( observedFromHere1 ), 0 );
        EXPECT_EQ( std::get<2>( observedFromHere1 ), 1 );
        EXPECT_EQ( std::get<3>( observedFromHere1 ), 0 );
        auto observedFromHere2 = tenorStringToYMDW( "TN" );
        auto observedFromHere3 = tenorStringToYMDW( "12D" );
        auto observedFromHere4 = tenorStringToYMDW( "3M" );
        auto observedFromHere5 = tenorStringToYMDW( "33W" );
        EXPECT_EQ( std::get<0>( observedFromHere5 ), 0 );
        EXPECT_EQ( std::get<1>( observedFromHere5 ), 0 );
        EXPECT_EQ( std::get<2>( observedFromHere5 ), 0 );
        EXPECT_EQ( std::get<3>( observedFromHere5 ), 33 );
        int y, m, d, w;
        std::tie( y, m, d, w ) = tenorStringToYMDW( "10Y" );
        EXPECT_EQ( y, 10 );
        EXPECT_EQ( w, 0 );
        auto observedFromHere6 = tenorStringToYMDW( "10Y" );
        auto observedFromHere7 = tenorStringToYMDW( "20Y5M" );
        auto observedFromHere8 = tenorStringToYMDW( "80Y5M68D" );
        EXPECT_EQ( std::get<0>( observedFromHere8 ), 80 );
        EXPECT_EQ( std::get<1>( observedFromHere8 ), 5 );
        EXPECT_EQ( std::get<2>( observedFromHere8 ), 68 );
        EXPECT_EQ( std::get<3>( observedFromHere8 ), 0 );
        auto observedFromHere9 = tenorStringToYMDW( "82Y125M" );
        auto observedFromHere10 = tenorStringToYMDW( "83Y126M68D" );
        EXPECT_EQ( std::get<0>( observedFromHere10 ), 83 );
        EXPECT_EQ( std::get<1>( observedFromHere10 ), 126 );
        EXPECT_EQ( std::get<2>( observedFromHere10 ), 68 );
        EXPECT_EQ( std::get<3>( observedFromHere10 ), 0 );
        auto observedFromHere11 = tenorStringToYMDW( "81Y168D" );




    };

    TEST( TestUtilities, UNIT_ContainerUtilities )
    {
        std::vector<std::string> dataVec = boost::assign::list_of( "a" )( "b" )( "c" );
        EXPECT_EQ( getIndexOf( "b", dataVec ), 1 );
        std::vector<int> dataVec2 = boost::assign::list_of( 3123 )( 51 )( 454 );
        EXPECT_EQ( getIndexOf( 454, dataVec2 ), 2 );
        std::vector<boost::gregorian::date> dataVec3 =
            boost::assign::list_of( boost::gregorian::from_simple_string( "2014-09-04" ) )( boost::gregorian::from_simple_string( "2015-10-24" ) )( boost::gregorian::from_simple_string( "2016-06-29" ) );
        EXPECT_EQ( getIndexOf( boost::gregorian::from_undelimited_string( "20160629" ), dataVec3 ), 2 );
    };

};