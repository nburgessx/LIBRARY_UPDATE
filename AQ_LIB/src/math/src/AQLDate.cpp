#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLDate.h"
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <string>

static const long DAYS_400               = 146097;   // number of days in 400 years
static const long DAYS_100               = 36524;    // number of days in 100 years
static const int  DAYS_4                 = 1461;     // number of days in 4 years

static const int  DEFAULT_DAYS_OF_YEAR   = 365;      // number of days in an usual year
static const int  MONTHS_OF_YEAR         = 12;       // number of months
static const int  DAYS_OF_WEEK           = 7;        // number of days in a week
static const int  WEEK_ADJUST            = 0;        // week adjust

// YEAR_RANK/MONTH_RANK moved to AQLDate.h as AQLDATE_CMP_YEAR_RANK/AQLDATE_CMP_MONTH_RANK
// (2026-09-20) - cmp() itself moved there too, see its comment for why.


/* 
    Number of Days in a Month and Cumulative Total Number of Days for the Year
    First table is for non-leap years and the second is for leap years
    
    Index References:
    -----------------
    First Index     [ 0 = Non-LeapYear, 1 = Leap-Year ]
    Second Index    [ 0 = DaysInMonth,  1 = Total Days from Start of Year ]
    Third Index     [ nth Month (base zero) ]

*/
static const unsigned short LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[2][2][12] =
{   
    {   
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        { 0, 31, 59, 90,120,151,181,212,243,273,304,334} 
    },
    {   {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        { 0, 31, 60, 91,121,152,182,213,244,274,305,335}
    }
};


// Formatring Constants
enum _MFORMAT {YYYY = 0, MM, DD, DEFAULT};       

static const char_t* FORMAT[4]          = {"YYYY", "MM", "DD", "DEFAULT"};
static const unsigned int FLENGTH[4]    = {4, 2, 2, 1};
static const char_t* OUT_FORM[4]        = {"%04d","%02d", "%02d", "%01d"};


inline int  DAY_OF_WEEK(long julius)  { return (julius%DAYS_OF_WEEK + WEEK_ADJUST); }


AQLDate::AQLDate(void) : leap_(0), year_(1), month_(1), day_(1), julius_(1)
{
}

AQLDate::AQLDate( const char_t* date, const char_t* format)
{
	setDate(date, format);
}

AQLDate::AQLDate( const AQLDate& date)
{
    copy(date);
}

AQLDate& AQLDate::operator=(const AQLDate& date)
{
    if (this != &date)
    {
        copy(date);
    }
    return *this;
}

AQLDate::~AQLDate(void)
{
}


void AQLDate::setDate(const char_t  *date, const char_t* format)
{
    formatWithString(date, format);
    leap_ = IS_LEAP_YEAR(year_);
    checkDate();                                  // validate before touching julius_, not after
    julius_.store(0, std::memory_order_relaxed);   // invalidate - see the julius_ field comment in AQLDate.h
}

void AQLDate::setSystemDate(void)
{
    struct tm date;
    time_t now;

    now = time(NULL);
    localtime_s(&date, &now);      // thread-safe: localtime() writes through a shared static buffer
    year_   = (unsigned short)(date.tm_year + 1900);
    month_  = (unsigned short)(date.tm_mon + 1);
    day_    = (unsigned short)(date.tm_mday);
    leap_   = IS_LEAP_YEAR(year_);
    julius_.store(0, std::memory_order_relaxed);
}

AQLDate AQLDate::today(void)
{
    AQLDate result;
    result.setSystemDate();
    return result;
}

bool AQLDate::isValidDate(const char_t* date, const char_t* format) noexcept
{
    try
    {
        AQLDate probe(date, format);
        (void)probe;
        return true;
    }
    catch (const AQLCoreError&)
    {
        return false;
    }
    catch (...)
    {
        return false;
    }
}

void AQLDate::setYear( unsigned int year)
{
    if (year == 0 )
    {
		AQLString msg = "Invalid year[";
		msg += AQLString((int)year) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    year_ = (unsigned short)year;
    leap_ = IS_LEAP_YEAR(year_);
    adjustDayForMonthEnd();
    julius_.store(0, std::memory_order_relaxed);
}

void AQLDate::setMonth(unsigned int month)
{
    if (month == 0 || month > 12)
    {
		AQLString msg = "Invalid month[";
		msg += AQLString((int)month) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    month_ = (unsigned short)month;
    adjustDayForMonthEnd();
    julius_.store(0, std::memory_order_relaxed);
}

void AQLDate::setDay(unsigned int day)
{
    if (day == 0 || day > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][0][month_ - 1]) 
    {
		AQLString msg = "Invalid day[";
		msg += AQLString((int)day) + "] for month=" + AQLString((int)month_);
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    long cached = julius_.load(std::memory_order_relaxed);
    if (cached != 0)
    {
        // if the Julian day has already been computed, keep it current with an O(1) adjustment
        // instead of forcing a full recompute
        julius_.store(cached + ((long)day-(long)day_), std::memory_order_relaxed);
    }
    day_ = (unsigned short)day;
}

void AQLDate::addDays(int adays)
{
    long julius = ensureJulius() + adays;
    if (julius < 0)
    {
		AQLString msg = "Invalid days[";
		msg += AQLString(adays) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    julius_.store(julius, std::memory_order_relaxed);
    juliusToDate(); //set all the variables
}

void AQLDate::addWeeks(int weeks)
{
    long julius = ensureJulius() + weeks * 7;
    if (julius < 0)
    {
		AQLString msg = "Invalid days[";
		msg += AQLString(weeks) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    julius_.store(julius, std::memory_order_relaxed);
    juliusToDate(); //set all the variables
}

void AQLDate::addMonths(const int months)
{
    int allMonths = 12 * year_ + month_ + months - 1;

    if (allMonths < 12)
    {
		AQLString msg = "Invalid months[";
		msg += AQLString(months) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    year_       = (unsigned short)(allMonths / 12);
    month_      = (unsigned short)(allMonths % 12) + 1;
    leap_       = IS_LEAP_YEAR(year_);
    adjustDayForMonthEnd();
    julius_.store(0, std::memory_order_relaxed);
}

void AQLDate::addYears(int years)
{
    if ( - years > (int)year_ )
    {
		AQLString msg = "Invalid years[";
		msg += AQLString(years) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    year_ = year_ + (unsigned short)years;
    leap_ = IS_LEAP_YEAR(year_);
    adjustDayForMonthEnd();
    julius_.store(0, std::memory_order_relaxed);
}

AQLDayOfWeekEnum AQLDate::dayOfWeek(void) const noexcept
{
    return AQLDayOfWeekEnum(DAY_OF_WEEK(ensureJulius()));
}

long AQLDate::julianDayNumber(void) const noexcept
{
    return ensureJulius();
}

AQLDate AQLDate::fromJulianDayNumber(long julianDay)
{
    if (julianDay < 1)
    {
        AQLString msg = "Invalid Julian day number[";
        msg += AQLString((int)julianDay) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    AQLDate result;
    result.julius_.store(julianDay, std::memory_order_relaxed);
    result.juliusToDate();
    return result;
}

// ---------------------------------------------------------------------------------------------
// Excel date serial conversion (the "1900 date system" Excel defaults to).
//
// Excel's date system has exactly one well-known landmine: it incorrectly treats 1900 as a leap
// year (it wasn't - 1900 is divisible by 100 but not 400), so it counts a fictitious 29 February
// 1900 as serial 60. Every real date from 1 March 1900 onward is therefore one serial number
// higher than a straight day-count from 1 January 1900 would give - the constants below encode
// that split explicitly rather than papering over it, which is what makes this trustworthy rather
// than "probably right".
//
// The two offset constants are this class's own Julian day number for 1900-01-01 (693596) and
// 1900-03-01 (693655) folded into "Julian day -> Excel serial" form, and were cross-checked two
// independent ways before trusting them: (1) by hand against this class's own dateToJulius()
// arithmetic for 1900-01-01/1900-02-28/1900-03-01, and (2) against two already-shipped constants
// elsewhere in this codebase that have nothing to do with this file - AQ_XLL/src/aqXllTools.cpp's
// excelLocalSerial() uses 25569 as the Unix-epoch (1970-01-01) Excel serial, and this class's own
// julianDayNumber() for 1970-01-01 is 719163; 719163 - 693594 = 25569, and separately, this class's
// julianDayNumber() for 2024-01-01 minus 693594 is 45292, the commonly-cited real Excel serial for
// that date. Three independent routes to the same constant.
namespace
{
    const long EXCEL_EPOCH_JULIAN_BEFORE_MARCH_1900 = 693595; // Julian day 693596 (1900-01-01) - 1
    const long EXCEL_EPOCH_JULIAN_ON_OR_AFTER_MARCH_1900 = 693594;
    const long EXCEL_MARCH_1_1900_JULIAN = 693655;            // this class's Julian day for 1900-03-01
    const long EXCEL_FICTITIOUS_FEB_29_1900_SERIAL = 60;
}

double AQLDate::toExcelDate(void) const
{
    const long julian = julianDayNumber();
    const long serial = (julian >= EXCEL_MARCH_1_1900_JULIAN)
                       ? (julian - EXCEL_EPOCH_JULIAN_ON_OR_AFTER_MARCH_1900)
                       : (julian - EXCEL_EPOCH_JULIAN_BEFORE_MARCH_1900);
    if (serial < 1)
    {
        AQLString msg = "toExcelDate: date is before 1900-01-01 [";
        msg += stringWithFormat() + "], which Excel's date system cannot represent";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    return static_cast<double>(serial);
}

AQLDate AQLDate::fromExcelDate(double excelSerial)
{
    // Truncate, don't round: a caller handing over a datetime serial (integer day + fractional
    // time-of-day) wants the calendar date it falls within, not the nearest one.
    const long serial = static_cast<long>(excelSerial);

    if (serial < 1)
    {
        AQLString msg = "fromExcelDate: serial[";
        msg += AQLString((double)excelSerial) + "] is before 1900-01-01, which Excel's date system cannot represent";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if (serial == EXCEL_FICTITIOUS_FEB_29_1900_SERIAL)
    {
        AQLString msg = "fromExcelDate: serial 60 is Excel's fictitious 29 February 1900 (an artefact of Excel's own 1900-leap-year bug) - not a real date";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    const long julian = (serial < EXCEL_FICTITIOUS_FEB_29_1900_SERIAL)
                       ? (serial + EXCEL_EPOCH_JULIAN_BEFORE_MARCH_1900)
                       : (serial + EXCEL_EPOCH_JULIAN_ON_OR_AFTER_MARCH_1900);
    return AQLDate::fromJulianDayNumber(julian);
}

std::vector<double> AQLDate::toExcelDates(const std::vector<AQLDate>& dates)
{
    std::vector<double> result;
    result.reserve(dates.size());
    for (const AQLDate& d : dates)
    {
        result.push_back(d.toExcelDate());
    }
    return result;
}

std::vector<AQLDate> AQLDate::fromExcelDates(const std::vector<double>& excelSerials)
{
    std::vector<AQLDate> result;
    result.reserve(excelSerials.size());
    for (double serial : excelSerials)
    {
        result.push_back(AQLDate::fromExcelDate(serial));
    }
    return result;
}

// dayOfMonth() / monthOfYear() / yearOfEra() are defined inline in AQLDate.h

AQLString AQLDate::stringWithFormat(const char_t* format) const
{
    AQLString st;
	try 
	{
        formatWithLong(st, format);
	}
	catch (AQLCoreError & e)
	{
		AQLString msg = "Invalid format = ";
		msg += format;
        throw e += AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
    return st;
}

AQLString AQLDate::convertDateToString( const char_t* format) const
{
    AQLString st;

    // Do nothing except return the null string if this is still the default-constructed sentinel
    // date. Checked via isNull() (year_/month_/day_), not julius_ - julius_ is now a lazy cache that
    // legitimately reads 0 ("not yet computed") for a perfectly valid, non-null date.
    if ( !isNull() )
    {
	    st = stringWithFormat( format );
    }

    return st;
}

// cmp() moved inline into AQLDate.h (2026-09-20 profiling fix) - see its comment there for why.

int AQLDate::intervalDays(const AQLDate& toDate) const noexcept
{
    return (int)(toDate.ensureJulius() - ensureJulius());
}

int AQLDate::intervalMonths(const AQLDate& toDate) const noexcept
{
    int days;
    int months;
    int years;
    intervalYMD(toDate, years, months, days);

    return months;     
}

int AQLDate::intervalYears(const AQLDate& toDate) const noexcept
{
    int days;
    int months;
    int years;
    intervalYMD(toDate, years, months, days);

    return years;
}

void AQLDate::intervalYMD(const AQLDate& toDate, int&  years, int&  months, int&  days) const noexcept
{
    const AQLDate* to;
    const AQLDate* from;
    int         sign;

    if (cmp(toDate) <= 0) 
    {
        to   = &toDate;
        from = this;
        sign = 1;
    }
    else
    {
        from = &toDate;
        to   = this;
        sign = -1;
    }
    
    days    = to->day_ - from->day_; 
	months  = to->month_ - from->month_;
	years   = to->year_ - from->year_;
    
    if (days < 0) 
    {
		if (to->month_ != 1)
		{
			// allocation to the days of one month "to"
			days += LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[to->leap_][0][to->month_ - 2];
			months -= 1;
		}
		else 
		{
            // in case of Jan.
			// modify "to" into "year_-1"/"12"/"days+31"
			days += 31; //
			months += 11;
			years -=1;
		}
		// to->month_ is not Feb. since there is 31 days in Jan.
		// and days < 0 means 31 is not added.
		if (days < 0)
		{
			// to->month_ - 3 is only ever reached here with to->month_ == 3 (borrowing a day count
			// short enough - February's 28/29 - to still leave days negative can only happen when
			// to->month_ is the month right after February); the +12/%12 makes that provably safe
			// to the static analyzer (C6385) instead of relying on that unproven cross-branch
			// invariant, with no change in behaviour for the one case that is actually reachable.
			days += LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[to->leap_][0][((int)to->month_ - 3 + 12) % 12];
			months -= 1;
		}
	}
	if (months < 0)
	{
		months += 12;
		years -= 1;
	}

    days   *= sign;
    months *= sign;
    years  *= sign;
}

int AQLDate::intervalToStartOfMonth(void) const noexcept
{
	return 1 -(int)day_;
}

int AQLDate::intervalToEndOfMonth(void) const noexcept
{
    return (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][0][month_ - 1] - (int)day_;
}

AQLDate AQLDate::startOfMonth(void) const
{
    AQLDate result(*this);
    result.addDays(intervalToStartOfMonth());
    return result;
}

AQLDate AQLDate::endOfMonth(void) const
{
    AQLDate result(*this);
    result.addDays(intervalToEndOfMonth());
    return result;
}

int AQLDate::daysInMonth(void) const noexcept
{
    return (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][0][month_ - 1];
}

int AQLDate::intervalToStartOfYear(void) const noexcept
{
    return 1 - (int)day_ - (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][1][month_ - 1];
}

int AQLDate::intervalToEndOfYear(void) const noexcept
{
    return   (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][1][11] + 31
           - (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][1][month_ - 1]
           - (int)day_;
}

int AQLDate::intervalToNextDateOfWeek(AQLDayOfWeekEnum next) const noexcept
{
    int ret = (int)next - (int)dayOfWeek();
    return (ret < 0 ? ret + DAYS_OF_WEEK : ret);
}

bool AQLDate::isLeapYear(void) const noexcept
{
	return leap_ == 1;
}

bool AQLDate::isStartOfMonth(void) const noexcept
{
	return day_ == 1;
}

bool AQLDate::isEndOfMonth(void) const noexcept
{
	return day_ == LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][0][month_ - 1];
}

bool AQLDate::isStartOfYear(void) const noexcept
{
	return day_ == 1 && month_ == 1;
}

bool AQLDate::isEndOfYear(void) const noexcept
{
	return day_ == 31 && month_ == 12;
}

void AQLDate::formatWithString(const char_t  *st, const char_t  *format)
{
    int  dateTime[3]    = {0, 0, 0};
    int  setFlag[3]     = {0, 0, 0};

    unsigned int     i;
    unsigned int     length;
    unsigned int     setPos;
    char_t           wkDateTime[5];

	// format check
	if (st == NULL || format == NULL)
	{
		AQLString msg = "Invalid Data ";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

    if ((length = STRLEN(st)) != STRLEN(format)) 
    {
		AQLString msg = "Invalid format ";
		msg += st;
		msg += " : ";
		msg += format;
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    
    for (i = 0; i < length;) 
    {
        if (format[i] == 'Y')
		{
			setPos = YYYY;
		}
        else if (format[i] == 'M') 
		{
			setPos = MM ;
		}
        else if (format[i] == 'D') 
		{
			setPos = DD;
		}
        else
        {
            if (format[i] != st[i]) 
            {
				AQLString msg = "Invalid format ";
				msg += st;
				msg += " : ";
				msg += format;
				msg += " at ";
				msg += AQLString((int)i);
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
            }
            i++;
            continue;
        }
		
        // match FORMAT
        if (memcmp(FORMAT[setPos], &(format[i]), FLENGTH[setPos]) != 0) 
        {
			// then error
            if (format[i] != st[i]) 
            {
				AQLString msg = "Invalid format ";
				msg += st;
				msg += " : ";
				msg += format;
				msg += " at ";
				msg += AQLString((int)i);
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
            }
			++i;
			continue;
        }
        if (setFlag[setPos] != 0) 
        {  // error
			AQLString msg = "Invalid format ";
			msg += st;
			msg += " : ";
			msg += format;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
		
        // all ok so set data
        strncpy(wkDateTime, &(st[i]), FLENGTH[setPos]);
        wkDateTime[FLENGTH[setPos]]     = '\0';
        char * pFirstNonNumber;
        dateTime[setPos]                = strtol( wkDateTime, &pFirstNonNumber, 10); // base 10 numbers
        setFlag[setPos]                 = 1;  //flag up
		i += FLENGTH[setPos];
    }
    
	year_  = (unsigned short)dateTime[0];
	month_ = (unsigned short)dateTime[1];
	day_   = (unsigned short)dateTime[2];

}

void AQLDate::formatWithLong(AQLString& rSt, const char_t  *format) const
{
    int             dateTime[3];
    unsigned int    i;
    unsigned int    setPos;
    // Sized for the worst case, not just the common one: OUT_FORM[YYYY] is "%04d", and year_ is
    // unsigned short (max 65535, 5 digits) - SPRINTF(..., "%04d", 65535) writes "65535" + a null
    // terminator, 6 chars, which overflowed the previous 5-char buffer by one byte for any year
    // >= 10000. MM/DD only ever hold 1-12/1-31 so 8 chars is generous headroom for all three.
    char_t          strFormat[8];
       
    rSt = format;

    dateTime[YYYY] = year_;
    dateTime[MM]   = month_;
    dateTime[DD]   = day_;

    for (i = 0; i < rSt.size();) 
	{
        if (format[i] == 'Y')      
		{
			setPos = YYYY;
		}
        else if (format[i] == 'M') 
		{
			setPos = MM;
		}
        else if (format[i] == 'D') 
		{
			setPos = DD;
		}
        else 
        {
            ++i;
            continue;
		}
        if (memcmp(FORMAT[setPos], &(format[i]), FLENGTH[setPos]) != 0) 
		{
            ++i;
            continue;
        }

        // set in accordance with format
        SPRINTF(strFormat, OUT_FORM[setPos], dateTime[setPos]);
        rSt.replace(i, strFormat);
        i += FLENGTH[setPos];
    }
}

void AQLDate::checkDate(void) const
{
    if (year_ == 0) 
    { 
		AQLString msg = "Invalid Data : year_[";
		msg += AQLString((int)year_) + "] is invalid";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if ( month_ == 0 ||  month_ > 12 )
    {
		AQLString msg = "Invalid Data : month[";
		msg += AQLString((int)month_) + "] is invalid";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if( day_ == 0 ||  
        day_ > (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][0][month_ - 1]) 
    {
		AQLString msg = "Invalid Data : day[";
		msg += AQLString((int)day_) + "] is invalid for month=";
		msg += AQLString((int)month_);
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
}

void AQLDate::copy(const AQLDate& d)
{
	day_ = d.day_;
	month_ = d.month_;
	year_ = d.year_;
	julius_.store(d.julius_.load(std::memory_order_relaxed), std::memory_order_relaxed);
	leap_ = d.leap_;
}

/*!
    @brief Adjust for month end if the day_ parameter is larger than month end
	e.g. if data is 2005/06/31, then it is modified into 2005/06/30
*/
void AQLDate::adjustDayForMonthEnd(void) 
{
    if (LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][0][month_-1] < day_) 
	{
        day_ = LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][0][month_-1];
	}
}

long AQLDate::computeJulius(void) const noexcept
{
    int year    = year_;
    int month   = month_;
    int day     = day_;
    int year400 = (year - 1) / 400;
    int mod400  = (year - 1) % 400;
    int year100 = mod400 / 100;
    int mod100  = mod400 % 100;
    int year4   = mod100 / 4;
    int year1   = mod100 % 4;

    long julius = DAYS_400 * year400 + DAYS_100 * year100 + DAYS_4 * year4
                + DEFAULT_DAYS_OF_YEAR * year1;

    julius += LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(year)][1][month - 1] + day;

    return julius;
}

long AQLDate::ensureJulius(void) const noexcept
{
    long julius = julius_.load(std::memory_order_relaxed);
    if (julius == 0)
    {
        // Idempotent: if two threads race here for the same never-yet-cached object, both compute
        // and store the identical correct value - safe with no lock, unlike a plain `mutable long`.
        julius = computeJulius();
        julius_.store(julius, std::memory_order_relaxed);
    }
    return julius;
}

// Public, kept for backward compatibility (some callers, e.g. DateUtilities.cpp, call this purely to
// pre-warm the cache). Unlike ensureJulius(), this always recomputes - matching its historical
// "recompute now" contract - which is harmless since it is idempotent for unchanged year_/month_/day_.
void AQLDate::dateToJulius(void) const noexcept
{
    julius_.store(computeJulius(), std::memory_order_relaxed);
}

void AQLDate::juliusToDate(void)
{
    int year400;
    int year100;
    int year4;
    int year1;
    int mod400;
    int mod100;
    int mod4;
    int mod1;
    int year;
    int month;
    int day;

    long julius = julius_.load(std::memory_order_relaxed);

    year400 = julius / DAYS_400;
    if ((mod400 = julius % DAYS_400) == 0)
    {
        year400 -= 1;
        mod400 = DAYS_400;
    }
    year100 = mod400 / DAYS_100;
    if (year100 == 4 || (mod100 = mod400 % DAYS_100) == 0) 
    {
        year100 -= 1;
        mod100 = mod400 - year100 * DAYS_100;
    }
    year4 = mod100 / DAYS_4;
    if ((mod4 = mod100 % DAYS_4) == 0) 
    {
        year4 -= 1;
        mod4 = DAYS_4;
    }
    year1 = mod4 / DEFAULT_DAYS_OF_YEAR;
    if (year1 == 4 || (mod1 = mod4 % DEFAULT_DAYS_OF_YEAR) == 0) 
    {
        year1 -= 1;
        mod1 = mod4 - year1 * DEFAULT_DAYS_OF_YEAR;
    }
    year = year400 * 400 + year100 * 100 + year4 * 4 + year1 + 1;
    leap_ = IS_LEAP_YEAR(year);
    for (month = 1; 
         month < 12 &&
         (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][1][month] < mod1; 
         month++) 
    {
         ;
    }
    day = mod1 - LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[leap_][1][month - 1];

	year_  = (unsigned short)year;
	month_ = (unsigned short)month;
	day_   = (unsigned short)day;
}

std::ostream& operator<<(std::ostream& os, const AQLDate& date)
{
    os << date.stringWithFormat();
    return os;
}

std::istream& operator>>(std::istream& is, AQLDate& date)
{
	std::string s;
	is >> s;
	date = AQLDate(s.c_str());
	return is;
}
