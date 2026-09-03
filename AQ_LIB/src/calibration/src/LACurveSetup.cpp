/*! @file
    @brief  M-Library Excel Addin Wrap class
*/
//  2007, AlgoQuantHub.

///X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACurveSetup.cpp
//
//  DESCRIPTION :       M-Library Excel Addin Lap class 
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

#include <LADataBasics.h>
#include <LADataVector.h>
#include <LADataMatrix.h>
#include <LADataReference.h>
#include <LAMathYieldCurve.h>
#include <LAMathYieldCurvePro.h>
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LACurveSetup.h"
#include "LACalibrateModelIRVanilla.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LADefinitionsCalibration.h"
#include "LAStaticData.h"
#include "LACoreDataService.h"
#include "LAMarketData.h"
#include "LAStaticDataManager.h"
#include <LAFunctionUtilities.h>
#include "LAMathDateUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathCorrelation.h"
#include "LACompoundingFunc.h"
#include "LACurveProperties.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAPriceDataFunction.h"
#include "LA1DIntegral.h"
#include "LACombinationFunc.h"
#include "LAFunction.h"
#include "LAShiftFunc.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAPriceCFGenUtility.h"
#include "LAPriceTradeValue.h"

//#include "LADefinitionsHazardRate.h"
//#include "LAHazardRateCreator.h"
//#include "LAMathCreditEntitySetUp.h"
//#include "LAMathSingleCredit.h"
#include "LA1DDataSet.h"
//#include "LAPriceAccruedIntFunc.h"
//#include "LAPriceCDS.h"

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

const LAString LF = "\n";

void
LACurveSetup::
setUpBasisCurve
(LADataInstance* dataInstance,
 const LAString& curveID, 
 const LAString& marketName, 
 const LAStringMatrix& basisRates, 
 const LAStringMatrix& basisConv,
 const LAStringMatrix& fwdFXs, 
 const LAStringMatrix& fwdConv,
 const LAStringMatrix& spotFXs, 
 const LAStringMatrix& generateProp,
 const LAStringMatrix& moneyConv,
 const LAString& curveNames)
{
	LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();

	LAObjectPool &objPool = dataInstance->getObjectPool();

	LAStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	LADate asofdate = LAMathDateUtilities::getLADate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
    LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	LAString currency;
	LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		LAObject& yldEntity = objHolder.get();
		const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		}
		else
		{
			if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = LAString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = LAString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	LAString tmpCurrency = currency; tmpCurrency.toLower();
	
	//set market rate	
	if (marketName == "" || marketName == STD) 
		throw LACoreInvalidData("Do not use STD or blank for basis curve name!",__FILE__,__LINE__);

	LAString staticDataSuffix;
	LAString suffix_data;
	staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	suffix_data = "_" + marketName;
	suffix_data.toLower();

	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);

	LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (useMarkets == MLIB_NO_DATA) useMarkets = "";
	LAString tmpCurveName = marketName; tmpCurveName.toUpper();
	if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
	{
		if (useMarkets == "") useMarkets = marketName;
		else useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + marketName;
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
	LAString basisfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + staticDataSuffix);
	if (basisfile == MLIB_NO_DATA)
	{
		basisfile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap") + suffix_data + LAString(".csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + staticDataSuffix, basisfile);
	}
	LAString basisstream;
	LAString usegrid = "";
	for (unsigned int i = 0; i < basisRates.size();i++)
	{
		basisstream += basisRates[i][0];
		const double brate = basisRates[i][1].getDoubleValue() * 10000.0;
		basisstream += "," + LAString(brate) + LF;

		if (basisRates[i].size() == 3)
		{
			LAString useGridFrag = basisRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid += basisRates[i][0] + ":";
			else usegrid += "NONE:";
		}
	}
	std::istringstream *pbasisstream = new std::istringstream(basisstream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(basisfile), pbasisstream);

	//fwdfx file
	LAString fwdfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FILE + staticDataSuffix);
	LAString fwdstream;
	LAString fwdusegrid = "";
	for (unsigned int i = 0; i < fwdFXs.size();i++)
	{
		fwdstream += fwdFXs[i][0];
		const double fwdfx = fwdFXs[i][1].getDoubleValue();
		fwdstream += "," + LAString(fwdfx) + LF;

		if (fwdFXs[i].size() == 3)
		{
			LAString useGridFrag = fwdFXs[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") fwdusegrid += fwdFXs[i][0] + ":";
			else fwdusegrid += "NONE:";
		}
	}
	std::istringstream *pfwdstream = new std::istringstream(fwdstream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fwdfile), pfwdstream);

	//basis info
	for(size_t i=0; i<basisConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.basis." + basisConv[i][0] + staticDataSuffix;
		key.toLower();
		LAString data = basisConv[i][1];
		if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		irStaticData.setStaticData(key,data);
	}
	//generator info
	for(size_t i=0; i<generateProp.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		LAString data = generateProp[i][1];
		data.toLower();
		irStaticData.setStaticData(key,data);
	}
	//money info
	for(size_t i=0; i<moneyConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv[i][0];
		key.toLower();
		LAString data = moneyConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key,data);
	}
	//fwdfx info
	for(size_t i=0; i<fwdConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.basis.fwdfx." + fwdConv[i][0] + staticDataSuffix;
		key.toLower();
		LAString data = fwdConv[i][1];
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
	LAMathFXEntity* pFwd = NULL;
	LAObjectHolder ehfx = objPool.getObject(FORWARDFX, ENCHKTYPE_NOCHECK);
	if (!ehfx.isDefined())
	{
		pFwd = new LAMathFXEntity(dataInstance);
		objPool.set(FORWARDFX,pFwd);
	}
	else
	{
		dynamic_cast<LAMathFXEntity &>(objPool.getObject(FORWARDFX).get()).reset();
		pFwd = &dynamic_cast<LAMathFXEntity &>(objPool.getObject(FORWARDFX).get());
	}
	pFwd->getName().convertFromString(FORWARDFX);
	pFwd->getFXType().convertFromString("FORWARDRATE");
	LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD, FORWARDFX);
	const int ccySize = spotFXs.size();
	LAStringVector ccys(ccySize);
	DoubleVector spotrates(ccySize);
	LAString unitccy = irStaticData.getStaticData(KEY_FXSPOTRATES_UNITCCY);
	unitccy.toUpper();
	for(unsigned int i = 0; i < ccySize; ++i)
	{
        if (spotFXs[i].size() < 3)
		{
			throw LACoreInvalidData("spotrate file size error", __FILE__, __LINE__);
		}
		LAString tmpccy = spotFXs[i][0];
		if (tmpccy.toUpper() != unitccy)
		{
			throw LACoreInvalidData("The first column of a spot rate file must be unit currency", __FILE__, __LINE__);
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
	LACalibrateModel *generator = new LACalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);
	delete generator;

	//ylddata
	LAString	CurveIDTool			= curveID + TOOL;
	LAString	CurveIDManager		= curveID + marketName + MANAGER;
	//curve entities manager (curve, grids, curveinformation entities)
	LAObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new LAObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);
}

