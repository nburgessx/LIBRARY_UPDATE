/*! @file
    @brief  FX ptberg model util class
*/
//  2012, Mizuho International London.
#ifndef MAPtbergUtils_h
#define MAPtbergUtils_h

///X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MAPtbergUtils.h
//
//  DESCRIPTION :       FX ptberg model util class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LADataInstance.h"
#include "LAFunctionManager.h"
#include "LACoreTemplateType.h"
#include "LAAlgorithm.h"
#include "LADataBasics.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataCalendar.h"
#include "LAMathDateCalculations.h"
#include "LAMathPathEntity.h"
#include "LAMathVolatility.h"
#include "LAPriceDataDayCount.h"
#include "LADefinitions.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAFileAccessor.h"
#include "LAMathVolFuncFX.h"
#include "LAPriceFXVolatility.h"
#include "LAConstant.h"



//===================== Class Declare MAPtbergUtils==================================
/*! 
    @brief FX ptberg model util class

   
*/
class MAPtbergUtils
{
public:
	//==============================================================================
	// check 3F calibration
	static bool is3FCalib(); 

private:
	// constructor
	MAPtbergUtils(void);
	// destructor
	~MAPtbergUtils(void);
	// copy constructor
	MAPtbergUtils(const MAPtbergUtils &rhs);
	MAPtbergUtils &operator=(const MAPtbergUtils &rhs);

};

#endif