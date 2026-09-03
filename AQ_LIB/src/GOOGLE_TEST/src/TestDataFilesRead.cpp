// TestDataFilesRead.cpp

// Includes: Google Test Library
#include <gTest/gTest.h>

// Includes: Standard Library
#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include <boost/lexical_cast.hpp>

// Includes: This Library
#include <LAString.h>
#include <ReadDataFile.h>

using etrading::ReadDataFile;

namespace google_test
{
    //
    // SetUp Test Fixture
    //
    class TestDataFilesRead : public testing::Test
    {
    protected:
        TestDataFilesRead() : inputFile_( "DataFiles/TestDataFilesRead.csv" ) {};

        // Setup gets invoked immediately before a test starts
        virtual void SetUp()                {}

        // TearDown gets invoked immediately after a test finishes
        virtual void TearDown()             {}

        const ReadDataFile::Load& inputFile() const
        {
            return inputFile_;
        }
        ReadDataFile::Load& inputFile()
        {
            return inputFile_;
        }

    private:
        ReadDataFile::Load inputFile_;
    };

    //
    // Call Test Fixture
    //

    TEST_F( TestDataFilesRead, UNIT_TestBasics )
    {
        const ReadDataFile::Load& inputFile = TestDataFilesRead::inputFile();
        EXPECT_THROW( inputFile["non-existent key"], ReadDataFile::Exception );
        EXPECT_THROW( inputFile["non-existent key"], LACoreAppError );
        EXPECT_THROW( inputFile["non-existent key"], std::exception );

        EXPECT_EQ( false, inputFile.hasItem( "non-existent key" ) );
        EXPECT_EQ( true, inputFile.hasItem( "curveID" ) );

        EXPECT_EQ( false, inputFile.hasItem( "CurveID" ) );	// lookup is case-sensitive

        ReadDataFile x0( "x0" );
        ReadDataFile x1( x0 );
        EXPECT_EQ( x0(), x1() );

        EXPECT_EQ( x0, x1 );

        EXPECT_EQ( x1(), LAString( "x0" ) );
        EXPECT_EQ( LAString( "x0" ), x1() );

        ReadDataFile x2;
        x2 = x0;
        EXPECT_EQ( x0, x2 );

        ReadDataFile x3;

        ReadDataFile x4( "x4" );
        x4 = x3;
        EXPECT_EQ( x3(), x4() );
    }

    TEST_F( TestDataFilesRead, UNIT_TestScalars )
    {
        const ReadDataFile::Load& inputFile = TestDataFilesRead::inputFile();

        LAString curveID = inputFile["curveID"];
        EXPECT_EQ( curveID, "EUR" );

        std::cout << inputFile["curveID"] << std::endl;

        EXPECT_EQ( LAString( "OIS" ), inputFile["marketName"]() );
        EXPECT_EQ( LAString( "string with blanks" ), inputFile["stringWithBlanks"]() );
        EXPECT_EQ( LAString( "OIS:USDOIS:USDDISCOUNT:USDDF:DF" ), inputFile["curveNames"]() );
        EXPECT_EQ( "ACT/360", inputFile["dayCount"]() );
        EXPECT_EQ( "SPLINE", inputFile["interpolation"]() );

        EXPECT_EQ( false, inputFile["myFalseFlag"] );
        EXPECT_EQ( true, inputFile["myTrueFlag"] );
    }

    TEST_F( TestDataFilesRead, UNIT_TestOptionalItems )
    {
        ReadDataFile::Load& inputFile = TestDataFilesRead::inputFile();

        const LAString missing( "does not exist" );

        EXPECT_THROW( inputFile[missing], LACoreAppError );

        const double d0 = inputFile.getOptional( missing, 3.141 );
        EXPECT_EQ( 3.141, d0 );

        const double d1 = inputFile.getOptional( "Euler3Digs", 1.234 );
        EXPECT_EQ( d1, 2.718 );

        LAStringMatrix m = inputFile.getOptional( missing );
        EXPECT_EQ( 0, m.size() );

        const char* missing1 = "does not exist1";

        EXPECT_THROW( inputFile[missing1], LACoreAppError );

        ReadDataFile w = inputFile.getOptional( "absent", ReadDataFile( "hello" ) );

        EXPECT_EQ( "hello", w() );
    }

