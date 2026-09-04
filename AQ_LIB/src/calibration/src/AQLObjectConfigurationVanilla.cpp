/*! @file
    @brief Object generator class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLObjectConfigurationVanilla.h"
#include "AQLString.h"
#include "AQLDataBasics.h"
#include "AQLPriceDataDayCount.h"
#include "AQLCoreDataService.h"
#include "AQLMarketDataHW.h"
#include "AQLDefinitions.h"
#include "AQLObjectConfiguration.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLMarketData.h"
#include "AQLDealUtils.h"
#include "AQLStaticData.h"
#include "AQLDataReference.h"
#include "AQLDataValuation.h"
#include "AQLPricePortfolioValue.h"
#include "AQLMathValuableEntity.h"
#include "AQLLinearRatesOptionValue.h"
#include "AQLLinearRatesOptionValueDataProvider.h"
#include "AQLPriceTradeValue.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLLinearFunc.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathIndexEntity.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLPriceArbFreeGenerator.h"
#include "AQLLinearInterpolation.h"
#include "AQLPriceConvergenceValue.h"
#include "AQLMathIRVanillaFuncUtility.h"
//#include "AQLPriceNDSSwaptionValue.h"
#include <algorithm>

#define CURRENTINDEX		"CURRENTINDEX"
#define CLOSESTINDEX		"CLOSESTINDEX"
#define NEXTINDEX			"NEXTINDEX"
#define PREVIOUSINDEX		"PREVIOUSINDEX"
#define STARTENDDATEMETHOD	"STARTENDDATEMETHOD"
#define SPOTRATERATIOMETHOD "SPOTRATERATIOMETHOD"
#define GRIDTERMRATIOMETHOD "GRIDTERMRATIOMETHOD"




using namespace std;
// constructor
/*!

*/
AQLObjectConfigurationVanilla::AQLObjectConfigurationVanilla()
: mLiborRateMap(), mLiborGridTermMap(),AQLObjectConfiguration()
//: AQLObjectConfiguration()
{
}

// destructor
/*!

*/
AQLObjectConfigurationVanilla::~AQLObjectConfigurationVanilla(void)
{
	std::map<AQLString, AQLInterpolationBase *>::iterator it = mLiborRateMap.begin();
	while (it != mLiborRateMap.end())
	{
		delete it->second;
		++it;
	}
	mLiborRateMap.clear();
	mLiborGridTermMap.clear();
}

// Setup ValuableEntity
void
AQLObjectConfigurationVanilla::setUpValuableEntity( AQLObjectPool& objPool ) const
{
	AQLString mTradeName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
	AQLObjectHolder objHolder = objPool.getObject(mTradeName, ENCHKTYPE_ISDEFINED);

	AQLString IsAddFwdPremPV = mpStaticData->getStaticData(KEY_SDE_PV_ADDITION_FWDPREMIUM);
 
    AQLString zeroCalc = AQLCoreDataService::getContext(ARG_KEY_ZEROCALC);
    if (zeroCalc == AQ_NO_DATA) {
        zeroCalc = "FALSE";
    }

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
		

            // set zero calculation
            trade.remove(PRICING_DATA_ZEROCALC);
            trade.add(PRICING_DATA_ZEROCALC, new AQLDataBool()).convertFromString(zeroCalc);
		
			//for past libor auto fixing
			trade.remove(PRICING_DATA_ISSAVEPASTFIXING);
			trade.add(PRICING_DATA_ISSAVEPASTFIXING, new AQLDataBool(true));

			// set Compounding fixed rates
			AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);
			setUpCompoundingFixedRates(asOfDate, trade);

			setUpRiskInfo(riskEVec, trade.get());

			// set flag for adding fwdpremium to PV
			if (IsAddFwdPremPV != AQ_NO_DATA)
			{	
				trade.remove(PRICING_DATA_ISADDFWDPREMPV);
				trade.add(PRICING_DATA_ISADDFWDPREMPV, new AQLDataBool()).convertFromString(IsAddFwdPremPV);
			}
		}
		
		if (!riskEVec.empty())
		{
			objHolder.remove(PRICING_DATA_RISKCALCINFOS);
			objHolder.add(PRICING_DATA_RISKCALCINFOS, new AQLDataMultiReference()).convertFromString(riskENames_str);
		}
		
		AQLDataInstance *dataInstance = objHolder.getDataInstance();
		dataInstance->getReferencePool().completeDependency();

	}
	else
	{

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

        // set zero calculation
        objHolder.remove(PRICING_DATA_ZEROCALC);
        objHolder.add(PRICING_DATA_ZEROCALC, new AQLDataBool()).convertFromString(zeroCalc);

		//for past libor auto fixing
		objHolder.remove(PRICING_DATA_ISSAVEPASTFIXING);
		objHolder.add(PRICING_DATA_ISSAVEPASTFIXING, new AQLDataBool(true));

		// set Compounding fixed rates
		AQLDate asOfDate = AQLMarketData::getAsofDate(objPool);
		setUpCompoundingFixedRates(asOfDate, objHolder);

		setUpRiskInfo(riskEVec, objHolder.get());
	}

}



