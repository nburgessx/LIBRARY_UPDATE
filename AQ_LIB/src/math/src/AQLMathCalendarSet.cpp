/*! @file
    @brief Implementation to manage the holiday information and the corresponding city name (any string).
*/


#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#endif

#include "AQLString.h"
#include "AQLMathCalendar.h"
#include "AQLMathCalendarSet.h"
#ifdef __HAS_MIC__

#endif
using namespace std;

#ifdef __HAS_MIC__
common_lib::StaticMutex AQLMathCalendarSet::mMutex;
#endif
AQLMathCalendarCollection        AQLMathCalendarSet::mCalendarCol;


/*!
    @brief default constructor
*/
AQLMathCalendarSet::AQLMathCalendarSet(void)
{
}

/*!
    @brief destructor
*/
AQLMathCalendarSet::~AQLMathCalendarSet()
{
}

/*!
    @brief set information corresponding to the name of the city holiday (any string)
    
    @param[in] cityName city holiday (any string)
    @param[in] calendar information
*/
void
AQLMathCalendarSet::setCalendarData(const AQLString &cityName, const AQLMathCalendar &calendar)
{
	try
	{
#ifdef __HAS_MIC__
		common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
		AQLString tmpCityName = cityName;
		tmpCityName.toUpper();
		mCalendarCol.setCalendarData(tmpCityName, calendar);
	}
	catch(AQLCoreError &e)
	{
		AQLCoreError ex("Error at AQLMathCalendarSet::setCalendarData", __FILE__, __LINE__);
		ex += e;
		throw ex;
	}
	catch(...)
	{
		throw AQLCoreError("Unexpected Error at AQLMathCalendarSet::setCalendarData", __FILE__, __LINE__);
	}
}

/*!
    @brief get calendar information from the selcected city name (any string)
    
    @param[in] name city name(any string)
    
    @return calendar information from the selcected city name
*/
const AQLMathCalendar&
AQLMathCalendarSet::getCalendar(const AQLString& name)
{
	try
	{
#ifdef __HAS_MIC__
		common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
		AQLString tmpCityName = name;
		tmpCityName.toUpper();
		const AQLMathCalendar &ret = mCalendarCol.getCalendarData(tmpCityName);
		return ret;
	}
	catch(AQLCoreError &e)
	{
		AQLCoreError ex("Error at AQLMathCalendarSet::getCalendar", __FILE__, __LINE__);
		ex += e;
		throw ex;
	}
	catch(...)
	{
		throw AQLCoreError("Unexpected Error at AQLMathCalendarSet::getCalendar", __FILE__, __LINE__);
	}
}

/*!
    @brief get inquired city names

    @param[out] string representing city name
*/
void 
AQLMathCalendarSet::getInquiredCityNames(std::vector<AQLString>& names)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	mCalendarCol.getInquiredCityNames(names);
}
