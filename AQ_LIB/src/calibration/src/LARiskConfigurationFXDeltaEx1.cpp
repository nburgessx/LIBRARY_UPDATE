/*! @file
    @brief FX delta setup class
*/
//  2008, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXDeltaEx1.cpp
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


#include "LARiskConfigurationFXDeltaEx1.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LALinearFunc.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationFXDeltaEx1::LARiskConfigurationFXDeltaEx1()
: LARiskConfigurationFX()
{
}

// destructor
/*!

*/
LARiskConfigurationFXDeltaEx1::~LARiskConfigurationFXDeltaEx1(void)
{
}

/*!
    @brief return operator1

	@return LAString
*/
LAString
LARiskConfigurationFXDeltaEx1::getOperator1(void) const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return operator2

	@return LAString
*/
LAString
LARiskConfigurationFXDeltaEx1::getOperator2(void) const
{
	return MLIB_NO_DATA;
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaEx1::getCoefficient1(const LAString &fx) const
{
	return LARiskConfiguration::getCoefficient1(fx);
}


/*!
    @brief return coefficient2

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaEx1::getCoefficient2(const LAString &fx) const
{
	return MLIB_NO_DATA;
}


/*!
    @brief return outputname1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaEx1::getOutPutName1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
										FX_KEY_RISK_FRONT_FX_DELTA_OUTPUTNAME);	
}

/*!
    @brief return outputname2
	
	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaEx1::getOutPutName2(const LAString &fx) const
{
	return MLIB_NO_DATA;
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationFXDeltaEx1::getRiskName(void) const
{
	return RISK_FRONT_FX_DELTA;
}

/*!
    @brief return bump direction

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaEx1::getBumpDirection(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return LARiskConfigurationYield::getBumpDirection(fx);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_BUMPDIRECTION);

}

/*!
    @brief return shift type

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDeltaEx1::getShiftType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return LARiskConfigurationYield::getShiftType(fx);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_SHIFTTYPE);

}

/*!
    @brief return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationFXDeltaEx1::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_DELTA_TARGET_FX);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationFXDeltaEx1::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_FX_DELTA_CALIBRATION_TARGET_FX);
}

/*!
    @brief return divid unit

	@param[in] fx
	@return double
*/
double
LARiskConfigurationFXDeltaEx1::getDivUnit(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return LARiskConfigurationYield::getDivUnit(fx);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString strDivUnit = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_DIVUNIT);
	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return shift value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationFXDeltaEx1::getShiftVal(const LAString &fx, SCENARIONUM scenarioNum) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey = LAMarketData::getFXKey(ccys[0], ccys[1]);

	double shiftVal = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_SPOT_SHIFTVAL).getDoubleValue();
	if (scenarioNum == SCENARIO_1)
	{
		LAString bumpDirection = getBumpDirection(fx);
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
LARiskConfigurationFXDeltaEx1::getCoefficientRatio1(const LAString &fx, LAObject& e, LADataInstance& dataInstance) const
{
	const LAStringVector ccys = fx.toToken(FX_DELIMITER);
	const LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]).toLower();
	
	const LAString isAdjustString = mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_FRONT_FX_DELTA_ISCOEFFICIENTRATIO);
	const bool isAdjust = (isAdjustString != MLIB_NO_DATA) && convertBoolFromStr(isAdjustString);
	const LAString shiftType = getShiftType(fx).toUpper();

	if ((shiftType == RISK_SHIFTTYPE_RATIO) && isAdjust)
	{
		// set target name
		LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
		
		// search shift target currency(foreign currency)
		LAStringVector ccys;
		LAMarketData::convertToCurrency(fx, ccys);
		const LAStringVector &fx_ccys = targetFX->getCurrencys().get();
		LAStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
		if (it == fx_ccys.end())
		{
			LAString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		const double spot = targetFX->getRate(ccys[1], ccys[0], 0.0);
		if (spot == 0.0)
		{
			throw LACoreInvalidData("Spot rate input error", __FILE__, __LINE__);
		}

		const double shiftval = getShiftVal(fx, SCENARIO_1) / 100.0;
		if (shiftval == 0.0)
		{
			throw LACoreInvalidData("Shift Value input error", __FILE__, __LINE__);
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
LARiskConfigurationFXDeltaEx1::getCoefficientRatio2(const LAString &fx, LAObject& e, LADataInstance& dataInstance) const
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
LARiskConfigurationFXDeltaEx1::isRiskCurrencyMode(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	if (ccys.size() != 2) return LARiskConfigurationYield::isRiskCurrencyMode(fx);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString proprslt = mpRiskStaticData->getStaticData(fxKey + 
													FX_KEY_RISK_FRONT_FX_DELTA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}
