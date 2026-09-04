/*! @file
    @brief AQLObjectConfigurationManager factory class
*/
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLObjectConfigurationManager.h"
#include "AQLDefinitions.h"
#include "AQLObjectConfiguration.h"
#include "AQLObjectConfigurationLMM.h"
#include "AQLObjectConfigurationHW.h"
#include "AQLObjectConfigurationVanilla.h"

#include <iostream>

AQLObjectConfigurationManager *AQLObjectConfigurationManager::mpInstance = 0;

//================ AQLObjectConfigurationManager ===================================
// constructor
/*!

*/
AQLObjectConfigurationManager::AQLObjectConfigurationManager(void)
{
}

// destructor
/*!

*/
AQLObjectConfigurationManager::~AQLObjectConfigurationManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return AQLObjectConfigurationManager *
*/
AQLObjectConfigurationManager *
AQLObjectConfigurationManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new AQLObjectConfigurationManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create object setupper

	@param[in] model
	@return AQLObjectConfiguration *
*/
AQLObjectConfiguration *
AQLObjectConfigurationManager::createEntitySetUpper(const AQLString &model) const
{
	AQLString tmpModel = model;
	tmpModel.toUpper();
	if (tmpModel == MODEL_LMM)
	{
		return new AQLObjectConfigurationLMM();
	}
	else if (tmpModel == MODEL_HW)
	{
		return new AQLObjectConfigurationHW();
	}
	else if (tmpModel == MODEL_IRVANILLA)
	{
		return new AQLObjectConfigurationVanilla();
	}
	else if (tmpModel == MODEL_IRSABR)
	{
		return new AQLObjectConfigurationVanilla();
	}
	return 0;
}


