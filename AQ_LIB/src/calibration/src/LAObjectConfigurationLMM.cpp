/*! @file
    @brief Object generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAObjectConfigurationLMM.cpp
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


#include "LAObjectConfigurationLMM.h"
#include "LAString.h"
#include "LADataBasics.h"
#include "LAPriceDataDayCount.h"
#include "LACoreDataService.h"
#include "LAMarketDataLMM.h"


using namespace std;
// constructor
/*!

*/
LAObjectConfigurationLMM::LAObjectConfigurationLMM()
: LAObjectConfiguration()
{
}

// destructor
/*!

*/
LAObjectConfigurationLMM::~LAObjectConfigurationLMM(void)
{
}

/*!
    @brief get sdetimegrid

	@param[out] timeGrid
*/
void
LAObjectConfigurationLMM::getSDETimeGrid(DoubleArray &timeGrid) const
{
	timeGrid.clear();
	DoubleArray tmp1;
	DoubleArray tmp2;
	BoolVector tmp3;
	int maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	LAString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	LAPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// create tenor 
	LAString freq = FREQ_SEMI_ANNUAL;
	if (LACoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	LAStringVector exTenor = LAMarketDataLMM::getCanonicalGridExTenor();
	const bool isDataOut = (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA);
 	LAMarketDataLMM::getCanonicalGrid(tmp1, timeGrid, tmp2, tmp3, asOfDate, dayCount, freq, maxTerm, exTenor, isDataOut);

	timeGrid.pop_back();
}


/*!
    @brief get sdetimegrid

	@param[out] timeGrid
*/
void
LAObjectConfigurationLMM::getSDEIntegralTimeGrid(DoubleArray &timeGrid) const
{
	timeGrid.clear();
	DoubleArray tmp1;
	DoubleArray tmp2;
	BoolVector tmp3;
	int maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	LAString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	LAPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// create integral time step 
	LAString freq = FREQ_SEMI_ANNUAL;
	if (LACoreDataService::getContext(ARG_KEY_SIMFREQ) == "QA" || LACoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	LAStringVector exTenor = LAMarketDataLMM::getGridExTenor();
	const bool isDataOut = (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA);
 	LAMarketDataLMM::getCanonicalGrid(tmp1, timeGrid, tmp2, tmp3, asOfDate, dayCount, freq, maxTerm, exTenor, isDataOut);
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