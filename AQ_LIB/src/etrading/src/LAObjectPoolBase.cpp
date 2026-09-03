//
// LAObjectPoolBase.cpp
// This file used to be called Calibrator.cpp and before that LACalibrateModel.cpp
//

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAObjectPoolBase.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLDataReference.h"
#include "AQLRatesSDEBase.h"
#include "LADefinitions.h"
#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LADealUtils.h"
#include "AQLMathVolatility.h"
#include "LAMarketData.h"


// constructor
/*!

*/
LAObjectPoolBase::LAObjectPoolBase()
: mpStaticData(&LACoreDataService::getStaticDataManager().getStaticData())
{
}

// destructor
/*!

*/
LAObjectPoolBase::~LAObjectPoolBase(void)
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
LAObjectPoolBase::generateSDE(const AQLString &key, AQLDataInstance &dataInstance, bool isMarketCreate, bool isFirst) const
{
	if (isFirst)
	{
		AQLRatesSDEBase *psde = createSDEInstance(key, dataInstance);
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
		AQLString name = getFunctionMasterResistName(key);
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
LAObjectPoolBase::isFwdFXConst(const AQLString& ccy) const
{
	AQLString tmpCcy = ccy;
	AQLString str = mpStaticData->getStaticData(tmpCcy.toLower() + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST);
	return str.toUpper() == "TRUE";
}

// 
/*!
    @brief is collateral ccy or not

	@param[in]   key  currency or fx ex.JPY/USD
	@param[out]   is collateral ccy or not
*/
bool
LAObjectPoolBase::isCollateral(const AQLString& ccy) const
{
	AQLString tmpCcy = ccy;
	AQLString str = mpStaticData->getStaticData(KEY_YIELD_COLLATERAL_CCY);
	return str.toUpper() == tmpCcy.toUpper();
}

// 
/*!
    @brief generate sde volatility data

	@param[in]  key  currency or fx ex.JPY/USD
	@param[in]  dataInstance
*/
void
LAObjectPoolBase::loadVolatilityDataAndCalibrate(const AQLString &key, AQLDataInstance &dataInstance) const
{
	AQLString sdeName = getSDEAttrName(key);
	AQLString volType = getVolType(key);
	volType.toUpper();

	AQLString volName = PREFIX_VOL + sdeName;
	AQLObjectHolder objHolder = dataInstance.getObjectPool().getObject(volName);
	AQLMathVolatility *volEntity = 0;
	if (!objHolder.isDefined())
	{
		// create risk object
		volEntity = new AQLMathVolatility(&dataInstance);
		dataInstance.getObjectPool().set(volName, volEntity);	
	}
	else
	{
		volEntity = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
		volEntity->reset();
	}
	//AQLMathVolatility *volEntity = new AQLMathVolatility(&dataInstance);
	AQLDataString &volAttrName = volEntity->getName();
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
			throw AQLCoreInvalidData("Volatility input type. only function or data is support", __FILE__, __LINE__); 
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
LAObjectPoolBase::isCalibTarget(const AQLString &ccy) const
{
	if (!LAMarketData::isCalibrateModel(LAMarketData::getModelName(ccy)))
	{
		return false;
	}

	AQLString targetccys = mpStaticData->getStaticData(KEY_SDE_PV_CALIB_TARGET_CURRENCY).toUpper();
	if (targetccys == "ALL")
	{
		return true;
	}
	else
	{
		AQLString tmpCurrency = ccy;
		tmpCurrency.toUpper();
		AQLStringVector targetVec = targetccys.toToken(MULTI_STATIC_DATA_DELIMITER);
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
LAObjectPoolBase::isCancelForFunding(const AQLString &ccy) const
{

	AQLStringVector simCur = MADealUtils::getSimulationSDECurrencys();
	if (simCur.size() == 0)
		return false;

	bool ret = (simCur.end() == std::find(simCur.begin(),simCur.end(),ccy));
	return ret;
}
