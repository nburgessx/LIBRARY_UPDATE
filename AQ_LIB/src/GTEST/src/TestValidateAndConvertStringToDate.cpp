// TestValidateAndConvertStringToDate.cpp

#include "ParameterValidation.h"
#include "AQLCoreAppError.h"
#include <gTest/gTest.h>

namespace google_test
{

	// Test if an date in AQLString format can be converted to AQLDate format
    TEST( TestValidateAndConvertStringToDate, UNIT_ConvertStringToDate_YYYYMMDD )
    {
		AQLDate expectedResult1("20180426");
		AQLString inputDateStr1("20180426");

        AQLDate actualResult1 = etrading::stringToDate( inputDateStr1, "Test Failure: Converting YYYYMMDD to Date" );
        EXPECT_EQ( expectedResult1, actualResult1 );

        // Expected vs Actual
        EXPECT_EQ( AQLDate("20180101"), etrading::stringToDate( AQLString("20180101") ) );
        EXPECT_EQ( AQLDate("20180102"), etrading::stringToDate( AQLString("20180102") ) );
        EXPECT_EQ( AQLDate("20180103"), etrading::stringToDate( AQLString("20180103") ) );
        EXPECT_EQ( AQLDate("20180104"), etrading::stringToDate( AQLString("20180104") ) );
        EXPECT_EQ( AQLDate("20180105"), etrading::stringToDate( AQLString("20180105") ) );

        EXPECT_EQ( AQLDate("20180106"), etrading::stringToDate( AQLString("20180106") ) );
        EXPECT_EQ( AQLDate("20180107"), etrading::stringToDate( AQLString("20180107") ) );
        EXPECT_EQ( AQLDate("20180108"), etrading::stringToDate( AQLString("20180108") ) );
        EXPECT_EQ( AQLDate("20180109"), etrading::stringToDate( AQLString("20180109") ) );
        EXPECT_EQ( AQLDate("20180110"), etrading::stringToDate( AQLString("20180110") ) );

        EXPECT_EQ( AQLDate("20180111"), etrading::stringToDate( AQLString("20180111") ) );
        EXPECT_EQ( AQLDate("20180112"), etrading::stringToDate( AQLString("20180112") ) );
        EXPECT_EQ( AQLDate("20180113"), etrading::stringToDate( AQLString("20180113") ) );
        EXPECT_EQ( AQLDate("20180114"), etrading::stringToDate( AQLString("20180114") ) );
        EXPECT_EQ( AQLDate("20180115"), etrading::stringToDate( AQLString("20180115") ) );

        EXPECT_EQ( AQLDate("20180116"), etrading::stringToDate( AQLString("20180116") ) );
        EXPECT_EQ( AQLDate("20180117"), etrading::stringToDate( AQLString("20180117") ) );
        EXPECT_EQ( AQLDate("20180118"), etrading::stringToDate( AQLString("20180118") ) );
        EXPECT_EQ( AQLDate("20180119"), etrading::stringToDate( AQLString("20180119") ) );
        EXPECT_EQ( AQLDate("20180120"), etrading::stringToDate( AQLString("20180120") ) );

        EXPECT_EQ( AQLDate("20180121"), etrading::stringToDate( AQLString("20180121") ) );
        EXPECT_EQ( AQLDate("20180122"), etrading::stringToDate( AQLString("20180122") ) );
        EXPECT_EQ( AQLDate("20180123"), etrading::stringToDate( AQLString("20180123") ) );
        EXPECT_EQ( AQLDate("20180124"), etrading::stringToDate( AQLString("20180124") ) );
        EXPECT_EQ( AQLDate("20180125"), etrading::stringToDate( AQLString("20180125") ) );

        EXPECT_EQ( AQLDate("20180126"), etrading::stringToDate( AQLString("20180126") ) );
        EXPECT_EQ( AQLDate("20180127"), etrading::stringToDate( AQLString("20180127") ) );
        EXPECT_EQ( AQLDate("20180128"), etrading::stringToDate( AQLString("20180128") ) );
        EXPECT_EQ( AQLDate("20180129"), etrading::stringToDate( AQLString("20180129") ) );
        EXPECT_EQ( AQLDate("20180130"), etrading::stringToDate( AQLString("20180130") ) );

        EXPECT_EQ( AQLDate("20180131"), etrading::stringToDate( AQLString("20180131") ) );
    }

