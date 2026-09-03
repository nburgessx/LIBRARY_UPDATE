#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LARiskConfigurationCreditSpreadDelta.h"
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

using namespace std;

// constructor
/*!

*/
LARiskConfigurationCreditSpreadDelta::LARiskConfigurationCreditSpreadDelta(void)
:LARiskConfiguration(false)
{
}

// destructor
/*!

*/
LARiskConfigurationCreditSpreadDelta::~LARiskConfigurationCreditSpreadDelta(void)
{
}

/*!
    @brief create risk object

	@param [in] objPool
	@return vector<pair<LAString, vector<LAObject *> > >
*/
vector<pair<LAString, vector<LAObject *> > >
LARiskConfigurationCreditSpreadDelta::createRiskEntity(LAObjectPool &objPool) const
{
	vector<pair<LAString, vector<LAObject *> > > ret;
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
	e->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
	vector<LAObject *> eVec(1, e);
	ret.push_back(make_pair(MLIB_NO_DATA, eVec));
	
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
LARiskConfigurationCreditSpreadDelta::setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const
{
	(void)ccy;
	//LADataInstance* dataInstance = e.getDataInstance();
	LAObjectPool& objPool = dataInstance.getObjectPool();

	const LAString portName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	
	LAObject& eport = objPool.getObject(portName,ENCHKTYPE_ISDEFINED).get();

	LADataHolder* dh;
	dh = &(eport.getData(CALIBRATION_DATA_UNDERLYINGS,ISNOTNULL));
	LADataMultiReference& traderef = dynamic_cast<LADataMultiReference &>(dh->get());

	bool isLookUpAttr = false;
	LAString isLookUpAttr_str = mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_ISLOOKUPFNDNGSPDATT);
	if (isLookUpAttr_str != MLIB_NO_DATA)
	{
		isLookUpAttr = convertBoolFromStr(isLookUpAttr_str);
	}

	unsigned int tradeSize = traderef.getSize();

	LAString ref;
	for (unsigned int i = 0; i < tradeSize; i++)
	{
		LAObject& etrade = traderef.get(i).get();

		dh = &(etrade.getData(PRICING_DATA_FUNDINGSPREADENTITY));
		// for JGB repackage
		if (!dh->isDefined() && isLookUpAttr) continue;
		// get funding spread object
		if (dh->isDefined() && !dh->isNull())
		{
			const LAObject& fndSpdEntity = dynamic_cast<LADataReference&>(dh->get()).get().get();
			const LAString& fndSpdEntityName = dynamic_cast<const LADataString &>(fndSpdEntity.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get());
			ref += fndSpdEntityName + ":";
		}

		//this is important flag : create isbuggegerecalc flga/////
		etrade.remove(PRICING_DATA_ISRECALCTRADEDATA);
		etrade.add(PRICING_DATA_ISRECALCTRADEDATA,new LADataBool(true));
		///////////////////////////////////////////////////////////

		dh = &(etrade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		LADataMultiReference& legref = dynamic_cast<LADataMultiReference &>(dh->get());
		
		if (legref.getSize() != 2)
			continue;
			//throw LACoreInvalidData("LegSize must be 2",__FILE__,__LINE__);
		LAObject& eleg = legref.get(1).get();

		dh = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference& cashref = dynamic_cast<LADataMultiReference &>(dh->get());
		unsigned int cashSize = cashref.getSize();

		for (unsigned int j = 0; j < cashSize; j++)
		{
			LAObject& ecash = cashref.get(j).get();
			
			//in case of notional cf or extracf as examples, we don't need couponinfos
			dh = &(ecash.getData(PRICING_DATA_COUPONINFOS));
			if (!dh->isDefined() || dh->isNull())
				continue;

			LADataMultiReference& couponref = dynamic_cast<LADataMultiReference &>(dh->get());

			if (couponref.getSize() != 1)
				throw LACoreInvalidData("Funding Coupon Size must be 1",__FILE__,__LINE__);
			LAObject& ecoupon = couponref.get(0).get();

			dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
			LADataMultiReference& indexref = dynamic_cast<LADataMultiReference &>(dh->get());

			if (indexref.getSize() != 1)
				throw LACoreInvalidData("Funding Index Size must be 1",__FILE__,__LINE__);
			LAObject& index = indexref.get(0).get();

			dh = &(index.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
			LAString indextype = dynamic_cast<LADataString &>(dh->get()).get();
			indextype.toUpper();

			if ("FIXEDRATE" == indextype)
			{
				//record a change libor
				bool isextra = false;
				dh = &(index.getData("IsExtraLibor"));
				if (dh->isDefined() && !dh->isNull())
					isextra = dynamic_cast<LADataBool &>(dh->get()).get();

				//for fixed rate generated by bond fixed type
				bool isbondfixed = false;
				dh = &(index.getData(PRICING_DATA_ISBONDFIXED));
				if (dh->isDefined() && !dh->isNull())
					isbondfixed = dynamic_cast<LADataBool &>(dh->get()).get();

				if (isextra || isbondfixed)
				{
					LAString name = dynamic_cast<LADataString &>(ecoupon.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
					ref += name + ":";
				}
				
				continue;
			}
			else if("LIBOR" == indextype)
			{
				LAString name = dynamic_cast<LADataString &>(ecoupon.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
				ref += name + ":";
			}
			else
			{
				throw LACoreInvalidData("Only FixedRate or Libor is supported",__FILE__,__LINE__);
			}
		}
	}

	LAString ret;
	if (ref.size() > 0)
	{
		ret = ref.subString(0, ref.size() - 2);
		e.remove(PRICING_DATA_TARGETNAMES);
		e.add(PRICING_DATA_TARGETNAMES, new LADataStrings()).convertFromString(ret);
	}
	else
	{
		//when target names does not exist, it means that all fixing are set. 
		//Therefore, we must calculate risk value is 0.
		
		LAString name("ForZeroCreditSpread");
		LAObjectHolder objHolder = objPool.getObject(name);
		if (!objHolder.isDefined())
		{
			LAObject* tmpe = new LAObject();
			tmpe->add(CALIBRATION_DATA_NAME,new LADataString(name));
			objPool.set(name, tmpe);
		}
		ret = name; 
		e.remove(PRICING_DATA_TARGETNAMES);
		e.add(PRICING_DATA_TARGETNAMES, new LADataStrings()).convertFromString(ret);

		//throw LACoreInvalidData("Credit Spread does not exist for this trade",__FILE__,__LINE__);
	}
	return;
	
	

}

 
/*!
    @brief create scenario1 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfigurationCreditSpreadDelta::createScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	return createCreditSpreadDeltaEntity(ccy, dataInstance, SCENARIO_1, index);
}

/*!
    @brief create scenario2 object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationCreditSpreadDelta::createScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	return createCreditSpreadDeltaEntity(ccy, dataInstance, SCENARIO_2, index);
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
LARiskConfigurationCreditSpreadDelta::createCreditSpreadDeltaEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	index;
	vector<LAObject *> ret;
	LAObjectPool& objPool = dataInstance.getObjectPool();
	LAString riskname = getRiskName();

	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}	

	LAObject& basee = objPool.getObject(riskname, ENCHKTYPE_ISDEFINED).get();
	LADataHolder* dh = &(basee.getData(PRICING_DATA_TARGETNAMES, ISNOTNULL));

	const LAStringVector& targetNames = dynamic_cast<const LADataStrings &>(dh->get()).get();
	ret.resize(targetNames.size());
	//when target names does not exist, it means that all fixing are set. 
		//Therefore, we must calculate risk value is 0.
	//LAString name("ForZeroCreditSpread");
	if (targetNames[0] == "ForZeroCreditSpread")
	{
		ret[0]  = &objPool.getObject(targetNames[0], ENCHKTYPE_ISDEFINED).get();
		return ret;
	}
	
	// get time ratio property
	bool isTimeRatio = true;
	LAString strIsTimeRatio = mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_SHIFTVAL_ISTIMERATIO);
	if (strIsTimeRatio != MLIB_NO_DATA)
	{
		isTimeRatio = convertBoolFromStr(strIsTimeRatio);
	}

	for (unsigned int i = 0; i < targetNames.size(); ++i)
	{
		LAObject& old_entity = objPool.getObject(targetNames[i], ENCHKTYPE_ISDEFINED).get();
		LAObject* new_entity = old_entity.clone();
		LAString new_name =  targetNames[i] + "_" + riskname + "_" + LAString(scenarioNum);
		new_entity->getData(CALIBRATION_DATA_NAME, ISNOTNULL).convertFromString(new_name);
		// change credit spread
		double shiftval = getShiftVal(ccy,scenarioNum);
		dh = &old_entity.getData(PRICING_DATA_FUNDINGSPREAD);
		if (dh->isDefined() && !dh->isNull())
		{
			// case of funding spread object
			double spread = dynamic_cast<LADataDouble &>(new_entity->getData(PRICING_DATA_FUNDINGSPREAD, ISNOTNULL).get()).get();
			dynamic_cast<LADataDouble &>(new_entity->getData(PRICING_DATA_FUNDINGSPREAD, ISNOTNULL).get()).set(spread + shiftval);
		}
		else
		{
			// case of coupon object
			// ratio check, get first index
			const LAObjectHolder &firstIndex = dynamic_cast<const LADataMultiReference &>(old_entity.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL).get()).get(0);
			const LADataHolder &ahRatio = firstIndex.getData("TermRatio", NOCHECK);
			if (isTimeRatio && ahRatio.isDefined() && !ahRatio.isNull())
			{
				const double ratio = dynamic_cast<const LADataDouble &>(ahRatio.get()).get();
				shiftval *= ratio;
			}
			
			dh = &(new_entity->getData(PRICING_DATA_COEFFICIENT,ISNOTNULL));
			DoubleVector coef = dynamic_cast<LADataDoubles &>(dh->get()).get();
			if (coef.size() != 2 /*|| coef[0] != 1.0 */)
				throw LACoreInvalidData("Error at Funding Coefficient",__FILE__,__LINE__);

			coef[1] += shiftval;
			dynamic_cast<LADataDoubles &>(dh->get()).set(coef);
		}

		objPool.set(new_name, new_entity);
		ret[i] = new_entity;
	}
	return ret;
}

/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationCreditSpreadDelta::getRiskName(void) const
{
	return RISK_OFFICIAL_YIELD_CREDITSPREADDELTA;
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationCreditSpreadDelta::getBumpDirection(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_BUMPDIRECTION);

}

/*!
    @brief return shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationCreditSpreadDelta::getShiftVal(const LAString &ccy, SCENARIONUM scenarioNum) const
{
	ccy;
	double shiftVal = mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_SHIFTVAL).getDoubleValue();
	//change bp val
	shiftVal /=10000.0;
	if (scenarioNum == SCENARIO_1)
	{
		LAString bumpDirection = getBumpDirection(ccy);
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
LARiskConfigurationCreditSpreadDelta::getDivUnit(const LAString &ccy) const
{	
	ccy;
	return mpRiskStaticData->getStaticData( 
								RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_DIVUNIT).getDoubleValue();
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationCreditSpreadDelta::getOutPutName1(const LAString &fx) const
{
	fx;
	return mpRiskStaticData->getStaticData(RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_OUTPUT);	
}
