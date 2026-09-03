/*! @file
    @brief Volatility setup class for calc risk
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolatility.cpp
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


#include "LARiskConfigurationVolatility.h"
#include "LAStaticData.h"
#include "LADealUtils.h"
#include <algorithm>

using namespace std;

// constructor
/*!

*/
LARiskConfigurationVolatility::LARiskConfigurationVolatility(bool fxFlg)
:LARiskConfiguration(fxFlg)
{
}

// destructor
/*!

*/
LARiskConfigurationVolatility::~LARiskConfigurationVolatility(void)
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
LARiskConfigurationVolatility::createScenario1Entity(const AQLString &key, AQLDataInstance &dataInstance, int index)  const
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
LARiskConfigurationVolatility::createScenario2Entity(const AQLString &key, AQLDataInstance &dataInstance, int index)  const
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
LARiskConfigurationVolatility::isTarget(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	bool ret = LARiskConfiguration::isTarget(ccy);

	AQLStringVector simsde = MADealUtils::getSimulationSDECurrencys();
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