    // Test if an date in Excel format can be converted to AQLDate format
    TEST( TestValidateAndConvertStringToDate, UNIT_ConvertStringToDate_ExcelFormat )
    {
		AQLDate expectedResult1("20180426");
		AQLString inputDateStr1("43216");

        AQLDate actualResult1 = etrading::stringToDate( inputDateStr1, "Test Failure: Converting Excel Date Strings to Date" );
        EXPECT_EQ( expectedResult1, actualResult1 );

        EXPECT_EQ( AQLDate("20180101"), etrading::stringToDate( AQLString("43101") ) );
        EXPECT_EQ( AQLDate("20180102"), etrading::stringToDate( AQLString("43102") ) );
        EXPECT_EQ( AQLDate("20180103"), etrading::stringToDate( AQLString("43103") ) );
        EXPECT_EQ( AQLDate("20180104"), etrading::stringToDate( AQLString("43104") ) );
        EXPECT_EQ( AQLDate("20180105"), etrading::stringToDate( AQLString("43105") ) );

        EXPECT_EQ( AQLDate("20180106"), etrading::stringToDate( AQLString("43106") ) );
        EXPECT_EQ( AQLDate("20180107"), etrading::stringToDate( AQLString("43107") ) );
        EXPECT_EQ( AQLDate("20180108"), etrading::stringToDate( AQLString("43108") ) );
        EXPECT_EQ( AQLDate("20180109"), etrading::stringToDate( AQLString("43109") ) );
        EXPECT_EQ( AQLDate("20180110"), etrading::stringToDate( AQLString("43110") ) );

        EXPECT_EQ( AQLDate("20180111"), etrading::stringToDate( AQLString("43111") ) );
        EXPECT_EQ( AQLDate("20180112"), etrading::stringToDate( AQLString("43112") ) );
        EXPECT_EQ( AQLDate("20180113"), etrading::stringToDate( AQLString("43113") ) );
        EXPECT_EQ( AQLDate("20180114"), etrading::stringToDate( AQLString("43114") ) );
        EXPECT_EQ( AQLDate("20180115"), etrading::stringToDate( AQLString("43115") ) );

        EXPECT_EQ( AQLDate("20180116"), etrading::stringToDate( AQLString("43116") ) );
        EXPECT_EQ( AQLDate("20180117"), etrading::stringToDate( AQLString("43117") ) );
        EXPECT_EQ( AQLDate("20180118"), etrading::stringToDate( AQLString("43118") ) );
        EXPECT_EQ( AQLDate("20180119"), etrading::stringToDate( AQLString("43119") ) );
        EXPECT_EQ( AQLDate("20180120"), etrading::stringToDate( AQLString("43120") ) );

        EXPECT_EQ( AQLDate("20180121"), etrading::stringToDate( AQLString("43121") ) );
        EXPECT_EQ( AQLDate("20180122"), etrading::stringToDate( AQLString("43122") ) );
        EXPECT_EQ( AQLDate("20180123"), etrading::stringToDate( AQLString("43123") ) );
        EXPECT_EQ( AQLDate("20180124"), etrading::stringToDate( AQLString("43124") ) );
        EXPECT_EQ( AQLDate("20180125"), etrading::stringToDate( AQLString("43125") ) );

        EXPECT_EQ( AQLDate("20180126"), etrading::stringToDate( AQLString("43126") ) );
        EXPECT_EQ( AQLDate("20180127"), etrading::stringToDate( AQLString("43127") ) );
        EXPECT_EQ( AQLDate("20180128"), etrading::stringToDate( AQLString("43128") ) );
        EXPECT_EQ( AQLDate("20180129"), etrading::stringToDate( AQLString("43129") ) );
        EXPECT_EQ( AQLDate("20180130"), etrading::stringToDate( AQLString("43130") ) );

        EXPECT_EQ( AQLDate("20180131"), etrading::stringToDate( AQLString("43131") ) );
    }

