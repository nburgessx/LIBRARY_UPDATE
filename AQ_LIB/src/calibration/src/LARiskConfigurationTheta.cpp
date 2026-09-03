#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LARiskConfigurationTheta.h"
#include "LADataInstance.h"
#include "LAObjectPool.h"
#include "LADataReference.h"
#include "LAPricePortfolioValue.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LAMathDateCalculations.h"
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
#include "LAPricePayOff.h"
#include "LAPriceTradeValue.h"
#include "LADealUtils.h"
#include "LAObjectConfiguration.h"
#include "LAObjectConfigurationManager.h"
#include "LAMathValuableEntity.h"
#include "LALinearRatesSwapTradeValue.h"
#include "LAPriceLSMCTradeValue.h"

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
	@return vector<pair<LAString, vector<LAObject *> > >
*/
vector<pair<LAString, vector<LAObject *> > >
LARiskConfigurationTheta::createRiskEntity(LAObjectPool &objPool) const
{
	LAStringVector ccys = MADealUtils::getSimulationSDECurrencys();
	vector<pair<LAString, vector<LAObject *> > > ret;
	const unsigned int ccyNum = ccys.size();

	// only for finding riskcurrency for using risk currency mode
	LAString tmpstr("TMPCCY");
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
			LAString name = getRiskName();
			//LAString name = ccys[i].toUpper() + "_" + getRiskName();
			LAObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
			LAObject *e = 0;
			if (!objHolder.isDefined())
			{
				// create risk object
				e = new LAObject();
			}
			else
			{
				e = &objHolder.get();
				e->reset();
			}
			e->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
			vector<LAObject *> eVec(1, e);
			ret.push_back(make_pair(ccys[i].toUpper(), eVec));

			//for yieldcredit spread delta we must set befor SetUpTargetNames
			objPool.set(name, e);
		}
	}
	else
	{
		//when no base shifts
		LAString name = getRiskName();
		LAObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
		LAObject *e = 0;
		if (!objHolder.isDefined())
		{
			// create risk object
			e = new LAObject();
		}
		else
		{
			e = &objHolder.get();
			e->reset();
		}
		e->add(PRICING_DATA_ISSETUPPAYOFF, new LADataBool(true));

		e->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
		vector<LAObject *> eVec(1, e);
		ret.push_back(make_pair(MLIB_NO_DATA, eVec));
		
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
LARiskConfigurationTheta::setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const
{
	(void)ccy;
	//LADataInstance* dataInstance = e.getDataInstance();
	LAObjectPool& objPool = dataInstance.getObjectPool();

	LAString ref;

	LAMathPathEntity* pPath = LAMarketData::getPathEnitty(objPool);
	if (pPath)
	{
		ref += pPath->getName().get() + ":";
		// get sde initial values
		const LADataMultiReference &initialValues = pPath->getInitialValues();
		for (unsigned int i = 0; i < initialValues.getSize(); ++i)
		{
			const LAString &name = dynamic_cast<const LADataString &>(initialValues.get(i).getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			if (ref.findString(name) < 0)
			{
				ref += name + ":";
			}
		}
		// fx object (usemodel)
		if (MADealUtils::getSDECurrencys().size() > 1)
		{
			LAMathFXEntity *pFX = LAMarketData::getFXEntity(objPool, "USEMODEL");
			if (!pFX)
			{
				throw LACoreInvalidData("USE Model FX Object is not set", __FILE__, __LINE__);
			}
			ref += dynamic_cast<const LADataString &>(pFX->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get() + ":";
		}
	}
	else
	{
		LAMathPlainVanillaEntity* pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
		if (pvanilla == NULL)
			return;

		//plain vanilla
		ref += pvanilla->getName().get() + ":";

		LADataHolder* dh;
		if (MADealUtils::getSDECurrencys().size() > 1)
		{
			LAObject& fxentity = pvanilla->getFXEntity().get().get();
			dh = &(fxentity.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
			//fx object
			ref += dynamic_cast<LADataString &>(dh->get()).get() + ":";
		}

		//yield object
		LADataMultiReference& curverefs = pvanilla->getIRCurves();
		for (unsigned int i = 0; i < curverefs.getSize(); i++)
		{
			LAMathYieldCurve& curve = dynamic_cast<LAMathYieldCurve &>(curverefs.get(i).get());
			LAObject& ylddata = curve.getYieldData().get().get();

			dh = &(ylddata.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
			ref += dynamic_cast<LADataString &>(dh->get()).get() + ":";

		}
	}

	bool isplchange = isPLChangeMode(ccy);
	if (isplchange)
	{
		LADate bfasOf;
		if (pPath)
		{
			bfasOf = pPath->getAsOfDate();
		}
		else
		{
			LAMathPlainVanillaEntity* pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
			if (!pvanilla)
			{
				throw LACoreInvalidData("Neither LAMathPathEntity nor LAMathPlainVanillaEntity exists.", __FILE__, __LINE__);
			}
			bfasOf = pvanilla->getAsOfDate();

		}
		int changeinterval = static_cast<int>(getShiftVal(ccy, SCENARIO_1));
		LAString days = LAString(changeinterval) + "D";
		// get cal and calc spot date
		LAPriceDataCalendar cal;
		LAString calStr = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_CALENDAR);
		if (calStr != MLIB_NO_DATA)
			cal.convertFromString(calStr);
		
		LAPriceDataSlidingRule fol(SLIDING_RULE_FOLLOWING);
		//fol.convertFromString(SLIDING_RULE_FOLLOWING);
		LADate afasOf = LAMathDateCalculations::getDate(bfasOf,days,fol,&cal,true);



		//LADate afasOf = LAMathDateCalculations::getDate(bfasOf,days,true);

		LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
		LAObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);
		if (dynamic_cast<const LADataValuation &>
		(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
		{
			// for portfolio
			const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
												(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
			const unsigned int tradeSize = unders.getSize();
			for (unsigned int i = 0; i < tradeSize; i++)
			{
				const LAMathObjectValue& trade = dynamic_cast<const LAMathObjectValue &>(unders.get(i).get());
				const LADataValuation& valuemehod = dynamic_cast<const LADataValuation &>
				(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

				if (valuemehod.isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) || valuemehod.isTypeOf(FN_IR_TRADEVALUE) || valuemehod.isTypeOf(FN_IR_LSMCTRADEVALUE))
				{
					const LADataHolder *dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						const LADataMultiReference &legs = dynamic_cast<const LADataMultiReference &>(dh->get());
						unsigned int legSize = legs.getSize();
						for (unsigned int j = 0; j < legSize; j++)
						{
							LAObject &eleg = legs.get(j).get();
							//get cashlet
							dh = &(eleg.getData(PRICING_DATA_CASHLETS, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								const LADataMultiReference &cashlets = dynamic_cast<const LADataMultiReference &>(dh->get());
								unsigned int cashletSize = cashlets.getSize();
								for (unsigned int k = 0; k < cashletSize; k++)
								{
									LAObject &ecashlet = cashlets.get(k).get();
									dh = &(ecashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
									const LADate& paydate = dynamic_cast<const LADataDate &>(dh->get()).get();
									//suppose payment date is refered order by paymentdate
									if (paydate > afasOf)
										break;
									
									if (paydate > bfasOf && paydate <= afasOf)
									{
										dh = &(ecashlet.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
										ref += dynamic_cast<const LADataString &>(dh->get()).get() + ":";
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
			const LAMathObjectValue& trade = dynamic_cast<const LAMathObjectValue &>(objHolder.get());
			const LADataValuation& valuemehod = dynamic_cast<const LADataValuation &>
			(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

			if (valuemehod.isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) || valuemehod.isTypeOf(FN_IR_TRADEVALUE) || valuemehod.isTypeOf(FN_IR_LSMCTRADEVALUE))
			{
				const LADataHolder *dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					const LADataMultiReference &legs = dynamic_cast<const LADataMultiReference &>(dh->get());
					unsigned int legSize = legs.getSize();
					for (unsigned int j = 0; j < legSize; j++)
					{
						LAObject &eleg = legs.get(j).get();
						//get cashlet
						dh = &(eleg.getData(PRICING_DATA_CASHLETS, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							const LADataMultiReference &cashlets = dynamic_cast<const LADataMultiReference &>(dh->get());
							unsigned int cashletSize = cashlets.getSize();
							for (unsigned int k = 0; k < cashletSize; k++)
							{
								LAObject &ecashlet = cashlets.get(k).get();
								dh = &(ecashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
								const LADate& paydate = dynamic_cast<const LADataDate &>(dh->get()).get();
								//suppose payment date is refered order by paymentdate
								if (paydate > afasOf)
									break;
								
								if (paydate > bfasOf && paydate <= afasOf)
								{
									dh = &(ecashlet.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
									ref += dynamic_cast<const LADataString &>(dh->get()).get() + ":";
									continue;	
								}
							}
						}
					}
				}
			}
		}
	}

	//LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	//LAObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);

	//LAString calcpayoffstr = mpStaticData->getStaticData(KEY_DEAL_ISCALCPAYOFFAFTERMATURITY).toUpper();
	//bool iscalcpayoffaftermatu = (calcpayoffstr == "TRUE");

	//LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	//LAString temp = staticData.getStaticData(KEY_SIMULATION_FUNDING_CHANGED);
 //     
	////set mLibor map for stub swap
	//setUpLiborRateMap(objPool);
	//
	//if (dynamic_cast<const LADataValuation &>
	//	(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	//{
	//	// for portfolio
	//	const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
	//										(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	//	const unsigned int tradeSize = unders.getSize();
	//	for (unsigned int i = 0; i < tradeSize; ++i)
	//	{
	//		setUpCallSpreadForDigitalOption(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
	//		//for stub swap
	//		setUpStubCoefficient(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));

	//		if (iscalcpayoffaftermatu)
	//		{
	//			unders.get(i).get().remove(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY);
	//			unders.get(i).get().add(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY, new LADataBool(iscalcpayoffaftermatu));
	//			
	//		}

	//		if (temp==MLIB_NO_DATA)
	//		{
	//			setUpRecalcTrade(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
 //           }

	//		
	//	}
	//}






	LAString ret;
	if (ref.size() > 0)
	{
		ret = ref.subString(0, ref.size() - 2);
		e.remove(PRICING_DATA_TARGETNAMES);
		e.add(PRICING_DATA_TARGETNAMES, new LADataStrings()).convertFromString(ret);
	}
	else
	{
		//throw LACoreInvalidData("Credit Spread does not exist for this trade",__FILE__,__LINE__);
	}


	return;


	

	


	//LAObject& eport = objPool.getObject(portName,ENCHKTYPE_ISDEFINED).get();

	//LADataHolder* dh;
	//dh = &(eport.getData(CALIBRATION_DATA_UNDERLYINGS,ISNOTNULL));
	//LADataMultiReference& traderef = dynamic_cast<LADataMultiReference &>(dh->get());

	//unsigned int tradeSize = traderef.getSize();

	//LAString ref;
	//for (unsigned int i = 0; i < tradeSize; i++)
	//{
	//	LAObject& etrade = traderef.get(i).get();

	//	//this is important flag : create isbuggegerecalc flga/////
	//	etrade.remove(PRICING_DATA_ISRECALCTRADEDATA);
	//	etrade.add(PRICING_DATA_ISRECALCTRADEDATA,new LADataBool(true));
	//	///////////////////////////////////////////////////////////

	//	dh = &(etrade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	//	LADataMultiReference& legref = dynamic_cast<LADataMultiReference &>(dh->get());
	//	
	//	if (legref.getSize() != 2)
	//		throw LACoreInvalidData("LegSize must be 2",__FILE__,__LINE__);
	//	LAObject& eleg = legref.get(1).get();

	//	dh = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	//	LADataMultiReference& cashref = dynamic_cast<LADataMultiReference &>(dh->get());
	//	unsigned int cashSize = cashref.getSize();

	//	for (unsigned int j = 0; j < cashSize; j++)
	//	{
	//		LAObject& ecash = cashref.get(j).get();
	//		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
	//		LADataMultiReference& couponref = dynamic_cast<LADataMultiReference &>(dh->get());

	//		if (couponref.getSize() != 1)
	//			throw LACoreInvalidData("Funding Coupon Size must be 1",__FILE__,__LINE__);
	//		LAObject& ecoupon = couponref.get(0).get();

	//		dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
	//		LADataMultiReference& indexref = dynamic_cast<LADataMultiReference &>(dh->get());

	//		if (indexref.getSize() != 1)
	//			throw LACoreInvalidData("Funding Index Size must be 1",__FILE__,__LINE__);
	//		LAObject& index = indexref.get(0).get();

	//		dh = &(index.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
	//		LAString indextype = dynamic_cast<LADataString &>(dh->get()).get();
	//		indextype.toUpper();

	//		if ("FIXEDRATE" == indextype)
	//		{
	//			//record a change libor
	//			bool isextra = false;
	//			dh = &(index.getData("IsExtraLibor"));
	//			if (dh->isDefined() && !dh->isNull())
	//				isextra = dynamic_cast<LADataBool &>(dh->get()).get();

	//			if (isextra)
	//			{
	//				LAString name = dynamic_cast<LADataString &>(ecoupon.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	//				ref += name + ":";
	//			}
	//			
	//			continue;
	//		}
	//		else if("LIBOR" == indextype)
	//		{
	//			LAString name = dynamic_cast<LADataString &>(ecoupon.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	//			ref += name + ":";
	//		}
	//		else
	//		{
	//			throw LACoreInvalidData("Only FixedRate or Libor is supported",__FILE__,__LINE__);
	//		}
	//	}
	//}

	//LAString ret;
	//if (ref.size() > 0)
	//{
	//	ret = ref.subString(0, ref.size() - 2);
	//	e.remove(PRICING_DATA_TARGETNAMES);
	//	e.add(PRICING_DATA_TARGETNAMES, new LADataStrings()).convertFromString(ret);
	//}
	//else
	//{
	//	throw LACoreInvalidData("Credit Spread does not exist for this trade",__FILE__,__LINE__);
	//}
	//return;
	
	

}

 
/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationTheta::createScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	return createThetaEntity(ccy, dataInstance, SCENARIO_1, index);
}

/*!
    @brief create scenario2 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationTheta::createScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	ccy,index,dataInstance;
	return vector<LAObject *>(0);
}

/*!
    @brief create yield object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationTheta::createThetaEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	index;
	vector<LAObject *> ret;
	LAObjectPool& objPool = dataInstance.getObjectPool();
	LAString riskname = getRiskName();

	LAObject& basee = objPool.getObject(riskname, ENCHKTYPE_ISDEFINED).get();
	LADataHolder* dh = &(basee.getData(PRICING_DATA_TARGETNAMES, ISNOTNULL));

	LADate asOfDate;
	LAMathPathEntity* pPath = LAMarketData::getPathEnitty(objPool);
	if (pPath)
	{
		asOfDate = pPath->getAsOfDate();
	}
	else
	{
	LAMathPlainVanillaEntity* pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
	if (pvanilla == NULL)
	{
		return vector<LAObject *>(0);
	}

		asOfDate = pvanilla->getAsOfDate();
	}

	int changeinterval = static_cast<int>(getShiftVal(ccy, SCENARIO_1));
	LAPriceDataCalendar cal;
	LAString calStr = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_CALENDAR);
	if (calStr != MLIB_NO_DATA)
		cal.convertFromString(calStr);
	LAPriceDataSlidingRule fol(SLIDING_RULE_FOLLOWING);
	LAString days = LAString(changeinterval) + "D";
	asOfDate = LAMathDateCalculations::getDate(asOfDate,days,fol,&cal,true);

	LADate cashadjDate;
	if (isPLChangeMode(ccy))
	{
		LAString days = "1D";
		cashadjDate = LAMathDateCalculations::getDate(asOfDate,days,fol,&cal,true);
	}


	const LAStringVector& targetNames = dynamic_cast<const LADataStrings &>(dh->get()).get();
	ret.resize(targetNames.size());
	for (unsigned int i = 0; i < targetNames.size(); i++)
	{
		LAString oldname = targetNames[i];
		LAObject& oldcoupon = objPool.getObject(oldname, ENCHKTYPE_ISDEFINED).get();

		LAObject* newcoupon = oldcoupon.clone();
		LAString newname =  oldname + "_" + riskname + "_" + LAString(scenarioNum);
		newcoupon->getData(CALIBRATION_DATA_NAME, ISNOTNULL).convertFromString(newname);
		objPool.set(newname, newcoupon);
		// setup path object
		if (newcoupon->isTypeOf(ENTITY_PATH))
		{
			LAString mainModel = LAMarketData::getModelName(MADealUtils::getSDECurrencys()[0]);
			const LAObjectConfiguration *setUpper = LAObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
			setUpper->setUpPathEntity(objPool, &newname);
			delete setUpper;
		}		
		//change asofdate
		/*dh = &(newcoupon->getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL));
		dynamic_cast<LADataDate &>(dh->get()).set(asOfDate);*/
		dh = &(newcoupon->getData(CALIBRATION_DATA_ASOFDATE,NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			dynamic_cast<LADataDate &>(dh->get()).set(asOfDate);
		}
		else //plchange mode 
		{
			dh = &(newcoupon->getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
			dynamic_cast<LADataDate &>(dh->get()).set(cashadjDate);
		}

		ret[i] = newcoupon;
	}
	return ret;
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationTheta::getRiskName(void) const
{
	return RISK_OFFICIAL_THETA;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationTheta::getBumpDirection(const LAString &ccy) const
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
LARiskConfigurationTheta::getShiftVal(const LAString &ccy, SCENARIONUM scenarioNum) const
{
	//theta shiftval is the length of time change
	(void)ccy;(void)scenarioNum;
	LAString shiftval = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_SHIFTVAL);
	//change bp val
	return shiftval.getDoubleValue();
}

/*!
    @brief return div unit value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationTheta::getDivUnit(const LAString &ccy) const
{	
	ccy;
	LAString val = mpRiskStaticData->getStaticData( 
								RISK_OFFICIAL_THETA_DIVUNIT);
	return val.getDoubleValue();
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationTheta::getOutPutName1(const LAString &fx) const
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
LARiskConfigurationTheta::isRiskCurrencyMode(const LAString &fx) const
{
	LAString tmpCurrency = fx;
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);

}


/*!
    @brief return isPLChangeMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationTheta::isPLChangeMode(const LAString &fx) const
{
	LAString tmpCurrency = fx;
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(RISK_OFFICIAL_THETA_WITHOUTCASH);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return (!convertBoolFromStr(proprslt));
}



