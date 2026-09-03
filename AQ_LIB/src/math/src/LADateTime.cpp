/*! @file
    @brief Implementation of basic class which provides date or time functions.
	
	This class is one that holds time information in addition to date information of LADate
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LADateTime.h"
#include <cstdlib>
#include <ctime>
#include <string.h>

static const int  SECONDS_MINUTE         = 60;       // seconds of a minute
static const int  SECONDS_HOUR           = 3600;     // seconds of an hour
static const long SECONDS_DAY            = 86400;    // seconds of a day
static const int  MINUTES_HOUR			 = 60;       // minutes of an hour
static const int  MINUTES_DAY			 = 1440;	 // minutes of a day
static const int  HOURS_DAY				 = 24;       // hours of a day

static const int HOUR_RANK				  = 10000;   // multiplier for date time comparison
static const int MINUTE_RANK              = 100;     // multiplier for date time comparison

/* Format*/
// constants for output format
enum _TFORMAT {HH=0, MI, SS, DEFAULT};         
static const char_t* FORMAT[4] = 
                {"HH", "MI", "SS", "DEFAULT"}; // string of constants for output format
static const unsigned int   FLENGTH[4] = {2, 2, 2, 1};  // number of strings of constants for output format
static const char_t* OUT_FORM[4] =
                {"%02d", "%02d", "%02d", "%01d"}; // output format

//////////////////////////// PUBLIC  ///////////////////////////////////////
//=========================  LIFECYCLE  ====================================
/*!
    @brief default constructor

    set as the default (= 1 Julian date) 0:00:00 of January 1 of the year 1 AD
*/
LADateTime::LADateTime(void) 
: LADate(), mHour(0), mMinute(0), mSecond(0) 
{ 
}

/*!
    @brief constructor
*/
LADateTime::LADateTime(
	const char_t*   date, 
	const char_t*   format)
{
	setDate(date, format);
}

/*!
    @brief copy constructor
*/
LADateTime::LADateTime(
	const LADateTime& date) 
{
    copy(date);
}

/*!
    @brief destructor
*/
LADateTime::~LADateTime(void) 
{
}

//=========================  OPERATION  ====================================
/*!
    @brief set the date according to format

    @param[in] date   date information
    @param[in] format format string (year YYYY, month MM, date DD, minute MI, and second SS)
*/
void
LADateTime::setDate(
	const char_t  *date, 
	const char_t*   format)
{
    formatWithString(date, format); 
}

/*!
    @brief set the (date) system time

    with the systet time, set the object to the data of date, hour, minute and second 
*/
void 
LADateTime::setSystemDate(void) 
{
    struct tm *date;
    time_t now;
    now = time(NULL);

    date   = localtime(&now);

	setYear((unsigned int)(date->tm_year + 1900));
    setMonth((unsigned int)(date->tm_mon + 1));
    setDay((unsigned int)(date->tm_mday));
	setHour((unsigned int)(date->tm_hour));
	setMinute((unsigned int)(date->tm_min));
	setSecond((unsigned int)(date->tm_sec));
}

