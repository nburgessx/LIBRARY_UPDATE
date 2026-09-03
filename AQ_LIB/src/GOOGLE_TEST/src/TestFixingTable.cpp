//// @Created: 27 Feb 2016
//// @Author: Hans Roggeman
//// @Department: ISO Front Office Development
////
//// The copyright to the computer program(s) herein
//// is the property of AlgoQuantHub.
//
//// Include: Google Test Library
//#include <gTest/gTest.h>
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

// Include: Google Test Library
#include <gTest/gTest.h>

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "DataSchema.h"
#include "CoreEnumerations.h"
#include "FreeObject.h"
#include "TypeName.h"
#include "Variant.h"
#include "DateUtilities.h"
#include "ContainerUtilities.h"
#include "ETradingException.h"
#include "TableDateDouble.h"
#include "ContainerUtilities.h"
#include "ConcurrentMap.h"
#include "EnvironmentAttacker.h"
#include "UserUtilities.h"
#include "SerializeContainedData.h"
#include "StatisticsUtilities.h"
#include "FileUtilities.h"
#include "EnvironmentPool.h"
#include "RuleInterface.h"
#include "LWOCurve.h"
#include "CurveBuildProperties.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "InitializeMLibGoogleTest.h"
#include "ExampleObjects.h"
#include "FixingTableSet.h"


using namespace etrading;

namespace google_test
{

    TEST( TestFixingTable, UNIT_TableDateDouble )
    {
        TableDateDouble table( "MyTable" );
        table.addDataPoint( boost::gregorian::date( 2024, 4, 24 ), 65.256 );
        table.addDataPoint( boost::gregorian::date( 2020, 4, 14 ), 61.256 );
        table.addDataPoint( boost::gregorian::date( 2014, 9, 4 ), 45.256 );
        table.addDataPoint( boost::gregorian::date( 2034, 1, 4 ), 105.256 );
        table.addDataPoint( boost::gregorian::date( 2044, 3, 4 ), 165.256 );

        EXPECT_EQ( table.getIndexOfDate( boost::gregorian::from_undelimited_string( "20140904" ) ), 0 );
        EXPECT_EQ( table.getIndexOfDate( boost::gregorian::from_undelimited_string( "20200414" ) ), 1 );
        EXPECT_EQ( table.getIndexOfDate( boost::gregorian::from_undelimited_string( "20240424" ) ), 2 );
        EXPECT_EQ( table.getIndexOfDate( boost::gregorian::from_undelimited_string( "20340104" ) ), 3 );
        EXPECT_EQ( table.getIndexOfDate( boost::gregorian::from_undelimited_string( "20440304" ) ), 4 );

        EXPECT_EQ( table.getIndexOfDate( boost::gregorian::from_undelimited_string( "22440304" ) ), -1 );
        EXPECT_EQ( table.getIndexOfDate( boost::gregorian::from_undelimited_string( "20010304" ) ), -1 );

        EXPECT_STREQ( table.getName().c_str(), "MyTable" );

        EXPECT_EQ( table.getData( 0 ).first, boost::gregorian::from_simple_string( "2014-09-04" ) );
        EXPECT_EQ( table.getData( 1 ).first, boost::gregorian::from_simple_string( "2020-04-14" ) );
        EXPECT_EQ( table.getData( 2 ).first, boost::gregorian::from_simple_string( "2024-04-24" ) );
        EXPECT_EQ( table.getData( 3 ).first, boost::gregorian::from_simple_string( "2034-01-04" ) );
        EXPECT_EQ( table.getData( 4 ).first, boost::gregorian::from_simple_string( "2044-03-04" ) );

        EXPECT_NE( table.getData( 0 ).second, 65.256 );
        EXPECT_DOUBLE_EQ( table.getData( 0 ).second, 45.256 );
        EXPECT_DOUBLE_EQ( table.getData( 1 ).second, 61.256 );
        EXPECT_NE( table.getData( 2 ).second, 45.256 );
        EXPECT_DOUBLE_EQ( table.getData( 2 ).second, 65.256 );
        EXPECT_DOUBLE_EQ( table.getData( 3 ).second, 105.256 );
        EXPECT_DOUBLE_EQ( table.getData( 4 ).second, 165.256 );

        std::vector<double> values = boost::assign::list_of( 563.34 )( 2342.907 )( 123.52 )( 1950.32 );

        TableDateDouble t2( "SecondTable" );
        std::vector<boost::gregorian::date> dates = boost::assign::list_of	( boost::gregorian::from_undelimited_string( "20430304" ) )
                ( boost::gregorian::from_undelimited_string( "20440304" ) )
                ( boost::gregorian::from_undelimited_string( "20440504" ) )
                ( boost::gregorian::from_undelimited_string( "21440304" ) );
        t2.setDates( dates );
        t2.setValues( values );

        EXPECT_EQ( t2.getData( 0 ).first, boost::gregorian::from_simple_string( "2043-03-04" ) );
        EXPECT_EQ( t2.getData( 1 ).first, boost::gregorian::from_simple_string( "2044-03-04" ) );
        EXPECT_EQ( t2.getData( 2 ).first, boost::gregorian::from_simple_string( "2044-05-04" ) );
        EXPECT_EQ( t2.getData( 3 ).first, boost::gregorian::from_simple_string( "2144-03-04" ) );

        EXPECT_DOUBLE_EQ( t2.getData( 0 ).second, 563.34 );
        EXPECT_DOUBLE_EQ( t2.getData( 1 ).second, values.at( 1 ) );
        EXPECT_DOUBLE_EQ( t2.getData( 2 ).second, 123.52 );
        EXPECT_DOUBLE_EQ( t2.getData( 3 ).second, 1950.32 );

        t2.clear();

        EXPECT_THROW( t2.getData( 1 ), ETradingException );
        try
        {
            t2.getData( 0 );
        }
        catch( ETradingException& eEx )
        {
            EXPECT_STREQ( eEx.what(), "Illegal Date index requested: 0 when there are 0 dates available" );
        }

        TableDateDouble t3( "ThirdTable" );
        std::vector<boost::gregorian::date> unorderedDates = boost::assign::list_of  ( boost::gregorian::from_undelimited_string( "20440304" ) )
                ( boost::gregorian::from_undelimited_string( "21440304" ) )
                ( boost::gregorian::from_undelimited_string( "20430304" ) )
                ( boost::gregorian::from_undelimited_string( "20440504" ) );
        EXPECT_TRUE( t3.setValues( values ) );
        
    };

