/*! @file
    @brief FX delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftDeltaOptionAnalytic.cpp
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


#include "LARiskConfigurationFXShiftDeltaOptionAnalytic.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LALinearFunc.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationFXShiftDeltaOptionAnalytic::LARiskConfigurationFXShiftDeltaOptionAnalytic(const LAString& risktype)
: LARiskConfigurationFXShiftDelta()
{
	mAnalyticRiskType = risktype;
}

// destructor
/*!

*/
LARiskConfigurationFXShiftDeltaOptionAnalytic::~LARiskConfigurationFXShiftDeltaOptionAnalytic(void)
{
}


vector<pair<LAString, vector<LAObject *> > >
LARiskConfigurationFXShiftDeltaOptionAnalytic::createRiskEntity(LAObjectPool &objPool) const
{
	return LARiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfigurationFXShiftDeltaOptionAnalytic::setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const
{
	return LARiskConfiguration::setUpOptionAnalyticTargetNames(ccy,e);
}
/*!
    @brief create scenario1 object 

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationFXShiftDeltaOptionAnalytic::createScenario1Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	return LARiskConfiguration::createOptionAnalyticEntity(fx,dataInstance,index);
}


/*!
    @brief create scenario2 object 

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationFXShiftDeltaOptionAnalytic::createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{	
	(void)fx;(void)dataInstance;(void)index;
	return vector<LAObject *>(0);
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXShiftDeltaOptionAnalytic::getCoefficient1(const LAString &fx) const
{
	(void)fx;
	return LAString("0.0:") + LAString("1.0") + LAString(":0.0");
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationFXShiftDeltaOptionAnalytic::getRiskName(void) const
{
	LAString ret = LARiskConfigurationFXShiftDelta::getRiskName();
	ret += LAString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;	
}

double 
LARiskConfigurationFXShiftDeltaOptionAnalytic::getShiftValForOptionAnalytic(const LAString &fx) const
{
	double shiftval = getShiftVal(fx,SCENARIO_1);
	const double divUnit = getDivUnit(fx);
	if (divUnit == 0.0)
	{	
		throw LACoreInvalidData("Divid unit = 0 !!", __FILE__, __LINE__);
	}
	
	shiftval /= divUnit;
	
	return shiftval;
}
