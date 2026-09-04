/*! @file
    @brief  IR Delta setup class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <functional>
#include <algorithm>
#include "AQLRiskConfigurationYieldIRDelta.h"
#include "AQLDataInstance.h"
#include "AQLBasic.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "AQLDefinitionsRisk.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLFileAccessor.h"
#include "AQLMarketData.h"
#include "AQLLinearFunc.h"
#include "AQLStaticData.h"
#include "AQLDealUtils.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationYieldIRDelta::AQLRiskConfigurationYieldIRDelta(void)
:AQLRiskConfigurationYield()
{
}

// destructor
/*!

*/
AQLRiskConfigurationYieldIRDelta::~AQLRiskConfigurationYieldIRDelta(void)
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
AQLRiskConfigurationYieldIRDelta::createYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
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
AQLRiskConfigurationYieldIRDelta::createZeroBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	const AQLString model = AQLMarketData::getModelName(ccy);
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
		throw  AQLCoreInvalidData("IR delta fail. Zero rate bump does not support bucket.", __FILE__, __LINE__);
	}
	// shift type
	AQLString shiftType = getShiftType(ccy);
	shiftType.toUpper();
	if (shiftType != RISK_SHIFTTYPE_DIFF)
	{
		throw  AQLCoreInvalidData("IR delta fail. Zero rate bump supports diff only.", __FILE__, __LINE__);
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
	AQLScenarioParam param;
	param.ccy = ccy;
	param.calcType= ccy + "_" + riskName + "_" + AQLString(scenarioNum) + "_" + AQLString(index);
	param.model = model;
	param.shiftType = shiftType;
	param.bumpDirection = bumpDirection;
	param.targetName = AQLMarketData::getBaseYieldName(ccy);
	param.targetCurveType = getCurveType(ccy);
	param.isBusinessDayRoll = true;
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();
	param.calendar = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MONEYMARKET_CALENDAR);
	AQLString recalc = mpRiskStaticData->getStaticData(
		tmpCurrency + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_RECALCBASISDFSONZERORATEBUMP + getCurveSuffix(ccy));
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
		// curve types are specified as market names (for instance, OISCURVE instead of AUDOIS/AUDDISCOUNT);
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
				AQLString(ccy).toLower() + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + postfix).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
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
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELDZERO);

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
AQLRiskConfigurationYieldIRDelta::createMarketBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
{
	const AQLString model = AQLMarketData::getModelName(ccy);
	const AQLString riskName = getRiskName();
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	
	// if scenario2 only updownshift
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject *>(0);
	}	
	//////////////////////////////////////////////////
	// property file grid and market rate grid check
	//////////////////////////////////////////////////
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();


	AQLString oismethod = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + getCurveSuffix(ccy)).toUpper();
	bool isoismode  = (oismethod != AQ_NO_DATA);
    AQLStringVector fMarketTerms;
    getMarketTerms(tmpCurrency, fMarketTerms);
    const bool onValFlg = find(fMarketTerms.begin(), fMarketTerms.end(), "ON")!=fMarketTerms.end();
    const bool tnValFlg = find(fMarketTerms.begin(), fMarketTerms.end(), "TN")!=fMarketTerms.end();
	sort(fMarketTerms.begin(), fMarketTerms.end());
	AQLStringVector::iterator endIt = unique(fMarketTerms.begin(), fMarketTerms.end());
	fMarketTerms.erase(endIt, fMarketTerms.end());

	const AQLStringVector grid = getShiftGridTerm(tmpCurrency);
	AQLStringVector checkGrid = grid;
	sort(checkGrid.begin(), checkGrid.end());
	// for bucket term
	AQLStringVector BucketTerm = getBucketGridTerm(tmpCurrency);
	vector<int> pos;
	if (BucketTerm[0] == AQ_NO_DATA)
	{
		const unsigned int checkSize = checkGrid.size();
		if (checkSize != fMarketTerms.size())
		{
			throw AQLCoreInvalidData("IR delta fail. Risk property term grid  and market rate term grid is not same.", __FILE__, __LINE__);
		}

		for (unsigned int i = 0; i < checkSize; ++i)
		{
			if (checkGrid[i].toUpper() != fMarketTerms[i].toUpper())
			{
				throw AQLCoreInvalidData("IR delta fail. Risk property term grid  and market rate term grid is not same.", __FILE__, __LINE__);
			}
		}
	}
	else
	{	
		// bucket grid case 
		//pos.push_back(0);
		AQLStringVector::iterator it;

		for (unsigned int i = 0;i < BucketTerm.size();++i)
		{
			it = find(checkGrid.begin(),checkGrid.end(),BucketTerm[i]);
			if (it == checkGrid.end())
			{
				throw AQLCoreInvalidData("IR delta fail. Risk property bucketterm grid isn't registrate.", __FILE__, __LINE__);
			}

			if (i != 0)
			{
				AQLStringVector::const_iterator it_now,it_before;
				it_now = find(grid.begin(),grid.end(),BucketTerm[i]);
				it_before = find(grid.begin(),grid.end(),BucketTerm[i-1]);
				if (it_before >= it_now)
				{
					throw AQLCoreInvalidData("IR delta fail. The order of bucket term is inconsistent!", __FILE__, __LINE__);
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

	// get base shift
	DoubleArray baseShiftVal = getBaseShifts(ccy, index);
	const unsigned int gridSize = grid.size();
	if (baseShiftVal.empty())
	{
		baseShiftVal.resize(gridSize, 0.0);
	}
	else
	{
		if (baseShiftVal.size() != gridSize)
		{
			throw AQLCoreInvalidData("Base shift grid is different from curve grid", __FILE__, __LINE__);
		}		
	}

	AQLStringVector paramGrid = grid;
	double firstValB = baseShiftVal[0];
	double firstValP = paraShiftVec[0];
	double firstValG = gridShiftVec[0];
	unsigned int maxIndex = getMaxGridIndex(ccy);
	// ON check
	if (!isoismode && !onValFlg)
	{
		paramGrid.insert(paramGrid.begin(), "ON");
		baseShiftVal.insert(baseShiftVal.begin(), firstValB);
		paraShiftVec.insert(paraShiftVec.begin(), firstValP);
		gridShiftVec.insert(gridShiftVec.begin(), firstValG);
		++maxIndex;
	}
	// TN check
	if (!isoismode && !tnValFlg)
	{
		paramGrid.insert(++paramGrid.begin(), "TN");
		baseShiftVal.insert(++baseShiftVal.begin(), firstValB);
		paraShiftVec.insert(++paraShiftVec.begin(), firstValP);
		gridShiftVec.insert(++gridShiftVec.begin(), firstValG);
		++maxIndex;
	}

	// convert grid for M-Lib
	unsigned int id = 0;
	unsigned int j = 0;
	IntArray gridGroupID(paramGrid.size());
	bool updateFlg = false;
	for (unsigned int i = 0; i < paramGrid.size(); ++i)
	{
		AQLString term = AQLMarketData::aqlConvertToTerm(paramGrid[i]);
		term.toUpper();

		// check shift group id
		if (!isoismode)
		{
			if (term == "ON")
			{
				id = 0;
			}
			else if (term == "TN")
			{
				if (onValFlg)
				{
					++id;
				}
			}
			else
			{
				if (!updateFlg)
				{
					if (tnValFlg)
					{
						++id;
					}
					updateFlg = true;
				}
				else
				{
					++id;
				}
			}
		}
		else
		{
			if (!updateFlg)
			{
				updateFlg = true;
			}
			else
			{
				++id;
			}
		}

		paramGrid[i] = term;
		//for bucket grid
		if (BucketTerm[0] == AQ_NO_DATA)
		{
			gridGroupID[i] = id;
		}
		else
		{
			//bucket mode
			//AQLString strterm;
			gridGroupID[i] = j;
			
			if(j != BucketTerm.size() && term == BucketTerm[j])
				++j;

		}
	}

	// set up param
	AQLScenarioParam param;
	param.ccy = ccy;
	//param.calcType= ccy + "_" + riskName + "_" + AQLString(scenarioNum) + "_" + AQLString(index);
	param.calcType= getCalcType(ccy, scenarioNum, index);
	param.model = model;
	param.shiftType = getShiftType(ccy);
	param.bumpDirection = getBumpDirection(ccy);
	param.targetName = AQLMarketData::getBaseYieldName(ccy);
	param.baseShiftVal = baseShiftVal;
	param.isAdjustDF = isAdjustDf(ccy);
	param.targetCurveType = getCurveType(ccy);

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
		param.maxIndex = maxIndex;
		param.gridShiftVec = gridShiftVec;
	}

	// create scenario
	AQLScenarioConfiguration *sceCreator = 
		AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<AQLObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

 
/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationYieldIRDelta::getShiftGridTerm(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_GRID_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRDelta::getOutPutName1(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRDelta::isGridSensitivity(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
												STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISGRIDSENSITIVITY + getCurveSuffix(ccy)));
}

/*!
    @Description: Tells IMM forward risk mode
    @param ccy [in] The currency
    @return 0 if no IMM-forward-risk-like care is applied,
            1 if market rate bump risk -> IMM forward risk conversion is applied,
            2 if market zero rate bump risk -> IMM forward risk conversion is applied
*/
int
AQLRiskConfigurationYieldIRDelta::getIMMFwdRiskMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString isIMMProp = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
											STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISIMMFWDRATEBUMP + getCurveSuffix(ccy));
	if ((isIMMProp == AQ_NO_DATA) || !convertBoolFromStr(isIMMProp))
	{
		return 0;
	}
	AQLString isZeroRateBump = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
											STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	return convertBoolFromStr(isZeroRateBump) ? 2 : 1;
}

std::vector<int>
AQLRiskConfigurationYieldIRDelta::getIMMTerm(const AQLString &ccy) const
{
	if (getIMMFwdRiskMode(ccy))
	{
		AQLStaticData& riskProp = AQLStaticDataManager::getRiskStaticData();
		AQLDataInts immTerm;
		immTerm.convertFromString(riskProp.getStaticData(
			(ccy + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_IMMFWDRATETERM).toLower() + getCurveSuffix(ccy)));
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
            AQLMathYieldCurvePro::setCurveType (for example, "JPYOIS") to deduce the forward rate changes
            against the market rate bumps
*/
AQLStringVector
AQLRiskConfigurationYieldIRDelta::getIMMRiskYieldCurveName(const AQLString &ccy) const
{
	std::vector<AQLString> riskYieldCurveNames;
	if (getIMMFwdRiskMode(ccy))
	{
		AQLString propName = (ccy + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE).toLower() + getCurveSuffix(ccy);
		AQLDataString riskYieldCurveName;
		riskYieldCurveName.convertFromString(AQLStaticDataManager::getStaticData().getStaticData(
			propName).toToken(MULTI_STATIC_DATA_DELIMITER).front());
		if (riskYieldCurveName.get() == AQ_NO_DATA)
		{
#ifndef NDEBUG
cout << "Property " << propName.getCString() << " is not set but "
     << (ccy + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISIMMFWDRATEBUMP).toLower() << getCurveSuffix(ccy).getCString() << " is set to TRUE" << endl;
#endif
			// Leave riskYieldCurveNames empty
		}
		else
		{
			riskYieldCurveNames.push_back(riskYieldCurveName);
		}
	}
	else
	{
		riskYieldCurveNames.push_back("");
	}
	return riskYieldCurveNames;
}

/*!
    @brief return isparallelshift

	@param[in] ccy
	@return bool 
*/
bool
AQLRiskConfigurationYieldIRDelta::isParallelShift(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISPARALLEL + getCurveSuffix(ccy)));
}


/*!
    @brief return riskname

	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRDelta::getRiskName(void) const
{
	return RISK_FRONT_YIELD_IRDELTA;
}


/*!
    @brief return grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationYieldIRDelta::getGridTerm(const AQLString &ccy) const
{
	if (isZeroBump(ccy))
	{
		return getShiftGridTerm(ccy);
	}
	else
	{
		AQLString tmpCurrency = ccy;
		tmpCurrency.toLower();
		AQLString oismethod = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + getCurveSuffix(ccy)).toUpper();
		if (oismethod != AQ_NO_DATA)
		{
			AQLStringVector boj;
			AQLStringVector fedfund;
			AQLStringVector ois;
			AQLStringVector on;
			AQLStringVector ret;
			// read fedfund file
			AQLStringMatrix fedfundDataMtx;
			AQLString fedfundFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + getCurveSuffix(ccy));
			if (fedfundFileName != AQ_NO_DATA)
			{
				AQLFileAccessor fedfundFile(AQLMarketData::getNumFileName(fedfundFileName));
				fedfundFile.readAllData(MARKET_DATA_DELIMITER, fedfundDataMtx);
				fedfundFile.close();
			}
			const unsigned int fedfundSize = fedfundDataMtx.size();

			// read ois file
			AQLString oisFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + getCurveSuffix(ccy));
			AQLFileAccessor oisFile(AQLMarketData::getNumFileName(oisFileName));
			AQLStringMatrix oisDataMtx;
			oisFile.readAllData(MARKET_DATA_DELIMITER, oisDataMtx);
			oisFile.close();
			const unsigned int oisSize = oisDataMtx.size();

			for (unsigned int i = 0; i < fedfundSize; ++i)
			{
				AQLString term = fedfundDataMtx[i][0].toUpper();
				fedfund.push_back(term);
			}
			for (unsigned int i = 0; i < oisSize; ++i)
			{
				AQLString term = oisDataMtx[i][0].toUpper();
				if (term.findString("BOJ") >= 0)
				{
					boj.push_back(term);
				}
				
				if (term.findString("ON") >= 0)
				{
					on.push_back(term);
				}
				else
				{
					ois.push_back(term);
				}
			}

			for (unsigned int i = 0; i < on.size(); ++i)
			{
				ret.push_back("OIS_" + on[i]);
			}
			for (unsigned int i = 0; i < fedfund.size(); ++i)
			{
				ret.push_back("FUTURE_"/*"OIS_"*/ + fedfund[i]);
			}
			for (unsigned int i = 0; i < ois.size(); ++i)
			{
				ret.push_back("OIS_" + ois[i]);
			}

			if (!boj.empty() && !fedfund.empty())
			{
				throw AQLCoreInvalidData("fed fund rate and boj rate is not input simultaneously", __FILE__, __LINE__);
			}
			return ret;
		}

		// read libor file
		AQLString liborFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + getCurveSuffix(ccy));
		AQLFileAccessor liborFile(AQLMarketData::getNumFileName(liborFileName));
		AQLStringMatrix liborDataMtx;
		liborFile.readAllData(MARKET_DATA_DELIMITER, liborDataMtx);
		liborFile.close();

		// read fra file
		AQLStringMatrix fra3DataMtx,fra6DataMtx,futureDataMtx;
		AQLString fra3FileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + getCurveSuffix(ccy));
		if (fra3FileName != AQ_NO_DATA)
		{
			AQLFileAccessor fra3File(AQLMarketData::getNumFileName(fra3FileName));
			fra3File.readAllData(MARKET_DATA_DELIMITER, fra3DataMtx);
			fra3File.close();
		}
		AQLString fra6FileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + getCurveSuffix(ccy));
		if (fra6FileName != AQ_NO_DATA)
		{
			AQLFileAccessor fra6File(AQLMarketData::getNumFileName(fra6FileName));
			fra6File.readAllData(MARKET_DATA_DELIMITER, fra6DataMtx);
			fra6File.close();
		}
		AQLString futureFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + getCurveSuffix(ccy));
		if (futureFileName != AQ_NO_DATA)
		{
			AQLFileAccessor futureFile(AQLMarketData::getNumFileName(futureFileName));
			futureFile.readAllData(MARKET_DATA_DELIMITER, futureDataMtx);
			futureFile.close();
		}

		// read swap file
		AQLString swapFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + getCurveSuffix(ccy));
		AQLFileAccessor swapFile(AQLMarketData::getNumFileName(swapFileName));
		AQLStringMatrix swapDataMtx;
		swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
		swapFile.close();

		const unsigned int liborSize = liborDataMtx.size();
		const unsigned int swapSize = swapDataMtx.size();
		const unsigned int fra3Size = fra3DataMtx.size();
		const unsigned int fra6Size = fra6DataMtx.size();
		const unsigned int futureSize = futureDataMtx.size();
		AQLStringVector libor(liborSize);
		AQLStringVector swap(swapSize);
		AQLStringVector fra3m(fra3Size);
		AQLStringVector fra6m(fra6Size);
		AQLStringVector future(futureSize);
		for (unsigned int i = 0; i < liborSize; ++i)
		{
			libor[i] = liborDataMtx[i][0].toUpper();
		}
		for (unsigned int i = 0; i < fra3Size; ++i)
		{
			fra3m[i] = fra3DataMtx[i][0].toUpper();
		}
		for (unsigned int i = 0; i < fra6Size; ++i)
		{
			fra6m[i] = fra6DataMtx[i][0].toUpper();
		}
		for (unsigned int i = 0; i < futureSize; ++i)
		{
			future[i] = futureDataMtx[i][0].toUpper();
		}
		for (unsigned int i = 0; i < swapSize; ++i)
		{
			swap[i] = swapDataMtx[i][0].toUpper();
		}
		sort(libor.begin(), libor.end(), AQLComp_StrTerm());
		sort(swap.begin(), swap.end(), AQLComp_StrTerm());
		sort(fra3m.begin(), fra3m.end(), AQLComp_StrTerm());
		sort(fra6m.begin(), fra6m.end(), AQLComp_StrTerm());
		sort(future.begin(), future.end(), AQLComp_StrTerm());

		const unsigned int marketSize = liborSize + swapSize+ fra3Size + fra6Size + futureSize;
		AQLStringVector ret(marketSize);
		for (unsigned int i = 0; i < liborSize; ++i)
		{
			ret[i] =  "LIBOR_" + libor[i];
		}
		for (unsigned int i = liborSize; i < liborSize + fra3Size; ++i)
		{
			ret[i] =  "FRA3M_" + fra3m[i - liborSize];
		}
		for (unsigned int i = liborSize + fra3Size; i < liborSize + fra3Size + fra6Size; ++i)
		{
			ret[i] =  "FRA6M_" + fra6m[i - liborSize - fra3Size];
		}
		for (unsigned int i = liborSize + fra3Size + fra6Size; i < liborSize + fra3Size + fra6Size + futureSize; ++i)
		{
			ret[i] =  "FUTURE_" + future[i - liborSize - fra3Size - fra6Size];
		}
		for (unsigned int i = liborSize + fra3Size + fra6Size + futureSize; i < marketSize; ++i)
		{
			ret[i] =  "SWAP_" + swap[i - liborSize - fra3Size - fra6Size - futureSize];
		}

		return ret;

	}
}

