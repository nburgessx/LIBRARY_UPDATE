/*! @file
    @brief  M-Library Excel Addin Wrap class
*/
//  2007, AlgoQuantHub.

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <AQLDataBasics.h>
#include <AQLDataVector.h>
#include <AQLDataMatrix.h>
#include <AQLDataReference.h>
#include <AQLMathYieldCurve.h>
#include <AQLMathYieldCurvePro.h>
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLCurveSetup.h"
#include "AQLCalibrateModelIRVanilla.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLStaticData.h"
#include "AQLCoreDataService.h"
#include "AQLMarketData.h"
#include "AQLStaticDataManager.h"
#include <AQLFunctionUtilities.h>
#include "AQLMathDateUtilities.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathCorrelation.h"
#include "AQLCompoundingFunc.h"
#include "AQLCurveProperties.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"
#include "AQLPriceDataFunction.h"
#include "AQL1DIntegral.h"
#include "AQLCombinationFunc.h"
#include "AQLFunction.h"
#include "AQLShiftFunc.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceCashFlowGenerator.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLPriceTradeValue.h"

//#include "AQLDefinitionsHazardRate.h"
//#include "AQLHazardRateCreator.h"
//#include "AQLMathCreditEntitySetUp.h"
//#include "AQLMathSingleCredit.h"
#include "AQL1DDataSet.h"
//#include "AQLPriceAccruedIntFunc.h"
//#include "AQLPriceCDS.h"

#include <memory>
#include <sstream>

#ifndef MARKETTYPE_SWAP
#define MARKETTYPE_SWAP "SWAP" 
#endif
#ifndef MARKETTYPE_BASIS
#define MARKETTYPE_BASIS "BASISSWAP" 
#endif
#ifndef MARKETTYPE_ZERORATE
#define MARKETTYPE_ZERORATE "ZERORATE" 
#endif
#ifndef CURVETYPE_FLOATER
#define CURVETYPE_FLOATER "FLOATER" 
#endif
#ifndef CURVETYPE_ARBFREE
#define CURVETYPE_ARBFREE "ARBFREE" 
#endif

#ifndef CURVENAME_3ML 
#define CURVENAME_3ML "3ML"
#endif
#ifndef CURVENAME_6ML
#define CURVENAME_6ML "6ML"
#endif
#ifndef CURVENAME_DF
#define CURVENAME_DF "DF"
#endif

const AQLString LF = "\n";

void
AQLCurveSetup::
setUpBasisCurve
(AQLDataInstance* dataInstance,
 const AQLString& curveID, 
 const AQLString& marketName, 
 const AQLStringMatrix& basisRates, 
 const AQLStringMatrix& basisConv,
 const AQLStringMatrix& fwdFXs, 
 const AQLStringMatrix& fwdConv,
 const AQLStringMatrix& spotFXs, 
 const AQLStringMatrix& generateProp,
 const AQLStringMatrix& moneyConv,
 const AQLString& curveNames)
{
	AQLStaticData &irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLObjectPool &objPool = dataInstance->getObjectPool();

	AQLStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	AQLDate asofdate = AQLMathDateUtilities::getAQLDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
    AQLCoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	AQLString currency;
	AQLObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		AQLObject& yldEntity = objHolder.get();
		const AQLDataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const AQLDataString&> (dh->get())).get();
		}
		else
		{
			if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = AQLString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = AQLString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	AQLString tmpCurrency = currency; tmpCurrency.toLower();
	
	//set market rate	
	if (marketName == "" || marketName == STD) 
		throw AQLCoreInvalidData("Do not use STD or blank for basis curve name!",__FILE__,__LINE__);

	AQLString staticDataSuffix;
	AQLString suffix_data;
	staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	suffix_data = "_" + marketName;
	suffix_data.toLower();

	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);

	AQLString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	AQLStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (useMarkets == AQ_NO_DATA) useMarkets = "";
	AQLString tmpCurveName = marketName; tmpCurveName.toUpper();
	if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
	{
		if (useMarkets == "") useMarkets = marketName;
		else useMarkets += AQLString(MULTI_STATIC_DATA_DELIMITER) + marketName;
	}
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);

	if (curveNames == "")
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, marketName);
	}
	else
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, curveNames);
	}
	
	//basis file
	AQLString basisfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + staticDataSuffix);
	if (basisfile == AQ_NO_DATA)
	{
		basisfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_basisswap") + suffix_data + AQLString(".csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + staticDataSuffix, basisfile);
	}
	AQLString basisstream;
	AQLString usegrid = "";
	for (unsigned int i = 0; i < basisRates.size();i++)
	{
		basisstream += basisRates[i][0];
		const double brate = basisRates[i][1].getDoubleValue() * 10000.0;
		basisstream += "," + AQLString(brate) + LF;

		if (basisRates[i].size() == 3)
		{
			AQLString useGridFrag = basisRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid += basisRates[i][0] + ":";
			else usegrid += "NONE:";
		}
	}
	std::istringstream *pbasisstream = new std::istringstream(basisstream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(basisfile), pbasisstream);

	//fwdfx file
	AQLString fwdfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FILE + staticDataSuffix);
	AQLString fwdstream;
	AQLString fwdusegrid = "";
	for (unsigned int i = 0; i < fwdFXs.size();i++)
	{
		fwdstream += fwdFXs[i][0];
		const double fwdfx = fwdFXs[i][1].getDoubleValue();
		fwdstream += "," + AQLString(fwdfx) + LF;

		if (fwdFXs[i].size() == 3)
		{
			AQLString useGridFrag = fwdFXs[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") fwdusegrid += fwdFXs[i][0] + ":";
			else fwdusegrid += "NONE:";
		}
	}
	std::istringstream *pfwdstream = new std::istringstream(fwdstream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(fwdfile), pfwdstream);

	//basis info
	for(size_t i=0; i<basisConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.basis." + basisConv[i][0] + staticDataSuffix;
		key.toLower();
		AQLString data = basisConv[i][1];
		if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		irStaticData.setStaticData(key,data);
	}
	//generator info
	for(size_t i=0; i<generateProp.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		AQLString data = generateProp[i][1];
		data.toLower();
		irStaticData.setStaticData(key,data);
	}
	//money info
	for(size_t i=0; i<moneyConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv[i][0];
		key.toLower();
		AQLString data = moneyConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key,data);
	}
	//fwdfx info
	for(size_t i=0; i<fwdConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.basis.fwdfx." + fwdConv[i][0] + staticDataSuffix;
		key.toLower();
		AQLString data = fwdConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key,data);
	}

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + staticDataSuffix);
	int find;
	if (usegrid != "") 
	{
		usegrid = usegrid.subString(0, usegrid.size() - 2);
		while ((find = usegrid.findString("NONE:")) != -1) usegrid.remove(find, 5);
		if ((find = usegrid.findString(":NONE")) != -1) usegrid.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + staticDataSuffix, usegrid);
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID + staticDataSuffix);
	if (fwdusegrid != "") 
	{
		fwdusegrid = fwdusegrid.subString(0, fwdusegrid.size() - 2);
		while ((find = fwdusegrid.findString("NONE:")) != -1) fwdusegrid.remove(find, 5);
		if ((find = fwdusegrid.findString(":NONE")) != -1) fwdusegrid.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID + staticDataSuffix, fwdusegrid);
	}

	//set fxentity
	AQLMathFXEntity* pFwd = NULL;
	AQLObjectHolder ehfx = objPool.getObject(FORWARDFX, ENCHKTYPE_NOCHECK);
	if (!ehfx.isDefined())
	{
		pFwd = new AQLMathFXEntity(dataInstance);
		objPool.set(FORWARDFX,pFwd);
	}
	else
	{
		dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FORWARDFX).get()).reset();
		pFwd = &dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FORWARDFX).get());
	}
	pFwd->getName().convertFromString(FORWARDFX);
	pFwd->getFXType().convertFromString("FORWARDRATE");
	AQLCoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD, FORWARDFX);
	const int ccySize = spotFXs.size();
	AQLStringVector ccys(ccySize);
	DoubleVector spotrates(ccySize);
	AQLString unitccy = irStaticData.getStaticData(KEY_FXSPOTRATES_UNITCCY);
	unitccy.toUpper();
	for(unsigned int i = 0; i < ccySize; ++i)
	{
        if (spotFXs[i].size() < 3)
		{
			throw AQLCoreInvalidData("spotrate file size error", __FILE__, __LINE__);
		}
		AQLString tmpccy = spotFXs[i][0];
		if (tmpccy.toUpper() != unitccy)
		{
			throw AQLCoreInvalidData("The first column of a spot rate file must be unit currency", __FILE__, __LINE__);
		}
		ccys[i] = spotFXs[i][1];
		ccys[i].toUpper();
		spotrates[i] = spotFXs[i][2].getDoubleValue();
	}
    pFwd->getCurrencys().set(ccys);
	pFwd->getSpotRates().set(spotrates);

	//set generate df
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + staticDataSuffix, MARKETTYPE_BASIS);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	AQLCalibrateModel *generator = new AQLCalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);
	delete generator;

	//ylddata
	AQLString	CurveIDTool			= curveID + TOOL;
	AQLString	CurveIDManager		= curveID + marketName + MANAGER;
	//curve entities manager (curve, grids, curveinformation entities)
	AQLObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new AQLObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new AQLDataString()			).convertFromString(AQLString(AQLTime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new AQLDataString()			).convertFromString(CurveIDManager);
}

