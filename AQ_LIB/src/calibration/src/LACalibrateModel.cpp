/*! @file
    @brief IR sde generator class
*/
//  2007, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModel.cpp
//
//  DESCRIPTION :        IR SDE generator 
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


#include "LAString.h"
#include "LACalibrateModel.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LADataReference.h"
#include "LARatesSDEBase.h"
#include "LADefinitions.h"
#include "LAMarketData.h"
#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LADealUtils.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS
#include "LAMathVolatility.h"
#endif


// constructor
/*!

*/
LACalibrateModel::LACalibrateModel()
: mpStaticData(&LACoreDataService::getStaticDataManager().getStaticData())
{
}

// destructor
/*!

*/
LACalibrateModel::~LACalibrateModel(void)
{
}


// 
/*!
    @brief generate sde

	@param[in]   key  currency or fx ex.JPY/USD
	@param[in]   dataInstance
	@param[in]   isMarketCreate market data create flag
	@param[in]   isFirst first execute flag
*/
void
LACalibrateModel::generateSDE(const LAString &key, LADataInstance &dataInstance, bool isMarketCreate, bool isFirst) const
{
	if (isFirst)
	{
		LARatesSDEBase *psde = createSDEInstance(key, dataInstance);
		// set vol
		setVolatility(key, *psde);
		// set drift
		setDrift(key, *psde);
		// set numeraire
		setNumeraire(key, *psde);
		// set integralfunction 
		setIntegralFunction(key, *psde);
		// set path
		setOutputTemplate(key, *psde);
		// set interpolation method
		setInterpolationMethod(key, *psde);

		// set function master
		LAString name = getFunctionMasterResistName(key);
		dataInstance.getFunctionMaster().setFunction(psde, name);
	}

	if (isMarketCreate)
	{
		// generate market data
		if (isCollateral(key))
		{
			if (isFirst)
				loadModelDataAndCalibrate(key, dataInstance, true, true);
		}
		else if (isFwdFXConst(key))
		{
			if (isFirst)
				loadModelDataAndCalibrate(key, dataInstance, true, false);
			else
				loadModelDataAndCalibrate(key, dataInstance, false, true);
		}
		else
		{
			if (!isFirst)
				loadModelDataAndCalibrate(key, dataInstance, true, true);
		}
	}
}

// 
/*!
    @brief is fwdfx constant curve or not

	@param[in]   key  currency or fx ex.JPY/USD
	@param[out]   is fwdfx constant curve or not
*/
bool
LACalibrateModel::isFwdFXConst(const LAString& ccy) const
{
	LAString tmpCcy = ccy;
	LAString str = mpStaticData->getStaticData(tmpCcy.toLower() + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST);
	return str.toUpper() == "TRUE";
}

// 
/*!
    @brief is collateral ccy or not

	@param[in]   key  currency or fx ex.JPY/USD
	@param[out]   is collateral ccy or not
*/
bool
LACalibrateModel::isCollateral(const LAString& ccy) const
{
	LAString tmpCcy = ccy;
	LAString str = mpStaticData->getStaticData(KEY_YIELD_COLLATERAL_CCY);
	return str.toUpper() == tmpCcy.toUpper();
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS
// 
/*!
    @brief generate sde volatility data

	@param[in]  key  currency or fx ex.JPY/USD
	@param[in]  dataInstance
*/
void
LACalibrateModel::loadVolatilityDataAndCalibrate(const LAString &key, LADataInstance &dataInstance) const
{
	LAString sdeName = getSDEAttrName(key);
	LAString volType = getVolType(key);
	volType.toUpper();

	LAString volName = PREFIX_VOL + sdeName;
	LAObjectHolder objHolder = dataInstance.getObjectPool().getObject(volName);
	LAMathVolatility *volEntity = 0;
	if (!objHolder.isDefined())
	{
		// create risk object
		volEntity = new LAMathVolatility(&dataInstance);
		dataInstance.getObjectPool().set(volName, volEntity);	
	}
	else
	{
		volEntity = &dynamic_cast<LAMathVolatility &>(objHolder.get());
		volEntity->reset();
	}
	//LAMathVolatility *volEntity = new LAMathVolatility(&dataInstance);
	LADataString &volAttrName = volEntity->getName();
	volAttrName.set(volName);

	if (!isCancelForFunding(key))
	{
	
		if (volType == INPUT_FUNC)
		{
			setUpVolFunc(key, *volEntity, dataInstance);
		}
		else if(volType == INPUT_DATA)
		{
			setUpVolData(key, *volEntity, dataInstance);
		}
		else
		{
			throw LACoreInvalidData("Volatility input type. only function or data is support", __FILE__, __LINE__); 
		}
	}

	// set data as reference
	dataInstance.getDataMaster().setData(volName, DATA_REFERENCE); 
	// set object pool
	//
	//if (dataInstance.getObjectPool().getObject(volName).isDefined())
	//{
	//	dataInstance.getObjectPool().remove(volName);
	//	//dataInstance.getObjectPool().getObject(volName).get().reset();
	//}
	//dataInstance.getObjectPool().set(volName, volEntity);	
}

/*!
    @brief return is calibration target

	@param[in] ccy
	@return bool 
*/
bool
LACalibrateModel::isCalibTarget(const LAString &ccy) const
{
	if (!LAMarketData::isCalibrateModel(LAMarketData::getModelName(ccy)))
	{
		return false;
	}

	LAString targetccys = mpStaticData->getStaticData(KEY_SDE_PV_CALIB_TARGET_CURRENCY).toUpper();
	if (targetccys == "ALL")
	{
		return true;
	}
	else
	{
		LAString tmpCurrency = ccy;
		tmpCurrency.toUpper();
		LAStringVector targetVec = targetccys.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (targetVec.end() != find(targetVec.begin(), targetVec.end(), tmpCurrency))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

/*!
    @brief return is cancel for funding 

	@param[in] ccy
	@return bool 
*/
bool
LACalibrateModel::isCancelForFunding(const LAString &ccy) const
{

	LAStringVector simCur = MADealUtils::getSimulationSDECurrencys(true);
	if (simCur.size() == 0)
		return false;

	bool ret = (simCur.end() == std::find(simCur.begin(),simCur.end(),ccy));
	return ret;
}
#endif
