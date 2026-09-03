/*! @file
    @brief Class to regist LMM master data
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLModelSetupLMM.cpp
//
//  DESCRIPTION :       LMM Master data regist class
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


#include "AQLModelSetupLMM.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLSobol.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsLMM.h"
#include "AQLCoreDataService.h"
#include "AQLStaticDataManager.h"
#include "AQLStaticData.h"
#include "AQLDealUtils.h"

//====================AQLModelSetupLMM ==============================

// constructor
/*!

*/
AQLModelSetupLMM::AQLModelSetupLMM(void) : AQLModelSetupBase()
{
}

// destructor
/*!

*/
AQLModelSetupLMM::~AQLModelSetupLMM(void)
{
}

// 
/*!
    @brief regist extra data data to attributemaster object

	@param[in,out] dm AQLPriceDataManager &	
	@return void 
*/
void
AQLModelSetupLMM::registAttrMasterEx(AQLPriceDataManager &dm)
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
AQLModelSetupLMM::registEntityMasterEx(AQLDataInstance &dataInstance)
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
AQLModelSetupLMM::registFunctionMasterEx(AQLDataInstance &dataInstance)
{
	AQLFunctionManager &fm = dataInstance.getFunctionMaster();
	// get property
	const AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	const unsigned long seedVal = static_cast<unsigned long>(staticData.getStaticData(KEY_SIMULATION_SEED).getDoubleValue());
	UlongArray seed(1, seedVal);
	// Sobol
	// direction integers
	AQLString sobolDI = staticData.getStaticData(KEY_SIMULATION_RAND_SOBOL_DIRECTIONINTEGERS);
	// factor num
	AQLString factorNumKey = STATIC_DATA_KEY_LMM_CORRELATION_CROSS_FACTOR_NUM;
	if (AQLDealUtils::getSDECurrencys().size() == 1)
	{
		factorNumKey = STATIC_DATA_KEY_LMM_CORRELATION_SIGLE_FACTOR_NUM;
	}

	AQLString baseCCY;
	if ( AQLDealUtils::getSimulationSDECurrencys().size() == 0 ) 
		baseCCY = AQLDealUtils::getSDECurrencys()[0].toLower();
	else
		baseCCY = AQLDealUtils::getSimulationSDECurrencys()[0].toLower(); 

	AQLString factorNumStr = staticData.getStaticData(baseCCY + factorNumKey);
	const unsigned int factorNum = factorNumStr.getIntValue();
	// is halley modification
	AQLDataBool tmp;
	tmp.convertFromString(staticData.getStaticData(KEY_SIMULATION_RAND_GAUSSIAN_ISHALLEYMODE));

	AQLSobol *pSobol = new AQLSobol(sobolDI, factorNum, tmp.get());
	fm.setFunction(pSobol, FN_RAND_SOBOL_STR);
	pSobol->setSeed(seed);
}




