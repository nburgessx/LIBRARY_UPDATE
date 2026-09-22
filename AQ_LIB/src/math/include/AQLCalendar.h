#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <map>
#include <memory>
#include <set>
#include "AQLCoreSystemError.h"
#include "AQLCoreAppError.h"
#include "AQLDate.h"
#include "AQLCoreTemplateType.h"

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#include <pthread.h>
#endif

class AQLMathCalendarData;
class AQLString;
/*!
    @brief Calendar class to represent holiday information.
*/
class AQLCalendar
{
public:

	// default constructor
	AQLCalendar(void);

	// copy constructor - a real (and cheap - a calendar's holiday deque is tiny) deep copy, not a
	// shared one. See the pCalData_ field comment for why this class dropped copy-on-write entirely
	// rather than just fixing its refcount race the way AQLNumericMatrix did.
	AQLCalendar(const AQLCalendar& cal);

	// move constructor - steals the other calendar's data outright, no allocation at all
	AQLCalendar(AQLCalendar&& cal) noexcept;

	// destructor
	~AQLCalendar(void);

	// assignment operator
	const AQLCalendar& operator=(const AQLCalendar& cal);

	// move assignment
	AQLCalendar& operator=(AQLCalendar&& cal) noexcept;

	// addition assignment operator
	const AQLCalendar& operator+=(const AQLCalendar& cal);

	// get the number of holiday (including both start and end) for a specified period of time
	int	countHoliday(const AQLDate& startDate, const AQLDate& endDate) const;

    // check whether holiday or not
	bool isHoliday(const AQLDate& date) const;

	// the specified day of the week is set to the holiday
	void setWeekly(const AQLDayOfWeekEnum weekly);

    // the specified day is set to the holiday 
	void setDays(const AQLString& days);
    
	// the specified multi days are set to the holiday
	void setDays(const AQLStringVector& daysVector);

    // set the date of X month Y of the week and Z day of the week on holidays(indefinite holiday)
	void setFlowDate(const int month, const int week, const AQLDayOfWeekEnum weekly);

    // the specified date is set to the holiday
	void setDate(const AQLDate& date);
    
	// the specified multi dates are set to the holiday
	void setDate(const DateVector& dateVector);

private:

	// Sole owner, always - no more copy-on-write. This used to share AQLMathCalendarData behind a
	// plain (non-atomic) int* refcount, the same "looks thread-safe, isn't" shape AQLString and
	// AQLNumericMatrix both had. Rather than just fix the race the way AQLNumericMatrix's was fixed
	// (shared_ptr - appropriate there because matrix arithmetic returns by value pervasively, a
	// genuinely copy-heavy usage shape), this class drops sharing entirely: an AQLCalendar is built
	// once via the setWeekly/setDays/setFlowDate/setDate calls, then read many times, often via a
	// `const AQLCalendar&` handed out of AQLMathCalendarCollection's map to many callers at once -
	// copying is rare, so there is nothing for O(1) sharing to usefully win here, only a real race
	// to lose. unique_ptr makes that "always exclusively owned" invariant structural rather than a
	// contract callers have to uphold by not aliasing a shared_ptr copy.
	//
	// This does NOT fix the other, independent race this class had: AQLMathCalendarData's lazy
	// holiday cache (holiday_/start_/end_), mutated from the const isHoliday()/countHoliday()
	// methods with no synchronization. Dropping COW only removes the cross-*object* aliasing risk;
	// concurrently reading the *same* AQLCalendar instance from multiple threads (exactly the
	// getCalendarData() scenario above) still needs that cache to be safe on its own, which is why
	// AQLMathCalendarData now has its own mutex (see its field comment in AQLCalendar.cpp).
	std::unique_ptr<AQLMathCalendarData>	pCalData_;
};



///////////////////////////////////////////////////////////////////////////
// 
// collection class AQLCalendar
//
///////////////////////////////////////////////////////////////////////////
/*! 
    @brief Collection class of holiday information

	Class to represent holilday information of cities.

*/
class AQLMathCalendarCollection
{
public:
    // default constructor
	AQLMathCalendarCollection(){};
    // destructor
	~AQLMathCalendarCollection(){};

    // get AQLCalendar object from the specified city names
	const AQLCalendar& getCalendarData(const AQLString& cityName);

    // get a list of city names
	void getMemberNames(std::vector<AQLString>& arr);

    // set city name and holiday information
    void setCalendarData(const AQLString &cityName, const AQLCalendar &calendar);

    // free calendar information 
    /*!
       clear all map data (holiday city, calendar information)   
    */
    void				destroy() { calendarList_.clear(); }
    
	// get inquired city names
    void  getCityNames(std::vector<AQLString>& names) const;

private:

    // not copyable - modern spelling of the old "declare private, never define" idiom, which is
    // also why the analyzer (VCR001) couldn't find a definition for operator=: there deliberately
    // wasn't one to find.
	AQLMathCalendarCollection(AQLMathCalendarCollection& calcol) = delete;
	AQLMathCalendarCollection& operator=(AQLMathCalendarCollection& calcol) = delete;
	
	// get day of the week(enum value) from the string format, e.g. "SUN"
	AQLDayOfWeekEnum	getWeeklyValue(const AQLString& weekly);

    // calendar information with city code name key
	std::map<AQLString, AQLCalendar>	calendarList_;
	
	// inquired calendar names
	std::set<AQLString>	cityNames_;
};
