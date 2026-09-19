/*! @file
    @brief Class declaration to manage the holiday information and the corresponding city name (any string).
*/

#ifndef AQLCalendarSet_h
#define AQLCalendarSet_h

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "AQLString.h"
#include "AQLCoreSystemError.h"
#include "AQLCoreAppError.h"
#include "AQLCalendar.h"


/*! 
    @brief Class to manage the holiday information and the corresponding city name (any string).
*/
class AQLCalendarSet
{
public:
    
    // default constructor
    AQLCalendarSet(void);

    // destructor
    ~AQLCalendarSet();
    
    // set information corresponding to the name of the city holiday (any string)
    void setCalendarData(const AQLString &cityName, const AQLCalendar &calendar);

    // get calendar information from the selcected city name (any string)
    //static const AQLCalendar&    getCalendar(const AQLString& name);

    static const AQLCalendar&    getCalendar(const AQLString& name);

    // get inquired city names
    static void  getCityNames(std::vector<AQLString>& names);

private:

    // only 1 copy exists since it is a static member
    // holiday information and data for multiple cities
    static AQLMathCalendarCollection     calendarCol_;
};
#endif
