/*! @file
    @brief Object generator class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLObjectConfigurationLMM.h"
#include "AQLString.h"
#include "AQLDataBasics.h"
#include "AQLPriceDataDayCount.h"
#include "AQLCoreDataService.h"
#include "AQLMarketDataLMM.h"


using namespace std;
// constructor
/*!

*/
AQLObjectConfigurationLMM::AQLObjectConfigurationLMM()
: AQLObjectConfiguration()
{
}

// destructor
/*!

*/
AQLObjectConfigurationLMM::~AQLObjectConfigurationLMM(void)
{
}

/*!
    @brief get sdetimegrid

	@param[out] timeGrid
*/
void
AQLObjectConfigurationLMM::getSDETimeGrid(DoubleArray &timeGrid) const
{
	timeGrid.clear();
	DoubleArray tmp1;
	DoubleArray tmp2;
	BoolVector tmp3;
	int maxTerm = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// create tenor 
	AQLString freq = FREQ_SEMI_ANNUAL;
	if (AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	AQLStringVector exTenor = AQLMarketDataLMM::getCanonicalGridExTenor();
	const bool isDataOut = (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA);
 	AQLMarketDataLMM::getCanonicalGrid(tmp1, timeGrid, tmp2, tmp3, asOfDate, dayCount, freq, maxTerm, exTenor, isDataOut);

	timeGrid.pop_back();
}


/*!
    @brief get sdetimegrid

	@param[out] timeGrid
*/
void
AQLObjectConfigurationLMM::getSDEIntegralTimeGrid(DoubleArray &timeGrid) const
{
	timeGrid.clear();
	DoubleArray tmp1;
	DoubleArray tmp2;
	BoolVector tmp3;
	int maxTerm = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// create integral time step 
	AQLString freq = FREQ_SEMI_ANNUAL;
	if (AQLCoreDataService::getContext(ARG_KEY_SIMFREQ) == "QA" || AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	AQLStringVector exTenor = AQLMarketDataLMM::getGridExTenor();
	const bool isDataOut = (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA);
 	AQLMarketDataLMM::getCanonicalGrid(tmp1, timeGrid, tmp2, tmp3, asOfDate, dayCount, freq, maxTerm, exTenor, isDataOut);
	timeGrid.pop_back();

	DoubleArray sdeTimeGrid;
	getSDETimeGrid(sdeTimeGrid);

	const double maxVal = sdeTimeGrid.back();
	DoubleArray::iterator it = --timeGrid.end();
	while (it != timeGrid.begin())
	{
		if (*it > maxVal)
		{
			timeGrid.erase(it);
			it = --timeGrid.end();
		}
		else
		{
			break;
		}
	}
	
}