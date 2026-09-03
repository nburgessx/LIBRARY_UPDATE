/*! @file
    @brief Class declaration to manage the holiday information and the corresponding city name (any string).

	*/


#ifndef LAMathCalendarSet_h
#define LAMathCalendarSet_h

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "LAString.h"
#include "LACoreSystemError.h"
#include "LACoreAppError.h"
#include "LAMathCalendar.h"


/*! 
    @brief Class to manage the holiday information and the corresponding city name (any string).
*/
class LAMathCalendarSet
{
public:
// LIFECYCLE
    // default constructor
    LAMathCalendarSet(void);
    // destructor
    ~LAMathCalendarSet();
    
    // set information corresponding to the name of the city holiday (any string)
    void setCalendarData(const LAString &cityName, const LAMathCalendar &calendar);

    // get calendar information from the selcected city name (any string)
    //static const LAMathCalendar&    getCalendar(const LAString& name);
    static const LAMathCalendar&    getCalendar(const LAString& name);
    // get inquired city names
    static void  getInquiredCityNames(std::vector<LAString>& names);

private:
// only 1 copy exists since it is a static member
    // holiday information and data for multiple cities
    static LAMathCalendarCollection     mCalendarCol;
#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif
};
#endif
