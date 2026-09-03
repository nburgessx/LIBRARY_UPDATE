// TestDatesECB.cpp

// Includes: This Library
#include "LAMathCentralBank.h"
#include "LADateHelpers.h"

#include "tryMirGetNextECBDate.h"
#include "tryMirGetECBStartDate.h"

#include "AQLCoreAppError.h"

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{
	 
    TEST( TestDatesECB, UNIT_ErrorHandling )
    {
        const AQLDate farAway( "21151008" );
        EXPECT_THROW( validation::tryMirGetNextECBDate( farAway ), AQLCoreError );
    }
	  
    TEST( TestDatesECB, UNIT_haveNextECBDate )
    {
        const AQLDate today( "20151008" );
        EXPECT_EQ(etrading::LADateHelpers::haveNextECBDate( today ), true );

        const AQLDate ecbLast = LAMathCentralBank::meetingSchedule( "ecb" ).back();
        EXPECT_EQ(etrading::LADateHelpers::haveNextECBDate( ecbLast ), false );
        EXPECT_EQ(etrading::LADateHelpers::haveNextECBDate( ecbLast, false ), true );

        const AQLDate farAway( "21151008" );
        EXPECT_EQ(etrading::LADateHelpers::haveNextECBDate( farAway ), false );
    }

    TEST( TestDatesECB, UNIT_getNextECBDate )
    {
        const AQLDate ecb1( "20150122" );
        const AQLDate ecb2( "20150305" );
        const AQLDate ecb3( "20150415" );

        const AQLDate d0( "20141231" );
        const AQLDate d1( "20150124" );

        EXPECT_EQ( validation::tryMirGetNextECBDate( d0 ), ecb1 );
        EXPECT_EQ( validation::tryMirGetNextECBDate( d0, false ), ecb1 );

        EXPECT_EQ( validation::tryMirGetNextECBDate( ecb1, false ), ecb1 );
        EXPECT_EQ( validation::tryMirGetNextECBDate( ecb1 ), ecb2 );

        EXPECT_EQ( validation::tryMirGetNextECBDate( d1 ), ecb2 );
        EXPECT_EQ( validation::tryMirGetNextECBDate( d1, false ), ecb2 );

        const AQLDate ecbLast = LAMathCentralBank::meetingSchedule( "ECB" ).back();
        EXPECT_EQ( validation::tryMirGetNextECBDate( ecbLast, false ), ecbLast );

        EXPECT_THROW( validation::tryMirGetNextECBDate( ecbLast ), AQLCoreError );
    }

    TEST( TestDatesECB, UNIT_getECBStartDate )
    {
        const AQLDate ecb1( "20160121" );		// Thursday
        const AQLDate start1( "20160127" );	// Wednesday after
        EXPECT_EQ( validation::tryMirGetECBStartDate( ecb1 ), start1 );

        const AQLDate ecb2( "20150415" );		// a Wednesday
        const AQLDate start2( "20150422" );	// a week later
        EXPECT_EQ( validation::tryMirGetECBStartDate( ecb2 ), start2 );

        const AQLDate ecb3( "20150416" );		// Thursday
        const AQLDate start3( "20150422" );	// Wednesday after
        EXPECT_EQ( validation::tryMirGetECBStartDate( ecb3 ), start3 );

        const AQLDate ecb4( "20150417" );		// Friday
        const AQLDate start4( "20150422" );	// Wednesday after
        EXPECT_EQ( validation::tryMirGetECBStartDate( ecb4 ), start4 );

        const AQLDate ecb5( "20150418" );		// Saturday
        const AQLDate start5( "20150422" );	// Wednesday after
        EXPECT_EQ( validation::tryMirGetECBStartDate( ecb5 ), start5 );

        const AQLDate ecb6( "20150419" );		// Sunday
        const AQLDate start6( "20150422" );	// Wednesday after
        EXPECT_EQ( validation::tryMirGetECBStartDate( ecb6 ), start6 );

        const AQLDate ecb7( "20150420" );		// Monday
        const AQLDate start7( "20150422" );	// Wednesday after
        EXPECT_EQ( validation::tryMirGetECBStartDate( ecb7 ), start7 );

        const AQLDate ecb8( "20150421" );		// Tuesday
        const AQLDate start8( "20150422" );	// Wednesday after
        EXPECT_EQ( validation::tryMirGetECBStartDate( ecb8 ), start8 );

        const AQLDate ecb9( "20150422" );		// Wednesday
        const AQLDate start9( "20150429" );	// a week later
        EXPECT_EQ( validation::tryMirGetECBStartDate( ecb9 ), start9 );
    }
}