void
AQLCurveSetup::
setUpFwdFXConstantCurve
(AQLDataInstance* dataInstance,
 const AQLString& curveID, 
 const AQLString& marketName, 
 const AQLStringMatrix& fwdfxconstConv,
 const AQLStringMatrix& generateProp,
 const AQLString& curveNames)
{
	AQLStaticData &irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLObjectPool &objPool = dataInstance->getObjectPool();

	AQLStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	AQLDate asofdate = AQLMathDateUtilities::getAQLDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
    AQLCoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	AQLString currency;
	AQLObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		AQLObject& yldEntity = objHolder.get();
		const AQLDataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const AQLDataString&> (dh->get())).get();
		}
		else
		{
			if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = AQLString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = AQLString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	AQLString tmpCurrency = currency; tmpCurrency.toLower();
	
	//set market rate	
	if (marketName == "" || marketName == STD) 
		throw AQLCoreInvalidData("Do not use STD or blank for basis curve name!",__FILE__,__LINE__);

	AQLString staticDataSuffix;
	AQLString suffix_data;
	staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	suffix_data = "_" + marketName;
	suffix_data.toLower();

	AQLString tmpCurveName = marketName; tmpCurveName.toUpper();
	AQLString isFwdFxCon = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST, "TRUE");
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FWDFXCONST_USEMARKET, marketName);

	if (curveNames == "")
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, marketName);
	}
	else
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, curveNames);
	}
	
	//basis info
	for(size_t i=0; i<fwdfxconstConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.basis." + fwdfxconstConv[i][0] + staticDataSuffix;
		key.toLower();
		AQLString data = fwdfxconstConv[i][1];
		if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		irStaticData.setStaticData(key,data);
	}
	for(size_t i=0; i<generateProp.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		AQLString data = generateProp[i][1];
		data.toLower();
		irStaticData.setStaticData(key,data);
	}

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	//set generate df
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + staticDataSuffix, MARKETTYPE_BASIS);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	AQLCalibrateModelIR *generator = new AQLCalibrateModelIRVanilla(currency);
	generator->loadFwdFXConstCurveDataAndCalibrate(currency, *dataInstance);
	delete generator;

	//ylddata
	AQLString	CurveIDTool			= curveID + TOOL;
	AQLString	CurveIDManager		= curveID + marketName + MANAGER;
	//curve entities manager (curve, grids, curveinformation entities)
	AQLObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new AQLObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new AQLDataString()			).convertFromString(AQLString(AQLTime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new AQLDataString()			).convertFromString(CurveIDManager);

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST, isFwdFxCon);
}

