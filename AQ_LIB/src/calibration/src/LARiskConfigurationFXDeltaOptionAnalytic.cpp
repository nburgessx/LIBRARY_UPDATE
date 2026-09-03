/*! @file
    @brief FX delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXDeltaOptionAnalytic.cpp
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


#include "LARiskConfigurationFXDeltaOptionAnalytic.h"
#include "LAMarketData.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationFXDeltaOptionAnalytic::LARiskConfigurationFXDeltaOptionAnalytic(const AQLString& risktype)
: LARiskConfigurationFXDeltaEx1()
{
	mAnalyticRiskType = risktype;
}

// destructor
/*!

*/
LARiskConfigurationFXDeltaOptionAnalytic::~LARiskConfigurationFXDeltaOptionAnalytic(void)
{
}


vector<pair<AQLString, vector<AQLObject *> > >
LARiskConfigurationFXDeltaOptionAnalytic::createRiskEntity(AQLObjectPool &objPool) const
{
	return LARiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfigurationFXDeltaOptionAnalytic::setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const
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
LARiskConfigurationFXDeltaOptionAnalytic::createScenario1Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
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
LARiskConfigurationFXDeltaOptionAnalytic::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
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
LARiskConfigurationFXDeltaOptionAnalytic::getCoefficient1(const AQLString &fx) const
{
	(void)fx;
	return AQLString("0.0:") + AQLString("1.0") + AQLString(":0.0");
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
LARiskConfigurationFXDeltaOptionAnalytic::getRiskName(void) const
{
	AQLString ret = LARiskConfigurationFXDeltaEx1::getRiskName();
	ret += AQLString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;	
}

double 
LARiskConfigurationFXDeltaOptionAnalytic::getShiftValForOptionAnalytic(const AQLString &fx) const
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
