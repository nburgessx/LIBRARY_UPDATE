/*! @file
    @brief FX delta setup class
*/
//  2007, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXDelta.cpp
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


#include "LARiskConfigurationFXDelta.h"
#include "LAMarketData.h"
#include "LAStaticData.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationFXDelta::LARiskConfigurationFXDelta()
: LARiskConfigurationFX()
{
}

// destructor
/*!

*/
LARiskConfigurationFXDelta::~LARiskConfigurationFXDelta(void)
{
}

/*!
    @brief return operator1

	@return LAString
*/
LAString
LARiskConfigurationFXDelta::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_FX_DELTA_OPERATOR1);
}

/*!
    @brief return operator2

	@return LAString
*/
LAString
LARiskConfigurationFXDelta::getOperator2(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_FX_DELTA_OPERATOR2);
}

/*!
    @brief return coefficient1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDelta::getCoefficient1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_FX_DELTA_COEFFICIENT1);
}

/*!
    @brief return coefficient2

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDelta::getCoefficient2(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey = LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
								FX_KEY_RISK_OFFICIAL_FX_DELTA_COEFFICIENT2);
}

/*!
    @brief return outputname1

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDelta::getOutPutName1(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
							FX_KEY_RISK_OFFICIAL_FX_DELTA_OUTPUT1);	
}

/*!
    @brief return outputname2
	
	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDelta::getOutPutName2(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return mpRiskStaticData->getStaticData(fxKey.toLower() + 
							FX_KEY_RISK_OFFICIAL_FX_DELTA_OUTPUT2);	
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationFXDelta::getRiskName(void) const
{
	return RISK_OFFICIAL_FX_DELTA;
}

/*!
    @brief return shift value

	@param[in] fx
	@return double
*/
double
LARiskConfigurationFXDelta::getShiftVal(const LAString &fx, SCENARIONUM scenarioNum) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey = LAMarketData::getFXKey(ccys[0], ccys[1]);

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

	@return LAString 
*/
LAString
LARiskConfigurationFXDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_FX_DELTA_TARGET_FX);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationFXDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_FX_DELTA_CALIBRATION_TARGET_FX);
}


/*!
    @brief return shift type

	@param[in] fx
	@return LAString
*/
LAString
LARiskConfigurationFXDelta::getShiftType(const LAString &fx) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	return  mpRiskStaticData->getStaticData(fxKey + FX_KEY_RISK_OFFICIAL_FX_DELTA_SHIFTTYPE);

}

/*!
    @brief return getCoefficientRatio1

	@param[in] fx
	@return double
*/
double
LARiskConfigurationFXDelta::getCoefficientRatio1(const LAString &fx, LAObject& e, LADataInstance& dataInstance) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	
	LAString isadjstr = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_OFFICIAL_FX_DELTA_ISCOEFFICIENTRATIO1);
	if (isadjstr == MLIB_NO_DATA)
		return 1.0;

	bool isadj = convertBoolFromStr(isadjstr);
	if (isadj)
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
		unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());

		double spot = targetFX->getRate(ccys[1], ccys[0], 0.0);
		if (spot == 0.0)
			throw LACoreInvalidData("Spot rate input error",__FILE__,__LINE__);

		LAString shiftType = getShiftType(fx);
		shiftType.toUpper();

		double shiftval = getShiftVal(fx, SCENARIO_1);
		if (shiftType == RISK_SHIFTTYPE_RATIO)
		{
			shiftval /= 100.0;
		}
		if (shiftval == 0.0)
			throw LACoreInvalidData("Shift Value input error",__FILE__,__LINE__);

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
LARiskConfigurationFXDelta::getCoefficientRatio2(const LAString &fx, LAObject& e, LADataInstance& dataInstance) const
{
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	
	LAString isadjstr = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_OFFICIAL_FX_DELTA_ISCOEFFICIENTRATIO2);
	if (isadjstr == MLIB_NO_DATA)
		return 1.0;

	bool isadj = convertBoolFromStr(isadjstr);
	if (isadj)
	{
		double val = getCoefficientRatio1(fx,e,dataInstance);
		val *= val;
		
		LAString isspotadjustmodestr = mpRiskStaticData->getStaticData(fxKey.toLower() + 
													FX_KEY_RISK_OFFICIAL_FX_DELTA_ISSPOTADJUSTMODEFORGAMMA);
		if (isspotadjustmodestr == MLIB_NO_DATA)
			throw LACoreInvalidData("No input of IsSpotAdjustMode", __FILE__,__LINE__);

		bool isspotadj = convertBoolFromStr(isspotadjustmodestr);
		if (!isspotadj)
			return val;
		else
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
LARiskConfigurationFXDelta::isRiskCurrencyMode(const LAString &fx) const
{
	LAString tmpCurrency = fx;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													FX_KEY_RISK_OFFICIAL_FX_DELTA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}


/*!
    @brief create scenario2 object 

	@param[in] key fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationFXDelta::createScenario2Entity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	vector<LAObject *> ret = createFXEntity(fx, dataInstance, SCENARIO_2,index);
	if (isCalibTarget(fx))
	{
		vector<LAObject *> volVec = createFXVolEntity(fx, dataInstance, SCENARIO_2, index);
		ret.insert(ret.end(), volVec.begin(), volVec.end());
	}
	return ret;
}
