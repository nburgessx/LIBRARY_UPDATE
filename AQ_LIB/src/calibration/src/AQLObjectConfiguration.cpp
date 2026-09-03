
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCoreGridConfiguation.h"
#include "AQLObjectConfiguration.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLObjectPool.h"
#include "AQLObjectHolder.h"
#include "AQLRatesSDEBase.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLDataProcedure.h"
#include "AQLPriceTradeValue.h"
#include "AQLPriceLSMCTradeValue.h"
#include "AQLPriceCashFlowGenerator.h"
#include "AQLMathPathEntity.h"
#include "AQLMathFXEntity.h"
#include "AQLMathAttrSDE.h"
#include "AQLMathIndexEntity.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLMathValuableEntity.h"
#include "AQLPriceDataRand.h"
#include "AQLPricePortfolioValue.h"
#include "AQLDefinitions.h"
#include "AQLCoreDataService.h"
#include "AQLStaticData.h"
#include "AQLMarketData.h"
#include "AQLDealUtils.h"
#include "AQLSobol.h"
#include "AQLPriceDataManager.h"
#include "AQLLinearRatesOptionValue.h"
#include "AQLLinearRatesOptionValueDataProvider.h"
#include "AQLPriceDataFunction.h"
#include "AQLPriceCouponForDigital2.h"
#include "AQLCalibrateModel.h"
#include "AQLModelConfiguration.h"


#define CPN		"CPN"
#define CPNCF	"CPNCF"
#define LIBOR	"LIBOR"

#define SWAP "SWAP"

#define DF2 "DF2"


using namespace std;
// constructor
/*!

*/
AQLObjectConfiguration::AQLObjectConfiguration()
: mpStaticData(&AQLCoreDataService::getStaticDataManager().getStaticData())
{
}

// destructor
/*!

*/
AQLObjectConfiguration::~AQLObjectConfiguration(void)
{
}


// 
/*!
    @brief setup object

	@param[out] dataInstance
*/
void
AQLObjectConfiguration::setUpEntityes(AQLDataInstance &dataInstance) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	// setup valuable object
	setUpValuableEntity(objPool);
	// setup path object
	//setUpPathEntity(objPool);
	// setup fx object
	//setUpFXEntity(objPool);

	// setup Future Fixing object
    AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
    AQLString isFixing = staticData.getStaticData(KEY_DEAL_ISFIXINGRATE);
	if (isFixing.toUpper() == "TRUE")
	{
		setUpFutureFixingEntity(objPool);
	}

	// setup extra object
	setUpExtraEntity(objPool);
}

// 
/*!
    @brief setup valuable object

	@param[out] object pool
*/
void
AQLObjectConfiguration::setUpValuableEntity(AQLObjectPool &objPool) const
{
	AQLString mTradeName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
	AQLObjectHolder objHolder = objPool.getObject(mTradeName, ENCHKTYPE_ISDEFINED);

	AQLString mcNum = mpStaticData->getStaticData(KEY_SIMULATION_MCNUM);
	AQLString isDetail =  mpStaticData->getStaticData(KEY_SIMULATION_ISDETAILOUTPUT);

    AQLString zeroCalc = AQLCoreDataService::getContext(ARG_KEY_ZEROCALC);
    if (zeroCalc == AQ_NO_DATA) {
        zeroCalc = "FALSE";
    }

	if (dynamic_cast<const AQLDataValuation &>
		(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)

	{
		//for risk currency mode
		AQLString fxName = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
		if (fxName != AQ_NO_DATA)
		{
			objHolder.get().remove(PRICING_DATA_FXRATE);
			objHolder.get().add(PRICING_DATA_FXRATE, new AQLDataReference()).convertFromString(fxName);
		}

		const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());

		const unsigned int size = unders.getSize();
		for (unsigned int i = 0; i < size; ++i)
		{
			AQLObjectHolder &trade = unders.get(i);
			// set is detailouput
			trade.remove(PRICING_DATA_ISDETAILOUTPUT);
			trade.add(PRICING_DATA_ISDETAILOUTPUT, new AQLDataBool()).convertFromString(isDetail);
		    // set mc
			trade.remove(PRICING_DATA_MCNUM);
			trade.add(PRICING_DATA_MCNUM, new AQLDataInt()).convertFromString(mcNum);
			// set lsmc if defined
			if (dynamic_cast<const AQLDataValuation &>
				(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_LSMCTRADEVALUE)
			{
				AQLString lsmcNum = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_MCNUM);
				trade.remove(PRICING_DATA_LSMCNUM);
				trade.add(PRICING_DATA_LSMCNUM, new AQLDataInt()).convertFromString(lsmcNum);
			}
		
			// ! Set isResultOut
			if ( AQLCoreDataService::getContext( ARG_KEY_RESULTOUT ) != AQ_NO_DATA )
			{
				trade.remove( PRICING_DATA_ISRESULTOUTPUT );
				trade.add( PRICING_DATA_ISRESULTOUTPUT, new AQLDataBool( true ) );
			}

			if(AQLCoreDataService::getContext(ARG_KEY_COMPOUNDEDRATEOUT) != AQ_NO_DATA)
			{
				trade.remove(PRICING_DATA_COMPOUNDED_RATE_OUT);
				trade.add(PRICING_DATA_COMPOUNDED_RATE_OUT, new AQLDataBool(true));
			}
			

            // set no calculation
            trade.remove(PRICING_DATA_ZEROCALC);
            trade.add(PRICING_DATA_ZEROCALC, new AQLDataBool()).convertFromString(zeroCalc);

			//for past libor auto fixing
			trade.remove(PRICING_DATA_ISSAVEPASTFIXING);
			trade.add(PRICING_DATA_ISSAVEPASTFIXING, new AQLDataBool(true));

			// set RangeAccrue fixed rates
			AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);
			AQLObjectConfiguration::setUpRangeAccrueFixedRates(asOfDate, trade);
			AQLObjectConfiguration::setUpCompoundingFixedRates(asOfDate, trade);
		}
		
		AQLDataInstance *dataInstance = objHolder.getDataInstance();
		dataInstance->getReferencePool().completeDependency();
	}
	else
	{

		// set is detailouput
		objHolder.remove(PRICING_DATA_ISDETAILOUTPUT);
		objHolder.add(PRICING_DATA_ISDETAILOUTPUT, new AQLDataBool()).convertFromString(isDetail);
		// set mc num
		objHolder.remove(PRICING_DATA_MCNUM);
		objHolder.add(PRICING_DATA_MCNUM, new AQLDataInt()).convertFromString(mcNum);		
		// set lsmc if defined
		if (dynamic_cast<const AQLDataValuation &>
			(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_LSMCTRADEVALUE)
		{
			AQLString lsmcNum = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_MCNUM);
			objHolder.remove(PRICING_DATA_LSMCNUM);
			objHolder.add(PRICING_DATA_LSMCNUM, new AQLDataInt()).convertFromString(lsmcNum);
		}

		// ! Set isResultOut
		if ( AQLCoreDataService::getContext( ARG_KEY_RESULTOUT ) != AQ_NO_DATA )
		{
			objHolder.remove( PRICING_DATA_ISRESULTOUTPUT );
			objHolder.add( PRICING_DATA_ISRESULTOUTPUT, new AQLDataBool( true ) );
		}

        if(AQLCoreDataService::getContext(ARG_KEY_COMPOUNDEDRATEOUT) != AQ_NO_DATA)
        {
            objHolder.remove(PRICING_DATA_COMPOUNDED_RATE_OUT);
            objHolder.add(PRICING_DATA_COMPOUNDED_RATE_OUT, new AQLDataBool(true));
        }
		

		// set no calculation
		objHolder.remove(PRICING_DATA_ZEROCALC);
		objHolder.add(PRICING_DATA_ZEROCALC, new AQLDataBool()).convertFromString(zeroCalc);

		//for past libor auto fixing
		objHolder.remove(PRICING_DATA_ISSAVEPASTFIXING);
		objHolder.add(PRICING_DATA_ISSAVEPASTFIXING, new AQLDataBool(true));

		// set RangeAccrue fixed rates
		AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);
		AQLObjectConfiguration::setUpRangeAccrueFixedRates(asOfDate, objHolder);
		AQLObjectConfiguration::setUpCompoundingFixedRates(asOfDate, objHolder);
	}
}


// 
/*!
    @brief setup risk info object

	@param[out] AQLDataInstance dataInstance
*/
void
AQLObjectConfiguration::setUpRiskInfo(AQLDataInstance &dataInstance) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString mTradeName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
	AQLObjectHolder objHolder = objPool.getObject(mTradeName, ENCHKTYPE_ISDEFINED);

	const AQLString riskENames_str = AQLCoreDataService::getContext(CONTEXT_KEY_RISKENTITY_NAMES);
	const AQLStringVector riskENames = riskENames_str.toToken(':');
	vector<const AQLObject *> riskEVec;
	for (unsigned int i = 0; i < riskENames.size(); ++i)
	{
		if (riskENames[i] != AQ_NO_DATA)
		{
			riskEVec.push_back(&objPool.getObject(riskENames[i], ENCHKTYPE_ISDEFINED).get());
		}
	}

	if (dynamic_cast<const AQLDataValuation &>
		(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)

	{
		//for risk currency mode
		AQLString fxName = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
		if (fxName != AQ_NO_DATA)
		{
			objHolder.get().remove(PRICING_DATA_FXRATE);
			objHolder.get().add(PRICING_DATA_FXRATE, new AQLDataReference()).convertFromString(fxName);
		}

		const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());

		const unsigned int size = unders.getSize();
		for (unsigned int i = 0; i < size; ++i)
		{
			AQLObjectHolder &trade = unders.get(i);			
			setUpRiskInfo(riskEVec, trade.get());
		}
		
		if (!riskEVec.empty())
		{
			objHolder.remove(PRICING_DATA_RISKCALCINFOS);
			objHolder.add(PRICING_DATA_RISKCALCINFOS, new AQLDataMultiReference()).convertFromString(riskENames_str);
		}
	}
}


void 
AQLObjectConfiguration::setUpRiskInfo(vector<const AQLObject *> &riskEVec,  AQLObject &e) const
{
	// set risk info
	for (unsigned int i = 0; i < riskEVec.size(); ++i)
	{
		const AQLStringVector &riskAttrNames = dynamic_cast<const AQLDataStrings &>(riskEVec[i]->getData(AP_CALIBRATION_DATA_RISK_TRADESETDATA).get()).get();
		for (unsigned int j = 0; j < riskAttrNames.size(); ++j)
		{
			e.remove(riskAttrNames[j]);
			e.add(riskAttrNames[j], riskEVec[i]->getData(riskAttrNames[j]).get().clone());
		}
	}

}
// 
/*!
    @brief setup path object

	@param[out] object pool
*/
void
AQLObjectConfiguration::setUpPathEntity(AQLObjectPool &objPool, const AQLString *pPathName) const
{

	AQLMathPathEntity *path = 0;
	if (pPathName)
	{
		path = &dynamic_cast<AQLMathPathEntity &>(objPool.getObject(*pPathName, ENCHKTYPE_ISDEFINED).get());
	}
	else
	{
		path = AQLMarketData::getPathEnitty(objPool);
	}
	if (!path)
	{
		throw AQLCoreInvalidData("Path object is not set in entitiy pool", __FILE__, __LINE__);
	}
	AQLString fxName = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	// set antithetic
	AQLString isAntithetic =  mpStaticData->getStaticData(KEY_SIMULATION_ISANTITHETIC);
	if (isAntithetic.toUpper() == "TRUE")
	{
		path->setAntithetic(true);
	}
	// set brownian bridge
	AQLString isBrownianBridge =  mpStaticData->getStaticData(KEY_SIMULATION_ISBROWNIANBRIDGE);
	if (isBrownianBridge.toUpper() == "TRUE")
	{
		path->setBrownianBridge(true);
	}

	// set sdenames and models
	AQLStringVector ccys = AQLDealUtils::getSDECurrencys(true);
	unsigned int ccySize = ccys.size();
	AQLStringVector sdeNames(ccySize);
	AQLStringVector models(ccySize);
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		sdeNames[i] = AQLMarketData::getSDEName(ccys[i]);
		models[i] = AQLMarketData::getModelName(ccys[i]);
		models[i].toUpper();
	}
	path->getSDEAttrNames().set(sdeNames);

	//set simsdenames
	AQLStringVector simccys = AQLDealUtils::getSimulationSDECurrencys(true);
	unsigned int simccySize = simccys.size();
	AQLStringVector simsdeNames(simccySize);
	for (unsigned int i = 0; i< simccySize; i++)
	{
		simsdeNames[i] = AQLMarketData::getSDEName(simccys[i]);;
	}
	path->getSimulationSDEAttrNames().set(simsdeNames);

	// set sdetimegrid
	DoubleArray sdeTimeGrid;
