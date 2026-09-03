/*! @file
    @brief  Basis Delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldBasisDelta.cpp
//
//  DESCRIPTION :       Basis Delta setup class
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


#include <functional>
#include <algorithm>
#include "ConstantDeclarations.h"
#include "LARiskConfigurationYieldBasisDelta.h"
#include "LADataInstance.h"
#include "LABasic.h"
#include "LAObjectPool.h"
#include "LADataReference.h"
#include "LADataProcedure.h"
#include "LADefinitionsRisk.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAFileAccessor.h"
#include "LAMarketData.h"
#include "LALinearFunc.h"
#include "LAStaticData.h"
#include "LADealUtils.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldBasisDelta::LARiskConfigurationYieldBasisDelta(void)
:LARiskConfigurationYield()
{
}

// destructor
/*!

*/
LARiskConfigurationYieldBasisDelta::~LARiskConfigurationYieldBasisDelta(void)
{
}

/*!
    @brief create yield object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationYieldBasisDelta::createYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	if (isZeroBump(ccy))
	{
		return createZeroBumpYieldEntity(ccy, dataInstance, scenarioNum, index);
	}
	else
	{
		return createMarketBumpYieldEntity(ccy, dataInstance, scenarioNum, index);
	}
}


/*!
    @brief create zero bump object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationYieldBasisDelta::createZeroBumpYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}
	// check
	// bucket term
	LAStringVector bucketTerm = getBucketGridTerm(ccy);
	if (!bucketTerm.empty() && bucketTerm[0] != MLIB_NO_DATA)
	{
		throw  LACoreInvalidData("Basis delta fail. Zero rate bump does not support bucket.", __FILE__, __LINE__);
	}
	// shift type
	LAString shiftType = getShiftType(ccy);
	shiftType.toUpper();
	if (shiftType != RISK_SHIFTTYPE_DIFF)
	{
		throw  LACoreInvalidData("Basis delta fail. Zero rate bump supports diff only.", __FILE__, __LINE__);
	}

	// get grid and shift val
	const LAStringVector grid = getShiftGridTerm(ccy);
	// get shift value
	DoubleArray paraShiftVec;
	DoubleArray gridShiftVec;	
	if (scenarioNum == SCENARIO_1)
	{
		getScenario1ParallelShift(ccy, paraShiftVec);
		gridShiftVec = getScenario1GridShift(ccy);
		if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
		{
			transform(paraShiftVec.begin(), paraShiftVec.end(), paraShiftVec.begin(), negate<double>());
			transform(gridShiftVec.begin(), gridShiftVec.end(), gridShiftVec.begin(), negate<double>());
		}
	}
	else
	{
		getScenario2ParallelShift(ccy, paraShiftVec);
		gridShiftVec = getScenario2GridShift(ccy);
		transform(paraShiftVec.begin(), paraShiftVec.end(), paraShiftVec.begin(), negate<double>());
		transform(gridShiftVec.begin(), gridShiftVec.end(), gridShiftVec.begin(), negate<double>());
	}

	// set up param
	MAScenarioParam param;
	param.ccy = ccy;
	param.calcType= ccy + "_" + riskName + "_" + LAString(scenarioNum) + "_" + LAString(index);
	param.model = model;
	param.shiftType = shiftType;
	param.bumpDirection = bumpDirection;
	param.targetName = LAMarketData::getBaseYieldName(ccy);
	param.targetCurveType = getCurveType(ccy);
	LAString tmpCurrency = ccy;
	tmpCurrency.toLower();
	LAString recalc = mpRiskStaticData->getStaticData(
		tmpCurrency + STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_RECALCBASISDFSONZERORATEBUMP + getCurveSuffix(ccy));
	if (recalc != MLIB_NO_DATA)
	{
		LADataBool attrBool;
		attrBool.convertFromString(recalc);
		param.recalcBasisDFsOnZeroRateBump = attrBool.get();
	}
	else
	{
		param.recalcBasisDFsOnZeroRateBump = false;
	}

	if (isParallelShift(ccy))
	{
		param.isParallel = true;
		param.paraShiftVec.clear();
		param.paraShiftVec.push_back(paraShiftVec.front());
	}

	if (isGridSensitivity(ccy))
	{
		// check
		if (grid.size() !=  gridShiftVec.size())
		{
			throw LACoreInvalidData("Grid size and grid shift size is not same !!", __FILE__, __LINE__); 
		}

		param.isGrid = true;
		param.isWave = isWave(ccy);
		param.gridTerm = grid;
		param.maxIndex = getMaxGridIndex(ccy);
		param.gridShiftVec = gridShiftVec;
	}

	if (getIMMFwdRiskMode(ccy))
	{
		// When zero rate bump -> IMM forward rate bump conversion is required,
		// curve types are specified as market names (for instance, 3M6MBASIS instead of AUD6MLFORECAST);
		// so we shall convert them into asigned curve names to make the scenario creator of zero rates work
		LAStringVector targets = param.targetCurveType.toToken('/');
		LAString lowerSTD = STD;
		lowerSTD.toLower();
		StringSet names;
		for (size_t i = 0, ie = targets.size(); i < ie; ++i)
		{
			LAString target = targets[i];
			target.toLower();
			LAString postfix = (target == lowerSTD) ? "" : "." + target;
			const LAStringVector assigned = mpStaticData->getStaticData(
				LAString(ccy).toLower() + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + postfix).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
			names.insert(assigned.begin(), assigned.end());
			if (target == lowerSTD)
			{
				names.insert(STD);
			}
		}
		param.targetCurveType = "";
		for (StringSet::const_iterator i = names.begin(), ie = names.end(); i != ie; ++i)
		{
			param.targetCurveType += *i;
			param.targetCurveType += "/";
		}
		if (param.targetCurveType.size() > 0U)
		{
			param.targetCurveType.remove(param.targetCurveType.size() - 1, 1);
		}
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELDZEROBASIS);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief create market bump object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfigurationYieldBasisDelta::createMarketBumpYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	const LAString model = LAMarketData::getModelName(ccy);
	const LAString riskName = getRiskName();
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	LAStringVector grid = getShiftGridTerm(ccy);
	LAStringVector BucketTerm = getBucketGridTerm(ccy);

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<LAObject *>(0);
	}	
	//check bucket grid
	if (BucketTerm[0] != MLIB_NO_DATA)
	{
		LAStringVector::iterator it;
		for (unsigned int i = 0;i < BucketTerm.size();++i)
		{
			it = find(grid.begin(),grid.end(),BucketTerm[i]);
			if (it == grid.end())
				throw LACoreInvalidData("Basis delta fail. Risk property bucketterm grid isn't registrate.", __FILE__, __LINE__);

			if (i != 0)
			{
				LAStringVector::const_iterator it_now,it_before;
				it_now = find(grid.begin(),grid.end(),BucketTerm[i]);
				it_before = find(grid.begin(),grid.end(),BucketTerm[i-1]);
				if (it_before >= it_now)
				{
					throw LACoreInvalidData("Basis delta fail. The order of bucket term is inconsistent!", __FILE__, __LINE__);
				}
			}
		}				
	}
	// get shift value
	DoubleArray paraShiftVec;
	DoubleArray gridShiftVec;	
	if (scenarioNum == SCENARIO_1)
	{
		getScenario1ParallelShift(ccy, paraShiftVec);
		gridShiftVec = getScenario1GridShift(ccy);
		if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
		{
			transform(paraShiftVec.begin(), paraShiftVec.end(), paraShiftVec.begin(), negate<double>());
			transform(gridShiftVec.begin(), gridShiftVec.end(), gridShiftVec.begin(), negate<double>());
		}
	}
	else
	{
		getScenario2ParallelShift(ccy, paraShiftVec);
		gridShiftVec = getScenario2GridShift(ccy);
		transform(paraShiftVec.begin(), paraShiftVec.end(), paraShiftVec.begin(), negate<double>());
		transform(gridShiftVec.begin(), gridShiftVec.end(), gridShiftVec.begin(), negate<double>());
	}


	unsigned int gridSize = grid.size();
	LAStringVector paramGrid(gridSize);
	IntArray gridGroupID(gridSize);
	unsigned int j = 0;
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		paramGrid[i] = LAMarketData::convertToMLibTerm(grid[i]);
		//for bucket grid
		if (BucketTerm[0] == "NONE" ||BucketTerm[0] == MLIB_NO_DATA)
		{
			gridGroupID[i] = i;
		}
		else
		{
			//bucket mode
			gridGroupID[i] = j;			
			if(j != BucketTerm.size() && grid[i] == BucketTerm[j])
				++j;
		}
	}

	// set up param
	MAScenarioParam param;
	param.ccy = ccy;
	param.calcType = getCalcType(ccy, scenarioNum, index);
	param.model = model;
	param.shiftType = getShiftType(ccy);
	param.basisType = getBasisType(ccy);
	param.bumpDirection = getBumpDirection(ccy);
	param.targetName = LAMarketData::getBaseYieldName(ccy);
	param.targetCurveType = getCurveType(ccy);
	param.isAdjustDF = isAdjustDf(ccy);
	param.isFwdFXZeroRateBump = isFwdFXZeroRateBump(ccy);
	param.isFwdPointBump = isFwdPointBump(ccy);

	LAString mktSuffix = "." + param.targetCurveType;
	mktSuffix.toLower();
	LAString tmpCcy = ccy;
	tmpCcy.toLower();
	LAString fwdFXPipsizeFactorString = mpStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_DENOMINATOR + mktSuffix,
																	tmpCcy + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_PIPSIZE + mktSuffix); // Alias Method: First Parameter Takes Priority
	if (fwdFXPipsizeFactorString != MLIB_NO_DATA)
		param.fwdfxDenominator = fwdFXPipsizeFactorString.getDoubleValue();

	// set targetCurveType for col-xccybasis delta
	LAObjectPool &objPool = dataInstance.getObjectPool();
	const LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());
	LAStringVector fCurveCcys = ycPro.getAffectingCcy();
	for (int unsigned i = 0; i < fCurveCcys.size(); ++i)
	{
		LAStringVector tmpMarkets = mpStaticData->getStaticData(fCurveCcys[i].toLower() + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(':');
		if (tmpMarkets[0] == MLIB_NO_DATA) continue;
		for (int i = 0; i < tmpMarkets.size(); ++i)
		{
			const LAStringVector tmpMarket = tmpMarkets[i].toUpper().toToken('_');
			if (tmpMarket.size() == 2)
			{
				if (tmpMarket[0] == ccy && tmpMarket[1] == param.targetCurveType)
				{
					param.targetCurveType = tmpMarkets[i];
					break;
				}
			}
		}
	}

	if (isParallelShift(ccy))
	{
		// check
		if (paramGrid.size() !=  paraShiftVec.size())
		{
			throw LACoreInvalidData("Grid size and parallel shift size is not same !!", __FILE__, __LINE__); 
		}
		param.isParallel = true;
		param.paraTerm = paramGrid;
		param.paraShiftVec = paraShiftVec;
	}

	if (isGridSensitivity(ccy))
	{
		// check
		if (paramGrid.size() !=  gridShiftVec.size())
		{
			throw LACoreInvalidData("Grid size and grid shift size is not same !!", __FILE__, __LINE__); 
		}

		param.isGrid = true;
		param.isWave = isWave(ccy);
		param.gridTerm = paramGrid;
		param.gridGroupID = gridGroupID;	
		//set max index
		param.maxIndex = getMaxGridIndex(ccy);
		param.gridShiftVec = gridShiftVec;
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELDBASIS);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

 

/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasisDelta::getOutPutName1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isGridSensitivity(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @Description: Tells IMM forward risk mode
    @param ccy [in] The currency
    @return 0 if no IMM-forward-risk-like care is applied,
            1 if market rate bump risk -> IMM forward risk conversion is applied,
            2 if market zero rate bump risk -> IMM forward risk conversion is applied
*/
int
LARiskConfigurationYieldBasisDelta::getIMMFwdRiskMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString isIMMProp = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
											STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISIMMFWDRATEBUMP + getCurveSuffix(ccy));
	if ((isIMMProp == MLIB_NO_DATA) || !convertBoolFromStr(isIMMProp))
	{
		return 0;
	}
	LAString isZeroRateBump = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
											STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	return convertBoolFromStr(isZeroRateBump) ? 2 : 1;
}