void 
AQLCurveSetup::
setUpSwapCurve
(AQLDataInstance* dataInstance,
 const AQLString& curveID,
 const AQLString& marketName,
 const AQLStringMatrix& generateProp, 
 const AQLStringMatrix& moneyConv,
 const AQLStringMatrix& liborRates, 
 const AQLStringMatrix& liborConv,
 const AQLStringMatrix& swapRates, 
 const AQLStringMatrix& swapConv,
 const AQLStringMatrix& fra3mRates,
 const AQLStringMatrix& fra6mRates,
 const AQLStringMatrix& fraConv,
 const AQLStringMatrix& futureRates, 
 const AQLStringMatrix& futureConv,
 const AQLStringMatrix& adjustSwapConv,
 const AQLStringMatrix& adjustSwapRates,
 const AQLString& curveNames,
 const AQLString& curveName_DF2)
{
	AQLStaticData &irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLObjectPool &objPool = dataInstance->getObjectPool();
	AQLObject* pyld = NULL;

	AQLStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	AQLDate asofdate	= AQLMathDateUtilities::getAQLDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	AQLCoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	AQLString currency;
	AQLObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		AQLObject& yldEntity = objHolder.get();
		const AQLDataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const AQLDataString&> (dh->get())).get();
		}
		else
		{
			if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = AQLString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = AQLString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	AQLString tmpCurrency = currency; tmpCurrency.toLower();

	AQLString generateCurveName = (marketName == "") ? STD : marketName;
	AQLString staticDataSuffix;
	AQLString suffix_data;
	AQLString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	AQLStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (useMarkets == AQ_NO_DATA) useMarkets = "";
	if (generateCurveName != STD)
	{
		staticDataSuffix = "." + generateCurveName;
		staticDataSuffix.toLower();
		AQLString tmpCurveName = generateCurveName; tmpCurveName.toUpper();
		if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
		{
			if (useMarkets == "") useMarkets = generateCurveName;
			else useMarkets += AQLString(MULTI_STATIC_DATA_DELIMITER) + generateCurveName;
		}	
	}
	else
	{	
		if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),SWAP)) 
		{
			if (useMarkets == "") useMarkets = SWAP;
			else useMarkets += AQLString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
		}
	}
	
	if (curveNames == "")
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix, marketName);
	}
	else
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix, curveNames);
	}

	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (curveName_DF2 != "" && marketName == STD)
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2, curveName_DF2);
	}

	if (generateCurveName != STD)
	{
		suffix_data = "_" + generateCurveName;
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT + staticDataSuffix);

	bool isswaptenoradjust = false;
	for(size_t i=0; i<generateProp.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		AQLString data = generateProp[i][1];
		if (key.findString("dfcurvename") == -1) data.toLower();
		irStaticData.setStaticData(key, data);

		AQLString tmpProp = generateProp[i][0];
		tmpProp.toLower();
		if (tmpProp == "isswaptenoradjust")
		{
			key = generateProp[i][1];
			isswaptenoradjust = (key.toUpper() == "TRUE");
		}
	}
	
	for(size_t i=0; i<moneyConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv[i][0] + staticDataSuffix;
		key.toLower(); 
		AQLString data = moneyConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}
	
	for(size_t i=0; i<liborConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.libor." + liborConv[i][0] + staticDataSuffix;
		key.toLower(); 
		AQLString data = liborConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<swapConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0] + staticDataSuffix;
		key.toLower(); 
		AQLString data = swapConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<fraConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.fra." + fraConv[i][0] + staticDataSuffix;
		key.toLower(); 
		AQLString data = fraConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<futureConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.future." + futureConv[i][0] + staticDataSuffix;
		key.toLower(); 
		AQLString data = futureConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	//////////////////modify for aud curve///////////////////////////////////////////////

	//swap adjust mode
	AQLString tmpBasisCurveName;
	if (isswaptenoradjust)
	{
		tmpBasisCurveName = THREESIXBASIS;
		AQLString adjsuffix_prop;
		AQLString adjsuffix_data;
		if (tmpBasisCurveName != STD)
		{
			adjsuffix_prop = "." + tmpBasisCurveName;
			adjsuffix_prop.toLower();
			adjsuffix_data = "_" + tmpBasisCurveName;
			adjsuffix_data.toLower();
		}
		
		for(size_t i=0; i<adjustSwapConv.size(); i++)
		{
			AQLString key = tmpCurrency + ".sde.yield.basis." + adjustSwapConv[i][0] + adjsuffix_prop;
			key.toLower(); 
			AQLString data = adjustSwapConv[i][1];
			if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
			irStaticData.setStaticData(key, data);
		}
	
		tmpBasisCurveName.toLower();
		AQLString basisEntityName = tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName;

		//swapfile
		AQLString adjfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + adjsuffix_prop);
		if (adjfile == AQ_NO_DATA)
		{
			adjfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_basisswap") + adjsuffix_data + AQLString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + adjsuffix_prop, adjfile);
		}
		AQLString adjstream;
		AQLString usegrid = "";
		for(size_t i=0; i<adjustSwapRates.size(); i++)
		{
			adjstream += adjustSwapRates[i][0];
			double adjrate = adjustSwapRates[i][1].getDoubleValue() * 10000.0;
			adjstream += "," + AQLString(adjrate) + LF;

			if (adjustSwapRates[i].size() == 3)
			{
				AQLString useGridFrag = adjustSwapRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid += adjustSwapRates[i][0] + ":";
				else usegrid += "NONE:";
			}
		}
		std::istringstream *padjstream = new std::istringstream(adjstream.getCString());
		AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(adjfile), padjstream);

		irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName);
		int find;
		if (usegrid != "") 
		{
			usegrid = usegrid.subString(0, usegrid.size() - 2);
			while ((find = usegrid.findString("NONE:")) != -1) usegrid.remove(find, 5);
			if ((find = usegrid.findString(":NONE")) != -1) usegrid.remove(find, 5);
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName, usegrid);
		}

		AQLString tmpCurveName = tmpBasisCurveName; tmpCurveName.toUpper();
		tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
		{ 
			useMarkets += AQLString(MULTI_STATIC_DATA_DELIMITER) + tmpBasisCurveName;
		}
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + tmpBasisCurveName, MARKETTYPE_BASIS);

		//////////////////modify for aud curve///////////////////////////////////////////////
	}

	AQLString usegrid_libor = "",usegrid_swap = "",usegrid_fra3m = "",usegrid_fra6m = "",usegrid_future = "";

	//set Libor Object;
	AQLString liborEntityName = tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix;

	AQLString liborfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix);
	if (liborfile == AQ_NO_DATA)
	{
		liborfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_libor") + suffix_data + AQLString(".csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix, liborfile);
	}
	AQLString liborstream;
	for(size_t i=0; i<liborRates.size(); i++)
	{
		liborstream += liborRates[i][0];
		double lrate = liborRates[i][1].getDoubleValue() * 100.0;
		liborstream += "," + AQLString(lrate) + LF;

		if (liborRates[i].size() == 3)
		{
			AQLString useGridFrag = liborRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_libor += liborRates[i][0] + ":";
			else usegrid_libor += "NONE:";
		}
	}
	std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(liborfile), pliborstream);

	//set Swap Object;
	AQLString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	if (swapfile == AQ_NO_DATA)
	{
		swapfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_swap") + suffix_data + AQLString(".csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix, swapfile);
	}
	AQLString swapstream;
	for(size_t i=0; i<swapRates.size(); i++)
	{
		swapstream += swapRates[i][0];
		double srate = swapRates[i][1].getDoubleValue() * 100.0;
		swapstream += "," + AQLString(srate) + LF;

		if (swapRates[i].size() == 3)
		{
			AQLString useGridFrag = swapRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_swap += swapRates[i][0] + ":";
			else usegrid_swap += "NONE:";
		}
	}
	std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(swapfile), pswapstream);

	//set fra3m Object;
	bool isFRAUse = false;
	AQLString isFRAUse_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	AQLDataBool tmpAttrB;
	if (isFRAUse_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFRAUse_str);
		isFRAUse = tmpAttrB.get();
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + staticDataSuffix);
	if (isFRAUse)
	{
		AQLString fra3mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + staticDataSuffix);
		if (fra3mfile == AQ_NO_DATA)
		{
			fra3mfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_3mfra") + suffix_data + AQLString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + staticDataSuffix, fra3mfile);
		}
		AQLString fra3mstream;
		for(size_t i=0; i<fra3mRates.size(); i++)
		{
			fra3mstream += fra3mRates[i][0];
			double frarate = fra3mRates[i][1].getDoubleValue() * 100.0;
			fra3mstream += "," + AQLString(frarate) + LF;

			if (fra3mRates[i].size() == 3)
			{
				AQLString useGridFrag = fra3mRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fra3m += fra3mRates[i][0] + ":";
				else usegrid_fra3m += "NONE:";
			}
		}
		std::istringstream *pfra3mstream = new std::istringstream(fra3mstream.getCString());
		AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(fra3mfile), pfra3mstream);		

		//set fra6m Object;
		AQLString fra6mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + staticDataSuffix);
		if (fra6mfile == AQ_NO_DATA)
		{
			fra6mfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_6mfra") + suffix_data + AQLString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + staticDataSuffix, fra6mfile);
		}
		AQLString fra6mstream;
		for(size_t i=0; i<fra6mRates.size(); i++)
		{
			fra6mstream += fra6mRates[i][0];
			double frarate = fra6mRates[i][1].getDoubleValue() * 100.0;
			fra6mstream += "," + AQLString(frarate) + LF;

			if (fra6mRates[i].size() == 3)
			{
				AQLString useGridFrag = fra6mRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fra6m += fra6mRates[i][0] + ":";
				else usegrid_fra6m += "NONE:";
			}
		}
		std::istringstream *pfra6mstream = new std::istringstream(fra6mstream.getCString());
		AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(fra6mfile), pfra6mstream);
	}

	//set future Object;
	bool isFutureUse = false;
	AQLString tmpFutureStr = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + staticDataSuffix);
	if (tmpFutureStr != AQ_NO_DATA)
	{
		AQLDataBool tmpAttrB;
		tmpAttrB.convertFromString(tmpFutureStr);
		isFutureUse = tmpAttrB.get();
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix);
	if (isFutureUse)
	{
		AQLString futureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix);
		if (futureFile == AQ_NO_DATA)
		{
			futureFile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_future") + suffix_data + AQLString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix, futureFile);
		}
		AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(futureFile), createFutureStream(futureRates, usegrid_future));
	}

	///////////////
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + staticDataSuffix);
	int find;
	if (usegrid_libor != "") 
	{
		usegrid_libor = usegrid_libor.subString(0, usegrid_libor.size() - 2);
		while ((find = usegrid_libor.findString("NONE:")) != -1) usegrid_libor.remove(find, 5);
		if ((find = usegrid_libor.findString(":NONE")) != -1) usegrid_libor.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix, usegrid_libor);
	}
	if (usegrid_swap != "") 
	{
		usegrid_swap = usegrid_swap.subString(0, usegrid_swap.size() - 2);
		while ((find = usegrid_swap.findString("NONE:")) != -1) usegrid_swap.remove(find, 5);
		if ((find = usegrid_swap.findString(":NONE")) != -1) usegrid_swap.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + staticDataSuffix, usegrid_swap);
	}
	if (usegrid_fra3m != "") 
	{
		usegrid_fra3m = usegrid_fra3m.subString(0, usegrid_fra3m.size() - 2);
		while ((find = usegrid_fra3m.findString("NONE:")) != -1) usegrid_fra3m.remove(find, 5);
		if ((find = usegrid_fra3m.findString(":NONE")) != -1) usegrid_fra3m.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID + staticDataSuffix, usegrid_fra3m);
	}
	if (usegrid_fra6m != "") 
	{
		usegrid_fra6m = usegrid_fra6m.subString(0, usegrid_fra6m.size() - 2);
		while ((find = usegrid_fra6m.findString("NONE:")) != -1) usegrid_fra6m.remove(find, 5);
		if ((find = usegrid_fra6m.findString(":NONE")) != -1) usegrid_fra6m.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID + staticDataSuffix, usegrid_fra6m);
	}
	if (usegrid_future != "") 
	{
		usegrid_future = usegrid_future.subString(0, usegrid_future.size() - 2);
		while ((find = usegrid_future.findString("NONE:")) != -1) usegrid_future.remove(find, 5);
		if ((find = usegrid_future.findString(":NONE")) != -1) usegrid_future.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + staticDataSuffix, usegrid_future);
	}

	AQLString tmpCurveName = generateCurveName; tmpCurveName.toUpper();
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
	AQLCalibrateModel *generator = new AQLCalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);

	//resist generatetime
	AQLString CurveIDTool = curveID + TOOL;
	AQLString CurveIDManager	= curveID + marketName + MANAGER;

	//curve entities manager (curve, grids, curveinformation entities)
	AQLObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new AQLObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new AQLDataString()			).convertFromString(AQLString(AQLTime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new AQLDataString()			).convertFromString(CurveIDManager);

	return;
};

