// TestValidateAndConvertStringToDate.cpp

#include "ParameterValidation.h"
#include "LACoreAppError.h"
#include <gTest/gTest.h>

namespace google_test
{

	// Test if an date in LAString format can be converted to LADate format
    TEST( TestValidateAndConvertStringToDate, UNIT_ConvertStringToDate_YYYYMMDD )
    {
		LADate expectedResult1("20180426");
		LAString inputDateStr1("20180426");

        LADate actualResult1 = etrading::stringToDate( inputDateStr1, "Test Failure: Converting YYYYMMDD to Date" );
        EXPECT_EQ( expectedResult1, actualResult1 );

        // Expected vs Actual
        EXPECT_EQ( LADate("20180101"), etrading::stringToDate( LAString("20180101") ) );
        EXPECT_EQ( LADate("20180102"), etrading::stringToDate( LAString("20180102") ) );
        EXPECT_EQ( LADate("20180103"), etrading::stringToDate( LAString("20180103") ) );
        EXPECT_EQ( LADate("20180104"), etrading::stringToDate( LAString("20180104") ) );
        EXPECT_EQ( LADate("20180105"), etrading::stringToDate( LAString("20180105") ) );

        EXPECT_EQ( LADate("20180106"), etrading::stringToDate( LAString("20180106") ) );
        EXPECT_EQ( LADate("20180107"), etrading::stringToDate( LAString("20180107") ) );
        EXPECT_EQ( LADate("20180108"), etrading::stringToDate( LAString("20180108") ) );
        EXPECT_EQ( LADate("20180109"), etrading::stringToDate( LAString("20180109") ) );
        EXPECT_EQ( LADate("20180110"), etrading::stringToDate( LAString("20180110") ) );

        EXPECT_EQ( LADate("20180111"), etrading::stringToDate( LAString("20180111") ) );
        EXPECT_EQ( LADate("20180112"), etrading::stringToDate( LAString("20180112") ) );
        EXPECT_EQ( LADate("20180113"), etrading::stringToDate( LAString("20180113") ) );
        EXPECT_EQ( LADate("20180114"), etrading::stringToDate( LAString("20180114") ) );
        EXPECT_EQ( LADate("20180115"), etrading::stringToDate( LAString("20180115") ) );

        EXPECT_EQ( LADate("20180116"), etrading::stringToDate( LAString("20180116") ) );
        EXPECT_EQ( LADate("20180117"), etrading::stringToDate( LAString("20180117") ) );
        EXPECT_EQ( LADate("20180118"), etrading::stringToDate( LAString("20180118") ) );
        EXPECT_EQ( LADate("20180119"), etrading::stringToDate( LAString("20180119") ) );
        EXPECT_EQ( LADate("20180120"), etrading::stringToDate( LAString("20180120") ) );

        EXPECT_EQ( LADate("20180121"), etrading::stringToDate( LAString("20180121") ) );
        EXPECT_EQ( LADate("20180122"), etrading::stringToDate( LAString("20180122") ) );
        EXPECT_EQ( LADate("20180123"), etrading::stringToDate( LAString("20180123") ) );
        EXPECT_EQ( LADate("20180124"), etrading::stringToDate( LAString("20180124") ) );
        EXPECT_EQ( LADate("20180125"), etrading::stringToDate( LAString("20180125") ) );

        EXPECT_EQ( LADate("20180126"), etrading::stringToDate( LAString("20180126") ) );
        EXPECT_EQ( LADate("20180127"), etrading::stringToDate( LAString("20180127") ) );
        EXPECT_EQ( LADate("20180128"), etrading::stringToDate( LAString("20180128") ) );
        EXPECT_EQ( LADate("20180129"), etrading::stringToDate( LAString("20180129") ) );
        EXPECT_EQ( LADate("20180130"), etrading::stringToDate( LAString("20180130") ) );

        EXPECT_EQ( LADate("20180131"), etrading::stringToDate( LAString("20180131") ) );
    }

