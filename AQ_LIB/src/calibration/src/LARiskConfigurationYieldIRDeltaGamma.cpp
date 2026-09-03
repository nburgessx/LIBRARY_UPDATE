/*! @file
    @brief  Yield delta gamma setup class
*/
//  2015, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRDeltaGamma.cpp
//
//  DESCRIPTION :       Yield delta gamma setup class
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

#include "LABasic.h"
#include "LALinearFunc.h"
#include "LARiskConfigurationYieldIRDeltaGamma.h"
#include "LAFileAccessor.h"
#include "LAMarketData.h"
#include "LADealUtils.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAStaticData.h"


using namespace std;


// constructor
/*!

*/
LARiskConfigurationYieldIRDeltaGamma::LARiskConfigurationYieldIRDeltaGamma(void)
:LARiskConfigurationYieldIRDelta()
{
}

// destructor
/*!

*/
LARiskConfigurationYieldIRDeltaGamma::~LARiskConfigurationYieldIRDeltaGamma(void)
{
}

/*!
    @brief return operator2

	@return LAString
*/
LAString
LARiskConfigurationYieldIRDeltaGamma::getOperator2(void) const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRDeltaGamma::getOutPutName2(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OUTPUTNAME2 + getCurveSuffix(ccy));
}


/*!
    @brief return coefficient1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRDeltaGamma::getCoefficient1(const LAString &ccy) const
{
	LAString bumpDirection = getPropBumpDirection(ccy);
	bumpDirection.toUpper();
	const double divUnit = getDivUnit(ccy);
	if (divUnit == 0.0)
	{
		throw LACoreInvalidData("Divid unit = 0 !!", __FILE__, __LINE__);
	}
	if (bumpDirection == RISK_BUMPDIRECTION_UPSHIFT)
	{
		const double val = 1.0 / divUnit;
		return LAString(-val) + LAString(":")
				+  LAString(val) + LAString(":0.0:0.0");
	}
	else if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
	{
		const double val = 1.0 / divUnit;
		return LAString(val) + LAString(":0.0:")
				+  LAString(-val) + LAString(":0.0");
	}
	else
	{
		const double val = 0.5 / divUnit;
		return LAString("0.0:") + LAString(val) + LAString(":")
						+  LAString(-val) + LAString(":0.0");
	}
}

/*!
    @brief return coefficient2

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRDeltaGamma::getCoefficient2(const LAString &ccy) const
{
	const double divUnit = getDivUnit(ccy);
	const double val = 1.0 / LAMath::pow(divUnit, 2.0);
	return LAString(-2.0 * val)  + LAString(":") + LAString(val) + LAString(":")
						+  LAString(val) + LAString(":0.0");

}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRDeltaGamma::getBumpDirection(const LAString &ccy) const
{
	return RISK_BUMPDIRECTION_UPDOWNSHIFT;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRDeltaGamma::getPropBumpDirection(const LAString &ccy) const
{
	return  LARiskConfigurationYieldIRDelta::getBumpDirection(ccy);
}
/*!
    @brief create zero bump object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<LAObject *> 
*/
//vector<LAObject *> 
//LARiskConfigurationYieldIRDeltaGamma::createZeroBumpYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
//{
//	throw LACoreInvalidData("ZeroRateBump is not supported in LARiskConfigurationYieldIRDeltaGamma", __FILE__, __LINE__);
//}
