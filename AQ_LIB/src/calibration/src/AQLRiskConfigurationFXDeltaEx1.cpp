/*! @file
    @brief FX delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationFXDeltaEx1.cpp
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


#include "AQLRiskConfigurationFXDeltaEx1.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLLinearFunc.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationFXDeltaEx1::AQLRiskConfigurationFXDeltaEx1()
: AQLRiskConfigurationFX()
{
}

// destructor
/*!

*/
AQLRiskConfigurationFXDeltaEx1::~AQLRiskConfigurationFXDeltaEx1(void)
{
}

/*!
    @brief return operator1

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getOperator1(void) const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return operator2

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getOperator2(void) const
{
	return AQ_NO_DATA;
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getCoefficient1(const AQLString &fx) const
{
	return AQLRiskConfiguration::getCoefficient1(fx);
}


/*!
    @brief return coefficient2

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getCoefficient2(const AQLString &fx) const
{
	return AQ_NO_DATA;
}


/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
										FX_KEY_RISK_FRONT_FX_DELTA_OUTPUTNAME);	
}

/*!
    @brief return outputname2
	
	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getOutPutName2(const AQLString &fx) const
{
	return AQ_NO_DATA;
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getRiskName(void) const
{
	return RISK_FRONT_FX_DELTA;
}

/*!
    @brief return bump direction

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getBumpDirection(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return AQLRiskConfigurationYield::getBumpDirection(fx);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_BUMPDIRECTION);

}

/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getShiftType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return AQLRiskConfigurationYield::getShiftType(fx);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_SHIFTTYPE);

}

/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_DELTA_TARGET_FX);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationFXDeltaEx1::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_DELTA_CALIBRATION_TARGET_FX);
}

/*!
    @brief return divid unit

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationFXDeltaEx1::getDivUnit(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return AQLRiskConfigurationYield::getDivUnit(fx);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString strDivUnit = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_DIVUNIT);
	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return shift value

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationFXDeltaEx1::getShiftVal(const AQLString &fx, SCENARIONUM scenarioNum) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey = AQLMarketData::getFXKey(ccys[0], ccys[1]);

	double shiftVal = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_SPOT_SHIFTVAL).getDoubleValue();
	if (scenarioNum == SCENARIO_1)
	{
		AQLString bumpDirection = getBumpDirection(fx);
		bumpDirection.toUpper();
		if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
		{
			shiftVal *= -1.0;		
		}
	}
	else if (scenarioNum == SCENARIO_2)
	{
		shiftVal *= -1.0;	
	}
	else
	{
		shiftVal *= 0.0;
	}
	return shiftVal;
}

/*!
    @brief return getCoefficientRatio1

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationFXDeltaEx1::getCoefficientRatio1(const AQLString &fx, AQLObject& e, AQLDataInstance& dataInstance) const
{
	const AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	const AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]).toLower();
	
	const AQLString isAdjustString = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_ISCOEFFICIENTRATIO);
	const bool isAdjust = (isAdjustString != AQ_NO_DATA) && convertBoolFromStr(isAdjustString);
	const AQLString shiftType = getShiftType(fx).toUpper();

	if ((shiftType == RISK_SHIFTTYPE_RATIO) && isAdjust)
	{
		// set target name
		AQLMathFXEntity *targetFX = AQLMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
		
		// search shift target currency(foreign currency)
		AQLStringVector ccys;
		AQLMarketData::convertToCurrency(fx, ccys);
		const AQLStringVector &fx_ccys = targetFX->getCurrencys().get();
		AQLStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
		if (it == fx_ccys.end())
		{
			AQLString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		const double spot = targetFX->getRate(ccys[1], ccys[0], 0.0);
		if (spot == 0.0)
		{
			throw AQLCoreInvalidData("Spot rate input error", __FILE__, __LINE__);
		}

		const double shiftval = getShiftVal(fx, SCENARIO_1) / 100.0;
		if (shiftval == 0.0)
		{
			throw AQLCoreInvalidData("Shift Value input error", __FILE__, __LINE__);
		}

		return 1.0/spot/shiftval;
	}
	else
	{
		return 1.0;
	}
}

/*!
    @brief return getCoefficientRatio2 (used for fx gamma with ratio bump)

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationFXDeltaEx1::getCoefficientRatio2(const AQLString &fx, AQLObject& e, AQLDataInstance& dataInstance) const
{
	const double val = getCoefficientRatio1(fx, e, dataInstance);
	return val * val; 
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] fx
	@return bool
*/
bool
AQLRiskConfigurationFXDeltaEx1::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return AQLRiskConfigurationYield::isRiskCurrencyMode(fx);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString proprslt = mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_FX_DELTA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}
