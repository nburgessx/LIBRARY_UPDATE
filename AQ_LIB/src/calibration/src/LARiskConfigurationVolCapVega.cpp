/*! @file
    @brief Cap vega setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolCapVega.cpp
//
//  DESCRIPTION :        Cap vega setup class
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


#include "LARiskConfigurationVolCapVega.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataFunction.h"
#include "LAPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LADealUtils.h"
#include "LAStaticData.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationVolCapVega::LARiskConfigurationVolCapVega(void)
: LARiskConfigurationVolParallel(false)
{
}

// destructor
/*!

*/
LARiskConfigurationVolCapVega::~LARiskConfigurationVolCapVega(void)
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
LARiskConfigurationVolCapVega::createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return LARiskConfigurationVolParallel::createVolatilityEntity(ccy, dataInstance, scenarioNum, index);
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
LARiskConfigurationVolCapVega::createVolatilityEntityOld(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum) const
{
	if (scenarioNum == SCENARIO_1)
	{
		return LARiskConfigurationVolParallel::createVolatilityEntityOld(ccy, dataInstance, scenarioNum);
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
LARiskConfigurationVolCapVega::getOperator1(void) const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_CAPVEGA_OPERATOR);
}


/*!
    @brief return coefficient1

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationVolCapVega::getCoefficient1(const AQLString &ccy) const
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
LARiskConfigurationVolCapVega::getOutPutName1(const AQLString &ccy) const
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
LARiskConfigurationVolCapVega::getRiskName(void) const
{
	return RISK_OFFICIAL_VOL_CAPVEGA;
}

/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
LARiskConfigurationVolCapVega::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_CAPVEGA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
LARiskConfigurationVolCapVega::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(KEY_RISK_OFFICIAL_VOL_CAPVEGA_CALIBRATION_TARGET_CURRENCY);
}