std::vector<int>
LARiskConfigurationYieldBasisDelta::getIMMTerm(const LAString &ccy) const
{
	if (getIMMFwdRiskMode(ccy))
	{
		LADataInts immTerm;
		immTerm.convertFromString(mpRiskStaticData->getStaticData(
			(ccy + STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_IMMFWDRATETERM).toLower() + getCurveSuffix(ccy)));
		return immTerm.get();
	}
	else
	{
		return std::vector<int>();
	}
}

/*!
    @Description: Returns (possibly term-wise) risk yield curve name(s) for IMM forward risks
    @param ccy [in] The currency
    @return An empty vector if all IMM forward risks shall be zero,
            a vector with one risk curve name if all risk curves are the same,
            a vector with risk curve names whose number is the same as the risk grids,
            where a risk curve name is the curve type which is suitable to be passed to
            LAMathYieldCurvePro::setCurveType (for example, "JPYOIS") to deduce the forward rate changes
            against the market rate bumps
*/
LAStringVector
LARiskConfigurationYieldBasisDelta::getIMMRiskYieldCurveName(const LAString &ccy) const
{
	if (!getIMMFwdRiskMode(ccy))
	{
		std::vector<LAString> v;
		v.push_back("");
		return v;
	}

	LAString ccyL = ccy;
	ccyL.toLower();
	LAString curveSuffix = getCurveSuffix(ccy);

	LAString assignedCurvePropName = ccyL + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + curveSuffix;

	LAString assignedCurve = mpStaticData->getStaticData(assignedCurvePropName).toToken(MULTI_STATIC_DATA_DELIMITER).front().toUpper();
	bool hasAssignedCurve = assignedCurve != MLIB_NO_DATA;
	LAString curveTypeL = getCurveType(ccy).toLower();
	LAString curveTypeU = curveTypeL;
	curveTypeU.toUpper();

	LADataBool isSwapTenorAdjustAttr;
	isSwapTenorAdjustAttr.convertFromString(mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST));
	if (isSwapTenorAdjustAttr.get())
	{
		LAString tenorSwapName = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toLower();
		if (tenorSwapName == curveTypeL)
		{
			LAStringVector useGrid = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_SWAP_USEGRID).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
			StringSet useGridSet(useGrid.begin(), useGrid.end());
			LAString baseFrequencyFloat = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT).toLower();
			LAString frequencyFloat = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT).toLower();
			LAStringVector useMarkets = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_USEMAKETS).toLower().toToken(MULTI_STATIC_DATA_DELIMITER);
			bool inUseMarkets = std::find(useMarkets.begin(), useMarkets.end(), curveTypeL) != useMarkets.end();

			LAStringVector terms = mpRiskStaticData->getStaticData(ccyL + STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_GRID_TERM + curveSuffix).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
			LAStringVector riskYieldCurveNames(terms.size());
			for (LAStringVector::size_type i = 0; i < terms.size(); ++i)
			{
				if (useGridSet.find(terms[i]) != useGridSet.end())
				{
					LAString frequencyFloatTerm = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + '.' + terms[i].toLower());
					if (frequencyFloatTerm != MLIB_NO_DATA)
					{
						if (baseFrequencyFloat != frequencyFloatTerm.toLower())
						{
							riskYieldCurveNames[i] = STD;
							continue;
						}
					}
					else
					{
						if (baseFrequencyFloat != frequencyFloat)
						{
							riskYieldCurveNames[i] = STD;
							continue;
						}
					}
				}
				if (inUseMarkets && hasAssignedCurve)
				{
					riskYieldCurveNames[i] = assignedCurve;
				}
				else
				{
					riskYieldCurveNames[i] = "";
				}
			}

			return riskYieldCurveNames;
		}
	}

	if (!hasAssignedCurve)
	{
#ifndef NDEBUG
cout << "Property " << assignedCurvePropName.getCString() << " is not set but "
     << (ccy + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISIMMFWDRATEBUMP).toLower() << getCurveSuffix(ccy).getCString() << " is set to TRUE" << endl;
#endif
		return std::vector<LAString>();
	}

	std::vector<LAString> riskYieldCurveNames;
	riskYieldCurveNames.push_back(assignedCurve);
	return riskYieldCurveNames;
}

