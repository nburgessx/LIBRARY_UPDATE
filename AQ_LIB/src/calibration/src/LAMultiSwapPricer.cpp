
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LADataBasics.h"
#include "LADataVector.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LAMultiSwapPricer.h"
#include "LACalibrateModelIRVanilla.h"
#include "LADefinitions.h"
#include "LAStaticData.h"
#include "LAFunctionUtilities.h"
#include "LAMathDateUtilities.h"
#include <cmath>
#include <map>

#define CALIBRATION_DATA_CURVEID					"CurveID"

#include "LAMathValuableEntity.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAPriceCashFlowGenerator.h"
#include "LALinearFunc.h"
#include "LAPricePayOff.h"
#include "LALinearRatesSwapTradeValue.h"
#include "LAPriceDataFunction.h"
#include "LAMathIndexEntity.h"
#include <algorithm>
#include "LAObject.h"
#include "LAPriceDataInterpolation.h"
#include "LASplineInterpolation.h"
#include "LALinearInterpolation.h"
#include "LADataMultiReference.h"
#include "LADataReference.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LADataProcedure.h"
#include "LAString.h"
#include "LADataMatrix.h"
#include "LAPriceDataManager.h"
#include "LAPriceConvergenceValue.h"

#include "LAPricePortfolioValue.h"
#include "LACalibrationUtilities.h"
#include "LACurveSetup.h"
#include "LACompoundingFunc.h"

#include "LAMarketData.h"

using namespace std;

