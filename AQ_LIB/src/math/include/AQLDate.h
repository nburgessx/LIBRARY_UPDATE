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
   
    AQLDayOfWeekEnum     dayOfWeek(void) const noexcept;

    int					dayOfMonth(void) const noexcept {return mDay;}
    int					monthOfYear(void) const noexcept {return mMonth;}
    int					yearOfEra(void) const noexcept {return mYear;}

    // Returns true if this AQLDate is still at its default-constructed sentinel value (never set to
    // a real calendar date) - matches the isNull() convention already used elsewhere in this codebase
    // (AQLPriceDataType, AQLDataMatrix, AQLDataHolder). O(1): mJulius is always kept up to date, so
    // this never triggers a Julian-day computation.
    bool                isNull(void) const noexcept {return mJulius <= 1;}

    // Converters
    virtual AQLString    stringWithFormat(const char_t* format="YYYYMMDD") const;
    virtual AQLString    convertDateToString(const char_t* format="YYYYMMDD") const;
    int					cmp(const AQLDate& rTime) const noexcept;

    // Time Intervals
    int					intervalDays(const AQLDate& toDate) const noexcept;
    int					intervalMonths(const AQLDate& toDate) const noexcept;
    int					intervalYears(const AQLDate& toDate) const noexcept;
    void                intervalYMD(const AQLDate& toDate, int& years, int& months, int& days) const noexcept;
    int					intervalToStartOfMonth(void) const noexcept;
	int					intervalToEndOfMonth(void) const noexcept;
    int					intervalToStartOfYear(void) const noexcept;
    int					intervalToEndOfYear(void) const noexcept;
	int					intervalToNextDateOfWeek(AQLDayOfWeekEnum next) const noexcept;

    bool                isLeapYear(void) const noexcept;
    bool                isStartOfMonth(void) const noexcept;
    bool                isEndOfMonth(void) const noexcept;
    bool                isStartOfYear(void) const noexcept;
    bool                isEndOfYear(void) const noexcept;

    // Operators
    bool                operator==(const AQLDate& a) const noexcept {return cmp(a) == 0;}
    bool                operator!=(const AQLDate& a) const noexcept {return cmp(a) != 0;}
    bool                operator<=(const AQLDate& a) const noexcept {return cmp(a) <= 0;}
    bool                operator>=(const AQLDate& a) const noexcept {return cmp(a) >= 0;}
    bool                operator>(const AQLDate& a)  const noexcept {return cmp(a) > 0;}
    bool                operator<(const AQLDate& a)  const noexcept {return cmp(a) < 0;}

	void                dateToJulius(void) const noexcept; // Made public so it can be called from DateUtilities

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

    // The Julian day count is now kept eagerly up to date by every constructor and mutator (see
    // AQLDate.cpp), rather than computed lazily on first use - this keeps date comparison (cmp())
    // on the fast integer-compare path unconditionally, and means no internal const method ever
    // needs to write to this field, which is what makes concurrent const access (comparisons,
    // interval calculations, dayOfWeek()) across threads safe. It stays `mutable` only because the
    // public dateToJulius() method - kept for backward compatibility - is const and still assigns to
    // it; calling it on an already-current object is idempotent (recomputes the same value).
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