/*!
    @Description: Returns the risk floor term, the date from which the first market rate bump risk is attributed to,
                  that is, if this is "3Y" and the first market bump term is "4Y", then the first risk is attributed to
                  the period [3Y, 4Y) instead of [baseDate, 4Y)
    @param ccy [in] The currency
    @arapm objPool [in] the object pool
    @return An empty string if there is no such risk floor term, a appropriate term string otherwise
*/
LAString
LARiskConfigurationYieldBasisDelta::getIMMRiskFloorTerm(const LAString &ccy, LAObjectPool &objPool) const
{
	if (getIMMFwdRiskMode(ccy))
	{
		LAString proName = LAMarketData::getBaseYieldProName(ccy);
		const LAMathYieldCurvePro &pro = dynamic_cast<const LAMathYieldCurvePro&>(objPool.getObject(proName).get());

		// For example, "USDOIS" -> "OISCURVE"
		LAString originalCurveName = pro.getMarketForCurve(getIMMRiskYieldCurveName(ccy).front()).toUpper();
		if (originalCurveName == OISCURVE)
		{
			if ((mpStaticData->getStaticData((ccy + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + '.' + OISCURVE).toLower()).toUpper() == LOBASIS)
			 && (mpStaticData->getStaticData((ccy + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + '.' + OISCURVE).toLower()).toUpper() == getCurveType(ccy).toUpper()))
			{
				LAStringVector useGrid = mpStaticData->getStaticData((ccy + STATIC_DATA_KEY_YIELD_OIS_USEGRID + '.' + OISCURVE).toLower()).toToken(MULTI_STATIC_DATA_DELIMITER);
				LAString longTerm = mpStaticData->getStaticData((ccy + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + '.' + OISCURVE).toLower());
				LAStringVector::const_iterator i = std::find(useGrid.begin(), useGrid.end(), longTerm);
				if ((i != useGrid.end()) && (i != useGrid.begin()))
				{
					--i;
					return *i;
				}
			}
		}
	}
	return "";
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationYieldBasisDelta::isParallelShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISPARALLEL + getCurveSuffix(ccy)));
}


