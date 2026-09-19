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

static const int  YEAR_RANK              = 10000;    // multiplier for date comparison(digit of the year of YYYYMMDD)
static const int  MONTH_RANK             = 100;      // multiplier for date comparison(digit of the month of YYYYMMDD)


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


AQLDate::AQLDate(void) : mLeap(0), mYear(1), mMonth(1), mDay(1), mJulius(1)
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
    mLeap = IS_LEAP_YEAR(mYear);
    checkDate();                                  // validate before touching mJulius, not after
    mJulius.store(0, std::memory_order_relaxed);   // invalidate - see the mJulius field comment in AQLDate.h
}

void AQLDate::setSystemDate(void)
{
    struct tm date;
    time_t now;

    now = time(NULL);
    localtime_s(&date, &now);      // thread-safe: localtime() writes through a shared static buffer
    mYear   = (unsigned short)(date.tm_year + 1900);
    mMonth  = (unsigned short)(date.tm_mon + 1);
    mDay    = (unsigned short)(date.tm_mday);
    mLeap   = IS_LEAP_YEAR(mYear);
    mJulius.store(0, std::memory_order_relaxed);
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
    mYear = (unsigned short)year;
    mLeap = IS_LEAP_YEAR(mYear);
    adjustDayForMonthEnd();
    mJulius.store(0, std::memory_order_relaxed);
}

