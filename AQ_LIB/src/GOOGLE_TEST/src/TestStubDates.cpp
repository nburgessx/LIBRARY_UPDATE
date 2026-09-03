// TestStubDates.cpp

// Initialize Google Test Library and include 'DECLARE TEST FIXTURE'
#include "InitializeAQGoogleTest.h"

// Includes
#include "LAMathDateUtilities.h"
#include "LAPriceDataSlidingRule.h"

// Namespaces
using namespace etrading;

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestStubDates);

	TEST_F( TestStubDates, UNIT_StubOnly_11Days_ShortStart )
    {
		const LADate startDate("20160602");
		const LADate endDate("20160611");
		LAString term = "1Y";
		
		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160613");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160611");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160613");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160611");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160611");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160611");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160613");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160609");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160608");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170606");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160606");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170606");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170606");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170606");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170606");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170606");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170606");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160608");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170608");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170608");
		LAString term = "1Y";
		
		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170608");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160609");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170609");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170609");
		LAString term = "1Y";
		
		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160609");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160610");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170612");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160610");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210608");
		LAString term = "1Y";
		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160608");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170608");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20210602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20200602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170608");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210609");
		LAString term = "1Y";
		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160609");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210608");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170608");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20210602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20200602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160609");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160610");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20170612");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20210602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20200602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = NONE_STUBTYPE;
		const LAString* rollConvention = NULL;

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, rollConvention, stubType );

		LADate expectedDate("20160610");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160610");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160610");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160615");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20170612");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160615");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20170621");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210711");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20210616");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210711");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString rollConvention = "IMM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20200617");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160610");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160610");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160630");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20170612");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20170610");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160602");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "EOm";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160630");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210609");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20170630");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210711");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20210630");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20210711");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString rollConvention = "EOM";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20200630");
		LADate actualDate = stubDateAndType.stubDate_;

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
		const LADate startDate("20160602");
		const LADate endDate("20160702");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160704");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCoupon_RegularSchedule )
    {
		const LADate startDate("20160602");
		const LADate endDate("20170602");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20170602");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( true, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_SingleCouponWithStub_IrregularSchedule )
    {
		const LADate startDate("20160602");
		const LADate endDate("20170702");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160704");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_RegularSchedule )
    {
		const LADate startDate("20160602");
		const LADate endDate("20210602");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20170602");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( true, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_IrregularSchedule )
    {
		const LADate startDate("20160602");
		const LADate endDate("20210702");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		LADate expectedDate("20160704");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Check isRegularSchedule
		EXPECT_EQ( false, stubDateAndType.isRegularSchedule_ );
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_NoStub_DateSchedule )
    {
		const LADate startDate("20160602");
		const LADate endDate("20210602");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		LADate expectedDate("20170602");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<LADate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<LADate> expectedSchedule = { LADate("20160602"),
												 LADate("20170602"),
												 LADate("20180602"),
												 LADate("20190602"),
												 LADate("20200602"),
												 LADate("20210602") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_ShortStart_DateSchedule )
    {
		const LADate startDate("20160602");
		const LADate endDate("20210702");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_START_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		LADate expectedDate("20160704");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<LADate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<LADate> expectedSchedule = { LADate("20160602"),
												 LADate("20160702"),
												 LADate("20170702"),
												 LADate("20180702"),
												 LADate("20190702"),
												 LADate("20200702"),
												 LADate("20210702") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_LongStart_DateSchedule )
    {
		const LADate startDate("20160602");
		const LADate endDate("20210702");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_START_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		LADate expectedDate("20170703");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<LADate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<LADate> expectedSchedule = { LADate("20160602"),
												 LADate("20170702"),
												 LADate("20180702"),
												 LADate("20190702"),
												 LADate("20200702"),
												 LADate("20210702") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_ShortEnd_DateSchedule )
    {
		const LADate startDate("20160602");
		const LADate endDate("20210702");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = SHORT_END_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		LADate expectedDate("20210602");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<LADate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<LADate> expectedSchedule = { LADate("20160602"),
												 LADate("20170602"),
												 LADate("20180602"),
												 LADate("20190602"),
												 LADate("20200602"),
												 LADate("20210602"),
												 LADate("20210702") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}

	TEST_F( TestStubDates, UNIT_MutlipleCoupons_LongEnd_DateSchedule )
    {
		const LADate startDate("20160602");
		const LADate endDate("20210702");
		LAString term = "1Y";

		const LAPriceDataSlidingRule busDayAdj(SLIDING_RULE_MOD_FOLLOWING);
		const LAPriceDataCalendar calendar(LAStringVector(1,"TGT"));
		const StubTypeEnum stubType = LONG_END_STUBTYPE;
		const LAString rollConvention = "NORMAL";

		StubDateAndType stubDateAndType = LAMathDateUtilities::getStubDateAndType( startDate, endDate, term, busDayAdj, calendar, &rollConvention, stubType );

		// Adjusted
		LADate expectedDate("20200602");
		LADate actualDate = stubDateAndType.stubDate_;

		// Stub Date Check
		EXPECT_EQ( expectedDate, actualDate );

		// Unadjusted Coupon Date Schedule
		std::vector<LADate> actualSchedule = stubDateAndType.unadjustedSchedule_;
		std::vector<LADate> expectedSchedule = { LADate("20160602"),
												 LADate("20170602"),
												 LADate("20180602"),
												 LADate("20190602"),
												 LADate("20200602"),
												 LADate("20210702") };
		
		ASSERT_EQ( expectedSchedule.size(), actualSchedule.size() );
		for ( size_t i = 0; i < actualSchedule.size(); ++i )
		{
			EXPECT_EQ( expectedSchedule[i], actualSchedule[i] );
		}
	}
}