DoubleArray
LAMultiSwapPricer::convergentPlainVanillaTrade(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade)
{
	LAObjectPool &objPool = dataInstance->getObjectPool();
	LAObjectHolder objHolder;
	LADataHolder* dh;
	//important clasee "lineparam1"
	LAObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	LAString asofdateInput = dynamic_cast<LADataString &>(eline.getData("CALC_ASOFDATE", ISNOTNULL).get());
	LADate basedate(asofdateInput.getCString(),"YYYYMMDD");


	///////////trade infors/////////////////////////
	//at first no calcvec
	LAString nocalcswapstr = maptrade["NoCalcSwapTrades"];
	LAStringVector tradeNoCalcInputVec;
	if (nocalcswapstr != "")
		throw LACoreInvalidData("No calc mode",__FILE__,__LINE__);
	//if (nocalcswapstr != "")
	//	tradeNoCalcInputVec = nocalcswapstr.toToken(':');
	
	//get tradevec;
	LAString calcswapstr = maptrade["CalcSwapTrades"];
	LAStringVector tradeInputVec;
	if (calcswapstr != "")
		tradeInputVec = calcswapstr.toToken(':');

	LAStringVector tradeVec;
	//if (tradeNoCalcInputVec.size() == 1)
	//	tradeVec = tradeNoCalcInputVec;
	//else if (tradeInputVec.size() == 1)
	//	tradeVec = tradeInputVec;
	//else 
	//	throw LACoreInvalidData("convergent set error",__FILE__,__LINE__);
	
	if (tradeInputVec.size() != 0)
		tradeVec = tradeInputVec;
	else 
		throw LACoreInvalidData("convergent set error",__FILE__,__LINE__);

	

	//in convergence case, the purpose is only getting the result,
	//we do not need save it, so that (1) clone (2) get the result (3) delete.

	
	//single trade 
	LAString key_suffix = "_CLONE";
	DoubleArray ret;
	for (unsigned int i = 0; i < tradeVec.size(); ++i)
	{
		createSingleTrade(dataInstance,maptrade,tradeVec[i],true, key_suffix,true);

		//from renee,the par rate should depend on the trade details entered 
		//(dates, leg conventions, etc) and not on the rate, spread or bid/offer.
		
		//create offerbid curve
		//createOfferBidAdjustEntity(dataInstance,maptrade,tradeVec,true,key_suffix);
		
		
		LAStringVector tmpVec(1, tradeVec[i] + key_suffix);
		//change into convergence intofixed rate;
		LAMathObjectValue& eval = dynamic_cast<LAMathObjectValue& >(objPool.getObject(tmpVec[0], ENCHKTYPE_ISDEFINED).get());
		eval.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).convertFromString(FN_IR_CONVERGENCEVALUE_STR);
		eval.remove(PRICING_DATA_SUBVALUE);
		eval.LAObject::add(PRICING_DATA_SUBVALUE, new LADataValuation()).convertFromString(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
		eval.remove(PRICING_DATA_DIRTYPRICE);
		eval.LAObject::add(PRICING_DATA_DIRTYPRICE, new LADataDouble(0.0));
		//check line product 
		LAString lineproduct = dynamic_cast<LADataString &>(eval.getData("LineProductType", ISNOTNULL).get()).get();
		LAString target = "";
		if (lineproduct == "SWAP" )
		{
			target = "LEG1FIXEDRATE";
			/*if (target !=  "LEG1FIXEDRATE" && target != "LEG2SPREAD")
				throw LACoreInvalidData("target error",__FILE__,__LINE__);*/

		}
		else if (lineproduct == "BASISSWAP")
		{
			target = "LEG1SPREAD";
			LAStringVector leg1Coeff_str = maptrade[tradeVec[i] + "/" + PRICING_DATA_COEFFICIENT + "/COUPON1"].toToken(':');
			if (leg1Coeff_str.size() != 2)
				throw LACoreInvalidData("convergentPlainVanillaTrade Failed! A size of leg1 coefficient is not 2!",__FILE__,__LINE__);
			double leg1Spread = leg1Coeff_str[1].getDoubleValue();

			LAStringVector leg2Coeff_str = maptrade[tradeVec[i] + "/" + PRICING_DATA_COEFFICIENT + "/COUPON2"].toToken(':');
			if (leg2Coeff_str.size() != 2)
				throw LACoreInvalidData("convergentPlainVanillaTrade Failed! A size of leg2 coefficient is not 2!",__FILE__,__LINE__);
			double leg2Spread = leg2Coeff_str[1].getDoubleValue();

			if (leg2Spread !=  0. && leg1Spread == 0.)
				target = "LEG2SPREAD";		
		}
		else if (lineproduct == "FRA")
		{
			target = "LEG1FIXEDRATE";
			/*if (target !=  "LEG1FIXEDRATE")
				throw LACoreInvalidData("target error",__FILE__,__LINE__);*/
		}

		eval.remove(PRICING_DATA_CONVERGENCETARGET);
		eval.LAObject::add(PRICING_DATA_CONVERGENCETARGET, new LADataString()).convertFromString(target);

		////set for offerbid curve pricing
		//eline.remove("offerbidentity");
		//eline.add("offerbidentity", new LADataReference()).convertFromString("lineofferbidentity");

		dataInstance->getReferencePool().completeDependency();
		DoubleVector convegVec;
		calcRiskAndPV(dataInstance,tmpVec,convegVec);
		
		//eline.remove("offerbidentity");

		
		dh = &(eval.getData(PRICING_DATA_CONVERGENCEVALUE, ISNOTNULL));
		double convValue = dynamic_cast<LADataDouble &>(dh->get());
		if (target == "LEG1FIXEDRATE")
			convValue *= 100; //% display
		else if (target == "LEG1SPREAD" || target == "LEG2SPREAD")
			convValue *= 10000; //basis point display
		ret.push_back(convValue);

		//in case of convergent at first we must expand cashflows.
		dh = &eval.getData(PRICING_DATA_CFGENERATOR, ISNOTNULL);
		LADataProcedure& modelDataObj = dynamic_cast<LADataProcedure&>(dh->get());
		const LAPriceCashFlowGenerator& cfgen = dynamic_cast<const LAPriceCashFlowGenerator &>(modelDataObj.getMethod());
		
		//delete cashlet coupon index
		//set Detail into Manual not to cashflow expand
		LADataMultiReference& refs = dynamic_cast<LADataMultiReference &>(eval.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		while (refs.getSize() > 0)
		{
			LAObjectHolder& ehleg = refs.get(0);
			cfgen.clearCashlets(ehleg.get());

			LAString legname = ehleg.getName();
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
LAMultiSwapPricer::calcPlainVanillaTrades(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade, DoubleVector& pvResultVec)
{
	LAObjectPool &objPool = dataInstance->getObjectPool();
	LAObjectHolder objHolder;
	//important clasee "lineparam1"
	LAObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();

	///////////trade infors/////////////////////////
	//at first no calcvec
	LAString nocalcswapstr = maptrade["NoCalcSwapTrades"];
	LAStringVector tradeNoCalcInputVec;
	if (nocalcswapstr != "")
		tradeNoCalcInputVec = nocalcswapstr.toToken(':');
	
	for (unsigned int i = 0; i < tradeNoCalcInputVec.size(); i++)
	{
		LAString tradeid = tradeNoCalcInputVec[i];
		LAMathObjectValue* vtrade = NULL;
		objHolder = objPool.getObject(tradeid);
		if (objHolder.isDefined())
		{
			if (!objHolder.isTypeOf(ENTITY_VENTITY))
				throw LACoreInvalidData("Trans ID Error",__FILE__,__LINE__);
			vtrade = dynamic_cast<LAMathObjectValue *>(&objHolder.get());
			vtrade->reset();
		}
		else
		{
			vtrade = new LAMathObjectValue(dataInstance);
			objPool.set(tradeid, vtrade);
		}

		//set mintradefile
		//name
		vtrade->getData(CALIBRATION_DATA_NAME, ISDEFINED).convertFromString(tradeid);
		vtrade->LAObject::remove("PV");
		vtrade->LAObject::add("PV", new LADataDouble(0.0));

		//line product
		LAString lineproduc = maptrade[tradeid + "/" + "LineProductType"];
		vtrade->LAObject::remove("LineProductType");
		vtrade->LAObject::add("LineProductType", new LADataString(lineproduc));

		//IsPnL
		LAString ispnl = maptrade[tradeid + "/" + "IsPnL"];
		vtrade->LAObject::remove("IsPnL");
		vtrade->LAObject::add("IsPnL", new LADataBool()).convertFromString(ispnl);

		//store pv result vec
		pvResultVec.push_back(0.0);

	}
	//////////////////////////////

	//get tradevec;
	LAString calcswapstr = maptrade["CalcSwapTrades"];
	LAStringVector tradeInputVec;
	if (calcswapstr != "")
		tradeInputVec = calcswapstr.toToken(':');
	
	for (unsigned int i = 0; i < tradeInputVec.size(); i++)
		createSingleTrade(dataInstance,maptrade,tradeInputVec[i],false, "");

	//if there exists easy risk, then set risk, otherwise only pv = not new anything in "riskentity"
	LAString riskentitystr;
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
		eline.add("riskentity", new LADataMultiReference()).convertFromString(riskentitystr);
	}
	
	dataInstance->getReferencePool().completeDependency();
	DoubleVector pvResultNonOfferBidVec;
	calcRiskAndPV(dataInstance,tradeInputVec,pvResultNonOfferBidVec);

	eline.remove("riskentity");

	
	//create offerbid curve
	createOfferBidAdjustEntity(dataInstance,maptrade,tradeInputVec,false, "");

	//set for offerbid curve pricing
	eline.remove("offerbidentity");
	eline.add("offerbidentity", new LADataReference()).convertFromString("lineofferbidentity");

	dataInstance->getReferencePool().completeDependency();
	calcRiskAndPV(dataInstance,tradeInputVec,pvResultVec);

	eline.remove("offerbidentity");

	return;
}

void
LAMultiSwapPricer::calcPlainVanillaTrades(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade, const LAStringVector& calcTargetVector, 
												 DoubleMatrix& pvResultMatrix)
{
	//check whether NoCalcTrade exists or not
	if (maptrade["NoCalcSwapTrades"] != "")
		throw LACoreInvalidData("No calc mode",__FILE__,__LINE__);

	//get tradevec;
	LAString calcswapstr = maptrade["CalcSwapTrades"];
	LAStringVector tradeVec;
	if (calcswapstr != "")
	{
		tradeVec = calcswapstr.toToken(':');
	}
	else
	{
		throw LACoreInvalidData("No calc trade",__FILE__,__LINE__);
	}

	DoubleVector pvResultVec;
	calcPlainVanillaTrades(dataInstance, maptrade, pvResultVec);

	DoubleVector parRateSpreadVec;
	if (find(calcTargetVector.begin(), calcTargetVector.end(), "ParRateSpread") != calcTargetVector.end())
	{
		parRateSpreadVec = convergentPlainVanillaTrade(dataInstance, maptrade);
	}

	LAObjectPool& objPool = dataInstance->getObjectPool();
	pvResultMatrix.clear();
	DoubleVector singleTradeResult(calcTargetVector.size());
	for (unsigned int i = 0; i < tradeVec.size(); ++i)
	{
		LAMathObjectValue& trade = dynamic_cast<LAMathObjectValue &>(objPool.getObject(tradeVec[i],ENCHKTYPE_ISDEFINED).get());

		for (unsigned int j = 0; j < calcTargetVector.size(); ++j)
		{
			LADataHolder* dh = &trade.getData(calcTargetVector[j]);

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
					LAString msg = "No " + calcTargetVector[j] + " in " + LAString(static_cast<int>(i)) + " th trade";
					throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
				}
			}
		}
		
		pvResultMatrix.push_back(singleTradeResult);
	}

}

void 
LAMultiSwapPricer::calcRiskAndPV(LADataInstance* dataInstance, const LAStringVector& tradeVec, DoubleVector& pvResultVec)
{
	LADataHolder* dh;
	LAObjectHolder objHolder;
	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAObject& eline = objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).get();
	
	unsigned int sizeScenario = 0;
	LAStringVector orgEntityName,pvriskScenario,riskDetail1,riskDetail2;
	LAStringVector discountcurves, paraEntityName;
	LAString ccyInput,discountcurve; 
	
	//get risk set object
	dh = &(eline.getData("riskentity", NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		//get risk info
		LADataMultiReference& refs = dynamic_cast<LADataMultiReference &>(dh->get());
		for (unsigned int i = 0; i < refs.getSize(); i++)
		{
			LAObject& erisk = refs.get(i).get();
			dh = &(erisk.getData("orgEntityName",ISNOTNULL));
			LAStringVector tmporgEntityName = dynamic_cast<LADataStrings &>(dh->get()).get();

			dh = &(erisk.getData("pvriskScenario", ISNOTNULL));
			LAStringVector tmppvriskScenario = dynamic_cast<LADataStrings &>(dh->get()).get();

			dh = &(erisk.getData("riskDetail1", ISNOTNULL));
			LAStringVector tmpriskDetail1 = dynamic_cast<LADataStrings &>(dh->get()).get();

			dh = &(erisk.getData("riskDetail2", ISNOTNULL));
			LAStringVector tmpriskDetail2 = dynamic_cast<LADataStrings &>(dh->get()).get();

			dh = &(erisk.getData("paramEntityName", ISNOTNULL));
			LAStringVector tmpparamEntityName = dynamic_cast<LADataStrings &>(dh->get()).get();

			if (tmppvriskScenario.size() != tmporgEntityName.size() ||
				tmppvriskScenario.size() != tmpriskDetail1.size() || 
				tmppvriskScenario.size() != tmpriskDetail2.size() ||
				tmppvriskScenario.size() != tmpparamEntityName.size() ||
				tmppvriskScenario.size() < 1)
				throw LACoreInvalidData("pv and risk set size error",__FILE__,__LINE__);

			//tmporgEntityName is curveID
			LAStringVector tmpdfcurves(tmporgEntityName.size());
			for (unsigned int j = 0; j < tmporgEntityName.size(); j++)
			{
				dh =&(eline.getData("CURVE_DISCOUNT", NOCHECK));
				LAString tmp_dfcurve;
				if (!dh->isDefined() || dh->isNull())
				{
					tmp_dfcurve = getDiscountCurveName(dataInstance, tmporgEntityName[j]);
				}
				else
				{
					tmp_dfcurve = dynamic_cast<LADataString &>(eline.getData("CURVE_DISCOUNT", NOCHECK).get());;
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
		ccyInput= dynamic_cast<LADataString &>(eline.getData("CALC_CURRENCY", ISNOTNULL).get());
	}

	//get set offer bid adjust pricing set
	LAStringVector offerbidcurves;
	LAStringVector orgcurves;
	LAStringVector tradenames;
	LAStringVector paranames;
	dh = &(eline.getData("offerbidentity", NOCHECK));
	bool isofferbidadj = false;
	if (dh->isDefined() && !dh->isNull())
	{
		//get offer bid info
		LADataReference& ref = dynamic_cast<LADataReference &>(dh->get());
		orgcurves = dynamic_cast<LADataStrings &>(ref.get().getData("orgEntityName",ISDEFINED).get()).get();
		offerbidcurves = dynamic_cast<LADataStrings &>(ref.get().getData("adjsutEntityName",ISDEFINED).get()).get();
		tradenames = dynamic_cast<LADataStrings &>(ref.get().getData("tradeEntityName",ISDEFINED).get()).get();
		paranames = dynamic_cast<LADataStrings &>(ref.get().getData("paramEntityName",ISDEFINED).get()).get();

		if (orgcurves.size() != offerbidcurves.size() || orgcurves.size() != tradenames.size() || orgcurves.size() != paranames.size())
			throw LACoreInvalidData("offer bid curve error",__FILE__,__LINE__);

		ccyInput= dynamic_cast<LADataString &>(eline.getData("CALC_CURRENCY", ISNOTNULL).get());

		if (orgcurves.size() >= 1)
			isofferbidadj = true;

	}


	/////////plain vanilla object///////////////////

	//get PV
	//////////////////////////////////////////////////
	LAString asofdateInput = dynamic_cast<LADataString &>(eline.getData("CALC_ASOFDATE", ISNOTNULL).get());
	LADate basedate(asofdateInput.getCString(),"YYYYMMDD");
	//dataInstance->getReferencePool().completeDependency();

	for (unsigned int i = 0; i < tradeVec.size(); i++)
	{
		//offer bid adjust case
		unsigned int pos = 0;
		LAMathObjectValue* vtrade = dynamic_cast<LAMathObjectValue* >(&objPool.getObject(tradeVec[i], ENCHKTYPE_ISDEFINED).get());
		if (isofferbidadj)
		{
			LAStringVector::iterator itpos = std::find(tradenames.begin(),tradenames.end(),tradeVec[i]);
			if (itpos != tradenames.end())
			{
				pos = static_cast<unsigned int>(itpos - tradenames.begin());
				LADataReference& refvnl = dynamic_cast<LADataReference &>(vtrade->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
				LAMathPlainVanillaEntity& pvanilla = dynamic_cast<LAMathPlainVanillaEntity &>(refvnl.get().get());
				if (pvanilla.getName().get() != paranames[pos])
					continue;

				pvanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).convertFromString(offerbidcurves[pos]);
				
			}

		}
		
		dh = &(vtrade->getData(CALIBRATION_DATA_VALUE, ISDEFINED));
		LADataValuation& val = dynamic_cast<LADataValuation &>(dh->get());
		double ret = val.value(basedate);
		//store pv and return;
		pvResultVec.push_back(ret);

		//offer bid adjust case
		if (isofferbidadj)
		{
		
			LADataReference& refvnl = dynamic_cast<LADataReference &>(vtrade->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
			LAMathPlainVanillaEntity& pvanilla = dynamic_cast<LAMathPlainVanillaEntity &>(refvnl.get().get());
			if (pvanilla.getName().get() != paranames[pos])
				continue;

			pvanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).convertFromString(orgcurves[pos]);
		}
	}
		
	LAString chgentityname;
	LAString orgentityname;
	LAString paraentityname;
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
		LAMathPlainVanillaEntity& pvanilla = dynamic_cast<LAMathPlainVanillaEntity &>(objPool.getObject(paraentityname,ENCHKTYPE_ISDEFINED).get());
		pvanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).convertFromString(chgentityname);
	
		for (unsigned int i = 0; i < tradeVec.size(); i++)
		{
			LAString tradeid = tradeVec[i];

			//in case path object is different such as libor df pricing or ois df pricing per each trade
			LAString traderefparam = getMarketParamfromTradeReference(dataInstance,tradeid);
			if (traderefparam != paraentityname)
				continue;
	
			//in case of zero rate risk, we must use zero risk of discount curve and index forecast curve
			if (pvriskScenario[h] == "ZERODELTA")
			{
				LAStringVector riskTargetVec;
				//discount
				riskTargetVec.push_back(discountcurves[h]);
				//find forecast
				for (unsigned int j = 0; j < 2; j++)
				{
					LAString indexInput;
					if (0 == j)
						indexInput = tradeid + "_leg1_coupon1_index1";
					else 
						indexInput = tradeid + "_leg2_coupon2_index2";

					objHolder = objPool.getObject(indexInput, ENCHKTYPE_ISDEFINED);
					dh = &(objHolder.getData(PRICING_DATA_BASISCURVE, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
						riskTargetVec.push_back(dynamic_cast<LADataString &>(dh->get()));

				}
			
				////if no target, no need for calc
				if (std::find(riskTargetVec.begin(),riskTargetVec.end(), riskDetail1[h]) == riskTargetVec.end())
					continue;
			}



			LAMathObjectValue* vtrade = dynamic_cast<LAMathObjectValue* >(&objPool.getObject(tradeid, ENCHKTYPE_ISDEFINED).get());
			//common set up for each deal				
			vtrade->remove(PRICING_DATA_ISCALCRISK);
			vtrade->LAObject::add(PRICING_DATA_ISCALCRISK, new LADataBool(true));
			//no need for data out info.
			vtrade->remove(PRICING_DATA_ISRESULTOUTPUT);
			vtrade->LAObject::add(PRICING_DATA_ISRESULTOUTPUT, new LADataBool(false));

			dh = &(vtrade->getData(CALIBRATION_DATA_VALUE, ISDEFINED));
			LADataValuation& val = dynamic_cast<LADataValuation &>(dh->get());
			
			double ret = val.value(basedate);
			//add the risk information on TradeData
			LAString key = "PVValueBy" + chgentityname;
			vtrade->remove(key);
			vtrade->LAObject::add(key, new LADataDouble(ret));

			//if we do not remove, the unexpected results might cause.				
			vtrade->remove(PRICING_DATA_ISCALCRISK);
			vtrade->remove(PRICING_DATA_ISRESULTOUTPUT);
			vtrade->LAObject::add(PRICING_DATA_ISRESULTOUTPUT, new LADataBool(true));
			
		}

		//set back to original scenario and 
		pvanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).convertFromString(orgentityname);
	}

	//re edit for trades grid sum and so on
	//zero case, oiszero + 3mzero.
	for (unsigned int i = 0; i < tradeVec.size(); i++)
	{
		LAString tradeid = tradeVec[i];
		LAMathObjectValue* vtrade = dynamic_cast<LAMathObjectValue *>(&objPool.getObject(tradeVec[i], ENCHKTYPE_ISDEFINED).get());
		double orgpv = dynamic_cast<LADataDouble &>(vtrade->LAObject::getData("PV",ISNOTNULL).get());
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
			LAString key = "PVValueBy" + chgentityname;
			dh = &	(vtrade->LAObject::getData(key,NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				chgpv = dynamic_cast<LADataDouble &>(dh->get());
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
				LAString shiftstr = riskDetail2[j].toToken('_')[0];
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
				LAStringVector tmpvec = riskDetail2[j].toToken(';');
				if (tmpvec.size() != 3)
					throw LACoreInvalidData("bump risk set error",__FILE__,__LINE__);
				LAString key = "SOURCEBUMPRISK_" + riskDetail1[j] + "_" + tmpvec[0] + "_" + tmpvec[1];
				vtrade->remove(key);
				vtrade->LAObject::add(key, new LADataDouble(chgpv- orgpv));
			}

			//zero bump risk
			if (pvriskScenario[j] == "ZERODELTA" && riskDetail2[j].findString(";") != -1)
			{
				//change general attrname;

				//from PVValueBy"curveID" + "_SOURCEDELTA_STD_swapRates;1Y;0
				//into ZEROBUMPRISK_STD_1Y
				LAStringVector tmpvec = riskDetail2[j].toToken(';');
				if (tmpvec.size() != 3)
					throw LACoreInvalidData("bump zero risk set error",__FILE__,__LINE__);
				LAString key = "ZEROBUMPRISK_" + riskDetail1[j] + "_" + tmpvec[0] + "_" + tmpvec[1];
				vtrade->remove(key);
				vtrade->LAObject::add(key, new LADataDouble(chgpv- orgpv));
			}

		}

		if (iszeropara)
		{
			vtrade->remove("Zero delta");
			vtrade->LAObject::add("Zero delta", new LADataDouble(zerodelta));
		}

		if (issourcepara)
		{
			vtrade->remove("Source delta");
			vtrade->LAObject::add("Source delta", new LADataDouble(sourcedeltaup));
		}

		if (isgammapara)
		{
			vtrade->remove("gamma");
			vtrade->LAObject::add("gamma", new LADataDouble(sourcedeltaup- sourcedeltadown));
		}
	}

	return;
}


void
LAMultiSwapPricer::setUpPreparetionForcalcTrades(LADataInstance* dataInstance, std::map<LAString, LAString> mapcalcinfo)
{
	
	LAObjectPool &objPool = dataInstance->getObjectPool();
	LAObjectHolder objHolder;
	LADataHolder* dh;

	/////////////////////////////
	//set up lineparam1
	setUpLineParams(dataInstance, mapcalcinfo);

	///////////plain vanilla object///////////////////
	LAString lineparam("lineparam1");
	LAObject* eline = &(objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get());


	LAStringVector curveIDs = mapcalcinfo[LAString("CURVE/") + CALIBRATION_DATA_CURVEID].toToken(':');
	LAStringVector params(curveIDs.size());
	//in case of global shift curveIDs change
	for (unsigned int i = 0; i < curveIDs.size(); i++)
		params[i] = setUpMarketParamsAndGlobalShift(dataInstance, mapcalcinfo, curveIDs[i]);

	eline->remove("CALC_MARKETPARAMS");
	eline->add("CALC_MARKETPARAMS", new LADataStrings(params));
	///////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////
	//set up risk object for zerodelta source delta and gamma
	LAStringVector orgEntityName, pvriskScenario, riskDetail1, riskDetail2;
	//zero delta;

	//set up zero delta it can be extended of bumprisk!!
	LAStringVector riskDetail1zerodelta(4);
	riskDetail1zerodelta[0] = dynamic_cast<LADataString &>(eline->getData("CURVE_3MFORECAST",ISNOTNULL).get());
	riskDetail1zerodelta[1] = dynamic_cast<LADataString &>(eline->getData("CURVE_6MFORECAST",ISNOTNULL).get());
	riskDetail1zerodelta[2] = dynamic_cast<LADataString &>(eline->getData("CURVE_1MFORECAST",ISNOTNULL).get());
	riskDetail1zerodelta[3] = dynamic_cast<LADataString &>(eline->getData("CURVE_OISFORECAST",ISNOTNULL).get());


	//set whether we prepare many curves for delta or gamma pricing
	LAString sourcestr = mapcalcinfo[LAString("CALC/") + "IsCalcSourceDelta"];
	LAString zerostr = mapcalcinfo[LAString("CALC/") + "IsCalcZeroDelta"];
	LAString gammastr = mapcalcinfo[LAString("CALC/") + "IsCalcGamma"];

	if (sourcestr.toUpper() == "FALSE" && gammastr.toUpper() == "TRUE")
		throw LACoreInvalidData("inconsitency of calc common info", __FILE__,__LINE__);

	LAString keyshiftval = mapcalcinfo[LAString("CALC/") + "ParaShiftScenario"];
	LAString upshiftstr, downshiftstr;
	if (keyshiftval != "")
	{
		upshiftstr = LAString(keyshiftval.getDoubleValue(),3);
		downshiftstr = LAString(keyshiftval.getDoubleValue() * -1.0 ,3);
	}
	else
	{
		upshiftstr = LAString(1.0, 3);
		downshiftstr = LAString(-1.0, 3);

	}

	////to calc zero para delta 1.000 bp
	if (zerostr == "TRUE")
	{
		LAStringVector riskDetail2zerodelta(1);
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
	LAString fcurvestrois = dynamic_cast<LADataString &>(dh->get()).get();

	dh = &(eline->getData("SWAP_FREQUENCY", ISNOTNULL));
	LAString swapfreq = dynamic_cast<LADataString &>(dh->get()).get();

//	dh = &(eline->getData("CURVE_3MFORECAST", ISNOTNULL));
	dh = &(eline->getData(LAString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL));
//	LAString fcurvestr3m = dynamic_cast<LADataString &>(dh->get()).get();
	LAString fcurvestrstd = dynamic_cast<LADataString &>(dh->get()).get();
	//source delta;
	if (sourcestr != "FALSE")
	{
		//set up source delta it can be extended of bumprisk!!

		LAStringVector riskDetail1sordelta(1);
//		riskDetail1sordelta[0] = fcurvestrois + "+" + fcurvestr3m;
		riskDetail1sordelta[0] = fcurvestrois + "+" + fcurvestrstd;

		//
		LAStringVector riskDetail2sordelta;
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
LAMultiSwapPricer::createSourceDeltaRiskEntity(LADataInstance* dataInstance, LAString scenarioname, LAStringVector riskDetail1sordelta, LAStringVector riskDetail2sordelta, bool isbumpgridauto, LAString paraName)
{

	LAObjectPool& objPool = dataInstance->getObjectPool();
	LADataHolder* dh;
	LAObjectHolder objHolder;

	LAString lineparam("lineparam1");
	LAObject* eline = &(objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get());

	//if paraName is directly selected, then only one curve is created, otherwise both curves (libor base curve and ois base curve)
	LAStringVector paramVec;
	if (paraName != "")
		paramVec.push_back(paraName);
	else
	{
		dh = &(eline->getData("CALC_MARKETPARAMS", ISNOTNULL));
		paramVec = dynamic_cast<LADataStrings &>(dh->get()).get();
	}
	LAStringVector::iterator itpara = paramVec.begin();
	
	dh = &(eline->getData("CALC_CURRENCY", ISNOTNULL));
	LAString ccyInput = dynamic_cast<LADataString &>(dh->get());

	//this curve id is imporatant for using zero delta para
	LAString oisforecurve = dynamic_cast<LADataString &>(objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).getData("CURVE_OISFORECAST", ISNOTNULL).get());
	LAString libor3mforecurve = dynamic_cast<LADataString &>(objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).getData("CURVE_3MFORECAST", ISNOTNULL).get());
	LAString libor6mforecurve = dynamic_cast<LADataString &>(objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).getData("CURVE_6MFORECAST", ISNOTNULL).get());
	LAString libor1mforecurve = dynamic_cast<LADataString &>(objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).getData("CURVE_1MFORECAST", ISNOTNULL).get());


	//set up risk object for source delta and gamma
	LAStringVector orgEntityNameSource, pvriskScenarioSource, riskDetail1Source, riskDetail2Source, paramEntityNameSource;
	while (itpara != paramVec.end())
	{
		//set up market bump rate
		LAMathPlainVanillaEntity& pvanilla = dynamic_cast<LAMathPlainVanillaEntity &>(objPool.getObject(*itpara,ENCHKTYPE_ISDEFINED).get());
		LAString curveID = pvanilla.getIRCurve(ccyInput).getYieldData().get().getName();

		LAMathYieldCurvePro& ypro = dynamic_cast<LAMathYieldCurvePro&>(objPool.getObject(LAString("PRO_") + PREFIX_YIELD + curveID, ENCHKTYPE_ISDEFINED).get());

		LAStringVector changed2Vec;
		if (isbumpgridauto)
		{
			//in case of riskDetail1sordelta[0] = "OISCurve"
			//in case of riskDetail2sordelt[1] = "oisRates"		
			//then changed2Vec is changed as oisRates;FFV1;0,oisRates;FFX1;1,.....
			
			if (riskDetail2sordelta.size() != 1)
				throw LACoreInvalidData("bump source delta set error",__FILE__,__LINE__);

			LAStringVector ret2Vec;
			LAStringVector tmpdetail2vec;
			LAString rateboxname = riskDetail2sordelta[0];
			if (rateboxname != "liborRates" && rateboxname != "swapRates" &&
				rateboxname != "fra3mRates" && rateboxname != "fra6mRates" &&
				rateboxname != "futureRates" && rateboxname != "basisMkt" && rateboxname != "oisRates")
				throw LACoreInvalidData("bump source initial error",__FILE__,__LINE__);

			LAString mkt_name = ypro.getMarketForCurve(riskDetail1sordelta[0]);
			//LAString curveorginfo = "lineparam1_" + curveID + riskDetail1sordelta[0];
			LAString curveorginfo = "lineparam1_" + curveID + mkt_name;
			LAObject* ecurveorg = &(objPool.getObject(curveorginfo,ENCHKTYPE_ISDEFINED).get());
			dh = &(ecurveorg->getData(rateboxname, ISNOTNULL));
			{
				const LAStringMatrix& ratemat = dynamic_cast<LADataStringMatrix& >(dh->get()).get();
				for (unsigned int j = 0; j < ratemat.size(); j++)
					tmpdetail2vec.push_back(rateboxname + ";" +ratemat[j][0] + ";" + LADataInt(j).convertToString());
			}

			changed2Vec = tmpdetail2vec;
		}
		else
			changed2Vec = riskDetail2sordelta;

		////set up order 
		LAStringVector setupOrder = getCurveNamesFromSetUpOrder(dataInstance, curveID);
		vector<LAString> mkt_name(setupOrder.size());
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
				LAString strSourceShiftCurve = "Temporary_SOURCEDELTA";
				for (unsigned int k = 0; k < setupOrder.size(); k++)
				{
					double shiftval = 0.0;
					LAString market ;
					bool isshiftarget = true;

					//imporatant judge
					//if (riskDetail1sordelta[i] ==  setupOrder[k] && changed2Vec[j] == "PARALLEL1BP")
					if (riskDetail1sordelta[i].findString(setupOrder[k]) != -1 && changed2Vec[j].findString("_BPPARALLEL") != -1)
					{
						//always +(-)x.xxx_BPPARALLEL
						LAString shiftstr = changed2Vec[j].toToken('_')[0];
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
						LAStringVector marketandgrid = changed2Vec[j].toToken(';');
						if (marketandgrid.size() != 3)
							throw LACoreInvalidData("market and grid error",__FILE__,__LINE__);
						
						shiftpos = static_cast<unsigned int>(marketandgrid[2].getIntValue());

						isshiftarget = true;

						market = marketandgrid[0];
					
					}
					else
					{
						isshiftarget = false;
					}


					//org object from setUpStoreFunc
					//LAString curveorginfo = "lineparam1_" + curveID + setupOrder[k];
					LAString curveorginfo = "lineparam1_" + curveID + mkt_name[k];
					LAObject* ecurveorg = &(objPool.getObject(curveorginfo,ENCHKTYPE_ISDEFINED).get());

					//create shift curves
					createShiftCurves(dataInstance, *ecurveorg, setupOrder[k], strSourceShiftCurve,isshiftarget,ispara, 
											shiftval,shiftpos,market,false);



				}
				//curve generate
				//name is very imporatant!!
				LAObject* eclone = objPool.getObject(strSourceShiftCurve, ENCHKTYPE_ISDEFINED).clone();
				//objPool.remove(strSourceShiftCurve);

				LAString keyname= curveID + "_SOURCEDELTA_";
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
	
	LAObject* eSource = NULL;
	objHolder = objPool.getObject(scenarioname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		eSource = new LAObject();
		objPool.set(scenarioname,eSource);
	
	}
	else
	{
		eSource = &(objHolder.get());
		eSource->clear();
	}
	eSource->add("orgEntityName", new LADataStrings(orgEntityNameSource));
	eSource->add("pvriskScenario", new LADataStrings(pvriskScenarioSource));
	eSource->add("riskDetail1", new LADataStrings(riskDetail1Source));
	eSource->add("riskDetail2", new LADataStrings(riskDetail2Source));
	eSource->add("paramEntityName", new LADataStrings(paramEntityNameSource));

	return;

}

void 
LAMultiSwapPricer::createZeroDeltaRiskEntity(LADataInstance* dataInstance, LAString scenarioname, LAStringVector riskDetail1zerodelta, LAStringVector riskDetail2zerodelta, bool isbumpgridauto, LAString paraName)
{

	LAObjectPool& objPool = dataInstance->getObjectPool();
	LADataHolder* dh;
	LAObjectHolder objHolder;

	LAString lineparam("lineparam1");
	LAObject* eline = &(objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get());

	//if paraName is directly selected, then only one curve is created, otherwise both curves (libor base curve and ois base curve)
	LAStringVector paramVec;
	if (paraName != "")
		paramVec.push_back(paraName);
	else
	{
		dh = &(eline->getData("CALC_MARKETPARAMS", ISNOTNULL));
		paramVec = dynamic_cast<LADataStrings &>(dh->get()).get();
	}
	LAStringVector::iterator itpara = paramVec.begin();

	dh = &(eline->getData("CALC_CURRENCY", ISNOTNULL));
	LAString ccyInput = dynamic_cast<LADataString &>(dh->get());
	dh = &(eline->getData("SWAP_FREQUENCY", ISNOTNULL));
	LAString swapfreq = dynamic_cast<LADataString &>(dh->get());


	//set up risk object for source delta and gamma
	LAStringVector orgEntityNameZero, pvriskScenarioZero, riskDetail1Zero, riskDetail2Zero, paramEntityNameZero;
	while (itpara != paramVec.end())
	{
		//set up market bump rate
		LAMathPlainVanillaEntity& pvanilla = dynamic_cast<LAMathPlainVanillaEntity &>(objPool.getObject(*itpara,ENCHKTYPE_ISDEFINED).get());
		LAString curveID = pvanilla.getIRCurve(ccyInput).getYieldData().get().getName();
		
		LAObject* porgyield = &(objPool.getObject(curveID, ENCHKTYPE_ISDEFINED).get());
		LAString yieldProName = "PRO_YIELD_" + curveID;
		LAMathYieldCurvePro* porgpro = &(dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(yieldProName, ENCHKTYPE_ISDEFINED).get()));
		LAString orgcurvetype = porgpro->getCurveType();


		//this curve id is imporatant for using zero delta para
		LAString suffix_data;
		unsigned int shiftpos = 0;
		bool ispara = true;
		for (unsigned int i = 0; i < riskDetail1zerodelta.size(); i++)
		{
			LAString curvetype = riskDetail1zerodelta[i];
			if (riskDetail1zerodelta[i] != STD)
			{
				suffix_data = "_" + curvetype;
			}

			UintArray gridPos;
			LAStringVector changed2Vec;
			if (isbumpgridauto)
			{
				const LAStringVector& zerodeltagrids = porgpro->getConversionMatrixTermTypes(curvetype).get();
				//this data should be added to avoid duplicate the same grid of dirrerent market
				//e.x. STD;swapRates1Y, STD:liborRates1Y
				const LAStringVector& markettypesvec = porgpro->getConversionMarketTypes(curvetype).get();

				if (zerodeltagrids.size() != markettypesvec.size())
					throw LACoreInvalidData("curve set error",__FILE__,__LINE__);

				LAStringVector rateboxvec(markettypesvec.size());
				for (unsigned int j = 0; j < markettypesvec.size(); j++)
				{
					LAString key = markettypesvec[j];
					if (BASIS == key)
					{
						rateboxvec[j] = "basisMkt";
					
					}
					else if (PAR == key && curvetype == dynamic_cast<LADataString &>(eline->getData("CURVE_OISFORECAST", ISNOTNULL).get()).get())
					{
						rateboxvec[j] = "oisRates";
					}
//					else if (PAR == key && curvetype == dynamic_cast<LADataString &>(eline->getData("CURVE_3MFORECAST", ISNOTNULL).get()).get())
					else if (PAR == key && curvetype == dynamic_cast<LADataString &>(eline->getData(LAString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get()).get())
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
						throw LACoreInvalidData("market type error",__FILE__,__LINE__);
				}

				LAStringVector tmpdetail2vec;
				for (unsigned int j = 0; j < zerodeltagrids.size(); j++)
					tmpdetail2vec.push_back(rateboxvec[j] + ";" + zerodeltagrids[j] + ";" + LADataInt(j).convertToString());

				changed2Vec = tmpdetail2vec;
				gridPos.resize(tmpdetail2vec.size());

				//in case of zero only supports grid zero delta
				
				dh = &(porgyield->getData(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM + suffix_data, ISNOTNULL));
				const DoubleVector& zerodeltaterms = dynamic_cast<const LADataDoubles &>(dh->get()).get();

				dh = &(porgyield->getData(CALIBRATION_DATA_TERMS + suffix_data, ISNOTNULL));
				const DoubleVector& terms = dynamic_cast<const LADataDoubles &>(dh->get()).get();

				for (unsigned int j = 0; j < zerodeltagrids.size(); j++)
					LACalibrationUtilities<double>::searchNearestPos(terms, zerodeltaterms[j], gridPos[j]);
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
					LAString shiftstr = changed2Vec[j].toToken('_')[0];
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
					LAStringVector marketandgrid = changed2Vec[j].toToken(';');
					if (marketandgrid.size() != 3)
						throw LACoreInvalidData("zero rate grid error",__FILE__,__LINE__);
					
					//shiftpos = static_cast<unsigned int>(marketandgrid[2].getIntValue());
				}

				LAString strShiftCurve = curveID + "_ZERODELTA_" + riskDetail1zerodelta[i] + "_" + changed2Vec[j] ;
				objHolder = objPool.getObject(strShiftCurve);
				if (objHolder.isDefined())
					objPool.remove(strShiftCurve);

				LAObject* shiftCurve = porgyield->clone();
				objPool.set(strShiftCurve, shiftCurve);
				shiftCurve->getData(CALIBRATION_DATA_NAME, ISDEFINED).convertFromString(strShiftCurve);

				if (ispara)
				{

					dh = &(shiftCurve->getData(CALIBRATION_DATA_TERMS + suffix_data, ISNOTNULL));
					const DoubleVector& terms = dynamic_cast<const LADataDoubles &>(dh->get()).get();
					
					dh = &(shiftCurve->getData(IR_CALIBRATION_DATA_DFS + suffix_data, ISNOTNULL));
					DoubleVector dfs = dynamic_cast<LADataDoubles &>(dh->get()).get();
					
					if (terms.size() != dfs.size())
						throw LACoreInvalidData("Discount Size Error",__FILE__,__LINE__);
					for (unsigned int k = 0; k < terms.size(); k++)
						dfs[k] *= LAMath::exp(-shiftval * terms[k]);

					dynamic_cast<LADataDoubles &>(dh->get()).set(dfs);
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

	LAObject* eSource = NULL;
	objHolder = objPool.getObject(scenarioname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		eSource = new LAObject();
		objPool.set(scenarioname,eSource);
	
	}
	else
	{
		eSource = &(objHolder.get());
		eSource->clear();
	}
	eSource->add("orgEntityName", new LADataStrings(orgEntityNameZero));
	eSource->add("pvriskScenario", new LADataStrings(pvriskScenarioZero));
	eSource->add("riskDetail1", new LADataStrings(riskDetail1Zero));
	eSource->add("riskDetail2", new LADataStrings(riskDetail2Zero));
	eSource->add("paramEntityName", new LADataStrings(paramEntityNameZero));

	return;

}

void
LAMultiSwapPricer::setUpStoreBasisCurve(LADataInstance* dataInstance,
                                const LAString& curveID, 
                                const LAString& basisCurveName, 
                                const LAStringMatrix& basisMkt, 
                                const LAStringMatrix& basisConv,
								const LAStringMatrix& generateProp,
								const LAStringMatrix& moneyConv)
{

	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAObject* curvestore = NULL;
	// Special Treatment for picking up market name through YieldCurvePro class
	LAString basisCurveNameTmp = basisCurveName;
	basisCurveNameTmp.toUpper();
	LAString keyname = "lineparam1_" + curveID + basisCurveNameTmp;
	LAObjectHolder objHolder = objPool.getObject(keyname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		curvestore = new LAObject();
		objPool.set(keyname, curvestore);
	}
	else
	{
		curvestore = &objHolder.get();
		curvestore->clear();
	}
	
	curvestore->add(CALIBRATION_DATA_NAME, new LADataString(keyname));
	curvestore->add("basisCurveName", new LADataString(basisCurveName));
	curvestore->add("basisMkt", new LADataStringMatrix(basisMkt));
	curvestore->add("basisConv", new LADataStringMatrix(basisConv));
	curvestore->add("generateProp", new LADataStringMatrix(generateProp));
	curvestore->add("moneyConv", new LADataStringMatrix(moneyConv));


	//LAString key = "SOURCEBUMPRISK_" + riskDetail1[j] + "_" + tmpvec[0] + "_" + tmpvec[1];
	//register attrmaster to output to be able to display excel
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	for (unsigned int i = 0; i < basisMkt.size(); i++)
	{
		LAString key = "SOURCEBUMPRISK_" + basisCurveName + "_" + "basisMkt" + "_" + basisMkt[i][0];
		dm.setData(key, DATA_DOUBLE);

		LAString zerokey = "ZEROBUMPRISK_" + basisCurveName + "_" + "basisMkt" + "_" + basisMkt[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}
}

void
LAMultiSwapPricer::setUpStoreSwapCurve(LADataInstance* dataInstance,
								 const LAString& curveID,
								 const LAString& genCurveName,
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
								 const LAStringMatrix& adjustSwapRates)
{

	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAObject* curvestore = NULL;
	// Special Treatment for picking up market name through YieldCurvePro class
	LAString genCurveNameTmp = "SWAP";
	LAString keyname = "lineparam1_" + curveID + genCurveNameTmp;
	LAObjectHolder objHolder = objPool.getObject(keyname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		curvestore = new LAObject();
		objPool.set(keyname, curvestore);
	}
	else
	{
		curvestore = &objHolder.get();
		curvestore->clear();
	}
	
	curvestore->add(CALIBRATION_DATA_NAME, new LADataString(keyname));
	curvestore->add("genCurveName", new LADataString(genCurveName));
	curvestore->add("generateProp", new LADataStringMatrix(generateProp));
	curvestore->add("moneyConv", new LADataStringMatrix(moneyConv));
	curvestore->add("liborRates", new LADataStringMatrix(liborRates));
	curvestore->add("liborConv", new LADataStringMatrix(liborConv));
	curvestore->add("swapRates", new LADataStringMatrix(swapRates));
	curvestore->add("swapConv", new LADataStringMatrix(swapConv));
	curvestore->add("fra3mRates", new LADataStringMatrix(fra3mRates));
	curvestore->add("fra6mRates", new LADataStringMatrix(fra6mRates));
	curvestore->add("fraConv", new LADataStringMatrix(fraConv));
	curvestore->add("futureRates", new LADataStringMatrix(futureRates));
	curvestore->add("futureConv", new LADataStringMatrix(futureConv));
	curvestore->add("adjustSwapConv", new LADataStringMatrix(adjustSwapConv));
	curvestore->add("adjustSwapRates", new LADataStringMatrix(adjustSwapRates));

	//register attrmaster to output to be able to display excel
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	for (unsigned int i = 0; i < liborRates.size(); i++)
	{
		LAString key = "SOURCEBUMPRISK_" + genCurveName + "_" + "liborRates" + "_" + liborRates[i][0];
		dm.setData(key, DATA_DOUBLE);

		LAString zerokey = "ZEROBUMPRISK_" + genCurveName + "_" + "liborRates" + "_" + liborRates[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}

	for (unsigned int i = 0; i < swapRates.size(); i++)
	{
		LAString key = "SOURCEBUMPRISK_" + genCurveName + "_" + "swapRates" + "_" + swapRates[i][0];
		dm.setData(key, DATA_DOUBLE);

		LAString zerokey = "ZEROBUMPRISK_" + genCurveName + "_" + "swapRates" + "_" + swapRates[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}

	for (unsigned int i = 0; i < futureRates.size(); i++)
	{
		LAString key = "SOURCEBUMPRISK_" + genCurveName + "_" + "futureRates" + "_" + futureRates[i][0];
		dm.setData(key, DATA_DOUBLE);

		LAString zerokey = "ZEROBUMPRISK_" + genCurveName + "_" + "futureRates" + "_" + futureRates[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}


}


void
LAMultiSwapPricer::setUpStoreOISCurve(LADataInstance* dataInstance,
								const LAString& curveID,
								const LAString& genCurveName,
								const LAStringMatrix& generateProp, 
								const LAStringMatrix& oisRates, 
								const LAStringMatrix& oisConv,
								const LAStringMatrix& histRates,
								const LAStringMatrix& lobasisRates, 
								const LAStringMatrix& lobasisConv, 
								const LAStringMatrix& swapRates, 
								const LAStringMatrix& swapConv)
{

	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAObject* curvestore = NULL;
	// Special Treatment for picking up market name through YieldCurvePro class
	LAString genCurveNameTmp = genCurveName;
	genCurveNameTmp.toUpper();
	LAString keyname = "lineparam1_" + curveID + genCurveNameTmp;
	LAObjectHolder objHolder = objPool.getObject(keyname, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		curvestore = new LAObject();
		objPool.set(keyname, curvestore);
	}
	else
	{
		curvestore = &objHolder.get();
		curvestore->clear();
	}
	
	curvestore->add(CALIBRATION_DATA_NAME, new LADataString(keyname));
	curvestore->add("genCurveName", new LADataString(genCurveName));
	curvestore->add("generateProp", new LADataStringMatrix(generateProp));
	curvestore->add("oisRates", new LADataStringMatrix(oisRates));
	curvestore->add("oisConv", new LADataStringMatrix(oisConv));
	curvestore->add("histRates", new LADataStringMatrix(histRates));
	curvestore->add("lobasisRates", new LADataStringMatrix(lobasisRates));
	curvestore->add("lobasisConv", new LADataStringMatrix(lobasisConv));
	curvestore->add("swapRates", new LADataStringMatrix(swapRates));
	curvestore->add("swapConv", new LADataStringMatrix(swapConv));

	LAPriceDataManager& dm = dataInstance->getDataMaster();
	for (unsigned int i = 0; i < oisRates.size(); i++)
	{
		LAString key = "SOURCEBUMPRISK_" + genCurveName + "_" + "oisRates" + "_" + oisRates[i][0];
		dm.setData(key, DATA_DOUBLE);

		LAString zerokey = "ZEROBUMPRISK_" + genCurveName + "_" + "oisRates" + "_" + oisRates[i][0];
		dm.setData(zerokey, DATA_DOUBLE);
	}
}


void
LAMultiSwapPricer::shiftMarketRate(LAStringMatrix& rateMat, double shiftval, bool ispara, unsigned int shiftpos)
{
	//check input mat
	if (rateMat.empty() || rateMat[0].size() < 2)
		throw LACoreInvalidData("Rate matrix error",__FILE__,__LINE__);

	for (unsigned int i = 0; i < rateMat.size(); i++)
	{
		if (!ispara)
		{
			//check shiftpos
			if (shiftpos >= rateMat.size())
				throw LACoreInvalidData("shift pos error",__FILE__,__LINE__);

			i = shiftpos;
		}

		double orgval = rateMat[i][1].getDoubleValue();
		//special case price flag is only "FF" or "ED", this is not temporary
		if (rateMat[i][0].findString("FF") == -1 && rateMat[i][0].findString("ED") == -1)
		{
			rateMat[i][1] = LAString(orgval+shiftval);

		}
		else
		{
			//future case
			rateMat[i][1] = LAString(orgval - shiftval * 100.0);
		}

		if (!ispara)
			return;
	}
}


LAMathObjectValue* 
LAMultiSwapPricer::createSingleTrade(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade, LAString orgtradeid, bool isduplicatemode, LAString key_suffix, bool istypicalpardeal)
{
	LAMathObjectValue* ret = NULL;
	LADataHolder* dh;
	LAObjectPool& objPool = dataInstance->getObjectPool();
	

	LAString copytradeid = orgtradeid;
	if (isduplicatemode)
		copytradeid += key_suffix;

	//important clasee "lineparam1"
	LAObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	map<LAString, LAString>::iterator itmap = maptrade.begin();


	LAString leg1Input = copytradeid + "_leg1";
	LAString leg2Input = copytradeid + "_leg2";
	LAString coupon1Input = leg1Input + "_coupon1";
	LAString coupon2Input = leg2Input + "_coupon2";
	LAString index1Input =  coupon1Input + "_index1";
	LAString index2Input =  coupon2Input + "_index2";

	//common info
	//calc info
	LAString ccyInput = dynamic_cast<LADataString &>(eline.getData("CALC_CURRENCY", ISNOTNULL).get());
	LAString asofdateInput = dynamic_cast<LADataString &>(eline.getData("CALC_ASOFDATE", ISNOTNULL).get());
	LAString valuedateInput = dynamic_cast<LADataString &>(eline.getData("CALC_VALUEDATE", ISNOTNULL).get());
	LADate basedate(asofdateInput.getCString(),"YYYYMMDD");

	// necessary in case of index set
	LAString fcurvestr1m = dynamic_cast<LADataString &>(eline.getData("CURVE_1MFORECAST", ISNOTNULL).get());
	LAString fcurvestr3m = dynamic_cast<LADataString &>(eline.getData("CURVE_3MFORECAST", ISNOTNULL).get());
	LAString fcurvestr6m = dynamic_cast<LADataString &>(eline.getData("CURVE_6MFORECAST", ISNOTNULL).get());
	LAString fcurvestrois = dynamic_cast<LADataString &>(eline.getData("CURVE_OISFORECAST", ISNOTNULL).get());
	LAString curveID = maptrade[orgtradeid + "/" + "CURVEID"];

	dh =&(eline.getData("CURVE_DISCOUNT", NOCHECK));
	LAString discountcurve;
	if (!dh->isDefined() || dh->isNull())
	{
		discountcurve = getDiscountCurveName(dataInstance, curveID);
	}
	else
	{
		discountcurve = dynamic_cast<LADataString &>(eline.getData("CURVE_DISCOUNT", NOCHECK).get());
	}

	//line product
	LAString lineproduc = maptrade[orgtradeid + "/" + "LineProductType"];
	LAString ispnl = maptrade[orgtradeid + "/" + "IsPnL"];
	LAObjectHolder objHolder = objPool.getObject(copytradeid);
	if (objHolder.isDefined())
	{
		if (!objHolder.isTypeOf(ENTITY_VENTITY))
			throw LACoreInvalidData("Trans ID Error",__FILE__,__LINE__);
		ret = dynamic_cast<LAMathObjectValue *>(&objHolder.get());
		ret->reset();
	}
	else
	{
		ret = new LAMathObjectValue(dataInstance);
		objPool.set(copytradeid, ret);
	}
	//set mintradefile
	//name
	ret->getData(CALIBRATION_DATA_NAME, ISDEFINED).convertFromString(copytradeid);
	//value
	ret->getData(CALIBRATION_DATA_VALUE, ISDEFINED).convertFromString(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
	//isdetailoutput
	ret->LAObject::add(PRICING_DATA_ISDETAILOUTPUT, new LADataBool(true));
	//cfgenerator
	ret->LAObject::add(PRICING_DATA_CFGENERATOR, new LADataProcedure()).convertFromString(FN_IR_CASHFLOWGENERATOR_STR);
	//currency
	ret->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccyInput);
	//settle
	ret->LAObject::add(PRICING_DATA_SETTLEDATE, new LADataDate()).convertFromString(valuedateInput);
	//valuedate
	ret->LAObject::add(PRICING_DATA_VALUEDATE, new LADataDate()).convertFromString(valuedateInput);
	//today
	ret->LAObject::add(PRICING_DATA_TODAY, new LADataDate()).convertFromString(asofdateInput);
	//underlyings
	ret->LAObject::add(CALIBRATION_DATA_UNDERLYINGS, new LADataMultiReference()).convertFromString(leg1Input + LAString(":") + leg2Input);
	//is result out ---> the same output as webtool
	ret->LAObject::add(PRICING_DATA_ISRESULTOUTPUT, new LADataBool(true));
	//temporary to avoid erro in past trade
	ret->LAObject::add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(true));
	//path object
	ret->LAObject::add(PRICING_DATA_PATHENTITY, new LADataReference()).convertFromString("marketparam1_" + curveID);
	//Line product type
	ret->LAObject::add("LineProductType", new LADataString(lineproduc));
	//IsPnL
	ret->LAObject::add("IsPnL", new LADataBool()).convertFromString(ispnl);

	//each single deal
	LAString tradetypekey = LAString("TRADE_") + lineproduc + "_";
	//is forward roll
	LAString isforwardInput = dynamic_cast<LADataString &>(eline.getData(tradetypekey + PRICING_DATA_ISFORWARDROLL, ISNOTNULL).get());
	//is forward interpolation
	LAString isforwardinterInput = dynamic_cast<LADataString &>(eline.getData(tradetypekey + PRICING_DATA_ISFWDINTERPOLATION, ISNOTNULL).get());

	//is paymenttiming (priority: trade(leg) > trade > commoninfo)
	LAString paytimingInput1, paytimingInput2;
	if (maptrade.find(orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING + "/LEG1") != maptrade.end())
		paytimingInput1 = maptrade[orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING + "/LEG1"];
	else if (maptrade.find(orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING) != maptrade.end())
		paytimingInput1 = maptrade[orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING];
	else
		paytimingInput1 = dynamic_cast<LADataString &>(eline.getData(tradetypekey + PRICING_DATA_PAYMENTTIMING, ISNOTNULL).get());

	if (maptrade.find(orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING + "/LEG2") != maptrade.end())
		paytimingInput2 = maptrade[orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING + "/LEG2"];
	else if (maptrade.find(orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING) != maptrade.end())
		paytimingInput2 = maptrade[orgtradeid + "/" + PRICING_DATA_PAYMENTTIMING];
	else
		paytimingInput2 = dynamic_cast<LADataString &>(eline.getData(tradetypekey + PRICING_DATA_PAYMENTTIMING, ISNOTNULL).get());


	//payment calendar (priority: trade(leg) > trade > commoninfo)
	LAString leg1calendarInput, leg2calendarInput;
	if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_CALENDAR + "/LEG1") != maptrade.end())
		leg1calendarInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_CALENDAR + "/LEG1"];
	else if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_CALENDAR) != maptrade.end())
		leg1calendarInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_CALENDAR];
	else
		leg1calendarInput = dynamic_cast<LADataString &>(eline.getData(tradetypekey + CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());

	if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_CALENDAR + "/LEG2") != maptrade.end())
		leg2calendarInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_CALENDAR + "/LEG2"];
	else if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_CALENDAR) != maptrade.end())
		leg2calendarInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_CALENDAR];
	else
		leg2calendarInput = dynamic_cast<LADataString &>(eline.getData(tradetypekey + CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());


	//trade business day convnention (priority: trade(leg) > trade > commoninfo)
	LAString bdayconvInput1, bdayconvInput2;
	if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/LEG1") != maptrade.end())
		bdayconvInput1 = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/LEG1"];
	else if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE) != maptrade.end())
		bdayconvInput1 = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE];
	else
		bdayconvInput1 = dynamic_cast<LADataString &>(eline.getData(tradetypekey + CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());

	if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/LEG2") != maptrade.end())
		bdayconvInput2 = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/LEG2"];
	else if (maptrade.find(orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE) != maptrade.end())
		bdayconvInput2 = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE];
	else
		bdayconvInput2 = dynamic_cast<LADataString &>(eline.getData(tradetypekey + CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());

	LAString leg1indextypeInput = maptrade[orgtradeid + "/" + PRICING_DATA_INDEXTYPE + "/INDEX1"];
	LAString leg2indextypeInput = maptrade[orgtradeid + "/" + PRICING_DATA_INDEXTYPE + "/INDEX2"];
	LAString leg1compfreqInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLFREQUENCY + "/LEG1"];
	LAString leg2compfreqInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLFREQUENCY + "/LEG2"];

	bool isleg1compounding = ("OIS" == leg1indextypeInput || leg1compfreqInput.size() > 0);
	bool isleg2compounding = ("OIS" == leg2indextypeInput || leg2compfreqInput.size() > 0);

	//this is important auto map of OIS index
	if (leg1indextypeInput == "OIS")
		leg1compfreqInput = "BUSINESS_DAYS";

	if (leg2indextypeInput == "OIS")
		leg2compfreqInput = "BUSINESS_DAYS";

	//set minlegfile
	LAObject* pleg1 = NULL;
	LAObject* pleg2 = NULL;
	objHolder = objPool.getObject(leg1Input);
	if (objHolder.isDefined())
	{
		pleg1 = &objHolder.get();
		//cf generate
		dh = &(ret->getData(PRICING_DATA_CFGENERATOR,ISDEFINED));
		LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>(dh->get());
		const LAPriceCashFlowGenerator& cfgen = dynamic_cast<const LAPriceCashFlowGenerator &>(modelDataObj.getMethod());
		cfgen.clearCashlets(*pleg1);
		pleg1->clear();
	}
	else
	{
		pleg1= new LAObject();
		objPool.set(leg1Input,pleg1);
	}

	//name 
	pleg1->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(leg1Input);
	//ccyInput
	pleg1->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccyInput);
	//inputtype
	pleg1->LAObject::add(PRICING_DATA_INPUTTYPE, new LADataString()).convertFromString("Detail");
	//isamortize
	pleg1->LAObject::add(PRICING_DATA_ISAMORTIZE, new LADataBool(false));
	//isnotionalend
	pleg1->LAObject::add(PRICING_DATA_ISNOTIONALEXCHANGEATEND, new LADataBool(false));
	//isnotionalstart
	pleg1->LAObject::add(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, new LADataBool(false));
	//underlyings
	pleg1->LAObject::add(PRICING_DATA_COUPONINFOS, new LADataMultiReference()).convertFromString(coupon1Input);
	//calendar
	pleg1->LAObject::add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(leg1calendarInput);
	//slidingrule
	LAString leg1conventionInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/Adjustment/LEG1"];
	//NO_CHANGE means unadjsuted
	if (leg1conventionInput != "NO_CHANGE")
		leg1conventionInput = bdayconvInput1; 
	
	pleg1->LAObject::add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(leg1conventionInput);
	//slidingrule (strat/end date)
	leg1conventionInput = maptrade[orgtradeid + "/" + PRICING_DATA_CASHFLOWSLIDINGRULE + "/Adjustment/LEG1"];
	//NO_CHANGE means unadjsuted
	if (leg1conventionInput != "NO_CHANGE")
		leg1conventionInput = bdayconvInput1; 
	
	pleg1->LAObject::add(PRICING_DATA_CASHFLOWSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(leg1conventionInput);
	//daycount
	LAString leg1daycountInput = maptrade[orgtradeid + "/" + PRICING_DATA_DAYCOUNT + "/LEG1"];
	pleg1->LAObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(leg1daycountInput);
	//startdate
	LAString leg1startdateInput = maptrade[orgtradeid + "/" + PRICING_DATA_STARTDATE + "/LEG1"];
	//change string into date 
	if (leg1startdateInput.findString('Y') != -1 || leg1startdateInput.findString('M') != -1 || 
		leg1startdateInput.findString('W') != -1 || leg1startdateInput.findString('D') != -1)
	{
		LADate tmpstartdate = LAMathDateUtilities::getDate(basedate,leg1startdateInput,leg1conventionInput,leg1calendarInput);
		pleg1->LAObject::add(PRICING_DATA_STARTDATE, new LADataDate(tmpstartdate));
	}
	else if (leg1startdateInput.size() == 8)
	{
		pleg1->LAObject::add(PRICING_DATA_STARTDATE, new LADataDate()).convertFromString(leg1startdateInput);
	}
	else 
		throw LACoreInvalidData("Leg1 StartDate input error",__FILE__,__LINE__);

	//enddate
	//change string into date
	LAString leg1enddateInput = maptrade[orgtradeid + "/" + PRICING_DATA_ENDDATE + "/LEG1"];
	if (leg1enddateInput.findString('Y') != -1 || leg1enddateInput.findString('M') != -1 || 
		leg1enddateInput.findString('W') != -1 || leg1enddateInput.findString('D') != -1)
	{
		LADate startdate = dynamic_cast<LADataDate &>(pleg1->getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
		LADate tmpenddate = LAMathDateUtilities::getDate(startdate,leg1enddateInput,leg1conventionInput,leg1calendarInput);
		pleg1->LAObject::add(PRICING_DATA_ENDDATE, new LADataDate(tmpenddate));
	}
	else if (leg1enddateInput.size() == 8)
	{
		pleg1->LAObject::add(PRICING_DATA_ENDDATE, new LADataDate()).convertFromString(leg1enddateInput);
	}
	else 
		throw LACoreInvalidData("Leg1 EndDate input error",__FILE__,__LINE__);

	//frequency
	LAString leg1fregInput = maptrade[orgtradeid + "/" + PRICING_DATA_FREQUENCY + "/LEG1"];
	pleg1->LAObject::add(PRICING_DATA_FREQUENCY, new LADataString()).convertFromString(leg1fregInput);
	//rollconvention
	LAString leg1rollconvInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLCONVENTION + "/LEG1"];
	if(leg1rollconvInput != "")
		pleg1->LAObject::add(PRICING_DATA_ROLLCONVENTION, new LADataString()).convertFromString(leg1rollconvInput);
	//notional
	LAString leg1notionalInput = maptrade[orgtradeid + "/" + PRICING_CALIBRATION_DATAOTIONAL + "/LEG1"];
	pleg1->LAObject::add(PRICING_CALIBRATION_DATAOTIONAL, new LADataDouble()).convertFromString(leg1notionalInput);
	//paymenttiming
	pleg1->LAObject::add(PRICING_DATA_PAYMENTTIMING, new LADataString()).convertFromString(paytimingInput1);
	//selectside
	LAString leg1PRInput = maptrade[orgtradeid + "/" + PRICING_DATA_SELECTSIDE + "/LEG1"];
	pleg1->LAObject::add(PRICING_DATA_SELECTSIDE, new LADataString()).convertFromString(leg1PRInput);
	//coupondays
	LAString leg1cdayInput = maptrade[orgtradeid + "/" + PRICING_DATA_COUPONDAY + "/LEG1"];
	pleg1->LAObject::add(PRICING_DATA_COUPONDAY, new LADataInt()).convertFromString(leg1cdayInput);
	//isforwardroll
	LAString leg1isforwardInput=maptrade[orgtradeid + "/" + PRICING_DATA_ISFORWARDROLL + "/LEG1"];
	if(leg1isforwardInput != "")	//if maptrade contains PRICING_DATA_ISFORWARDROLL, overwittern
		isforwardInput = leg1isforwardInput;
	pleg1->LAObject::add(PRICING_DATA_ISFORWARDROLL, new LADataBool()).convertFromString(isforwardInput);
	//discountcurve
	pleg1->LAObject::add(PRICING_DATA_DISCOUNTCURVE, new LADataString()).convertFromString(discountcurve);

	//LAString leg1compfreqInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLFREQUENCY + "/LEG1"];
	pleg1->LAObject::add(PRICING_DATA_ROLLFREQUENCY, new LADataString()).convertFromString(leg1compfreqInput);
	
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

				LADate tmppaydate = dynamic_cast<LADataDate &>(pleg1->getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
				
				//upfront payment is always spot
				DoubleVector upfeevec(1,upfee);
				pleg1->LAObject::add(PRICING_DATA_UPFRONTFEES, new LADataDoubles(upfeevec));
				DateVector payvec(1,tmppaydate);
				pleg1->LAObject::add(PRICING_DATA_UPFRONTPAYMENTDATES, new LADataDates(payvec));
			}
		}
	}
	//firstodddate
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIRSTODDDATE + "/LEG1");
	if(itmap != maptrade.end()){
		LAString leg1firstodddateInput = itmap->second;
		pleg1->LAObject::add(PRICING_DATA_FIRSTODDDATE, new LADataDate()).convertFromString(leg1firstodddateInput);
	}
	//lastodddate
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_LASTODDDATE + "/LEG1");
	if(itmap != maptrade.end()){
		LAString leg1lastodddateInput = itmap->second;
		pleg1->LAObject::add(PRICING_DATA_LASTODDDATE, new LADataDate()).convertFromString(leg1lastodddateInput);
	}
	//firsoddindextype
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIRSTODDINDEXTYPE + "/LEG1");
	if(itmap != maptrade.end()){
		LAString leg1firstoddindexInput = itmap->second;
		pleg1->LAObject::add(PRICING_DATA_FIRSTODDINDEXTYPE, new LADataString()).convertFromString(leg1firstoddindexInput);
	}
	//lastoddindextype
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_LASTODDINDEXTYPE + "/LEG1");
	if(itmap != maptrade.end()){
		LAString leg1firstoddindexInput = itmap->second;
		pleg1->LAObject::add(PRICING_DATA_LASTODDINDEXTYPE, new LADataString()).convertFromString(leg1firstoddindexInput);
	}
	

	objHolder = objPool.getObject(leg2Input);
	if (objHolder.isDefined())
	{
		pleg2 = &objHolder.get();
		//pleg2->clear();
		//cf generate
		dh = &(ret->getData(PRICING_DATA_CFGENERATOR,ISDEFINED));
		LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>(dh->get());
		const LAPriceCashFlowGenerator& cfgen = dynamic_cast<const LAPriceCashFlowGenerator &>(modelDataObj.getMethod());
		cfgen.clearCashlets(*pleg2);
		pleg2->clear();
	}
	else
	{
		pleg2= new LAObject();
		objPool.set(leg2Input,pleg2);
	}

	//name 
	pleg2->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(leg2Input);
	//ccyInput
	pleg2->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccyInput);
	//inputtype
	pleg2->LAObject::add(PRICING_DATA_INPUTTYPE, new LADataString()).convertFromString("Detail");
	//isamortize
	pleg2->LAObject::add(PRICING_DATA_ISAMORTIZE, new LADataBool(false));
	//isnotionalend
	pleg2->LAObject::add(PRICING_DATA_ISNOTIONALEXCHANGEATEND, new LADataBool(false));
	//isnotionalstart
	pleg2->LAObject::add(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, new LADataBool(false));
	//underlyings
	pleg2->LAObject::add(PRICING_DATA_COUPONINFOS, new LADataMultiReference()).convertFromString(coupon2Input);
	//calendar
	pleg2->LAObject::add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(leg2calendarInput);
	//slidingrule (payment)
	LAString leg2conventionInput = maptrade[orgtradeid + "/" + CALIBRATION_DATA_SLIDINGRULE + "/Adjustment/LEG2"];
	//NO_CHANGE means unadjsuted
	if (leg2conventionInput != "NO_CHANGE")
		leg2conventionInput = bdayconvInput2; 

	pleg2->LAObject::add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(leg2conventionInput);
	//slidingrule (strat/end date)
	leg2conventionInput = maptrade[orgtradeid + "/" + PRICING_DATA_CASHFLOWSLIDINGRULE + "/Adjustment/LEG2"];
	//NO_CHANGE means unadjsuted
	if (leg2conventionInput != "NO_CHANGE")
		leg2conventionInput = bdayconvInput2; 
	
	pleg2->LAObject::add(PRICING_DATA_CASHFLOWSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(leg2conventionInput);

	//daycount
	LAString leg2daycountInput = maptrade[orgtradeid + "/" + PRICING_DATA_DAYCOUNT + "/LEG2"];
	pleg2->LAObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(leg2daycountInput);
	
	//startdate
	LAString leg2startdateInput = maptrade[orgtradeid + "/" + PRICING_DATA_STARTDATE + "/LEG2"];
	//change string into date 
	if (leg2startdateInput.findString('Y') != -1 || leg2startdateInput.findString('M') != -1 || 
		leg2startdateInput.findString('W') != -1 || leg2startdateInput.findString('D') != -1)
	{
		LADate tmpstartdate = LAMathDateUtilities::getDate(basedate,leg2startdateInput,leg2conventionInput,leg2calendarInput);
		pleg2->LAObject::add(PRICING_DATA_STARTDATE, new LADataDate(tmpstartdate));
	}
	else if (leg2startdateInput.size() == 8)
	{
		pleg2->LAObject::add(PRICING_DATA_STARTDATE, new LADataDate()).convertFromString(leg2startdateInput);
	}
	else 
		throw LACoreInvalidData("Leg2 StartDate input error",__FILE__,__LINE__);

	//enddate
	//change string into date
	LAString leg2enddateInput = maptrade[orgtradeid + "/" + PRICING_DATA_ENDDATE + "/LEG2"];
	if (leg2enddateInput.findString('Y') != -1 || leg2enddateInput.findString('M') != -1 || 
		leg2enddateInput.findString('W') != -1 || leg2enddateInput.findString('D') != -1)
	{
		LADate startdate = dynamic_cast<LADataDate &>(pleg2->getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
		LADate tmpenddate = LAMathDateUtilities::getDate(startdate,leg2enddateInput,leg2conventionInput,leg2calendarInput);
		pleg2->LAObject::add(PRICING_DATA_ENDDATE, new LADataDate(tmpenddate));
	}
	else if (leg2enddateInput.size() == 8)
	{
		pleg2->LAObject::add(PRICING_DATA_ENDDATE, new LADataDate()).convertFromString(leg2enddateInput);
	}
	else 
		throw LACoreInvalidData("Leg2 EndDate input error",__FILE__,__LINE__);

	//frequency
	LAString leg2fregInput = maptrade[orgtradeid + "/" + PRICING_DATA_FREQUENCY + "/LEG2"];
	pleg2->LAObject::add(PRICING_DATA_FREQUENCY, new LADataString()).convertFromString(leg2fregInput);
	//roll convention
	LAString leg2rollconvInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLCONVENTION + "/LEG2"];
	if(leg2rollconvInput != "")
		pleg2->LAObject::add(PRICING_DATA_ROLLCONVENTION, new LADataString()).convertFromString(leg2rollconvInput);
	//notional
	LAString leg2notionalInput = maptrade[orgtradeid + "/" + PRICING_CALIBRATION_DATAOTIONAL + "/LEG2"];
	pleg2->LAObject::add(PRICING_CALIBRATION_DATAOTIONAL, new LADataDouble()).convertFromString(leg2notionalInput);
	//paymenttiming
	pleg2->LAObject::add(PRICING_DATA_PAYMENTTIMING, new LADataString()).convertFromString(paytimingInput2);
	//selectside
	LAString leg2PRInput = maptrade[orgtradeid + "/" + PRICING_DATA_SELECTSIDE + "/LEG2"];
	pleg2->LAObject::add(PRICING_DATA_SELECTSIDE, new LADataString()).convertFromString(leg2PRInput);
	//coupondays
	LAString leg2cdayInput = maptrade[orgtradeid + "/" + PRICING_DATA_COUPONDAY + "/LEG2"];
	pleg2->LAObject::add(PRICING_DATA_COUPONDAY, new LADataInt()).convertFromString(leg2cdayInput);
	//isforwardroll 
	LAString leg2isforwardInput=maptrade[orgtradeid + "/" + PRICING_DATA_ISFORWARDROLL + "/LEG2"];
	if(leg2isforwardInput != "")	//if maptrade contains PRICING_DATA_ISFORWARDROLL, overwittern
		isforwardInput = leg2isforwardInput;
	pleg2->LAObject::add(PRICING_DATA_ISFORWARDROLL, new LADataBool()).convertFromString(isforwardInput);
	//discountcurve
	pleg2->LAObject::add(PRICING_DATA_DISCOUNTCURVE, new LADataString()).convertFromString(discountcurve);

	//LAString leg2compfreqInput = maptrade[orgtradeid + "/" + PRICING_DATA_ROLLFREQUENCY + "/LEG2"];
	pleg2->LAObject::add(PRICING_DATA_ROLLFREQUENCY, new LADataString()).convertFromString(leg2compfreqInput);

	//firstodddate
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIRSTODDDATE + "/LEG2");
	if(itmap != maptrade.end()){
		LAString leg2firstodddateInput = itmap->second;
		pleg2->LAObject::add(PRICING_DATA_FIRSTODDDATE, new LADataDate()).convertFromString(leg2firstodddateInput);
	}
	//lastodddate
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_LASTODDDATE + "/LEG2");
	if(itmap != maptrade.end()){
		LAString leg2lastodddateInput = itmap->second;
		pleg2->LAObject::add(PRICING_DATA_LASTODDDATE, new LADataDate()).convertFromString(leg2lastodddateInput);
	}
	//firsoddindextype
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIRSTODDINDEXTYPE + "/LEG2");
	if(itmap != maptrade.end()){
		LAString leg2firstoddindexInput = itmap->second;
		pleg2->LAObject::add(PRICING_DATA_FIRSTODDINDEXTYPE, new LADataString()).convertFromString(leg2firstoddindexInput);
	}
	//lastoddindextype
	itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_LASTODDINDEXTYPE + "/LEG2");
	if(itmap != maptrade.end()){
		LAString leg2firstoddindexInput = itmap->second;
		pleg2->LAObject::add(PRICING_DATA_LASTODDINDEXTYPE, new LADataString()).convertFromString(leg2firstoddindexInput);
	}
	//UpfrontFees	
	//UpfrontPaymentDates

	//set mincouponfile
	LAObject* pcoupon1 = NULL;
	LAObject* pcoupon2 = NULL;
	
	objHolder = objPool.getObject(coupon1Input);
	if (objHolder.isDefined())
	{
		pcoupon1 = &objHolder.get();
		pcoupon1->clear();
	}
	else
	{
		pcoupon1= new LAObject();
		objPool.set(coupon1Input,pcoupon1);
	}

	//name
	pcoupon1->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(coupon1Input);
	//ccyInput
	pcoupon1->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccyInput);
	//operator
	pcoupon1->LAObject::add(PRICING_DATA_OPERATOR, new LAPriceDataFunction()).convertFromString(FN_LINEAR_STR);
	//underlyings
	pcoupon1->LAObject::add(PRICING_DATA_INDEXINFOS, new LADataMultiReference()).convertFromString(index1Input);
	//coeff
	if (!istypicalpardeal)
	{
		LAString leg1coeffInput = maptrade[orgtradeid + "/" + PRICING_DATA_COEFFICIENT + "/COUPON1"];
		//if (!isleg1compounding)
		pcoupon1->LAObject::add(PRICING_DATA_COEFFICIENT, new LADataDoubles()).convertFromString(leg1coeffInput);
		/*else
			pcoupon1->LAObject::add(PRICING_DATA_COEFFICIENT, new LADataDoubles()).convertFromString("1.0:0.0");*/
	}
	// requirement by Lenny
	else
	{
		pcoupon1->LAObject::add(PRICING_DATA_COEFFICIENT, new LADataDoubles()).convertFromString("1.0:0.0");
	}


	objHolder = objPool.getObject(coupon2Input);
	if (objHolder.isDefined())
	{
		pcoupon2 = &objHolder.get();
		pcoupon2->clear();
	}
	else
	{
		pcoupon2= new LAObject();
		objPool.set(coupon2Input,pcoupon2);
	}

	//name
	pcoupon2->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(coupon2Input);
	//ccyInput
	pcoupon2->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccyInput);
	//operator
	pcoupon2->LAObject::add(PRICING_DATA_OPERATOR, new LAPriceDataFunction()).convertFromString(FN_LINEAR_STR);
	//underlyings
	pcoupon2->LAObject::add(PRICING_DATA_INDEXINFOS, new LADataMultiReference()).convertFromString(index2Input);
	//coeff
	if (!istypicalpardeal)
	{
		LAString leg2coeffInput = maptrade[orgtradeid + "/" + PRICING_DATA_COEFFICIENT + "/COUPON2"];
		//if (!isleg2compounding)
		pcoupon2->LAObject::add(PRICING_DATA_COEFFICIENT, new LADataDoubles()).convertFromString(leg2coeffInput);
		/*else
			pcoupon2->LAObject::add(PRICING_DATA_COEFFICIENT, new LADataDoubles()).convertFromString("1.0:0.0");*/
	}
	// requirement by Lenny
	else
	{
		pcoupon2->LAObject::add(PRICING_DATA_COEFFICIENT, new LADataDoubles()).convertFromString("1.0:0.0");
	}

	//set minindexfile
	LAObject* pindex2 = NULL;
	LAObject* pindex1 = NULL;


	//counpon1 index setting --------------------------------  

	objHolder = objPool.getObject(index1Input);
	if (objHolder.isDefined())
	{
		pindex1 = &objHolder.get();
		pindex1->clear();
	}
	else
	{
		pindex1= new LAObject();
		objPool.set(index1Input,pindex1);
	}

	//name
	pindex1->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(index1Input);
	//ccyInput
	pindex1->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccyInput);

	//indextype 
	//if "" then LIBOR 
	LAString leg1acessoryInput = maptrade[orgtradeid + "/" + PRICING_DATA_ACCESSORY + "/INDEX1"];
	if (leg1indextypeInput == "OIS")
	{
		//in case of OIS we must change offset,spotlag as 0 and Accessory must be 1D
		if (leg1acessoryInput.toUpper() != "1D")
			throw LACoreInvalidData("ois, ff index set error",__FILE__,__LINE__);

	}
	else if (leg1indextypeInput == "FIXEDRATE" || leg1indextypeInput.size() < 1)
	{
		leg1indextypeInput = "FIXEDRATE";
	}
	else if (leg1indextypeInput != "LIBOR")
		throw LACoreInvalidData("index set error",__FILE__,__LINE__);

	if (isleg1compounding)
	{
		LAString leg1coeffInput = maptrade[orgtradeid + "/" + PRICING_DATA_COEFFICIENT + "/COUPON1"];
		LAStringVector tmp = leg1coeffInput.toToken(':');
		if (tmp.size() != 2) 
			throw LACoreInvalidData("coefficient error", __FILE__,__LINE__);

		//pindex1->LAObject::add(PRICING_DATA_COMPOUNDINGMARGIN, new LADataDouble(tmp[1].getDoubleValue()));

		LAString leg1compcalcInput = maptrade[orgtradeid + "/" + PRICING_DATA_OBSERVATIONOPERATOR + "/INDEX1"];
		if (leg1compcalcInput == "COMPOUNDING")
		{
			pleg1->LAObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new LAPriceDataFunction()).convertFromString(FN_COMPOUNDING1_STR);
		}
		else if (leg1compcalcInput == "FLATCOMPOUNDING")
		{
			pleg1->LAObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new LAPriceDataFunction()).convertFromString(FN_COMPOUNDING8_STR);
		}
		else if (leg1compcalcInput == "DAILYAVERAGING")
		{
			pleg1->LAObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new LAPriceDataFunction()).convertFromString(FN_COMPOUNDING10_STR);
			pleg1->LAObject::add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new LADataBool(true));
		}
		else if (leg1compcalcInput == "AVERAGING")
		{
			pleg1->LAObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new LAPriceDataFunction()).convertFromString(FN_COMPOUNDING10_STR);
			pleg1->LAObject::add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new LADataBool(false));
		}
		else
			throw LACoreInvalidData("compounding function error",__FILE__,__LINE__);

        pleg1->add(PRICING_DATA_ISCOMPOUNDINGCOUPON, new LADataBool(true));
	}

	//each single deal
	LAString leg1indextypekey = LAString("INDEX_") + ccyInput + "_" + leg1indextypeInput + "_";
	LAString leg1offset,leg1fixingtimingInput,leg1fixingcalendarInput;
	if (leg1indextypeInput != "FIXEDRATE")
	{
		//spot lag
		leg1offset = dynamic_cast<LADataString &>(eline.getData(leg1indextypekey + PRICING_DATA_SPOTLAG, ISNOTNULL).get());
		//fixing timing
		leg1fixingtimingInput = dynamic_cast<LADataString &>(eline.getData(leg1indextypekey + PRICING_DATA_FIXINGTIMING, ISNOTNULL).get());
		//fixing calendar
		if (maptrade.find(orgtradeid + "/" + leg1indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR + "/LEG1") != maptrade.end())
			leg1fixingcalendarInput = maptrade[orgtradeid + "/" + leg1indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR + "/LEG1"];
		else if (maptrade.find(orgtradeid + "/" + leg1indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR) != maptrade.end())
			leg1fixingcalendarInput = maptrade[orgtradeid + "/" + leg1indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR];
		else
			leg1fixingcalendarInput = dynamic_cast<LADataString &>(eline.getData(leg1indextypekey + PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get());
	}

	////daycount
	pindex1->LAObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(leg1daycountInput);
	
	//accessory
	if (leg1indextypeInput != "FIXEDRATE")
	{
		pindex1->LAObject::add(PRICING_DATA_ACCESSORY, new LADataString()).convertFromString(leg1acessoryInput);
		//calendar
		pindex1->LAObject::add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(leg1fixingcalendarInput);
		//off set
		pindex1->LAObject::add(PRICING_DATA_OFFSET, new LADataInt()).convertFromString(leg1offset);
		////spot lag
		pindex1->LAObject::add(PRICING_DATA_SPOTLAG, new LADataInt()).convertFromString(leg1offset);
		//fixing timing
		pindex1->LAObject::add(PRICING_DATA_FIXINGTIMING, new LADataString()).convertFromString(leg1fixingtimingInput);
		//
		pindex1->LAObject::add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString("FOLLOWING");
		////fixing slidingrule
		pindex1->LAObject::add(PRICING_DATA_FIXINGSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString("FOLLOWING");
		//fixing calendar
		pindex1->LAObject::add(PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(leg1fixingcalendarInput);

		//basiscurve 
		LAString leg1forcuaveInput;
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
			throw LACoreInvalidData("Index Curve Set Error",__FILE__,__LINE__);
		
		pindex1->LAObject::add(PRICING_DATA_BASISCURVE, new LADataString()).convertFromString(leg1forcuaveInput);
		//basisinterpolation
		pindex1->LAObject::add(PRICING_DATA_BASISINTERPOLATION, new LADataString()).convertFromString(FN_LINEARINTERPOLATION_STR);
	}
	
	if (leg1indextypeInput == "LIBOR")
	{
		//is fwd intepolation
		pindex1->LAObject::add(PRICING_DATA_ISFWDINTERPOLATION, new LADataBool()).convertFromString(isforwardinterInput);
		//fwd intepolation 
		pindex1->LAObject::add(PRICING_DATA_FWDINTERPOLATION, new LAPriceDataInterpolation()).convertFromString(FN_SPLINEINTERPOLATION_STR);
		//frn
		pindex1->LAObject::add(PRICING_DATA_INDEXGENERATEMETHOD, new LADataString()).convertFromString("FRN");
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
				LAString leg1rateInput = itmap->second;
				pindex1->LAObject::add(PRICING_DATA_FIXEDRATE, new LADataDouble()).convertFromString(leg1rateInput);

				//index type
				pindex1->LAObject::add(PRICING_DATA_INDEXTYPE, new LADataString()).convertFromString("FIXEDRATE");

				//index name
				pleg1->LAObject::add(PRICING_DATA_INDEXNAME, new LADataString()).convertFromString("FIXEDRATE");
			}
			else 
				throw LACoreInvalidData("fixed rate set error",__FILE__,__LINE__);
		}
		else if (lineproduc == "BASISSWAP")
		{
			//basis case
			////libor freq
			pindex1->LAObject::add(PRICING_DATA_FREQUENCY, new LADataString()).convertFromString("SIMPLE");
			//inex type
			pindex1->LAObject::add(PRICING_DATA_INDEXTYPE, new LADataString()).convertFromString(leg1indextypeInput);
			//index name
			pleg1->LAObject::add(PRICING_DATA_INDEXNAME, new LADataString()).convertFromString(ccyInput.toUpper() + "_" + leg1indextypeInput);
		}
		else 
			throw LACoreInvalidData("product error",__FILE__,__LINE__);
	}
	else
	{
		//
		if (lineproduc == "SWAP" || lineproduc == "FRA")
		{
			itmap = maptrade.find(orgtradeid + "/" + PRICING_DATA_FIXEDRATE + "/INDEX1");
			LAString leg1rateInput = itmap->second;
			if (itmap != maptrade.end())
				pindex1->LAObject::add(PRICING_DATA_FIXEDRATE, new LADataDouble()).convertFromString(leg1rateInput);
			else
				pindex1->LAObject::add(PRICING_DATA_FIXEDRATE, new LADataDouble()).convertFromString("0.01");

			//inex type
			pindex1->LAObject::add(PRICING_DATA_INDEXTYPE, new LADataString()).convertFromString("FIXEDRATE");
			//inex name
			pleg1->LAObject::add(PRICING_DATA_INDEXNAME, new LADataString()).convertFromString("FIXEDRATE");
		}
		else if (lineproduc == "BASISSWAP")
		{
			//basis case
			////libor freq
			pindex1->LAObject::add(PRICING_DATA_FREQUENCY, new LADataString()).convertFromString("SIMPLE");
			//inex type
			pindex1->LAObject::add(PRICING_DATA_INDEXTYPE, new LADataString()).convertFromString(leg1indextypeInput);
			//inex name
			pleg1->LAObject::add(PRICING_DATA_INDEXNAME, new LADataString()).convertFromString(ccyInput.toUpper() + "_" + leg1indextypeInput);
		}
		else 
			throw LACoreInvalidData("product error",__FILE__,__LINE__);
	}
	
	//first fixing rate
	//first fixing basedate
	if (!istypicalpardeal)
	{
	
		itmap = maptrade.find(orgtradeid + "/" + "FirstFixingRate" + "/INDEX1");
		if (itmap != maptrade.end())
		{
			LAString leg1firstfixngrateInput = itmap->second;
			pindex1->LAObject::add(PRICING_DATA_FIRSTFIXINGRATE, new LADataDouble()).convertFromString(leg1firstfixngrateInput);
			pindex1->LAObject::add(PRICING_DATA_FIRSTFIXINGBASEDATE, new LADataDate()).convertFromString(asofdateInput);
		}
	}

	//add oddindexinfo and generate oddindexinfo incase there are firstodddate or lastodddate in paymentdates
	LADataHolder* ah_temp1 = &(pleg1->getData(PRICING_DATA_FIRSTODDDATE, NOCHECK));
	LADataHolder* ah_temp2 = &(pleg1->getData(PRICING_DATA_LASTODDDATE, NOCHECK));
	if(leg1indextypeInput == "LIBOR" && (ah_temp1->isDefined() || ah_temp2->isDefined()))
	{
		// we must add two libor index except index of trade for stub interpolation.
		vector<LAString> oddIndex1Input(2);
		oddIndex1Input[0] = coupon1Input + "_oddIndex1";
		oddIndex1Input[1] = coupon1Input + "_oddIndex2";
		pcoupon1->LAObject::add(PRICING_DATA_ODDINDEXINFOS, new LADataMultiReference()).convertFromString(oddIndex1Input[0] + ":" + oddIndex1Input[1]);

		vector<LAString> accessory(2), basisCurve(2);
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
			throw LACoreInvalidData("invaild accessory type for stub interpolation.",__FILE__,__LINE__);

		for (unsigned int i = 0; i < oddIndex1Input.size(); i++ )
		{
			LAObject* pindex1 = NULL;
			objHolder = objPool.getObject(oddIndex1Input[i]);
			if (objHolder.isDefined())
			{
				pindex1 = &objHolder.get();
				pindex1->clear();
			}
			else
			{
				pindex1= new LAObject();
				objPool.set(oddIndex1Input[i], pindex1);
			}

			//name
			pindex1->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(oddIndex1Input[i]);
			//ccyInput
			pindex1->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccyInput);
			////daycount
			pindex1->LAObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(leg1daycountInput);
			//accessory
			pindex1->LAObject::add(PRICING_DATA_ACCESSORY, new LADataString()).convertFromString(accessory[i]);
			//calendar
			pindex1->LAObject::add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(leg1fixingcalendarInput);
			//off set
			pindex1->LAObject::add(PRICING_DATA_OFFSET, new LADataInt()).convertFromString(leg1offset);
			////spot lag
			pindex1->LAObject::add(PRICING_DATA_SPOTLAG, new LADataInt()).convertFromString(leg1offset);
			//fixing timing
			pindex1->LAObject::add(PRICING_DATA_FIXINGTIMING, new LADataString()).convertFromString(leg1fixingtimingInput);
			//
			pindex1->LAObject::add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString("FOLLOWING");
			////fixing slidingrule
			pindex1->LAObject::add(PRICING_DATA_FIXINGSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString("FOLLOWING");
			//fixing calendar
			pindex1->LAObject::add(PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(leg1fixingcalendarInput);
			//fwd intepolation 
			pindex1->LAObject::add(PRICING_DATA_ISFWDINTERPOLATION, new LADataBool()).convertFromString(isforwardinterInput);
			pindex1->LAObject::add(PRICING_DATA_FWDINTERPOLATION, new LAPriceDataInterpolation()).convertFromString(FN_SPLINEINTERPOLATION_STR);
			//frn
			pindex1->LAObject::add(PRICING_DATA_INDEXGENERATEMETHOD, new LADataString()).convertFromString("FRN");
			////libor freq
			pindex1->LAObject::add(PRICING_DATA_FREQUENCY, new LADataString()).convertFromString("SIMPLE");
			//inex type
			pindex1->LAObject::add(PRICING_DATA_INDEXTYPE, new LADataString()).convertFromString(leg1indextypeInput);
			////index name
			//pleg1->LAObject::add(PRICING_DATA_INDEXNAME, new LADataString()).convertFromString(ccyInput.toUpper() + "_" + leg1indextypeInput);
			//basiscurve
			pindex1->LAObject::add(PRICING_DATA_BASISCURVE, new LADataString()).convertFromString(basisCurve[i]);
			//basisinterpolation
			pindex1->LAObject::add(PRICING_DATA_BASISINTERPOLATION, new LADataString()).convertFromString(FN_LINEARINTERPOLATION_STR);

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
		pindex2= new LAObject();
		objPool.set(index2Input,pindex2);
	}

	//name
	pindex2->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(index2Input);
	//ccyInput
	pindex2->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccyInput);

	//indextype 
	//if "" then LIBOR 
	LAString leg2acessoryInput = maptrade[orgtradeid + "/" + PRICING_DATA_ACCESSORY + "/INDEX2"];
	if (leg2indextypeInput == "OIS")
	{
		//in case of OIS we must change offset,spotlag as 0 and Accessory must be 1D
		if (leg2acessoryInput.toUpper() != "1D")
			throw LACoreInvalidData("index set error",__FILE__,__LINE__);
	}
	else if (leg2indextypeInput == "FIXEDRATE" || leg2indextypeInput.size() < 1)
	{
		throw LACoreInvalidData("index set error",__FILE__,__LINE__);
	}
	else if (leg2indextypeInput != "LIBOR")
		throw LACoreInvalidData("index set error",__FILE__,__LINE__);

	if (isleg2compounding)
	{
		LAString leg2coeffInput = maptrade[orgtradeid + "/" + PRICING_DATA_COEFFICIENT + "/COUPON2"];
		LAStringVector tmp = leg2coeffInput.toToken(':');
		if (tmp.size() != 2) 
			throw LACoreInvalidData("coefficient error", __FILE__,__LINE__);

		//pindex2->LAObject::add(PRICING_DATA_COMPOUNDINGMARGIN, new LADataDouble(tmp[1].getDoubleValue()));

		LAString leg2compcalcInput = maptrade[orgtradeid + "/" + PRICING_DATA_OBSERVATIONOPERATOR + "/INDEX2"];
		if (leg2compcalcInput == "COMPOUNDING")
			pleg2->LAObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new LAPriceDataFunction()).convertFromString(FN_COMPOUNDING1_STR);
		else if (leg2compcalcInput == "DAILYAVERAGING")
		{
			pleg2->LAObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new LAPriceDataFunction()).convertFromString(FN_COMPOUNDING10_STR);
			pleg2->LAObject::add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new LADataBool(true));
		}
		else if (leg2compcalcInput == "AVERAGING")
		{
			pleg2->LAObject::add(PRICING_DATA_COMPOUNDING_FUNCTION, new LAPriceDataFunction()).convertFromString(FN_COMPOUNDING10_STR);
			pleg2->LAObject::add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new LADataBool(false));
		}
		else
			throw LACoreInvalidData("compounding function error",__FILE__,__LINE__);

        pleg2->add(PRICING_DATA_ISCOMPOUNDINGCOUPON, new LADataBool(true));
	}



	//each single deal
	LAString leg2indextypekey = LAString("INDEX_") + ccyInput + "_" + leg2indextypeInput + "_";
	//spot lag
	LAString leg2offset = dynamic_cast<LADataString &>(eline.getData(leg2indextypekey + PRICING_DATA_SPOTLAG, ISNOTNULL).get());
	//fixing timing
	LAString leg2fixingtimingInput = dynamic_cast<LADataString &>(eline.getData(leg2indextypekey + PRICING_DATA_FIXINGTIMING, ISNOTNULL).get());
	//fixing calendar
	LAString leg2fixingcalendarInput;
	if (maptrade.find(orgtradeid + "/" + leg2indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR + "/LEG2") != maptrade.end())
		leg2fixingcalendarInput = maptrade[orgtradeid + "/" + leg2indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR + "/LEG2"];
	else if (maptrade.find(orgtradeid + "/" + leg2indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR) != maptrade.end())
		leg2fixingcalendarInput = maptrade[orgtradeid + "/" + leg2indextypeInput + "/" + PRICING_DATA_FIXINGCALENDAR];
	else
		leg2fixingcalendarInput = dynamic_cast<LADataString &>(eline.getData(leg2indextypekey + PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get());
	//accessory
	pindex2->LAObject::add(PRICING_DATA_ACCESSORY, new LADataString()).convertFromString(leg2acessoryInput);
	//calendar
	pindex2->LAObject::add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(leg2fixingcalendarInput);
	//daycount
	pindex2->LAObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(leg2daycountInput);
	//libor freq
	pindex2->LAObject::add(PRICING_DATA_FREQUENCY, new LADataString()).convertFromString("SIMPLE");
	//inex type
	pindex2->LAObject::add(PRICING_DATA_INDEXTYPE, new LADataString()).convertFromString(leg2indextypeInput);
	//sliding rule
	pindex2->LAObject::add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString("FOLLOWING");
	//spot lag
	pindex2->LAObject::add(PRICING_DATA_SPOTLAG, new LADataInt()).convertFromString(leg2offset);
	//fixing timing
	pindex2->LAObject::add(PRICING_DATA_FIXINGTIMING, new LADataString()).convertFromString(leg2fixingtimingInput);
	//off set
	pindex2->LAObject::add(PRICING_DATA_OFFSET, new LADataInt()).convertFromString(leg2offset);
	//fixing slidingrule
	pindex2->LAObject::add(PRICING_DATA_FIXINGSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString("FOLLOWING");
	//fixing calendar
	pindex2->LAObject::add(PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(leg2fixingcalendarInput);
	//is fwd intepolation
	if (leg2indextypeInput == "LIBOR")
	{
		pindex2->LAObject::add(PRICING_DATA_ISFWDINTERPOLATION, new LADataBool()).convertFromString(isforwardinterInput);
		//fwd intepolation 
		pindex2->LAObject::add(PRICING_DATA_FWDINTERPOLATION, new LAPriceDataInterpolation()).convertFromString(FN_SPLINEINTERPOLATION_STR);
		//frn
		pindex2->LAObject::add(PRICING_DATA_INDEXGENERATEMETHOD, new LADataString()).convertFromString("FRN");
		
	}

	if (leg2indextypeInput == "FIXEDRATE")
	{
		//index name
		pleg2->LAObject::add(PRICING_DATA_INDEXNAME, new LADataString()).convertFromString("FIXEDRATE");
	}
	else 
	{
		//index name
		pleg2->LAObject::add(PRICING_DATA_INDEXNAME, new LADataString()).convertFromString(ccyInput.toUpper() + "_" + leg2indextypeInput);
	}
	
	//basiscurve
	LAString leg2forcuaveInput;
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
		throw LACoreInvalidData("Index Curve Set Error",__FILE__,__LINE__);
	
	pindex2->LAObject::add(PRICING_DATA_BASISCURVE, new LADataString()).convertFromString(leg2forcuaveInput);
	//basisinterpolation
	pindex2->LAObject::add(PRICING_DATA_BASISINTERPOLATION, new LADataString()).convertFromString(FN_LINEARINTERPOLATION_STR);

	//first fixing rate
	//first fixing basedate
	if (!istypicalpardeal)
	{
		itmap = maptrade.find(orgtradeid + "/" + "FirstFixingRate" + "/INDEX2");
		if (itmap != maptrade.end())
		{
			LAString leg2firstfixngrateInput = itmap->second;
			pindex2->LAObject::add(PRICING_DATA_FIRSTFIXINGRATE, new LADataDouble()).convertFromString(leg2firstfixngrateInput);
			pindex2->LAObject::add(PRICING_DATA_FIRSTFIXINGBASEDATE, new LADataDate()).convertFromString(asofdateInput);
		}
	}

	//add oddindexinfo and generate oddindexinfo incase there are firstodddate or lastodddate in paymentdates
	ah_temp1 = &(pleg2->getData(PRICING_DATA_FIRSTODDDATE, NOCHECK));
	ah_temp2 = &(pleg2->getData(PRICING_DATA_LASTODDDATE, NOCHECK));
	if(leg2indextypeInput == "LIBOR" && (ah_temp1->isDefined() || ah_temp2->isDefined()))
	{
		// we must add two libor index except index of trade for stub interpolation.
		vector<LAString> oddIndex2Input(2);
		oddIndex2Input[0] = coupon2Input + "_oddIndex1";
		oddIndex2Input[1] = coupon2Input + "_oddIndex2";
		pcoupon2->LAObject::add(PRICING_DATA_ODDINDEXINFOS, new LADataMultiReference()).convertFromString(oddIndex2Input[0] + ":" + oddIndex2Input[1]);

		vector<LAString> accessory(2), basisCurve(2);
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
			throw LACoreInvalidData("invaild accessory type for stub interpolation.",__FILE__,__LINE__);

		for (unsigned int i = 0; i < oddIndex2Input.size(); i++ )
		{
			LAObject* pindex2 = NULL;
			objHolder = objPool.getObject(oddIndex2Input[i]);
			if (objHolder.isDefined())
			{
				pindex2 = &objHolder.get();
				pindex2->clear();
			}
			else
			{
				pindex2= new LAObject();
				objPool.set(oddIndex2Input[i], pindex2);
			}

			//name
			pindex2->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(oddIndex2Input[i]);
			//ccyInput
			pindex2->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccyInput);
			////daycount
			pindex2->LAObject::add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(leg2daycountInput);
			//accessory
			pindex2->LAObject::add(PRICING_DATA_ACCESSORY, new LADataString()).convertFromString(accessory[i]);
			//calendar
			pindex2->LAObject::add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(leg2fixingcalendarInput);
			//off set
			pindex2->LAObject::add(PRICING_DATA_OFFSET, new LADataInt()).convertFromString(leg2offset);
			////spot lag
			pindex2->LAObject::add(PRICING_DATA_SPOTLAG, new LADataInt()).convertFromString(leg2offset);
			//fixing timing
			pindex2->LAObject::add(PRICING_DATA_FIXINGTIMING, new LADataString()).convertFromString(leg2fixingtimingInput);
			//
			pindex2->LAObject::add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString("FOLLOWING");
			////fixing slidingrule
			pindex2->LAObject::add(PRICING_DATA_FIXINGSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString("FOLLOWING");
			//fixing calendar
			pindex2->LAObject::add(PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(leg2fixingcalendarInput);
			//fwd intepolation 
			pindex2->LAObject::add(PRICING_DATA_ISFWDINTERPOLATION, new LADataBool()).convertFromString(isforwardinterInput);
			pindex2->LAObject::add(PRICING_DATA_FWDINTERPOLATION, new LAPriceDataInterpolation()).convertFromString(FN_SPLINEINTERPOLATION_STR);
			//frn
			pindex2->LAObject::add(PRICING_DATA_INDEXGENERATEMETHOD, new LADataString()).convertFromString("FRN");
			////libor freq
			pindex2->LAObject::add(PRICING_DATA_FREQUENCY, new LADataString()).convertFromString("SIMPLE");
			//inex type
			pindex2->LAObject::add(PRICING_DATA_INDEXTYPE, new LADataString()).convertFromString(leg2indextypeInput);
			////index name
			//pleg1->LAObject::add(PRICING_DATA_INDEXNAME, new LADataString()).convertFromString(ccyInput.toUpper() + "_" + leg1indextypeInput);
			//basiscurve
			pindex2->LAObject::add(PRICING_DATA_BASISCURVE, new LADataString()).convertFromString(basisCurve[i]);
			//basisinterpolation
			pindex2->LAObject::add(PRICING_DATA_BASISINTERPOLATION, new LADataString()).convertFromString(FN_LINEARINTERPOLATION_STR);

		}

	}

	dataInstance->getReferencePool().completeDependency();

	//generate !! and change MANUAL not to regenerate CF

	//in case of convergent at first we must expand cashflows.
	dh = &(ret->getData(PRICING_DATA_CFGENERATOR, ISNOTNULL));
	LADataProcedure& modelDataObj = dynamic_cast<LADataProcedure&>(dh->get());
	modelDataObj.calibrateModel(basedate);
	pleg1->getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).convertFromString("MANUAL");
	pleg2->getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).convertFromString("MANUAL");

	//setUpFixingRateFromPastRates
	setUpFixingRateFromPastRates(dataInstance,copytradeid, basedate);

	dataInstance->getReferencePool().completeDependency();

	return ret;
}

void 
LAMultiSwapPricer::createOfferBidAdjustEntity(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade, LAStringVector orgtradevec, bool isduplicatemode, LAString key_suffix)
{
	
	LADataHolder* dh;
	LAObjectPool& objPool = dataInstance->getObjectPool();

	LAStringVector orgEntityNameVec,adjustEntityNameVec,tradeNameVec,paraNameVec;

	//important clasee "lineparam1"
	LAObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	// necessary in case of index set
	LAString fcurvestr1m = dynamic_cast<LADataString &>(eline.getData("CURVE_1MFORECAST", ISNOTNULL).get());
	LAString fcurvestr3m = dynamic_cast<LADataString &>(eline.getData("CURVE_3MFORECAST", ISNOTNULL).get());
	LAString fcurvestr6m = dynamic_cast<LADataString &>(eline.getData("CURVE_6MFORECAST", ISNOTNULL).get());
	LAString fcurvestrois = dynamic_cast<LADataString &>(eline.getData("CURVE_OISFORECAST", ISNOTNULL).get());
	// std forcast
	LAString swapfreq = dynamic_cast<LADataString &>(eline.getData("SWAP_FREQUENCY", ISNOTNULL).get());
	LAString fcurvestrstd = dynamic_cast<LADataString &>(eline.getData(LAString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get());


	map<LAString, LAString>::iterator itmap = maptrade.begin();
	for (unsigned int i = 0; i < orgtradevec.size(); i++)
	{
		LAString copytradeid = orgtradevec[i];
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
		LAMathObjectValue& etrade = dynamic_cast<LAMathObjectValue &>(objPool.getObject(copytradeid, ENCHKTYPE_ISDEFINED).get());

		dh = &(etrade.getData("LineProductType", ISNOTNULL));
		LAString linetype = dynamic_cast<LADataString &>(dh->get());
		linetype.toUpper();

		LAStringVector riskDetail1obadjust(1);
		LAStringVector riskDetail2obadjust(1);

		bool ispositivedirection = true;
		if (linetype == "SWAP" || linetype == "FRA")
		{
			LAString leg1name = copytradeid + "_leg1";
			LAObject& eleg1 = objPool.getObject(leg1name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eleg1.getData(PRICING_DATA_SELECTSIDE,ISNOTNULL));
			//leg 1 is fixed side. if leg1 rcv direction is positive
			LAString leg1PRname = dynamic_cast<LADataString &>(dh->get());
			if (leg1PRname.toUpper() == "RCV")
				ispositivedirection = true;
			else 			
				ispositivedirection = false;

			if (!ispositivedirection)
				offerbidspread *= -1;

			riskDetail1obadjust[0] =  fcurvestrois + "+" + fcurvestrstd;
			riskDetail2obadjust[0] =  LAString(offerbidspread,3) + "_BPPARALLEL";
		}
		else if (linetype == "BASISSWAP")
		{
			
			LAString leg1name = copytradeid + "_leg1";
			LAObject& eleg1 = objPool.getObject(leg1name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eleg1.getData(PRICING_DATA_SELECTSIDE,ISNOTNULL));
			LAString leg1PRname = dynamic_cast<LADataString &>(dh->get());
			LAString index1name =  copytradeid + "_leg1_coupon1_index1";
			LAObject& eindex1 = objPool.getObject(index1name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eindex1.getData(PRICING_DATA_BASISCURVE,ISNOTNULL));
			LAString index1curve = dynamic_cast<LADataString &>(dh->get());

			LAString leg2name = copytradeid + "_leg2";
			LAObject& eleg2 = objPool.getObject(leg2name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eleg2.getData(PRICING_DATA_SELECTSIDE,ISNOTNULL));
			LAString leg2PRname = dynamic_cast<LADataString &>(dh->get());
			LAString index2name =  copytradeid + "_leg2_coupon2_index2";
			LAObject& eindex2 = objPool.getObject(index2name, ENCHKTYPE_ISDEFINED).get();
			dh = &(eindex2.getData(PRICING_DATA_BASISCURVE,ISNOTNULL));
			LAString index2curve = dynamic_cast<LADataString &>(dh->get());

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
				throw LACoreInvalidData("index pair is wrong setting",__FILE__,__LINE__);

			if (!ispositivedirection)
				offerbidspread *= -1;

			riskDetail2obadjust[0] =  LAString(offerbidspread,3) + "_BPPARALLEL";
		}

		LAString curveID = getCurveIDfromTradeReference(dataInstance,orgtradevec[i]);
		LAString paraname = getMarketParamfromTradeReference(dataInstance,orgtradevec[i]);
		
		LAString keyname= curveID + "_SOURCEDELTA_";
		keyname += riskDetail1obadjust[0] + "_" + riskDetail2obadjust[0];

		LAObjectHolder objHolder = objPool.getObject(keyname,ENCHKTYPE_NOCHECK);
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
	LAObject* pob = NULL;
	LAObjectHolder objHolder = objPool.getObject("lineofferbidentity", ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		pob = new LAObject();
		objPool.set("lineofferbidentity",pob);
	
	}
	else
	{
		pob = &(objHolder.get());
		pob->clear();
	}
	pob->add(CALIBRATION_DATA_NAME, new LADataString("lineofferbidentity"));

	
	pob->remove("orgEntityName");
	pob->add("orgEntityName", new LADataStrings(orgEntityNameVec));

	pob->remove("adjsutEntityName");
	pob->add("adjsutEntityName", new LADataStrings(adjustEntityNameVec));

	pob->remove("tradeEntityName");
	pob->add("tradeEntityName", new LADataStrings(tradeNameVec));

	pob->remove("paramEntityName");
	pob->add("paramEntityName", new LADataStrings(paraNameVec));

	return;
}


void
LAMultiSwapPricer::calcPortfolioRiskAndPV(LADataInstance* dataInstance, std::map<LAString, LAString> mapriskinfo)
{

	LADataHolder* dh;
	LAObjectPool& objPool = dataInstance->getObjectPool();

	//calc ports
	LAStringVector tradeVec;
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
			bool ispnl = dynamic_cast<LADataBool &>(dh->get());
			if (!ispnl)
				continue;
		}
		tradeVec.push_back(it->first);
	}

	if (tradeVec.size() < 1)
		throw LACoreInvalidData("no trades for risk calc",__FILE__,__LINE__);

	//important clasee "lineparam1"
	LAObject& eline = objPool.getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	
	// necessary in case of index set
	LAString fcurvestr1m = dynamic_cast<LADataString &>(eline.getData("CURVE_1MFORECAST", ISNOTNULL).get());
	LAString fcurvestr3m = dynamic_cast<LADataString &>(eline.getData("CURVE_3MFORECAST", ISNOTNULL).get());
	LAString fcurvestr6m = dynamic_cast<LADataString &>(eline.getData("CURVE_6MFORECAST", ISNOTNULL).get());
	LAString fcurvestrois = dynamic_cast<LADataString &>(eline.getData("CURVE_OISFORECAST", ISNOTNULL).get());
	// std forcast
	LAString swapfreq = dynamic_cast<LADataString &>(eline.getData("SWAP_FREQUENCY", ISNOTNULL).get());
	LAString fcurvestrstd = dynamic_cast<LADataString &>(eline.getData(LAString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get());
	////to calc gamma 1bp up and 1bp down need developing
	
	//swap shift
	LAStringVector marketboxvec(1);
	LAStringVector rateboxvec(1);
	LAString risktarget;
	LAString riskentitystr = "";
	
	risktarget = mapriskinfo[LAString("RISK/") + "Swap"].toUpper();
	if ("TRUE" == risktarget)
	{
		marketboxvec[0] = "swapRates";
//		rateboxvec[0] = fcurvestr3m;
		rateboxvec[0] = fcurvestrstd;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPSWAP", rateboxvec, marketboxvec,true);

		riskentitystr += "lineriskentitySOURCEBUMPSWAP";
		riskentitystr += ":";
	}

	risktarget = mapriskinfo[LAString("RISK/") + "Libor"].toUpper();
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

	risktarget = mapriskinfo[LAString("RISK/") + "FRA3M"].toUpper();
	if ("TRUE" == risktarget)
	{
		//future shift
		marketboxvec[0] = "fra3mRates";
		rateboxvec[0] = fcurvestrstd;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPFRA3M", rateboxvec, marketboxvec,true);
		riskentitystr += "lineriskentitySOURCEBUMPFRA3M";
		riskentitystr += ":";
	}

	risktarget = mapriskinfo[LAString("RISK/") + "FRA6M"].toUpper();
	if ("TRUE" == risktarget)
	{
		//future shift
		marketboxvec[0] = "fra6mRates";
		rateboxvec[0] = fcurvestrstd;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPFRA6M", rateboxvec, marketboxvec,true);
		riskentitystr += "lineriskentitySOURCEBUMPFRA6M";
		riskentitystr += ":";
	}


	risktarget = mapriskinfo[LAString("RISK/") + "Future"].toUpper();
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

	risktarget = mapriskinfo[LAString("RISK/") + "OISCurve"].toUpper();
	if ("TRUE" == risktarget)
	{
		//ois shift
		marketboxvec[0] = "oisRates";
		rateboxvec[0] = fcurvestrois;
		createSourceDeltaRiskEntity(dataInstance, "lineriskentitySOURCEBUMPOIS", rateboxvec, marketboxvec,true);
		riskentitystr += "lineriskentitySOURCEBUMPOIS";
		riskentitystr += ":";
	}


	risktarget = mapriskinfo[LAString("RISK/") + "3M6MBasis"].toUpper();
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

	//risktarget = mapriskinfo[LAString("RISK/") + "1M3MBasis"].toUpper();
	risktarget = mapriskinfo[LAString("RISK/") + "1M" + swapfreq + "Basis"].toUpper();
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
		throw LACoreInvalidData("1MFrocast input error",__FILE__,__LINE__);

	risktarget = mapriskinfo[LAString("RISK/") + "ZeroRiskTemporary"];
	if ("TRUE" == risktarget)
	{
		LAStringVector marketboxvec;
		LAStringVector rateboxvec(4);
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
	eline.add("riskentity", new LADataMultiReference()).convertFromString(riskentitystr);

	
	DoubleVector pvResultVec;
	calcRiskAndPV(dataInstance,tradeVec,pvResultVec);

	eline.remove("riskentity");

	//regsitered tradevecs eline. it is used when calling get risk result function.
	eline.remove("tradeforportrisk");
	if (tradeVec.size() != 0)
		eline.add("tradeforportrisk", new LADataStrings(tradeVec));

	return;
}


double 
LAMultiSwapPricer::getPortfolioRiskResult(LADataInstance* dataInstance, LAString marketkey, LAString marketgrid, LAString risktype)
{
	double ret = 0.0;

	LAObject& eline = dataInstance->getObjectPool().getObject("lineparam1", ENCHKTYPE_ISDEFINED).get();
	LADataHolder* dh = &(eline.getData("tradeforportrisk"));
	if (!dh->isDefined() || dh->isNull())
		return 0.0;

	const LAStringVector& tradeVec = dynamic_cast<LADataStrings &>(dh->get()).get();

	//now only support marketkey is curve box.
	LAString marketStr = marketkey;
	if (marketStr != "Libor" && marketStr != "Swap" && marketStr != "FRA3M" && marketStr != "FRA6M" && marketStr != "Future" &&
		marketStr != "3M6MBasis" && marketStr != "1M3MBasis" && marketStr != "1M6MBasis" &&
		marketStr != "OISCurve")
		throw LACoreInvalidData("market key error",__FILE__,__LINE__);


	//this is related with CalcportfolioRiskAndPV function
	//in source delta, typical example is 
	//LAString key = "SOURCEBUMPRISK_" + riskDetail1[j] + "_" + tmpvec[0] + "_" + tmpvec[1];

	LAString marketbox;
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
	LAString fcurvestr1m = dynamic_cast<LADataString &>(eline.getData("CURVE_1MFORECAST", ISNOTNULL).get());
	LAString fcurvestr3m = dynamic_cast<LADataString &>(eline.getData("CURVE_3MFORECAST", ISNOTNULL).get());
	LAString fcurvestr6m = dynamic_cast<LADataString &>(eline.getData("CURVE_6MFORECAST", ISNOTNULL).get());
	LAString fcurvestrois = dynamic_cast<LADataString &>(eline.getData("CURVE_OISFORECAST", ISNOTNULL).get());
	// std forcast
	LAString swapfreq = dynamic_cast<LADataString &>(eline.getData("SWAP_FREQUENCY", ISNOTNULL).get());
	LAString fcurvestrstd = dynamic_cast<LADataString &>(eline.getData(LAString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get());

	
	//search gencurvename from market box !! it depends on Currency so that we change the map in JPY case.
	LAString riskDetail;
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
		throw LACoreInvalidData("risktyp input error",__FILE__,__LINE__);

	LAString key = risktype + "RISK_" + riskDetail + "_" + marketbox + "_" + marketgrid;


	for (unsigned int i = 0; i < tradeVec.size(); i++)
	{
		LAObject& val = dataInstance->getObjectPool().getObject(tradeVec[i], ENCHKTYPE_ISDEFINED).get();
		dh = &(val.getData(key));
		if (dh->isDefined() && !dh->isNull())
			ret += dynamic_cast<LADataDouble &>(dh->get());
	
	}

	return ret;
}


void
LAMultiSwapPricer::storePastRates(LADataInstance* dataInstance, const LAStringMatrix& IndexInfo, const LAStringMatrix& PastRates)
{

	LAObjectPool &objPool = dataInstance->getObjectPool();
	LAObjectHolder objHolder;

	LAStringMatrix tmpInfos = IndexInfo;

	///////////////////line param object///////////////////////////////
	LAString lineparam("lineparam1");
	objHolder = objPool.getObject(lineparam, ENCHKTYPE_NOCHECK);
	LAObject* eline = NULL;
	if (!objHolder.isDefined())
	{
		eline = new LAObject();
		objPool.set(lineparam, eline);
	}
	else
	{
		eline = &(objHolder.get());
	}

	LAString ccy = LAFunctionUtilities::findElement(tmpInfos,"Currency",0,1,true,true).toUpper();
	LAString indextype = LAFunctionUtilities::findElement(tmpInfos,"Rate",0,1,true,true).toUpper();
	LAString accessory;
	if (indextype == "LIBOR")
	{
		accessory = LAFunctionUtilities::findElement(tmpInfos,"Term",0,1,true,true);
	}
	else if (indextype == "FF" || indextype == "OIS")
	{
		indextype = "OIS"; 
		accessory = "1D";
	}
	else 
		throw LACoreInvalidData("Rate type supports only Libor, FF and OIS",__FILE__,__LINE__);

	LAString keynamesuffix = "_" + ccy + "_" + indextype + "_" + accessory; 

	unsigned int pastSize = PastRates.size();
	DoubleVector ratesvec(pastSize);
	DateVector pastdates(pastSize);

	if (pastSize == 0)
	{
		throw LACoreInvalidData("PastRates set error",__FILE__,__LINE__);
	}
	
	if (pastSize != 0 && PastRates[0].size() < 2)
	{
		throw LACoreInvalidData("PastRates set error",__FILE__,__LINE__);
	}

	map<LADate, double> map_date_rate;
	for (unsigned int i = 0; i < pastSize; i++)
	{
		pastdates[i] = LAMathDateUtilities::getLADate(PastRates[i][0]);
		map_date_rate[pastdates[i]] = PastRates[i][1].getDoubleValue();
		//ratesvec[i] = PastRates[i][1].getDoubleValue();
	}

	sort(pastdates.begin(), pastdates.end());
	for (unsigned int i = 0; i < pastSize; i++)
	{
		ratesvec[i] = map_date_rate[pastdates[i]];
	}

	eline->remove("PastDates"+ keynamesuffix);
	eline->add("PastDates"+ keynamesuffix, new LADataDates(pastdates));

	eline->remove("PastRates"+ keynamesuffix);
	eline->add("PastRates"+ keynamesuffix, new LADataDoubles(ratesvec));



	return;

}

void
LAMultiSwapPricer::setUpFixingRateFromPastRates(LADataInstance* dataInstance, LAString copytradeid, const LADate& asOfDate)
{

	LADataHolder* dh;
	LAObjectPool &objPool = dataInstance->getObjectPool();

	LAObject& eline = objPool.getObject("lineparam1",ENCHKTYPE_ISDEFINED).get();
	
	LAObjectHolder objHolder = objPool.getObject(copytradeid, ENCHKTYPE_ISDEFINED);

	LAMathObjectValue& val = dynamic_cast<LAMathObjectValue&>(objHolder.get());
	
	dh = &(val.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	LADataMultiReference& legs = dynamic_cast<LADataMultiReference &>(dh->get());
	unsigned int legSize = legs.getSize();

	
	for (unsigned int i = 0; i < legSize; i++)
	{
		bool issearchend = false;

		dh = &(legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;
		
		LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference &>(dh->get());
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
			const LADate& paydate = dynamic_cast<LADataDate &>(dh->get()).get();
			if (paydate < asOfDate)
				continue;

			dh = &(cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
				continue;
			
				//multi coupon is only stub case
			LADataMultiReference& coupons = dynamic_cast<LADataMultiReference &>(dh->get());
			unsigned int couponSize = coupons.getSize();
			for (unsigned int k = 0; k < couponSize; k++)
			{
				dh = &(coupons.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
				if (!dh->isDefined() || dh->isNull())
					continue;
		
				LADataMultiReference& indexs = dynamic_cast<LADataMultiReference &>(dh->get());
				LAObject& eindex = indexs.get(0).get();

				//first priority is FixingDates
				dh = &(eindex.getData(PRICING_DATA_FIXINGDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					const DateVector& fixingdates = dynamic_cast<const LADataDates &>(dh->get()).get();
					LADate firstfixingdate = fixingdates[0];
					
					if (firstfixingdate > asOfDate)
						issearchend = true;
					else
					{
						//set up past rates
						LAString ccy = dynamic_cast<LADataString &>(eindex.getData(IR_CALIBRATION_DATA_CURRENCY, ISDEFINED).get());
						LAString accessory = dynamic_cast<LADataString &>(eindex.getData(PRICING_DATA_ACCESSORY, ISDEFINED).get());

						LAString keynamesuffix;
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
							
							throw LACoreInvalidData("past rates have not been stored", __FILE__,__LINE__);
						}
							
						DateVector pastdates = dynamic_cast<LADataDates &>(dh->get()).get();

						unsigned int firstpos = 0;
						bool isexist = LAAlgorithm::find<DateVector, LADate>(pastdates, firstfixingdate, 0, pastdates.size()-1, firstpos);
						if (!isexist)
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (firstfixingdate == asOfDate)
								continue;

							LAString ans = "fixing date " + LADataDate(firstfixingdate).convertToString() + " is not set";
							throw LACoreInvalidData(ans.getCString(), __FILE__,__LINE__);
						}

						dh = &(eline.getData("PastRates" + keynamesuffix, ISDEFINED));
						DoubleVector pastrates = dynamic_cast<LADataDoubles &>(dh->get()).get();
						if (pastdates.size() != pastrates.size())
							throw LACoreInvalidData("Past rates size error",__FILE__,__LINE__);

						pastrates.erase(pastrates.begin(),pastrates.begin() + firstpos);
						pastdates.erase(pastdates.begin(),pastdates.begin() + firstpos);

						eindex.remove(PRICING_DATA_OBSERVATIONDATES);
						eindex.add(PRICING_DATA_OBSERVATIONDATES, new LADataDates(pastdates));

						eindex.remove(PRICING_DATA_OBSERVATIONRATES);
						eindex.add(PRICING_DATA_OBSERVATIONRATES, new LADataDoubles(pastrates));

						/*double fixedrate = pastrates[pos];
						eindex.remove(PRICING_DATA_FIXEDRATE);
						eindex.add(PRICING_DATA_FIXEDRATE, new LADataDouble(fixedrate));*/
					}
					
					continue;
				}
			
			
				dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
				if (!dh->isDefined() || dh->isNull())
					continue;


				if (dynamic_cast<LADataString &>(dh->get()).get() == "LIBOR")
				{
					dh = &(eindex.getData(PRICING_DATA_FIXINGDATE, ISDEFINED));
					const LADate& fixingdate = dynamic_cast<LADataDate &>(dh->get()).get();
					
					//if fixing does not come, go out of for loop
					if (fixingdate > asOfDate)
						issearchend = true;
					else
					{
						//set up past rates
						LAString ccy = dynamic_cast<LADataString &>(eindex.getData(IR_CALIBRATION_DATA_CURRENCY, ISDEFINED).get());
						LAString accessory = dynamic_cast<LADataString &>(eindex.getData(PRICING_DATA_ACCESSORY, ISDEFINED).get());

						//if already have fixed rate, then we do not need to get from pastrates
						dh = &(eindex.getData(PRICING_DATA_FIXEDRATE, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
							continue;


						LAString keynamesuffix = "_" + ccy + "_LIBOR_" + accessory;
						dh = &(eline.getData("PastDates" + keynamesuffix, NOCHECK));
						if (!dh->isDefined() || dh->isNull())
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (fixingdate == asOfDate)
								continue;
							
							throw LACoreInvalidData("past rates have not been stored", __FILE__,__LINE__);
						}
							
						const DateVector& pastdates = dynamic_cast<LADataDates &>(dh->get()).get();

						unsigned int pos = 0;
						bool isexist = LAAlgorithm::find<DateVector, LADate>(pastdates, fixingdate, 0, pastdates.size()-1, pos);
						if (!isexist)
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (fixingdate == asOfDate)
								continue;

							LAString ans = "fixing date " + LADataDate(fixingdate).convertToString() + " is not set";
							throw LACoreInvalidData(ans.getCString(), __FILE__,__LINE__);
						}

						dh = &(eline.getData("PastRates" + keynamesuffix, ISDEFINED));
						const DoubleVector& pastrates = dynamic_cast<LADataDoubles &>(dh->get()).get();
						if (pastdates.size() != pastrates.size())
							throw LACoreInvalidData("Past rates size error",__FILE__,__LINE__);

						double fixedrate = pastrates[pos];
						eindex.remove(PRICING_DATA_FIXEDRATE);
						eindex.add(PRICING_DATA_FIXEDRATE, new LADataDouble(fixedrate));
					}
				}
				else if (dynamic_cast<LADataString &>(dh->get()).get() == "OIS")
				{

					dh = &(eindex.getData(PRICING_DATA_OBSERVATIONSTARTDATE, ISDEFINED));
					const LADate& firstfixingdate = dynamic_cast<LADataDate &>(dh->get()).get();

					dh = &(eindex.getData(PRICING_DATA_OBSERVATIONENDDATE, ISDEFINED));
					const LADate& lastfixingdate = dynamic_cast<LADataDate &>(dh->get()).get();
					
					//if fixing does not come, go out of for loop
					if (firstfixingdate > asOfDate)
						issearchend = true;
					else
					{
						//set up past rates
						LAString ccy = dynamic_cast<LADataString &>(eindex.getData(IR_CALIBRATION_DATA_CURRENCY, ISDEFINED).get());
						LAString accessory = dynamic_cast<LADataString &>(eindex.getData(PRICING_DATA_ACCESSORY, ISDEFINED).get());

						LAString keynamesuffix = "_" + ccy + "_OIS_" + accessory;
						dh = &(eline.getData("PastDates" + keynamesuffix, NOCHECK));
						if (!dh->isDefined() || dh->isNull())
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (firstfixingdate == asOfDate)
								continue;
							
							throw LACoreInvalidData("past rates have not been stored", __FILE__,__LINE__);
						}
							
						DateVector pastdates = dynamic_cast<LADataDates &>(dh->get()).get();

						unsigned int firstpos = 0;
						bool isexist = LAAlgorithm::find<DateVector, LADate>(pastdates, firstfixingdate, 0, pastdates.size()-1, firstpos);
						if (!isexist)
						{
							//if not exist and today fixing then, we adopt forecast, that is, we do not have any action.
							if (firstfixingdate == asOfDate)
								continue;

							LAString ans = "fixing date " + LADataDate(firstfixingdate).convertToString() + " is not set";
							throw LACoreInvalidData(ans.getCString(), __FILE__,__LINE__);
						}

						dh = &(eline.getData("PastRates" + keynamesuffix, ISDEFINED));
						DoubleVector pastrates = dynamic_cast<LADataDoubles &>(dh->get()).get();
						if (pastdates.size() != pastrates.size())
							throw LACoreInvalidData("Past rates size error",__FILE__,__LINE__);

						pastrates.erase(pastrates.begin(),pastrates.begin() + firstpos);
						pastdates.erase(pastdates.begin(),pastdates.begin() + firstpos);

						eindex.remove(PRICING_DATA_OBSERVATIONDATES);
						eindex.add(PRICING_DATA_OBSERVATIONDATES, new LADataDates(pastdates));

						eindex.remove(PRICING_DATA_OBSERVATIONRATES);
						eindex.add(PRICING_DATA_OBSERVATIONRATES, new LADataDoubles(pastrates));

						/*double fixedrate = pastrates[pos];
						eindex.remove(PRICING_DATA_FIXEDRATE);
						eindex.add(PRICING_DATA_FIXEDRATE, new LADataDouble(fixedrate));*/
					}
				}
			}
		}
		
	}




	return;



}


LAString
LAMultiSwapPricer::setUpMarketParamsAndGlobalShift(LADataInstance* dataInstance, std::map<LAString, LAString> mapcalcinfo, LAString curveID)
{

	LADataHolder* dh;
	LAObjectPool &objPool = dataInstance->getObjectPool();
	LAObjectHolder objHolder;

	map<LAString, LAString>::iterator itcalcmap = mapcalcinfo.begin();

	LAString marketparamInput = "marketparam1_" + curveID;
	LAMathPlainVanillaEntity* pvanilla = NULL;
	objHolder = objPool.getObject(marketparamInput);
	if (objHolder.isDefined())
	{
		if (!objHolder.isTypeOf(ENTITY_PLAINVANILLA))
			throw LACoreInvalidData("Vanilla Setup Error",__FILE__,__LINE__);

		pvanilla = dynamic_cast<LAMathPlainVanillaEntity *>(&objHolder.get());
		pvanilla->reset();
	}
	else
	{
		pvanilla = new LAMathPlainVanillaEntity(dataInstance);
		objPool.set(marketparamInput, pvanilla);
	}

	//name
	pvanilla->getData(CALIBRATION_DATA_NAME, ISDEFINED).convertFromString(marketparamInput);
	//daycount
	pvanilla->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISDEFINED).convertFromString("ACT/365_ISDA");
	//asofdate
	LAObject* pyldEntity = &(objPool.getObject(curveID,ENCHKTYPE_ISDEFINED).get());
	dh = &(pyldEntity->getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL));
	LAString asofdateInput = dh->convertToString();
	pvanilla->getData(CALIBRATION_DATA_ASOFDATE, ISDEFINED).convertFromString(asofdateInput);
	
	//ircurs
	//LAStringVector irccys(ccy,1)
	dh = &(pyldEntity->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL));
	LAString ccyInput = dynamic_cast<LADataString &>(dh->get()).get();
	LAStringVector ccys = ccyInput.toToken(':');
	if (ccys.size() > 1)
	{
		throw LACoreInvalidData("Multi currency pricing is not supported at the moment.", __FILE__, __LINE__);
	}
	pvanilla->getIRCurrencys().set(ccys);
	
	//set yield curve
	LAStringVector yieldNames(ccys.size());
	if (LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST).toUpper() == "TRUE")
	{
		std::for_each(yieldNames.begin(), yieldNames.end(), [curveID](LAString& yieldName) { yieldName = PREFIX_YIELD + curveID; });
	}
	else
	{
		std::transform(ccys.cbegin(), ccys.cend(), yieldNames.begin(), [](const LAString& ccy) { return LAMarketData::getBaseYieldName(ccy); });
	}
	for (auto&& yieldName : yieldNames)
	{
		objHolder = objPool.getObject(yieldName, ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			LAMathYieldCurve* pYield = new LAMathYieldCurve(dataInstance);
			pYield->getName().convertFromString(yieldName);
			pYield->getYieldData().convertFromString(curveID);
			pYield->getInterpolation().convertFromString(FN_SPLINEINTERPOLATION_STR);
			objPool.set(yieldName, pYield);
		}
	}
	pvanilla->getIRCurves().convertFromString(LADataStrings(yieldNames).convertToString());

	//set names of yield curve pro
	LAStringVector yieldProNames(ccys.size());
	if (LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST).toUpper() == "TRUE")
	{
		std::transform(yieldNames.cbegin(), yieldNames.cend(), yieldProNames.begin(), [](const LAString& yieldName) { return LAString("PRO_") + yieldName; });
	}
	else
	{
		std::transform(ccys.cbegin(), ccys.cend(), yieldProNames.begin(), [](const LAString& ccy) { return LAMarketData::getBaseYieldProName(ccy); });
	}
	for (auto&& yieldProName : yieldProNames)
	{
		objHolder = objPool.getObject(yieldProName, ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			LAMathYieldCurvePro* pYieldPro = new LAMathYieldCurvePro(dataInstance);
			pYieldPro->getName().convertFromString(yieldProName);
			pYieldPro->getYieldData().convertFromString(curveID);
			pYieldPro->getInterpolation().convertFromString(FN_SPLINEINTERPOLATION_STR);
			objPool.set(yieldProName, pYieldPro);
		}
	}
	pvanilla->getIRCurveProNames().set(yieldProNames);

	//temporary 
	//interpolationset, usually, we should on curve generate
	LAString fcurvestr1m = mapcalcinfo[LAString("CURVE/") + "1MForecast"];
	LAString fcurvestr3m = mapcalcinfo[LAString("CURVE/") + "3MForecast"];
	LAString fcurvestr6m = mapcalcinfo[LAString("CURVE/") + "6MForecast"];
	LAString fcurvestrois = mapcalcinfo[LAString("CURVE/") + "OISForecast"];
	pvanilla->getIRCurve(ccyInput).getInterpolation(fcurvestr1m).convertFromString(FN_SPLINEINTERPOLATION_STR);
	pvanilla->getIRCurve(ccyInput).getInterpolation(fcurvestr3m).convertFromString(FN_SPLINEINTERPOLATION_STR);
	pvanilla->getIRCurve(ccyInput).getInterpolation(fcurvestr6m).convertFromString(FN_SPLINEINTERPOLATION_STR);
	pvanilla->getIRCurve(ccyInput).getInterpolation(fcurvestrois).convertFromString(FN_SPLINEINTERPOLATION_STR);
	///////////////////////////////////////////////////////////////////////////////

	//global shift scenario////////////////////////////////////////////////
	std::map<LAString, LAString>::iterator it = mapcalcinfo.find(LAString("CURVE/") + "GlobalShiftScenario");
	if (it != mapcalcinfo.end() && it->second != "" && it->second.getDoubleValue() != 0.0)
	{

		////set up order 
		LAStringVector setupOrder = getCurveNamesFromSetUpOrder(dataInstance, curveID);
		LAString glshiftstr = it->second;
		double glshiftval = glshiftstr.getDoubleValue() * 0.0001;
		LAString orgcurveID = curveID;
		curveID += "_GlobalShift_" + glshiftstr + "BP";
		for (unsigned int j = 0; j < setupOrder.size(); j++)
		{
			LAString generateCurveName = setupOrder[j];
			
			
			LAString orgkeyname = "lineparam1_" + orgcurveID + generateCurveName;
			LAString newkeyname = "lineparam1_" + curveID + generateCurveName;
			
			objHolder = objPool.getObject(newkeyname, ENCHKTYPE_NOCHECK);
			if (objHolder.isDefined())
				objPool.remove(newkeyname);

							
			LAObject* eglcurvestore = objPool.getObject(orgkeyname, ENCHKTYPE_ISDEFINED).clone();
			objPool.set(newkeyname, eglcurvestore);
			eglcurvestore->getData(CALIBRATION_DATA_NAME, ISNOTNULL).convertFromString(newkeyname);

			bool isshiftarget = (generateCurveName == fcurvestrois || generateCurveName == fcurvestr3m);

			//create shiftcurve
			createShiftCurves(dataInstance, *eglcurvestore, generateCurveName,curveID,isshiftarget,true, 
											glshiftval,0,"ALL",true);
		}


		//reset into curve pro
		LAString yieldName = "YIELD_" + curveID;
		pvanilla->getIRCurves().convertFromString(yieldName);
		////reset yldpro
		//LAString yieldProName = "PRO_YIELD_" + curveID;
		//pvanilla->getIRCurvePros().convertFromString(yieldProName);

		//asofdate
		pyldEntity = &(objPool.getObject(curveID,ENCHKTYPE_ISDEFINED).get());
		dh = &(pyldEntity->getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL));
		LAString asofdateInput = dh->convertToString();
		pvanilla->getData(CALIBRATION_DATA_ASOFDATE, ISDEFINED).convertFromString(asofdateInput);
		
		//ircurs
		//LAStringVector irccys(ccy,1)
		dh = &(pyldEntity->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL));
		LAString ccyInput = dynamic_cast<LADataString &>(dh->get());
		pvanilla->getIRCurrencys().convertFromString(ccyInput);
	}

	LAString ret = marketparamInput;
	return ret;
}


