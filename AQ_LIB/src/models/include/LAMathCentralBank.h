#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDate.h"
#include "AQLCoreTemplateType.h"

#include <map>

//
// Provide access to the monetary committee meetings schedules of central banks 
//
// SYNPOSIS:
//     #include "LAMathCentralBank.h"
//
//     const std::vector<AQLDate>& ecbDates = LAMathCentralBank::meetingSchedule("ECB"); 
//
class LAMathCentralBank
{
	friend void setupCBSchedule( const AQLString*, const bool );			// in LibSetup
	friend void setupCBScheduleETrading( const AQLString*, const bool );	// in LibSetupETrading

public:
	typedef std::map<AQLString, DateVector> Schedules;

	// return sorted list of monetary committee meeting dates of a central bank
	// given by its three-letter abbreviation (case insensitive);
	static const DateVector& meetingSchedule(const AQLString& centralBank);

private:
	// disable instantiation
	LAMathCentralBank();
	~LAMathCentralBank();

    // for use in LibSetup, which executes in single main thread; hence no need for this to be thread-safe
	static Schedules& schedules();

	// initialise schedule dates
	static void loadScheduleDates();

	// load schedule dates from hard-coded defaults
	static void loadScheduleDatesFromDefaults();

};
