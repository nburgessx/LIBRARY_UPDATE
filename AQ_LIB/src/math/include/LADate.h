#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"
#include "LACoreAppError.h"
#include <iostream>

// LADate LADayOfWeekEnum Enumerator
enum LADayOfWeekEnum {SUN = 0, MON, TUE, WED, THU, FRI, SAT}; 

class LADate
{

public:
    
    // default date is 01/01/01
    LADate(void);

    // constructor
	explicit LADate(const char_t* date, const char_t* format="YYYYMMDD");

    // copy constructor
    LADate(const LADate& date);
    
    // destructor
    virtual ~LADate();

    // Accessors
    virtual void        setDate(const char_t* date, const char_t* format="YYYYMMDD");
    virtual void        setSystemDate(void);

    void                setYear(unsigned int year);
    void                setMonth(unsigned int month);
    void                setDay(unsigned int day);

    void                addDays(int days);
	void				addWeeks(int weeks);
    void                addMonths(int months);
    void                addYears(int years);
   
    LADayOfWeekEnum     dayOfWeek(void) const;

    int					dayOfMonth(void) const;
    int					monthOfYear(void) const;
    int					yearOfEra(void) const;

    // Converters
    virtual LAString    stringWithFormat(const char_t* format="YYYYMMDD") const;
    virtual LAString    convertDateToString(const char_t* format="YYYYMMDD") const;
    int					cmp(const LADate& rTime) const;

    // Time Intervals
    int					intervalDays(const LADate& toDate) const;
    int					intervalMonths(const LADate& toDate) const;
    int					intervalYears(const LADate& toDate) const;
    void                intervalYMD(const LADate& toDate, int& years, int& months, int& days) const;
    int					intervalToStartOfMonth(void) const;
	int					intervalToEndOfMonth(void) const;
    int					intervalToStartOfYear(void) const;
    int					intervalToEndOfYear(void) const;
	int					intervalToNextDateOfWeek(LADayOfWeekEnum next) const;

    bool                isLeapYear(void) const;
    bool                isStartOfMonth(void) const;
    bool                isEndOfMonth(void) const;
    bool                isStartOfYear(void) const;
    bool                isEndOfYear(void) const;

    // Operators
    bool                operator==(const LADate& a) const {return cmp(a) == 0;}
    bool                operator!=(const LADate& a) const {return cmp(a) != 0;}
    bool                operator<=(const LADate& a) const {return cmp(a) <= 0;}
    bool                operator>=(const LADate& a) const {return cmp(a) >= 0;}
    bool                operator>(const LADate& a)  const {return cmp(a) > 0;}
    bool                operator<(const LADate& a)  const {return cmp(a) < 0;}

	void                dateToJulius(void) const; // Made public so it can be called from DateUtilities

protected:
    
    void				formatWithString(const char_t *st,  const char_t *format);
    void		        formatWithLong(LAString& str, const char_t *format) const;
    void                checkDate(void) const;
    void		        copy(const LADate& rTime);

    // If the day member mDay is larger than month end then set it by adjusting for month end
    void                adjustDayForMonthEnd(void);

private:
    
	void                juliusToDate(void);

    unsigned short     mLeap;
    unsigned short     mYear;
    unsigned short     mMonth;
    unsigned short     mDay;
    mutable long       mJulius;
};

// String Stream Operators
std::ostream& operator<<(std::ostream& os, const LADate& date);
std::istream& operator>>(std::istream& os, LADate& date);

// Helper Method: isLeapYear
inline unsigned short IS_LEAP_YEAR(int year)
{
    return ( ( year %4 == 0 && year %100 != 0 ) || year %400 == 0 ) ? 1 : 0;
}

