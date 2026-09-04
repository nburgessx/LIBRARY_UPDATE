/*! @file
    @brief FX delta setup class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRiskConfigurationFXShiftDeltaOptionAnalytic.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLLinearFunc.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationFXShiftDeltaOptionAnalytic::AQLRiskConfigurationFXShiftDeltaOptionAnalytic(const AQLString& risktype)
: AQLRiskConfigurationFXShiftDelta()
{
	mAnalyticRiskType = risktype;
}

// destructor
/*!

*/
AQLRiskConfigurationFXShiftDeltaOptionAnalytic::~AQLRiskConfigurationFXShiftDeltaOptionAnalytic(void)
{
}


vector<pair<AQLString, vector<AQLObject *> > >
AQLRiskConfigurationFXShiftDeltaOptionAnalytic::createRiskEntity(AQLObjectPool &objPool) const
{
	return AQLRiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
AQLRiskConfigurationFXShiftDeltaOptionAnalytic::setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const
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
AQLRiskConfigurationFXShiftDeltaOptionAnalytic::createScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
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
AQLRiskConfigurationFXShiftDeltaOptionAnalytic::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{	
	(void)fx;(void)dataInstance;(void)index;
	return vector<AQLObject *>(0);
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDeltaOptionAnalytic::getCoefficient1(const AQLString &fx) const
{
	(void)fx;
	return AQLString("0.0:") + AQLString("1.0") + AQLString(":0.0");
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXShiftDeltaOptionAnalytic::getRiskName(void) const
{
	AQLString ret = AQLRiskConfigurationFXShiftDelta::getRiskName();
	ret += AQLString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;	
}

double 
AQLRiskConfigurationFXShiftDeltaOptionAnalytic::getShiftValForOptionAnalytic(const AQLString &fx) const
{
	double shiftval = getShiftVal(fx,SCENARIO_1);
	const double divUnit = getDivUnit(fx);
	if (divUnit == 0.0)
	{	
		throw AQLCoreInvalidData("Divid unit = 0 !!", __FILE__, __LINE__);
	}
	
	shiftval /= divUnit;
	
	return shiftval;
}
