/*! @file
    @brief Object generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAObjectConfigurationVanilla.cpp
//
//  DESCRIPTION :        Object setupper
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


#include "LAObjectConfigurationVanilla.h"
#include "LAString.h"
#include "LADataBasics.h"
#include "LAPriceDataDayCount.h"
#include "LACoreDataService.h"
#include "LAMarketDataHW.h"
#include "LADefinitions.h"
#include "LAObjectConfiguration.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAMarketData.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "LADataReference.h"
#include "LADataValuation.h"
#include "LAPricePortfolioValue.h"
#include "LAMathValuableEntity.h"
#include "LALinearRatesOptionValue.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LAPriceTradeValue.h"
#include "LALinearRatesSwapTradeValue.h"
#include "LALinearFunc.h"
#include "LAPriceDataFunction.h"
#include "LAMathIndexEntity.h"
#include "LAMathYieldCurvePro.h"
#include "LAPriceArbFreeGenerator.h"
#include "LALinearInterpolation.h"
#include "LAPriceConvergenceValue.h"
#include "LAMathIRVanillaFuncUtility.h"
//#include "LAPriceNDSSwaptionValue.h"
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
LAObjectConfigurationVanilla::LAObjectConfigurationVanilla()
: mLiborRateMap(), mLiborGridTermMap(),LAObjectConfiguration()
//: LAObjectConfiguration()
{
}

// destructor
/*!

*/
LAObjectConfigurationVanilla::~LAObjectConfigurationVanilla(void)
{
	std::map<LAString, LAInterpolationBase *>::iterator it = mLiborRateMap.begin();
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
LAObjectConfigurationVanilla::setUpValuableEntity( LAObjectPool& objPool ) const
{
	LAString mTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(mTradeName, ENCHKTYPE_ISDEFINED);

	LAString IsAddFwdPremPV = mpStaticData->getStaticData(KEY_SDE_PV_ADDITION_FWDPREMIUM);
 
    LAString zeroCalc = LACoreDataService::getContext(ARG_KEY_ZEROCALC);
    if (zeroCalc == MLIB_NO_DATA) {
        zeroCalc = "FALSE";
    }

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
		

            // set zero calculation
            trade.remove(PRICING_DATA_ZEROCALC);
            trade.add(PRICING_DATA_ZEROCALC, new LADataBool()).convertFromString(zeroCalc);
		
			//for past libor auto fixing
			trade.remove(PRICING_DATA_ISSAVEPASTFIXING);
			trade.add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(true));

			// set Compounding fixed rates
			LADate asOfDate = LAMarketData::getAsofDate(objPool);
			setUpCompoundingFixedRates(asOfDate, trade);

			setUpRiskInfo(riskEVec, trade.get());

			// set flag for adding fwdpremium to PV
			if (IsAddFwdPremPV != MLIB_NO_DATA)
			{	
				trade.remove(PRICING_DATA_ISADDFWDPREMPV);
				trade.add(PRICING_DATA_ISADDFWDPREMPV, new LADataBool()).convertFromString(IsAddFwdPremPV);
			}
		}
		
		if (!riskEVec.empty())
		{
			objHolder.remove(PRICING_DATA_RISKCALCINFOS);
			objHolder.add(PRICING_DATA_RISKCALCINFOS, new LADataMultiReference()).convertFromString(riskENames_str);
		}
		
		LADataInstance *dataInstance = objHolder.getDataInstance();
		dataInstance->getReferencePool().completeDependency();

	}
	else
	{

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

        // set zero calculation
        objHolder.remove(PRICING_DATA_ZEROCALC);
        objHolder.add(PRICING_DATA_ZEROCALC, new LADataBool()).convertFromString(zeroCalc);

		//for past libor auto fixing
		objHolder.remove(PRICING_DATA_ISSAVEPASTFIXING);
		objHolder.add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(true));

		// set Compounding fixed rates
		LADate asOfDate = LAMarketData::getAsofDate(objPool);
		setUpCompoundingFixedRates(asOfDate, objHolder);

		setUpRiskInfo(riskEVec, objHolder.get());
	}

}



