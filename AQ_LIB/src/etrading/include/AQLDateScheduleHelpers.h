//
// AQLDateScheduleHelpers.h
// *** This file used to be called "LAMathDateFuncUti1ity.h" ***
#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "AQLString.h"
#include "ConstantDeclarations.h"
#include "AQLTime.h"
//////////////////////////

// Forward Declarations
class AQLPriceDataSlidingRule;
class AQLPriceDataCalendar;
class AQLPriceDataDayCount;

namespace etrading
{
    class AQLDateScheduleHelpers
    {
    public:
        //change excel date into AQLDate
        static AQLDate getLADate(const int excel_date);

        //static AQLDate getLADate( AQLString& excel_date_str);
        static AQLDate getLADate(const AQLString& excel_date_str);

        //change excel date into AQLString
        static AQLString getLAStringDate(const int excel_date);

        //change MDate into excel date
        static int getExcelDate(const AQLDate & date);

        static AQLDate firstStubDateFromStubType(const AQLDate & startDate, const AQLDate & endDate, AQLString & term);
        static AQLDate lastStubDateFromStubType(const AQLDate & startDate, const AQLDate & endDate, AQLString & term);

        static DateVector calcDatesWithLag(const DateVector &				dates,
                                           const AQLString &					term,
                                           const AQLPriceDataSlidingRule &   slidingRule,
                                           const AQLPriceDataCalendar *      pCalendar,
                                           const bool &						isAfter,
                                           const AQLString *					rollConvention);

		// Generate a Date Schedule with appropriate use of stubs
		// Note that there is a duplicate method AQLMathDateUtilities::generateSchedule
		// Default Short/Long Start is determined by AQLMathDateUtilities::getStubDateAndType
        static DateVector generateSchedule(const AQLDate&		start,
										   const AQLDate&		end,
										   AQLString&			data_frequency,
										   AQLString&			slidingRuleString,
										   AQLString&			calendarString,
										   const AQLDate*		firstStubDate = NULL,
										   const AQLDate*		lastStubDate = NULL,
										   const int*			rollDay = NULL,
										   const bool			is_start_roll = true,
										   const AQLString*		rollConvention = NULL,
										   const AQLString*		stubType = NULL);

        static bool isValidDate(const AQLDate & dateToValidate);

        static AQLDate getDateFromTerm(const AQLDate& fromdate, const double termy, const AQLPriceDataDayCount& daycount, bool includelast = false);
        static AQLDate getDateFromTerm(AQLDate& fromdate, double termy, AQLString& daycount, bool includelast = false);
        static AQLDate getDate(const AQLDate& basedate, const AQLString& term, const AQLString& slidingRule, const AQLString& calendar);
        static AQLDate getDateWithRollConv(const AQLDate& basedate, const AQLString& term, const AQLString& slidingRule, const AQLString& calendar, const AQLString* roll_conv = NULL);
        static DateVector getMultiDate(const DateVector& basedate, const AQLString& term, const AQLString& slidingRule, const AQLString& calendar, const AQLString* roll_conv = NULL);
        static AQLDate getDateWithRoll(AQLDate& basedate, AQLString& term, AQLString& slidingRule, AQLString& calendar, int roll);
        static double getDayFromTerm(AQLDate& fromdate, double termy, AQLString& daycount, bool includelast = false);
        static double getTermFromDay(AQLDate& fromdate, double termd, AQLString& daycount, bool includelast = false);

        static double getTerm(const AQLDate& fromdate, const AQLDate& todate, AQLString& daycount, bool includelast = false,
                              const AQLString* frequency = NULL,
                              const AQLString* calendar = NULL,
                              const AQLString* slidingrule = NULL,
                              const std::vector<AQLDate>* startdates = NULL,
                              const std::vector<AQLDate>* enddates = NULL);

        static AQLDate getIMMDate1(const int& year, const int& month, AQLString& calendar, AQLString& slidingRule);
        static AQLDate getIMMDate2(const int& year, const int& number, AQLString& calendar, AQLString& slidingRule);
        static AQLDate getIMMDate3(const AQLDate& basedate, const int& number, AQLString& calendar, AQLString& slidingRule);

        static DateMatrix calcRegularDates(const AQLString& frequency,
            const AQLString& calendar,
            const AQLString& slidingrule,
            const std::vector<AQLDate>& startdates,
            const std::vector<AQLDate>& enddates);

    private:
        AQLDateScheduleHelpers(void);
        ~AQLDateScheduleHelpers(void);
        AQLDateScheduleHelpers(const AQLDateScheduleHelpers &rhs);
        AQLDateScheduleHelpers &operator=(const AQLDateScheduleHelpers &rhs);
    };

    bool is_last_business_day_temp(const AQLDate& d, const AQLString& cal);

    int StringToMonthInteger(AQLString mstr);

    AQLPriceDataDayCount Daycount(AQLString daycountConvention);

    AQLPriceDataDayCount ModelDaycount();

    double YearFraction(AQLPriceDataDayCount daycount, AQLDate valDate, AQLDate date);

    double ModelTime(AQLDate valDate, AQLDate targetDate);

    AQLDate LAStringToDate(AQLString date);

    AQLString FrequencyToTerm(AQLString frequency);

    double TermToYearLength(AQLString term);

    int TermToMonthLength(AQLString term);

    AQLDate CalendarAdvance(AQLDate baseDate, AQLString term, AQLPriceDataSlidingRule slidingRule, AQLPriceDataCalendar calendar);

}