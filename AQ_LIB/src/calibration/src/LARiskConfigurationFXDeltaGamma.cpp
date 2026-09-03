/*! @file
    @brief FX delta gamma setup class
*/
//  2016, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXDeltaGamma.cpp
//
//  DESCRIPTION :        FX delta gamma setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBasic.h"
#include "LARiskConfigurationFXDeltaGamma.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "AQLLinearFunc.h"

#include "LAFileAccessor.h"
#include "LADealUtils.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationFXDeltaGamma::LARiskConfigurationFXDeltaGamma()
: LARiskConfigurationFXDeltaEx1()
{
}

// destructor
/*!

*/
LARiskConfigurationFXDeltaGamma::~LARiskConfigurationFXDeltaGamma(void)
{
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationFXDeltaGamma::getCoefficient1(const AQLString &fx) const
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
LARiskConfigurationFXDeltaGamma::getOperator2(void) const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return coefficient2

	@param[in] fx
	@return AQLString
*/
AQLString
LARiskConfigurationFXDeltaGamma::getCoefficient2(const AQLString &fx) const
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
LARiskConfigurationFXDeltaGamma::getOutPutName2(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
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
LARiskConfigurationFXDeltaGamma::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	vector<AQLObject *> ret = LARiskConfigurationFX::createFXEntity(fx, dataInstance, SCENARIO_2,index);

	// fx vol calibration has been replaced to extra scenario
	//if (isCalibTarget(fx))
	//{
	//	vector<AQLObject *> volVec = LARiskConfigurationFX::createFXVolEntity(fx, dataInstance, SCENARIO_2, index);
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
LARiskConfigurationFXDeltaGamma::getBumpDirection(const AQLString &ccy) const
{
	return RISK_BUMPDIRECTION_UPDOWNSHIFT;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationFXDeltaGamma::getPropBumpDirection(const AQLString &ccy) const
{
	return  LARiskConfigurationFXDeltaEx1::getBumpDirection(ccy);
}