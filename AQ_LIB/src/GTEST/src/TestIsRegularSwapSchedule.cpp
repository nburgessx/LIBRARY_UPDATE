// TestIsRegularSwapSchedule.cpp
#include "ScheduleValidation.h"

// Include: Google Test Library
//#include <gTest/gTest.h>
#include "InitializeGoogleTest.h"	
#include "Dependency.h"


// In the etrading project within most swaps we perform schedule validation. Specifically within the float
// leg of a swap transaction we check if we need to calculate a stub rate or apply a regular rate reset or fixing.

// Within the ScheduleValidation.cpp we have a critical method, which checks if a float leg is a regular or bespoke float
// leg. Regular floating legs do not require stub treatment, whereas bespoke legs do. 

// Here we test and ensure that this critical schedule checking method is functioning correctly.


//  // @brief			Validate if swap has regular date schedule, i.e. with stub(s)
//  // @param [in]		swapStart			    Swap start date
//  // @param [in]		givenSwapEnd		    Swap end date as given (can either be a date adjusted for holiday, or not)
//  // @param [in]		isMaturityDateAdjusted	    Swap maturity date given as a tenor? ---- This field tells us if the swapEndDate was calculated from a tenor using unadjusted  businessDayConvention(s)
//  // @param [in]		freq				    Swap floating frequency
//  // @param [in]		busDayAdj			    Swap business date adjustment convention
//  // @param [in]		calendar			    Swap calendar
//  // @param [in]		rollDay				    Roll day
//  // @param [in]		rollConvention		    Swap roll convention
//  // @output			TRUE for having irregular schedule
//  //
//  bool isRegularSwapSchedule( const AQLDate& swapStart,
//	                            const AQLDate& givenSwapEnd,
//	                            bool isMaturityDateAdjusted,
//	                            const AQLString& freq,
//	                            const AQLString& busDayAdj,
//	                            const AQLString& calendar,
//	                            int rollDay,
//	                            const AQLString& rollConvention )

using namespace etrading;

