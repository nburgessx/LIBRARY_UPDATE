//
// LADateScheduleHelpers.h
// *** This file used to be called "LAMathDateFuncUti1ity.h" ***
#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LAString.h"
#include "ConstantDeclarations.h"
#include "LATime.h"
//////////////////////////

// Forward Declarations
class LAPriceDataSlidingRule;
class LAPriceDataCalendar;
class LAPriceDataDayCount;

namespace etrading
{
    class LADateScheduleHelpers
    {
    public:
        //change excel date into LADate
        static LADate getLADate(const int excel_date);

        //static LADate getLADate( LAString& excel_date_str);
        static LADate getLADate(const LAString& excel_date_str);

        //change excel date into LAString
        static LAString getLAStringDate(const int excel_date);

        //change MDate into excel date
        static int getExcelDate(const LADate & date);

        static LADate firstStubDateFromStubType(const LADate & startDate, const LADate & endDate, LAString & term);
        static LADate lastStubDateFromStubType(const LADate & startDate, const LADate & endDate, LAString & term);

        static DateVector calcDatesWithLag(const DateVector &				dates,
                                           const LAString &					term,
                                           const LAPriceDataSlidingRule &   slidingRule,
                                           const LAPriceDataCalendar *      pCalendar,
                                           const bool &						isAfter,
                                           const LAString *					rollConvention);

		// Generate a Date Schedule with appropriate use of stubs
		// Note that there is a duplicate method LAMathDateUtilities::generateSchedule
		// Default Short/Long Start is determined by LAMathDateUtilities::getStubDateAndType
        static DateVector generateSchedule(const LADate&		start,
										   const LADate&		end,
										   LAString&			data_frequency,
										   LAString&			slidingRuleString,
										   LAString&			calendarString,
										   const LADate*		firstStubDate = NULL,
										   const LADate*		lastStubDate = NULL,
										   const int*			rollDay = NULL,
										   const bool			is_start_roll = true,
										   const LAString*		rollConvention = NULL,
										   const LAString*		stubType = NULL);

        static bool isValidDate(const LADate & dateToValidate);

        static LADate getDateFromTerm(const LADate& fromdate, const double termy, const LAPriceDataDayCount& daycount, bool includelast = false);
        static LADate getDateFromTerm(LADate& fromdate, double termy, LAString& daycount, bool includelast = false);
        static LADate getDate(const LADate& basedate, const LAString& term, const LAString& slidingRule, const LAString& calendar);
        static LADate getDateWithRollConv(const LADate& basedate, const LAString& term, const LAString& slidingRule, const LAString& calendar, const LAString* roll_conv = NULL);
        static DateVector getMultiDate(const DateVector& basedate, const LAString& term, const LAString& slidingRule, const LAString& calendar, const LAString* roll_conv = NULL);
        static LADate getDateWithRoll(LADate& basedate, LAString& term, LAString& slidingRule, LAString& calendar, int roll);
        static double getDayFromTerm(LADate& fromdate, double termy, LAString& daycount, bool includelast = false);
        static double getTermFromDay(LADate& fromdate, double termd, LAString& daycount, bool includelast = false);

        static double getTerm(const LADate& fromdate, const LADate& todate, LAString& daycount, bool includelast = false,
                              const LAString* frequency = NULL,
                              const LAString* calendar = NULL,
                              const LAString* slidingrule = NULL,
                              const std::vector<LADate>* startdates = NULL,
                              const std::vector<LADate>* enddates = NULL);

        static LADate getIMMDate1(const int& year, const int& month, LAString& calendar, LAString& slidingRule);
        static LADate getIMMDate2(const int& year, const int& number, LAString& calendar, LAString& slidingRule);
        static LADate getIMMDate3(const LADate& basedate, const int& number, LAString& calendar, LAString& slidingRule);

        static DateMatrix calcRegularDates(const LAString& frequency,
            const LAString& calendar,
            const LAString& slidingrule,
            const std::vector<LADate>& startdates,
            const std::vector<LADate>& enddates);

    private:
        LADateScheduleHelpers(void);
        ~LADateScheduleHelpers(void);
        LADateScheduleHelpers(const LADateScheduleHelpers &rhs);
        LADateScheduleHelpers &operator=(const LADateScheduleHelpers &rhs);
    };

    bool is_last_business_day_temp(const LADate& d, const LAString& cal);

    int StringToMonthInteger(LAString mstr);

    LAPriceDataDayCount Daycount(LAString daycountConvention);

    LAPriceDataDayCount ModelDaycount();

    double YearFraction(LAPriceDataDayCount daycount, LADate valDate, LADate date);

    double ModelTime(LADate valDate, LADate targetDate);

    LADate LAStringToDate(LAString date);

    LAString FrequencyToTerm(LAString frequency);

    double TermToYearLength(LAString term);

    int TermToMonthLength(LAString term);

    LADate CalendarAdvance(LADate baseDate, LAString term, LAPriceDataSlidingRule slidingRule, LAPriceDataCalendar calendar);

}