    // Test if an date in DD/MM/YYYY format can be converted to AQLDate format
    TEST( TestValidateAndConvertStringToDate, UNIT_ConvertStringToDate_DD_MM_YYYY )
    {
		AQLDate expectedResult1("20180426");
		AQLString inputDateStr1("26/04/2018");

        AQLDate actualResult1 = etrading::stringToDate( inputDateStr1, "Test Failure: Converting DD/MM/YYYY Date Strings to Date" );
        EXPECT_EQ( expectedResult1, actualResult1 );

        EXPECT_EQ( AQLDate("20180101"), etrading::stringToDate( AQLString("01/01/2018") ) );
        EXPECT_EQ( AQLDate("20180102"), etrading::stringToDate( AQLString("02/01/2018") ) );
        EXPECT_EQ( AQLDate("20180103"), etrading::stringToDate( AQLString("03/01/2018") ) );
        EXPECT_EQ( AQLDate("20180104"), etrading::stringToDate( AQLString("04/01/2018") ) );
        EXPECT_EQ( AQLDate("20180105"), etrading::stringToDate( AQLString("05/01/2018") ) );

        EXPECT_EQ( AQLDate("20180106"), etrading::stringToDate( AQLString("06/01/2018") ) );
        EXPECT_EQ( AQLDate("20180107"), etrading::stringToDate( AQLString("07/01/2018") ) );
        EXPECT_EQ( AQLDate("20180108"), etrading::stringToDate( AQLString("08/01/2018") ) );
        EXPECT_EQ( AQLDate("20180109"), etrading::stringToDate( AQLString("09/01/2018") ) );
        EXPECT_EQ( AQLDate("20180110"), etrading::stringToDate( AQLString("10/01/2018") ) );

        EXPECT_EQ( AQLDate("20180111"), etrading::stringToDate( AQLString("11/01/2018") ) );
        EXPECT_EQ( AQLDate("20180112"), etrading::stringToDate( AQLString("12/01/2018") ) );
        EXPECT_EQ( AQLDate("20180113"), etrading::stringToDate( AQLString("13/01/2018") ) );
        EXPECT_EQ( AQLDate("20180114"), etrading::stringToDate( AQLString("14/01/2018") ) );
        EXPECT_EQ( AQLDate("20180115"), etrading::stringToDate( AQLString("15/01/2018") ) );

        EXPECT_EQ( AQLDate("20180116"), etrading::stringToDate( AQLString("16/01/2018") ) );
        EXPECT_EQ( AQLDate("20180117"), etrading::stringToDate( AQLString("17/01/2018") ) );
        EXPECT_EQ( AQLDate("20180118"), etrading::stringToDate( AQLString("18/01/2018") ) );
        EXPECT_EQ( AQLDate("20180119"), etrading::stringToDate( AQLString("19/01/2018") ) );
        EXPECT_EQ( AQLDate("20180120"), etrading::stringToDate( AQLString("20/01/2018") ) );

        EXPECT_EQ( AQLDate("20180121"), etrading::stringToDate( AQLString("21/01/2018") ) );
        EXPECT_EQ( AQLDate("20180122"), etrading::stringToDate( AQLString("22/01/2018") ) );
        EXPECT_EQ( AQLDate("20180123"), etrading::stringToDate( AQLString("23/01/2018") ) );
        EXPECT_EQ( AQLDate("20180124"), etrading::stringToDate( AQLString("24/01/2018") ) );
        EXPECT_EQ( AQLDate("20180125"), etrading::stringToDate( AQLString("25/01/2018") ) );

        EXPECT_EQ( AQLDate("20180126"), etrading::stringToDate( AQLString("26/01/2018") ) );
        EXPECT_EQ( AQLDate("20180127"), etrading::stringToDate( AQLString("27/01/2018") ) );
        EXPECT_EQ( AQLDate("20180128"), etrading::stringToDate( AQLString("28/01/2018") ) );
        EXPECT_EQ( AQLDate("20180129"), etrading::stringToDate( AQLString("29/01/2018") ) );
        EXPECT_EQ( AQLDate("20180130"), etrading::stringToDate( AQLString("30/01/2018") ) );

        EXPECT_EQ( AQLDate("20180131"), etrading::stringToDate( AQLString("31/01/2018") ) );
    }

