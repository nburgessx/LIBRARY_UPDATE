/*! @file
    @brief  FX Vega setup class 
*/
//  2008, Mizuho International London.
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
#include "LAString.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LAPriceDataFunction.h"
#include "LAPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADealUtils.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LALinearFunc.h"


using namespace std;

// constructor
/*!

*/
LARiskConfigurationVolFXVegaOptionAnalytic::LARiskConfigurationVolFXVegaOptionAnalytic(const LAString& risktype)
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

vector<pair<LAString, vector<LAObject *> > >
LARiskConfigurationVolFXVegaOptionAnalytic::createRiskEntity(LAObjectPool &objPool) const
{
	return LARiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfigurationVolFXVegaOptionAnalytic::setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const
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
LARiskConfigurationVolFXVegaOptionAnalytic::createScenario1Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
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
LARiskConfigurationVolFXVegaOptionAnalytic::createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{	
	(void)fx;(void)dataInstance;(void)index;
	return vector<LAObject *>(0);
}




/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationVolFXVegaOptionAnalytic::getRiskName(void) const
{
	LAString ret = LARiskConfigurationVolFXVega::getRiskName();
	ret += LAString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationVolFXVegaOptionAnalytic::isWave(const LAString &fx) const
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
LARiskConfigurationVolFXVegaOptionAnalytic::isGridSensitivity(const LAString &fx) const
{
	(void)fx;
	return false;
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationVolFXVegaOptionAnalytic::getCoefficient1(const LAString &fx) const
{
	(void)fx;
	return LAString("0.0:") + LAString("1.0") + LAString(":0.0");
}

double 
LARiskConfigurationVolFXVegaOptionAnalytic::getShiftValForOptionAnalytic(const LAString &fx) const
{
	
	double shiftval = getScenario1ShiftValue(fx);
	LAString bumpdirection = getBumpDirection(fx);
	if (bumpdirection.toUpper() == RISK_BUMPDIRECTION_DOWNSHIFT)
		shiftval *= -1.0;

	const double divUnit = getDivUnit(fx);
	if (divUnit == 0.0)
	{	
		throw LACoreInvalidData("Divid unit = 0 !!", __FILE__, __LINE__);
	}
	
	shiftval /= divUnit;
	
	return shiftval;
}




