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

//===================== Class Declare LAMarketDataLMM==================================
/*! 
    @brief LMM(Libor market model) util class

   
*/
class LAMarketDataLMM
{
public:
	//==============================================================================
	// get canonincal libor grid
	static void getCanonicalGrid(DoubleArray &tenor_30_360, DoubleArray &tenor,
		DoubleArray &deltatenor, BoolVector &exFlag, const LADate &asOfDate, const LAPriceDataDayCount &dayCount, const LAString &freq, int max, 
		LAStringVector &exTenor, const bool isDataOut = false);
	//==============================================================================
	//// setup volatility function
	//static void setUpVolFunc(const LAString &calibFileName, const LAString &adjFileName, const LAString &maxFileName, 
	//	const DoubleArray &tenor_30_360, const DoubleArray &tenor, double skew, LADataInstance &dataInstance, LAMathVolatility &vol); 
	////==============================================================================
	//// create volatility function
	//static std::vector<LAFunctionBase *> createVolFunc(const LAString &calibFileName, const LAString &adjFileName, const LAString &maxFileName, 
	//								const DoubleArray &tenor_30_360, const DoubleArray &tenor, double skew); 
	////==============================================================================
	//// get volatility data matrix
	//static void getVolMatrix(const LAString &calibFileName, const LAString &adjFileName, const LAString &maxFileName, 
	//								const DoubleArray &tenor_30_360, const DoubleArray &tenor, double skew, DoubleMatrix &data); 

	//==============================================
	// get skew
	static double getSkew(const LAString &currency);

	//==============================================
	// get skew
	static double getConstShift(const LAString &currency);

	//==============================================
	// get max term
	static double getMaxTerm(const LAString &currency);

	//==============================================
	// get extra tenor for simulation grid
	static LAStringVector getGridExTenor();

	//==============================================
	// get extra tenor for canonical grid
	static LAStringVector getCanonicalGridExTenor();

private:
	// constructor
	LAMarketDataLMM(void);
	// destructor
	~LAMarketDataLMM(void);
	// copy constructor
	LAMarketDataLMM(const LAMarketDataLMM &rhs);
	LAMarketDataLMM &operator=(const LAMarketDataLMM &rhs);
};
