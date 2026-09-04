/*! @file
    @brief Class to regist HW master data
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLModelSetupHW.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLSobol.h"
#include "AQLDefinitions.h"
#include "AQLCoreDataService.h"
#include "AQLStaticDataManager.h"
#include "AQLStaticData.h"
#include "AQLDealUtils.h"

//====================AQLModelSetupHW ==============================

// constructor
/*!

*/
AQLModelSetupHW::AQLModelSetupHW(void) : AQLModelSetupBase()
{
}

// destructor
/*!

*/
AQLModelSetupHW::~AQLModelSetupHW(void)
{
}

// 
/*!
    @brief regist extra data data to attributemaster object

	@param[in,out] dm AQLPriceDataManager &	
	@return void 
*/
void
AQLModelSetupHW::registAttrMasterEx(AQLPriceDataManager &dm)
{
	dm;
}


// 
/*!
    @brief regist extra object data to entitymaster object

	@param[in,out] dataInstance AQLDataInstance &	
	@return void 
*/
void
AQLModelSetupHW::registEntityMasterEx(AQLDataInstance &dataInstance)
{
	dataInstance;
}

// 
/*!
    @brief regist extra function data to functionmaster object

	@param[in,out] dataInstance AQLDataInstance &
	@return void 
*/
void
AQLModelSetupHW::registFunctionMasterEx(AQLDataInstance &dataInstance)
{
	AQLFunctionManager &fm = dataInstance.getFunctionMaster();
	// get property
	const AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	const unsigned long seedVal = static_cast<unsigned long>(staticData.getStaticData(KEY_SIMULATION_SEED).getDoubleValue());
	UlongArray seed(1, seedVal);
	// Sobol
	// direction integers
	AQLString sobolDI = staticData.getStaticData(KEY_SIMULATION_RAND_SOBOL_DIRECTIONINTEGERS);
	// is halley modification
	AQLDataBool tmp;
	tmp.convertFromString(staticData.getStaticData(KEY_SIMULATION_RAND_GAUSSIAN_ISHALLEYMODE));
	// factor
	unsigned int factor = AQLDealUtils::getSimulationSDECurrencys(true).size();
	if (0 == factor)
		factor = AQLDealUtils::getSDECurrencys(true).size();
	
	AQLSobol *pSobol = new AQLSobol(sobolDI, factor, tmp.get());
	fm.setFunction(pSobol, FN_RAND_SOBOL_STR);
	pSobol->setSeed(seed);	
}




