/*! @file
    @brief  FX Vega setup class 
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldBasisDeltaOptionAnalytic.cpp
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
#include "LARiskConfigurationYieldBasisDeltaOptionAnalytic.h"
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
#include "LALinearRatesOptionValue.h"


using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldBasisDeltaOptionAnalytic::LARiskConfigurationYieldBasisDeltaOptionAnalytic(const LAString& risktype)
: LARiskConfigurationYieldBasisDelta()
{
	(void)risktype;
	//mAnalyticRiskType = risktype;
}

// destructor
/*!

*/
LARiskConfigurationYieldBasisDeltaOptionAnalytic::~LARiskConfigurationYieldBasisDeltaOptionAnalytic(void)
{
}

vector<pair<LAString, vector<LAObject *> > >
LARiskConfigurationYieldBasisDeltaOptionAnalytic::createRiskEntity(LAObjectPool &objPool) const
{
	return LARiskConfiguration::createOptionAnalyticRiskEntity(objPool);
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfigurationYieldBasisDeltaOptionAnalytic::setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const
{
	return LARiskConfiguration::setUpOptionAnalyticTargetNames(ccy,e);
}
/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYieldBasisDeltaOptionAnalytic::createScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	return LARiskConfiguration::createOptionAnalyticEntity(ccy,dataInstance,index);
}


/*!
    @brief create scenario2 object 

	@param[in] key ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationYieldBasisDeltaOptionAnalytic::createScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{	
	(void)ccy;(void)dataInstance;(void)index;
	return vector<LAObject *>(0);
}

/*!
    @brief create extra object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<vector<LAObject *> > 
*/
vector<vector<LAObject *> > 
LARiskConfigurationYieldBasisDeltaOptionAnalytic::createExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	(void)ccy;(void)dataInstance;(void)scenarioNum;(void)index;
	vector<vector<LAObject *> > ret(0);
	return ret;
}




/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationYieldBasisDeltaOptionAnalytic::getRiskName(void) const
{
	LAString ret = LARiskConfigurationYieldBasisDelta::getRiskName();
	ret += LAString("_") + RISK_FRONT_OPTIONANALYTIC;
	return ret;
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDeltaOptionAnalytic::isWave(const LAString &ccy) const
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
LARiskConfigurationYieldBasisDeltaOptionAnalytic::isGridSensitivity(const LAString &ccy) const
{
	ccy;
	return false;
}

/*!
    @brief return coefficient1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasisDeltaOptionAnalytic::getCoefficient1(const LAString &ccy) const
{
	(void)ccy;
	return LAString("0.0:") + LAString("1.0") + LAString(":0.0");
}

double 
LARiskConfigurationYieldBasisDeltaOptionAnalytic::getShiftValForOptionAnalytic(const LAString &ccy) const
{
	
	double shiftval = getScenario1ParallelShift(ccy);
	LAString bumpdirection = getBumpDirection(ccy);
	if (bumpdirection.toUpper() == RISK_BUMPDIRECTION_DOWNSHIFT)
		shiftval *= -1.0;


	const double divUnit = getDivUnit(ccy);
	if (divUnit == 0.0)
	{	
		throw LACoreInvalidData("Divid unit = 0 !!", __FILE__, __LINE__);
	}
	
	shiftval /= divUnit;
	
	return shiftval;
}

LAString
LARiskConfigurationYieldBasisDeltaOptionAnalytic::getAnalyticalRiskType(const LAString& ccy, LAObject& e) const
{
	LAString ret;
	LAString ircur = ccy;
	ircur.toUpper();

	LADataHolder* dh = &(e.getData(PRICING_DATA_DOMESTICCURRENCY, ISNOTNULL));
	LAString domcur = dynamic_cast<LADataString &>(dh->get()).get();

	if(ircur == domcur.toUpper())
	{
		mAnalyticRiskType = LAString(RHO);
		return mAnalyticRiskType;
	}
	else
	{
		mAnalyticRiskType = LAString(PHI);
		return mAnalyticRiskType;
	}
}


bool 
LARiskConfigurationYieldBasisDeltaOptionAnalytic::IsSucceedAnalyticalRiskType(const LAString& ccy, LAObject& e) const
{
	LAString ret;
	LAString ircur = ccy;
	ircur.toUpper();

	LADataHolder* dh = &(e.getData(PRICING_DATA_DOMESTICCURRENCY, ISNOTNULL));
	LAString domcur = dynamic_cast<LADataString &>(dh->get()).get();

	if(ircur == domcur.toUpper())
	{
		return true;
	}

	dh = &(e.getData(PRICING_DATA_FOREIGNCURRENCY, ISNOTNULL));
	LAString forcur = dynamic_cast<LADataString &>(dh->get()).get();

	if(ircur == forcur.toUpper())
	{
		return true;
	}
	else
	{
		return false;
	}

}