LAString
LAMultiSwapPricer::getDiscountCurveName(LADataInstance* dataInstance, LAString curveID)
{
	LAString ret;
	LAObjectPool& objPool = dataInstance->getObjectPool();




	const LAString lineparam("lineparam1");
	const LAObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();
    const LAObject* pyldEntity = &(objPool.getObject(curveID, ENCHKTYPE_ISDEFINED).get());

    
	const LAString swapfreq = dynamic_cast<const LADataString &>(eline->getData("SWAP_FREQUENCY", ISNOTNULL).get()).get();

	const LAString ois_fcurve_name = dynamic_cast<const LADataString &>(eline->getData("CURVE_OISFORECAST", ISNOTNULL).get()).get();
	const LAString std_fcurve_name = dynamic_cast<const LADataString &>(eline->getData(LAString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get()).get();


    const LAString& ccy = dynamic_cast<const LADataString&>(pyldEntity->getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL).get()).get();
    //const LAString& ypro_name = "PRO_" + LAMarketData::getBaseYieldName(ccy);
    const LAString& ypro_name = LAString("PRO_") + PREFIX_YIELD + curveID;
    const LAMathYieldCurvePro& ypro = dynamic_cast<const LAMathYieldCurvePro&>(objPool.getObject(ypro_name, ENCHKTYPE_ISDEFINED).get());
    const LAString& ois_mkt_name = ypro.getMarketForCurve(ois_fcurve_name);


	
    const LAString ois_dcurve_data_name = IR_CALIBRATION_DATA_DFCURVENAME + LAString("_") + ois_mkt_name;
	const LAString std_dcurve_name = dynamic_cast<const LADataString &>(pyldEntity->getData(IR_CALIBRATION_DATA_DFCURVENAME, ISNOTNULL).get());
	const LAString ois_dcurve_name = dynamic_cast<const LADataString &>(pyldEntity->getData(ois_dcurve_data_name , ISNOTNULL).get());







	if ("ITSELF" == std_dcurve_name && std_fcurve_name == ois_dcurve_name)
		ret = std_fcurve_name;
	else if (ois_fcurve_name == std_dcurve_name && "ITSELF" == ois_dcurve_name)
		ret = ois_fcurve_name;
	else
		throw LACoreInvalidData("Discount curve set logic is not consistent", __FILE__,__LINE__);

	return ret;
}



