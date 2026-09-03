#define _HAS_STD_BYTE 0

/*! @file
    @brief Hull-White sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelHW3F.cpp
//
//  DESCRIPTION :        Hull-White SDE generator 
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
//#define __DATAOUT__

#include "AQLDataInstance.h"
#include "LACalibrateModelHW3F.h"
#include "LAPriceDriftHWQuantAdjustment3F.h"
#include "LARatesHWIntegral3F.h"
#include "LARatesSpotSDEQuantAdjustment.h"
#include "LAStaticData.h"
#include "LADealUtils.h"
#include "LAMarketData.h"
#include "LAPriceQuantAdjustmentHWFXDD.h"
#include "LADefinitionsHW.h"
#include "LADefinitionsPtberg.h"
#include "AQLFunctionManager.h"


using namespace std;

// constructor
/*!
	@param[in] baseCurrency

*/
LACalibrateModelHW3F::LACalibrateModelHW3F(const AQLString &baseCurrency)
: LACalibrateModelHW(baseCurrency)
{
}

// destructor
/*!

*/
LACalibrateModelHW3F::~LACalibrateModelHW3F(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in]  currency
	@param[in]  dataInstance
*/
LARatesSDEBase *
LACalibrateModelHW3F::createSDEInstance(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	(void)dataInstance;
	AQLString key_ccy = currency;
	AQLString sdeName = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);

	LARatesSpotSDE *psde = 0;
	AQLString tmp_baseccy = mBaseCurrency;
	tmp_baseccy.toLower();

	SDE_TYPE type = getSDEType(currency);
	if (MADealUtils::getSDECurrencys().size() != 1)
	{
		// for cross cccy
		if (key_ccy != tmp_baseccy && !isZeroVol(key_ccy) && !isSZModel(currency))
		{
			// avoiding the error in getFXKey
			AQLString bccy = MADealUtils::getSDECurrencys()[0];
			if (bccy.toLower() != tmp_baseccy && bccy != key_ccy)
				tmp_baseccy = bccy;
			
			// get domestic(base) ir sde name
			AQLString sdeBase = mpStaticData->getStaticData(tmp_baseccy + STATIC_DATA_FX_KEY_SDE_NAME);

			// forein drift
			// get fx sde name
			AQLString key_fx = LAMarketData::getFXKey(tmp_baseccy, key_ccy);
			AQLString fx_sdeName = mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_NAME);
			const double irThreshold = LAMarketData::getStaticDataValue(*mpStaticData, key_ccy, KEY_HW_QUANTOADJUST_THRESHOLD).getDoubleValue();
			const double fxThreshold = LAMarketData::getStaticDataValue(*mpStaticData, key_fx, KEY_PTBERG_QUANTOADJUST_THRESHOLD).getDoubleValue();
			LAPriceQuantAdjustmentHWFXDD *quantAduster = new LAPriceQuantAdjustmentHWFXDD(sdeName, fx_sdeName, false, fxThreshold, irThreshold);
			const AQLString quantName = sdeName + "_QuantAdjustmentHWFXDD";
			dataInstance.getFunctionMaster().setFunction(quantAduster, quantName);

			psde =  new LARatesSpotSDEQuantAdjustment(type, quantAduster);
		}
		else
		{
			// domestic drift
			psde =  new LARatesSpotSDE(type);
		}
	}
	else
	{
		// check only one ccy
		if (key_ccy != tmp_baseccy)
		{
			throw AQLCoreInvalidData("Sigle currency support only one currency", __FILE__, __LINE__);
		}

		psde =  new LARatesSpotSDE(type);
	}

	return psde;
}


/*!
	@brief set integral function

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelHW3F::setIntegralFunction(const AQLString &currency, LARatesSDEBase &sde) const
{
	AQLString sdeName = getSDEAttrName(currency);
	sde.setIntegralFunction(new LARatesHWIntegral3F(LOG_INTEGRAL, sdeName));
}


// 
/*!
    @brief create createForeinDrift

	@param[in] fx
	@param[in] sdeBase
	@param[in] sdeName
	@param[in] fx_sdeName

	@return drift pointer
*/
AQLFunctionBase*
LACalibrateModelHW3F::createForeinDrift(const AQLString &fx, const AQLString &sdeBase, const AQLString &sdeName, const AQLString &fx_sdeName) const
{
	return new LAPriceDriftHWQuantAdjustment3F(sdeBase, sdeName, fx_sdeName, 
		new LAPriceDriftHW(sdeName), LAMarketData::getStaticDataValue(*mpStaticData, fx, KEY_PTBERG_QUANTOADJUST_THRESHOLD).getDoubleValue());
}
