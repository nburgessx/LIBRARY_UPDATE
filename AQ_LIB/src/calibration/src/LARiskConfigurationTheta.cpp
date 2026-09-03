#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LARiskConfigurationTheta.h"
#include "AQLDataInstance.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
#include "AQLPricePortfolioValue.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLMathDateCalculations.h"
#include "LADefinitionsRisk.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAMarketData.h"
#include "LALogger.h"
#include "LALogManager.h"
#include "LAStaticData.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "LADefinitionsCalibration.h"
#include "AQLPricePayOff.h"
#include "AQLPriceTradeValue.h"
#include "LADealUtils.h"
#include "LAObjectConfiguration.h"
#include "LAObjectConfigurationManager.h"
#include "AQLMathValuableEntity.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLPriceLSMCTradeValue.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationTheta::LARiskConfigurationTheta(void)
:LARiskConfiguration(false)
{
}

// destructor
/*!

*/
LARiskConfigurationTheta::~LARiskConfigurationTheta(void)
{
}

/*!
    @brief create risk object

	@param [in] objPool
	@return vector<pair<AQLString, vector<AQLObject *> > >
*/
vector<pair<AQLString, vector<AQLObject *> > >
LARiskConfigurationTheta::createRiskEntity(AQLObjectPool &objPool) const
{
	AQLStringVector ccys = MADealUtils::getSimulationSDECurrencys();
	vector<pair<AQLString, vector<AQLObject *> > > ret;
	const unsigned int ccyNum = ccys.size();

	// only for finding riskcurrency for using risk currency mode
	AQLString tmpstr("TMPCCY");
	bool isriskccymode = isRiskCurrencyMode(tmpstr);
	if (isriskccymode)
	{
		for (unsigned int i = 0; i < ccyNum; ++i)
		{
			if (!isTarget(ccys[i]))
			{
				continue;
			}
			if (ccys[i].findString(FX_DELIMITER) < 0)
			{
				continue;
			}
			
			//when no base shifts
			AQLString name = getRiskName();
			//AQLString name = ccys[i].toUpper() + "_" + getRiskName();
			AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
			AQLObject *e = 0;
			if (!objHolder.isDefined())
			{
				// create risk object
				e = new AQLObject();
			}
			else
			{
				e = &objHolder.get();
				e->reset();
			}
			e->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(name);
			vector<AQLObject *> eVec(1, e);
			ret.push_back(make_pair(ccys[i].toUpper(), eVec));

			//for yieldcredit spread delta we must set befor SetUpTargetNames
			objPool.set(name, e);
		}
	}
	else
	{
		//when no base shifts
		AQLString name = getRiskName();
		AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
		AQLObject *e = 0;
		if (!objHolder.isDefined())
		{
			// create risk object
			e = new AQLObject();
		}
		else
		{
			e = &objHolder.get();
			e->reset();
		}
		e->add(PRICING_DATA_ISSETUPPAYOFF, new AQLDataBool(true));

		e->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(name);
		vector<AQLObject *> eVec(1, e);
		ret.push_back(make_pair(AQ_NO_DATA, eVec));
		
		//for yieldcredit spread delta we must set befor SetUpTargetNames
		objPool.set(name, e);
	}
	return ret;
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfigurationTheta::setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const
{
	(void)ccy;
	//AQLDataInstance* dataInstance = e.getDataInstance();
	AQLObjectPool& objPool = dataInstance.getObjectPool();

	AQLString ref;

	AQLMathPathEntity* pPath = LAMarketData::getPathEnitty(objPool);
	if (pPath)
	{
		ref += pPath->getName().get() + ":";
		// get sde initial values
		const AQLDataMultiReference &initialValues = pPath->getInitialValues();
		for (unsigned int i = 0; i < initialValues.getSize(); ++i)
		{
			const AQLString &name = dynamic_cast<const AQLDataString &>(initialValues.get(i).getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			if (ref.findString(name) < 0)
			{
				ref += name + ":";
			}
		}
		// fx object (usemodel)
		if (MADealUtils::getSDECurrencys().size() > 1)
		{
			AQLMathFXEntity *pFX = LAMarketData::getFXEntity(objPool, "USEMODEL");
			if (!pFX)
			{
				throw AQLCoreInvalidData("USE Model FX Object is not set", __FILE__, __LINE__);
			}
			ref += dynamic_cast<const AQLDataString &>(pFX->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get() + ":";
		}
	}
	else
	{
		AQLMathPlainVanillaEntity* pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
		if (pvanilla == NULL)
			return;

		//plain vanilla
		ref += pvanilla->getName().get() + ":";

		AQLDataHolder* dh;
		if (MADealUtils::getSDECurrencys().size() > 1)
		{
			AQLObject& fxentity = pvanilla->getFXEntity().get().get();
			dh = &(fxentity.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
			//fx object
			ref += dynamic_cast<AQLDataString &>(dh->get()).get() + ":";
		}

		//yield object
		AQLDataMultiReference& curverefs = pvanilla->getIRCurves();
		for (unsigned int i = 0; i < curverefs.getSize(); i++)
		{
			AQLMathYieldCurve& curve = dynamic_cast<AQLMathYieldCurve &>(curverefs.get(i).get());
			AQLObject& ylddata = curve.getYieldData().get().get();

			dh = &(ylddata.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
			ref += dynamic_cast<AQLDataString &>(dh->get()).get() + ":";

		}
	}

	bool isplchange = isPLChangeMode(ccy);
	if (isplchange)
	{
		AQLDate bfasOf;
		if (pPath)
		{
			bfasOf = pPath->getAsOfDate();
		}
		else
		{
			AQLMathPlainVanillaEntity* pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
			if (!pvanilla)
			{
				throw AQLCoreInvalidData("Neither AQLMathPathEntity nor AQLMathPlainVanillaEntity exists.", __FILE__, __LINE__);
			}
			bfasOf = pvanilla->getAsOfDate();

		}
		int changeinterval = static_cast<int>(getShiftVal(ccy, SCENARIO_1));
		AQLString days = AQLString(changeinterval) + "D";
		// get cal and calc spot date
		AQLPriceDataCalendar cal;
		AQLString calStr = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_CALENDAR);
		if (calStr != AQ_NO_DATA)
			cal.convertFromString(calStr);
		
		AQLPriceDataSlidingRule fol(SLIDING_RULE_FOLLOWING);
		//fol.convertFromString(SLIDING_RULE_FOLLOWING);
		AQLDate afasOf = AQLMathDateCalculations::getDate(bfasOf,days,fol,&cal,true);



		//AQLDate afasOf = AQLMathDateCalculations::getDate(bfasOf,days,true);

		AQLString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
		AQLObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);
		if (dynamic_cast<const AQLDataValuation &>
		(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
		{
			// for portfolio
			const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
												(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
			const unsigned int tradeSize = unders.getSize();
			for (unsigned int i = 0; i < tradeSize; i++)
			{
				const AQLMathObjectValue& trade = dynamic_cast<const AQLMathObjectValue &>(unders.get(i).get());
				const AQLDataValuation& valuemehod = dynamic_cast<const AQLDataValuation &>
				(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

				if (valuemehod.isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) || valuemehod.isTypeOf(FN_IR_TRADEVALUE) || valuemehod.isTypeOf(FN_IR_LSMCTRADEVALUE))
				{
					const AQLDataHolder *dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						const AQLDataMultiReference &legs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
						unsigned int legSize = legs.getSize();
						for (unsigned int j = 0; j < legSize; j++)
						{
							AQLObject &eleg = legs.get(j).get();
							//get cashlet
							dh = &(eleg.getData(PRICING_DATA_CASHLETS, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								const AQLDataMultiReference &cashlets = dynamic_cast<const AQLDataMultiReference &>(dh->get());
								unsigned int cashletSize = cashlets.getSize();
								for (unsigned int k = 0; k < cashletSize; k++)
								{
									AQLObject &ecashlet = cashlets.get(k).get();
									dh = &(ecashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
									const AQLDate& paydate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
									//suppose payment date is refered order by paymentdate
									if (paydate > afasOf)
										break;
									
									if (paydate > bfasOf && paydate <= afasOf)
									{
										dh = &(ecashlet.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
										ref += dynamic_cast<const AQLDataString &>(dh->get()).get() + ":";
										continue;	
									}
								}
							}
						}
					}
				}
			}
		}
		else
		{
			const AQLMathObjectValue& trade = dynamic_cast<const AQLMathObjectValue &>(objHolder.get());
			const AQLDataValuation& valuemehod = dynamic_cast<const AQLDataValuation &>
			(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

			if (valuemehod.isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) || valuemehod.isTypeOf(FN_IR_TRADEVALUE) || valuemehod.isTypeOf(FN_IR_LSMCTRADEVALUE))
			{
				const AQLDataHolder *dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					const AQLDataMultiReference &legs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
					unsigned int legSize = legs.getSize();
					for (unsigned int j = 0; j < legSize; j++)
					{
						AQLObject &eleg = legs.get(j).get();
						//get cashlet
						dh = &(eleg.getData(PRICING_DATA_CASHLETS, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							const AQLDataMultiReference &cashlets = dynamic_cast<const AQLDataMultiReference &>(dh->get());
							unsigned int cashletSize = cashlets.getSize();
							for (unsigned int k = 0; k < cashletSize; k++)
							{
								AQLObject &ecashlet = cashlets.get(k).get();
								dh = &(ecashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
								const AQLDate& paydate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
								//suppose payment date is refered order by paymentdate
								if (paydate > afasOf)
									break;
								
								if (paydate > bfasOf && paydate <= afasOf)
								{
									dh = &(ecashlet.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
									ref += dynamic_cast<const AQLDataString &>(dh->get()).get() + ":";
									continue;	
								}
							}
						}
					}
				}
			}
		}
	}

	//AQLString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	//AQLObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);

	//AQLString calcpayoffstr = mpStaticData->getStaticData(KEY_DEAL_ISCALCPAYOFFAFTERMATURITY).toUpper();
	//bool iscalcpayoffaftermatu = (calcpayoffstr == "TRUE");

	//LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	//AQLString temp = staticData.getStaticData(KEY_SIMULATION_FUNDING_CHANGED);
 //     
	////set mLibor map for stub swap
	//setUpLiborRateMap(objPool);
	//
	//if (dynamic_cast<const AQLDataValuation &>
	//	(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	//{
	//	// for portfolio
	//	const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
	//										(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	//	const unsigned int tradeSize = unders.getSize();
	//	for (unsigned int i = 0; i < tradeSize; ++i)
	//	{
	//		setUpCallSpreadForDigitalOption(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
	//		//for stub swap
	//		setUpStubCoefficient(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));

	//		if (iscalcpayoffaftermatu)
	//		{
	//			unders.get(i).get().remove(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY);
	//			unders.get(i).get().add(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY, new AQLDataBool(iscalcpayoffaftermatu));
	//			
	//		}

	//		if (temp==AQ_NO_DATA)
	//		{
	//			setUpRecalcTrade(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
 //           }

	//		
	//	}
	//}






	AQLString ret;
	if (ref.size() > 0)
	{
		ret = ref.subString(0, ref.size() - 2);
		e.remove(PRICING_DATA_TARGETNAMES);
		e.add(PRICING_DATA_TARGETNAMES, new AQLDataStrings()).convertFromString(ret);
	}
	else
	{
		//throw AQLCoreInvalidData("Credit Spread does not exist for this trade",__FILE__,__LINE__);
	}


	return;


	

	


	//AQLObject& eport = objPool.getObject(portName,ENCHKTYPE_ISDEFINED).get();

	//AQLDataHolder* dh;
	//dh = &(eport.getData(CALIBRATION_DATA_UNDERLYINGS,ISNOTNULL));
	//AQLDataMultiReference& traderef = dynamic_cast<AQLDataMultiReference &>(dh->get());

	//unsigned int tradeSize = traderef.getSize();

	//AQLString ref;
	//for (unsigned int i = 0; i < tradeSize; i++)
	//{
	//	AQLObject& etrade = traderef.get(i).get();

	//	//this is important flag : create isbuggegerecalc flga/////
	//	etrade.remove(PRICING_DATA_ISRECALCTRADEDATA);
	//	etrade.add(PRICING_DATA_ISRECALCTRADEDATA,new AQLDataBool(true));
	//	///////////////////////////////////////////////////////////

	//	dh = &(etrade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	//	AQLDataMultiReference& legref = dynamic_cast<AQLDataMultiReference &>(dh->get());
	//	
	//	if (legref.getSize() != 2)
	//		throw AQLCoreInvalidData("LegSize must be 2",__FILE__,__LINE__);
	//	AQLObject& eleg = legref.get(1).get();

	//	dh = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	//	AQLDataMultiReference& cashref = dynamic_cast<AQLDataMultiReference &>(dh->get());
	//	unsigned int cashSize = cashref.getSize();

	//	for (unsigned int j = 0; j < cashSize; j++)
	//	{
	//		AQLObject& ecash = cashref.get(j).get();
	//		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
	//		AQLDataMultiReference& couponref = dynamic_cast<AQLDataMultiReference &>(dh->get());

	//		if (couponref.getSize() != 1)
	//			throw AQLCoreInvalidData("Funding Coupon Size must be 1",__FILE__,__LINE__);
	//		AQLObject& ecoupon = couponref.get(0).get();

	//		dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
	//		AQLDataMultiReference& indexref = dynamic_cast<AQLDataMultiReference &>(dh->get());

	//		if (indexref.getSize() != 1)
	//			throw AQLCoreInvalidData("Funding Index Size must be 1",__FILE__,__LINE__);
	//		AQLObject& index = indexref.get(0).get();

	//		dh = &(index.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
	//		AQLString indextype = dynamic_cast<AQLDataString &>(dh->get()).get();
	//		indextype.toUpper();

	//		if ("FIXEDRATE" == indextype)
	//		{
	//			//record a change libor
	//			bool isextra = false;
	//			dh = &(index.getData("IsExtraLibor"));
	//			if (dh->isDefined() && !dh->isNull())
	//				isextra = dynamic_cast<AQLDataBool &>(dh->get()).get();

	//			if (isextra)
	//			{
	//				AQLString name = dynamic_cast<AQLDataString &>(ecoupon.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	//				ref += name + ":";
	//			}
	//			
	//			continue;
	//		}
	//		else if("LIBOR" == indextype)
	//		{
	//			AQLString name = dynamic_cast<AQLDataString &>(ecoupon.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	//			ref += name + ":";
	//		}
	//		else
	//		{
	//			throw AQLCoreInvalidData("Only FixedRate or Libor is supported",__FILE__,__LINE__);
	//		}
	//	}
	//}

	//AQLString ret;
	//if (ref.size() > 0)
	//{
	//	ret = ref.subString(0, ref.size() - 2);
	//	e.remove(PRICING_DATA_TARGETNAMES);
	//	e.add(PRICING_DATA_TARGETNAMES, new AQLDataStrings()).convertFromString(ret);
	//}
	//else
	//{
	//	throw AQLCoreInvalidData("Credit Spread does not exist for this trade",__FILE__,__LINE__);
	//}
	//return;
	
	

}

 
/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
LARiskConfigurationTheta::createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	return createThetaEntity(ccy, dataInstance, SCENARIO_1, index);
}

/*!
    @brief create scenario2 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
LARiskConfigurationTheta::createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	ccy,index,dataInstance;
	return vector<AQLObject *>(0);
}

/*!
    @brief create yield object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
LARiskConfigurationTheta::createThetaEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	index;
	vector<AQLObject *> ret;
	AQLObjectPool& objPool = dataInstance.getObjectPool();
	AQLString riskname = getRiskName();

	AQLObject& basee = objPool.getObject(riskname, ENCHKTYPE_ISDEFINED).get();
	AQLDataHolder* dh = &(basee.getData(PRICING_DATA_TARGETNAMES, ISNOTNULL));

	AQLDate asOfDate;
	AQLMathPathEntity* pPath = LAMarketData::getPathEnitty(objPool);
	if (pPath)
	{
		asOfDate = pPath->getAsOfDate();
	}
	else
	{
	AQLMathPlainVanillaEntity* pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
	if (pvanilla == NULL)
	{
		return vector<AQLObject *>(0);
	}

		asOfDate = pvanilla->getAsOfDate();
	}

	int changeinterval = static_cast<int>(getShiftVal(ccy, SCENARIO_1));
	AQLPriceDataCalendar cal;
	AQLString calStr = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_CALENDAR);
	if (calStr != AQ_NO_DATA)
		cal.convertFromString(calStr);
	AQLPriceDataSlidingRule fol(SLIDING_RULE_FOLLOWING);
	AQLString days = AQLString(changeinterval) + "D";
	asOfDate = AQLMathDateCalculations::getDate(asOfDate,days,fol,&cal,true);

	AQLDate cashadjDate;
	if (isPLChangeMode(ccy))
	{
		AQLString days = "1D";
		cashadjDate = AQLMathDateCalculations::getDate(asOfDate,days,fol,&cal,true);
	}


	const AQLStringVector& targetNames = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
	ret.resize(targetNames.size());
	for (unsigned int i = 0; i < targetNames.size(); i++)
	{
		AQLString oldname = targetNames[i];
		AQLObject& oldcoupon = objPool.getObject(oldname, ENCHKTYPE_ISDEFINED).get();

		AQLObject* newcoupon = oldcoupon.clone();
		AQLString newname =  oldname + "_" + riskname + "_" + AQLString(scenarioNum);
		newcoupon->getData(CALIBRATION_DATA_NAME, ISNOTNULL).convertFromString(newname);
		objPool.set(newname, newcoupon);
		// setup path object
		if (newcoupon->isTypeOf(ENTITY_PATH))
		{
			AQLString mainModel = LAMarketData::getModelName(MADealUtils::getSDECurrencys()[0]);
			const LAObjectConfiguration *setUpper = LAObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
			setUpper->setUpPathEntity(objPool, &newname);
			delete setUpper;
		}		
		//change asofdate
		/*dh = &(newcoupon->getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL));
		dynamic_cast<AQLDataDate &>(dh->get()).set(asOfDate);*/
		dh = &(newcoupon->getData(CALIBRATION_DATA_ASOFDATE,NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			dynamic_cast<AQLDataDate &>(dh->get()).set(asOfDate);
		}
		else //plchange mode 
		{
			dh = &(newcoupon->getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
			dynamic_cast<AQLDataDate &>(dh->get()).set(cashadjDate);
		}

		ret[i] = newcoupon;
	}
	return ret;
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
LARiskConfigurationTheta::getRiskName(void) const
{
	return RISK_OFFICIAL_THETA;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationTheta::getBumpDirection(const AQLString &ccy) const
{
	(void)ccy;
	return RISK_BUMPDIRECTION_UPSHIFT;
}

/*!
    @brief return shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationTheta::getShiftVal(const AQLString &ccy, SCENARIONUM scenarioNum) const
{
	//theta shiftval is the length of time change
	(void)ccy;(void)scenarioNum;
	AQLString shiftval = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_SHIFTVAL);
	//change bp val
	return shiftval.getDoubleValue();
}

/*!
    @brief return div unit value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationTheta::getDivUnit(const AQLString &ccy) const
{	
	ccy;
	AQLString val = mpRiskStaticData->getStaticData( 
								RISK_OFFICIAL_THETA_DIVUNIT);
	return val.getDoubleValue();
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationTheta::getOutPutName1(const AQLString &fx) const
{
	fx;
	return mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_OUTPUTNAME);	
}

/*!
    @brief return isRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationTheta::isRiskCurrencyMode(const AQLString &fx) const
{
	AQLString tmpCurrency = fx;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);

}


/*!
    @brief return isPLChangeMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationTheta::isPLChangeMode(const AQLString &fx) const
{
	AQLString tmpCurrency = fx;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_WITHOUTCASH);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return (!convertBoolFromStr(proprslt));
}



