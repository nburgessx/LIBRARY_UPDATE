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


#include "LABasic.h"
#include "LARiskConfigurationFXDeltaGamma.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LALinearFunc.h"

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
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaGamma::getCoefficient1(const LAString &fx) const
{
	LAString bumpDirection = getPropBumpDirection(fx);
	bumpDirection.toUpper();
	const double divUnit = getDivUnit(fx);
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
    @brief return operator2

	@return LAString
*/
LAString
LARiskConfigurationFXDeltaGamma::getOperator2(void) const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return coefficient2

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaGamma::getCoefficient2(const LAString &fx) const
{
	const double divUnit = getDivUnit(fx);
	const double val = 1.0 / LAMath::pow(divUnit, 2.0);
	return LAString(-2.0 * val)  + LAString(":") + LAString(val) + LAString(":")
						+  LAString(val) + LAString(":0.0");
}

/*!
    @brief return outputname2

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaGamma::getOutPutName2(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
										FX_KEY_RISK_FRONT_FX_DELTA_OUTPUTNAME2);	
}

/*!
    @brief create scenario2 object 

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationFXDeltaGamma::createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	vector<LAObject *> ret = LARiskConfigurationFX::createFXEntity(fx, dataInstance, SCENARIO_2,index);

	// fx vol calibration has been replaced to extra scenario
	//if (isCalibTarget(fx))
	//{
	//	vector<LAObject *> volVec = LARiskConfigurationFX::createFXVolEntity(fx, dataInstance, SCENARIO_2, index);
	//	ret.insert(ret.end(), volVec.begin(), volVec.end());
	//}
	return ret;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaGamma::getBumpDirection(const LAString &ccy) const
{
	return RISK_BUMPDIRECTION_UPDOWNSHIFT;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaGamma::getPropBumpDirection(const LAString &ccy) const
{
	return  LARiskConfigurationFXDeltaEx1::getBumpDirection(ccy);
}