
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLMultiSwapPricer.h"
#include "AQLCalibrateModelIRVanilla.h"
#include "AQLDefinitions.h"
#include "AQLStaticData.h"
#include "AQLFunctionUtilities.h"
#include "AQLMathDateUtilities.h"
#include <cmath>
#include <map>

#define CALIBRATION_DATA_CURVEID					"CurveID"

#include "AQLMathValuableEntity.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLPriceCashFlowGenerator.h"
#include "AQLLinearFunc.h"
#include "AQLPricePayOff.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathIndexEntity.h"
#include <algorithm>
#include "AQLObject.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLLinearInterpolation.h"
#include "AQLDataMultiReference.h"
#include "AQLDataReference.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLDataProcedure.h"
#include "AQLString.h"
#include "AQLDataMatrix.h"
#include "AQLPriceDataManager.h"
#include "AQLPriceConvergenceValue.h"

#include "AQLPricePortfolioValue.h"
#include "AQLCalibrationUtilities.h"
#include "AQLCurveSetup.h"
#include "AQLCompoundingFunc.h"

#include "AQLMarketData.h"

using namespace std;

DoubleArray
AQLMultiSwapPricer::convergentPlainVanillaTrade(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade)
{
	AQLObjectPool &objPool = dataInstance->getObjectPool();
	AQLObjectHolder objHolder;
	AQLDataHolder* dh;
	//important clasee "lineparam1"
	AQLObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	AQLString asofdateInput = dynamic_cast<AQLDataString &>(eline.getData("CALC_ASOFDATE", ISNOTNULL).get());
	AQLDate basedate(asofdateInput.getCString(),"YYYYMMDD");


	///////////trade infors/////////////////////////
	//at first no calcvec
	AQLString nocalcswapstr = maptrade["NoCalcSwapTrades"];
	AQLStringVector tradeNoCalcInputVec;
	if (nocalcswapstr != "")
		throw AQLCoreInvalidData("No calc mode",__FILE__,__LINE__);
	//if (nocalcswapstr != "")
	//	tradeNoCalcInputVec = nocalcswapstr.toToken(':');
	
	//get tradevec;
	AQLString calcswapstr = maptrade["CalcSwapTrades"];
	AQLStringVector tradeInputVec;
	if (calcswapstr != "")
		tradeInputVec = calcswapstr.toToken(':');

	AQLStringVector tradeVec;
	//if (tradeNoCalcInputVec.size() == 1)
	//	tradeVec = tradeNoCalcInputVec;
	//else if (tradeInputVec.size() == 1)
	//	tradeVec = tradeInputVec;
	//else 
	//	throw AQLCoreInvalidData("convergent set error",__FILE__,__LINE__);
	
	if (tradeInputVec.size() != 0)
		tradeVec = tradeInputVec;
	else 
		throw AQLCoreInvalidData("convergent set error",__FILE__,__LINE__);

	

	//in convergence case, the purpose is only getting the result,
	//we do not need save it, so that (1) clone (2) get the result (3) delete.

	
	//single trade 
	AQLString key_suffix = "_CLONE";
	DoubleArray ret;
	for (unsigned int i = 0; i < tradeVec.size(); ++i)
	{
		createSingleTrade(dataInstance,maptrade,tradeVec[i],true, key_suffix,true);

		//from renee,the par rate should depend on the trade details entered 
		//(dates, leg conventions, etc) and not on the rate, spread or bid/offer.
		
		//create offerbid curve
		//createOfferBidAdjustEntity(dataInstance,maptrade,tradeVec,true,key_suffix);
		
		
		AQLStringVector tmpVec(1, tradeVec[i] + key_suffix);
		//change into convergence intofixed rate;
		AQLMathObjectValue& eval = dynamic_cast<AQLMathObjectValue& >(objPool.getObject(tmpVec[0], ENCHKTYPE_ISDEFINED).get());
		eval.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).convertFromString(FN_IR_CONVERGENCEVALUE_STR);
		eval.remove(PRICING_DATA_SUBVALUE);
		eval.AQLObject::add(PRICING_DATA_SUBVALUE, new AQLDataValuation()).convertFromString(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
		eval.remove(PRICING_DATA_DIRTYPRICE);
		eval.AQLObject::add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(0.0));
		//check line product 
		AQLString lineproduct = dynamic_cast<AQLDataString &>(eval.getData("LineProductType", ISNOTNULL).get()).get();
		AQLString target = "";
		if (lineproduct == "SWAP" )
		{
			target = "LEG1FIXEDRATE";
			/*if (target !=  "LEG1FIXEDRATE" && target != "LEG2SPREAD")
				throw AQLCoreInvalidData("target error",__FILE__,__LINE__);*/

		}
		else if (lineproduct == "BASISSWAP")
		{
			target = "LEG1SPREAD";
			AQLStringVector leg1Coeff_str = maptrade[tradeVec[i] + "/" + PRICING_DATA_COEFFICIENT + "/COUPON1"].toToken(':');
			if (leg1Coeff_str.size() != 2)
				throw AQLCoreInvalidData("convergentPlainVanillaTrade Failed! A size of leg1 coefficient is not 2!",__FILE__,__LINE__);
			double leg1Spread = leg1Coeff_str[1].getDoubleValue();

			AQLStringVector leg2Coeff_str = maptrade[tradeVec[i] + "/" + PRICING_DATA_COEFFICIENT + "/COUPON2"].toToken(':');
			if (leg2Coeff_str.size() != 2)
				throw AQLCoreInvalidData("convergentPlainVanillaTrade Failed! A size of leg2 coefficient is not 2!",__FILE__,__LINE__);
			double leg2Spread = leg2Coeff_str[1].getDoubleValue();

			if (leg2Spread !=  0. && leg1Spread == 0.)
				target = "LEG2SPREAD";		
		}
		else if (lineproduct == "FRA")
		{
			target = "LEG1FIXEDRATE";
			/*if (target !=  "LEG1FIXEDRATE")
				throw AQLCoreInvalidData("target error",__FILE__,__LINE__);*/
		}

		eval.remove(PRICING_DATA_CONVERGENCETARGET);
		eval.AQLObject::add(PRICING_DATA_CONVERGENCETARGET, new AQLDataString()).convertFromString(target);

		////set for offerbid curve pricing
		//eline.remove("offerbidentity");
		//eline.add("offerbidentity", new AQLDataReference()).convertFromString("lineofferbidentity");

		dataInstance->getReferencePool().completeDependency();
		DoubleVector convegVec;
		calcRiskAndPV(dataInstance,tmpVec,convegVec);
		
		//eline.remove("offerbidentity");

		
		dh = &(eval.getData(PRICING_DATA_CONVERGENCEVALUE, ISNOTNULL));
		double convValue = dynamic_cast<AQLDataDouble &>(dh->get());
		if (target == "LEG1FIXEDRATE")
			convValue *= 100; //% display
		else if (target == "LEG1SPREAD" || target == "LEG2SPREAD")
			convValue *= 10000; //basis point display
		ret.push_back(convValue);

		//in case of convergent at first we must expand cashflows.
		dh = &eval.getData(PRICING_DATA_CFGENERATOR, ISNOTNULL);
		AQLDataProcedure& modelDataObj = dynamic_cast<AQLDataProcedure&>(dh->get());
		const AQLPriceCashFlowGenerator& cfgen = dynamic_cast<const AQLPriceCashFlowGenerator &>(modelDataObj.getMethod());
		
		//delete cashlet coupon index
		//set Detail into Manual not to cashflow expand
		AQLDataMultiReference& refs = dynamic_cast<AQLDataMultiReference &>(eval.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		while (refs.getSize() > 0)
		{
			AQLObjectHolder& ehleg = refs.get(0);
			cfgen.clearCashlets(ehleg.get());

			AQLString legname = ehleg.getName();
			refs.remove(legname);
			//delete leg
			objPool.remove(legname);
		}
		//delete trade
		objPool.remove(tradeVec[i]+key_suffix);
	}

	return ret;
}


void
AQLMultiSwapPricer::calcPlainVanillaTrades(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade, DoubleVector& pvResultVec)
{
	AQLObjectPool &objPool = dataInstance->getObjectPool();
	AQLObjectHolder objHolder;
	//important clasee "lineparam1"
	AQLObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();

	///////////trade infors/////////////////////////
	//at first no calcvec
	AQLString nocalcswapstr = maptrade["NoCalcSwapTrades"];
	AQLStringVector tradeNoCalcInputVec;
	if (nocalcswapstr != "")
		tradeNoCalcInputVec = nocalcswapstr.toToken(':');
	
	for (unsigned int i = 0; i < tradeNoCalcInputVec.size(); i++)
	{
		AQLString tradeid = tradeNoCalcInputVec[i];
		AQLMathObjectValue* vtrade = NULL;
		objHolder = objPool.getObject(tradeid);
		if (objHolder.isDefined())
		{
			if (!objHolder.isTypeOf(ENTITY_VENTITY))
				throw AQLCoreInvalidData("Trans ID Error",__FILE__,__LINE__);
			vtrade = dynamic_cast<AQLMathObjectValue *>(&objHolder.get());
			vtrade->reset();
		}
		else
		{
			vtrade = new AQLMathObjectValue(dataInstance);
			objPool.set(tradeid, vtrade);
		}

		//set mintradefile
		//name
		vtrade->getData(CALIBRATION_DATA_NAME, ISDEFINED).convertFromString(tradeid);
		vtrade->AQLObject::remove("PV");
		vtrade->AQLObject::add("PV", new AQLDataDouble(0.0));

		//line product
		AQLString lineproduc = maptrade[tradeid + "/" + "LineProductType"];
		vtrade->AQLObject::remove("LineProductType");
		vtrade->AQLObject::add("LineProductType", new AQLDataString(lineproduc));

		//IsPnL
		AQLString ispnl = maptrade[tradeid + "/" + "IsPnL"];
		vtrade->AQLObject::remove("IsPnL");
		vtrade->AQLObject::add("IsPnL", new AQLDataBool()).convertFromString(ispnl);

		//store pv result vec
		pvResultVec.push_back(0.0);

	}
	//////////////////////////////

	//get tradevec;
	AQLString calcswapstr = maptrade["CalcSwapTrades"];
	AQLStringVector tradeInputVec;
	if (calcswapstr != "")
		tradeInputVec = calcswapstr.toToken(':');
	
	for (unsigned int i = 0; i < tradeInputVec.size(); i++)
		createSingleTrade(dataInstance,maptrade,tradeInputVec[i],false, "");

	//if there exists easy risk, then set risk, otherwise only pv = not new anything in "riskentity"
	AQLString riskentitystr;
	if (objPool.getObject("lineriskentityZERODELTA").isDefined())
		riskentitystr += ":lineriskentityZERODELTA";
		
	if (objPool.getObject("lineriskentitySOURCEDELTA").isDefined())
		riskentitystr += ":lineriskentitySOURCEDELTA";

	//remove ":"
	if (riskentitystr.size() > 0)
		riskentitystr = riskentitystr.subString(1, riskentitystr.size() -1);
	
	//set linedelta
	eline.remove("riskentity");
	if (riskentitystr.size() > 0)
	{
		eline.add("riskentity", new AQLDataMultiReference()).convertFromString(riskentitystr);
	}
	
	dataInstance->getReferencePool().completeDependency();
	DoubleVector pvResultNonOfferBidVec;
	calcRiskAndPV(dataInstance,tradeInputVec,pvResultNonOfferBidVec);

	eline.remove("riskentity");

	
	//create offerbid curve
	createOfferBidAdjustEntity(dataInstance,maptrade,tradeInputVec,false, "");

	//set for offerbid curve pricing
	eline.remove("offerbidentity");
	eline.add("offerbidentity", new AQLDataReference()).convertFromString("lineofferbidentity");

	dataInstance->getReferencePool().completeDependency();
	calcRiskAndPV(dataInstance,tradeInputVec,pvResultVec);

	eline.remove("offerbidentity");

	return;
}

void
AQLMultiSwapPricer::calcPlainVanillaTrades(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade, const AQLStringVector& calcTargetVector, 
												 DoubleMatrix& pvResultMatrix)
{
	//check whether NoCalcTrade exists or not
	if (maptrade["NoCalcSwapTrades"] != "")
		throw AQLCoreInvalidData("No calc mode",__FILE__,__LINE__);

	//get tradevec;
	AQLString calcswapstr = maptrade["CalcSwapTrades"];
	AQLStringVector tradeVec;
	if (calcswapstr != "")
	{
		tradeVec = calcswapstr.toToken(':');
	}
	else
	{
		throw AQLCoreInvalidData("No calc trade",__FILE__,__LINE__);
	}

	DoubleVector pvResultVec;
	calcPlainVanillaTrades(dataInstance, maptrade, pvResultVec);

	DoubleVector parRateSpreadVec;
	if (find(calcTargetVector.begin(), calcTargetVector.end(), "ParRateSpread") != calcTargetVector.end())
	{
		parRateSpreadVec = convergentPlainVanillaTrade(dataInstance, maptrade);
	}

	AQLObjectPool& objPool = dataInstance->getObjectPool();
	pvResultMatrix.clear();
	DoubleVector singleTradeResult(calcTargetVector.size());
	for (unsigned int i = 0; i < tradeVec.size(); ++i)
	{
		AQLMathObjectValue& trade = dynamic_cast<AQLMathObjectValue &>(objPool.getObject(tradeVec[i],ENCHKTYPE_ISDEFINED).get());

		for (unsigned int j = 0; j < calcTargetVector.size(); ++j)
		{
			AQLDataHolder* dh = &trade.getData(calcTargetVector[j]);

			if (calcTargetVector[j] == "PV")
			{
				singleTradeResult[j] = pvResultVec[i];
			}
			else if (calcTargetVector[j] == "ParRateSpread")
			{
				singleTradeResult[j] = parRateSpreadVec[i];
			}
			else
			{
				if (dh->isDefined() && !dh->isNull())
				{
					singleTradeResult[j] = dh->get().convertToString().getDoubleValue();
				}
				else
				{
					AQLString msg = "No " + calcTargetVector[j] + " in " + AQLString(static_cast<int>(i)) + " th trade";
					throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
				}
			}
		}
		
		pvResultMatrix.push_back(singleTradeResult);
	}

}

