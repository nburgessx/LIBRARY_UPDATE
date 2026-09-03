/*! @file
    @brief AQLScenarioConfigurationManager manager class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLScenarioConfigurationManager.cpp
//
//  DESCRIPTION :       AQLScenarioConfigurationManager 
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


#include "AQLScenarioConfigurationManager.h"
#include "AQLScenarioConfigurationYieldCurve.h"
#include "AQLScenarioConfigurationYieldCurveZero.h"
#include "AQLScenarioConfigurationYieldCurveBasis.h"
#include "AQLScenarioConfigurationYieldCurveMultiParallel.h"
#include "AQLScenarioConfigurationFX.h"
#include "AQLScenarioConfigurationVolatility.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"


AQLScenarioConfigurationManager *AQLScenarioConfigurationManager::mpInstance = 0;

//================ AQLScenarioConfigurationManager ===================================
// constructor
/*!

*/
AQLScenarioConfigurationManager::AQLScenarioConfigurationManager(void)
{
}

// destructor
/*!

*/
AQLScenarioConfigurationManager::~AQLScenarioConfigurationManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  AQLScenarioConfigurationManager *
*/
AQLScenarioConfigurationManager *
AQLScenarioConfigurationManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new AQLScenarioConfigurationManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create Scenario creator

	@param[in] type
	@return  AQLScenarioConfiguration *
*/
AQLScenarioConfiguration *
AQLScenarioConfigurationManager::createScenarioCreator(const AQLString &type) const
{
	AQLString tmpType = type;
	tmpType.toUpper();
	if (tmpType == RISK_SCENARIO_YIELD)
	{
		return new AQLScenarioConfigurationYieldCurve();
	}
	else if (tmpType == RISK_SCENARIO_YIELDZERO)
	{
		return new AQLScenarioConfigurationYieldCurveZero();
	}
	else if (tmpType == RISK_SCENARIO_YIELDZEROBASIS)
	{
		return new AQLScenarioConfigurationYieldCurveZero(true);
	}
	else if (tmpType == RISK_SCENARIO_YIELDBASIS)
	{
		return new AQLScenarioConfigurationYieldCurveBasis();
	}
	else if (tmpType == RISK_SCENARIO_YIELDMULTIPARALLEL)
	{
		return new AQLScenarioConfigurationYieldCurveMultiParallel();
	}
	else if (tmpType == RISK_SCENARIO_VOL)
	{
		return new AQLScenarioConfigurationVolatility();
	}
	else if (tmpType == RISK_SCENARIO_FX)
	{
		return new AQLScenarioConfigurationFX();
	}

	return 0;
}