/*!
    @brief return bucket grid term

	@param[in] ccy
	@return vector<AQLString>
*/
vector<AQLString>
AQLRiskConfigurationYieldIRDelta::getBucketGridTerm(const AQLString &ccy) const
{
	AQLStringVector ret = getGridRangeTerm(ccy);
	if (!ret.empty()) return ret;
	AQLString tmpccy = ccy;
	AQLString strBucketGrid = mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_BUCKET_TERM + getCurveSuffix(ccy));
	AQLStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == AQ_NO_DATA)
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
AQLRiskConfigurationYieldIRDelta::getPropertyBucketGridTerm(const AQLString &ccy) const
{
	AQLString tmpccy = ccy;
	return mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_BUCKET_TERM + getCurveSuffix(ccy));
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@param[in] vals
*/
void
AQLRiskConfigurationYieldIRDelta::getScenario1ParallelShift(const AQLString &ccy, DoubleArray &vals) const
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
AQLRiskConfigurationYieldIRDelta::getScenario1ParallelShiftStr(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_PARALLEL_SHIFTVAL + getCurveSuffix(ccy));
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
void
AQLRiskConfigurationYieldIRDelta::getScenario2ParallelShift(const AQLString &ccy, DoubleArray &vals) const
{
	getScenario1ParallelShift(ccy, vals);
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationYieldIRDelta::getScenario1ParallelShift(const AQLString &ccy) const
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
AQLRiskConfigurationYieldIRDelta::getScenario2ParallelShift(const AQLString &ccy) const
{
	return getScenario1ParallelShift(ccy);
}

/*!
    @brief return scenario1 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRDelta::getScenario1GridShift(const AQLString &ccy) const
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
AQLRiskConfigurationYieldIRDelta::getScenario1GridShiftStr(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
	    						STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_GRID_SHIFTVAL + getCurveSuffix(ccy));


}

/*!
    @brief return scenario2 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
AQLRiskConfigurationYieldIRDelta::getScenario2GridShift(const AQLString &ccy) const
{
	return getScenario1GridShift(ccy);
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRDelta::getBumpDirection(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_BUMPDIRECTION + getCurveSuffix(ccy));

}


/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRDelta::isWave(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief return shift type

	@param[in] ccy
	@return AQLString
*/
AQLString
AQLRiskConfigurationYieldIRDelta::getShiftType(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SHIFTTYPE + getCurveSuffix(ccy));

}


