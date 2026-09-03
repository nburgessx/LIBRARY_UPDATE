/*! @file
    @brief FX delta setup class
*/
//  2008, Mizuho International London.
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
LARiskConfigurationFXDeltaOptionAnalytic::LARiskConfigurationFXDeltaOptionAnalytic(const LAString& risktype)
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


vector<pair<LAString, vector<LAObject *> > >
LARiskConfigurationFXDeltaOptionAnalytic::createRiskEntity(LAObjectPool &objPool) const
{
	return LARiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfigurationFXDeltaOptionAnalytic::setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const
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
LARiskConfigurationFXDeltaOptionAnalytic::createScenario1Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
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
LARiskConfigurationFXDeltaOptionAnalytic::createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
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
LARiskConfigurationFXDeltaOptionAnalytic::getCoefficient1(const LAString &fx) const
{
	(void)fx;
	return LAString("0.0:") + LAString("1.0") + LAString(":0.0");
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationFXDeltaOptionAnalytic::getRiskName(void) const
{
	LAString ret = LARiskConfigurationFXDeltaEx1::getRiskName();
	ret += LAString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;	
}

double 
LARiskConfigurationFXDeltaOptionAnalytic::getShiftValForOptionAnalytic(const LAString &fx) const
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