// setup  pathentity
void 
AQLObjectConfigurationVanilla::setUpPathEntity(AQLObjectPool &objPool, const AQLString *pPathName) const
{
	AQLMathPlainVanillaEntity *pvanilla = AQLMarketData::getPlainVanillaEntity(objPool);
	if (!pvanilla)
	{
		throw AQLCoreInvalidData("Plain Vanilla object is not set in entitiy pool", __FILE__, __LINE__);
	}	
	// set sdenames and models
	AQLStringVector ccys = AQLDealUtils::getSDECurrencys();
	AQLStringVector simCurs = AQLDealUtils::getSimulationSDECurrencys();
	//if (simCurs.size() != 0)
	//{
	//	ccys = simCurs;
	//}
	const unsigned int ccySize = ccys.size();

	AQLStringVector ircurs,irsimcurs,fxcurs;
	for (unsigned int i = 0; i < ccySize; i++)
	{
		AQLString cur = ccys[i];
		if (-1 == cur.findString('/'))
		{
			ircurs.push_back(cur.toUpper());
		}
		else
		{
			fxcurs.push_back(cur.toUpper());
		}
	}

	for (unsigned int i = 0; i < simCurs.size(); i++)
	{
		AQLString cur = simCurs[i];
		if (-1 == cur.findString('/'))
		{
			irsimcurs.push_back(cur.toUpper());
		}
	}
	
	// set initial value
	// first ccy 
	pvanilla->getIRCurrencys().set(ircurs);
	pvanilla->getIRSimCurrencys().set(irsimcurs);
	AQLString sdeName = mpStaticData->getStaticData(ircurs[0].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	AQLString initialValue =  PREFIX_YIELD + sdeName;
	// set main path curve
	AQLString pathCurve = mpStaticData->getStaticData(ircurs[0] + STATIC_DATA_KEY_YIELD_PATHCURVE);
	if (pathCurve == AQ_NO_DATA)
	{
		pvanilla->getIRCurveTypes().push_back(STD);
	}
	else
	{
		pvanilla->getIRCurveTypes().push_back(pathCurve);
	}
	AQLString irvols = PREFIX_VOL + sdeName;
	AQLString model = AQLMarketData::getModelName(ircurs[0]);

	//set curveproname
	AQLString initialPro = AQLMarketData::getBaseYieldProName(ircurs[0]);
	for (unsigned int i = 1; i < ircurs.size(); ++i)
	{
		AQLString sdeName = mpStaticData->getStaticData(ircurs[i].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		initialValue += AQLString(":") + PREFIX_YIELD + sdeName;
		// set main path curve
		pathCurve = mpStaticData->getStaticData(ircurs[i] + STATIC_DATA_KEY_YIELD_PATHCURVE);
		if (pathCurve == AQ_NO_DATA)
		{
			pvanilla->getIRCurveTypes().push_back(STD);
		}
		else
		{
			pvanilla->getIRCurveTypes().push_back(pathCurve);
		}
		if (model != AQLMarketData::getModelName(ircurs[i]))
		{
			throw AQLCoreInvalidData("Model is not same with main currency.", __FILE__, __LINE__);
		}
		irvols += AQLString(":") + PREFIX_VOL + sdeName;

		initialPro += AQLString(":") + AQLMarketData::getBaseYieldProName(ircurs[i]);
	}
	// set curve
	pvanilla->getIRCurves().convertFromString(initialValue);
	pvanilla->setUpIRCurveTypes();

	pvanilla->getIRCurveProNames().convertFromString(initialPro);
	AQLStaticData &riskProp = AQLCoreDataService::getStaticDataManager().getRiskStaticData();
	/*if ("TRUE" == riskProp.getStaticData(RISK_KEY_ANALYTIC_ISANALYTIC).toUpper())
	{
		for (unsigned int i =0; i < ircurs.size(); ++i)
		{
			AQLString ccy = ircurs[i];
			pvanilla->getIRCurvePro(ccy.toUpper()).setCurveDependencyMap();
		}
	}*/
	//if ("ANALYTIC" == riskProp.getStaticData(RISK_KEY_ANALYTIC_CALCTYPE).toUpper())
	//{
	//	for (unsigned int i =0; i < ircurs.size(); ++i)
	//	{
	//		AQLString ccy = ircurs[i];
	//		pvanilla->getIRCurvePro(ccy.toUpper()).setCurveDependencyMap();
	//	}
	//}


	
	if (AQLMarketData::isCalibrateModel(model))
	{
		// set volatility
		pvanilla->getIRVolatilitys().convertFromString(irvols);
	}
	// set fx
	if (!fxcurs.empty())
	{
		pvanilla->getFXCurrencys().set(fxcurs);
		AQLString fxName = AQLCoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
		if (fxName == AQ_NO_DATA)
		{
			throw AQLCoreInvalidData("Forward FX Object is not set.", __FILE__, __LINE__);
		}
		pvanilla->getFXEntity().convertFromString(fxName);
		AQLString fxvols = PREFIX_VOL + mpStaticData->getStaticData(fxcurs[0].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		for (unsigned int i = 1; i < fxcurs.size(); i++)
		{
			AQLString tmp = PREFIX_VOL + mpStaticData->getStaticData(fxcurs[i].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
			fxvols += ":" + tmp;
		}
		pvanilla->getFXVolatilitys().convertFromString(fxvols);
	}

	return;
}

// 
/*!
    @brief setup extra object

	@param[out] object pool
*/
void
AQLObjectConfigurationVanilla::setUpExtraEntity(AQLObjectPool &objPool) const
{
	
	/////////////////////////////
	// setup extra object
	/////////////////////////////
	setUpFwdInterpolation(objPool);
	AQLString mainTradeName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
	AQLObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);

	AQLString calcpayoffstr = mpStaticData->getStaticData(KEY_DEAL_ISCALCPAYOFFAFTERMATURITY).toUpper();
	bool iscalcpayoffaftermatu = (calcpayoffstr == "TRUE");

	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString temp = staticData.getStaticData(KEY_SIMULATION_FUNDING_CHANGED);
      
	//set mLibor map for stub swap
	//setUpLiborRateMap(objPool);
	
	if (dynamic_cast<const AQLDataValuation &>
		(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const AQLDataMultiReference &unders = dynamic_cast<const AQLDataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			//set up asofdate for trade object
			setUpAsOfDateForTradeEntity(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));

			setUpCallSpreadForFXDigitalOption(dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			setUpCallSpreadForIRDigitalOption(dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			//for stub swap
			//setUpStubCoefficient(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));

			if (iscalcpayoffaftermatu)
			{
				unders.get(i).get().remove(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY);
				unders.get(i).get().add(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY, new AQLDataBool(iscalcpayoffaftermatu));
				
			}

			if (temp==AQ_NO_DATA)
			{
				setUpRecalcTrade(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
            }

			setUpRenotionalFXInfo(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));

			setUpConvexityAdjustInfo(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			
			setUpDirtyPrice(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));	
			// setUpPastIndexRate
			setUpPastIndexRate(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			//set FundingSpread 
			setUpFundingSpread(objPool,dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));

			setUpSpotRateRatioMethod(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));

			const bool isMarkov = setUpMarkovFunctionalParameter(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));

			if (isMarkov) {
				setUpExtraLibor(objPool, dynamic_cast<AQLMathObjectValue &>(unders.get(i).get()));
			}
		}
	}
	else
	{
		setUpCallSpreadForFXDigitalOption(dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		setUpCallSpreadForIRDigitalOption(dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

		//for stub swap
		//setUpStubCoefficient(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

		//set up asofdate for trade object
		setUpAsOfDateForTradeEntity(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

		if (iscalcpayoffaftermatu)
		{
			objHolder.get().remove(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY);
			objHolder.get().add(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY, new AQLDataBool(iscalcpayoffaftermatu));
		}

		if (temp==AQ_NO_DATA)
		{
			setUpRecalcTrade(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
        }

		setUpRenotionalFXInfo(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

		setUpConvexityAdjustInfo(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		
		setUpDirtyPrice(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));		
		// setUpPastIndexRate
		setUpPastIndexRate(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		//set FundingSpread 
		setUpFundingSpread(objPool,dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

		setUpSpotRateRatioMethod(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

		const bool isMarkov = setUpMarkovFunctionalParameter(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));

		if (isMarkov) {
			setUpExtraLibor(objPool, dynamic_cast<AQLMathObjectValue &>(objHolder.get()));
		}
	}
}

// 
/*!
    @brief setUpCallSpreadForFXDigitalOption

	set digitalspread from property file
	
	@param[in] trade object
	@param[out] trade object
*/
void
AQLObjectConfigurationVanilla::setUpCallSpreadForFXDigitalOption(AQLMathObjectValue &trade) const
{
	const AQLDataValuation& valueMethod = dynamic_cast<const AQLDataValuation &>
			(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

	if (!valueMethod.isTypeOf(FN_FXDIGITALOPTIONVALUE) && 
		!valueMethod.isTypeOf(FN_FXDIGITALCALLSPREADOPTIONVALUE) &&
		!valueMethod.isTypeOf(FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE))
		return;

	AQLDataHolder* dh;

	//excel calculation mode
	dh = &(trade.getData(PRICING_DATA_DIGITALSPREAD,NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		return;
	
	dh = &(trade.getData(PRICING_DATA_DOMESTICCURRENCY, ISNOTNULL));
	AQLString domcur = dynamic_cast<AQLDataString &>(dh->get()).get();

	dh = &(trade.getData(PRICING_DATA_FOREIGNCURRENCY, ISNOTNULL));
	AQLString forcur = dynamic_cast<AQLDataString &>(dh->get()).get();

	AQLString cur = domcur.toLower() + '/' + forcur.toLower();
	
	
	AQLString iscallspread = mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_ISCALLSPREAD).toUpper();
	if (iscallspread == "TRUE")
	{
		if (valueMethod.isTypeOf(FN_FXDIGITALOPTIONVALUE))
			trade.getData(CALIBRATION_DATA_VALUE).convertFromString(FN_FXDIGITALCALLSPREADOPTIONVALUE_STR);
		//spreadval
		AQLString spreadval =  mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_CALLSPREADVALUE);
		trade.remove(PRICING_DATA_DIGITALSPREAD);
		trade.AQLObject::add(PRICING_DATA_DIGITALSPREAD, new AQLDataDouble()).convertFromString(spreadval);

		//buyselldistinguishflag
		AQLString buysellflag =  mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_BUYSELLDISTINGUISH);
		trade.remove(PRICING_DATA_BUYSELLDISTINGUISH);
		trade.AQLObject::add(PRICING_DATA_BUYSELLDISTINGUISH, new AQLDataBool()).convertFromString(buysellflag);



	}
	else if (iscallspread == "FALSE" && valueMethod.isTypeOf(FN_FXDIGITALCALLSPREADOPTIONVALUE))
		trade.getData(CALIBRATION_DATA_VALUE).convertFromString(FN_FXDIGITALOPTIONVALUE_STR);
	else
		throw AQLCoreInvalidData("Call spread property error",__FILE__,__LINE__);

	return;
}

// 
/*!
@brief setUpCallSpreadForIRDigitalOption

set digitalspread from property file

@param[in] trade object
@param[out] trade object
*/
void
AQLObjectConfigurationVanilla::setUpCallSpreadForIRDigitalOption(AQLMathObjectValue &trade) const
{
	const AQLDataValuation& valueMethod = dynamic_cast<const AQLDataValuation &>
		(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

	if (!valueMethod.isTypeOf(FN_IR_CAPFLOOROPTIONVALUE))
		return;

	AQLDataHolder* dh;

	//excel calculation mode
	dh = &(trade.getData(PRICING_DATA_DIGITALSPREAD, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		return;

	dh = &trade.getData(PRICING_DATA_PREMIUMCURRENCY, ISNOTNULL);
	AQLString cur = dynamic_cast<AQLDataString &>(dh->get()).get();
	cur.toLower();

	AQLString iscallspread = mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_ISCALLSPREAD).toUpper();
	if (iscallspread == "TRUE")
	{
		//spreadval
		AQLString spreadval = mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_CALLSPREADVALUE);
		trade.remove(PRICING_DATA_DIGITALSPREAD);
		trade.AQLObject::add(PRICING_DATA_DIGITALSPREAD, new AQLDataDouble()).convertFromString(spreadval);
	}

	return;
}

// 
/*!
    @brief setUpLiborRateMap

	set digitalspread from property file
	
	@param[in] object pool
	
*/
void 
AQLObjectConfigurationVanilla::setUpLiborRateMap(AQLObjectPool &objPool) const
{
	AQLDataHolder* dh;
	AQLMathPlainVanillaEntity *pvanilla = AQLMarketData::getPlainVanillaEntity(objPool);
	if (!pvanilla)
	{
		throw AQLCoreInvalidData("Plain Vanilla object is not set in entitiy pool", __FILE__, __LINE__);
	}
	AQLDate asOf = pvanilla->getAsOfDate().get();
	AQLPriceDataDayCount dc(ACT_365_ISDA);


	const AQLStringVector &ircurs = pvanilla->getIRCurrencys().get();
	for (unsigned int i = 0; i < ircurs.size(); i++)
	{
		AQLString ccy = ircurs[i];
		ccy.toLower();

		AQLString yldproname =  AQLMarketData::getBaseYieldProName(ccy);
		AQLMathYieldCurvePro &bYieldPro = dynamic_cast<AQLMathYieldCurvePro &>
						(objPool.getObject(yldproname, ENCHKTYPE_ISDEFINED).get());

		const AQLDataMultiReference &refMarketDatas = bYieldPro.getMarketData();

		const unsigned int dataSize =refMarketDatas.getSize();
		if (dataSize == 0)
		{
			return;
		}
		DoubleVector termvec,ratevec;
		for (unsigned int j = 0; j < dataSize; ++j)
		{
			AQLObject *data = &refMarketDatas.get(j).get();
			AQLString type = dynamic_cast<const AQLDataString&> ((data->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			type.toUpper();
			if (type !=  YIELD_TYPE_ZERO)
				continue;

			double rate  = dynamic_cast<AQLDataDouble &>(data->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();
			const AQLString termStr = dynamic_cast<const AQLDataString &>(data->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();
			const AQLDate& spotdate = dynamic_cast<const AQLDataDate&> ((data->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
			const AQLPriceDataCalendar& cal  = dynamic_cast<const AQLPriceDataCalendar&> ((data->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const AQLPriceDataSlidingRule& sld  = dynamic_cast<const AQLPriceDataSlidingRule&> ((data->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			const AQLString freq = dynamic_cast<const AQLDataString &>(data->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
			const AQLDataBool& eom  = dynamic_cast<const AQLDataBool&> ((data->getData(IR_CALIBRATION_DATA_ISEOMROLL, ISNOTNULL)).get());
			AQLString roll_conv("");
			if (freq == "LUNAR") roll_conv = "LUNAR";
			else if (eom) roll_conv = "EOM";
			else roll_conv = "NORMAL";

			AQLDate enddate = AQLMathDateCalculations::getDate(spotdate, termStr, sld, &cal, true, &roll_conv);
		
			double term = dc.getTerm(spotdate,enddate,false);

			termvec.push_back(term);
			ratevec.push_back(rate);
		}
		if (termvec.size() < 1)
		{
			bool isFwdFX = false;
			const AQLDataHolder& dh = bYieldPro.getYieldData().get().getData(IR_CALIBRATION_DATA_ISFWDFX);
			if (dh.isDefined() && !dh.isNull()) 
				isFwdFX = dynamic_cast<const AQLDataBool&>(dh.get()).get();
			if (!isFwdFX)
				throw AQLCoreInvalidData("Libor Rate Map Error",__FILE__,__LINE__);
		}
		
		AQLLinearInterpolation* pInter = new AQLLinearInterpolation();
		pInter->set(termvec, ratevec);

		mLiborRateMap.insert(std::make_pair(ircurs[i], pInter));
		mLiborGridTermMap.insert(std::make_pair(ircurs[i], termvec));
	}
}

//// 
///*!
//    @brief setUpStubCoefficient
//
//	set digitalspread from property file
//	
//	@param[in] object pool
//	@param[out] trade object
//*/
//void
//AQLObjectConfigurationVanilla::setUpStubCoefficient(AQLObjectPool &objPool,  AQLMathObjectValue &trade) const
//{
//	
//	const AQLDataValuation& valuemehod = dynamic_cast<const AQLDataValuation &>
//			(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
//	if (!valuemehod.isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) 
//		&& !valuemehod.isTypeOf(FN_IR_SWAPTIONVALUEFROMCASHFLOW))
//		return;
//	
//	AQLDataHolder* dh;
//
//	//check stub or not
//	bool isoddtrade = false;
//	dh = &(trade.getData(PRICING_DATA_ISODDTRADE, NOCHECK));
//	if (dh->isDefined() && !dh->isNull())
//		isoddtrade =  dynamic_cast<const AQLDataBool &>(dh->get()).get();
//	if (!isoddtrade)
//		return;
//	
//	//dh = &(trade.getData(PRICING_DATA_ODDINDEXTYPE, NOCHECK));
//	//if (!dh->isDefined() || dh->isNull())
//	//	return;
//
//	AQLPriceDataDayCount dc(ACT_365_ISDA);
//	AQLMathPlainVanillaEntity *pvanilla = AQLMarketData::getPlainVanillaEntity(objPool);
//	if (!pvanilla)
//	{
//		throw AQLCoreInvalidData("Plain Vanilla object is not set in entitiy pool", __FILE__, __LINE__);
//	}
//	AQLDate asOf = pvanilla->getAsOfDate().get();
//
//	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
//	AQLDataMultiReference &legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
//	unsigned int legSize = legs.getSize();
//
//	for (unsigned int i = 0; i < legSize; i++)
//	{
//		AQLObject &eleg = legs.get(i).get();
//		//get cashlet
//		dh = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
//		AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(dh->get());
//		unsigned int cashletSize = cashlets.getSize();
//		for (unsigned int j = 0; j < cashletSize; j++)
//		{
//			AQLObject &ecashlet = cashlets.get(j).get();
//			//get firstcoupon
//			dh = &(ecashlet.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
//			if (!dh->isDefined() || dh->isNull())
//				continue;
//
//			AQLDataMultiReference &coupons = dynamic_cast<AQLDataMultiReference &>(dh->get());
//			AQLObject &firstcoupon = coupons.get(0).get();
//
//			//check whether firstcoupon is stubperiod or not
//			
//			//check fn_linear or not
//			dh = &(firstcoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
//			AQLPriceDataFunction &method = dynamic_cast<AQLPriceDataFunction &>(dh->get());
//			if (!method.isTypeOf(FN_LINEAR))
//				continue;
//		
//			//check coefficient is -9999 or not
//			dh = &(firstcoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
//			AQLDataMultiReference &indexs = dynamic_cast<AQLDataMultiReference &>(dh->get());
//			unsigned int indexSize = indexs.getSize();
//
//			dh = &(firstcoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
//			AQLDataDoubles &coeffs = dynamic_cast<AQLDataDoubles &>(dh->get());
//			DoubleVector coefvec = coeffs.get();
//
//			// in case of stub setting but index is 1 
//			if (coefvec.size() == 2 && coefvec[0] == -9999.0 && indexSize == 1)
//			{
//				coefvec[0] = 1.0;
//				//set
//				coeffs.set(coefvec);
//				continue;
//			}
//
//			//index must be more than 2
//			if (coefvec.size() != indexSize + 1 || indexSize < 2)
//				continue;
//
//			bool isstubcoupon = true;
//			for (unsigned k = 0; k < coefvec.size() - 1; k++)
//			{
//				if (coefvec[k] != -9999.0)
//				{	
//					isstubcoupon = false;
//					break;
//				}
//			}
//			if (isstubcoupon)
//			{
//
//				for (unsigned k = 0; k < coefvec.size() - 1; k++)
//					coefvec[k] = 0.0;
//
//				dh = &(firstcoupon.getData(PRICING_DATA_ODDINDEXTYPE, ISNOTNULL));
//				AQLString stubtype =  dynamic_cast<AQLDataString &>(dh->get()).get();
//				stubtype.toUpper();
//				
//				if (stubtype == CURRENTINDEX)
//				{
//					//first index is main index
//					coefvec[0] = 1.0;
//					coeffs.set(coefvec);
//					continue;
//				
//				}
//				else if (stubtype == STARTENDDATEMETHOD)
//				{
//					//first index is main index
//					DateVector datevec(2);
//					dh = &(ecashlet.getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL));
//					datevec[0] = dynamic_cast<AQLDataDate &>(dh->get()).get();
//					dh = &(ecashlet.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL));
//					datevec[1] = dynamic_cast<AQLDataDate &>(dh->get()).get();
//
//
//					AQLObject &eindex = indexs.get(0).get();
//					dh = &(eindex.getData(PRICING_DATA_DATESFORINDEXGENERATE, NOCHECK));
//					
//					if (dh->isDefined() && !dh->isNull())
//					{
//						dynamic_cast<AQLDataDates &>(dh->get()).set(datevec);
//					}
//					else
//					{
//						eindex.add(PRICING_DATA_DATESFORINDEXGENERATE, new AQLDataDates(datevec));
//					}
//
//					coefvec[0] = 1.0;
//					coeffs.set(coefvec);
//					continue;
//				}
//				
//				//get currency
//				dh = &(firstcoupon.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL));
//				AQLString ccy = dynamic_cast<AQLDataString &>(dh->get()).get();
//				//exchange stubcoupon
//				dh = &(ecashlet.getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL));
//				const AQLDate &startdate = dynamic_cast<AQLDataDate &>(dh->get()).get();
//				dh = &(ecashlet.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL));
//				const AQLDate &enddate = dynamic_cast<AQLDataDate &>(dh->get()).get();
//				double term = dc.getTerm(startdate,enddate,false);
//				if (stubtype == SPOTRATERATIOMETHOD)
//				{
//					std::map<AQLString, AQLInterpolationBase*>::iterator it = mLiborRateMap.find(ccy);
//					if (it == mLiborRateMap.end())
//						throw AQLCoreInvalidData("Odd Coefficient Error",__FILE__,__LINE__);
//
//					AQLLinearInterpolation* inter = dynamic_cast<AQLLinearInterpolation* >(it->second);
//					double targetrate = inter->value(term);
//
//					//change ratio
//					map<double, unsigned int> mapRatePos;
//					for (unsigned l = 0 ; l < indexSize; l++)
//					{
//						AQLObject &eindex = indexs.get(l).get();
//						dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
//						AQLString termStr = dynamic_cast<AQLDataString &>(dh->get()).get();
//
//						dh = &(eindex.getData(PRICING_DATA_SPOTLAG, ISNOTNULL));
//						int spotlag = dynamic_cast<AQLDataInt &>(dh->get()).get();
//
//						dh = &(eindex.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
//						AQLPriceDataSlidingRule &sld = dynamic_cast<AQLPriceDataSlidingRule &>(dh->get());
//
//						dh = &(eindex.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
//						AQLPriceDataCalendar &cal = dynamic_cast<AQLPriceDataCalendar &>(dh->get());
//						
//						AQLDate spotdate = cal.getBusinessDay(asOf,spotlag);
//						AQLDate enddate = AQLMathDateCalculations::getDate(spotdate, termStr, true);
//						enddate = (!sld.isNull() && !cal.isNull()) ? sld.getDate(enddate, cal) : enddate;
//						
//						double term = dc.getTerm(spotdate,enddate,false);
//						double rate = inter->value(term);
//
//						mapRatePos.insert(std::make_pair(rate,l));
//					}
//
//					map<double, unsigned int>::iterator itRate = mapRatePos.lower_bound(targetrate);
//					double smallrate,bigrate;
//					unsigned int smallpos, bigpos;
//					if (itRate == mapRatePos.begin())
//					{
//						smallrate = itRate->first;
//						smallpos = itRate->second;
//						
//						bigrate = (++itRate)->first;
//						bigpos = itRate->second;
//					}
//					else if (itRate == mapRatePos.end())
//					{
//						bigrate = (--itRate)->first;
//						bigpos = itRate->second;
//						
//						smallrate = (--itRate)->first;
//						smallpos = itRate->second;
//					}
//					else
//					{
//						bigrate = itRate->first;
//						bigpos = itRate->second;
//						
//						smallrate = (--itRate)->first;
//						smallpos = itRate->second;
//					}
//					coefvec[smallpos] = (bigrate-targetrate)/(bigrate-smallrate);
//					coefvec[bigpos] = (targetrate-smallrate)/(bigrate-smallrate);
//				}
//				else
//				{
//					double targetterm = 0.0;
//					std::map<AQLString, DoubleVector>::iterator itvec = mLiborGridTermMap.find(ccy);
//					if (itvec == mLiborGridTermMap.end())
//						throw AQLCoreInvalidData("Odd Coefficient Error",__FILE__,__LINE__);
//					DoubleVector termvec = itvec->second;
//					DoubleVector::iterator itTargetTerm = std::upper_bound(termvec.begin(),termvec.end(),term);
//					if (itTargetTerm == termvec.end())
//						--itTargetTerm;
//
//					if (stubtype == NEXTINDEX)
//					{
//						targetterm = *itTargetTerm;
//					}
//					else if(stubtype == PREVIOUSINDEX)
//					{
//						if (itTargetTerm == termvec.begin())
//							targetterm = *itTargetTerm;
//						else
//							targetterm = *(--itTargetTerm);
//					}
//					else if (stubtype == CLOSESTINDEX)
//					{
//						if (itTargetTerm == termvec.begin())
//							targetterm = *itTargetTerm;
//						else
//						{
//							double afterm = *itTargetTerm;
//							double bfterm = *(--itTargetTerm);
//
//							targetterm = (afterm - term > term - bfterm) ? bfterm : afterm;
//						}
//					}
//					else if (stubtype == GRIDTERMRATIOMETHOD)
//					{
//						targetterm = term;
//					}
//					else
//						throw AQLCoreInvalidData("Stub Type Error",__FILE__,__LINE__);
//					
//					//change ratio
//					map<double, unsigned int> mapTermPos;
//					for (unsigned l = 0 ; l < indexSize; l++)
//					{
//						AQLObject &eindex = indexs.get(l).get();
//						dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
//						AQLString termStr = dynamic_cast<AQLDataString &>(dh->get()).get();
//
//						dh = &(eindex.getData(PRICING_DATA_SPOTLAG, ISNOTNULL));
//						int spotlag = dynamic_cast<AQLDataInt &>(dh->get()).get();
//
//						dh = &(eindex.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
//						AQLPriceDataSlidingRule &sld = dynamic_cast<AQLPriceDataSlidingRule &>(dh->get());
//
//						dh = &(eindex.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
//						AQLPriceDataCalendar &cal = dynamic_cast<AQLPriceDataCalendar &>(dh->get());
//						
//						AQLDate spotdate = cal.getBusinessDay(asOf,spotlag);
//						AQLDate enddate = AQLMathDateCalculations::getDate(spotdate, termStr, true);
//						enddate = (!sld.isNull() && !cal.isNull()) ? sld.getDate(enddate, cal) : enddate;
//						
//						double term = dc.getTerm(spotdate,enddate,false);
//						mapTermPos.insert(std::make_pair(term,l));
//					}
//					
//					map<double, unsigned int>::iterator itTerm = mapTermPos.lower_bound(targetterm);
//					double smallterm,bigterm;
//					unsigned int smallpos, bigpos;
//					if (itTerm == mapTermPos.begin())
//					{
//						smallterm = itTerm->first;
//						smallpos = itTerm->second;
//						
//						bigterm = (++itTerm)->first;
//						bigpos = itTerm->second;
//					}
//					else if (itTerm == mapTermPos.end())
//					{
//						bigterm = (--itTerm)->first;
//						bigpos = itTerm->second;
//						
//						smallterm = (--itTerm)->first;
//						smallpos = itTerm->second;
//					}
//					else
//					{
//						bigterm = itTerm->first;
//						bigpos = itTerm->second;
//						
//						smallterm = (--itTerm)->first;
//						smallpos = itTerm->second;
//					}
//					coefvec[smallpos] = (bigterm-targetterm)/(bigterm-smallterm);
//					coefvec[bigpos] = (targetterm-smallterm)/(bigterm-smallterm);
//				}
//				//set
//				coeffs.set(coefvec);
//			}
//		}
//	}
//}


void
AQLObjectConfigurationVanilla::setUpRenotionalFXInfo(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{
	const AQLDataValuation& attrval = dynamic_cast<const AQLDataValuation& >(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
	const AQLCoreValuation* pVal = &attrval.getMethod();
	if (pVal->isTypeOf(FN_IR_CONVERGENCEVALUE))
	{
		const AQLDataValuation& attrval = dynamic_cast<const AQLDataValuation& >(trade.getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
		pVal = &attrval.getMethod();
	}

	if (!pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE))
	{
		return;
	}

	//get ir properties
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	//get leg object
	AQLDataMultiReference &legs = dynamic_cast<AQLDataMultiReference &>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	unsigned int legSize = legs.getSize();
	for (unsigned int i = 0; i < legSize; i++)
	{
		AQLObject &leg = legs.get(i).get();
		//get currency
		AQLString ccy = dynamic_cast<AQLDataString &>(leg.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
		ccy.toLower();
		//get renotional FX information
		bool isRenAdj = false;
		AQLString tmpRenAdj_str = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST).toUpper();
		if (tmpRenAdj_str != AQ_NO_DATA)
		{
			AQLDataBool tmpAttrB;
			tmpAttrB.convertFromString(tmpRenAdj_str);
			isRenAdj = tmpAttrB.get();
		}

		leg.remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
		leg.add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new AQLDataBool(isRenAdj));
		leg.remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET);
		leg.add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new AQLDataBool(isRenAdj));
	}
	
	return;
}

void
AQLObjectConfigurationVanilla::setUpConvexityAdjustInfo(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{
	//const AQLDataValuation& attrval = dynamic_cast<const AQLDataValuation& >(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
	//const AQLCoreValuation* pVal = &attrval.getMethod();
	//if (pVal->isTypeOf(FN_IR_CONVERGENCEVALUE))
	//{
	//	const AQLDataValuation& attrval = dynamic_cast<const AQLDataValuation& >(trade.getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
	//	pVal = &attrval.getMethod();
	//}

	//if (!pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE))
	//{
	//	return;
	//}

	//get ir properties
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	// trade
	AQLDataHolder* dh;
	dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
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

									if (indexType == LIBOR)
									{
										// get value method
										const AQLDataValuation& attrval = dynamic_cast<const AQLDataValuation&>(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
										const AQLCoreValuation* pVal = &attrval.getMethod();

										bool isDelayedSwapConvexityAdjusted = false;
										if (pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE))
										{
											// convexity adjust model
											dh = &(indexInfos.get(l).getData(PRICING_DATA_CONVEXITYADJUSTMENT, NOCHECK));
											if (dh->isDefined() && !dh->isNull())
											{
												AQLString caModel = dynamic_cast<AQLDataString&>(dh->get()).get();
												caModel.toUpper();
												indexInfos.get(l).add(PRICING_DATA_CAMODEL, new AQLDataString()).convertFromString(caModel);
												isDelayedSwapConvexityAdjusted = AQLMathIndexEntity::isDelayedConvexityAdjustModel(caModel);
											}
										}
										if (isDelayedSwapConvexityAdjusted || pVal->isTypeOf(FN_IR_CAPFLOOROPTIONVALUE))
										{
											//get currency
											AQLString ccy = dynamic_cast<AQLDataString &>(indexInfos.get(l).getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
											ccy.toLower();
											AQLString tmp_prop;
											// threshold
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_THRESHOLD).toUpper();
											indexInfos.get(l).add(PRICING_DATA_CATHRESHOLD, new AQLDataDouble()).convertFromString(tmp_prop);
										}
									}
									else if (indexType == CMS)
									{
										//CFCalcStartEndDate
										std::vector<AQLDate> cfcalc_startDate;
										std::vector<AQLDate> cfcalc_endDate;
										for (unsigned int m = 0; m < cashlets.getSize(); ++m)
										{
											dh = &(cashlets.get(m).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
											if (dh->isDefined() && !dh->isNull())
											{
												// cashlets
												cfcalc_startDate.push_back(dynamic_cast<const AQLDataDate &>(cashlets.get(m).get().getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get());
												cfcalc_endDate.push_back(dynamic_cast<const AQLDataDate &>(cashlets.get(m).get().getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL).get()).get());
											}
										}
										// get value method
										const AQLDataValuation& attrval = dynamic_cast<const AQLDataValuation& >(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
										const AQLCoreValuation* pVal = &attrval.getMethod();
										if (pVal->isTypeOf(FN_IR_CONVERGENCEVALUE))
										{
											const AQLDataValuation& attrval = dynamic_cast<const AQLDataValuation& >(trade.getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
											pVal = &attrval.getMethod();
										}
										// convexity adjust is 0 in case of capfloor
										if (pVal->isTypeOf(FN_IR_CAPFLOOROPTIONVALUE))
										{
											indexInfos.get(l).add(PRICING_DATA_CAMODEL, new AQLDataString()).convertFromString(CMS_CA_ZERO);
										}
										else
										{
											//get currency
											AQLString ccy = dynamic_cast<AQLDataString &>(indexInfos.get(l).getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
											ccy.toLower();
											AQLString tmp_prop;
											// model
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_MODEL).toUpper();
											indexInfos.get(l).add(PRICING_DATA_CAMODEL, new AQLDataString()).convertFromString(tmp_prop);
											// threshold
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_THRESHOLD).toUpper();
											indexInfos.get(l).add(PRICING_DATA_CATHRESHOLD, new AQLDataDouble()).convertFromString(tmp_prop);
											// integral condition
											DoubleArray intCondi;
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_INTEGRALUPPERBOUND).toUpper();
											intCondi.push_back(tmp_prop.getDoubleValue());
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_INTEGRALLOWERBOUND).toUpper();
											intCondi.push_back(tmp_prop.getDoubleValue());
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_INTEGRALSTEPS).toUpper();
											intCondi.push_back(tmp_prop.getDoubleValue());
											indexInfos.get(l).add(PRICING_DATA_CAINTEGRALCONDITION, new AQLDataDoubles(intCondi));
											// CFStartdate CFEnddate
											indexInfos.get(l).add(PRICING_DATA_CFCALCSTARTDATES, new AQLDataDates(cfcalc_startDate));
											indexInfos.get(l).add(PRICING_DATA_CFCALCENDDATES, new AQLDataDates(cfcalc_endDate));
											// swaption premium integral step
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_PREMIUMINTEGRALSTEPS).toUpper(); 
											if (tmp_prop != AQ_NO_DATA)
											{
												int integralStep = tmp_prop.getIntValue();
												indexInfos.get(l).add(PRICING_DATA_CAPREMIUMINTEGRALSTEP, new AQLDataInt(integralStep));
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

void
AQLObjectConfigurationVanilla::setUpSpotRateRatioMethod(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLDataHolder* dh;
	dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (!dh->isDefined() || dh->isNull())
		return;

	AQLDataMultiReference &legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
	for (unsigned int i = 0; i < legs.getSize(); ++i)
	{
		dh = &legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK);
		if(!dh->isDefined() || dh->isNull())
			continue;

		const AQLDataMultiReference &cashlets = dynamic_cast<const AQLDataMultiReference &>(dh->get());
		for (unsigned int j = 0; j < cashlets.getSize(); ++j)
		{
			dh = &(cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if(!dh->isDefined() || dh->isNull())
				continue;

			AQLDataMultiReference &couponinfos = dynamic_cast<AQLDataMultiReference &>(dh->get());	
			for (unsigned int k = 0; k < couponinfos.getSize(); ++k)
			{
				dh = &(couponinfos.get(k).getData(PRICING_DATA_ODDINDEXTYPE, NOCHECK));
				if(!dh->isDefined() || dh->isNull())
					continue;

				AQLString currency = dynamic_cast<const AQLDataString &>(couponinfos.get(k).getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
				AQLString tolerance = staticData.getStaticData(currency.toLower() + STATIC_DATA_KEY_YIELD_SPOTRATERATIOMETHOD_TOLERANCE);
				if(tolerance != AQ_NO_DATA)
					couponinfos.get(k).add(PRICING_DATA_TOLERANCEFORSPOTRATERATIOMETHOD, new AQLDataDouble()).convertFromString(tolerance);
			}
		}
	}

	return; 
}

bool AQLObjectConfigurationVanilla::setUpMarkovFunctionalParameter(AQLObjectPool &objPool, AQLMathObjectValue &trade) const
{
	const AQLDataValuation& valuemehod = dynamic_cast<const AQLDataValuation &>
		(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

	//if (!valuemehod.isTypeOf(FN_IR_NDSSWAPTIONVALUE))
	//	return false;

	AQLDataHolder* dh;

	//excel calculation mode
	AQLString cur;
	dh = &(trade.getData(PRICING_DATA_CURRENCY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		cur = dynamic_cast<AQLDataString &>(dh->get()).get();
	}
	else
	{
		dh = &(trade.getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			cur = dynamic_cast<AQLDataString &>(dh->get()).get();
		}
		else
		{
			dh = &(trade.getData(PRICING_DATA_CURRENCY, ISNOTNULL)); // throw
		}
	}
	AQLString markovMeanReversion = mpStaticData->getStaticData(cur.toLower() + STATIC_DATA_KEY_DEAL_MARKOVFUNCTIONAL_MEANREVERSION).toUpper();

	//trade.remove(PRICING_DATA_MARKOVFUNCTIONAL_MEANREVERSION);
	//trade.AQLObject::add(PRICING_DATA_MARKOVFUNCTIONAL_MEANREVERSION, new AQLDataDouble()).convertFromString(markovMeanReversion);

	return true;
}
