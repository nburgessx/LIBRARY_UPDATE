// TestDatesSwapSchedule.cpp

// Includes: Google Test Library
#include <gTest/gTest.h>

// Includes: This Library
#include "SwapSchedule.h"
#include "QuickTest.h"
#include "LADateScheduleHelpers.h"

// Namespaces
using namespace etrading;

namespace google_test
{

    //
    // SetUp Test Fixture
    //

    // This fixture is inheriting QuickTest which is a TimeOut check
    // No need to inherit "public testing::Test" since QuickTest is already doing this.
    class TestDatesSwapSchedule : public QuickTest
    {
    protected:
        TestDatesSwapSchedule()
        {
            scheduleObject_ = SwapSchedule( AQLDate( "20150522" ),       // accrualStartDate,
                                            AQLDate( "20200522" ),       // accrualEndDate,
                                            AQLString( "30/360" ),       // accrualDaycount
                                            AQLString( "ANNUAL" ),       // accrualFrequency
                                            AQLString( "NO_CHANGE" ),    // accrualRollConvention ( slidingRule )
                                            AQLString( "TGT" ),          // accrualCalendar
                                            AQLString( "ANNUAL" ),       // paymentFrequency
                                            AQLString( "NO_CHANGE" ),    // paymentRollConvention ( slidingRule )
                                            AQLString( "TGT" ),          // paymentCalendar
                                            AQLString( "0D" ),           // paymentLag
                                            AQLString( "0D" ),           // fixingLag
                                            AQLString( "NO_CHANGE" ),    // fixingRollConvention ( slidingRule )
                                            AQLString( "TGT" ),          // fixingCalendar
                                            AQLDate(),                   // firstStub ( First Odd Date )
                                            AQLDate(),                   // lastStub ( Last Odd Date )
                                            AQLString( "" ),             // Roll Day ( e.g. 9th of month ) or Roll Convention ( IMM, ECM, EOM, End of Month ... )
                                            true,                       // isStartRoll
                                            true );                     // isFixingInAdvance
        };

        virtual ~TestDatesSwapSchedule() {}
        virtual void SetUp()        {};     // Setup gets invoked immediately before a test starts
        virtual void TearDown()     {};     // TearDown gets invoked immediately after a test finishes

        virtual SwapSchedule ScheduleObject()
        {
            return scheduleObject_;
        };

    private:
        SwapSchedule scheduleObject_;
    };


    //
    // Call Test Fixture
    //
    TEST_F( TestDatesSwapSchedule, CONSISTENCY_CheckScheduleSize )
    {
        // Asserts are fatal errors causing further tests to be abandoned
        ASSERT_EQ( 5, ScheduleObject().getSchedule().size() );
    }

    TEST_F( TestDatesSwapSchedule, UNIT_CheckFixingDates )
    {
        // Get the Schedule
        DoubleMatrix schedule = ScheduleObject().getSchedule();

        // Asserts are fatal errors causing further tests to be abandoned
        ASSERT_EQ( 5, schedule.size() )       << "#Error: Expected 5 Rows of Data"    << endl;
        ASSERT_EQ( 5, schedule[0].size() )    << "#Error: Expected 5 Columns of Data" << endl;

        // Fixing Dates are in the First Column of the Schedule
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20150522" ) ), schedule[0][0] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20160522" ) ), schedule[1][0] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20170522" ) ), schedule[2][0] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20180522" ) ), schedule[3][0] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20190522" ) ), schedule[4][0] );
    }

    TEST_F( TestDatesSwapSchedule, SNAPSHOT_CheckAccrualStartDates )
    {
        // Get the Schedule
        DoubleMatrix schedule = ScheduleObject().getSchedule();

        // Asserts are fatal errors causing further tests to be abandoned
        ASSERT_EQ( 5, schedule.size() )       << "#Error: Expected 5 Rows of Data"    << endl;
        ASSERT_EQ( 5, schedule[0].size() )    << "#Error: Expected 5 Columns of Data" << endl;

        // Fixing Dates are in the Second Column of the Schedule
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20150522" ) ), schedule[0][1] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20160522" ) ), schedule[1][1] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20170522" ) ), schedule[2][1] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20180522" ) ), schedule[3][1] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20190522" ) ), schedule[4][1] );
    }

    TEST_F( TestDatesSwapSchedule, SNAPSHOT_CheckAccrualEndDates )
    {
        // Get the Schedule
        DoubleMatrix schedule = ScheduleObject().getSchedule();

        // Asserts are fatal errors causing further tests to be abandoned
        ASSERT_EQ( 5, schedule.size() )       << "#Error: Expected 5 Rows of Data"    << endl;
        ASSERT_EQ( 5, schedule[0].size() )    << "#Error: Expected 5 Columns of Data" << endl;

        // Fixing Dates are in the Third Column of the Schedule
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20160522" ) ), schedule[0][2] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20170522" ) ), schedule[1][2] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20180522" ) ), schedule[2][2] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20190522" ) ), schedule[3][2] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20200522" ) ), schedule[4][2] );
    }

    TEST_F( TestDatesSwapSchedule, SNAPSHOT_CheckAccrualPeriods )
    {
        // Get the Schedule
        DoubleMatrix schedule = ScheduleObject().getSchedule();

        // Asserts are fatal errors causing further tests to be abandoned
        ASSERT_EQ( 5, schedule.size() )       << "#Error: Expected 5 Rows of Data"    << endl;
        ASSERT_EQ( 5, schedule[0].size() )    << "#Error: Expected 5 Columns of Data" << endl;

        // Fixing Dates are in the Forth Column of the Schedule
        EXPECT_EQ( 1.0, schedule[0][3] );
        EXPECT_EQ( 1.0, schedule[1][3] );
        EXPECT_EQ( 1.0, schedule[2][3] );
        EXPECT_EQ( 1.0, schedule[3][3] );
        EXPECT_EQ( 1.0, schedule[4][3] );
    }

    TEST_F( TestDatesSwapSchedule, UNIT_CheckPaymentDates )
    {
        // Get the Schedule
        DoubleMatrix schedule = ScheduleObject().getSchedule();

        // Asserts are fatal errors causing further tests to be abandoned
        ASSERT_EQ( 5, schedule.size() )       << "#Error: Expected 5 Rows of Data"    << endl;
        ASSERT_EQ( 5, schedule[0].size() )    << "#Error: Expected 5 Columns of Data" << endl;

        // Fixing Dates are in the Fifth Column of the Schedule
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20160522" ) ), schedule[0][4] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20170522" ) ), schedule[1][4] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20180522" ) ), schedule[2][4] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20190522" ) ), schedule[3][4] );
        EXPECT_EQ( ( double )etrading::LADateScheduleHelpers::getExcelDate( AQLDate( "20200522" ) ), schedule[4][4] );
    }
}
