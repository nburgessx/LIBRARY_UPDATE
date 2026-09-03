#define _HAS_STD_BYTE 0

/*! @file
    @brief M-Library Deal util class
*/
//  2007, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MADealUtils.cpp
//
//  DESCRIPTION :       Deal util class
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


#include "LADealUtils.h"
#include "LADataInstance.h"
#include "LACoreTemplateType.h"
#include "LADataValuation.h"
#include "LADataBasics.h"
#include "LADataMultiReference.h"
#include "LAPricePortfolioValue.h"
#include "LAPricePayOff.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAPriceTradeValue.h"
#include "LADefinitions.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAStaticData.h"
#include "LAFileAccessor.h"
#include "LALogManager.h"
#include "LALogger.h"
#include "LADefinitionsCalibration.h"
#include "LAMarketData.h"
#include "LALinearRatesOptionValue.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LALinearRatesSwapTradeValue.h"
#include "LAMathIndexEntity.h"
#include "LAPriceIRSwaptionValueFromCashFlow.h"
#include "LAMathInterpolationUtilities.h"
#include "LAMathDateCalculations.h"
#include "LAPriceConvergenceValue.h"
#include "LADefinitionsIRSABR.h"
#include "LAPriceIRCapFloorOptionValue.h"
//#include "LAPriceNDSSwaptionValue.h"
#include <algorithm>
#include <cmath>

using namespace std;

// constructor
/*!

*/
MADealUtils::MADealUtils(void)
{
}

// destructor
/*!

*/
MADealUtils::~MADealUtils(void)
{
}


// 
/*!
    @brief get sde currencys

	@return LAStringVector
*/
LAStringVector 
MADealUtils::getSDECurrencys(const bool isIncludeVol)
{
	// get currencys
	LAStringVector ret;
	LAString ccys = LACoreDataService::getContext(ARG_KEY_CURRENCY);
	ret = ccys.toToken(':');
	if (isIncludeVol)
	{
		addStochasticVolIndex(ret);
	}
	return ret;
}

// 
/*!
    @brief get simulationsde currencys

	@return LAStringVector
*/
LAStringVector 
MADealUtils::getSimulationSDECurrencys(const bool isIncludeVol)
{
	// get currencys
	LAStringVector ret;
	LAString ccys = LACoreDataService::getContext(ARG_KEY_SIMCURRENCY);
	if (ccys != MLIB_NO_DATA)
	{
		ret = ccys.toToken(':');
		if (isIncludeVol)
		{
			addStochasticVolIndex(ret);
		}
	}
	return ret;
}


// 
/*!
    @brief get sde currencys all

	@return LAStringVector
*/
LAStringVector 
MADealUtils::getSDECurrencysAll()
{
	// get currencys
	LAStringVector ret;
	LAString ccys = LACoreDataService::getContext(ARG_KEY_CURRENCY);
	if (ccys != MLIB_NO_DATA)
	{
		LAStringVector irvec, fxvec;
		ret = ccys.toToken(':');
		for (unsigned int i = 0; i < ret.size(); i++)
		{
			if (ret[i].findString(FX_DELIMITER) < 0)
				irvec.push_back(ret[i]);
			else
			{
				fxvec.push_back(ret[i]);
				
			}
		}


		if (irvec.size() < 2)
			return ret;

		LAStringVector addvec;
		for (unsigned int i = 0; i < irvec.size()-1; i++)
		{
			for (unsigned int j = i+1; j < irvec.size(); j++)
			{
				LAString chkccy1 = irvec[i] + '/' + irvec[j];
				if (std::find(fxvec.begin(), fxvec.end(), chkccy1) == fxvec.end())
				{
					addvec.push_back(chkccy1);
				}
				LAString chkccy2 = irvec[j] + '/' + irvec[i];
				if (std::find(fxvec.begin(), fxvec.end(), chkccy2) == fxvec.end())
				{
					addvec.push_back(chkccy2);
				}

			}
		}
		
		ret.insert(ret.end(),addvec.begin(),addvec.end());
		
	}

	return ret;
}