void
LACurveSetup::
setUpFwdFXConstantCurve
(LADataInstance* dataInstance,
 const LAString& curveID, 
 const LAString& marketName, 
 const LAStringMatrix& fwdfxconstConv,
 const LAStringMatrix& generateProp,
 const LAString& curveNames)
{
	LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();

	LAObjectPool &objPool = dataInstance->getObjectPool();

	LAStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	LADate asofdate = LAMathDateUtilities::getLADate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
    LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	LAString currency;
	LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		LAObject& yldEntity = objHolder.get();
		const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		}
		else
		{
			if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = LAString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = LAString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	LAString tmpCurrency = currency; tmpCurrency.toLower();
	
	//set market rate	
	if (marketName == "" || marketName == STD) 
		throw LACoreInvalidData("Do not use STD or blank for basis curve name!",__FILE__,__LINE__);

	LAString staticDataSuffix;
	LAString suffix_data;
	staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	suffix_data = "_" + marketName;
	suffix_data.toLower();

	LAString tmpCurveName = marketName; tmpCurveName.toUpper();
	LAString isFwdFxCon = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST);
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
		LAString key = tmpCurrency + ".sde.yield.basis." + fwdfxconstConv[i][0] + staticDataSuffix;
		key.toLower();
		LAString data = fwdfxconstConv[i][1];
		if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		irStaticData.setStaticData(key,data);
	}
	for(size_t i=0; i<generateProp.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		LAString data = generateProp[i][1];
		data.toLower();
		irStaticData.setStaticData(key,data);
	}

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	//set generate df
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + staticDataSuffix, MARKETTYPE_BASIS);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	LACalibrateModelIR *generator = new LACalibrateModelIRVanilla(currency);
	generator->loadFwdFXConstCurveDataAndCalibrate(currency, *dataInstance);
	delete generator;

	//ylddata
	LAString	CurveIDTool			= curveID + TOOL;
	LAString	CurveIDManager		= curveID + marketName + MANAGER;
	//curve entities manager (curve, grids, curveinformation entities)
	LAObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new LAObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST, isFwdFxCon);
}

void 
LACurveSetup::
setUpSwapCurve
(LADataInstance* dataInstance,
 const LAString& curveID,
 const LAString& marketName,
 const LAStringMatrix& generateProp, 
 const LAStringMatrix& moneyConv,
 const LAStringMatrix& liborRates, 
 const LAStringMatrix& liborConv,
 const LAStringMatrix& swapRates, 
 const LAStringMatrix& swapConv,
 const LAStringMatrix& fra3mRates,
 const LAStringMatrix& fra6mRates,
 const LAStringMatrix& fraConv,
 const LAStringMatrix& futureRates, 
 const LAStringMatrix& futureConv,
 const LAStringMatrix& adjustSwapConv,
 const LAStringMatrix& adjustSwapRates,
 const LAString& curveNames,
 const LAString& curveName_DF2)
{
	LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();

	LAObjectPool &objPool = dataInstance->getObjectPool();
	LAObject* pyld = NULL;

	LAStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	LADate asofdate	= LAMathDateUtilities::getLADate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	LAString currency;
	LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		LAObject& yldEntity = objHolder.get();
		const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		}
		else
		{
			if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = LAString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = LAString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	LAString tmpCurrency = currency; tmpCurrency.toLower();

	LAString generateCurveName = (marketName == "") ? STD : marketName;
	LAString staticDataSuffix;
	LAString suffix_data;
	LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (useMarkets == MLIB_NO_DATA) useMarkets = "";
	if (generateCurveName != STD)
	{
		staticDataSuffix = "." + generateCurveName;
		staticDataSuffix.toLower();
		LAString tmpCurveName = generateCurveName; tmpCurveName.toUpper();
		if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
		{
			if (useMarkets == "") useMarkets = generateCurveName;
			else useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + generateCurveName;
		}	
	}
	else
	{	
		if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),SWAP)) 
		{
			if (useMarkets == "") useMarkets = SWAP;
			else useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
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
		LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		LAString data = generateProp[i][1];
		if (key.findString("dfcurvename") == -1) data.toLower();
		irStaticData.setStaticData(key, data);

		LAString tmpProp = generateProp[i][0];
		tmpProp.toLower();
		if (tmpProp == "isswaptenoradjust")
		{
			key = generateProp[i][1];
			isswaptenoradjust = (key.toUpper() == "TRUE");
		}
	}
	
	for(size_t i=0; i<moneyConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv[i][0] + staticDataSuffix;
		key.toLower(); 
		LAString data = moneyConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}
	
	for(size_t i=0; i<liborConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.libor." + liborConv[i][0] + staticDataSuffix;
		key.toLower(); 
		LAString data = liborConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<swapConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0] + staticDataSuffix;
		key.toLower(); 
		LAString data = swapConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<fraConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.fra." + fraConv[i][0] + staticDataSuffix;
		key.toLower(); 
		LAString data = fraConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<futureConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.future." + futureConv[i][0] + staticDataSuffix;
		key.toLower(); 
		LAString data = futureConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	//////////////////modify for aud curve///////////////////////////////////////////////

	//swap adjust mode
	LAString tmpBasisCurveName;
	if (isswaptenoradjust)
	{
		tmpBasisCurveName = THREESIXBASIS;
		LAString adjsuffix_prop;
		LAString adjsuffix_data;
		if (tmpBasisCurveName != STD)
		{
			adjsuffix_prop = "." + tmpBasisCurveName;
			adjsuffix_prop.toLower();
			adjsuffix_data = "_" + tmpBasisCurveName;
			adjsuffix_data.toLower();
		}
		
		for(size_t i=0; i<adjustSwapConv.size(); i++)
		{
			LAString key = tmpCurrency + ".sde.yield.basis." + adjustSwapConv[i][0] + adjsuffix_prop;
			key.toLower(); 
			LAString data = adjustSwapConv[i][1];
			if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
			irStaticData.setStaticData(key, data);
		}
	
		tmpBasisCurveName.toLower();
		LAString basisEntityName = tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName;

		//swapfile
		LAString adjfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + adjsuffix_prop);
		if (adjfile == MLIB_NO_DATA)
		{
			adjfile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap") + adjsuffix_data + LAString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + adjsuffix_prop, adjfile);
		}
		LAString adjstream;
		LAString usegrid = "";
		for(size_t i=0; i<adjustSwapRates.size(); i++)
		{
			adjstream += adjustSwapRates[i][0];
			double adjrate = adjustSwapRates[i][1].getDoubleValue() * 10000.0;
			adjstream += "," + LAString(adjrate) + LF;

			if (adjustSwapRates[i].size() == 3)
			{
				LAString useGridFrag = adjustSwapRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid += adjustSwapRates[i][0] + ":";
				else usegrid += "NONE:";
			}
		}
		std::istringstream *padjstream = new std::istringstream(adjstream.getCString());
		LACoreDataService::setIStringStream(LAMarketData::getNumFileName(adjfile), padjstream);

		irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName);
		int find;
		if (usegrid != "") 
		{
			usegrid = usegrid.subString(0, usegrid.size() - 2);
			while ((find = usegrid.findString("NONE:")) != -1) usegrid.remove(find, 5);
			if ((find = usegrid.findString(":NONE")) != -1) usegrid.remove(find, 5);
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName, usegrid);
		}

		LAString tmpCurveName = tmpBasisCurveName; tmpCurveName.toUpper();
		tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
		{ 
			useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + tmpBasisCurveName;
		}
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + tmpBasisCurveName, MARKETTYPE_BASIS);

		//////////////////modify for aud curve///////////////////////////////////////////////
	}

	LAString usegrid_libor = "",usegrid_swap = "",usegrid_fra3m = "",usegrid_fra6m = "",usegrid_future = "";

	//set Libor Object;
	LAString liborEntityName = tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix;

	LAString liborfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix);
	if (liborfile == MLIB_NO_DATA)
	{
		liborfile = LAString("data/in/") + tmpCurrency + LAString("_yield_libor") + suffix_data + LAString(".csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix, liborfile);
	}
	LAString liborstream;
	for(size_t i=0; i<liborRates.size(); i++)
	{
		liborstream += liborRates[i][0];
		double lrate = liborRates[i][1].getDoubleValue() * 100.0;
		liborstream += "," + LAString(lrate) + LF;

		if (liborRates[i].size() == 3)
		{
			LAString useGridFrag = liborRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_libor += liborRates[i][0] + ":";
			else usegrid_libor += "NONE:";
		}
	}
	std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(liborfile), pliborstream);

	//set Swap Object;
	LAString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	if (swapfile == MLIB_NO_DATA)
	{
		swapfile = LAString("data/in/") + tmpCurrency + LAString("_yield_swap") + suffix_data + LAString(".csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix, swapfile);
	}
	LAString swapstream;
	for(size_t i=0; i<swapRates.size(); i++)
	{
		swapstream += swapRates[i][0];
		double srate = swapRates[i][1].getDoubleValue() * 100.0;
		swapstream += "," + LAString(srate) + LF;

		if (swapRates[i].size() == 3)
		{
			LAString useGridFrag = swapRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_swap += swapRates[i][0] + ":";
			else usegrid_swap += "NONE:";
		}
	}
	std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(swapfile), pswapstream);

	//set fra3m Object;
	bool isFRAUse = false;
	LAString isFRAUse_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	LADataBool tmpAttrB;
	if (isFRAUse_str != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(isFRAUse_str);
		isFRAUse = tmpAttrB.get();
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + staticDataSuffix);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + staticDataSuffix);
	if (isFRAUse)
	{
		LAString fra3mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + staticDataSuffix);
		if (fra3mfile == MLIB_NO_DATA)
		{
			fra3mfile = LAString("data/in/") + tmpCurrency + LAString("_yield_3mfra") + suffix_data + LAString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + staticDataSuffix, fra3mfile);
		}
		LAString fra3mstream;
		for(size_t i=0; i<fra3mRates.size(); i++)
		{
			fra3mstream += fra3mRates[i][0];
			double frarate = fra3mRates[i][1].getDoubleValue() * 100.0;
			fra3mstream += "," + LAString(frarate) + LF;

			if (fra3mRates[i].size() == 3)
			{
				LAString useGridFrag = fra3mRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fra3m += fra3mRates[i][0] + ":";
				else usegrid_fra3m += "NONE:";
			}
		}
		std::istringstream *pfra3mstream = new std::istringstream(fra3mstream.getCString());
		LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fra3mfile), pfra3mstream);		

		//set fra6m Object;
		LAString fra6mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + staticDataSuffix);
		if (fra6mfile == MLIB_NO_DATA)
		{
			fra6mfile = LAString("data/in/") + tmpCurrency + LAString("_yield_6mfra") + suffix_data + LAString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + staticDataSuffix, fra6mfile);
		}
		LAString fra6mstream;
		for(size_t i=0; i<fra6mRates.size(); i++)
		{
			fra6mstream += fra6mRates[i][0];
			double frarate = fra6mRates[i][1].getDoubleValue() * 100.0;
			fra6mstream += "," + LAString(frarate) + LF;

			if (fra6mRates[i].size() == 3)
			{
				LAString useGridFrag = fra6mRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fra6m += fra6mRates[i][0] + ":";
				else usegrid_fra6m += "NONE:";
			}
		}
		std::istringstream *pfra6mstream = new std::istringstream(fra6mstream.getCString());
		LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fra6mfile), pfra6mstream);
	}

	//set future Object;
	bool isFutureUse = false;
	LAString tmpFutureStr = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + staticDataSuffix);
	if (tmpFutureStr != MLIB_NO_DATA)
	{
		LADataBool tmpAttrB;
		tmpAttrB.convertFromString(tmpFutureStr);
		isFutureUse = tmpAttrB.get();
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix);
	if (isFutureUse)
	{
		LAString futureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix);
		if (futureFile == MLIB_NO_DATA)
		{
			futureFile = LAString("data/in/") + tmpCurrency + LAString("_yield_future") + suffix_data + LAString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix, futureFile);
		}
		LACoreDataService::setIStringStream(LAMarketData::getNumFileName(futureFile), createFutureStream(futureRates, usegrid_future));
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

	LAString tmpCurveName = generateCurveName; tmpCurveName.toUpper();
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
	LACalibrateModel *generator = new LACalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);

	//resist generatetime
	LAString CurveIDTool = curveID + TOOL;
	LAString CurveIDManager	= curveID + marketName + MANAGER;

	//curve entities manager (curve, grids, curveinformation entities)
	LAObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new LAObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);

	return;
};