void AQLDate::setMonth(unsigned int month)
{
    if (month == 0 || month > 12)
    {
		AQLString msg = "Invalid month[";
		msg += AQLString((int)month) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mMonth = (unsigned short)month;
    adjustDayForMonthEnd();
    mJulius.store(0, std::memory_order_relaxed);
}

void AQLDate::setDay(unsigned int day)
{
    if (day == 0 || day > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth - 1]) 
    {
		AQLString msg = "Invalid day[";
		msg += AQLString((int)day) + "] for month=" + AQLString((int)mMonth);
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    long cached = mJulius.load(std::memory_order_relaxed);
    if (cached != 0)
    {
        // if the Julian day has already been computed, keep it current with an O(1) adjustment
        // instead of forcing a full recompute
        mJulius.store(cached + ((long)day-(long)mDay), std::memory_order_relaxed);
    }
    mDay = (unsigned short)day;
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
    mJulius.store(julius, std::memory_order_relaxed);
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
    mJulius.store(julius, std::memory_order_relaxed);
    juliusToDate(); //set all the variables
}

void AQLDate::addMonths(const int months)
{
    int allMonths = 12 * mYear + mMonth + months - 1;

    if (allMonths < 12)
    {
		AQLString msg = "Invalid months[";
		msg += AQLString(months) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    mYear       = (unsigned short)(allMonths / 12);
    mMonth      = (unsigned short)(allMonths % 12) + 1;
    mLeap       = IS_LEAP_YEAR(mYear);
    adjustDayForMonthEnd();
    mJulius.store(0, std::memory_order_relaxed);
}

void AQLDate::addYears(int years)
{
    if ( - years > (int)mYear )
    {
		AQLString msg = "Invalid years[";
		msg += AQLString(years) + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mYear = mYear + (unsigned short)years;
    mLeap = IS_LEAP_YEAR(mYear);
    adjustDayForMonthEnd();
    mJulius.store(0, std::memory_order_relaxed);
}

AQLDayOfWeekEnum AQLDate::dayOfWeek(void) const noexcept
{
    return AQLDayOfWeekEnum(DAY_OF_WEEK(ensureJulius()));
}

long AQLDate::julianDayNumber(void) const noexcept
{
    return ensureJulius();
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
    // date. Checked via isNull() (mYear/mMonth/mDay), not mJulius - mJulius is now a lazy cache that
    // legitimately reads 0 ("not yet computed") for a perfectly valid, non-null date.
    if ( !isNull() )
    {
	    st = stringWithFormat( format );
    }

    return st;
}

/*!
    @brief make a comparison of date. Returns a positive value for a new date

    @param[in] rDate    date to compare

    @retval     > 0     new date
    @retval     0       same date
    @retval     < 0     old date
*/
int AQLDate::cmp(const AQLDate& rDate) const noexcept
{
    // mJulius is a lazy cache (see the field comment in AQLDate.h): opportunistically use it if both
    // sides already have it (one relaxed atomic load each, cheaper than the decimal ranking below and
    // exact for any valid calendar date), but never force a computation neither side already paid
    // for - most callers (schedule-generation loops that mutate a date and immediately compare it)
    // never populate the cache at all, so the decimal path is the common case, not a fallback.
    long thisJulius = mJulius.load(std::memory_order_relaxed);
    long thatJulius = rDate.mJulius.load(std::memory_order_relaxed);
    if ( thisJulius != 0 && thatJulius != 0 )
    {
        if ( thisJulius < thatJulius ) return -1;
        if ( thisJulius > thatJulius ) return 1;
        return 0;
    }

    return (((int)mYear - (int)(rDate.mYear)) * YEAR_RANK +
            ((int)mMonth - (int)(rDate.mMonth)) * MONTH_RANK +
            ((int)mDay - (int)(rDate.mDay)));
}

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
    
    days    = to->mDay - from->mDay; 
	months  = to->mMonth - from->mMonth;
	years   = to->mYear - from->mYear;
    
    if (days < 0) 
    {
		if (to->mMonth != 1)
		{
			// allocation to the days of one month "to"
			days += LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[to->mLeap][0][to->mMonth - 2];
			months -= 1;
		}
		else 
		{
            // in case of Jan.
			// modify "to" into "mYear-1"/"12"/"days+31"
			days += 31; //
			months += 11;
			years -=1;
		}
		// to->mMonth is not Feb. since there is 31 days in Jan.
		// and days < 0 means 31 is not added.
		if (days < 0) 
		{ 
			days += LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[to->mLeap][0][to->mMonth - 3];
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
	return 1 -(int)mDay;
}

int AQLDate::intervalToEndOfMonth(void) const noexcept
{
    return (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth - 1] - (int)mDay;
}

int AQLDate::intervalToStartOfYear(void) const noexcept
{
    return 1 - (int)mDay - (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][1][mMonth - 1];
}

int AQLDate::intervalToEndOfYear(void) const noexcept
{
    return   (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][1][11] + 31
           - (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][1][mMonth - 1]
           - (int)mDay;
}

int AQLDate::intervalToNextDateOfWeek(AQLDayOfWeekEnum next) const noexcept
{
    int ret = (int)next - (int)dayOfWeek();
    return (ret < 0 ? ret + DAYS_OF_WEEK : ret);
}

bool AQLDate::isLeapYear(void) const noexcept
{
	return mLeap == 1;
}

bool AQLDate::isStartOfMonth(void) const noexcept
{
	return mDay == 1;
}

bool AQLDate::isEndOfMonth(void) const noexcept
{
	return mDay == LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth - 1];
}

bool AQLDate::isStartOfYear(void) const noexcept
{
	return mDay == 1 && mMonth == 1;
}

bool AQLDate::isEndOfYear(void) const noexcept
{
	return mDay == 31 && mMonth == 12;
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
    
	mYear  = (unsigned short)dateTime[0];
	mMonth = (unsigned short)dateTime[1];
	mDay   = (unsigned short)dateTime[2];

}

void AQLDate::formatWithLong(AQLString& rSt, const char_t  *format) const
{
    int             dateTime[3];
    unsigned int    i;
    unsigned int    setPos;
    // Sized for the worst case, not just the common one: OUT_FORM[YYYY] is "%04d", and mYear is
    // unsigned short (max 65535, 5 digits) - SPRINTF(..., "%04d", 65535) writes "65535" + a null
    // terminator, 6 chars, which overflowed the previous 5-char buffer by one byte for any year
    // >= 10000. MM/DD only ever hold 1-12/1-31 so 8 chars is generous headroom for all three.
    char_t          strFormat[8];
       
    rSt = format;

    dateTime[YYYY] = mYear;
    dateTime[MM]   = mMonth;
    dateTime[DD]   = mDay;

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
    if (mYear == 0) 
    { 
		AQLString msg = "Invalid Data : mYear[";
		msg += AQLString((int)mYear) + "] is invalid";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if ( mMonth == 0 ||  mMonth > 12 )
    {
		AQLString msg = "Invalid Data : month[";
		msg += AQLString((int)mMonth) + "] is invalid";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if( mDay == 0 ||  
        mDay > (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth - 1]) 
    {
		AQLString msg = "Invalid Data : day[";
		msg += AQLString((int)mDay) + "] is invalid for month=";
		msg += AQLString((int)mMonth);
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
}

void AQLDate::copy(const AQLDate& d)
{
	mDay = d.mDay;
	mMonth = d.mMonth;
	mYear = d.mYear;
	mJulius.store(d.mJulius.load(std::memory_order_relaxed), std::memory_order_relaxed);
	mLeap = d.mLeap;
}

/*!
    @brief Adjust for month end if the mDay parameter is larger than month end
	e.g. if data is 2005/06/31, then it is modified into 2005/06/30
*/
void AQLDate::adjustDayForMonthEnd(void) 
{
    if (LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth-1] < mDay) 
	{
        mDay = LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth-1];
	}
}

long AQLDate::computeJulius(void) const noexcept
{
    int year    = mYear;
    int month   = mMonth;
    int day     = mDay;
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
    long julius = mJulius.load(std::memory_order_relaxed);
    if (julius == 0)
    {
        // Idempotent: if two threads race here for the same never-yet-cached object, both compute
        // and store the identical correct value - safe with no lock, unlike a plain `mutable long`.
        julius = computeJulius();
        mJulius.store(julius, std::memory_order_relaxed);
    }
    return julius;
}

// Public, kept for backward compatibility (some callers, e.g. DateUtilities.cpp, call this purely to
// pre-warm the cache). Unlike ensureJulius(), this always recomputes - matching its historical
// "recompute now" contract - which is harmless since it is idempotent for unchanged mYear/mMonth/mDay.
void AQLDate::dateToJulius(void) const noexcept
{
    mJulius.store(computeJulius(), std::memory_order_relaxed);
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

    long julius = mJulius.load(std::memory_order_relaxed);

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
    mLeap = IS_LEAP_YEAR(year);
    for (month = 1; 
         month < 12 &&
         (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][1][month] < mod1; 
         month++) 
    {
         ;
    }
    day = mod1 - LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][1][month - 1];

	mYear  = (unsigned short)year;
	mMonth = (unsigned short)month;
	mDay   = (unsigned short)day;
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
