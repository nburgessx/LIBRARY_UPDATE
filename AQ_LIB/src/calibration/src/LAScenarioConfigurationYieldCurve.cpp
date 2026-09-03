/*! @file
    @brief Yield Risk Scenario create class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationYieldCurve.cpp
//
//  DESCRIPTION :       Yield Risk Scenario create class
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
// minimum rate
#define MIN_RATE 0.0000001 //0.001bp

#include "LADataInstance.h"
#include "LACoreReferencePool.h"
#include "LAObjectPool.h"
#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataProcedure.h"
#include "LADataMultiReference.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LAScenarioConfigurationYieldCurve.h"
#include "LAMarketData.h"
#include "LABasic.h"
#include "LAPriceYieldGenerator.h"
#include "LAPriceArbFreeGenerator.h"

using namespace std;

#ifndef SWAP
#define SWAP	"SWAP"
#endif

// constructor
/*!

*/
LAScenarioConfigurationYieldCurve::LAScenarioConfigurationYieldCurve(void)
{
}

// destructor
/*!

*/
LAScenarioConfigurationYieldCurve::~LAScenarioConfigurationYieldCurve(void)
{
}


 
/*!
    @brief Risk Scenario create method

	@param[int,out] dataInstance
	@param[in] param
	@return vector<MBEnity *>
*/
vector<LAObject *>
LAScenarioConfigurationYieldCurve::createScenario(LADataInstance &dataInstance, const MAScenarioParam &param) const
{
	// extra Xccy curve case
	if (param.isExtraFwdFXConstCurveForFXDelta)
	{
		return createScenarioExtraFwdFXConstCurveForFXDelta(dataInstance, param);
	}

	// extra forward fx constant curve case
	if (param.isExtraXccyCurveForFXDelta)
	{
		return createScenarioExtraXccyCurveForFXDelta(dataInstance, param);
	}

	//foreign yield case
	if (param.isForeignCcy)
	{
		return createScenarioForeignYield(dataInstance, param);
	}
	//collateral yield case
	if (param.isCollateralCcy)
	{
		return createScenarioCollateralYield(dataInstance, param);
	}
	// get ArbFree Flag
	const bool isArbFree = dynamic_cast<const LADataBool&>(dynamic_cast<LAMathYieldCurvePro &>
						(dataInstance.getObjectPool().getObject(LAMarketData::getBaseYieldProName(param.ccy), ENCHKTYPE_ISDEFINED).get())
						.getIsArbFree()).get();

	vector<LAObject *> ret(0);
	LAObjectPool &objPool = dataInstance.getObjectPool();

	//////////////////////////////////////////////////
	// get market data
	//////////////////////////////////////////////////
	// get yield
	LAString bYieldName = param.targetName;
	
	const LAMathYieldCurve &bYield = dynamic_cast<const LAMathYieldCurve &>
					(objPool.getObject(bYieldName, ENCHKTYPE_ISDEFINED).get());

	// get yield data
	const LAObject &bYieldData = bYield.getYieldData().get().get();
	const LAString bYieldDataName = dynamic_cast<const LADataString &>
						(bYieldData.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

	// get yield data pro
	LAMathYieldCurvePro &bYieldPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(param.ccy), ENCHKTYPE_ISDEFINED).get());

	const LADate asOfDate = LAMarketData::getAsofDate(objPool);
	LAString shiftType = param.shiftType;
	shiftType.toUpper();
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();

	LAString attrSuffix = "";
	if (param.targetCurveType != STD)
	{
		attrSuffix = "_" + param.targetCurveType;
	}

	// flag to setBasis
	bool isBasis = false;
	LADataHolder &ahBasis = bYieldPro.getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (ahBasis.isDefined() && !ahBasis.isNull())
	{
		isBasis = true;
	}
	LADataHolder &ahMBasisDF = bYieldPro.getData(IR_CALIBRATION_DATA_MAINBASISDF, NOCHECK);
	if (ahMBasisDF.isDefined() && !ahMBasisDF.isNull())
	{
		isBasis = true;
	}

	LADataMultiReference &refMarketDatas = dynamic_cast<LADataMultiReference &>
		                                  (bYieldPro.getData(CALIBRATION_DATA_MARKETDATA + attrSuffix, ISNOTNULL).get());
	const unsigned int marketSize = refMarketDatas.getSize();
	unsigned int basisSize  = 0;
	for (unsigned int i = 0; i < marketSize; ++i)
	{
		LAString type = dynamic_cast<const LADataString &>(refMarketDatas.get(i)
									.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
		type.toUpper();
		if (type == YIELD_TYPE_BASIS)
		{
			++basisSize;
		}
	}
	if (basisSize > 0)
	{
		isBasis = true;
	}

	// flag to collateral curve
	bool isCollateral = false;
	if (bYieldPro.getColAffectingCcy().size() > 0)
	{
		isCollateral = true;
	}
	// flag to fwdfx const curve
	bool isFwdFXConst = false;
	if (bYieldPro.getColAffectedCcy().size() > 0)
	{
		isFwdFXConst = true;
	}

	const unsigned int rateSize = marketSize - basisSize;

	// get base rates and sort terms
	DoubleArray baseRates(rateSize);   // base rate
	DoubleArray baseRatesShift(rateSize); // base rate plus base shift value
	LAStringVector marketTerms(rateSize);

	if (!param.baseShiftVal.empty() && param.baseShiftVal.size() != rateSize)
	{
		throw LACoreInvalidData("Base shift error.  Term and shifval size is not consistent", __FILE__, __LINE__);
	}

	LAString tmpCurrency = param.ccy;
	tmpCurrency.toUpper();

	// for aud
	// original ref string
	LAString aud_origRefString = refMarketDatas.convertToString();
	aud_origRefString.exchange("\"","");
	// ref string for first curve generate  
	LAString aud_firstRefString;
	// store original swap rate
	map<LAString, double> aud_origSwapRate;
	LADate date_3y = LAMathDateCalculations::getDate(asOfDate, "3Y", true);
	unsigned int aud_swapIndex = 0;

	for (unsigned int i = 0; i < rateSize; ++i)
	{
		LAObjectHolder &objHolder = refMarketDatas.get(i);
		LADataDouble &attrRate = dynamic_cast<LADataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
		baseRates[i] = attrRate.get();
		// add base shift
		if (param.baseShiftVal.size() > i)
		{
#ifdef ZEROFLOOR
			attrRate.set(LAMath::max(baseRates[i] + param.baseShiftVal[i], MIN_RATE));
#else
			attrRate.set(baseRates[i] + param.baseShiftVal[i]);
#endif
			baseRatesShift[i] = attrRate.get();
		}
		else
		{
			baseRatesShift[i] = attrRate.get();
		}
		const LAString &dataType = dynamic_cast<const LADataString &>(objHolder.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
		if (dataType == YIELD_TYPE_O_N)
		{
			marketTerms[i] = "ON";
		}
		else if (dataType == YIELD_TYPE_T_N)
		{
			marketTerms[i] = "TN";
		}
		else
		{
			marketTerms[i] = dynamic_cast<const LADataString &>(objHolder.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();
		}

		if (param.isAudExtra && tmpCurrency == CURRENCY_AUD)
		{
			if (dataType == YIELD_TYPE_O_N || dataType == YIELD_TYPE_T_N)
			{
				aud_firstRefString += objHolder.getName() + ":";
			}
			else
			{
				LADate date = LAMathDateCalculations::getDate(asOfDate, marketTerms[i], true);
				if (date_3y >= date)
				{
					aud_firstRefString += objHolder.getName() + ":";
					if (dataType == YIELD_TYPE_PAR)
					{
						aud_swapIndex = i;
						aud_origSwapRate.insert(make_pair(marketTerms[i], baseRatesShift[i]));
					}
				}
			}
		}
	}
	if (aud_firstRefString.size() > 1)
	{
		aud_firstRefString = aud_firstRefString.subString(0, aud_firstRefString.size() - 2);
	}

	// parallel shift case
	if (param.isParallel)
	{
		if (param.paraShiftVec.empty() || param.paraShiftVec.size() != param.paraTerm.size())
		{
			throw LACoreInvalidData("Parallel shift error. Term and shifval size is not consistent", __FILE__, __LINE__);
		}
		LAString suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Parallel";
		// clone yield data and yield
		const LAString cYieldDataName = bYieldDataName + suffix; 
		LAObject *cYieldData = 0;
		LAObjectHolder objHolder = objPool.getObject(cYieldDataName, ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			cYieldData = bYieldData.clone();
			cYieldData->remove(CALIBRATION_DATA_NAME);
			cYieldData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(cYieldDataName);
			objPool.set(cYieldDataName, cYieldData);
			//remove curve data
			bYieldPro.removeAllCuveData(*cYieldData);
		}
		else
		{
			cYieldData = &objHolder.get();
		}

		LAMathYieldCurve *cYield = 0;
		const LAString cYieldName = bYieldName + suffix; 
		objHolder = objPool.getObject(cYieldName, ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			cYield = dynamic_cast<LAMathYieldCurve *>(bYield.clone());
			cYield->getName().set(cYieldName);
		}
		else
		{
			cYield = &dynamic_cast<LAMathYieldCurve &>(objHolder.get());
		}
	
		// set yield data yield curve pro and yield curve
		cYield->getYieldData().convertFromString(cYieldDataName);
		bYieldPro.getYieldData().convertFromString(cYieldDataName);

		// shift market rate
		const unsigned int size =  param.paraTerm.size();
		for (unsigned int i = 0; i < size; ++i)
		{
			const unsigned int targetSize = static_cast<unsigned int>(count(marketTerms.begin(), marketTerms.end(), param.paraTerm[i]));
			if (targetSize == 0)
			{
				LAString msg = "Term " + param.paraTerm[i] + " is not in market data.";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			LAStringVector::iterator t_it_first = marketTerms.begin();
			for (unsigned int j = targetSize; j > 0; --j)
			{
				// search terms
				t_it_first = find(t_it_first, marketTerms.end(), param.paraTerm[i]);
				if (t_it_first == marketTerms.end())
				{
					throw LACoreInvalidData("Parallel shift error. Param term and market term is not consistent.", __FILE__, __LINE__); 
				}
				// calc reference index
				const unsigned int index = static_cast<unsigned int>(t_it_first - marketTerms.begin());
				LAObjectHolder &eh_m = refMarketDatas.get(index);

				LADataDouble &attrRate = dynamic_cast<LADataDouble &>(eh_m.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
				const LAString &dataType = dynamic_cast<const LADataString &>(eh_m.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
				double shiftVal = param.paraShiftVec[index];
				// rate case
				if (shiftType == RISK_SHIFTTYPE_RATIO)
				{
					shiftVal *= baseRatesShift[index];
				}
				// set shift val
#ifdef ZEROFLOOR
				attrRate.set(LAMath::max(baseRatesShift[index] + shiftVal, MIN_RATE));
#else
				attrRate.set(baseRatesShift[index] + shiftVal);
#endif
				
				++t_it_first;
			}
		}
		// if aud call generate curve 2 times
		if (param.isAudExtra && tmpCurrency == CURRENCY_AUD && aud_firstRefString.size() > 0)
		{
			refMarketDatas.convertFromString(aud_firstRefString);
			// generate yield data
			LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
								(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			dataInstance.getReferencePool().completeDependency();
			modelDataObj.calibrateModel(asOfDate);
			
			//reset market data as libor
			LAMarketData::resetMarketDataUseL(bYieldPro, tmpCurrency, &param.targetCurveType);
			// restore market ref
			refMarketDatas.convertFromString(aud_origRefString);
		}

		// generate yield data
		LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
							(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

		bYieldPro.clearBCurveGenerateMap();
		modelDataObj.calibrateModel(asOfDate);

		const LADataHolder* dh = &bYieldPro.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
		// generate swap curve	
		if (isArbFree && dh->isDefined() && !dh->isNull())
		{
			const LAStringVector &swapCurves = dynamic_cast<const LADataStrings &>(dh->get()).get();
			bYieldPro.clearGCurveGenerateMap();
			bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
			for (size_t i=0; i<swapCurves.size(); i++)
			{
				bYieldPro.LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(swapCurves[i]));
				modelDataObj = dynamic_cast<LADataProcedure &> (bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
				modelDataObj.calibrateModel(asOfDate);
				bYieldPro.LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
			}
			bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
		}
		
		if (param.isAdjustDF)
		{
			const std::map<LAString, LAString> &assignedCurveMktMap = bYieldPro.getAssignedCurveMktMap();
			for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
			{
				if (!bYieldPro.isBasisCurve(it->first))
				{
					LAMarketData::adjustDiscountFactor(*cYieldData, it->first);
				}
			}
		}

		// re-generate basis curve
		if (isBasis && (isCollateral || isFwdFXConst))
		{
			// set basis
			bYieldPro.setBasisRates();
		}
		if (isFwdFXConst)
		{
			// calc fwdfx constant curve
			bYieldPro.calcFwdFXConstantCurve();
		}
		if (isBasis && !(isCollateral || isFwdFXConst))
		{
			// set basis after fwdfx constant curve
			bYieldPro.setBasisRates();
		}
		// set floater df
		LADataHolder &ahFloater = bYieldPro.getData(IR_CALIBRATION_DATA_FLOATERDFS, NOCHECK);
		if (ahFloater.isDefined() && !ahFloater.isNull())
		{
			LAString floaterName = dynamic_cast<LADataString &>(ahFloater.get()).get();
			bYieldPro.setFloater(floaterName);
		}
		
		dataInstance.getReferencePool().completeDependency();			
		ret.push_back(cYield);

		// if aud restore original swap rate
		if (param.isAudExtra && tmpCurrency == CURRENCY_AUD && !aud_origSwapRate.empty())
		{
			LAMarketData::restoreSwapRateFromL(bYieldPro, aud_origSwapRate, tmpCurrency, &param.targetCurveType);
		}

		// set base rate (include shift value)
		for (unsigned int i = 0; i < rateSize; ++i)
		{
			LAObjectHolder &objHolder = refMarketDatas.get(i);
			LADataDouble &attrRate = dynamic_cast<LADataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
			attrRate.set(baseRatesShift[i]);
		}
	}

	// grid shift case
	if (param.isGrid)
	{
		if (param.gridShiftVec.empty() || param.gridShiftVec.size() != param.gridTerm.size())
		{
			throw LACoreInvalidData("Grid shift error. Term and shifval size is not consistent", __FILE__, __LINE__);
		}

		if (param.gridGroupID.empty() || param.gridGroupID.size() != param.gridTerm.size())
		{
			throw LACoreInvalidData("Grid group error. Term and group(shift group) size is not consistent", __FILE__, __LINE__);
		}

		map<LAString, double> aud_waveSwapRate;
		const unsigned int gridSize = param.gridTerm.size();
		unsigned int counter = 0;
		bool endFlg = false;
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			if (endFlg)
			{
				break;
			}
			const unsigned int targetSize = static_cast<int>(count(marketTerms.begin(), marketTerms.end(), param.gridTerm[i]));
			LAStringVector::iterator t_it_first = marketTerms.begin();
			for (unsigned int j = targetSize; j > 0; --j)
			{
				// search terms
				t_it_first = find(t_it_first, marketTerms.end(), param.gridTerm[i]);
				if (t_it_first == marketTerms.end())
				{
					throw LACoreInvalidData("Grid shift error. Grid term and market term is not consistent.", __FILE__, __LINE__); 
				}
				// calc reference index
				const unsigned int index = static_cast<unsigned int>(t_it_first - marketTerms.begin());
				// check max index
				if (i != 0 && param.gridGroupID[i] != param.gridGroupID[i - 1] && i > param.maxIndex)
				{
					endFlg = true;
					break;
				}
				LAObjectHolder &eh_m = refMarketDatas.get(index);
				LADataDouble &attrRate = dynamic_cast<LADataDouble &>(eh_m.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
				const LAString &dataType = dynamic_cast<const LADataString &>(eh_m.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
				double shiftVal = param.gridShiftVec[index];
				// rate case
				if (shiftType == RISK_SHIFTTYPE_RATIO)
				{
					shiftVal *= baseRatesShift[index];
				}
				// set shift val
#ifdef ZEROFLOOR
				attrRate.set(LAMath::max(baseRatesShift[index] + shiftVal, MIN_RATE));
#else
				attrRate.set(baseRatesShift[index] + shiftVal);
#endif
				if (dataType == YIELD_TYPE_PAR && index <= aud_swapIndex)
				{
#ifdef ZEROFLOOR
					aud_waveSwapRate.insert(make_pair(marketTerms[index], LAMath::max(baseRatesShift[index] + shiftVal, MIN_RATE)));
#else
					aud_waveSwapRate.insert(make_pair(marketTerms[index], baseRatesShift[index] + shiftVal));
#endif
				}

				// if same group with after shift, continue(not generate curve)
				if (i != gridSize - 1 && param.gridGroupID[i] == param.gridGroupID[i + 1])
				{
					++t_it_first;
					continue;
				}

				LAString suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" +  LAString(static_cast<int>(counter++));
				// clone yield data and yield
				const LAString cYieldDataName = bYieldDataName + suffix; 
				LAObject *cYieldData = 0;
				LAObjectHolder objHolder = objPool.getObject(cYieldDataName, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					cYieldData = bYieldData.clone();
					cYieldData->remove(CALIBRATION_DATA_NAME);
					cYieldData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(cYieldDataName);
					objPool.set(cYieldDataName, cYieldData);
					//remove curve data
					bYieldPro.removeAllCuveData(*cYieldData);
				}
				else
				{
					cYieldData = &objHolder.get();
				}

				LAMathYieldCurve *cYield = 0;
				const LAString cYieldName = bYieldName + suffix; 
				objHolder = objPool.getObject(cYieldName, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					cYield = dynamic_cast<LAMathYieldCurve *>(bYield.clone());
					cYield->getName().set(cYieldName);
				}
				else
				{
					cYield = &dynamic_cast<LAMathYieldCurve &>(objHolder.get());
				}

				// set yield data yield curve pro and yield curve
				cYield->getYieldData().convertFromString(cYieldDataName);
				bYieldPro.getYieldData().convertFromString(cYieldDataName);
				

				// if aud call generate curve 2 times
				if (param.isAudExtra && tmpCurrency == CURRENCY_AUD && aud_firstRefString.size() > 0)
				{
					refMarketDatas.convertFromString(aud_firstRefString);
					// generate yield data
					LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
										(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

					dataInstance.getReferencePool().completeDependency();
					modelDataObj.calibrateModel(asOfDate);

					//reset market data as libor
					LAMarketData::resetMarketDataUseL(bYieldPro, tmpCurrency, &param.targetCurveType);
					// restore market ref
					refMarketDatas.convertFromString(aud_origRefString);
				}

				// generate yield data
				LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
									(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
				dataInstance.getReferencePool().completeDependency();
		        bYieldPro.clearBCurveGenerateMap();
				modelDataObj.calibrateModel(asOfDate);

				const LADataHolder* dh = &bYieldPro.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
				// generate swap curve	
				if (isArbFree && dh->isDefined() && !dh->isNull())
				{
					const LAStringVector &swapCurves = dynamic_cast<const LADataStrings &>(dh->get()).get();
					bYieldPro.clearGCurveGenerateMap();
					bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
					for (size_t i=0; i<swapCurves.size(); i++)
					{
						bYieldPro.LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(swapCurves[i]));
						modelDataObj = dynamic_cast<LADataProcedure &> (bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
						modelDataObj.calibrateModel(asOfDate);
						bYieldPro.LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
					}
					bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
				}

				if (param.isAdjustDF)
				{
					const std::map<LAString, LAString> &assignedCurveMktMap = bYieldPro.getAssignedCurveMktMap();
					for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
					{
						if (!bYieldPro.isBasisCurve(it->first))
						{
							LAMarketData::adjustDiscountFactor(*cYieldData, it->first);
						}
					}
				}

				// re-generate basis curve
				if (isBasis && (isCollateral || isFwdFXConst))
				{
					// set basis
					bYieldPro.setBasisRates();
				}
				if (isFwdFXConst)
				{
					// calc fwdfx constant curve
					bYieldPro.calcFwdFXConstantCurve();
				}
				if (isBasis && !(isCollateral || isFwdFXConst))
				{
					// set basis after fwdfx constant curve
					bYieldPro.setBasisRates();
				}
				// set floater df
				LADataHolder &ahFloater = bYieldPro.getData(IR_CALIBRATION_DATA_FLOATERDFS, NOCHECK);
				if (ahFloater.isDefined() && !ahFloater.isNull())
				{
					LAString floaterName = dynamic_cast<LADataString &>(ahFloater.get()).get();
					bYieldPro.setFloater(floaterName);
				}

				ret.push_back(cYield);

				// if aud restore original swap rate
				if (param.isAudExtra && tmpCurrency == CURRENCY_AUD && !aud_origSwapRate.empty())
				{
					LAMarketData::restoreSwapRateFromL(bYieldPro, aud_origSwapRate, tmpCurrency, &param.targetCurveType);
				}

				if (!param.isWave)
				{
					for (unsigned int k = 0; k <= index; ++k)
					{
						// set base rate
						dynamic_cast<LADataDouble &>(refMarketDatas.get(k).getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).set(baseRatesShift[k]);
					}
				}
				else if (param.isAudExtra && tmpCurrency == CURRENCY_AUD && !aud_waveSwapRate.empty())
				{
					LAMarketData::restoreSwapRateFromL(bYieldPro, aud_waveSwapRate, tmpCurrency, &param.targetCurveType);
				}
				++t_it_first;
			}
		}
	}
	// set base value
	for (unsigned int i = 0; i < rateSize; ++i)
	{
		LAObjectHolder &objHolder = refMarketDatas.get(i);
		LADataDouble &attrRate = dynamic_cast<LADataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
		attrRate.set(baseRates[i]);
	}

	bYieldPro.LAObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	// hirayake data out
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
	}

	return ret;
}