void 
AQLMultiSwapPricer::calcRiskAndPV(AQLDataInstance* dataInstance, const AQLStringVector& tradeVec, DoubleVector& pvResultVec)
{
	AQLDataHolder* dh;
	AQLObjectHolder objHolder;
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLObject& eline = objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).get();
	
	unsigned int sizeScenario = 0;
	AQLStringVector orgEntityName,pvriskScenario,riskDetail1,riskDetail2;
	AQLStringVector discountcurves, paraEntityName;
	AQLString ccyInput,discountcurve; 
	
	//get risk set object
	dh = &(eline.getData("riskentity", NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		//get risk info
		AQLDataMultiReference& refs = dynamic_cast<AQLDataMultiReference &>(dh->get());
		for (unsigned int i = 0; i < refs.getSize(); i++)
		{
			AQLObject& erisk = refs.get(i).get();
			dh = &(erisk.getData("orgEntityName",ISNOTNULL));
			AQLStringVector tmporgEntityName = dynamic_cast<AQLDataStrings &>(dh->get()).get();

			dh = &(erisk.getData("pvriskScenario", ISNOTNULL));
			AQLStringVector tmppvriskScenario = dynamic_cast<AQLDataStrings &>(dh->get()).get();

			dh = &(erisk.getData("riskDetail1", ISNOTNULL));
			AQLStringVector tmpriskDetail1 = dynamic_cast<AQLDataStrings &>(dh->get()).get();

			dh = &(erisk.getData("riskDetail2", ISNOTNULL));
			AQLStringVector tmpriskDetail2 = dynamic_cast<AQLDataStrings &>(dh->get()).get();

			dh = &(erisk.getData("paramEntityName", ISNOTNULL));
			AQLStringVector tmpparamEntityName = dynamic_cast<AQLDataStrings &>(dh->get()).get();

			if (tmppvriskScenario.size() != tmporgEntityName.size() ||
				tmppvriskScenario.size() != tmpriskDetail1.size() || 
				tmppvriskScenario.size() != tmpriskDetail2.size() ||
				tmppvriskScenario.size() != tmpparamEntityName.size() ||
				tmppvriskScenario.size() < 1)
				throw AQLCoreInvalidData("pv and risk set size error",__FILE__,__LINE__);

			//tmporgEntityName is curveID
			AQLStringVector tmpdfcurves(tmporgEntityName.size());
			for (unsigned int j = 0; j < tmporgEntityName.size(); j++)
			{
				dh =&(eline.getData("CURVE_DISCOUNT", NOCHECK));
				AQLString tmp_dfcurve;
				if (!dh->isDefined() || dh->isNull())
				{
					tmp_dfcurve = getDiscountCurveName(dataInstance, tmporgEntityName[j]);
				}
				else
				{
					tmp_dfcurve = dynamic_cast<AQLDataString &>(eline.getData("CURVE_DISCOUNT", NOCHECK).get());;
				}
				tmpdfcurves[j] = tmp_dfcurve;
			}
			
			orgEntityName.insert(orgEntityName.end(),tmporgEntityName.begin(),tmporgEntityName.end());
			pvriskScenario.insert(pvriskScenario.end(), tmppvriskScenario.begin(), tmppvriskScenario.end());
			riskDetail1.insert(riskDetail1.end(), tmpriskDetail1.begin(), tmpriskDetail1.end());
			riskDetail2.insert(riskDetail2.end(), tmpriskDetail2.begin(), tmpriskDetail2.end());
			discountcurves.insert(discountcurves.end(),tmpdfcurves.begin(),tmpdfcurves.end());
			paraEntityName.insert(paraEntityName.end(),tmpparamEntityName.begin(),tmpparamEntityName.end());
		}

		sizeScenario = pvriskScenario.size();
		ccyInput= dynamic_cast<AQLDataString &>(eline.getData("CALC_CURRENCY", ISNOTNULL).get());
	}

	//get set offer bid adjust pricing set
	AQLStringVector offerbidcurves;
	AQLStringVector orgcurves;
	AQLStringVector tradenames;
	AQLStringVector paranames;
	dh = &(eline.getData("offerbidentity", NOCHECK));
	bool isofferbidadj = false;
	if (dh->isDefined() && !dh->isNull())
	{
		//get offer bid info
		AQLDataReference& ref = dynamic_cast<AQLDataReference &>(dh->get());
		orgcurves = dynamic_cast<AQLDataStrings &>(ref.get().getData("orgEntityName",ISDEFINED).get()).get();
		offerbidcurves = dynamic_cast<AQLDataStrings &>(ref.get().getData("adjsutEntityName",ISDEFINED).get()).get();
		tradenames = dynamic_cast<AQLDataStrings &>(ref.get().getData("tradeEntityName",ISDEFINED).get()).get();
		paranames = dynamic_cast<AQLDataStrings &>(ref.get().getData("paramEntityName",ISDEFINED).get()).get();

		if (orgcurves.size() != offerbidcurves.size() || orgcurves.size() != tradenames.size() || orgcurves.size() != paranames.size())
			throw AQLCoreInvalidData("offer bid curve error",__FILE__,__LINE__);

		ccyInput= dynamic_cast<AQLDataString &>(eline.getData("CALC_CURRENCY", ISNOTNULL).get());

		if (orgcurves.size() >= 1)
			isofferbidadj = true;

	}


	/////////plain vanilla object///////////////////

	//get PV
	//////////////////////////////////////////////////
	AQLString asofdateInput = dynamic_cast<AQLDataString &>(eline.getData("CALC_ASOFDATE", ISNOTNULL).get());
	AQLDate basedate(asofdateInput.getCString(),"YYYYMMDD");
	//dataInstance->getReferencePool().completeDependency();

	for (unsigned int i = 0; i < tradeVec.size(); i++)
	{
		//offer bid adjust case
		unsigned int pos = 0;
		AQLMathObjectValue* vtrade = dynamic_cast<AQLMathObjectValue* >(&objPool.getObject(tradeVec[i], ENCHKTYPE_ISDEFINED).get());
		if (isofferbidadj)
		{
			AQLStringVector::iterator itpos = std::find(tradenames.begin(),tradenames.end(),tradeVec[i]);
			if (itpos != tradenames.end())
			{
				pos = static_cast<unsigned int>(itpos - tradenames.begin());
				AQLDataReference& refvnl = dynamic_cast<AQLDataReference &>(vtrade->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
				AQLMathPlainVanillaEntity& pvanilla = dynamic_cast<AQLMathPlainVanillaEntity &>(refvnl.get().get());
				if (pvanilla.getName().get() != paranames[pos])
					continue;

				pvanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).convertFromString(offerbidcurves[pos]);
				
			}

		}
		
		dh = &(vtrade->getData(CALIBRATION_DATA_VALUE, ISDEFINED));
		AQLDataValuation& val = dynamic_cast<AQLDataValuation &>(dh->get());
		double ret = val.value(basedate);
		//store pv and return;
		pvResultVec.push_back(ret);

		//offer bid adjust case
		if (isofferbidadj)
		{
		
			AQLDataReference& refvnl = dynamic_cast<AQLDataReference &>(vtrade->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
			AQLMathPlainVanillaEntity& pvanilla = dynamic_cast<AQLMathPlainVanillaEntity &>(refvnl.get().get());
			if (pvanilla.getName().get() != paranames[pos])
				continue;

			pvanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).convertFromString(orgcurves[pos]);
		}
	}
		
	AQLString chgentityname;
	AQLString orgentityname;
	AQLString paraentityname;
	//get risk
	//////////////////////////////////////////////////
	for (unsigned int h = 0; h < sizeScenario; h++)
	{
		
		//set up risk scenario
		orgentityname = orgEntityName[h];
		chgentityname = orgEntityName[h] + "_" + pvriskScenario[h] + "_" + riskDetail1[h];
		if (riskDetail2[h] != "")
			chgentityname += "_" + riskDetail2[h];
		
		//orgentityname is curveID
		paraentityname = paraEntityName[h];
		AQLMathPlainVanillaEntity& pvanilla = dynamic_cast<AQLMathPlainVanillaEntity &>(objPool.getObject(paraentityname,ENCHKTYPE_ISDEFINED).get());
		pvanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).convertFromString(chgentityname);
	
		for (unsigned int i = 0; i < tradeVec.size(); i++)
		{
			AQLString tradeid = tradeVec[i];

			//in case path object is different such as libor df pricing or ois df pricing per each trade
			AQLString traderefparam = getMarketParamfromTradeReference(dataInstance,tradeid);
			if (traderefparam != paraentityname)
				continue;
	
			//in case of zero rate risk, we must use zero risk of discount curve and index forecast curve
			if (pvriskScenario[h] == "ZERODELTA")
			{
				AQLStringVector riskTargetVec;
				//discount
				riskTargetVec.push_back(discountcurves[h]);
				//find forecast
				for (unsigned int j = 0; j < 2; j++)
				{
					AQLString indexInput;
					if (0 == j)
						indexInput = tradeid + "_leg1_coupon1_index1";
					else 
						indexInput = tradeid + "_leg2_coupon2_index2";

					objHolder = objPool.getObject(indexInput, ENCHKTYPE_ISDEFINED);
					dh = &(objHolder.getData(PRICING_DATA_BASISCURVE, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
						riskTargetVec.push_back(dynamic_cast<AQLDataString &>(dh->get()));

				}
			
				////if no target, no need for calc
				if (std::find(riskTargetVec.begin(),riskTargetVec.end(), riskDetail1[h]) == riskTargetVec.end())
					continue;
			}



			AQLMathObjectValue* vtrade = dynamic_cast<AQLMathObjectValue* >(&objPool.getObject(tradeid, ENCHKTYPE_ISDEFINED).get());
			//common set up for each deal				
			vtrade->remove(PRICING_DATA_ISCALCRISK);
			vtrade->AQLObject::add(PRICING_DATA_ISCALCRISK, new AQLDataBool(true));
			//no need for data out info.
			vtrade->remove(PRICING_DATA_ISRESULTOUTPUT);
			vtrade->AQLObject::add(PRICING_DATA_ISRESULTOUTPUT, new AQLDataBool(false));

			dh = &(vtrade->getData(CALIBRATION_DATA_VALUE, ISDEFINED));
			AQLDataValuation& val = dynamic_cast<AQLDataValuation &>(dh->get());
			
			double ret = val.value(basedate);
			//add the risk information on TradeData
			AQLString key = "PVValueBy" + chgentityname;
			vtrade->remove(key);
			vtrade->AQLObject::add(key, new AQLDataDouble(ret));

			//if we do not remove, the unexpected results might cause.				
			vtrade->remove(PRICING_DATA_ISCALCRISK);
			vtrade->remove(PRICING_DATA_ISRESULTOUTPUT);
			vtrade->AQLObject::add(PRICING_DATA_ISRESULTOUTPUT, new AQLDataBool(true));
			
		}

		//set back to original scenario and 
		pvanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).convertFromString(orgentityname);
	}

	//re edit for trades grid sum and so on
	//zero case, oiszero + 3mzero.
	for (unsigned int i = 0; i < tradeVec.size(); i++)
	{
		AQLString tradeid = tradeVec[i];
		AQLMathObjectValue* vtrade = dynamic_cast<AQLMathObjectValue *>(&objPool.getObject(tradeVec[i], ENCHKTYPE_ISDEFINED).get());
		double orgpv = dynamic_cast<AQLDataDouble &>(vtrade->AQLObject::getData("PV",ISNOTNULL).get());
		double zerodelta = 0.0;
		double sourcedeltaup = 0.0;
		double sourcedeltadown = 0.0;
		double gamma = 0.0;
		bool iszeropara= false;
		bool issourcepara = false;
		bool isgammapara = false;

		//get zero rate val
		for (unsigned int j = 0; j< sizeScenario; j++)
		{
			chgentityname = orgEntityName[j] + "_" + pvriskScenario[j] + "_" + riskDetail1[j];
			if (riskDetail2[j] != "")
				chgentityname += "_" + riskDetail2[j];

			double chgpv = 0.0;
			AQLString key = "PVValueBy" + chgentityname;
			dh = &	(vtrade->AQLObject::getData(key,NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				chgpv = dynamic_cast<AQLDataDouble &>(dh->get());
			}
			else
			{
				//zero rate case
				continue;
			}

			if (pvriskScenario[j] == "ZERODELTA" && riskDetail2[j].findString("_BPPARALLEL") != -1)
			{
				iszeropara = true;
				zerodelta += chgpv - orgpv;
			}

			if (pvriskScenario[j] == "SOURCEDELTA" && riskDetail2[j].findString("_BPPARALLEL") != -1)
			{
				AQLString shiftstr = riskDetail2[j].toToken('_')[0];
				double shiftval = shiftstr.getDoubleValue();
				if (shiftval >= 0.0)
				{
					issourcepara = true;
					sourcedeltaup += chgpv - orgpv;
				}
				else
				{
					isgammapara = true;
					sourcedeltadown += orgpv - chgpv;
				
				}
			}
				
			//source bump risk
			if (pvriskScenario[j] == "SOURCEDELTA" && riskDetail2[j].findString(";") != -1)
			{
				//change general attrname;

				//from PVValueBy"curveID" + "_SOURCEDELTA_STD_swapRates;1Y;0
				//into SOURCEBUMPRISK_STD_swapRates_1Y
				AQLStringVector tmpvec = riskDetail2[j].toToken(';');
				if (tmpvec.size() != 3)
					throw AQLCoreInvalidData("bump risk set error",__FILE__,__LINE__);
				AQLString key = "SOURCEBUMPRISK_" + riskDetail1[j] + "_" + tmpvec[0] + "_" + tmpvec[1];
				vtrade->remove(key);
				vtrade->AQLObject::add(key, new AQLDataDouble(chgpv- orgpv));
			}

			//zero bump risk
			if (pvriskScenario[j] == "ZERODELTA" && riskDetail2[j].findString(";") != -1)
			{
				//change general attrname;

				//from PVValueBy"curveID" + "_SOURCEDELTA_STD_swapRates;1Y;0
				//into ZEROBUMPRISK_STD_1Y
				AQLStringVector tmpvec = riskDetail2[j].toToken(';');
				if (tmpvec.size() != 3)
					throw AQLCoreInvalidData("bump zero risk set error",__FILE__,__LINE__);
				AQLString key = "ZEROBUMPRISK_" + riskDetail1[j] + "_" + tmpvec[0] + "_" + tmpvec[1];
				vtrade->remove(key);
				vtrade->AQLObject::add(key, new AQLDataDouble(chgpv- orgpv));
			}

		}

		if (iszeropara)
		{
			vtrade->remove("Zero delta");
			vtrade->AQLObject::add("Zero delta", new AQLDataDouble(zerodelta));
		}

		if (issourcepara)
		{
			vtrade->remove("Source delta");
			vtrade->AQLObject::add("Source delta", new AQLDataDouble(sourcedeltaup));
		}

		if (isgammapara)
		{
			vtrade->remove("gamma");
			vtrade->AQLObject::add("gamma", new AQLDataDouble(sourcedeltaup- sourcedeltadown));
		}
	}

	return;
}


void
AQLMultiSwapPricer::setUpPreparetionForcalcTrades(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> mapcalcinfo)
{
	
	AQLObjectPool &objPool = dataInstance->getObjectPool();
	AQLObjectHolder objHolder;
	AQLDataHolder* dh;

	/////////////////////////////
	//set up lineparam1
	setUpLineParams(dataInstance, mapcalcinfo);

	///////////plain vanilla object///////////////////
	AQLString lineparam("lineparam1");
	AQLObject* eline = &(objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get());


	AQLStringVector curveIDs = mapcalcinfo[AQLString("CURVE/") + CALIBRATION_DATA_CURVEID].toToken(':');
	AQLStringVector params(curveIDs.size());
	//in case of global shift curveIDs change
	for (unsigned int i = 0; i < curveIDs.size(); i++)
		params[i] = setUpMarketParamsAndGlobalShift(dataInstance, mapcalcinfo, curveIDs[i]);

	eline->remove("CALC_MARKETPARAMS");
	eline->add("CALC_MARKETPARAMS", new AQLDataStrings(params));
	///////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////
	//set up risk object for zerodelta source delta and gamma
	AQLStringVector orgEntityName, pvriskScenario, riskDetail1, riskDetail2;
	//zero delta;

	//set up zero delta it can be extended of bumprisk!!
	AQLStringVector riskDetail1zerodelta(4);
	riskDetail1zerodelta[0] = dynamic_cast<AQLDataString &>(eline->getData("CURVE_3MFORECAST",ISNOTNULL).get());
	riskDetail1zerodelta[1] = dynamic_cast<AQLDataString &>(eline->getData("CURVE_6MFORECAST",ISNOTNULL).get());
	riskDetail1zerodelta[2] = dynamic_cast<AQLDataString &>(eline->getData("CURVE_1MFORECAST",ISNOTNULL).get());
	riskDetail1zerodelta[3] = dynamic_cast<AQLDataString &>(eline->getData("CURVE_OISFORECAST",ISNOTNULL).get());


	//set whether we prepare many curves for delta or gamma pricing
	AQLString sourcestr = mapcalcinfo[AQLString("CALC/") + "IsCalcSourceDelta"];
	AQLString zerostr = mapcalcinfo[AQLString("CALC/") + "IsCalcZeroDelta"];
	AQLString gammastr = mapcalcinfo[AQLString("CALC/") + "IsCalcGamma"];

	if (sourcestr.toUpper() == "FALSE" && gammastr.toUpper() == "TRUE")
		throw AQLCoreInvalidData("inconsitency of calc common info", __FILE__,__LINE__);

	AQLString keyshiftval = mapcalcinfo[AQLString("CALC/") + "ParaShiftScenario"];
	AQLString upshiftstr, downshiftstr;
	if (keyshiftval != "")
	{
		upshiftstr = AQLString(keyshiftval.getDoubleValue(),3);
		downshiftstr = AQLString(keyshiftval.getDoubleValue() * -1.0 ,3);
	}
	else
	{
		upshiftstr = AQLString(1.0, 3);
		downshiftstr = AQLString(-1.0, 3);

	}

	////to calc zero para delta 1.000 bp
	if (zerostr == "TRUE")
	{
		AQLStringVector riskDetail2zerodelta(1);
		riskDetail2zerodelta[0] = upshiftstr + "_BPPARALLEL";

		//for both curves (libobasecurve oisbasecurve)
		for (unsigned int i = 0; i < curveIDs.size(); i++)
			createZeroDeltaRiskEntity(dataInstance, "lineriskentityZERODELTA", riskDetail1zerodelta, riskDetail2zerodelta, false);
		
		
	}
	else
	{
		objPool.remove("lineriskentityZERODELTA");
	}

	
	
	
	dh = &(eline->getData("CURVE_OISFORECAST", ISNOTNULL));
	AQLString fcurvestrois = dynamic_cast<AQLDataString &>(dh->get()).get();

	dh = &(eline->getData("SWAP_FREQUENCY", ISNOTNULL));
	AQLString swapfreq = dynamic_cast<AQLDataString &>(dh->get()).get();

//	dh = &(eline->getData("CURVE_3MFORECAST", ISNOTNULL));
	dh = &(eline->getData(AQLString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL));
//	AQLString fcurvestr3m = dynamic_cast<AQLDataString &>(dh->get()).get();
	AQLString fcurvestrstd = dynamic_cast<AQLDataString &>(dh->get()).get();
	//source delta;
	if (sourcestr != "FALSE")
	{
		//set up source delta it can be extended of bumprisk!!

		AQLStringVector riskDetail1sordelta(1);
//		riskDetail1sordelta[0] = fcurvestrois + "+" + fcurvestr3m;
		riskDetail1sordelta[0] = fcurvestrois + "+" + fcurvestrstd;

		//
		AQLStringVector riskDetail2sordelta;
		riskDetail2sordelta.push_back(upshiftstr + "_BPPARALLEL");
		
		////to calc gamma 1bp up and 1bp down need developing
		if (gammastr == "TRUE")
		{
			riskDetail2sordelta.push_back(downshiftstr + "_BPPARALLEL");
		}

		//for both curves (libobasecurve oisbasecurve)
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEDELTA", riskDetail1sordelta, riskDetail2sordelta, false);
	}
	else
	{
		objPool.remove("lineriskentitySOURCEDELTA");
	}


	return;
}

void 
AQLMultiSwapPricer::createSourceDeltaRiskEntity(AQLDataInstance* dataInstance, AQLString scenarioname, AQLStringVector riskDetail1sordelta, AQLStringVector riskDetail2sordelta, bool isbumpgridauto, AQLString paraName)
{

	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLDataHolder* dh;
	AQLObjectHolder objHolder;

	AQLString lineparam("lineparam1");
	AQLObject* eline = &(objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get());

	//if paraName is directly selected, then only one curve is created, otherwise both curves (libor base curve and ois base curve)
	AQLStringVector paramVec;
	if (paraName != "")
		paramVec.push_back(paraName);
	else
	{
		dh = &(eline->getData("CALC_MARKETPARAMS", ISNOTNULL));
		paramVec = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}
	AQLStringVector::iterator itpara = paramVec.begin();
	
	dh = &(eline->getData("CALC_CURRENCY", ISNOTNULL));
	AQLString ccyInput = dynamic_cast<AQLDataString &>(dh->get());

	//this curve id is imporatant for using zero delta para
	AQLString oisforecurve = dynamic_cast<AQLDataString &>(objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).getData("CURVE_OISFORECAST", ISNOTNULL).get());
	AQLString libor3mforecurve = dynamic_cast<AQLDataString &>(objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).getData("CURVE_3MFORECAST", ISNOTNULL).get());
	AQLString libor6mforecurve = dynamic_cast<AQLDataString &>(objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).getData("CURVE_6MFORECAST", ISNOTNULL).get());
	AQLString libor1mforecurve = dynamic_cast<AQLDataString &>(objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).getData("CURVE_1MFORECAST", ISNOTNULL).get());


	//set up risk object for source delta and gamma
	AQLStringVector orgEntityNameSource, pvriskScenarioSource, riskDetail1Source, riskDetail2Source, paramEntityNameSource;
	while (itpara != paramVec.end())
	{
		//set up market bump rate
		AQLMathPlainVanillaEntity& pvanilla = dynamic_cast<AQLMathPlainVanillaEntity &>(objPool.getObject(*itpara,ENCHKTYPE_ISDEFINED).get());
		AQLString curveID = pvanilla.getIRCurve(ccyInput).getYieldData().get().getName();

		AQLMathYieldCurvePro& ypro = dynamic_cast<AQLMathYieldCurvePro&>(objPool.getObject(AQLString("PRO_") + PREFIX_YIELD + curveID, ENCHKTYPE_ISDEFINED).get());

		AQLStringVector changed2Vec;
		if (isbumpgridauto)
		{
			//in case of riskDetail1sordelta[0] = "OISCurve"
			//in case of riskDetail2sordelt[1] = "oisRates"		
			//then changed2Vec is changed as oisRates;FFV1;0,oisRates;FFX1;1,.....
			
			if (riskDetail2sordelta.size() != 1)
				throw AQLCoreInvalidData("bump source delta set error",__FILE__,__LINE__);

			AQLStringVector ret2Vec;
			AQLStringVector tmpdetail2vec;
			AQLString rateboxname = riskDetail2sordelta[0];
			if (rateboxname != "liborRates" && rateboxname != "swapRates" &&
				rateboxname != "fra3mRates" && rateboxname != "fra6mRates" &&
				rateboxname != "futureRates" && rateboxname != "basisMkt" && rateboxname != "oisRates")
				throw AQLCoreInvalidData("bump source initial error",__FILE__,__LINE__);

			AQLString mkt_name = ypro.getMarketForCurve(riskDetail1sordelta[0]);
			//AQLString curveorginfo = "lineparam1_" + curveID + riskDetail1sordelta[0];
			AQLString curveorginfo = "lineparam1_" + curveID + mkt_name;
			AQLObject* ecurveorg = &(objPool.getObject(curveorginfo,ENCHKTYPE_ISDEFINED).get());
			dh = &(ecurveorg->getData(rateboxname, ISNOTNULL));
			{
				const AQLStringMatrix& ratemat = dynamic_cast<AQLDataStringMatrix& >(dh->get()).get();
				for (unsigned int j = 0; j < ratemat.size(); j++)
					tmpdetail2vec.push_back(rateboxname + ";" +ratemat[j][0] + ";" + AQLDataInt(j).convertToString());
			}

			changed2Vec = tmpdetail2vec;
		}
		else
			changed2Vec = riskDetail2sordelta;

		////set up order 
		AQLStringVector setupOrder = getCurveNamesFromSetUpOrder(dataInstance, curveID);
		vector<AQLString> mkt_name(setupOrder.size());
		for(unsigned int i = 0; i < setupOrder.size(); i++)
		{
			mkt_name[i] = ypro.getMarketForCurve(setupOrder[i]);
		}

		
		unsigned int shiftpos = 0;
		bool ispara = true;
		for (unsigned int i = 0; i < riskDetail1sordelta.size(); i++)
		{
			for (unsigned int j = 0; j < changed2Vec.size(); j++)
			{
				AQLString strSourceShiftCurve = "Temporary_SOURCEDELTA";
				for (unsigned int k = 0; k < setupOrder.size(); k++)
				{
					double shiftval = 0.0;
					AQLString market ;
					bool isshiftarget = true;

					//imporatant judge
					//if (riskDetail1sordelta[i] ==  setupOrder[k] && changed2Vec[j] == "PARALLEL1BP")
					if (riskDetail1sordelta[i].findString(setupOrder[k]) != -1 && changed2Vec[j].findString("_BPPARALLEL") != -1)
					{
						//always +(-)x.xxx_BPPARALLEL
						AQLString shiftstr = changed2Vec[j].toToken('_')[0];
						shiftval = shiftstr.getDoubleValue();
						shiftval *= 0.0001;
						ispara = true;
						shiftpos = 0;
						isshiftarget = true;
						market = "ALL";
					
					}
					else if( riskDetail1sordelta[i].findString(setupOrder[k]) != -1 && changed2Vec[j].findString(";") != -1)
					{
						shiftval = 0.0001;
						ispara = false;
						//e.x. swapRates;1Y;0
						AQLStringVector marketandgrid = changed2Vec[j].toToken(';');
						if (marketandgrid.size() != 3)
							throw AQLCoreInvalidData("market and grid error",__FILE__,__LINE__);
						
						shiftpos = static_cast<unsigned int>(marketandgrid[2].getIntValue());

						isshiftarget = true;

						market = marketandgrid[0];
					
					}
					else
					{
						isshiftarget = false;
					}


					//org object from setUpStoreFunc
					//AQLString curveorginfo = "lineparam1_" + curveID + setupOrder[k];
					AQLString curveorginfo = "lineparam1_" + curveID + mkt_name[k];
					AQLObject* ecurveorg = &(objPool.getObject(curveorginfo,ENCHKTYPE_ISDEFINED).get());

					//create shift curves
					createShiftCurves(dataInstance, *ecurveorg, setupOrder[k], strSourceShiftCurve,isshiftarget,ispara, 
											shiftval,shiftpos,market,false);



				}
				//curve generate
				//name is very imporatant!!
				AQLObject* eclone = objPool.getObject(strSourceShiftCurve, ENCHKTYPE_ISDEFINED).clone();
				//objPool.remove(strSourceShiftCurve);

				AQLString keyname= curveID + "_SOURCEDELTA_";
				keyname += riskDetail1sordelta[i] + "_" + changed2Vec[j];
				if (objPool.getObject(keyname).isDefined())
					objPool.remove(keyname);

				eclone->getData(CALIBRATION_DATA_NAME,ISNOTNULL).convertFromString(keyname);
				objPool.set(keyname, eclone);
		

				//store information
				orgEntityNameSource.push_back(curveID);
				pvriskScenarioSource.push_back("SOURCEDELTA");
				riskDetail1Source.push_back(riskDetail1sordelta[i]);
				riskDetail2Source.push_back(changed2Vec[j]);
				paramEntityNameSource.push_back(*itpara);

			}
		
		}
		itpara++;
	}
	
	AQLObject* eSource = NULL;
	objHolder = objPool.getObject(scenarioname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		eSource = new AQLObject();
		objPool.set(scenarioname,eSource);
	
	}
	else
	{
		eSource = &(objHolder.get());
		eSource->clear();
	}
	eSource->add("orgEntityName", new AQLDataStrings(orgEntityNameSource));
	eSource->add("pvriskScenario", new AQLDataStrings(pvriskScenarioSource));
	eSource->add("riskDetail1", new AQLDataStrings(riskDetail1Source));
	eSource->add("riskDetail2", new AQLDataStrings(riskDetail2Source));
	eSource->add("paramEntityName", new AQLDataStrings(paramEntityNameSource));

	return;

}

