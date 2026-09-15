// Include: Google Test Library
#include <gTest/gTest.h>

#include <string>

#include "tryAqGenerator.h"

namespace google_test
{
    TEST( TestAqGeneratorValidate, UNIT_ValidatesRealGeneratorsAsOK )
    {
        EXPECT_EQ( validation::tryAqGeneratorValidate( "SWAP_GENERATOR", "USD_3ML" ), "OK" );
        EXPECT_EQ( validation::tryAqGeneratorValidate( "BOND_GENERATOR", "US_TREASURY_TYPE1" ), "OK" );
        EXPECT_EQ( validation::tryAqGeneratorValidate( "CURVE_GENERATOR", "USD_OIS" ), "OK" );
    }

    TEST( TestAqGeneratorValidate, UNIT_ReportsUnknownGeneratorRatherThanThrowing )
    {
        const std::string result = validation::tryAqGeneratorValidate( "SWAP_GENERATOR", "NOT_A_REAL_GENERATOR" );
        EXPECT_NE( result.find( "Invalid" ), std::string::npos );
    }
}
