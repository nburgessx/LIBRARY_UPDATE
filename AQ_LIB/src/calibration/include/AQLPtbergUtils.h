/*! @file
    @brief  FX ptberg model util class
*/
#ifndef AQLPtbergUtils_h
#define AQLPtbergUtils_h

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLCoreTemplateType.h"
#include "AQLAlgorithm.h"
#include "AQLDataBasics.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataCalendar.h"
#include "AQLDateCalculations.h"
#include "AQLMathPathEntity.h"
#include "AQLMathVolatility.h"
#include "AQLPriceDataDayCount.h"
#include "AQLDefinitions.h"
#include "AQLCoreDataService.h"
#include "AQLStaticDataManager.h"
#include "AQLFileAccessor.h"
#include "AQLMathVolFuncFX.h"
#include "AQLPriceFXVolatility.h"
#include "AQLConstant.h"



//===================== Class Declare AQLPtbergUtils==================================
/*! 
    @brief FX ptberg model util class

   
*/
class AQLPtbergUtils
{
public:
	//==============================================================================
	// check 3F calibration
	static bool is3FCalib(); 

private:
	// constructor
	AQLPtbergUtils(void);
	// destructor
	~AQLPtbergUtils(void);
	// copy constructor
	AQLPtbergUtils(const AQLPtbergUtils &rhs);
	AQLPtbergUtils &operator=(const AQLPtbergUtils &rhs);

};

#endif