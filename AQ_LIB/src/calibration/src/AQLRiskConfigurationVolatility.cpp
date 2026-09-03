/*! @file
    @brief Volatility setup class for calc risk
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationVolatility.cpp
//
//  DESCRIPTION :       Volatility setup class for calc risk 
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


#include "AQLRiskConfigurationVolatility.h"
#include "AQLStaticData.h"
#include "AQLDealUtils.h"
#include <algorithm>

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationVolatility::AQLRiskConfigurationVolatility(bool fxFlg)
:AQLRiskConfiguration(fxFlg)
{
}

// destructor
/*!

*/
AQLRiskConfigurationVolatility::~AQLRiskConfigurationVolatility(void)
{
}


 
/*!
    @brief create scenario1 object 

	@param[in] key ccy or fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationVolatility::createScenario1Entity(const AQLString &key, AQLDataInstance &dataInstance, int index)  const
{
	index;
	if (isRealCalib())
	{
		return createVolatilityEntity(key, dataInstance, SCENARIO_1, index);
	}
	else
	{
		return createVolatilityEntityOld(key, dataInstance, SCENARIO_1);
	}
}


/*!
    @brief create scenario2 object 

	@param[in] key ccy or fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationVolatility::createScenario2Entity(const AQLString &key, AQLDataInstance &dataInstance, int index)  const
{
	index;
	if (isRealCalib())
	{
		return createVolatilityEntity(key, dataInstance, SCENARIO_2, index);
	}
	else
	{
		return createVolatilityEntityOld(key, dataInstance, SCENARIO_2);
	}
}

/*!
    @brief return istargetccy

	@param[in] ccy
	@return bool 
*/
bool
AQLRiskConfigurationVolatility::isTarget(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	bool ret = AQLRiskConfiguration::isTarget(ccy);

	AQLStringVector simsde = AQLDealUtils::getSimulationSDECurrencys();
	unsigned int sdesize = simsde.size();

	if (sdesize == 0)
		return ret;

	//modify
	if (simsde.end() == std::find(simsde.begin(),simsde.end(),tmpCurrency))
	{
		return false;
	}
	return ret;

	/*if (simsde.end() != std::find(simsde.begin(),simsde.end(),tmpCurrency))
	{
		return true;
	}
	else
	{
		return false;
	}*/
}
