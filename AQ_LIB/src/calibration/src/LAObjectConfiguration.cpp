
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LACoreGridConfiguation.h"
#include "LAObjectConfiguration.h"
#include "LAString.h"
#include "LADataInstance.h"
#include "LAObjectPool.h"
#include "LAObjectHolder.h"
#include "LARatesSDEBase.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataProcedure.h"
#include "LAPriceTradeValue.h"
#include "LAPriceLSMCTradeValue.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"
#include "LAMathAttrSDE.h"
#include "LAMathIndexEntity.h"
#include "LAPriceDataInterpolation.h"
#include "LAMathValuableEntity.h"
#include "LAPriceDataRand.h"
#include "LAPricePortfolioValue.h"
#include "LADefinitions.h"
#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LAMarketData.h"
#include "LADealUtils.h"
#include "LASobol.h"
#include "LAPriceDataManager.h"
#include "LALinearRatesOptionValue.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LAPriceDataFunction.h"
#include "LAPriceCouponForDigital2.h"
#include "LACalibrateModel.h"
#include "LAModelConfiguration.h"


#define CPN		"CPN"
#define CPNCF	"CPNCF"
#define LIBOR	"LIBOR"

#define SWAP "SWAP"

#define DF2 "DF2"


using namespace std;
// constructor
/*!

*/
LAObjectConfiguration::LAObjectConfiguration()
: mpStaticData(&LACoreDataService::getStaticDataManager().getStaticData())
{
}

// destructor
/*!

*/
LAObjectConfiguration::~LAObjectConfiguration(void)
{
}


// 
/*!
    @brief setup object

	@param[out] dataInstance
*/
void
LAObjectConfiguration::setUpEntityes(LADataInstance &dataInstance) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	// setup valuable object
	setUpValuableEntity(objPool);
	// setup path object
	//setUpPathEntity(objPool);
	// setup fx object
	//setUpFXEntity(objPool);

	// setup Future Fixing object
    LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
    LAString isFixing = staticData.getStaticData(KEY_DEAL_ISFIXINGRATE);
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
LAObjectConfiguration::setUpValuableEntity(LAObjectPool &objPool) const
{
	LAString mTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(mTradeName, ENCHKTYPE_ISDEFINED);

	LAString mcNum = mpStaticData->getStaticData(KEY_SIMULATION_MCNUM);
	LAString isDetail =  mpStaticData->getStaticData(KEY_SIMULATION_ISDETAILOUTPUT);

    LAString zeroCalc = LACoreDataService::getContext(ARG_KEY_ZEROCALC);
    if (zeroCalc == MLIB_NO_DATA) {
        zeroCalc = "FALSE";
    }

	if (dynamic_cast<const LADataValuation &>
		(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)

	{
		//for risk currency mode
		LAString fxName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
		if (fxName != MLIB_NO_DATA)
		{
			objHolder.get().remove(PRICING_DATA_FXRATE);
			objHolder.get().add(PRICING_DATA_FXRATE, new LADataReference()).convertFromString(fxName);
		}

		const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());

		const unsigned int size = unders.getSize();
		for (unsigned int i = 0; i < size; ++i)
		{
			LAObjectHolder &trade = unders.get(i);
			// set is detailouput
			trade.remove(PRICING_DATA_ISDETAILOUTPUT);
			trade.add(PRICING_DATA_ISDETAILOUTPUT, new LADataBool()).convertFromString(isDetail);
		    // set mc
			trade.remove(PRICING_DATA_MCNUM);
			trade.add(PRICING_DATA_MCNUM, new LADataInt()).convertFromString(mcNum);
			// set lsmc if defined
			if (dynamic_cast<const LADataValuation &>
				(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_LSMCTRADEVALUE)
			{
				LAString lsmcNum = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_MCNUM);
				trade.remove(PRICING_DATA_LSMCNUM);
				trade.add(PRICING_DATA_LSMCNUM, new LADataInt()).convertFromString(lsmcNum);
			}
		
			// ! Set isResultOut
			if ( LACoreDataService::getContext( ARG_KEY_RESULTOUT ) != MLIB_NO_DATA )
			{
				trade.remove( PRICING_DATA_ISRESULTOUTPUT );
				trade.add( PRICING_DATA_ISRESULTOUTPUT, new LADataBool( true ) );
			}

			if(LACoreDataService::getContext(ARG_KEY_COMPOUNDEDRATEOUT) != MLIB_NO_DATA)
			{
				trade.remove(PRICING_DATA_COMPOUNDED_RATE_OUT);
				trade.add(PRICING_DATA_COMPOUNDED_RATE_OUT, new LADataBool(true));
			}
			

            // set no calculation
            trade.remove(PRICING_DATA_ZEROCALC);
            trade.add(PRICING_DATA_ZEROCALC, new LADataBool()).convertFromString(zeroCalc);

			//for past libor auto fixing
			trade.remove(PRICING_DATA_ISSAVEPASTFIXING);
			trade.add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(true));

			// set RangeAccrue fixed rates
			LADate asOfDate = LAMarketData::getAsofDate(objPool);
			LAObjectConfiguration::setUpRangeAccrueFixedRates(asOfDate, trade);
			LAObjectConfiguration::setUpCompoundingFixedRates(asOfDate, trade);
		}
		
		LADataInstance *dataInstance = objHolder.getDataInstance();
		dataInstance->getReferencePool().completeDependency();
	}
	else
	{

		// set is detailouput
		objHolder.remove(PRICING_DATA_ISDETAILOUTPUT);
		objHolder.add(PRICING_DATA_ISDETAILOUTPUT, new LADataBool()).convertFromString(isDetail);
		// set mc num
		objHolder.remove(PRICING_DATA_MCNUM);
		objHolder.add(PRICING_DATA_MCNUM, new LADataInt()).convertFromString(mcNum);		
		// set lsmc if defined
		if (dynamic_cast<const LADataValuation &>
			(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_LSMCTRADEVALUE)
		{
			LAString lsmcNum = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_MCNUM);
			objHolder.remove(PRICING_DATA_LSMCNUM);
			objHolder.add(PRICING_DATA_LSMCNUM, new LADataInt()).convertFromString(lsmcNum);
		}

		// ! Set isResultOut
		if ( LACoreDataService::getContext( ARG_KEY_RESULTOUT ) != MLIB_NO_DATA )
		{
			objHolder.remove( PRICING_DATA_ISRESULTOUTPUT );
			objHolder.add( PRICING_DATA_ISRESULTOUTPUT, new LADataBool( true ) );
		}

        if(LACoreDataService::getContext(ARG_KEY_COMPOUNDEDRATEOUT) != MLIB_NO_DATA)
        {
            objHolder.remove(PRICING_DATA_COMPOUNDED_RATE_OUT);
            objHolder.add(PRICING_DATA_COMPOUNDED_RATE_OUT, new LADataBool(true));
        }
		

		// set no calculation
		objHolder.remove(PRICING_DATA_ZEROCALC);
		objHolder.add(PRICING_DATA_ZEROCALC, new LADataBool()).convertFromString(zeroCalc);

		//for past libor auto fixing
		objHolder.remove(PRICING_DATA_ISSAVEPASTFIXING);
		objHolder.add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(true));

		// set RangeAccrue fixed rates
		LADate asOfDate = LAMarketData::getAsofDate(objPool);
		LAObjectConfiguration::setUpRangeAccrueFixedRates(asOfDate, objHolder);
		LAObjectConfiguration::setUpCompoundingFixedRates(asOfDate, objHolder);
	}
}


// 
/*!
    @brief setup risk info object

	@param[out] LADataInstance dataInstance
*/
void
LAObjectConfiguration::setUpRiskInfo(LADataInstance &dataInstance) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAString mTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(mTradeName, ENCHKTYPE_ISDEFINED);

	const LAString riskENames_str = LACoreDataService::getContext(CONTEXT_KEY_RISKENTITY_NAMES);
	const LAStringVector riskENames = riskENames_str.toToken(':');
	vector<const LAObject *> riskEVec;
	for (unsigned int i = 0; i < riskENames.size(); ++i)
	{
		if (riskENames[i] != MLIB_NO_DATA)
		{
			riskEVec.push_back(&objPool.getObject(riskENames[i], ENCHKTYPE_ISDEFINED).get());
		}
	}

	if (dynamic_cast<const LADataValuation &>
		(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)

	{
		//for risk currency mode
		LAString fxName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
		if (fxName != MLIB_NO_DATA)
		{
			objHolder.get().remove(PRICING_DATA_FXRATE);
			objHolder.get().add(PRICING_DATA_FXRATE, new LADataReference()).convertFromString(fxName);
		}

		const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());

		const unsigned int size = unders.getSize();
		for (unsigned int i = 0; i < size; ++i)
		{
			LAObjectHolder &trade = unders.get(i);			
			setUpRiskInfo(riskEVec, trade.get());
		}
		
		if (!riskEVec.empty())
		{
			objHolder.remove(PRICING_DATA_RISKCALCINFOS);
			objHolder.add(PRICING_DATA_RISKCALCINFOS, new LADataMultiReference()).convertFromString(riskENames_str);
		}
	}
}