/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationYieldBasisDelta::getRiskName(void) const
{
	return RISK_FRONT_YIELD_BASISDELTA;
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldBasisDelta::getShiftGridTerm(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_GRID_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldBasisDelta::getGridTerm(const LAString &ccy) const
{
	LAStringVector ret;
	const LAString curveType = getCurveType(ccy) + "_";
	if (isZeroBump(ccy))
	{
		ret = getShiftGridTerm(ccy);
		for (int i = 0; i < ret.size(); ++i)
		{
			ret[i] = curveType + ret[i];
		}
	}
	else
	{
		LAString tmpCurrency = ccy;
		tmpCurrency.toLower();

		// read fwdfx file
		LAStringMatrix fwdfxDataMtx;
		LAString fwdfxFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FILE + getCurveSuffix(ccy));
		if (fwdfxFileName != MLIB_NO_DATA)
		{
			MAFileAccessor fwdfxFile(LAMarketData::getNumFileName(fwdfxFileName));
			fwdfxFile.readAllData(MARKET_DATA_DELIMITER, fwdfxDataMtx);
			fwdfxFile.close();
		}
		// read basis file
		LAStringMatrix basisDataMtx;
		LAString basisFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + getCurveSuffix(ccy));
		if (basisFileName != MLIB_NO_DATA)
		{
			MAFileAccessor basisFile(LAMarketData::getNumFileName(basisFileName));
			basisFile.readAllData(MARKET_DATA_DELIMITER, basisDataMtx);
			basisFile.close();
		}

		// get terms
		const unsigned int fwdfxSize = fwdfxDataMtx.size();
		const unsigned int basisSize = basisDataMtx.size();
		LAStringVector fwdfx(fwdfxSize);
		LAStringVector basis(basisSize);
		for (unsigned int i = 0; i < fwdfxSize; ++i)
		{
			fwdfx[i] = fwdfxDataMtx[i][0].toUpper();
		}
		for (unsigned int i = 0; i < basisSize; ++i)
		{
			basis[i] = basisDataMtx[i][0].toUpper();
		}
		sort(fwdfx.begin(), fwdfx.end(), MAComp_StrTerm());
		sort(basis.begin(), basis.end(), MAComp_StrTerm());

		// set grids
		for (unsigned int i = 0; i < fwdfxSize; ++i)
		{
			ret.push_back("FWDFX_" + fwdfx[i]);
		}
		for (unsigned int i = 0; i < basisSize; ++i)
		{
			ret.push_back(curveType + basis[i]);
		}
	}
	return ret;
}

