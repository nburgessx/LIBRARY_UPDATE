/*! @file
    @brief  M-Library Deal util class
*/
//  2007, AlgoQuantHub.
#ifndef AQLDealUtils_h
#define AQLDealUtils_h

///X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLDealUtils.h
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


#include "AQLCoreTemplateType.h"


struct AQLIndexData;
class AQLObjectHolder;
class AQLObjectPool;
class AQLDataMultiReference;
class AQLDate;
class AQLDate;

//===================== Class Declare AQLDealUtils==================================
/*! 
    @brief Deal class 

   
*/
class AQLDealUtils
{
public:
	//==============================================================================
	// get sde currencys
	static AQLStringVector getSDECurrencys(const bool isIncludeVol = false);
	//==============================================================================
	// get simulation sde currencys
	static AQLStringVector getSimulationSDECurrencys(const bool isIncludeVol = false);
	//==============================================================================
	// get  sde currencys all
	static AQLStringVector getSDECurrencysAll();
	//==============================================================================
	// get maxterm
	static int getMaxTerm(AQLObjectPool &objPool, const AQLDate &asOfDate, AQLString tradetype = AQLString("EXODERIVA"));
	//==============================================================================
	// get applied maturity
	static AQLString getAppMat();
	//==============================================================================
	// get all single cuccencys
	static AQLStringVector getAllSingleCurrencys();
	//==============================================================================
	// get ir volatility grids which are used for calculation
	static BoolMatrix getCalibTargetIRVolGrids(AQLObjectPool& objPool, 
											   const AQLDate& asOfDate,
											   const AQLString& ccy,
											   const AQLString& underlying,
											   const bool isPropSource = false);

private:
	// constructor
	AQLDealUtils(void);
	// destructor
	~AQLDealUtils(void);
	// copy constructor
	AQLDealUtils(const AQLDealUtils &rhs);
	AQLDealUtils &operator=(const AQLDealUtils &rhs);

	static int getCMSYears(const AQLDataMultiReference &ref, const AQLString &indexAttr);
	
	//==============================================================================
	// get one trade maxterm
	static int getMaxTerm(const AQLObjectHolder &objHolder, const AQLDate &asOfDate);
	static int getMaxTermFromPlainVanilla(const AQLObjectHolder &objHolder, const AQLDate &asOfDate);

	static void addCalibTargetFlag( BoolMatrix& calibTaretMat,
									double expiry, 
									double tenor,
									const DoubleVector& expiryVec,
									const DoubleVector& tenorVec);

	static void addStochasticVolIndex(AQLStringVector& ccys);

};

#endif