// 
/*!
    @brief get sde currencys all

	@return LAStringVector
*/
LAStringVector 
MADealUtils::getAllSingleCurrencys()
{
	// get currencys
	LAStringVector ret;
	LAString ccys = LACoreDataService::getContext(ARG_KEY_CURRENCY);
	if (ccys != MLIB_NO_DATA)
	{
		//LAStringVector irvec, fxvec;
		LAStringVector sdeccys = ccys.toToken(':');
		for (unsigned int i = 0; i < sdeccys.size(); i++)
		{
			if (sdeccys[i].findString(FX_DELIMITER) < 0)
				ret.push_back(sdeccys[i]);
			else
			{
				LAStringVector tmp = sdeccys[i].toToken(FX_DELIMITER);
				if (std::find(ret.begin(),ret.end(),tmp[0]) == ret.end())
					ret.push_back(tmp[0]);

				if (std::find(ret.begin(),ret.end(),tmp[1]) == ret.end())
					ret.push_back(tmp[1]);
			}
		}
	}	

	return ret;
}





// 
/*!
    @brief get cms max years

	@param[in] ref
	@return int
*/
int
MADealUtils::getCMSYears(const LADataMultiReference &ref, const LAString &indexAttr)
{
	int ret = 0;
	const int refSize = ref.getSize();
	// reference loop
	for (int i = 0; i < refSize; ++i)
	{
		const LADataMultiReference &indexes = dynamic_cast<const LADataMultiReference &>
												(ref.get(i).getData(indexAttr, ISNOTNULL).get());
		const int indexSize = indexes.getSize();
		// index info loop
		for(int j = 0; j < indexSize; ++j)
		{
			LAString indexType = dynamic_cast<const LADataString &>
									(indexes.get(j).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get();
			if (indexType.toUpper() == "CMS")
			{
				LAString accessary = dynamic_cast<const LADataString &>
										(indexes.get(j).getData(PRICING_DATA_ACCESSORY, ISNOTNULL).get());
				LAString suffix = accessary.subString(accessary.size() - 1, accessary.size() - 1);

				if (suffix.toUpper() == "Y")
				{
					int years = accessary.subString(0, accessary.size() - 2).getIntValue();
					if (ret < years)
					{
						ret = years;
					}
				}
				else
				{
					if (ret < 1)
					{
						ret = 1;
					}
				}
			}
		}
	}
	return ret;
}
// 
/*!
    @brief get max term

	@param[in] objPool
	@param[in] asOfDate
	@return int
*/
int
MADealUtils::getMaxTerm(LAObjectPool &objPool, const LADate &asOfDate, LAString tradetype)
{

	LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);



	try
	{
		// calc maxterm
		// if fail return MAX_TERM
		if (dynamic_cast<const LADataValuation &>
			(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
		{
			int maxTerm = 0;
			const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
												(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
			const int tradeSize = unders.getSize();
			for (int i = 0; i < tradeSize; ++i)
			{
				int tmpTerm = 0;
				if (tradetype == LAString("EXODERIVA"))
				{
					tmpTerm = getMaxTerm(unders.get(i), asOfDate);
				}
				else if (tradetype == LAString("VANILLA"))
				{
					tmpTerm = getMaxTermFromPlainVanilla(unders.get(i),asOfDate);
				}
				else 
				throw LACoreInvalidData("MaxTermError",__FILE__,__LINE__);

				if (tmpTerm > maxTerm)
				{
					maxTerm = tmpTerm;
				}
			}
			return maxTerm;		
		}
		else
		{
			if (tradetype == LAString("EXODERIVA"))
			{
				return getMaxTerm(objHolder, asOfDate);
			}
			else if (tradetype == LAString("VANILLA"))
			{
				return getMaxTermFromPlainVanilla(objHolder, asOfDate);
			}
			else 
				throw LACoreInvalidData("MaxTermError",__FILE__,__LINE__);
		}
	}
	catch(LACoreError &e)
	{
		e.print();
		MALogger &logger = LACoreDataService::getLogManager().getLogger();
		logger.info(" calc term is fail term max value", __FILE__, __LINE__);
		logger.info(e.getMsg(), __FILE__, __LINE__);
		return MAX_TERM;			
	}
	catch(...)
	{
		MALogger &logger = LACoreDataService::getLogManager().getLogger();
		logger.info(" calc term is fail term max value", __FILE__, __LINE__);
		return MAX_TERM;			
	}
}

// 
/*!
    @brief get one deal max term

	@return int
*/
int
MADealUtils::getMaxTerm(const LAObjectHolder &objHolder, const LADate &asOfDate)
{
	// max date
	LADate maxDate;
	maxDate.setSystemDate();
	int addYears = 0;

	const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
										(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());

	const int legSize = unders.getSize();
	for (int i = 0; i < legSize; ++i)
	{
		const LAObjectHolder &leg = unders.get(i);
		
		LAString inputType = dynamic_cast<const LADataString &>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
		const LADataHolder &attrEndDate = leg.getData(PRICING_DATA_ENDDATE, NOCHECK);
		if (attrEndDate.isDefined() && !attrEndDate.isNull())
		{
			// end date
			const LADate &endDate = dynamic_cast<const LADataDate &>(attrEndDate.get()).get();
			if (endDate > maxDate)
			{
				maxDate = endDate;
			}
		}
		else
		{
			// if end date is null, endterm is indispensable
			int endTerm = leg.getData(PRICING_DATA_ENDTERM, ISNOTNULL).convertToString().getIntValue();
			LADate date =  dynamic_cast<const LADataDate &>(leg.getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
			date.addYears(endTerm);
			if ( date > maxDate)
			{
				maxDate = date;
			}
		}

		if (inputType.toUpper() == "MANUAL")
		{
			const LADataHolder &attrCashlets = leg.getData(PRICING_DATA_CASHLETS, NOCHECK);
			if (attrCashlets.isDefined() && !attrCashlets.isNull())
			{
				const LADataMultiReference &cashlets = dynamic_cast<const LADataMultiReference &>(attrCashlets.get());
				const int cashletSize = cashlets.getSize();
				// cashlet loop
				for (int j = 0; j < cashletSize; ++j)
				{
					const LADataHolder &attrCoupons = cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
					if (attrCoupons.isDefined() && !attrCoupons.isNull())
					{
						const LADataMultiReference &coupons = dynamic_cast<const LADataMultiReference &>(attrCoupons.get());
						int tmp = getCMSYears(coupons, PRICING_DATA_INDEXINFOS);
						if (addYears < tmp)
						{
							addYears = tmp;
						}
					}
					const LADataHolder &attrRangeInfos = cashlets.get(j).getData(PRICING_DATA_RANGEACCRUEINFOS, NOCHECK);
					if (attrRangeInfos.isDefined() && !attrRangeInfos.isNull())
					{
						const LADataMultiReference &rangeinfos = dynamic_cast<const LADataMultiReference &>(attrRangeInfos.get());
						int tmp = getCMSYears(rangeinfos, PRICING_DATA_RANGEACCRUEINDEXINFOS);
						if (addYears < tmp)
						{
							addYears = tmp;
						}
					}
				}
			}
		}
		else
		{
			const LADataMultiReference &coupons = dynamic_cast<const LADataMultiReference &>
										(leg.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL).get());
			int tmp = getCMSYears(coupons, PRICING_DATA_INDEXINFOS);
			if (addYears < tmp)
			{
				addYears = tmp;
			}
			const LADataHolder &attrRangeInfos = leg.getData(PRICING_DATA_RANGEACCRUEINFOS, NOCHECK);
			if (attrRangeInfos.isDefined() && !attrRangeInfos.isNull())
			{
				const LADataMultiReference &rangeinfos = dynamic_cast<const LADataMultiReference &>(attrRangeInfos.get());
				int tmp = getCMSYears(rangeinfos, PRICING_DATA_RANGEACCRUEINDEXINFOS);
				if (addYears < tmp)
				{
					addYears = tmp;
				}
			}
		}
	}
	// calc maxterm
	int years = asOfDate.intervalYears(maxDate);
	return (years < 1) ? 1 : years + addYears;

}

// 
/*!
    @brief get one deal max term from fx vanilla option

	@return int
*/
int
MADealUtils::getMaxTermFromPlainVanilla(const LAObjectHolder &objHolder, const LADate &asOfDate)
{
	// max date
	LADate maxDate;
	maxDate.setSystemDate();
	//int addYears = 0;
	int addYears = 1;

	const LADataHolder* dh;
	const LAObject& e = objHolder.get();
	const LADataValuation& attrval = dynamic_cast<const LADataValuation& >(e.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
	const LACoreValuation& val = attrval.getMethod();
	//if option
	if (val.isTypeOf(FN_PLAINVANILLAVALUE))
	{
		const LALinearRatesOptionValue & val2 = dynamic_cast<const LALinearRatesOptionValue &>(val);
		const LADate& expirydate = val2.getMaturityDate(e,NULL);
		const LADate& deliverydate = val2.getDeliveryDate(e,NULL);
		maxDate = (expirydate > deliverydate) ? expirydate : deliverydate;
	}
	else if(val.isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE))
	{
		return MADealUtils::getMaxTerm(objHolder, asOfDate) + addYears;
	}

	// calc maxterm
	int years = asOfDate.intervalYears(maxDate);
	return (years < 1) ? 1 : years + addYears;

}


// 
/*!
    @brief get applied maturity

	@return LAString
*/
LAString 
MADealUtils::getAppMat()
{
	LAString appMat = LACoreDataService::getContext(ARG_KEY_APPMAT);

	if (appMat == MLIB_NO_DATA)
	{
		return STANDARD
	}

	return appMat.toLower();
}

// 
/*!
    @brief get ir volatility grids which are used for calculation

	@return LAStringVector
*/
BoolMatrix 
MADealUtils::getCalibTargetIRVolGrids(LAObjectPool &objPool, 
									  const LADate &asOfDate,
									  const LAString& ccy,
									  const LAString& underlying,
									  const bool isPropSource)
{
	LAString ccy_small =ccy;
	ccy_small.toLower();
	//set daycount
	LAPriceDataDayCount act365ISDA(ACT_365_ISDA);

	//get calib property
	LAStaticData &calibProp = LACoreDataService::getStaticDataManager().getCalibStaticData();

	//expiry string and expiry vector
	// optionmaturity
	LAStringVector expiryVec_str = calibProp.getStaticData(ccy_small + STATIC_DATA_KEY_CALIB_IRSABR_OPTIONMATURITY + "." + underlying).toToken(MULTI_STATIC_DATA_DELIMITER);
	DoubleVector expiryVec(expiryVec_str.size());
	//sliding rule
	LAString sr_str = calibProp.getStaticData(ccy_small + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONSLIDINGRULE + "." + underlying).toUpper();
	LAPriceDataSlidingRule sr;
	sr.convertFromString(sr_str);
	//swaption fixing calendar
	LAString cal_str = calibProp.getStaticData(ccy_small + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFIXINGCALENDAR + "." + underlying);
	LAPriceDataCalendar cal;
	cal.convertFromString(cal_str);
	for (size_t i = 0; i < expiryVec_str.size(); ++i)
	{
		LADate toDate = LAMathDateCalculations::getDate(asOfDate, expiryVec_str[i], sr, &cal,true);
		expiryVec[i] = act365ISDA.getTerm(asOfDate, toDate, true);
	}
	// tenor string and tenor vector
	LAStringVector tenorVec_str = calibProp.getStaticData(ccy_small + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTENOR + "." + underlying).toToken(MULTI_STATIC_DATA_DELIMITER);
	DoubleVector tenorVec(tenorVec_str.size());
	for (unsigned int i = 0; i < tenorVec_str.size(); i++)
	{
		int y,m,d,w;
		LAMathDateCalculations::termStrtoYMDW(tenorVec_str[i], y, m, d, w);
		tenorVec[i] = static_cast<double > (y) + static_cast<double > (m) / 12;
	}

	if (expiryVec.size() == 0)
		throw LACoreInvalidData("Expiry dates has nod ata!",__FILE__,__LINE__);

	if (tenorVec.size() == 0)
		throw LACoreInvalidData("Expiry dates has nod ata!",__FILE__,__LINE__);

	BoolVector tmp(tenorVec.size(), false);
	BoolMatrix ret(expiryVec.size(), tmp);

	if (isPropSource)
	{
		const LAString fileName = LAMarketData::getNumFileName(calibProp.getStaticData(ccy_small + STATIC_DATA_KEY_CALIB_IRSABR_TARGETVOLGRID_FILE + "." + underlying), MLIBID);
		MAFileAccessor file(fileName);
		LAStringMatrix dataMtx;
		file.readAllData(MARKET_DATA_DELIMITER, dataMtx);
		file.close();

		for (unsigned int i = 0; i < dataMtx.size(); ++i)
		{
			if (dataMtx[i].size() != 2)
			{
				throw LACoreInvalidData("IRSABR ReductionInfo file format is wrong", __FILE__, __LINE__);
			}
			const LAString expiryDate = dataMtx[i][0].toUpper();
			const double expiryTerm = LAMath::max(act365ISDA.getTerm(asOfDate, LADate(expiryDate.getCString())), 0.0);
			LAStringVector tenorInfos = dataMtx[i][1].toToken(':');
			if (tenorInfos.size() == 1)
			{
				const LAString tenor = tenorInfos[0].toUpper();
				int y, m, d, w;
				LAMathDateCalculations::termStrtoYMDW(tenor, y, m, d, w);
				const double tenor_d = static_cast<double>(y) + static_cast<double>(m) / 12;
				addCalibTargetFlag(ret, expiryTerm, tenor_d, expiryVec, tenorVec);
			}
			else if (tenorInfos.size() == 3)
			{
				LAPriceIRSwaptionValueFromCashFlow val_swaption;
				const LADate startDate(tenorInfos[0].getCString());
				const LADate endDate(tenorInfos[1].getCString());
				LAObject entityInfo;
				entityInfo.add(PRICING_DATA_STARTDATE, new LADataDate(startDate));
				entityInfo.add(PRICING_DATA_ENDDATE, new LADataDate(endDate));

				//get frequency
				const LAString accessory = tenorInfos[2].toUpper();
				const LAString frequency = LAMarketData::getFrequency(accessory);
				//get tenor
				const LAString tenor = val_swaption.getNearestTenorString(entityInfo, frequency);
				int y, m, d, w;
				LAMathDateCalculations::termStrtoYMDW(tenor, y, m, d, w);
				const double tenor_d = static_cast<double>(y) + static_cast<double>(m) / 12;
				addCalibTargetFlag(ret, expiryTerm, tenor_d, expiryVec, tenorVec);
			}
			else
			{
				LAString msg = "TenorInfos format is wrong";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

		}		
	}
	else
	{
		LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
		LAObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);

		std::vector<LAObject* > tradeEntities;
		if (dynamic_cast<const LADataValuation &>
			(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
		{
			int maxTerm = 0;
			const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
												(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
			const int tradeSize = unders.getSize();
			for (int i = 0; i < tradeSize; ++i)
			{
				tradeEntities.push_back(&unders.get(i).get());
			}
		}
		else
		{
			tradeEntities.push_back(&objHolder.get());
		}

		LAString underlying_calib = underlying;
		underlying_calib.toLower();
		for (int i = 0; i < tradeEntities.size(); ++i)
		{
			const LADataHolder* dh;
			const LADataValuation& attrval = dynamic_cast<const LADataValuation& >(tradeEntities[i]->getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
			const LACoreValuation* pVal = &attrval.getMethod();
			// if value function is convergence value, we get sub value
			if (pVal->isTypeOf(FN_IR_CONVERGENCEVALUE))
			{
				const LADataValuation& attrval = dynamic_cast<const LADataValuation& >(tradeEntities[i]->getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
				pVal = &attrval.getMethod();
			}

			double expiryTerm;
			LAString tenor = "";
			if (pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) || pVal->isTypeOf(FN_IR_CAPFLOOROPTIONVALUE))
			{
				if(pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE)){
					const LALinearRatesSwapTradeValue* swap_trade_value = dynamic_cast<const LALinearRatesSwapTradeValue*>(pVal);
					if(!swap_trade_value->hasCashflow(*tradeEntities[i])) continue;
				}
				if(pVal->isTypeOf(FN_IR_CAPFLOOROPTIONVALUE)){
					const LAPriceIRCapFloorOptionValue* capfloor_value = dynamic_cast<const LAPriceIRCapFloorOptionValue*>(pVal);
					if(!capfloor_value->hasCashflow(*tradeEntities[i])) continue;
				}

				const LADataMultiReference &legs = dynamic_cast<const LADataMultiReference &>(tradeEntities[i]->getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
				for (size_t i_leg = 0; i_leg < legs.getSize(); ++i_leg)
				{
					const LADataMultiReference &cashlets = dynamic_cast<const LADataMultiReference &>(legs.get(i_leg).getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
					for (size_t i_chashlet = 0; i_chashlet < cashlets.getSize(); ++i_chashlet)
					{
						dh = &cashlets.get(i_chashlet).getData(PRICING_DATA_COUPONINFOS, NOCHECK);
						if (dh->isDefined() && !dh->isNull())
						{
							const LADataMultiReference &coupons = dynamic_cast<const LADataMultiReference &>(dh->get());
							for (size_t i_cpn = 0; i_cpn < coupons.getSize(); ++i_cpn)
							{
								dh = &coupons.get(i_cpn).getData(PRICING_DATA_INDEXINFOS, NOCHECK);
								if (dh->isDefined() && !dh->isNull())
								{
									const LADataMultiReference &indexs = dynamic_cast<const LADataMultiReference &>(dh->get());
									for (size_t i_index = 0; i_index < indexs.getSize(); ++i_index)
									{
										//check currency
										dh = &indexs.get(i_index).getData(PRICING_DATA_CURRENCY, NOCHECK);
										if (!dh->isDefined() || dh->isNull()) continue;
										LAString ccy_index = dynamic_cast<const LADataString &> (dh->get()).get();
										ccy_index.toLower();
										if (ccy_index != ccy_small) continue;
										//check index type
										LAString indexType = dynamic_cast<const LADataString &>(indexs.get(i_index).getData(IR_MODEL_DATA_INDEXTYPE, ISNOTNULL).get()).get();
										indexType.toUpper();

										bool isDelayedConvexityAdjusted = false;
										dh = &(indexs.get(i_index).getData(PRICING_DATA_CONVEXITYADJUSTMENT, NOCHECK));
										if (dh->isDefined() && !dh->isNull())
										{
											const LAString& caModel = dynamic_cast<const LADataString&>(dh->get()).get();
											isDelayedConvexityAdjusted = LAMathIndexEntity::isDelayedConvexityAdjustModel(caModel) && (indexType == LIBOR);
										}

										if (pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) && indexType != CMS && !isDelayedConvexityAdjusted) continue;
										if (pVal->isTypeOf(FN_IR_CAPFLOOROPTIONVALUE) && indexType != CMS && indexType != LIBOR) continue;
										//check underlying
										LAString underlying_index = dynamic_cast<const LADataString &>(indexs.get(i_index).getData(PRICING_DATA_VOLATILITYUNDERLYING, ISNOTNULL).get()).get();
										underlying_index.toLower();
										if (underlying_calib != underlying_index) continue;
										//get expiry term
										/*if (pVal->isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE))
										{
											const LAPriceDataCalendar& cal = dynamic_cast<const LAPriceDataCalendar &>(indexs.get(i_index).getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
											int spotLag = dynamic_cast<const LADataInt &>(indexs.get(i_index).getData(PRICING_DATA_SPOTLAG, ISNOTNULL).get()).get();
											const LADate& fixingDate = dynamic_cast<const LADataDate &>(indexs.get(i_index).getData(PRICING_DATA_FIXINGDATE, ISNOTNULL).get()).get();
											LADate spotDate = cal.getBusinessDay(fixingDate, spotLag);
											expiryTerm = act365ISDA.getTerm(asOfDate, spotDate);
										}
										else if (pVal->isTypeOf(FN_IR_CAPFLOOROPTIONVALUE))
										{*/
										const LADate& fixingDate = dynamic_cast<const LADataDate &>(indexs.get(i_index).getData(PRICING_DATA_FIXINGDATE, ISNOTNULL).get()).get();
										expiryTerm = act365ISDA.getTerm(asOfDate, fixingDate);
										//}

										if (expiryTerm < 0.) expiryTerm = 0.;
										//get tenor
										tenor = dynamic_cast<const LADataString &>(indexs.get(i_index).getData(PRICING_DATA_ACCESSORY, ISNOTNULL).get()).get();
										int y,m,d,w;
										LAMathDateCalculations::termStrtoYMDW(tenor, y, m, d, w);
										double tenor_d = static_cast<double > (y) + static_cast<double > (m) / 12;
									
										addCalibTargetFlag(ret, expiryTerm, tenor_d, expiryVec, tenorVec);

									}
								}
							}
						}
					}
				}
			}
			else if (pVal->isTypeOf(FN_IR_SWAPTIONVALUEFROMCASHFLOW))
			{
				//get leg
				const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference &>(tradeEntities[i]->getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
				if (legs.getSize() != 2)
					throw LACoreInvalidData("Swaption Underlyings error",__FILE__,__LINE__);
				const LAPriceIRSwaptionValueFromCashFlow& val_swaption = dynamic_cast<const LAPriceIRSwaptionValueFromCashFlow &> (*pVal);
				if(!val_swaption.hasCashflow(*tradeEntities[i])) continue;
                
				//check currency
				LAString ccy_index = dynamic_cast<const LADataString &>(legs.get(0).get().getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
				ccy_index.toLower();
				if (ccy_index != ccy_small) continue;
				//check underlying
				LAString underlying_trade = dynamic_cast<const LADataString &>(tradeEntities[i]->getData(PRICING_DATA_VOLATILITYUNDERLYING, ISNOTNULL).get()).get();
				underlying_trade.toLower();
				if (underlying_calib != underlying_trade) continue;

				const LADate& maturityDate = dynamic_cast<const LADataDate &>(tradeEntities[i]->getData(PRICING_DATA_EXPIRYDATE, ISNOTNULL).get()).get();
				expiryTerm = act365ISDA.getTerm(asOfDate, maturityDate);


				//get float leg number
				unsigned int floatLegNum = val_swaption.getFloatLegNum(*tradeEntities[i]);
				//get float leg object
				LAObject& floatleg = legs.get(floatLegNum).get();
				//get frequency
				LAString frequency = val_swaption.getFrequencyFromIndexGenerator(*tradeEntities[i], floatLegNum);
				//get tenor
				tenor = val_swaption.getNearestTenorString(floatleg, frequency);
				int y,m,d,w;
				LAMathDateCalculations::termStrtoYMDW(tenor, y, m, d, w);
				double tenor_d = static_cast<double > (y) + static_cast<double > (m) / 12;

				expiryTerm = LAMath::max(expiryTerm, 0.0);
				tenor_d = LAMath::max(tenor_d, 0.0);
				addCalibTargetFlag(ret, expiryTerm, tenor_d, expiryVec, tenorVec);
			}
//#ifndef RH6
//			else if(pVal->isTypeOf(FN_IR_NDSSWAPTIONVALUE))
//			{
//				DoubleVector expiryYears;
//				DoubleVector tenorYears;
//				LAPriceNDSSwaptionValue::getVolCalibrationGrid(asOfDate, *tradeEntities[i], expiryYears, tenorYears);
//				for (auto i_grid = 0; i_grid < expiryYears.size(); ++i_grid) {
//					addCalibTargetFlag(ret, expiryYears[i_grid], tenorYears[i_grid], expiryVec, tenorVec);
//				}
//			}
//#endif
			else
			{
				throw LACoreInvalidData(pVal->getType() + " does not use swaption volatility matrix!",__FILE__,__LINE__);
			}
		}
	}

	return ret;
}

void 
MADealUtils::addCalibTargetFlag( BoolMatrix& calibTaretMat,
								 double expiry, 
								 double tenor, 
								 const DoubleVector& expiryVec,
								 const DoubleVector& tenorVec)
{
	if(expiry<0. || tenor<0.) 
	{
		throw LACoreInvalidData("expiry or tenor is negative!",__FILE__,__LINE__);
	}

    size_t size_te = tenorVec.size();
    size_t size_ex = expiryVec.size();

	unsigned int index_te;
	if( tenorVec.front() >= tenor )
	{
		index_te = 0;
	}
	else if( tenorVec.back() <= tenor )
	{
		index_te = size_te;
	}
	else
	{
		index_te = LAMathInterpolationUtilities::searchIndex(tenorVec, tenor);
	}

	unsigned int index_ex;
	if( expiryVec.front()>=expiry )
	{
		index_ex = 0;
	}
	else if( expiryVec.back() <= expiry )
	{
		index_ex = size_ex;
	}
	else
	{
		index_ex = LAMathInterpolationUtilities::searchIndex(expiryVec, expiry);
	}

	if( index_ex == 0 && index_te == 0 )
	{
		calibTaretMat[0][0] = true;
	}
	else if( index_ex == size_ex && index_te == 0 )
	{
		calibTaretMat[size_ex-1][0] = true;
	}
	else if( index_ex == size_ex && index_te == size_te)
	{
		calibTaretMat[size_ex-1][size_te-1] = true;
	}
	else if( index_ex == 0 && index_te == size_te)
	{
		calibTaretMat[0][size_te-1] = true;
	}
	else if( index_ex == 0 )
	{
		calibTaretMat[0][index_te-1] = true;
		calibTaretMat[0][index_te] = true;
	}
	else if(index_ex == size_ex) 
	{
		calibTaretMat[size_ex-1][index_te-1] = true;
		calibTaretMat[size_ex-1][index_te] = true;
	}
	else if(index_te == 0) 
	{
		calibTaretMat[index_ex-1][0] = true;
		calibTaretMat[index_ex][0] = true;
	}
	else if(index_te == size_te) 
	{
		calibTaretMat[index_ex-1][size_te-1] = true;
		calibTaretMat[index_ex][size_te-1] = true;
	}
	else
	{
		calibTaretMat[index_ex-1][index_te-1] = true;
		calibTaretMat[index_ex-1][index_te] = true;
		calibTaretMat[index_ex][index_te-1] = true;
		calibTaretMat[index_ex][index_te] = true;
	}
}

void 
MADealUtils::addStochasticVolIndex(LAStringVector& ccys)
{
	unsigned int initialsize = ccys.size();
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	for (int i = 0; i < initialsize; ++i)
	{
		LAString ccy(ccys[i]);
		LAString model(LAMarketData::getModelName(ccys[i]));
		if(staticData.getStaticData(ccy.toLower() + ".sde." + model.toLower() + ".volatility.isstochastic").toUpper() == "TRUE")
		{
			ccys.push_back(ccys[i] + POSTFIX_VOL);
		}
	}
}