// update for XLL Plus ////////////////////////////////////////////////////////////////////////////
	//getSDETimeGrid(sdeTimeGrid);
	//path->getSDETimeGrid().set(sdeTimeGrid);
	AQLString timegridContext = AQLCoreDataService::getContext(CONTEXT_KEY_SDE_TIMEGRID);
	if (timegridContext==AQ_NO_DATA)
	{
		getSDETimeGrid(sdeTimeGrid);
		path->getSDETimeGrid().set(sdeTimeGrid);
	}
	else
	{
		path->getSDETimeGrid().convertFromString(timegridContext);
		sdeTimeGrid=path->getSDETimeGrid().get();
	}
//////////////////////////////////////////////////////////////////////////
	DoubleArray sdeIntegralTimeGrid;
	getSDEIntegralTimeGrid(sdeIntegralTimeGrid);
	path->getSDEIntegralTimeGrid().set(sdeIntegralTimeGrid);

	//AQLString mainModel = getModelName();
	//mainModel.toLower();
	//// set sdedividnum
	//AQLString divNum = mpStaticData->getStaticData("sde." + mainModel + ".integral.divnum");

	//if (divNum != AQ_NO_DATA)
	//{
	//	path->getSDEIntegralDivNum().convertFromString(divNum);
	//}

	//// set sdedividnum
	//AQLString integralTimeGridStr = mpStaticData->getStaticData("sde." + mainModel + ".integral.timegrid");

	//if (integralTimeGridStr != AQ_NO_DATA)
	//{
	//	AQLDataDoubles tmpAttr;
	//	tmpAttr.convertFromString(integralTimeGridStr);
	//	DoubleArray integralTimeGrid = tmpAttr.get();
	//	integralTimeGrid.push_back(sdeTimeGrid.back());
	//	path->getSDEIntegralTimeGrid().set(integralTimeGrid);
	//}

	// set rand generator
	AQLString randGenerator =  mpStaticData->getStaticData(KEY_SIMULATION_RADNTYPE);
	path->getRand().convertFromString(randGenerator);

	// set initial value
	// first ccy 
	AQLString firstccy(ccys[0]);
	AQLString initialValue =  PREFIX_YIELD + mpStaticData->getStaticData(firstccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	AQLString ycProNames = AQLMarketData::getBaseYieldProName(ccys[0]);
	for (unsigned int i = 1; i < ccySize; ++i)
	{
		AQLString tmpccy(ccys[i]);
		// check vol or fx
		if (ccys[i].findString(POSTFIX_VOL) >= 0)
		{
			// initial value is saved in the object of spot sde volatility
			tmpccy.remove(tmpccy.findString(POSTFIX_VOL), AQLString(POSTFIX_VOL).size());
			initialValue += ":" + AQLString(PREFIX_VOL) + mpStaticData->getStaticData(tmpccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		}
		else if (ccys[i].findString(FX_DELIMITER) >= 0)
		{
			initialValue += ":" + fxName;
		}
		else
		{
			initialValue += ":" + AQLString(PREFIX_YIELD) + mpStaticData->getStaticData(tmpccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
			ycProNames += ":" + AQLMarketData::getBaseYieldProName(ccys[i]);
		}
		
	}
	path->getInitialValues().convertFromString(initialValue);
	path->getIRCurveProNames().convertFromString(ycProNames);

	// ir sde num and fx sde num
	unsigned int irNum = 0;
	unsigned int fxNum = 0;
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		if (ccys[i].findString(FX_DELIMITER) >= 0 || ccys[i].findString(POSTFIX_VOL) >= 0)
		{	
			break;
		}
		irNum = i + 1;
	}
	fxNum = ccySize - irNum;

	AQLStaticData &propG = AQLCoreDataService::getStaticDataManager().getIRGStaticData();
	const double maxMem = propG.getStaticData(KEY_GRID_TASK_MAX_MEMORY).getDoubleValue();
	const double buffer = mpStaticData->getStaticData(KEY_SIMULATION_PATH_CACHE_BUFFER).getDoubleValue();
	const int fwdLiborNum = sdeTimeGrid.size() - 1;

	double one_pathSize = 2.0 * (fwdLiborNum * (fwdLiborNum + 1) * 4.0 * irNum) + fwdLiborNum * 8.0 * fxNum;
	// set path cache
	const unsigned int pathCash = static_cast<unsigned int>((maxMem - buffer) / one_pathSize);
	path->setCacheSize(pathCash);

	//get simulation sde curruncy (which is used when funding change)
	// set sde correaltion 
	if (ccySize > 1 && simccySize > 1)
	{
		DoubleMatrix corData;
		AQLMarketData::getSDECorrelation(simccys, corData);
		path->getCorrelationMatrix().set(corData);
	}
	else if (ccySize > 1 && simccySize == 0)
	{
		DoubleMatrix corData;
		AQLMarketData::getSDECorrelation(ccys, corData);
		path->getCorrelationMatrix().set(corData);
	}


	// set sde data
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		AQLString tmpccy(ccys[i]);
		AQLString tmpmodel(models[i]);
		// set sde data
		AQLString sdeName;
		AQLString type;
		AQLString sdeFuncName;
		// check type
		if (ccys[i].findString(POSTFIX_VOL) >= 0)
		{
			tmpccy.remove(tmpccy.findString(POSTFIX_VOL), AQLString(POSTFIX_VOL).size());
			tmpmodel.remove(models[i].findString(POSTFIX_VOL), AQLString(POSTFIX_VOL).size());
			tmpmodel.toLower();
			
			sdeName = mpStaticData->getStaticData(tmpccy.toLower() + ".volatility" + STATIC_DATA_FX_KEY_SDE_NAME);
			sdeFuncName = mpStaticData->getStaticData(tmpccy.toLower() + ".volatility.sde." + tmpmodel + ".function.name");
			type = "VOL";
		}
		else
		{
			sdeName = mpStaticData->getStaticData(tmpccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
			sdeFuncName = mpStaticData->getStaticData(tmpccy.toLower() + ".sde." + tmpmodel.toLower() + ".function.name");
			if (ccys[i].findString(FX_DELIMITER) < 0)
			{
				type = "IR";
			}
			else
			{
				type = "FX";
			}
		}

		AQLString sdeAttrValue = type + ":" + ccys[i].toUpper() + ":" + sdeFuncName;
		AQLMathAttrSDE *sde = new AQLMathAttrSDE();
		sde->convertFromString(sdeAttrValue);
		path->AQLObject::remove(sdeName);
		path->AQLObject::add(sdeName ,sde);

		if (type == "IR" && AQLMarketData::isCorUse(models[i]))
		{
			// set correlation
			AQLString corName = PREFIX_COR + sdeName;
			AQLDataReference *corRef = new AQLDataReference();
			corRef->convertFromString(corName);
			path->AQLObject::remove(corName);
			path->AQLObject::add(corName ,corRef);
		}

		// set volatility
		AQLString volName = PREFIX_VOL + sdeName;
		AQLDataReference *volRef = new AQLDataReference();
		volRef->convertFromString(volName);
		path->AQLObject::remove(volName);
		path->AQLObject::add(volName ,volRef);
	}
}

// 
/*!
    @brief setup fx object

	@param[out] object pool
*/
void
AQLObjectConfiguration::setUpFXEntity(AQLObjectPool &objPool) const
{
	AQLStringVector ccys = AQLDealUtils::getSDECurrencys();
	unsigned int ccySize = ccys.size();

	if (ccySize == 1)
	{
		// single ccy no fx object
		return ;
	}

	vector<AQLMathFXEntity *> fxs;
	EntityIter it = objPool.begin();
	while (it != objPool.end())
	{
		if (it->second.isTypeOf(ENTITY_FX))
		{
			fxs.push_back(&dynamic_cast<AQLMathFXEntity &>(it->second.get()));
		}
		++it;
	}

	AQLString yieldCurves;

	const AQLStringVector &fxCurrencys = fxs[0]->getCurrencys().get();
	const unsigned int fxCSize = fxCurrencys.size();
	vector<int> sdePos(fxCSize);

	// set sde position
	int usd_pos_no_sde = -1;
	for (unsigned int i = 0; i < fxCSize; ++i)
	{
		AQLStringVector::const_iterator it = find(ccys.begin(), ccys.end(), fxCurrencys[i]);
		if (it == ccys.end())
		{
			if (fxCurrencys[i] != CURRENCY_USD)
			{
				AQLString tmpccy = fxCurrencys[i];
				yieldCurves += (PREFIX_YIELD + mpStaticData->getStaticData(tmpccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME) + ":");
				continue;
				//throw AQLCoreInvalidData(" FX currency (except USD) must setted as SDE !!", __FILE__, __LINE__);
			}
			else
			{
				AQLString tmpUSD = CURRENCY_USD;
				yieldCurves += (PREFIX_YIELD + mpStaticData->getStaticData(tmpUSD.toLower() + STATIC_DATA_FX_KEY_SDE_NAME) + ":");
				sdePos[i] = usd_pos_no_sde;
				continue;
			}
		}
		sdePos[i] = static_cast<int>(it - ccys.begin());

		AQLString tmpccy = ccys[sdePos[i]];
		yieldCurves += (PREFIX_YIELD + mpStaticData->getStaticData(tmpccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME) + ":");
	}

	const unsigned int fxNum = fxs.size();
	for (unsigned int i = 0; i < fxNum; ++i)
	{
		fxs[i]->getYieldCurves().convertFromString(yieldCurves.subString(0, yieldCurves.size() - 2));
	}

	for (unsigned int i = 0; i < fxCSize; ++i)
	{
		// if multi currency usd market data is need
		if (find(ccys.begin(), ccys.end(), fxCurrencys[i]) == ccys.end())
		{
			AQLCalibrateModel *generatorTmp = AQLModelConfiguration::getInstance()->
						createSDEGenerator(AQLMarketData::getModelName(fxCurrencys[i]));
		// update for XLL Plus //////////////////////////////////// 
		/*
			generatorUSD->loadModelDataAndCalibrate(CURRENCY_USD, dataInstance);
		*/

			AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
			AQLString temp = staticData.getStaticData(KEY_SDE_XCCY_USD_IS_GENERATE_ONLY_CURVE);
			if (temp==AQ_NO_DATA)
				temp = "FALSE";
			AQLDataBool isOnlyCurve;
			isOnlyCurve.convertFromString(temp);

			if (isOnlyCurve.get())
				generatorTmp->loadModelDataAndCalibrate(fxCurrencys[i], *(fxs[0]->getDataInstance()), true, false);
			else
				generatorTmp->loadModelDataAndCalibrate(fxCurrencys[i], *(fxs[0]->getDataInstance()));
		///////////////////////////////////////////////////////////		
			delete generatorTmp;
		}
	}



	
}

// 
/*!
    @brief setup extra object

	@param[out] object pool
*/
void
AQLObjectConfiguration::setUpExtraEntity(AQLObjectPool &objPool) const
{
	/////////////////////////////
	// setup extra object
	/////////////////////////////
	setUpFwdInterpolation(objPool);
	// set forward rate for Libor index which has -9999 fixingrate 
	AQLString mainTradeName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
	AQLObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);

	if (dynamic_cast<const AQLDataValuation &>
	(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			//complete bond funding leg
			setUpBondFundingLeg(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));

			//set up asofdate for trade object
			setUpAsOfDateForTradeEntity(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			//set up past coupon and past coupon payoffs for tarn product
			setUpPastCouponPayOffs(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			//set up LSMC coterm
			setUpCoTermOfLSMCIndex(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			//set up discount curve
			setUpDiscountCurve(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));

			//set FundingSpread 
			setUpFundingSpread(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			setUpExtraLibor(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			//set recalctradedata flag for each tradeentity			 
			// update for XLL Plus ////////////////////////////////////
            // setUpRecalcTrade(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
            AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
            AQLString temp = staticData.getStaticData(KEY_SIMULATION_FUNDING_CHANGED);
            if (temp==AQ_NO_DATA){
                setUpRecalcTrade(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
            }
            ///////////////////////////////////////////////////////////
			setUpDigitalCallSpreadCoupon(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			// set up dirty price
			setUpDirtyPrice(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			// setUpPastIndexRate
	        setUpPastIndexRate(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			// setUpScaleShftOfLSMC
	        setUpScaleShftOfLSMC(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			// setUpSVDToleranceForLSMC
			setUpSVDToleranceForLSMC(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
		}
	}
	else
	{
		//complete bond funding 
		setUpBondFundingLeg(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

		//set up asofdate for trade object
		setUpAsOfDateForTradeEntity(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		//set up past coupon and past coupon payoffs for tarn product
		setUpPastCouponPayOffs(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		//set up LSMC coterm
		setUpCoTermOfLSMCIndex(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		//set up discount curve
		setUpDiscountCurve(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

		//set FundingSpread 
		setUpFundingSpread(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		setUpExtraLibor(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		//set recalctradedata flag for each tradeentity
		// update for XLL Plus ////////////////////////////////////
        // setUpRecalcTrade(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
        AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
        AQLString temp = staticData.getStaticData(KEY_SIMULATION_FUNDING_CHANGED);
        if (temp==AQ_NO_DATA){
            setUpRecalcTrade(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
        }
        /////////////////////////////////////////////////
		setUpDigitalCallSpreadCoupon(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		// set up dirty price
		setUpDirtyPrice(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		// setUpPastIndexRate
		setUpPastIndexRate(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		// setUpScaleShftOfLSMC
		setUpScaleShftOfLSMC(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		// setUpSVDToleranceForLSMC
		setUpSVDToleranceForLSMC(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
	}
}

// 
/*!
    @brief setup extra Libor

	set forward rate for Libor index which has -9999 fixingrate 
	
	@param[in] object pool
	@param[out] trade object
*/
void
AQLObjectConfiguration::setUpExtraLibor(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const
{
	/////////////////////////////
	// set extra libor rate
	/////////////////////////////
	AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);

	AQLPriceDataDayCount dayCount(ACT_360);
	const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
										(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	const unsigned int legSize = unders.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		AQLObjectHolder &leg = unders.get(i);		
		AQLString inputType = dynamic_cast<const AQLDataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			AQLDataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				// cashlet loop
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					AQLDataHolder &attrCoupons = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
					if (attrCoupons.isDefined() && !attrCoupons.isNull())
					{
						AQLDataMultiReference &coupons = dynamic_cast<AQLDataMultiReference &>(attrCoupons.get());
						const unsigned int couponSize = coupons.getSize();
						// coupon loop
						for (unsigned int k = 0; k < couponSize; ++k)
						{
							// get ccy
							const AQLString &ccy = dynamic_cast<const AQLDataString &>(coupons.get(k).getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
							AQLDataHolder &attrIndexs = coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								AQLDataMultiReference &indexs = dynamic_cast<AQLDataMultiReference &>(attrIndexs.get());
								const unsigned int indexSize = indexs.getSize();
								// index loop
								for (unsigned int l = 0; l < indexSize; ++l)
								{
									AQLDataHolder &attrFixedRate = indexs.get(l).getData(PRICING_DATA_FIXEDRATE, NOCHECK);
									if (attrFixedRate.isDefined() && !attrFixedRate.isNull())
									{
										const double fixedRate = dynamic_cast<const AQLDataDouble &>(attrFixedRate.get()).get();
										// type
										AQLString indexType = indexs.get(l).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).convertToString();
										indexType.toUpper();
										indexType.exchange("\"", "");
										// check
										if (fixedRate == EXTRA_FIXEDRATE && (indexType == "FIXEDRATE" || indexType == "LIBOR"))
										{	
											// calc start date, 3 days after asofdate (sliding=following  calendar=TkB)
											AQLPriceDataSlidingRule sliding; // sliding
											sliding.convertFromString(SLIDING_FOLLOWING);
											AQLPriceDataCalendar cal;  // calendar
											cal.convertFromString(CITY_TkB);
											const AQLDate startDate = AQLMathDateCalculations::getDate(asOfDate, "3D", sliding, &cal, true);
											// get yield object
											const AQLString yieldName = AQLMarketData::getBaseYieldName(ccy);
											const AQLMathYieldCurve &curve = dynamic_cast<const AQLMathYieldCurve &>(objPool.getObject(yieldName, ENCHKTYPE_ISDEFINED).get());

											//modify get cfstartdate and paymentdate
											const AQLDate &cfstartDate = dynamic_cast<const AQLDataDate &>(cashlets.get(j).getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get();
											const AQLDate &paymentDate = dynamic_cast<const AQLDataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
											const double largeterm = dayCount.getTerm(cfstartDate, paymentDate);
											double ratio = 0.0;
											if (startDate < paymentDate)
											{
												// calc libor
												const double term = dayCount.getTerm(startDate, paymentDate);
												//const double rate = (curve.getDF(asOfDate, startDate) / curve.getDF(asOfDate, paymentDate) - 1.0) / term;
												double rate = (curve.getDF(asOfDate, startDate) / curve.getDF(asOfDate, paymentDate) - 1.0) / term;
												
												//modify
												ratio = term/largeterm;
												rate *= ratio;
												
												// set libor
												dynamic_cast<AQLDataDouble &>(attrFixedRate.get()).set(rate);
											}
											else
											{
												// set rate zero
												dynamic_cast<AQLDataDouble &>(attrFixedRate.get()).set(0.0);
											}

											//modify spread
											DoubleVector spreads = dynamic_cast<AQLDataDoubles &>(coupons.get(k).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).get();
											if (spreads.size() != 2)
												throw AQLCoreInvalidData("Coefficient must be size 2",__FILE__,__LINE__);

											spreads[1] *= ratio;
											dynamic_cast<AQLDataDoubles &>(coupons.get(k).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).set(spreads);
											

											//record a change libor
											indexs.get(l).remove("IsExtraLibor");
											indexs.get(l).add("IsExtraLibor", new AQLDataBool(true));
											indexs.get(l).remove("TermRatio");
											indexs.get(l).add("TermRatio", new AQLDataDouble(ratio));


										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// 
/*!
    @brief setup future fixing object

	@param[out] object pool
*/
void
AQLObjectConfiguration::setUpFutureFixingEntity(AQLObjectPool &objPool) const
{
	/////////////////////////////
	// setup future fixing object
	/////////////////////////////
	// set fixing rate for index which has fixingdate prior to asofdate
	AQLString mainTradeName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
	AQLObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);

	if (dynamic_cast<const AQLDataValuation &>
	(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			setUpFutureFixingFunc(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
		}
	}
	else
	{
		setUpFutureFixingFunc(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
	}
}

// 
/*!
    @brief setup future fixing

	// set fixing rate for index which has fixingdate prior to asofdate	
	@param[in] object pool
	@param[out] trade object
*/
void
AQLObjectConfiguration::setUpFutureFixingFunc(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const
{
	/////////////////////////////
	// set future fixing rate
	/////////////////////////////
	AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);
	AQLPriceDataDayCount dayCount(ACT_360);
	const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
										(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	const unsigned int legSize = unders.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		AQLObjectHolder &leg = unders.get(i);		
		AQLString inputType = dynamic_cast<const AQLDataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			AQLDataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					AQLDataHolder &attrCoupons = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
					if (attrCoupons.isDefined() && !attrCoupons.isNull())
					{
						AQLDataMultiReference &coupons = dynamic_cast<AQLDataMultiReference &>(attrCoupons.get());
						const unsigned int couponSize = coupons.getSize();
						// coupon loop
						for (unsigned int k = 0; k < couponSize; ++k)
						{
							AQLDataHolder &attrIndexs = coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								AQLDataMultiReference &indexs = dynamic_cast<AQLDataMultiReference &>(attrIndexs.get());
								const unsigned int indexSize = indexs.getSize();
								// index loop
								for (unsigned int l = 0; l < indexSize; ++l)
								{
									AQLDataHolder &attrfixingdate = indexs.get(l).getData(PRICING_DATA_FIXINGDATE, NOCHECK);
									if (attrfixingdate.isDefined() && !attrfixingdate.isNull())
									{
										// get fixingdate
										const AQLDate fixingdate = dynamic_cast<const AQLDataDate &>(attrfixingdate.get()).get();
										if (asOfDate > fixingdate)
										{
											// get fixingrate
											AQLDataHolder &attrFixedRate = indexs.get(l).getData(PRICING_DATA_FIXEDRATE, NOCHECK);
											if (!attrFixedRate.isDefined() || attrFixedRate.isNull())
											{
												AQLDataHolder &attrindexinfo = indexs.get(l).getData(CALIBRATION_DATA_NAME, ISNOTNULL);
												const AQLString& indexinfo_name = dynamic_cast<const AQLDataString &>(attrindexinfo.get()).get();
												
												// get fixedrate
												double fixedrate = getFixedRate(KEY_DEAL_FIXINGRATE, indexinfo_name);
												// set fixedrate
												AQLObject* pindex = &indexs.get(l).get();
												pindex->add(PRICING_DATA_FIXEDRATE, new AQLDataDouble(fixedrate));

												// get type
												AQLDataHolder &attrIndexType = indexs.get(l).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL);
												AQLString indexType = attrIndexType.convertToString();
												indexType.toUpper();
												indexType.exchange("\"", "");
												// check type
												if (indexType != "FIXEDRATE")
												{
													dynamic_cast<AQLDataString&>(attrIndexType.get()).set(PRICING_DATA_FIXEDRATE);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			if (i == 0)  // only Leg1
			{
				// for Tarn and Rachet
				AQLDataHolder &attrPaymentDates = leg.getData(PRICING_DATA_PAYMENTDATES, NOCHECK);
				if (attrPaymentDates.isDefined() && !attrPaymentDates.isNull())
				{
						if (attrCashlets.isDefined() && !attrCashlets.isNull())
						{
						// set payment dates
							AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(attrCashlets.get());
							const unsigned int cashletSize = cashlets.getSize();
							DateVector paymentdates_tmp;

						// cashlet loop
						for (unsigned int j = 0; j < cashletSize; ++j)
						{
							// get payment date
							const AQLDate &paymentDate = dynamic_cast<const AQLDataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
							if (paymentDate > asOfDate)
							{
								break;
							}
							paymentdates_tmp.push_back(paymentDate);
						}
						dynamic_cast<AQLDataDates &>(attrPaymentDates.get()).set(paymentdates_tmp);
					
						// if payoffs exist
						AQLDataHolder &attrCouponPayOffs = leg.getData(PRICING_DATA_COUPONPAYOFFS, NOCHECK);
						if (attrCouponPayOffs.isDefined() && !attrCouponPayOffs.isNull())
						{
							DoubleVector couponpayoffs_tmp;
							unsigned int counter = 0;
							// cashlet loop
							for (unsigned int j = 0; j < cashletSize; ++j)
							{
								// get payment date
								const AQLDate &paymentDate = dynamic_cast<const AQLDataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
								double couponpayoff = 0.0;
								if (paymentDate > asOfDate)
								{
									break;
								}
								couponpayoffs_tmp.push_back(couponpayoff);
							}
							dynamic_cast<AQLDataDoubles &>(attrCouponPayOffs.get()).set(couponpayoffs_tmp);
						}

						// if coupons not exist, add coupons in specific condition
						// cashlet loop
						for (unsigned int j = 0; j < cashletSize; ++j)
						{
							AQLDataHolder &attrCoupons = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
							if (attrCoupons.isDefined() && !attrCoupons.isNull())
							{
								AQLDataMultiReference &coupons = dynamic_cast<AQLDataMultiReference &>(attrCoupons.get());
								const unsigned int couponSize = coupons.getSize();
								// coupon loop
								for (unsigned int k = 0; k < couponSize; ++k)
								{
									AQLDataHolder &attrIndexs = coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
									if (attrIndexs.isDefined() && !attrIndexs.isNull())
									{
										AQLDataMultiReference &indexs = dynamic_cast<AQLDataMultiReference &>(attrIndexs.get());
										const unsigned int indexSize = indexs.getSize();
										// index loop
										for (unsigned int l = 0; l < indexSize; ++l)
										{
											// get type
											AQLDataHolder &attrIndexType = indexs.get(l).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL);
											AQLString indexType = attrIndexType.convertToString();
											indexType.toUpper();
											indexType.exchange("\"", "");
											if (indexType == CPN || indexType == CPNCF)
											{
												const AQLDate& basedate = dynamic_cast<const AQLDataDate &>(trade.getData(PRICING_DATA_TODAY, ISNOTNULL).get()).get();
												const AQLDate& observationStartDate = dynamic_cast<const AQLDataDate &>(indexs.get(l).getData(PRICING_DATA_OBSERVATIONSTARTDATE, ISNOTNULL).get()).get();
												const AQLDate& observationEndDate = dynamic_cast<const AQLDataDate &>(indexs.get(l).getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL).get()).get();

												//get past paymentdate
												DateVector needdates;
												const AQLDataHolder* dh = &(unders.get(0).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
												const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference&>(dh->get());
												for (unsigned int i = 0; i < cashlets.getSize(); i++)
												{
													dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
													if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment			
													dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
													const AQLDate& paymentdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
													if (paymentdate < observationStartDate) continue;
													if (paymentdate > basedate || paymentdate > observationEndDate) break;
													needdates.push_back(paymentdate);
												}
												if (needdates.size() > 0)
												{
													AQLDataHolder &attrCoupons = leg.getData(PRICING_DATA_COUPONS, NOCHECK);
													if (!attrCoupons.isDefined() || attrCoupons.isNull())
													{
														// add coupons
														AQLObject* pleg = &leg.get();
														DoubleVector couponVec;
														couponVec.push_back(0.0);
														pleg->add(PRICING_DATA_COUPONS, new AQLDataDoubles(couponVec));
													}
												}
											}
										}
									}
								}
							}
						}

						// if coupons exist
						AQLDataHolder &attrCoupons = leg.getData(PRICING_DATA_COUPONS, NOCHECK);
						if (attrCoupons.isDefined() && !attrCoupons.isNull())
						{
								DateVector paymentdates_tmp;
							DoubleVector coupons_tmp;

								// cashlet loop
								for (unsigned int j = 0; j < cashletSize; ++j)
								{
									// get payment date
									const AQLDate &paymentDate = dynamic_cast<const AQLDataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
								double coupon = 0.0;
									if (paymentDate > asOfDate)
									{
										break;
									}
								coupons_tmp.push_back(coupon);
							}
							dynamic_cast<AQLDataDoubles &>(attrCoupons.get()).set(coupons_tmp);
						}
					}
				}
				// for RangeAccrue fixing
				if (attrCashlets.isDefined() && !attrCashlets.isNull())
				{
					AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(attrCashlets.get());
					const unsigned int cashletSize = cashlets.getSize();
					unsigned int counter = 0;
					AQLString rangeaccrue_observationdates;
					AQLString rangeaccrue_observationrates;
					bool isRangeAccrue = false;
					// cashlet loop
					for (unsigned int j = 0; j < cashletSize; ++j)
					{
						// check rangeaccrue info
						AQLDataHolder &attrIsRA = cashlets.get(j).getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK);
						if (attrIsRA.isDefined() && !attrIsRA.get().isNull())
						{
							if (attrIsRA.get().convertToString().toUpper() == "TRUE")
						{
							isRangeAccrue = true;
							// check payment date
							const AQLDate &paymentDate = dynamic_cast<const AQLDataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
							if (paymentDate > asOfDate)
								break;
							counter++;
						}
						}
						else
						{
							counter++;
							isRangeAccrue = false;
						}
					}
					if (isRangeAccrue)
					{
						if (counter >= cashlets.getSize())
						{
							continue;
						}
						// check rangeaccrue info
						AQLDataHolder &attrIsRA = cashlets.get(counter).getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK);
						if (!attrIsRA.isDefined() || attrIsRA.isNull())
							throw AQLCoreInvalidData("IsRangeAccrue is not registed.",__FILE__,__LINE__);
	
						// get observationdates and rates
						DoubleVector fixedrateVec(0);
						DateVector fixeddateVec(0);
	
						if (counter != 0)
						{
							// get paymentdate before asofdate
							const AQLDate &paymentDate = dynamic_cast<const AQLDataDate &>(cashlets.get(counter - 1).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
						
							AQLDate adddate = paymentDate;
							unsigned int pos = 1;
							while (adddate <= asOfDate)
							{
								fixeddateVec.push_back(adddate);
								fixedrateVec.push_back(0.01);
								adddate.addDays(pos);
							}
						}
	
						//set observationdates and observationrates
						AQLDataHolder &attrIndexs = cashlets.get(counter).getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL);
						if (attrIndexs.isDefined() && !attrIndexs.isNull())
						{
							AQLDataMultiReference &raindexs = dynamic_cast<AQLDataMultiReference &>(attrIndexs.get());
							const unsigned int raindexSize = raindexs.getSize();
							// index loop
							for (unsigned int k = 0; k < raindexSize; ++k)
							{
								// check fixingrate
								AQLDataHolder &attrFixedRate = raindexs.get(k).getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK);
								if (attrFixedRate.isDefined() && !attrFixedRate.isNull())
								{
									//change object
									AQLObject* pindex = &raindexs.get(k).get();
									//remove
									pindex->remove(PRICING_DATA_OBSERVATIONDATES);
									pindex->remove(PRICING_DATA_OBSERVATIONRATES);
									//add
									pindex->add(PRICING_DATA_OBSERVATIONDATES, new AQLDataDates(fixeddateVec));
									pindex->add(PRICING_DATA_OBSERVATIONRATES, new AQLDataDoubles(fixedrateVec));
								}
							}
						}
					}
				}

			}
		}
	}
}

/*!
    @brief get fixed rate from property file

	@param[in] key
	@param[in] index
	@return double value

*/
double
AQLObjectConfiguration::getFixedRate(const AQLString &key, const AQLString &index) const
{
    AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString index_tmp = index;
	index_tmp.remove(0, 13);
    AQLString fixedrate = staticData.getStaticData(key + "." + index_tmp);

	if (fixedrate == AQ_NO_DATA)
	{
		fixedrate = staticData.getStaticData(key + ".standard");
		if (fixedrate == AQ_NO_DATA)
			fixedrate = "0.01";
	}
	return fixedrate.getDoubleValue();
}


// 
/*!
    @brief setup recalc trade

	set forward rate for Libor index which has -9999 fixingrate 
	
	@param[in] object pool
	@param[out] trade object
*/
void
AQLObjectConfiguration::setUpRecalcTrade(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const
{
	//set isfunding true
	AQLDataHolder* dh;
	dh = &(trade.getData(PRICING_DATA_FUNDINGCHANGEINFO,NOCHECK));
	bool ismdy = false;
	if (dh->isDefined() && !dh->isNull())
		ismdy = true;

	if (!ismdy)
		return;

	AQLObject& fginfo = dynamic_cast<AQLDataReference &>(dh->get()).get().get();

	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
	if (legs.getSize() != 2)
		throw AQLCoreInvalidData("Leg size must be 2",__FILE__,__LINE__);

	//get basecurrency
	AQLObject& fdleg = legs.get(1).get();
	bool isbndfnd = false;
	dh = &fdleg.getData(PRICING_DATA_ISBONDFUNDINGLEG, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isbndfnd = dynamic_cast<const AQLDataBool&>(dh->get());
	}
	AQLString basecur;
	if (isbndfnd)
	{
		dh = &fdleg.getData(PRICING_DATA_BONDFUNDINGCURRENCY, ISNOTNULL);
	}
	else
	{
		dh = &(fdleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		AQLDataMultiReference& fdcashs = dynamic_cast<AQLDataMultiReference &>(dh->get());
		AQLObject& fdcash0 = fdcashs.get(0).get();
		dh = &(fdcash0.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	}
	basecur = dynamic_cast<AQLDataString &>(dh->get()).get();
	basecur.toLower();

	//get mdycurrency
	AQLObject& strleg = legs.get(0).get();
	dh = &(strleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	AQLDataMultiReference& strcashs = dynamic_cast<AQLDataMultiReference &>(dh->get());
	AQLObject& strcash0 = strcashs.get(0).get();
	dh = &(strcash0.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	AQLString mdycur = dynamic_cast<AQLDataString &>(dh->get()).get();
	mdycur.toLower();
	
	
	//set base daycount
	AQLString basedc = mpStaticData->getStaticData(basecur + STATIC_DATA_KEY_FUNDING_DAYCOUNT).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGDAYCOUNT, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_BASEFUNDINGDAYCOUNT);
		fginfo.AQLObject::add(PRICING_DATA_BASEFUNDINGDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(basedc);
	}
	
	//set base slidingrule
	AQLString basesr = mpStaticData->getStaticData(basecur + STATIC_DATA_KEY_FUNDING_SLIDINGRULE).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGSLIDINGRULE, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_BASEFUNDINGSLIDINGRULE);
		fginfo.AQLObject::add(PRICING_DATA_BASEFUNDINGSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(basesr);
	}

	//set base calendar
	AQLString basecal = mpStaticData->getStaticData(basecur + STATIC_DATA_KEY_FUNDING_CALENDAR);
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGCALENDAR, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_BASEFUNDINGCALENDAR);
		fginfo.AQLObject::add(PRICING_DATA_BASEFUNDINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(basecal);
	}

	//set base index daycount
	AQLString baseindexdc = mpStaticData->getStaticData(basecur + STATIC_DATA_KEY_FUNDING_DAYCOUNT).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT);
		fginfo.AQLObject::add(PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(baseindexdc);
	}

	//set mdy daycount
	AQLString mdydc = mpStaticData->getStaticData(mdycur + STATIC_DATA_KEY_FUNDING_DAYCOUNT).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT);
		fginfo.AQLObject::add(PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(mdydc);
	}
	
	//set mdy slidingrule
	AQLString mdysr = mpStaticData->getStaticData(mdycur + STATIC_DATA_KEY_FUNDING_SLIDINGRULE).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE);
		fginfo.AQLObject::add(PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(mdysr);
	}

	//set mdy calendar
	AQLString mdycal = mpStaticData->getStaticData(mdycur + STATIC_DATA_KEY_FUNDING_CALENDAR);
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGCALENDAR, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGCALENDAR);
		fginfo.AQLObject::add(PRICING_DATA_MODIFIEDFUNDINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(mdycal);
	}

	//set mdy index daycount
	AQLString mdyindexdc = mpStaticData->getStaticData(mdycur + STATIC_DATA_KEY_FUNDING_DAYCOUNT).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT);
		fginfo.AQLObject::add(PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(mdyindexdc);
	}
}

// 
/*!
    @brief setUpFundingSpread	
	@param[in] object pool
	@param[out] trade object
*/
void
AQLObjectConfiguration::setUpFundingSpread(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const
{
	AQLDataHolder* dh;

	// check whether the data 'Underlyings' exists
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
	{
		return;
	}

	// get leg size
	const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
										(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	const unsigned int legSize = unders.getSize();

	// set yield curve pro
	dh = &(trade.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLString& ccy = dynamic_cast<const AQLDataString&> ((trade.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get()).get();
		const AQLString& yieldProName = AQLMarketData::getBaseYieldProName(ccy);
		trade.remove(PRICING_DATA_YIELDPRONAME);
		trade.AQLObject::add(PRICING_DATA_YIELDPRONAME, new AQLDataString()).convertFromString(yieldProName);
	}

	// get fundingspreadfile
	if (mFdSpdMap.empty())
	{
		//read funding spread file
		AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
		AQLString fdspdfile = staticData.getStaticData(KEY_FUNDINGSPREAD_FILE);
		if (AQLCoreDataService::isFileExist(AQLMarketData::getNumFileName(fdspdfile)))
		{
			AQLStringMatrix fdspdMatrix;
			AQLFileAccessor fdspdFile(AQLMarketData::getNumFileName(fdspdfile));
			fdspdFile.readAllData(',',fdspdMatrix);
			if (fdspdMatrix.size() > 0)
			{
				for (unsigned int i = 0; i < fdspdMatrix.size(); i++)
				{
					if (fdspdMatrix[i].size() < 2)
						throw AQLCoreInvalidData("funding spread file error",__FILE__,__LINE__);

					mFdSpdMap.insert(make_pair(fdspdMatrix[i][0],fdspdMatrix[i][1].getDoubleValue()));
				}
			}
		}
	}

	// set funding spread
	AQLString trade_name = trade.getName().get();
	map<AQLString, double>::iterator itspd = mFdSpdMap.find(trade_name);
	if (legSize == 1)
	{
		double spread = 0.;
		if (itspd != mFdSpdMap.end())
		{
			spread = itspd->second * 0.0001;
		}

		// set funding spread object
		AQLString fndSpdEntityName = trade_name + "_FUNDINGSPREAD";
		AQLObject* fndSpdEntity = NULL;
		if(!objPool.getObject(fndSpdEntityName).isDefined())
		{	
			fndSpdEntity = new AQLObject;
			objPool.set(fndSpdEntityName, fndSpdEntity);
			fndSpdEntity->add(CALIBRATION_DATA_NAME, new AQLDataString(fndSpdEntityName));
		}
		else
		{
			fndSpdEntity = &objPool.getObject(fndSpdEntityName).get();
		}
		fndSpdEntity->remove(PRICING_DATA_FUNDINGSPREAD);
		fndSpdEntity->add(PRICING_DATA_FUNDINGSPREAD, new AQLDataDouble(spread));
		trade.remove(PRICING_DATA_FUNDINGSPREADENTITY);
		trade.AQLObject::add(PRICING_DATA_FUNDINGSPREADENTITY, new AQLDataReference()).convertFromString(fndSpdEntityName);
	}
	else if (legSize == 2)
	{
		if (itspd == mFdSpdMap.end())
			return;

		double spread = itspd->second * 0.0001;

		// set funding spread object
		AQLString fndSpdEntityName = trade_name + "_FUNDINGSPREAD";
		AQLObject* fndSpdEntity = NULL;
		if(!objPool.getObject(fndSpdEntityName).isDefined())
		{	
			fndSpdEntity = new AQLObject;
			objPool.set(fndSpdEntityName, fndSpdEntity);
			fndSpdEntity->add(CALIBRATION_DATA_NAME, new AQLDataString(fndSpdEntityName));
		}
		else
		{
			fndSpdEntity = &objPool.getObject(fndSpdEntityName).get();
		}
		fndSpdEntity->remove(PRICING_DATA_FUNDINGSPREAD);
		fndSpdEntity->add(PRICING_DATA_FUNDINGSPREAD, new AQLDataDouble(spread));
		trade.remove(PRICING_DATA_FUNDINGSPREADENTITY);
		trade.AQLObject::add(PRICING_DATA_FUNDINGSPREADENTITY, new AQLDataReference()).convertFromString(fndSpdEntityName);
		
		// set coupon object
		DoubleVector val(2);
		val[0] = 1.0;
		val[1] = spread;

		//set isfunding true
		AQLDate asOf = AQLMarketData::getAsofDate(objPool);

		AQLObject& leg = unders.get(1).get();

		dh = &(leg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(dh->get());
		const unsigned int cashletSize = cashlets.getSize();

		for (unsigned int i = 0; i < cashletSize; i++)
		{
			AQLObject& cashlet = cashlets.get(i).get();
			
			//in case of notional cf or extracf as examples, we don't need couponinfos
			dh = &(cashlet.getData(PRICING_DATA_COUPONINFOS));
			if (!dh->isDefined() || dh->isNull())
				continue;

			//dh = &(cashlet.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
			AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference &>(dh->get());
			
			if (coupons.getSize() != 1)
				throw AQLCoreInvalidData("FundingCoupon Size must be 1",__FILE__,__LINE__);
			
			AQLObject& coupon = coupons.get(0).get();
		
			//check whether already fixed or not
			dh = &(coupon.getData(PRICING_DATA_INDEXINFOS));
			if (!dh ->isDefined() || dh->isNull())
				continue;

			AQLDataMultiReference& indexs = dynamic_cast<AQLDataMultiReference &>(dh->get());
			if (indexs.getSize() != 1)
				throw AQLCoreInvalidData("FundingIndex Size must be 1",__FILE__,__LINE__);
			AQLObject& index = indexs.get(0).get();
			dh = &(index.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
			if (!dh ->isDefined() || dh->isNull())
				continue;
			
			AQLDate fixingdate = dynamic_cast<AQLDataDate &>(dh->get()).get();

			double fixedrate = 0.0;
			dh = &(index.getData(PRICING_DATA_FIXEDRATE, NOCHECK));
			if (dh ->isDefined() && !dh->isNull())
				fixedrate = dynamic_cast<AQLDataDouble &>(dh->get()).get();

			if (fixingdate < asOf && fixedrate != -9999.0)
				continue;

			//set coefficient
			dh = &(coupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
			const DoubleVector& coeff = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();

			if (coeff.size() != 2)
				throw AQLCoreInvalidData ("FundingCoupon Coefficient must be size 2",__FILE__,__LINE__);

			dynamic_cast<AQLDataDoubles &>(dh->get()).set(val);
		}
	}
}

// 
/*!
    @brief setUpDirtyPrice
	@param[in] object pool
	@param[out] trade object
*/
void
AQLObjectConfiguration::setUpDirtyPrice(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const
{
	AQLDataHolder* dh;

	// get fundingspreadfile
	if (mDirtyPriceMap.empty())
	{
		//read funding spread file
		AQLString fileName = mpStaticData->getStaticData(KEY_DEAL_DIRTYPRICE_FILE);
		if (AQLCoreDataService::isFileExist(AQLMarketData::getNumFileName(fileName)))
		{
			AQLStringMatrix dirtyMatrix;
			AQLFileAccessor file(AQLMarketData::getNumFileName(fileName));
			file.readAllData(',', dirtyMatrix);
			for (unsigned int i = 0; i < dirtyMatrix.size(); i++)
			{
				if (dirtyMatrix[i].size() < 2)
				{
					throw AQLCoreInvalidData("dirty price file format error",__FILE__,__LINE__);
				}
				mDirtyPriceMap.insert(make_pair(dirtyMatrix[i][0], dirtyMatrix[i][1].getDoubleValue()));
			}
		}
	}

	// set funding spread
	const AQLString &name = trade.getName().get();
	map<AQLString, double>::const_iterator it = mDirtyPriceMap.find(name);
	if (it == mDirtyPriceMap.end())
	{
		return;
	}
	trade.remove(PRICING_DATA_DIRTYPRICE);
	trade.AQLObject::add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(it->second));
}

// 
/*!
    @brief setUpDigitalCallSpreadCoupon	
	@param[in] object pool
	@param[out] trade object
*/
void
AQLObjectConfiguration::setUpDigitalCallSpreadCoupon(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const
{
	// setup Future Fixing object
    AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();


	AQLDataHolder* dh;

	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());
	unsigned int legSize = legs.getSize();

	for (unsigned int i = 0; i < legSize; i++)
	{
		AQLObject& legentity = legs.get(i).get();
		dh = &(legentity.getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference&>(dh->get());
		unsigned int cashletSize = cashlets.getSize();

		for (unsigned int j = 0; j < cashletSize; j++)
		{
			AQLObject& cashletentity = cashlets.get(j).get();
			dh = &(cashletentity.getData(PRICING_DATA_COUPONSELECTOPERATOR, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
				continue;
			
			AQLPriceDataFunction& method = dynamic_cast<AQLPriceDataFunction&>(dh->get());
			
			AQLFunctionBase& couponfunc = method.getFunction();
			if (!couponfunc.isTypeOf(FN_CPNSLTOPERATORFORDIGITAL2))
				continue;


			dh = &(cashletentity.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
			const AQLDataMultiReference& coupons = dynamic_cast<const AQLDataMultiReference&>(dh->get());

			if (coupons.getSize() == 0)
				throw AQLCoreInvalidData("Coupon size error",__FILE__,__LINE__);

			AQLObject& coupponentity = coupons.get(0).get();
			dh = &(coupponentity.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
			const AQLDataMultiReference& indexs = dynamic_cast<const AQLDataMultiReference&>(dh->get());
			if (indexs.getSize() == 0)
				throw AQLCoreInvalidData("Index size error",__FILE__,__LINE__);

			//check index object is fx or not
			AQLObject& indexentity = indexs.get(0).get();
			dh = &(indexentity.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
			AQLString indextype = dynamic_cast<const AQLDataString&>(dh->get()).get();
			if (indextype.toUpper() != AQLString("FXRATE"))
				continue;

			dh = &(indexentity.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
			AQLString indexcur = dynamic_cast<const AQLDataString&>(dh->get()).get();
			indexcur.toLower();

			AQLString iscallspreadstr = staticData.getStaticData(indexcur + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD);
			if (iscallspreadstr.toUpper() != AQLString("TRUE"))
				continue;

			AQLString spreadvalstr = staticData.getStaticData(indexcur + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE);
			if (spreadvalstr == AQ_NO_DATA)
				throw AQLCoreInvalidData("DigitalCoupon CallSpread error",__FILE__,__LINE__);

			//set coupon digitalmode
			AQLPriceCouponForDigital2& digitalcoupon = dynamic_cast<AQLPriceCouponForDigital2&>(couponfunc);
			digitalcoupon.setCallSpread(true);
			digitalcoupon.setCallSpreadValue(spreadvalstr.getDoubleValue());
		}
	
	}
	


	return;
}

// 
/*!
    @brief forward interpolation

	@param[out] object pool
*/
void
AQLObjectConfiguration::setUpFwdInterpolation(AQLObjectPool &objPool) const
{
	mIsFwdInterMap.clear();
	mFwdInterMap.clear();
	AQLStringVector ccys = AQLCoreDataService::getContext(ARG_KEY_CURRENCY).toToken(MULTI_STATIC_DATA_DELIMITER);
	for (unsigned int i = 0; i < ccys.size(); ++i)
	{
		// fx return
		if (ccys[i].findString("/") >= 0)
		{
			continue;
		}
		AQLString strIsInter = mpStaticData->getStaticData(ccys[i].toLower() + STATIC_DATA_KEY_INDEX_LIBOR_ISFWDINTER).toUpper();
		if (strIsInter == "TRUE")
		{
			mIsFwdInterMap[ccys[i].toUpper()] = true;
			mFwdInterMap[ccys[i].toUpper()] = mpStaticData->getStaticData(ccys[i].toLower() + STATIC_DATA_KEY_INDEX_LIBOR_FWDINTERPOLATION);
		}
		else
		{
			mIsFwdInterMap[ccys[i].toUpper()] = false;
		}
		const AQLMathYieldCurvePro &yp = dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(AQLMarketData::getBaseYieldProName(ccys[i]), ENCHKTYPE_ISDEFINED).get());
		const map<AQLString, AQLString>& mktMap = yp.getAssignedCurveMktMap();
		map<AQLString, AQLString>::const_iterator it = mktMap.begin();

		while (it != mktMap.end())
		{
			AQLString suffix = "";
			if (it->second != SWAP)
			{
				suffix = "." + it->second;
				suffix.toLower();
			}
			strIsInter = mpStaticData->getStaticData(ccys[i].toLower() + STATIC_DATA_KEY_INDEX_LIBOR_ISFWDINTER + suffix).toUpper();
			AQLString key = ccys[i].toUpper() + "_" + it->first;
			if (strIsInter == "TRUE")
			{
				mIsFwdInterMap[key] = true;
				mFwdInterMap[key] = mpStaticData->getStaticData(ccys[i].toLower() + STATIC_DATA_KEY_INDEX_LIBOR_FWDINTERPOLATION + suffix);
			}
			else
			{
				mIsFwdInterMap[key] = false;
			}
			++it;
		}
	}
	AQLString mainTradeName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
	AQLObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);
	setUpFwdInterpolation(dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

}

// 
/*!
    @brief forward interpolation

	@param[out] object pool
*/
void
AQLObjectConfiguration::setUpFwdInterpolation(AQLMathObjectValue &trade) const
{
	if (trade.isMethodTypeOf(FN_IR_PORTFOLIOVALUE))
	{
		AQLDataMultiReference &ref = dynamic_cast<AQLDataMultiReference &>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		for (unsigned int i = 0; i < ref.getSize(); ++i)
		{
			setUpFwdInterpolation(dynamic_cast<AQLMathObjectValue &>(ref.get(i).get()));
		}
	}
	else
	{
		AQLDataHolder *dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			AQLDataMultiReference &legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
			for (unsigned int i = 0; i < legs.getSize(); ++i)
			{
				// leg
				dh = &legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					const AQLDataMultiReference &cashlets = dynamic_cast<const AQLDataMultiReference &>(dh->get());
					for (unsigned int j = 0; j < cashlets.getSize(); ++j)
					{
						// cashlets
						dh = &(cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							AQLDataMultiReference &couponinfos = dynamic_cast<AQLDataMultiReference &>(dh->get());	
							for (unsigned int k = 0; k < couponinfos.getSize(); ++k)
							{
								// coupon
								dh = &(couponinfos.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
								if (dh->isDefined() && !dh->isNull())
								{
									AQLDataMultiReference &indexInfos = dynamic_cast<AQLDataMultiReference &>(dh->get());	
									for (unsigned int l = 0; l < indexInfos.getSize(); ++l)
									{
										// indexinfos
										AQLString indexType = dynamic_cast<const AQLDataString &>(indexInfos.get(l).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get();
										indexType.toUpper();
										if (indexType == LIBOR || indexType == CMS)
										{
											AQLString ccy = dynamic_cast<const AQLDataString &>(indexInfos.get(l).getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
											ccy.toUpper();
											AQLString key = ccy;
											dh = &(indexInfos.get(l).getData(PRICING_DATA_BASISCURVE, NOCHECK));
											if (dh->isDefined() && !dh->isNull())
											{
												key += "_" + dynamic_cast<AQLDataString &>(dh->get()).get();
											}
											if (mIsFwdInterMap[key])
											{
												indexInfos.get(l).remove(PRICING_DATA_ISFWDINTERPOLATION);
												indexInfos.get(l).remove(PRICING_DATA_FWDINTERPOLATION);
												indexInfos.get(l).add(PRICING_DATA_ISFWDINTERPOLATION, new AQLDataBool(true));
												indexInfos.get(l).add(PRICING_DATA_FWDINTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(mFwdInterMap[key]);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


// 
/*!
    @brief coterm set up

	@param[out] objPool
	@param[out] trade
*/

void
AQLObjectConfiguration::setUpCoTermOfLSMCIndex(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{

	AQLDataHolder* dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	AQLObject& callinfo = dynamic_cast<AQLDataReference &>(dh->get()).get().get();

	dh = &(callinfo.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;
	AQLDataMultiReference& refs = dynamic_cast<AQLDataMultiReference &>(dh->get());

	
	//get coterm of this trade;
	AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);
	AQLDate eDate;
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_ENDDATE, NOCHECK));
		AQLDate legeDate;
		if (dh->isDefined() && !dh->isNull())
			legeDate = dynamic_cast<AQLDataDate &>(dh->get()).get();

		eDate = (legeDate > eDate ) ? legeDate : eDate;
	}

	for (unsigned int i = 0 ; i < refs.getSize(); i++)
	{
		AQLObject& eindex = refs.get(i).get();
		dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
		AQLString tmpstr = dynamic_cast<AQLDataString &>(dh->get());

		if (-1 == tmpstr.findString("Co-Term"))
			continue;

		//set co term
		dh = &(eindex.getData(PRICING_DATA_FREQUENCY, ISNOTNULL));
		AQLString freq = dynamic_cast<AQLDataString &>(dh->get()).get();
		AQLString coTerm = "Co-Term";
		coTerm += createCoTermTenorString(asOfDate, eDate, freq); 
		dynamic_cast<AQLDataString &>(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL).get()).set(coTerm);
	}
	
	return;

}

// 
/*!
    @brief set up scale and shift flag of LSMC

	@param[out] objPool
	@param[out] trade
*/

void
AQLObjectConfiguration::setUpScaleShftOfLSMC(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{

	AQLDataHolder* dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	AQLObject& callinfo = dynamic_cast<AQLDataReference &>(dh->get()).get().get();

	AQLString tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_ISSHIFTY).toUpper();
	callinfo.remove(PRICING_DATA_ISSHIFT_Y);
	if (tmp != AQ_NO_DATA && tmp == "TRUE")
	{
		callinfo.add(PRICING_DATA_ISSHIFT_Y, new AQLDataBool(true));
	}
	tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_ISSHIFTX).toUpper();
	callinfo.remove(PRICING_DATA_ISSHIFT_X);
	if (tmp != AQ_NO_DATA && tmp == "TRUE")
	{
		callinfo.add(PRICING_DATA_ISSHIFT_X, new AQLDataBool(true));
	}

	tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_ISSCALEY).toUpper();
	callinfo.remove(PRICING_DATA_ISSCALE_Y);
	if (tmp != AQ_NO_DATA && tmp == "TRUE")
	{
		callinfo.add(PRICING_DATA_ISSCALE_Y, new AQLDataBool(true));
	}
	tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_ISSCALEX).toUpper();
	callinfo.remove(PRICING_DATA_ISSCALE_X);
	if (tmp != AQ_NO_DATA && tmp == "TRUE")
	{
		callinfo.add(PRICING_DATA_ISSCALE_X, new AQLDataBool(true));
	}
}




void
AQLObjectConfiguration::setUpAsOfDateForTradeEntity(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{

	//set isfunding true
	AQLDate asOf = AQLMarketData::getAsofDate(objPool);
	AQLDataHolder* dh;

	// check type
	AQLDataValuation &valueTypeAttr = dynamic_cast<AQLDataValuation &>(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
	
	//tradevalue, lsmctradevalue plainvanillaswaptradevalue
	if (valueTypeAttr.isTypeOf(FN_IR_TRADEVALUE))
	{
		trade.remove(PRICING_DATA_TODAY);
		trade.AQLObject::add(PRICING_DATA_TODAY, new AQLDataDate(asOf));
	}

    dh = &trade.getData(CALIBRATION_DATA_ASOFDATE);
    if(!dh->isDefined() || dh->isNull()){
        trade.remove(CALIBRATION_DATA_ASOFDATE);
        trade.add(CALIBRATION_DATA_ASOFDATE).convertFromString(asOf.stringWithFormat());
    }

    dh = &(trade.getData(PRICING_DATA_SETTLEDATE));
    if(!dh->isDefined() || dh->isNull()){
        AQLString settlestr = AQLCoreDataService::getContext(ARG_KEY_SETTLEDATE);
        trade.remove(PRICING_DATA_SETTLEDATE);
        if (settlestr != AQ_NO_DATA)
            trade.AQLObject::add(PRICING_DATA_SETTLEDATE, new AQLDataDate()).convertFromString(settlestr);
        else
            trade.AQLObject::add(PRICING_DATA_SETTLEDATE, new AQLDataDate(asOf));
    }	

	
	return;

}


//==============================================
void 
AQLObjectConfiguration::setUpPastCouponPayOffs(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{
	
	AQLDataHolder* dh;
	dh = &(trade.getData(PRICING_DATA_TRIGGERINFOS, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	AQLDataMultiReference& trgrefs = dynamic_cast<AQLDataMultiReference &>(dh->get());

	for (unsigned int i = 0; i < trgrefs.getSize(); i++)
	{


		AQLObject& etrigger = trgrefs.get(i).get();
		dh = &(etrigger.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		AQLObject& indexinfo = dynamic_cast<AQLDataMultiReference &>(dh->get()).get(0).get();

		//index type
		dh = &(indexinfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));		
		AQLString indextype = dynamic_cast<const AQLDataString&>(dh->get()).get();
		indextype.toUpper();
		if (indextype != "CPN" && indextype != "CPNCF")
		{
			//not tarn case
			continue;
		}
		
		//index accessories
		unsigned int legno = 0;
		dh = &(indexinfo.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
		AQLString accessory  = dynamic_cast<const AQLDataString&>(dh->get()).get();
		accessory.toUpper();
		if (accessory.findString("LEG") == 0 && accessory.size() > 3)
		{
			legno = accessory.subString(3, accessory.size() - 1).getIntValue();
			if (legno > 0) legno -= 1;
			else
			{
				//not tarn case
				continue;
			}
		}
		else
		{
			//not tarn case
			continue;
		}

		//target leg
		AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);

		dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		AQLObject& eleg = dynamic_cast<AQLDataMultiReference &>(dh->get()).get(legno).get();

		//cashlets
		DateVector paymentdateVec;
		DoubleVector couponVec, couponpayoffVec;
		dh = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		AQLDataMultiReference& cashlets = dynamic_cast<AQLDataMultiReference &>(dh->get());
		for (unsigned int j = 0 ; j < cashlets.getSize(); j++)
		{
			AQLObject& ecashlet = cashlets.get(j).get();
			dh = &(ecashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
			AQLDate paydate = dynamic_cast<AQLDataDate &>(dh->get()).get();
			if (paydate > asOfDate)
				break;

			//in case of notional exchange cashlet, we have to skip it
			dh = &(ecashlet.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference &>(dh->get());
				AQLObject& ecoupon = coupons.get(0).get();
				
				dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
				AQLDataMultiReference& indices = dynamic_cast<AQLDataMultiReference &>(dh->get());
				AQLObject& eindex = indices.get(0).get();

				//fixed rate
				dh = &(eindex.getData(PRICING_DATA_FIXEDRATE));
                double fixedrate = dh->isDefined() && !dh->isNull() ? dynamic_cast<AQLDataDouble &>(dh->get()).get() : 0;

				//notional
				dh = &(ecashlet.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL));
				double notional = dynamic_cast<AQLDataDouble &>(dh->get()).get();

				//startdate, enddate, daycount to get tau
				dh = &(ecashlet.getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL));
				const AQLDate& sdate = dynamic_cast<const AQLDataDate &>(dh->get()).get();

				dh = &(ecashlet.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL));
				const AQLDate& edate = dynamic_cast<const AQLDataDate &>(dh->get()).get();

				dh = &(ecashlet.getData(PRICING_DATA_DAYCOUNT, ISNOTNULL));
				const AQLPriceDataDayCount& dc = dynamic_cast<AQLPriceDataDayCount &>(dh->get());

				double tau = dc.getTerm(sdate,edate,false);

				paymentdateVec.push_back(paydate);
				couponVec.push_back(fixedrate);
				couponpayoffVec.push_back(notional * fixedrate * tau);
			}
		
		}

		//if we have past coupons
		if (paymentdateVec.size() > 0)
		{
            if(!(dh = &eleg.getData(PRICING_DATA_PAYMENTDATES, NOCHECK))->isDefined() || dh->isNull()){
                eleg.remove(PRICING_DATA_PAYMENTDATES);
                eleg.add(PRICING_DATA_PAYMENTDATES, new AQLDataDates(paymentdateVec));
            }
            if(!(dh = &eleg.getData(PRICING_DATA_COUPONS, NOCHECK))->isDefined() || dh->isNull()){
                eleg.remove(PRICING_DATA_COUPONS);
                eleg.add(PRICING_DATA_COUPONS, new AQLDataDoubles(couponVec));
            }
            if(!(dh = &eleg.getData(PRICING_DATA_COUPONPAYOFFS, NOCHECK))->isDefined() || dh->isNull()){
                eleg.remove(PRICING_DATA_COUPONPAYOFFS);
                eleg.add(PRICING_DATA_COUPONPAYOFFS, new AQLDataDoubles(couponpayoffVec));
            }
		}
	}
	
	return;
}


/*!
	@brief set up discount curve

	@param[in] objPool
	@param[in,out] trade object 

	@return
*/
void 
AQLObjectConfiguration::setUpDiscountCurve(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{
	
	AQLDataHolder *dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		AQLString ccy =  dynamic_cast<const AQLDataString &>(trade.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
		ccy.toLower();
		AQLString dCurve = DF2;
		const AQLString dCurve_p = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_DF2);
		if (dCurve_p != AQ_NO_DATA)
		{
			dCurve = dCurve_p;
		}
		AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
		for (unsigned int i = 0; i < legs.getSize(); i++)
		{
			dh = &(legs.get(i).getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				legs.get(i).remove(PRICING_DATA_DISCOUNTCURVE);
				legs.get(i).add(PRICING_DATA_DISCOUNTCURVE, new AQLDataString(dCurve));
			}
		}
	}
	return;
}

AQLString AQLObjectConfiguration::createCoTermTenorString(const AQLDate& today, const AQLDate& matdate, const AQLString& freq) const
{
    // define
    const AQLString CONST_FREQUENCY_ANNUAL="Annual";
    const AQLString CONST_FREQUENCY_SEMI_ANNUAL="Semi-Annual";
    const AQLString CONST_FREQUENCY_QUARTERLY="Quarterly";
    const AQLString CONST_FREQUENCY_MONTHLY="Monthly";

    const AQLString ACCESSORY_CO_TERM_YEAR="Y";
    const AQLString ACCESSORY_CO_TERM_MONTH="M";
    const AQLString CO_TERM_MIN_MONTH="6";



    try
    {
        int matYear, matMonth, matDay;
        int todayYear, todayMonth, todayDay;

        // >>> split matdate
        matYear  = matdate.yearOfEra();
        matMonth = matdate.monthOfYear();
        matDay   = matdate.dayOfMonth();

        // >>> split today
        todayYear  = today.yearOfEra();
        todayMonth = today.monthOfYear();
        todayDay   = today.dayOfMonth();

        int diffYear  = matYear - todayYear;
        int diffMonth = matMonth - todayMonth;
        int diffDay   = matDay - todayDay;


        // >>> month round 
        if (diffDay<-15)
        {
            // round down...
            diffMonth--;
        }
        else if (diffDay>15)
        {
            // round up!
            diffMonth++;
        }

        // >>> diff month check
        if (diffMonth<0)
        {
            diffYear--;
            diffMonth += 12;
        }

        // >>> diff year check
        if (diffYear<0)
        {
			throw AQLCoreInvalidData("date diff failed.",__FILE__,__LINE__);
        }



        // >>> make string to each frequency
        AQLString ret;
        if (freq==CONST_FREQUENCY_ANNUAL)
        {
            // annual(1Y)
            if (diffMonth<6)
            {
                // nothing to do.
            }
            else
            {
                // add years
                diffYear++;
            }

            // create string!
            ret += AQLString(diffYear) + ACCESSORY_CO_TERM_YEAR;
        }
		else if (freq==CONST_FREQUENCY_SEMI_ANNUAL
				|| freq==CONST_FREQUENCY_QUARTERLY
				|| freq==CONST_FREQUENCY_MONTHLY)
        {
            // annual(6M)
            if (diffMonth<3)
            {
                // round down...
                diffMonth = 0;
            }
            else if (diffMonth<9)
            {
                // adjusted "6"
                diffMonth = 6;
            }
            else
            {
                // round up!
                diffMonth = 0;
                diffYear++;
            }

            // create string!
			if (diffYear == 0 && diffMonth == 0)
			{
				ret += CO_TERM_MIN_MONTH + ACCESSORY_CO_TERM_MONTH;
			}
			else
			{
				ret += AQLString(diffYear) + ACCESSORY_CO_TERM_YEAR;

				if(diffMonth>0)
				{
					ret += AQLString(diffMonth) + ACCESSORY_CO_TERM_MONTH;
				}
			}
        }
        else
        {
            throw AQLCoreInvalidData("illegal frequency.",__FILE__,__LINE__);
        }

        return ret;
    }
	catch (...)
    {
        throw AQLCoreInvalidData("error in createCoTermTenorString",__FILE__,__LINE__);
	}
}

void AQLObjectConfiguration::setUpRangeAccrueFixedRates(const AQLDate& asOfDate, AQLObjectHolder& ehTrade) const
{
	// read fixing rate file
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString ratefile = staticData.getStaticData(KEY_DEAL_RATE_FILE);
	if (ratefile == AQ_NO_DATA)
	{
		return;
	}
	AQLStringMatrix ratesMatrix;
	AQLFileAccessor ratesFile(AQLMarketData::getNumFileName(ratefile));
	ratesFile.readAllData(MARKET_DATA_DELIMITER,ratesMatrix);
	ratesFile.close();

	const AQLDataMultiReference &legs = dynamic_cast<const AQLDataMultiReference &>
										(ehTrade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	const unsigned int legSize = legs.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		AQLObjectHolder &leg = legs.get(i);		
		AQLString inputType = dynamic_cast<const AQLDataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			AQLDataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				unsigned int counter = 0;
				bool isRangeAccrue = false;
				// cashlet loop
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					// check rangeaccrue info
					AQLDataHolder &attrIsRA = cashlets.get(j).getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK);
					if (attrIsRA.isDefined() && !attrIsRA.get().isNull())
					{
						if (attrIsRA.get().convertToString().toUpper() == "TRUE")
						{
							isRangeAccrue = true;
							// check payment date
							const AQLDate &paymentDate = dynamic_cast<const AQLDataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
							if (paymentDate > asOfDate)
								break;
							counter++;
						}
					}
					else
					{
						counter++;
						isRangeAccrue = false;
					}
				}
				if (isRangeAccrue)
				{
					if (counter >= cashlets.getSize())
					{
						continue;
					}

					// check rangeaccrue info
					AQLDataHolder &attrIsRA = cashlets.get(counter).getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK);
					if (!attrIsRA.isDefined() || attrIsRA.isNull())
						throw AQLCoreInvalidData("IsRangeAccrue is not registed.",__FILE__,__LINE__);

					// check asofdate								
					const AQLDate &startDate = dynamic_cast<const AQLDataDate &>
						(cashlets.get(counter).getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get();
					const AQLDate &endDate = dynamic_cast<const AQLDataDate &>
						(cashlets.get(counter).getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL).get()).get();
					if (asOfDate < startDate || endDate < asOfDate)
					{
						continue;
					}

					//set observationdates and observationrates
					AQLDataHolder &attrInfos = cashlets.get(counter).getData(PRICING_DATA_RANGEACCRUEINFOS, NOCHECK);
					if (attrInfos.isDefined() && !attrInfos.isNull()) // multi index case
					{
						AQLDataMultiReference &rainfos = dynamic_cast<AQLDataMultiReference &>(attrInfos.get());
						const unsigned int rainfoSize = rainfos.getSize();
						// info loop
						for (unsigned int kk = 0; kk < rainfoSize; ++kk)
						{
							AQLDataHolder &attrIndexs = rainfos.get(kk).getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								AQLDataMultiReference &raindexs = dynamic_cast<AQLDataMultiReference &>(attrIndexs.get());
								AQLObjectConfiguration::setUpIndexFixedRates(ratesMatrix, raindexs);
							}
							AQLDataHolder &attrBoundaryIndexs = rainfos.get(kk).getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK);
							if (attrBoundaryIndexs.isDefined() && !attrBoundaryIndexs.isNull())
							{
								AQLDataMultiReference &rabindexs = dynamic_cast<AQLDataMultiReference &>(attrBoundaryIndexs.get());
								AQLObjectConfiguration::setUpBoundaryIndexFixedRate(ratesMatrix, rabindexs);
							}
						}
					}
					else // single index case
					{
						AQLDataHolder &attrIndexs = cashlets.get(counter).getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, NOCHECK);
						if (attrIndexs.isDefined() && !attrIndexs.isNull())
						{
							AQLDataMultiReference &raindexs = dynamic_cast<AQLDataMultiReference &>(attrIndexs.get());
							AQLObjectConfiguration::setUpIndexFixedRates(ratesMatrix, raindexs);
						}
						AQLDataHolder &attrBoundaryIndexs = cashlets.get(counter).getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK);
						if (attrBoundaryIndexs.isDefined() && !attrBoundaryIndexs.isNull())
						{
							AQLDataMultiReference &rabindexs = dynamic_cast<AQLDataMultiReference &>(attrBoundaryIndexs.get());
							AQLObjectConfiguration::setUpBoundaryIndexFixedRate(ratesMatrix, rabindexs);
						}
					}
				}
			}
		}
	}
}

void AQLObjectConfiguration::setUpIndexFixedRates_ObsvOpr(const AQLStringMatrix& ratesMatrix, AQLDataMultiReference &indexs) const
{
	const unsigned int indexSize = indexs.getSize();
	// index loop
	for (unsigned int i = 0; i < indexSize; ++i)
	{
		// check observation operator
		AQLDataHolder &attrObsvOpr = indexs.get(i).getData(PRICING_DATA_OBSERVATIONOPERATOR, NOCHECK);
		if (!attrObsvOpr.isDefined() || attrObsvOpr.isNull()) continue;

		// get index name
		const AQLDataHolder &attrIndexName = indexs.get(i).getData(PRICING_DATA_INDEXNAME, NOCHECK);
		if (!attrIndexName.isDefined() || attrIndexName.isNull()) continue;
		const AQLString& indexName = dynamic_cast<const AQLDataString &>(attrIndexName.get()).get();

		DateVector dateVec(0);
		DoubleVector rateVec(0);
		bool isFound = false;
		for (unsigned int ii = 0; ii < ratesMatrix.size(); ++ii)
		{
			if (indexName != ratesMatrix[ii][0]) continue;

			AQLStringVector strDateVec = ratesMatrix[ii][1].toToken(MULTI_STATIC_DATA_DELIMITER);
			AQLStringVector strRateVec = ratesMatrix[ii][2].toToken(MULTI_STATIC_DATA_DELIMITER);
			for (unsigned int jj = 0; jj < strDateVec.size(); ++jj)
			{
				AQLDate date(strDateVec[jj].getCString());
				dateVec.push_back(date);
				rateVec.push_back(strRateVec[jj].getDoubleValue());
			}
			isFound = true;
			break;
		}
		if (!isFound) continue;

		//change object
		AQLObject* pindex = &indexs.get(i).get();
		//remove
		pindex->remove(PRICING_DATA_OBSERVATIONDATES);
		pindex->remove(PRICING_DATA_OBSERVATIONRATES);
		//add
		pindex->add(PRICING_DATA_OBSERVATIONDATES, new AQLDataDates(dateVec));
		pindex->add(PRICING_DATA_OBSERVATIONRATES, new AQLDataDoubles(rateVec));
	}
}

void AQLObjectConfiguration::setUpIndexFixedRates(const AQLStringMatrix& ratesMatrix, AQLDataMultiReference &indexs) const
{
	const unsigned int indexSize = indexs.getSize();
	// index loop
	for (unsigned int i = 0; i < indexSize; ++i)
	{
		// get index name
		const AQLDataHolder &attrIndexName = indexs.get(i).getData(PRICING_DATA_INDEXNAME, NOCHECK);
		if (!attrIndexName.isDefined() || attrIndexName.isNull()) continue;
		const AQLString& indexName = dynamic_cast<const AQLDataString &>(attrIndexName.get()).get();

		DateVector dateVec(0);
		DoubleVector rateVec(0);
		bool isFound = false;
		for (unsigned int ii = 0; ii < ratesMatrix.size(); ++ii)
		{
			if (indexName != ratesMatrix[ii][0]) continue;

			AQLStringVector strDateVec = ratesMatrix[ii][1].toToken(MULTI_STATIC_DATA_DELIMITER);
			AQLStringVector strRateVec = ratesMatrix[ii][2].toToken(MULTI_STATIC_DATA_DELIMITER);
			for (unsigned int jj = 0; jj < strDateVec.size(); ++jj)
			{
				AQLDate date(strDateVec[jj].getCString());
				dateVec.push_back(date);
				rateVec.push_back(strRateVec[jj].getDoubleValue());
			}
			isFound = true;
			break;
		}
		if (!isFound) continue;

		//change object
		AQLObject* pindex = &indexs.get(i).get();
		//remove
		pindex->remove(PRICING_DATA_OBSERVATIONDATES);
		pindex->remove(PRICING_DATA_OBSERVATIONRATES);
		//add
		pindex->add(PRICING_DATA_OBSERVATIONDATES, new AQLDataDates(dateVec));
		pindex->add(PRICING_DATA_OBSERVATIONRATES, new AQLDataDoubles(rateVec));
	}
}

void AQLObjectConfiguration::setUpBoundaryIndexFixedRate(const AQLStringMatrix& ratesMatrix, AQLDataMultiReference &indexs) const
{
	const unsigned int indexSize = indexs.getSize();
	// index loop
	for (unsigned int i = 0; i < indexSize; ++i)
	{
		// get index name
		const AQLDataHolder &attrIndexName = indexs.get(i).getData(PRICING_DATA_INDEXNAME, NOCHECK);
		if (!attrIndexName.isDefined() || attrIndexName.isNull()) continue;
		const AQLString& indexName = dynamic_cast<const AQLDataString &>(attrIndexName.get()).get();
		
		// get fixing date
		const AQLDataHolder &attrFixingDate = indexs.get(i).getData(PRICING_DATA_FIXINGDATE, NOCHECK);
		if (!attrFixingDate.isDefined() || attrFixingDate.isNull()) continue;
		const AQLDate& fixingdate = dynamic_cast<const AQLDataDate&>(attrFixingDate.get()).get();

		double fixedrate;
		bool isFound = false;
		for (unsigned int ii = 0; ii < ratesMatrix.size(); ++ii)
		{
			if (indexName != ratesMatrix[ii][0]) continue;

			AQLStringVector strDateVec = ratesMatrix[ii][1].toToken(MULTI_STATIC_DATA_DELIMITER);
			AQLStringVector strRateVec = ratesMatrix[ii][2].toToken(MULTI_STATIC_DATA_DELIMITER);
			for (unsigned int jj = 0; jj < strDateVec.size(); ++jj)
			{
				AQLDate date(strDateVec[jj].getCString());
				if (date == fixingdate)
				{
					fixedrate = strRateVec[jj].getDoubleValue();
					isFound = true;
					break;
				}
			}
			if (isFound) break;
		}
		if (!isFound) continue;

		//change object
		AQLObject* pindex = &indexs.get(i).get();
		//remove
		pindex->remove(PRICING_DATA_FIXEDRATE);
		//add
		pindex->add(PRICING_DATA_FIXEDRATE, new AQLDataDouble(fixedrate));
	}
}

void AQLObjectConfiguration::setUpPastIndexRate(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{
	// read fixing rate file
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString ratefile = staticData.getStaticData(KEY_DEAL_RATE_FILE);
	if (ratefile == AQ_NO_DATA)
	{
		return;
	}
	AQLStringMatrix ratesMatrix;

	AQLString filename=AQLMarketData::getNumFileName(ratefile);

	if (!AQLCoreDataService::isFileExist(filename))
	{
		return;
	}
	
	AQLFileAccessor ratesFile(filename);
	ratesFile.readAllData(MARKET_DATA_DELIMITER,ratesMatrix);
	ratesFile.close();

	if(ratesMatrix.empty())
	{
		return;
	}

	AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);

	const AQLDataHolder& ahUnders = trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (!ahUnders.isDefined() || ahUnders.isNull())
	{
		return;
	}
	const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>(ahUnders.get());
	const unsigned int legSize = unders.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		AQLObjectHolder &leg = unders.get(i);		
		AQLString inputType = dynamic_cast<const AQLDataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			AQLDataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				// cashlet loop
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					AQLDataHolder &attrCoupons = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
					if (attrCoupons.isDefined() && !attrCoupons.isNull())
					{
						AQLDataMultiReference &coupons = dynamic_cast<AQLDataMultiReference &>(attrCoupons.get());
						const unsigned int couponSize = coupons.getSize();
						// coupon loop
						for (unsigned int k = 0; k < couponSize; ++k)
						{
							// get ccy
							AQLDataHolder &attrIndexs = coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								AQLDataMultiReference &indexs = dynamic_cast<AQLDataMultiReference &>(attrIndexs.get());
								const unsigned int indexSize = indexs.getSize();
								// index loop
								for (unsigned int l = 0; l < indexSize; ++l)
								{

									AQLDataHolder &attrfixingdate = indexs.get(l).getData(PRICING_DATA_FIXINGDATE, NOCHECK);
									if (attrfixingdate.isDefined() && !attrfixingdate.isNull())
									{
										// get fixingdate
										const AQLDate fixingdate = dynamic_cast<const AQLDataDate &>(attrfixingdate.get()).get();
										if (asOfDate > fixingdate)
										{
											// get index name
											const AQLDataHolder &attrIndexName = indexs.get(l).getData(PRICING_DATA_INDEXNAME, NOCHECK);
											if (!attrIndexName.isDefined() || attrIndexName.isNull()) continue;								
											const AQLString& indexName = dynamic_cast<const AQLDataString &>(attrIndexName.get()).get();

											double fixedrate;
											bool isFound = false;
											for (unsigned int ii = 0; ii < ratesMatrix.size(); ++ii)
											{
												if (indexName != ratesMatrix[ii][0]) continue;

												AQLStringVector strDateVec = ratesMatrix[ii][1].toToken(MULTI_STATIC_DATA_DELIMITER);
												AQLStringVector strRateVec = ratesMatrix[ii][2].toToken(MULTI_STATIC_DATA_DELIMITER);
												for (unsigned int jj = 0; jj < strDateVec.size(); ++jj)
												{
													AQLDate date(strDateVec[jj].getCString());
													if (date == fixingdate)
														{
															fixedrate = strRateVec[jj].getDoubleValue();
															isFound = true;

															//change object
															AQLObject* pindex = &indexs.get(l).get();
															//remove
															pindex->remove(PRICING_DATA_FIXEDRATE);
															//add
															pindex->add(PRICING_DATA_FIXEDRATE, new AQLDataDouble(fixedrate));
															break;
													    }		
												}
												if (isFound) break;
											}
										}

									}
								}
							}
						}
					}
				}
			}
		}
	}

	return;

}

void AQLObjectConfiguration::setUpCompoundingFixedRates(const AQLDate& asOfDate, AQLObjectHolder& ehTrade) const
{
	// read fixing rate file
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString ratefile = staticData.getStaticData(KEY_DEAL_RATE_FILE);
	if (ratefile == AQ_NO_DATA)
	{
		return;
	}
	AQLStringMatrix ratesMatrix;
	AQLFileAccessor ratesFile(AQLMarketData::getNumFileName(ratefile));
	ratesFile.readAllData(MARKET_DATA_DELIMITER,ratesMatrix);
	ratesFile.close();

	const AQLDataHolder& attrLegs = ehTrade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (!attrLegs.isDefined() || attrLegs.isNull())
	{
		return;
	}
	const AQLDataMultiReference &legs = dynamic_cast<const AQLDataMultiReference &>(attrLegs.get());
	const unsigned int legSize = legs.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		AQLObjectHolder &leg = legs.get(i);		
		AQLString inputType = dynamic_cast<const AQLDataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			AQLDataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				unsigned int counter = 0;
				bool isCompounding = false;
				// cashlet loop
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					// check rangeaccrue info
					AQLDataHolder &attrIsCP = cashlets.get(j).getData(PRICING_DATA_ISCOMPOUNDINGCOUPON, NOCHECK);
					if (attrIsCP.isDefined() && !attrIsCP.get().isNull())
					{
						if (attrIsCP.get().convertToString().toUpper() == "TRUE")
						{
							isCompounding = true;
							// check payment date
							const AQLDate &paymentDate = dynamic_cast<const AQLDataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
							if (paymentDate > asOfDate)
								break;
							counter++;
						}
					}
					else
					{
						counter++;
						isCompounding = false;
					}
				}
				if (isCompounding)
				{
					if (counter >= cashlets.getSize())
					{
						continue;
					}

					// check rangeaccrue info
					AQLDataHolder &attrIsCP = cashlets.get(counter).getData(PRICING_DATA_ISCOMPOUNDINGCOUPON, NOCHECK);
					if (!attrIsCP.isDefined() || attrIsCP.isNull())
						throw AQLCoreInvalidData("IsCompoundingCoupon is not registed.",__FILE__,__LINE__);

					// check asofdate								
					const AQLDate &startDate = dynamic_cast<const AQLDataDate &>
						(cashlets.get(counter).getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get();
					const AQLDate &endDate = dynamic_cast<const AQLDataDate &>
						(cashlets.get(counter).getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL).get()).get();
					if (asOfDate < startDate || endDate < asOfDate)
					{
						continue;
					}

					//set observationdates and observationrates
					AQLDataHolder &attrInfos = cashlets.get(counter).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
					if (attrInfos.isDefined() && !attrInfos.isNull()) // multi index case
					{
						AQLDataMultiReference &cpinfos = dynamic_cast<AQLDataMultiReference &>(attrInfos.get());
						const unsigned int cpinfoSize = cpinfos.getSize();

						// info loop
						for (unsigned int kk = 0; kk < cpinfoSize; ++kk)
						{
							AQLDataHolder &attrIndexs = cpinfos.get(kk).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								AQLDataMultiReference &indexs = dynamic_cast<AQLDataMultiReference &>(attrIndexs.get());
								AQLObjectConfiguration::setUpIndexFixedRates(ratesMatrix, indexs);
							}
						}
					}
				}
				else
				{
					// cashlet loop
					for (unsigned int j = 0; j < cashletSize; ++j)
					{
						// check asofdate
						AQLDate startDate, endDate;
						AQLDataHolder *attrStartDate = &cashlets.get(j).getData(PRICING_DATA_CFCALCSTARTDATE, NOCHECK);
						if (attrStartDate->isDefined() && !attrStartDate->isNull())
						{
							startDate = dynamic_cast<const AQLDataDate &> (attrStartDate->get());
						}
						else
						{
							continue;
						}

						AQLDataHolder *attrEndDate = &cashlets.get(j).getData(PRICING_DATA_CFCALCENDDATE, NOCHECK);
						if (attrEndDate->isDefined() && !attrEndDate->isNull())
						{
							endDate = dynamic_cast<const AQLDataDate &> (attrEndDate->get());
						}
						else
						{
							continue;
						}

						if (asOfDate < startDate || endDate < asOfDate)
						{
							continue;
						}

						//set observationdates and observationrates
						AQLDataHolder &attrInfos = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
						if (attrInfos.isDefined() && !attrInfos.isNull()) // multi index case
						{
							AQLDataMultiReference &cpinfos = dynamic_cast<AQLDataMultiReference &>(attrInfos.get());
							const unsigned int cpinfoSize = cpinfos.getSize();

							// info loop
							for (unsigned int kk = 0; kk < cpinfoSize; ++kk)
							{
								AQLDataHolder &attrIndexs = cpinfos.get(kk).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
								if (attrIndexs.isDefined() && !attrIndexs.isNull())
								{
									AQLDataMultiReference &indexs = dynamic_cast<AQLDataMultiReference &>(attrIndexs.get());
									AQLObjectConfiguration::setUpIndexFixedRates_ObsvOpr(ratesMatrix, indexs);
								}
							}
						}
					}
				}
			}
		}
	}
}

/*!
    @brief set up scale and SVD tolerance for  LSMC

	@param[out] objPool
	@param[out] trade
*/

void
AQLObjectConfiguration::setUpSVDToleranceForLSMC(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{

	AQLDataHolder* dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	AQLObject& callinfo = dynamic_cast<AQLDataReference &>(dh->get()).get().get();

	AQLString tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_SVDTOLERANCE).toUpper();
	callinfo.remove(PRICING_DATA_SVDTOLERANCE);
	if (tmp != AQ_NO_DATA)
	{
		callinfo.add(PRICING_DATA_SVDTOLERANCE, new AQLDataDouble()).convertFromString(tmp);
	}
}

/*!
    @brief set up nominal funding leg of strctured bond in an asset swap method.

	@param[out] objPool
	@param[out] trade
*/

void
AQLObjectConfiguration::setUpBondFundingLeg(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{
	AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);

	AQLDataHolder* dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (!dh->isDefined() || dh->isNull())
		return;


	// find bond funding leg
	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference&>(dh->get());
	unsigned int legSize = legs.getSize();
	vector<unsigned int> bond_funding_leg_pos;
	for (unsigned int i = 0; i < legSize; ++i)
	{
		dh = &legs.get(i).getData(PRICING_DATA_ISBONDFUNDINGLEG, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			if (dynamic_cast<AQLDataBool&>(dh->get()).get())
			{
				bond_funding_leg_pos.push_back(i);
			}
		}
	}

	if(bond_funding_leg_pos.size() == 0)
		return;


	// cashflow generator without modifying trade
	trade.AQLObject::add(PRICING_DATA_DONOTMODIFYTRADE, new AQLDataBool(true));

	dh = &trade.getData(PRICING_DATA_CFGENERATOR, ISNOTNULL);
	AQLDataProcedure& modelDataObj = dynamic_cast<AQLDataProcedure&>(dh->get());
	modelDataObj.calibrateModel(asOfDate);

	trade.AQLObject::remove(PRICING_DATA_DONOTMODIFYTRADE);
	for (unsigned int i = 0; i < bond_funding_leg_pos.size(); ++i)
	{
		dynamic_cast<AQLDataString&>(legs.get(bond_funding_leg_pos[i]).getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).set("MANUAL");
	}


	// Bond funding special treatment:
	// Set -9999 to fixed rate of already fixed index 
	// so that set up for extra libor will be invoked later.
	for (unsigned int i = 0; i < bond_funding_leg_pos.size(); ++i)
	{
		dh = &legs.get(bond_funding_leg_pos[i]).getData(PRICING_DATA_CASHLETS, ISNOTNULL);
		AQLDataMultiReference& cashs = dynamic_cast<AQLDataMultiReference&>(dh->get());
		for (unsigned int j = 0; j < cashs.getSize(); ++j)
		{
			const AQLDate& paymentdate = dynamic_cast<const AQLDataDate&>(cashs.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get());
			dh = &cashs.get(j).get().getData(PRICING_DATA_COUPONINFOS, NOCHECK);
			if (!dh->isDefined() || dh->isNull())
				continue;
			AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference&>(dh->get());
			for (unsigned int k = 0; k < coupons.getSize(); ++k)
			{
				dh = &coupons.get(k).get().getData(PRICING_DATA_INDEXINFOS, NOCHECK);
				if (!dh->isDefined() || dh->isNull())
					continue;
				AQLDataMultiReference& indices = dynamic_cast<AQLDataMultiReference&>(dh->get());
				for (unsigned int l = 0; l < indices.getSize(); ++l)
				{
					const AQLDate& fixingdate = dynamic_cast<const AQLDataDate&>(indices.get(l).getData(PRICING_DATA_FIXINGDATE, ISNOTNULL).get());
					if(fixingdate > asOfDate)
						continue;

					AQLObject& targetindexinfo = indices.get(l).get();
					AQLDataString& attrIndexType = dynamic_cast<AQLDataString&>(targetindexinfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get());
					AQLString indexType = attrIndexType.get();
					indexType.toUpper();
					if (indexType != "FIXEDRATE" && indexType != "LIBOR")
						continue;
				
					targetindexinfo.remove(PRICING_DATA_FIXEDRATE);
					if (paymentdate > asOfDate)
					{
						targetindexinfo.add(PRICING_DATA_FIXEDRATE, new AQLDataDouble(EXTRA_FIXEDRATE));
					}
					else
					{
						targetindexinfo.add(PRICING_DATA_FIXEDRATE, new AQLDataDouble(0.));
					}
					attrIndexType.set("FixedRate");
				}
			}
		}
	}

}


