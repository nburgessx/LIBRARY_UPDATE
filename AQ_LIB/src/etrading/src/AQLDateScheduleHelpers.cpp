//
// AQLDateScheduleHelpers.cpp

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLDateScheduleHelpers.h"
#include "AQLFunctionUtilities.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLDateHelpers.h"
#include "AQLDataReference.h"
#include "AQLAnalyticFormula.h"
#include "AQLBlackScholesCalc.h"
#include "AQLDataProcedure.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceDataConvention.h"
#include "AQLDataMultiReference.h"
#include "AQLMathDateUtilities.h"	// for getStubDateAndType
#include "ExceptionMacros.h"		// AQ_REQUIRE, AQ_THROW

#include <cmath>
#include <map>
#include <algorithm>

using namespace std;


namespace etrading
{

    //change excel date into AQLDate
    AQLDate
        AQLDateScheduleHelpers::getAQLDate(const int excel_date)
    {
        AQLDate    ret_date("19900101");
        const int excel_base = 32874;
        ret_date.addDays(excel_date - excel_base);
        return ret_date;
    }
    //change excel date into AQLDate
    AQLDate
        AQLDateScheduleHelpers::getAQLDate(const AQLString& excel_date_str)
    {
        AQLDate    ret_date("19900101");
        const int excel_base = 32874;
        ret_date.addDays(excel_date_str.getIntValue() - excel_base);
        return ret_date;
    }
    //change excel date into AQLString
    AQLString
        AQLDateScheduleHelpers::getAQLStringDate(const int excel_date)
    {
        AQLString  ret_str = getAQLDate(excel_date).stringWithFormat("YYYYMMDD");
        return ret_str;
    }

    //change MDate into excel date
    int
        AQLDateScheduleHelpers::getExcelDate(const AQLDate & date)
    {
        AQLDate    base("19900101");
        const int excel_base = 32874;
        return excel_base + base.intervalDays(date);
    }

    AQLDate AQLDateScheduleHelpers::firstStubDateFromStubType(const AQLDate & startDate, const AQLDate & endDate, AQLString & term)
    {
        upper(term);
        bool isAfter = false; // This tells getDate to calculate dates backwards from the End Date

        AQLDate result = AQLDateHelpers::getDate(endDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), isAfter, NULL);

        // Ensure the first stub is not before the start date
        if (result < startDate)
            return startDate;

        int maxCount = 20000;
        int counter = 0;

        // Update the Stub Dates
        AQLDate longStubDate = result;
        AQLDate shortStubDate = result;

        // We are rolling backwards in time
        while (shortStubDate > startDate)
        {
            // Update Long Stub Date to the previous Short Stub Date
            longStubDate = shortStubDate;

            // Update the Short Stub Date
            shortStubDate = AQLDateHelpers::getDate(shortStubDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), isAfter, NULL);

            // Update the result if the first stub is not before the start date else break out of the while loop
            if (shortStubDate > startDate)
            {
                result = longStubDate;
            }
            else
            {
                // Case Where Long and Short Start Stubs give the same result i.e. we have a regular stub which is neither short or long
                if (shortStubDate == startDate)
                {
                    result = startDate;
                }
                break;
            }

            counter++;