    TEST_F( TestDataFilesRead, UNIT_TestVectors )
    {
        const ReadDataFile::Load& inputFile = TestDataFilesRead::inputFile();

        const std::vector<double> terms = inputFile["terms"];
        EXPECT_EQ( terms[0], 0.25 );
        EXPECT_EQ( terms[1], 0.5 );
        EXPECT_EQ( terms[11], 50 );

        std::cout << "terms: " << inputFile["terms"] << std::endl;

        const std::vector<int> fibo = inputFile["fibo"];
        EXPECT_EQ( fibo[0], 1 );
        EXPECT_EQ( fibo[1], 1 );
        EXPECT_EQ( fibo[11], 144 );

        std::cout << "fibo: " << inputFile["fibo"] << std::endl;

        const std::vector<double> results = inputFile["results"];
        const double tolerance = 0.00001;
        EXPECT_NEAR( 0.99961137288460877, results[0], tolerance );
        EXPECT_NEAR( 0.32302550878348, results[10], tolerance );

        std::vector<LADate> mydates = inputFile["mydates"];
        EXPECT_EQ( LADate( "20150714" ), mydates[1] );
        EXPECT_THROW( inputFile["mydates"][100], LACoreAppError );

        std::cout << "mydates: " << inputFile["mydates"] << std::endl;

        std::vector<LAString> mystrings = inputFile["mystrings"];
        EXPECT_EQ( "lksdjf", mystrings[0] );

        std::cout << "mystrings: " << inputFile["mystrings"] << std::endl;

        std::vector<bool> myflags = inputFile["myflags"];

        std::cout << "myflags: " << inputFile["myflags"] << std::endl;

        EXPECT_EQ( true, myflags[0] );
        EXPECT_EQ( false, myflags[1] );
        EXPECT_EQ( true, myflags[2] );
        EXPECT_EQ( false, myflags[3] );

        ReadDataFile x0( mystrings );

        ReadDataFile x1( x0 );
        ReadDataFile x2( "junk" );
        x2 = x1;

        for ( std::size_t i = 0; i != mystrings.size(); ++i )
        {
            EXPECT_EQ( x0[i], x1[i] );
            EXPECT_EQ( x0[i], x2[i] );
        }
    }

    TEST_F( TestDataFilesRead, UNIT_TestAssociativeArrays )
    {
        const ReadDataFile::Load& inputFile = TestDataFilesRead::inputFile();

        ReadDataFile oisConv = inputFile["OISConventions"];

        std::cout << "oisConv: " << oisConv << std::endl;

        EXPECT_THROW( oisConv[ LAString("Calender") ] , std::exception );

        EXPECT_EQ( "NYB", oisConv[ LAString("Calendar") ] );

        EXPECT_EQ( 31, boost::lexical_cast<int>( oisConv[ LAString("EOMDay") ] ) );

        EXPECT_EQ( "DAYCOUNT", oisConv( 1, 0 ) );
        EXPECT_EQ( "ACT/360", oisConv( 1, 1 ) );

        EXPECT_EQ( "31", oisConv( "EOMDay", "value" ) );
        EXPECT_EQ( "31", oisConv( "EOMDay", 1 ) );
        EXPECT_EQ( "31", oisConv( 8, "value" ) );
        EXPECT_EQ( "31", oisConv( 8, 1 ) );
        EXPECT_EQ( "CALENDAR", oisConv( 0, 0 ) );

        EXPECT_THROW( oisConv( "xxx", 1 ), LACoreAppError );
        EXPECT_THROW( oisConv( "yyy", 17 ), LACoreAppError );
        EXPECT_THROW( oisConv( 42, 84 ), LACoreAppError );

        const LAStringMatrix& m = oisConv;

        EXPECT_EQ( "CALENDAR", m[0][0] );
        EXPECT_EQ( "NYB", m[0][1] );

        ReadDataFile x1( oisConv );
        ReadDataFile x2;
        x2 = x1;

        EXPECT_THROW( x1[ LAString( "Calender" ) ], std::exception );
        EXPECT_THROW( x2[ LAString( "Calender" ) ], std::exception );

        EXPECT_EQ( oisConv[ LAString( "Calendar" ) ], x1[ LAString( "Calendar" ) ] );
        EXPECT_EQ( oisConv[ LAString( "Calendar" ) ], x2[ LAString( "Calendar" ) ] );

    }

