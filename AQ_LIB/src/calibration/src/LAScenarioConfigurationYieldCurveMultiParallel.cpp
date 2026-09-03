/*! @file
    @brief Yield Risk Scenario create class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationYieldCurveMultiParallel.cpp
//
//  DESCRIPTION :       Yield Risk Scenario create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
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
#include "LAScenarioConfigurationYieldCurveMultiParallel.h"
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
LAScenarioConfigurationYieldCurveMultiParallel::LAScenarioConfigurationYieldCurveMultiParallel(void)
{
}

// destructor
/*!

*/
LAScenarioConfigurationYieldCurveMultiParallel::~LAScenarioConfigurationYieldCurveMultiParallel(void)
{
}


 
/*!
    @brief Risk Scenario create method

	@param[int,out] dataInstance
	@param[in] param
	@return vector<MBEnity *>
*/
vector<LAObject *>
LAScenarioConfigurationYieldCurveMultiParallel::createScenario(LADataInstance &dataInstance, const MAScenarioParam &param) const
{
	// get ArbFree Flag
	const bool isArbFree = dynamic_cast<const LADataBool&>(dynamic_cast<LAMathYieldCurvePro &>
						(dataInstance.getObjectPool().getObject(LAMarketData::getBaseYieldProName(param.ccy), ENCHKTYPE_ISDEFINED).get())
						.getIsArbFree()).get();

	if (param.isGrid)
	{
		throw LACoreInvalidData("Grid shift is not supported in LAScenarioConfigurationYieldCurveMultiParallel", __FILE__, __LINE__);
	}
	if (!param.isParallel)
	{
		throw LACoreInvalidData("Parallel shift must be specified in LAScenarioConfigurationYieldCurveMultiParallel", __FILE__, __LINE__);
	}
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
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();

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
	LAStringVector targetCurveTypes = param.targetCurveType.toToken('/');
	if (!isBasis)
	{
		for (unsigned int i = 0; i < targetCurveTypes.size(); ++i)
		{
			LAString attrSuffix = "";
			LAString tmp = targetCurveTypes[i];
			if (tmp.toUpper() != BASECURVE)
			{
				attrSuffix = "_" + targetCurveTypes[i];
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
					isBasis = true;
					break;
				}
			}
		}
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

	if (param.paraShiftVec.empty())
	{
		throw LACoreInvalidData("Shift Val is empty", __FILE__, __LINE__);
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
	LAString shiftType = param.shiftType;
	shiftType.toUpper();
	DoubleMatrix baseRatesMtx(targetCurveTypes.size());
	vector<LADataMultiReference *> refVec(targetCurveTypes.size());
	for (unsigned int i = 0; i < targetCurveTypes.size(); ++i)
	{
		LAString attrSuffix = "";
		LAString tmp = targetCurveTypes[i];
		tmp.toUpper();
		if (tmp != BASECURVE && tmp.toUpper() != STD)
		{
			attrSuffix = "_" + targetCurveTypes[i];
		}
		refVec[i] = &dynamic_cast<LADataMultiReference &>
												(bYieldPro.getData(CALIBRATION_DATA_MARKETDATA + attrSuffix, ISNOTNULL).get());
		const unsigned int marketSize = refVec[i]->getSize();
		baseRatesMtx[i].resize(marketSize);   // base rate
		DoubleArray baseRatesShift(marketSize); // base rate plus base shift value
		for (unsigned int j = 0; j < marketSize; ++j)
		{
			LAObjectHolder &objHolder =refVec[i]->get(j);
			LADataDouble &attrRate = dynamic_cast<LADataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
			baseRatesMtx[i][j] = attrRate.get();
			LAString type = dynamic_cast<const LADataString &>(objHolder.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
			type.toUpper();
			if (type == YIELD_TYPE_BASIS)
			{
				continue;
			}

			if (!param.baseShiftVal.empty())
			{
	#ifdef ZEROFLOOR
				attrRate.set(LAMath::max(baseRatesMtx[i][j] + param.baseShiftVal[0], MIN_RATE));
	#else
				attrRate.set(baseRatesMtx[i][j] + param.baseShiftVal[0]);
	#endif
			}
			baseRatesShift[j] = attrRate.get();
			double shiftVal = param.paraShiftVec[0];
			// rate case
			if (shiftType == RISK_SHIFTTYPE_RATIO)
			{
				shiftVal *= baseRatesShift[j];
			}
			// set shift val
#ifdef ZEROFLOOR
			attrRate.set(LAMath::max(baseRatesShift[j] + shiftVal, MIN_RATE));
#else
			attrRate.set(baseRatesShift[j] + shiftVal);
#endif
		}
	}
	// generate yield data
	LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
						(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

	const LADate asOfDate = LAMarketData::getAsofDate(objPool);
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
		//bYieldPro.removeCuveData(*cYieldData, FWDFXCONST);
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

	// set base rate (include shift value)
	for (unsigned int i = 0; i < targetCurveTypes.size(); ++i)
	{
		const unsigned int marketSize = refVec[i]->getSize();
		for (unsigned int j = 0; j < marketSize; ++j)
		{
			LAObjectHolder &objHolder =  refVec[i]->get(j);
			LADataDouble &attrRate = dynamic_cast<LADataDouble &>(objHolder.getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
			attrRate.set(baseRatesMtx[i][j]);
		}
	}
	return vector<LAObject *>(1, cYield);
}
