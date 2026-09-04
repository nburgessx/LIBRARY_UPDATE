/*! @file
    @brief  FX Vega setup class 
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "AQLRiskConfigurationYieldBasisDeltaOptionAnalytic.h"
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
#include "AQLLinearRatesOptionValue.h"


using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::AQLRiskConfigurationYieldBasisDeltaOptionAnalytic(const AQLString& risktype)
: AQLRiskConfigurationYieldBasisDelta()
{
	(void)risktype;
	//mAnalyticRiskType = risktype;
}

// destructor
/*!

*/
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::~AQLRiskConfigurationYieldBasisDeltaOptionAnalytic(void)
{
}

vector<pair<AQLString, vector<AQLObject *> > >
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::createRiskEntity(AQLObjectPool &objPool) const
{
	return AQLRiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const
{
	return AQLRiskConfiguration::setUpOptionAnalyticTargetNames(ccy,e);
}
/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	return AQLRiskConfiguration::createOptionAnalyticEntity(ccy,dataInstance,index);
}


/*!
    @brief create scenario2 object 

	@param[in] key ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{	
	(void)ccy;(void)dataInstance;(void)index;
	return vector<AQLObject *>(0);
}

/*!
    @brief create extra object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<vector<AQLObject *> > 
*/
vector<vector<AQLObject *> > 
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::createExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	(void)ccy;(void)dataInstance;(void)scenarioNum;(void)index;
	vector<vector<AQLObject *> > ret(0);
	return ret;
}




/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::getRiskName(void) const
{
	AQLString ret = AQLRiskConfigurationYieldBasisDelta::getRiskName();
	ret += AQLString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::isWave(const AQLString &ccy) const
{
	(void)ccy;
	return false;
}

/*!
    @brief return isGridSensitivity

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::isGridSensitivity(const AQLString &ccy) const
{
	ccy;
	return false;
}

/*!
    @brief return coefficient1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::getCoefficient1(const AQLString &ccy) const
{
	(void)ccy;
	return AQLString("0.0:") + AQLString("1.0") + AQLString(":0.0");
}

double 
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::getShiftValForOptionAnalytic(const AQLString &ccy) const
{
	
	double shiftval = getScenario1ParallelShift(ccy);
	AQLString bumpdirection = getBumpDirection(ccy);
	if (bumpdirection.toUpper() == RISK_BUMPDIRECTION_DOWNSHIFT)
		shiftval *= -1.0;


	const double divUnit = getDivUnit(ccy);
	if (divUnit == 0.0)
	{	
		throw AQLCoreInvalidData("Divid unit = 0 !!", __FILE__, __LINE__);
	}
	
	shiftval /= divUnit;
	
	return shiftval;
}

AQLString
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::getAnalyticalRiskType(const AQLString& ccy, AQLObject& e) const
{
	AQLString ret;
	AQLString ircur = ccy;
	ircur.toUpper();

	AQLDataHolder* dh = &(e.getData(PRICING_DATA_DOMESTICCURRENCY, ISNOTNULL));
	AQLString domcur = dynamic_cast<AQLDataString &>(dh->get()).get();

	if(ircur == domcur.toUpper())
	{
		mAnalyticRiskType = AQLString(RHO);
		return mAnalyticRiskType;
	}
	else
	{
		mAnalyticRiskType = AQLString(PHI);
		return mAnalyticRiskType;
	}
}


bool 
AQLRiskConfigurationYieldBasisDeltaOptionAnalytic::IsSucceedAnalyticalRiskType(const AQLString& ccy, AQLObject& e) const
{
	AQLString ret;
	AQLString ircur = ccy;
	ircur.toUpper();

	AQLDataHolder* dh = &(e.getData(PRICING_DATA_DOMESTICCURRENCY, ISNOTNULL));
	AQLString domcur = dynamic_cast<AQLDataString &>(dh->get()).get();

	if(ircur == domcur.toUpper())
	{
		return true;
	}

	dh = &(e.getData(PRICING_DATA_FOREIGNCURRENCY, ISNOTNULL));
	AQLString forcur = dynamic_cast<AQLDataString &>(dh->get()).get();

	if(ircur == forcur.toUpper())
	{
		return true;
	}
	else
	{
		return false;
	}

}
