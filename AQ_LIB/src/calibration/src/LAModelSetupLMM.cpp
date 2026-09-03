/*! @file
    @brief Class to regist LMM master data
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAModelSetupLMM.cpp
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


#include "LAModelSetupLMM.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLSobol.h"
#include "LADefinitions.h"
#include "LADefinitionsLMM.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAStaticData.h"
#include "LADealUtils.h"

//====================LAModelSetupLMM ==============================

// constructor
/*!

*/
LAModelSetupLMM::LAModelSetupLMM(void) : LAModelSetupBase()
{
}

// destructor
/*!

*/
LAModelSetupLMM::~LAModelSetupLMM(void)
{
}

// 
/*!
    @brief regist extra data data to attributemaster object

	@param[in,out] dm AQLPriceDataManager &	
	@return void 
*/
void
LAModelSetupLMM::registAttrMasterEx(AQLPriceDataManager &dm)
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
LAModelSetupLMM::registEntityMasterEx(AQLDataInstance &dataInstance)
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
LAModelSetupLMM::registFunctionMasterEx(AQLDataInstance &dataInstance)
{
	AQLFunctionManager &fm = dataInstance.getFunctionMaster();
	// get property
	const LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	const unsigned long seedVal = static_cast<unsigned long>(staticData.getStaticData(KEY_SIMULATION_SEED).getDoubleValue());
	UlongArray seed(1, seedVal);
	// Sobol
	// direction integers
	AQLString sobolDI = staticData.getStaticData(KEY_SIMULATION_RAND_SOBOL_DIRECTIONINTEGERS);
	// factor num
	AQLString factorNumKey = STATIC_DATA_KEY_LMM_CORRELATION_CROSS_FACTOR_NUM;
	if (MADealUtils::getSDECurrencys().size() == 1)
	{
		factorNumKey = STATIC_DATA_KEY_LMM_CORRELATION_SIGLE_FACTOR_NUM;
	}

	AQLString baseCCY;
	if ( MADealUtils::getSimulationSDECurrencys().size() == 0 ) 
		baseCCY = MADealUtils::getSDECurrencys()[0].toLower();
	else
		baseCCY = MADealUtils::getSimulationSDECurrencys()[0].toLower(); 

	AQLString factorNumStr = staticData.getStaticData(baseCCY + factorNumKey);
	const unsigned int factorNum = factorNumStr.getIntValue();
	// is halley modification
	AQLDataBool tmp;
	tmp.convertFromString(staticData.getStaticData(KEY_SIMULATION_RAND_GAUSSIAN_ISHALLEYMODE));

	AQLSobol *pSobol = new AQLSobol(sobolDI, factorNum, tmp.get());
	fm.setFunction(pSobol, FN_RAND_SOBOL_STR);
	pSobol->setSeed(seed);
}




