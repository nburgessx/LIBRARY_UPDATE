#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <map>
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
class AQLMathCalendar
{
public:
//  LIFECYCLE
    // default constructor
	AQLMathCalendar(void);
    // copy constructor
	AQLMathCalendar(const AQLMathCalendar& cal);
    // destructor
	~AQLMathCalendar(void);

//  OPERATOR
	// assignment operator
	const AQLMathCalendar& operator=(const AQLMathCalendar& cal);
	// addition assignment operator
	const AQLMathCalendar& operator+=(const AQLMathCalendar& cal);

//  QUERY
	// get the number of holiday (including both start and end) for a specified period of time
	int	countHoliday(const AQLDate& startDate, const AQLDate& endDate) const;

    // check whether holiday or not
	bool isHoliday(const AQLDate& date) const;

//  OPERATION
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

	AQLMathCalendarData*		mpCalData;  //! holiday data
	mutable int*		mpRefCount; //! reference counter to the holiday data(AQLMathCalendarData object)
	// free the memory acquired
	void clear(void);
	// shallow copy of the project
	void copy(const AQLMathCalendar& cal);
	//	reset the reference counter of AQLMathCalendarData object that the object holds
	void makeUnShared(void);
#ifdef __HAS_MIC__
	static common_lib::StaticMutex  mMutex;
#endif
};



///////////////////////////////////////////////////////////////////////////
// 
// collection class AQLMathCalendar
//
///////////////////////////////////////////////////////////////////////////
/*! 
    @brief Collection class of holiday information

	Class to represent holilday information of cities.

*/
class AQLMathCalendarCollection
{
public:
//  LIFECYCLE
    // default constructor
	AQLMathCalendarCollection(){};
    // destructor
	~AQLMathCalendarCollection(){};

//  QUERY
    // get AQLMathCalendar object from the specified city names
	const AQLMathCalendar& getCalendarData(const AQLString& cityName);

    // get a list of city names
	void getMemberNames(std::vector<AQLString>& arr);

// ysuzuki
    // set city name and holiday information
    void setCalendarData(const AQLString &cityName, const AQLMathCalendar &calendar);

    // free calendar information 
    /*!
       clear all map data (holiday city, calendar information)   
    */
    void				destroy() { mCalendarList.clear(); }
    // get inquired city names
    void  getInquiredCityNames(std::vector<AQLString>& names) const;

private:
    // not available
	AQLMathCalendarCollection(AQLMathCalendarCollection& calcol);
    // not available
	AQLMathCalendarCollection& operator=(AQLMathCalendarCollection& calcol);
	// get day of the week(enum value) from the string format, e.g. "SUN"
	AQLDayOfWeekEnum	getWeeklyValue(const AQLString& weekly);

    // calendar information with city code name key
	std::map<AQLString, AQLMathCalendar>	mCalendarList;
	// inquired calendar names
	std::set<AQLString>	mInqCityNames;
};