void 
AQLMultiSwapPricer::createZeroDeltaRiskEntity(AQLDataInstance* dataInstance, AQLString scenarioname, AQLStringVector riskDetail1zerodelta, AQLStringVector riskDetail2zerodelta, bool isbumpgridauto, AQLString paraName)
{

	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLDataHolder* dh;
	AQLObjectHolder objHolder;

	AQLString lineparam("lineparam1");
	AQLObject* eline = &(objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get());

	//if paraName is directly selected, then only one curve is created, otherwise both curves (libor base curve and ois base curve)
	AQLStringVector paramVec;
	if (paraName != "")
		paramVec.push_back(paraName);
	else
	{
		dh = &(eline->getData("CALC_MARKETPARAMS", ISNOTNULL));
		paramVec = dynamic_cast<AQLDataStrings &>(dh->get()).get();
	}
	AQLStringVector::iterator itpara = paramVec.begin();

	dh = &(eline->getData("CALC_CURRENCY", ISNOTNULL));
	AQLString ccyInput = dynamic_cast<AQLDataString &>(dh->get());
	dh = &(eline->getData("SWAP_FREQUENCY", ISNOTNULL));
	AQLString swapfreq = dynamic_cast<AQLDataString &>(dh->get());


	//set up risk object for source delta and gamma
	AQLStringVector orgEntityNameZero, pvriskScenarioZero, riskDetail1Zero, riskDetail2Zero, paramEntityNameZero;
	while (itpara != paramVec.end())
	{
		//set up market bump rate
		AQLMathPlainVanillaEntity& pvanilla = dynamic_cast<AQLMathPlainVanillaEntity &>(objPool.getObject(*itpara,ENCHKTYPE_ISDEFINED).get());
		AQLString curveID = pvanilla.getIRCurve(ccyInput).getYieldData().get().getName();
		
		AQLObject* porgyield = &(objPool.getObject(curveID, ENCHKTYPE_ISDEFINED).get());
		AQLString yieldProName = "PRO_YIELD_" + curveID;
		AQLMathYieldCurvePro* porgpro = &(dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(yieldProName, ENCHKTYPE_ISDEFINED).get()));
		AQLString orgcurvetype = porgpro->getCurveType();


		//this curve id is imporatant for using zero delta para
		AQLString suffix_data;
		unsigned int shiftpos = 0;
		bool ispara = true;
		for (unsigned int i = 0; i < riskDetail1zerodelta.size(); i++)
		{
			AQLString curvetype = riskDetail1zerodelta[i];
			if (riskDetail1zerodelta[i] != STD)
			{
				suffix_data = "_" + curvetype;
			}

			UintArray gridPos;
			AQLStringVector changed2Vec;
			if (isbumpgridauto)
			{
				const AQLStringVector& zerodeltagrids = porgpro->getConversionMatrixTermTypes(curvetype).get();
				//this data should be added to avoid duplicate the same grid of dirrerent market
				//e.x. STD;swapRates1Y, STD:liborRates1Y
				const AQLStringVector& markettypesvec = porgpro->getConversionMarketTypes(curvetype).get();

				if (zerodeltagrids.size() != markettypesvec.size())
					throw AQLCoreInvalidData("curve set error",__FILE__,__LINE__);

				AQLStringVector rateboxvec(markettypesvec.size());
				for (unsigned int j = 0; j < markettypesvec.size(); j++)
				{
					AQLString key = markettypesvec[j];
					if (BASIS == key)
					{
						rateboxvec[j] = "basisMkt";
					
					}
					else if (PAR == key && curvetype == dynamic_cast<AQLDataString &>(eline->getData("CURVE_OISFORECAST", ISNOTNULL).get()).get())
					{
						rateboxvec[j] = "oisRates";
					}
//					else if (PAR == key && curvetype == dynamic_cast<AQLDataString &>(eline->getData("CURVE_3MFORECAST", ISNOTNULL).get()).get())
					else if (PAR == key && curvetype == dynamic_cast<AQLDataString &>(eline->getData(AQLString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get()).get())
					{
						rateboxvec[j] = "swapRates";
					}
					else if (ZERO == key)
					{
						rateboxvec[j] = "liborRates";
					}
					else if (O_N == key || T_N == key)
					{
						rateboxvec[j] = "liborRates";
					}
					else if (FUTURE == key)
					{
						rateboxvec[j] = "futureRates";
					}
					else if (BOJ == key || FEDFUNDRATE == key)
					{
						rateboxvec[j] = "oisRates";
					}
					else if (FRA6M == key)
					{
						rateboxvec[j] = "fra6mRates";
					}
					else if (FRA3M == key)
					{
						rateboxvec[j] = "fra3mRates";
					}
					else 
						throw AQLCoreInvalidData("market type error",__FILE__,__LINE__);
				}

				AQLStringVector tmpdetail2vec;
				for (unsigned int j = 0; j < zerodeltagrids.size(); j++)
					tmpdetail2vec.push_back(rateboxvec[j] + ";" + zerodeltagrids[j] + ";" + AQLDataInt(j).convertToString());

				changed2Vec = tmpdetail2vec;
				gridPos.resize(tmpdetail2vec.size());

				//in case of zero only supports grid zero delta
				
				dh = &(porgyield->getData(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM + suffix_data, ISNOTNULL));
				const DoubleVector& zerodeltaterms = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();

				dh = &(porgyield->getData(CALIBRATION_DATA_TERMS + suffix_data, ISNOTNULL));
				const DoubleVector& terms = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();

				for (unsigned int j = 0; j < zerodeltagrids.size(); j++)
					AQLCalibrationUtilities<double>::searchNearestPos(terms, zerodeltaterms[j], gridPos[j]);
			}
			else
				changed2Vec = riskDetail2zerodelta;

			

			for (unsigned int j = 0; j < changed2Vec.size(); j++)
			{
				double shiftval = 0.0;
				
				//imporatant judge
				if (changed2Vec[j].findString("_BPPARALLEL") != -1)
				{
					//always +(-)x.xxx_BPPARALLEL
					AQLString shiftstr = changed2Vec[j].toToken('_')[0];
					shiftval = shiftstr.getDoubleValue();
					shiftval *= 0.0001;
					ispara = true;
					shiftpos = 0;
					
				}
				else if( changed2Vec[j].findString(";") != -1)
				{
					shiftval = 0.0001;
					ispara = false;
					//e.x. 1Y;0
					AQLStringVector marketandgrid = changed2Vec[j].toToken(';');
					if (marketandgrid.size() != 3)
						throw AQLCoreInvalidData("zero rate grid error",__FILE__,__LINE__);
					
					//shiftpos = static_cast<unsigned int>(marketandgrid[2].getIntValue());
				}

				AQLString strShiftCurve = curveID + "_ZERODELTA_" + riskDetail1zerodelta[i] + "_" + changed2Vec[j] ;
				objHolder = objPool.getObject(strShiftCurve);
				if (objHolder.isDefined())
					objPool.remove(strShiftCurve);

				AQLObject* shiftCurve = porgyield->clone();
				objPool.set(strShiftCurve, shiftCurve);
				shiftCurve->getData(CALIBRATION_DATA_NAME, ISDEFINED).convertFromString(strShiftCurve);

				if (ispara)
				{

					dh = &(shiftCurve->getData(CALIBRATION_DATA_TERMS + suffix_data, ISNOTNULL));
					const DoubleVector& terms = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();
					
					dh = &(shiftCurve->getData(IR_CALIBRATION_DATA_DFS + suffix_data, ISNOTNULL));
					DoubleVector dfs = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
					
					if (terms.size() != dfs.size())
						throw AQLCoreInvalidData("Discount Size Error",__FILE__,__LINE__);
					for (unsigned int k = 0; k < terms.size(); k++)
						dfs[k] *= AQLMath::exp(-shiftval * terms[k]);

					dynamic_cast<AQLDataDoubles &>(dh->get()).set(dfs);
				}
				else
				{
					porgpro->setCurveType(riskDetail1zerodelta[i]);
					porgpro->getYieldData().convertFromString(strShiftCurve);
					porgpro->setDFByShiftZero(shiftval, j, gridPos);
					porgpro->getYieldData().convertFromString(curveID);

				}

				//store information
				orgEntityNameZero.push_back(curveID);
				pvriskScenarioZero.push_back("ZERODELTA");
				riskDetail1Zero.push_back(riskDetail1zerodelta[i]);
				riskDetail2Zero.push_back(changed2Vec[j]);
				paramEntityNameZero.push_back(*itpara);
			
			}
		
		}

		porgpro->setCurveType(orgcurvetype);
		itpara++;
	}

	AQLObject* eSource = NULL;
	objHolder = objPool.getObject(scenarioname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		eSource = new AQLObject();
		objPool.set(scenarioname,eSource);
	
	}
	else
	{
		eSource = &(objHolder.get());
		eSource->clear();
	}
	eSource->add("orgEntityName", new AQLDataStrings(orgEntityNameZero));
	eSource->add("pvriskScenario", new AQLDataStrings(pvriskScenarioZero));
	eSource->add("riskDetail1", new AQLDataStrings(riskDetail1Zero));
	eSource->add("riskDetail2", new AQLDataStrings(riskDetail2Zero));
	eSource->add("paramEntityName", new AQLDataStrings(paramEntityNameZero));

	return;

}