    TEST_F( TestDataFilesRead, UNIT_TestTables )
    {
        const ReadDataFile::Load& inputFile = TestDataFilesRead::inputFile();

        ReadDataFile oisRates = inputFile["OISRates"];

        std::cout << "oisRates: " << oisRates << std::endl;

        EXPECT_EQ( "0.00132", oisRates( "1M", "rate" ) );
        EXPECT_EQ( "TRUE", oisRates( "12M", "useQuote" ) );

        EXPECT_THROW( oisRates( "99M", "rate" ), LACoreAppError );
        EXPECT_THROW( oisRates( "9M", "xxxxx" ), LACoreAppError );

        const LAStringMatrix& rates = oisRates;

        EXPECT_EQ( 20, oisRates.rows() );
        EXPECT_EQ( 5, oisRates.cols() );
        EXPECT_EQ( 20, rates.size() );
        EXPECT_EQ( 5, rates[0].size() );

        EXPECT_EQ( "", rates[2][2] );

        EXPECT_EQ( "5M", oisRates( "5M", "term" ) );
        EXPECT_EQ( "0.00177", oisRates( "4M", 1 ) );
        EXPECT_EQ( "0.00177", oisRates( 4, "rate" ) );
        EXPECT_EQ( "0.00177", oisRates( 4, 1 ) );

        EXPECT_THROW( oisRates( "99M", 1 ), LACoreAppError );
        EXPECT_THROW( oisRates( "5M", 17 ), LACoreAppError );
        EXPECT_THROW( oisRates( 88, 44 ), LACoreAppError );

        EXPECT_THROW( std::vector<double> v = inputFile["OISRates"], ReadDataFile::Exception );
        EXPECT_THROW( std::vector<double> v = inputFile["OISRates"], LACoreAppError );

        ReadDataFile x0( oisRates );
        ReadDataFile x1( "x1" );
        x1 = x0;

        EXPECT_EQ( oisRates( 4, 1 ), x0( 4, 1 ) );
        EXPECT_EQ( oisRates( 4, 1 ), x1( 4, 1 ) );
    }

    TEST_F( TestDataFilesRead, UNIT_TestDuplicates )
    {
        const ReadDataFile::Load& inputFile = TestDataFilesRead::inputFile();

        ReadDataFile dupesArr = inputFile["DupesArr"];
        EXPECT_THROW( dupesArr[ LAString("aaa")], LACoreAppError );
        EXPECT_EQ( "AAA", dupesArr( 0, 0 ) );
        EXPECT_EQ( "111", dupesArr( 0, "value" ) );
        EXPECT_EQ( "999", dupesArr( 2, 1 ) );

        EXPECT_THROW( LAString aaa = dupesArr[ LAString("aaa")], LACoreAppError );

        ReadDataFile dupesTab = inputFile["DupesTab"];
        EXPECT_THROW( dupesTab( "aaa", "col1" ), LACoreAppError );
        EXPECT_EQ( "AAA", dupesTab( 0, 0 ) );
        EXPECT_EQ( "111", dupesTab( 0, "col1" ) );
        EXPECT_EQ( "xxx", dupesTab( 1, "col2" ) );
    }
}