    TEST( TestFixingTable, UNIT_FixingTableSet )
    {
        FixingTableSet allFixingTables( "FIXINGS" ); // could be static in implementation
        EXPECT_FALSE( allFixingTables.has( EUR, CURVE_TENOR_6M ) );

        std::vector<boost::gregorian::date> fixingDates =
            boost::assign::list_of( boost::gregorian::date( 2009, 6, 6 ) )( boost::gregorian::date( 2009, 12, 8 ) )( boost::gregorian::date( 2010, 6, 6 ) );

        FixingTable eur6MTable( "EUR_6M_FIXINGS_20101010", fixingDates, boost::assign::list_of( -0.05 )( 0.02 )( -0.02 ), CURVE_TENOR_6M, EUR );
        allFixingTables.insert( eur6MTable );

        EXPECT_TRUE( allFixingTables.has( EUR, CURVE_TENOR_6M ) );
        EXPECT_TRUE( allFixingTables.has( "EUR_6M_FIXINGS_20101010" ) );

        auto& fixTable = allFixingTables.access( "EUR_6M_FIXINGS_20101010" );
        auto variantMatrix = fixTable.getVariantMatrix();  // XllPlusTips::populateExcelArrayWithVariantMatrixByColumn gives easy population in excel

        std::cout << variantMatrix.size() << std::endl;

        EXPECT_DOUBLE_EQ( fixTable.getData( 0 ).second, -0.05 );
        EXPECT_EQ( fixTable.getIndexOfDate( boost::gregorian::date( 2009, 12, 8 ) ), 1 );
        EXPECT_EQ( fixTable.getData( 0 ).first, boost::gregorian::date( 2009, 6, 6 ) );

    };

}