/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldBasisDelta::getBucketGridTerm(const LAString &ccy) const
{
	LAStringVector ret = getGridRangeTerm(ccy);
	if (!ret.empty()) return ret;
	LAString tmpccy = ccy;
	LAString strBucketGrid = mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BUCKET_GRID_TERM + getCurveSuffix(ccy));
	LAStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" ||BucketTerm[0] == MLIB_NO_DATA)
	{
		ret.push_back(MLIB_NO_DATA);
		return ret;
	}
	else
	{
		unsigned int gridMax = getMaxGridIndex(ccy);
		LAStringVector tmpgridTerm = getShiftGridTerm(ccy);
	
		for (unsigned int i = 0;i < BucketTerm.size();++i)
		{
			LAStringVector::iterator it;
			LAString strgrid = BucketTerm[i].toUpper();
			it = find(tmpgridTerm.begin(),tmpgridTerm.end(),strgrid);
			unsigned int pos = static_cast<unsigned int>(it - tmpgridTerm.begin());
			if (pos >= gridMax)
			{
				ret.push_back(BucketTerm[i]);
				break;
			}
			else
			{	
					ret.push_back(BucketTerm[i]);
			}
			if (i == BucketTerm.size() - 1 && pos <= gridMax)
				ret.push_back(tmpgridTerm[gridMax]);
		}
		return ret;
	}
}