    // Test if an date in Excel format can be converted to LADate format
    TEST( TestValidateAndConvertStringToDate, UNIT_ConvertStringToDate_ExcelFormat )
    {
		LADate expectedResult1("20180426");
		LAString inputDateStr1("43216");

        LADate actualResult1 = etrading::stringToDate( inputDateStr1, "Test Failure: Converting Excel Date Strings to Date" );
        EXPECT_EQ( expectedResult1, actualResult1 );

        EXPECT_EQ( LADate("20180101"), etrading::stringToDate( LAString("43101") ) );
        EXPECT_EQ( LADate("20180102"), etrading::stringToDate( LAString("43102") ) );
        EXPECT_EQ( LADate("20180103"), etrading::stringToDate( LAString("43103") ) );
        EXPECT_EQ( LADate("20180104"), etrading::stringToDate( LAString("43104") ) );
        EXPECT_EQ( LADate("20180105"), etrading::stringToDate( LAString("43105") ) );

        EXPECT_EQ( LADate("20180106"), etrading::stringToDate( LAString("43106") ) );
        EXPECT_EQ( LADate("20180107"), etrading::stringToDate( LAString("43107") ) );
        EXPECT_EQ( LADate("20180108"), etrading::stringToDate( LAString("43108") ) );
        EXPECT_EQ( LADate("20180109"), etrading::stringToDate( LAString("43109") ) );
        EXPECT_EQ( LADate("20180110"), etrading::stringToDate( LAString("43110") ) );

        EXPECT_EQ( LADate("20180111"), etrading::stringToDate( LAString("43111") ) );
        EXPECT_EQ( LADate("20180112"), etrading::stringToDate( LAString("43112") ) );
        EXPECT_EQ( LADate("20180113"), etrading::stringToDate( LAString("43113") ) );
        EXPECT_EQ( LADate("20180114"), etrading::stringToDate( LAString("43114") ) );
        EXPECT_EQ( LADate("20180115"), etrading::stringToDate( LAString("43115") ) );

        EXPECT_EQ( LADate("20180116"), etrading::stringToDate( LAString("43116") ) );
        EXPECT_EQ( LADate("20180117"), etrading::stringToDate( LAString("43117") ) );
        EXPECT_EQ( LADate("20180118"), etrading::stringToDate( LAString("43118") ) );
        EXPECT_EQ( LADate("20180119"), etrading::stringToDate( LAString("43119") ) );
        EXPECT_EQ( LADate("20180120"), etrading::stringToDate( LAString("43120") ) );

        EXPECT_EQ( LADate("20180121"), etrading::stringToDate( LAString("43121") ) );
        EXPECT_EQ( LADate("20180122"), etrading::stringToDate( LAString("43122") ) );
        EXPECT_EQ( LADate("20180123"), etrading::stringToDate( LAString("43123") ) );
        EXPECT_EQ( LADate("20180124"), etrading::stringToDate( LAString("43124") ) );
        EXPECT_EQ( LADate("20180125"), etrading::stringToDate( LAString("43125") ) );

        EXPECT_EQ( LADate("20180126"), etrading::stringToDate( LAString("43126") ) );
        EXPECT_EQ( LADate("20180127"), etrading::stringToDate( LAString("43127") ) );
        EXPECT_EQ( LADate("20180128"), etrading::stringToDate( LAString("43128") ) );
        EXPECT_EQ( LADate("20180129"), etrading::stringToDate( LAString("43129") ) );
        EXPECT_EQ( LADate("20180130"), etrading::stringToDate( LAString("43130") ) );

        EXPECT_EQ( LADate("20180131"), etrading::stringToDate( LAString("43131") ) );
    }

