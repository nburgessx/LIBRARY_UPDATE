/*! @file
    @brief Cap vega setup class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRiskConfigurationVolCapVega.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLPricePortfolioValue.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"
#include "AQLDealUtils.h"
#include "AQLStaticData.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationVolCapVega::AQLRiskConfigurationVolCapVega(void)
: AQLRiskConfigurationVolParallel(false)
{
}

// destructor
/*!

*/
AQLRiskConfigurationVolCapVega::~AQLRiskConfigurationVolCapVega(void)
{
}


/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
AQLRiskConfigurationVolCapVega::createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return AQLRiskConfigurationVolParallel::createVolatilityEntity(ccy, dataInstance, scenarioNum, index);
	}
	else
	{
		return vector<AQLObject *>(0); 
	}
}


/*!
    @brief create volatility object

	@param[in] ccy
	@param[out] dataInstance
	@param[in] scenario
	@return vector<AQLObject *>
*/
vector<AQLObject *>
AQLRiskConfigurationVolCapVega::createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return AQLRiskConfigurationVolParallel::createVolatilityEntityOld(ccy, dataInstance, scenarioNum);
	}
	else
	{
		return vector<AQLObject *>(0); 
	}
}
/*!
    @brief return operator1

	@return AQLString
*/
AQLString
AQLRiskConfigurationVolCapVega::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_CAPVEGA_OPERATOR);
}


/*!
    @brief return coefficient1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolCapVega::getCoefficient1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_COEFFICIENT);
}


/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationVolCapVega::getOutPutName1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_OUTPUT);
}


/*!
    @brief return getriskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationVolCapVega::getRiskName(void) const
{
	return RISK_OFFICIAL_VOL_CAPVEGA;
}

/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationVolCapVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_CAPVEGA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationVolCapVega::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_CAPVEGA_CALIBRATION_TARGET_CURRENCY);
}
