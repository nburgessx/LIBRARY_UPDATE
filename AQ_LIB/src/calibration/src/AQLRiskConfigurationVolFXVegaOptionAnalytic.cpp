/*! @file
    @brief  FX Vega setup class 
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "AQLRiskConfigurationVolFXVegaOptionAnalytic.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLPricePortfolioValue.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLDealUtils.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLLinearFunc.h"


using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationVolFXVegaOptionAnalytic::AQLRiskConfigurationVolFXVegaOptionAnalytic(const AQLString& risktype)
: AQLRiskConfigurationVolFXVega()
{
	mAnalyticRiskType = risktype;
}

// destructor
/*!

*/
AQLRiskConfigurationVolFXVegaOptionAnalytic::~AQLRiskConfigurationVolFXVegaOptionAnalytic(void)
{
}

vector<pair<AQLString, vector<AQLObject *> > >
AQLRiskConfigurationVolFXVegaOptionAnalytic::createRiskEntity(AQLObjectPool &objPool) const
{
	return AQLRiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
AQLRiskConfigurationVolFXVegaOptionAnalytic::setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const
{
	return AQLRiskConfiguration::setUpOptionAnalyticTargetNames(ccy,e);
}
/*!
    @brief create scenario1 object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationVolFXVegaOptionAnalytic::createScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	return AQLRiskConfiguration::createOptionAnalyticEntity(fx,dataInstance,index);
}


/*!
    @brief create scenario2 object 

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationVolFXVegaOptionAnalytic::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{	
	(void)fx;(void)dataInstance;(void)index;
	return vector<AQLObject *>(0);
}




/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationVolFXVegaOptionAnalytic::getRiskName(void) const
{
	AQLString ret = AQLRiskConfigurationVolFXVega::getRiskName();
	ret += AQLString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationVolFXVegaOptionAnalytic::isWave(const AQLString &fx) const
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
AQLRiskConfigurationVolFXVegaOptionAnalytic::isGridSensitivity(const AQLString &fx) const
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
AQLRiskConfigurationVolFXVegaOptionAnalytic::getCoefficient1(const AQLString &fx) const
{
	(void)fx;
	return AQLString("0.0:") + AQLString("1.0") + AQLString(":0.0");
}

double 
AQLRiskConfigurationVolFXVegaOptionAnalytic::getShiftValForOptionAnalytic(const AQLString &fx) const
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