void 
LACurveSetup::
SetUpArbFreeCurve
(LADataInstance* dataInstance,
 const LAString& curveID,
 const LAStringMatrix& generateProp, 
 const LAStringMatrix& moneyConv,
 const LAStringMatrix& liborRates, 
 const LAStringMatrix& liborConv,
 const LAStringMatrix& swapRates, 
 const LAStringMatrix& swapConv,
 const LAStringMatrix& fra3mRates,
 const LAStringMatrix& fra6mRates,
 const LAStringMatrix& fraConv,
 const LAStringMatrix& xccyBasisRates, 
 const LAStringMatrix& xccyBasisConv,
 const LAStringMatrix& threeSixRates,
 const LAStringMatrix& threeSixConv,
 const LAStringMatrix& futureRates, 
 const LAStringMatrix& futureConv,
 const LAStringMatrix& adjustData,
 const LAString& curveNames_3ML,
 const LAString& curveNames_6ML,
 const LAString& curveNames_DF,
 const LAString& curveName_DF2)
{
	LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();

	LAObjectPool &objPool = dataInstance->getObjectPool();
	LAObject* pyld = NULL;	
	
	LAStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	LADate asofdate	= LAMathDateUtilities::getLADate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	LAString currency;
	LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		LAObject& yldEntity = objHolder.get();
		const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		}
		else
		{
			if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = LAString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = LAString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	LAString tmpCurrency = currency; tmpCurrency.toLower();

	LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (useMarkets == MLIB_NO_DATA) useMarkets = "";
	if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),SWAP)) 
	{
		if (useMarkets == "") useMarkets = SWAP;
		else useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT);

	for(size_t i=0; i<generateProp.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		LAString data = generateProp[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}
	
	for(size_t i=0; i<moneyConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv[i][0];
		key.toLower(); 
		LAString data = moneyConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}
	
	for(size_t i=0; i<liborConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.libor." + liborConv[i][0];
		key.toLower(); 
		LAString data = liborConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<swapConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0];
		key.toLower(); 
		LAString data = swapConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<fraConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.fra." + fraConv[i][0];
		key.toLower(); 
		LAString data = fraConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	for(size_t i=0; i<xccyBasisConv.size(); i++)
	{
		LAString key;
		LAString data = xccyBasisConv[i][1];
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
				useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + XCCYBASIS;
			}
		}
		key.toLower(); 
		irStaticData.setStaticData(key, data);
	}
	if (tmpCurrency != "usd") irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + ".xccybasis", MARKETTYPE_BASIS);

	for(size_t i=0; i<threeSixConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.basis." + threeSixConv[i][0] + ".3m6mbasis";
		key.toLower(); 
		LAString data = threeSixConv[i][1];
		if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		irStaticData.setStaticData(key, data);
	}
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + ".3m6mbasis", MARKETTYPE_BASIS);

	for(size_t i=0; i<futureConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.future." + futureConv[i][0];
		key.toLower(); 
		LAString data = futureConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}

	LAString usegrid_libor = "",usegrid_swap = "",usegrid_fra3m = "",usegrid_fra6m = "",usegrid_xccy = "",
		usegrid_3m6m = "",usegrid_future = "";

	//set Libor Object;
	LAString liborfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE);
	if (liborfile == MLIB_NO_DATA)
	{
		liborfile = LAString("data/in/") + tmpCurrency + LAString("_yield_libor.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE, liborfile);
	}
	LAString liborstream;
	for(size_t i=0; i<liborRates.size(); i++)
	{
		liborstream += liborRates[i][0];
		double lrate = liborRates[i][1].getDoubleValue() * 100.0;
		liborstream += "," + LAString(lrate) + LF;

		if (liborRates[i].size() == 3)
		{
			LAString useGridFrag = liborRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_libor += liborRates[i][0] + ":";
			else usegrid_libor += "NONE:";
		}
	}
	std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(liborfile), pliborstream);

	//set Swap Object;
	LAString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE);
	if (swapfile == MLIB_NO_DATA)
	{
		swapfile = LAString("data/in/") + tmpCurrency + LAString("_yield_swap.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE, swapfile);
	}
	LAString swapstream;
	for(size_t i=0; i<swapRates.size(); i++)
	{
		swapstream += swapRates[i][0];
		double srate = swapRates[i][1].getDoubleValue() * 100.0;
		swapstream += "," + LAString(srate) + LF;

		if (swapRates[i].size() == 3)
		{
			LAString useGridFrag = swapRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_swap += swapRates[i][0] + ":";
			else usegrid_swap += "NONE:";
		}
	}
	std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(swapfile), pswapstream);

	//set fra3m Object;
	bool isFRAUse = false;
	LAString isFRAUse_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE);
	LADataBool tmpAttrB;
	if (isFRAUse_str != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(isFRAUse_str);
		isFRAUse = tmpAttrB.get();
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE);
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE);
	if (isFRAUse)
	{
		LAString fra3mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE);
		if (fra3mfile == MLIB_NO_DATA)
		{
			fra3mfile = LAString("data/in/") + tmpCurrency + LAString("_yield_3mfra.csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE, fra3mfile);
		}
		LAString fra3mstream;
		for(size_t i=0; i<fra3mRates.size(); i++)
		{
			fra3mstream += fra3mRates[i][0];
			double frarate = fra3mRates[i][1].getDoubleValue() * 100.0;
			fra3mstream += "," + LAString(frarate) + LF;

			if (fra3mRates[i].size() == 3)
			{
				LAString useGridFrag = fra3mRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fra3m += fra3mRates[i][0] + ":";
				else usegrid_fra3m += "NONE:";
			}
		}
		std::istringstream *pfra3mstream = new std::istringstream(fra3mstream.getCString());
		LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fra3mfile), pfra3mstream);		

		//set fra6m Object;
		LAString fra6mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE);
		if (fra6mfile == MLIB_NO_DATA)
		{
			fra6mfile = LAString("data/in/") + tmpCurrency + LAString("_yield_6mfra.csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE, fra6mfile);
		}
		LAString fra6mstream;
		for(size_t i=0; i<fra6mRates.size(); i++)
		{
			fra6mstream += fra6mRates[i][0];
			double frarate = fra6mRates[i][1].getDoubleValue() * 100.0;
			fra6mstream += "," + LAString(frarate) + LF;

			if (fra6mRates[i].size() == 3)
			{
				LAString useGridFrag = fra6mRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fra6m += fra6mRates[i][0] + ":";
				else usegrid_fra6m += "NONE:";
			}
		}
		std::istringstream *pfra6mstream = new std::istringstream(fra6mstream.getCString());
		LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fra6mfile), pfra6mstream);
	}

	///Xccy Basis Rate
	LAString xccyBasisFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "xccybasis");
	if (xccyBasisFile == MLIB_NO_DATA)
	{
		xccyBasisFile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap_xccybasis.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "xccybasis", xccyBasisFile);
	}
	LAString xccyBasisStream;
	for(size_t i=0; i<xccyBasisRates.size(); i++)
	{
		xccyBasisStream += xccyBasisRates[i][0];
		double brate = xccyBasisRates[i][1].getDoubleValue() * 10000.0;
		xccyBasisStream += "," + LAString(brate) + LF;

		if (xccyBasisRates[i].size() == 3)
		{
			LAString useGridFrag = xccyBasisRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_xccy += xccyBasisRates[i][0] + ":";
			else usegrid_xccy += "NONE:";
		}
	}
	std::istringstream *pXccyBasisStream = new std::istringstream(xccyBasisStream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(xccyBasisFile), pXccyBasisStream);

	///36 Basis Rate
	LAString threeSixFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "3m6mbasis");
	if (threeSixFile == MLIB_NO_DATA)
	{
		threeSixFile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap_3m6mbasis.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "3m6mbasis", threeSixFile);
	}
	LAString threeSixStream;
	for(size_t i=0; i<threeSixRates.size(); i++)
	{
		threeSixStream += threeSixRates[i][0];
		double brate = threeSixRates[i][1].getDoubleValue() * 10000.0;
		threeSixStream += "," + LAString(brate) + LF;

		if (threeSixRates[i].size() == 3)
		{
			LAString useGridFrag = threeSixRates[i][2]; upper(useGridFrag);
			if (useGridFrag == "TRUE") usegrid_3m6m += threeSixRates[i][0] + ":";
			else usegrid_3m6m += "NONE:";
		}
	}
	std::istringstream *pThreeSixStream = new std::istringstream(threeSixStream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(threeSixFile), pThreeSixStream);

	if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),THREESIXBASIS) && threeSixRates.size())
	{
		useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + THREESIXBASIS;
	}

	//set future Object;
	bool isFutureUse = false;
	LAString tmpFutureStr = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE);
	if (tmpFutureStr != MLIB_NO_DATA)
	{
		tmpAttrB.convertFromString(tmpFutureStr);
		isFutureUse = tmpAttrB.get();
	}
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE);
	if (isFutureUse)
	{
		LAString futureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE);
		if (futureFile == MLIB_NO_DATA)
		{
			futureFile = LAString("data/in/") + tmpCurrency + LAString("_yield_future") + LAString(".csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE, futureFile);
		}
		LACoreDataService::setIStringStream(LAMarketData::getNumFileName(futureFile), createFutureStream(futureRates, usegrid_future));
	}
	
	//set Adjust Data Object;
	LAString adjustValueFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + "xccybasis");
	if (adjustValueFile == MLIB_NO_DATA)
	{
		adjustValueFile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisadjust.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + "xccybasis", adjustValueFile);
	}
	LAString adjustValueStream;
	for(size_t i=0; i<adjustData.size(); i++)
	{
		adjustValueStream += adjustData[i][0];
		adjustValueStream += "," + adjustData[i][1] + LF;	
	}
	std::istringstream *pAdjustValueStream = new std::istringstream(adjustValueStream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(adjustValueFile), pAdjustValueStream);


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

	LACalibrateModel *generator = new LACalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);

	//resist generatetime
	LAString	CurveIDTool			= curveID + TOOL;
	LAString	CurveIDManager		= curveID + CURVETYPE_ARBFREE + MANAGER;

	//curve entities manager (curve, grids, curveinformation entities)
	LAObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new LAObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);

	return;
};

