// Include: Google Test Library
#include <gTest/gTest.h>

#include <string>

#include "tryAqGenerator.h"

namespace google_test
{
    TEST( TestAqGeneratorDisplay, UNIT_DisplaysEachGeneratorType )
    {
        const etrading::VariantMatrix swapProperties = validation::tryAqGeneratorDisplay( "SWAP_GENERATOR", "USD_3ML" );
        EXPECT_FALSE( swapProperties.empty() );

        const etrading::VariantMatrix bondProperties = validation::tryAqGeneratorDisplay( "BOND_GENERATOR", "US_TREASURY_TYPE1" );
        EXPECT_FALSE( bondProperties.empty() );

        const etrading::VariantMatrix curveProperties = validation::tryAqGeneratorDisplay( "CURVE_GENERATOR", "USD_OIS" );
        EXPECT_FALSE( curveProperties.empty() );
    }

    TEST( TestAqGeneratorDisplay, UNIT_ThrowsOnUnknownGeneratorName )
    {
        EXPECT_THROW( validation::tryAqGeneratorDisplay( "SWAP_GENERATOR", "NOT_A_REAL_GENERATOR" ), std::exception );
    }
}
