/*! @file
    @brief Implementation to manage the holiday information and the corresponding city name (any string).
*/


#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#endif

#include "AQLString.h"
#include "AQLCalendar.h"
#include "AQLCalendarSet.h"
using namespace std;

AQLMathCalendarCollection        AQLCalendarSet::calendarCol_;


/*!
    @brief default constructor
*/
AQLCalendarSet::AQLCalendarSet(void)
{
}

/*!
    @brief destructor
*/
AQLCalendarSet::~AQLCalendarSet()
{
}

/*!
    @brief set information corresponding to the name of the city holiday (any string)
    
    @param[in] cityName city holiday (any string)
    @param[in] calendar information
*/
void
AQLCalendarSet::setCalendarData(const AQLString &cityName, const AQLCalendar &calendar)
{
	try
	{
		AQLString tmpCityName = cityName;
		tmpCityName.toUpper();
		calendarCol_.setCalendarData(tmpCityName, calendar);
	}
	catch(AQLCoreError &e)
	{
		AQLCoreError ex("Error at AQLCalendarSet::setCalendarData", __FILE__, __LINE__);
		ex += e;
		throw ex;
	}
	catch(...)
	{
		throw AQLCoreError("Unexpected Error at AQLCalendarSet::setCalendarData", __FILE__, __LINE__);
	}
}

/*!
    @brief get calendar information from the selcected city name (any string)
    
    @param[in] name city name(any string)
    
    @return calendar information from the selcected city name
*/
const AQLCalendar&
AQLCalendarSet::getCalendar(const AQLString& name)
{
	try
	{
		AQLString tmpCityName = name;
		tmpCityName.toUpper();
		const AQLCalendar &ret = calendarCol_.getCalendarData(tmpCityName);
		return ret;
	}
	catch(AQLCoreError &e)
	{
		AQLCoreError ex("Error at AQLCalendarSet::getCalendar", __FILE__, __LINE__);
		ex += e;
		throw ex;
	}
	catch(...)
	{
		throw AQLCoreError("Unexpected Error at AQLCalendarSet::getCalendar", __FILE__, __LINE__);
	}
}

/*!
    @brief get inquired city names

    @param[out] string representing city name
*/
void 
AQLCalendarSet::getCityNames(std::vector<AQLString>& names)
{
	calendarCol_.getCityNames(names);
}