void 
LACurveSetup::
setUpOISCurve
(LADataInstance* dataInstance,
const LAString& curveID,
const LAString& marketName,
const LAStringMatrix& generateProp, 
const LAStringMatrix& oisRates, 
const LAStringMatrix& oisConv,
const LAString& curveNames,
const LAStringMatrix& histRates,
const LAStringMatrix& lobasisRates, 
const LAStringMatrix& lobasisConv, 
const LAStringMatrix& swapRates, 
const LAStringMatrix& swapConv
)
{
	LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();

	LAObjectPool &objPool = dataInstance->getObjectPool();
	LAObject* pyld = NULL;
	
	LAStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	LADate asofdate	= LAMathDateUtilities::getLADate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	LAString currency;
	LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		LAObject& yldEntity = objHolder.get();
		const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		}
		else
		{
			if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = LAString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = LAString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	LAString tmpCurrency = currency; tmpCurrency.toLower();

	if (marketName == "" || marketName == STD) 
		throw LACoreInvalidData("Do not use STD or blank for ois curve name!",__FILE__,__LINE__);
	LAString staticDataSuffix;
	LAString suffix_data;
	staticDataSuffix = "." + marketName;
	staticDataSuffix.toLower();
	LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	if (useMarkets == MLIB_NO_DATA) useMarkets = "";
	LAString tmpCurveName = marketName; tmpCurveName.toUpper();
	if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
	{
		if (useMarkets == "") useMarkets = marketName;
		else useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + marketName;
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
		LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		LAString data = generateProp[i][1];
		if (key.findString("dfcurvename") == -1) data.toLower();
		irStaticData.setStaticData(key, data);
	}
	
	for(size_t i=0; i<oisConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.ois." + oisConv[i][0] + staticDataSuffix;
		key.toLower(); 
		LAString data = oisConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}	

	LAString oisFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + staticDataSuffix);
	if (oisFile == MLIB_NO_DATA)
	{
		oisFile = LAString("data/in/") + tmpCurrency + LAString("_yield_ois_oiscurve.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + staticDataSuffix, oisFile);
	}

	LAString fedFundFutureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + staticDataSuffix);
	if (fedFundFutureFile == MLIB_NO_DATA)
	{
		fedFundFutureFile = LAString("data/in/") + tmpCurrency + LAString("_yield_fffuture.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + staticDataSuffix, fedFundFutureFile);
	}

	LAString histFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + staticDataSuffix);
	if (histFile == MLIB_NO_DATA)
	{
		histFile = LAString("data/in/") + tmpCurrency + LAString("_yield_historical_ois_oiscurve.csv");
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + staticDataSuffix, histFile);
	}

	LAString oisStream;
	LAString fedFundFutureStream;
	LAString usegrid_ois = "";
	LAString usegrid_fffuture = "";
	for(size_t i=0; i<oisRates.size(); i++)
	{
		if (oisRates[i][0].findString("FF") != -1)
		{
			fedFundFutureStream += oisRates[i][0];

			if (oisRates[i].size() != 3 && oisRates[i].size() != 5)
				throw LACoreInvalidData("FF input size error",__FILE__,__LINE__);

			//in case of FF, quoted value is price
			double oisRate = oisRates[i][1].getDoubleValue();
			fedFundFutureStream += "," + LAString(oisRate);

			if (oisRates[i].size() == 5)
			{
				const LADate& startdate = LAMathDateUtilities::getLADate(oisRates[i][2]);
				LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				fedFundFutureStream += "," + startdate_str;
				const LADate& enddate = LAMathDateUtilities::getLADate(oisRates[i][3]);
				LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				fedFundFutureStream += "," + enddate_str;
			}

			fedFundFutureStream += LF;
			
			if (oisRates[i].size() == 5)
			{
				LAString useGridFrag = oisRates[i][4]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_fffuture += oisRates[i][0] + ":";
				else usegrid_fffuture += "NONE:";
			}
			else if (oisRates[i].size() == 3)
			{
				LAString useGridFrag = oisRates[i][2]; upper(useGridFrag);
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
					throw LACoreInvalidData("short term market needs StartDate and EndDate",__FILE__,__LINE__);

				double oisRate = oisRates[i][1].getDoubleValue() * 100.0;
				oisStream += "," + LAString(oisRate);

				const LADate& startdate = LAMathDateUtilities::getLADate(oisRates[i][2]);
				LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				oisStream += "," + startdate_str;
				const LADate& enddate = LAMathDateUtilities::getLADate(oisRates[i][3]);
				LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				oisStream += "," + enddate_str;
			}
			else //normal case
			{
				double oisRate = oisRates[i][1].getDoubleValue() * 100.0;
				oisStream += "," + LAString(oisRate);
			}
			
			oisStream += LF;
			
			if (oisRates[i].size() == 5)
			{
				LAString useGridFrag = oisRates[i][4]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_ois += oisRates[i][0] + ":";
				else usegrid_ois += "NONE:";
			}
			else if (oisRates[i].size() == 3)
			{
				LAString useGridFrag = oisRates[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid_ois += oisRates[i][0] + ":";
				else usegrid_ois += "NONE:";
			}
		}
	}
	std::istringstream *pOISStream = new std::istringstream(oisStream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(oisFile), pOISStream);
	std::istringstream *pFedFundFutureStream = new std::istringstream(fedFundFutureStream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fedFundFutureFile), pFedFundFutureStream);


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

	LAString histStream;
	for(size_t i=0; i<histRates.size(); i++)
	{
		const LADate& histdate = LAMathDateUtilities::getLADate(histRates[i][0]);
		LAString histdate_str = histdate.stringWithFormat("YYYYMMDD");
		histStream += histdate_str;
		double histRate = histRates[i][1].getDoubleValue() * 100.0;
		histStream += "," + LAString(histRate);
		histStream += LF;
	}
	std::istringstream *pHISTStream = new std::istringstream(histStream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(histFile), pHISTStream);

	LAString lobasisname = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + staticDataSuffix);
	if (lobasisname == MLIB_NO_DATA)
	{
		lobasisname = LOBASIS;
		LAString key = LAString(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + staticDataSuffix).toLower();
		irStaticData.setStaticData(key, lobasisname);
	}
	LAString suffix_lob_prop("." + lobasisname);
	LAString suffix_lob_data("_" + lobasisname);
	for(size_t i=0; i<lobasisConv.size(); i++)
	{
		LAString key(tmpCurrency + ".sde.yield.basis." + lobasisConv[i][0] + suffix_lob_prop);
		key.toLower(); 
		LAString data = lobasisConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}
	LAString lobasisfile = irStaticData.getStaticData(LAString(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + suffix_lob_prop).toLower());
	if (lobasisfile == MLIB_NO_DATA)
	{
		lobasisfile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap") + suffix_lob_data + LAString(".csv");
		lobasisfile.toLower();
		LAString key(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + suffix_lob_prop);
		key.toLower();
		irStaticData.setStaticData(key, lobasisfile);
	}
	LAString lobasisstream;
	for(size_t i=0; i<lobasisRates.size(); i++)
	{
		lobasisstream += lobasisRates[i][0];
		const double lobrate = lobasisRates[i][1].getDoubleValue() * 10000.0;
		lobasisstream += "," + LAString(lobrate) + LF;
	}
	std::istringstream *plobasisstream = new std::istringstream(lobasisstream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(lobasisfile), plobasisstream);

	for(size_t i=0; i<swapConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0] + staticDataSuffix;
		key.toLower(); 
		LAString data = swapConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key, data);
	}	
	LAString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	if (swapfile == MLIB_NO_DATA)
	{
		swapfile = LAString("data/in/") + tmpCurrency + LAString("_yield_swap") + suffix_data + LAString(".csv");
		swapfile.toLower();
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix, swapfile);
	}
	LAString swapstream;
	for(size_t i=0; i<swapRates.size(); i++)
	{
		swapstream += swapRates[i][0];
		double srate = swapRates[i][1].getDoubleValue() * 100.0;
		swapstream += "," + LAString(srate) + LF;
	}
	std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(swapfile), pswapstream);
	

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + staticDataSuffix, MARKETTYPE_SWAP);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	LACalibrateModel *generator = new LACalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);

	//resist generatetime
	LAString	CurveIDTool			= curveID + TOOL;
	LAString	CurveIDManager		= curveID + marketName + MANAGER;
	//curve entities manager (curve, grids, curveinformation entities)
	LAObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new LAObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);

	return;
};

