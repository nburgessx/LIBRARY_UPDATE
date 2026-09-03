/*! @file
    @brief  FX Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolFXVegaOptionAnalytic.cpp
//
//  DESCRIPTION :         FX Vega setup class 
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


#include <algorithm>
#include "LARiskConfigurationVolFXVegaOptionAnalytic.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataFunction.h"
#include "LAPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADealUtils.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "AQLLinearFunc.h"


using namespace std;

// constructor
/*!

*/
LARiskConfigurationVolFXVegaOptionAnalytic::LARiskConfigurationVolFXVegaOptionAnalytic(const AQLString& risktype)
: LARiskConfigurationVolFXVega()
{
	mAnalyticRiskType = risktype;
}

// destructor
/*!

*/
LARiskConfigurationVolFXVegaOptionAnalytic::~LARiskConfigurationVolFXVegaOptionAnalytic(void)
{
}

vector<pair<AQLString, vector<AQLObject *> > >
LARiskConfigurationVolFXVegaOptionAnalytic::createRiskEntity(AQLObjectPool &objPool) const
{
	return LARiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfigurationVolFXVegaOptionAnalytic::setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const
{
	return LARiskConfiguration::setUpOptionAnalyticTargetNames(ccy,e);
}
/*!
    @brief create scenario1 object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
LARiskConfigurationVolFXVegaOptionAnalytic::createScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	return LARiskConfiguration::createOptionAnalyticEntity(fx,dataInstance,index);
}


/*!
    @brief create scenario2 object 

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
LARiskConfigurationVolFXVegaOptionAnalytic::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{	
	(void)fx;(void)dataInstance;(void)index;
	return vector<AQLObject *>(0);
}




/*!
    @brief return riskname

	@return AQLString
*/
AQLString
LARiskConfigurationVolFXVegaOptionAnalytic::getRiskName(void) const
{
	AQLString ret = LARiskConfigurationVolFXVega::getRiskName();
	ret += AQLString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolFXVegaOptionAnalytic::isWave(const AQLString &fx) const
{
	(void)fx;
	return false;
}

/*!
    @brief return isGridSensitivity

	@param[in] fx
	@return bool
*/
bool
LARiskConfigurationVolFXVegaOptionAnalytic::isGridSensitivity(const AQLString &fx) const
{
	(void)fx;
	return false;
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationVolFXVegaOptionAnalytic::getCoefficient1(const AQLString &fx) const
{
	(void)fx;
	return AQLString("0.0:") + AQLString("1.0") + AQLString(":0.0");
}

double 
LARiskConfigurationVolFXVegaOptionAnalytic::getShiftValForOptionAnalytic(const AQLString &fx) const
{
	
	double shiftval = getScenario1ShiftValue(fx);
	AQLString bumpdirection = getBumpDirection(fx);
	if (bumpdirection.toUpper() == RISK_BUMPDIRECTION_DOWNSHIFT)
		shiftval *= -1.0;

	const double divUnit = getDivUnit(fx);
	if (divUnit == 0.0)
	{	
		throw AQLCoreInvalidData("Divid unit = 0 !!", __FILE__, __LINE__);
	}
	
	shiftval /= divUnit;
	
	return shiftval;
}




