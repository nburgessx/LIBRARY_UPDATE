/*! @file
    @brief Class to regist HW master data
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAModelSetupHW.cpp
//
//  DESCRIPTION :       HW Master data regist class
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


#include "LAModelSetupHW.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LASobol.h"
#include "LADefinitions.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAStaticData.h"
#include "LADealUtils.h"

//====================LAModelSetupHW ==============================

// constructor
/*!

*/
LAModelSetupHW::LAModelSetupHW(void) : LAModelSetupBase()
{
}

// destructor
/*!

*/
LAModelSetupHW::~LAModelSetupHW(void)
{
}

// 
/*!
    @brief regist extra data data to attributemaster object

	@param[in,out] dm LAPriceDataManager &	
	@return void 
*/
void
LAModelSetupHW::registAttrMasterEx(LAPriceDataManager &dm)
{
	dm;
}


// 
/*!
    @brief regist extra object data to entitymaster object

	@param[in,out] dataInstance LADataInstance &	
	@return void 
*/
void
LAModelSetupHW::registEntityMasterEx(LADataInstance &dataInstance)
{
	dataInstance;
}

// 
/*!
    @brief regist extra function data to functionmaster object

	@param[in,out] dataInstance LADataInstance &
	@return void 
*/
void
LAModelSetupHW::registFunctionMasterEx(LADataInstance &dataInstance)
{
	LAFunctionManager &fm = dataInstance.getFunctionMaster();
	// get property
	const LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	const unsigned long seedVal = static_cast<unsigned long>(staticData.getStaticData(KEY_SIMULATION_SEED).getDoubleValue());
	UlongArray seed(1, seedVal);
	// Sobol
	// direction integers
	LAString sobolDI = staticData.getStaticData(KEY_SIMULATION_RAND_SOBOL_DIRECTIONINTEGERS);
	// is halley modification
	LADataBool tmp;
	tmp.convertFromString(staticData.getStaticData(KEY_SIMULATION_RAND_GAUSSIAN_ISHALLEYMODE));
	// factor
	unsigned int factor = MADealUtils::getSimulationSDECurrencys(true).size();
	if (0 == factor)
		factor = MADealUtils::getSDECurrencys(true).size();
	
	LASobol *pSobol = new LASobol(sobolDI, factor, tmp.get());
	fm.setFunction(pSobol, FN_RAND_SOBOL_STR);
	pSobol->setSeed(seed);	
}