void
LACurveSetup::
setUpFloater
(LADataInstance* dataInstance,
 const LAString& curveID,
 const LAString& discountCurveName,
 const LAString& forecastCurveName,
 const LAStringMatrix& generateProp, 
 const LAStringMatrix& basisMkt, 
 const LAStringMatrix& basisConv,
 const LAStringMatrix& swapConv,
 const LAStringMatrix& adjustData)
{
	LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();

	LAObjectPool &objPool = dataInstance->getObjectPool();

	LAStringMatrix tmpInfo = generateProp;
	upper(tmpInfo);
	LAString currency;
	LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		LAObject& yldEntity = objHolder.get();
		const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		if(dh->isDefined() && !dh->isNull())
		{
			currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		}
		else
		{
			if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			{
				currency = LAString("DUMMY");
			}
			else
			{
				currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			} 		
		}
	}
	else
	{
		if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		{
			currency = LAString("DUMMY");
		}
		else
		{
			currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		} 	
	}
	LAString tmpCurrency = currency; tmpCurrency.toLower();
	irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);

	//insert property
	LAStringMatrix tmpProp(basisConv.size() + swapConv.size() + generateProp.size(), LAStringVector(2));
	LAString tmpBasisCurveName = XCCYBASIS; tmpBasisCurveName.toLower();
	for(size_t i=0; i<basisConv.size(); i++)
	{		
		LAString key = tmpCurrency + ".sde.yield.basis." + basisConv[i][0] + "." + tmpBasisCurveName;
		key.toLower();
		LAString data = basisConv[i][1];
		if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		irStaticData.setStaticData(key,data);
	}

	for(size_t i=0; i<swapConv.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0];
		key.toLower(); 
		LAString data = swapConv[i][1];
		data.toLower();
		irStaticData.setStaticData(key,data);
	}

	for(size_t i=0; i<generateProp.size(); i++)
	{
		LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		key.toLower();
		LAString data = generateProp[i][1];
		data.toUpper();
		irStaticData.setStaticData(key,data);
	}

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT, discountCurveName);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FLOATER_FORECAST, forecastCurveName);

	if (!basisMkt.empty())
	{
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME, XCCYBASIS);

		LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
		LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (useMarkets == MLIB_NO_DATA) useMarkets = "";
		if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),XCCYBASIS)) 
		{
			if (useMarkets == "") useMarkets = XCCYBASIS;
			else useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + XCCYBASIS;
		}
		irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
		
		LAString tmpBasisCurveName = XCCYBASIS;
		tmpBasisCurveName.toLower();

		LAString basisfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName);
		if (basisfile == MLIB_NO_DATA)
		{
			basisfile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap_xccybasis.csv");
			irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName, basisfile);
		}
		LAString basisstream;
		LAString usegrid = "";
		for(size_t i=0; i<basisMkt.size(); i++)
		{
			basisstream += basisMkt[i][0];
			double brate = basisMkt[i][1].getDoubleValue() * 10000.0;
			basisstream += "," + LAString(brate) + LF;

			if (basisMkt[i].size() == 3)
			{
				LAString useGridFrag = basisMkt[i][2]; upper(useGridFrag);
				if (useGridFrag == "TRUE") usegrid += basisMkt[i][0] + ":";
				else usegrid += "NONE:";
			}
		}
		std::istringstream *pbasisstream = new std::istringstream(basisstream.getCString());
		LACoreDataService::setIStringStream(LAMarketData::getNumFileName(basisfile), pbasisstream);

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
			LAString adjustValueFile = irStaticData.getStaticData(tmpCurrency + 
				STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + tmpBasisCurveName);
			if (adjustValueFile == MLIB_NO_DATA)
			{
				adjustValueFile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisadjust.csv");
				irStaticData.setStaticData(tmpCurrency + 
					STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + tmpBasisCurveName, adjustValueFile);
			}
			LAString adjustValueStream;
			for(size_t i=0; i<adjustData.size(); i++)
			{
				adjustValueStream += adjustData[i][0];
				adjustValueStream += "," + adjustData[i][1] + LF;	
			}
			std::istringstream *pAdjustValueStream = new std::istringstream(adjustValueStream.getCString());
			LACoreDataService::setIStringStream(LAMarketData::getNumFileName(adjustValueFile), pAdjustValueStream);
		}
	}

	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, CURVETYPE_FLOATER);
	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS, CURVETYPE_FLOATER);
	LACalibrateModel *generator = new LACalibrateModelIRVanilla(currency);
	generator->loadModelDataAndCalibrate(currency, *dataInstance, true, false);

	delete generator;

	//ylddata
	LAString	CurveIDTool			= curveID + TOOL;
	LAString	CurveIDManager		= curveID + CURVETYPE_FLOATER+ MANAGER;
	//curve entities manager (curve, grids, curveinformation entities)
	LAObject* mae = NULL;
	if(!objPool.getObject(CurveIDManager).isDefined())
	{	
		mae = new LAObject;
		objPool.set(CurveIDManager,mae);
	}
	else
	{
		objPool.getObject(CurveIDManager).get().clear();
		mae	= &objPool.getObject(CurveIDManager).get();
	}
	mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);

}

