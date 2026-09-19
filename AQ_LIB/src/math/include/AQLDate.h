#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLCoreAppError.h"
#include <atomic>
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

    // assignment operator (needed explicitly now that mJulius is a std::atomic, whose own
    // assignment operator is deleted - see the field comment below)
    AQLDate&            operator=(const AQLDate& date);

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
    // (AQLPriceDataType, AQLDataMatrix, AQLDataHolder). Checked directly off mYear/mMonth/mDay (the
    // sentinel is exactly 0001-01-01) rather than mJulius, which - now that it is a lazily-computed
    // cache (see the field comment below) - can legitimately read 0 ("not yet computed") for a
    // perfectly valid, non-null date. O(1), no Julian-day computation triggered either way.
    bool                isNull(void) const noexcept {return mYear <= 1 && mMonth <= 1 && mDay <= 1;}

    // The Julian day number for this date (see the mJulius field comment). Computes and caches it on
    // first call if not already cached; O(1) amortised, thread-safe for concurrent readers.
    long                julianDayNumber(void) const noexcept;

    // Parses `date` against `format` the same way the constructor/setDate() do, but returns false
    // instead of throwing on a malformed or out-of-range input. Intended for validation-heavy call
    // sites (e.g. validation::ScheduleValidation's "is this string a date?" checks) that currently
    // pay for a C++ exception per rejected candidate purely to test format validity.
    static bool         isValidDate(const char_t* date, const char_t* format="YYYYMMDD") noexcept;

    // Today's date in the local timezone - equivalent to default-constructing and calling
    // setSystemDate(), as a single expression.
    static AQLDate      today(void);

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

    // Computes the Julian day number from the current mYear/mMonth/mDay - the pure arithmetic core
    // of dateToJulius(), with no caching side effect (see computeJulius()/ensureJulius() in the cpp).
    long                computeJulius(void) const noexcept;

    // Returns the cached Julian day number, computing and caching it first if it is not yet valid
    // (mJulius == 0). This is the lazy, thread-safe counterpart of the eager scheme this class used
    // briefly: see the mJulius field comment.
    long                ensureJulius(void) const noexcept;

	void                juliusToDate(void);

    unsigned short     mLeap;
    unsigned short     mYear;
    unsigned short     mMonth;
    unsigned short     mDay;

    // The Julian day count is a lazily-computed, thread-safe cache: 0 means "not yet computed for
    // the current mYear/mMonth/mDay" (every valid calendar date - year >= 1 - produces a Julian day
    // of at least 1, so 0 is an unambiguous sentinel). Every mutator invalidates it with a cheap
    // store of 0 rather than paying for the division/modulo chain in computeJulius() up front; the
    // cost is only paid by whichever const method first actually needs the value (cmp()'s fast path,
    // dayOfWeek(), intervalDays(), julianDayNumber(), the public dateToJulius()) via ensureJulius().
    //
    // This class went through an eager variant of this field for one session (compute on every
    // mutation, never 0) on the reasoning that it keeps const methods from ever writing to mutable
    // state, which matters for objects read concurrently by multiple threads without synchronization.
    // That safety property is real and worth keeping, but eager recomputation on every mutation was a
    // measured net loss: the hottest AQLDate code paths in this library (schedule generation - e.g.
    // AQLDateScheduleHelpers.cpp's IMM-date rolling, coupon/cashflow schedule construction) mutate a
    // date many times (addMonths/addYears/setYear/setMonth in a loop) and only ever compare it, never
    // read its Julian day directly - so eager mode paid for a computation on every single mutation
    // that the old, purely decimal cmp() never even needed. `std::atomic<long>` gets both properties
    // at once: mutators stay a cheap relaxed store (no computation), and the lazy compute-and-cache
    // in ensureJulius() is idempotent, so two threads racing to fill the cache for the first time both
    // compute and store the identical correct value - safe, with no lock and no torn read/write,
    // unlike the plain `mutable long` this field held before the eager change (which had the same
    // lazy-write-from-a-const-method shape, just without the atomic - a real, if narrow, data race
    // under the C++ memory model for a date genuinely read from multiple threads at once).
    //
    // Audited (2026-09-19): no AQLDate anywhere in etrading/calibration/models/validation/AQ_XLL/
    // AQ_API mutates a class-member (as opposed to a local/stack) AQLDate after that owning object
    // has been published into the AQObj handle pool or otherwise shared - every setDate()/addMonths()/
    // etc. call on a member field happens during that object's own single-threaded construction. So
    // lazy invalidation cannot reintroduce a mutate-while-reading race in practice; the atomic is
    // there so that stays true even for a date object a caller does share and read concurrently
    // without ever mutating it again (the actually-safe pattern), not because current code needs it.
    mutable std::atomic<long> mJulius;
};

// String Stream Operators
std::ostream& operator<<(std::ostream& os, const AQLDate& date);
std::istream& operator>>(std::istream& os, AQLDate& date);

// Helper Method: isLeapYear
inline unsigned short IS_LEAP_YEAR(int year)
{
    return ( ( year %4 == 0 && year %100 != 0 ) || year %400 == 0 ) ? 1 : 0;
}