LAString
LAMultiSwapPricer::getCurveIDfromTradeReference(LADataInstance* dataInstance, LAString tradeID)
{

	LAString ret;
	LAObjectPool& objPool = dataInstance->getObjectPool();
	LADataHolder* dh;

	LAString lineparam("lineparam1");
	LAObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();

	LAString ccyInput = dynamic_cast<LADataString &>(eline->getData("CALC_CURRENCY", ISNOTNULL).get());
	
	//discount curve search
	LAObjectHolder objHolder = objPool.getObject(tradeID, ENCHKTYPE_ISDEFINED);
	LAMathObjectValue& val = dynamic_cast<LAMathObjectValue &>(objHolder.get());

	dh = &(val.getData(PRICING_DATA_PATHENTITY, ISNOTNULL));
	LADataReference& ref = dynamic_cast<LADataReference& >(dh->get());
	LAMathPlainVanillaEntity& vanilla = dynamic_cast<LAMathPlainVanillaEntity &>(ref.get().get());
	
	LADataReference& yielddata = dynamic_cast<LADataReference& >(vanilla.getIRCurve(ccyInput).getData(IR_CALIBRATION_DATA_YIELDDATA, ISNOTNULL).get());
	return yielddata.get().getName();

}

LAString
LAMultiSwapPricer::getMarketParamfromTradeReference(LADataInstance* dataInstance, LAString tradeID)
{

	LAString ret;
	LAObjectPool& objPool = dataInstance->getObjectPool();
	LADataHolder* dh;

	LAString lineparam("lineparam1");
	LAObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();

	LAString ccyInput = dynamic_cast<LADataString &>(eline->getData("CALC_CURRENCY", ISNOTNULL).get());
	
	//discount curve search
	LAObjectHolder objHolder = objPool.getObject(tradeID, ENCHKTYPE_ISDEFINED);
	LAMathObjectValue& val = dynamic_cast<LAMathObjectValue &>(objHolder.get());

	dh = &(val.getData(PRICING_DATA_PATHENTITY, ISNOTNULL));
	LADataReference& ref = dynamic_cast<LADataReference &>(dh->get());

	return ref.get().getName();

}

