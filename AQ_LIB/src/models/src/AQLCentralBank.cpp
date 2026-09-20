#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#pragma warning(disable:4996)	// strncpy is safe if we make sure a null character is added, which we do here
#endif

#include "AQLCentralBank.h"
#include "AQLCoreAppError.h"

#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#if defined(WIN32) || defined(WIN64)
#include <boost/thread/once.hpp>
#include <boost/static_assert.hpp>
#else
#include <mutex>
#endif

namespace 
{
#if defined(WIN32) || defined(WIN64)
	boost::once_flag schedulesLoaded = BOOST_ONCE_INIT;
#else
	std::once_flag schedulesLoaded;
#endif

	const char* CBDefault[] = { "ECB", "BOE", "FED" };

	const size_t nCBDefault = sizeof(CBDefault) / sizeof(const char*);

    // Note that ECB dates are published and referred to as: "Press conference following the Governing Council meeting of the ECB"
	// https://www.ecb.europa.eu/press/calendars/mgcgc/html/index.en.html
    
	// dates need to be sorted
    // These dates are taken from the above link, which is typically updated once a year for the following year. Both the below
    // and the Central Bank Holiday Calendar file CBSchedule.csv should be updated. The calendar file can be found as follows:
    // %AlgoQuantLib%/resouce/config/CBSchedule.csv
	int schedulesDefault[][nCBDefault] = {
		// ECB,		BoE,	Fed
		{ 20150122,	0,		0 },
		{ 20150305,	0,		0 },
		{ 20150415,	0,		0 },
		{ 20150603,	0,		0 },
		{ 20150716,	0,		0 },
		{ 20150903,	0,		0 },
		{ 20151022,	0,		0 },
		{ 20151203,	0,		0 },
		{ 20160121,	0,		0 },
		{ 20160310,	0,		0 },
		{ 20160421,	0,		0 },
		{ 20160602,	0,		0 },
		{ 20160721,	0,		0 },
		{ 20160809,	0,		0 },
		{ 20161020,	0,		0 },
		{ 20161208,	0,		0 },
        { 20170119,	0,		0 },
        { 20170309,	0,		0 },
        { 20170427,	0,		0 },
        { 20170608,	0,		0 },
        { 20170720,	0,		0 },
        { 20170907,	0,		0 },
        { 20171026,	0,		0 },
        { 20171214,	0,		0 },
        { 20180125,	0,		0 },
        { 20180308,	0,		0 },
        { 20180426,	0,		0 },
        { 20180614,	0,		0 },
        { 20180726,	0,		0 },
        { 20180913,	0,		0 },
        { 20181025,	0,		0 },
        { 20181213,	0,		0 },
		{ 20190124,	0,		0 },
		{ 20190307,	0,		0 },
		{ 20190410,	0,		0 },
		{ 20190606,	0,		0 },
		{ 20190725,	0,		0 },
		{ 20190912,	0,		0 },
		{ 20191024,	0,		0 },
		{ 20191212,	0,		0 },
		{ 20200121,	0,		0 },
		{ 20200320,	0,		0 },
		{ 20200430,	0,		0 },
		{ 20200604,	0,		0 },
		{ 20200716,	0,		0 },
		{ 20200910,	0,		0 },
		{ 20201029,	0,		0 },
		{ 20201210,	0,		0 },
		{ 20210121,	0,		0 },
		{ 20210311,	0,		0 },
		{ 20210422,	0,		0 },
		{ 20210722,	0,		0 },
		{ 20210909,	0,		0 },
		{ 20211028,	0,		0 },
		{ 20211216,	0,		0 }
	};
}

std::map<AQLString, DateVector>& AQLCentralBank::schedules()
{
	static std::map<AQLString, DateVector> sched;
	return sched;
}

void AQLCentralBank::loadScheduleDates()
{
	if (schedules().size() == 0) {
		// configuration file not loaded - use hard-coded defaults
		loadScheduleDatesFromDefaults();
	}
}

void AQLCentralBank::loadScheduleDatesFromDefaults()
{
#if defined(WIN32) || defined(WIN64)
	BOOST_STATIC_ASSERT( sizeof(schedulesDefault) % (sizeof(int) * nCBDefault) == 0 );
#else
	assert( sizeof(schedulesDefault) % (sizeof(int) * nCBDefault) == 0 );
#endif

	Schedules& scheds = schedules();

	const size_t rows = sizeof(schedulesDefault)/(sizeof(int) * nCBDefault);
	const size_t cols = nCBDefault;
	for (size_t j = 0; j != cols; ++j) {
		AQLString cb(CBDefault[j]);

		DateVector& dvec = scheds[cb.toUpper()];

		assert(dvec.size() == 0);

		dvec.reserve(rows);

		for (size_t i = 0; i != rows; ++i) {
			const int yyyymmdd = schedulesDefault[i][j];

			if (yyyymmdd == 0) continue;
			
			assert(0 <= yyyymmdd && yyyymmdd <= 99991231);
			const std::div_t res0 = std::div(yyyymmdd, 100);
			const int dd = res0.rem;
			assert(0 <= dd && dd <= 31);
			const std::div_t res1 = std::div(res0.quot, 100);
			const int mm = res1.rem;
			assert(0 <= mm && mm <= 12);
			const int yyyy = res1.quot;
			assert(0 <= yyyy && yyyy <= 9999);

			AQLDate d;
			d.setYear(yyyy);
			d.setMonth(mm);
			d.setDay(dd);

			dvec.push_back(d);
		}
	}
}

const DateVector& AQLCentralBank::meetingSchedule(const AQLString& cb_input)
{
#if defined(WIN32) || defined(WIN64)
	boost::call_once(&loadScheduleDates, schedulesLoaded);
#else	
	std::call_once(schedulesLoaded, loadScheduleDates);
#endif

	const Schedules& scheds = schedules();

	AQLString cb(cb_input);
	Schedules::const_iterator it = scheds.find(cb.toUpper());

	if (it != scheds.end()) {
		return it->second;
	}

	throw AQLCoreAppError("unsupported central bank", __FILE__, __LINE__);
}
