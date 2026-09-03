#ifdef __GNUG__
#pragma implementation
#endif

#include "LADate.h"
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


LADate::LADate(void) : mLeap(0), mYear(1), mMonth(1), mDay(1), mJulius(1) 
{ 
}

LADate::LADate( const char_t* date, const char_t* format)
{
	setDate(date, format);
}

LADate::LADate( const LADate& date) 
{
    copy(date);
}

LADate::~LADate(void) 
{
}


void LADate::setDate(const char_t  *date, const char_t* format)
{
    formatWithString(date, format); 
    mLeap = IS_LEAP_YEAR(mYear);
    mJulius = 0;
    checkDate();
}

void LADate::setSystemDate(void) 
{
    struct tm *date;
    time_t now;
    
    now     = time(NULL);
    date    = localtime(&now);
    mYear   = (unsigned short)(date->tm_year + 1900);
    mMonth  = (unsigned short)(date->tm_mon + 1);
    mDay    = (unsigned short)(date->tm_mday);
    mLeap   = IS_LEAP_YEAR(mYear);
    mJulius = 0; 
}

void LADate::setYear( unsigned int year)
{
    if (year == 0 ) 
    {
		LAString msg = "Invalid year[";
		msg += LAString((int)year) + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mYear = (unsigned short)year;
    mLeap = IS_LEAP_YEAR(mYear);
    adjustDayForMonthEnd();
    mJulius = 0;
}

void LADate::setMonth(unsigned int month)
{
    if (month == 0 || month > 12) 
    {
		LAString msg = "Invalid month[";
		msg += LAString((int)month) + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mMonth = (unsigned short)month;
    adjustDayForMonthEnd();
    mJulius = 0;  // clear julian date
}

void LADate::setDay(unsigned int day)
{
    if (day == 0 || day > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth - 1]) 
    {
		LAString msg = "Invalid day[";
		msg += LAString((int)day) + "] for month=" + LAString((int)mMonth);
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if (mJulius != 0) 
    {
        //if julian date has been set
        mJulius += ((long)day-(long)mDay);
    } 
    mDay = (unsigned short)day;
}

void LADate::addDays(int adays)
{
    if (mJulius == 0)
	{
		dateToJulius();
	}
    mJulius += adays;
    if (mJulius < 0) 
    {
		LAString msg = "Invalid days[";
		msg += LAString(adays) + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    juliusToDate(); //set all the variables
}

void LADate::addWeeks(int weeks)
{
    if (mJulius == 0)
	{
		dateToJulius();
	}
    mJulius += weeks * 7;
    if (mJulius < 0) 
    {
		LAString msg = "Invalid days[";
		msg += LAString(weeks) + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    juliusToDate(); //set all the variables
}

void LADate::addMonths(const int months)
{
    int allMonths = 12 * mYear + mMonth + months - 1;
    
    if (allMonths < 12) 
    {
		LAString msg = "Invalid months[";
		msg += LAString(months) + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    mYear       = (unsigned short)(allMonths / 12);
    mMonth      = (unsigned short)(allMonths % 12) + 1;
    mLeap       = IS_LEAP_YEAR(mYear);
    mJulius     = 0;
    adjustDayForMonthEnd();

    
}

void LADate::addYears(int years)
{
    if ( - years > (int)mYear ) 
    {
		LAString msg = "Invalid years[";
		msg += LAString(years) + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    mYear = mYear + (unsigned short)years;
    mLeap = IS_LEAP_YEAR(mYear);
    adjustDayForMonthEnd();
    mJulius = 0;
}

LADayOfWeekEnum LADate::dayOfWeek(void) const
{
    if (mJulius == 0) 
	{
		dateToJulius();
	}
    return LADayOfWeekEnum(DAY_OF_WEEK(mJulius));
}

int LADate::dayOfMonth(void) const
{
    return mDay;
}

int LADate::monthOfYear(void) const
{
    return mMonth;
}

int LADate::yearOfEra(void) const
{
     return mYear;
}

LAString LADate::stringWithFormat(const char_t* format) const
{
    LAString st;
	try 
	{
        formatWithLong(st, format);
	}
	catch (LACoreError & e)
	{
		LAString msg = "Invalid format = ";
		msg += format;
        throw e += LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
    return st;
}

LAString LADate::convertDateToString( const char_t* format) const
{
    LAString st;
    
    // Do nothing except return the null string if the julius date is less than or equal to one
    // Note: LADate initializes the mJulius to 1 by default.
    if ( mJulius > 1 )
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
int LADate::cmp(const LADate& rDate) const
{ 
    return (((int)mYear - (int)(rDate.mYear)) * YEAR_RANK +
            ((int)mMonth - (int)(rDate.mMonth)) * MONTH_RANK +
            ((int)mDay - (int)(rDate.mDay)));
}

int LADate::intervalDays(const LADate& toDate) const 
{
    if (mJulius == 0) 
	{
		dateToJulius();
	}
    if(toDate.mJulius == 0) 
	{
		toDate.dateToJulius();
	}
    return (int)(toDate.mJulius - mJulius);
}

int LADate::intervalMonths(const LADate& toDate) const 
{
    int days;
    int months;
    int years;
    intervalYMD(toDate, years, months, days);

    return months;     
}

int LADate::intervalYears(const LADate& toDate) const 
{
    int days;
    int months;
    int years;
    intervalYMD(toDate, years, months, days);

    return years;
}

void LADate::intervalYMD(const LADate& toDate, int&  years, int&  months, int&  days) const 
{
    const LADate* to;
    const LADate* from;
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

int LADate::intervalToStartOfMonth(void) const 
{
	return 1 -(int)mDay;
}

int LADate::intervalToEndOfMonth(void) const 
{
    return (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth - 1] - (int)mDay;
}

int LADate::intervalToStartOfYear(void) const 
{
    return 1 - (int)mDay - (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][1][mMonth - 1];
}

int LADate::intervalToEndOfYear(void) const 
{
    return   (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][1][11] + 31  
           - (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][1][mMonth - 1] 
           - (int)mDay;
}

int LADate::intervalToNextDateOfWeek(LADayOfWeekEnum next) const 
{
    int ret = (int)next - (int)dayOfWeek();
    return (ret < 0 ? ret + DAYS_OF_WEEK : ret);
}

bool LADate::isLeapYear(void) const 
{
	return mLeap == 1;
}

bool LADate::isStartOfMonth(void) const 
{
	return mDay == 1;
}

bool LADate::isEndOfMonth(void) const 
{
	return mDay == LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth - 1]; 
}

bool LADate::isStartOfYear(void) const 
{
	return mDay == 1 && mMonth == 1;
}

bool LADate::isEndOfYear(void) const 
{
	return mDay == 31 && mMonth == 12;
}

void LADate::formatWithString(const char_t  *st, const char_t  *format)
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
		LAString msg = "Invalid Data ";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

    if ((length = STRLEN(st)) != STRLEN(format)) 
    {
		LAString msg = "Invalid format ";
		msg += st;
		msg += " : ";
		msg += format;
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
				LAString msg = "Invalid format ";
				msg += st;
				msg += " : ";
				msg += format;
				msg += " at ";
				msg += LAString((int)i);
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
				LAString msg = "Invalid format ";
				msg += st;
				msg += " : ";
				msg += format;
				msg += " at ";
				msg += LAString((int)i);
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
            }
			++i;
			continue;
        }
        if (setFlag[setPos] != 0) 
        {  // error
			LAString msg = "Invalid format ";
			msg += st;
			msg += " : ";
			msg += format;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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

void LADate::formatWithLong(LAString& rSt, const char_t  *format) const
{
    int             dateTime[3];
    unsigned int    i;
    unsigned int    setPos;
    char_t          strFormat[5];
       
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

void LADate::checkDate(void) const
{
    if (mYear == 0) 
    { 
		LAString msg = "Invalid Data : mYear[";
		msg += LAString((int)mYear) + "] is invalid";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if ( mMonth == 0 ||  mMonth > 12 )
    {
		LAString msg = "Invalid Data : month[";
		msg += LAString((int)mMonth) + "] is invalid";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if( mDay == 0 ||  
        mDay > (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth - 1]) 
    {
		LAString msg = "Invalid Data : day[";
		msg += LAString((int)mDay) + "] is invalid for month=";
		msg += LAString((int)mMonth);
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
}

void LADate::copy(const LADate& d) 
{
	mDay = d.mDay;
	mMonth = d.mMonth;
	mYear = d.mYear;
	mJulius = d.mJulius;
	mLeap = d.mLeap;
}

/*!
    @brief Adjust for month end if the mDay parameter is larger than month end
	e.g. if data is 2005/06/31, then it is modified into 2005/06/30
*/
void LADate::adjustDayForMonthEnd(void) 
{
    if (LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth-1] < mDay) 
	{
        mDay = LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[mLeap][0][mMonth-1];
	}
}

void LADate::dateToJulius(void) const
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

    mJulius = DAYS_400 * year400 + DAYS_100 * year100 + DAYS_4 * year4
            + DEFAULT_DAYS_OF_YEAR * year1;

    mJulius +=LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(year)][1][month - 1] + day;

}

void LADate::juliusToDate(void)
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
  
    year400 = mJulius / DAYS_400;
    if ((mod400 = mJulius % DAYS_400) == 0) 
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

std::ostream& operator<<(std::ostream& os, const LADate& date)
{
    os << date.stringWithFormat();
    return os;
}

std::istream& operator>>(std::istream& is, LADate& date)
{
	std::string s;
	is >> s;
	date = LADate(s.c_str());
	return is;
}
