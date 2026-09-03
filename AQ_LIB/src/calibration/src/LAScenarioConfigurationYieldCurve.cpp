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

#include "AQLDataInstance.h"
#include "AQLCoreReferencePool.h"
#include "AQLObjectPool.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "AQLDataMultiReference.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "LAScenarioConfigurationYieldCurve.h"
#include "LAMarketData.h"
#include "AQLBasic.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLPriceArbFreeGenerator.h"

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
vector<AQLObject *>
LAScenarioConfigurationYieldCurve::createScenario(AQLDataInstance &dataInstance, const MAScenarioParam &param) const
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
	const bool isArbFree = dynamic_cast<const AQLDataBool&>(dynamic_cast<AQLMathYieldCurvePro &>
						(dataInstance.getObjectPool().getObject(LAMarketData::getBaseYieldProName(param.ccy), ENCHKTYPE_ISDEFINED).get())
						.getIsArbFree()).get();

	vector<AQLObject *> ret(0);
	AQLObjectPool &objPool = dataInstance.getObjectPool();

	//////////////////////////////////////////////////
	// get market data
	//////////////////////////////////////////////////
	// get yield
	AQLString bYieldName = param.targetName;
	
	const AQLMathYieldCurve &bYield = dynamic_cast<const AQLMathYieldCurve &>
					(objPool.getObject(bYieldName, ENCHKTYPE_ISDEFINED).get());

	// get yield data
	const AQLObject &bYieldData = bYield.getYieldData().get().get();
	const AQLString bYieldDataName = dynamic_cast<const AQLDataString &>
						(bYieldData.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

	// get yield data pro
	AQLMathYieldCurvePro &bYieldPro = dynamic_cast<AQLMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(param.ccy), ENCHKTYPE_ISDEFINED).get());

	const AQLDate asOfDate = LAMarketData::getAsofDate(objPool);
	AQLString shiftType = param.shiftType;
	shiftType.toUpper();
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();

	AQLString attrSuffix = "";
	if (param.targetCurveType != STD)
	{
		attrSuffix = "_" + param.targetCurveType;
	}

	// flag to setBasis
	bool isBasis = false;
	AQLDataHolder &ahBasis = bYieldPro.getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (ahBasis.isDefined() && !ahBasis.isNull())
	{
		isBasis = true;
	}
	AQLDataHolder &ahMBasisDF = bYieldPro.getData(IR_CALIBRATION_DATA_MAINBASISDF, NOCHECK);
	if (ahMBasisDF.isDefined() && !ahMBasisDF.isNull())
	{
		isBasis = true;
	}

	AQLDataMultiReference &refMarketDatas = dynamic_cast<AQLDataMultiReference &>
		                                  (bYieldPro.getData(CALIBRATION_DATA_MARKETDATA + attrSuffix, ISNOTNULL).get());
	const unsigned int marketSize = refMarketDatas.getSize();
	unsigned int basisSize  = 0;
	for (unsigned int i = 0; i < marketSize; ++i)
	{
		AQLString type = dynamic_cast<const AQLDataString &>(refMarketDatas.get(i)
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
	AQLStringVector marketTerms(rateSize);

	if (!param.baseShiftVal.empty() && param.baseShiftVal.size() != rateSize)
	{
		throw AQLCoreInvalidData("Base shift error.  Term and shifval size is not consistent", __FILE__, __LINE__);
	}

	AQLString tmpCurrency = param.ccy;
	tmpCurrency.toUpper();

	// for aud
	// original ref string
	AQLString aud_origRefString = refMarketDatas.convertToString();
	aud_origRefString.exchange("\"","");
	// ref string for first curve generate  
	AQLString aud_firstRefString;
	// store original swap rate
	map<AQLString, double> aud_origSwapRate;
	AQLDate date_3y = AQLMathDateCalculations::getDate(asOfDate, "3Y", true);
	unsigned int aud_swapIndex = 0;

	for (unsigned int i = 0; i < rateSize; ++i)
	{
		AQLObjectHolder &objHolder = refMarketDatas.get(i);
		AQLDataDouble &attrRate = dynamic_cast<AQLDataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
		baseRates[i] = attrRate.get();
		// add base shift
		if (param.baseShiftVal.size() > i)
		{
#ifdef ZEROFLOOR
			attrRate.set(AQLMath::max(baseRates[i] + param.baseShiftVal[i], MIN_RATE));
#else
			attrRate.set(baseRates[i] + param.baseShiftVal[i]);
#endif
			baseRatesShift[i] = attrRate.get();
		}
		else
		{
			baseRatesShift[i] = attrRate.get();
		}
		const AQLString &dataType = dynamic_cast<const AQLDataString &>(objHolder.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
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
			marketTerms[i] = dynamic_cast<const AQLDataString &>(objHolder.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();
		}

		if (param.isAudExtra && tmpCurrency == CURRENCY_AUD)
		{
			if (dataType == YIELD_TYPE_O_N || dataType == YIELD_TYPE_T_N)
			{
				aud_firstRefString += objHolder.getName() + ":";
			}
			else
			{
				AQLDate date = AQLMathDateCalculations::getDate(asOfDate, marketTerms[i], true);
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
			throw AQLCoreInvalidData("Parallel shift error. Term and shifval size is not consistent", __FILE__, __LINE__);
		}
		AQLString suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Parallel";
		// clone yield data and yield
		const AQLString cYieldDataName = bYieldDataName + suffix; 
		AQLObject *cYieldData = 0;
		AQLObjectHolder objHolder = objPool.getObject(cYieldDataName, ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			cYieldData = bYieldData.clone();
			cYieldData->remove(CALIBRATION_DATA_NAME);
			cYieldData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(cYieldDataName);
			objPool.set(cYieldDataName, cYieldData);
			//remove curve data
			bYieldPro.removeAllCuveData(*cYieldData);
		}
		else
		{
			cYieldData = &objHolder.get();
		}

		AQLMathYieldCurve *cYield = 0;
		const AQLString cYieldName = bYieldName + suffix; 
		objHolder = objPool.getObject(cYieldName, ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			cYield = dynamic_cast<AQLMathYieldCurve *>(bYield.clone());
			cYield->getName().set(cYieldName);
		}
		else
		{
			cYield = &dynamic_cast<AQLMathYieldCurve &>(objHolder.get());
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
				AQLString msg = "Term " + param.paraTerm[i] + " is not in market data.";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			AQLStringVector::iterator t_it_first = marketTerms.begin();
			for (unsigned int j = targetSize; j > 0; --j)
			{
				// search terms
				t_it_first = find(t_it_first, marketTerms.end(), param.paraTerm[i]);
				if (t_it_first == marketTerms.end())
				{
					throw AQLCoreInvalidData("Parallel shift error. Param term and market term is not consistent.", __FILE__, __LINE__); 
				}
				// calc reference index
				const unsigned int index = static_cast<unsigned int>(t_it_first - marketTerms.begin());
				AQLObjectHolder &eh_m = refMarketDatas.get(index);

				AQLDataDouble &attrRate = dynamic_cast<AQLDataDouble &>(eh_m.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
				const AQLString &dataType = dynamic_cast<const AQLDataString &>(eh_m.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
				double shiftVal = param.paraShiftVec[index];
				// rate case
				if (shiftType == RISK_SHIFTTYPE_RATIO)
				{
					shiftVal *= baseRatesShift[index];
				}
				// set shift val
#ifdef ZEROFLOOR
				attrRate.set(AQLMath::max(baseRatesShift[index] + shiftVal, MIN_RATE));
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
			AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
								(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			dataInstance.getReferencePool().completeDependency();
			modelDataObj.calibrateModel(asOfDate);
			
			//reset market data as libor
			LAMarketData::resetMarketDataUseL(bYieldPro, tmpCurrency, &param.targetCurveType);
			// restore market ref
			refMarketDatas.convertFromString(aud_origRefString);
		}

		// generate yield data
		AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
							(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

		bYieldPro.clearBCurveGenerateMap();
		modelDataObj.calibrateModel(asOfDate);

		const AQLDataHolder* dh = &bYieldPro.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
		// generate swap curve	
		if (isArbFree && dh->isDefined() && !dh->isNull())
		{
			const AQLStringVector &swapCurves = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
			bYieldPro.clearGCurveGenerateMap();
			bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
			for (size_t i=0; i<swapCurves.size(); i++)
			{
				bYieldPro.AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(swapCurves[i]));
				modelDataObj = dynamic_cast<AQLDataProcedure &> (bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
				modelDataObj.calibrateModel(asOfDate);
				bYieldPro.AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
			}
			bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
		}
		
		if (param.isAdjustDF)
		{
			const std::map<AQLString, AQLString> &assignedCurveMktMap = bYieldPro.getAssignedCurveMktMap();
			for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
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
		AQLDataHolder &ahFloater = bYieldPro.getData(IR_CALIBRATION_DATA_FLOATERDFS, NOCHECK);
		if (ahFloater.isDefined() && !ahFloater.isNull())
		{
			AQLString floaterName = dynamic_cast<AQLDataString &>(ahFloater.get()).get();
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
			AQLObjectHolder &objHolder = refMarketDatas.get(i);
			AQLDataDouble &attrRate = dynamic_cast<AQLDataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
			attrRate.set(baseRatesShift[i]);
		}
	}

	// grid shift case
	if (param.isGrid)
	{
		if (param.gridShiftVec.empty() || param.gridShiftVec.size() != param.gridTerm.size())
		{
			throw AQLCoreInvalidData("Grid shift error. Term and shifval size is not consistent", __FILE__, __LINE__);
		}

		if (param.gridGroupID.empty() || param.gridGroupID.size() != param.gridTerm.size())
		{
			throw AQLCoreInvalidData("Grid group error. Term and group(shift group) size is not consistent", __FILE__, __LINE__);
		}

		map<AQLString, double> aud_waveSwapRate;
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
			AQLStringVector::iterator t_it_first = marketTerms.begin();
			for (unsigned int j = targetSize; j > 0; --j)
			{
				// search terms
				t_it_first = find(t_it_first, marketTerms.end(), param.gridTerm[i]);
				if (t_it_first == marketTerms.end())
				{
					throw AQLCoreInvalidData("Grid shift error. Grid term and market term is not consistent.", __FILE__, __LINE__); 
				}
				// calc reference index
				const unsigned int index = static_cast<unsigned int>(t_it_first - marketTerms.begin());
				// check max index
				if (i != 0 && param.gridGroupID[i] != param.gridGroupID[i - 1] && i > param.maxIndex)
				{
					endFlg = true;
					break;
				}
				AQLObjectHolder &eh_m = refMarketDatas.get(index);
				AQLDataDouble &attrRate = dynamic_cast<AQLDataDouble &>(eh_m.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
				const AQLString &dataType = dynamic_cast<const AQLDataString &>(eh_m.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
				double shiftVal = param.gridShiftVec[index];
				// rate case
				if (shiftType == RISK_SHIFTTYPE_RATIO)
				{
					shiftVal *= baseRatesShift[index];
				}
				// set shift val
#ifdef ZEROFLOOR
				attrRate.set(AQLMath::max(baseRatesShift[index] + shiftVal, MIN_RATE));
#else
				attrRate.set(baseRatesShift[index] + shiftVal);
#endif
				if (dataType == YIELD_TYPE_PAR && index <= aud_swapIndex)
				{
#ifdef ZEROFLOOR
					aud_waveSwapRate.insert(make_pair(marketTerms[index], AQLMath::max(baseRatesShift[index] + shiftVal, MIN_RATE)));
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

				AQLString suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" +  AQLString(static_cast<int>(counter++));
				// clone yield data and yield
				const AQLString cYieldDataName = bYieldDataName + suffix; 
				AQLObject *cYieldData = 0;
				AQLObjectHolder objHolder = objPool.getObject(cYieldDataName, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					cYieldData = bYieldData.clone();
					cYieldData->remove(CALIBRATION_DATA_NAME);
					cYieldData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(cYieldDataName);
					objPool.set(cYieldDataName, cYieldData);
					//remove curve data
					bYieldPro.removeAllCuveData(*cYieldData);
				}
				else
				{
					cYieldData = &objHolder.get();
				}

				AQLMathYieldCurve *cYield = 0;
				const AQLString cYieldName = bYieldName + suffix; 
				objHolder = objPool.getObject(cYieldName, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					cYield = dynamic_cast<AQLMathYieldCurve *>(bYield.clone());
					cYield->getName().set(cYieldName);
				}
				else
				{
					cYield = &dynamic_cast<AQLMathYieldCurve &>(objHolder.get());
				}

				// set yield data yield curve pro and yield curve
				cYield->getYieldData().convertFromString(cYieldDataName);
				bYieldPro.getYieldData().convertFromString(cYieldDataName);
				

				// if aud call generate curve 2 times
				if (param.isAudExtra && tmpCurrency == CURRENCY_AUD && aud_firstRefString.size() > 0)
				{
					refMarketDatas.convertFromString(aud_firstRefString);
					// generate yield data
					AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
										(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

					dataInstance.getReferencePool().completeDependency();
					modelDataObj.calibrateModel(asOfDate);

					//reset market data as libor
					LAMarketData::resetMarketDataUseL(bYieldPro, tmpCurrency, &param.targetCurveType);
					// restore market ref
					refMarketDatas.convertFromString(aud_origRefString);
				}

				// generate yield data
				AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
									(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
				dataInstance.getReferencePool().completeDependency();
		        bYieldPro.clearBCurveGenerateMap();
				modelDataObj.calibrateModel(asOfDate);

				const AQLDataHolder* dh = &bYieldPro.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
				// generate swap curve	
				if (isArbFree && dh->isDefined() && !dh->isNull())
				{
					const AQLStringVector &swapCurves = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
					bYieldPro.clearGCurveGenerateMap();
					bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRYIELDGENERATOR_STR);
					for (size_t i=0; i<swapCurves.size(); i++)
					{
						bYieldPro.AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(swapCurves[i]));
						modelDataObj = dynamic_cast<AQLDataProcedure &> (bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
						modelDataObj.calibrateModel(asOfDate);
						bYieldPro.AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
					}
					bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
				}

				if (param.isAdjustDF)
				{
					const std::map<AQLString, AQLString> &assignedCurveMktMap = bYieldPro.getAssignedCurveMktMap();
					for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
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
				AQLDataHolder &ahFloater = bYieldPro.getData(IR_CALIBRATION_DATA_FLOATERDFS, NOCHECK);
				if (ahFloater.isDefined() && !ahFloater.isNull())
				{
					AQLString floaterName = dynamic_cast<AQLDataString &>(ahFloater.get()).get();
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
						dynamic_cast<AQLDataDouble &>(refMarketDatas.get(k).getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).set(baseRatesShift[k]);
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
		AQLObjectHolder &objHolder = refMarketDatas.get(i);
		AQLDataDouble &attrRate = dynamic_cast<AQLDataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
		attrRate.set(baseRates[i]);
	}

	bYieldPro.AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);

	// hirayake data out
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
	}

	return ret;
}
