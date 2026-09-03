/*! @file
    @brief LAModelSetup manger class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MAMasterRegistManager.cpp
//
//  DESCRIPTION :       LAModelSetup manger
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


#include "LAModelManager.h"
#include "LAString.h"
#include "LADefinitions.h"
#include "LAModelSetupLMM.h"
#include "LAModelSetupHW.h"
#include "LAModelSetupVanilla.h"

MAMasterRegistManager *MAMasterRegistManager::mpInstance = 0;

//================ MAMasterRegistManager ===================================
// constructor
/*!

*/
MAMasterRegistManager::MAMasterRegistManager(void)
{
}

// destructor
/*!

*/
MAMasterRegistManager::~MAMasterRegistManager(void)
{
}


// 
/*!
    @brief get unique instance

	@return  MAMasterRegistManager *
*/
MAMasterRegistManager *
MAMasterRegistManager::getInstance(void)
{
	if (!mpInstance)
	{
		mpInstance = new MAMasterRegistManager;
	}
	return mpInstance;

}

// 
/*!
    @brief create LAModelSetup

	return pointer is need to delete

	@param name
	@return  LAModelSetupBase *
*/
LAModelSetupBase *
MAMasterRegistManager::createRegister(const LAString &model)
{
	LAString tmpModel = model;
	tmpModel.toUpper();
	if (tmpModel == MODEL_LMM)
	{
		return new LAModelSetupLMM();
	}
	else if (tmpModel == MODEL_HW)
	{
		return new LAModelSetupHW();
	}
	else if (tmpModel == MODEL_IRVANILLA)
	{
		return new LAModelSetupVanilla();
	}
	else if (tmpModel == MODEL_IRSABR)
	{
		return new LAModelSetupVanilla();
	}
	else
	{
		return 0;
	}
}