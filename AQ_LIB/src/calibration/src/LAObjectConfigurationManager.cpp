/*! @file
    @brief LAObjectConfigurationManager factory class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAObjectConfigurationManager.cpp
//
//  DESCRIPTION :       LAObjectConfigurationManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAObjectConfigurationManager.h"
#include "LADefinitions.h"
#include "LAObjectConfiguration.h"
#include "LAObjectConfigurationLMM.h"
#include "LAObjectConfigurationHW.h"
#include "LAObjectConfigurationVanilla.h"

#include <iostream>

LAObjectConfigurationManager *LAObjectConfigurationManager::mpInstance = 0;

//================ LAObjectConfigurationManager ===================================
// constructor
/*!

*/
LAObjectConfigurationManager::LAObjectConfigurationManager(void)
{
}

// destructor
/*!

*/
LAObjectConfigurationManager::~LAObjectConfigurationManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return LAObjectConfigurationManager *
*/
LAObjectConfigurationManager *
LAObjectConfigurationManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new LAObjectConfigurationManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create object setupper

	@param[in] model
	@return LAObjectConfiguration *
*/
LAObjectConfiguration *
LAObjectConfigurationManager::createEntitySetUpper(const AQLString &model) const
{
	AQLString tmpModel = model;
	tmpModel.toUpper();
	if (tmpModel == MODEL_LMM)
	{
		return new LAObjectConfigurationLMM();
	}
	else if (tmpModel == MODEL_HW)
	{
		return new LAObjectConfigurationHW();
	}
	else if (tmpModel == MODEL_IRVANILLA)
	{
		return new LAObjectConfigurationVanilla();
	}
	else if (tmpModel == MODEL_IRSABR)
	{
		return new LAObjectConfigurationVanilla();
	}
	return 0;
}


