/*! @file
    @brief LAScenarioConfigurationManager manager class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationManager.cpp
//
//  DESCRIPTION :       LAScenarioConfigurationManager 
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


#include "LAScenarioConfigurationManager.h"
#include "LAScenarioConfigurationYieldCurve.h"
#include "LAScenarioConfigurationYieldCurveZero.h"
#include "LAScenarioConfigurationYieldCurveBasis.h"
#include "LAScenarioConfigurationYieldCurveMultiParallel.h"
#include "LAScenarioConfigurationFX.h"
#include "LAScenarioConfigurationVolatility.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"


LAScenarioConfigurationManager *LAScenarioConfigurationManager::mpInstance = 0;

//================ LAScenarioConfigurationManager ===================================
// constructor
/*!

*/
LAScenarioConfigurationManager::LAScenarioConfigurationManager(void)
{
}

// destructor
/*!

*/
LAScenarioConfigurationManager::~LAScenarioConfigurationManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  LAScenarioConfigurationManager *
*/
LAScenarioConfigurationManager *
LAScenarioConfigurationManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new LAScenarioConfigurationManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create Scenario creator

	@param[in] type
	@return  LAScenarioConfiguration *
*/
LAScenarioConfiguration *
LAScenarioConfigurationManager::createScenarioCreator(const AQLString &type) const
{
	AQLString tmpType = type;
	tmpType.toUpper();
	if (tmpType == RISK_SCENARIO_YIELD)
	{
		return new LAScenarioConfigurationYieldCurve();
	}
	else if (tmpType == RISK_SCENARIO_YIELDZERO)
	{
		return new LAScenarioConfigurationYieldCurveZero();
	}
	else if (tmpType == RISK_SCENARIO_YIELDZEROBASIS)
	{
		return new LAScenarioConfigurationYieldCurveZero(true);
	}
	else if (tmpType == RISK_SCENARIO_YIELDBASIS)
	{
		return new LAScenarioConfigurationYieldCurveBasis();
	}
	else if (tmpType == RISK_SCENARIO_YIELDMULTIPARALLEL)
	{
		return new LAScenarioConfigurationYieldCurveMultiParallel();
	}
	else if (tmpType == RISK_SCENARIO_VOL)
	{
		return new LAScenarioConfigurationVolatility();
	}
	else if (tmpType == RISK_SCENARIO_FX)
	{
		return new LAScenarioConfigurationFX();
	}

	return 0;
}