// 
/*!
    @brief setup for irserver

	create property file stream

*/
void
LACurveSetup::
setUpForIRServer()
{
	LACoreDataService::initialize();
	LACoreDataService::setContext(CONTEXT_KEY_ISPRICER, "TRUE");
	LACoreDataService::setContext(CONTEXT_KEY_ISEXCELREQUEST, "TRUE");
	LACoreDataService::setContext(ARG_KEY_FILENUM, "");

	// create property stream
	std::istringstream *pPropSf = new std::istringstream();
	std::istringstream *pRPropSf = new std::istringstream();
	std::istringstream *pGpropSf = new std::istringstream();
	std::istringstream *pCprofSf = new std::istringstream();
	std::istringstream *pXprofSf = new std::istringstream();
	std::istringstream *pCreditPropSf = new std::istringstream();
	// set prop
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName("ir.properties"), pPropSf);
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName("grid.properties"), pGpropSf);
	// set risk prop
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	staticData.setStaticData(KEY_RISK_SCENARIO_FILE, "risk.properties"); 
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName("risk.properties"), pRPropSf);
	// set max leg num
	staticData.setStaticData(KEY_DEAL_LEG_MAXNUM, "2");
	// set polynomial dimension
	staticData.setStaticData(KEY_SIMULATION_LSMC_BASEFUNCDIM, "2");
	//set calib prop
	//this is for avoiding LACoreDataService::getContext(ARG_KEY_NOCALIBTHREAD) == MLIB_NO_DATA
	LACoreDataService::setContext(ARG_KEY_NOCALIBTHREAD, "tmp");
	staticData.setStaticData(KEY_CALIB_SCENARIO_FILE, "calib.properties"); 
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName("calib.properties"), pCprofSf);
	staticData.setStaticData(KEY_SDE_CALIB_MODEL, "hw:ptberg");
	staticData.setStaticData(KEY_SDE_PV_CALIB_TARGET_CURRENCY, "ALL");
	//set xva prop
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName("xva.properties"), pXprofSf);
	//set credit prop
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName("credit.properties"), pCreditPropSf);

	//set isrealcalibmode //default is false
	LAStaticData &riskStaticData = LACoreDataService::getStaticDataManager().getRiskStaticData();
	riskStaticData.setStaticData(RISK_KEY_CALIB_ISREALCALIB,"false");
	//riskStaticData.setStaticData(RISK_KEY_CALIB_ISREALCALIB,"true");
	
	//this is for avoiding !LAMarketData::isCalibrateEnd(dIRModelName, objPool) ...
	LAStaticData &cprop = LACoreDataService::getStaticDataManager().getCalibStaticData();
	cprop.setStaticData(KEY_CALIB_WAIT_TIME, "10");
	//seriarize set
	cprop.setStaticData(KEY_CALIB_SERIALIZE_STATUS, "normal");
	
	//testisadjusdf
	LAString tmpstr = LAString("jpy"); 
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
	LAString tmpfxstr = LAString("jpy/usd");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");

	tmpfxstr = LAString("jpy/aud");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");

	tmpfxstr = LAString("jpy/eur");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");

	tmpfxstr = LAString("jpy/gbp");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");
}

