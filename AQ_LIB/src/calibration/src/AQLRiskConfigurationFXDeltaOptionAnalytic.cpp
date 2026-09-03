/*! @file
    @brief FX delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationFXDeltaOptionAnalytic.cpp
//
//  DESCRIPTION :        FX delta setup class
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


#include "AQLRiskConfigurationFXDeltaOptionAnalytic.h"
#include "AQLMarketData.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationFXDeltaOptionAnalytic::AQLRiskConfigurationFXDeltaOptionAnalytic(const AQLString& risktype)
: AQLRiskConfigurationFXDeltaEx1()
{
	mAnalyticRiskType = risktype;
}

// destructor
/*!

*/
AQLRiskConfigurationFXDeltaOptionAnalytic::~AQLRiskConfigurationFXDeltaOptionAnalytic(void)
{
}


vector<pair<AQLString, vector<AQLObject *> > >
AQLRiskConfigurationFXDeltaOptionAnalytic::createRiskEntity(AQLObjectPool &objPool) const
{
	return AQLRiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
AQLRiskConfigurationFXDeltaOptionAnalytic::setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const
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
AQLRiskConfigurationFXDeltaOptionAnalytic::createScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
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
AQLRiskConfigurationFXDeltaOptionAnalytic::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
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
AQLRiskConfigurationFXDeltaOptionAnalytic::getCoefficient1(const AQLString &fx) const
{
	(void)fx;
	return AQLString("0.0:") + AQLString("1.0") + AQLString(":0.0");
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaOptionAnalytic::getRiskName(void) const
{
	AQLString ret = AQLRiskConfigurationFXDeltaEx1::getRiskName();
	ret += AQLString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;	
}

double 
AQLRiskConfigurationFXDeltaOptionAnalytic::getShiftValForOptionAnalytic(const AQLString &fx) const
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