    // Test if an date in YYYY/MM/DD format can be converted to AQLDate format
    TEST( TestValidateAndConvertStringToDate, UNIT_ConvertStringToDate_YYYY_MM_DD )
    {
		AQLDate expectedResult1("20180426");
		AQLString inputDateStr1("2018/04/26");

        AQLDate actualResult1 = etrading::stringToDate( inputDateStr1, "Test Failure: Converting YYYY/MM/DD Date Strings to Date" );
        EXPECT_EQ( expectedResult1, actualResult1 );

        EXPECT_EQ( AQLDate("20180101"), etrading::stringToDate( AQLString("2018/01/01") ) );
        EXPECT_EQ( AQLDate("20180102"), etrading::stringToDate( AQLString("2018/01/02") ) );
        EXPECT_EQ( AQLDate("20180103"), etrading::stringToDate( AQLString("2018/01/03") ) );
        EXPECT_EQ( AQLDate("20180104"), etrading::stringToDate( AQLString("2018/01/04") ) );
        EXPECT_EQ( AQLDate("20180105"), etrading::stringToDate( AQLString("2018/01/05") ) );

        EXPECT_EQ( AQLDate("20180106"), etrading::stringToDate( AQLString("2018/01/06") ) );
        EXPECT_EQ( AQLDate("20180107"), etrading::stringToDate( AQLString("2018/01/07") ) );
        EXPECT_EQ( AQLDate("20180108"), etrading::stringToDate( AQLString("2018/01/08") ) );
        EXPECT_EQ( AQLDate("20180109"), etrading::stringToDate( AQLString("2018/01/09") ) );
        EXPECT_EQ( AQLDate("20180110"), etrading::stringToDate( AQLString("2018/01/10") ) );

        EXPECT_EQ( AQLDate("20180111"), etrading::stringToDate( AQLString("2018/01/11") ) );
        EXPECT_EQ( AQLDate("20180112"), etrading::stringToDate( AQLString("2018/01/12") ) );
        EXPECT_EQ( AQLDate("20180113"), etrading::stringToDate( AQLString("2018/01/13") ) );
        EXPECT_EQ( AQLDate("20180114"), etrading::stringToDate( AQLString("2018/01/14") ) );
        EXPECT_EQ( AQLDate("20180115"), etrading::stringToDate( AQLString("2018/01/15") ) );

        EXPECT_EQ( AQLDate("20180116"), etrading::stringToDate( AQLString("2018/01/16") ) );
        EXPECT_EQ( AQLDate("20180117"), etrading::stringToDate( AQLString("2018/01/17") ) );
        EXPECT_EQ( AQLDate("20180118"), etrading::stringToDate( AQLString("2018/01/18") ) );
        EXPECT_EQ( AQLDate("20180119"), etrading::stringToDate( AQLString("2018/01/19") ) );
        EXPECT_EQ( AQLDate("20180120"), etrading::stringToDate( AQLString("2018/01/20") ) );

        EXPECT_EQ( AQLDate("20180121"), etrading::stringToDate( AQLString("2018/01/21") ) );
        EXPECT_EQ( AQLDate("20180122"), etrading::stringToDate( AQLString("2018/01/22") ) );
        EXPECT_EQ( AQLDate("20180123"), etrading::stringToDate( AQLString("2018/01/23") ) );
        EXPECT_EQ( AQLDate("20180124"), etrading::stringToDate( AQLString("2018/01/24") ) );
        EXPECT_EQ( AQLDate("20180125"), etrading::stringToDate( AQLString("2018/01/25") ) );

        EXPECT_EQ( AQLDate("20180126"), etrading::stringToDate( AQLString("2018/01/26") ) );
        EXPECT_EQ( AQLDate("20180127"), etrading::stringToDate( AQLString("2018/01/27") ) );
        EXPECT_EQ( AQLDate("20180128"), etrading::stringToDate( AQLString("2018/01/28") ) );
        EXPECT_EQ( AQLDate("20180129"), etrading::stringToDate( AQLString("2018/01/29") ) );
        EXPECT_EQ( AQLDate("20180130"), etrading::stringToDate( AQLString("2018/01/30") ) );
        
        EXPECT_EQ( AQLDate("20180131"), etrading::stringToDate( AQLString("2018/01/31") ) );
    }
}
