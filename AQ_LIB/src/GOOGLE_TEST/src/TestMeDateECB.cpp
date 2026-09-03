// TestMeDateECB.cpp

// Includes: This Library
#include "LADateHelpers.h"
#include "tryMeDateCentralBank.h"
#include "ExceptionMacros.h"
#include "TestHelperUtilities.h"
#include "DateUtilities.h"

// Include: Google Test Library
#include <gTest/gTest.h>

namespace google_test
{
	TEST(ECB_Calendars, UNIT_ECB_Calendar_Expiry_Test)
	{
		const AQLDate currentDate = etrading::getCurrentMLibDate();
		AQLDate thirdMeetingDate;
		AQLDate forthMeetingDate;
		AQLDate fifthMeetingDate;
		AQLDate sixthMeetingDate;

		//
		// 3RD ECB MEETING DATE
		// ---------------------------------------------------------------
		// Throw ***ERROR *** if 3rd ECB Meeting Date Unavailable
		try
		{
			thirdMeetingDate = validation::tryMeDateNthECBMeetingDate(currentDate, 3);
		}
		catch (...)
		{
			AQ_THROW("ECB Calendar has Expired!!! - Less than 3 ECB Meeting Dates Available")
		}

		//
		// 4TH ECB MEETING DATE
		// ---------------------------------------------------------------
		// Throw *** WARNING *** if 4th ECB Meeting Date Unavailable
		//
		try
		{
			forthMeetingDate = validation::tryMeDateNextECBMeetingDate(thirdMeetingDate);
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
			fifthMeetingDate = validation::tryMeDateNextECBMeetingDate(forthMeetingDate);
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
			sixthMeetingDate = validation::tryMeDateNextECBMeetingDate(fifthMeetingDate);
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
		const AQLDate actualECBMeetingDate2 = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate1);

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

		const AQLDate actualECBMeetingDate2  = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate1);
		const AQLDate actualECBMeetingDate3  = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate2);
		const AQLDate actualECBMeetingDate4  = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate3);
		const AQLDate actualECBMeetingDate5  = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate4);
		const AQLDate actualECBMeetingDate6  = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate5);
		const AQLDate actualECBMeetingDate7  = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate6);
		const AQLDate actualECBMeetingDate8  = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate7);
		const AQLDate actualECBMeetingDate9  = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate8);
		const AQLDate actualECBMeetingDate10 = validation::tryMeDateNextECBMeetingDate(expectedECBMeetingDate9);

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

		const AQLDate actualECBMeetingDate1  = validation::tryMeDateNthECBMeetingDate(asOfDate, 1);
		const AQLDate actualECBMeetingDate2  = validation::tryMeDateNthECBMeetingDate(asOfDate, 2);
		const AQLDate actualECBMeetingDate3  = validation::tryMeDateNthECBMeetingDate(asOfDate, 3);
		const AQLDate actualECBMeetingDate4  = validation::tryMeDateNthECBMeetingDate(asOfDate, 4);
		const AQLDate actualECBMeetingDate5  = validation::tryMeDateNthECBMeetingDate(asOfDate, 5);
		const AQLDate actualECBMeetingDate6  = validation::tryMeDateNthECBMeetingDate(asOfDate, 6);
		const AQLDate actualECBMeetingDate7  = validation::tryMeDateNthECBMeetingDate(asOfDate, 7);
		const AQLDate actualECBMeetingDate8  = validation::tryMeDateNthECBMeetingDate(asOfDate, 8);
		const AQLDate actualECBMeetingDate9  = validation::tryMeDateNthECBMeetingDate(asOfDate, 9);
		const AQLDate actualECBMeetingDate10 = validation::tryMeDateNthECBMeetingDate(asOfDate, 10);

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
		const AQLDate actualECBSwapStartDate2 = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate1);

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

		const AQLDate actualECBSwapStartDate2  = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate1);
		const AQLDate actualECBSwapStartDate3  = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate2);
		const AQLDate actualECBSwapStartDate4  = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate3);
		const AQLDate actualECBSwapStartDate5  = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate4);
		const AQLDate actualECBSwapStartDate6  = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate5);
		const AQLDate actualECBSwapStartDate7  = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate6);
		const AQLDate actualECBSwapStartDate8  = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate7);
		const AQLDate actualECBSwapStartDate9  = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate8);
		const AQLDate actualECBSwapStartDate10 = validation::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate9);

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

		const AQLDate actualECBSwapStartDate1  = validation::tryMeDateNthECBSwapStartDate(asOfDate, 1);
		const AQLDate actualECBSwapStartDate2  = validation::tryMeDateNthECBSwapStartDate(asOfDate, 2);
		const AQLDate actualECBSwapStartDate3  = validation::tryMeDateNthECBSwapStartDate(asOfDate, 3);
		const AQLDate actualECBSwapStartDate4  = validation::tryMeDateNthECBSwapStartDate(asOfDate, 4);
		const AQLDate actualECBSwapStartDate5  = validation::tryMeDateNthECBSwapStartDate(asOfDate, 5);
		const AQLDate actualECBSwapStartDate6  = validation::tryMeDateNthECBSwapStartDate(asOfDate, 6);
		const AQLDate actualECBSwapStartDate7  = validation::tryMeDateNthECBSwapStartDate(asOfDate, 7);
		const AQLDate actualECBSwapStartDate8  = validation::tryMeDateNthECBSwapStartDate(asOfDate, 8);
		const AQLDate actualECBSwapStartDate9  = validation::tryMeDateNthECBSwapStartDate(asOfDate, 9);
		const AQLDate actualECBSwapStartDate10 = validation::tryMeDateNthECBSwapStartDate(asOfDate, 10);

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
		const AQLDate actualECBSwapEndDate2 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate1);

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

		const AQLDate actualECBSwapEndDate2 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate1);
		const AQLDate actualECBSwapEndDate3 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate2);
		const AQLDate actualECBSwapEndDate4 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate3);
		const AQLDate actualECBSwapEndDate5 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate4);
		const AQLDate actualECBSwapEndDate6 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate5);
		const AQLDate actualECBSwapEndDate7 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate6);
		const AQLDate actualECBSwapEndDate8 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate7);
		const AQLDate actualECBSwapEndDate9 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate8);
		const AQLDate actualECBSwapEndDate10 = validation::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate9);

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

		const AQLDate actualECBSwapEndDate1  = validation::tryMeDateNthECBSwapEndDate(asOfDate, 1);
		const AQLDate actualECBSwapEndDate2  = validation::tryMeDateNthECBSwapEndDate(asOfDate, 2);
		const AQLDate actualECBSwapEndDate3  = validation::tryMeDateNthECBSwapEndDate(asOfDate, 3);
		const AQLDate actualECBSwapEndDate4  = validation::tryMeDateNthECBSwapEndDate(asOfDate, 4);
		const AQLDate actualECBSwapEndDate5  = validation::tryMeDateNthECBSwapEndDate(asOfDate, 5);
		const AQLDate actualECBSwapEndDate6  = validation::tryMeDateNthECBSwapEndDate(asOfDate, 6);
		const AQLDate actualECBSwapEndDate7  = validation::tryMeDateNthECBSwapEndDate(asOfDate, 7);
		const AQLDate actualECBSwapEndDate8  = validation::tryMeDateNthECBSwapEndDate(asOfDate, 8);
		const AQLDate actualECBSwapEndDate9  = validation::tryMeDateNthECBSwapEndDate(asOfDate, 9);
		const AQLDate actualECBSwapEndDate10 = validation::tryMeDateNthECBSwapEndDate(asOfDate, 10);

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