void 
LACurveSetup::
setUpDefaultIRStaticData(LADataInstance& dataInstance)
{
	const LAString* filepath = LACurveProperties::ir_prop_path();
    if(filepath==NULL) return;

	std::ifstream fin;
	fin.open(filepath->getCString());
	if (!fin){
		std::stringstream sst;
		sst << "cannot open ir properties file" << std::endl 
			<< filepath
			;
		throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
	}

    LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();
	std::string line;
	size_t line_num = 0;
	while (getline(fin, line)){
		line_num++;
		const char *c_line = line.c_str();
		LAString tmpstr(c_line);
		LAStringVector tmp = tmpstr.toToken('=');
		if (tmp.size() != 2){
			std::stringstream sst;
			sst << "ir proerties file format is invalid" << std::endl
				<< "file : " << filepath << std::endl
				<< "line : " << line_num << std::endl
				<< "contents : " << line
				;
			throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
		}

		irStaticData.setStaticData(tmp[0],tmp[1]);
	}
	fin.close();
}

void 
LACurveSetup::
setUpDefaultIRStaticData(LADataInstance& dataInstance, LAString filepath)
{
    LACurveProperties::set_ir_prop_path(filepath);
    setUpDefaultIRStaticData(dataInstance);
}

void 
LACurveSetup::
setUpDefaultCalibStaticData(LADataInstance& dataInstance)
{
	const LAString* filepath = LACurveProperties::calib_prop_path();
    if(filepath==NULL) return;

	std::ifstream fin;
	fin.open(filepath->getCString());
	if (!fin){
		std::stringstream sst;
		sst << "cannot open calib properties file" << std::endl 
			<< filepath
			;
		throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
	}

    LAStaticData &calibprop = LACoreDataService::getStaticDataManager().getCalibStaticData();
	std::string line;
	size_t line_num = 0;
	while (getline(fin, line)){
		line_num++;
		const char *c_line = line.c_str();
		LAString tmpstr(c_line);
		LAStringVector tmp = tmpstr.toToken('=');
		if (tmp.size() != 2){
			std::stringstream sst;
			sst << "calib proerties file format is invalid" << std::endl
				<< "file : " << filepath << std::endl
				<< "line : " << line_num << std::endl
				<< "contents : " << line
				;
			throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
		}

		calibprop.setStaticData(tmp[0],tmp[1]);
	}
	fin.close();
}

void 
LACurveSetup::
setUpDefaultCalibStaticData(LADataInstance& dataInstance, LAString filepath)
{
    LACurveProperties::set_calib_prop_path(filepath);
    setUpDefaultCalibStaticData(dataInstance);
}
/*!
    @brief setup for irserver

	create property file stream

*/
void
LACurveSetup
::resetService()
{
	LACoreDataService::finalize();
	setUpForIRServer();
}

LAString
LACurveSetup::
getIRStaticData(const LAString& key)
{
	LACoreDataService::setContext(ARG_KEY_FILENUM, "");
	LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();
	return irStaticData.getStaticData(key);
}

std::istringstream* 
LACurveSetup::createFutureStream(const LAStringMatrix& future_rates, LAString& usegrid_future)
{
    LAString futureStream;
    for(size_t i=0; i<future_rates.size(); i++)
    {
        if (future_rates[i].size() <= 4)
        {
            if (future_rates[i][0].size() < 3)
                throw LACoreInvalidData("future size error",__FILE__,__LINE__);

            futureStream += future_rates[i][0];
            double futureRate = future_rates[i][1].getDoubleValue();
            futureStream += "," + LAString(futureRate);
            double futureVol = future_rates[i][2].getDoubleValue();
            futureStream += "," + LAString(futureVol) + LF;

            if (future_rates[i].size() == 4)
            {
                LAString useGridFrag = future_rates[i][3]; upper(useGridFrag);
                if (useGridFrag == "TRUE") usegrid_future += future_rates[i][0] + ":";
            }
        }
        else if (future_rates[i].size() <= 6)
        {
            futureStream += future_rates[i][0];
            const LADate& startdate = LAMathDateUtilities::getLADate(future_rates[i][1]);
            LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
            futureStream += "," + startdate_str;
            const LADate& enddate = LAMathDateUtilities::getLADate(future_rates[i][2]);
            LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
            futureStream += "," + enddate_str;
            double futureRate = future_rates[i][3].getDoubleValue();
            futureStream += "," + LAString(futureRate);
            double futureVol = future_rates[i][4].getDoubleValue();
            futureStream += "," + LAString(futureVol) + LF;

            if (future_rates[i].size() == 6)
            {
                LAString useGridFrag = future_rates[i][5]; upper(useGridFrag);
                if (useGridFrag == "TRUE") usegrid_future += future_rates[i][0] + ":";
				else usegrid_future += "NONE:";
            }
        }
    }
    return new std::istringstream(futureStream.getCString());
}

