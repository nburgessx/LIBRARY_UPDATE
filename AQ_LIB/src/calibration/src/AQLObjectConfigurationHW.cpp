/*! @file
    @brief Object generator class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLObjectConfigurationHW.h"
#include "AQLString.h"
#include "AQLDataBasics.h"
#include "AQLPriceDataDayCount.h"
#include "AQLCoreDataService.h"
#include "AQLMarketDataHW.h"
#include "AQLDefinitions.h"


using namespace std;
// constructor
/*!

*/
AQLObjectConfigurationHW::AQLObjectConfigurationHW()
: AQLObjectConfiguration()
{
}

// destructor
/*!

*/
AQLObjectConfigurationHW::~AQLObjectConfigurationHW(void)
{
}

/*!
    @brief get sdetimegrid

	@param[out] timeGrid
*/
void
AQLObjectConfigurationHW::getSDETimeGrid(DoubleArray &timeGrid) const
{
	timeGrid.clear();
	int maxTerm = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	const AQLString canonicalFreq = AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ);
	AQLMarketDataHW::getCanonicalGrid(timeGrid, asOfDate, dayCount, maxTerm, false, &canonicalFreq);
	timeGrid.pop_back();
}

/*!
    @brief get sdetimegrid

	@param[out] timeGrid
*/
void
AQLObjectConfigurationHW::getSDEIntegralTimeGrid(DoubleArray &timeGrid) const
{
	getSDETimeGrid(timeGrid);
}