void 
AQLCurveSetup::
SetUpArbFreeCurve
(AQLDataInstance* dataInstance,
 const AQLString& curveID,
 const AQLStringMatrix& generateProp, 
 const AQLStringMatrix& moneyConv,
 const AQLStringMatrix& liborRates, 
 const AQLStringMatrix& liborConv,
 const AQLStringMatrix& swapRates, 
 const AQLStringMatrix& swapConv,
 const AQLStringMatrix& fra3mRates,
 const AQLStringMatrix& fra6mRates,
 const AQLStringMatrix& fraConv,
 const AQLStringMatrix& xccyBasisRates, 
 const AQLStringMatrix& xccyBasisConv,
 const AQLStringMatrix& threeSixRates,
 const AQLStringMatrix& threeSixConv,
 const AQLStringMatrix& futureRates, 
 const AQLStringMatrix& futureConv,
 const AQLStringMatrix& adjustData,
 const AQLString& curveNames_3ML,
 const AQLString& curveNames_6ML,
 const AQLString& curveNames_DF,
 const AQLString& curveName_DF2)
{
	AQLStaticData &irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLObjectPool &objPool = dataInstance->getObjectPool();
	AQLObject* pyld = NULL;	
	
	AQLStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	AQLDate asofdate	= AQLMathDateUtilities::getAQLDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	AQLCoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	AQLString currency;
	AQLObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		AQLObject& yldEntity = objHolder.get();
		const AQLDataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const AQLDataString&> (dh->get())).get();
		}
		else
		{
			if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = AQLString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = AQLString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	AQLString tmpCurrency = currency; tmpCurrency.toLower();

	AQLString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	AQLStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (useMarkets == AQ_NO_DATA) useMarkets = "";
	if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),SWAP)) 
	{
		if (useMarkets == "") useMarkets = SWAP;
		else useMarkets += AQLString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT);

	for(size_t i=0; i<generateProp.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		AQLString data = generateProp[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}
	
	for(size_t i=0; i<moneyConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv[i][0];
		key.toLower(); 
		AQLString data = moneyConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}
	
	for(size_t i=0; i<liborConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.libor." + liborConv[i][0];
		key.toLower(); 
		AQLString data = liborConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<swapConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0];
		key.toLower(); 
		AQLString data = swapConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<fraConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.fra." + fraConv[i][0];
		key.toLower(); 
		AQLString data = fraConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<xccyBasisConv.size(); i++)
	{
		AQLString key;
		AQLString data = xccyBasisConv[i][1];
		if (tmpCurrency == "usd")
		{
			key = tmpCurrency + ".sde.yield.basis." + xccyBasisConv[i][0];
		}
		else
		{
			key = tmpCurrency + ".sde.yield.basis." + xccyBasisConv[i][0] + ".xccybasis";
			if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
			tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
			if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),XCCYBASIS)) 
			{
				useMarkets += AQLString(MULTI_STATIC_DATA_DELIMITER) + XCCYBASIS;
			}
		}
		key.toLower(); 
		irStaticData.setStaticData(key, data);
	}
	if (tmpCurrency != "usd") irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + ".xccybasis", MARKETTYPE_BASIS);

	for(size_t i=0; i<threeSixConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.basis." + threeSixConv[i][0] + ".3m6mbasis";
		key.toLower(); 
		AQLString data = threeSixConv[i][1];
		if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		irStaticData.setStaticData(key, data);
	}
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + ".3m6mbasis", MARKETTYPE_BASIS);

	for(size_t i=0; i<futureConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.future." + futureConv[i][0];
		key.toLower(); 
		AQLString data = futureConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	AQLString usegrid_libor = "",usegrid_swap = "",usegrid_fra3m = "",usegrid_fra6m = "",usegrid_xccy = "",
		usegrid_3m6m = "",usegrid_future = "";

	//set Libor Object;
	AQLString liborfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE);
	if (liborfile == AQ_NO_DATA)
	{
		liborfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_libor.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE, liborfile);
	}
	AQLString liborstream;
	for(size_t i=0; i<liborRates.size(); i++)
	{
		liborstream += liborRates[i][0];
		double lrate = liborRates[i][1].getDoubleValue() * 100.0;
		liborstream += "," + AQLString(lrate) + LF;

		if (liborRates[i].size() == 3)
		{
			AQLString useGridFrag = liborRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_libor += liborRates[i][0] + ":";
			else usegrid_libor += "NONE:";
		}
	}
	std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(liborfile), pliborstream);

	//set Swap Object;
	AQLString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE);
	if (swapfile == AQ_NO_DATA)
	{
		swapfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_swap.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE, swapfile);
	}
	AQLString swapstream;
	for(size_t i=0; i<swapRates.size(); i++)
	{
		swapstream += swapRates[i][0];
		double srate = swapRates[i][1].getDoubleValue() * 100.0;
		swapstream += "," + AQLString(srate) + LF;

		if (swapRates[i].size() == 3)
		{
			AQLString useGridFrag = swapRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_swap += swapRates[i][0] + ":";
			else usegrid_swap += "NONE:";
		}
	}
	std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(swapfile), pswapstream);

	//set fra3m Object;
	bool isFRAUse = false;
	AQLString isFRAUse_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE);
	AQLDataBool tmpAttrB;
	if (isFRAUse_str != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(isFRAUse_str);
		isFRAUse = tmpAttrB.get();
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE);
	if (isFRAUse)
	{
		AQLString fra3mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE);
		if (fra3mfile == AQ_NO_DATA)
		{
			fra3mfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_3mfra.csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE, fra3mfile);
		}
		AQLString fra3mstream;
		for(size_t i=0; i<fra3mRates.size(); i++)
		{
			fra3mstream += fra3mRates[i][0];
			double frarate = fra3mRates[i][1].getDoubleValue() * 100.0;
			fra3mstream += "," + AQLString(frarate) + LF;

			if (fra3mRates[i].size() == 3)
			{
				AQLString useGridFrag = fra3mRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fra3m += fra3mRates[i][0] + ":";
				else usegrid_fra3m += "NONE:";
			}
		}
		std::istringstream *pfra3mstream = new std::istringstream(fra3mstream.getCString());
		AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(fra3mfile), pfra3mstream);		

		//set fra6m Object;
		AQLString fra6mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE);
		if (fra6mfile == AQ_NO_DATA)
		{
			fra6mfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_6mfra.csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE, fra6mfile);
		}
		AQLString fra6mstream;
		for(size_t i=0; i<fra6mRates.size(); i++)
		{
			fra6mstream += fra6mRates[i][0];
			double frarate = fra6mRates[i][1].getDoubleValue() * 100.0;
			fra6mstream += "," + AQLString(frarate) + LF;

			if (fra6mRates[i].size() == 3)
			{
				AQLString useGridFrag = fra6mRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fra6m += fra6mRates[i][0] + ":";
				else usegrid_fra6m += "NONE:";
			}
		}
		std::istringstream *pfra6mstream = new std::istringstream(fra6mstream.getCString());
		AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(fra6mfile), pfra6mstream);
	}

	///Xccy Basis Rate
	AQLString xccyBasisFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "xccybasis");
	if (xccyBasisFile == AQ_NO_DATA)
	{
		xccyBasisFile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_basisswap_xccybasis.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "xccybasis", xccyBasisFile);
	}
	AQLString xccyBasisStream;
	for(size_t i=0; i<xccyBasisRates.size(); i++)
	{
		xccyBasisStream += xccyBasisRates[i][0];
		double brate = xccyBasisRates[i][1].getDoubleValue() * 10000.0;
		xccyBasisStream += "," + AQLString(brate) + LF;

		if (xccyBasisRates[i].size() == 3)
		{
			AQLString useGridFrag = xccyBasisRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_xccy += xccyBasisRates[i][0] + ":";
			else usegrid_xccy += "NONE:";
		}
	}
	std::istringstream *pXccyBasisStream = new std::istringstream(xccyBasisStream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(xccyBasisFile), pXccyBasisStream);

	///36 Basis Rate
	AQLString threeSixFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "3m6mbasis");
	if (threeSixFile == AQ_NO_DATA)
	{
		threeSixFile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_basisswap_3m6mbasis.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "3m6mbasis", threeSixFile);
	}
	AQLString threeSixStream;
	for(size_t i=0; i<threeSixRates.size(); i++)
	{
		threeSixStream += threeSixRates[i][0];
		double brate = threeSixRates[i][1].getDoubleValue() * 10000.0;
		threeSixStream += "," + AQLString(brate) + LF;

		if (threeSixRates[i].size() == 3)
		{
			AQLString useGridFrag = threeSixRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_3m6m += threeSixRates[i][0] + ":";
			else usegrid_3m6m += "NONE:";
		}
	}
	std::istringstream *pThreeSixStream = new std::istringstream(threeSixStream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(threeSixFile), pThreeSixStream);

	if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),THREESIXBASIS) && threeSixRates.size())
	{
		useMarkets += AQLString(MULTI_STATIC_DATA_DELIMITER) + THREESIXBASIS;
	}

	//set future Object;
	bool isFutureUse = false;
	AQLString tmpFutureStr = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE);
	if (tmpFutureStr != AQ_NO_DATA)
	{
		tmpAttrB.convertFromString(tmpFutureStr);
		isFutureUse = tmpAttrB.get();
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE);
	if (isFutureUse)
	{
		AQLString futureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE);
		if (futureFile == AQ_NO_DATA)
		{
			futureFile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_future") + AQLString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE, futureFile);
		}
		AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(futureFile), createFutureStream(futureRates, usegrid_future));
	}
	
	//set Adjust Data Object;
	AQLString adjustValueFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + "xccybasis");
	if (adjustValueFile == AQ_NO_DATA)
	{
		adjustValueFile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_basisadjust.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + "xccybasis", adjustValueFile);
	}
	AQLString adjustValueStream;
	for(size_t i=0; i<adjustData.size(); i++)
	{
		adjustValueStream += adjustData[i][0];
		adjustValueStream += "," + adjustData[i][1] + LF;	
	}
	std::istringstream *pAdjustValueStream = new std::istringstream(adjustValueStream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(adjustValueFile), pAdjustValueStream);


	///////////////
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + "3m6mbasis");
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + "xccybasis");
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID);
	int find;
	if (usegrid_libor != "") 
	{
		usegrid_libor = usegrid_libor.subString(0, usegrid_libor.size() - 2);
		while ((find = usegrid_libor.findString("NONE:")) != -1) usegrid_libor.remove(find, 5);
		if ((find = usegrid_libor.findString(":NONE")) != -1) usegrid_libor.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID, usegrid_libor);
	}
	if (usegrid_swap != "") 
	{
		usegrid_swap = usegrid_swap.subString(0, usegrid_swap.size() - 2);
		while ((find = usegrid_swap.findString("NONE:")) != -1) usegrid_swap.remove(find, 5);
		if ((find = usegrid_swap.findString(":NONE")) != -1) usegrid_swap.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID, usegrid_swap);
	}
	if (usegrid_fra3m != "") 
	{
		usegrid_fra3m = usegrid_fra3m.subString(0, usegrid_fra3m.size() - 2);
		while ((find = usegrid_fra3m.findString("NONE:")) != -1) usegrid_fra3m.remove(find, 5);
		if ((find = usegrid_fra3m.findString(":NONE")) != -1) usegrid_fra3m.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID, usegrid_fra3m);
	}
	if (usegrid_fra6m != "") 
	{
		usegrid_fra6m = usegrid_fra6m.subString(0, usegrid_fra6m.size() - 2);
		while ((find = usegrid_fra6m.findString("NONE:")) != -1) usegrid_fra6m.remove(find, 5);
		if ((find = usegrid_fra6m.findString(":NONE")) != -1) usegrid_fra6m.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID, usegrid_fra6m);
	}
	if (usegrid_3m6m != "") 
	{
		usegrid_3m6m = usegrid_3m6m.subString(0, usegrid_3m6m.size() - 2);
		while ((find = usegrid_3m6m.findString("NONE:")) != -1) usegrid_3m6m.remove(find, 5);
		if ((find = usegrid_3m6m.findString(":NONE")) != -1) usegrid_3m6m.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + "3m6mbasis", usegrid_3m6m);
	}
	if (usegrid_xccy != "") 
	{
		usegrid_xccy = usegrid_xccy.subString(0, usegrid_xccy.size() - 2);
		while ((find = usegrid_xccy.findString("NONE:")) != -1) usegrid_xccy.remove(find, 5);
		if ((find = usegrid_xccy.findString(":NONE")) != -1) usegrid_xccy.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + "xccybasis", usegrid_xccy);
	}
	if (usegrid_future != "") 
	{
		usegrid_future = usegrid_future.subString(0, usegrid_future.size() - 2);
		while ((find = usegrid_future.findString("NONE:")) != -1) usegrid_future.remove(find, 5);
		if ((find = usegrid_future.findString(":NONE")) != -1) usegrid_future.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID, usegrid_future);
	}

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, CURVETYPE_ARBFREE);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE, "TRUE");

	if (curveNames_3ML == "")
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MLCURVENAME, CURVENAME_3ML);
	}
	else
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MLCURVENAME, curveNames_3ML);
	}
	if (curveNames_6ML == "")
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MLCURVENAME, CURVENAME_6ML);
	}
	else
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MLCURVENAME, curveNames_6ML);
	}
	if (curveNames_DF == "")
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DFCURVENAME, CURVENAME_DF);
	}
	else
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DFCURVENAME, curveNames_DF);
	}
	
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	if (curveName_DF2 != "")
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2, curveName_DF2);
	}

	AQLCalibrateModel *generator = new AQLCalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);

	//resist generatetime
	AQLString	CurveIDTool			= curveID + TOOL;
	AQLString	CurveIDManager		= curveID + CURVETYPE_ARBFREE + MANAGER;

	//curve entities manager (curve, grids, curveinformation entities)
	AQLObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new AQLObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new AQLDataString()			).convertFromString(AQLString(AQLTime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new AQLDataString()			).convertFromString(CurveIDManager);

	return;
};