    // Test if an date in DD/MM/YYYY format can be converted to LADate format
    TEST( TestValidateAndConvertStringToDate, UNIT_ConvertStringToDate_DD_MM_YYYY )
    {
		LADate expectedResult1("20180426");
		LAString inputDateStr1("26/04/2018");

        LADate actualResult1 = etrading::stringToDate( inputDateStr1, "Test Failure: Converting DD/MM/YYYY Date Strings to Date" );
        EXPECT_EQ( expectedResult1, actualResult1 );

        EXPECT_EQ( LADate("20180101"), etrading::stringToDate( LAString("01/01/2018") ) );
        EXPECT_EQ( LADate("20180102"), etrading::stringToDate( LAString("02/01/2018") ) );
        EXPECT_EQ( LADate("20180103"), etrading::stringToDate( LAString("03/01/2018") ) );
        EXPECT_EQ( LADate("20180104"), etrading::stringToDate( LAString("04/01/2018") ) );
        EXPECT_EQ( LADate("20180105"), etrading::stringToDate( LAString("05/01/2018") ) );

        EXPECT_EQ( LADate("20180106"), etrading::stringToDate( LAString("06/01/2018") ) );
        EXPECT_EQ( LADate("20180107"), etrading::stringToDate( LAString("07/01/2018") ) );
        EXPECT_EQ( LADate("20180108"), etrading::stringToDate( LAString("08/01/2018") ) );
        EXPECT_EQ( LADate("20180109"), etrading::stringToDate( LAString("09/01/2018") ) );
        EXPECT_EQ( LADate("20180110"), etrading::stringToDate( LAString("10/01/2018") ) );

        EXPECT_EQ( LADate("20180111"), etrading::stringToDate( LAString("11/01/2018") ) );
        EXPECT_EQ( LADate("20180112"), etrading::stringToDate( LAString("12/01/2018") ) );
        EXPECT_EQ( LADate("20180113"), etrading::stringToDate( LAString("13/01/2018") ) );
        EXPECT_EQ( LADate("20180114"), etrading::stringToDate( LAString("14/01/2018") ) );
        EXPECT_EQ( LADate("20180115"), etrading::stringToDate( LAString("15/01/2018") ) );

        EXPECT_EQ( LADate("20180116"), etrading::stringToDate( LAString("16/01/2018") ) );
        EXPECT_EQ( LADate("20180117"), etrading::stringToDate( LAString("17/01/2018") ) );
        EXPECT_EQ( LADate("20180118"), etrading::stringToDate( LAString("18/01/2018") ) );
        EXPECT_EQ( LADate("20180119"), etrading::stringToDate( LAString("19/01/2018") ) );
        EXPECT_EQ( LADate("20180120"), etrading::stringToDate( LAString("20/01/2018") ) );

        EXPECT_EQ( LADate("20180121"), etrading::stringToDate( LAString("21/01/2018") ) );
        EXPECT_EQ( LADate("20180122"), etrading::stringToDate( LAString("22/01/2018") ) );
        EXPECT_EQ( LADate("20180123"), etrading::stringToDate( LAString("23/01/2018") ) );
        EXPECT_EQ( LADate("20180124"), etrading::stringToDate( LAString("24/01/2018") ) );
        EXPECT_EQ( LADate("20180125"), etrading::stringToDate( LAString("25/01/2018") ) );

        EXPECT_EQ( LADate("20180126"), etrading::stringToDate( LAString("26/01/2018") ) );
        EXPECT_EQ( LADate("20180127"), etrading::stringToDate( LAString("27/01/2018") ) );
        EXPECT_EQ( LADate("20180128"), etrading::stringToDate( LAString("28/01/2018") ) );
        EXPECT_EQ( LADate("20180129"), etrading::stringToDate( LAString("29/01/2018") ) );
        EXPECT_EQ( LADate("20180130"), etrading::stringToDate( LAString("30/01/2018") ) );

        EXPECT_EQ( LADate("20180131"), etrading::stringToDate( LAString("31/01/2018") ) );
    }

