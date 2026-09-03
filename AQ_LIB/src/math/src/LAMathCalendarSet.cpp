/*! @file
    @brief Implementation to manage the holiday information and the corresponding city name (any string).
*/


#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#endif

#include "LAString.h"
#include "LAMathCalendar.h"
#include "LAMathCalendarSet.h"
#ifdef __HAS_MIC__

#endif
using namespace std;

#ifdef __HAS_MIC__
common_lib::StaticMutex LAMathCalendarSet::mMutex;
#endif
LAMathCalendarCollection        LAMathCalendarSet::mCalendarCol;


/*!
    @brief default constructor
*/
LAMathCalendarSet::LAMathCalendarSet(void)
{
}

/*!
    @brief destructor
*/
LAMathCalendarSet::~LAMathCalendarSet()
{
}

/*!
    @brief set information corresponding to the name of the city holiday (any string)
    
    @param[in] cityName city holiday (any string)
    @param[in] calendar information
*/
void
LAMathCalendarSet::setCalendarData(const LAString &cityName, const LAMathCalendar &calendar)
{
	try
	{
#ifdef __HAS_MIC__
		common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
		LAString tmpCityName = cityName;
		tmpCityName.toUpper();
		mCalendarCol.setCalendarData(tmpCityName, calendar);
	}
	catch(LACoreError &e)
	{
		LACoreError ex("Error at LAMathCalendarSet::setCalendarData", __FILE__, __LINE__);
		ex += e;
		throw ex;
	}
	catch(...)
	{
		throw LACoreError("Unexpected Error at LAMathCalendarSet::setCalendarData", __FILE__, __LINE__);
	}
}

/*!
    @brief get calendar information from the selcected city name (any string)
    
    @param[in] name city name(any string)
    
    @return calendar information from the selcected city name
*/
const LAMathCalendar&
LAMathCalendarSet::getCalendar(const LAString& name)
{
	try
	{
#ifdef __HAS_MIC__
		common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
		LAString tmpCityName = name;
		tmpCityName.toUpper();
		const LAMathCalendar &ret = mCalendarCol.getCalendarData(tmpCityName);
		return ret;
	}
	catch(LACoreError &e)
	{
		LACoreError ex("Error at LAMathCalendarSet::getCalendar", __FILE__, __LINE__);
		ex += e;
		throw ex;
	}
	catch(...)
	{
		throw LACoreError("Unexpected Error at LAMathCalendarSet::getCalendar", __FILE__, __LINE__);
	}
}

/*!
    @brief get inquired city names

    @param[out] string representing city name
*/
void 
LAMathCalendarSet::getInquiredCityNames(std::vector<LAString>& names)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	mCalendarCol.getInquiredCityNames(names);
}
