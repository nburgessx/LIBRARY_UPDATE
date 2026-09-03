/*! @file
    @brief FX delta setup class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationFXDelta.cpp
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


#include "AQLRiskConfigurationFXDelta.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationFXDelta::AQLRiskConfigurationFXDelta()
: AQLRiskConfigurationFX()
{
}

// destructor
/*!

*/
AQLRiskConfigurationFXDelta::~AQLRiskConfigurationFXDelta(void)
{
}

/*!
    @brief return operator1

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDelta::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_FX_DELTA_OPERATOR1);
}

/*!
    @brief return operator2

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDelta::getOperator2(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_FX_DELTA_OPERATOR2);
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDelta::getCoefficient1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_FX_DELTA_COEFFICIENT1);
}

/*!
    @brief return coefficient2

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDelta::getCoefficient2(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey = AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_FX_DELTA_COEFFICIENT2);
}

/*!
    @brief return outputname1

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDelta::getOutPutName1(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
							FX_KEY_RISK_OFFICIAL_FX_DELTA_OUTPUT1);	
}

/*!
    @brief return outputname2
	
	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDelta::getOutPutName2(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
							FX_KEY_RISK_OFFICIAL_FX_DELTA_OUTPUT2);	
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDelta::getRiskName(void) const
{
	return RISK_OFFICIAL_FX_DELTA;
}

/*!
    @brief return shift value

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationFXDelta::getShiftVal(const AQLString &fx, SCENARIONUM scenarioNum) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey = AQLMarketData::getFXKey(ccys[0], ccys[1]);

	if (scenarioNum == SCENARIO_1)
	{
		return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_OFFICIAL_FX_DELTA_SHIFT1).getDoubleValue();
	}
	else
	{
		return mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_OFFICIAL_FX_DELTA_SHIFT2).getDoubleValue();
	}
}

/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationFXDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_FX_DELTA_TARGET_FX);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationFXDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_FX_DELTA_CALIBRATION_TARGET_FX);
}


/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
AQLString
AQLRiskConfigurationFXDelta::getShiftType(const AQLString &fx) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_OFFICIAL_FX_DELTA_SHIFTTYPE);

}

/*!
    @brief return getCoefficientRatio1

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationFXDelta::getCoefficientRatio1(const AQLString &fx, AQLObject& e, AQLDataInstance& dataInstance) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	
	AQLString isadjstr = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_OFFICIAL_FX_DELTA_ISCOEFFICIENTRATIO1);
	if (isadjstr == AQ_NO_DATA)
		return 1.0;

	bool isadj = convertBoolFromStr(isadjstr);
	if (isadj)
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
		unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());

		double spot = targetFX->getRate(ccys[1], ccys[0], 0.0);
		if (spot == 0.0)
			throw AQLCoreInvalidData("Spot rate input error",__FILE__,__LINE__);

		AQLString shiftType = getShiftType(fx);
		shiftType.toUpper();

		double shiftval = getShiftVal(fx, SCENARIO_1);
		if (shiftType == RISK_SHIFTTYPE_RATIO)
		{
			shiftval /= 100.0;
		}
		if (shiftval == 0.0)
			throw AQLCoreInvalidData("Shift Value input error",__FILE__,__LINE__);

		return 1.0/spot/shiftval;
	}
	else
		return 1.0;

}

/*!
    @brief return getCoefficientRatio2

	@param[in] fx
	@return double
*/
double
AQLRiskConfigurationFXDelta::getCoefficientRatio2(const AQLString &fx, AQLObject& e, AQLDataInstance& dataInstance) const
{
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString fxKey =  AQLMarketData::getFXKey(ccys[0], ccys[1]);
	
	AQLString isadjstr = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_OFFICIAL_FX_DELTA_ISCOEFFICIENTRATIO2);
	if (isadjstr == AQ_NO_DATA)
		return 1.0;

	bool isadj = convertBoolFromStr(isadjstr);
	if (isadj)
	{
		double val = getCoefficientRatio1(fx,e,dataInstance);
		val *= val;
		
		AQLString isspotadjustmodestr = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_OFFICIAL_FX_DELTA_ISSPOTADJUSTMODEFORGAMMA);
		if (isspotadjustmodestr == AQ_NO_DATA)
			throw AQLCoreInvalidData("No input of IsSpotAdjustMode", __FILE__,__LINE__);

		bool isspotadj = convertBoolFromStr(isspotadjustmodestr);
		if (!isspotadj)
			return val;
		else
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
			unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());
			double spot = targetFX->getRate(ccys[1],ccys[0],0.0);
			val *= spot/100.0;
			return val;
		}
	}
	else 
		return 1.0;
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationFXDelta::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLString tmpCurrency = fx;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_OFFICIAL_FX_DELTA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}


/*!
    @brief create scenario2 object 

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationFXDelta::createScenario2Entity(const AQLString &fx, AQLDataInstance &dataInstance, int index)  const
{
	vector<AQLObject *> ret = createFXEntity(fx, dataInstance, SCENARIO_2,index);
	if (isCalibTarget(fx))
	{
		vector<AQLObject *> volVec = createFXVolEntity(fx, dataInstance, SCENARIO_2, index);
		ret.insert(ret.end(), volVec.begin(), volVec.end());
	}
	return ret;
}
