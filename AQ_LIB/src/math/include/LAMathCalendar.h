#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <map>
#include <set>
#include "LACoreSystemError.h"
#include "LACoreAppError.h"
#include "LADate.h"
#include "LACoreTemplateType.h"

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#include <pthread.h>
#endif

class LAMathCalendarData;
class LAString;
/*!
    @brief Calendar class to represent holiday information.
*/
class LAMathCalendar
{
public:
//  LIFECYCLE
    // default constructor
	LAMathCalendar(void);
    // copy constructor
	LAMathCalendar(const LAMathCalendar& cal);
    // destructor
	~LAMathCalendar(void);

//  OPERATOR
	// assignment operator
	const LAMathCalendar& operator=(const LAMathCalendar& cal);
	// addition assignment operator
	const LAMathCalendar& operator+=(const LAMathCalendar& cal);

//  QUERY
	// get the number of holiday (including both start and end) for a specified period of time
	int	countHoliday(const LADate& startDate, const LADate& endDate) const;

    // check whether holiday or not
	bool isHoliday(const LADate& date) const;

//  OPERATION
	// the specified day of the week is set to the holiday
	void setWeekly(const LADayOfWeekEnum weekly);

    // the specified day is set to the holiday 
	void setDays(const LAString& days);
    // the specified multi days are set to the holiday
	void setDays(const LAStringVector& daysVector);

    // set the date of X month Y of the week and Z day of the week on holidays(indefinite holiday)
	void setFlowDate(const int month, const int week, const LADayOfWeekEnum weekly);

    // the specified date is set to the holiday
	void setDate(const LADate& date);
    // the specified multi dates are set to the holiday
	void setDate(const DateVector& dateVector);

private:

	LAMathCalendarData*		mpCalData;  //! holiday data
	mutable int*		mpRefCount; //! reference counter to the holiday data(LAMathCalendarData object)
	// free the memory acquired
	void clear(void);
	// shallow copy of the project
	void copy(const LAMathCalendar& cal);
	//	reset the reference counter of LAMathCalendarData object that the object holds
	void makeUnShared(void);
#ifdef __HAS_MIC__
	static common_lib::StaticMutex  mMutex;
#endif
};



///////////////////////////////////////////////////////////////////////////
// 
// collection class LAMathCalendar
//
///////////////////////////////////////////////////////////////////////////
/*! 
    @brief Collection class of holiday information

	Class to represent holilday information of cities.

*/
class LAMathCalendarCollection
{
public:
//  LIFECYCLE
    // default constructor
	LAMathCalendarCollection(){};
    // destructor
	~LAMathCalendarCollection(){};

//  QUERY
    // get LAMathCalendar object from the specified city names
	const LAMathCalendar& getCalendarData(const LAString& cityName);

    // get a list of city names
	void getMemberNames(std::vector<LAString>& arr);

// ysuzuki
    // set city name and holiday information
    void setCalendarData(const LAString &cityName, const LAMathCalendar &calendar);

    // free calendar information 
    /*!
       clear all map data (holiday city, calendar information)   
    */
    void				destroy() { mCalendarList.clear(); }
    // get inquired city names
    void  getInquiredCityNames(std::vector<LAString>& names) const;

private:
    // not available
	LAMathCalendarCollection(LAMathCalendarCollection& calcol);
    // not available
	LAMathCalendarCollection& operator=(LAMathCalendarCollection& calcol);
	// get day of the week(enum value) from the string format, e.g. "SUN"
	LADayOfWeekEnum	getWeeklyValue(const LAString& weekly);

    // calendar information with city code name key
	std::map<LAString, LAMathCalendar>	mCalendarList;
	// inquired calendar names
	std::set<LAString>	mInqCityNames;
};
