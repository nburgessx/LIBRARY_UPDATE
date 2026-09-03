/*! @file
    @brief Object generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAObjectConfigurationHW.cpp
//
//  DESCRIPTION :        Object setupper
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAObjectConfigurationHW.h"
#include "AQLString.h"
#include "AQLDataBasics.h"
#include "AQLPriceDataDayCount.h"
#include "LACoreDataService.h"
#include "LAMarketDataHW.h"
#include "LADefinitions.h"


using namespace std;
// constructor
/*!

*/
LAObjectConfigurationHW::LAObjectConfigurationHW()
: LAObjectConfiguration()
{
}

// destructor
/*!

*/
LAObjectConfigurationHW::~LAObjectConfigurationHW(void)
{
}

/*!
    @brief get sdetimegrid

	@param[out] timeGrid
*/
void
LAObjectConfigurationHW::getSDETimeGrid(DoubleArray &timeGrid) const
{
	timeGrid.clear();
	int maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	const AQLString canonicalFreq = LACoreDataService::getContext(ARG_KEY_CANONICALFREQ);
	LAMarketDataHW::getCanonicalGrid(timeGrid, asOfDate, dayCount, maxTerm, false, &canonicalFreq);
	timeGrid.pop_back();
}

/*!
    @brief get sdetimegrid

	@param[out] timeGrid
*/
void
LAObjectConfigurationHW::getSDEIntegralTimeGrid(DoubleArray &timeGrid) const
{
	getSDETimeGrid(timeGrid);
}