namespace google_test
{
    DECLARE_TEST_FIXTURE(TestIsRegularSwapSchedule)

    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_ANNUAL )
    {
        const AQLDate        regularStartDate            = AQLDate("20180301");
        const AQLDate        regularEndDate              = AQLDate("20190301");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("ANNUAL");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20190315");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_SEMI_ANNUAL )
    {
        const AQLDate        regularStartDate            = AQLDate("20180301");
        const AQLDate        regularEndDate              = AQLDate("20180903"); // Start + 6M = Monday 3rd-Sep-18
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("SEMI-ANNUAL");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180915");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180301");
        const AQLDate        regularEndDate              = AQLDate("20180601");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_QUARTERLY_WITH_LARGE_FAILURE_TESTS )
    {
        const AQLDate        regularStartDate            = AQLDate("20180301");
        const AQLDate        regularEndDate              = AQLDate("20180601");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );

        // More Tests for Failure - Irrgular Start Dates
        // ---------------------------------------------

        // 1st is a good day
        const bool irregularStart2  = etrading::isRegularSwapSchedule( AQLDate("20180302"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart3  = etrading::isRegularSwapSchedule( AQLDate("20180303"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart4  = etrading::isRegularSwapSchedule( AQLDate("20180304"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart5  = etrading::isRegularSwapSchedule( AQLDate("20180305"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart6  = etrading::isRegularSwapSchedule( AQLDate("20180306"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart7  = etrading::isRegularSwapSchedule( AQLDate("20180307"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart8  = etrading::isRegularSwapSchedule( AQLDate("20180308"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart9  = etrading::isRegularSwapSchedule( AQLDate("20180309"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart10 = etrading::isRegularSwapSchedule( AQLDate("20180310"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart11 = etrading::isRegularSwapSchedule( AQLDate("20180311"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart12 = etrading::isRegularSwapSchedule( AQLDate("20180312"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart13 = etrading::isRegularSwapSchedule( AQLDate("20180313"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart14 = etrading::isRegularSwapSchedule( AQLDate("20180314"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart15 = etrading::isRegularSwapSchedule( AQLDate("20180315"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart16 = etrading::isRegularSwapSchedule( AQLDate("20180316"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart17 = etrading::isRegularSwapSchedule( AQLDate("20180317"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart18 = etrading::isRegularSwapSchedule( AQLDate("20180318"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart19 = etrading::isRegularSwapSchedule( AQLDate("20180319"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart20 = etrading::isRegularSwapSchedule( AQLDate("20180320"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart21 = etrading::isRegularSwapSchedule( AQLDate("20180321"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart22 = etrading::isRegularSwapSchedule( AQLDate("20180322"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart23 = etrading::isRegularSwapSchedule( AQLDate("20180323"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart24 = etrading::isRegularSwapSchedule( AQLDate("20180324"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart25 = etrading::isRegularSwapSchedule( AQLDate("20180325"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart26 = etrading::isRegularSwapSchedule( AQLDate("20180326"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart27 = etrading::isRegularSwapSchedule( AQLDate("20180327"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart28 = etrading::isRegularSwapSchedule( AQLDate("20180328"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart29 = etrading::isRegularSwapSchedule( AQLDate("20180329"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart30 = etrading::isRegularSwapSchedule( AQLDate("20180330"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart31 = etrading::isRegularSwapSchedule( AQLDate("20180331"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        
        // 1st is a good day
        EXPECT_EQ( false, irregularStart2  );
        EXPECT_EQ( false, irregularStart3  );
        EXPECT_EQ( false, irregularStart4  );
        EXPECT_EQ( false, irregularStart5  );
        EXPECT_EQ( false, irregularStart6  );
        EXPECT_EQ( false, irregularStart7  );
        EXPECT_EQ( false, irregularStart8  );
        EXPECT_EQ( false, irregularStart9  );
        EXPECT_EQ( false, irregularStart10 );
        EXPECT_EQ( false, irregularStart11 );
        EXPECT_EQ( false, irregularStart12 );
        EXPECT_EQ( false, irregularStart13 );
        EXPECT_EQ( false, irregularStart14 );
        EXPECT_EQ( false, irregularStart15 );
        EXPECT_EQ( false, irregularStart16 );
        EXPECT_EQ( false, irregularStart17 );
        EXPECT_EQ( false, irregularStart18 );
        EXPECT_EQ( false, irregularStart19 );
        EXPECT_EQ( false, irregularStart20 );
        EXPECT_EQ( false, irregularStart21 );
        EXPECT_EQ( false, irregularStart22 );
        EXPECT_EQ( false, irregularStart23 );
        EXPECT_EQ( false, irregularStart24 );
        EXPECT_EQ( false, irregularStart25 );
        EXPECT_EQ( false, irregularStart26 );
        EXPECT_EQ( false, irregularStart27 );
        EXPECT_EQ( false, irregularStart28 );
        EXPECT_EQ( false, irregularStart29 );
        EXPECT_EQ( false, irregularStart30 );
        EXPECT_EQ( false, irregularStart31 );

        // More Tests for Failure - Irrgular End Dates
        // ---------------------------------------------

        // 1st is a good end day
        const bool irregularEnd2  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180602"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd3  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180603"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd4  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180604"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd5  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180605"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd6  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180606"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd7  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180607"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd8  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180608"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd9  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180609"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd10 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180610"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd11 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180611"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd12 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180612"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd13 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180613"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd14 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180614"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd15 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180615"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd16 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180616"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd17 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180617"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd18 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180618"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd19 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180619"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd20 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180620"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd21 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180621"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd22 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180622"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd23 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180623"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd24 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180624"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd25 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180625"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd26 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180626"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd27 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180627"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd28 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180628"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd29 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180629"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd30 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180630"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        
        // 1st is a good end day
        EXPECT_EQ( false, irregularEnd2  );
        EXPECT_EQ( false, irregularEnd3  );
        EXPECT_EQ( false, irregularEnd4  );
        EXPECT_EQ( false, irregularEnd5  );
        EXPECT_EQ( false, irregularEnd6  );
        EXPECT_EQ( false, irregularEnd7  );
        EXPECT_EQ( false, irregularEnd8  );
        EXPECT_EQ( false, irregularEnd9  );
        EXPECT_EQ( false, irregularEnd10 );
        EXPECT_EQ( false, irregularEnd11 );
        EXPECT_EQ( false, irregularEnd12 );
        EXPECT_EQ( false, irregularEnd13 );
        EXPECT_EQ( false, irregularEnd14 );
        EXPECT_EQ( false, irregularEnd15 );
        EXPECT_EQ( false, irregularEnd16 );
        EXPECT_EQ( false, irregularEnd17 );
        EXPECT_EQ( false, irregularEnd18 );
        EXPECT_EQ( false, irregularEnd19 );
        EXPECT_EQ( false, irregularEnd20 );
        EXPECT_EQ( false, irregularEnd21 );
        EXPECT_EQ( false, irregularEnd22 );
        EXPECT_EQ( false, irregularEnd23 );
        EXPECT_EQ( false, irregularEnd24 );
        EXPECT_EQ( false, irregularEnd25 );
        EXPECT_EQ( false, irregularEnd26 );
        EXPECT_EQ( false, irregularEnd27 );
        EXPECT_EQ( false, irregularEnd28 );
        EXPECT_EQ( false, irregularEnd29 );
        EXPECT_EQ( false, irregularEnd30 );

    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_MONTHLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180301");
        const AQLDate        regularEndDate              = AQLDate("20180401");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("MONTHLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180415");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_IMM_ROLL_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180321");
        const AQLDate        regularEndDate              = AQLDate("20180620");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("IMM");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_IMM_ROLL_QUARTERLY_WITH_LARGE_FAILURE_TESTS )
    {
        const AQLDate        regularStartDate            = AQLDate("20180321");
        const AQLDate        regularEndDate              = AQLDate("20180620");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("IMM");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );

        // More Tests for Failure - Irrgular Start Dates
        // ---------------------------------------------

        // 21 is a good start day
        const bool irregularStart1  = etrading::isRegularSwapSchedule( AQLDate("20180301"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart2  = etrading::isRegularSwapSchedule( AQLDate("20180302"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart3  = etrading::isRegularSwapSchedule( AQLDate("20180303"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart4  = etrading::isRegularSwapSchedule( AQLDate("20180304"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart5  = etrading::isRegularSwapSchedule( AQLDate("20180305"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart6  = etrading::isRegularSwapSchedule( AQLDate("20180306"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart7  = etrading::isRegularSwapSchedule( AQLDate("20180307"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart8  = etrading::isRegularSwapSchedule( AQLDate("20180308"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart9  = etrading::isRegularSwapSchedule( AQLDate("20180309"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart10 = etrading::isRegularSwapSchedule( AQLDate("20180310"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart11 = etrading::isRegularSwapSchedule( AQLDate("20180311"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart12 = etrading::isRegularSwapSchedule( AQLDate("20180312"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart13 = etrading::isRegularSwapSchedule( AQLDate("20180313"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart14 = etrading::isRegularSwapSchedule( AQLDate("20180314"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart15 = etrading::isRegularSwapSchedule( AQLDate("20180315"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart16 = etrading::isRegularSwapSchedule( AQLDate("20180316"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart17 = etrading::isRegularSwapSchedule( AQLDate("20180317"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart18 = etrading::isRegularSwapSchedule( AQLDate("20180318"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart19 = etrading::isRegularSwapSchedule( AQLDate("20180319"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart20 = etrading::isRegularSwapSchedule( AQLDate("20180320"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        // 21st is a good start day
        const bool irregularStart22 = etrading::isRegularSwapSchedule( AQLDate("20180322"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart23 = etrading::isRegularSwapSchedule( AQLDate("20180323"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart24 = etrading::isRegularSwapSchedule( AQLDate("20180324"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart25 = etrading::isRegularSwapSchedule( AQLDate("20180325"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart26 = etrading::isRegularSwapSchedule( AQLDate("20180326"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart27 = etrading::isRegularSwapSchedule( AQLDate("20180327"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart28 = etrading::isRegularSwapSchedule( AQLDate("20180328"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart29 = etrading::isRegularSwapSchedule( AQLDate("20180329"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart30 = etrading::isRegularSwapSchedule( AQLDate("20180330"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart31 = etrading::isRegularSwapSchedule( AQLDate("20180331"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        
        // 21 is a good start day
        EXPECT_EQ( false, irregularStart1  );
        EXPECT_EQ( false, irregularStart2  );
        EXPECT_EQ( false, irregularStart3  );
        EXPECT_EQ( false, irregularStart4  );
        EXPECT_EQ( false, irregularStart5  );
        EXPECT_EQ( false, irregularStart6  );
        EXPECT_EQ( false, irregularStart7  );
        EXPECT_EQ( false, irregularStart8  );
        EXPECT_EQ( false, irregularStart9  );
        EXPECT_EQ( false, irregularStart10 );
        EXPECT_EQ( false, irregularStart11 );
        EXPECT_EQ( false, irregularStart12 );
        EXPECT_EQ( false, irregularStart13 );
        EXPECT_EQ( false, irregularStart14 );
        EXPECT_EQ( false, irregularStart15 );
        EXPECT_EQ( false, irregularStart16 );
        EXPECT_EQ( false, irregularStart17 );
        EXPECT_EQ( false, irregularStart18 );
        EXPECT_EQ( false, irregularStart19 );
        EXPECT_EQ( false, irregularStart20 );
        // 21st is a good start day
        EXPECT_EQ( false, irregularStart22 );
        EXPECT_EQ( false, irregularStart23 );
        EXPECT_EQ( false, irregularStart24 );
        EXPECT_EQ( false, irregularStart25 );
        EXPECT_EQ( false, irregularStart26 );
        EXPECT_EQ( false, irregularStart27 );
        EXPECT_EQ( false, irregularStart28 );
        EXPECT_EQ( false, irregularStart29 );
        EXPECT_EQ( false, irregularStart30 );
        EXPECT_EQ( false, irregularStart31 );

        // More Tests for Failure - Irrgular End Dates
        // ---------------------------------------------

        // 20 is a good end day
        const bool irregularEnd1  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180601"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd2  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180602"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd3  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180603"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd4  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180604"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd5  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180605"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd6  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180606"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd7  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180607"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd8  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180608"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd9  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180609"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd10 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180610"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd11 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180611"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd12 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180612"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd13 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180613"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd14 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180614"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd15 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180615"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd16 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180616"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd17 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180617"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd18 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180618"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd19 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180619"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        // 20th is a good end day
        const bool irregularEnd21 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180621"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd22 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180622"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd23 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180623"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd24 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180624"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd25 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180625"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd26 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180626"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd27 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180627"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd28 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180628"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd29 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180629"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd30 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180630"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        
        // 20 is a good end day
        EXPECT_EQ( false, irregularEnd1  );
        EXPECT_EQ( false, irregularEnd2  );
        EXPECT_EQ( false, irregularEnd3  );
        EXPECT_EQ( false, irregularEnd4  );
        EXPECT_EQ( false, irregularEnd5  );
        EXPECT_EQ( false, irregularEnd6  );
        EXPECT_EQ( false, irregularEnd7  );
        EXPECT_EQ( false, irregularEnd8  );
        EXPECT_EQ( false, irregularEnd9  );
        EXPECT_EQ( false, irregularEnd10 );
        EXPECT_EQ( false, irregularEnd11 );
        EXPECT_EQ( false, irregularEnd12 );
        EXPECT_EQ( false, irregularEnd13 );
        EXPECT_EQ( false, irregularEnd14 );
        EXPECT_EQ( false, irregularEnd15 );
        EXPECT_EQ( false, irregularEnd16 );
        EXPECT_EQ( false, irregularEnd17 );
        EXPECT_EQ( false, irregularEnd18 );
        EXPECT_EQ( false, irregularEnd19 );
        // 20th is a good end day
        EXPECT_EQ( false, irregularEnd21 );
        EXPECT_EQ( false, irregularEnd22 );
        EXPECT_EQ( false, irregularEnd23 );
        EXPECT_EQ( false, irregularEnd24 );
        EXPECT_EQ( false, irregularEnd25 );
        EXPECT_EQ( false, irregularEnd26 );
        EXPECT_EQ( false, irregularEnd27 );
        EXPECT_EQ( false, irregularEnd28 );
        EXPECT_EQ( false, irregularEnd29 );
        EXPECT_EQ( false, irregularEnd30 );

    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_EOM_ROLL_ANNUAL )
    {
        const AQLDate        regularStartDate            = AQLDate("20180329"); // EOM is 29th Due to Good Froday Holiday on 30th
        const AQLDate        regularEndDate              = AQLDate("20190329");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("ANNUAL");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("EOM");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20190315");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_EOM_ROLL_SEMI_ANNUAL )
    {
        const AQLDate        regularStartDate            = AQLDate("20180329"); // EOM is 29th Due to Good Froday Holiday on 30th
        const AQLDate        regularEndDate              = AQLDate("20180928");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("SEMI-ANNUAL");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("EOM");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180915");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_EOM_ROLL_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180329"); // EOM is 29th Due to Good Froday Holiday on 30th
        const AQLDate        regularEndDate              = AQLDate("20180629");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("EOM");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_EOM_ROLL_QUARTERLY_WITH_LARGE_FAILURE_TESTS )
    {
        const AQLDate        regularStartDate            = AQLDate("20180329"); // EOM is 29th Due to Good Froday Holiday on 30th
        const AQLDate        regularEndDate              = AQLDate("20180629");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("EOM");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );

        // More Tests for Failure - Irrgular Start Dates
        // ---------------------------------------------

        // 29 is a good start day (30th is a holiday)
        const bool irregularStart1  = etrading::isRegularSwapSchedule( AQLDate("20180301"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart2  = etrading::isRegularSwapSchedule( AQLDate("20180302"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart3  = etrading::isRegularSwapSchedule( AQLDate("20180303"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart4  = etrading::isRegularSwapSchedule( AQLDate("20180304"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart5  = etrading::isRegularSwapSchedule( AQLDate("20180305"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart6  = etrading::isRegularSwapSchedule( AQLDate("20180306"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart7  = etrading::isRegularSwapSchedule( AQLDate("20180307"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart8  = etrading::isRegularSwapSchedule( AQLDate("20180308"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart9  = etrading::isRegularSwapSchedule( AQLDate("20180309"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart10 = etrading::isRegularSwapSchedule( AQLDate("20180310"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart11 = etrading::isRegularSwapSchedule( AQLDate("20180311"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart12 = etrading::isRegularSwapSchedule( AQLDate("20180312"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart13 = etrading::isRegularSwapSchedule( AQLDate("20180313"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart14 = etrading::isRegularSwapSchedule( AQLDate("20180314"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart15 = etrading::isRegularSwapSchedule( AQLDate("20180315"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart16 = etrading::isRegularSwapSchedule( AQLDate("20180316"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart17 = etrading::isRegularSwapSchedule( AQLDate("20180317"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart18 = etrading::isRegularSwapSchedule( AQLDate("20180318"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart19 = etrading::isRegularSwapSchedule( AQLDate("20180319"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart20 = etrading::isRegularSwapSchedule( AQLDate("20180320"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart21 = etrading::isRegularSwapSchedule( AQLDate("20180321"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart22 = etrading::isRegularSwapSchedule( AQLDate("20180322"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart23 = etrading::isRegularSwapSchedule( AQLDate("20180323"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart24 = etrading::isRegularSwapSchedule( AQLDate("20180324"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart25 = etrading::isRegularSwapSchedule( AQLDate("20180325"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart26 = etrading::isRegularSwapSchedule( AQLDate("20180326"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart27 = etrading::isRegularSwapSchedule( AQLDate("20180327"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart28 = etrading::isRegularSwapSchedule( AQLDate("20180328"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        // 29th is good start day
        const bool irregularStart30 = etrading::isRegularSwapSchedule( AQLDate("20180330"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularStart31 = etrading::isRegularSwapSchedule( AQLDate("20180331"), regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        
        // 29 is a good start day (30th is a holiday)
        EXPECT_EQ( false, irregularStart1  );
        EXPECT_EQ( false, irregularStart2  );
        EXPECT_EQ( false, irregularStart3  );
        EXPECT_EQ( false, irregularStart4  );
        EXPECT_EQ( false, irregularStart5  );
        EXPECT_EQ( false, irregularStart6  );
        EXPECT_EQ( false, irregularStart7  );
        EXPECT_EQ( false, irregularStart8  );
        EXPECT_EQ( false, irregularStart9  );
        EXPECT_EQ( false, irregularStart10 );
        EXPECT_EQ( false, irregularStart11 );
        EXPECT_EQ( false, irregularStart12 );
        EXPECT_EQ( false, irregularStart13 );
        EXPECT_EQ( false, irregularStart14 );
        EXPECT_EQ( false, irregularStart15 );
        EXPECT_EQ( false, irregularStart16 );
        EXPECT_EQ( false, irregularStart17 );
        EXPECT_EQ( false, irregularStart18 );
        EXPECT_EQ( false, irregularStart19 );
        EXPECT_EQ( false, irregularStart20 );
        EXPECT_EQ( false, irregularStart21 );
        EXPECT_EQ( false, irregularStart22 );
        EXPECT_EQ( false, irregularStart23 );
        EXPECT_EQ( false, irregularStart24 );
        EXPECT_EQ( false, irregularStart25 );
        EXPECT_EQ( false, irregularStart26 );
        EXPECT_EQ( false, irregularStart27 );
        EXPECT_EQ( false, irregularStart28 );
        // 29th is a good start day
        EXPECT_EQ( false, irregularStart30 );

        // More Tests for Failure - Irrgular End Dates
        // ---------------------------------------------

        // 29 is a good end day (30th is a holiday)
        const bool irregularEnd1  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180601"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd2  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180602"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd3  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180603"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd4  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180604"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd5  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180605"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd6  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180606"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd7  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180607"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd8  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180608"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd9  = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180609"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd10 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180610"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd11 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180611"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd12 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180612"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd13 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180613"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd14 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180614"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd15 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180615"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd16 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180616"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd17 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180617"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd18 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180618"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd19 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180619"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd20 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180619"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd21 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180621"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd22 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180622"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd23 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180623"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd24 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180624"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd25 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180625"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd26 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180626"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd27 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180627"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool irregularEnd28 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180628"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        // 29th is a good end day

        // 30th is a holiday, when isMaturityAdjusted = false the isRegulareSwapSchedule function will adjust the end date and give a true result
        //const bool irregularEnd30 = etrading::isRegularSwapSchedule( regularStartDate, AQLDate("20180630"), isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        
        // 29 is a good end day (30th is a holiday)
        EXPECT_EQ( false, irregularEnd1  );
        EXPECT_EQ( false, irregularEnd2  );
        EXPECT_EQ( false, irregularEnd3  );
        EXPECT_EQ( false, irregularEnd4  );
        EXPECT_EQ( false, irregularEnd5  );
        EXPECT_EQ( false, irregularEnd6  );
        EXPECT_EQ( false, irregularEnd7  );
        EXPECT_EQ( false, irregularEnd8  );
        EXPECT_EQ( false, irregularEnd9  );
        EXPECT_EQ( false, irregularEnd10 );
        EXPECT_EQ( false, irregularEnd11 );
        EXPECT_EQ( false, irregularEnd12 );
        EXPECT_EQ( false, irregularEnd13 );
        EXPECT_EQ( false, irregularEnd14 );
        EXPECT_EQ( false, irregularEnd15 );
        EXPECT_EQ( false, irregularEnd16 );
        EXPECT_EQ( false, irregularEnd17 );
        EXPECT_EQ( false, irregularEnd18 );
        EXPECT_EQ( false, irregularEnd19 );
        EXPECT_EQ( false, irregularEnd20 );
        EXPECT_EQ( false, irregularEnd21 );
        EXPECT_EQ( false, irregularEnd22 );
        EXPECT_EQ( false, irregularEnd23 );
        EXPECT_EQ( false, irregularEnd24 );
        EXPECT_EQ( false, irregularEnd25 );
        EXPECT_EQ( false, irregularEnd26 );
        EXPECT_EQ( false, irregularEnd27 );
        EXPECT_EQ( false, irregularEnd28 );
        // 29th is a good end day

        // 30th is a holiday, when isMaturityAdjusted = false the isRegulareSwapSchedule function will adjust the end date and give a true result
        //EXPECT_EQ( false, irregularEnd30 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_EOM_ROLL_MONTHLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180329");
        const AQLDate        regularEndDate              = AQLDate("20180430");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("MONTHLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 0;
	    const AQLString      rollConvention              = AQLString("EOM");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180415");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_1_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180301");
        const AQLDate        regularEndDate              = AQLDate("20180601");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 1;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_2_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180302");
        const AQLDate        regularEndDate              = AQLDate("20180602");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 2;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_3_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180303");
        const AQLDate        regularEndDate              = AQLDate("20180603");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 3;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_4_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180304");
        const AQLDate        regularEndDate              = AQLDate("20180604");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 4;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_5_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180305");
        const AQLDate        regularEndDate              = AQLDate("20180605");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 5;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_6_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180306");
        const AQLDate        regularEndDate              = AQLDate("20180606");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 6;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_7_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180307");
        const AQLDate        regularEndDate              = AQLDate("20180607");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 7;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_8_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180308");
        const AQLDate        regularEndDate              = AQLDate("20180608");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 8;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_9_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180309");
        const AQLDate        regularEndDate              = AQLDate("20180609");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 9;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }
    

    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_10_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180310");
        const AQLDate        regularEndDate              = AQLDate("20180610");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 10;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_11_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180311");
        const AQLDate        regularEndDate              = AQLDate("20180611");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 11;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_12_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180312");
        const AQLDate        regularEndDate              = AQLDate("20180612");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 12;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_13_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180313");
        const AQLDate        regularEndDate              = AQLDate("20180613");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 13;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_14_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180314");
        const AQLDate        regularEndDate              = AQLDate("20180614");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 14;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_15_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180315");
        const AQLDate        regularEndDate              = AQLDate("20180615");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 15;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_16_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180316");
        const AQLDate        regularEndDate              = AQLDate("20180616");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 16;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_17_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180317");
        const AQLDate        regularEndDate              = AQLDate("20180617");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 17;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_18_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180318");
        const AQLDate        regularEndDate              = AQLDate("20180618");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 18;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_19_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180319");
        const AQLDate        regularEndDate              = AQLDate("20180619");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 19;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_20_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180320");
        const AQLDate        regularEndDate              = AQLDate("20180620");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 20;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_21_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180321");
        const AQLDate        regularEndDate              = AQLDate("20180621");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 21;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_22_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180322");
        const AQLDate        regularEndDate              = AQLDate("20180622");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 22;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_23_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180323");
        const AQLDate        regularEndDate              = AQLDate("20180623");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 23;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_24_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180324");
        const AQLDate        regularEndDate              = AQLDate("20180624");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 24;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_25_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180325");
        const AQLDate        regularEndDate              = AQLDate("20180625");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 25;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_26_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180326");
        const AQLDate        regularEndDate              = AQLDate("20180626");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 26;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_27_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180327");
        const AQLDate        regularEndDate              = AQLDate("20180627");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 27;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_28_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180328");
        const AQLDate        regularEndDate              = AQLDate("20180628");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 28;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_29_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180329");
        const AQLDate        regularEndDate              = AQLDate("20180629");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 29;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_30_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180330");
        const AQLDate        regularEndDate              = AQLDate("20180630");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 30;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_ROLLDAY_31_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180331");
        const AQLDate        regularEndDate              = AQLDate("20180630");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 31;
	    const AQLString      rollConvention              = AQLString();
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_1_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180301");
        const AQLDate        regularEndDate              = AQLDate("20180601");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 1;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_2_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180302");
        const AQLDate        regularEndDate              = AQLDate("20180602");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 2;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_3_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180303");
        const AQLDate        regularEndDate              = AQLDate("20180603");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 3;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_4_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180304");
        const AQLDate        regularEndDate              = AQLDate("20180604");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 4;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_5_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180305");
        const AQLDate        regularEndDate              = AQLDate("20180605");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 5;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_6_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180306");
        const AQLDate        regularEndDate              = AQLDate("20180606");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 6;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_7_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180307");
        const AQLDate        regularEndDate              = AQLDate("20180607");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 7;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_8_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180308");
        const AQLDate        regularEndDate              = AQLDate("20180608");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 8;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_9_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180309");
        const AQLDate        regularEndDate              = AQLDate("20180609");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 9;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }
    

    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_10_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180310");
        const AQLDate        regularEndDate              = AQLDate("20180610");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 10;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_11_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180311");
        const AQLDate        regularEndDate              = AQLDate("20180611");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 11;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_12_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180312");
        const AQLDate        regularEndDate              = AQLDate("20180612");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 12;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_13_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180313");
        const AQLDate        regularEndDate              = AQLDate("20180613");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 13;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_14_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180314");
        const AQLDate        regularEndDate              = AQLDate("20180614");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 14;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_15_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180315");
        const AQLDate        regularEndDate              = AQLDate("20180615");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 15;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_16_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180316");
        const AQLDate        regularEndDate              = AQLDate("20180616");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 16;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_17_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180317");
        const AQLDate        regularEndDate              = AQLDate("20180617");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 17;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_18_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180318");
        const AQLDate        regularEndDate              = AQLDate("20180618");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 18;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_19_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180319");
        const AQLDate        regularEndDate              = AQLDate("20180619");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 19;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_20_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180320");
        const AQLDate        regularEndDate              = AQLDate("20180620");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 20;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_21_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180321");
        const AQLDate        regularEndDate              = AQLDate("20180621");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 21;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_22_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180322");
        const AQLDate        regularEndDate              = AQLDate("20180622");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 22;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_23_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180323");
        const AQLDate        regularEndDate              = AQLDate("20180623");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 23;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_24_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180324");
        const AQLDate        regularEndDate              = AQLDate("20180624");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 24;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_25_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180325");
        const AQLDate        regularEndDate              = AQLDate("20180625");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 25;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_26_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180326");
        const AQLDate        regularEndDate              = AQLDate("20180626");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 26;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_27_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180327");
        const AQLDate        regularEndDate              = AQLDate("20180627");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 27;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_28_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180328");
        const AQLDate        regularEndDate              = AQLDate("20180628");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 28;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_29_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180329");
        const AQLDate        regularEndDate              = AQLDate("20180629");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 29;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180301");
        const AQLDate        irregularEndDate            = AQLDate("20180601");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_30_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180330");
        const AQLDate        regularEndDate              = AQLDate("20180630");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 30;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }


    TEST_F( TestIsRegularSwapSchedule, UNIT_NORMAL_ROLL_WITH_ROLLDAY_31_QUARTERLY )
    {
        const AQLDate        regularStartDate            = AQLDate("20180331");
        const AQLDate        regularEndDate              = AQLDate("20180630");
	    const bool          isMaturityDateAdjusted      = false; // Is the maturity date holiday adjusted
	    const AQLString      frequency                   = AQLString("QUARTERLY");
	    const AQLString      busDayAdjustment            = AQLString("MOD_FOLLOWING");
	    const AQLString      calendar                    = AQLString("TGT");
	    const int           rollDay                     = 31;
	    const AQLString      rollConvention              = AQLString("NORMAL");
   
        const AQLDate        irregularStartDate          = AQLDate("20180315");
        const AQLDate        irregularEndDate            = AQLDate("20180615");

        // Test(s) for Success
        // ---------------------------
        const bool actualResult1     = etrading::isRegularSwapSchedule( regularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult1   = true;
    
        // Test(s) for Failure
        // ---------------------------
        const bool actualResult2     = etrading::isRegularSwapSchedule( regularStartDate, irregularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult2   = false;
    
        const bool actualResult3     = etrading::isRegularSwapSchedule( irregularStartDate, regularEndDate, isMaturityDateAdjusted, frequency, busDayAdjustment, calendar, rollDay, rollConvention );
        const bool expectedResult3   = false;
    
        // Results
        // ---------------------------
        EXPECT_EQ( expectedResult1, actualResult1 );
        EXPECT_EQ( expectedResult2, actualResult2 );
        EXPECT_EQ( expectedResult3, actualResult3 );
    }
}