void
LAMultiSwapPricer::setUpLineParams(LADataInstance* dataInstance, std::map<LAString, LAString> mapcalcinfo)
{
	LAObjectHolder objHolder;
	LAObjectPool& objPool = dataInstance->getObjectPool();

	///////////////////line param object///////////////////////////////
	//interpolationset, usually, we should on curve generate
	LAString fcurvestr1m = mapcalcinfo[LAString("CURVE/") + "1MForecast"];
	LAString fcurvestr3m = mapcalcinfo[LAString("CURVE/") + "3MForecast"];
	LAString fcurvestr6m = mapcalcinfo[LAString("CURVE/") + "6MForecast"];
	LAString fcurvestrois = mapcalcinfo[LAString("CURVE/") + "OISForecast"];
	LAString discount = mapcalcinfo[LAString("CURVE/") + "Discount"];
	LAString lineparam("lineparam1");
	objHolder = objPool.getObject(lineparam, ENCHKTYPE_NOCHECK);
	LAObject* eline = NULL;
	if (!objHolder.isDefined())
	{
		eline = new LAObject();
		objPool.set(lineparam, eline);
	}
	else
	{
		eline = &(objHolder.get());
	}
	
	//calc info
	LAString calcstr = mapcalcinfo[LAString("CALC/") + IR_CALIBRATION_DATA_CURRENCY];
	eline->remove("CALC_CURRENCY");
	eline->add("CALC_CURRENCY", new LADataString(calcstr));

	LAString calcasofstr = mapcalcinfo[LAString("CALC/") + CALIBRATION_DATA_ASOFDATE];
	eline->remove("CALC_ASOFDATE");
	eline->add("CALC_ASOFDATE", new LADataString(calcasofstr));

	LAString calcvaluestr = mapcalcinfo[LAString("CALC/") + CALIBRATION_DATA_VALUEDATE];
	eline->remove("CALC_VALUEDATE");
	eline->add("CALC_VALUEDATE", new LADataString(calcvaluestr));


	//curve info
	eline->remove("CURVE_1MFORECAST");
	eline->add("CURVE_1MFORECAST", new LADataString(fcurvestr1m));

	eline->remove("CURVE_3MFORECAST");
	eline->add("CURVE_3MFORECAST", new LADataString(fcurvestr3m));
	
	eline->remove("CURVE_6MFORECAST");
	eline->add("CURVE_6MFORECAST", new LADataString(fcurvestr6m));

	eline->remove("CURVE_OISFORECAST");
	eline->add("CURVE_OISFORECAST", new LADataString(fcurvestrois));

	eline->remove("CURVE_DISCOUNT");
	if (discount.isDefined())
	{
		eline->add("CURVE_DISCOUNT", new LADataString(discount));
	}

	
	// isforward roll
	LAString isforwardstr1 = mapcalcinfo[LAString("TRADE/SWAP/") + PRICING_DATA_ISFORWARDROLL];
	LAString isforwardstr2 = mapcalcinfo[LAString("TRADE/BASISSWAP/") + PRICING_DATA_ISFORWARDROLL];
	LAString isforwardstr3 = mapcalcinfo[LAString("TRADE/FRA/") + PRICING_DATA_ISFORWARDROLL];
	
	eline->remove(LAString("TRADE_SWAP_") + PRICING_DATA_ISFORWARDROLL);
	eline->add(LAString("TRADE_SWAP_") + PRICING_DATA_ISFORWARDROLL, new LADataString(isforwardstr1));

	eline->remove(LAString("TRADE_BASISSWAP_") + PRICING_DATA_ISFORWARDROLL);
	eline->add(LAString("TRADE_BASISSWAP_") + PRICING_DATA_ISFORWARDROLL, new LADataString(isforwardstr2));

	//up to now, fra is always true
	eline->remove(LAString("TRADE_FRA_") + PRICING_DATA_ISFORWARDROLL);
	eline->add(LAString("TRADE_FRA_") + PRICING_DATA_ISFORWARDROLL, new LADataString(isforwardstr3));

	// isforward inter
	LAString isforwardinterstr1 = mapcalcinfo[LAString("TRADE/SWAP/") + PRICING_DATA_ISFWDINTERPOLATION];
	LAString isforwardinterstr2 = mapcalcinfo[LAString("TRADE/BASISSWAP/") + PRICING_DATA_ISFWDINTERPOLATION];
	LAString isforwardinterstr3 = mapcalcinfo[LAString("TRADE/FRA/") + PRICING_DATA_ISFWDINTERPOLATION];

	eline->remove(LAString("TRADE_SWAP_") + PRICING_DATA_ISFWDINTERPOLATION);
	eline->add(LAString("TRADE_SWAP_") + PRICING_DATA_ISFWDINTERPOLATION, new LADataString(isforwardinterstr1));

	eline->remove(LAString("TRADE_BASISSWAP_") + PRICING_DATA_ISFWDINTERPOLATION);
	eline->add(LAString("TRADE_BASISSWAP_") + PRICING_DATA_ISFWDINTERPOLATION, new LADataString(isforwardinterstr2));

	eline->remove(LAString("TRADE_FRA_") + PRICING_DATA_ISFWDINTERPOLATION);
	eline->add(LAString("TRADE_FRA_") + PRICING_DATA_ISFWDINTERPOLATION, new LADataString(isforwardinterstr3));

	// paymenttiming
	LAString paytimingstr1 = mapcalcinfo[LAString("TRADE/SWAP/") + PRICING_DATA_PAYMENTTIMING];
	LAString paytimingstr2 = mapcalcinfo[LAString("TRADE/BASISSWAP/") + PRICING_DATA_PAYMENTTIMING];
	LAString paytimingstr3 = mapcalcinfo[LAString("TRADE/FRA/") + PRICING_DATA_PAYMENTTIMING];
	
	eline->remove(LAString("TRADE_SWAP_") + PRICING_DATA_PAYMENTTIMING);
	eline->add(LAString("TRADE_SWAP_") + PRICING_DATA_PAYMENTTIMING, new LADataString(paytimingstr1));
	
	eline->remove(LAString("TRADE_BASISSWAP_") + PRICING_DATA_PAYMENTTIMING);
	eline->add(LAString("TRADE_BASISSWAP_") + PRICING_DATA_PAYMENTTIMING, new LADataString(paytimingstr2));
	//up to now, fra is always advance payment
	eline->remove(LAString("TRADE_FRA_") + PRICING_DATA_PAYMENTTIMING);
	eline->add(LAString("TRADE_FRA_") + PRICING_DATA_PAYMENTTIMING, new LADataString(paytimingstr3));

	//payment calendar
	LAString paycalstr1 = mapcalcinfo[LAString("TRADE/SWAP/") + CALIBRATION_DATA_CALENDAR];
	LAString paycalstr2 = mapcalcinfo[LAString("TRADE/BASISSWAP/") + CALIBRATION_DATA_CALENDAR];
	LAString paycalstr3 = mapcalcinfo[LAString("TRADE/FRA/") + CALIBRATION_DATA_CALENDAR];
	
	eline->remove(LAString("TRADE_SWAP_") + CALIBRATION_DATA_CALENDAR);
	eline->add(LAString("TRADE_SWAP_") + CALIBRATION_DATA_CALENDAR, new LADataString(paycalstr1));

	eline->remove(LAString("TRADE_BASISSWAP_") + CALIBRATION_DATA_CALENDAR);
	eline->add(LAString("TRADE_BASISSWAP_") + CALIBRATION_DATA_CALENDAR, new LADataString(paycalstr2));

	eline->remove(LAString("TRADE_FRA_") + CALIBRATION_DATA_CALENDAR);
	eline->add(LAString("TRADE_FRA_") + CALIBRATION_DATA_CALENDAR, new LADataString(paycalstr3));

	//payment business days convention
	LAString bdconv1 = mapcalcinfo[LAString("TRADE/SWAP/") + CALIBRATION_DATA_SLIDINGRULE];
	LAString bdconv2 = mapcalcinfo[LAString("TRADE/BASISSWAP/") + CALIBRATION_DATA_SLIDINGRULE];
	LAString bdconv3 = mapcalcinfo[LAString("TRADE/FRA/") + CALIBRATION_DATA_SLIDINGRULE];

	eline->remove(LAString("TRADE_SWAP_") + CALIBRATION_DATA_SLIDINGRULE);
	eline->add(LAString("TRADE_SWAP_") + CALIBRATION_DATA_SLIDINGRULE, new LADataString(bdconv1));

	eline->remove(LAString("TRADE_BASISSWAP_") + CALIBRATION_DATA_SLIDINGRULE);
	eline->add(LAString("TRADE_BASISSWAP_") + CALIBRATION_DATA_SLIDINGRULE, new LADataString(bdconv2));

	eline->remove(LAString("TRADE_FRA_") + CALIBRATION_DATA_SLIDINGRULE);
	eline->add(LAString("TRADE_FRA_") + CALIBRATION_DATA_SLIDINGRULE, new LADataString(bdconv3));

	LAString ccyInput = mapcalcinfo[LAString("CALC/") + IR_CALIBRATION_DATA_CURRENCY];
	//fixing calenar
	LAString fixingcalstr1 = mapcalcinfo[LAString("INDEX/LIBOR/") + PRICING_DATA_FIXINGCALENDAR];
	LAString fixingcalstr2 = mapcalcinfo[LAString("INDEX/OIS/") + PRICING_DATA_FIXINGCALENDAR];

	eline->remove(LAString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_FIXINGCALENDAR);
	eline->add(LAString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_FIXINGCALENDAR,  new LADataString(fixingcalstr1));

	eline->remove(LAString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_FIXINGCALENDAR);
	eline->add(LAString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_FIXINGCALENDAR,  new LADataString(fixingcalstr2));

	//fixing timing
	LAString fixingtimingstr1 = mapcalcinfo[LAString("INDEX/LIBOR/") + PRICING_DATA_FIXINGTIMING];
	LAString fixingtimingstr2 = mapcalcinfo[LAString("INDEX/OIS/") + PRICING_DATA_FIXINGTIMING];

	eline->remove(LAString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_FIXINGTIMING);
	eline->add(LAString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_FIXINGTIMING,  new LADataString(fixingtimingstr1));

	eline->remove(LAString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_FIXINGTIMING);
	eline->add(LAString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_FIXINGTIMING,  new LADataString(fixingtimingstr2));

	//spot lag
	LAString spotlagstr1 = mapcalcinfo[LAString("INDEX/LIBOR/") + PRICING_DATA_SPOTLAG];
	LAString spotlagstr2 = mapcalcinfo[LAString("INDEX/OIS/") + PRICING_DATA_SPOTLAG];

	eline->remove(LAString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_SPOTLAG);
	eline->add(LAString("INDEX_") + ccyInput + "_LIBOR_" + PRICING_DATA_SPOTLAG,  new LADataString(spotlagstr1));

	eline->remove(LAString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_SPOTLAG);
	eline->add(LAString("INDEX_") + ccyInput + "_OIS_" + PRICING_DATA_SPOTLAG,  new LADataString(spotlagstr2));

	//swap frequency
	LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString tmpCurrency;
	tmpCurrency = calcstr; tmpCurrency.toLower();
	LAString swapfreq = irStaticData.getStaticData(tmpCurrency + ".sde.yield.swap.frequencyfloat");
	if(swapfreq == "quarterly")
		swapfreq = "3M";
	else if(swapfreq == "semi-annual")
		swapfreq = "6M";
	else if(swapfreq == "monthly" || swapfreq == "lunar")
		swapfreq = "1M";
	else
		throw LACoreInvalidData("Cannot find swap frequency",__FILE__,__LINE__);

	if(calcstr == "AUD")	//special case!!
		swapfreq = "3M";
	eline->remove("SWAP_FREQUENCY");
	eline->add("SWAP_FREQUENCY",  new LADataString(swapfreq));


}