void
AQLMultiSwapPricer::setUpStoreBasisCurve(AQLDataInstance* dataInstance,
                                const AQLString& curveID, 
                                const AQLString& basisCurveName, 
                                const AQLStringMatrix& basisMkt, 
                                const AQLStringMatrix& basisConv,
								const AQLStringMatrix& generateProp,
								const AQLStringMatrix& moneyConv)
{

	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLObject* curvestore = NULL;
	// Special Treatment for picking up market name through YieldCurvePro class
	AQLString basisCurveNameTmp = basisCurveName;
	basisCurveNameTmp.toUpper();
	AQLString keyname = "lineparam1_" + curveID + basisCurveNameTmp;
	AQLObjectHolder objHolder = objPool.getObject(keyname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		curvestore = new AQLObject();
		objPool.set(keyname, curvestore);
	}
	else
	{
		curvestore = &objHolder.get();
		curvestore->clear();
	}
	
	curvestore->add(CALIBRATION_DATA_NAME, new AQLDataString(keyname));
	curvestore->add("basisCurveName", new AQLDataString(basisCurveName));
	curvestore->add("basisMkt", new AQLDataStringMatrix(basisMkt));
	curvestore->add("basisConv", new AQLDataStringMatrix(basisConv));
	curvestore->add("generateProp", new AQLDataStringMatrix(generateProp));
	curvestore->add("moneyConv", new AQLDataStringMatrix(moneyConv));


	//AQLString key = "SOURCEBUMPRISK_" + riskDetail1[j] + "_" + tmpvec[0] + "_" + tmpvec[1];
	//register attrmaster to output to be able to display excel
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	for (unsigned int i = 0; i < basisMkt.size(); i++)
	{
		AQLString key = "SOURCEBUMPRISK_" + basisCurveName + "_" + "basisMkt" + "_" + basisMkt[i][0];
		dm.setData(key, DATA_DOUBLE);

		AQLString zerokey = "ZEROBUMPRISK_" + basisCurveName + "_" + "basisMkt" + "_" + basisMkt[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}
}

void
AQLMultiSwapPricer::setUpStoreSwapCurve(AQLDataInstance* dataInstance,
								 const AQLString& curveID,
								 const AQLString& genCurveName,
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
								 const AQLStringMatrix& adjustSwapRates)
{

	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLObject* curvestore = NULL;
	// Special Treatment for picking up market name through YieldCurvePro class
	AQLString genCurveNameTmp = "SWAP";
	AQLString keyname = "lineparam1_" + curveID + genCurveNameTmp;
	AQLObjectHolder objHolder = objPool.getObject(keyname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		curvestore = new AQLObject();
		objPool.set(keyname, curvestore);
	}
	else
	{
		curvestore = &objHolder.get();
		curvestore->clear();
	}
	
	curvestore->add(CALIBRATION_DATA_NAME, new AQLDataString(keyname));
	curvestore->add("genCurveName", new AQLDataString(genCurveName));
	curvestore->add("generateProp", new AQLDataStringMatrix(generateProp));
	curvestore->add("moneyConv", new AQLDataStringMatrix(moneyConv));
	curvestore->add("liborRates", new AQLDataStringMatrix(liborRates));
	curvestore->add("liborConv", new AQLDataStringMatrix(liborConv));
	curvestore->add("swapRates", new AQLDataStringMatrix(swapRates));
	curvestore->add("swapConv", new AQLDataStringMatrix(swapConv));
	curvestore->add("fra3mRates", new AQLDataStringMatrix(fra3mRates));
	curvestore->add("fra6mRates", new AQLDataStringMatrix(fra6mRates));
	curvestore->add("fraConv", new AQLDataStringMatrix(fraConv));
	curvestore->add("futureRates", new AQLDataStringMatrix(futureRates));
	curvestore->add("futureConv", new AQLDataStringMatrix(futureConv));
	curvestore->add("adjustSwapConv", new AQLDataStringMatrix(adjustSwapConv));
	curvestore->add("adjustSwapRates", new AQLDataStringMatrix(adjustSwapRates));

	//register attrmaster to output to be able to display excel
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	for (unsigned int i = 0; i < liborRates.size(); i++)
	{
		AQLString key = "SOURCEBUMPRISK_" + genCurveName + "_" + "liborRates" + "_" + liborRates[i][0];
		dm.setData(key, DATA_DOUBLE);

		AQLString zerokey = "ZEROBUMPRISK_" + genCurveName + "_" + "liborRates" + "_" + liborRates[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}

	for (unsigned int i = 0; i < swapRates.size(); i++)
	{
		AQLString key = "SOURCEBUMPRISK_" + genCurveName + "_" + "swapRates" + "_" + swapRates[i][0];
		dm.setData(key, DATA_DOUBLE);

		AQLString zerokey = "ZEROBUMPRISK_" + genCurveName + "_" + "swapRates" + "_" + swapRates[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}

	for (unsigned int i = 0; i < futureRates.size(); i++)
	{
		AQLString key = "SOURCEBUMPRISK_" + genCurveName + "_" + "futureRates" + "_" + futureRates[i][0];
		dm.setData(key, DATA_DOUBLE);

		AQLString zerokey = "ZEROBUMPRISK_" + genCurveName + "_" + "futureRates" + "_" + futureRates[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}


}


void
AQLMultiSwapPricer::setUpStoreOISCurve(AQLDataInstance* dataInstance,
								const AQLString& curveID,
								const AQLString& genCurveName,
								const AQLStringMatrix& generateProp, 
								const AQLStringMatrix& oisRates, 
								const AQLStringMatrix& oisConv,
								const AQLStringMatrix& histRates,
								const AQLStringMatrix& lobasisRates, 
								const AQLStringMatrix& lobasisConv, 
								const AQLStringMatrix& swapRates, 
								const AQLStringMatrix& swapConv)
{

	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLObject* curvestore = NULL;
	// Special Treatment for picking up market name through YieldCurvePro class
	AQLString genCurveNameTmp = genCurveName;
	genCurveNameTmp.toUpper();
	AQLString keyname = "lineparam1_" + curveID + genCurveNameTmp;
	AQLObjectHolder objHolder = objPool.getObject(keyname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		curvestore = new AQLObject();
		objPool.set(keyname, curvestore);
	}
	else
	{
		curvestore = &objHolder.get();
		curvestore->clear();
	}
	
	curvestore->add(CALIBRATION_DATA_NAME, new AQLDataString(keyname));
	curvestore->add("genCurveName", new AQLDataString(genCurveName));
	curvestore->add("generateProp", new AQLDataStringMatrix(generateProp));
	curvestore->add("oisRates", new AQLDataStringMatrix(oisRates));
	curvestore->add("oisConv", new AQLDataStringMatrix(oisConv));
	curvestore->add("histRates", new AQLDataStringMatrix(histRates));
	curvestore->add("lobasisRates", new AQLDataStringMatrix(lobasisRates));
	curvestore->add("lobasisConv", new AQLDataStringMatrix(lobasisConv));
	curvestore->add("swapRates", new AQLDataStringMatrix(swapRates));
	curvestore->add("swapConv", new AQLDataStringMatrix(swapConv));

	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	for (unsigned int i = 0; i < oisRates.size(); i++)
	{
		AQLString key = "SOURCEBUMPRISK_" + genCurveName + "_" + "oisRates" + "_" + oisRates[i][0];
		dm.setData(key, DATA_DOUBLE);

		AQLString zerokey = "ZEROBUMPRISK_" + genCurveName + "_" + "oisRates" + "_" + oisRates[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}
}


void
AQLMultiSwapPricer::shiftMarketRate(AQLStringMatrix& rateMat, double shiftval, bool ispara, unsigned int shiftpos)
{
	//check input mat
	if (rateMat.empty() || rateMat[0].size() < 2)
		throw AQLCoreInvalidData("Rate matrix error",__FILE__,__LINE__);

	for (unsigned int i = 0; i < rateMat.size(); i++)
	{
		if (!ispara)
		{
			//check shiftpos
			if (shiftpos >= rateMat.size())
				throw AQLCoreInvalidData("shift pos error",__FILE__,__LINE__);

			i = shiftpos;
		}

		double orgval = rateMat[i][1].getDoubleValue();
		//special case price flag is only "FF" or "ED", this is not temporary
		if (rateMat[i][0].findString("FF") == -1 && rateMat[i][0].findString("ED") == -1)
		{
			rateMat[i][1] = AQLString(orgval+shiftval);

		}
		else
		{
			//future case
			rateMat[i][1] = AQLString(orgval - shiftval * 100.0);
		}

		if (!ispara)
			return;
	}
}


AQLMathObjectValue* 
AQLMultiSwapPricer::createSingleTrade(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade, AQLString orgtradeid, bool isduplicatemode, AQLString key_suffix, bool istypicalpardeal)
{
	AQLMathObjectValue* ret = NULL;
	AQLDataHolder* dh;
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	

	AQLString copytradeid = orgtradeid;
	if (isduplicatemode)
		copytradeid += key_suffix;

	//important clasee "lineparam1"
	AQLObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	map<AQLString, AQLString>::iterator itmap = maptrade.begin();


	AQLString leg1Input = copytradeid + "_leg1";
	AQLString leg2Input = copytradeid + "_leg2";
	AQLString coupon1Input = leg1Input + "_coupon1";
	AQLString coupon2Input = leg2Input + "_coupon2";
	AQLString index1Input =  coupon1Input + "_index1";
	AQLString index2Input =  coupon2Input + "_index2";

	//common info
	//calc info
	AQLString ccyInput = dynamic_cast<AQLDataString &>(eline.getData("CALC_CURRENCY", ISNOTNULL).get());
	AQLString asofdateInput = dynamic_cast<AQLDataString &>(eline.getData("CALC_ASOFDATE", ISNOTNULL).get());
	AQLString valuedateInput = dynamic_cast<AQLDataString &>(eline.getData("CALC_VALUEDATE", ISNOTNULL).get());
	AQLDate basedate(asofdateInput.getCString(),"YYYYMMDD");

	// necessary in case of index set
	AQLString fcurvestr1m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_1MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr3m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_3MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr6m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_6MFORECAST", ISNOTNULL).get());
	AQLString fcurvestrois = dynamic_cast<AQLDataString &>(eline.getData("CURVE_OISFORECAST", ISNOTNULL).get());
	AQLString curveID = maptrade[orgtradeid + "/" + "CURVEID"];

	dh =&(eline.getData("CURVE_DISCOUNT", NOCHECK));
	AQLString discountcurve;
	if (!dh->isDefined() || dh->isNull())
	{
		discountcurve = getDiscountCurveName(dataInstance, curveID);
	}
	else
	{
		discountcurve = dynamic_cast<AQLDataString &>(eline.getData("CURVE_DISCOUNT", NOCHECK).get());
	}

	//line product
	AQLString lineproduc = maptrade[orgtradeid + "/" + "LineProductType"];
	AQLString ispnl = maptrade[orgtradeid + "/" + "IsPnL"];
	AQLObjectHolder objHolder = objPool.getObject(copytradeid);
	if (objHolder.isDefined())
	{
		if (!objHolder.isTypeOf(ENTITY_VENTITY))
			throw AQLCoreInvalidData("Trans ID Error",__FILE__,__LINE__);
		ret = dynamic_cast<AQLMathObjectValue *>(&objHolder.get());
		ret->reset();
	}
	else
	{
		ret = new AQLMathObjectValue(dataInstance);
		objPool.set(copytradeid, ret);
	}
	//set mintradefile
	//name
	ret->getData(CALIBRATION_DATA_NAME, ISDEFINED).convertFromString(copytradeid);
	//value
	ret->getData(CALIBRATION_DATA_VALUE, ISDEFINED).convertFromString(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
	//isdetailoutput
	ret->AQLObject::add(PRICING_DATA_ISDETAILOUTPUT, new AQLDataBool(true));
	//cfgenerator
	ret->AQLObject::add(PRICING_DATA_CFGENERATOR, new AQLDataProcedure()).convertFromString(FN_IR_CASHFLOWGENERATOR_STR);
	//currency
	ret->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccyInput);
	//settle
	ret->AQLObject::add(PRICING_DATA_SETTLEDATE, new AQLDataDate()).convertFromString(valuedateInput);
	//valuedate
	ret->AQLObject::add(PRICING_DATA_VALUEDATE, new AQLDataDate()).convertFromString(valuedateInput);
	//today
	ret->AQLObject::add(PRICING_DATA_TODAY, new AQLDataDate()).convertFromString(asofdateInput);
	//underlyings
	ret->AQLObject::add(CALIBRATION_DATA_UNDERLYINGS, new AQLDataMultiReference()).convertFromString(leg1Input + AQLString(":") + leg2Input);
	//is result out ---> the same output as webtool
	ret->AQLObject::add(PRICING_DATA_ISRESULTOUTPUT, new AQLDataBool(true));
	//temporary to avoid erro in past trade
	ret->AQLObject::add(PRICING_DATA_ISSAVEPASTFIXING, new AQLDataBool(true));
	//path object
	ret->AQLObject::add(PRICING_DATA_PATHENTITY, new AQLDataReference()).convertFromString("marketparam1_" + curveID);
	//Line product type
	ret->AQLObject::add("LineProductType", new AQLDataString(lineproduc));
	//IsPnL
	ret->AQLObject::add("IsPnL", new AQLDataBool()).convertFromString(ispnl);

	//each single deal
	AQLString tradetypekey = AQLString("TRADE_") + lineproduc + "_";
	//is forward roll
	AQLString isforwardInput = dynamic_cast<AQLDataString &>(eline.getData(tradetypekey + PRICING_DATA_ISFORWARDROLL, ISNOTNULL).get());
	//is forward interpolation
	AQLString isforwardinterInput = dynamic_cast<AQLDataString &>(eline.getData(tradetypekey + PRICING_DATA_ISFWDINTERPOLATION, ISNOTNULL).get());

	//is paymenttiming (priority: trade(leg) > trade > commoninfo)
	AQLString paytimingInput1, paytimingInput2;
	if (maptrade.find(orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING + "/LEG1") != maptrade.end())
		paytimingInput1 = maptrade[orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING + "/LEG1"];
	else if (maptrade.find(orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING) != maptrade.end())
		paytimingInput1 = maptrade[orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING];
	else
		paytimingInput1 = dynamic_cast<AQLDataString &>(eline.getData(tradetypekey + PRICING_DATA_PAYMENTTIMING, ISNOTNULL).get());

	if (maptrade.find(orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING + "/LEG2") != maptrade.end())
		paytimingInput2 = maptrade[orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING + "/LEG2"];
	else if (maptrade.find(orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING) != maptrade.end())
		paytimingInput2 = maptrade[orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING];
	else
		paytimingInput2 = dynamic_cast<AQLDataString &>(eline.getData(tradetypekey + PRICING_DATA_PAYMENTTIMING, ISNOTNULL).get());


	//payment calendar (priority: trade(leg) > trade > commoninfo)
	AQLString leg1calendarInput, leg2calendarInput;
	if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_CALENDAR + "/LEG1") != maptrade.end())
		leg1calendarInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_CALENDAR + "/LEG1"];
	else if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_CALENDAR) != maptrade.end())
		leg1calendarInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_CALENDAR];
	else
		leg1calendarInput = dynamic_cast<AQLDataString &>(eline.getData(tradetypekey + CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());

	if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_CALENDAR + "/LEG2") != maptrade.end())
		leg2calendarInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_CALENDAR + "/LEG2"];
	else if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_CALENDAR) != maptrade.end())
		leg2calendarInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_CALENDAR];
	else
		leg2calendarInput = dynamic_cast<AQLDataString &>(eline.getData(tradetypekey + CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());


	//trade business day convnention (priority: trade(leg) > trade > commoninfo)
	AQLString bdayconvInput1, bdayconvInput2;
	if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/LEG1") != maptrade.end())
		bdayconvInput1 = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/LEG1"];
	else if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE) != maptrade.end())
		bdayconvInput1 = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE];
	else
		bdayconvInput1 = dynamic_cast<AQLDataString &>(eline.getData(tradetypekey + CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());

	if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/LEG2") != maptrade.end())
		bdayconvInput2 = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/LEG2"];
	else if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE) != maptrade.end())
		bdayconvInput2 = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE];
	else
		bdayconvInput2 = dynamic_cast<AQLDataString &>(eline.getData(tradetypekey + CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());

	AQLString leg1indextypeInput = maptrade[orgtradeid + "/" + PRICING_DATA_INDEXTYPE + "/INDEX1"];
	AQLString leg2indextypeInput = maptrade[orgtradeid + "/" + PRICING_DATA_INDEXTYPE + "/INDEX2"];
	AQLString leg1compfreqInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLFREQUENCY + "/LEG1"];
	AQLString leg2compfreqInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLFREQUENCY + "/LEG2"];

	bool isleg1compounding = ("OIS" == leg1indextypeInput || leg1compfreqInput.size() > 0);
	bool isleg2compounding = ("OIS" == leg2indextypeInput || leg2compfreqInput.size() > 0);

	//this is important auto map of OIS index
	if (leg1indextypeInput == "OIS")
		leg1compfreqInput = "BUSINESS_DAYS";

	if (leg2indextypeInput == "OIS")
		leg2compfreqInput = "BUSINESS_DAYS";

	//set minlegfile
	AQLObject* pleg1 = NULL;
	AQLObject* pleg2 = NULL;
	objHolder = objPool.getObject(leg1Input);
	if (objHolder.isDefined())
	{
		pleg1 = &objHolder.get();
		//cf generate
		dh = &(ret->getData(PRICING_DATA_CFGENERATOR,ISDEFINED));
		AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>(dh->get());
		const AQLPriceCashFlowGenerator& cfgen = dynamic_cast<const AQLPriceCashFlowGenerator &>(modelDataObj.getMethod());
		cfgen.clearCashlets(*pleg1);
		pleg1->clear();
	}
	else
	{
		pleg1= new AQLObject();
		objPool.set(leg1Input,pleg1);
	}

	//name 
	pleg1->AQLObject::add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(leg1Input);
	//ccyInput
	pleg1->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccyInput);
	//inputtype
	pleg1->AQLObject::add(PRICING_DATA_INPUTTYPE, new AQLDataString()).convertFromString("Detail");
	//isamortize
	pleg1->AQLObject::add(PRICING_DATA_ISAMORTIZE, new AQLDataBool(false));
	//isnotionalend
	pleg1->AQLObject::add(PRICING_DATA_ISNOTIONALEXCHANGEATEND, new AQLDataBool(false));
	//isnotionalstart
	pleg1->AQLObject::add(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, new AQLDataBool(false));
	//underlyings
	pleg1->AQLObject::add(PRICING_DATA_COUPONINFOS, new AQLDataMultiReference()).convertFromString(coupon1Input);
	//calendar
	pleg1->AQLObject::add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(leg1calendarInput);
	//slidingrule
	AQLString leg1conventionInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/Adjustment/LEG1"];
	//NO_CHANGE means unadjsuted
	if (leg1conventionInput != "NO_CHANGE")
		leg1conventionInput = bdayconvInput1; 
	
	pleg1->AQLObject::add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(leg1conventionInput);
	//slidingrule (strat/end date)
	leg1conventionInput = maptrade[orgtradeid + "/" + PRICING_DATA_CASHFLOWSLIDINGRULE + "/Adjustment/LEG1"];
	//NO_CHANGE means unadjsuted
	if (leg1conventionInput != "NO_CHANGE")
		leg1conventionInput = bdayconvInput1; 
	
	pleg1->AQLObject::add(PRICING_DATA_CASHFLOWSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(leg1conventionInput);
	//daycount
	AQLString leg1daycountInput = maptrade[orgtradeid + "/" + PRICING_DATA_DAYCOUNT + "/LEG1"];
	pleg1->AQLObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(leg1daycountInput);
	//startdate
	AQLString leg1startdateInput = maptrade[orgtradeid + "/" + PRICING_DATA_STARTDATE + "/LEG1"];
	//change string into date 
	if (leg1startdateInput.findString('Y') != -1 || leg1startdateInput.findString('M') != -1 || 
		leg1startdateInput.findString('W') != -1 || leg1startdateInput.findString('D') != -1)
	{
		AQLDate tmpstartdate = AQLMathDateUtilities::getDate(basedate,leg1startdateInput,leg1conventionInput,leg1calendarInput);
		pleg1->AQLObject::add(PRICING_DATA_STARTDATE, new AQLDataDate(tmpstartdate));
	}
	else if (leg1startdateInput.size() == 8)
	{
		pleg1->AQLObject::add(PRICING_DATA_STARTDATE, new AQLDataDate()).convertFromString(leg1startdateInput);
	}
	else 
		throw AQLCoreInvalidData("Leg1 StartDate input error",__FILE__,__LINE__);

	//enddate
	//change string into date
	AQLString leg1enddateInput = maptrade[orgtradeid + "/" + PRICING_DATA_ENDDATE + "/LEG1"];
	if (leg1enddateInput.findString('Y') != -1 || leg1enddateInput.findString('M') != -1 || 
		leg1enddateInput.findString('W') != -1 || leg1enddateInput.findString('D') != -1)
	{
		AQLDate startdate = dynamic_cast<AQLDataDate &>(pleg1->getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
		AQLDate tmpenddate = AQLMathDateUtilities::getDate(startdate,leg1enddateInput,leg1conventionInput,leg1calendarInput);
		pleg1->AQLObject::add(PRICING_DATA_ENDDATE, new AQLDataDate(tmpenddate));
	}
	else if (leg1enddateInput.size() == 8)
	{
		pleg1->AQLObject::add(PRICING_DATA_ENDDATE, new AQLDataDate()).convertFromString(leg1enddateInput);
	}
	else 
		throw AQLCoreInvalidData("Leg1 EndDate input error",__FILE__,__LINE__);

	//frequency
	AQLString leg1fregInput = maptrade[orgtradeid + "/" + PRICING_DATA_FREQUENCY + "/LEG1"];
	pleg1->AQLObject::add(PRICING_DATA_FREQUENCY, new AQLDataString()).convertFromString(leg1fregInput);
	//rollconvention
	AQLString leg1rollconvInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLCONVENTION + "/LEG1"];
	if(leg1rollconvInput != "")
		pleg1->AQLObject::add(PRICING_DATA_ROLLCONVENTION, new AQLDataString()).convertFromString(leg1rollconvInput);
	//notional
	AQLString leg1notionalInput = maptrade[orgtradeid + "/" + PRICING_CALIBRATION_DATAOTIONAL + "/LEG1"];
	pleg1->AQLObject::add(PRICING_CALIBRATION_DATAOTIONAL, new AQLDataDouble()).convertFromString(leg1notionalInput);
	//paymenttiming
	pleg1->AQLObject::add(PRICING_DATA_PAYMENTTIMING, new AQLDataString()).convertFromString(paytimingInput1);
	//selectside
	AQLString leg1PRInput = maptrade[orgtradeid + "/" + PRICING_DATA_SELECTSIDE + "/LEG1"];
	pleg1->AQLObject::add(PRICING_DATA_SELECTSIDE, new AQLDataString()).convertFromString(leg1PRInput);
	//coupondays
	AQLString leg1cdayInput = maptrade[orgtradeid + "/" + PRICING_DATA_COUPONDAY + "/LEG1"];
	pleg1->AQLObject::add(PRICING_DATA_COUPONDAY, new AQLDataInt()).convertFromString(leg1cdayInput);
	//isforwardroll
	AQLString leg1isforwardInput=maptrade[orgtradeid + "/" + PRICING_DATA_ISFORWARDROLL + "/LEG1"];
	if(leg1isforwardInput != "")	//if maptrade contains PRICING_DATA_ISFORWARDROLL, overwittern
		isforwardInput = leg1isforwardInput;
	pleg1->AQLObject::add(PRICING_DATA_ISFORWARDROLL, new AQLDataBool()).convertFromString(isforwardInput);
	//discountcurve
	pleg1->AQLObject::add(PRICING_DATA_DISCOUNTCURVE, new AQLDataString()).convertFromString(discountcurve);

	//AQLString leg1compfreqInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLFREQUENCY + "/LEG1"];
	pleg1->AQLObject::add(PRICING_DATA_ROLLFREQUENCY, new AQLDataString()).convertFromString(leg1compfreqInput);
	
	//UpfrontFees	
	//UpfrontPaymentDates
	if (!istypicalpardeal)
	{
		itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_UPFRONTFEES);
		if (itmap != maptrade.end())
		{
			double upfee = itmap->second.getDoubleValue();
			//even in case of blankd upfee itmap->second.getDoubleValue(); = 0.0;
			if (upfee != 0.0)
			{
				if (leg1PRInput == "PAY")
					upfee *= -1;

				AQLDate tmppaydate = dynamic_cast<AQLDataDate &>(pleg1->getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
				
				//upfront payment is always spot
				DoubleVector upfeevec(1,upfee);
				pleg1->AQLObject::add(PRICING_DATA_UPFRONTFEES, new AQLDataDoubles(upfeevec));
				DateVector payvec(1,tmppaydate);
				pleg1->AQLObject::add(PRICING_DATA_UPFRONTPAYMENTDATES, new AQLDataDates(payvec));
			}
		}
	}
	//firstodddate
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIRSTODDDATE + "/LEG1");
	if(itmap != maptrade.end()){
		AQLString leg1firstodddateInput = itmap->second;
		pleg1->AQLObject::add(PRICING_DATA_FIRSTODDDATE, new AQLDataDate()).convertFromString(leg1firstodddateInput);
	}
	//lastodddate
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_LASTODDDATE + "/LEG1");
	if(itmap != maptrade.end()){
		AQLString leg1lastodddateInput = itmap->second;
		pleg1->AQLObject::add(PRICING_DATA_LASTODDDATE, new AQLDataDate()).convertFromString(leg1lastodddateInput);
	}
	//firsoddindextype
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIRSTODDINDEXTYPE + "/LEG1");
	if(itmap != maptrade.end()){
		AQLString leg1firstoddindexInput = itmap->second;
		pleg1->AQLObject::add(PRICING_DATA_FIRSTODDINDEXTYPE, new AQLDataString()).convertFromString(leg1firstoddindexInput);
	}
	//lastoddindextype
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_LASTODDINDEXTYPE + "/LEG1");
	if(itmap != maptrade.end()){
		AQLString leg1firstoddindexInput = itmap->second;
		pleg1->AQLObject::add(PRICING_DATA_LASTODDINDEXTYPE, new AQLDataString()).convertFromString(leg1firstoddindexInput);
	}
	

	objHolder = objPool.getObject(leg2Input);
	if (objHolder.isDefined())
	{
		pleg2 = &objHolder.get();
		//pleg2->clear();
		//cf generate
		dh = &(ret->getData(PRICING_DATA_CFGENERATOR,ISDEFINED));
		AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>(dh->get());
		const AQLPriceCashFlowGenerator& cfgen = dynamic_cast<const AQLPriceCashFlowGenerator &>(modelDataObj.getMethod());
		cfgen.clearCashlets(*pleg2);
		pleg2->clear();
	}
	else
	{
		pleg2= new AQLObject();
		objPool.set(leg2Input,pleg2);
	}

	//name 
	pleg2->AQLObject::add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(leg2Input);
	//ccyInput
	pleg2->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccyInput);
	//inputtype
	pleg2->AQLObject::add(PRICING_DATA_INPUTTYPE, new AQLDataString()).convertFromString("Detail");
	//isamortize
	pleg2->AQLObject::add(PRICING_DATA_ISAMORTIZE, new AQLDataBool(false));
	//isnotionalend
	pleg2->AQLObject::add(PRICING_DATA_ISNOTIONALEXCHANGEATEND, new AQLDataBool(false));
	//isnotionalstart
	pleg2->AQLObject::add(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, new AQLDataBool(false));
	//underlyings
	pleg2->AQLObject::add(PRICING_DATA_COUPONINFOS, new AQLDataMultiReference()).convertFromString(coupon2Input);
	//calendar
	pleg2->AQLObject::add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(leg2calendarInput);
	//slidingrule (payment)
	AQLString leg2conventionInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/Adjustment/LEG2"];
	//NO_CHANGE means unadjsuted
	if (leg2conventionInput != "NO_CHANGE")
		leg2conventionInput = bdayconvInput2; 

	pleg2->AQLObject::add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(leg2conventionInput);
	//slidingrule (strat/end date)
	leg2conventionInput = maptrade[orgtradeid + "/" + PRICING_DATA_CASHFLOWSLIDINGRULE + "/Adjustment/LEG2"];
	//NO_CHANGE means unadjsuted
	if (leg2conventionInput != "NO_CHANGE")
		leg2conventionInput = bdayconvInput2; 
	
	pleg2->AQLObject::add(PRICING_DATA_CASHFLOWSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(leg2conventionInput);

	//daycount
	AQLString leg2daycountInput = maptrade[orgtradeid + "/" + PRICING_DATA_DAYCOUNT + "/LEG2"];
	pleg2->AQLObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(leg2daycountInput);
	
	//startdate
	AQLString leg2startdateInput = maptrade[orgtradeid + "/" + PRICING_DATA_STARTDATE + "/LEG2"];
	//change string into date 
	if (leg2startdateInput.findString('Y') != -1 || leg2startdateInput.findString('M') != -1 || 
		leg2startdateInput.findString('W') != -1 || leg2startdateInput.findString('D') != -1)
	{
		AQLDate tmpstartdate = AQLMathDateUtilities::getDate(basedate,leg2startdateInput,leg2conventionInput,leg2calendarInput);
		pleg2->AQLObject::add(PRICING_DATA_STARTDATE, new AQLDataDate(tmpstartdate));
	}
	else if (leg2startdateInput.size() == 8)
	{
		pleg2->AQLObject::add(PRICING_DATA_STARTDATE, new AQLDataDate()).convertFromString(leg2startdateInput);
	}
	else 
		throw AQLCoreInvalidData("Leg2 StartDate input error",__FILE__,__LINE__);

	//enddate
	//change string into date
	AQLString leg2enddateInput = maptrade[orgtradeid + "/" + PRICING_DATA_ENDDATE + "/LEG2"];
	if (leg2enddateInput.findString('Y') != -1 || leg2enddateInput.findString('M') != -1 || 
		leg2enddateInput.findString('W') != -1 || leg2enddateInput.findString('D') != -1)
	{
		AQLDate startdate = dynamic_cast<AQLDataDate &>(pleg2->getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
		AQLDate tmpenddate = AQLMathDateUtilities::getDate(startdate,leg2enddateInput,leg2conventionInput,leg2calendarInput);
		pleg2->AQLObject::add(PRICING_DATA_ENDDATE, new AQLDataDate(tmpenddate));
	}
	else if (leg2enddateInput.size() == 8)
	{
		pleg2->AQLObject::add(PRICING_DATA_ENDDATE, new AQLDataDate()).convertFromString(leg2enddateInput);
	}
	else 
		throw AQLCoreInvalidData("Leg2 EndDate input error",__FILE__,__LINE__);

	//frequency
	AQLString leg2fregInput = maptrade[orgtradeid + "/" + PRICING_DATA_FREQUENCY + "/LEG2"];
	pleg2->AQLObject::add(PRICING_DATA_FREQUENCY, new AQLDataString()).convertFromString(leg2fregInput);
	//roll convention
	AQLString leg2rollconvInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLCONVENTION + "/LEG2"];
	if(leg2rollconvInput != "")
		pleg2->AQLObject::add(PRICING_DATA_ROLLCONVENTION, new AQLDataString()).convertFromString(leg2rollconvInput);
	//notional
	AQLString leg2notionalInput = maptrade[orgtradeid + "/" + PRICING_CALIBRATION_DATAOTIONAL + "/LEG2"];
	pleg2->AQLObject::add(PRICING_CALIBRATION_DATAOTIONAL, new AQLDataDouble()).convertFromString(leg2notionalInput);
	//paymenttiming
	pleg2->AQLObject::add(PRICING_DATA_PAYMENTTIMING, new AQLDataString()).convertFromString(paytimingInput2);
	//selectside
	AQLString leg2PRInput = maptrade[orgtradeid + "/" + PRICING_DATA_SELECTSIDE + "/LEG2"];
	pleg2->AQLObject::add(PRICING_DATA_SELECTSIDE, new AQLDataString()).convertFromString(leg2PRInput);
	//coupondays
	AQLString leg2cdayInput = maptrade[orgtradeid + "/" + PRICING_DATA_COUPONDAY + "/LEG2"];
	pleg2->AQLObject::add(PRICING_DATA_COUPONDAY, new AQLDataInt()).convertFromString(leg2cdayInput);
	//isforwardroll 
	AQLString leg2isforwardInput=maptrade[orgtradeid + "/" + PRICING_DATA_ISFORWARDROLL + "/LEG2"];
	if(leg2isforwardInput != "")	//if maptrade contains PRICING_DATA_ISFORWARDROLL, overwittern
		isforwardInput = leg2isforwardInput;
	pleg2->AQLObject::add(PRICING_DATA_ISFORWARDROLL, new AQLDataBool()).convertFromString(isforwardInput);
	//discountcurve
	pleg2->AQLObject::add(PRICING_DATA_DISCOUNTCURVE, new AQLDataString()).convertFromString(discountcurve);

	//AQLString leg2compfreqInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLFREQUENCY + "/LEG2"];
	pleg2->AQLObject::add(PRICING_DATA_ROLLFREQUENCY, new AQLDataString()).convertFromString(leg2compfreqInput);

	//firstodddate
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIRSTODDDATE + "/LEG2");
	if(itmap != maptrade.end()){
		AQLString leg2firstodddateInput = itmap->second;
		pleg2->AQLObject::add(PRICING_DATA_FIRSTODDDATE, new AQLDataDate()).convertFromString(leg2firstodddateInput);
	}
	//lastodddate
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_LASTODDDATE + "/LEG2");
	if(itmap != maptrade.end()){
		AQLString leg2lastodddateInput = itmap->second;
		pleg2->AQLObject::add(PRICING_DATA_LASTODDDATE, new AQLDataDate()).convertFromString(leg2lastodddateInput);
	}
	//firsoddindextype
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIRSTODDINDEXTYPE + "/LEG2");
	if(itmap != maptrade.end()){
		AQLString leg2firstoddindexInput = itmap->second;
		pleg2->AQLObject::add(PRICING_DATA_FIRSTODDINDEXTYPE, new AQLDataString()).convertFromString(leg2firstoddindexInput);
	}
	//lastoddindextype
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_LASTODDINDEXTYPE + "/LEG2");
	if(itmap != maptrade.end()){
		AQLString leg2firstoddindexInput = itmap->second;
		pleg2->AQLObject::add(PRICING_DATA_LASTODDINDEXTYPE, new AQLDataString()).convertFromString(leg2firstoddindexInput);
	}
	//UpfrontFees	
	//UpfrontPaymentDates

	//set mincouponfile
	AQLObject* pcoupon1 = NULL;
	AQLObject* pcoupon2 = NULL;
	
	objHolder = objPool.getObject(coupon1Input);
	if (objHolder.isDefined())
	{
		pcoupon1 = &objHolder.get();
		pcoupon1->clear();
	}
	else
	{
		pcoupon1= new AQLObject();
		objPool.set(coupon1Input,pcoupon1);
	}

	//name
	pcoupon1->AQLObject::add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(coupon1Input);
	//ccyInput
	pcoupon1->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccyInput);
	//operator
	pcoupon1->AQLObject::add(PRICING_DATA_OPERATOR, new AQLPriceDataFunction()).convertFromString(FN_LINEAR_STR);
	//underlyings
	pcoupon1->AQLObject::add(PRICING_DATA_INDEXINFOS, new AQLDataMultiReference()).convertFromString(index1Input);
	//coeff
	if (!istypicalpardeal)
	{
		AQLString leg1coeffInput = maptrade[orgtradeid + "/" + PRICING_DATA_COEFFICIENT + "/COUPON1"];
		//if (!isleg1compounding)
		pcoupon1->AQLObject::add(PRICING_DATA_COEFFICIENT, new AQLDataDoubles()).convertFromString(leg1coeffInput);
		/*else
			pcoupon1->AQLObject::add(PRICING_DATA_COEFFICIENT, new AQLDataDoubles()).convertFromString("1.0:0.0");*/
	}
	// requirement by Lenny
	else
	{
		pcoupon1->AQLObject::add(PRICING_DATA_COEFFICIENT, new AQLDataDoubles()).convertFromString("1.0:0.0");
	}


	objHolder = objPool.getObject(coupon2Input);
	if (objHolder.isDefined())
	{
		pcoupon2 = &objHolder.get();
		pcoupon2->clear();
	}
	else
	{
		pcoupon2= new AQLObject();
		objPool.set(coupon2Input,pcoupon2);
	}

	//name
	pcoupon2->AQLObject::add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(coupon2Input);
	//ccyInput
	pcoupon2->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccyInput);
	//operator
	pcoupon2->AQLObject::add(PRICING_DATA_OPERATOR, new AQLPriceDataFunction()).convertFromString(FN_LINEAR_STR);
	//underlyings
	pcoupon2->AQLObject::add(PRICING_DATA_INDEXINFOS, new AQLDataMultiReference()).convertFromString(index2Input);
	//coeff
	if (!istypicalpardeal)
	{
		AQLString leg2coeffInput = maptrade[orgtradeid + "/" + PRICING_DATA_COEFFICIENT + "/COUPON2"];
		//if (!isleg2compounding)
		pcoupon2->AQLObject::add(PRICING_DATA_COEFFICIENT, new AQLDataDoubles()).convertFromString(leg2coeffInput);
		/*else
			pcoupon2->AQLObject::add(PRICING_DATA_COEFFICIENT, new AQLDataDoubles()).convertFromString("1.0:0.0");*/
	}
	// requirement by Lenny
	else
	{
		pcoupon2->AQLObject::add(PRICING_DATA_COEFFICIENT, new AQLDataDoubles()).convertFromString("1.0:0.0");
	}

	//set minindexfile
	AQLObject* pindex2 = NULL;
	AQLObject* pindex1 = NULL;


	//counpon1 index setting --------------------------------  

	objHolder = objPool.getObject(index1Input);
	if (objHolder.isDefined())
	{
		pindex1 = &objHolder.get();
		pindex1->clear();
	}
	else
	{
		pindex1= new AQLObject();
		objPool.set(index1Input,pindex1);
	}

	//name
	pindex1->AQLObject::add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(index1Input);
	//ccyInput
	pindex1->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccyInput);

	//indextype 
	//if "" then LIBOR 
	AQLString leg1acessoryInput = maptrade[orgtradeid + "/" + PRICING_DATA_ACCESSORY + "/INDEX1"];
	if (leg1indextypeInput == "OIS")
	{
		//in case of OIS we must change offset,spotlag as 0 and Accessory must be 1D
		if (leg1acessoryInput.toUpper() != "1D")
			throw AQLCoreInvalidData("ois, ff index set error",__FILE__,__LINE__);

	}
	else if (leg1indextypeInput == "FIXEDRATE" || leg1indextypeInput.size() < 1)
	{
		leg1indextypeInput = "FIXEDRATE";
	}
	else if (leg1indextypeInput != "LIBOR")
		throw AQLCoreInvalidData("index set error",__FILE__,__LINE__);

	if (isleg1compounding)
	{
		AQLString leg1coeffInput = maptrade[orgtradeid + "/" + PRICING_DATA_COEFFICIENT + "/COUPON1"];
		AQLStringVector tmp = leg1coeffInput.toToken(':');
		if (tmp.size() != 2) 
			throw AQLCoreInvalidData("coefficient error", __FILE__,__LINE__);

		//pindex1->AQLObject::add(PRICING_DATA_COMPOUNDINGMARGIN, new AQLDataDouble(tmp[1].getDoubleValue()));

		AQLString leg1compcalcInput = maptrade[orgtradeid + "/" + PRICING_DATA_OBSERVATIONOPERATOR + "/INDEX1"];
		if (leg1compcalcInput == "COMPOUNDING")
		{
			pleg1->AQLObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new AQLPriceDataFunction()).convertFromString(FN_COMPOUNDING1_STR);
		}
		else if (leg1compcalcInput == "FLATCOMPOUNDING")
		{
			pleg1->AQLObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new AQLPriceDataFunction()).convertFromString(FN_COMPOUNDING8_STR);
		}
		else if (leg1compcalcInput == "DAILYAVERAGING")
		{
			pleg1->AQLObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new AQLPriceDataFunction()).convertFromString(FN_COMPOUNDING10_STR);
			pleg1->AQLObject::add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new AQLDataBool(true));
		}
		else if (leg1compcalcInput == "AVERAGING")
		{
			pleg1->AQLObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new AQLPriceDataFunction()).convertFromString(FN_COMPOUNDING10_STR);
			pleg1->AQLObject::add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new AQLDataBool(false));
		}
		else
			throw AQLCoreInvalidData("compounding function error",__FILE__,__LINE__);

        pleg1->add(PRICING_DATA_ISCOMPOUNDINGCOUPON, new AQLDataBool(true));
	}

	//each single deal
	AQLString leg1indextypekey = AQLString("INDEX_") + ccyInput + "_" + leg1indextypeInput + "_";
	AQLString leg1offset,leg1fixingtimingInput,leg1fixingcalendarInput;
	if (leg1indextypeInput != "FIXEDRATE")
	{
		//spot lag
		leg1offset = dynamic_cast<AQLDataString &>(eline.getData(leg1indextypekey + PRICING_DATA_SPOTLAG, ISNOTNULL).get());
		//fixing timing
		leg1fixingtimingInput = dynamic_cast<AQLDataString &>(eline.getData(leg1indextypekey + PRICING_DATA_FIXINGTIMING, ISNOTNULL).get());
		//fixing calendar
		if (maptrade.find(orgtradeid + "/" + leg1indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR + "/LEG1") != maptrade.end())
			leg1fixingcalendarInput = maptrade[orgtradeid + "/" + leg1indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR + "/LEG1"];
		else if (maptrade.find(orgtradeid + "/" + leg1indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR) != maptrade.end())
			leg1fixingcalendarInput = maptrade[orgtradeid + "/" + leg1indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR];
		else
			leg1fixingcalendarInput = dynamic_cast<AQLDataString &>(eline.getData(leg1indextypekey + PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get());
	}

	////daycount
	pindex1->AQLObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(leg1daycountInput);
	
	//accessory
	if (leg1indextypeInput != "FIXEDRATE")
	{
		pindex1->AQLObject::add(PRICING_DATA_ACCESSORY, new AQLDataString()).convertFromString(leg1acessoryInput);
		//calendar
		pindex1->AQLObject::add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(leg1fixingcalendarInput);
		//off set
		pindex1->AQLObject::add(PRICING_DATA_OFFSET, new AQLDataInt()).convertFromString(leg1offset);
		////spot lag
		pindex1->AQLObject::add(PRICING_DATA_SPOTLAG, new AQLDataInt()).convertFromString(leg1offset);
		//fixing timing
		pindex1->AQLObject::add(PRICING_DATA_FIXINGTIMING, new AQLDataString()).convertFromString(leg1fixingtimingInput);
		//
		pindex1->AQLObject::add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString("FOLLOWING");
		////fixing slidingrule
		pindex1->AQLObject::add(PRICING_DATA_FIXINGSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString("FOLLOWING");
		//fixing calendar
		pindex1->AQLObject::add(PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(leg1fixingcalendarInput);

		//basiscurve 
		AQLString leg1forcuaveInput;
		if ("OIS" == leg1indextypeInput)
			leg1forcuaveInput = fcurvestrois;
		else if ("1M" == leg1acessoryInput)
			leg1forcuaveInput = fcurvestr1m;
		else if ("3M" == leg1acessoryInput)
			leg1forcuaveInput = fcurvestr3m;
		else if ("6M" == leg1acessoryInput)
			leg1forcuaveInput = fcurvestr6m;
		else if ("1Y" == leg1acessoryInput)
			leg1forcuaveInput = fcurvestr6m;
		else
			throw AQLCoreInvalidData("Index Curve Set Error",__FILE__,__LINE__);
		
		pindex1->AQLObject::add(PRICING_DATA_BASISCURVE, new AQLDataString()).convertFromString(leg1forcuaveInput);
		//basisinterpolation
		pindex1->AQLObject::add(PRICING_DATA_BASISINTERPOLATION, new AQLDataString()).convertFromString(FN_LINEARINTERPOLATION_STR);
	}
	
	if (leg1indextypeInput == "LIBOR")
	{
		//is fwd intepolation
		pindex1->AQLObject::add(PRICING_DATA_ISFWDINTERPOLATION, new AQLDataBool()).convertFromString(isforwardinterInput);
		//fwd intepolation 
		pindex1->AQLObject::add(PRICING_DATA_FWDINTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(FN_SPLINEINTERPOLATION_STR);
		//frn
		pindex1->AQLObject::add(PRICING_DATA_INDEXGENERATEMETHOD, new AQLDataString()).convertFromString("FRN");
	}

	//fixedrate
	if (!istypicalpardeal)
	{
		if (lineproduc == "SWAP" || lineproduc == "FRA")
		{
			itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIXEDRATE + "/INDEX1");
			if (itmap != maptrade.end())
			{
				//fixed rate case
				AQLString leg1rateInput = itmap->second;
				pindex1->AQLObject::add(PRICING_DATA_FIXEDRATE, new AQLDataDouble()).convertFromString(leg1rateInput);

				//index type
				pindex1->AQLObject::add(PRICING_DATA_INDEXTYPE, new AQLDataString()).convertFromString("FIXEDRATE");

				//index name
				pleg1->AQLObject::add(PRICING_DATA_INDEXNAME, new AQLDataString()).convertFromString("FIXEDRATE");
			}
			else 
				throw AQLCoreInvalidData("fixed rate set error",__FILE__,__LINE__);
		}
		else if (lineproduc == "BASISSWAP")
		{
			//basis case
			////libor freq
			pindex1->AQLObject::add(PRICING_DATA_FREQUENCY, new AQLDataString()).convertFromString("SIMPLE");
			//inex type
			pindex1->AQLObject::add(PRICING_DATA_INDEXTYPE, new AQLDataString()).convertFromString(leg1indextypeInput);
			//index name
			pleg1->AQLObject::add(PRICING_DATA_INDEXNAME, new AQLDataString()).convertFromString(ccyInput.toUpper() + "_" + leg1indextypeInput);
		}
		else 
			throw AQLCoreInvalidData("product error",__FILE__,__LINE__);
	}
	else
	{
		//
		if (lineproduc == "SWAP" || lineproduc == "FRA")
		{
			itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIXEDRATE + "/INDEX1");
			AQLString leg1rateInput = itmap->second;
			if (itmap != maptrade.end())
				pindex1->AQLObject::add(PRICING_DATA_FIXEDRATE, new AQLDataDouble()).convertFromString(leg1rateInput);
			else
				pindex1->AQLObject::add(PRICING_DATA_FIXEDRATE, new AQLDataDouble()).convertFromString("0.01");

			//inex type
			pindex1->AQLObject::add(PRICING_DATA_INDEXTYPE, new AQLDataString()).convertFromString("FIXEDRATE");
			//inex name
			pleg1->AQLObject::add(PRICING_DATA_INDEXNAME, new AQLDataString()).convertFromString("FIXEDRATE");
		}
		else if (lineproduc == "BASISSWAP")
		{
			//basis case
			////libor freq
			pindex1->AQLObject::add(PRICING_DATA_FREQUENCY, new AQLDataString()).convertFromString("SIMPLE");
			//inex type
			pindex1->AQLObject::add(PRICING_DATA_INDEXTYPE, new AQLDataString()).convertFromString(leg1indextypeInput);
			//inex name
			pleg1->AQLObject::add(PRICING_DATA_INDEXNAME, new AQLDataString()).convertFromString(ccyInput.toUpper() + "_" + leg1indextypeInput);
		}
		else 
			throw AQLCoreInvalidData("product error",__FILE__,__LINE__);
	}
	
	//first fixing rate
	//first fixing basedate
	if (!istypicalpardeal)
	{
	
		itmap = maptrade.find(orgtradeid + "/" + "FirstFixingRate" + "/INDEX1");
		if (itmap != maptrade.end())
		{
			AQLString leg1firstfixngrateInput = itmap->second;
			pindex1->AQLObject::add(PRICING_DATA_FIRSTFIXINGRATE, new AQLDataDouble()).convertFromString(leg1firstfixngrateInput);
			pindex1->AQLObject::add(PRICING_DATA_FIRSTFIXINGBASEDATE, new AQLDataDate()).convertFromString(asofdateInput);
		}
	}

	//add oddindexinfo and generate oddindexinfo incase there are firstodddate or lastodddate in paymentdates
	AQLDataHolder* ah_temp1 = &(pleg1->getData(PRICING_DATA_FIRSTODDDATE, NOCHECK));
	AQLDataHolder* ah_temp2 = &(pleg1->getData(PRICING_DATA_LASTODDDATE, NOCHECK));
	if(leg1indextypeInput == "LIBOR" && (ah_temp1->isDefined() || ah_temp2->isDefined()))
	{
		// we must add two libor index except index of trade for stub interpolation.
		vector<AQLString> oddIndex1Input(2);
		oddIndex1Input[0] = coupon1Input + "_oddIndex1";
		oddIndex1Input[1] = coupon1Input + "_oddIndex2";
		pcoupon1->AQLObject::add(PRICING_DATA_ODDINDEXINFOS, new AQLDataMultiReference()).convertFromString(oddIndex1Input[0] + ":" + oddIndex1Input[1]);

		vector<AQLString> accessory(2), basisCurve(2);
		if("1M" == leg1acessoryInput)
		{
			accessory[0] = "3M";
			accessory[1] = "6M";
			basisCurve[0] = fcurvestr3m;
			basisCurve[1] = fcurvestr6m;
		}
		else if("3M" == leg1acessoryInput)
		{
			accessory[0] = "1M";
			accessory[1] = "6M";
			basisCurve[0] = fcurvestr1m;
			basisCurve[1] = fcurvestr6m;
		}
		else if("6M" == leg1acessoryInput || "1Y" == leg1acessoryInput)
		{
			accessory[0] = "1M";
			accessory[1] = "3M";
			basisCurve[0] = fcurvestr1m;
			basisCurve[1] = fcurvestr3m;
		}
		else
			throw AQLCoreInvalidData("invaild accessory type for stub interpolation.",__FILE__,__LINE__);

		for (unsigned int i = 0; i < oddIndex1Input.size(); i++ )
		{
			AQLObject* pindex1 = NULL;
			objHolder = objPool.getObject(oddIndex1Input[i]);
			if (objHolder.isDefined())
			{
				pindex1 = &objHolder.get();
				pindex1->clear();
			}
			else
			{
				pindex1= new AQLObject();
				objPool.set(oddIndex1Input[i], pindex1);
			}

			//name
			pindex1->AQLObject::add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(oddIndex1Input[i]);
			//ccyInput
			pindex1->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccyInput);
			////daycount
			pindex1->AQLObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(leg1daycountInput);
			//accessory
			pindex1->AQLObject::add(PRICING_DATA_ACCESSORY, new AQLDataString()).convertFromString(accessory[i]);
			//calendar
			pindex1->AQLObject::add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(leg1fixingcalendarInput);
			//off set
			pindex1->AQLObject::add(PRICING_DATA_OFFSET, new AQLDataInt()).convertFromString(leg1offset);
			////spot lag
			pindex1->AQLObject::add(PRICING_DATA_SPOTLAG, new AQLDataInt()).convertFromString(leg1offset);
			//fixing timing
			pindex1->AQLObject::add(PRICING_DATA_FIXINGTIMING, new AQLDataString()).convertFromString(leg1fixingtimingInput);
			//
			pindex1->AQLObject::add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString("FOLLOWING");
			////fixing slidingrule
			pindex1->AQLObject::add(PRICING_DATA_FIXINGSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString("FOLLOWING");
			//fixing calendar
			pindex1->AQLObject::add(PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(leg1fixingcalendarInput);
			//fwd intepolation 
			pindex1->AQLObject::add(PRICING_DATA_ISFWDINTERPOLATION, new AQLDataBool()).convertFromString(isforwardinterInput);
			pindex1->AQLObject::add(PRICING_DATA_FWDINTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(FN_SPLINEINTERPOLATION_STR);
			//frn
			pindex1->AQLObject::add(PRICING_DATA_INDEXGENERATEMETHOD, new AQLDataString()).convertFromString("FRN");
			////libor freq
			pindex1->AQLObject::add(PRICING_DATA_FREQUENCY, new AQLDataString()).convertFromString("SIMPLE");
			//inex type
			pindex1->AQLObject::add(PRICING_DATA_INDEXTYPE, new AQLDataString()).convertFromString(leg1indextypeInput);
			////index name
			//pleg1->AQLObject::add(PRICING_DATA_INDEXNAME, new AQLDataString()).convertFromString(ccyInput.toUpper() + "_" + leg1indextypeInput);
			//basiscurve
			pindex1->AQLObject::add(PRICING_DATA_BASISCURVE, new AQLDataString()).convertFromString(basisCurve[i]);
			//basisinterpolation
			pindex1->AQLObject::add(PRICING_DATA_BASISINTERPOLATION, new AQLDataString()).convertFromString(FN_LINEARINTERPOLATION_STR);

		}

	}

	//counpon1 index setting --------------------------------  

	objHolder = objPool.getObject(index2Input);
	if (objHolder.isDefined())
	{
		pindex2 = &objHolder.get();
		pindex2->clear();
	}
	else
	{
		pindex2= new AQLObject();
		objPool.set(index2Input,pindex2);
	}

	//name
	pindex2->AQLObject::add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(index2Input);
	//ccyInput
	pindex2->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccyInput);

	//indextype 
	//if "" then LIBOR 
	AQLString leg2acessoryInput = maptrade[orgtradeid + "/" + PRICING_DATA_ACCESSORY + "/INDEX2"];
	if (leg2indextypeInput == "OIS")
	{
		//in case of OIS we must change offset,spotlag as 0 and Accessory must be 1D
		if (leg2acessoryInput.toUpper() != "1D")
			throw AQLCoreInvalidData("index set error",__FILE__,__LINE__);
	}
	else if (leg2indextypeInput == "FIXEDRATE" || leg2indextypeInput.size() < 1)
	{
		throw AQLCoreInvalidData("index set error",__FILE__,__LINE__);
	}
	else if (leg2indextypeInput != "LIBOR")
		throw AQLCoreInvalidData("index set error",__FILE__,__LINE__);

	if (isleg2compounding)
	{
		AQLString leg2coeffInput = maptrade[orgtradeid + "/" + PRICING_DATA_COEFFICIENT + "/COUPON2"];
		AQLStringVector tmp = leg2coeffInput.toToken(':');
		if (tmp.size() != 2) 
			throw AQLCoreInvalidData("coefficient error", __FILE__,__LINE__);

		//pindex2->AQLObject::add(PRICING_DATA_COMPOUNDINGMARGIN, new AQLDataDouble(tmp[1].getDoubleValue()));

		AQLString leg2compcalcInput = maptrade[orgtradeid + "/" + PRICING_DATA_OBSERVATIONOPERATOR + "/INDEX2"];
		if (leg2compcalcInput == "COMPOUNDING")
			pleg2->AQLObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new AQLPriceDataFunction()).convertFromString(FN_COMPOUNDING1_STR);
		else if (leg2compcalcInput == "DAILYAVERAGING")
		{
			pleg2->AQLObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new AQLPriceDataFunction()).convertFromString(FN_COMPOUNDING10_STR);
			pleg2->AQLObject::add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new AQLDataBool(true));
		}
		else if (leg2compcalcInput == "AVERAGING")
		{
			pleg2->AQLObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new AQLPriceDataFunction()).convertFromString(FN_COMPOUNDING10_STR);
			pleg2->AQLObject::add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new AQLDataBool(false));
		}
		else
			throw AQLCoreInvalidData("compounding function error",__FILE__,__LINE__);

        pleg2->add(PRICING_DATA_ISCOMPOUNDINGCOUPON, new AQLDataBool(true));
	}



	//each single deal
	AQLString leg2indextypekey = AQLString("INDEX_") + ccyInput + "_" + leg2indextypeInput + "_";
	//spot lag
	AQLString leg2offset = dynamic_cast<AQLDataString &>(eline.getData(leg2indextypekey + PRICING_DATA_SPOTLAG, ISNOTNULL).get());
	//fixing timing
	AQLString leg2fixingtimingInput = dynamic_cast<AQLDataString &>(eline.getData(leg2indextypekey + PRICING_DATA_FIXINGTIMING, ISNOTNULL).get());
	//fixing calendar
	AQLString leg2fixingcalendarInput;
	if (maptrade.find(orgtradeid + "/" + leg2indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR + "/LEG2") != maptrade.end())
		leg2fixingcalendarInput = maptrade[orgtradeid + "/" + leg2indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR + "/LEG2"];
	else if (maptrade.find(orgtradeid + "/" + leg2indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR) != maptrade.end())
		leg2fixingcalendarInput = maptrade[orgtradeid + "/" + leg2indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR];
	else
		leg2fixingcalendarInput = dynamic_cast<AQLDataString &>(eline.getData(leg2indextypekey + PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get());
	//accessory
	pindex2->AQLObject::add(PRICING_DATA_ACCESSORY, new AQLDataString()).convertFromString(leg2acessoryInput);
	//calendar
	pindex2->AQLObject::add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(leg2fixingcalendarInput);
	//daycount
	pindex2->AQLObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(leg2daycountInput);
	//libor freq
	pindex2->AQLObject::add(PRICING_DATA_FREQUENCY, new AQLDataString()).convertFromString("SIMPLE");
	//inex type
	pindex2->AQLObject::add(PRICING_DATA_INDEXTYPE, new AQLDataString()).convertFromString(leg2indextypeInput);
	//sliding rule
	pindex2->AQLObject::add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString("FOLLOWING");
	//spot lag
	pindex2->AQLObject::add(PRICING_DATA_SPOTLAG, new AQLDataInt()).convertFromString(leg2offset);
	//fixing timing
	pindex2->AQLObject::add(PRICING_DATA_FIXINGTIMING, new AQLDataString()).convertFromString(leg2fixingtimingInput);
	//off set
	pindex2->AQLObject::add(PRICING_DATA_OFFSET, new AQLDataInt()).convertFromString(leg2offset);
	//fixing slidingrule
	pindex2->AQLObject::add(PRICING_DATA_FIXINGSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString("FOLLOWING");
	//fixing calendar
	pindex2->AQLObject::add(PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(leg2fixingcalendarInput);
	//is fwd intepolation
	if (leg2indextypeInput == "LIBOR")
	{
		pindex2->AQLObject::add(PRICING_DATA_ISFWDINTERPOLATION, new AQLDataBool()).convertFromString(isforwardinterInput);
		//fwd intepolation 
		pindex2->AQLObject::add(PRICING_DATA_FWDINTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(FN_SPLINEINTERPOLATION_STR);
		//frn
		pindex2->AQLObject::add(PRICING_DATA_INDEXGENERATEMETHOD, new AQLDataString()).convertFromString("FRN");
		
	}

	if (leg2indextypeInput == "FIXEDRATE")
	{
		//index name
		pleg2->AQLObject::add(PRICING_DATA_INDEXNAME, new AQLDataString()).convertFromString("FIXEDRATE");
	}
	else 
	{
		//index name
		pleg2->AQLObject::add(PRICING_DATA_INDEXNAME, new AQLDataString()).convertFromString(ccyInput.toUpper() + "_" + leg2indextypeInput);
	}
	
	//basiscurve
	AQLString leg2forcuaveInput;
	if ("OIS" == leg2indextypeInput)
		leg2forcuaveInput = fcurvestrois;
	else if ("1M" == leg2acessoryInput)
		leg2forcuaveInput = fcurvestr1m;
	else if ("3M" == leg2acessoryInput)
		leg2forcuaveInput = fcurvestr3m;
	else if ("6M" == leg2acessoryInput)
		leg2forcuaveInput = fcurvestr6m;
	else if ("1Y" == leg2acessoryInput)
		leg2forcuaveInput = fcurvestr6m;
	else
		throw AQLCoreInvalidData("Index Curve Set Error",__FILE__,__LINE__);
	
	pindex2->AQLObject::add(PRICING_DATA_BASISCURVE, new AQLDataString()).convertFromString(leg2forcuaveInput);
	//basisinterpolation
	pindex2->AQLObject::add(PRICING_DATA_BASISINTERPOLATION, new AQLDataString()).convertFromString(FN_LINEARINTERPOLATION_STR);

	//first fixing rate
	//first fixing basedate
	if (!istypicalpardeal)
	{
		itmap = maptrade.find(orgtradeid + "/" + "FirstFixingRate" + "/INDEX2");
		if (itmap != maptrade.end())
		{
			AQLString leg2firstfixngrateInput = itmap->second;
			pindex2->AQLObject::add(PRICING_DATA_FIRSTFIXINGRATE, new AQLDataDouble()).convertFromString(leg2firstfixngrateInput);
			pindex2->AQLObject::add(PRICING_DATA_FIRSTFIXINGBASEDATE, new AQLDataDate()).convertFromString(asofdateInput);
		}
	}

	//add oddindexinfo and generate oddindexinfo incase there are firstodddate or lastodddate in paymentdates
	ah_temp1 = &(pleg2->getData(PRICING_DATA_FIRSTODDDATE, NOCHECK));
	ah_temp2 = &(pleg2->getData(PRICING_DATA_LASTODDDATE, NOCHECK));
	if(leg2indextypeInput == "LIBOR" && (ah_temp1->isDefined() || ah_temp2->isDefined()))
	{
		// we must add two libor index except index of trade for stub interpolation.
		vector<AQLString> oddIndex2Input(2);
		oddIndex2Input[0] = coupon2Input + "_oddIndex1";
		oddIndex2Input[1] = coupon2Input + "_oddIndex2";
		pcoupon2->AQLObject::add(PRICING_DATA_ODDINDEXINFOS, new AQLDataMultiReference()).convertFromString(oddIndex2Input[0] + ":" + oddIndex2Input[1]);

		vector<AQLString> accessory(2), basisCurve(2);
		if("1M" == leg2acessoryInput)
		{
			accessory[0] = "3M";
			accessory[1] = "6M";
			basisCurve[0] = fcurvestr3m;
			basisCurve[1] = fcurvestr6m;
		}
		else if("3M" == leg2acessoryInput)
		{
			accessory[0] = "1M";
			accessory[1] = "6M";
			basisCurve[0] = fcurvestr1m;
			basisCurve[1] = fcurvestr6m;
		}
		else if("6M" == leg2acessoryInput || "1Y" == leg2acessoryInput)
		{
			accessory[0] = "1M";
			accessory[1] = "3M";
			basisCurve[0] = fcurvestr1m;
			basisCurve[1] = fcurvestr3m;
		}
		else
			throw AQLCoreInvalidData("invaild accessory type for stub interpolation.",__FILE__,__LINE__);

		for (unsigned int i = 0; i < oddIndex2Input.size(); i++ )
		{
			AQLObject* pindex2 = NULL;
			objHolder = objPool.getObject(oddIndex2Input[i]);
			if (objHolder.isDefined())
			{
				pindex2 = &objHolder.get();
				pindex2->clear();
			}
			else
			{
				pindex2= new AQLObject();
				objPool.set(oddIndex2Input[i], pindex2);
			}

			//name
			pindex2->AQLObject::add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(oddIndex2Input[i]);
			//ccyInput
			pindex2->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccyInput);
			////daycount
			pindex2->AQLObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(leg2daycountInput);
			//accessory
			pindex2->AQLObject::add(PRICING_DATA_ACCESSORY, new AQLDataString()).convertFromString(accessory[i]);
			//calendar
			pindex2->AQLObject::add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(leg2fixingcalendarInput);
			//off set
			pindex2->AQLObject::add(PRICING_DATA_OFFSET, new AQLDataInt()).convertFromString(leg2offset);
			////spot lag
			pindex2->AQLObject::add(PRICING_DATA_SPOTLAG, new AQLDataInt()).convertFromString(leg2offset);
			//fixing timing
			pindex2->AQLObject::add(PRICING_DATA_FIXINGTIMING, new AQLDataString()).convertFromString(leg2fixingtimingInput);
			//
			pindex2->AQLObject::add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString("FOLLOWING");
			////fixing slidingrule
			pindex2->AQLObject::add(PRICING_DATA_FIXINGSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString("FOLLOWING");
			//fixing calendar
			pindex2->AQLObject::add(PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(leg2fixingcalendarInput);
			//fwd intepolation 
			pindex2->AQLObject::add(PRICING_DATA_ISFWDINTERPOLATION, new AQLDataBool()).convertFromString(isforwardinterInput);
			pindex2->AQLObject::add(PRICING_DATA_FWDINTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(FN_SPLINEINTERPOLATION_STR);
			//frn
			pindex2->AQLObject::add(PRICING_DATA_INDEXGENERATEMETHOD, new AQLDataString()).convertFromString("FRN");
			////libor freq
			pindex2->AQLObject::add(PRICING_DATA_FREQUENCY, new AQLDataString()).convertFromString("SIMPLE");
			//inex type
			pindex2->AQLObject::add(PRICING_DATA_INDEXTYPE, new AQLDataString()).convertFromString(leg2indextypeInput);
			////index name
			//pleg1->AQLObject::add(PRICING_DATA_INDEXNAME, new AQLDataString()).convertFromString(ccyInput.toUpper() + "_" + leg1indextypeInput);
			//basiscurve
			pindex2->AQLObject::add(PRICING_DATA_BASISCURVE, new AQLDataString()).convertFromString(basisCurve[i]);
			//basisinterpolation
			pindex2->AQLObject::add(PRICING_DATA_BASISINTERPOLATION, new AQLDataString()).convertFromString(FN_LINEARINTERPOLATION_STR);

		}

	}

	dataInstance->getReferencePool().completeDependency();

	//generate !! and change MANUAL not to regenerate CF

	//in case of convergent at first we must expand cashflows.
	dh = &(ret->getData(PRICING_DATA_CFGENERATOR, ISNOTNULL));
	AQLDataProcedure& modelDataObj = dynamic_cast<AQLDataProcedure&>(dh->get());
	modelDataObj.calibrateModel(basedate);
	pleg1->getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).convertFromString("MANUAL");
	pleg2->getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).convertFromString("MANUAL");

	//setUpFixingRateFromPastRates
	setUpFixingRateFromPastRates(dataInstance,copytradeid, basedate);

	dataInstance->getReferencePool().completeDependency();

	return ret;
}

void 
AQLMultiSwapPricer::createOfferBidAdjustEntity(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade, AQLStringVector orgtradevec, bool isduplicatemode, AQLString key_suffix)
{
	
	AQLDataHolder* dh;
	AQLObjectPool& objPool = dataInstance->getObjectPool();

	AQLStringVector orgEntityNameVec,adjustEntityNameVec,tradeNameVec,paraNameVec;

	//important clasee "lineparam1"
	AQLObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	// necessary in case of index set
	AQLString fcurvestr1m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_1MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr3m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_3MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr6m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_6MFORECAST", ISNOTNULL).get());
	AQLString fcurvestrois = dynamic_cast<AQLDataString &>(eline.getData("CURVE_OISFORECAST", ISNOTNULL).get());
	// std forcast
	AQLString swapfreq = dynamic_cast<AQLDataString &>(eline.getData("SWAP_FREQUENCY", ISNOTNULL).get());
	AQLString fcurvestrstd = dynamic_cast<AQLDataString &>(eline.getData(AQLString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get());


	map<AQLString, AQLString>::iterator itmap = maptrade.begin();
	for (unsigned int i = 0; i < orgtradevec.size(); i++)
	{
		AQLString copytradeid = orgtradevec[i];
		if (isduplicatemode)
			copytradeid += key_suffix;

		double offerbidspread = 0.0;
		//map must be original id
		itmap = maptrade.find(orgtradevec[i] + "/" + "OfferBidAdjust");
		if (itmap != maptrade.end())
			offerbidspread = itmap->second.getDoubleValue();

		//in case of 0 we do not have to generate offer bid curve
		if (0.0 == offerbidspread)
			continue;

		//from product type and rcv pay, index info, we create new curve
		AQLMathObjectValue& etrade = dynamic_cast<AQLMathObjectValue &>(objPool.getObject(copytradeid, ENCHKTYPE_ISDEFINED).get());

		dh = &(etrade.getData("LineProductType", ISNOTNULL));
		AQLString linetype = dynamic_cast<AQLDataString &>(dh->get());
		linetype.toUpper();

		AQLStringVector riskDetail1obadjust(1);
		AQLStringVector riskDetail2obadjust(1);

		bool ispositivedirection = true;
		if (linetype == "SWAP" || linetype == "FRA")
		{
			AQLString leg1name = copytradeid + "_leg1";
			AQLObject& eleg1 = objPool.getObject(leg1name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eleg1.getData(PRICING_DATA_SELECTSIDE,ISNOTNULL));
			//leg 1 is fixed side. if leg1 rcv direction is positive
			AQLString leg1PRname = dynamic_cast<AQLDataString &>(dh->get());
			if (leg1PRname.toUpper() == "RCV")
				ispositivedirection = true;
			else 			
				ispositivedirection = false;

			if (!ispositivedirection)
				offerbidspread *= -1;

			riskDetail1obadjust[0] =  fcurvestrois + "+" + fcurvestrstd;
			riskDetail2obadjust[0] =  AQLString(offerbidspread,3) + "_BPPARALLEL";
		}
		else if (linetype == "BASISSWAP")
		{
			
			AQLString leg1name = copytradeid + "_leg1";
			AQLObject& eleg1 = objPool.getObject(leg1name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eleg1.getData(PRICING_DATA_SELECTSIDE,ISNOTNULL));
			AQLString leg1PRname = dynamic_cast<AQLDataString &>(dh->get());
			AQLString index1name =  copytradeid + "_leg1_coupon1_index1";
			AQLObject& eindex1 = objPool.getObject(index1name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eindex1.getData(PRICING_DATA_BASISCURVE,ISNOTNULL));
			AQLString index1curve = dynamic_cast<AQLDataString &>(dh->get());

			AQLString leg2name = copytradeid + "_leg2";
			AQLObject& eleg2 = objPool.getObject(leg2name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eleg2.getData(PRICING_DATA_SELECTSIDE,ISNOTNULL));
			AQLString leg2PRname = dynamic_cast<AQLDataString &>(dh->get());
			AQLString index2name =  copytradeid + "_leg2_coupon2_index2";
			AQLObject& eindex2 = objPool.getObject(index2name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eindex2.getData(PRICING_DATA_BASISCURVE,ISNOTNULL));
			AQLString index2curve = dynamic_cast<AQLDataString &>(dh->get());

			//if there is at least one month index riskDetail1obadjust[0] = 1mcurve.
			//1st
			if (index1curve == fcurvestr1m)
			{
				riskDetail1obadjust[0] = fcurvestr1m;
				if (leg1PRname.toUpper() == "RCV")
					ispositivedirection = true;
				else
					ispositivedirection = false;
			}
			else if (index2curve == fcurvestr1m)
			{
				riskDetail1obadjust[0] = fcurvestr1m;
				if (leg2PRname.toUpper() == "RCV")
					ispositivedirection = true;
				else
					ispositivedirection = false;
			}
			//2
//			else if (index1curve == fcurvestr6m)
			else if (index1curve == fcurvestr6m && swapfreq != "6M")
			{
				riskDetail1obadjust[0] = fcurvestr6m;
				if (leg1PRname.toUpper() == "RCV")
					ispositivedirection = false;
				else
					ispositivedirection = true;
			}
//			else if (index2curve == fcurvestr6m)
			else if (index2curve == fcurvestr6m && swapfreq != "6M")
			{
				riskDetail1obadjust[0] = fcurvestr6m;
				if (leg2PRname.toUpper() == "RCV")
					ispositivedirection = false;
				else
					ispositivedirection = true;
			}
			//3
			else if (index1curve == fcurvestr3m && swapfreq != "3M")
			{
				riskDetail1obadjust[0] = fcurvestr3m;
				if (leg1PRname.toUpper() == "RCV")
					ispositivedirection = false;
				else
					ispositivedirection = true;
			}
			else if (index2curve == fcurvestr3m && swapfreq != "3M")
			{
				riskDetail1obadjust[0] = fcurvestr3m;
				if (leg2PRname.toUpper() == "RCV")
					ispositivedirection = false;
				else
					ispositivedirection = true;
			}
			else 
				throw AQLCoreInvalidData("index pair is wrong setting",__FILE__,__LINE__);

			if (!ispositivedirection)
				offerbidspread *= -1;

			riskDetail2obadjust[0] =  AQLString(offerbidspread,3) + "_BPPARALLEL";
		}

		AQLString curveID = getCurveIDfromTradeReference(dataInstance,orgtradevec[i]);
		AQLString paraname = getMarketParamfromTradeReference(dataInstance,orgtradevec[i]);
		
		AQLString keyname= curveID + "_SOURCEDELTA_";
		keyname += riskDetail1obadjust[0] + "_" + riskDetail2obadjust[0];

		AQLObjectHolder objHolder = objPool.getObject(keyname,ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			//if no exist
			//make curve
			createSourceDeltaRiskEntity(dataInstance, "offerbidtemprary", riskDetail1obadjust, riskDetail2obadjust, false, paraname);
		}
		
		orgEntityNameVec.push_back(curveID);
		adjustEntityNameVec.push_back(keyname);
		tradeNameVec.push_back(copytradeid);
		paraNameVec.push_back(paraname);
	
	}


	//if b/o adjust create it and calc PV again
	AQLObject* pob = NULL;
	AQLObjectHolder objHolder = objPool.getObject("lineofferbidentity", ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		pob = new AQLObject();
		objPool.set("lineofferbidentity",pob);
	
	}
	else
	{
		pob = &(objHolder.get());
		pob->clear();
	}
	pob->add(CALIBRATION_DATA_NAME, new AQLDataString("lineofferbidentity"));

	
	pob->remove("orgEntityName");
	pob->add("orgEntityName", new AQLDataStrings(orgEntityNameVec));

	pob->remove("adjsutEntityName");
	pob->add("adjsutEntityName", new AQLDataStrings(adjustEntityNameVec));

	pob->remove("tradeEntityName");
	pob->add("tradeEntityName", new AQLDataStrings(tradeNameVec));

	pob->remove("paramEntityName");
	pob->add("paramEntityName", new AQLDataStrings(paraNameVec));

	return;
}


void
AQLMultiSwapPricer::calcPortfolioRiskAndPV(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> mapriskinfo)
{

	AQLDataHolder* dh;
	AQLObjectPool& objPool = dataInstance->getObjectPool();

	//calc ports
	AQLStringVector tradeVec;
	EntityIter it = objPool.begin();
	for (it = objPool.begin(); it != objPool.end(); it++)
	{
		if (!it->second.isTypeOf(ENTITY_VENTITY))
			continue;

		//search linepricer input
		dh = &(it->second.getData("LineProductType", NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		//search lineprice pnl flag if false not calc risk
		dh = &(it->second.getData("IsPnL", NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			bool ispnl = dynamic_cast<AQLDataBool &>(dh->get());
			if (!ispnl)
				continue;
		}
		tradeVec.push_back(it->first);
	}

	if (tradeVec.size() < 1)
		throw AQLCoreInvalidData("no trades for risk calc",__FILE__,__LINE__);

	//important clasee "lineparam1"
	AQLObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	
	// necessary in case of index set
	AQLString fcurvestr1m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_1MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr3m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_3MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr6m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_6MFORECAST", ISNOTNULL).get());
	AQLString fcurvestrois = dynamic_cast<AQLDataString &>(eline.getData("CURVE_OISFORECAST", ISNOTNULL).get());
	// std forcast
	AQLString swapfreq = dynamic_cast<AQLDataString &>(eline.getData("SWAP_FREQUENCY", ISNOTNULL).get());
	AQLString fcurvestrstd = dynamic_cast<AQLDataString &>(eline.getData(AQLString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get());
	////to calc gamma 1bp up and 1bp down need developing
	
	//swap shift
	AQLStringVector marketboxvec(1);
	AQLStringVector rateboxvec(1);
	AQLString risktarget;
	AQLString riskentitystr = "";
	
	risktarget = mapriskinfo[AQLString("RISK/") + "Swap"].toUpper();
	if ("TRUE" == risktarget)
	{
		marketboxvec[0] = "swapRates";
//		rateboxvec[0] = fcurvestr3m;
		rateboxvec[0] = fcurvestrstd;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPSWAP", rateboxvec, marketboxvec,true);

		riskentitystr += "lineriskentitySOURCEBUMPSWAP";
		riskentitystr += ":";
	}

	risktarget = mapriskinfo[AQLString("RISK/") + "Libor"].toUpper();
	if ("TRUE" == risktarget)
	{
		//libor shift
		marketboxvec[0] = "liborRates";
//		rateboxvec[0] = fcurvestr3m;
		rateboxvec[0] = fcurvestrstd;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPLIBOR", rateboxvec, marketboxvec,true);
		riskentitystr += "lineriskentitySOURCEBUMPLIBOR";
		riskentitystr += ":";
	}

	risktarget = mapriskinfo[AQLString("RISK/") + "FRA3M"].toUpper();
	if ("TRUE" == risktarget)
	{
		//future shift
		marketboxvec[0] = "fra3mRates";
		rateboxvec[0] = fcurvestrstd;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPFRA3M", rateboxvec, marketboxvec,true);
		riskentitystr += "lineriskentitySOURCEBUMPFRA3M";
		riskentitystr += ":";
	}

	risktarget = mapriskinfo[AQLString("RISK/") + "FRA6M"].toUpper();
	if ("TRUE" == risktarget)
	{
		//future shift
		marketboxvec[0] = "fra6mRates";
		rateboxvec[0] = fcurvestrstd;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPFRA6M", rateboxvec, marketboxvec,true);
		riskentitystr += "lineriskentitySOURCEBUMPFRA6M";
		riskentitystr += ":";
	}


	risktarget = mapriskinfo[AQLString("RISK/") + "Future"].toUpper();
	if ("TRUE" == risktarget)
	{
		//future shift
		marketboxvec[0] = "futureRates";
//		rateboxvec[0] = fcurvestr3m;
		rateboxvec[0] = fcurvestrstd;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPFUTURE", rateboxvec, marketboxvec,true);
		riskentitystr += "lineriskentitySOURCEBUMPFUTURE";
		riskentitystr += ":";
	}

	risktarget = mapriskinfo[AQLString("RISK/") + "OISCurve"].toUpper();
	if ("TRUE" == risktarget)
	{
		//ois shift
		marketboxvec[0] = "oisRates";
		rateboxvec[0] = fcurvestrois;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPOIS", rateboxvec, marketboxvec,true);
		riskentitystr += "lineriskentitySOURCEBUMPOIS";
		riskentitystr += ":";
	}


	risktarget = mapriskinfo[AQLString("RISK/") + "3M6MBasis"].toUpper();
	if ("TRUE" == risktarget)
	{
		marketboxvec[0] = "basisMkt";
//		rateboxvec[0] = fcurvestr6m;
		if(swapfreq == "3M"){
			rateboxvec[0] = fcurvestr6m;
		} else if(swapfreq == "6M"){
			rateboxvec[0] = fcurvestr3m;
		}
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMP3M6MBASIS", rateboxvec, marketboxvec,true);
		riskentitystr += "lineriskentitySOURCEBUMP3M6MBASIS";
		riskentitystr += ":";
	}

	//risktarget = mapriskinfo[AQLString("RISK/") + "1M3MBasis"].toUpper();
	risktarget = mapriskinfo[AQLString("RISK/") + "1M" + swapfreq + "Basis"].toUpper();
	if ("TRUE" == risktarget)
	{
		marketboxvec[0] = "basisMkt";
		rateboxvec[0] = fcurvestr1m;
	//	createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMP1M3MBASIS", rateboxvec, marketboxvec,true);
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMP1M" + swapfreq + "BASIS", rateboxvec, marketboxvec,true);
	//	riskentitystr += "lineriskentitySOURCEBUMP1M3MBASIS";
		riskentitystr += "lineriskentitySOURCEBUMP1M" + swapfreq + "BASIS";
		riskentitystr += ":";
	}
	else if("" == risktarget)
		// there is something wrong because 1M forcast riskentity must be set and will not be empty.
		throw AQLCoreInvalidData("1MFrocast input error",__FILE__,__LINE__);

	risktarget = mapriskinfo[AQLString("RISK/") + "ZeroRiskTemporary"];
	if ("TRUE" == risktarget)
	{
		AQLStringVector marketboxvec;
		AQLStringVector rateboxvec(4);
//		rateboxvec[0] = fcurvestr3m;
//		rateboxvec[1] = fcurvestrois;
//		rateboxvec[2] = fcurvestr6m;
//		rateboxvec[3] = fcurvestr1m;
		if(swapfreq == "3M") {
			rateboxvec[0] = fcurvestr3m;
			rateboxvec[1] = fcurvestrois;
			rateboxvec[2] = fcurvestr6m;
			rateboxvec[3] = fcurvestr1m;
		} else if(swapfreq == "6M") {
			rateboxvec[0] = fcurvestr6m;
			rateboxvec[1] = fcurvestrois;
			rateboxvec[2] = fcurvestr3m;
			rateboxvec[3] = fcurvestr1m;
		}

		createZeroDeltaRiskEntity(dataInstance,"lineriskentityZEROBAMP", rateboxvec, marketboxvec,true);

		riskentitystr += "lineriskentityZEROBAMP";
		riskentitystr += ":";

	}

	if (riskentitystr != "")
		riskentitystr = riskentitystr.subString(0, riskentitystr.size() - 2);
	//set linedelta
	eline.remove("riskentity");
	eline.add("riskentity", new AQLDataMultiReference()).convertFromString(riskentitystr);

	
	DoubleVector pvResultVec;
	calcRiskAndPV(dataInstance,tradeVec,pvResultVec);

	eline.remove("riskentity");

	//regsitered tradevecs eline. it is used when calling get risk result function.
	eline.remove("tradeforportrisk");
	if (tradeVec.size() != 0)
		eline.add("tradeforportrisk", new AQLDataStrings(tradeVec));

	return;
}


double 
AQLMultiSwapPricer::getPortfolioRiskResult(AQLDataInstance* dataInstance, AQLString marketkey, AQLString marketgrid, AQLString risktype)
{
	double ret = 0.0;

	AQLObject& eline = dataInstance->getObjectPool().getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	AQLDataHolder* dh = &(eline.getData("tradeforportrisk"));
	if (!dh->isDefined() || dh->isNull())
		return 0.0;

	const AQLStringVector& tradeVec = dynamic_cast<AQLDataStrings &>(dh->get()).get();

	//now only support marketkey is curve box.
	AQLString marketStr = marketkey;
	if (marketStr != "Libor" && marketStr != "Swap" && marketStr != "FRA3M" && marketStr != "FRA6M" && marketStr != "Future" &&
		marketStr != "3M6MBasis" && marketStr != "1M3MBasis" && marketStr != "1M6MBasis" &&
		marketStr != "OISCurve")
		throw AQLCoreInvalidData("market key error",__FILE__,__LINE__);


	//this is related with CalcportfolioRiskAndPV function
	//in source delta, typical example is 
	//AQLString key = "SOURCEBUMPRISK_" + riskDetail1[j] + "_" + tmpvec[0] + "_" + tmpvec[1];

	AQLString marketbox;
	if (marketStr == "Libor")
		marketbox = "liborRates";
	else if (marketStr == "Swap")
		marketbox = "swapRates";
	else if (marketStr == "FRA3M")
		marketbox = "fra3mRates";
	else if (marketStr == "FRA6M")
		marketbox = "fra6mRates";
	else if (marketStr == "Future")
		marketbox = "futureRates";
	else if (marketStr == "3M6MBasis")
		marketbox = "basisMkt";
	else if (marketStr == "1M3MBasis")
		marketbox = "basisMkt";
	else if (marketStr == "1M6MBasis")
		marketbox = "basisMkt";
	else if (marketStr == "OISCurve")
		marketbox = "oisRates";

	// necessary in case of index set
	AQLString fcurvestr1m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_1MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr3m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_3MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr6m = dynamic_cast<AQLDataString &>(eline.getData("CURVE_6MFORECAST", ISNOTNULL).get());
	AQLString fcurvestrois = dynamic_cast<AQLDataString &>(eline.getData("CURVE_OISFORECAST", ISNOTNULL).get());
	// std forcast
	AQLString swapfreq = dynamic_cast<AQLDataString &>(eline.getData("SWAP_FREQUENCY", ISNOTNULL).get());
	AQLString fcurvestrstd = dynamic_cast<AQLDataString &>(eline.getData(AQLString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get());

	
	//search gencurvename from market box !! it depends on Currency so that we change the map in JPY case.
	AQLString riskDetail;
	if (marketStr == "Libor")
		riskDetail = fcurvestrstd;
	else if	(marketStr == "Swap")
		riskDetail = fcurvestrstd;
	else if	(marketStr == "FRA3M")
		riskDetail = fcurvestrstd;
	else if	(marketStr == "FRA6M")
		riskDetail = fcurvestrstd;
	else if	(marketStr == "Future")
		riskDetail = fcurvestrstd;
	else if	(marketStr == "3M6MBasis")
	{
		if (swapfreq == "3M")
			riskDetail = fcurvestr6m;
		else if	(swapfreq == "6M")
			riskDetail = fcurvestr3m;
	}
	else if	(marketStr == "1M3MBasis" || marketStr == "1M6MBasis")
		riskDetail = fcurvestr1m;
	else if	(marketStr == "OISCurve")
		riskDetail = fcurvestrois;

	//risktype = sourcebump or zero bump
	if (risktype != "ZEROBUMP" && risktype != "SOURCEBUMP")
		throw AQLCoreInvalidData("risktyp input error",__FILE__,__LINE__);

	AQLString key = risktype + "RISK_" + riskDetail + "_" + marketbox + "_" + marketgrid;


	for (unsigned int i = 0; i < tradeVec.size(); i++)
	{
		AQLObject& val = dataInstance->getObjectPool().getObject(tradeVec[i], ENCHKTYPE_ISDEFINED).get();
		dh = &(val.getData(key));
		if (dh->isDefined() && !dh->isNull())
			ret += dynamic_cast<AQLDataDouble &>(dh->get());
	
	}

	return ret;
}


void
AQLMultiSwapPricer::storePastRates(AQLDataInstance* dataInstance, const AQLStringMatrix& IndexInfo, const AQLStringMatrix& PastRates)
{

	AQLObjectPool &objPool = dataInstance->getObjectPool();
	AQLObjectHolder objHolder;

	AQLStringMatrix tmpInfos = IndexInfo;

	///////////////////line param object///////////////////////////////
	AQLString lineparam("lineparam1");
	objHolder = objPool.getObject(lineparam, ENCHKTYPE_NOCHECK);
	AQLObject* eline = NULL;
	if (!objHolder.isDefined())
	{
		eline = new AQLObject();
		objPool.set(lineparam, eline);
	}
	else
	{
		eline = &(objHolder.get());
	}

	AQLString ccy = AQLFunctionUtilities::findElement(tmpInfos,"Currency",0,1,true,true).toUpper();
	AQLString indextype = AQLFunctionUtilities::findElement(tmpInfos,"Rate",0,1,true,true).toUpper();
	AQLString accessory;
	if (indextype == "LIBOR")
	{
		accessory = AQLFunctionUtilities::findElement(tmpInfos,"Term",0,1,true,true);
	}
	else if (indextype == "FF" || indextype == "OIS")
	{
		indextype = "OIS"; 
		accessory = "1D";
	}
	else 
		throw AQLCoreInvalidData("Rate type supports only Libor, FF and OIS",__FILE__,__LINE__);

	AQLString keynamesuffix = "_" + ccy + "_" + indextype + "_" + accessory; 

	unsigned int pastSize = PastRates.size();
	DoubleVector ratesvec(pastSize);
	DateVector pastdates(pastSize);

	if (pastSize == 0)
	{
		throw AQLCoreInvalidData("PastRates set error",__FILE__,__LINE__);
	}
	
	if (pastSize != 0 && PastRates[0].size() < 2)
	{
		throw AQLCoreInvalidData("PastRates set error",__FILE__,__LINE__);
	}

	map<AQLDate, double> map_date_rate;
	for (unsigned int i = 0; i < pastSize; i++)
	{
		pastdates[i] = AQLMathDateUtilities::getAQLDate(PastRates[i][0]);
		map_date_rate[pastdates[i]] = PastRates[i][1].getDoubleValue();
		//ratesvec[i] = PastRates[i][1].getDoubleValue();
	}

	sort(pastdates.begin(), pastdates.end());
	for (unsigned int i = 0; i < pastSize; i++)
	{
		ratesvec[i] = map_date_rate[pastdates[i]];
	}

	eline->remove("PastDates"+ keynamesuffix);
	eline->add("PastDates"+ keynamesuffix, new AQLDataDates(pastdates));

	eline->remove("PastRates"+ keynamesuffix);
	eline->add("PastRates"+ keynamesuffix, new AQLDataDoubles(ratesvec));



	return;

}

void
AQLMultiSwapPricer::setUpFixingRateFromPastRates(AQLDataInstance* dataInstance, AQLString copytradeid, const AQLDate& asOfDate)
{

	AQLDataHolder* dh;
	AQLObjectPool &objPool = dataInstance->getObjectPool();

	AQLObject& eline = objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).get();
	
	AQLObjectHolder objHolder = objPool.getObject(copytradeid, ENCHKTYPE_ISDEFINED);

	AQLMathObjectValue& val = dynamic_cast<AQLMathObjectValue&>(objHolder.get());
	
	dh = &(val.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
	unsigned int legSize = legs.getSize();

	
	for (unsigned int i = 0; i < legSize; i++)
	{
		bool issearchend = false;

		dh = &(legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;
		
		AQLDataMultiReference& cashlets = dynamic_cast<AQLDataMultiReference &>(dh->get());
		unsigned int cashletSize = cashlets.getSize();
		for (unsigned int j = 0; j < cashletSize; j++)
		{
			if (issearchend)
			{
				//it means go out of for cashlet loop
				j = cashletSize -1;
				continue;
			}

			//if past cashflow, continu
			dh = &(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISDEFINED));
			const AQLDate& paydate = dynamic_cast<AQLDataDate &>(dh->get()).get();
			if (paydate < asOfDate)
				continue;

			dh = &(cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
				continue;
			
				//multi coupon is only stub case
			AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference &>(dh->get());
			unsigned int couponSize = coupons.getSize();
			for (unsigned int k = 0; k < couponSize; k++)
			{
				dh = &(coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
				if (!dh->isDefined() || dh->isNull())
					continue;
		
				AQLDataMultiReference& indexs = dynamic_cast<AQLDataMultiReference &>(dh->get());
				AQLObject& eindex = indexs.get(0).get();

				//first priority is FixingDates
				dh = &(eindex.getData(PRICING_DATA_FIXINGDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					const DateVector& fixingdates = dynamic_cast<const AQLDataDates &>(dh->get()).get();
					AQLDate firstfixingdate = fixingdates[0];
					
					if (firstfixingdate > asOfDate)
						issearchend = true;
					else
					{
						//set up past rates
						AQLString ccy = dynamic_cast<AQLDataString &>(eindex.getData(IR_CALIBRATION_DATA_CURRENCY, ISDEFINED).get());
						AQLString accessory = dynamic_cast<AQLDataString &>(eindex.getData(PRICING_DATA_ACCESSORY, ISDEFINED).get());

						AQLString keynamesuffix;
						if (accessory == "1D")
						{
							keynamesuffix = "_" + ccy + "_OIS_" + accessory;
						}
						else
						{
							keynamesuffix = "_" + ccy + "_LIBOR_" + accessory;
						}

						dh = &(eline.getData("PastDates" + keynamesuffix, NOCHECK));
						if (!dh->isDefined() || dh->isNull())
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (firstfixingdate == asOfDate)
								continue;
							
							throw AQLCoreInvalidData("past rates have not been stored", __FILE__,__LINE__);
						}
							
						DateVector pastdates = dynamic_cast<AQLDataDates &>(dh->get()).get();

						unsigned int firstpos = 0;
						bool isexist = AQLAlgorithm::find<DateVector, AQLDate>(pastdates, firstfixingdate, 0, pastdates.size()-1, firstpos);
						if (!isexist)
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (firstfixingdate == asOfDate)
								continue;

							AQLString ans = "fixing date " + AQLDataDate(firstfixingdate).convertToString() + " is not set";
							throw AQLCoreInvalidData(ans.getCString(), __FILE__,__LINE__);
						}

						dh = &(eline.getData("PastRates" + keynamesuffix, ISDEFINED));
						DoubleVector pastrates = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
						if (pastdates.size() != pastrates.size())
							throw AQLCoreInvalidData("Past rates size error",__FILE__,__LINE__);

						pastrates.erase(pastrates.begin(),pastrates.begin() + firstpos);
						pastdates.erase(pastdates.begin(),pastdates.begin() + firstpos);

						eindex.remove(PRICING_DATA_OBSERVATIONDATES);
						eindex.add(PRICING_DATA_OBSERVATIONDATES, new AQLDataDates(pastdates));

						eindex.remove(PRICING_DATA_OBSERVATIONRATES);
						eindex.add(PRICING_DATA_OBSERVATIONRATES, new AQLDataDoubles(pastrates));

						/*double fixedrate = pastrates[pos];
						eindex.remove(PRICING_DATA_FIXEDRATE);
						eindex.add(PRICING_DATA_FIXEDRATE, new AQLDataDouble(fixedrate));*/
					}
					
					continue;
				}
			
			
				dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
				if (!dh->isDefined() || dh->isNull())
					continue;


				if (dynamic_cast<AQLDataString &>(dh->get()).get() == "LIBOR")
				{
					dh = &(eindex.getData(PRICING_DATA_FIXINGDATE, ISDEFINED));
					const AQLDate& fixingdate = dynamic_cast<AQLDataDate &>(dh->get()).get();
					
					//if fixing does not come, go out of for loop
					if (fixingdate > asOfDate)
						issearchend = true;
					else
					{
						//set up past rates
						AQLString ccy = dynamic_cast<AQLDataString &>(eindex.getData(IR_CALIBRATION_DATA_CURRENCY, ISDEFINED).get());
						AQLString accessory = dynamic_cast<AQLDataString &>(eindex.getData(PRICING_DATA_ACCESSORY, ISDEFINED).get());

						//if already have fixed rate, then we do not need to get from pastrates
						dh = &(eindex.getData(PRICING_DATA_FIXEDRATE, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
							continue;


						AQLString keynamesuffix = "_" + ccy + "_LIBOR_" + accessory;
						dh = &(eline.getData("PastDates" + keynamesuffix, NOCHECK));
						if (!dh->isDefined() || dh->isNull())
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (fixingdate == asOfDate)
								continue;
							
							throw AQLCoreInvalidData("past rates have not been stored", __FILE__,__LINE__);
						}
							
						const DateVector& pastdates = dynamic_cast<AQLDataDates &>(dh->get()).get();

						unsigned int pos = 0;
						bool isexist = AQLAlgorithm::find<DateVector, AQLDate>(pastdates, fixingdate, 0, pastdates.size()-1, pos);
						if (!isexist)
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (fixingdate == asOfDate)
								continue;

							AQLString ans = "fixing date " + AQLDataDate(fixingdate).convertToString() + " is not set";
							throw AQLCoreInvalidData(ans.getCString(), __FILE__,__LINE__);
						}

						dh = &(eline.getData("PastRates" + keynamesuffix, ISDEFINED));
						const DoubleVector& pastrates = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
						if (pastdates.size() != pastrates.size())
							throw AQLCoreInvalidData("Past rates size error",__FILE__,__LINE__);

						double fixedrate = pastrates[pos];
						eindex.remove(PRICING_DATA_FIXEDRATE);
						eindex.add(PRICING_DATA_FIXEDRATE, new AQLDataDouble(fixedrate));
					}
				}
				else if (dynamic_cast<AQLDataString &>(dh->get()).get() == "OIS")
				{

					dh = &(eindex.getData(PRICING_DATA_OBSERVATIONSTARTDATE, ISDEFINED));
					const AQLDate& firstfixingdate = dynamic_cast<AQLDataDate &>(dh->get()).get();

					dh = &(eindex.getData(PRICING_DATA_OBSERVATIONENDDATE, ISDEFINED));
					const AQLDate& lastfixingdate = dynamic_cast<AQLDataDate &>(dh->get()).get();
					
					//if fixing does not come, go out of for loop
					if (firstfixingdate > asOfDate)
						issearchend = true;
					else
					{
						//set up past rates
						AQLString ccy = dynamic_cast<AQLDataString &>(eindex.getData(IR_CALIBRATION_DATA_CURRENCY, ISDEFINED).get());
						AQLString accessory = dynamic_cast<AQLDataString &>(eindex.getData(PRICING_DATA_ACCESSORY, ISDEFINED).get());

						AQLString keynamesuffix = "_" + ccy + "_OIS_" + accessory;
						dh = &(eline.getData("PastDates" + keynamesuffix, NOCHECK));
						if (!dh->isDefined() || dh->isNull())
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (firstfixingdate == asOfDate)
								continue;
							
							throw AQLCoreInvalidData("past rates have not been stored", __FILE__,__LINE__);
						}
							
						DateVector pastdates = dynamic_cast<AQLDataDates &>(dh->get()).get();

						unsigned int firstpos = 0;
						bool isexist = AQLAlgorithm::find<DateVector, AQLDate>(pastdates, firstfixingdate, 0, pastdates.size()-1, firstpos);
						if (!isexist)
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (firstfixingdate == asOfDate)
								continue;

							AQLString ans = "fixing date " + AQLDataDate(firstfixingdate).convertToString() + " is not set";
							throw AQLCoreInvalidData(ans.getCString(), __FILE__,__LINE__);
						}

						dh = &(eline.getData("PastRates" + keynamesuffix, ISDEFINED));
						DoubleVector pastrates = dynamic_cast<AQLDataDoubles &>(dh->get()).get();
						if (pastdates.size() != pastrates.size())
							throw AQLCoreInvalidData("Past rates size error",__FILE__,__LINE__);

						pastrates.erase(pastrates.begin(),pastrates.begin() + firstpos);
						pastdates.erase(pastdates.begin(),pastdates.begin() + firstpos);

						eindex.remove(PRICING_DATA_OBSERVATIONDATES);
						eindex.add(PRICING_DATA_OBSERVATIONDATES, new AQLDataDates(pastdates));

						eindex.remove(PRICING_DATA_OBSERVATIONRATES);
						eindex.add(PRICING_DATA_OBSERVATIONRATES, new AQLDataDoubles(pastrates));

						/*double fixedrate = pastrates[pos];
						eindex.remove(PRICING_DATA_FIXEDRATE);
						eindex.add(PRICING_DATA_FIXEDRATE, new AQLDataDouble(fixedrate));*/
					}
				}
			}
		}
		
	}




	return;



}


AQLString
AQLMultiSwapPricer::setUpMarketParamsAndGlobalShift(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> mapcalcinfo, AQLString curveID)
{

	AQLDataHolder* dh;
	AQLObjectPool &objPool = dataInstance->getObjectPool();
	AQLObjectHolder objHolder;

	map<AQLString, AQLString>::iterator itcalcmap = mapcalcinfo.begin();

	AQLString marketparamInput = "marketparam1_" + curveID;
	AQLMathPlainVanillaEntity* pvanilla = NULL;
	objHolder = objPool.getObject(marketparamInput);
	if (objHolder.isDefined())
	{
		if (!objHolder.isTypeOf(ENTITY_PLAINVANILLA))
			throw AQLCoreInvalidData("Vanilla Setup Error",__FILE__,__LINE__);

		pvanilla = dynamic_cast<AQLMathPlainVanillaEntity *>(&objHolder.get());
		pvanilla->reset();
	}
	else
	{
		pvanilla = new AQLMathPlainVanillaEntity(dataInstance);
		objPool.set(marketparamInput, pvanilla);
	}

	//name
	pvanilla->getData(CALIBRATION_DATA_NAME, ISDEFINED).convertFromString(marketparamInput);
	//daycount
	pvanilla->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISDEFINED).convertFromString("ACT/365_ISDA");
	//asofdate
	AQLObject* pyldEntity = &(objPool.getObject(curveID,ENCHKTYPE_ISDEFINED).get());
	dh = &(pyldEntity->getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL));
	AQLString asofdateInput = dh->convertToString();
	pvanilla->getData(CALIBRATION_DATA_ASOFDATE, ISDEFINED).convertFromString(asofdateInput);
	
	//ircurs
	//AQLStringVector irccys(ccy,1)
	dh = &(pyldEntity->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL));
	AQLString ccyInput = dynamic_cast<AQLDataString &>(dh->get()).get();
	AQLStringVector ccys = ccyInput.toToken(':');
	if (ccys.size() > 1)
	{
		throw AQLCoreInvalidData("Multi currency pricing is not supported at the moment.", __FILE__, __LINE__);
	}
	pvanilla->getIRCurrencys().set(ccys);
	
	//set yield curve
	AQLStringVector yieldNames(ccys.size());
	if (AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST).toUpper() == "TRUE")
	{
		std::for_each(yieldNames.begin(), yieldNames.end(), [curveID](AQLString& yieldName) { yieldName = PREFIX_YIELD + curveID; });
	}
	else
	{
		std::transform(ccys.cbegin(), ccys.cend(), yieldNames.begin(), [](const AQLString& ccy) { return AQLMarketData::getBaseYieldName(ccy); });
	}
	for (auto&& yieldName : yieldNames)
	{
		objHolder = objPool.getObject(yieldName, ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			AQLMathYieldCurve* pYield = new AQLMathYieldCurve(dataInstance);
			pYield->getName().convertFromString(yieldName);
			pYield->getYieldData().convertFromString(curveID);
			pYield->getInterpolation().convertFromString(FN_SPLINEINTERPOLATION_STR);
			objPool.set(yieldName, pYield);
		}
	}
	pvanilla->getIRCurves().convertFromString(AQLDataStrings(yieldNames).convertToString());

	//set names of yield curve pro
	AQLStringVector yieldProNames(ccys.size());
	if (AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST).toUpper() == "TRUE")
	{
		std::transform(yieldNames.cbegin(), yieldNames.cend(), yieldProNames.begin(), [](const AQLString& yieldName) { return AQLString("PRO_") + yieldName; });
	}
	else
	{
		std::transform(ccys.cbegin(), ccys.cend(), yieldProNames.begin(), [](const AQLString& ccy) { return AQLMarketData::getBaseYieldProName(ccy); });
	}
	for (auto&& yieldProName : yieldProNames)
	{
		objHolder = objPool.getObject(yieldProName, ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			AQLMathYieldCurvePro* pYieldPro = new AQLMathYieldCurvePro(dataInstance);
			pYieldPro->getName().convertFromString(yieldProName);
			pYieldPro->getYieldData().convertFromString(curveID);
			pYieldPro->getInterpolation().convertFromString(FN_SPLINEINTERPOLATION_STR);
			objPool.set(yieldProName, pYieldPro);
		}
	}
	pvanilla->getIRCurveProNames().set(yieldProNames);

	//temporary 
	//interpolationset, usually, we should on curve generate
	AQLString fcurvestr1m = mapcalcinfo[AQLString("CURVE/") + "1MForecast"];
	AQLString fcurvestr3m = mapcalcinfo[AQLString("CURVE/") + "3MForecast"];
	AQLString fcurvestr6m = mapcalcinfo[AQLString("CURVE/") + "6MForecast"];
	AQLString fcurvestrois = mapcalcinfo[AQLString("CURVE/") + "OISForecast"];
	pvanilla->getIRCurve(ccyInput).getInterpolation(fcurvestr1m).convertFromString(FN_SPLINEINTERPOLATION_STR);
	pvanilla->getIRCurve(ccyInput).getInterpolation(fcurvestr3m).convertFromString(FN_SPLINEINTERPOLATION_STR);
	pvanilla->getIRCurve(ccyInput).getInterpolation(fcurvestr6m).convertFromString(FN_SPLINEINTERPOLATION_STR);
	pvanilla->getIRCurve(ccyInput).getInterpolation(fcurvestrois).convertFromString(FN_SPLINEINTERPOLATION_STR);
	///////////////////////////////////////////////////////////////////////////////

	//global shift scenario////////////////////////////////////////////////
	std::map<AQLString, AQLString>::iterator it = mapcalcinfo.find(AQLString("CURVE/") + "GlobalShiftScenario");
	if (it != mapcalcinfo.end() && it->second != "" && it->second.getDoubleValue() != 0.0)
	{

		////set up order 
		AQLStringVector setupOrder = getCurveNamesFromSetUpOrder(dataInstance, curveID);
		AQLString glshiftstr = it->second;
		double glshiftval = glshiftstr.getDoubleValue() * 0.0001;
		AQLString orgcurveID = curveID;
		curveID += "_GlobalShift_" + glshiftstr + "BP";
		for (unsigned int j = 0; j < setupOrder.size(); j++)
		{
			AQLString generateCurveName = setupOrder[j];
			
			
			AQLString orgkeyname = "lineparam1_" + orgcurveID + generateCurveName;
			AQLString newkeyname = "lineparam1_" + curveID + generateCurveName;
			
			objHolder = objPool.getObject(newkeyname, ENCHKTYPE_NOCHECK);
			if (objHolder.isDefined())
				objPool.remove(newkeyname);

							
			AQLObject* eglcurvestore = objPool.getObject(orgkeyname, ENCHKTYPE_ISDEFINED).clone();
			objPool.set(newkeyname, eglcurvestore);
			eglcurvestore->getData(CALIBRATION_DATA_NAME, ISNOTNULL).convertFromString(newkeyname);

			bool isshiftarget = (generateCurveName == fcurvestrois || generateCurveName == fcurvestr3m);

			//create shiftcurve
			createShiftCurves(dataInstance, *eglcurvestore, generateCurveName,curveID,isshiftarget,true, 
											glshiftval,0,"ALL",true);
		}


		//reset into curve pro
		AQLString yieldName = "YIELD_" + curveID;
		pvanilla->getIRCurves().convertFromString(yieldName);
		////reset yldpro
		//AQLString yieldProName = "PRO_YIELD_" + curveID;
		//pvanilla->getIRCurvePros().convertFromString(yieldProName);

		//asofdate
		pyldEntity = &(objPool.getObject(curveID,ENCHKTYPE_ISDEFINED).get());
		dh = &(pyldEntity->getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL));
		AQLString asofdateInput = dh->convertToString();
		pvanilla->getData(CALIBRATION_DATA_ASOFDATE, ISDEFINED).convertFromString(asofdateInput);
		
		//ircurs
		//AQLStringVector irccys(ccy,1)
		dh = &(pyldEntity->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL));
		AQLString ccyInput = dynamic_cast<AQLDataString &>(dh->get());
		pvanilla->getIRCurrencys().convertFromString(ccyInput);
	}

	AQLString ret = marketparamInput;
	return ret;
}


AQLString
AQLMultiSwapPricer::getDiscountCurveName(AQLDataInstance* dataInstance, AQLString curveID)
{
	AQLString ret;
	AQLObjectPool& objPool = dataInstance->getObjectPool();




	const AQLString lineparam("lineparam1");
	const AQLObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();
    const AQLObject* pyldEntity = &(objPool.getObject(curveID, ENCHKTYPE_ISDEFINED).get());

    
	const AQLString swapfreq = dynamic_cast<const AQLDataString &>(eline->getData("SWAP_FREQUENCY", ISNOTNULL).get()).get();

	const AQLString ois_fcurve_name = dynamic_cast<const AQLDataString &>(eline->getData("CURVE_OISFORECAST", ISNOTNULL).get()).get();
	const AQLString std_fcurve_name = dynamic_cast<const AQLDataString &>(eline->getData(AQLString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get()).get();


    const AQLString& ccy = dynamic_cast<const AQLDataString&>(pyldEntity->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
    //const AQLString& ypro_name = "PRO_" + AQLMarketData::getBaseYieldName(ccy);
    const AQLString& ypro_name = AQLString("PRO_") + PREFIX_YIELD + curveID;
    const AQLMathYieldCurvePro& ypro = dynamic_cast<const AQLMathYieldCurvePro&>(objPool.getObject(ypro_name, ENCHKTYPE_ISDEFINED).get());
    const AQLString& ois_mkt_name = ypro.getMarketForCurve(ois_fcurve_name);


	
    const AQLString ois_dcurve_data_name = IR_CALIBRATION_DATA_DFCURVENAME + AQLString("_") + ois_mkt_name;
	const AQLString std_dcurve_name = dynamic_cast<const AQLDataString &>(pyldEntity->getData(IR_CALIBRATION_DATA_DFCURVENAME, ISNOTNULL).get());
	const AQLString ois_dcurve_name = dynamic_cast<const AQLDataString &>(pyldEntity->getData(ois_dcurve_data_name , ISNOTNULL).get());







	if ("ITSELF" == std_dcurve_name && std_fcurve_name == ois_dcurve_name)
		ret = std_fcurve_name;
	else if (ois_fcurve_name == std_dcurve_name && "ITSELF" == ois_dcurve_name)
		ret = ois_fcurve_name;
	else
		throw AQLCoreInvalidData("Discount curve set logic is not consistent", __FILE__,__LINE__);

	return ret;
}



AQLString
AQLMultiSwapPricer::getCurveIDfromTradeReference(AQLDataInstance* dataInstance, AQLString tradeID)
{

	AQLString ret;
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLDataHolder* dh;

	AQLString lineparam("lineparam1");
	AQLObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();

	AQLString ccyInput = dynamic_cast<AQLDataString &>(eline->getData("CALC_CURRENCY", ISNOTNULL).get());
	
	//discount curve search
	AQLObjectHolder objHolder = objPool.getObject(tradeID, ENCHKTYPE_ISDEFINED);
	AQLMathObjectValue& val = dynamic_cast<AQLMathObjectValue &>(objHolder.get());

	dh = &(val.getData(PRICING_DATA_PATHENTITY, ISNOTNULL));
	AQLDataReference& ref = dynamic_cast<AQLDataReference& >(dh->get());
	AQLMathPlainVanillaEntity& vanilla = dynamic_cast<AQLMathPlainVanillaEntity &>(ref.get().get());
	
	AQLDataReference& yielddata = dynamic_cast<AQLDataReference& >(vanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).get());
	return yielddata.get().getName();

}

AQLString
AQLMultiSwapPricer::getMarketParamfromTradeReference(AQLDataInstance* dataInstance, AQLString tradeID)
{

	AQLString ret;
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLDataHolder* dh;

	AQLString lineparam("lineparam1");
	AQLObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();

	AQLString ccyInput = dynamic_cast<AQLDataString &>(eline->getData("CALC_CURRENCY", ISNOTNULL).get());
	
	//discount curve search
	AQLObjectHolder objHolder = objPool.getObject(tradeID, ENCHKTYPE_ISDEFINED);
	AQLMathObjectValue& val = dynamic_cast<AQLMathObjectValue &>(objHolder.get());

	dh = &(val.getData(PRICING_DATA_PATHENTITY, ISNOTNULL));
	AQLDataReference& ref = dynamic_cast<AQLDataReference &>(dh->get());

	return ref.get().getName();

}

void
AQLMultiSwapPricer::setUpLineParams(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> mapcalcinfo)
{
	AQLObjectHolder objHolder;
	AQLObjectPool& objPool = dataInstance->getObjectPool();

	///////////////////line param object///////////////////////////////
	//interpolationset, usually, we should on curve generate
	AQLString fcurvestr1m = mapcalcinfo[AQLString("CURVE/") + "1MForecast"];
	AQLString fcurvestr3m = mapcalcinfo[AQLString("CURVE/") + "3MForecast"];
	AQLString fcurvestr6m = mapcalcinfo[AQLString("CURVE/") + "6MForecast"];
	AQLString fcurvestrois = mapcalcinfo[AQLString("CURVE/") + "OISForecast"];
	AQLString discount = mapcalcinfo[AQLString("CURVE/") + "Discount"];
	AQLString lineparam("lineparam1");
	objHolder = objPool.getObject(lineparam, ENCHKTYPE_NOCHECK);
	AQLObject* eline = NULL;
	if (!objHolder.isDefined())
	{
		eline = new AQLObject();
		objPool.set(lineparam, eline);
	}
	else
	{
		eline = &(objHolder.get());
	}
	
	//calc info
	AQLString calcstr = mapcalcinfo[AQLString("CALC/") + IR_CALIBRATION_DATA_CURRENCY];
	eline->remove("CALC_CURRENCY");
	eline->add("CALC_CURRENCY", new AQLDataString(calcstr));

	AQLString calcasofstr = mapcalcinfo[AQLString("CALC/") + CALIBRATION_DATA_ASOFDATE];
	eline->remove("CALC_ASOFDATE");
	eline->add("CALC_ASOFDATE", new AQLDataString(calcasofstr));

	AQLString calcvaluestr = mapcalcinfo[AQLString("CALC/") + CALIBRATION_DATA_VALUEDATE];
	eline->remove("CALC_VALUEDATE");
	eline->add("CALC_VALUEDATE", new AQLDataString(calcvaluestr));


	//curve info
	eline->remove("CURVE_1MFORECAST");
	eline->add("CURVE_1MFORECAST", new AQLDataString(fcurvestr1m));

	eline->remove("CURVE_3MFORECAST");
	eline->add("CURVE_3MFORECAST", new AQLDataString(fcurvestr3m));
	
	eline->remove("CURVE_6MFORECAST");
	eline->add("CURVE_6MFORECAST", new AQLDataString(fcurvestr6m));

	eline->remove("CURVE_OISFORECAST");
	eline->add("CURVE_OISFORECAST", new AQLDataString(fcurvestrois));

	eline->remove("CURVE_DISCOUNT");
	if (discount.isDefined())
	{
		eline->add("CURVE_DISCOUNT", new AQLDataString(discount));
	}

	
	// isforward roll
	AQLString isforwardstr1 = mapcalcinfo[AQLString("TRADE/SWAP/") + PRICING_DATA_ISFORWARDROLL];
	AQLString isforwardstr2 = mapcalcinfo[AQLString("TRADE/BASISSWAP/") + PRICING_DATA_ISFORWARDROLL];
	AQLString isforwardstr3 = mapcalcinfo[AQLString("TRADE/FRA/") + PRICING_DATA_ISFORWARDROLL];
	
	eline->remove(AQLString("TRADE_SWAP_") + PRICING_DATA_ISFORWARDROLL);
	eline->add(AQLString("TRADE_SWAP_") + PRICING_DATA_ISFORWARDROLL, new AQLDataString(isforwardstr1));

	eline->remove(AQLString("TRADE_BASISSWAP_") + PRICING_DATA_ISFORWARDROLL);
	eline->add(AQLString("TRADE_BASISSWAP_") + PRICING_DATA_ISFORWARDROLL, new AQLDataString(isforwardstr2));

	//up to now, fra is always true
	eline->remove(AQLString("TRADE_FRA_") + PRICING_DATA_ISFORWARDROLL);
	eline->add(AQLString("TRADE_FRA_") + PRICING_DATA_ISFORWARDROLL, new AQLDataString(isforwardstr3));

	// isforward inter
	AQLString isforwardinterstr1 = mapcalcinfo[AQLString("TRADE/SWAP/") + PRICING_DATA_ISFWDINTERPOLATION];
	AQLString isforwardinterstr2 = mapcalcinfo[AQLString("TRADE/BASISSWAP/") + PRICING_DATA_ISFWDINTERPOLATION];
	AQLString isforwardinterstr3 = mapcalcinfo[AQLString("TRADE/FRA/") + PRICING_DATA_ISFWDINTERPOLATION];

	eline->remove(AQLString("TRADE_SWAP_") + PRICING_DATA_ISFWDINTERPOLATION);
	eline->add(AQLString("TRADE_SWAP_") + PRICING_DATA_ISFWDINTERPOLATION, new AQLDataString(isforwardinterstr1));

	eline->remove(AQLString("TRADE_BASISSWAP_") + PRICING_DATA_ISFWDINTERPOLATION);
	eline->add(AQLString("TRADE_BASISSWAP_") + PRICING_DATA_ISFWDINTERPOLATION, new AQLDataString(isforwardinterstr2));

	eline->remove(AQLString("TRADE_FRA_") + PRICING_DATA_ISFWDINTERPOLATION);
	eline->add(AQLString("TRADE_FRA_") + PRICING_DATA_ISFWDINTERPOLATION, new AQLDataString(isforwardinterstr3));

	// paymenttiming
	AQLString paytimingstr1 = mapcalcinfo[AQLString("TRADE/SWAP/") + PRICING_DATA_PAYMENTTIMING];
	AQLString paytimingstr2 = mapcalcinfo[AQLString("TRADE/BASISSWAP/") + PRICING_DATA_PAYMENTTIMING];
	AQLString paytimingstr3 = mapcalcinfo[AQLString("TRADE/FRA/") + PRICING_DATA_PAYMENTTIMING];
	
	eline->remove(AQLString("TRADE_SWAP_") + PRICING_DATA_PAYMENTTIMING);
	eline->add(AQLString("TRADE_SWAP_") + PRICING_DATA_PAYMENTTIMING, new AQLDataString(paytimingstr1));
	
	eline->remove(AQLString("TRADE_BASISSWAP_") + PRICING_DATA_PAYMENTTIMING);
	eline->add(AQLString("TRADE_BASISSWAP_") + PRICING_DATA_PAYMENTTIMING, new AQLDataString(paytimingstr2));
	//up to now, fra is always advance payment
	eline->remove(AQLString("TRADE_FRA_") + PRICING_DATA_PAYMENTTIMING);
	eline->add(AQLString("TRADE_FRA_") + PRICING_DATA_PAYMENTTIMING, new AQLDataString(paytimingstr3));

	//payment calendar
	AQLString paycalstr1 = mapcalcinfo[AQLString("TRADE/SWAP/") + CALIBRATION_DATA_CALENDAR];
	AQLString paycalstr2 = mapcalcinfo[AQLString("TRADE/BASISSWAP/") + CALIBRATION_DATA_CALENDAR];
	AQLString paycalstr3 = mapcalcinfo[AQLString("TRADE/FRA/") + CALIBRATION_DATA_CALENDAR];
	
	eline->remove(AQLString("TRADE_SWAP_") + CALIBRATION_DATA_CALENDAR);
	eline->add(AQLString("TRADE_SWAP_") + CALIBRATION_DATA_CALENDAR, new AQLDataString(paycalstr1));

	eline->remove(AQLString("TRADE_BASISSWAP_") + CALIBRATION_DATA_CALENDAR);
	eline->add(AQLString("TRADE_BASISSWAP_") + CALIBRATION_DATA_CALENDAR, new AQLDataString(paycalstr2));

	eline->remove(AQLString("TRADE_FRA_") + CALIBRATION_DATA_CALENDAR);
	eline->add(AQLString("TRADE_FRA_") + CALIBRATION_DATA_CALENDAR, new AQLDataString(paycalstr3));

	//payment business days convention
	AQLString bdconv1 = mapcalcinfo[AQLString("TRADE/SWAP/") + CALIBRATION_DATA_SLIDINGRULE];
	AQLString bdconv2 = mapcalcinfo[AQLString("TRADE/BASISSWAP/") + CALIBRATION_DATA_SLIDINGRULE];
	AQLString bdconv3 = mapcalcinfo[AQLString("TRADE/FRA/") + CALIBRATION_DATA_SLIDINGRULE];

	eline->remove(AQLString("TRADE_SWAP_") + CALIBRATION_DATA_SLIDINGRULE);
	eline->add(AQLString("TRADE_SWAP_") + CALIBRATION_DATA_SLIDINGRULE, new AQLDataString(bdconv1));

	eline->remove(AQLString("TRADE_BASISSWAP_") + CALIBRATION_DATA_SLIDINGRULE);
	eline->add(AQLString("TRADE_BASISSWAP_") + CALIBRATION_DATA_SLIDINGRULE, new AQLDataString(bdconv2));

	eline->remove(AQLString("TRADE_FRA_") + CALIBRATION_DATA_SLIDINGRULE);
	eline->add(AQLString("TRADE_FRA_") + CALIBRATION_DATA_SLIDINGRULE, new AQLDataString(bdconv3));

	AQLString ccyInput = mapcalcinfo[AQLString("CALC/") + IR_CALIBRATION_DATA_CURRENCY];
	//fixing calenar
	AQLString fixingcalstr1 = mapcalcinfo[AQLString("INDEX/LIBOR/") + PRICING_DATA_FIXINGCALENDAR];
	AQLString fixingcalstr2 = mapcalcinfo[AQLString("INDEX/OIS/") + PRICING_DATA_FIXINGCALENDAR];

	eline->remove(AQLString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_FIXINGCALENDAR);
	eline->add(AQLString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_FIXINGCALENDAR,  new AQLDataString(fixingcalstr1));

	eline->remove(AQLString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_FIXINGCALENDAR);
	eline->add(AQLString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_FIXINGCALENDAR,  new AQLDataString(fixingcalstr2));

	//fixing timing
	AQLString fixingtimingstr1 = mapcalcinfo[AQLString("INDEX/LIBOR/") + PRICING_DATA_FIXINGTIMING];
	AQLString fixingtimingstr2 = mapcalcinfo[AQLString("INDEX/OIS/") + PRICING_DATA_FIXINGTIMING];

	eline->remove(AQLString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_FIXINGTIMING);
	eline->add(AQLString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_FIXINGTIMING,  new AQLDataString(fixingtimingstr1));

	eline->remove(AQLString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_FIXINGTIMING);
	eline->add(AQLString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_FIXINGTIMING,  new AQLDataString(fixingtimingstr2));

	//spot lag
	AQLString spotlagstr1 = mapcalcinfo[AQLString("INDEX/LIBOR/") + PRICING_DATA_SPOTLAG];
	AQLString spotlagstr2 = mapcalcinfo[AQLString("INDEX/OIS/") + PRICING_DATA_SPOTLAG];

	eline->remove(AQLString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_SPOTLAG);
	eline->add(AQLString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_SPOTLAG,  new AQLDataString(spotlagstr1));

	eline->remove(AQLString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_SPOTLAG);
	eline->add(AQLString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_SPOTLAG,  new AQLDataString(spotlagstr2));

	//swap frequency
	AQLStaticData &irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString tmpCurrency;
	tmpCurrency = calcstr; tmpCurrency.toLower();
	AQLString swapfreq = irStaticData.getStaticData(tmpCurrency + ".sde.yield.swap.frequencyfloat");
	if(swapfreq == "quarterly")
		swapfreq = "3M";
	else if(swapfreq == "semi-annual")
		swapfreq = "6M";
	else if(swapfreq == "monthly" || swapfreq == "lunar")
		swapfreq = "1M";
	else
		throw AQLCoreInvalidData("Cannot find swap frequency",__FILE__,__LINE__);

	if(calcstr == "AUD")	//special case!!
		swapfreq = "3M";
	eline->remove("SWAP_FREQUENCY");
	eline->add("SWAP_FREQUENCY",  new AQLDataString(swapfreq));


}

//in the current case, there exist four curves libor3m libor6m libor1m and ois
//if we need more curve to generate, we have to modify this function
//e.g. xccy curves (arbitrage free case) case, we need more.
AQLStringVector
AQLMultiSwapPricer::getCurveNamesFromSetUpOrder(AQLDataInstance* dataInstance, AQLString curveID)
{
	AQLStringVector ret(4);
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLString lineparam("lineparam1");
	AQLObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();

	AQLString swapfreq = dynamic_cast<AQLDataString &>(eline->getData("SWAP_FREQUENCY", ISNOTNULL).get());

	AQLString fcurvestr1m = dynamic_cast<AQLDataString &>(eline->getData("CURVE_1MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr3m = dynamic_cast<AQLDataString &>(eline->getData("CURVE_3MFORECAST", ISNOTNULL).get());
	AQLString fcurvestr6m = dynamic_cast<AQLDataString &>(eline->getData("CURVE_6MFORECAST", ISNOTNULL).get());
	AQLString fcurvestrois = dynamic_cast<AQLDataString &>(eline->getData("CURVE_OISFORECAST", ISNOTNULL).get());
	AQLString fcurvestrstd = dynamic_cast<AQLDataString &>(eline->getData(AQLString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get());

	
	AQLDataHolder* dh;
	dh =&(eline->getData("CURVE_DISCOUNT", NOCHECK));
	AQLString discountcurve;
	if (!dh->isDefined() || dh->isNull())
	{
		discountcurve = getDiscountCurveName(dataInstance,curveID);
	}
	else
	{
		discountcurve = dynamic_cast<AQLDataString &>(eline->getData("CURVE_DISCOUNT", NOCHECK).get());;
	}
	

	//AQLString discountcurve = getDiscountCurveName(dataInstance, curveID);
	////set up order 
	bool isoisbase = true;
	if (discountcurve == fcurvestrois)
		isoisbase = true;
	else if (discountcurve == fcurvestrstd)
		isoisbase = false;
	else
		throw AQLCoreInvalidData("Curve set inconsisntent",__FILE__,__LINE__);

	bool is3mbase = true;
	if (swapfreq != "3M")
		is3mbase = false;

	if (isoisbase && is3mbase)
	{
		ret[0] = fcurvestrois;
		ret[1] = fcurvestr3m;
		ret[2] = fcurvestr6m;
		ret[3] = fcurvestr1m;
		
	}
	else if (!isoisbase && is3mbase)
	{
		ret[0] = fcurvestr3m;
		ret[1] = fcurvestr6m;
		ret[2] = fcurvestr1m;
		ret[3] = fcurvestrois;
	}

	else if (isoisbase && !is3mbase)
	{
		ret[0] = fcurvestrois;
		ret[1] = fcurvestr6m;
		ret[2] = fcurvestr3m;
		ret[3] = fcurvestr1m;
	}

	else if (!isoisbase && !is3mbase)
	{
		ret[0] = fcurvestr6m;
		ret[1] = fcurvestr3m;
		ret[2] = fcurvestr1m;
		ret[3] = fcurvestrois;
	}


	return ret;

}

void
AQLMultiSwapPricer::createShiftCurves(AQLDataInstance* dataInstance, AQLObject& eorgCurve, AQLString targetMarketName, AQLString newCurveID, bool isshiftarget, bool ispara, 
											double shiftval, unsigned int shiftpos, AQLString market, bool isglobalscenario)
{
	AQLDataHolder* dh;
	AQLDataHolder* se;
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	
	AQLString lineparam("lineparam1");
	AQLObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();

	AQLString swapfreq = dynamic_cast<AQLDataString &>(eline->getData("SWAP_FREQUENCY", ISNOTNULL).get());

	AQLString libor1mforecurve = dynamic_cast<AQLDataString &>(eline->getData("CURVE_1MFORECAST", ISNOTNULL).get());
	AQLString libor3mforecurve = dynamic_cast<AQLDataString &>(eline->getData("CURVE_3MFORECAST", ISNOTNULL).get());
	AQLString libor6mforecurve = dynamic_cast<AQLDataString &>(eline->getData("CURVE_6MFORECAST", ISNOTNULL).get());
	AQLString oisforecurve = dynamic_cast<AQLDataString &>(eline->getData("CURVE_OISFORECAST", ISNOTNULL).get());

	// boolean of libor base curve
	bool isbase, isnobase;
	if(swapfreq == "3M"){
		isbase = (targetMarketName == libor3mforecurve);
		isnobase = (targetMarketName == libor1mforecurve || targetMarketName == libor6mforecurve);
	} else if(swapfreq == "6M"){
		isbase = (targetMarketName == libor6mforecurve);
		isnobase = (targetMarketName == libor1mforecurve || targetMarketName == libor3mforecurve);
	}

	//mir set up ois
	if (targetMarketName == oisforecurve)
	{	
		AQLString genCurveName;
		dh = &(eorgCurve.getData("genCurveName", ISNOTNULL));
		AQLString tmp_genCurveName;
		se =&(eline->getData("CURVE_DISCOUNT", NOCHECK));
		if (!se->isDefined() || se->isNull())
		{
			tmp_genCurveName = genCurveName = dynamic_cast<AQLDataString &>(dh->get());
		}
		else
		{
			tmp_genCurveName = dynamic_cast<AQLDataString &>(eline->getData("CURVE_DISCOUNT", NOCHECK).get());
		}
		genCurveName = tmp_genCurveName;
		
		AQLStringMatrix generateProp,oisRates,oisConv;
		dh = &(eorgCurve.getData("generateProp"));
		if (dh->isDefined() && !dh->isNull())
			generateProp = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("oisRates"));
		if (dh->isDefined() && !dh->isNull())
			oisRates = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("oisConv"));
		if (dh->isDefined() && !dh->isNull())
			oisConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		AQLStringMatrix histRates;
		dh = &(eorgCurve.getData("histRates"));
		if (dh->isDefined() && !dh->isNull())
			histRates = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		//change shift rate
		if (isshiftarget)
			AQLMultiSwapPricer::shiftMarketRate(oisRates,shiftval,ispara,shiftpos);

		if (isglobalscenario)
			dynamic_cast<AQLDataStringMatrix &>(eorgCurve.getData("oisRates", ISNOTNULL).get()).set(oisRates);
		
		//AQLCurveSetup::setUpOISCurve(dataInstance,newCurveID,genCurveName,generateProp,oisRates,oisConv,"", histRates);
		AQLCurveSetup::setUpOISCurve(dataInstance,newCurveID,genCurveName,generateProp,oisRates,oisConv,targetMarketName, histRates);
	}
//	else if (targetMarketName == libor3mforecurve)
	else if (isbase)
	{
		//mir set up swap;
		AQLString genCurveName;
		dh = &(eorgCurve.getData("genCurveName", ISNOTNULL));
		genCurveName = dynamic_cast<AQLDataString &>(dh->get());

		AQLStringMatrix generateProp,moneyConv,liborRates,liborConv,swapRates,swapConv,
			fra3mRates,fra6mRates,fraConv,futureRates,futureConv,adjustSwapConv,adjustSwapRates;
		
		dh = &(eorgCurve.getData("generateProp"));
		if (dh->isDefined() && !dh->isNull())
			generateProp = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("moneyConv"));
		if (dh->isDefined() && !dh->isNull())
			moneyConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("liborRates"));
		if (dh->isDefined() && !dh->isNull())
			liborRates = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("liborConv"));
		if (dh->isDefined() && !dh->isNull())
			liborConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("swapRates"));
		if (dh->isDefined() && !dh->isNull())
			swapRates = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("swapConv"));
		if (dh->isDefined() && !dh->isNull())
			swapConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fra3mRates"));
		if (dh->isDefined() && !dh->isNull())
			fra3mRates = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fra6mRates"));
		if (dh->isDefined() && !dh->isNull())
			fra6mRates = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fraConv"));
		if (dh->isDefined() && !dh->isNull())
			fraConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("futureRates"));
		if (dh->isDefined() && !dh->isNull())
			futureRates = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("futureConv"));
		if (dh->isDefined() && !dh->isNull())
			futureConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("adjustSwapConv"));
		if (dh->isDefined() && !dh->isNull())
			adjustSwapConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("adjustSwapRates"));
		if (dh->isDefined() && !dh->isNull())
			adjustSwapRates = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();


		//change shift rate
		if (!liborRates.empty() && isshiftarget && market == "ALL")
		{
			AQLMultiSwapPricer::shiftMarketRate(liborRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<AQLDataStringMatrix &>(eorgCurve.getData("liborRates").get()).set(liborRates);
		}
		else if (!liborRates.empty() && isshiftarget && market == "liborRates")
			AQLMultiSwapPricer::shiftMarketRate(liborRates,shiftval,ispara,shiftpos);

		if (!swapRates.empty() && isshiftarget && market == "ALL")
		{
			AQLMultiSwapPricer::shiftMarketRate(swapRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<AQLDataStringMatrix &>(eorgCurve.getData("swapRates").get()).set(swapRates);
		}
		else if (!swapRates.empty() && isshiftarget && market == "swapRates")
			AQLMultiSwapPricer::shiftMarketRate(swapRates,shiftval,ispara,shiftpos);

		if (!fra3mRates.empty() && isshiftarget && market == "ALL")
		{
			AQLMultiSwapPricer::shiftMarketRate(fra3mRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<AQLDataStringMatrix &>(eorgCurve.getData("fra3mRates").get()).set(fra3mRates);
		
		}
		else if (!fra3mRates.empty() && isshiftarget && market == "fra3mRates")
			AQLMultiSwapPricer::shiftMarketRate(fra3mRates,shiftval,ispara,shiftpos);

		if (!fra6mRates.empty() && isshiftarget && market == "ALL")
		{
			AQLMultiSwapPricer::shiftMarketRate(fra6mRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<AQLDataStringMatrix &>(eorgCurve.getData("fra6mRates").get()).set(fra6mRates);

		}
		else if (!fra6mRates.empty() && isshiftarget && market == "fra6mRates")
			AQLMultiSwapPricer::shiftMarketRate(fra6mRates,shiftval,ispara,shiftpos);

		if (!futureRates.empty() && isshiftarget && market == "ALL")
		{
			AQLMultiSwapPricer::shiftMarketRate(futureRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<AQLDataStringMatrix &>(eorgCurve.getData("futureRates").get()).set(futureRates);

		}
		else if (!futureRates.empty() && isshiftarget && market == "futureRates")
			AQLMultiSwapPricer::shiftMarketRate(futureRates,shiftval,ispara,shiftpos);
		
		if (!adjustSwapRates.empty() && isshiftarget && market == "ALL")
		{
			AQLMultiSwapPricer::shiftMarketRate(adjustSwapRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<AQLDataStringMatrix &>(eorgCurve.getData("adjustSwapRates").get()).set(adjustSwapRates);
		}
		else if (!adjustSwapRates.empty() && isshiftarget && market == "adjustSwapRates")
			AQLMultiSwapPricer::shiftMarketRate(adjustSwapRates,shiftval,ispara,shiftpos);

		AQLCurveSetup::setUpSwapCurve(dataInstance,newCurveID,genCurveName,generateProp,
								moneyConv,liborRates,liborConv,swapRates,swapConv,
								fra3mRates,fra6mRates,fraConv,futureRates,futureConv,
								//adjustSwapConv,adjustSwapRates,"","");
								adjustSwapConv,adjustSwapRates,targetMarketName,"");

	}
//	else if (targetMarketName == libor1mforecurve || targetMarketName == libor6mforecurve)
	else if (isnobase)
	{
		//mir set up basis swap
		AQLString basisCurveName;
		dh = &(eorgCurve.getData("basisCurveName", ISNOTNULL));
		basisCurveName = dynamic_cast<AQLDataString &>(dh->get());

		AQLStringMatrix basisMkt,basisConv,fwdFX,fwdConv,spotFX,generateProp,moneyConv;
		
		dh = &(eorgCurve.getData("basisMkt"));
		if (dh->isDefined() && !dh->isNull())
			basisMkt = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("basisConv"));
		if (dh->isDefined() && !dh->isNull())
			basisConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fwdFX"));
		if (dh->isDefined() && !dh->isNull())
			fwdFX = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fwdConv"));
		if (dh->isDefined() && !dh->isNull())
			fwdConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("spotFX"));
		if (dh->isDefined() && !dh->isNull())
			spotFX = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("generateProp"));
		if (dh->isDefined() && !dh->isNull())
			generateProp = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("moneyConv"));
		if (dh->isDefined() && !dh->isNull())
			moneyConv = dynamic_cast<AQLDataStringMatrix &>(dh->get()).get();


		//change shift rate
		if (isshiftarget && market == "ALL")
			AQLMultiSwapPricer::shiftMarketRate(basisMkt,shiftval,ispara,shiftpos);
		else if (isshiftarget && market == "basisMkt")
			AQLMultiSwapPricer::shiftMarketRate(basisMkt,shiftval,ispara,shiftpos);

		//AQLCurveSetup::setUpBasisCurve(dataInstance,newCurveID,basisCurveName,basisMkt,basisConv,generateProp,moneyConv,"");
		AQLCurveSetup::setUpBasisCurve(dataInstance,newCurveID,basisCurveName,basisMkt,basisConv,fwdFX,fwdConv,spotFX,generateProp,moneyConv,targetMarketName);
	}
	else
		throw AQLCoreInvalidData("Source Delta Scenario Error",__FILE__,__LINE__);


	return;
}