void 
AQLCurveSetup::
setUpOISCurve
(AQLDataInstance* dataInstance,
const AQLString& curveID,
const AQLString& marketName,
const AQLStringMatrix& generateProp, 
const AQLStringMatrix& oisRates, 
const AQLStringMatrix& oisConv,
const AQLString& curveNames,
const AQLStringMatrix& histRates,
const AQLStringMatrix& lobasisRates, 
const AQLStringMatrix& lobasisConv, 
const AQLStringMatrix& swapRates, 
const AQLStringMatrix& swapConv
)
{
	AQLStaticData &irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLObjectPool &objPool = dataInstance->getObjectPool();
	AQLObject* pyld = NULL;
	
	AQLStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	AQLDate asofdate	= AQLMathDateUtilities::getAQLDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	AQLCoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	AQLString currency;
	AQLObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		AQLObject& yldEntity = objHolder.get();
		const AQLDataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const AQLDataString&> (dh->get())).get();
		}
		else
		{
			if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = AQLString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = AQLString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	AQLString tmpCurrency = currency; tmpCurrency.toLower();

	if (marketName == "" || marketName == STD) 
		throw AQLCoreInvalidData("Do not use STD or blank for ois curve name!",__FILE__,__LINE__);
	AQLString staticDataSuffix;
	AQLString suffix_data;
	staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	AQLString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	AQLStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (useMarkets == AQ_NO_DATA) useMarkets = "";
	AQLString tmpCurveName = marketName; tmpCurveName.toUpper();
	if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
	{
		if (useMarkets == "") useMarkets = marketName;
		else useMarkets += AQLString(MULTI_STATIC_DATA_DELIMITER) + marketName;
	}
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
	if (curveNames == "")
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix, marketName);
	}
	else
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix, curveNames);
	}

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + staticDataSuffix, DAILYCOMPOUNDING);

	suffix_data = "_" + marketName;
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix);

	for(size_t i=0; i<generateProp.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		AQLString data = generateProp[i][1];
		if (key.findString("dfcurvename") == -1) data.toLower();
		irStaticData.setStaticData(key, data);
	}
	
	for(size_t i=0; i<oisConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.ois." + oisConv[i][0] + staticDataSuffix;
		key.toLower(); 
		AQLString data = oisConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}	

	AQLString oisFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + staticDataSuffix);
	if (oisFile == AQ_NO_DATA)
	{
		oisFile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_ois_oiscurve.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + staticDataSuffix, oisFile);
	}

	AQLString fedFundFutureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + staticDataSuffix);
	if (fedFundFutureFile == AQ_NO_DATA)
	{
		fedFundFutureFile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_fffuture.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + staticDataSuffix, fedFundFutureFile);
	}

	AQLString histFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + staticDataSuffix);
	if (histFile == AQ_NO_DATA)
	{
		histFile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_historical_ois_oiscurve.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + staticDataSuffix, histFile);
	}

	AQLString oisStream;
	AQLString fedFundFutureStream;
	AQLString usegrid_ois = "";
	AQLString usegrid_fffuture = "";
	for(size_t i=0; i<oisRates.size(); i++)
	{
		if (oisRates[i][0].findString("FF") != -1)
		{
			fedFundFutureStream += oisRates[i][0];

			if (oisRates[i].size() != 3 && oisRates[i].size() != 5)
				throw AQLCoreInvalidData("FF input size error",__FILE__,__LINE__);

			//in case of FF, quoted value is price
			double oisRate = oisRates[i][1].getDoubleValue();
			fedFundFutureStream += "," + AQLString(oisRate);

			if (oisRates[i].size() == 5)
			{
				const AQLDate& startdate = AQLMathDateUtilities::getAQLDate(oisRates[i][2]);
				AQLString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				fedFundFutureStream += "," + startdate_str;
				const AQLDate& enddate = AQLMathDateUtilities::getAQLDate(oisRates[i][3]);
				AQLString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				fedFundFutureStream += "," + enddate_str;
			}

			fedFundFutureStream += LF;
			
			if (oisRates[i].size() == 5)
			{
				AQLString useGridFrag = oisRates[i][4]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fffuture += oisRates[i][0] + ":";
				else usegrid_fffuture += "NONE:";
			}
			else if (oisRates[i].size() == 3)
			{
				AQLString useGridFrag = oisRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fffuture += oisRates[i][0] + ":";
				else usegrid_fffuture += "NONE:";
			}
		}
		else
		{
			oisStream += oisRates[i][0];
			if (oisRates[i][0].findString("BOJ") != -1 || oisRates[i][0].findString("EUSF") != -1)
			{
				if (oisRates[i].size() < 4)
					throw AQLCoreInvalidData("short term market needs StartDate and EndDate",__FILE__,__LINE__);

				double oisRate = oisRates[i][1].getDoubleValue() * 100.0;
				oisStream += "," + AQLString(oisRate);

				const AQLDate& startdate = AQLMathDateUtilities::getAQLDate(oisRates[i][2]);
				AQLString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				oisStream += "," + startdate_str;
				const AQLDate& enddate = AQLMathDateUtilities::getAQLDate(oisRates[i][3]);
				AQLString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				oisStream += "," + enddate_str;
			}
			else //normal case
			{
				double oisRate = oisRates[i][1].getDoubleValue() * 100.0;
				oisStream += "," + AQLString(oisRate);
			}
			
			oisStream += LF;
			
			if (oisRates[i].size() == 5)
			{
				AQLString useGridFrag = oisRates[i][4]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_ois += oisRates[i][0] + ":";
				else usegrid_ois += "NONE:";
			}
			else if (oisRates[i].size() == 3)
			{
				AQLString useGridFrag = oisRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_ois += oisRates[i][0] + ":";
				else usegrid_ois += "NONE:";
			}
		}
	}
	std::istringstream *pOISStream = new std::istringstream(oisStream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(oisFile), pOISStream);
	std::istringstream *pFedFundFutureStream = new std::istringstream(fedFundFutureStream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(fedFundFutureFile), pFedFundFutureStream);


	///////////////
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + staticDataSuffix);
	int find;
	if (usegrid_ois != "") 
	{
		usegrid_ois = usegrid_ois.subString(0, usegrid_ois.size() - 2);
		while ((find = usegrid_ois.findString("NONE:")) != -1) usegrid_ois.remove(find, 5);
		if ((find = usegrid_ois.findString(":NONE")) != -1) usegrid_ois.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + staticDataSuffix, usegrid_ois);
	}
	if (usegrid_fffuture != "") 
	{
		usegrid_fffuture = usegrid_fffuture.subString(0, usegrid_fffuture.size() - 2);
		while ((find = usegrid_fffuture.findString("NONE:")) != -1) usegrid_fffuture.remove(find, 5);
		if ((find = usegrid_fffuture.findString(":NONE")) != -1) usegrid_fffuture.remove(find, 5);
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID + staticDataSuffix, usegrid_fffuture);
	}

	AQLString histStream;
	for(size_t i=0; i<histRates.size(); i++)
	{
		const AQLDate& histdate = AQLMathDateUtilities::getAQLDate(histRates[i][0]);
		AQLString histdate_str = histdate.stringWithFormat("YYYYMMDD");
		histStream += histdate_str;
		double histRate = histRates[i][1].getDoubleValue() * 100.0;
		histStream += "," + AQLString(histRate);
		histStream += LF;
	}
	std::istringstream *pHISTStream = new std::istringstream(histStream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(histFile), pHISTStream);

	AQLString lobasisname = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + staticDataSuffix);
	if (lobasisname == AQ_NO_DATA)
	{
		lobasisname = LOBASIS;
		AQLString key = AQLString(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + staticDataSuffix).toLower();
		irStaticData.setStaticData(key, lobasisname);
	}
	AQLString suffix_lob_prop("." + lobasisname);
	AQLString suffix_lob_data("_" + lobasisname);
	for(size_t i=0; i<lobasisConv.size(); i++)
	{
		AQLString key(tmpCurrency + ".sde.yield.basis." + lobasisConv[i][0] + suffix_lob_prop);
		key.toLower(); 
		AQLString data = lobasisConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}
	AQLString lobasisfile = irStaticData.getStaticData(AQLString(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + suffix_lob_prop).toLower());
	if (lobasisfile == AQ_NO_DATA)
	{
		lobasisfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_basisswap") + suffix_lob_data + AQLString(".csv");
		lobasisfile.toLower();
		AQLString key(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + suffix_lob_prop);
		key.toLower();
		irStaticData.setStaticData(key, lobasisfile);
	}
	AQLString lobasisstream;
	for(size_t i=0; i<lobasisRates.size(); i++)
	{
		lobasisstream += lobasisRates[i][0];
		const double lobrate = lobasisRates[i][1].getDoubleValue() * 10000.0;
		lobasisstream += "," + AQLString(lobrate) + LF;
	}
	std::istringstream *plobasisstream = new std::istringstream(lobasisstream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(lobasisfile), plobasisstream);

	for(size_t i=0; i<swapConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0] + staticDataSuffix;
		key.toLower(); 
		AQLString data = swapConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}	
	AQLString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	if (swapfile == AQ_NO_DATA)
	{
		swapfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_swap") + suffix_data + AQLString(".csv");
		swapfile.toLower();
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix, swapfile);
	}
	AQLString swapstream;
	for(size_t i=0; i<swapRates.size(); i++)
	{
		swapstream += swapRates[i][0];
		double srate = swapRates[i][1].getDoubleValue() * 100.0;
		swapstream += "," + AQLString(srate) + LF;
	}
	std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(swapfile), pswapstream);
	

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + staticDataSuffix, MARKETTYPE_SWAP);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	AQLCalibrateModel *generator = new AQLCalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);

	//resist generatetime
	AQLString	CurveIDTool			= curveID + TOOL;
	AQLString	CurveIDManager		= curveID + marketName + MANAGER;
	//curve entities manager (curve, grids, curveinformation entities)
	AQLObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new AQLObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new AQLDataString()			).convertFromString(AQLString(AQLTime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new AQLDataString()			).convertFromString(CurveIDManager);

	return;
};

