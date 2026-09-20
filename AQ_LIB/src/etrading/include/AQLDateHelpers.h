//
// AQLDateHelpers.h
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLCentralBank.h"
#include "AQLDateCalculations.h"

class AQLString;
class AQLObject;
class AQLPriceDataSlidingRule;
class AQLPriceDataCalendar;

namespace etrading
{
	class AQLDateHelpers
    {
    public:
		
		// calculate stub schedule dates
        static void generateSchedule( const AQLDate& unadjustedStart,
					 				  const AQLDate& unadjustedEnd,
					 				  const AQLString& data_frequency,
					 				  const bool inArrears,
					 				  const AQLDate* firstStubDate,
					 				  const AQLDate* lastStubDate,
					 				  const int* rollDay,
					 				  DateVector& out,
					 				  const AQLPriceDataSlidingRule* busDayAdj = NULL,
					 				  const AQLPriceDataCalendar* calendar = NULL,
					 				  const bool isStartRoll = true,
					 				  const AQLString* roll_convention = NULL );
        
        // Set term from string to integer 
        static	void				termStrtoYMDW(const AQLString& term, int& y, int& m, int& d, int& w);

		// ! Set term from string to integer - Enhanced version of the above which handles calendar days, c
		static	void				termStrtoYMDWC(const AQLString& term, int& y, int& m, int& d, int& w, int& c);

        // get date and roll (day is actual day)  
        static	AQLDate				getDate(const AQLDate& basedate,
                                            const AQLString& term,
                                            bool rollForwards,
                                            const AQLString* roll_conv = 0);
        
        // get date, roll and slide (if roll convention is set, day is actual day, otherwise business day.)
        static	AQLDate				getDate(const AQLDate& basedate,
                                            const AQLString& term,
                                            const AQLPriceDataSlidingRule& busdayrule,
                                            const AQLPriceDataCalendar* pCal,
                                            bool rollForwards,
                                            const AQLString* roll_conv = 0);

        // get IMM date
        static	AQLDate				getIMMDate(const int& y, const int& m, bool isOddMonth = false);

        // get IMM date
        static	AQLDate				getImmEndDate(const AQLDate& startDate, const AQLString& strTerm, int lag = 0);

        /*!
        @brief get futures contract start date

        @param[in]      month   futures contract month
        @param[in]      year    futures contract year
        @param[out]     returns the futures contract start date
        */
        static AQLDate               getFuturesContractStartDate(const unsigned int& month, const unsigned int& year);

        // get IMM date from future term
        static	AQLDate				getIMMDateFromTerm(const AQLDate& baseDate, const AQLString& futureTerm);
        // calc FX spot date
        static  AQLDate				getFXSpotDate(const AQLString& keyFX,
												  const AQLDate& basedate,
												  const AQLString& calStr,
												  int spotlag,
												  bool rollForwards);

        // return true iff there is a scheduled Governing Council of the ECB monetary policy meeting 
        // on or after baseDate
        static bool					haveNextECBDate(const AQLDate& baseDate, bool strictlyAfter = true);

        // get the date of the next Governing Council of the ECB monetary policy meeting 
        // on or after baseDate; throw an exception after the last scheduled meeting
        static AQLDate				getNextECBDate(const AQLDate& baseDate, bool strictlyAfter = true);

        // get the start date of an ECB Swap with a given ECB date;
        // the start date is always the Wednesday strictly after the ECB date, 
        // even if the latter happens to be a Wednesday (GC, 12-10-2015).
        // note that the end date of an ECB Swap is the start date of the next ECB Swap;
        static AQLDate				getECBStartDate(const AQLDate& ecbDate);

        // return true iff there is a scheduled central bank meeting on or after baseDate
        static bool haveNextCBDate(const AQLString& centralBankId, const AQLDate& baseDate, bool strictlyAfter);

        // get date of next central bank meeting on or after baseDate; 
        // throw AQLAppError after last scheduled meeting
        static AQLDate getNextCBDate(const AQLString& centralBankId, const AQLDate& baseDate, bool strictlyAfter);

