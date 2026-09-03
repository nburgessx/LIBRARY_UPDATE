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
		const LADate currentDate = etrading::getCurrentMLibDate();
		LADate thirdMeetingDate;
		LADate forthMeetingDate;
		LADate fifthMeetingDate;
		LADate sixthMeetingDate;

		//
		// 3RD ECB MEETING DATE
		// ---------------------------------------------------------------
		// Throw ***ERROR *** if 3rd ECB Meeting Date Unavailable
		try
		{
			thirdMeetingDate = validation_api::tryMeDateNthECBMeetingDate(currentDate, 3);
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
			forthMeetingDate = validation_api::tryMeDateNextECBMeetingDate(thirdMeetingDate);
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
			fifthMeetingDate = validation_api::tryMeDateNextECBMeetingDate(forthMeetingDate);
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
			sixthMeetingDate = validation_api::tryMeDateNextECBMeetingDate(fifthMeetingDate);
		}
		catch (...)
		{
			GTEST_WARNING("ECB Calendar is about to Expire!!! - Less than 6 ECB Meeting Dates Available")
		}
	}

	TEST(TestDatesECB, UNIT_NEXT_MeetingDate)
	{
		const LADate expectedECBMeetingDate1 = LADate("20190725");
		const LADate expectedECBMeetingDate2 = LADate("20190912");
		const LADate actualECBMeetingDate2 = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate1);

		EXPECT_EQ(expectedECBMeetingDate2, actualECBMeetingDate2);
	}

	TEST(TestECBDateChain, UNIT_MeetingDates)
	{
		const LADate expectedECBMeetingDate1  = LADate("20190725");
		const LADate expectedECBMeetingDate2  = LADate("20190912");
		const LADate expectedECBMeetingDate3  = LADate("20191024");
		const LADate expectedECBMeetingDate4  = LADate("20191212");
		const LADate expectedECBMeetingDate5  = LADate("20200121");
		const LADate expectedECBMeetingDate6  = LADate("20200320");
		const LADate expectedECBMeetingDate7  = LADate("20200430");
		const LADate expectedECBMeetingDate8  = LADate("20200604");
		const LADate expectedECBMeetingDate9  = LADate("20200716");
		const LADate expectedECBMeetingDate10 = LADate("20200910");

		const LADate actualECBMeetingDate2  = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate1);
		const LADate actualECBMeetingDate3  = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate2);
		const LADate actualECBMeetingDate4  = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate3);
		const LADate actualECBMeetingDate5  = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate4);
		const LADate actualECBMeetingDate6  = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate5);
		const LADate actualECBMeetingDate7  = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate6);
		const LADate actualECBMeetingDate8  = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate7);
		const LADate actualECBMeetingDate9  = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate8);
		const LADate actualECBMeetingDate10 = validation_api::tryMeDateNextECBMeetingDate(expectedECBMeetingDate9);

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
		const LADate asOfDate = LADate("20190610");

		const LADate expectedECBMeetingDate1	= LADate("20190725");
		const LADate expectedECBMeetingDate2	= LADate("20190912");
		const LADate expectedECBMeetingDate3	= LADate("20191024");
		const LADate expectedECBMeetingDate4	= LADate("20191212");
		const LADate expectedECBMeetingDate5	= LADate("20200121");
		const LADate expectedECBMeetingDate6	= LADate("20200320");
		const LADate expectedECBMeetingDate7	= LADate("20200430");
		const LADate expectedECBMeetingDate8	= LADate("20200604");
		const LADate expectedECBMeetingDate9	= LADate("20200716");
		const LADate expectedECBMeetingDate10   = LADate("20200910");

		const LADate actualECBMeetingDate1  = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 1);
		const LADate actualECBMeetingDate2  = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 2);
		const LADate actualECBMeetingDate3  = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 3);
		const LADate actualECBMeetingDate4  = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 4);
		const LADate actualECBMeetingDate5  = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 5);
		const LADate actualECBMeetingDate6  = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 6);
		const LADate actualECBMeetingDate7  = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 7);
		const LADate actualECBMeetingDate8  = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 8);
		const LADate actualECBMeetingDate9  = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 9);
		const LADate actualECBMeetingDate10 = validation_api::tryMeDateNthECBMeetingDate(asOfDate, 10);

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
		const LADate expectedECBSwapStartDate1 = LADate("20190731");
		const LADate expectedECBSwapStartDate2 = LADate("20190918");
		const LADate actualECBSwapStartDate2 = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate1);

		EXPECT_EQ(expectedECBSwapStartDate2, actualECBSwapStartDate2);
	}

	TEST(TestECBDateChain, UNIT_SwapStartDates)
	{
		const LADate expectedECBSwapStartDate1  = LADate("20190731");
		const LADate expectedECBSwapStartDate2  = LADate("20190918");
		const LADate expectedECBSwapStartDate3  = LADate("20191030");
		const LADate expectedECBSwapStartDate4  = LADate("20191218");
		const LADate expectedECBSwapStartDate5  = LADate("20200122");
		const LADate expectedECBSwapStartDate6  = LADate("20200325");
		const LADate expectedECBSwapStartDate7  = LADate("20200506");
		const LADate expectedECBSwapStartDate8  = LADate("20200610");
		const LADate expectedECBSwapStartDate9  = LADate("20200722");
		const LADate expectedECBSwapStartDate10 = LADate("20200916");

		const LADate actualECBSwapStartDate2  = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate1);
		const LADate actualECBSwapStartDate3  = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate2);
		const LADate actualECBSwapStartDate4  = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate3);
		const LADate actualECBSwapStartDate5  = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate4);
		const LADate actualECBSwapStartDate6  = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate5);
		const LADate actualECBSwapStartDate7  = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate6);
		const LADate actualECBSwapStartDate8  = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate7);
		const LADate actualECBSwapStartDate9  = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate8);
		const LADate actualECBSwapStartDate10 = validation_api::tryMeDateNextECBSwapStartDate(expectedECBSwapStartDate9);

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
		const LADate asOfDate = LADate("20190610");

		const LADate expectedECBSwapStartDate1  = LADate("20190731");
		const LADate expectedECBSwapStartDate2  = LADate("20190918");
		const LADate expectedECBSwapStartDate3  = LADate("20191030");
		const LADate expectedECBSwapStartDate4  = LADate("20191218");
		const LADate expectedECBSwapStartDate5  = LADate("20200122");
		const LADate expectedECBSwapStartDate6  = LADate("20200325");
		const LADate expectedECBSwapStartDate7  = LADate("20200506");
		const LADate expectedECBSwapStartDate8  = LADate("20200610");
		const LADate expectedECBSwapStartDate9  = LADate("20200722");
		const LADate expectedECBSwapStartDate10 = LADate("20200916");

		const LADate actualECBSwapStartDate1  = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 1);
		const LADate actualECBSwapStartDate2  = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 2);
		const LADate actualECBSwapStartDate3  = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 3);
		const LADate actualECBSwapStartDate4  = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 4);
		const LADate actualECBSwapStartDate5  = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 5);
		const LADate actualECBSwapStartDate6  = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 6);
		const LADate actualECBSwapStartDate7  = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 7);
		const LADate actualECBSwapStartDate8  = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 8);
		const LADate actualECBSwapStartDate9  = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 9);
		const LADate actualECBSwapStartDate10 = validation_api::tryMeDateNthECBSwapStartDate(asOfDate, 10);

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
		const LADate expectedECBSwapEndDate1 = LADate("20190917");
		const LADate expectedECBSwapEndDate2 = LADate("20191029");
		const LADate actualECBSwapEndDate2 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate1);

		EXPECT_EQ(expectedECBSwapEndDate2, actualECBSwapEndDate2);
	}

	TEST(TestECBDateChain, UNIT_SwapEndDates)
	{
		const LADate expectedECBSwapEndDate1 = LADate("20190917");
		const LADate expectedECBSwapEndDate2 = LADate("20191029");
		const LADate expectedECBSwapEndDate3 = LADate("20191217");
		const LADate expectedECBSwapEndDate4 = LADate("20200121");
		const LADate expectedECBSwapEndDate5 = LADate("20200324");
		const LADate expectedECBSwapEndDate6 = LADate("20200505");
		const LADate expectedECBSwapEndDate7 = LADate("20200609");
		const LADate expectedECBSwapEndDate8 = LADate("20200721");
		const LADate expectedECBSwapEndDate9 = LADate("20200915");
		const LADate expectedECBSwapEndDate10 = LADate("20201103");

		const LADate actualECBSwapEndDate2 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate1);
		const LADate actualECBSwapEndDate3 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate2);
		const LADate actualECBSwapEndDate4 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate3);
		const LADate actualECBSwapEndDate5 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate4);
		const LADate actualECBSwapEndDate6 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate5);
		const LADate actualECBSwapEndDate7 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate6);
		const LADate actualECBSwapEndDate8 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate7);
		const LADate actualECBSwapEndDate9 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate8);
		const LADate actualECBSwapEndDate10 = validation_api::tryMeDateNextECBSwapEndDate(expectedECBSwapEndDate9);

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
		const LADate asOfDate = LADate("20190610");

		const LADate expectedECBSwapEndDate1  = LADate("20190917");
		const LADate expectedECBSwapEndDate2  = LADate("20191029");
		const LADate expectedECBSwapEndDate3  = LADate("20191217");
		const LADate expectedECBSwapEndDate4  = LADate("20200121");
		const LADate expectedECBSwapEndDate5  = LADate("20200324");
		const LADate expectedECBSwapEndDate6  = LADate("20200505");
		const LADate expectedECBSwapEndDate7  = LADate("20200609");
		const LADate expectedECBSwapEndDate8  = LADate("20200721");
		const LADate expectedECBSwapEndDate9  = LADate("20200915");
		const LADate expectedECBSwapEndDate10 = LADate("20201103");

		const LADate actualECBSwapEndDate1  = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 1);
		const LADate actualECBSwapEndDate2  = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 2);
		const LADate actualECBSwapEndDate3  = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 3);
		const LADate actualECBSwapEndDate4  = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 4);
		const LADate actualECBSwapEndDate5  = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 5);
		const LADate actualECBSwapEndDate6  = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 6);
		const LADate actualECBSwapEndDate7  = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 7);
		const LADate actualECBSwapEndDate8  = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 8);
		const LADate actualECBSwapEndDate9  = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 9);
		const LADate actualECBSwapEndDate10 = validation_api::tryMeDateNthECBSwapEndDate(asOfDate, 10);

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
