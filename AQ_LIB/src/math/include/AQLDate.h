#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLCoreAppError.h"
#include <iostream>

// AQLDate AQLDayOfWeekEnum Enumerator
enum AQLDayOfWeekEnum {SUN = 0, MON, TUE, WED, THU, FRI, SAT}; 

class AQLDate
{

public:
    
    // default date is 01/01/01
    AQLDate(void);

    // constructor
	explicit AQLDate(const char_t* date, const char_t* format="YYYYMMDD");

    // copy constructor
    AQLDate(const AQLDate& date);
    
    // destructor
    virtual ~AQLDate();

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
   
    AQLDayOfWeekEnum     dayOfWeek(void) const;

    int					dayOfMonth(void) const;
    int					monthOfYear(void) const;
    int					yearOfEra(void) const;

    // Converters
    virtual AQLString    stringWithFormat(const char_t* format="YYYYMMDD") const;
    virtual AQLString    convertDateToString(const char_t* format="YYYYMMDD") const;
    int					cmp(const AQLDate& rTime) const;

    // Time Intervals
    int					intervalDays(const AQLDate& toDate) const;
    int					intervalMonths(const AQLDate& toDate) const;
    int					intervalYears(const AQLDate& toDate) const;
    void                intervalYMD(const AQLDate& toDate, int& years, int& months, int& days) const;
    int					intervalToStartOfMonth(void) const;
	int					intervalToEndOfMonth(void) const;
    int					intervalToStartOfYear(void) const;
    int					intervalToEndOfYear(void) const;
	int					intervalToNextDateOfWeek(AQLDayOfWeekEnum next) const;

    bool                isLeapYear(void) const;
    bool                isStartOfMonth(void) const;
    bool                isEndOfMonth(void) const;
    bool                isStartOfYear(void) const;
    bool                isEndOfYear(void) const;

    // Operators
    bool                operator==(const AQLDate& a) const {return cmp(a) == 0;}
    bool                operator!=(const AQLDate& a) const {return cmp(a) != 0;}
    bool                operator<=(const AQLDate& a) const {return cmp(a) <= 0;}
    bool                operator>=(const AQLDate& a) const {return cmp(a) >= 0;}
    bool                operator>(const AQLDate& a)  const {return cmp(a) > 0;}
    bool                operator<(const AQLDate& a)  const {return cmp(a) < 0;}

	void                dateToJulius(void) const; // Made public so it can be called from DateUtilities

protected:
    
    void				formatWithString(const char_t *st,  const char_t *format);
    void		        formatWithLong(AQLString& str, const char_t *format) const;
    void                checkDate(void) const;
    void		        copy(const AQLDate& rTime);

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
std::ostream& operator<<(std::ostream& os, const AQLDate& date);
std::istream& operator>>(std::istream& os, AQLDate& date);

// Helper Method: isLeapYear
inline unsigned short IS_LEAP_YEAR(int year)
{
    return ( ( year %4 == 0 && year %100 != 0 ) || year %400 == 0 ) ? 1 : 0;
}