//in the current case, there exist four curves libor3m libor6m libor1m and ois
//if we need more curve to generate, we have to modify this function
//e.g. xccy curves (arbitrage free case) case, we need more.
LAStringVector
LAMultiSwapPricer::getCurveNamesFromSetUpOrder(LADataInstance* dataInstance, LAString curveID)
{
	LAStringVector ret(4);
	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAString lineparam("lineparam1");
	LAObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();

	LAString swapfreq = dynamic_cast<LADataString &>(eline->getData("SWAP_FREQUENCY", ISNOTNULL).get());

	LAString fcurvestr1m = dynamic_cast<LADataString &>(eline->getData("CURVE_1MFORECAST", ISNOTNULL).get());
	LAString fcurvestr3m = dynamic_cast<LADataString &>(eline->getData("CURVE_3MFORECAST", ISNOTNULL).get());
	LAString fcurvestr6m = dynamic_cast<LADataString &>(eline->getData("CURVE_6MFORECAST", ISNOTNULL).get());
	LAString fcurvestrois = dynamic_cast<LADataString &>(eline->getData("CURVE_OISFORECAST", ISNOTNULL).get());
	LAString fcurvestrstd = dynamic_cast<LADataString &>(eline->getData(LAString("CURVE_") + swapfreq + "FORECAST", ISNOTNULL).get());

	
	LADataHolder* dh;
	dh =&(eline->getData("CURVE_DISCOUNT", NOCHECK));
	LAString discountcurve;
	if (!dh->isDefined() || dh->isNull())
	{
		discountcurve = getDiscountCurveName(dataInstance,curveID);
	}
	else
	{
		discountcurve = dynamic_cast<LADataString &>(eline->getData("CURVE_DISCOUNT", NOCHECK).get());;
	}
	

	//LAString discountcurve = getDiscountCurveName(dataInstance, curveID);
	////set up order 
	bool isoisbase = true;
	if (discountcurve == fcurvestrois)
		isoisbase = true;
	else if (discountcurve == fcurvestrstd)
		isoisbase = false;
	else
		throw LACoreInvalidData("Curve set inconsisntent",__FILE__,__LINE__);

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
LAMultiSwapPricer::createShiftCurves(LADataInstance* dataInstance, LAObject& eorgCurve, LAString targetMarketName, LAString newCurveID, bool isshiftarget, bool ispara, 
											double shiftval, unsigned int shiftpos, LAString market, bool isglobalscenario)
{
	LADataHolder* dh;
	LADataHolder* se;
	LAObjectPool& objPool = dataInstance->getObjectPool();
	
	LAString lineparam("lineparam1");
	LAObject* eline = &objPool.getObject(lineparam, ENCHKTYPE_ISDEFINED).get();

	LAString swapfreq = dynamic_cast<LADataString &>(eline->getData("SWAP_FREQUENCY", ISNOTNULL).get());

	LAString libor1mforecurve = dynamic_cast<LADataString &>(eline->getData("CURVE_1MFORECAST", ISNOTNULL).get());
	LAString libor3mforecurve = dynamic_cast<LADataString &>(eline->getData("CURVE_3MFORECAST", ISNOTNULL).get());
	LAString libor6mforecurve = dynamic_cast<LADataString &>(eline->getData("CURVE_6MFORECAST", ISNOTNULL).get());
	LAString oisforecurve = dynamic_cast<LADataString &>(eline->getData("CURVE_OISFORECAST", ISNOTNULL).get());

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
		LAString genCurveName;
		dh = &(eorgCurve.getData("genCurveName", ISNOTNULL));
		LAString tmp_genCurveName;
		se =&(eline->getData("CURVE_DISCOUNT", NOCHECK));
		if (!se->isDefined() || se->isNull())
		{
			tmp_genCurveName = genCurveName = dynamic_cast<LADataString &>(dh->get());
		}
		else
		{
			tmp_genCurveName = dynamic_cast<LADataString &>(eline->getData("CURVE_DISCOUNT", NOCHECK).get());
		}
		genCurveName = tmp_genCurveName;
		
		LAStringMatrix generateProp,oisRates,oisConv;
		dh = &(eorgCurve.getData("generateProp"));
		if (dh->isDefined() && !dh->isNull())
			generateProp = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("oisRates"));
		if (dh->isDefined() && !dh->isNull())
			oisRates = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("oisConv"));
		if (dh->isDefined() && !dh->isNull())
			oisConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		LAStringMatrix histRates;
		dh = &(eorgCurve.getData("histRates"));
		if (dh->isDefined() && !dh->isNull())
			histRates = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		//change shift rate
		if (isshiftarget)
			LAMultiSwapPricer::shiftMarketRate(oisRates,shiftval,ispara,shiftpos);

		if (isglobalscenario)
			dynamic_cast<LADataStringMatrix &>(eorgCurve.getData("oisRates", ISNOTNULL).get()).set(oisRates);
		
		//LACurveSetup::setUpOISCurve(dataInstance,newCurveID,genCurveName,generateProp,oisRates,oisConv,"", histRates);
		LACurveSetup::setUpOISCurve(dataInstance,newCurveID,genCurveName,generateProp,oisRates,oisConv,targetMarketName, histRates);
	}
