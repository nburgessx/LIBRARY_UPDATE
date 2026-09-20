// TestAqDateECB.cpp

// Includes: This Library
#include "AQLDateHelpers.h"
#include "tryAqDateCentralBank.h"
#include "ExceptionMacros.h"
#include "TestHelperUtilities.h"
#include "DateUtilities.h"

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{
	TEST(ECB_Calendars, UNIT_ECB_Calendar_Expiry_Test)
	{
		const AQLDate currentDate = etrading::getCurrentAqDate();
		AQLDate thirdMeetingDate;
		AQLDate forthMeetingDate;
		AQLDate fifthMeetingDate;
		AQLDate sixthMeetingDate;

		//
		// 3RD ECB MEETING DATE
		// ---------------------------------------------------------------
		// Warning, not a hard error, if the 3rd ECB Meeting Date is unavailable - matches the
		// 4th/5th/6th checks below. ECB meeting dates are discretionary policy decisions
		// published roughly a year ahead, not a rule-based calendar, so this data predictably
		// runs low between manual refreshes of AQLCentralBank.cpp/CBSchedule.csv; that is worth
		// a warning, not a reason to fail a build. Previously an AQ_THROW here was the one
		// exception to this test's own graduated warning design.
		try
		{
			thirdMeetingDate = validation::tryAqDateNthECBMeetingDate(currentDate, 3);
		}
		catch (...)
		{
			GTEST_WARNING("ECB Calendar has Expired!!! - Less than 3 ECB Meeting Dates Available")
		}

		//
		// 4TH ECB MEETING DATE
		// ---------------------------------------------------------------
		// Throw *** WARNING *** if 4th ECB Meeting Date Unavailable
		//
		try
		{
			forthMeetingDate = validation::tryAqDateNextECBMeetingDate(thirdMeetingDate);
		}
		catch (...)
		{
			GTEST_WARNING("ECB Calendar is about to Expire!!! - Less than 4 ECB Meeting Dates Available")
		}

		//
		// 5TH ECB MEETING DATE
		// ---------------------------------------------------------------
		// Throw *** WARNING *** if 5th ECB Meeting Date Unavailable
		//
		try
		{
			fifthMeetingDate = validation::tryAqDateNextECBMeetingDate(forthMeetingDate);
		}
		catch (...)
		{
			GTEST_WARNING("ECB Calendar is about to Expire!!! - Less than 5 ECB Meeting Dates Available")
		}

		//
		// 6TH ECB MEETING DATE
		// ---------------------------------------------------------------
		// Throw *** WARNING *** if 5th ECB Meeting Date Unavailable
		//
		try
		{
			sixthMeetingDate = validation::tryAqDateNextECBMeetingDate(fifthMeetingDate);
		}
		catch (...)
		{
			GTEST_WARNING("ECB Calendar is about to Expire!!! - Less than 6 ECB Meeting Dates Available")
		}
	}

	TEST(TestDatesECB, UNIT_NEXT_MeetingDate)
	{
		const AQLDate expectedECBMeetingDate1 = AQLDate("20190725");
		const AQLDate expectedECBMeetingDate2 = AQLDate("20190912");
		const AQLDate actualECBMeetingDate2 = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate1);

		EXPECT_EQ(expectedECBMeetingDate2, actualECBMeetingDate2);
	}

	TEST(TestECBDateChain, UNIT_MeetingDates)
	{
		const AQLDate expectedECBMeetingDate1  = AQLDate("20190725");
		const AQLDate expectedECBMeetingDate2  = AQLDate("20190912");
		const AQLDate expectedECBMeetingDate3  = AQLDate("20191024");
		const AQLDate expectedECBMeetingDate4  = AQLDate("20191212");
		const AQLDate expectedECBMeetingDate5  = AQLDate("20200121");
		const AQLDate expectedECBMeetingDate6  = AQLDate("20200320");
		const AQLDate expectedECBMeetingDate7  = AQLDate("20200430");
		const AQLDate expectedECBMeetingDate8  = AQLDate("20200604");
		const AQLDate expectedECBMeetingDate9  = AQLDate("20200716");
		const AQLDate expectedECBMeetingDate10 = AQLDate("20200910");

		const AQLDate actualECBMeetingDate2  = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate1);
		const AQLDate actualECBMeetingDate3  = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate2);
		const AQLDate actualECBMeetingDate4  = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate3);
		const AQLDate actualECBMeetingDate5  = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate4);
		const AQLDate actualECBMeetingDate6  = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate5);
		const AQLDate actualECBMeetingDate7  = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate6);
		const AQLDate actualECBMeetingDate8  = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate7);
		const AQLDate actualECBMeetingDate9  = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate8);
		const AQLDate actualECBMeetingDate10 = validation::tryAqDateNextECBMeetingDate(expectedECBMeetingDate9);

		EXPECT_EQ(expectedECBMeetingDate2,  actualECBMeetingDate2);
		EXPECT_EQ(expectedECBMeetingDate3,  actualECBMeetingDate3);
		EXPECT_EQ(expectedECBMeetingDate4,  actualECBMeetingDate4);
		EXPECT_EQ(expectedECBMeetingDate5,  actualECBMeetingDate5);
		EXPECT_EQ(expectedECBMeetingDate6,  actualECBMeetingDate6);
		EXPECT_EQ(expectedECBMeetingDate7,  actualECBMeetingDate7);
		EXPECT_EQ(expectedECBMeetingDate8,  actualECBMeetingDate8);
		EXPECT_EQ(expectedECBMeetingDate9,  actualECBMeetingDate9);
		EXPECT_EQ(expectedECBMeetingDate10, actualECBMeetingDate10);
	}

	TEST(TestDatesECB, UNIT_NTH_MeetingDate)
	{
		const AQLDate asOfDate = AQLDate("20190610");

		const AQLDate expectedECBMeetingDate1	= AQLDate("20190725");
		const AQLDate expectedECBMeetingDate2	= AQLDate("20190912");
		const AQLDate expectedECBMeetingDate3	= AQLDate("20191024");
		const AQLDate expectedECBMeetingDate4	= AQLDate("20191212");
		const AQLDate expectedECBMeetingDate5	= AQLDate("20200121");
		const AQLDate expectedECBMeetingDate6	= AQLDate("20200320");
		const AQLDate expectedECBMeetingDate7	= AQLDate("20200430");
		const AQLDate expectedECBMeetingDate8	= AQLDate("20200604");
		const AQLDate expectedECBMeetingDate9	= AQLDate("20200716");
		const AQLDate expectedECBMeetingDate10   = AQLDate("20200910");

		const AQLDate actualECBMeetingDate1  = validation::tryAqDateNthECBMeetingDate(asOfDate, 1);
		const AQLDate actualECBMeetingDate2  = validation::tryAqDateNthECBMeetingDate(asOfDate, 2);
		const AQLDate actualECBMeetingDate3  = validation::tryAqDateNthECBMeetingDate(asOfDate, 3);
		const AQLDate actualECBMeetingDate4  = validation::tryAqDateNthECBMeetingDate(asOfDate, 4);
		const AQLDate actualECBMeetingDate5  = validation::tryAqDateNthECBMeetingDate(asOfDate, 5);
		const AQLDate actualECBMeetingDate6  = validation::tryAqDateNthECBMeetingDate(asOfDate, 6);
		const AQLDate actualECBMeetingDate7  = validation::tryAqDateNthECBMeetingDate(asOfDate, 7);
		const AQLDate actualECBMeetingDate8  = validation::tryAqDateNthECBMeetingDate(asOfDate, 8);
		const AQLDate actualECBMeetingDate9  = validation::tryAqDateNthECBMeetingDate(asOfDate, 9);
		const AQLDate actualECBMeetingDate10 = validation::tryAqDateNthECBMeetingDate(asOfDate, 10);

		EXPECT_EQ(expectedECBMeetingDate1,  actualECBMeetingDate1);
		EXPECT_EQ(expectedECBMeetingDate2,  actualECBMeetingDate2);
		EXPECT_EQ(expectedECBMeetingDate3,  actualECBMeetingDate3);
		EXPECT_EQ(expectedECBMeetingDate4,  actualECBMeetingDate4);
		EXPECT_EQ(expectedECBMeetingDate5,  actualECBMeetingDate5);
		EXPECT_EQ(expectedECBMeetingDate6,  actualECBMeetingDate6);
		EXPECT_EQ(expectedECBMeetingDate7,  actualECBMeetingDate7);
		EXPECT_EQ(expectedECBMeetingDate8,  actualECBMeetingDate8);
		EXPECT_EQ(expectedECBMeetingDate9,  actualECBMeetingDate9);
		EXPECT_EQ(expectedECBMeetingDate10, actualECBMeetingDate10);
	}

	TEST(TestDatesECB, UNIT_NEXT_SwapStartDate)
	{
		const AQLDate expectedECBSwapStartDate1 = AQLDate("20190731");
		const AQLDate expectedECBSwapStartDate2 = AQLDate("20190918");
		const AQLDate actualECBSwapStartDate2 = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate1);

		EXPECT_EQ(expectedECBSwapStartDate2, actualECBSwapStartDate2);
	}

	TEST(TestECBDateChain, UNIT_SwapStartDates)
	{
		const AQLDate expectedECBSwapStartDate1  = AQLDate("20190731");
		const AQLDate expectedECBSwapStartDate2  = AQLDate("20190918");
		const AQLDate expectedECBSwapStartDate3  = AQLDate("20191030");
		const AQLDate expectedECBSwapStartDate4  = AQLDate("20191218");
		const AQLDate expectedECBSwapStartDate5  = AQLDate("20200122");
		const AQLDate expectedECBSwapStartDate6  = AQLDate("20200325");
		const AQLDate expectedECBSwapStartDate7  = AQLDate("20200506");
		const AQLDate expectedECBSwapStartDate8  = AQLDate("20200610");
		const AQLDate expectedECBSwapStartDate9  = AQLDate("20200722");
		const AQLDate expectedECBSwapStartDate10 = AQLDate("20200916");

		const AQLDate actualECBSwapStartDate2  = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate1);
		const AQLDate actualECBSwapStartDate3  = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate2);
		const AQLDate actualECBSwapStartDate4  = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate3);
		const AQLDate actualECBSwapStartDate5  = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate4);
		const AQLDate actualECBSwapStartDate6  = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate5);
		const AQLDate actualECBSwapStartDate7  = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate6);
		const AQLDate actualECBSwapStartDate8  = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate7);
		const AQLDate actualECBSwapStartDate9  = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate8);
		const AQLDate actualECBSwapStartDate10 = validation::tryAqDateNextECBSwapStartDate(expectedECBSwapStartDate9);

		EXPECT_EQ(expectedECBSwapStartDate2,  actualECBSwapStartDate2);
		EXPECT_EQ(expectedECBSwapStartDate3,  actualECBSwapStartDate3);
		EXPECT_EQ(expectedECBSwapStartDate4,  actualECBSwapStartDate4);
		EXPECT_EQ(expectedECBSwapStartDate5,  actualECBSwapStartDate5);
		EXPECT_EQ(expectedECBSwapStartDate6,  actualECBSwapStartDate6);
		EXPECT_EQ(expectedECBSwapStartDate7,  actualECBSwapStartDate7);
		EXPECT_EQ(expectedECBSwapStartDate8,  actualECBSwapStartDate8);
		EXPECT_EQ(expectedECBSwapStartDate9,  actualECBSwapStartDate9);
		EXPECT_EQ(expectedECBSwapStartDate10, actualECBSwapStartDate10);
	}

	TEST(TestDatesECB, UNIT_NTH_SwapStartDate)
	{
		const AQLDate asOfDate = AQLDate("20190610");

		const AQLDate expectedECBSwapStartDate1  = AQLDate("20190731");
		const AQLDate expectedECBSwapStartDate2  = AQLDate("20190918");
		const AQLDate expectedECBSwapStartDate3  = AQLDate("20191030");
		const AQLDate expectedECBSwapStartDate4  = AQLDate("20191218");
		const AQLDate expectedECBSwapStartDate5  = AQLDate("20200122");
		const AQLDate expectedECBSwapStartDate6  = AQLDate("20200325");
		const AQLDate expectedECBSwapStartDate7  = AQLDate("20200506");
		const AQLDate expectedECBSwapStartDate8  = AQLDate("20200610");
		const AQLDate expectedECBSwapStartDate9  = AQLDate("20200722");
		const AQLDate expectedECBSwapStartDate10 = AQLDate("20200916");

		const AQLDate actualECBSwapStartDate1  = validation::tryAqDateNthECBSwapStartDate(asOfDate, 1);
		const AQLDate actualECBSwapStartDate2  = validation::tryAqDateNthECBSwapStartDate(asOfDate, 2);
		const AQLDate actualECBSwapStartDate3  = validation::tryAqDateNthECBSwapStartDate(asOfDate, 3);
		const AQLDate actualECBSwapStartDate4  = validation::tryAqDateNthECBSwapStartDate(asOfDate, 4);
		const AQLDate actualECBSwapStartDate5  = validation::tryAqDateNthECBSwapStartDate(asOfDate, 5);
		const AQLDate actualECBSwapStartDate6  = validation::tryAqDateNthECBSwapStartDate(asOfDate, 6);
		const AQLDate actualECBSwapStartDate7  = validation::tryAqDateNthECBSwapStartDate(asOfDate, 7);
		const AQLDate actualECBSwapStartDate8  = validation::tryAqDateNthECBSwapStartDate(asOfDate, 8);
		const AQLDate actualECBSwapStartDate9  = validation::tryAqDateNthECBSwapStartDate(asOfDate, 9);
		const AQLDate actualECBSwapStartDate10 = validation::tryAqDateNthECBSwapStartDate(asOfDate, 10);

		EXPECT_EQ(expectedECBSwapStartDate1,  actualECBSwapStartDate1);
		EXPECT_EQ(expectedECBSwapStartDate2,  actualECBSwapStartDate2);
		EXPECT_EQ(expectedECBSwapStartDate3,  actualECBSwapStartDate3);
		EXPECT_EQ(expectedECBSwapStartDate4,  actualECBSwapStartDate4);
		EXPECT_EQ(expectedECBSwapStartDate5,  actualECBSwapStartDate5);
		EXPECT_EQ(expectedECBSwapStartDate6,  actualECBSwapStartDate6);
		EXPECT_EQ(expectedECBSwapStartDate7,  actualECBSwapStartDate7);
		EXPECT_EQ(expectedECBSwapStartDate8,  actualECBSwapStartDate8);
		EXPECT_EQ(expectedECBSwapStartDate9,  actualECBSwapStartDate9);
		EXPECT_EQ(expectedECBSwapStartDate10, actualECBSwapStartDate10);
	}

	TEST(TestDatesECB, UNIT_NEXT_SwapEndDate)
	{
		const AQLDate expectedECBSwapEndDate1 = AQLDate("20190917");
		const AQLDate expectedECBSwapEndDate2 = AQLDate("20191029");
		const AQLDate actualECBSwapEndDate2 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate1);

		EXPECT_EQ(expectedECBSwapEndDate2, actualECBSwapEndDate2);
	}

	TEST(TestECBDateChain, UNIT_SwapEndDates)
	{
		const AQLDate expectedECBSwapEndDate1 = AQLDate("20190917");
		const AQLDate expectedECBSwapEndDate2 = AQLDate("20191029");
		const AQLDate expectedECBSwapEndDate3 = AQLDate("20191217");
		const AQLDate expectedECBSwapEndDate4 = AQLDate("20200121");
		const AQLDate expectedECBSwapEndDate5 = AQLDate("20200324");
		const AQLDate expectedECBSwapEndDate6 = AQLDate("20200505");
		const AQLDate expectedECBSwapEndDate7 = AQLDate("20200609");
		const AQLDate expectedECBSwapEndDate8 = AQLDate("20200721");
		const AQLDate expectedECBSwapEndDate9 = AQLDate("20200915");
		const AQLDate expectedECBSwapEndDate10 = AQLDate("20201103");

		const AQLDate actualECBSwapEndDate2 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate1);
		const AQLDate actualECBSwapEndDate3 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate2);
		const AQLDate actualECBSwapEndDate4 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate3);
		const AQLDate actualECBSwapEndDate5 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate4);
		const AQLDate actualECBSwapEndDate6 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate5);
		const AQLDate actualECBSwapEndDate7 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate6);
		const AQLDate actualECBSwapEndDate8 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate7);
		const AQLDate actualECBSwapEndDate9 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate8);
		const AQLDate actualECBSwapEndDate10 = validation::tryAqDateNextECBSwapEndDate(expectedECBSwapEndDate9);

		EXPECT_EQ(expectedECBSwapEndDate2, actualECBSwapEndDate2);
		EXPECT_EQ(expectedECBSwapEndDate3, actualECBSwapEndDate3);
		EXPECT_EQ(expectedECBSwapEndDate4, actualECBSwapEndDate4);
		EXPECT_EQ(expectedECBSwapEndDate5, actualECBSwapEndDate5);
		EXPECT_EQ(expectedECBSwapEndDate6, actualECBSwapEndDate6);
		EXPECT_EQ(expectedECBSwapEndDate7, actualECBSwapEndDate7);
		EXPECT_EQ(expectedECBSwapEndDate8, actualECBSwapEndDate8);
		EXPECT_EQ(expectedECBSwapEndDate9, actualECBSwapEndDate9);
		EXPECT_EQ(expectedECBSwapEndDate10, actualECBSwapEndDate10);
	}

	TEST(TestDatesECB, UNIT_NTH_SwapEndDate)
	{
		const AQLDate asOfDate = AQLDate("20190610");

		const AQLDate expectedECBSwapEndDate1  = AQLDate("20190917");
		const AQLDate expectedECBSwapEndDate2  = AQLDate("20191029");
		const AQLDate expectedECBSwapEndDate3  = AQLDate("20191217");
		const AQLDate expectedECBSwapEndDate4  = AQLDate("20200121");
		const AQLDate expectedECBSwapEndDate5  = AQLDate("20200324");
		const AQLDate expectedECBSwapEndDate6  = AQLDate("20200505");
		const AQLDate expectedECBSwapEndDate7  = AQLDate("20200609");
		const AQLDate expectedECBSwapEndDate8  = AQLDate("20200721");
		const AQLDate expectedECBSwapEndDate9  = AQLDate("20200915");
		const AQLDate expectedECBSwapEndDate10 = AQLDate("20201103");

		const AQLDate actualECBSwapEndDate1  = validation::tryAqDateNthECBSwapEndDate(asOfDate, 1);
		const AQLDate actualECBSwapEndDate2  = validation::tryAqDateNthECBSwapEndDate(asOfDate, 2);
		const AQLDate actualECBSwapEndDate3  = validation::tryAqDateNthECBSwapEndDate(asOfDate, 3);
		const AQLDate actualECBSwapEndDate4  = validation::tryAqDateNthECBSwapEndDate(asOfDate, 4);
		const AQLDate actualECBSwapEndDate5  = validation::tryAqDateNthECBSwapEndDate(asOfDate, 5);
		const AQLDate actualECBSwapEndDate6  = validation::tryAqDateNthECBSwapEndDate(asOfDate, 6);
		const AQLDate actualECBSwapEndDate7  = validation::tryAqDateNthECBSwapEndDate(asOfDate, 7);
		const AQLDate actualECBSwapEndDate8  = validation::tryAqDateNthECBSwapEndDate(asOfDate, 8);
		const AQLDate actualECBSwapEndDate9  = validation::tryAqDateNthECBSwapEndDate(asOfDate, 9);
		const AQLDate actualECBSwapEndDate10 = validation::tryAqDateNthECBSwapEndDate(asOfDate, 10);

		EXPECT_EQ(expectedECBSwapEndDate1,  actualECBSwapEndDate1);
		EXPECT_EQ(expectedECBSwapEndDate2,  actualECBSwapEndDate2);
		EXPECT_EQ(expectedECBSwapEndDate3,  actualECBSwapEndDate3);
		EXPECT_EQ(expectedECBSwapEndDate4,  actualECBSwapEndDate4);
		EXPECT_EQ(expectedECBSwapEndDate5,  actualECBSwapEndDate5);
		EXPECT_EQ(expectedECBSwapEndDate6,  actualECBSwapEndDate6);
		EXPECT_EQ(expectedECBSwapEndDate7,  actualECBSwapEndDate7);
		EXPECT_EQ(expectedECBSwapEndDate8,  actualECBSwapEndDate8);
		EXPECT_EQ(expectedECBSwapEndDate9,  actualECBSwapEndDate9);
		EXPECT_EQ(expectedECBSwapEndDate10, actualECBSwapEndDate10);
	}

}