/*!
    @brief return target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRDELTA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return AQLString 
*/
AQLString
AQLRiskConfigurationYieldIRDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRDELTA_CALIBRATION_TARGET_CURRENCY);
}


/*!
    @brief return divid unit

	@param[in] ccy
	@return double
*/
double
AQLRiskConfigurationYieldIRDelta::getDivUnit(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	AQLString strDivUnit = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_DIVUNIT + getCurveSuffix(ccy));

	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
AQLRiskConfigurationYieldIRDelta::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRDELTA_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief returnisRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
AQLRiskConfigurationYieldIRDelta::isRiskCurrencyMode(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISRISKCURRENCYMODE);
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
AQLRiskConfigurationYieldIRDelta::isZeroBump(const AQLString &ccy) const
{
	AQLString tmpCurrency = ccy;
	//if MA_NODATA return false;
	AQLString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	if (proprslt == AQ_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}

void 
AQLRiskConfigurationYieldIRDelta::getMarketTerms(const AQLString& ccy, AQLStringVector& terms) const
{
    AQLString ccy_lower = ccy; ccy_lower.toLower();	

	const bool isoismode  = (mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + getCurveSuffix(ccy_lower)).toUpper() != AQ_NO_DATA);

	AQLStringMatrix liborDataMtx, swapDataMtx,fra3DataMtx,fra6DataMtx,futureDataMtx;
	
	//read future file
	AQLString futureFileName = isoismode ? mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + getCurveSuffix(ccy_lower)) :
                                          mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_FUTURE_FILE + getCurveSuffix(ccy_lower));
	if (futureFileName != AQ_NO_DATA)
	{
		AQLFileAccessor futureFile(AQLMarketData::getNumFileName(futureFileName));
		futureFile.readAllData(MARKET_DATA_DELIMITER, futureDataMtx);
		futureFile.close();
	}

	// read swap file
    AQLString swapFileName = isoismode ? mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_OIS_FILE + getCurveSuffix(ccy_lower)) :
                                        mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_SWAP_FILE + getCurveSuffix(ccy_lower));
	AQLFileAccessor swapFile(AQLMarketData::getNumFileName(swapFileName));
	swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
	swapFile.close();

	if (!isoismode)
	{
		// read libor file
		AQLString liborFileName = mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_LIBOR_FILE + getCurveSuffix(ccy_lower));
		AQLFileAccessor liborFile(AQLMarketData::getNumFileName(liborFileName));
		liborFile.readAllData(MARKET_DATA_DELIMITER, liborDataMtx);
		liborFile.close();
		//read fra file
		AQLString fra3FileName = mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_3MFRA_FILE + getCurveSuffix(ccy_lower));
		if (fra3FileName != AQ_NO_DATA)
		{
			AQLFileAccessor fra3File(AQLMarketData::getNumFileName(fra3FileName));
			fra3File.readAllData(MARKET_DATA_DELIMITER, fra3DataMtx);
			fra3File.close();
		}
		AQLString fra6FileName = mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_6MFRA_FILE + getCurveSuffix(ccy_lower));
		if (fra6FileName != AQ_NO_DATA)
		{
			AQLFileAccessor fra6File(AQLMarketData::getNumFileName(fra6FileName));
			fra6File.readAllData(MARKET_DATA_DELIMITER, fra6DataMtx);
			fra6File.close();
		}
	}

	const unsigned int liborSize = liborDataMtx.size();
	const unsigned int swapSize = swapDataMtx.size();
	const int fra3Size = fra3DataMtx.size();
	const int fra6Size = fra6DataMtx.size();
	const int futureSize = futureDataMtx.size();
	const unsigned int fMarketSize = liborSize + swapSize+ fra3Size + fra6Size + futureSize;

	terms.resize(fMarketSize);

	for (unsigned int i = 0; i < liborSize; ++i)
	{
		terms[i] = liborDataMtx[i][0].toUpper();
	}
	for (unsigned int i = liborSize; i < liborSize  + fra3Size; ++i)
	{
		terms[i] = fra3DataMtx[i - liborSize][0].toUpper();
	}
	for (unsigned int i = liborSize  + fra3Size; i < liborSize  + fra3Size + fra6Size; ++i)
	{
		terms[i] = fra6DataMtx[i - liborSize - fra3Size][0].toUpper();
	}
	for (unsigned int i = liborSize  + fra3Size + fra6Size; i < liborSize  + fra3Size + fra6Size + futureSize; ++i)
	{
		terms[i] = futureDataMtx[i - liborSize - fra3Size - fra6Size][0].toUpper();
	}
	for (unsigned int i = liborSize  + fra3Size + fra6Size + futureSize; i < fMarketSize; ++i)
	{
		terms[i] = swapDataMtx[i - liborSize - fra3Size - fra6Size - futureSize][0].toUpper();
	}
    
}

/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<AQLObject *>
AQLRiskConfigurationYieldIRDelta::createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const
{
	if (omitNotionalExposure(ccy))
	{
		return createIRBaseScenarioEntity(ccy,dataInstance,index);
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
AQLRiskConfigurationYieldIRDelta::getBaseOutPutName(const AQLString &ccy , int index) const
{
	if (omitNotionalExposure(ccy))
	{
		AQLString tmpCcy = ccy;
		tmpCcy.toLower();
		AQLString ret = mpRiskStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OUTPUTNAME + getCurveSuffix(ccy));
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
AQLRiskConfigurationYieldIRDelta::omitNotionalExposure(const AQLString &ccy) const
{
	AQLString tmpCcy = ccy;
	tmpCcy.toLower();
	AQLString omitNotionalExposure = mpRiskStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OMITNOTIONALEXPOSURE);
	if (omitNotionalExposure == AQ_NO_DATA)
	{
		return false;
	}
	else
	{
		return convertBoolFromStr(omitNotionalExposure);
	}
}

