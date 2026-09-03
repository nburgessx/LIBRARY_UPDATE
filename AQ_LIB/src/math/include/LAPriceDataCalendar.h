#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "LAPriceDataType.h"
#include "LACoreAppError.h"
#include "LACoreSystemError.h"
#include "LAString.h"
#include "LAMathCalendar.h"
#include "LACoreTemplateType.h"
#include <set>

// LAPriceDataCalendar Data ID
#define DATA_CALENDAR  40

/*! 
    @brief Data calendar class to represent the data (type calendar).
*/
class LAPriceDataCalendar : public LAPriceDataType
{
public:
//  LIFECYCLE

	// default constructor
    LAPriceDataCalendar(void);
	// constructor
	LAPriceDataCalendar(LAStringVector city);
	// copy constructor
    LAPriceDataCalendar(const LAPriceDataCalendar& attr);
	// destructor
	virtual ~LAPriceDataCalendar(void);

//  QUERY
    // deep copy of the data object
    virtual LAPriceDataType*	clone() const;

    // return (calendar city name) in the string representation
    virtual LAString	convertToString(void) const;

    // get calendar that the class has
	const std::set<const LAMathCalendar *>&	getCalendarSet(void)const;
   // get calendar that the class has
	const LAMathCalendar	&getCalendar(const bool isCache = true) const;

    // get the business date added to (s) from base date(d)
	LADate				getBusinessDay(const LADate& d, int s) const;
    
    // count the number of business days between two dates
	int				    businessDays( const LADate& fromDate, const LADate& toDate ) const;
    
    // get end of month date
	LADate				getEOMDay(const LADate& d) const;
    // get end of week date
	LADate				getEOWDay(const LADate& d) const;
	// get beginning of month date
	LADate				getBOMDay(const LADate& d) const;

//  OPERATION
    // set the holiday Data calendar from the specified city code string
    virtual void		convertFromString(const LAString& str);

protected:
    // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);

    // compare the contents against the other objecet
    virtual int			compare(const LAPriceDataType& a) const;
 //   // get calendar that the class has
	//const LAMathCalendar&	getCalendar(void)const;

private:
	std::set<const LAMathCalendar *>	mCalendarSet;		// holiday calendar
	mutable LAMathCalendar  *mpCalendar;		// holiday calendar
	StringSet			mCity;			// set of cities
	static LAMathCalendar	mStdCalendar;	// standard calendar (Sat. and Sun. are set to be holiday)
	static bool			mInitialize;	// initialize flag of standard calendar
#ifdef __HAS_MIC__
	static common_lib::StaticMutex  mMutex;
#endif
};

