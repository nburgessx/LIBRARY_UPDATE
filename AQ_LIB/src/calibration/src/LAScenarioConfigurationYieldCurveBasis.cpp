#define _HAS_STD_BYTE 0

/*! @file
    @brief Yield Risk Scenario create class
*/
//  2008, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationYieldCurveBasis.cpp
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
#include "LAPriceArbFreeGenerator.h"
#include "LAScenarioConfigurationYieldCurveBasis.h"
#include "LAMarketData.h"
#include "LABasic.h"
#include "LAPriceYieldGenerator.h"

using namespace std;

// constructor
/*!

*/
LAScenarioConfigurationYieldCurveBasis::LAScenarioConfigurationYieldCurveBasis(void)
{
}

// destructor
/*!

*/
LAScenarioConfigurationYieldCurveBasis::~LAScenarioConfigurationYieldCurveBasis(void)
{
}


 
/*!
    @brief Risk Scenario create method

	@param[int,out] dataInstance
	@param[in] param
	@return vector<MBEnity *>
*/
vector<LAObject *>
LAScenarioConfigurationYieldCurveBasis::createScenario(LADataInstance &dataInstance, const MAScenarioParam &param) const
{
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

	const LADataHolder *dh = &bYieldPro.getData(IR_CALIBRATION_DATA_DFCURVENAME, NOCHECK);

	LAString dfCurveName = ITSELF;
	if (dh->isDefined() && !dh->isNull())
	{
		dfCurveName = dynamic_cast<const LADataString &>(dh->get()).get();
	}

	LAString marketName = bYieldPro.getMarketForCurve(dfCurveName);

	// foreign curve refer or not
	bool isForeignRefer = false;
	LAString tmpTgtCurveType = param.targetCurveType;
	if (tmpTgtCurveType.toToken('_').size() == 2) 
	{
		isForeignRefer = true;
		tmpTgtCurveType = tmpTgtCurveType.toToken('_').back();
	}

	LAString attrSuffix = "";
	if (tmpTgtCurveType != STD)
	{
		attrSuffix = "_" + tmpTgtCurveType;
	}

    LADataMultiReference& refMarket = dynamic_cast<LADataMultiReference &>(bYieldPro.getData(CALIBRATION_DATA_MARKETDATA + attrSuffix, ISNOTNULL).get());
	const unsigned int marketSize = refMarket.getSize();
	vector<LAObjectHolder> basisVec;
	vector<LAString> basisType; 
	for (unsigned int i = 0; i < marketSize; ++i)
	{
		LAString type = dynamic_cast<const LADataString &>
							(refMarket.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
		type.toUpper();
		if (type == YIELD_TYPE_BASIS || (type == YIELD_TYPE_FWDFX/* && param.isFwdFX*/))
		{
			basisVec.push_back(refMarket.get(i));
			basisType.push_back(type);
		}
	}

	const unsigned int basisSize = basisVec.size();
	if (basisSize == 0)
	{
		throw LACoreInvalidData("Basis rate is not set.", __FILE__, __LINE__);
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

	// get base rates and sort terms
	DoubleArray baseRates(basisSize);   
	LAStringVector marketTerms(basisSize);
	dataInstance.getReferencePool().completeDependency();
	for (unsigned int i = 0; i < basisSize; ++i)
	{
		LAObjectHolder &objHolder = basisVec[i];
		LADataDouble &attrRate = dynamic_cast<LADataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
		baseRates[i] = attrRate.get();
		marketTerms[i] = dynamic_cast<const LADataString &>(objHolder.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();
	}

	// Swap Tenor Change
	const bool isSwapTenorAdjust = dynamic_cast<const LADataBool&>(bYieldPro.getIsSwapTenorAdjust()).get();

	// Checkif wheather affect to base curve
	dh = &bYieldPro.getData(IR_CALIBRATION_DATA_ISAFFECTINGBASECURVE + attrSuffix, NOCHECK);
	bool isAffectBaseCurve = false;
	if (dh->isDefined() && !dh->isNull())
	{
		isAffectBaseCurve = dynamic_cast<const LADataBool&>(dh->get()).get();
	}
	isAffectBaseCurve = isAffectBaseCurve || isSwapTenorAdjust || isArbFree;

	// parallel shift case
	if (param.isParallel)
	{
		if (param.paraShiftVec.empty() || param.paraShiftVec.size() != param.paraTerm.size() || param.paraShiftVec.size() != basisSize)
		{
			throw LACoreInvalidData("Parallel shift error. Term and shifval size or term and basis size is not consistent", __FILE__, __LINE__);
		}
		LAString suffix = "_" + param.calcType + "_" + tmpTgtCurveType + "_Parallel";
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

			//If the discount curve of the Currency is a FWDFXCONST market name generator, 
			//then changing the xccy will change the effect the base curve therefore we need to recalculate all the yield curves
			if(marketName == "FWDFXCONST")
			{
				bYieldPro.removeAllCuveData(*cYieldData);
				bYieldPro.clearBCurveGenerateMap();
			}
			else
			{
				if (isAffectBaseCurve) bYieldPro.removeAllCuveData(*cYieldData);
				bYieldPro.removeBasisCuveData(*cYieldData);
				//bYieldPro.removeCuveData(*cYieldData, FWDFXCONST);
				LADataHolder &ahFloater = bYieldPro.getData(IR_CALIBRATION_DATA_FLOATERDFS, NOCHECK);
				if (ahFloater.isDefined() && !ahFloater.isNull()) 
				{
					LAString floaterName = dynamic_cast<LADataString &>(ahFloater.get()).get();
					bYieldPro.removeCuveData(*cYieldData, floaterName);
				}
			}
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
	
		// set yield data to yield curve pro and yield curve
		cYield->getYieldData().convertFromString(cYieldDataName);
		bYieldPro.getYieldData().convertFromString(cYieldDataName);

		// shift basis rate
		const unsigned int size =  param.paraTerm.size();
		for (unsigned int i = 0; i < size; ++i)
		{
			LAString paraTerm = param.paraTerm[i];
			paraTerm.toUpper();
			marketTerms[i].toUpper();

			if (paraTerm != marketTerms[i])
			{
				throw LACoreInvalidData("Parallel shift error. Param term and market term is not consistent.", __FILE__, __LINE__); 
			}

			double shiftValue = param.paraShiftVec[i];
			if (shiftType == RISK_SHIFTTYPE_DIFF && basisType[i] == YIELD_TYPE_FWDFX)
			{
				if (param.isFwdPointBump)
				{
					if (param.isFwdFXZeroRateBump)
					{
						throw LACoreInvalidData("FWD FX Basis Delta shift bump error. Both isFwdPointBump & isFwdFXZeroRateBump should not be TRUE.", __FILE__, __LINE__);
					}
					shiftValue *= 10000.;	//replication of initial input in BPs
					shiftValue /= param.fwdfxDenominator;
				}
				else if (param.isFwdFXZeroRateBump)
				{
					// Get today FX Rate of param.ccy and the cross currency basis base currency (USD).
					const LAMathFXEntity& fxEntity = dynamic_cast<const LAMathFXEntity&>(bYieldPro.getFXEntity().get().get());
					double todayFxRate = fxEntity.getRate(CURRENCY_USD, param.ccy, 0.);

					double gridTime = LAMarketData::getCalendarTime(asOfDate, marketTerms[i]);
					shiftValue *= todayFxRate * gridTime;
				}

			}
			createBumpedRate(basisVec[i], shiftType, baseRates[i], shiftValue);
		}

		if (isAffectBaseCurve) 
		{
			// generate yield data
			LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &> (bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
			dataInstance.getReferencePool().completeDependency();
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
		}

		//If the discount curve of the Currency is a FWDFXCONST market name generator, 
		//then changing the xccy will change the effect the base curve therefore we need to recalculate all the yield curves
		if(marketName == "FWDFXCONST")
		{
		    //generate yield data
			LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
								(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
			modelDataObj.calibrateModel(asOfDate);
		}

		dataInstance.getReferencePool().completeDependency();


		// re-generate basis curve
		if (isCollateral || isFwdFXConst)
		{
			// set basis
			bYieldPro.setBasisRates();
		}
		if (isFwdFXConst)
		{
			// calc fwdfx constant curve
			bYieldPro.removeCuveData(*cYieldData, FWDFXCONST);
			bYieldPro.calcFwdFXConstantCurve();
		}
		if (!isCollateral && !isFwdFXConst)
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

		// copy bumped xccybasis rates for foreign yield
		if (isForeignRefer)
		{
			copyBumpedDataForForeignYield(basisVec, suffix, objPool, *cYieldData, attrSuffix);
		}

		ret.push_back(cYield);

		// restore basis rate
		restoreBumpedRate(basisVec, baseRates);
	}

	// grid shift case
	if (param.isGrid)
	{
		if (param.gridShiftVec.empty() || param.gridShiftVec.size() != param.gridTerm.size() ||  param.gridShiftVec.size() != basisSize)
		{
			throw LACoreInvalidData("Grid shift error. Term and shifval size or term and basis size is not consistent", __FILE__, __LINE__);
		}

		if (param.gridGroupID.empty() || param.gridGroupID.size() != param.gridTerm.size())
		{
			throw LACoreInvalidData("Grid group error. Term and group(shift group) size is not consistent", __FILE__, __LINE__);
		}

		const unsigned int gridSize = param.gridTerm.size();
		unsigned int counter = 0;
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			LAString gridTerm = param.gridTerm[i];
			gridTerm.toUpper();
			marketTerms[i].toUpper();

			if (gridTerm != marketTerms[i])
			{
				throw LACoreInvalidData("Grid shift error. Grid term and market term is not consistent.", __FILE__, __LINE__); 
			}

			if (i != 0 && param.gridGroupID[i] != param.gridGroupID[i - 1] && i > param.maxIndex)
			{
				break;
			}
			
			double shiftValue = param.gridShiftVec[i];
			if (shiftType == RISK_SHIFTTYPE_DIFF && basisType[i] == YIELD_TYPE_FWDFX)
			{
				if (param.isFwdPointBump)
				{
					if (param.isFwdFXZeroRateBump)
					{
						throw LACoreInvalidData("FWD FX Basis Delta shift bump error. Both isFwdPointBump & isFwdFXZeroRateBump should not be TRUE.", __FILE__, __LINE__);
					}
					shiftValue *= 10000.;	//replication of initial input in BPs
					shiftValue /= param.fwdfxDenominator;
				}
				else if (param.isFwdFXZeroRateBump)
				{
					// Get today FX Rate of param.ccy and the cross currency basis base currency (USD).
					const LAMathFXEntity& fxEntity = dynamic_cast<const LAMathFXEntity&>(bYieldPro.getFXEntity().get().get());
					double todayFxRate = fxEntity.getRate(CURRENCY_USD, param.ccy, 0.);

					double gridTime = LAMarketData::getCalendarTime(asOfDate, marketTerms[i]);
					shiftValue *= todayFxRate * gridTime;
				}

			}
			createBumpedRate(basisVec[i], shiftType, baseRates[i], shiftValue);
			
			// if same group with after shift, continue(not generate curve)
			if (i != gridSize - 1 && param.gridGroupID[i] == param.gridGroupID[i + 1])
			{
				continue;
			}

			LAString suffix = "_" + param.calcType + "_" + tmpTgtCurveType + "_Grid_" +  LAString(static_cast<int>(counter++));
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

				if(marketName == "FWDFXCONST")
				{
					bYieldPro.removeAllCuveData(*cYieldData);
					bYieldPro.clearBCurveGenerateMap();
				}
				else
				{
					if (isAffectBaseCurve) bYieldPro.removeAllCuveData(*cYieldData);
					bYieldPro.removeBasisCuveData(*cYieldData);
					LADataHolder &ahFloater = bYieldPro.getData(IR_CALIBRATION_DATA_FLOATERDFS, NOCHECK);
					if (ahFloater.isDefined() && !ahFloater.isNull()) 
					{
						LAString floaterName = dynamic_cast<LADataString &>(ahFloater.get()).get();
						bYieldPro.removeCuveData(*cYieldData, floaterName);
					}
				}

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

			if(marketName == "FWDFXCONST")
			{
				// generate yield data
				LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
									(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
				modelDataObj.calibrateModel(asOfDate);
			}
			dataInstance.getReferencePool().completeDependency();

			// Swap Tenor Change
			if (isAffectBaseCurve) 
			{
				// generate yield data
				LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &> (bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());
				dataInstance.getReferencePool().completeDependency();
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
			}
			dataInstance.getReferencePool().completeDependency();

			// re-generate basis curve
			if (isCollateral || isFwdFXConst)
			{
				// set basis
				bYieldPro.setBasisRates();
			}

			if (isFwdFXConst)
			{
				// calc fwdfx constant curve
				bYieldPro.removeCuveData(*cYieldData, FWDFXCONST);
				bYieldPro.calcFwdFXConstantCurve();
			}
			if (!isCollateral && !isFwdFXConst)
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

			// copy bumped xccybasis rates for foreign yield
			if (isForeignRefer)
			{
				copyBumpedDataForForeignYield(basisVec, suffix, objPool, *cYieldData, attrSuffix);
			}
			
			ret.push_back(cYield);

			if (!param.isWave)
			{
				for (unsigned int j = 0; j <= i; ++j)
				{
					// set base rate
					dynamic_cast<LADataDouble &>(basisVec[j].getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).set(baseRates[j]);
				}
			}
		}
	}

	// set base value
	restoreBumpedRate(basisVec, baseRates);

	// data out for Curveinfo
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
    {
		dataInstance.getReferencePool().completeDependency();
		const LAString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM);
		const unsigned int yieldSize = ret.size();
		ofstream fout;
		for (unsigned int i = 0; i < yieldSize; ++i)
		{
			const LADataReference &yd = dynamic_cast<const LAMathYieldCurve *>(ret[i])->getYieldData();
			LAString ydName = yd.convertToString();
			ydName.exchange("\"","");
			const LAObjectHolder ydHolder = objPool.getObject(ydName, ENCHKTYPE_ISDEFINED);
			const LAString dirName = LACoreDataService::getOutputDirectory(); 
			const LAString fileName  = dirName + ydName + fileSuffix + ".csv";
			const DoubleArray &terms = 
				dynamic_cast<const LADataDoubles &>(ydHolder.getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();
			const DoubleArray &dfs = 
				dynamic_cast<const LADataDoubles &>(ydHolder.getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL).get()).get();

			int size = terms.size();
			if (size != static_cast<int>(dfs.size()))
			{
				throw LACoreInvalidData("Term size and df size must be same !!", __FILE__, __LINE__);
			}
			fout.open(fileName.getCString());

			const LADataHolder &dfAttr2 = ydHolder.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
			if (dfAttr2.isDefined() && !dfAttr2.isNull())
			{
				const DoubleArray &dfs2 = dynamic_cast<const LADataDoubles &>(dfAttr2.get()).get();
				for (int i = 0; i < size; ++i)
				{
					LAString termOStr = LAString(terms[i]);
					LAString dfOStr = LAString(dfs[i]);
					LAString dfOStr2 = LAString(dfs2[i]);
					fout << termOStr.getCString() << "," << dfOStr.getCString() << "," << dfOStr2.getCString() << std::endl;
				}
			}
			else
			{
				for (int i = 0; i < size; ++i)
				{
					LAString termOStr = LAString(terms[i]);
					LAString dfOStr = LAString(dfs[i]);
					fout << termOStr.getCString() << "," << dfOStr.getCString() << std::endl;
				}
			}
			fout.close();
		}
	}

	return ret;

}

/*!
    @brief create bumped rates

	@param[in,out] basis
	@param[in] shiftType
	@param[in] baseRate
	@param[in] shiftVal
*/
void 
LAScenarioConfigurationYieldCurveBasis::createBumpedRate(LAObjectHolder& basis, const LAString& shiftType, const double baseRate, const double shiftVal) const
{
	LADataDouble &attrRate = dynamic_cast<LADataDouble &>(basis.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
	// rate case
	double tmpShiftVal = shiftVal;
	if (shiftType == RISK_SHIFTTYPE_RATIO)
	{
		tmpShiftVal *= baseRate;
	}
	// set shift val
	attrRate.set(baseRate + tmpShiftVal);
}

/*!
    @brief restore bumped rates

	@param[in,out] basisVec
	@param[in] baseRates
*/
void 
LAScenarioConfigurationYieldCurveBasis::restoreBumpedRate(vector<LAObjectHolder>& basisVec, const DoubleArray& baseRates) const
{
	for (unsigned int i = 0; i < basisVec.size(); ++i)
	{
		LAObjectHolder &objHolder = basisVec[i];
		LADataDouble &attrRate = dynamic_cast<LADataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
		attrRate.set(baseRates[i]);
	}
}

/*!
    @brief copy bumped data for foreign yield

	@param[in] basisVec
	@param[in] suffix
	@param[in] objPool
	@param[in,out] yieldData
	@param[in] attrSuffix
*/
void 
LAScenarioConfigurationYieldCurveBasis::copyBumpedDataForForeignYield(const vector<LAObjectHolder>& basisVec, const LAString& suffix, LAObjectPool& objPool, LAObject& yieldData, const LAString& attrSuffix) const
{
	LAString refData;
	for (unsigned int i = 0; i < basisVec.size(); ++i)
	{
		const LAObjectHolder& ehOrg = basisVec[i];
		const LAString cpyName = dynamic_cast<const LADataString &>(ehOrg.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get() + suffix;
		LAObjectHolder ehCpy = objPool.getObject(cpyName, ENCHKTYPE_NOCHECK);
		LAObject* cpyData;
		if (!ehCpy.isDefined())
		{
			cpyData = ehOrg.clone();
			cpyData->remove(CALIBRATION_DATA_NAME);
			cpyData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(cpyName);
			objPool.set(cpyName, cpyData);
		}
		else
		{
			cpyData = &ehCpy.get();
		}
		refData += cpyName + ":";
	}
	refData = refData.subString(0, refData.size() - 2);
	yieldData.remove(CALIBRATION_DATA_MARKETDATA + attrSuffix);
	yieldData.add(CALIBRATION_DATA_MARKETDATA + attrSuffix, new LADataMultiReference()).convertFromString(refData);
}