//	else if (targetMarketName == libor3mforecurve)
	else if (isbase)
	{
		//mir set up swap;
		LAString genCurveName;
		dh = &(eorgCurve.getData("genCurveName", ISNOTNULL));
		genCurveName = dynamic_cast<LADataString &>(dh->get());

		LAStringMatrix generateProp,moneyConv,liborRates,liborConv,swapRates,swapConv,
			fra3mRates,fra6mRates,fraConv,futureRates,futureConv,adjustSwapConv,adjustSwapRates;
		
		dh = &(eorgCurve.getData("generateProp"));
		if (dh->isDefined() && !dh->isNull())
			generateProp = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("moneyConv"));
		if (dh->isDefined() && !dh->isNull())
			moneyConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("liborRates"));
		if (dh->isDefined() && !dh->isNull())
			liborRates = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("liborConv"));
		if (dh->isDefined() && !dh->isNull())
			liborConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("swapRates"));
		if (dh->isDefined() && !dh->isNull())
			swapRates = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("swapConv"));
		if (dh->isDefined() && !dh->isNull())
			swapConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fra3mRates"));
		if (dh->isDefined() && !dh->isNull())
			fra3mRates = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fra6mRates"));
		if (dh->isDefined() && !dh->isNull())
			fra6mRates = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fraConv"));
		if (dh->isDefined() && !dh->isNull())
			fraConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("futureRates"));
		if (dh->isDefined() && !dh->isNull())
			futureRates = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("futureConv"));
		if (dh->isDefined() && !dh->isNull())
			futureConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("adjustSwapConv"));
		if (dh->isDefined() && !dh->isNull())
			adjustSwapConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("adjustSwapRates"));
		if (dh->isDefined() && !dh->isNull())
			adjustSwapRates = dynamic_cast<LADataStringMatrix &>(dh->get()).get();


		//change shift rate
		if (!liborRates.empty() && isshiftarget && market == "ALL")
		{
			LAMultiSwapPricer::shiftMarketRate(liborRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<LADataStringMatrix &>(eorgCurve.getData("liborRates").get()).set(liborRates);
		}
		else if (!liborRates.empty() && isshiftarget && market == "liborRates")
			LAMultiSwapPricer::shiftMarketRate(liborRates,shiftval,ispara,shiftpos);

		if (!swapRates.empty() && isshiftarget && market == "ALL")
		{
			LAMultiSwapPricer::shiftMarketRate(swapRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<LADataStringMatrix &>(eorgCurve.getData("swapRates").get()).set(swapRates);
		}
		else if (!swapRates.empty() && isshiftarget && market == "swapRates")
			LAMultiSwapPricer::shiftMarketRate(swapRates,shiftval,ispara,shiftpos);

		if (!fra3mRates.empty() && isshiftarget && market == "ALL")
		{
			LAMultiSwapPricer::shiftMarketRate(fra3mRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<LADataStringMatrix &>(eorgCurve.getData("fra3mRates").get()).set(fra3mRates);
		
		}
		else if (!fra3mRates.empty() && isshiftarget && market == "fra3mRates")
			LAMultiSwapPricer::shiftMarketRate(fra3mRates,shiftval,ispara,shiftpos);

		if (!fra6mRates.empty() && isshiftarget && market == "ALL")
		{
			LAMultiSwapPricer::shiftMarketRate(fra6mRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<LADataStringMatrix &>(eorgCurve.getData("fra6mRates").get()).set(fra6mRates);

		}
		else if (!fra6mRates.empty() && isshiftarget && market == "fra6mRates")
			LAMultiSwapPricer::shiftMarketRate(fra6mRates,shiftval,ispara,shiftpos);

		if (!futureRates.empty() && isshiftarget && market == "ALL")
		{
			LAMultiSwapPricer::shiftMarketRate(futureRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<LADataStringMatrix &>(eorgCurve.getData("futureRates").get()).set(futureRates);

		}
		else if (!futureRates.empty() && isshiftarget && market == "futureRates")
			LAMultiSwapPricer::shiftMarketRate(futureRates,shiftval,ispara,shiftpos);
		
		if (!adjustSwapRates.empty() && isshiftarget && market == "ALL")
		{
			LAMultiSwapPricer::shiftMarketRate(adjustSwapRates,shiftval,ispara,shiftpos);
			if (isglobalscenario)
				dynamic_cast<LADataStringMatrix &>(eorgCurve.getData("adjustSwapRates").get()).set(adjustSwapRates);
		}
		else if (!adjustSwapRates.empty() && isshiftarget && market == "adjustSwapRates")
			LAMultiSwapPricer::shiftMarketRate(adjustSwapRates,shiftval,ispara,shiftpos);

		LACurveSetup::setUpSwapCurve(dataInstance,newCurveID,genCurveName,generateProp,
								moneyConv,liborRates,liborConv,swapRates,swapConv,
								fra3mRates,fra6mRates,fraConv,futureRates,futureConv,
								//adjustSwapConv,adjustSwapRates,"","");
								adjustSwapConv,adjustSwapRates,targetMarketName,"");

	}
//	else if (targetMarketName == libor1mforecurve || targetMarketName == libor6mforecurve)
	else if (isnobase)
	{
		//mir set up basis swap
		LAString basisCurveName;
		dh = &(eorgCurve.getData("basisCurveName", ISNOTNULL));
		basisCurveName = dynamic_cast<LADataString &>(dh->get());

		LAStringMatrix basisMkt,basisConv,fwdFX,fwdConv,spotFX,generateProp,moneyConv;
		
		dh = &(eorgCurve.getData("basisMkt"));
		if (dh->isDefined() && !dh->isNull())
			basisMkt = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("basisConv"));
		if (dh->isDefined() && !dh->isNull())
			basisConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fwdFX"));
		if (dh->isDefined() && !dh->isNull())
			fwdFX = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("fwdConv"));
		if (dh->isDefined() && !dh->isNull())
			fwdConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("spotFX"));
		if (dh->isDefined() && !dh->isNull())
			spotFX = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("generateProp"));
		if (dh->isDefined() && !dh->isNull())
			generateProp = dynamic_cast<LADataStringMatrix &>(dh->get()).get();

		dh = &(eorgCurve.getData("moneyConv"));
		if (dh->isDefined() && !dh->isNull())
			moneyConv = dynamic_cast<LADataStringMatrix &>(dh->get()).get();


		//change shift rate
		if (isshiftarget && market == "ALL")
			LAMultiSwapPricer::shiftMarketRate(basisMkt,shiftval,ispara,shiftpos);
		else if (isshiftarget && market == "basisMkt")
			LAMultiSwapPricer::shiftMarketRate(basisMkt,shiftval,ispara,shiftpos);

		//LACurveSetup::setUpBasisCurve(dataInstance,newCurveID,basisCurveName,basisMkt,basisConv,generateProp,moneyConv,"");
		LACurveSetup::setUpBasisCurve(dataInstance,newCurveID,basisCurveName,basisMkt,basisConv,fwdFX,fwdConv,spotFX,generateProp,moneyConv,targetMarketName);
	}
	else
		throw LACoreInvalidData("Source Delta Scenario Error",__FILE__,__LINE__);


	return;
}
