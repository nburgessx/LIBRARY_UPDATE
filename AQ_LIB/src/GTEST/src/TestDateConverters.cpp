// TestDateConverters.h

#include "DateUtilities.h"
#include <gTest/gTest.h>


namespace google_test
{
    
    // ===========================================================================================================================
    // *** Supported REGEX Date Formats - see "DateUtilities.h ***
    
    // *** VERY IMPORTANT *** Please do not change the order of the DATE_REGEX list. If adding new types add to the end of the list, this is
    // becuase the "toGregorianDateFromREGEX" method in DateUtilities.cpp iterates on the elements on the list to parse the REGEX expression.
    // static const std::vector<boost::regex> DATE_REGEX  =
	//	boost::assign::list_of("\\d{5}")( "\\d{8}" )( "\\d{4}-\\d{2}-\\d{2}" )( "(\\d{2,4})\\/(\\d{2})(?:\\/?(\\d{2}))?" )
    //                          ( "\\d{4}\\\\\\d{2}\\\\\\d{2}" )( "(\\d{2})\\/(\\d{2})(?:\\/?(\\d{2,4}))?" )
    //                          ( "\\d{2}-\\d{2}-\\d{4}" )( "\\d{2}\\\\\\d{2}\\\\\\d{4}" );
    
    // ===========================================================================================================================

    TEST( TestDateConverters, UNIT_toLADateFromREGEX )
    {
        // Date Format 1: Excel Integer 43686 = 09-Aug-2019
        std::string dateFormat1 = "43686";
        AQLDate expectedDate1( "20190809" );
        AQLDate actualDate1 = etrading::toAQLDateFromREGEX( dateFormat1 );
        EXPECT_EQ( expectedDate1, actualDate1 );

        // Date Format 2: YYYYMMDD
        std::string dateFormat2 = "20190809";
        AQLDate expectedDate2( "20190809" );
        AQLDate actualDate2 = etrading::toAQLDateFromREGEX( dateFormat2 );
        EXPECT_EQ( expectedDate2, actualDate2 );

        // Date Format 3: YYYY-MM-DD
        std::string dateFormat3 = "2019-08-09";
        AQLDate expectedDate3( "20190809" );
        AQLDate actualDate3 = etrading::toAQLDateFromREGEX( dateFormat3 );
        EXPECT_EQ( expectedDate3, actualDate3 );

        // Date Format 4: DD-MM-YYYY
        std::string dateFormat4 = "09-08-2019";
        AQLDate expectedDate4( "20190809" );
        AQLDate actualDate4 = etrading::toAQLDateFromREGEX( dateFormat4 );
        EXPECT_EQ( expectedDate4, actualDate4 );
    }

    TEST( TestDateConverters, UNIT_toGregorianDateFromREGEX )
    {
        // Date Format 1: Excel Integer 43686 = 09-Aug-2019
        std::string dateFormat1 = "43686";
        boost::gregorian::date expectedDate1( 2019, 8, 9 );
        boost::gregorian::date actualDate1 = etrading::toGregorianDateFromREGEX( dateFormat1 );
        EXPECT_EQ( expectedDate1, actualDate1 );

        // Date Format 2: YYYYMMDD
        std::string dateFormat2 = "20190809";
        boost::gregorian::date expectedDate2( 2019, 8, 9 );
        boost::gregorian::date actualDate2 = etrading::toGregorianDateFromREGEX( dateFormat2 );
        EXPECT_EQ( expectedDate2, actualDate2 );

        // Date Format 3: YYYY-MM-DD
        std::string dateFormat3 = "2019-08-09";
        boost::gregorian::date expectedDate3( 2019, 8, 9 );
        boost::gregorian::date actualDate3 = etrading::toGregorianDateFromREGEX( dateFormat3 );
        EXPECT_EQ( expectedDate3, actualDate3 );

        // Date Format 4: DD-MM-YYYY
        std::string dateFormat4 = "09-08-2019";
        boost::gregorian::date expectedDate4( 2019, 8, 9 );
        boost::gregorian::date actualDate4 = etrading::toGregorianDateFromREGEX( dateFormat4 );
        EXPECT_EQ( expectedDate4, actualDate4 );
    }

    TEST( TestDateConverters, UNIT_toLADateFromGregorianDate )
    {
        // 09-Aug-2019
        boost::gregorian::date gregorianDate( 2019, 8, 9 );
        AQLDate expectedDate("20190809");
        AQLDate actualDate = etrading::toAQLDateFromGregorianDate( gregorianDate );
        EXPECT_EQ( expectedDate, actualDate );
    }

    TEST( TestDateConverters, UNIT_toYYYYMMDDFromGregorianDate )
    {
        // 09-Aug-2019
        boost::gregorian::date gregorianDate( 2019, 8, 9 );
        std::string expectedDate("20190809");
        std::string actualDate = etrading::toYYYYMMDDFromGregorianDate( gregorianDate );
        EXPECT_EQ( expectedDate, actualDate );
    }

    TEST( TestDateConverters, UNIT_toYYYYMMDDFromDate )
    {
        // 09-Aug-2019
        AQLDate laDate("20190809");
        std::string expectedDate("20190809");
        std::string actualDate = etrading::toYYYYMMDDFromDate( laDate );
        EXPECT_EQ( expectedDate, actualDate );
    }
    
    TEST( TestDateConverters, UNIT_toGregorianDateFromLADate )
    {
        // 09-Aug-2019
        AQLDate laDate("20190809");
        boost::gregorian::date expectedDate( 2019, 8, 9 );
        boost::gregorian::date actualDate = etrading::toGregorianDateFromLADate( laDate );
        EXPECT_EQ( expectedDate, actualDate );
    }

    TEST( TestDateConverters, UNIT_toGregorianDateFromYYYYMMDD )
    {
        // 09-Aug-2019
        std::string dateYYYYMMDD = "20190809";
        boost::gregorian::date expectedDate( 2019, 8, 9 );
        boost::gregorian::date actualDate = etrading::toGregorianDateFromYYYYMMDD( dateYYYYMMDD );
        EXPECT_EQ( expectedDate, actualDate );
    }
    
    TEST( TestDateConverters, UNIT_toExcelDateFromGregorianDate )
    {
        // 09-Aug-2019 = 43686
        boost::gregorian::date gregorianDate( 2019, 8, 9 );
        int expectedDate = 43686;
        int actualDate = etrading::toExcelDateFromGregorianDate( gregorianDate );
        EXPECT_EQ( expectedDate, actualDate );
    }
    
    TEST( TestDateConverters, UNIT_toGregorianDateFromExcelDate )
    {
        // 09-Aug-2019 = 43686
        int excelDate = 43686;
        boost::gregorian::date expectedDate( 2019, 8, 9 );
        boost::gregorian::date actualDate = etrading::toGregorianDateFromExcelDate( excelDate );
        EXPECT_EQ( expectedDate, actualDate );
    }

}

