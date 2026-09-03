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
#include "AQLDataInstance.h"
#include "AQLBasic.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "LADefinitionsRisk.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAFileAccessor.h"
#include "LAMarketData.h"
#include "AQLLinearFunc.h"
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
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
LARiskConfigurationYieldBasisDelta::createYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
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
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
LARiskConfigurationYieldBasisDelta::createZeroBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	const AQLString model = LAMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}
	// check
	// bucket term
	AQLStringVector bucketTerm = getBucketGridTerm(ccy);
	if (!bucketTerm.empty() && bucketTerm[0] != AQ_NO_DATA)
	{
		throw  AQLCoreInvalidData("Basis delta fail. Zero rate bump does not support bucket.", __FILE__, __LINE__);
	}
	// shift type
	AQLString shiftType = getShiftType(ccy);
	shiftType.toUpper();
	if (shiftType != RISK_SHIFTTYPE_DIFF)
	{
		throw  AQLCoreInvalidData("Basis delta fail. Zero rate bump supports diff only.", __FILE__, __LINE__);
	}

	// get grid and shift val
	const AQLStringVector grid = getShiftGridTerm(ccy);
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
	param.calcType= ccy + "_" + riskName + "_" + AQLString(scenarioNum) + "_" + AQLString(index);
	param.model = model;
	param.shiftType = shiftType;
	param.bumpDirection = bumpDirection;
	param.targetName = LAMarketData::getBaseYieldName(ccy);
	param.targetCurveType = getCurveType(ccy);
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();
	AQLString recalc = mpRiskStaticData->getStaticData(
		tmpCurrency + STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_RECALCBASISDFSONZERORATEBUMP + getCurveSuffix(ccy));
	if (recalc != AQ_NO_DATA)
	{
		AQLDataBool attrBool;
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
			throw AQLCoreInvalidData("Grid size and grid shift size is not same !!", __FILE__, __LINE__); 
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
		AQLStringVector targets = param.targetCurveType.toToken('/');
		AQLString lowerSTD = STD;
		lowerSTD.toLower();
		StringSet names;
		for (size_t i = 0, ie = targets.size(); i < ie; ++i)
		{
			AQLString target = targets[i];
			target.toLower();
			AQLString postfix = (target == lowerSTD) ? "" : "." + target;
			const AQLStringVector assigned = mpStaticData->getStaticData(
				AQLString(ccy).toLower() + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + postfix).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
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

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief create market bump object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] scenario
	@param[in] index
	@return vector<AQLObject *> 
*/
vector<AQLObject *> 
LARiskConfigurationYieldBasisDelta::createMarketBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	const AQLString model = LAMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	AQLStringVector grid = getShiftGridTerm(ccy);
	AQLStringVector BucketTerm = getBucketGridTerm(ccy);

	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}	
	//check bucket grid
	if (BucketTerm[0] != AQ_NO_DATA)
	{
		AQLStringVector::iterator it;
		for (unsigned int i = 0;i < BucketTerm.size();++i)
		{
			it = find(grid.begin(),grid.end(),BucketTerm[i]);
			if (it == grid.end())
				throw AQLCoreInvalidData("Basis delta fail. Risk property bucketterm grid isn't registrate.", __FILE__, __LINE__);

			if (i != 0)
			{
				AQLStringVector::const_iterator it_now,it_before;
				it_now = find(grid.begin(),grid.end(),BucketTerm[i]);
				it_before = find(grid.begin(),grid.end(),BucketTerm[i-1]);
				if (it_before >= it_now)
				{
					throw AQLCoreInvalidData("Basis delta fail. The order of bucket term is inconsistent!", __FILE__, __LINE__);
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
	AQLStringVector paramGrid(gridSize);
	IntArray gridGroupID(gridSize);
	unsigned int j = 0;
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		paramGrid[i] = LAMarketData::convertToMLibTerm(grid[i]);
		//for bucket grid
		if (BucketTerm[0] == "NONE" ||BucketTerm[0] == AQ_NO_DATA)
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

	AQLString mktSuffix = "." + param.targetCurveType;
	mktSuffix.toLower();
	AQLString tmpCcy = ccy;
	tmpCcy.toLower();
	AQLString fwdFXPipsizeFactorString = mpStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_DENOMINATOR + mktSuffix,
																	tmpCcy + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_PIPSIZE + mktSuffix); // Alias Method: First Parameter Takes Priority
	if (fwdFXPipsizeFactorString != AQ_NO_DATA)
		param.fwdfxDenominator = fwdFXPipsizeFactorString.getDoubleValue();

	// set targetCurveType for col-xccybasis delta
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	const LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());
	AQLStringVector fCurveCcys = ycPro.getAffectingCcy();
	for (int unsigned i = 0; i < fCurveCcys.size(); ++i)
	{
		AQLStringVector tmpMarkets = mpStaticData->getStaticData(fCurveCcys[i].toLower() + STATIC_DATA_KEY_YIELD_USEMAKETS).toToken(':');
		if (tmpMarkets[0] == AQ_NO_DATA) continue;
		for (int i = 0; i < tmpMarkets.size(); ++i)
		{
			const AQLStringVector tmpMarket = tmpMarkets[i].toUpper().toToken('_');
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
			throw AQLCoreInvalidData("Grid size and parallel shift size is not same !!", __FILE__, __LINE__); 
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
			throw AQLCoreInvalidData("Grid size and grid shift size is not same !!", __FILE__, __LINE__); 
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

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

 

/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYieldBasisDelta::getOutPutName1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isGridSensitivity(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
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
LARiskConfigurationYieldBasisDelta::getIMMFwdRiskMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString isIMMProp = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
											STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISIMMFWDRATEBUMP + getCurveSuffix(ccy));
	if ((isIMMProp == AQ_NO_DATA) || !convertBoolFromStr(isIMMProp))
	{
		return 0;
	}
	AQLString isZeroRateBump = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
											STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	return convertBoolFromStr(isZeroRateBump) ? 2 : 1;
}

std::vector<int>
LARiskConfigurationYieldBasisDelta::getIMMTerm(const AQLString &ccy) const
{
	if (getIMMFwdRiskMode(ccy))
	{
		AQLDataInts immTerm;
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
AQLStringVector
LARiskConfigurationYieldBasisDelta::getIMMRiskYieldCurveName(const AQLString &ccy) const
{
	if (!getIMMFwdRiskMode(ccy))
	{
		std::vector<AQLString> v;
		v.push_back("");
		return v;
	}

	AQLString ccyL = ccy;
	ccyL.toLower();
	AQLString curveSuffix = getCurveSuffix(ccy);

	AQLString assignedCurvePropName = ccyL + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + curveSuffix;

	AQLString assignedCurve = mpStaticData->getStaticData(assignedCurvePropName).toToken(MULTI_STATIC_DATA_DELIMITER).front().toUpper();
	bool hasAssignedCurve = assignedCurve != AQ_NO_DATA;
	AQLString curveTypeL = getCurveType(ccy).toLower();
	AQLString curveTypeU = curveTypeL;
	curveTypeU.toUpper();

	AQLDataBool isSwapTenorAdjustAttr;
	isSwapTenorAdjustAttr.convertFromString(mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST));
	if (isSwapTenorAdjustAttr.get())
	{
		AQLString tenorSwapName = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME).toLower();
		if (tenorSwapName == curveTypeL)
		{
			AQLStringVector useGrid = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_SWAP_USEGRID).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
			StringSet useGridSet(useGrid.begin(), useGrid.end());
			AQLString baseFrequencyFloat = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT).toLower();
			AQLString frequencyFloat = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT).toLower();
			AQLStringVector useMarkets = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_USEMAKETS).toLower().toToken(MULTI_STATIC_DATA_DELIMITER);
			bool inUseMarkets = std::find(useMarkets.begin(), useMarkets.end(), curveTypeL) != useMarkets.end();

			AQLStringVector terms = mpRiskStaticData->getStaticData(ccyL + STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_GRID_TERM + curveSuffix).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
			AQLStringVector riskYieldCurveNames(terms.size());
			for (AQLStringVector::size_type i = 0; i < terms.size(); ++i)
			{
				if (useGridSet.find(terms[i]) != useGridSet.end())
				{
					AQLString frequencyFloatTerm = mpStaticData->getStaticData(ccyL + STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT + '.' + terms[i].toLower());
					if (frequencyFloatTerm != AQ_NO_DATA)
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
		return std::vector<AQLString>();
	}

	std::vector<AQLString> riskYieldCurveNames;
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
AQLString
LARiskConfigurationYieldBasisDelta::getIMMRiskFloorTerm(const AQLString &ccy, AQLObjectPool &objPool) const
{
	if (getIMMFwdRiskMode(ccy))
	{
		AQLString proName = LAMarketData::getBaseYieldProName(ccy);
		const LAMathYieldCurvePro &pro = dynamic_cast<const LAMathYieldCurvePro&>(objPool.getObject(proName).get());

		// For example, "USDOIS" -> "OISCURVE"
		AQLString originalCurveName = pro.getMarketForCurve(getIMMRiskYieldCurveName(ccy).front()).toUpper();
		if (originalCurveName == OISCURVE)
		{
			if ((mpStaticData->getStaticData((ccy + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + '.' + OISCURVE).toLower()).toUpper() == LOBASIS)
			 && (mpStaticData->getStaticData((ccy + STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME + '.' + OISCURVE).toLower()).toUpper() == getCurveType(ccy).toUpper()))
			{
				AQLStringVector useGrid = mpStaticData->getStaticData((ccy + STATIC_DATA_KEY_YIELD_OIS_USEGRID + '.' + OISCURVE).toLower()).toToken(MULTI_STATIC_DATA_DELIMITER);
				AQLString longTerm = mpStaticData->getStaticData((ccy + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + '.' + OISCURVE).toLower());
				AQLStringVector::const_iterator i = std::find(useGrid.begin(), useGrid.end(), longTerm);
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
LARiskConfigurationYieldBasisDelta::isParallelShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISPARALLEL + getCurveSuffix(ccy)));
}


/*!
    @brief return riskname

	@return AQLString
*/
AQLString
LARiskConfigurationYieldBasisDelta::getRiskName(void) const
{
	return RISK_FRONT_YIELD_BASISDELTA;
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
LARiskConfigurationYieldBasisDelta::getShiftGridTerm(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_GRID_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
LARiskConfigurationYieldBasisDelta::getGridTerm(const AQLString &ccy) const
{
	AQLStringVector ret;
	const AQLString curveType = getCurveType(ccy) + "_";
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
		AQLString tmpCurrency = ccy;
		tmpCurrency.toLower();

		// read fwdfx file
		AQLStringMatrix fwdfxDataMtx;
		AQLString fwdfxFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FILE + getCurveSuffix(ccy));
		if (fwdfxFileName != AQ_NO_DATA)
		{
			MAFileAccessor fwdfxFile(LAMarketData::getNumFileName(fwdfxFileName));
			fwdfxFile.readAllData(MARKET_DATA_DELIMITER, fwdfxDataMtx);
			fwdfxFile.close();
		}
		// read basis file
		AQLStringMatrix basisDataMtx;
		AQLString basisFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + getCurveSuffix(ccy));
		if (basisFileName != AQ_NO_DATA)
		{
			MAFileAccessor basisFile(LAMarketData::getNumFileName(basisFileName));
			basisFile.readAllData(MARKET_DATA_DELIMITER, basisDataMtx);
			basisFile.close();
		}

		// get terms
		const unsigned int fwdfxSize = fwdfxDataMtx.size();
		const unsigned int basisSize = basisDataMtx.size();
		AQLStringVector fwdfx(fwdfxSize);
		AQLStringVector basis(basisSize);
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
	@return vector<AQLString>
*/
vector<AQLString>
LARiskConfigurationYieldBasisDelta::getBucketGridTerm(const AQLString &ccy) const
{
	AQLStringVector ret = getGridRangeTerm(ccy);
	if (!ret.empty()) return ret;
	AQLString tmpccy = ccy;
	AQLString strBucketGrid = mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BUCKET_GRID_TERM + getCurveSuffix(ccy));
	AQLStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" ||BucketTerm[0] == AQ_NO_DATA)
	{
		ret.push_back(AQ_NO_DATA);
		return ret;
	}
	else
	{
		unsigned int gridMax = getMaxGridIndex(ccy);
		AQLStringVector tmpgridTerm = getShiftGridTerm(ccy);
	
		for (unsigned int i = 0;i < BucketTerm.size();++i)
		{
			AQLStringVector::iterator it;
			AQLString strgrid = BucketTerm[i].toUpper();
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
	@return vector<AQLString>
*/
AQLString
LARiskConfigurationYieldBasisDelta::getPropertyBucketGridTerm(const AQLString &ccy) const
{
	AQLString tmpccy = ccy;
	return mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BUCKET_GRID_TERM + getCurveSuffix(ccy));
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@param[in] vals
*/
void
LARiskConfigurationYieldBasisDelta::getScenario1ParallelShift(const AQLString &ccy, DoubleArray &vals) const
{
	vals.clear();
	AQLString tmpCurrency = ccy;
	AQLString strShift = getScenario1ParallelShiftStr(ccy);

	vals =  convertToRateValues(strShift.toToken(MULTI_STATIC_DATA_DELIMITER));

	AQLString shiftType = getShiftType(ccy);
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
	@return AQLString
*/
AQLString 
LARiskConfigurationYieldBasisDelta::getScenario1ParallelShiftStr(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_PARALLEL_SHIFTVAL + getCurveSuffix(ccy));
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
void
LARiskConfigurationYieldBasisDelta::getScenario2ParallelShift(const AQLString &ccy, DoubleArray &vals) const
{
	getScenario1ParallelShift(ccy, vals);
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldBasisDelta::getScenario1ParallelShift(const AQLString &ccy) const
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
LARiskConfigurationYieldBasisDelta::getScenario2ParallelShift(const AQLString &ccy) const
{
	return getScenario1ParallelShift(ccy);
}

/*!
    @brief return scenario1 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldBasisDelta::getScenario1GridShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strShift = getScenario1GridShiftStr(ccy);

	DoubleArray ret =  convertToRateValues(strShift.toToken(MULTI_STATIC_DATA_DELIMITER));

	AQLString shiftType = getShiftType(ccy);
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
	@return AQLString
*/
AQLString
LARiskConfigurationYieldBasisDelta::getScenario1GridShiftStr(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
	    						STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_GRID_SHIFTVAL + getCurveSuffix(ccy));


}

/*!
    @brief return scenario2 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldBasisDelta::getScenario2GridShift(const AQLString &ccy) const
{
	return getScenario1GridShift(ccy);
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYieldBasisDelta::getBumpDirection(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BUMPDIRECTION + getCurveSuffix(ccy));

}


/*!
	@brief is forward FX zero rate bump

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isFwdFXZeroRateBump(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString tmpIsFwdFXZeroRateBump = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
		STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISFWDFXZERORATEBUMP + getCurveSuffix(ccy));
	if (tmpIsFwdFXZeroRateBump == AQ_NO_DATA) return false;
	return convertBoolFromStr(tmpIsFwdFXZeroRateBump);
}

/*!
	@brief is 1 forward FX point bump

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isFwdPointBump(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString tmpIsFwdPointBump = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
		STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISFWDPOINTBUMP + getCurveSuffix(ccy));
	if (tmpIsFwdPointBump == AQ_NO_DATA) return true;
	return convertBoolFromStr(tmpIsFwdPointBump);
}

/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isWave(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief return shift type

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYieldBasisDelta::getShiftType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SHIFTTYPE + getCurveSuffix(ccy));

}

/*!
    @brief return basis type

	@param[in] ccy
	@return AQLString
*/
AQLString
LARiskConfigurationYieldBasisDelta::getBasisType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BASISTYPE + getCurveSuffix(ccy));

}


/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
LARiskConfigurationYieldBasisDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_BASISDELTA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
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
LARiskConfigurationYieldBasisDelta::getDivUnit(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strDivUnit = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
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
LARiskConfigurationYieldBasisDelta::isTarget(const AQLString &ccy) const
{
	AQLString basisCurrency = mpStaticData->getStaticData(KEY_SDE_BASIS_BASE_CURRENCY);
	AQLString tmpCurrency = ccy;
	tmpCurrency.toUpper();

	if (basisCurrency != AQ_NO_DATA)
	{
		if (basisCurrency.toUpper() == tmpCurrency)
		{
			return false;
		}
	}

	AQLString targetccys = getTargetCurrencies();
	targetccys.toUpper();

	if (targetccys == "ALL")
	{
		return true;
	}
	else
	{
		AQLStringVector targetVec = targetccys.toToken(MULTI_STATIC_DATA_DELIMITER);
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
LARiskConfigurationYieldBasisDelta::isRiskCurrencyMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);

}


/*!
    @brief check zero rate bump

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldBasisDelta::isZeroBump(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	if (proprslt == AQ_NO_DATA)
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
vector<AQLObject *>
LARiskConfigurationYieldBasisDelta::createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	if (omitNotionalExposure(ccy))
	{
		return createBasisBaseScenarioEntity(ccy,dataInstance,index);
	}
	else
	{
		return std::vector<AQLObject*>(0);
	}
}

/*!
    @brief return baseoutputname

	@param[in] ccy
	@param[in] index
	@return AQLString
*/
AQLString
LARiskConfigurationYieldBasisDelta::getBaseOutPutName(const AQLString &ccy , int index) const
{
	if (omitNotionalExposure(ccy))
	{
		AQLString tmpCcy = ccy;
		tmpCcy.toLower();
		AQLString ret = mpRiskStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_OUTPUTNAME + getCurveSuffix(ccy));
		ret += AQLString("_BasePV");
		return ret;
	}
	else
	{
		return AQ_NO_DATA;
	}
}

/*!
    @brief return shift type

	@param[in] fx
	@return AQLString
*/
bool
LARiskConfigurationYieldBasisDelta::omitNotionalExposure(const AQLString &ccy) const
{
	AQLString tmpCcy = ccy;
	tmpCcy.toLower();
	AQLString omitNotionalExposure = mpRiskStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_OMITNOTIONALEXPOSURE + getCurveSuffix(ccy));
	if (omitNotionalExposure == AQ_NO_DATA)
	{
		return false;
	}
	else
	{
		return convertBoolFromStr(omitNotionalExposure);
	}
}
