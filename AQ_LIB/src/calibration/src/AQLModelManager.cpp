/*! @file
    @brief AQLModelSetup manger class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLMasterRegistManager.cpp
//
//  DESCRIPTION :       AQLModelSetup manger
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


#include "AQLModelManager.h"
#include "AQLString.h"
#include "AQLDefinitions.h"
#include "AQLModelSetupLMM.h"
#include "AQLModelSetupHW.h"
#include "AQLModelSetupVanilla.h"

AQLMasterRegistManager *AQLMasterRegistManager::mpInstance = 0;

//================ AQLMasterRegistManager ===================================
// constructor
/*!

*/
AQLMasterRegistManager::AQLMasterRegistManager(void)
{
}

// destructor
/*!

*/
AQLMasterRegistManager::~AQLMasterRegistManager(void)
{
}


// 
/*!
    @brief get unique instance

	@return  AQLMasterRegistManager *
*/
AQLMasterRegistManager *
AQLMasterRegistManager::getInstance(void)
{
	if (!mpInstance)
	{
		mpInstance = new AQLMasterRegistManager;
	}
	return mpInstance;

}

// 
/*!
    @brief create AQLModelSetup

	return pointer is need to delete

	@param name
	@return  AQLModelSetupBase *
*/
AQLModelSetupBase *
AQLMasterRegistManager::createRegister(const AQLString &model)
{
	AQLString tmpModel = model;
	tmpModel.toUpper();
	if (tmpModel == MODEL_LMM)
	{
		return new AQLModelSetupLMM();
	}
	else if (tmpModel == MODEL_HW)
	{
		return new AQLModelSetupHW();
	}
	else if (tmpModel == MODEL_IRVANILLA)
	{
		return new AQLModelSetupVanilla();
	}
	else if (tmpModel == MODEL_IRSABR)
	{
		return new AQLModelSetupVanilla();
	}
	else
	{
		return 0;
	}
}