        // get month from future term
        static	unsigned int		changeFutureMonthFormat(const AQLString& futureMonth);

        // convert from double grid to dates grid *** Use for smooth dates, since ACT/365_ISDA causes leap year irregularities ***
        static void convertToDateGrid(const AQLDate &asofDate, const DoubleArray &terms, DateVector &dates);

        // Return the frequency in months for comparing two frequencies, not for accurate calculations
		// *** Duplicate method in AQLDateCalculations.cpp ***
        static double getPeriodFrequencyInMonths(const AQLString& freq);

        // calc compounding times
        static int calcCompoundingTimes(const AQLString& freq_rst, const AQLString& freq_pay);

        // get fedfund future dates from fedfundfuter term
        static DateVector			getFFDatesFromTerm(const AQLDate& baseDate, const AQLString& fedfundTerm);

        // return next date with given weekday (e.g., Wednesday) on of after given baseDate
        static AQLDate getNextWeekdayDate(AQLDayOfWeekEnum weekday, const AQLDate& baseDate, bool strictlyAfter);

		// Nth ECB Dates: Meeting, Swap Start and Swap End Dates
		// Note: StrictlyAfter = roll the ECB date if it falls on the asOfDate
		static AQLDate getNthECBMeetingDate(const AQLDate& asOfDate, const int n, bool strictlyAfter = false );
		static AQLDate getNthECBSwapStartDate(const AQLDate& asOfDate, const int n, bool strictlyAfter = false );
		static AQLDate getNthECBSwapEndDate(const AQLDate& asOfDate, const int n, bool strictlyAfter = false );
    
		// Next ECB Dates: Meeting, Swap Start and Swap End Dates
		// Note: StrictlyAfter = roll the ECB date if it falls on the inputDate
		static AQLDate getNextECBMeetingDate(const AQLDate& meetingDate);
		static AQLDate getNextECBSwapStartDate(const AQLDate& swapStartDate);
		static AQLDate getNextECBSwapEndDate(const AQLDate& swapEndDate);

	protected:


    private:

        // calculate regular schedule dates
        static DateVector generateRegularSchedule( const AQLDate& start,
                                                   const AQLDate& end,
                                                   const AQLString& data_frequency,
                                                   const bool inArrears,
                                                   const int* rollDay,
                                                   const AQLPriceDataSlidingRule* busDayAdj,
                                                   const AQLPriceDataCalendar* calendar,
                                                   const bool isStartRoll,
                                                   const AQLString* roll_convention = NULL );

        // get adjusted date
        static AQLDate getAdjDate(const AQLPriceDataSlidingRule* busDayAdj, const AQLPriceDataCalendar* calendar, const AQLDate &date);
        // set date
        static void setDate(const int* rollDay, AQLDate &date, const RollConventionEnum rollConventionEnum);
        // slide date
        static AQLDate slideDate(const AQLDate& date, const AQLPriceDataSlidingRule& srule, const AQLPriceDataCalendar* pCal);
        // roll date
        static AQLDate rollDate(const AQLDate& date, const AQLString* roll_conv);
        // common implementation of have-/getNext [ECB/Fed/BoE] Date

        // return true iff there is a scheduled central bank meeting on or after baseDate,
        // in which case result gets updated with the meeting date
        static bool getIfExistsNextCBDate(const AQLString& centralBankId, const AQLDate& baseDate, bool strictlyAfter, AQLDate& result);
    };

    //
    // inline method implementation
    //

    inline bool	AQLDateHelpers::haveNextECBDate(const AQLDate& baseDate, bool strictlyAfter)
    {
        return haveNextCBDate("ECB", baseDate, strictlyAfter);
    }

    inline AQLDate AQLDateHelpers::getNextECBDate(const AQLDate& baseDate, bool strictlyAfter)
    {
        return getNextCBDate("ECB", baseDate, strictlyAfter);
    }

    inline AQLDate AQLDateHelpers::getECBStartDate(const AQLDate& ecbDate)
    {
        return getNextWeekdayDate(WED, ecbDate, true);
    }

}