    // Test if an date in YYYY/MM/DD format can be converted to LADate format
    TEST( TestValidateAndConvertStringToDate, UNIT_ConvertStringToDate_YYYY_MM_DD )
    {
		LADate expectedResult1("20180426");
		LAString inputDateStr1("2018/04/26");

        LADate actualResult1 = etrading::stringToDate( inputDateStr1, "Test Failure: Converting YYYY/MM/DD Date Strings to Date" );
        EXPECT_EQ( expectedResult1, actualResult1 );

        EXPECT_EQ( LADate("20180101"), etrading::stringToDate( LAString("2018/01/01") ) );
        EXPECT_EQ( LADate("20180102"), etrading::stringToDate( LAString("2018/01/02") ) );
        EXPECT_EQ( LADate("20180103"), etrading::stringToDate( LAString("2018/01/03") ) );
        EXPECT_EQ( LADate("20180104"), etrading::stringToDate( LAString("2018/01/04") ) );
        EXPECT_EQ( LADate("20180105"), etrading::stringToDate( LAString("2018/01/05") ) );

        EXPECT_EQ( LADate("20180106"), etrading::stringToDate( LAString("2018/01/06") ) );
        EXPECT_EQ( LADate("20180107"), etrading::stringToDate( LAString("2018/01/07") ) );
        EXPECT_EQ( LADate("20180108"), etrading::stringToDate( LAString("2018/01/08") ) );
        EXPECT_EQ( LADate("20180109"), etrading::stringToDate( LAString("2018/01/09") ) );
        EXPECT_EQ( LADate("20180110"), etrading::stringToDate( LAString("2018/01/10") ) );

        EXPECT_EQ( LADate("20180111"), etrading::stringToDate( LAString("2018/01/11") ) );
        EXPECT_EQ( LADate("20180112"), etrading::stringToDate( LAString("2018/01/12") ) );
        EXPECT_EQ( LADate("20180113"), etrading::stringToDate( LAString("2018/01/13") ) );
        EXPECT_EQ( LADate("20180114"), etrading::stringToDate( LAString("2018/01/14") ) );
        EXPECT_EQ( LADate("20180115"), etrading::stringToDate( LAString("2018/01/15") ) );

        EXPECT_EQ( LADate("20180116"), etrading::stringToDate( LAString("2018/01/16") ) );
        EXPECT_EQ( LADate("20180117"), etrading::stringToDate( LAString("2018/01/17") ) );
        EXPECT_EQ( LADate("20180118"), etrading::stringToDate( LAString("2018/01/18") ) );
        EXPECT_EQ( LADate("20180119"), etrading::stringToDate( LAString("2018/01/19") ) );
        EXPECT_EQ( LADate("20180120"), etrading::stringToDate( LAString("2018/01/20") ) );

        EXPECT_EQ( LADate("20180121"), etrading::stringToDate( LAString("2018/01/21") ) );
        EXPECT_EQ( LADate("20180122"), etrading::stringToDate( LAString("2018/01/22") ) );
        EXPECT_EQ( LADate("20180123"), etrading::stringToDate( LAString("2018/01/23") ) );
        EXPECT_EQ( LADate("20180124"), etrading::stringToDate( LAString("2018/01/24") ) );
        EXPECT_EQ( LADate("20180125"), etrading::stringToDate( LAString("2018/01/25") ) );

        EXPECT_EQ( LADate("20180126"), etrading::stringToDate( LAString("2018/01/26") ) );
        EXPECT_EQ( LADate("20180127"), etrading::stringToDate( LAString("2018/01/27") ) );
        EXPECT_EQ( LADate("20180128"), etrading::stringToDate( LAString("2018/01/28") ) );
        EXPECT_EQ( LADate("20180129"), etrading::stringToDate( LAString("2018/01/29") ) );
        EXPECT_EQ( LADate("20180130"), etrading::stringToDate( LAString("2018/01/30") ) );
        
        EXPECT_EQ( LADate("20180131"), etrading::stringToDate( LAString("2018/01/31") ) );
    }
}
