// TestStubDates.cpp

// Initialize Google Test Library and include 'DECLARE TEST FIXTURE'
#include "InitializeGoogleTest.h"

// Includes
#include "AQLMathDateUtilities.h"
#include "AQLPriceDataSlidingRule.h"

// Namespaces
using namespace etrading;

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestStubDates);

	TEST_F( TestStubDates, UNIT_StubOnly_11Days_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160611");
		AQLString term = "1Y";
		
		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160613");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
		
	}

	TEST_F( TestStubDates, UNIT_StubOnly_11Days_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160611");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160613");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_11Days_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160611");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_11Days_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160611");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_11Days_DefaultShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160611");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160613");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_7Days_DefaultShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160609");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_6Days_DefaultLongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160608");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With4DayStub_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170606");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160606");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With4DayStub_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170606");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170606");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With4DayStub_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170606");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With4DayStub_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170606");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With4DayStub_DefaultToLongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170606");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170606");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With6DayStub_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160608");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With6DayStub_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170608");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With6DayStub_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With6DayStub_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With6DayStub_DefaultToLongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170608");
		AQLString term = "1Y";
		
		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170608");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With7DayStub_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160609");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With7DayStub_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170609");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With7DayStub_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With7DayStub_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With7DayStub_DefaultToShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170609");
		AQLString term = "1Y";
		
		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160609");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With8DayStub_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160610");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With8DayStub_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170612");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With8DayStub_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With8DayStub_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_With8DayStub_DefaultToShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160610");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With6DayStub_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210608");
		AQLString term = "1Y";
		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160608");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With6DayStub_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170608");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With6DayStub_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20210602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With6DayStub_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20200602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With6DayStub_DefaultToLongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170608");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With7DayStub_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210609");
		AQLString term = "1Y";
		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160609");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With7DayStub_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210608");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170608");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With7DayStub_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20210602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With7DayStub_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20200602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With7DayStub_DefaultToShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160609");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With8DayStub_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160610");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With8DayStub_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20170612");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With8DayStub_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20210602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With8DayStub_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20200602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCoupons_With8DayStub_DefaultToShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const AQLString* rollConvention = NULL;

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		AQLDate expectedDate("20160610");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( true, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_IMM_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160610");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_IMM_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160610");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_IMM_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_IMM_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponPlusStub_IMM_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160615");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponPlusStub_IMM_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20170612");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponPlusStub_IMM_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponPlusStub_IMM_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCouponsPlusStub_IMM_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160615");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCouponsPlusStub_IMM_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20170621");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCouponsPlusStub_IMM_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210711");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20210616");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCouponsPlusStub_IMM_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210711");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString rollConvention = "IMM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20200617");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_EOM_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160610");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_EOM_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160610");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_EOM_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_EOM_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponPlusStub_EOM_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160630");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponPlusStub_EOM_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20170612");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponPlusStub_EOM_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponPlusStub_EOM_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170610");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCouponsPlusStub_EOM_ShortStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "EOm";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160630");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCouponsPlusStub_EOM_LongStart )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210609");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20170630");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_START_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( true, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( false, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCouponsPlusStub_EOM_ShortEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210711");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20210630");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::SHORT_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( true, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MultipleCouponsPlusStub_EOM_LongEnd )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210711");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString rollConvention = "EOM";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20200630");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Stub Data Checks
		EXPECT_EQ( etrading::LONG_END_STUBTYPE, stubDateAndType.stubTypeEnum_ );
		EXPECT_EQ( false, stubDateAndType.usingDefaultStub_ );
		EXPECT_EQ( false, stubDateAndType.isFrontStub_ );
		EXPECT_EQ( false, stubDateAndType.isShortStub_ );
		EXPECT_EQ( true, stubDateAndType.isStartRoll_ );
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_StubOnly_IregularSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20160702");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160704");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_RegularSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170602");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20170602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( true, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponWithStub_IrregularSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20170702");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160704");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_RegularSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210602");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20170602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( true, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_IrregularSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210702");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		AQLDate expectedDate("20160704");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_NoStub_DateSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210602");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		AQLDate expectedDate("20170602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<AQLDate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<AQLDate> expectedSchedule = { AQLDate("20160602"),
												 AQLDate("20170602"),
												 AQLDate("20180602"),
												 AQLDate("20190602"),
												 AQLDate("20200602"),
												 AQLDate("20210602") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_ShortStart_DateSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210702");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		AQLDate expectedDate("20160704");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<AQLDate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<AQLDate> expectedSchedule = { AQLDate("20160602"),
												 AQLDate("20160702"),
												 AQLDate("20170702"),
												 AQLDate("20180702"),
												 AQLDate("20190702"),
												 AQLDate("20200702"),
												 AQLDate("20210702") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_LongStart_DateSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210702");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		AQLDate expectedDate("20170703");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<AQLDate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<AQLDate> expectedSchedule = { AQLDate("20160602"),
												 AQLDate("20170702"),
												 AQLDate("20180702"),
												 AQLDate("20190702"),
												 AQLDate("20200702"),
												 AQLDate("20210702") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_ShortEnd_DateSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210702");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		AQLDate expectedDate("20210602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<AQLDate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<AQLDate> expectedSchedule = { AQLDate("20160602"),
												 AQLDate("20170602"),
												 AQLDate("20180602"),
												 AQLDate("20190602"),
												 AQLDate("20200602"),
												 AQLDate("20210602"),
												 AQLDate("20210702") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_LongEnd_DateSchedule )
    {
		const AQLDate startDate("20160602");
		const AQLDate endDate("20210702");
		AQLString term = "1Y";

		const AQLPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const AQLPriceDataCalendar calendar(AQLStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const AQLString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = AQLMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		AQLDate expectedDate("20200602");
		AQLDate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<AQLDate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<AQLDate> expectedSchedule = { AQLDate("20160602"),
												 AQLDate("20170602"),
												 AQLDate("20180602"),
												 AQLDate("20190602"),
												 AQLDate("20200602"),
												 AQLDate("20210702") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}
}