void 
LAObjectConfiguration::setUpRiskInfo(vector<const LAObject *> &riskEVec,  LAObject &e) const
{
	// set risk info
	for (unsigned int i = 0; i < riskEVec.size(); ++i)
	{
		const LAStringVector &riskAttrNames = dynamic_cast<const LADataStrings &>(riskEVec[i]->getData(AP_CALIBRATION_DATA_RISK_TRADESETDATA).get()).get();
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
LAObjectConfiguration::setUpPathEntity(LAObjectPool &objPool, const LAString *pPathName) const
{

	LAMathPathEntity *path = 0;
	if (pPathName)
	{
		path = &dynamic_cast<LAMathPathEntity &>(objPool.getObject(*pPathName, ENCHKTYPE_ISDEFINED).get());
	}
	else
	{
		path = LAMarketData::getPathEnitty(objPool);
	}
	if (!path)
	{
		throw LACoreInvalidData("Path object is not set in entitiy pool", __FILE__, __LINE__);
	}
	LAString fxName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	// set antithetic
	LAString isAntithetic =  mpStaticData->getStaticData(KEY_SIMULATION_ISANTITHETIC);
	if (isAntithetic.toUpper() == "TRUE")
	{
		path->setAntithetic(true);
	}
	// set brownian bridge
	LAString isBrownianBridge =  mpStaticData->getStaticData(KEY_SIMULATION_ISBROWNIANBRIDGE);
	if (isBrownianBridge.toUpper() == "TRUE")
	{
		path->setBrownianBridge(true);
	}

	// set sdenames and models
	LAStringVector ccys = MADealUtils::getSDECurrencys(true);
	unsigned int ccySize = ccys.size();
	LAStringVector sdeNames(ccySize);
	LAStringVector models(ccySize);
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		sdeNames[i] = LAMarketData::getSDEName(ccys[i]);
		models[i] = LAMarketData::getModelName(ccys[i]);
		models[i].toUpper();
	}
	path->getSDEAttrNames().set(sdeNames);

	//set simsdenames
	LAStringVector simccys = MADealUtils::getSimulationSDECurrencys(true);
	unsigned int simccySize = simccys.size();
	LAStringVector simsdeNames(simccySize);
	for (unsigned int i = 0; i< simccySize; i++)
	{
		simsdeNames[i] = LAMarketData::getSDEName(simccys[i]);;
	}
	path->getSimulationSDEAttrNames().set(simsdeNames);

	// set sdetimegrid
	DoubleArray sdeTimeGrid;
// update for XLL Plus ////////////////////////////////////////////////////////////////////////////
	//getSDETimeGrid(sdeTimeGrid);
	//path->getSDETimeGrid().set(sdeTimeGrid);
	LAString timegridContext = LACoreDataService::getContext(CONTEXT_KEY_SDE_TIMEGRID);
	if (timegridContext==MLIB_NO_DATA)
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

	//LAString mainModel = getModelName();
	//mainModel.toLower();
	//// set sdedividnum
	//LAString divNum = mpStaticData->getStaticData("sde." + mainModel + ".integral.divnum");

	//if (divNum != MLIB_NO_DATA)
	//{
	//	path->getSDEIntegralDivNum().convertFromString(divNum);
	//}

	//// set sdedividnum
	//LAString integralTimeGridStr = mpStaticData->getStaticData("sde." + mainModel + ".integral.timegrid");

	//if (integralTimeGridStr != MLIB_NO_DATA)
	//{
	//	LADataDoubles tmpAttr;
	//	tmpAttr.convertFromString(integralTimeGridStr);
	//	DoubleArray integralTimeGrid = tmpAttr.get();
	//	integralTimeGrid.push_back(sdeTimeGrid.back());
	//	path->getSDEIntegralTimeGrid().set(integralTimeGrid);
	//}

	// set rand generator
	LAString randGenerator =  mpStaticData->getStaticData(KEY_SIMULATION_RADNTYPE);
	path->getRand().convertFromString(randGenerator);

	// set initial value
	// first ccy 
	LAString firstccy(ccys[0]);
	LAString initialValue =  PREFIX_YIELD + mpStaticData->getStaticData(firstccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	LAString ycProNames = LAMarketData::getBaseYieldProName(ccys[0]);
	for (unsigned int i = 1; i < ccySize; ++i)
	{
		LAString tmpccy(ccys[i]);
		// check vol or fx
		if (ccys[i].findString(POSTFIX_VOL) >= 0)
		{
			// initial value is saved in the object of spot sde volatility
			tmpccy.remove(tmpccy.findString(POSTFIX_VOL), LAString(POSTFIX_VOL).size());
			initialValue += ":" + LAString(PREFIX_VOL) + mpStaticData->getStaticData(tmpccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		}
		else if (ccys[i].findString(FX_DELIMITER) >= 0)
		{
			initialValue += ":" + fxName;
		}
		else
		{
			initialValue += ":" + LAString(PREFIX_YIELD) + mpStaticData->getStaticData(tmpccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
			ycProNames += ":" + LAMarketData::getBaseYieldProName(ccys[i]);
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

	LAStaticData &propG = LACoreDataService::getStaticDataManager().getIRGStaticData();
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
		LAMarketData::getSDECorrelation(simccys, corData);
		path->getCorrelationMatrix().set(corData);
	}
	else if (ccySize > 1 && simccySize == 0)
	{
		DoubleMatrix corData;
		LAMarketData::getSDECorrelation(ccys, corData);
		path->getCorrelationMatrix().set(corData);
	}


	// set sde data
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		LAString tmpccy(ccys[i]);
		LAString tmpmodel(models[i]);
		// set sde data
		LAString sdeName;
		LAString type;
		LAString sdeFuncName;
		// check type
		if (ccys[i].findString(POSTFIX_VOL) >= 0)
		{
			tmpccy.remove(tmpccy.findString(POSTFIX_VOL), LAString(POSTFIX_VOL).size());
			tmpmodel.remove(models[i].findString(POSTFIX_VOL), LAString(POSTFIX_VOL).size());
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

		LAString sdeAttrValue = type + ":" + ccys[i].toUpper() + ":" + sdeFuncName;
		LAMathAttrSDE *sde = new LAMathAttrSDE();
		sde->convertFromString(sdeAttrValue);
		path->LAObject::remove(sdeName);
		path->LAObject::add(sdeName ,sde);

		if (type == "IR" && LAMarketData::isCorUse(models[i]))
		{
			// set correlation
			LAString corName = PREFIX_COR + sdeName;
			LADataReference *corRef = new LADataReference();
			corRef->convertFromString(corName);
			path->LAObject::remove(corName);
			path->LAObject::add(corName ,corRef);
		}

		// set volatility
		LAString volName = PREFIX_VOL + sdeName;
		LADataReference *volRef = new LADataReference();
		volRef->convertFromString(volName);
		path->LAObject::remove(volName);
		path->LAObject::add(volName ,volRef);
	}
}

// 
/*!
    @brief setup fx object

	@param[out] object pool
*/
void
LAObjectConfiguration::setUpFXEntity(LAObjectPool &objPool) const
{
	LAStringVector ccys = MADealUtils::getSDECurrencys();
	unsigned int ccySize = ccys.size();

	if (ccySize == 1)
	{
		// single ccy no fx object
		return ;
	}

	vector<LAMathFXEntity *> fxs;
	EntityIter it = objPool.begin();
	while (it != objPool.end())
	{
		if (it->second.isTypeOf(ENTITY_FX))
		{
			fxs.push_back(&dynamic_cast<LAMathFXEntity &>(it->second.get()));
		}
		++it;
	}

	LAString yieldCurves;

	const LAStringVector &fxCurrencys = fxs[0]->getCurrencys().get();
	const unsigned int fxCSize = fxCurrencys.size();
	vector<int> sdePos(fxCSize);

	// set sde position
	int usd_pos_no_sde = -1;
	for (unsigned int i = 0; i < fxCSize; ++i)
	{
		LAStringVector::const_iterator it = find(ccys.begin(), ccys.end(), fxCurrencys[i]);
		if (it == ccys.end())
		{
			if (fxCurrencys[i] != CURRENCY_USD)
			{
				LAString tmpccy = fxCurrencys[i];
				yieldCurves += (PREFIX_YIELD + mpStaticData->getStaticData(tmpccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME) + ":");
				continue;
				//throw LACoreInvalidData(" FX currency (except USD) must setted as SDE !!", __FILE__, __LINE__);
			}
			else
			{
				LAString tmpUSD = CURRENCY_USD;
				yieldCurves += (PREFIX_YIELD + mpStaticData->getStaticData(tmpUSD.toLower() + STATIC_DATA_FX_KEY_SDE_NAME) + ":");
				sdePos[i] = usd_pos_no_sde;
				continue;
			}
		}
		sdePos[i] = static_cast<int>(it - ccys.begin());

		LAString tmpccy = ccys[sdePos[i]];
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
			LACalibrateModel *generatorTmp = LAModelConfiguration::getInstance()->
						createSDEGenerator(LAMarketData::getModelName(fxCurrencys[i]));
		// update for XLL Plus //////////////////////////////////// 
		/*
			generatorUSD->loadModelDataAndCalibrate(CURRENCY_USD, dataInstance);
		*/

			LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
			LAString temp = staticData.getStaticData(KEY_SDE_XCCY_USD_IS_GENERATE_ONLY_CURVE);
			if (temp==MLIB_NO_DATA)
				temp = "FALSE";
			LADataBool isOnlyCurve;
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
LAObjectConfiguration::setUpExtraEntity(LAObjectPool &objPool) const
{
	/////////////////////////////
	// setup extra object
	/////////////////////////////
	setUpFwdInterpolation(objPool);
	// set forward rate for Libor index which has -9999 fixingrate 
	LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);

	if (dynamic_cast<const LADataValuation &>
	(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			//complete bond funding leg
			setUpBondFundingLeg(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));

			//set up asofdate for trade object
			setUpAsOfDateForTradeEntity(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			//set up past coupon and past coupon payoffs for tarn product
			setUpPastCouponPayOffs(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			//set up LSMC coterm
			setUpCoTermOfLSMCIndex(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			//set up discount curve
			setUpDiscountCurve(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));

			//set FundingSpread 
			setUpFundingSpread(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			setUpExtraLibor(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			//set recalctradedata flag for each tradeentity			 
			// update for XLL Plus ////////////////////////////////////
            // setUpRecalcTrade(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
            LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
            LAString temp = staticData.getStaticData(KEY_SIMULATION_FUNDING_CHANGED);
            if (temp==MLIB_NO_DATA){
                setUpRecalcTrade(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
            }
            ///////////////////////////////////////////////////////////
			setUpDigitalCallSpreadCoupon(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			// set up dirty price
			setUpDirtyPrice(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			// setUpPastIndexRate
	        setUpPastIndexRate(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			// setUpScaleShftOfLSMC
	        setUpScaleShftOfLSMC(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			// setUpSVDToleranceForLSMC
			setUpSVDToleranceForLSMC(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
		}
	}
	else
	{
		//complete bond funding 
		setUpBondFundingLeg(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));

		//set up asofdate for trade object
		setUpAsOfDateForTradeEntity(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		//set up past coupon and past coupon payoffs for tarn product
		setUpPastCouponPayOffs(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		//set up LSMC coterm
		setUpCoTermOfLSMCIndex(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		//set up discount curve
		setUpDiscountCurve(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));

		//set FundingSpread 
		setUpFundingSpread(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		setUpExtraLibor(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		//set recalctradedata flag for each tradeentity
		// update for XLL Plus ////////////////////////////////////
        // setUpRecalcTrade(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
        LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
        LAString temp = staticData.getStaticData(KEY_SIMULATION_FUNDING_CHANGED);
        if (temp==MLIB_NO_DATA){
            setUpRecalcTrade(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
        }
        /////////////////////////////////////////////////
		setUpDigitalCallSpreadCoupon(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		// set up dirty price
		setUpDirtyPrice(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		// setUpPastIndexRate
		setUpPastIndexRate(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		// setUpScaleShftOfLSMC
		setUpScaleShftOfLSMC(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		// setUpSVDToleranceForLSMC
		setUpSVDToleranceForLSMC(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));
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
LAObjectConfiguration::setUpExtraLibor(LAObjectPool &objPool,  LAMathObjectValue &trade) const
{
	/////////////////////////////
	// set extra libor rate
	/////////////////////////////
	LADate asOfDate = LAMarketData::getAsofDate(objPool);

	LAPriceDataDayCount dayCount(ACT_360);
	const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
										(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	const unsigned int legSize = unders.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		LAObjectHolder &leg = unders.get(i);		
		LAString inputType = dynamic_cast<const LADataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			LADataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				// cashlet loop
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					LADataHolder &attrCoupons = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
					if (attrCoupons.isDefined() && !attrCoupons.isNull())
					{
						LADataMultiReference &coupons = dynamic_cast<LADataMultiReference &>(attrCoupons.get());
						const unsigned int couponSize = coupons.getSize();
						// coupon loop
						for (unsigned int k = 0; k < couponSize; ++k)
						{
							// get ccy
							const LAString &ccy = dynamic_cast<const LADataString &>(coupons.get(k).getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
							LADataHolder &attrIndexs = coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								LADataMultiReference &indexs = dynamic_cast<LADataMultiReference &>(attrIndexs.get());
								const unsigned int indexSize = indexs.getSize();
								// index loop
								for (unsigned int l = 0; l < indexSize; ++l)
								{
									LADataHolder &attrFixedRate = indexs.get(l).getData(PRICING_DATA_FIXEDRATE, NOCHECK);
									if (attrFixedRate.isDefined() && !attrFixedRate.isNull())
									{
										const double fixedRate = dynamic_cast<const LADataDouble &>(attrFixedRate.get()).get();
										// type
										LAString indexType = indexs.get(l).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).convertToString();
										indexType.toUpper();
										indexType.exchange("\"", "");
										// check
										if (fixedRate == EXTRA_FIXEDRATE && (indexType == "FIXEDRATE" || indexType == "LIBOR"))
										{	
											// calc start date, 3 days after asofdate (sliding=following  calendar=TkB)
											LAPriceDataSlidingRule sliding; // sliding
											sliding.convertFromString(SLIDING_FOLLOWING);
											LAPriceDataCalendar cal;  // calendar
											cal.convertFromString(CITY_TkB);
											const LADate startDate = LAMathDateCalculations::getDate(asOfDate, "3D", sliding, &cal, true);
											// get yield object
											const LAString yieldName = LAMarketData::getBaseYieldName(ccy);
											const LAMathYieldCurve &curve = dynamic_cast<const LAMathYieldCurve &>(objPool.getObject(yieldName, ENCHKTYPE_ISDEFINED).get());

											//modify get cfstartdate and paymentdate
											const LADate &cfstartDate = dynamic_cast<const LADataDate &>(cashlets.get(j).getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get();
											const LADate &paymentDate = dynamic_cast<const LADataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
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
												dynamic_cast<LADataDouble &>(attrFixedRate.get()).set(rate);
											}
											else
											{
												// set rate zero
												dynamic_cast<LADataDouble &>(attrFixedRate.get()).set(0.0);
											}

											//modify spread
											DoubleVector spreads = dynamic_cast<LADataDoubles &>(coupons.get(k).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).get();
											if (spreads.size() != 2)
												throw LACoreInvalidData("Coefficient must be size 2",__FILE__,__LINE__);

											spreads[1] *= ratio;
											dynamic_cast<LADataDoubles &>(coupons.get(k).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).set(spreads);
											

											//record a change libor
											indexs.get(l).remove("IsExtraLibor");
											indexs.get(l).add("IsExtraLibor", new LADataBool(true));
											indexs.get(l).remove("TermRatio");
											indexs.get(l).add("TermRatio", new LADataDouble(ratio));


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
LAObjectConfiguration::setUpFutureFixingEntity(LAObjectPool &objPool) const
{
	/////////////////////////////
	// setup future fixing object
	/////////////////////////////
	// set fixing rate for index which has fixingdate prior to asofdate
	LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);

	if (dynamic_cast<const LADataValuation &>
	(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			setUpFutureFixingFunc(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
		}
	}
	else
	{
		setUpFutureFixingFunc(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
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
LAObjectConfiguration::setUpFutureFixingFunc(LAObjectPool &objPool,  LAMathObjectValue &trade) const
{
	/////////////////////////////
	// set future fixing rate
	/////////////////////////////
	LADate asOfDate = LAMarketData::getAsofDate(objPool);
	LAPriceDataDayCount dayCount(ACT_360);
	const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
										(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	const unsigned int legSize = unders.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		LAObjectHolder &leg = unders.get(i);		
		LAString inputType = dynamic_cast<const LADataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			LADataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					LADataHolder &attrCoupons = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
					if (attrCoupons.isDefined() && !attrCoupons.isNull())
					{
						LADataMultiReference &coupons = dynamic_cast<LADataMultiReference &>(attrCoupons.get());
						const unsigned int couponSize = coupons.getSize();
						// coupon loop
						for (unsigned int k = 0; k < couponSize; ++k)
						{
							LADataHolder &attrIndexs = coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								LADataMultiReference &indexs = dynamic_cast<LADataMultiReference &>(attrIndexs.get());
								const unsigned int indexSize = indexs.getSize();
								// index loop
								for (unsigned int l = 0; l < indexSize; ++l)
								{
									LADataHolder &attrfixingdate = indexs.get(l).getData(PRICING_DATA_FIXINGDATE, NOCHECK);
									if (attrfixingdate.isDefined() && !attrfixingdate.isNull())
									{
										// get fixingdate
										const LADate fixingdate = dynamic_cast<const LADataDate &>(attrfixingdate.get()).get();
										if (asOfDate > fixingdate)
										{
											// get fixingrate
											LADataHolder &attrFixedRate = indexs.get(l).getData(PRICING_DATA_FIXEDRATE, NOCHECK);
											if (!attrFixedRate.isDefined() || attrFixedRate.isNull())
											{
												LADataHolder &attrindexinfo = indexs.get(l).getData(CALIBRATION_DATA_NAME, ISNOTNULL);
												const LAString& indexinfo_name = dynamic_cast<const LADataString &>(attrindexinfo.get()).get();
												
												// get fixedrate
												double fixedrate = getFixedRate(KEY_DEAL_FIXINGRATE, indexinfo_name);
												// set fixedrate
												LAObject* pindex = &indexs.get(l).get();
												pindex->add(PRICING_DATA_FIXEDRATE, new LADataDouble(fixedrate));

												// get type
												LADataHolder &attrIndexType = indexs.get(l).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL);
												LAString indexType = attrIndexType.convertToString();
												indexType.toUpper();
												indexType.exchange("\"", "");
												// check type
												if (indexType != "FIXEDRATE")
												{
													dynamic_cast<LADataString&>(attrIndexType.get()).set(PRICING_DATA_FIXEDRATE);
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
				LADataHolder &attrPaymentDates = leg.getData(PRICING_DATA_PAYMENTDATES, NOCHECK);
				if (attrPaymentDates.isDefined() && !attrPaymentDates.isNull())
				{
						if (attrCashlets.isDefined() && !attrCashlets.isNull())
						{
						// set payment dates
							LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(attrCashlets.get());
							const unsigned int cashletSize = cashlets.getSize();
							DateVector paymentdates_tmp;

						// cashlet loop
						for (unsigned int j = 0; j < cashletSize; ++j)
						{
							// get payment date
							const LADate &paymentDate = dynamic_cast<const LADataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
							if (paymentDate > asOfDate)
							{
								break;
							}
							paymentdates_tmp.push_back(paymentDate);
						}
						dynamic_cast<LADataDates &>(attrPaymentDates.get()).set(paymentdates_tmp);
					
						// if payoffs exist
						LADataHolder &attrCouponPayOffs = leg.getData(PRICING_DATA_COUPONPAYOFFS, NOCHECK);
						if (attrCouponPayOffs.isDefined() && !attrCouponPayOffs.isNull())
						{
							DoubleVector couponpayoffs_tmp;
							unsigned int counter = 0;
							// cashlet loop
							for (unsigned int j = 0; j < cashletSize; ++j)
							{
								// get payment date
								const LADate &paymentDate = dynamic_cast<const LADataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
								double couponpayoff = 0.0;
								if (paymentDate > asOfDate)
								{
									break;
								}
								couponpayoffs_tmp.push_back(couponpayoff);
							}
							dynamic_cast<LADataDoubles &>(attrCouponPayOffs.get()).set(couponpayoffs_tmp);
						}

						// if coupons not exist, add coupons in specific condition
						// cashlet loop
						for (unsigned int j = 0; j < cashletSize; ++j)
						{
							LADataHolder &attrCoupons = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
							if (attrCoupons.isDefined() && !attrCoupons.isNull())
							{
								LADataMultiReference &coupons = dynamic_cast<LADataMultiReference &>(attrCoupons.get());
								const unsigned int couponSize = coupons.getSize();
								// coupon loop
								for (unsigned int k = 0; k < couponSize; ++k)
								{
									LADataHolder &attrIndexs = coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
									if (attrIndexs.isDefined() && !attrIndexs.isNull())
									{
										LADataMultiReference &indexs = dynamic_cast<LADataMultiReference &>(attrIndexs.get());
										const unsigned int indexSize = indexs.getSize();
										// index loop
										for (unsigned int l = 0; l < indexSize; ++l)
										{
											// get type
											LADataHolder &attrIndexType = indexs.get(l).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL);
											LAString indexType = attrIndexType.convertToString();
											indexType.toUpper();
											indexType.exchange("\"", "");
											if (indexType == CPN || indexType == CPNCF)
											{
												const LADate& basedate = dynamic_cast<const LADataDate &>(trade.getData(PRICING_DATA_TODAY, ISNOTNULL).get()).get();
												const LADate& observationStartDate = dynamic_cast<const LADataDate &>(indexs.get(l).getData(PRICING_DATA_OBSERVATIONSTARTDATE, ISNOTNULL).get()).get();
												const LADate& observationEndDate = dynamic_cast<const LADataDate &>(indexs.get(l).getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL).get()).get();

												//get past paymentdate
												DateVector needdates;
												const LADataHolder* dh = &(unders.get(0).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
												const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(dh->get());
												for (unsigned int i = 0; i < cashlets.getSize(); i++)
												{
													dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
													if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment			
													dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
													const LADate& paymentdate = dynamic_cast<const LADataDate&>(dh->get()).get();
													if (paymentdate < observationStartDate) continue;
													if (paymentdate > basedate || paymentdate > observationEndDate) break;
													needdates.push_back(paymentdate);
												}
												if (needdates.size() > 0)
												{
													LADataHolder &attrCoupons = leg.getData(PRICING_DATA_COUPONS, NOCHECK);
													if (!attrCoupons.isDefined() || attrCoupons.isNull())
													{
														// add coupons
														LAObject* pleg = &leg.get();
														DoubleVector couponVec;
														couponVec.push_back(0.0);
														pleg->add(PRICING_DATA_COUPONS, new LADataDoubles(couponVec));
													}
												}
											}
										}
									}
								}
							}
						}

						// if coupons exist
						LADataHolder &attrCoupons = leg.getData(PRICING_DATA_COUPONS, NOCHECK);
						if (attrCoupons.isDefined() && !attrCoupons.isNull())
						{
								DateVector paymentdates_tmp;
							DoubleVector coupons_tmp;

								// cashlet loop
								for (unsigned int j = 0; j < cashletSize; ++j)
								{
									// get payment date
									const LADate &paymentDate = dynamic_cast<const LADataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
								double coupon = 0.0;
									if (paymentDate > asOfDate)
									{
										break;
									}
								coupons_tmp.push_back(coupon);
							}
							dynamic_cast<LADataDoubles &>(attrCoupons.get()).set(coupons_tmp);
						}
					}
				}
				// for RangeAccrue fixing
				if (attrCashlets.isDefined() && !attrCashlets.isNull())
				{
					LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(attrCashlets.get());
					const unsigned int cashletSize = cashlets.getSize();
					unsigned int counter = 0;
					LAString rangeaccrue_observationdates;
					LAString rangeaccrue_observationrates;
					bool isRangeAccrue = false;
					// cashlet loop
					for (unsigned int j = 0; j < cashletSize; ++j)
					{
						// check rangeaccrue info
						LADataHolder &attrIsRA = cashlets.get(j).getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK);
						if (attrIsRA.isDefined() && !attrIsRA.get().isNull())
						{
							if (attrIsRA.get().convertToString().toUpper() == "TRUE")
						{
							isRangeAccrue = true;
							// check payment date
							const LADate &paymentDate = dynamic_cast<const LADataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
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
						LADataHolder &attrIsRA = cashlets.get(counter).getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK);
						if (!attrIsRA.isDefined() || attrIsRA.isNull())
							throw LACoreInvalidData("IsRangeAccrue is not registed.",__FILE__,__LINE__);
	
						// get observationdates and rates
						DoubleVector fixedrateVec(0);
						DateVector fixeddateVec(0);
	
						if (counter != 0)
						{
							// get paymentdate before asofdate
							const LADate &paymentDate = dynamic_cast<const LADataDate &>(cashlets.get(counter - 1).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
						
							LADate adddate = paymentDate;
							unsigned int pos = 1;
							while (adddate <= asOfDate)
							{
								fixeddateVec.push_back(adddate);
								fixedrateVec.push_back(0.01);
								adddate.addDays(pos);
							}
						}
	
						//set observationdates and observationrates
						LADataHolder &attrIndexs = cashlets.get(counter).getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL);
						if (attrIndexs.isDefined() && !attrIndexs.isNull())
						{
							LADataMultiReference &raindexs = dynamic_cast<LADataMultiReference &>(attrIndexs.get());
							const unsigned int raindexSize = raindexs.getSize();
							// index loop
							for (unsigned int k = 0; k < raindexSize; ++k)
							{
								// check fixingrate
								LADataHolder &attrFixedRate = raindexs.get(k).getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK);
								if (attrFixedRate.isDefined() && !attrFixedRate.isNull())
								{
									//change object
									LAObject* pindex = &raindexs.get(k).get();
									//remove
									pindex->remove(PRICING_DATA_OBSERVATIONDATES);
									pindex->remove(PRICING_DATA_OBSERVATIONRATES);
									//add
									pindex->add(PRICING_DATA_OBSERVATIONDATES, new LADataDates(fixeddateVec));
									pindex->add(PRICING_DATA_OBSERVATIONRATES, new LADataDoubles(fixedrateVec));
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
LAObjectConfiguration::getFixedRate(const LAString &key, const LAString &index) const
{
    LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString index_tmp = index;
	index_tmp.remove(0, 13);
    LAString fixedrate = staticData.getStaticData(key + "." + index_tmp);

	if (fixedrate == MLIB_NO_DATA)
	{
		fixedrate = staticData.getStaticData(key + ".standard");
		if (fixedrate == MLIB_NO_DATA)
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
LAObjectConfiguration::setUpRecalcTrade(LAObjectPool &objPool,  LAMathObjectValue &trade) const
{
	//set isfunding true
	LADataHolder* dh;
	dh = &(trade.getData(PRICING_DATA_FUNDINGCHANGEINFO,NOCHECK));
	bool ismdy = false;
	if (dh->isDefined() && !dh->isNull())
		ismdy = true;

	if (!ismdy)
		return;

	LAObject& fginfo = dynamic_cast<LADataReference &>(dh->get()).get().get();

	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference &>(dh->get());
	if (legs.getSize() != 2)
		throw LACoreInvalidData("Leg size must be 2",__FILE__,__LINE__);

	//get basecurrency
	LAObject& fdleg = legs.get(1).get();
	bool isbndfnd = false;
	dh = &fdleg.getData(PRICING_DATA_ISBONDFUNDINGLEG, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isbndfnd = dynamic_cast<const LADataBool&>(dh->get());
	}
	LAString basecur;
	if (isbndfnd)
	{
		dh = &fdleg.getData(PRICING_DATA_BONDFUNDINGCURRENCY, ISNOTNULL);
	}
	else
	{
		dh = &(fdleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference& fdcashs = dynamic_cast<LADataMultiReference &>(dh->get());
		LAObject& fdcash0 = fdcashs.get(0).get();
		dh = &(fdcash0.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	}
	basecur = dynamic_cast<LADataString &>(dh->get()).get();
	basecur.toLower();

	//get mdycurrency
	LAObject& strleg = legs.get(0).get();
	dh = &(strleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	LADataMultiReference& strcashs = dynamic_cast<LADataMultiReference &>(dh->get());
	LAObject& strcash0 = strcashs.get(0).get();
	dh = &(strcash0.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	LAString mdycur = dynamic_cast<LADataString &>(dh->get()).get();
	mdycur.toLower();
	
	
	//set base daycount
	LAString basedc = mpStaticData->getStaticData(basecur + STATIC_DATA_KEY_FUNDING_DAYCOUNT).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGDAYCOUNT, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_BASEFUNDINGDAYCOUNT);
		fginfo.LAObject::add(PRICING_DATA_BASEFUNDINGDAYCOUNT, new LAPriceDataDayCount()).convertFromString(basedc);
	}
	
	//set base slidingrule
	LAString basesr = mpStaticData->getStaticData(basecur + STATIC_DATA_KEY_FUNDING_SLIDINGRULE).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGSLIDINGRULE, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_BASEFUNDINGSLIDINGRULE);
		fginfo.LAObject::add(PRICING_DATA_BASEFUNDINGSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(basesr);
	}

	//set base calendar
	LAString basecal = mpStaticData->getStaticData(basecur + STATIC_DATA_KEY_FUNDING_CALENDAR);
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGCALENDAR, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_BASEFUNDINGCALENDAR);
		fginfo.LAObject::add(PRICING_DATA_BASEFUNDINGCALENDAR, new LAPriceDataCalendar()).convertFromString(basecal);
	}

	//set base index daycount
	LAString baseindexdc = mpStaticData->getStaticData(basecur + STATIC_DATA_KEY_FUNDING_DAYCOUNT).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT);
		fginfo.LAObject::add(PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(baseindexdc);
	}

	//set mdy daycount
	LAString mdydc = mpStaticData->getStaticData(mdycur + STATIC_DATA_KEY_FUNDING_DAYCOUNT).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT);
		fginfo.LAObject::add(PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT, new LAPriceDataDayCount()).convertFromString(mdydc);
	}
	
	//set mdy slidingrule
	LAString mdysr = mpStaticData->getStaticData(mdycur + STATIC_DATA_KEY_FUNDING_SLIDINGRULE).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE);
		fginfo.LAObject::add(PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(mdysr);
	}

	//set mdy calendar
	LAString mdycal = mpStaticData->getStaticData(mdycur + STATIC_DATA_KEY_FUNDING_CALENDAR);
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGCALENDAR, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGCALENDAR);
		fginfo.LAObject::add(PRICING_DATA_MODIFIEDFUNDINGCALENDAR, new LAPriceDataCalendar()).convertFromString(mdycal);
	}

	//set mdy index daycount
	LAString mdyindexdc = mpStaticData->getStaticData(mdycur + STATIC_DATA_KEY_FUNDING_DAYCOUNT).toUpper();
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT, NOCHECK));
	if(!dh->isDefined() || dh->isNull())
	{
		fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT);
		fginfo.LAObject::add(PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT, new LAPriceDataDayCount()).convertFromString(mdyindexdc);
	}
}

// 
/*!
    @brief setUpFundingSpread	
	@param[in] object pool
	@param[out] trade object
*/
void
LAObjectConfiguration::setUpFundingSpread(LAObjectPool &objPool,  LAMathObjectValue &trade) const
{
	LADataHolder* dh;

	// check whether the data 'Underlyings' exists
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
	{
		return;
	}

	// get leg size
	const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
										(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	const unsigned int legSize = unders.getSize();

	// set yield curve pro
	dh = &(trade.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAString& ccy = dynamic_cast<const LADataString&> ((trade.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get()).get();
		const LAString& yieldProName = LAMarketData::getBaseYieldProName(ccy);
		trade.remove(PRICING_DATA_YIELDPRONAME);
		trade.LAObject::add(PRICING_DATA_YIELDPRONAME, new LADataString()).convertFromString(yieldProName);
	}

	// get fundingspreadfile
	if (mFdSpdMap.empty())
	{
		//read funding spread file
		LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
		LAString fdspdfile = staticData.getStaticData(KEY_FUNDINGSPREAD_FILE);
		if (LACoreDataService::isFileExist(LAMarketData::getNumFileName(fdspdfile)))
		{
			LAStringMatrix fdspdMatrix;
			MAFileAccessor fdspdFile(LAMarketData::getNumFileName(fdspdfile));
			fdspdFile.readAllData(',',fdspdMatrix);
			if (fdspdMatrix.size() > 0)
			{
				for (unsigned int i = 0; i < fdspdMatrix.size(); i++)
				{
					if (fdspdMatrix[i].size() < 2)
						throw LACoreInvalidData("funding spread file error",__FILE__,__LINE__);

					mFdSpdMap.insert(make_pair(fdspdMatrix[i][0],fdspdMatrix[i][1].getDoubleValue()));
				}
			}
		}
	}

	// set funding spread
	LAString trade_name = trade.getName().get();
	map<LAString, double>::iterator itspd = mFdSpdMap.find(trade_name);
	if (legSize == 1)
	{
		double spread = 0.;
		if (itspd != mFdSpdMap.end())
		{
			spread = itspd->second * 0.0001;
		}

		// set funding spread object
		LAString fndSpdEntityName = trade_name + "_FUNDINGSPREAD";
		LAObject* fndSpdEntity = NULL;
		if(!objPool.getObject(fndSpdEntityName).isDefined())
		{	
			fndSpdEntity = new LAObject;
			objPool.set(fndSpdEntityName, fndSpdEntity);
			fndSpdEntity->add(CALIBRATION_DATA_NAME, new LADataString(fndSpdEntityName));
		}
		else
		{
			fndSpdEntity = &objPool.getObject(fndSpdEntityName).get();
		}
		fndSpdEntity->remove(PRICING_DATA_FUNDINGSPREAD);
		fndSpdEntity->add(PRICING_DATA_FUNDINGSPREAD, new LADataDouble(spread));
		trade.remove(PRICING_DATA_FUNDINGSPREADENTITY);
		trade.LAObject::add(PRICING_DATA_FUNDINGSPREADENTITY, new LADataReference()).convertFromString(fndSpdEntityName);
	}
	else if (legSize == 2)
	{
		if (itspd == mFdSpdMap.end())
			return;

		double spread = itspd->second * 0.0001;

		// set funding spread object
		LAString fndSpdEntityName = trade_name + "_FUNDINGSPREAD";
		LAObject* fndSpdEntity = NULL;
		if(!objPool.getObject(fndSpdEntityName).isDefined())
		{	
			fndSpdEntity = new LAObject;
			objPool.set(fndSpdEntityName, fndSpdEntity);
			fndSpdEntity->add(CALIBRATION_DATA_NAME, new LADataString(fndSpdEntityName));
		}
		else
		{
			fndSpdEntity = &objPool.getObject(fndSpdEntityName).get();
		}
		fndSpdEntity->remove(PRICING_DATA_FUNDINGSPREAD);
		fndSpdEntity->add(PRICING_DATA_FUNDINGSPREAD, new LADataDouble(spread));
		trade.remove(PRICING_DATA_FUNDINGSPREADENTITY);
		trade.LAObject::add(PRICING_DATA_FUNDINGSPREADENTITY, new LADataReference()).convertFromString(fndSpdEntityName);
		
		// set coupon object
		DoubleVector val(2);
		val[0] = 1.0;
		val[1] = spread;

		//set isfunding true
		LADate asOf = LAMarketData::getAsofDate(objPool);

		LAObject& leg = unders.get(1).get();

		dh = &(leg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(dh->get());
		const unsigned int cashletSize = cashlets.getSize();

		for (unsigned int i = 0; i < cashletSize; i++)
		{
			LAObject& cashlet = cashlets.get(i).get();
			
			//in case of notional cf or extracf as examples, we don't need couponinfos
			dh = &(cashlet.getData(PRICING_DATA_COUPONINFOS));
			if (!dh->isDefined() || dh->isNull())
				continue;

			//dh = &(cashlet.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
			LADataMultiReference& coupons = dynamic_cast<LADataMultiReference &>(dh->get());
			
			if (coupons.getSize() != 1)
				throw LACoreInvalidData("FundingCoupon Size must be 1",__FILE__,__LINE__);
			
			LAObject& coupon = coupons.get(0).get();
		
			//check whether already fixed or not
			dh = &(coupon.getData(PRICING_DATA_INDEXINFOS));
			if (!dh ->isDefined() || dh->isNull())
				continue;

			LADataMultiReference& indexs = dynamic_cast<LADataMultiReference &>(dh->get());
			if (indexs.getSize() != 1)
				throw LACoreInvalidData("FundingIndex Size must be 1",__FILE__,__LINE__);
			LAObject& index = indexs.get(0).get();
			dh = &(index.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
			if (!dh ->isDefined() || dh->isNull())
				continue;
			
			LADate fixingdate = dynamic_cast<LADataDate &>(dh->get()).get();

			double fixedrate = 0.0;
			dh = &(index.getData(PRICING_DATA_FIXEDRATE, NOCHECK));
			if (dh ->isDefined() && !dh->isNull())
				fixedrate = dynamic_cast<LADataDouble &>(dh->get()).get();

			if (fixingdate < asOf && fixedrate != -9999.0)
				continue;

			//set coefficient
			dh = &(coupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
			const DoubleVector& coeff = dynamic_cast<const LADataDoubles &>(dh->get()).get();

			if (coeff.size() != 2)
				throw LACoreInvalidData ("FundingCoupon Coefficient must be size 2",__FILE__,__LINE__);

			dynamic_cast<LADataDoubles &>(dh->get()).set(val);
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
LAObjectConfiguration::setUpDirtyPrice(LAObjectPool &objPool,  LAMathObjectValue &trade) const
{
	LADataHolder* dh;

	// get fundingspreadfile
	if (mDirtyPriceMap.empty())
	{
		//read funding spread file
		LAString fileName = mpStaticData->getStaticData(KEY_DEAL_DIRTYPRICE_FILE);
		if (LACoreDataService::isFileExist(LAMarketData::getNumFileName(fileName)))
		{
			LAStringMatrix dirtyMatrix;
			MAFileAccessor file(LAMarketData::getNumFileName(fileName));
			file.readAllData(',', dirtyMatrix);
			for (unsigned int i = 0; i < dirtyMatrix.size(); i++)
			{
				if (dirtyMatrix[i].size() < 2)
				{
					throw LACoreInvalidData("dirty price file format error",__FILE__,__LINE__);
				}
				mDirtyPriceMap.insert(make_pair(dirtyMatrix[i][0], dirtyMatrix[i][1].getDoubleValue()));
			}
		}
	}

	// set funding spread
	const LAString &name = trade.getName().get();
	map<LAString, double>::const_iterator it = mDirtyPriceMap.find(name);
	if (it == mDirtyPriceMap.end())
	{
		return;
	}
	trade.remove(PRICING_DATA_DIRTYPRICE);
	trade.LAObject::add(PRICING_DATA_DIRTYPRICE, new LADataDouble(it->second));
}

// 
/*!
    @brief setUpDigitalCallSpreadCoupon	
	@param[in] object pool
	@param[out] trade object
*/
void
LAObjectConfiguration::setUpDigitalCallSpreadCoupon(LAObjectPool &objPool,  LAMathObjectValue &trade) const
{
	// setup Future Fixing object
    LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();


	LADataHolder* dh;

	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
	unsigned int legSize = legs.getSize();

	for (unsigned int i = 0; i < legSize; i++)
	{
		LAObject& legentity = legs.get(i).get();
		dh = &(legentity.getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(dh->get());
		unsigned int cashletSize = cashlets.getSize();

		for (unsigned int j = 0; j < cashletSize; j++)
		{
			LAObject& cashletentity = cashlets.get(j).get();
			dh = &(cashletentity.getData(PRICING_DATA_COUPONSELECTOPERATOR, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
				continue;
			
			LAPriceDataFunction& method = dynamic_cast<LAPriceDataFunction&>(dh->get());
			
			LAFunctionBase& couponfunc = method.getFunction();
			if (!couponfunc.isTypeOf(FN_CPNSLTOPERATORFORDIGITAL2))
				continue;


			dh = &(cashletentity.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
			const LADataMultiReference& coupons = dynamic_cast<const LADataMultiReference&>(dh->get());

			if (coupons.getSize() == 0)
				throw LACoreInvalidData("Coupon size error",__FILE__,__LINE__);

			LAObject& coupponentity = coupons.get(0).get();
			dh = &(coupponentity.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
			const LADataMultiReference& indexs = dynamic_cast<const LADataMultiReference&>(dh->get());
			if (indexs.getSize() == 0)
				throw LACoreInvalidData("Index size error",__FILE__,__LINE__);

			//check index object is fx or not
			LAObject& indexentity = indexs.get(0).get();
			dh = &(indexentity.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
			LAString indextype = dynamic_cast<const LADataString&>(dh->get()).get();
			if (indextype.toUpper() != LAString("FXRATE"))
				continue;

			dh = &(indexentity.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
			LAString indexcur = dynamic_cast<const LADataString&>(dh->get()).get();
			indexcur.toLower();

			LAString iscallspreadstr = staticData.getStaticData(indexcur + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD);
			if (iscallspreadstr.toUpper() != LAString("TRUE"))
				continue;

			LAString spreadvalstr = staticData.getStaticData(indexcur + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE);
			if (spreadvalstr == MLIB_NO_DATA)
				throw LACoreInvalidData("DigitalCoupon CallSpread error",__FILE__,__LINE__);

			//set coupon digitalmode
			LAPriceCouponForDigital2& digitalcoupon = dynamic_cast<LAPriceCouponForDigital2&>(couponfunc);
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
LAObjectConfiguration::setUpFwdInterpolation(LAObjectPool &objPool) const
{
	mIsFwdInterMap.clear();
	mFwdInterMap.clear();
	LAStringVector ccys = LACoreDataService::getContext(ARG_KEY_CURRENCY).toToken(MULTI_STATIC_DATA_DELIMITER);
	for (unsigned int i = 0; i < ccys.size(); ++i)
	{
		// fx return
		if (ccys[i].findString("/") >= 0)
		{
			continue;
		}
		LAString strIsInter = mpStaticData->getStaticData(ccys[i].toLower() + STATIC_DATA_KEY_INDEX_LIBOR_ISFWDINTER).toUpper();
		if (strIsInter == "TRUE")
		{
			mIsFwdInterMap[ccys[i].toUpper()] = true;
			mFwdInterMap[ccys[i].toUpper()] = mpStaticData->getStaticData(ccys[i].toLower() + STATIC_DATA_KEY_INDEX_LIBOR_FWDINTERPOLATION);
		}
		else
		{
			mIsFwdInterMap[ccys[i].toUpper()] = false;
		}
		const LAMathYieldCurvePro &yp = dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(LAMarketData::getBaseYieldProName(ccys[i]), ENCHKTYPE_ISDEFINED).get());
		const map<LAString, LAString>& mktMap = yp.getAssignedCurveMktMap();
		map<LAString, LAString>::const_iterator it = mktMap.begin();

		while (it != mktMap.end())
		{
			LAString suffix = "";
			if (it->second != SWAP)
			{
				suffix = "." + it->second;
				suffix.toLower();
			}
			strIsInter = mpStaticData->getStaticData(ccys[i].toLower() + STATIC_DATA_KEY_INDEX_LIBOR_ISFWDINTER + suffix).toUpper();
			LAString key = ccys[i].toUpper() + "_" + it->first;
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
	LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);
	setUpFwdInterpolation(dynamic_cast<LAMathObjectValue &>(objHolder.get()));

}

// 
/*!
    @brief forward interpolation

	@param[out] object pool
*/
void
LAObjectConfiguration::setUpFwdInterpolation(LAMathObjectValue &trade) const
{
	if (trade.isMethodTypeOf(FN_IR_PORTFOLIOVALUE))
	{
		LADataMultiReference &ref = dynamic_cast<LADataMultiReference &>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		for (unsigned int i = 0; i < ref.getSize(); ++i)
		{
			setUpFwdInterpolation(dynamic_cast<LAMathObjectValue &>(ref.get(i).get()));
		}
	}
	else
	{
		LADataHolder *dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(dh->get());
			for (unsigned int i = 0; i < legs.getSize(); ++i)
			{
				// leg
				dh = &legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					const LADataMultiReference &cashlets = dynamic_cast<const LADataMultiReference &>(dh->get());
					for (unsigned int j = 0; j < cashlets.getSize(); ++j)
					{
						// cashlets
						dh = &(cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							LADataMultiReference &couponinfos = dynamic_cast<LADataMultiReference &>(dh->get());	
							for (unsigned int k = 0; k < couponinfos.getSize(); ++k)
							{
								// coupon
								dh = &(couponinfos.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
								if (dh->isDefined() && !dh->isNull())
								{
									LADataMultiReference &indexInfos = dynamic_cast<LADataMultiReference &>(dh->get());	
									for (unsigned int l = 0; l < indexInfos.getSize(); ++l)
									{
										// indexinfos
										LAString indexType = dynamic_cast<const LADataString &>(indexInfos.get(l).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get();
										indexType.toUpper();
										if (indexType == LIBOR || indexType == CMS)
										{
											LAString ccy = dynamic_cast<const LADataString &>(indexInfos.get(l).getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
											ccy.toUpper();
											LAString key = ccy;
											dh = &(indexInfos.get(l).getData(PRICING_DATA_BASISCURVE, NOCHECK));
											if (dh->isDefined() && !dh->isNull())
											{
												key += "_" + dynamic_cast<LADataString &>(dh->get()).get();
											}
											if (mIsFwdInterMap[key])
											{
												indexInfos.get(l).remove(PRICING_DATA_ISFWDINTERPOLATION);
												indexInfos.get(l).remove(PRICING_DATA_FWDINTERPOLATION);
												indexInfos.get(l).add(PRICING_DATA_ISFWDINTERPOLATION, new LADataBool(true));
												indexInfos.get(l).add(PRICING_DATA_FWDINTERPOLATION, new LAPriceDataInterpolation()).convertFromString(mFwdInterMap[key]);
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
LAObjectConfiguration::setUpCoTermOfLSMCIndex(LAObjectPool &objPool, LAMathObjectValue &trade) const
{

	LADataHolder* dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	LAObject& callinfo = dynamic_cast<LADataReference &>(dh->get()).get().get();

	dh = &(callinfo.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;
	LADataMultiReference& refs = dynamic_cast<LADataMultiReference &>(dh->get());

	
	//get coterm of this trade;
	LADate asOfDate = LAMarketData::getAsofDate(objPool);
	LADate eDate;
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference &>(dh->get());
	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_ENDDATE, NOCHECK));
		LADate legeDate;
		if (dh->isDefined() && !dh->isNull())
			legeDate = dynamic_cast<LADataDate &>(dh->get()).get();

		eDate = (legeDate > eDate ) ? legeDate : eDate;
	}

	for (unsigned int i = 0 ; i < refs.getSize(); i++)
	{
		LAObject& eindex = refs.get(i).get();
		dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
		LAString tmpstr = dynamic_cast<LADataString &>(dh->get());

		if (-1 == tmpstr.findString("Co-Term"))
			continue;

		//set co term
		dh = &(eindex.getData(PRICING_DATA_FREQUENCY, ISNOTNULL));
		LAString freq = dynamic_cast<LADataString &>(dh->get()).get();
		LAString coTerm = "Co-Term";
		coTerm += createCoTermTenorString(asOfDate, eDate, freq); 
		dynamic_cast<LADataString &>(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL).get()).set(coTerm);
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
LAObjectConfiguration::setUpScaleShftOfLSMC(LAObjectPool &objPool, LAMathObjectValue &trade) const
{

	LADataHolder* dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	LAObject& callinfo = dynamic_cast<LADataReference &>(dh->get()).get().get();

	LAString tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_ISSHIFTY).toUpper();
	callinfo.remove(PRICING_DATA_ISSHIFT_Y);
	if (tmp != MLIB_NO_DATA && tmp == "TRUE")
	{
		callinfo.add(PRICING_DATA_ISSHIFT_Y, new LADataBool(true));
	}
	tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_ISSHIFTX).toUpper();
	callinfo.remove(PRICING_DATA_ISSHIFT_X);
	if (tmp != MLIB_NO_DATA && tmp == "TRUE")
	{
		callinfo.add(PRICING_DATA_ISSHIFT_X, new LADataBool(true));
	}

	tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_ISSCALEY).toUpper();
	callinfo.remove(PRICING_DATA_ISSCALE_Y);
	if (tmp != MLIB_NO_DATA && tmp == "TRUE")
	{
		callinfo.add(PRICING_DATA_ISSCALE_Y, new LADataBool(true));
	}
	tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_ISSCALEX).toUpper();
	callinfo.remove(PRICING_DATA_ISSCALE_X);
	if (tmp != MLIB_NO_DATA && tmp == "TRUE")
	{
		callinfo.add(PRICING_DATA_ISSCALE_X, new LADataBool(true));
	}
}




void
LAObjectConfiguration::setUpAsOfDateForTradeEntity(LAObjectPool &objPool, LAMathObjectValue &trade) const
{

	//set isfunding true
	LADate asOf = LAMarketData::getAsofDate(objPool);
	LADataHolder* dh;

	// check type
	LADataValuation &valueTypeAttr = dynamic_cast<LADataValuation &>(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
	
	//tradevalue, lsmctradevalue plainvanillaswaptradevalue
	if (valueTypeAttr.isTypeOf(FN_IR_TRADEVALUE))
	{
		trade.remove(PRICING_DATA_TODAY);
		trade.LAObject::add(PRICING_DATA_TODAY, new LADataDate(asOf));
	}

    dh = &trade.getData(CALIBRATION_DATA_ASOFDATE);
    if(!dh->isDefined() || dh->isNull()){
        trade.remove(CALIBRATION_DATA_ASOFDATE);
        trade.add(CALIBRATION_DATA_ASOFDATE).convertFromString(asOf.stringWithFormat());
    }

    dh = &(trade.getData(PRICING_DATA_SETTLEDATE));
    if(!dh->isDefined() || dh->isNull()){
        LAString settlestr = LACoreDataService::getContext(ARG_KEY_SETTLEDATE);
        trade.remove(PRICING_DATA_SETTLEDATE);
        if (settlestr != MLIB_NO_DATA)
            trade.LAObject::add(PRICING_DATA_SETTLEDATE, new LADataDate()).convertFromString(settlestr);
        else
            trade.LAObject::add(PRICING_DATA_SETTLEDATE, new LADataDate(asOf));
    }	

	
	return;

}


//==============================================
void 
LAObjectConfiguration::setUpPastCouponPayOffs(LAObjectPool &objPool, LAMathObjectValue &trade) const
{
	
	LADataHolder* dh;
	dh = &(trade.getData(PRICING_DATA_TRIGGERINFOS, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	LADataMultiReference& trgrefs = dynamic_cast<LADataMultiReference &>(dh->get());

	for (unsigned int i = 0; i < trgrefs.getSize(); i++)
	{


		LAObject& etrigger = trgrefs.get(i).get();
		dh = &(etrigger.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		LAObject& indexinfo = dynamic_cast<LADataMultiReference &>(dh->get()).get(0).get();

		//index type
		dh = &(indexinfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));		
		LAString indextype = dynamic_cast<const LADataString&>(dh->get()).get();
		indextype.toUpper();
		if (indextype != "CPN" && indextype != "CPNCF")
		{
			//not tarn case
			continue;
		}
		
		//index accessories
		unsigned int legno = 0;
		dh = &(indexinfo.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
		LAString accessory  = dynamic_cast<const LADataString&>(dh->get()).get();
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
		LADate asOfDate = LAMarketData::getAsofDate(objPool);

		dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		LAObject& eleg = dynamic_cast<LADataMultiReference &>(dh->get()).get(legno).get();

		//cashlets
		DateVector paymentdateVec;
		DoubleVector couponVec, couponpayoffVec;
		dh = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference &>(dh->get());
		for (unsigned int j = 0 ; j < cashlets.getSize(); j++)
		{
			LAObject& ecashlet = cashlets.get(j).get();
			dh = &(ecashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
			LADate paydate = dynamic_cast<LADataDate &>(dh->get()).get();
			if (paydate > asOfDate)
				break;

			//in case of notional exchange cashlet, we have to skip it
			dh = &(ecashlet.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				LADataMultiReference& coupons = dynamic_cast<LADataMultiReference &>(dh->get());
				LAObject& ecoupon = coupons.get(0).get();
				
				dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
				LADataMultiReference& indices = dynamic_cast<LADataMultiReference &>(dh->get());
				LAObject& eindex = indices.get(0).get();

				//fixed rate
				dh = &(eindex.getData(PRICING_DATA_FIXEDRATE));
                double fixedrate = dh->isDefined() && !dh->isNull() ? dynamic_cast<LADataDouble &>(dh->get()).get() : 0;

				//notional
				dh = &(ecashlet.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL));
				double notional = dynamic_cast<LADataDouble &>(dh->get()).get();

				//startdate, enddate, daycount to get tau
				dh = &(ecashlet.getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL));
				const LADate& sdate = dynamic_cast<const LADataDate &>(dh->get()).get();

				dh = &(ecashlet.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL));
				const LADate& edate = dynamic_cast<const LADataDate &>(dh->get()).get();

				dh = &(ecashlet.getData(PRICING_DATA_DAYCOUNT, ISNOTNULL));
				const LAPriceDataDayCount& dc = dynamic_cast<LAPriceDataDayCount &>(dh->get());

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
                eleg.add(PRICING_DATA_PAYMENTDATES, new LADataDates(paymentdateVec));
            }
            if(!(dh = &eleg.getData(PRICING_DATA_COUPONS, NOCHECK))->isDefined() || dh->isNull()){
                eleg.remove(PRICING_DATA_COUPONS);
                eleg.add(PRICING_DATA_COUPONS, new LADataDoubles(couponVec));
            }
            if(!(dh = &eleg.getData(PRICING_DATA_COUPONPAYOFFS, NOCHECK))->isDefined() || dh->isNull()){
                eleg.remove(PRICING_DATA_COUPONPAYOFFS);
                eleg.add(PRICING_DATA_COUPONPAYOFFS, new LADataDoubles(couponpayoffVec));
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
LAObjectConfiguration::setUpDiscountCurve(LAObjectPool &objPool, LAMathObjectValue &trade) const
{
	
	LADataHolder *dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LAString ccy =  dynamic_cast<const LADataString &>(trade.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
		ccy.toLower();
		LAString dCurve = DF2;
		const LAString dCurve_p = mpStaticData->getStaticData(ccy + STATIC_DATA_KEY_YIELD_DF2);
		if (dCurve_p != MLIB_NO_DATA)
		{
			dCurve = dCurve_p;
		}
		LADataMultiReference& legs = dynamic_cast<LADataMultiReference &>(dh->get());
		for (unsigned int i = 0; i < legs.getSize(); i++)
		{
			dh = &(legs.get(i).getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				legs.get(i).remove(PRICING_DATA_DISCOUNTCURVE);
				legs.get(i).add(PRICING_DATA_DISCOUNTCURVE, new LADataString(dCurve));
			}
		}
	}
	return;
}

LAString LAObjectConfiguration::createCoTermTenorString(const LADate& today, const LADate& matdate, const LAString& freq) const
{
    // define
    const LAString CONST_FREQUENCY_ANNUAL="Annual";
    const LAString CONST_FREQUENCY_SEMI_ANNUAL="Semi-Annual";
    const LAString CONST_FREQUENCY_QUARTERLY="Quarterly";
    const LAString CONST_FREQUENCY_MONTHLY="Monthly";

    const LAString ACCESSORY_CO_TERM_YEAR="Y";
    const LAString ACCESSORY_CO_TERM_MONTH="M";
    const LAString CO_TERM_MIN_MONTH="6";



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
			throw LACoreInvalidData("date diff failed.",__FILE__,__LINE__);
        }



        // >>> make string to each frequency
        LAString ret;
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
            ret += LAString(diffYear) + ACCESSORY_CO_TERM_YEAR;
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
				ret += LAString(diffYear) + ACCESSORY_CO_TERM_YEAR;

				if(diffMonth>0)
				{
					ret += LAString(diffMonth) + ACCESSORY_CO_TERM_MONTH;
				}
			}
        }
        else
        {
            throw LACoreInvalidData("illegal frequency.",__FILE__,__LINE__);
        }

        return ret;
    }
	catch (...)
    {
        throw LACoreInvalidData("error in createCoTermTenorString",__FILE__,__LINE__);
	}
}

void LAObjectConfiguration::setUpRangeAccrueFixedRates(const LADate& asOfDate, LAObjectHolder& ehTrade) const
{
	// read fixing rate file
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString ratefile = staticData.getStaticData(KEY_DEAL_RATE_FILE);
	if (ratefile == MLIB_NO_DATA)
	{
		return;
	}
	LAStringMatrix ratesMatrix;
	MAFileAccessor ratesFile(LAMarketData::getNumFileName(ratefile));
	ratesFile.readAllData(MARKET_DATA_DELIMITER,ratesMatrix);
	ratesFile.close();

	const LADataMultiReference &legs = dynamic_cast<const LADataMultiReference &>
										(ehTrade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	const unsigned int legSize = legs.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		LAObjectHolder &leg = legs.get(i);		
		LAString inputType = dynamic_cast<const LADataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			LADataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				unsigned int counter = 0;
				bool isRangeAccrue = false;
				// cashlet loop
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					// check rangeaccrue info
					LADataHolder &attrIsRA = cashlets.get(j).getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK);
					if (attrIsRA.isDefined() && !attrIsRA.get().isNull())
					{
						if (attrIsRA.get().convertToString().toUpper() == "TRUE")
						{
							isRangeAccrue = true;
							// check payment date
							const LADate &paymentDate = dynamic_cast<const LADataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
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
					LADataHolder &attrIsRA = cashlets.get(counter).getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK);
					if (!attrIsRA.isDefined() || attrIsRA.isNull())
						throw LACoreInvalidData("IsRangeAccrue is not registed.",__FILE__,__LINE__);

					// check asofdate								
					const LADate &startDate = dynamic_cast<const LADataDate &>
						(cashlets.get(counter).getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get();
					const LADate &endDate = dynamic_cast<const LADataDate &>
						(cashlets.get(counter).getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL).get()).get();
					if (asOfDate < startDate || endDate < asOfDate)
					{
						continue;
					}

					//set observationdates and observationrates
					LADataHolder &attrInfos = cashlets.get(counter).getData(PRICING_DATA_RANGEACCRUEINFOS, NOCHECK);
					if (attrInfos.isDefined() && !attrInfos.isNull()) // multi index case
					{
						LADataMultiReference &rainfos = dynamic_cast<LADataMultiReference &>(attrInfos.get());
						const unsigned int rainfoSize = rainfos.getSize();
						// info loop
						for (unsigned int kk = 0; kk < rainfoSize; ++kk)
						{
							LADataHolder &attrIndexs = rainfos.get(kk).getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								LADataMultiReference &raindexs = dynamic_cast<LADataMultiReference &>(attrIndexs.get());
								LAObjectConfiguration::setUpIndexFixedRates(ratesMatrix, raindexs);
							}
							LADataHolder &attrBoundaryIndexs = rainfos.get(kk).getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK);
							if (attrBoundaryIndexs.isDefined() && !attrBoundaryIndexs.isNull())
							{
								LADataMultiReference &rabindexs = dynamic_cast<LADataMultiReference &>(attrBoundaryIndexs.get());
								LAObjectConfiguration::setUpBoundaryIndexFixedRate(ratesMatrix, rabindexs);
							}
						}
					}
					else // single index case
					{
						LADataHolder &attrIndexs = cashlets.get(counter).getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, NOCHECK);
						if (attrIndexs.isDefined() && !attrIndexs.isNull())
						{
							LADataMultiReference &raindexs = dynamic_cast<LADataMultiReference &>(attrIndexs.get());
							LAObjectConfiguration::setUpIndexFixedRates(ratesMatrix, raindexs);
						}
						LADataHolder &attrBoundaryIndexs = cashlets.get(counter).getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK);
						if (attrBoundaryIndexs.isDefined() && !attrBoundaryIndexs.isNull())
						{
							LADataMultiReference &rabindexs = dynamic_cast<LADataMultiReference &>(attrBoundaryIndexs.get());
							LAObjectConfiguration::setUpBoundaryIndexFixedRate(ratesMatrix, rabindexs);
						}
					}
				}
			}
		}
	}
}

void LAObjectConfiguration::setUpIndexFixedRates_ObsvOpr(const LAStringMatrix& ratesMatrix, LADataMultiReference &indexs) const
{
	const unsigned int indexSize = indexs.getSize();
	// index loop
	for (unsigned int i = 0; i < indexSize; ++i)
	{
		// check observation operator
		LADataHolder &attrObsvOpr = indexs.get(i).getData(PRICING_DATA_OBSERVATIONOPERATOR, NOCHECK);
		if (!attrObsvOpr.isDefined() || attrObsvOpr.isNull()) continue;

		// get index name
		const LADataHolder &attrIndexName = indexs.get(i).getData(PRICING_DATA_INDEXNAME, NOCHECK);
		if (!attrIndexName.isDefined() || attrIndexName.isNull()) continue;
		const LAString& indexName = dynamic_cast<const LADataString &>(attrIndexName.get()).get();

		DateVector dateVec(0);
		DoubleVector rateVec(0);
		bool isFound = false;
		for (unsigned int ii = 0; ii < ratesMatrix.size(); ++ii)
		{
			if (indexName != ratesMatrix[ii][0]) continue;

			LAStringVector strDateVec = ratesMatrix[ii][1].toToken(MULTI_STATIC_DATA_DELIMITER);
			LAStringVector strRateVec = ratesMatrix[ii][2].toToken(MULTI_STATIC_DATA_DELIMITER);
			for (unsigned int jj = 0; jj < strDateVec.size(); ++jj)
			{
				LADate date(strDateVec[jj].getCString());
				dateVec.push_back(date);
				rateVec.push_back(strRateVec[jj].getDoubleValue());
			}
			isFound = true;
			break;
		}
		if (!isFound) continue;

		//change object
		LAObject* pindex = &indexs.get(i).get();
		//remove
		pindex->remove(PRICING_DATA_OBSERVATIONDATES);
		pindex->remove(PRICING_DATA_OBSERVATIONRATES);
		//add
		pindex->add(PRICING_DATA_OBSERVATIONDATES, new LADataDates(dateVec));
		pindex->add(PRICING_DATA_OBSERVATIONRATES, new LADataDoubles(rateVec));
	}
}

void LAObjectConfiguration::setUpIndexFixedRates(const LAStringMatrix& ratesMatrix, LADataMultiReference &indexs) const
{
	const unsigned int indexSize = indexs.getSize();
	// index loop
	for (unsigned int i = 0; i < indexSize; ++i)
	{
		// get index name
		const LADataHolder &attrIndexName = indexs.get(i).getData(PRICING_DATA_INDEXNAME, NOCHECK);
		if (!attrIndexName.isDefined() || attrIndexName.isNull()) continue;
		const LAString& indexName = dynamic_cast<const LADataString &>(attrIndexName.get()).get();

		DateVector dateVec(0);
		DoubleVector rateVec(0);
		bool isFound = false;
		for (unsigned int ii = 0; ii < ratesMatrix.size(); ++ii)
		{
			if (indexName != ratesMatrix[ii][0]) continue;

			LAStringVector strDateVec = ratesMatrix[ii][1].toToken(MULTI_STATIC_DATA_DELIMITER);
			LAStringVector strRateVec = ratesMatrix[ii][2].toToken(MULTI_STATIC_DATA_DELIMITER);
			for (unsigned int jj = 0; jj < strDateVec.size(); ++jj)
			{
				LADate date(strDateVec[jj].getCString());
				dateVec.push_back(date);
				rateVec.push_back(strRateVec[jj].getDoubleValue());
			}
			isFound = true;
			break;
		}
		if (!isFound) continue;

		//change object
		LAObject* pindex = &indexs.get(i).get();
		//remove
		pindex->remove(PRICING_DATA_OBSERVATIONDATES);
		pindex->remove(PRICING_DATA_OBSERVATIONRATES);
		//add
		pindex->add(PRICING_DATA_OBSERVATIONDATES, new LADataDates(dateVec));
		pindex->add(PRICING_DATA_OBSERVATIONRATES, new LADataDoubles(rateVec));
	}
}

void LAObjectConfiguration::setUpBoundaryIndexFixedRate(const LAStringMatrix& ratesMatrix, LADataMultiReference &indexs) const
{
	const unsigned int indexSize = indexs.getSize();
	// index loop
	for (unsigned int i = 0; i < indexSize; ++i)
	{
		// get index name
		const LADataHolder &attrIndexName = indexs.get(i).getData(PRICING_DATA_INDEXNAME, NOCHECK);
		if (!attrIndexName.isDefined() || attrIndexName.isNull()) continue;
		const LAString& indexName = dynamic_cast<const LADataString &>(attrIndexName.get()).get();
		
		// get fixing date
		const LADataHolder &attrFixingDate = indexs.get(i).getData(PRICING_DATA_FIXINGDATE, NOCHECK);
		if (!attrFixingDate.isDefined() || attrFixingDate.isNull()) continue;
		const LADate& fixingdate = dynamic_cast<const LADataDate&>(attrFixingDate.get()).get();

		double fixedrate;
		bool isFound = false;
		for (unsigned int ii = 0; ii < ratesMatrix.size(); ++ii)
		{
			if (indexName != ratesMatrix[ii][0]) continue;

			LAStringVector strDateVec = ratesMatrix[ii][1].toToken(MULTI_STATIC_DATA_DELIMITER);
			LAStringVector strRateVec = ratesMatrix[ii][2].toToken(MULTI_STATIC_DATA_DELIMITER);
			for (unsigned int jj = 0; jj < strDateVec.size(); ++jj)
			{
				LADate date(strDateVec[jj].getCString());
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
		LAObject* pindex = &indexs.get(i).get();
		//remove
		pindex->remove(PRICING_DATA_FIXEDRATE);
		//add
		pindex->add(PRICING_DATA_FIXEDRATE, new LADataDouble(fixedrate));
	}
}

void LAObjectConfiguration::setUpPastIndexRate(LAObjectPool &objPool, LAMathObjectValue &trade) const
{
	// read fixing rate file
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString ratefile = staticData.getStaticData(KEY_DEAL_RATE_FILE);
	if (ratefile == MLIB_NO_DATA)
	{
		return;
	}
	LAStringMatrix ratesMatrix;

	LAString filename=LAMarketData::getNumFileName(ratefile);

	if (!LACoreDataService::isFileExist(filename))
	{
		return;
	}
	
	MAFileAccessor ratesFile(filename);
	ratesFile.readAllData(MARKET_DATA_DELIMITER,ratesMatrix);
	ratesFile.close();

	if(ratesMatrix.empty())
	{
		return;
	}

	LADate asOfDate = LAMarketData::getAsofDate(objPool);

	const LADataHolder& ahUnders = trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (!ahUnders.isDefined() || ahUnders.isNull())
	{
		return;
	}
	const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>(ahUnders.get());
	const unsigned int legSize = unders.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		LAObjectHolder &leg = unders.get(i);		
		LAString inputType = dynamic_cast<const LADataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			LADataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				// cashlet loop
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					LADataHolder &attrCoupons = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
					if (attrCoupons.isDefined() && !attrCoupons.isNull())
					{
						LADataMultiReference &coupons = dynamic_cast<LADataMultiReference &>(attrCoupons.get());
						const unsigned int couponSize = coupons.getSize();
						// coupon loop
						for (unsigned int k = 0; k < couponSize; ++k)
						{
							// get ccy
							LADataHolder &attrIndexs = coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								LADataMultiReference &indexs = dynamic_cast<LADataMultiReference &>(attrIndexs.get());
								const unsigned int indexSize = indexs.getSize();
								// index loop
								for (unsigned int l = 0; l < indexSize; ++l)
								{

									LADataHolder &attrfixingdate = indexs.get(l).getData(PRICING_DATA_FIXINGDATE, NOCHECK);
									if (attrfixingdate.isDefined() && !attrfixingdate.isNull())
									{
										// get fixingdate
										const LADate fixingdate = dynamic_cast<const LADataDate &>(attrfixingdate.get()).get();
										if (asOfDate > fixingdate)
										{
											// get index name
											const LADataHolder &attrIndexName = indexs.get(l).getData(PRICING_DATA_INDEXNAME, NOCHECK);
											if (!attrIndexName.isDefined() || attrIndexName.isNull()) continue;								
											const LAString& indexName = dynamic_cast<const LADataString &>(attrIndexName.get()).get();

											double fixedrate;
											bool isFound = false;
											for (unsigned int ii = 0; ii < ratesMatrix.size(); ++ii)
											{
												if (indexName != ratesMatrix[ii][0]) continue;

												LAStringVector strDateVec = ratesMatrix[ii][1].toToken(MULTI_STATIC_DATA_DELIMITER);
												LAStringVector strRateVec = ratesMatrix[ii][2].toToken(MULTI_STATIC_DATA_DELIMITER);
												for (unsigned int jj = 0; jj < strDateVec.size(); ++jj)
												{
													LADate date(strDateVec[jj].getCString());
													if (date == fixingdate)
														{
															fixedrate = strRateVec[jj].getDoubleValue();
															isFound = true;

															//change object
															LAObject* pindex = &indexs.get(l).get();
															//remove
															pindex->remove(PRICING_DATA_FIXEDRATE);
															//add
															pindex->add(PRICING_DATA_FIXEDRATE, new LADataDouble(fixedrate));
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

void LAObjectConfiguration::setUpCompoundingFixedRates(const LADate& asOfDate, LAObjectHolder& ehTrade) const
{
	// read fixing rate file
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString ratefile = staticData.getStaticData(KEY_DEAL_RATE_FILE);
	if (ratefile == MLIB_NO_DATA)
	{
		return;
	}
	LAStringMatrix ratesMatrix;
	MAFileAccessor ratesFile(LAMarketData::getNumFileName(ratefile));
	ratesFile.readAllData(MARKET_DATA_DELIMITER,ratesMatrix);
	ratesFile.close();

	const LADataHolder& attrLegs = ehTrade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (!attrLegs.isDefined() || attrLegs.isNull())
	{
		return;
	}
	const LADataMultiReference &legs = dynamic_cast<const LADataMultiReference &>(attrLegs.get());
	const unsigned int legSize = legs.getSize();
	for (unsigned int i = 0; i < legSize; ++i)
	{
		LAObjectHolder &leg = legs.get(i);		
		LAString inputType = dynamic_cast<const LADataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		if (inputType.toUpper() == "MANUAL")
		{
			LADataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(attrCashlets.get());
				const unsigned int cashletSize = cashlets.getSize();
				unsigned int counter = 0;
				bool isCompounding = false;
				// cashlet loop
				for (unsigned int j = 0; j < cashletSize; ++j)
				{
					// check rangeaccrue info
					LADataHolder &attrIsCP = cashlets.get(j).getData(PRICING_DATA_ISCOMPOUNDINGCOUPON, NOCHECK);
					if (attrIsCP.isDefined() && !attrIsCP.get().isNull())
					{
						if (attrIsCP.get().convertToString().toUpper() == "TRUE")
						{
							isCompounding = true;
							// check payment date
							const LADate &paymentDate = dynamic_cast<const LADataDate &>(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
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
					LADataHolder &attrIsCP = cashlets.get(counter).getData(PRICING_DATA_ISCOMPOUNDINGCOUPON, NOCHECK);
					if (!attrIsCP.isDefined() || attrIsCP.isNull())
						throw LACoreInvalidData("IsCompoundingCoupon is not registed.",__FILE__,__LINE__);

					// check asofdate								
					const LADate &startDate = dynamic_cast<const LADataDate &>
						(cashlets.get(counter).getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get();
					const LADate &endDate = dynamic_cast<const LADataDate &>
						(cashlets.get(counter).getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL).get()).get();
					if (asOfDate < startDate || endDate < asOfDate)
					{
						continue;
					}

					//set observationdates and observationrates
					LADataHolder &attrInfos = cashlets.get(counter).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
					if (attrInfos.isDefined() && !attrInfos.isNull()) // multi index case
					{
						LADataMultiReference &cpinfos = dynamic_cast<LADataMultiReference &>(attrInfos.get());
						const unsigned int cpinfoSize = cpinfos.getSize();

						// info loop
						for (unsigned int kk = 0; kk < cpinfoSize; ++kk)
						{
							LADataHolder &attrIndexs = cpinfos.get(kk).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
							if (attrIndexs.isDefined() && !attrIndexs.isNull())
							{
								LADataMultiReference &indexs = dynamic_cast<LADataMultiReference &>(attrIndexs.get());
								LAObjectConfiguration::setUpIndexFixedRates(ratesMatrix, indexs);
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
						LADate startDate, endDate;
						LADataHolder *attrStartDate = &cashlets.get(j).getData(PRICING_DATA_CFCALCSTARTDATE, NOCHECK);
						if (attrStartDate->isDefined() && !attrStartDate->isNull())
						{
							startDate = dynamic_cast<const LADataDate &> (attrStartDate->get());
						}
						else
						{
							continue;
						}

						LADataHolder *attrEndDate = &cashlets.get(j).getData(PRICING_DATA_CFCALCENDDATE, NOCHECK);
						if (attrEndDate->isDefined() && !attrEndDate->isNull())
						{
							endDate = dynamic_cast<const LADataDate &> (attrEndDate->get());
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
						LADataHolder &attrInfos = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
						if (attrInfos.isDefined() && !attrInfos.isNull()) // multi index case
						{
							LADataMultiReference &cpinfos = dynamic_cast<LADataMultiReference &>(attrInfos.get());
							const unsigned int cpinfoSize = cpinfos.getSize();

							// info loop
							for (unsigned int kk = 0; kk < cpinfoSize; ++kk)
							{
								LADataHolder &attrIndexs = cpinfos.get(kk).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
								if (attrIndexs.isDefined() && !attrIndexs.isNull())
								{
									LADataMultiReference &indexs = dynamic_cast<LADataMultiReference &>(attrIndexs.get());
									LAObjectConfiguration::setUpIndexFixedRates_ObsvOpr(ratesMatrix, indexs);
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
LAObjectConfiguration::setUpSVDToleranceForLSMC(LAObjectPool &objPool, LAMathObjectValue &trade) const
{

	LADataHolder* dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	LAObject& callinfo = dynamic_cast<LADataReference &>(dh->get()).get().get();

	LAString tmp = mpStaticData->getStaticData(KEY_SIMULATION_LSMC_SVDTOLERANCE).toUpper();
	callinfo.remove(PRICING_DATA_SVDTOLERANCE);
	if (tmp != MLIB_NO_DATA)
	{
		callinfo.add(PRICING_DATA_SVDTOLERANCE, new LADataDouble()).convertFromString(tmp);
	}
}

/*!
    @brief set up nominal funding leg of strctured bond in an asset swap method.

	@param[out] objPool
	@param[out] trade
*/

void
LAObjectConfiguration::setUpBondFundingLeg(LAObjectPool &objPool, LAMathObjectValue &trade) const
{
	LADate asOfDate = LAMarketData::getAsofDate(objPool);

	LADataHolder* dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (!dh->isDefined() || dh->isNull())
		return;


	// find bond funding leg
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());
	unsigned int legSize = legs.getSize();
	vector<unsigned int> bond_funding_leg_pos;
	for (unsigned int i = 0; i < legSize; ++i)
	{
		dh = &legs.get(i).getData(PRICING_DATA_ISBONDFUNDINGLEG, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			if (dynamic_cast<LADataBool&>(dh->get()).get())
			{
				bond_funding_leg_pos.push_back(i);
			}
		}
	}

	if(bond_funding_leg_pos.size() == 0)
		return;


	// cashflow generator without modifying trade
	trade.LAObject::add(PRICING_DATA_DONOTMODIFYTRADE, new LADataBool(true));

	dh = &trade.getData(PRICING_DATA_CFGENERATOR, ISNOTNULL);
	LADataProcedure& modelDataObj = dynamic_cast<LADataProcedure&>(dh->get());
	modelDataObj.calibrateModel(asOfDate);

	trade.LAObject::remove(PRICING_DATA_DONOTMODIFYTRADE);
	for (unsigned int i = 0; i < bond_funding_leg_pos.size(); ++i)
	{
		dynamic_cast<LADataString&>(legs.get(bond_funding_leg_pos[i]).getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).set("MANUAL");
	}


	// Bond funding special treatment:
	// Set -9999 to fixed rate of already fixed index 
	// so that set up for extra libor will be invoked later.
	for (unsigned int i = 0; i < bond_funding_leg_pos.size(); ++i)
	{
		dh = &legs.get(bond_funding_leg_pos[i]).getData(PRICING_DATA_CASHLETS, ISNOTNULL);
		LADataMultiReference& cashs = dynamic_cast<LADataMultiReference&>(dh->get());
		for (unsigned int j = 0; j < cashs.getSize(); ++j)
		{
			const LADate& paymentdate = dynamic_cast<const LADataDate&>(cashs.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get());
			dh = &cashs.get(j).get().getData(PRICING_DATA_COUPONINFOS, NOCHECK);
			if (!dh->isDefined() || dh->isNull())
				continue;
			LADataMultiReference& coupons = dynamic_cast<LADataMultiReference&>(dh->get());
			for (unsigned int k = 0; k < coupons.getSize(); ++k)
			{
				dh = &coupons.get(k).get().getData(PRICING_DATA_INDEXINFOS, NOCHECK);
				if (!dh->isDefined() || dh->isNull())
					continue;
				LADataMultiReference& indices = dynamic_cast<LADataMultiReference&>(dh->get());
				for (unsigned int l = 0; l < indices.getSize(); ++l)
				{
					const LADate& fixingdate = dynamic_cast<const LADataDate&>(indices.get(l).getData(PRICING_DATA_FIXINGDATE, ISNOTNULL).get());
					if(fixingdate > asOfDate)
						continue;

					LAObject& targetindexinfo = indices.get(l).get();
					LADataString& attrIndexType = dynamic_cast<LADataString&>(targetindexinfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get());
					LAString indexType = attrIndexType.get();
					indexType.toUpper();
					if (indexType != "FIXEDRATE" && indexType != "LIBOR")
						continue;
				
					targetindexinfo.remove(PRICING_DATA_FIXEDRATE);
					if (paymentdate > asOfDate)
					{
						targetindexinfo.add(PRICING_DATA_FIXEDRATE, new LADataDouble(EXTRA_FIXEDRATE));
					}
					else
					{
						targetindexinfo.add(PRICING_DATA_FIXEDRATE, new LADataDouble(0.));
					}
					attrIndexType.set("FixedRate");
				}
			}
		}
	}

}


