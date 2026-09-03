/*! @file
    @brief  FX Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRDeltaOptionAnalytic.cpp
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
#include "LARiskConfigurationYieldIRDeltaOptionAnalytic.h"
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
#include "LALinearRatesOptionValue.h"


using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldIRDeltaOptionAnalytic::LARiskConfigurationYieldIRDeltaOptionAnalytic(const AQLString& risktype)
: LARiskConfigurationYieldIRDelta()
{
	(void)risktype;
	//mAnalyticRiskType = risktype;
}

// destructor
/*!

*/
LARiskConfigurationYieldIRDeltaOptionAnalytic::~LARiskConfigurationYieldIRDeltaOptionAnalytic(void)
{
}

vector<pair<AQLString, vector<AQLObject *> > >
LARiskConfigurationYieldIRDeltaOptionAnalytic::createRiskEntity(AQLObjectPool &objPool) const
{
	return LARiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfigurationYieldIRDeltaOptionAnalytic::setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const
{
	return LARiskConfiguration::setUpOptionAnalyticTargetNames(ccy,e);
}
/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
LARiskConfigurationYieldIRDeltaOptionAnalytic::createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	return LARiskConfiguration::createOptionAnalyticEntity(ccy,dataInstance,index);
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
LARiskConfigurationYieldIRDeltaOptionAnalytic::createExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	(void)ccy;(void)dataInstance;(void)scenarioNum;(void)index;
	vector<vector<AQLObject *> > ret(0);
	return ret;
}


/*!
    @brief create scenario2 object 

	@param[in] key ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
LARiskConfigurationYieldIRDeltaOptionAnalytic::createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{	
	(void)ccy;(void)dataInstance;(void)index;
	return vector<AQLObject *>(0);
}




/*!
    @brief return riskname

	@return AQLString
*/
AQLString
LARiskConfigurationYieldIRDeltaOptionAnalytic::getRiskName(void) const
{
	AQLString ret = LARiskConfigurationYieldIRDelta::getRiskName();
	ret += AQLString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRDeltaOptionAnalytic::isWave(const AQLString &ccy) const
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
LARiskConfigurationYieldIRDeltaOptionAnalytic::isGridSensitivity(const AQLString &ccy) const
{
	(void)ccy;
	return false;
}

/*!
    @brief return coefficient1

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYieldIRDeltaOptionAnalytic::getCoefficient1(const AQLString &ccy) const
{
	(void)ccy;
	return AQLString("0.0:") + AQLString("1.0") + AQLString(":0.0");
}

double 
LARiskConfigurationYieldIRDeltaOptionAnalytic::getShiftValForOptionAnalytic(const AQLString &ccy) const
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
LARiskConfigurationYieldIRDeltaOptionAnalytic::getAnalyticalRiskType(const AQLString& ccy, AQLObject& e) const
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
LARiskConfigurationYieldIRDeltaOptionAnalytic::IsSucceedAnalyticalRiskType(const AQLString& ccy, AQLObject& e) const
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