            AQ_THROW_IF( counter >= maxCount, "Unable to find the stub date for the stub type provided." );
        }

        return result;
    }

    AQLDate
        AQLDateScheduleHelpers::lastStubDateFromStubType(const AQLDate & startDate, const AQLDate & endDate, AQLString & term)
    {
        upper(term);
        bool isAfter = true; // This tells getDate to calculate dates forwards from the Start Date

        AQLDate result = AQLDateHelpers::getDate(startDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), isAfter, NULL);

        // Ensure the last stub is not after the end date
        if (result > endDate)
            return endDate;

        int maxCount = 20000;
        int counter = 0;

        // Update the Stub Dates
        AQLDate longStubDate = result;
        AQLDate shortStubDate = result;

        // We are rolling forwards in time
        while (shortStubDate < endDate)
        {
            // Update Long Stub Date to the previous Short Stub Date
            longStubDate = shortStubDate;

            // Update the Short Stub Date
            shortStubDate = AQLDateHelpers::getDate(shortStubDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), isAfter, NULL);

            // Update the result if the last stub is not after the end date else break out of the while loop
            if (shortStubDate < endDate)
            {
                result = longStubDate;
            }
            else
            {
                // Case Where Long and Short Start Stubs give the same result i.e. we have a regular stub which is neither short or long
                if (shortStubDate == endDate)
                {
                    result = endDate;
                }
                break;
            }

            counter++;

            if (counter >= maxCount)
                AQ_THROW( "Unable to find the stub date for the stub type provided." );
        }

        return result;
    }

    DateVector
        AQLDateScheduleHelpers::calcDatesWithLag(const DateVector &				dates,
												const AQLString &				term,
												const AQLPriceDataSlidingRule &  slidingRule,
												const AQLPriceDataCalendar *     pCalendar,
												const bool &					isAfter,
												const AQLString *				rollConvention)
    {
        DateVector results;

        for (unsigned int i = 0; i < dates.size(); ++i)
        {
            results.push_back(AQLDateHelpers::getDate(dates[i], term, slidingRule, pCalendar, isAfter, rollConvention));
        }

        return results;
    }

    // Generate a Date Schedule with appropriate use of stubs
	// Note that there is a duplicate method AQLMathDateUtilities::generateSchedule
	// Default Short/Long Start is determined by AQLMathDateUtilities::getStubDateAndType
    DateVector AQLDateScheduleHelpers::generateSchedule(const AQLDate& unadjustedStart,
													   const AQLDate& unadjustedEnd,
													   AQLString& data_frequency,
													   AQLString& slidingRuleString,
													   AQLString& calendarString,
													   const AQLDate* firstStubDate,
													   const AQLDate* lastStubDate,
													   const int* rollDay,
													   const bool is_start_roll,
													   const AQLString* rollConvention,
													   const AQLString* stubType)
    {
        //change nospace & upper
        upper(data_frequency);
        upper(slidingRuleString);
        upper(calendarString);

        AQLPriceDataSlidingRule slidingRule;
        slidingRule.convertFromString(slidingRuleString);
        
        AQLPriceDataCalendar calendar;
        calendar.convertFromString(calendarString);
        
		DateVector results;

        // ////////////////////////////////////////////////////////////////////////////////////////////////////
        // 
        // OPTIONAL: Apply Stub Type logic
        //
        // ////////////////////////////////////////////////////////////////////////////////////////////////////

        // Added to resolve const keyword issues
        bool isStartRoll = is_start_roll;

        AQLDate thisFirstStubDate;
        AQLDate thisLastStubDate;

        AQLDate* pFirstStubDate = NULL;
        AQLDate* pLastStubDate = NULL;

        if (firstStubDate != NULL)
        {
            thisFirstStubDate = *firstStubDate;
            pFirstStubDate = &thisFirstStubDate;
        }

        if (lastStubDate != NULL)
        {
            thisLastStubDate = *lastStubDate;
            pLastStubDate = &thisLastStubDate;
        }

        // Get the equivalent term from data_frequency
        AQLString term;
        if (data_frequency == "ANNUAL")
            term = "1Y";
        else if (data_frequency == "SEMI-ANNUAL")
            term = "6M";
        else if (data_frequency == "QUARTERLY")
            term = "3M";
        else if (data_frequency == "MONTHLY")
            term = "1M";
        else if (data_frequency == "WEEKLY")
            term = "1W";
		else if (data_frequency == "BUSINESS_DAYS" || data_frequency == "DAILY" )
			term = "1D";
        else if ((data_frequency == "NONE" || data_frequency == "LUNAR") && stubType == NULL);
        else
            AQ_THROW( "Frequency, must be Annual, Semi-Annual, Quarterly, Monthly, Weekly or Daily." );

		// Initialize StubDateAndType Struct
		StubDateAndType stubInfo;

		// Default Stub Type
		if (stubType == NULL && firstStubDate == NULL && lastStubDate == NULL)
		{
			// Market Default to Short Start Stub, however if Stub Days is < 7 Days the Default becomes Long Start Stub
			stubInfo = AQLMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::NONE_STUBTYPE );

			isStartRoll		= stubInfo.isStartRoll_;
			pFirstStubDate	= &stubInfo.stubDate_;
			pLastStubDate	= NULL;
		}

        // Override the stubs dates to generate the correct stub types
        if (stubType != NULL)
        {
            AQLString stubTypeString = *stubType;
            upper(stubTypeString);

            if (stubTypeString == "NONE")
            {
				if ( firstStubDate == NULL || lastStubDate == NULL )
				{ 
					// Market Default to Short Start Stub, however if Stub Days is < 7 Days the Default becomes Long Start Stub
					stubInfo = AQLMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::NONE_STUBTYPE );
				
					isStartRoll		= stubInfo.isStartRoll_;
					pFirstStubDate	= &stubInfo.stubDate_;
					pLastStubDate	= NULL;
				}
            }
            else if (stubTypeString == "SHORTSTART" || stubTypeString == "SS")
            {
				AQ_THROW_IF(firstStubDate != NULL || lastStubDate != NULL, "Must not specifiy 'StubType' with 'FirstStubDate' or 'LastStubDate'." )
                
				stubInfo = AQLMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::SHORT_START_STUBTYPE );
                
				isStartRoll		= stubInfo.isStartRoll_;
                pFirstStubDate	= &stubInfo.stubDate_;
                pLastStubDate	= NULL;
            }
            else if (stubTypeString == "LONGSTART" || stubTypeString == "LS")
            {
				AQ_THROW_IF(firstStubDate != NULL || lastStubDate != NULL, "Must not specifiy 'StubType' with 'FirstStubDate' or 'LastStubDate'." )

                stubInfo = AQLMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::LONG_START_STUBTYPE );
                
				isStartRoll		= stubInfo.isStartRoll_;
                pFirstStubDate	= &stubInfo.stubDate_;
                pLastStubDate	= NULL;
            }
            else if (stubTypeString == "SHORTEND" || stubTypeString == "SE")
            {
				AQ_THROW_IF(firstStubDate != NULL || lastStubDate != NULL, "Must not specifiy 'StubType' with 'FirstStubDate' or 'LastStubDate'." )

                stubInfo = AQLMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::SHORT_END_STUBTYPE );

                isStartRoll		= stubInfo.isStartRoll_;
                pFirstStubDate	= NULL;
                pLastStubDate	= &stubInfo.stubDate_;
            }
            else if (stubTypeString == "LONGEND" || stubTypeString == "LE")
            {
				AQ_THROW_IF(firstStubDate != NULL || lastStubDate != NULL, "Must not specifiy 'StubType' with 'FirstStubDate' or 'LastStubDate'." )

                stubInfo = AQLMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::LONG_END_STUBTYPE );

                isStartRoll		= stubInfo.isStartRoll_;
                pFirstStubDate	= NULL;
                pLastStubDate	= &stubInfo.stubDate_;
            }
            else
            {
                AQ_THROW("Stub Type must be None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).")
            }
        }
        // ////////////////////////////////////////////////////////////////////////////////////////////////////

        AQLDateHelpers::generateSchedule(unadjustedStart,
										unadjustedEnd,
										data_frequency,
										true,
										pFirstStubDate,
										pLastStubDate,
										rollDay,
										results,
										&slidingRule,
										&calendar,
										isStartRoll,
										rollConvention );

        if (results.front() != unadjustedStart)
		{
			results.insert(results.begin(), unadjustedStart);
		}

        return results;
    }

    /*!
    @brief Function to build a dates schedule for swap pricing

    @param[in] testDate
    @param[in] throwString

    @returns True if 'dateToValidate' is valid and false otherwise

    */
    bool
        AQLDateScheduleHelpers::isValidDate(const AQLDate & dateToValidate)
    {
        const unsigned short  mDay = dateToValidate.dayOfMonth();
        const unsigned short  mMonth = dateToValidate.monthOfYear();
        const unsigned short  mYear = dateToValidate.yearOfEra();
        const unsigned short  mLeap = dateToValidate.isLeapYear();

        // number of days each month and the number of days from the beginning of the year in an usual yer and a leap year(0 start)
        const unsigned short LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[2][2][12] =
        { { { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
        { 0, 31, 59, 90,120,151,181,212,243,273,304,334 } },
        { { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
        { 0, 31, 60, 91,121,152,182,213,244,274,305,335 } } };

        // Test for a valid date
        if (mYear <= 1900)
            return false;   //dateToValidate is invalid

        if (mYear == 0)
            return false;   //dateToValidate is invalid

        if (mMonth == 0 || mMonth > 12)
            return false;   //dateToValidate is invalid

        if (mDay == 0 || mDay > (int)(LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth - 1]))
            return false;   //dateToValidate is invalid

                            // dateToValidate is valid
        return true;
    }

    AQLDate
        AQLDateScheduleHelpers::getDateFromTerm(const AQLDate& fromdate, const double termy, const AQLPriceDataDayCount& daycount, bool includelast)
    {
        AQLPriceDataConvention convention(daycount.getDayCount(), CONT); // Continous compounding rate convention

        AQLDate ret = fromdate;
        double fullDay = AQLPriceCFGenUtility::round(convention.getDayTerm(ret, termy), RoundFunction::ROUND, 0);
        ret.addDays(fullDay);

        return ret;
    }

    AQLDate
        AQLDateScheduleHelpers::getDateFromTerm(AQLDate& fromdate, double termy, AQLString& dayCountString, bool includelast)
    {
        includelast;
        //change nospace & upper
        upper(dayCountString);

        AQLPriceDataDayCount daycount;
        daycount.setDayCount(AQLCoreComponentManager::getDayCount(dayCountString));
        AQLDate ret = getDateFromTerm(fromdate, termy, daycount);
        return ret;
    }

    double
        AQLDateScheduleHelpers::getDayFromTerm(AQLDate& fromdate, double termy, AQLString& daycount, bool includelast)
    {
        //change nospace & upper
        upper(daycount);

        AQLPriceDataDayCount dc;
        dc.setDayCount(AQLCoreComponentManager::getDayCount(daycount));
        AQLPriceDataConvention conv(dc.getDayCount(), CONT);

        //false means not includelast
        double ret = dc.getDayTerm(fromdate, termy, includelast);
        return ret;
    }
    double
        AQLDateScheduleHelpers::getTermFromDay(AQLDate& fromdate, double termd, AQLString& daycount, bool includelast)
    {
        //change nospace & upper
        upper(daycount);

        AQLPriceDataDayCount dc;
        dc.setDayCount(AQLCoreComponentManager::getDayCount(daycount));
        AQLPriceDataConvention conv(dc.getDayCount(), CONT);

        //false means not includelast
        double ret = dc.getTerm(fromdate, termd, includelast);
        return ret;
    }
    double
        AQLDateScheduleHelpers::getTerm(const AQLDate& fromdate, const AQLDate& todate, AQLString& daycount, bool includelast,
            const AQLString* frequency,
            const AQLString* Calendar,
            const AQLString* SlidingRule,
            const std::vector<AQLDate>* startdates,
            const std::vector<AQLDate>* enddates)
    {
        //change nospace & upper
        upper(daycount);

        AQLPriceDataDayCount dc;
        dc.setDayCount(AQLCoreComponentManager::getDayCount(daycount));

        if (dc.getDayCount() == ACT_ACT_ICMA)
        {
            DateMatrix regular_startenddates = AQLDateScheduleHelpers::calcRegularDates(*frequency, *Calendar, *SlidingRule, *startdates, *enddates);
            dc.setCouponsInYear(12 / AQLDateHelpers::getPeriodFrequencyInMonths(*frequency));
            dc.setCouponStartDates(regular_startenddates[0]);
            dc.setCouponEndDates(regular_startenddates[1]);
        }

        AQLPriceDataConvention conv(dc.getDayCount(), CONT);
        //false means not includelast
        double ret = dc.getTerm(fromdate, todate, includelast);
        return ret;
    }

    AQLDate
        AQLDateScheduleHelpers::getDate(const AQLDate& basedate, const AQLString& term, const AQLString& slidingrule, const AQLString& calendar)
    {
        //calendar
        AQLPriceDataCalendar cal;
        cal.convertFromString(calendar);

        //sliding rule
        AQLPriceDataSlidingRule sr;
        sr.convertFromString(slidingrule);

        AQLDate ret = AQLDateHelpers::getDate(basedate, term, sr, &cal, true);
        return ret;
    }

    AQLDate
        AQLDateScheduleHelpers::getDateWithRollConv(const AQLDate& basedate, const AQLString& term, const AQLString& slidingRule, const AQLString& calendar, const AQLString* roll_conv)
    {
        //calendar
        AQLPriceDataCalendar cal;
        cal.convertFromString(calendar);

        //sliding rule
        AQLPriceDataSlidingRule sr;
        sr.convertFromString(slidingRule);

        AQLDate ret = AQLDateHelpers::getDate(basedate, term, sr, &cal, true, roll_conv);

        return ret;
    }

    DateVector AQLDateScheduleHelpers::getMultiDate(const DateVector& basedate, const AQLString& term, const AQLString& slidingrule, const AQLString& calendar, const AQLString* roll_conv)
    {
        //calendar
        AQLPriceDataCalendar cal;
        cal.convertFromString(calendar);
        
		//sliding rule
        AQLPriceDataSlidingRule sr;
        sr.convertFromString(slidingrule);

        DateVector results( basedate.size() );
        for (size_t i = 0; i < basedate.size(); i++)
        {
            results[i] = AQLDateHelpers::getDate(basedate[i], term, sr, &cal, true, roll_conv);
        }
        return results;
    }

    AQLDate
        AQLDateScheduleHelpers::getDateWithRoll(AQLDate& basedate, AQLString& term, AQLString& slidingrule, AQLString& calendar, int roll)
    {
        //change nospace & upper
        upper(term);
        upper(slidingrule);
        upper(calendar);
        //sliding rule
        AQLPriceDataSlidingRule sr;
        sr.convertFromString(slidingrule);
        //calendar
        AQLPriceDataCalendar cal;
        cal.convertFromString(calendar);

        int y, m, d, w;
        AQLDateHelpers::termStrtoYMDW(term, y, m, d, w);
        y *= roll;
        m *= roll;
        d *= roll;
        AQLString multiterm = AQLString(y) + "y" + AQLString(m) + "m" + AQLString(d) + "d";

        AQLDate ret = AQLDateHelpers::getDate(basedate, multiterm, sr, &cal, true);
        return ret;
    }

    AQLDate
        AQLDateScheduleHelpers::getIMMDate1(const int& year, const int& month, AQLString& calendar, AQLString& slidingRule)
    {
        upper(slidingRule);
        upper(calendar);
        AQLPriceDataSlidingRule sr;
        sr.convertFromString(slidingRule);
        AQLPriceDataCalendar cal;
        cal.convertFromString(calendar);

        AQLDate date = AQLDateHelpers::getIMMDate(year, month);
        return sr.getDate(date, cal);
    }

    AQLDate
        AQLDateScheduleHelpers::getIMMDate2(const int& year, const int& number, AQLString& calendar, AQLString& slidingRule)
    {
        if (number < 0 || number > 5) AQ_THROW( "IMM Dates in a year are 4 days." );

        upper(slidingRule);
        upper(calendar);
        AQLPriceDataSlidingRule sr;
        sr.convertFromString(slidingRule);
        AQLPriceDataCalendar cal;
        cal.convertFromString(calendar);

        AQLDate date = AQLDateHelpers::getIMMDate(year, number * 3);
        return sr.getDate(date, cal);
    }

    AQLDate
        AQLDateScheduleHelpers::getIMMDate3(const AQLDate& basedate, const int& number, AQLString& calendar, AQLString& slidingRule)
    {
        upper(slidingRule);
        upper(calendar);
        AQLPriceDataSlidingRule sr;
        sr.convertFromString(slidingRule);
        AQLPriceDataCalendar cal;
        cal.convertFromString(calendar);

        int y_baseDate = basedate.yearOfEra();
        int m_baseDate = basedate.monthOfYear();

        AQLDate nextIMMDate;
        nextIMMDate.setYear(y_baseDate);
        nextIMMDate.setMonth(m_baseDate);

        if (m_baseDate % 3 == 0)
        {
            AQLDate IMMDate_baseDateMonth = AQLDateHelpers::getIMMDate(y_baseDate, m_baseDate);
            IMMDate_baseDateMonth = sr.getDate(IMMDate_baseDateMonth, cal);
            if (IMMDate_baseDateMonth <= basedate) nextIMMDate.addMonths(3);
        }
        else if (m_baseDate % 3 == 1)
        {
            nextIMMDate.addMonths(2);
        }
        else if (m_baseDate % 3 == 2)
        {
            nextIMMDate.addMonths(1);
        }
        nextIMMDate.addMonths((number - 1) * 3);
        nextIMMDate = AQLDateHelpers::getIMMDate(nextIMMDate.yearOfEra(), nextIMMDate.monthOfYear());
        return sr.getDate(nextIMMDate, cal);
    }

    // calc regular (non-stub) payment dates for daycount ACT/ACT.ICMA
    DateMatrix
        AQLDateScheduleHelpers::calcRegularDates(const AQLString& frequency,
            const AQLString& Calendar,
            const AQLString& SlidingRule,
            const std::vector<AQLDate>& startdates,
            const std::vector<AQLDate>& enddates)
    {
        AQLPriceDataCalendar calendar;
        calendar.convertFromString(Calendar);
        AQLPriceDataSlidingRule slidingrule;
        slidingrule.convertFromString(SlidingRule);
        AQLString term = (AQLString)(AQLDateHelpers::getPeriodFrequencyInMonths(frequency)) + "M";

        bool is_sorted_Start = true;
        bool is_sorted_End = true;
        for (unsigned int i = 1; i < startdates.size(); ++i)
        {
            if (startdates[i - 1] >= startdates[i])
            {
                is_sorted_Start = false;
            }
        }
        for (unsigned int i = 1; i < enddates.size(); ++i)
        {
            if (enddates[i - 1] >= enddates[i])
            {
                is_sorted_End = false;
            }
        }

        if (startdates.size() != enddates.size() ||
            startdates.size() == 0 ||
            !is_sorted_Start ||
            !is_sorted_End)
        {
            AQLString msg = "Illegal StartDates and/or EndDates.";
            AQ_THROW( msg.getCString() );
        }

        // merge StartDates and EndDates as a single date vector
        std::vector<AQLDate> startenddates;
        std::set_union(startdates.begin(), startdates.end(),
            enddates.begin(), enddates.end(),
            std::inserter(startenddates, startenddates.end()));

        if (startenddates.size() != startdates.size() + 1) // case when Start[i+1] does not coincide with End[i]
        {
            AQLString msg = "Illegal StartDates and/or EndDates.";
            AQ_THROW( msg.getCString() );
        }

        std::vector<AQLDate> regular_startdates;
        std::vector<AQLDate> regular_enddates;
        for (unsigned int i = 0; i < startdates.size(); ++i)
        {
            AQLDate startdate = startdates[i];
            AQLDate enddate = enddates[i];

            if (i == 0)
            {
                regular_startdates.push_back(AQLDateHelpers::getDate(enddate, "-" + term, slidingrule, &calendar, true));
                regular_enddates.push_back(enddate);
                while (regular_startdates.back() > startdate) // Long at start
                {
                    regular_enddates.push_back(regular_startdates.back());
                    regular_startdates.push_back(AQLDateHelpers::getDate(regular_enddates.back(), "-" + term, slidingrule, &calendar, true));
                }
            }
            else if (i == startdates.size() - 1)
            {
                regular_startdates.push_back(startdate);
                regular_enddates.push_back(AQLDateHelpers::getDate(startdate, term, slidingrule, &calendar, true));
                while (regular_enddates.back() < enddate) // Long at end
                {
                    regular_startdates.push_back(regular_enddates.back());
                    regular_enddates.push_back(AQLDateHelpers::getDate(regular_startdates.back(), term, slidingrule, &calendar, true));
                }
            }
            else
            {
                regular_startdates.push_back(startdate);
                regular_enddates.push_back(enddate);
            }
        }
        std::sort(regular_startdates.begin(), regular_startdates.end());
        std::sort(regular_enddates.begin(), regular_enddates.end());

        DateMatrix ret;
        ret.push_back(regular_startdates);
        ret.push_back(regular_enddates);
        return ret;
    }

    bool is_last_business_day_temp(const AQLDate& d, const AQLString& cal)
    {
        const AQLDate next_day = AQLDateScheduleHelpers::getDate(d, "1d", "FOLLOWING", cal);
        return next_day.monthOfYear() != d.monthOfYear();
    }

    int StringToMonthInteger(AQLString mstr)
    {
        int m;
        if (mstr == "Jan") m = 1;
        else if (mstr == "Feb") m = 2;
        else if (mstr == "Mar") m = 3;
        else if (mstr == "Apr") m = 4;
        else if (mstr == "May") m = 5;
        else if (mstr == "Jun") m = 6;
        else if (mstr == "Jul") m = 7;
        else if (mstr == "Aug") m = 8;
        else if (mstr == "Sep") m = 9;
        else if (mstr == "Oct") m = 10;
        else if (mstr == "Nov") m = 11;
        else if (mstr == "Dec") m = 12;
        else AQ_THROW( "invalid month" );

        return m;
    }

    AQLString FrequencyToTerm(AQLString frequency)
    {
        AQLString frequency_(frequency);
        frequency_.toUpper();
        if (frequency_ == BUSINESS_DAYS || frequency_ == DAILY)
            return "1D";
        else if (frequency_ == WEEKLY)
            return "1W";
        else if (frequency_ == LUNAR)
            return "28D";
        else if (frequency_ == MONTHLY)
            return "1M";
        else if (frequency_ == QUARTERLY)
            return "3M";
        else if (frequency_ == SEMI_ANNUAL)
            return "6M";
        else if (frequency_ == ANNUAL)
            return "12M";
        else
            AQ_THROW( "Unknown frequency in conversion to Term" );
    }

    AQLPriceDataDayCount ModelDaycount()
    {
        AQLString daycountConvention = AQLString("ACT/365");
        return etrading::Daycount(daycountConvention);
    }

    double ModelTime(AQLDate valDate, AQLDate targetDate)
    {
        AQLPriceDataDayCount dayCount = ModelDaycount();
        return etrading::YearFraction(dayCount, valDate, targetDate);
    }

    AQLDate AQLStringToDate(AQLString date)
    {
        AQLDate ret;
        int slushCheck = date.findString("/");
        if (slushCheck == -1)
        {
            ret = AQLDateScheduleHelpers::getAQLDate(date);
        }
        else
        {
            if (slushCheck == 4) date.remove(4, 1);
            else AQ_THROW( "Input error" );

            slushCheck = date.findString("/");
            if (slushCheck == 6) { date.remove(6, 1); }
            else if (slushCheck == 5)
            {
                date.remove(5, 1);
                date.insert(4, "0");
            }
            else AQ_THROW( "Input error" );

            if (date.size() == 7)
            {
                date.insert(6, "0");
            }
            else if (date.size() != 8) AQ_THROW( "Input error" );
            AQLDate ret_(date.getCString()); ret = ret_;
        }

        return ret;
    }

    double YearFraction(AQLPriceDataDayCount daycount, AQLDate valDate, AQLDate date)
    {
        return daycount.getTerm(valDate, date, false);
    }

    AQLPriceDataDayCount Daycount(AQLString daycountConvention)
    {
        AQLFunctionUtilities::upperdelspace(daycountConvention);
        AQLPriceDataDayCount dc;
        dc.setDayCount(AQLCoreComponentManager::getDayCount(daycountConvention));
        return dc;
    }

    double TermToYearLength(AQLString term)
    {
        size_t termSize = term.size();
        AQLString unit = term.subString(termSize - 1, termSize - 1);
        double length = term.subString(0, termSize - 2).getDoubleValue();
        if (unit == "Y")
            return length;
        else if (unit == "M")
            return length / 12.0;
        else
            AQ_THROW( "Invalid term in conversion to year length" );
    }

    int TermToMonthLength(AQLString term)
    {
        size_t termSize = term.size();
        AQLString unit = term.subString(termSize - 1, termSize - 1);
        int length = term.subString(0, termSize - 2).getIntValue();
        if (unit == "M")
            return length;
        else if (unit == "Y")
            return 12 * length;
        else
            AQ_THROW( "Invalid term in conversion to month length" );
    }


    AQLDate CalendarAdvance(AQLDate baseDate, AQLString term, AQLPriceDataSlidingRule slidingRule, AQLPriceDataCalendar calendar)
    {
        AQLString* roll_conv = 0;
        return AQLDateHelpers::getDate(baseDate, term, slidingRule, &calendar, true, roll_conv);
    }

}