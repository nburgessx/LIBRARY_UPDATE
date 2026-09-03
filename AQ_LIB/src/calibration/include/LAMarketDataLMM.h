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
		DoubleArray &deltatenor, BoolVector &exFlag, const AQLDate &asOfDate, const AQLPriceDataDayCount &dayCount, const AQLString &freq, int max, 
		AQLStringVector &exTenor, const bool isDataOut = false);
	//==============================================================================
	//// setup volatility function
	//static void setUpVolFunc(const AQLString &calibFileName, const AQLString &adjFileName, const AQLString &maxFileName, 
	//	const DoubleArray &tenor_30_360, const DoubleArray &tenor, double skew, AQLDataInstance &dataInstance, AQLMathVolatility &vol); 
	////==============================================================================
	//// create volatility function
	//static std::vector<AQLFunctionBase *> createVolFunc(const AQLString &calibFileName, const AQLString &adjFileName, const AQLString &maxFileName, 
	//								const DoubleArray &tenor_30_360, const DoubleArray &tenor, double skew); 
	////==============================================================================
	//// get volatility data matrix
	//static void getVolMatrix(const AQLString &calibFileName, const AQLString &adjFileName, const AQLString &maxFileName, 
	//								const DoubleArray &tenor_30_360, const DoubleArray &tenor, double skew, DoubleMatrix &data); 

	//==============================================
	// get skew
	static double getSkew(const AQLString &currency);

	//==============================================
	// get skew
	static double getConstShift(const AQLString &currency);

	//==============================================
	// get max term
	static double getMaxTerm(const AQLString &currency);

	//==============================================
	// get extra tenor for simulation grid
	static AQLStringVector getGridExTenor();

	//==============================================
	// get extra tenor for canonical grid
	static AQLStringVector getCanonicalGridExTenor();

private:
	// constructor
	LAMarketDataLMM(void);
	// destructor
	~LAMarketDataLMM(void);
	// copy constructor
	LAMarketDataLMM(const LAMarketDataLMM &rhs);
	LAMarketDataLMM &operator=(const LAMarketDataLMM &rhs);
};
