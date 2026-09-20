#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLRiskConfigurationCreditSpreadDelta.h"
#include "AQLDataInstance.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
#include "AQLPricePortfolioValue.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLDateCalculations.h"
#include "AQLDefinitionsRisk.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLMarketData.h"
#include "AQLLogger.h"
#include "AQLLogManager.h"
#include "AQLStaticData.h"
#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLPricePayOff.h"
#include "AQLPriceTradeValue.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationCreditSpreadDelta::AQLRiskConfigurationCreditSpreadDelta(void)
:AQLRiskConfiguration(false)
{
}

// destructor
/*!

*/
AQLRiskConfigurationCreditSpreadDelta::~AQLRiskConfigurationCreditSpreadDelta(void)
{
}

/*!
    @brief create risk object

	@param [in] objPool
	@return vector<pair<AQLString, vector<AQLObject *> > >
*/
vector<pair<AQLString, vector<AQLObject *> > >
AQLRiskConfigurationCreditSpreadDelta::createRiskEntity(AQLObjectPool &objPool) const
{
	vector<pair<AQLString, vector<AQLObject *> > > ret;
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
	e->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(name);
	vector<AQLObject *> eVec(1, e);
	ret.push_back(make_pair(AQ_NO_DATA, eVec));
	
	//for yieldcredit spread delta we must set befor SetUpTargetNames
	objPool.set(name, e);
	
	return ret;
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
AQLRiskConfigurationCreditSpreadDelta::setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const
{
	(void)ccy;
	//AQLDataInstance* dataInstance = e.getDataInstance();
	AQLObjectPool& objPool = dataInstance.getObjectPool();

	const AQLString portName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
	
	AQLObject& eport = objPool.getObject(portName,ENCHKTYPE_ISDEFINED).get();

	AQLDataHolder* dh;
	dh = &(eport.getData(CALIBRATION_DATA_UNDERLYINGS,ISNOTNULL));
	AQLDataMultiReference& traderef = dynamic_cast<AQLDataMultiReference &>(dh->get());

	bool isLookUpAttr = false;
	AQLString isLookUpAttr_str = mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_ISLOOKUPFNDNGSPDATT);
	if (isLookUpAttr_str != AQ_NO_DATA)
	{
		isLookUpAttr = convertBoolFromStr(isLookUpAttr_str);
	}

	unsigned int tradeSize = traderef.getSize();

	AQLString ref;
	for (unsigned int i = 0; i < tradeSize; i++)
	{
		AQLObject& etrade = traderef.get(i).get();

		dh = &(etrade.getData(PRICING_DATA_FUNDINGSPREADENTITY));
		// for JGB repackage
		if (!dh->isDefined() && isLookUpAttr) continue;
		// get funding spread object
		if (dh->isDefined() && !dh->isNull())
		{
			const AQLObject& fndSpdEntity = dynamic_cast<AQLDataReference&>(dh->get()).get().get();
			const AQLString& fndSpdEntityName = dynamic_cast<const AQLDataString &>(fndSpdEntity.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get());
			ref += fndSpdEntityName + ":";
		}

		//this is important flag : create isbuggegerecalc flga/////
		etrade.remove(PRICING_DATA_ISRECALCTRADEDATA);
		etrade.add(PRICING_DATA_ISRECALCTRADEDATA,new AQLDataBool(true));
		///////////////////////////////////////////////////////////

		dh = &(etrade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		AQLDataMultiReference& legref = dynamic_cast<AQLDataMultiReference &>(dh->get());
		
		if (legref.getSize() != 2)
			continue;
			//throw AQLCoreInvalidData("LegSize must be 2",__FILE__,__LINE__);
		AQLObject& eleg = legref.get(1).get();

		dh = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		AQLDataMultiReference& cashref = dynamic_cast<AQLDataMultiReference &>(dh->get());
		unsigned int cashSize = cashref.getSize();

		for (unsigned int j = 0; j < cashSize; j++)
		{
			AQLObject& ecash = cashref.get(j).get();
			
			//in case of notional cf or extracf as examples, we don't need couponinfos
			dh = &(ecash.getData(PRICING_DATA_COUPONINFOS));
			if (!dh->isDefined() || dh->isNull())
				continue;

			AQLDataMultiReference& couponref = dynamic_cast<AQLDataMultiReference &>(dh->get());

			if (couponref.getSize() != 1)
				throw AQLCoreInvalidData("Funding Coupon Size must be 1",__FILE__,__LINE__);
			AQLObject& ecoupon = couponref.get(0).get();

			dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
			AQLDataMultiReference& indexref = dynamic_cast<AQLDataMultiReference &>(dh->get());

			if (indexref.getSize() != 1)
				throw AQLCoreInvalidData("Funding Index Size must be 1",__FILE__,__LINE__);
			AQLObject& index = indexref.get(0).get();

			dh = &(index.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
			AQLString indextype = dynamic_cast<AQLDataString &>(dh->get()).get();
			indextype.toUpper();

			if ("FIXEDRATE" == indextype)
			{
				//record a change libor
				bool isextra = false;
				dh = &(index.getData("IsExtraLibor"));
				if (dh->isDefined() && !dh->isNull())
					isextra = dynamic_cast<AQLDataBool &>(dh->get()).get();

				//for fixed rate generated by bond fixed type
				bool isbondfixed = false;
				dh = &(index.getData(PRICING_DATA_ISBONDFIXED));
				if (dh->isDefined() && !dh->isNull())
					isbondfixed = dynamic_cast<AQLDataBool &>(dh->get()).get();

				if (isextra || isbondfixed)
				{
					AQLString name = dynamic_cast<AQLDataString &>(ecoupon.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
					ref += name + ":";
				}
				
				continue;
			}
			else if("LIBOR" == indextype)
			{
				AQLString name = dynamic_cast<AQLDataString &>(ecoupon.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
				ref += name + ":";
			}
			else
			{
				throw AQLCoreInvalidData("Only FixedRate or Libor is supported",__FILE__,__LINE__);
			}
		}
	}

	AQLString ret;
	if (ref.size() > 0)
	{
		ret = ref.subString(0, ref.size() - 2);
		e.remove(PRICING_DATA_TARGETNAMES);
		e.add(PRICING_DATA_TARGETNAMES, new AQLDataStrings()).convertFromString(ret);
	}
	else
	{
		//when target names does not exist, it means that all fixing are set. 
		//Therefore, we must calculate risk value is 0.
		
		AQLString name("ForZeroCreditSpread");
		AQLObjectHolder objHolder = objPool.getObject(name);
		if (!objHolder.isDefined())
		{
			AQLObject* tmpe = new AQLObject();
			tmpe->add(CALIBRATION_DATA_NAME,new AQLDataString(name));
			objPool.set(name, tmpe);
		}
		ret = name; 
		e.remove(PRICING_DATA_TARGETNAMES);
		e.add(PRICING_DATA_TARGETNAMES, new AQLDataStrings()).convertFromString(ret);

		//throw AQLCoreInvalidData("Credit Spread does not exist for this trade",__FILE__,__LINE__);
	}
	return;
	
	

}

 
/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *>
*/
vector<AQLObject *> 
AQLRiskConfigurationCreditSpreadDelta::createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	return createCreditSpreadDeltaEntity(ccy, dataInstance, SCENARIO_1, index);
}

/*!
    @brief create scenario2 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
AQLRiskConfigurationCreditSpreadDelta::createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const
{
	return createCreditSpreadDeltaEntity(ccy, dataInstance, SCENARIO_2, index);
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
AQLRiskConfigurationCreditSpreadDelta::createCreditSpreadDeltaEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	index;
	vector<AQLObject *> ret;
	AQLObjectPool& objPool = dataInstance.getObjectPool();
	AQLString riskname = getRiskName();

	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}	

	AQLObject& basee = objPool.getObject(riskname, ENCHKTYPE_ISDEFINED).get();
	AQLDataHolder* dh = &(basee.getData(PRICING_DATA_TARGETNAMES, ISNOTNULL));

	const AQLStringVector& targetNames = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
	ret.resize(targetNames.size());
	//when target names does not exist, it means that all fixing are set. 
		//Therefore, we must calculate risk value is 0.
	//AQLString name("ForZeroCreditSpread");
	if (targetNames[0] == "ForZeroCreditSpread")
	{
		ret[0]  = &objPool.getObject(targetNames[0], ENCHKTYPE_ISDEFINED).get();
		return ret;
	}
	
	// get time ratio property
	bool isTimeRatio = true;
	AQLString strIsTimeRatio = mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_SHIFTVAL_ISTIMERATIO);
	if (strIsTimeRatio != AQ_NO_DATA)
	{
		isTimeRatio = convertBoolFromStr(strIsTimeRatio);
	}

	for (unsigned int i = 0; i < targetNames.size(); ++i)
	{
		AQLObject& old_entity = objPool.getObject(targetNames[i], ENCHKTYPE_ISDEFINED).get();
		AQLObject* new_entity = old_entity.clone();
		AQLString new_name =  targetNames[i] + "_" + riskname + "_" + AQLString(scenarioNum);
		new_entity->getData(CALIBRATION_DATA_NAME, ISNOTNULL).convertFromString(new_name);
		// change credit spread
		double shiftval = getShiftVal(ccy,scenarioNum);
		dh = &old_entity.getData(PRICING_DATA_FUNDINGSPREAD);
		if (dh->isDefined() && !dh->isNull())
		{
			// case of funding spread object
			double spread = dynamic_cast<AQLDataDouble &>(new_entity->getData(PRICING_DATA_FUNDINGSPREAD, ISNOTNULL).get()).get();
			dynamic_cast<AQLDataDouble &>(new_entity->getData(PRICING_DATA_FUNDINGSPREAD, ISNOTNULL).get()).set(spread + shiftval);
		}
		else
		{
			// case of coupon object
			// ratio check, get first index
			const AQLObjectHolder &firstIndex = dynamic_cast<const AQLDataMultiReference &>(old_entity.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL).get()).get(0);
			const AQLDataHolder &ahRatio = firstIndex.getData("TermRatio", NOCHECK);
			if (isTimeRatio && ahRatio.isDefined() && !ahRatio.isNull())
			{
				const double ratio = dynamic_cast<const AQLDataDouble &>(ahRatio.get()).get();
				shiftval *= ratio;
			}
			
			dh = &(new_entity->getData(PRICING_DATA_COEFFICIENT,ISNOTNULL));
			DoubleVector coef = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
			if (coef.size() != 2 /*|| coef[0] != 1.0 */)
				throw AQLCoreInvalidData("Error at Funding Coefficient",__FILE__,__LINE__);

			coef[1] += shiftval;
			dynamic_cast<AQLDataDoubles &>(dh->get()).set(coef);
		}

		objPool.set(new_name, new_entity);
		ret[i] = new_entity;
	}
	return ret;
}

/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationCreditSpreadDelta::getRiskName(void) const
{
	return RISK_OFFICIAL_YIELD_CREDITSPREADDELTA;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationCreditSpreadDelta::getBumpDirection(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_BUMPDIRECTION);

}

/*!
    @brief return shift value

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationCreditSpreadDelta::getShiftVal(const AQLString &ccy, SCENARIONUM scenarioNum) const
{
	ccy;
	double shiftVal = mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_SHIFTVAL).getDoubleValue();
	//change bp val
	shiftVal /=10000.0;
	if (scenarioNum == SCENARIO_1)
	{
		AQLString bumpDirection = getBumpDirection(ccy);
		bumpDirection.toUpper();
		if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
		{
			shiftVal *= -1;		
		}
	}
	else
	{
		shiftVal *= -1.0;	
	}
	return shiftVal;
}

/*!
    @brief return div unit value

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationCreditSpreadDelta::getDivUnit(const AQLString &ccy) const
{	
	ccy;
	return mpRiskStaticData->getStaticData( 
								RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_DIVUNIT).getDoubleValue();
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationCreditSpreadDelta::getOutPutName1(const AQLString &fx) const
{
	fx;
	return mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_OUTPUT);	
}