/*!
    @brief set hour
   
    if time is set by 24, then forward the date and set by 0. If number is larger than 24, then an exception occurs

    @param[in] hour hour to be set
*/
void 
LADateTime::setHour(
	unsigned int hour)
{
    if (hour > 24 ) 
    {
		LAString msg = "Invalid hour[";
		msg += LAString((int)hour) + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
	// foward the date and chenge 24 PM into 0 AM
	if (hour == 24) 
	{
		addDays(1);
		hour = 0;
	}
	// set
    mHour = (unsigned short)hour;
}

/*!
    @brief set minute

	if minute is set by 60, then forward the hour and set by 0. If number is larger than 60, then an exception occurs.

    @param[in] min minute to be set
*/
void
LADateTime::setMinute(
	unsigned int min)
{

    if (min > 60) 
    {
		LAString msg = "Invalid minutes[";
		msg += LAString((int)min) + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
	// foward the hour and chenge 60 min. into 0 min.
	if (min == 60)
	{
		addHours(1);
		min = 0;
	}
    mMinute = (unsigned short)min;
}

/*!
    @brief set second
    
	if second is set by 60, then forward the minute and set by 0. If number is larger than 60, then an exception occurs.

    @param[in] sec second to be set
*/
void 
LADateTime::setSecond(
	unsigned int sec)
{

    if (sec > 60) 
    {
		LAString msg = "Invalid Second[";
		msg += LAString((int)sec) + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
	// foward the sec and chenge 60 into 0.
	if (sec == 60)
	{
		addMinutes(1);
		sec = 0;
	}
	mSecond = (unsigned short)sec;
}

/*!
    @brief add hours
    
    @param[in] hours hours to be added
*/
void 
LADateTime::addHours(const int hours)
{
	int total = mHour + hours;
	int days, hour;
	// adjust the number of days
	if (total < 0)
	{
		int total_p = -total;
		days = total_p / HOURS_DAY + 1;
		hour = days * HOURS_DAY - total_p;
	}
	else
	{
		days = total / HOURS_DAY;
		hour = total - days * HOURS_DAY;
	}
	// need to adjust days ?
	if (days != 0)
	{
		addDays(days);
	}
	mHour = (unsigned short)hour;
}

/*!
    @brief add minutes
    
    @param[in] mins minutes to be added
*/
void
LADateTime::addMinutes(const int mins)
{
	int total = mMinute + mins;
	int hours, min;
	// adjust the number of hours
	if (total < 0)
	{
		int total_p = -total;
		hours = total_p / MINUTES_HOUR + 1;
		min = hours * MINUTES_HOUR - total_p;
	}
	else
	{
		hours = total / MINUTES_HOUR;
		min = total - hours * MINUTES_HOUR;
	}
	if (hours != 0)
	{  // adjust the number of hours
		addHours(hours);
	}
	mMinute = (unsigned short)min;
}

/*!
    @brief add seconds
    
    @param[in] secs seconds to be added
*/
void 
LADateTime::addSeconds(const int secs)
{
	int total = mSecond + secs;
	int mins, sec;
	// adjust the number of minutes
	if (total < 0)
	{
		int total_p = -total;
		mins = total_p / SECONDS_MINUTE + 1;
		sec = mins * SECONDS_MINUTE - total_p;
	}
	else
	{
		mins = total / SECONDS_MINUTE;
		sec = total - mins * SECONDS_MINUTE;
	}
	if (mins != 0)
	{  // adjust the number of minutes
		addMinutes(mins);
	}
	mSecond = (unsigned short)sec;
}

//=====================  QUERY  ============================================
/*!
    @brief return hour
    
    @return hour(023)
*/
int 
LADateTime::getHour(void) const
{
    return mHour;
}

/*!
    @brief return minute
    
    @return minute(059)
*/
int 
LADateTime::getMinute(void) const
{
    return mMinute;
}

/*!
    @brief return second
    
    @return second(059)
*/
int 
LADateTime::getSecond(void) const
{
     return mSecond;
}

/*!
    @brief the date and time according to the format

    @param[in] format date and time format string(year YYYY, month MM, Date DD, when is HH, minutes MI, expressed in seconds SS)
    @return date and time string according to the specified format
*/
LAString 
LADateTime::stringWithFormat(
	const char_t*   format) const 
{
    //TODO : OK??
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

/*!
    @brief make a comparison of time. Returns a positive value when their own is the new time.

    @param[in] rDate date time to compare

    @retval > 0 new date time
    @retval 0  same date time
    @retval < 0 old date time
*/
int
LADateTime::cmp(
	const LADateTime& rDate) const
{
	int ret = LADate::cmp(rDate);
	if (ret == 0)
	{
		ret = (((int)mHour - (int)(rDate.mHour)) * HOUR_RANK +
            ((int)mMinute - (int)(rDate.mMinute)) * MINUTE_RANK +
            ((int)mSecond - (int)(rDate.mSecond)));
	}
	return ret;
}

//////////////////////////// PROTECTED /////////////////////////////////////
/*!
    @brief set the date according to a format

    @param[in] st date string to be set
    @param[in] format date time format
*/
void
LADateTime::formatWithString(
	const char_t  *st,  
	const char_t  *format)
{
	// Format check
	if (st == NULL || format == NULL)
	{
		LAString msg = "Invalid Data ";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
    unsigned int     length;
    if ((length = STRLEN(st)) != STRLEN(format)) 
    {
		LAString msg = "Invalid format ";
		msg += st;
		msg += " : ";
		msg += format;
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
	// arguments check
    int  dateTime[3]    = {0, 0, 0};
    int   setFlag[3]     = {0, 0, 0};
	// memory to hand base class
	LAString format_s(format);

    unsigned int     i, j;
    unsigned int     setPos;
    char_t              wkDateTime[5];

    for (i = 0; i < length;) 
    {
        if (format[i] == 'H')
		{
			setPos = HH;
		}
        else if (format[i] == 'M') 
		{
			setPos = MI;
		}
        else if (format[i] == 'S') 
		{
			setPos = SS;
		}
		else
		{
			++i;
			continue; //chek on the base class
		}
		// match Format ?
		if (memcmp(FORMAT[setPos], &(format[i]), 
						            FLENGTH[setPos]) != 0)
		{
			++i;
			continue;
		}
		// 
        if (setFlag[setPos] != 0) 
        { // error in case of multi definition
			LAString msg = "Invalid format ";
			msg += st;
			msg += " : ";
			msg += format;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
		// copy
		// convert Format
		for (j = 0; j < FLENGTH[setPos]; ++j)
		{
			format_s.charUpdate(i+j, st[i+j]);
		}
		// get data
        strncpy(wkDateTime, &(st[i]), FLENGTH[setPos]);
        wkDateTime[FLENGTH[setPos]] = '\0';
        char * pFirstNonNumber;
        dateTime[setPos] = strtol( wkDateTime, &pFirstNonNumber, 10 ); // base 10 numbers
        setFlag[setPos]    = 1;  //flag up
		i += FLENGTH[setPos];
    }
	// set date time
	LADate::setDate(st, format_s.getCString());
	// set time
	setHour((unsigned int)dateTime[0]);
    setMinute((unsigned int)dateTime[1]);
    setSecond((unsigned int)dateTime[2]);

}

/*!
    @brief get the date according to a format

    @param[out] rSt date time string to get
    @param[in] format date time format
*/
void 
LADateTime::formatWithLong(
	LAString& rSt,  
	const char_t  *format) const
{

    int             dateTime[3];
    unsigned int    i;
    unsigned int    setPos;
    char_t             strFormat[5];
       
    rSt = format;

    dateTime[HH]   = mHour;
    dateTime[MI]   = mMinute;
    dateTime[SS]   = mSecond;

    for (i = 0; i < rSt.size();) 
	{
        if (format[i] == 'H')      
		{
			setPos = HH;
		}
        else if (format[i] == 'M') 
		{
			setPos = MI;
		}
        else if (format[i] == 'S') 
		{
			setPos = SS;
		}
        else 
        {
            i++;
            continue;
		}
        if (memcmp(FORMAT[setPos], &(format[i]), FLENGTH[setPos]) != 0) 
		{
			++i;
			continue;
        }
        SPRINTF(strFormat, OUT_FORM[setPos], dateTime[setPos]);
        rSt.replace(i, strFormat);
        i += FLENGTH[setPos];
    }
	// entrust the process to the parent class
	LADate::formatWithLong(rSt,rSt.getCString());
}

/*!
    @brief copy time object

    @param[in] d original time object
*/void 
LADateTime::copy(
	const LADateTime& d) 
{
	LADate::copy(d);
	mHour = d.mHour;
	mMinute = d.mMinute;
	mSecond = d.mSecond;
}

