#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif


#include "AQLCoreTemplateType.h"

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


class AQLDate;
class AQLString;
class AQLPriceDataDayCount;


//===================== Class Declare AQLMarketDataHW==================================
/*! 
    @brief HW(Libor market model) util class

   
*/
class AQLMarketDataHW
{
public:
	//==============================================================================
	// get canonincal libor grid
	static void getCanonicalGrid(DoubleArray &tenor,const AQLDate &asOfDate, const AQLPriceDataDayCount &dayCount, int max, bool isReflesh = false, const AQLString *p_freq_str = 0);
	//==============================================================================
	// get Hull-White parameter (alpha, sigma)
	static void getHullWhiteParam(const AQLString &fileName, DoubleArray &t_grid, DoubleArray &alph, DoubleArray &sigma);
	//==============================================================================

private:
	// constructor
	AQLMarketDataHW(void);
	// destructor
	~AQLMarketDataHW(void);
	// copy constructor
	AQLMarketDataHW(const AQLMarketDataHW &rhs);
	AQLMarketDataHW &operator=(const AQLMarketDataHW &rhs);
};
