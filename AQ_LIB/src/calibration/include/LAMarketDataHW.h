#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif


#include "LACoreTemplateType.h"

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


class LADate;
class LAString;
class LAPriceDataDayCount;


//===================== Class Declare LAMarketDataHW==================================
/*! 
    @brief HW(Libor market model) util class

   
*/
class LAMarketDataHW
{
public:
	//==============================================================================
	// get canonincal libor grid
	static void getCanonicalGrid(DoubleArray &tenor,const LADate &asOfDate, const LAPriceDataDayCount &dayCount, int max, bool isReflesh = false, const LAString *p_freq_str = 0);
	//==============================================================================
	// get Hull-White parameter (alpha, sigma)
	static void getHullWhiteParam(const LAString &fileName, DoubleArray &t_grid, DoubleArray &alph, DoubleArray &sigma);
	//==============================================================================

private:
	// constructor
	LAMarketDataHW(void);
	// destructor
	~LAMarketDataHW(void);
	// copy constructor
	LAMarketDataHW(const LAMarketDataHW &rhs);
	LAMarketDataHW &operator=(const LAMarketDataHW &rhs);
};
