// Include: Google Test Library
#include <gTest/gTest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "tryAqGenerator.h"

namespace google_test
{
    TEST( TestAqGeneratorList, UNIT_ListsGeneratorsOfEachType )
    {
        // Swap, Bond and Curve generators all ship real seed files under
        // resources\config - assert against files known to be there rather
        // than a fixed count, so adding more seeds does not break this test.
        const std::vector<std::string> swapGenerators = validation::tryAqGeneratorList( "SWAP_GENERATOR" );
        EXPECT_FALSE( swapGenerators.empty() );
        EXPECT_NE( std::find( swapGenerators.cbegin(), swapGenerators.cend(), "USD_3ML" ), swapGenerators.cend() );

        const std::vector<std::string> bondGenerators = validation::tryAqGeneratorList( "BOND_GENERATOR" );
        EXPECT_FALSE( bondGenerators.empty() );
        EXPECT_NE( std::find( bondGenerators.cbegin(), bondGenerators.cend(), "US_TREASURY_TYPE1" ), bondGenerators.cend() );

        const std::vector<std::string> curveGenerators = validation::tryAqGeneratorList( "CURVE_GENERATOR" );
        EXPECT_FALSE( curveGenerators.empty() );
        EXPECT_NE( std::find( curveGenerators.cbegin(), curveGenerators.cend(), "USD_OIS" ), curveGenerators.cend() );
    }

    TEST( TestAqGeneratorList, UNIT_RejectsNonGeneratorType )
    {
        EXPECT_THROW( validation::tryAqGeneratorList( "SWAP_OBJECT" ), std::exception );
    }
}
