// TestDatesECB.cpp

// Includes: This Library
#include "LAMathCentralBank.h"
#include "LADateHelpers.h"

#include "tryMirGetNextECBDate.h"
#include "tryMirGetECBStartDate.h"

#include "LACoreAppError.h"

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{
	 
    TEST( TestDatesECB, UNIT_ErrorHandling )
    {
        const LADate farAway( "21151008" );
        EXPECT_THROW( validation_api::tryMirGetNextECBDate( farAway ), LACoreError );
    }
	  
    TEST( TestDatesECB, UNIT_haveNextECBDate )
    {
        const LADate today( "20151008" );
        EXPECT_EQ(etrading::LADateHelpers::haveNextECBDate( today ), true );

        const LADate ecbLast = LAMathCentralBank::meetingSchedule( "ecb" ).back();
        EXPECT_EQ(etrading::LADateHelpers::haveNextECBDate( ecbLast ), false );
        EXPECT_EQ(etrading::LADateHelpers::haveNextECBDate( ecbLast, false ), true );

        const LADate farAway( "21151008" );
        EXPECT_EQ(etrading::LADateHelpers::haveNextECBDate( farAway ), false );
    }

    TEST( TestDatesECB, UNIT_getNextECBDate )
    {
        const LADate ecb1( "20150122" );
        const LADate ecb2( "20150305" );
        const LADate ecb3( "20150415" );

        const LADate d0( "20141231" );
        const LADate d1( "20150124" );

        EXPECT_EQ( validation_api::tryMirGetNextECBDate( d0 ), ecb1 );
        EXPECT_EQ( validation_api::tryMirGetNextECBDate( d0, false ), ecb1 );

        EXPECT_EQ( validation_api::tryMirGetNextECBDate( ecb1, false ), ecb1 );
        EXPECT_EQ( validation_api::tryMirGetNextECBDate( ecb1 ), ecb2 );

        EXPECT_EQ( validation_api::tryMirGetNextECBDate( d1 ), ecb2 );
        EXPECT_EQ( validation_api::tryMirGetNextECBDate( d1, false ), ecb2 );

        const LADate ecbLast = LAMathCentralBank::meetingSchedule( "ECB" ).back();
        EXPECT_EQ( validation_api::tryMirGetNextECBDate( ecbLast, false ), ecbLast );

        EXPECT_THROW( validation_api::tryMirGetNextECBDate( ecbLast ), LACoreError );
    }

    TEST( TestDatesECB, UNIT_getECBStartDate )
    {
        const LADate ecb1( "20160121" );		// Thursday
        const LADate start1( "20160127" );	// Wednesday after
        EXPECT_EQ( validation_api::tryMirGetECBStartDate( ecb1 ), start1 );

        const LADate ecb2( "20150415" );		// a Wednesday
        const LADate start2( "20150422" );	// a week later
        EXPECT_EQ( validation_api::tryMirGetECBStartDate( ecb2 ), start2 );

        const LADate ecb3( "20150416" );		// Thursday
        const LADate start3( "20150422" );	// Wednesday after
        EXPECT_EQ( validation_api::tryMirGetECBStartDate( ecb3 ), start3 );

        const LADate ecb4( "20150417" );		// Friday
        const LADate start4( "20150422" );	// Wednesday after
        EXPECT_EQ( validation_api::tryMirGetECBStartDate( ecb4 ), start4 );

        const LADate ecb5( "20150418" );		// Saturday
        const LADate start5( "20150422" );	// Wednesday after
        EXPECT_EQ( validation_api::tryMirGetECBStartDate( ecb5 ), start5 );

        const LADate ecb6( "20150419" );		// Sunday
        const LADate start6( "20150422" );	// Wednesday after
        EXPECT_EQ( validation_api::tryMirGetECBStartDate( ecb6 ), start6 );

        const LADate ecb7( "20150420" );		// Monday
        const LADate start7( "20150422" );	// Wednesday after
        EXPECT_EQ( validation_api::tryMirGetECBStartDate( ecb7 ), start7 );

        const LADate ecb8( "20150421" );		// Tuesday
        const LADate start8( "20150422" );	// Wednesday after
        EXPECT_EQ( validation_api::tryMirGetECBStartDate( ecb8 ), start8 );

        const LADate ecb9( "20150422" );		// Wednesday
        const LADate start9( "20150429" );	// a week later
        EXPECT_EQ( validation_api::tryMirGetECBStartDate( ecb9 ), start9 );
    }
}
