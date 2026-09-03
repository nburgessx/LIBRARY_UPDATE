/*! @file
    @brief Class declaration to manage the holiday information and the corresponding city name (any string).

	*/


#ifndef AQLMathCalendarSet_h
#define AQLMathCalendarSet_h

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "AQLString.h"
#include "AQLCoreSystemError.h"
#include "AQLCoreAppError.h"
#include "AQLMathCalendar.h"


/*! 
    @brief Class to manage the holiday information and the corresponding city name (any string).
*/
class AQLMathCalendarSet
{
public:
// LIFECYCLE
    // default constructor
    AQLMathCalendarSet(void);
    // destructor
    ~AQLMathCalendarSet();
    
    // set information corresponding to the name of the city holiday (any string)
    void setCalendarData(const AQLString &cityName, const AQLMathCalendar &calendar);

    // get calendar information from the selcected city name (any string)
    //static const AQLMathCalendar&    getCalendar(const AQLString& name);
    static const AQLMathCalendar&    getCalendar(const AQLString& name);
    // get inquired city names
    static void  getInquiredCityNames(std::vector<AQLString>& names);

private:
// only 1 copy exists since it is a static member
    // holiday information and data for multiple cities
    static AQLMathCalendarCollection     mCalendarCol;
#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif
};
#endif
