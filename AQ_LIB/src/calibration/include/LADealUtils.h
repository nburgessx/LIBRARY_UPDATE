/*! @file
    @brief  M-Library Deal util class
*/
//  2007, AlgoQuantHub.
#ifndef MADealUtils_h
#define MADealUtils_h

///X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MADealUtils.h
//
//  DESCRIPTION :       M-Library Deal util class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LACoreTemplateType.h"


struct MAIndexData;
class LAObjectHolder;
class LAObjectPool;
class LADataMultiReference;
class LADate;
class LADate;

//===================== Class Declare MADealUtils==================================
/*! 
    @brief Deal class 

   
*/
class MADealUtils
{
public:
	//==============================================================================
	// get sde currencys
	static LAStringVector getSDECurrencys(const bool isIncludeVol = false);
	//==============================================================================
	// get simulation sde currencys
	static LAStringVector getSimulationSDECurrencys(const bool isIncludeVol = false);
	//==============================================================================
	// get  sde currencys all
	static LAStringVector getSDECurrencysAll();
	//==============================================================================
	// get maxterm
	static int getMaxTerm(LAObjectPool &objPool, const LADate &asOfDate, LAString tradetype = LAString("EXODERIVA"));
	//==============================================================================
	// get applied maturity
	static LAString getAppMat();
	//==============================================================================
	// get all single cuccencys
	static LAStringVector getAllSingleCurrencys();
	//==============================================================================
	// get ir volatility grids which are used for calculation
	static BoolMatrix getCalibTargetIRVolGrids(LAObjectPool& objPool, 
											   const LADate& asOfDate,
											   const LAString& ccy,
											   const LAString& underlying,
											   const bool isPropSource = false);

private:
	// constructor
	MADealUtils(void);
	// destructor
	~MADealUtils(void);
	// copy constructor
	MADealUtils(const MADealUtils &rhs);
	MADealUtils &operator=(const MADealUtils &rhs);

	static int getCMSYears(const LADataMultiReference &ref, const LAString &indexAttr);
	
	//==============================================================================
	// get one trade maxterm
	static int getMaxTerm(const LAObjectHolder &objHolder, const LADate &asOfDate);
	static int getMaxTermFromPlainVanilla(const LAObjectHolder &objHolder, const LADate &asOfDate);

	static void addCalibTargetFlag( BoolMatrix& calibTaretMat,
									double expiry, 
									double tenor,
									const DoubleVector& expiryVec,
									const DoubleVector& tenorVec);

	static void addStochasticVolIndex(LAStringVector& ccys);

};

#endif