void
AQLCurveSetup::
setUpFloater
(AQLDataInstance* dataInstance,
 const AQLString& curveID,
 const AQLString& discountCurveName,
 const AQLString& forecastCurveName,
 const AQLStringMatrix& generateProp, 
 const AQLStringMatrix& basisMkt, 
 const AQLStringMatrix& basisConv,
 const AQLStringMatrix& swapConv,
 const AQLStringMatrix& adjustData)
{
	AQLStaticData &irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();

	AQLObjectPool &objPool = dataInstance->getObjectPool();

	AQLStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	AQLString currency;
	AQLObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		AQLObject& yldEntity = objHolder.get();
		const AQLDataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const AQLDataString&> (dh->get())).get();
		}
		else
		{
			if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = AQLString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (AQLFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = AQLString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	AQLString tmpCurrency = currency; tmpCurrency.toLower();
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);

	//insert property
	AQLStringMatrix tmpProp(basisConv.size() + swapConv.size() + generateProp.size(), AQLStringVector(2));
	AQLString tmpBasisCurveName = XCCYBASIS; tmpBasisCurveName.toLower();
	for(size_t i=0; i<basisConv.size(); i++)
	{		
		AQLString key = tmpCurrency + ".sde.yield.basis." + basisConv[i][0] + "." + tmpBasisCurveName;
		key.toLower();
		AQLString data = basisConv[i][1];
		if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		irStaticData.setStaticData(key,data);
	}

	for(size_t i=0; i<swapConv.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0];
		key.toLower(); 
		AQLString data = swapConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key,data);
	}

	for(size_t i=0; i<generateProp.size(); i++)
	{
		AQLString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		AQLString data = generateProp[i][1];
		data.toUpper();
		irStaticData.setStaticData(key,data);
	}

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT, discountCurveName);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FLOATER_FORECAST, forecastCurveName);

	if (!basisMkt.empty())
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME, XCCYBASIS);

		AQLString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
		AQLStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (useMarkets == AQ_NO_DATA) useMarkets = "";
		if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),XCCYBASIS)) 
		{
			if (useMarkets == "") useMarkets = XCCYBASIS;
			else useMarkets += AQLString(MULTI_STATIC_DATA_DELIMITER) + XCCYBASIS;
		}
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
		
		AQLString tmpBasisCurveName = XCCYBASIS;
		tmpBasisCurveName.toLower();

		AQLString basisfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName);
		if (basisfile == AQ_NO_DATA)
		{
			basisfile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_basisswap_xccybasis.csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName, basisfile);
		}
		AQLString basisstream;
		AQLString usegrid = "";
		for(size_t i=0; i<basisMkt.size(); i++)
		{
			basisstream += basisMkt[i][0];
			double brate = basisMkt[i][1].getDoubleValue() * 10000.0;
			basisstream += "," + AQLString(brate) + LF;

			if (basisMkt[i].size() == 3)
			{
				AQLString useGridFrag = basisMkt[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid += basisMkt[i][0] + ":";
				else usegrid += "NONE:";
			}
		}
		std::istringstream *pbasisstream = new std::istringstream(basisstream.getCString());
		AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(basisfile), pbasisstream);

		irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName);
		int find;
		if (usegrid != "") 
		{
			usegrid = usegrid.subString(0, usegrid.size() - 2);
			while ((find = usegrid.findString("NONE:")) != -1) usegrid.remove(find, 5);
			if ((find = usegrid.findString(":NONE")) != -1) usegrid.remove(find, 5);
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName, usegrid);
		}

		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + tmpBasisCurveName, MARKETTYPE_BASIS);

		//set Adjust Data Object;
		if (adjustData.size() > 0)
		{
			AQLString adjustValueFile = irStaticData.getStaticData(tmpCurrency + 
				STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + tmpBasisCurveName);
			if (adjustValueFile == AQ_NO_DATA)
			{
				adjustValueFile = AQLString("data/in/") + tmpCurrency + AQLString("_yield_basisadjust.csv");
				irStaticData.setStaticData(tmpCurrency + 
					STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + tmpBasisCurveName, adjustValueFile);
			}
			AQLString adjustValueStream;
			for(size_t i=0; i<adjustData.size(); i++)
			{
				adjustValueStream += adjustData[i][0];
				adjustValueStream += "," + adjustData[i][1] + LF;	
			}
			std::istringstream *pAdjustValueStream = new std::istringstream(adjustValueStream.getCString());
			AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(adjustValueFile), pAdjustValueStream);
		}
	}

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, CURVETYPE_FLOATER);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS, CURVETYPE_FLOATER);
	AQLCalibrateModel *generator = new AQLCalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);

	delete generator;

	//ylddata
	AQLString	CurveIDTool			= curveID + TOOL;
	AQLString	CurveIDManager		= curveID + CURVETYPE_FLOATER+ MANAGER;
	//curve entities manager (curve, grids, curveinformation entities)
	AQLObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new AQLObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new AQLDataString()			).convertFromString(AQLString(AQLTime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new AQLDataString()			).convertFromString(CurveIDManager);

}