// setup  pathentity
void 
LAObjectConfigurationVanilla::setUpPathEntity(LAObjectPool &objPool, const LAString *pPathName) const
{
	LAMathPlainVanillaEntity *pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
	if (!pvanilla)
	{
		throw LACoreInvalidData("Plain Vanilla object is not set in entitiy pool", __FILE__, __LINE__);
	}	
	// set sdenames and models
	LAStringVector ccys = MADealUtils::getSDECurrencys();
	LAStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	//if (simCurs.size() != 0)
	//{
	//	ccys = simCurs;
	//}
	const unsigned int ccySize = ccys.size();

	LAStringVector ircurs,irsimcurs,fxcurs;
	for (unsigned int i = 0; i < ccySize; i++)
	{
		LAString cur = ccys[i];
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
		LAString cur = simCurs[i];
		if (-1 == cur.findString('/'))
		{
			irsimcurs.push_back(cur.toUpper());
		}
	}
	
	// set initial value
	// first ccy 
	pvanilla->getIRCurrencys().set(ircurs);
	pvanilla->getIRSimCurrencys().set(irsimcurs);
	LAString sdeName = mpStaticData->getStaticData(ircurs[0].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	LAString initialValue =  PREFIX_YIELD + sdeName;
	// set main path curve
	LAString pathCurve = mpStaticData->getStaticData(ircurs[0] + STATIC_DATA_KEY_YIELD_PATHCURVE);
	if (pathCurve == MLIB_NO_DATA)
	{
		pvanilla->getIRCurveTypes().push_back(STD);
	}
	else
	{
		pvanilla->getIRCurveTypes().push_back(pathCurve);
	}
	LAString irvols = PREFIX_VOL + sdeName;
	LAString model = LAMarketData::getModelName(ircurs[0]);

	//set curveproname
	LAString initialPro = LAMarketData::getBaseYieldProName(ircurs[0]);
	for (unsigned int i = 1; i < ircurs.size(); ++i)
	{
		LAString sdeName = mpStaticData->getStaticData(ircurs[i].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		initialValue += LAString(":") + PREFIX_YIELD + sdeName;
		// set main path curve
		pathCurve = mpStaticData->getStaticData(ircurs[i] + STATIC_DATA_KEY_YIELD_PATHCURVE);
		if (pathCurve == MLIB_NO_DATA)
		{
			pvanilla->getIRCurveTypes().push_back(STD);
		}
		else
		{
			pvanilla->getIRCurveTypes().push_back(pathCurve);
		}
		if (model != LAMarketData::getModelName(ircurs[i]))
		{
			throw LACoreInvalidData("Model is not same with main currency.", __FILE__, __LINE__);
		}
		irvols += LAString(":") + PREFIX_VOL + sdeName;

		initialPro += LAString(":") + LAMarketData::getBaseYieldProName(ircurs[i]);
	}
	// set curve
	pvanilla->getIRCurves().convertFromString(initialValue);
	pvanilla->setUpIRCurveTypes();

	pvanilla->getIRCurveProNames().convertFromString(initialPro);
	LAStaticData &riskProp = LACoreDataService::getStaticDataManager().getRiskStaticData();
	/*if ("TRUE" == riskProp.getStaticData(RISK_KEY_ANALYTIC_ISANALYTIC).toUpper())
	{
		for (unsigned int i =0; i < ircurs.size(); ++i)
		{
			LAString ccy = ircurs[i];
			pvanilla->getIRCurvePro(ccy.toUpper()).setCurveDependencyMap();
		}
	}*/
	//if ("ANALYTIC" == riskProp.getStaticData(RISK_KEY_ANALYTIC_CALCTYPE).toUpper())
	//{
	//	for (unsigned int i =0; i < ircurs.size(); ++i)
	//	{
	//		LAString ccy = ircurs[i];
	//		pvanilla->getIRCurvePro(ccy.toUpper()).setCurveDependencyMap();
	//	}
	//}


	
	if (LAMarketData::isCalibrateModel(model))
	{
		// set volatility
		pvanilla->getIRVolatilitys().convertFromString(irvols);
	}
	// set fx
	if (!fxcurs.empty())
	{
		pvanilla->getFXCurrencys().set(fxcurs);
		LAString fxName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
		if (fxName == MLIB_NO_DATA)
		{
			throw LACoreInvalidData("Forward FX Object is not set.", __FILE__, __LINE__);
		}
		pvanilla->getFXEntity().convertFromString(fxName);
		LAString fxvols = PREFIX_VOL + mpStaticData->getStaticData(fxcurs[0].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		for (unsigned int i = 1; i < fxcurs.size(); i++)
		{
			LAString tmp = PREFIX_VOL + mpStaticData->getStaticData(fxcurs[i].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
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
LAObjectConfigurationVanilla::setUpExtraEntity(LAObjectPool &objPool) const
{
	
	/////////////////////////////
	// setup extra object
	/////////////////////////////
	setUpFwdInterpolation(objPool);
	LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);

	LAString calcpayoffstr = mpStaticData->getStaticData(KEY_DEAL_ISCALCPAYOFFAFTERMATURITY).toUpper();
	bool iscalcpayoffaftermatu = (calcpayoffstr == "TRUE");

	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString temp = staticData.getStaticData(KEY_SIMULATION_FUNDING_CHANGED);
      
	//set mLibor map for stub swap
	//setUpLiborRateMap(objPool);
	
	if (dynamic_cast<const LADataValuation &>
		(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
	{
		// for portfolio
		const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
											(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		const unsigned int tradeSize = unders.getSize();
		for (unsigned int i = 0; i < tradeSize; ++i)
		{
			//set up asofdate for trade object
			setUpAsOfDateForTradeEntity(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));

			setUpCallSpreadForFXDigitalOption(dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			setUpCallSpreadForIRDigitalOption(dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			//for stub swap
			//setUpStubCoefficient(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));

			if (iscalcpayoffaftermatu)
			{
				unders.get(i).get().remove(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY);
				unders.get(i).get().add(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY, new LADataBool(iscalcpayoffaftermatu));
				
			}

			if (temp==MLIB_NO_DATA)
			{
				setUpRecalcTrade(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
            }

			setUpRenotionalFXInfo(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));

			setUpConvexityAdjustInfo(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			
			setUpDirtyPrice(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));	
			// setUpPastIndexRate
			setUpPastIndexRate(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			//set FundingSpread 
			setUpFundingSpread(objPool,dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));

			setUpSpotRateRatioMethod(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));

			const bool isMarkov = setUpMarkovFunctionalParameter(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));

			if (isMarkov) {
				setUpExtraLibor(objPool, dynamic_cast<LAMathObjectValue &>(unders.get(i).get()));
			}
		}
	}
	else
	{
		setUpCallSpreadForFXDigitalOption(dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		setUpCallSpreadForIRDigitalOption(dynamic_cast<LAMathObjectValue &>(objHolder.get()));

		//for stub swap
		//setUpStubCoefficient(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));

		//set up asofdate for trade object
		setUpAsOfDateForTradeEntity(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));

		if (iscalcpayoffaftermatu)
		{
			objHolder.get().remove(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY);
			objHolder.get().add(PRICING_DATA_ISCALCPAYOFFAFTERMATURITY, new LADataBool(iscalcpayoffaftermatu));
		}

		if (temp==MLIB_NO_DATA)
		{
			setUpRecalcTrade(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
        }

		setUpRenotionalFXInfo(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));

		setUpConvexityAdjustInfo(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		
		setUpDirtyPrice(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));		
		// setUpPastIndexRate
		setUpPastIndexRate(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
		//set FundingSpread 
		setUpFundingSpread(objPool,dynamic_cast<LAMathObjectValue &>(objHolder.get()));

		setUpSpotRateRatioMethod(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));

		const bool isMarkov = setUpMarkovFunctionalParameter(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));

		if (isMarkov) {
			setUpExtraLibor(objPool, dynamic_cast<LAMathObjectValue &>(objHolder.get()));
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
LAObjectConfigurationVanilla::setUpCallSpreadForFXDigitalOption(LAMathObjectValue &trade) const
{
	const LADataValuation& valueMethod = dynamic_cast<const LADataValuation &>
			(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

	if (!valueMethod.isTypeOf(FN_FXDIGITALOPTIONVALUE) && 
		!valueMethod.isTypeOf(FN_FXDIGITALCALLSPREADOPTIONVALUE) &&
		!valueMethod.isTypeOf(FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE))
		return;

	LADataHolder* dh;

	//excel calculation mode
	dh = &(trade.getData(PRICING_DATA_DIGITALSPREAD,NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		return;
	
	dh = &(trade.getData(PRICING_DATA_DOMESTICCURRENCY, ISNOTNULL));
	LAString domcur = dynamic_cast<LADataString &>(dh->get()).get();

	dh = &(trade.getData(PRICING_DATA_FOREIGNCURRENCY, ISNOTNULL));
	LAString forcur = dynamic_cast<LADataString &>(dh->get()).get();

	LAString cur = domcur.toLower() + '/' + forcur.toLower();
	
	
	LAString iscallspread = mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_ISCALLSPREAD).toUpper();
	if (iscallspread == "TRUE")
	{
		if (valueMethod.isTypeOf(FN_FXDIGITALOPTIONVALUE))
			trade.getData(CALIBRATION_DATA_VALUE).convertFromString(FN_FXDIGITALCALLSPREADOPTIONVALUE_STR);
		//spreadval
		LAString spreadval =  mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_CALLSPREADVALUE);
		trade.remove(PRICING_DATA_DIGITALSPREAD);
		trade.LAObject::add(PRICING_DATA_DIGITALSPREAD, new LADataDouble()).convertFromString(spreadval);

		//buyselldistinguishflag
		LAString buysellflag =  mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_BUYSELLDISTINGUISH);
		trade.remove(PRICING_DATA_BUYSELLDISTINGUISH);
		trade.LAObject::add(PRICING_DATA_BUYSELLDISTINGUISH, new LADataBool()).convertFromString(buysellflag);



	}
	else if (iscallspread == "FALSE" && valueMethod.isTypeOf(FN_FXDIGITALCALLSPREADOPTIONVALUE))
		trade.getData(CALIBRATION_DATA_VALUE).convertFromString(FN_FXDIGITALOPTIONVALUE_STR);
	else
		throw LACoreInvalidData("Call spread property error",__FILE__,__LINE__);

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
LAObjectConfigurationVanilla::setUpCallSpreadForIRDigitalOption(LAMathObjectValue &trade) const
{
	const LADataValuation& valueMethod = dynamic_cast<const LADataValuation &>
		(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

	if (!valueMethod.isTypeOf(FN_IR_CAPFLOOROPTIONVALUE))
		return;

	LADataHolder* dh;

	//excel calculation mode
	dh = &(trade.getData(PRICING_DATA_DIGITALSPREAD, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		return;

	dh = &trade.getData(PRICING_DATA_PREMIUMCURRENCY, ISNOTNULL);
	LAString cur = dynamic_cast<LADataString &>(dh->get()).get();
	cur.toLower();

	LAString iscallspread = mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_ISCALLSPREAD).toUpper();
	if (iscallspread == "TRUE")
	{
		//spreadval
		LAString spreadval = mpStaticData->getStaticData(cur + STATIC_DATA_KEY_DEAL_DIGITALOPTION_CALLSPREADVALUE);
		trade.remove(PRICING_DATA_DIGITALSPREAD);
		trade.LAObject::add(PRICING_DATA_DIGITALSPREAD, new LADataDouble()).convertFromString(spreadval);
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
LAObjectConfigurationVanilla::setUpLiborRateMap(LAObjectPool &objPool) const
{
	LADataHolder* dh;
	LAMathPlainVanillaEntity *pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
	if (!pvanilla)
	{
		throw LACoreInvalidData("Plain Vanilla object is not set in entitiy pool", __FILE__, __LINE__);
	}
	LADate asOf = pvanilla->getAsOfDate().get();
	LAPriceDataDayCount dc(ACT_365_ISDA);


	const LAStringVector &ircurs = pvanilla->getIRCurrencys().get();
	for (unsigned int i = 0; i < ircurs.size(); i++)
	{
		LAString ccy = ircurs[i];
		ccy.toLower();

		LAString yldproname =  LAMarketData::getBaseYieldProName(ccy);
		LAMathYieldCurvePro &bYieldPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(yldproname, ENCHKTYPE_ISDEFINED).get());

		const LADataMultiReference &refMarketDatas = bYieldPro.getMarketData();

		const unsigned int dataSize =refMarketDatas.getSize();
		if (dataSize == 0)
		{
			return;
		}
		DoubleVector termvec,ratevec;
		for (unsigned int j = 0; j < dataSize; ++j)
		{
			LAObject *data = &refMarketDatas.get(j).get();
			LAString type = dynamic_cast<const LADataString&> ((data->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			type.toUpper();
			if (type !=  YIELD_TYPE_ZERO)
				continue;

			double rate  = dynamic_cast<LADataDouble &>(data->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();
			const LAString termStr = dynamic_cast<const LADataString &>(data->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();
			const LADate& spotdate = dynamic_cast<const LADataDate&> ((data->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
			const LAPriceDataCalendar& cal  = dynamic_cast<const LAPriceDataCalendar&> ((data->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const LAPriceDataSlidingRule& sld  = dynamic_cast<const LAPriceDataSlidingRule&> ((data->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			const LAString freq = dynamic_cast<const LADataString &>(data->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
			const LADataBool& eom  = dynamic_cast<const LADataBool&> ((data->getData(IR_CALIBRATION_DATA_ISEOMROLL, ISNOTNULL)).get());
			LAString roll_conv("");
			if (freq == "LUNAR") roll_conv = "LUNAR";
			else if (eom) roll_conv = "EOM";
			else roll_conv = "NORMAL";

			LADate enddate = LAMathDateCalculations::getDate(spotdate, termStr, sld, &cal, true, &roll_conv);
		
			double term = dc.getTerm(spotdate,enddate,false);

			termvec.push_back(term);
			ratevec.push_back(rate);
		}
		if (termvec.size() < 1)
		{
			bool isFwdFX = false;
			const LADataHolder& dh = bYieldPro.getYieldData().get().getData(IR_CALIBRATION_DATA_ISFWDFX);
			if (dh.isDefined() && !dh.isNull()) 
				isFwdFX = dynamic_cast<const LADataBool&>(dh.get()).get();
			if (!isFwdFX)
				throw LACoreInvalidData("Libor Rate Map Error",__FILE__,__LINE__);
		}
		
		LALinearInterpolation* pInter = new LALinearInterpolation();
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
//LAObjectConfigurationVanilla::setUpStubCoefficient(LAObjectPool &objPool,  LAMathObjectValue &trade) const
//{
//	
//	const LADataValuation& valuemehod = dynamic_cast<const LADataValuation &>
//			(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
//	if (!valuemehod.isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) 
//		&& !valuemehod.isTypeOf(FN_IR_SWAPTIONVALUEFROMCASHFLOW))
//		return;
//	
//	LADataHolder* dh;
//
//	//check stub or not
//	bool isoddtrade = false;
//	dh = &(trade.getData(PRICING_DATA_ISODDTRADE, NOCHECK));
//	if (dh->isDefined() && !dh->isNull())
//		isoddtrade =  dynamic_cast<const LADataBool &>(dh->get()).get();
//	if (!isoddtrade)
//		return;
//	
//	//dh = &(trade.getData(PRICING_DATA_ODDINDEXTYPE, NOCHECK));
//	//if (!dh->isDefined() || dh->isNull())
//	//	return;
//
//	LAPriceDataDayCount dc(ACT_365_ISDA);
//	LAMathPlainVanillaEntity *pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
//	if (!pvanilla)
//	{
//		throw LACoreInvalidData("Plain Vanilla object is not set in entitiy pool", __FILE__, __LINE__);
//	}
//	LADate asOf = pvanilla->getAsOfDate().get();
//
//	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
//	LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(dh->get());
//	unsigned int legSize = legs.getSize();
//
//	for (unsigned int i = 0; i < legSize; i++)
//	{
//		LAObject &eleg = legs.get(i).get();
//		//get cashlet
//		dh = &(eleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
//		LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(dh->get());
//		unsigned int cashletSize = cashlets.getSize();
//		for (unsigned int j = 0; j < cashletSize; j++)
//		{
//			LAObject &ecashlet = cashlets.get(j).get();
//			//get firstcoupon
//			dh = &(ecashlet.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
//			if (!dh->isDefined() || dh->isNull())
//				continue;
//
//			LADataMultiReference &coupons = dynamic_cast<LADataMultiReference &>(dh->get());
//			LAObject &firstcoupon = coupons.get(0).get();
//
//			//check whether firstcoupon is stubperiod or not
//			
//			//check fn_linear or not
//			dh = &(firstcoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
//			LAPriceDataFunction &method = dynamic_cast<LAPriceDataFunction &>(dh->get());
//			if (!method.isTypeOf(FN_LINEAR))
//				continue;
//		
//			//check coefficient is -9999 or not
//			dh = &(firstcoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
//			LADataMultiReference &indexs = dynamic_cast<LADataMultiReference &>(dh->get());
//			unsigned int indexSize = indexs.getSize();
//
//			dh = &(firstcoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
//			LADataDoubles &coeffs = dynamic_cast<LADataDoubles &>(dh->get());
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
//				LAString stubtype =  dynamic_cast<LADataString &>(dh->get()).get();
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
//					datevec[0] = dynamic_cast<LADataDate &>(dh->get()).get();
//					dh = &(ecashlet.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL));
//					datevec[1] = dynamic_cast<LADataDate &>(dh->get()).get();
//
//
//					LAObject &eindex = indexs.get(0).get();
//					dh = &(eindex.getData(PRICING_DATA_DATESFORINDEXGENERATE, NOCHECK));
//					
//					if (dh->isDefined() && !dh->isNull())
//					{
//						dynamic_cast<LADataDates &>(dh->get()).set(datevec);
//					}
//					else
//					{
//						eindex.add(PRICING_DATA_DATESFORINDEXGENERATE, new LADataDates(datevec));
//					}
//
//					coefvec[0] = 1.0;
//					coeffs.set(coefvec);
//					continue;
//				}
//				
//				//get currency
//				dh = &(firstcoupon.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL));
//				LAString ccy = dynamic_cast<LADataString &>(dh->get()).get();
//				//exchange stubcoupon
//				dh = &(ecashlet.getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL));
//				const LADate &startdate = dynamic_cast<LADataDate &>(dh->get()).get();
//				dh = &(ecashlet.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL));
//				const LADate &enddate = dynamic_cast<LADataDate &>(dh->get()).get();
//				double term = dc.getTerm(startdate,enddate,false);
//				if (stubtype == SPOTRATERATIOMETHOD)
//				{
//					std::map<LAString, LAInterpolationBase*>::iterator it = mLiborRateMap.find(ccy);
//					if (it == mLiborRateMap.end())
//						throw LACoreInvalidData("Odd Coefficient Error",__FILE__,__LINE__);
//
//					LALinearInterpolation* inter = dynamic_cast<LALinearInterpolation* >(it->second);
//					double targetrate = inter->value(term);
//
//					//change ratio
//					map<double, unsigned int> mapRatePos;
//					for (unsigned l = 0 ; l < indexSize; l++)
//					{
//						LAObject &eindex = indexs.get(l).get();
//						dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
//						LAString termStr = dynamic_cast<LADataString &>(dh->get()).get();
//
//						dh = &(eindex.getData(PRICING_DATA_SPOTLAG, ISNOTNULL));
//						int spotlag = dynamic_cast<LADataInt &>(dh->get()).get();
//
//						dh = &(eindex.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
//						LAPriceDataSlidingRule &sld = dynamic_cast<LAPriceDataSlidingRule &>(dh->get());
//
//						dh = &(eindex.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
//						LAPriceDataCalendar &cal = dynamic_cast<LAPriceDataCalendar &>(dh->get());
//						
//						LADate spotdate = cal.getBusinessDay(asOf,spotlag);
//						LADate enddate = LAMathDateCalculations::getDate(spotdate, termStr, true);
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
//					std::map<LAString, DoubleVector>::iterator itvec = mLiborGridTermMap.find(ccy);
//					if (itvec == mLiborGridTermMap.end())
//						throw LACoreInvalidData("Odd Coefficient Error",__FILE__,__LINE__);
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
//						throw LACoreInvalidData("Stub Type Error",__FILE__,__LINE__);
//					
//					//change ratio
//					map<double, unsigned int> mapTermPos;
//					for (unsigned l = 0 ; l < indexSize; l++)
//					{
//						LAObject &eindex = indexs.get(l).get();
//						dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
//						LAString termStr = dynamic_cast<LADataString &>(dh->get()).get();
//
//						dh = &(eindex.getData(PRICING_DATA_SPOTLAG, ISNOTNULL));
//						int spotlag = dynamic_cast<LADataInt &>(dh->get()).get();
//
//						dh = &(eindex.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
//						LAPriceDataSlidingRule &sld = dynamic_cast<LAPriceDataSlidingRule &>(dh->get());
//
//						dh = &(eindex.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
//						LAPriceDataCalendar &cal = dynamic_cast<LAPriceDataCalendar &>(dh->get());
//						
//						LADate spotdate = cal.getBusinessDay(asOf,spotlag);
//						LADate enddate = LAMathDateCalculations::getDate(spotdate, termStr, true);
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
LAObjectConfigurationVanilla::setUpRenotionalFXInfo(LAObjectPool &objPool, LAMathObjectValue &trade) const
{
	const LADataValuation& attrval = dynamic_cast<const LADataValuation& >(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
	const LACoreValuation* pVal = &attrval.getMethod();
	if (pVal->isTypeOf(FN_IR_CONVERGENCEVALUE))
	{
		const LADataValuation& attrval = dynamic_cast<const LADataValuation& >(trade.getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
		pVal = &attrval.getMethod();
	}

	if (!pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE))
	{
		return;
	}

	//get ir properties
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	//get leg object
	LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	unsigned int legSize = legs.getSize();
	for (unsigned int i = 0; i < legSize; i++)
	{
		LAObject &leg = legs.get(i).get();
		//get currency
		LAString ccy = dynamic_cast<LADataString &>(leg.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
		ccy.toLower();
		//get renotional FX information
		bool isRenAdj = false;
		LAString tmpRenAdj_str = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST).toUpper();
		if (tmpRenAdj_str != MLIB_NO_DATA)
		{
			LADataBool tmpAttrB;
			tmpAttrB.convertFromString(tmpRenAdj_str);
			isRenAdj = tmpAttrB.get();
		}

		leg.remove(IR_CALIBRATION_DATA_ISRENOTIONALADJUST);
		leg.add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new LADataBool(isRenAdj));
		leg.remove(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET);
		leg.add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new LADataBool(isRenAdj));
	}
	
	return;
}

void
LAObjectConfigurationVanilla::setUpConvexityAdjustInfo(LAObjectPool &objPool, LAMathObjectValue &trade) const
{
	//const LADataValuation& attrval = dynamic_cast<const LADataValuation& >(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
	//const LACoreValuation* pVal = &attrval.getMethod();
	//if (pVal->isTypeOf(FN_IR_CONVERGENCEVALUE))
	//{
	//	const LADataValuation& attrval = dynamic_cast<const LADataValuation& >(trade.getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
	//	pVal = &attrval.getMethod();
	//}

	//if (!pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE))
	//{
	//	return;
	//}

	//get ir properties
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	// trade
	LADataHolder* dh;
	dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
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

									if (indexType == LIBOR)
									{
										// get value method
										const LADataValuation& attrval = dynamic_cast<const LADataValuation&>(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
										const LACoreValuation* pVal = &attrval.getMethod();

										bool isDelayedSwapConvexityAdjusted = false;
										if (pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE))
										{
											// convexity adjust model
											dh = &(indexInfos.get(l).getData(PRICING_DATA_CONVEXITYADJUSTMENT, NOCHECK));
											if (dh->isDefined() && !dh->isNull())
											{
												LAString caModel = dynamic_cast<LADataString&>(dh->get()).get();
												caModel.toUpper();
												indexInfos.get(l).add(PRICING_DATA_CAMODEL, new LADataString()).convertFromString(caModel);
												isDelayedSwapConvexityAdjusted = LAMathIndexEntity::isDelayedConvexityAdjustModel(caModel);
											}
										}
										if (isDelayedSwapConvexityAdjusted || pVal->isTypeOf(FN_IR_CAPFLOOROPTIONVALUE))
										{
											//get currency
											LAString ccy = dynamic_cast<LADataString &>(indexInfos.get(l).getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
											ccy.toLower();
											LAString tmp_prop;
											// threshold
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_THRESHOLD).toUpper();
											indexInfos.get(l).add(PRICING_DATA_CATHRESHOLD, new LADataDouble()).convertFromString(tmp_prop);
										}
									}
									else if (indexType == CMS)
									{
										//CFCalcStartEndDate
										std::vector<LADate> cfcalc_startDate;
										std::vector<LADate> cfcalc_endDate;
										for (unsigned int m = 0; m < cashlets.getSize(); ++m)
										{
											dh = &(cashlets.get(m).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
											if (dh->isDefined() && !dh->isNull())
											{
												// cashlets
												cfcalc_startDate.push_back(dynamic_cast<const LADataDate &>(cashlets.get(m).get().getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get());
												cfcalc_endDate.push_back(dynamic_cast<const LADataDate &>(cashlets.get(m).get().getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL).get()).get());
											}
										}
										// get value method
										const LADataValuation& attrval = dynamic_cast<const LADataValuation& >(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
										const LACoreValuation* pVal = &attrval.getMethod();
										if (pVal->isTypeOf(FN_IR_CONVERGENCEVALUE))
										{
											const LADataValuation& attrval = dynamic_cast<const LADataValuation& >(trade.getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
											pVal = &attrval.getMethod();
										}
										// convexity adjust is 0 in case of capfloor
										if (pVal->isTypeOf(FN_IR_CAPFLOOROPTIONVALUE))
										{
											indexInfos.get(l).add(PRICING_DATA_CAMODEL, new LADataString()).convertFromString(CMS_CA_ZERO);
										}
										else
										{
											//get currency
											LAString ccy = dynamic_cast<LADataString &>(indexInfos.get(l).getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
											ccy.toLower();
											LAString tmp_prop;
											// model
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_MODEL).toUpper();
											indexInfos.get(l).add(PRICING_DATA_CAMODEL, new LADataString()).convertFromString(tmp_prop);
											// threshold
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_THRESHOLD).toUpper();
											indexInfos.get(l).add(PRICING_DATA_CATHRESHOLD, new LADataDouble()).convertFromString(tmp_prop);
											// integral condition
											DoubleArray intCondi;
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_INTEGRALUPPERBOUND).toUpper();
											intCondi.push_back(tmp_prop.getDoubleValue());
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_INTEGRALLOWERBOUND).toUpper();
											intCondi.push_back(tmp_prop.getDoubleValue());
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_INTEGRALSTEPS).toUpper();
											intCondi.push_back(tmp_prop.getDoubleValue());
											indexInfos.get(l).add(PRICING_DATA_CAINTEGRALCONDITION, new LADataDoubles(intCondi));
											// CFStartdate CFEnddate
											indexInfos.get(l).add(PRICING_DATA_CFCALCSTARTDATES, new LADataDates(cfcalc_startDate));
											indexInfos.get(l).add(PRICING_DATA_CFCALCENDDATES, new LADataDates(cfcalc_endDate));
											// swaption premium integral step
											tmp_prop = staticData.getStaticData(ccy + STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_PREMIUMINTEGRALSTEPS).toUpper(); 
											if (tmp_prop != MLIB_NO_DATA)
											{
												int integralStep = tmp_prop.getIntValue();
												indexInfos.get(l).add(PRICING_DATA_CAPREMIUMINTEGRALSTEP, new LADataInt(integralStep));
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
LAObjectConfigurationVanilla::setUpSpotRateRatioMethod(LAObjectPool &objPool, LAMathObjectValue &trade) const
{
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();

	LADataHolder* dh;
	dh = &trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK);
	if (!dh->isDefined() || dh->isNull())
		return;

	LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(dh->get());
	for (unsigned int i = 0; i < legs.getSize(); ++i)
	{
		dh = &legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK);
		if(!dh->isDefined() || dh->isNull())
			continue;

		const LADataMultiReference &cashlets = dynamic_cast<const LADataMultiReference &>(dh->get());
		for (unsigned int j = 0; j < cashlets.getSize(); ++j)
		{
			dh = &(cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if(!dh->isDefined() || dh->isNull())
				continue;

			LADataMultiReference &couponinfos = dynamic_cast<LADataMultiReference &>(dh->get());	
			for (unsigned int k = 0; k < couponinfos.getSize(); ++k)
			{
				dh = &(couponinfos.get(k).getData(PRICING_DATA_ODDINDEXTYPE, NOCHECK));
				if(!dh->isDefined() || dh->isNull())
					continue;

				LAString currency = dynamic_cast<const LADataString &>(couponinfos.get(k).getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
				LAString tolerance = staticData.getStaticData(currency.toLower() + STATIC_DATA_KEY_YIELD_SPOTRATERATIOMETHOD_TOLERANCE);
				if(tolerance != MLIB_NO_DATA)
					couponinfos.get(k).add(PRICING_DATA_TOLERANCEFORSPOTRATERATIOMETHOD, new LADataDouble()).convertFromString(tolerance);
			}
		}
	}

	return; 
}

bool LAObjectConfigurationVanilla::setUpMarkovFunctionalParameter(LAObjectPool &objPool, LAMathObjectValue &trade) const
{
	const LADataValuation& valuemehod = dynamic_cast<const LADataValuation &>
		(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

	//if (!valuemehod.isTypeOf(FN_IR_NDSSWAPTIONVALUE))
	//	return false;

	LADataHolder* dh;

	//excel calculation mode
	LAString cur;
	dh = &(trade.getData(PRICING_DATA_CURRENCY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		cur = dynamic_cast<LADataString &>(dh->get()).get();
	}
	else
	{
		dh = &(trade.getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			cur = dynamic_cast<LADataString &>(dh->get()).get();
		}
		else
		{
			dh = &(trade.getData(PRICING_DATA_CURRENCY, ISNOTNULL)); // throw
		}
	}
	LAString markovMeanReversion = mpStaticData->getStaticData(cur.toLower() + STATIC_DATA_KEY_DEAL_MARKOVFUNCTIONAL_MEANREVERSION).toUpper();

	//trade.remove(PRICING_DATA_MARKOVFUNCTIONAL_MEANREVERSION);
	//trade.LAObject::add(PRICING_DATA_MARKOVFUNCTIONAL_MEANREVERSION, new LADataDouble()).convertFromString(markovMeanReversion);

	return true;
}
