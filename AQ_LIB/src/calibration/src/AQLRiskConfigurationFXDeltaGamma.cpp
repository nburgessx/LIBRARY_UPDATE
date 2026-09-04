/*! @file
    @brief FX delta gamma setup class
*/
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBasic.h"
#include "AQLRiskConfigurationFXDeltaGamma.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLLinearFunc.h"

#include "AQLFileAccessor.h"
#include "AQLDealUtils.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationFXDeltaGamma::AQLRiskConfigurationFXDeltaGamma()
: AQLRiskConfigurationFXDeltaEx1()
{
}

// destructor
/*!

*/
AQLRiskConfigurationFXDeltaGamma::~AQLRiskConfigurationFXDeltaGamma(void)
{
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaGamma::getCoefficient1(const AQLString &fx) const
{
	AQLString bumpDirection = getPropBumpDirection(fx);
	bumpDirection.toUpper();
	const double divUnit = getDivUnit(fx);
	if (divUnit == 0.0)
	{
		throw AQLCoreInvalidData("Divid unit = 0 !!", __FILE__, __LINE__);
	}
	if (bumpDirection == RISK_BUMPDIRECTION_UPSHIFT)
	{
		const double val = 1.0 / divUnit;
		return AQLString(-val) + AQLString(":")
				+  AQLString(val) + AQLString(":0.0:0.0");
	}
	else if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
	{
		const double val = 1.0 / divUnit;
		return AQLString(val) + AQLString(":0.0:")
				+  AQLString(-val) + AQLString(":0.0");
	}
	else
	{
		const double val = 0.5 / divUnit;
		return AQLString("0.0:") + AQLString(val) + AQLString(":")
						+  AQLString(-val) + AQLString(":0.0");
	}
}

/*!
    @brief return operator2

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaGamma::getOperator2(void) const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return coefficient2

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaGamma::getCoefficient2(const AQLString &fx) const
{
	const double divUnit = getDivUnit(fx);
	const double val = 1.0 / AQLMath::pow(divUnit, 2.0);
	return AQLString(-2.0 * val)  + AQLString(":") + AQLString(val) + AQLString(":")
						+  AQLString(val) + AQLString(":0.0");
}

/*!
    @brief return outputname2

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaGamma::getOutPutName2(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
										FX_KEY_RISK_FRONT_FX_DELTA_OUTPUTNAME2);	
}

/*!
    @brief create scenario2 object 

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationFXDeltaGamma::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	vector<AQLObject *> ret = AQLRiskConfigurationFX::createFXEntity(fx, dataInstance, SCENARIO_2,index);

	// fx vol calibration has been replaced to extra scenario
	//if (isCalibTarget(fx))
	//{
	//	vector<AQLObject *> volVec = AQLRiskConfigurationFX::createFXVolEntity(fx, dataInstance, SCENARIO_2, index);
	//	ret.insert(ret.end(), volVec.begin(), volVec.end());
	//}
	return ret;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaGamma::getBumpDirection(const AQLString &ccy) const
{
	return RISK_BUMPDIRECTION_UPDOWNSHIFT;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaGamma::getPropBumpDirection(const AQLString &ccy) const
{
	return  AQLRiskConfigurationFXDeltaEx1::getBumpDirection(ccy);
}