// 
/*!
    @brief setup for irserver

	create property file stream

*/
void
AQLCurveSetup::
setUpForIRServer()
{
	AQLCoreDataService::initialize();
	AQLCoreDataService::setContext(CONTEXT_KEY_ISPRICER, "TRUE");
	AQLCoreDataService::setContext(CONTEXT_KEY_ISEXCELREQUEST, "TRUE");
	AQLCoreDataService::setContext(ARG_KEY_FILENUM, "");

	// create property stream
	std::istringstream *pPropSf = new std::istringstream();
	std::istringstream *pRPropSf = new std::istringstream();
	std::istringstream *pGpropSf = new std::istringstream();
	std::istringstream *pCprofSf = new std::istringstream();
	std::istringstream *pXprofSf = new std::istringstream();
	std::istringstream *pCreditPropSf = new std::istringstream();
	// set prop
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName("ir.properties"), pPropSf);
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName("grid.properties"), pGpropSf);
	// set risk prop
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	staticData.setStaticData(KEY_RISK_SCENARIO_FILE, "risk.properties"); 
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName("risk.properties"), pRPropSf);
	// set max leg num
	staticData.setStaticData(KEY_DEAL_LEG_MAXNUM, "2");
	// set polynomial dimension
	staticData.setStaticData(KEY_SIMULATION_LSMC_BASEFUNCDIM, "2");
	//set calib prop
	//this is for avoiding AQLCoreDataService::getContext(ARG_KEY_NOCALIBTHREAD) == AQ_NO_DATA
	AQLCoreDataService::setContext(ARG_KEY_NOCALIBTHREAD, "tmp");
	staticData.setStaticData(KEY_CALIB_SCENARIO_FILE, "calib.properties"); 
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName("calib.properties"), pCprofSf);
	staticData.setStaticData(KEY_SDE_CALIB_MODEL, "hw:ptberg");
	staticData.setStaticData(KEY_SDE_PV_CALIB_TARGET_CURRENCY, "ALL");
	//set xva prop
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName("xva.properties"), pXprofSf);
	//set credit prop
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName("credit.properties"), pCreditPropSf);

	//set isrealcalibmode //default is false
	AQLStaticData &riskStaticData = AQLCoreDataService::getStaticDataManager().getRiskStaticData();
	riskStaticData.setStaticData(RISK_KEY_CALIB_ISREALCALIB,"false");
	//riskStaticData.setStaticData(RISK_KEY_CALIB_ISREALCALIB,"true");
	
	//this is for avoiding !AQLMarketData::isCalibrateEnd(dIRModelName, objPool) ...
	AQLStaticData &cprop = AQLCoreDataService::getStaticDataManager().getCalibStaticData();
	cprop.setStaticData(KEY_CALIB_WAIT_TIME, "10");
	//seriarize set
	cprop.setStaticData(KEY_CALIB_SERIALIZE_STATUS, "normal");
	
	//testisadjusdf
	AQLString tmpstr = AQLString("jpy"); 
	staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");
	tmpstr = "usd";
	staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");
	tmpstr = "aud";
	staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");
	tmpstr = "eur";
	staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");
	tmpstr = "gbp";
	staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");

	//digitalcoupon set
	AQLString tmpfxstr = AQLString("jpy/usd");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");

	tmpfxstr = AQLString("jpy/aud");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");

	tmpfxstr = AQLString("jpy/eur");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");

	tmpfxstr = AQLString("jpy/gbp");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");
}

void 
AQLCurveSetup::
setUpDefaultIRStaticData(AQLDataInstance& dataInstance)
{
	const AQLString* filepath = AQLCurveProperties::ir_prop_path();
    if(filepath==NULL) return;

	std::ifstream fin;
	fin.open(filepath->getCString());
	if (!fin){
		std::stringstream sst;
		sst << "cannot open ir properties file" << std::endl 
			<< filepath
			;
		throw AQLCoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
	}

    AQLStaticData &irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	std::string line;
	size_t line_num = 0;
	while (getline(fin, line)){
		line_num++;
		const char *c_line = line.c_str();
		AQLString tmpstr(c_line);
		AQLStringVector tmp = tmpstr.toToken('=');
		if (tmp.size() != 2){
			std::stringstream sst;
			sst << "ir proerties file format is invalid" << std::endl
				<< "file : " << filepath << std::endl
				<< "line : " << line_num << std::endl
				<< "contents : " << line
				;
			throw AQLCoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
		}

		irStaticData.setStaticData(tmp[0],tmp[1]);
	}
	fin.close();
}

void 
AQLCurveSetup::
setUpDefaultIRStaticData(AQLDataInstance& dataInstance, AQLString filepath)
{
    AQLCurveProperties::set_ir_prop_path(filepath);
    setUpDefaultIRStaticData(dataInstance);
}

void 
AQLCurveSetup::
setUpDefaultCalibStaticData(AQLDataInstance& dataInstance)
{
	const AQLString* filepath = AQLCurveProperties::calib_prop_path();
    if(filepath==NULL) return;

	std::ifstream fin;
	fin.open(filepath->getCString());
	if (!fin){
		std::stringstream sst;
		sst << "cannot open calib properties file" << std::endl 
			<< filepath
			;
		throw AQLCoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
	}

    AQLStaticData &calibprop = AQLCoreDataService::getStaticDataManager().getCalibStaticData();
	std::string line;
	size_t line_num = 0;
	while (getline(fin, line)){
		line_num++;
		const char *c_line = line.c_str();
		AQLString tmpstr(c_line);
		AQLStringVector tmp = tmpstr.toToken('=');
		if (tmp.size() != 2){
			std::stringstream sst;
			sst << "calib proerties file format is invalid" << std::endl
				<< "file : " << filepath << std::endl
				<< "line : " << line_num << std::endl
				<< "contents : " << line
				;
			throw AQLCoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
		}

		calibprop.setStaticData(tmp[0],tmp[1]);
	}
	fin.close();
}

void 
AQLCurveSetup::
setUpDefaultCalibStaticData(AQLDataInstance& dataInstance, AQLString filepath)
{
    AQLCurveProperties::set_calib_prop_path(filepath);
    setUpDefaultCalibStaticData(dataInstance);
}
/*!
    @brief setup for irserver

	create property file stream

*/
void
AQLCurveSetup
::resetService()
{
	AQLCoreDataService::finalize();
	setUpForIRServer();
}

AQLString
AQLCurveSetup::
getIRStaticData(const AQLString& key)
{
	AQLCoreDataService::setContext(ARG_KEY_FILENUM, "");
	AQLStaticData &irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	return irStaticData.getStaticData(key);
}

std::istringstream* 
AQLCurveSetup::createFutureStream(const AQLStringMatrix& future_rates, AQLString& usegrid_future)
{
    AQLString futureStream;
    for(size_t i=0; i<future_rates.size(); i++)
    {
        if (future_rates[i].size() <= 4)
        {
            if (future_rates[i][0].size() < 3)
                throw AQLCoreInvalidData("future size error",__FILE__,__LINE__);

            futureStream += future_rates[i][0];
            double futureRate = future_rates[i][1].getDoubleValue();
            futureStream += "," + AQLString(futureRate);
            double futureVol = future_rates[i][2].getDoubleValue();
            futureStream += "," + AQLString(futureVol) + LF;

            if (future_rates[i].size() == 4)
            {
                AQLString useGridFrag = future_rates[i][3]; upper(useGridFrag);
                if (useGridFrag == "TRUE") usegrid_future += future_rates[i][0] + ":";
            }
        }
        else if (future_rates[i].size() <= 6)
        {
            futureStream += future_rates[i][0];
            const AQLDate& startdate = AQLMathDateUtilities::getAQLDate(future_rates[i][1]);
            AQLString startdate_str = startdate.stringWithFormat("YYYYMMDD");
            futureStream += "," + startdate_str;
            const AQLDate& enddate = AQLMathDateUtilities::getAQLDate(future_rates[i][2]);
            AQLString enddate_str = enddate.stringWithFormat("YYYYMMDD");
            futureStream += "," + enddate_str;
            double futureRate = future_rates[i][3].getDoubleValue();
            futureStream += "," + AQLString(futureRate);
            double futureVol = future_rates[i][4].getDoubleValue();
            futureStream += "," + AQLString(futureVol) + LF;

            if (future_rates[i].size() == 6)
            {
                AQLString useGridFrag = future_rates[i][5]; upper(useGridFrag);
                if (useGridFrag == "TRUE") usegrid_future += future_rates[i][0] + ":";
				else usegrid_future += "NONE:";
            }
        }
    }
    return new std::istringstream(futureStream.getCString());
}