double
LACurveSetup::compound(LADataInstance* dataInstance,
                           const LAString& curveID,
                           const LAString& forecastCurveName,
                           const LADate& start_date,
                           const LADate& end_date,
                           const double spread,
                           const LAString& frequency_,
                           const bool is_start_roll,
                           const LAString& roll_convention_,
                           const LAString& calendar_,
                           const LAString& sliding_rule_,
                           const LAString& day_count_,
                           const LAString& interpolation_,
                           const LAString& compound_type_,
                           const LADate* first_odd,
                           const LADate* last_odd)
{
    LAString frequency = frequency_; frequency.toUpper();
    LAString roll_convention = roll_convention_; roll_convention.toUpper();
    LAString calendar = calendar_; calendar.toUpper();
    LAString sliding_rule = sliding_rule_; sliding_rule.toUpper();
    LAString day_count = day_count_; day_count.toUpper();
    LAString interpolation = interpolation_; interpolation.toUpper();
    LAString compound_type_str = compound_type_; compound_type_str.toUpper();
    
    LACurveSetup::COMPOUND_TYPE compound_type;
    if(compound_type_str=="NORMAL") compound_type = LACurveSetup::COMPOUND_NORMAL;
    else if(compound_type_str=="FLAT") compound_type = LACurveSetup::COMPOUND_FLAT;
    else if(compound_type_str=="SIMPLE") compound_type = LACurveSetup::COMPOUND_SIMPLE;
    else if(compound_type_str=="AVERAGE") compound_type = LACurveSetup::AVERAGE;
    else{
        LAString msg;
        msg += "Unknown compound type:";
        msg += compound_type_;
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }




    LAPriceDataCalendar data_cal; data_cal.convertFromString(calendar);
    LAPriceDataSlidingRule data_sr; data_sr.convertFromString(sliding_rule);


    DateVector payment_dates;
    LAMathDateCalculations::generateSchedule(
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



    const LAString freq = SIMPLE;

	const LAMathYieldCurve& yc = LAMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveID);
	const LAObject& yieldData = yc.getYieldData().get().get();
	LAString suffix;
	if (forecastCurveName != STD)
	{
		suffix = "_" + forecastCurveName;
	}
	const LADataHolder* dh = &(yieldData.getData(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, NOCHECK));
	const bool is_fwd_inter = dh->isDefined() && !dh->isNull();

    DoubleArray rates = LAMathCurveFuncUtility::getMultiForwardRate(start_dates,
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


    LAPriceDataDayCount dc; dc.convertFromString(day_count);
    DoubleVector x(3*n);
    copy(rates.begin(), rates.end(), x.begin());
    for(size_t i = 0; i < n; i++){
        x[n+i]     = dc.getTerm(start_dates[i], end_dates[i]);
        x[n*2 + i] = spread;
    }
	std::shared_ptr<LACompoundMethod> method;
	// if compound type is simple or average, we need to add whole term to final element
	// this operation is al so implemented in LAPricePayOffToolCompound::calculateCouponCashflow().
    switch(compound_type){
        case COMPOUND_NORMAL:
            {
                method.reset(new LACompoundMethod7());
                break;
            }
        case COMPOUND_FLAT:
            {
                method.reset(new LACompoundMethod8());
                break;
            }
        case COMPOUND_SIMPLE:
            {
                method.reset(new LACompoundMethod9());
                x.push_back(dc.getTerm(start_date, end_date));
                break;
            }
        case AVERAGE:
            {
                method.reset(new LACompoundMethod10());
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
LACurveSetup::setStaticDataValue(LAStaticData &staticData, const LAString &key, const LAString &val, const bool is_override)
{
	if (is_override)
	{
		staticData.setStaticData(key , val);
	}
	else
	{
		const LAString orig = staticData.getStaticData(key);
		if (orig == MLIB_NO_DATA)
		{
			staticData.setStaticData(key , val);
		}
	}
}

void
LACurveSetup::SetUpFundingSpread(LADataInstance* dataInstance, const LAStringMatrix &fundingSpread)
{
	// save in string stream
	LAString stream;
	for (unsigned int i = 0; i < fundingSpread.size(); ++i)
	{
		stream += fundingSpread[i][0];
		stream += "," + fundingSpread[i][1] + LF;
	}
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString fdspdfile = staticData.getStaticData(KEY_FUNDINGSPREAD_FILE);
	std::istringstream *pstream = new std::istringstream(stream.getCString());
	LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fdspdfile), pstream);
}

void
LACurveSetup::CalcMeanAndCovariance(const DoubleMatrix& data, DoubleArray& mean, DoubleMatrix& covar)
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
LACurveSetup::SetUpPCA(LADataInstance* dataInstance, const DoubleMatrix& corr, const size_t no_factors, const LAString& id)
{

	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAMathCorrelation* cor_obj = NULL;
	LAString name = PREFIX_COR + id;
	if(!objPool.getObject(name).isDefined())
	{
		cor_obj = new LAMathCorrelation(dataInstance);
		objPool.set(name, cor_obj);
	}
	else
	{
		cor_obj = &dynamic_cast<LAMathCorrelation &>(objPool.getObject(name).get());
		cor_obj->reset();
	}

	if (no_factors > corr.size())
		throw LACoreInvalidData("factornumber is bigger than correlation size", __FILE__, __LINE__);

	DoubleArray dummy_tgrid(corr.size());
	for(size_t i = 0; i < corr.size(); i++)
	{
		dummy_tgrid[i] = static_cast<double>(i + 1); 
	}
	cor_obj->getTGrid().set(dummy_tgrid);

	cor_obj->getIsMultiVol().set(false);

	dynamic_cast<LADataInt&>(cor_obj->getData(IR_CALIBRATION_DATA_FACTORNUM_AFTER, ISDEFINED).get()).set(no_factors);
	dynamic_cast<LADataBool&>(cor_obj->getData(IR_CALIBRATION_DATA_ISOPTIM, ISDEFINED).get()).set(false);

	cor_obj->setCorrelation(corr);
	cor_obj->calcFactorLoading();
}

DoubleMatrix
LACurveSetup::GetPCAResult(LADataInstance* dataInstance, const LAString& type, const LAString& id)
{
	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAString name = PREFIX_COR + id;
	LAMathCorrelation cor_obj = dynamic_cast<LAMathCorrelation& >(objPool.getObject(name, ENCHKTYPE_ISDEFINED).get());

	DoubleMatrix ret;
	if(type == LAString("EIGEN_VECTORS"))
	{
		ret = dynamic_cast<LADataDoubleMatrix& >(cor_obj.getData(IR_CALIBRATION_DATA_EIGENVECTORS, ISNOTNULL).get()).get();
	}
	else if(type == LAString("EIGEN_VALUES"))
	{
		DoubleArray tmp = dynamic_cast<LADataDoubles&>(cor_obj.getData(IR_CALIBRATION_DATA_EIGENVALUES, ISNOTNULL).get()).get();
		ret.push_back(tmp);
	}
	else if (type == LAString("POV"))
	{
		DoubleArray tmp = dynamic_cast<LADataDoubles&>(cor_obj.getData(IR_CALIBRATION_DATA_EIGENVALUES, ISNOTNULL).get()).get();
		double sum_tmp = 0.;
		for(unsigned int i = 0; i < tmp.size(); i++)
			sum_tmp += tmp[i];
		for(unsigned int i = 0; i < tmp.size(); i++)
			tmp[i] /= sum_tmp;
		ret.push_back(tmp);
	}
	else
	{
		LAString msg = LAString("Unknown result type: ") + type;
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	return ret;

}

