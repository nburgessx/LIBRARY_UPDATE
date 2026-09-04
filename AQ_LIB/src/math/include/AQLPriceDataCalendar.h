#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "AQLPriceDataType.h"
#include "AQLCoreAppError.h"
#include "AQLCoreSystemError.h"
#include "AQLString.h"
#include "AQLMathCalendar.h"
#include "AQLCoreTemplateType.h"
#include <set>
#include <vector>

// AQLPriceDataCalendar Data ID
#define DATA_CALENDAR  40

// Holiday-centre join delimiter. A multi-centre calendar string is written
// "SYB+LNB" (Sydney + London business days). The legacy ':' is still accepted
// on input until the shipped generator JSON is migrated (Phase 6.4) -- see
// rebrand/calendar_delimiter_sites.md.
constexpr char CALENDAR_CENTRE_DELIMITER = '+';

// Split a holiday-centre string into individual centre codes. Accepts the new
// '+' separator and, transitionally, the legacy ':'. Accepting both is safe:
// centre codes are alphanumeric and never contain either character.
AQLStringVector splitCalendarCentres( const AQLString& centres );

/*! 
    @brief Data calendar class to represent the data (type calendar).
*/
class AQLPriceDataCalendar : public AQLPriceDataType
{
public:
//  LIFECYCLE

	// default constructor
    AQLPriceDataCalendar(void);
	// constructor
	AQLPriceDataCalendar(AQLStringVector city);
	// copy constructor
    AQLPriceDataCalendar(const AQLPriceDataCalendar& attr);
	// destructor
	virtual ~AQLPriceDataCalendar(void);

//  QUERY
    // deep copy of the data object
    virtual AQLPriceDataType*	clone() const;

    // return (calendar city name) in the string representation
    virtual AQLString	convertToString(void) const;

    // get calendar that the class has
	const std::set<const AQLMathCalendar *>&	getCalendarSet(void)const;
   // get calendar that the class has
	const AQLMathCalendar	&getCalendar(const bool isCache = true) const;

    // get the business date added to (s) from base date(d)
	AQLDate				getBusinessDay(const AQLDate& d, int s) const;
    
    // count the number of business days between two dates
	int				    businessDays( const AQLDate& fromDate, const AQLDate& toDate ) const;
    
    // get end of month date
	AQLDate				getEOMDay(const AQLDate& d) const;
    // get end of week date
	AQLDate				getEOWDay(const AQLDate& d) const;
	// get beginning of month date
	AQLDate				getBOMDay(const AQLDate& d) const;

//  OPERATION
    // set the holiday Data calendar from the specified city code string
    virtual void		convertFromString(const AQLString& str);

protected:
    // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);

    // compare the contents against the other objecet
    virtual int			compare(const AQLPriceDataType& a) const;
 //   // get calendar that the class has
	//const AQLMathCalendar&	getCalendar(void)const;

private:
	std::set<const AQLMathCalendar *>	mCalendarSet;		// holiday calendar
	mutable AQLMathCalendar  *mpCalendar;		// holiday calendar
	StringSet			mCity;			// set of cities
	static AQLMathCalendar	mStdCalendar;	// standard calendar (Sat. and Sun. are set to be holiday)
	static bool			mInitialize;	// initialize flag of standard calendar
#ifdef __HAS_MIC__
	static common_lib::StaticMutex  mMutex;
#endif
};