double
AQLCurveSetup::compound(AQLDataInstance* dataInstance,
                           const AQLString& curveID,
                           const AQLString& forecastCurveName,
                           const AQLDate& start_date,
                           const AQLDate& end_date,
                           const double spread,
                           const AQLString& frequency_,
                           const bool is_start_roll,
                           const AQLString& roll_convention_,
                           const AQLString& calendar_,
                           const AQLString& sliding_rule_,
                           const AQLString& day_count_,
                           const AQLString& interpolation_,
                           const AQLString& compound_type_,
                           const AQLDate* first_odd,
                           const AQLDate* last_odd)
{
    AQLString frequency = frequency_; frequency.toUpper();
    AQLString roll_convention = roll_convention_; roll_convention.toUpper();
    AQLString calendar = calendar_; calendar.toUpper();
    AQLString sliding_rule = sliding_rule_; sliding_rule.toUpper();
    AQLString day_count = day_count_; day_count.toUpper();
    AQLString interpolation = interpolation_; interpolation.toUpper();
    AQLString compound_type_str = compound_type_; compound_type_str.toUpper();
    
    AQLCurveSetup::COMPOUND_TYPE compound_type;
    if(compound_type_str=="NORMAL") compound_type = AQLCurveSetup::COMPOUND_NORMAL;
    else if(compound_type_str=="FLAT") compound_type = AQLCurveSetup::COMPOUND_FLAT;
    else if(compound_type_str=="SIMPLE") compound_type = AQLCurveSetup::COMPOUND_SIMPLE;
    else if(compound_type_str=="AVERAGE") compound_type = AQLCurveSetup::AVERAGE;
    else{
        AQLString msg;
        msg += "Unknown compound type:";
        msg += compound_type_;
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }




    AQLPriceDataCalendar data_cal; data_cal.convertFromString(calendar);
    AQLPriceDataSlidingRule data_sr; data_sr.convertFromString(sliding_rule);


    DateVector payment_dates;
    AQLMathDateCalculations::generateSchedule(
        start_date,
        end_date,
        frequency,
        true, 
        first_odd,
        last_odd,
        NULL, 
        payment_dates,
        &data_sr,
        &data_cal,
        is_start_roll,
        &roll_convention);



    const size_t n = payment_dates.size();
    DateVector start_dates(n), end_dates(n);
    start_dates[0] = start_date;
    end_dates[0] = payment_dates[0];
    for(size_t i = 1; i < n; i++){
        start_dates[i] = payment_dates[i-1];
        end_dates[i] = payment_dates[i];
    }



    const AQLString freq = SIMPLE;

	const AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveID);
	const AQLObject& yieldData = yc.getYieldData().get().get();
	AQLString suffix;
	if (forecastCurveName != STD)
	{
		suffix = "_" + forecastCurveName;
	}
	const AQLDataHolder* dh = &(yieldData.getData(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, NOCHECK));
	const bool is_fwd_inter = dh->isDefined() && !dh->isNull();

    DoubleArray rates = AQLMathCurveFuncUtility::getMultiForwardRate(start_dates,
                                                                  end_dates,
                                                                  dataInstance,
                                                                  curveID,
                                                                  freq,
                                                                  day_count,
                                                                  sliding_rule,
                                                                  calendar,
                                                                  interpolation,
                                                                  forecastCurveName,
																  is_fwd_inter);


    AQLPriceDataDayCount dc; dc.convertFromString(day_count);
    DoubleVector x(3*n);
    copy(rates.begin(), rates.end(), x.begin());
    for(size_t i = 0; i < n; i++){
        x[n+i]     = dc.getTerm(start_dates[i], end_dates[i]);
        x[n*2 + i] = spread;
    }
	std::shared_ptr<AQLCompoundMethod> method;
	// if compound type is simple or average, we need to add whole term to final element
	// this operation is al so implemented in AQLPricePayOffToolCompound::calculateCouponCashflow().
    switch(compound_type){
        case COMPOUND_NORMAL:
            {
                method.reset(new AQLCompoundMethod7());
                break;
            }
        case COMPOUND_FLAT:
            {
                method.reset(new AQLCompoundMethod8());
                break;
            }
        case COMPOUND_SIMPLE:
            {
                method.reset(new AQLCompoundMethod9());
                x.push_back(dc.getTerm(start_date, end_date));
                break;
            }
        case AVERAGE:
            {
                method.reset(new AQLCompoundMethod10());
				x.push_back(dc.getTerm(start_date, end_date));
				break;
            }
    }

    return (*method)(x);
}

/*!
    @brief set value to property accessor

	@param[in,out] prop
	@param[in] key
	@param[in] value
	@param[in] override flag

*/
void
AQLCurveSetup::setStaticDataValue(AQLStaticData &staticData, const AQLString &key, const AQLString &val, const bool is_override)
{
	if (is_override)
	{
		staticData.setStaticData(key , val);
	}
	else
	{
		const AQLString orig = staticData.getStaticData(key);
		if (orig == AQ_NO_DATA)
		{
			staticData.setStaticData(key , val);
		}
	}
}

void
AQLCurveSetup::SetUpFundingSpread(AQLDataInstance* dataInstance, const AQLStringMatrix &fundingSpread)
{
	// save in string stream
	AQLString stream;
	for (unsigned int i = 0; i < fundingSpread.size(); ++i)
	{
		stream += fundingSpread[i][0];
		stream += "," + fundingSpread[i][1] + LF;
	}
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString fdspdfile = staticData.getStaticData(KEY_FUNDINGSPREAD_FILE);
	std::istringstream *pstream = new std::istringstream(stream.getCString());
	AQLCoreDataService::setIStringStream(AQLMarketData::getNumFileName(fdspdfile), pstream);
}

void
AQLCurveSetup::CalcMeanAndCovariance(const DoubleMatrix& data, DoubleArray& mean, DoubleMatrix& covar)
{
	size_t ValNum = data.size();
	size_t ObsNum = data[0].size();
	mean.resize(ValNum);
	for (size_t j = 0; j < ValNum; j++)
	{
		double tmp = 0.;
		for(size_t k = 0; k < ObsNum; k++)
		{
			tmp += data[j][k];
		}
		mean[j] = tmp / static_cast<double>(ObsNum);
	}

	covar.resize(ValNum);
	for (size_t i = 0; i < ValNum; i++)
	{
		covar[i].resize(ValNum);
		for(size_t j = i; j < ValNum; j++)
		{
			double tmp = 0.;
			for(size_t k = 0; k < ObsNum; k++)
			{
				tmp += (data[i][k] - mean[i]) * (data[j][k] - mean[j]);
			}
			covar[i][j] = tmp / static_cast<double>(ObsNum);
		}
	}

	for (size_t i = 0; i < ValNum; i++)
		for(size_t j = i; j < ValNum; j++)
			covar[j][i] = covar[i][j];


}

void
AQLCurveSetup::SetUpPCA(AQLDataInstance* dataInstance, const DoubleMatrix& corr, const size_t no_factors, const AQLString& id)
{

	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLMathCorrelation* cor_obj = NULL;
	AQLString name = PREFIX_COR + id;
	if(!objPool.getObject(name).isDefined())
	{
		cor_obj = new AQLMathCorrelation(dataInstance);
		objPool.set(name, cor_obj);
	}
	else
	{
		cor_obj = &dynamic_cast<AQLMathCorrelation &>(objPool.getObject(name).get());
		cor_obj->reset();
	}

	if (no_factors > corr.size())
		throw AQLCoreInvalidData("factornumber is bigger than correlation size", __FILE__, __LINE__);

	DoubleArray dummy_tgrid(corr.size());
	for(size_t i = 0; i < corr.size(); i++)
	{
		dummy_tgrid[i] = static_cast<double>(i + 1); 
	}
	cor_obj->getTGrid().set(dummy_tgrid);

	cor_obj->getIsMultiVol().set(false);

	dynamic_cast<AQLDataInt&>(cor_obj->getData(IR_CALIBRATION_DATA_FACTORNUM_AFTER, ISDEFINED).get()).set(no_factors);
	dynamic_cast<AQLDataBool&>(cor_obj->getData(IR_CALIBRATION_DATA_ISOPTIM, ISDEFINED).get()).set(false);

	cor_obj->setCorrelation(corr);
	cor_obj->calcFactorLoading();
}

DoubleMatrix
AQLCurveSetup::GetPCAResult(AQLDataInstance* dataInstance, const AQLString& type, const AQLString& id)
{
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLString name = PREFIX_COR + id;
	AQLMathCorrelation cor_obj = dynamic_cast<AQLMathCorrelation& >(objPool.getObject(name, ENCHKTYPE_ISDEFINED).get());

	DoubleMatrix ret;
	if(type == AQLString("EIGEN_VECTORS"))
	{
		ret = dynamic_cast<AQLDataDoubleMatrix& >(cor_obj.getData(IR_CALIBRATION_DATA_EIGENVECTORS, ISNOTNULL).get()).get();
	}
	else if(type == AQLString("EIGEN_VALUES"))
	{
		DoubleArray tmp = dynamic_cast<AQLDataDoubles&>(cor_obj.getData(IR_CALIBRATION_DATA_EIGENVALUES, ISNOTNULL).get()).get();
		ret.push_back(tmp);
	}
	else if (type == AQLString("POV"))
	{
		DoubleArray tmp = dynamic_cast<AQLDataDoubles&>(cor_obj.getData(IR_CALIBRATION_DATA_EIGENVALUES, ISNOTNULL).get()).get();
		double sum_tmp = 0.;
		for(unsigned int i = 0; i < tmp.size(); i++)
			sum_tmp += tmp[i];
		for(unsigned int i = 0; i < tmp.size(); i++)
			tmp[i] /= sum_tmp;
		ret.push_back(tmp);
	}
	else
	{
		AQLString msg = AQLString("Unknown result type: ") + type;
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	return ret;

}