/*!
    @brief return property bucket grid term

	@param[in] ccy
	@return vector<LAString>
*/
LAString
LARiskConfigurationYieldBasisDelta::getPropertyBucketGridTerm(const LAString &ccy) const
{
	LAString tmpccy = ccy;
	return mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BUCKET_GRID_TERM + getCurveSuffix(ccy));
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@param[in] vals
*/
void
LARiskConfigurationYieldBasisDelta::getScenario1ParallelShift(const LAString &ccy, DoubleArray &vals) const
{
	vals.clear();
	LAString tmpCurrency = ccy;
	LAString strShift = getScenario1ParallelShiftStr(ccy);

	vals =  convertToRateValues(strShift.toToken(MULTI_STATIC_DATA_DELIMITER));

	LAString shiftType = getShiftType(ccy);
	shiftType.toUpper();

	if (shiftType == RISK_SHIFTTYPE_RATIO)
	{
		DoubleArray multiple_percent(vals.size(), 100.0);
		transform(vals.begin(), vals.end(), multiple_percent.begin(), vals.begin(), multiplies<double>());
	}
}

/*!
    @brief return scenario1 parallel shift value(string)

	@param[in] ccy
	@return LAString
*/
LAString 
LARiskConfigurationYieldBasisDelta::getScenario1ParallelShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_PARALLEL_SHIFTVAL + getCurveSuffix(ccy));
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
void
LARiskConfigurationYieldBasisDelta::getScenario2ParallelShift(const LAString &ccy, DoubleArray &vals) const
{
	getScenario1ParallelShift(ccy, vals);
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldBasisDelta::getScenario1ParallelShift(const LAString &ccy) const
{
	DoubleArray tmp;
	getScenario1ParallelShift(ccy, tmp);
	return tmp[0];
}

/*!
    @brief return scenario2 parallel shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldBasisDelta::getScenario2ParallelShift(const LAString &ccy) const
{
	return getScenario1ParallelShift(ccy);
}

/*!
    @brief return scenario1 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldBasisDelta::getScenario1GridShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strShift = getScenario1GridShiftStr(ccy);

	DoubleArray ret =  convertToRateValues(strShift.toToken(MULTI_STATIC_DATA_DELIMITER));

	LAString shiftType = getShiftType(ccy);
	shiftType.toUpper();

	if (shiftType == RISK_SHIFTTYPE_DIFF)
	{
		return ret;
	}
	else
	{
		DoubleArray percent_multiple(ret.size(), 100.0);
		transform(ret.begin(), ret.end(), percent_multiple.begin(), ret.begin(), multiplies<double>());
		return ret;
	}
}


/*!
    @brief return scenario1 grid shift values(string)

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasisDelta::getScenario1GridShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
	    						STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_GRID_SHIFTVAL + getCurveSuffix(ccy));


}

/*!
    @brief return scenario2 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldBasisDelta::getScenario2GridShift(const LAString &ccy) const
{
	return getScenario1GridShift(ccy);
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasisDelta::getBumpDirection(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BUMPDIRECTION + getCurveSuffix(ccy));

}


/*!
	@brief is forward FX zero rate bump

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isFwdFXZeroRateBump(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString tmpIsFwdFXZeroRateBump = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
		STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISFWDFXZERORATEBUMP + getCurveSuffix(ccy));
	if (tmpIsFwdFXZeroRateBump == MLIB_NO_DATA) return false;
	return convertBoolFromStr(tmpIsFwdFXZeroRateBump);
}

/*!
	@brief is 1 forward FX point bump

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isFwdPointBump(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString tmpIsFwdPointBump = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
		STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISFWDPOINTBUMP + getCurveSuffix(ccy));
	if (tmpIsFwdPointBump == MLIB_NO_DATA) return true;
	return convertBoolFromStr(tmpIsFwdPointBump);
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isWave(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief return shift type

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasisDelta::getShiftType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SHIFTTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return basis type

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldBasisDelta::getBasisType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BASISTYPE + getCurveSuffix(ccy));

}


/*!
    @brief return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldBasisDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_BASISDELTA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldBasisDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_BASISDELTA_CALIBRATION_TARGET_CURRENCY);
}


/*!
    @brief return divid unit

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldBasisDelta::getDivUnit(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strDivUnit = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_DIVUNIT + getCurveSuffix(ccy));

	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
LARiskConfigurationYieldBasisDelta::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_BASISDELTA_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief return istargetccy

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfigurationYieldBasisDelta::isTarget(const LAString &ccy) const
{
	LAString basisCurrency = mpStaticData->getStaticData(KEY_SDE_BASIS_BASE_CURRENCY);
	LAString tmpCurrency = ccy;
	tmpCurrency.toUpper();

	if (basisCurrency != MLIB_NO_DATA)
	{
		if (basisCurrency.toUpper() == tmpCurrency)
		{
			return false;
		}
	}

	LAString targetccys = getTargetCurrencies();
	targetccys.toUpper();

	if (targetccys == "ALL")
	{
		return true;
	}
	else
	{
		LAStringVector targetVec = targetccys.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (targetVec.end() != find(targetVec.begin(), targetVec.end(), tmpCurrency))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isRiskCurrencyMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);

}


/*!
    @brief check zero rate bump

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isZeroBump(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}

/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfigurationYieldBasisDelta::createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	if (omitNotionalExposure(ccy))
	{
		return createBasisBaseScenarioEntity(ccy,dataInstance,index);
	}
	else
	{
		return std::vector<LAObject*>(0);
	}
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfigurationYieldBasisDelta::getBaseOutPutName(const LAString &ccy , int index) const
{
	if (omitNotionalExposure(ccy))
	{
		LAString tmpCcy = ccy;
		tmpCcy.toLower();
		LAString ret = mpRiskStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_OUTPUTNAME + getCurveSuffix(ccy));
		ret += LAString("_BasePV");
		return ret;
	}
	else
	{
		return MLIB_NO_DATA;
	}
}

/*!
    @brief return shift type

	@param[in] fx
	@return LAString
*/
bool
LARiskConfigurationYieldBasisDelta::omitNotionalExposure(const LAString &ccy) const
{
	LAString tmpCcy = ccy;
	tmpCcy.toLower();
	LAString omitNotionalExposure = mpRiskStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_OMITNOTIONALEXPOSURE + getCurveSuffix(ccy));
	if (omitNotionalExposure == MLIB_NO_DATA)
	{
		return false;
	}
	else
	{
		return convertBoolFromStr(omitNotionalExposure);
	}
}
