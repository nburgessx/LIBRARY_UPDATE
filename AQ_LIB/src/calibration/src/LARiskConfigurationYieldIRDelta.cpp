/*! @file
    @brief  IR Delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRDelta.cpp
//
//  DESCRIPTION :       IR Delta setup class
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
#include "LARiskConfigurationYieldIRDelta.h"
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
LARiskConfigurationYieldIRDelta::LARiskConfigurationYieldIRDelta(void)
:LARiskConfigurationYield()
{
}

// destructor
/*!

*/
LARiskConfigurationYieldIRDelta::~LARiskConfigurationYieldIRDelta(void)
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
LARiskConfigurationYieldIRDelta::createYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
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
LARiskConfigurationYieldIRDelta::createZeroBumpYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
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
		throw  LACoreInvalidData("IR delta fail. Zero rate bump does not support bucket.", __FILE__, __LINE__);
	}
	// shift type
	LAString shiftType = getShiftType(ccy);
	shiftType.toUpper();
	if (shiftType != RISK_SHIFTTYPE_DIFF)
	{
		throw  LACoreInvalidData("IR delta fail. Zero rate bump supports diff only.", __FILE__, __LINE__);
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
	param.isBusinessDayRoll = true;
	LAString tmpCurrency = ccy;
	tmpCurrency.toLower();
	param.calendar = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MONEYMARKET_CALENDAR);
	LAString recalc = mpRiskStaticData->getStaticData(
		tmpCurrency + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_RECALCBASISDFSONZERORATEBUMP + getCurveSuffix(ccy));
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
		// curve types are specified as market names (for instance, OISCURVE instead of AUDOIS/AUDDISCOUNT);
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
				LAString(ccy).toLower() + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + postfix).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
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
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELDZERO);

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
LARiskConfigurationYieldIRDelta::createMarketBumpYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index)  const
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
	//////////////////////////////////////////////////
	// property file grid and market rate grid check
	//////////////////////////////////////////////////
	LAString tmpCurrency = ccy;
	tmpCurrency.toLower();


	LAString oismethod = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + getCurveSuffix(ccy)).toUpper();
	bool isoismode  = (oismethod != MLIB_NO_DATA);
    LAStringVector fMarketTerms;
    getMarketTerms(tmpCurrency, fMarketTerms);
    const bool onValFlg = find(fMarketTerms.begin(), fMarketTerms.end(), "ON")!=fMarketTerms.end();
    const bool tnValFlg = find(fMarketTerms.begin(), fMarketTerms.end(), "TN")!=fMarketTerms.end();
	sort(fMarketTerms.begin(), fMarketTerms.end());
	LAStringVector::iterator endIt = unique(fMarketTerms.begin(), fMarketTerms.end());
	fMarketTerms.erase(endIt, fMarketTerms.end());

	const LAStringVector grid = getShiftGridTerm(tmpCurrency);
	LAStringVector checkGrid = grid;
	sort(checkGrid.begin(), checkGrid.end());
	// for bucket term
	LAStringVector BucketTerm = getBucketGridTerm(tmpCurrency);
	vector<int> pos;
	if (BucketTerm[0] == MLIB_NO_DATA)
	{
		const unsigned int checkSize = checkGrid.size();
		if (checkSize != fMarketTerms.size())
		{
			throw LACoreInvalidData("IR delta fail. Risk property term grid  and market rate term grid is not same.", __FILE__, __LINE__);
		}

		for (unsigned int i = 0; i < checkSize; ++i)
		{
			if (checkGrid[i].toUpper() != fMarketTerms[i].toUpper())
			{
				throw LACoreInvalidData("IR delta fail. Risk property term grid  and market rate term grid is not same.", __FILE__, __LINE__);
			}
		}
	}
	else
	{	
		// bucket grid case 
		//pos.push_back(0);
		LAStringVector::iterator it;

		for (unsigned int i = 0;i < BucketTerm.size();++i)
		{
			it = find(checkGrid.begin(),checkGrid.end(),BucketTerm[i]);
			if (it == checkGrid.end())
			{
				throw LACoreInvalidData("IR delta fail. Risk property bucketterm grid isn't registrate.", __FILE__, __LINE__);
			}

			if (i != 0)
			{
				LAStringVector::const_iterator it_now,it_before;
				it_now = find(grid.begin(),grid.end(),BucketTerm[i]);
				it_before = find(grid.begin(),grid.end(),BucketTerm[i-1]);
				if (it_before >= it_now)
				{
					throw LACoreInvalidData("IR delta fail. The order of bucket term is inconsistent!", __FILE__, __LINE__);
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
			throw LACoreInvalidData("Base shift grid is different from curve grid", __FILE__, __LINE__);
		}		
	}

	LAStringVector paramGrid = grid;
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
		LAString term = LAMarketData::convertToMLibTerm(paramGrid[i]);
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
		if (BucketTerm[0] == MLIB_NO_DATA)
		{
			gridGroupID[i] = id;
		}
		else
		{
			//bucket mode
			//LAString strterm;
			gridGroupID[i] = j;
			
			if(j != BucketTerm.size() && term == BucketTerm[j])
				++j;

		}
	}

	// set up param
	MAScenarioParam param;
	param.ccy = ccy;
	//param.calcType= ccy + "_" + riskName + "_" + LAString(scenarioNum) + "_" + LAString(index);
	param.calcType= getCalcType(ccy, scenarioNum, index);
	param.model = model;
	param.shiftType = getShiftType(ccy);
	param.bumpDirection = getBumpDirection(ccy);
	param.targetName = LAMarketData::getBaseYieldName(ccy);
	param.baseShiftVal = baseShiftVal;
	param.isAdjustDF = isAdjustDf(ccy);
	param.targetCurveType = getCurveType(ccy);

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
		param.maxIndex = maxIndex;
		param.gridShiftVec = gridShiftVec;
	}

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

 
/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldIRDelta::getShiftGridTerm(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strGrid = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_GRID_TERM + getCurveSuffix(ccy));

	return strGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief return outputname1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRDelta::getOutPutName1(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OUTPUTNAME + getCurveSuffix(ccy));
}

/*!
    @brief return isgridsensitivity

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRDelta::isGridSensitivity(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
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
LARiskConfigurationYieldIRDelta::getIMMFwdRiskMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString isIMMProp = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
											STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISIMMFWDRATEBUMP + getCurveSuffix(ccy));
	if ((isIMMProp == MLIB_NO_DATA) || !convertBoolFromStr(isIMMProp))
	{
		return 0;
	}
	LAString isZeroRateBump = mpRiskStaticData->getStaticData(tmpCurrency.toLower() +
											STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	return convertBoolFromStr(isZeroRateBump) ? 2 : 1;
}

std::vector<int>
LARiskConfigurationYieldIRDelta::getIMMTerm(const LAString &ccy) const
{
	if (getIMMFwdRiskMode(ccy))
	{
		LAStaticData& riskProp = LAStaticDataManager::getRiskStaticData();
		LADataInts immTerm;
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
            LAMathYieldCurvePro::setCurveType (for example, "JPYOIS") to deduce the forward rate changes
            against the market rate bumps
*/
LAStringVector
LARiskConfigurationYieldIRDelta::getIMMRiskYieldCurveName(const LAString &ccy) const
{
	std::vector<LAString> riskYieldCurveNames;
	if (getIMMFwdRiskMode(ccy))
	{
		LAString propName = (ccy + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE).toLower() + getCurveSuffix(ccy);
		LADataString riskYieldCurveName;
		riskYieldCurveName.convertFromString(LAStaticDataManager::getStaticData().getStaticData(
			propName).toToken(MULTI_STATIC_DATA_DELIMITER).front());
		if (riskYieldCurveName.get() == MLIB_NO_DATA)
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
LARiskConfigurationYieldIRDelta::isParallelShift(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISPARALLEL + getCurveSuffix(ccy)));
}


/*!
    @brief return riskname

	@return LAString
*/
LAString
LARiskConfigurationYieldIRDelta::getRiskName(void) const
{
	return RISK_FRONT_YIELD_IRDELTA;
}


/*!
    @brief return grid term

	@param[in] ccy
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldIRDelta::getGridTerm(const LAString &ccy) const
{
	if (isZeroBump(ccy))
	{
		return getShiftGridTerm(ccy);
	}
	else
	{
		LAString tmpCurrency = ccy;
		tmpCurrency.toLower();
		LAString oismethod = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + getCurveSuffix(ccy)).toUpper();
		if (oismethod != MLIB_NO_DATA)
		{
			LAStringVector boj;
			LAStringVector fedfund;
			LAStringVector ois;
			LAStringVector on;
			LAStringVector ret;
			// read fedfund file
			LAStringMatrix fedfundDataMtx;
			LAString fedfundFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + getCurveSuffix(ccy));
			if (fedfundFileName != MLIB_NO_DATA)
			{
				MAFileAccessor fedfundFile(LAMarketData::getNumFileName(fedfundFileName));
				fedfundFile.readAllData(MARKET_DATA_DELIMITER, fedfundDataMtx);
				fedfundFile.close();
			}
			const unsigned int fedfundSize = fedfundDataMtx.size();

			// read ois file
			LAString oisFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + getCurveSuffix(ccy));
			MAFileAccessor oisFile(LAMarketData::getNumFileName(oisFileName));
			LAStringMatrix oisDataMtx;
			oisFile.readAllData(MARKET_DATA_DELIMITER, oisDataMtx);
			oisFile.close();
			const unsigned int oisSize = oisDataMtx.size();

			for (unsigned int i = 0; i < fedfundSize; ++i)
			{
				LAString term = fedfundDataMtx[i][0].toUpper();
				fedfund.push_back(term);
			}
			for (unsigned int i = 0; i < oisSize; ++i)
			{
				LAString term = oisDataMtx[i][0].toUpper();
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
				throw LACoreInvalidData("fed fund rate and boj rate is not input simultaneously", __FILE__, __LINE__);
			}
			return ret;
		}

		// read libor file
		LAString liborFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + getCurveSuffix(ccy));
		MAFileAccessor liborFile(LAMarketData::getNumFileName(liborFileName));
		LAStringMatrix liborDataMtx;
		liborFile.readAllData(MARKET_DATA_DELIMITER, liborDataMtx);
		liborFile.close();

		// read fra file
		LAStringMatrix fra3DataMtx,fra6DataMtx,futureDataMtx;
		LAString fra3FileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE + getCurveSuffix(ccy));
		if (fra3FileName != MLIB_NO_DATA)
		{
			MAFileAccessor fra3File(LAMarketData::getNumFileName(fra3FileName));
			fra3File.readAllData(MARKET_DATA_DELIMITER, fra3DataMtx);
			fra3File.close();
		}
		LAString fra6FileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE + getCurveSuffix(ccy));
		if (fra6FileName != MLIB_NO_DATA)
		{
			MAFileAccessor fra6File(LAMarketData::getNumFileName(fra6FileName));
			fra6File.readAllData(MARKET_DATA_DELIMITER, fra6DataMtx);
			fra6File.close();
		}
		LAString futureFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + getCurveSuffix(ccy));
		if (futureFileName != MLIB_NO_DATA)
		{
			MAFileAccessor futureFile(LAMarketData::getNumFileName(futureFileName));
			futureFile.readAllData(MARKET_DATA_DELIMITER, futureDataMtx);
			futureFile.close();
		}

		// read swap file
		LAString swapFileName = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + getCurveSuffix(ccy));
		MAFileAccessor swapFile(LAMarketData::getNumFileName(swapFileName));
		LAStringMatrix swapDataMtx;
		swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
		swapFile.close();

		const unsigned int liborSize = liborDataMtx.size();
		const unsigned int swapSize = swapDataMtx.size();
		const unsigned int fra3Size = fra3DataMtx.size();
		const unsigned int fra6Size = fra6DataMtx.size();
		const unsigned int futureSize = futureDataMtx.size();
		LAStringVector libor(liborSize);
		LAStringVector swap(swapSize);
		LAStringVector fra3m(fra3Size);
		LAStringVector fra6m(fra6Size);
		LAStringVector future(futureSize);
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
		sort(libor.begin(), libor.end(), MAComp_StrTerm());
		sort(swap.begin(), swap.end(), MAComp_StrTerm());
		sort(fra3m.begin(), fra3m.end(), MAComp_StrTerm());
		sort(fra6m.begin(), fra6m.end(), MAComp_StrTerm());
		sort(future.begin(), future.end(), MAComp_StrTerm());

		const unsigned int marketSize = liborSize + swapSize+ fra3Size + fra6Size + futureSize;
		LAStringVector ret(marketSize);
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
	@return vector<LAString>
*/
vector<LAString>
LARiskConfigurationYieldIRDelta::getBucketGridTerm(const LAString &ccy) const
{
	LAStringVector ret = getGridRangeTerm(ccy);
	if (!ret.empty()) return ret;
	LAString tmpccy = ccy;
	LAString strBucketGrid = mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_BUCKET_TERM + getCurveSuffix(ccy));
	LAStringVector BucketTerm = strBucketGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
	BucketTerm[0].toUpper();
	if (BucketTerm[0] == "NONE" || BucketTerm[0] == MLIB_NO_DATA)
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
LARiskConfigurationYieldIRDelta::getPropertyBucketGridTerm(const LAString &ccy) const
{
	LAString tmpccy = ccy;
	return mpRiskStaticData->getStaticData(tmpccy.toLower() + 
									STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_BUCKET_TERM + getCurveSuffix(ccy));
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@param[in] vals
*/
void
LARiskConfigurationYieldIRDelta::getScenario1ParallelShift(const LAString &ccy, DoubleArray &vals) const
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
LARiskConfigurationYieldIRDelta::getScenario1ParallelShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_PARALLEL_SHIFTVAL + getCurveSuffix(ccy));
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
void
LARiskConfigurationYieldIRDelta::getScenario2ParallelShift(const LAString &ccy, DoubleArray &vals) const
{
	getScenario1ParallelShift(ccy, vals);
}

/*!
    @brief return scenario1 parallel shift value

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldIRDelta::getScenario1ParallelShift(const LAString &ccy) const
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
LARiskConfigurationYieldIRDelta::getScenario2ParallelShift(const LAString &ccy) const
{
	return getScenario1ParallelShift(ccy);
}

/*!
    @brief return scenario1 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldIRDelta::getScenario1GridShift(const LAString &ccy) const
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
LARiskConfigurationYieldIRDelta::getScenario1GridShiftStr(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
	    						STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_GRID_SHIFTVAL + getCurveSuffix(ccy));


}

/*!
    @brief return scenario2 grid shift values

	@param[in] ccy
	@return DoubleArray
*/
DoubleArray
LARiskConfigurationYieldIRDelta::getScenario2GridShift(const LAString &ccy) const
{
	return getScenario1GridShift(ccy);
}

/*!
    @brief return bump direction

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRDelta::getBumpDirection(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_BUMPDIRECTION + getCurveSuffix(ccy));

}


/*!
    @brief return iswave

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRDelta::isWave(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISWAVE + getCurveSuffix(ccy)));
}

/*!
    @brief return shift type

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfigurationYieldIRDelta::getShiftType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return  mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SHIFTTYPE + getCurveSuffix(ccy));

}


/*!
    @brief return target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRDelta::getTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRDELTA_TARGET_CURRENCY);
}

/*!
    @brief return calibration target currencies

	@return LAString 
*/
LAString
LARiskConfigurationYieldIRDelta::getCalibTargetCurrencies() const
{
	return mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRDELTA_CALIBRATION_TARGET_CURRENCY);
}


/*!
    @brief return divid unit

	@param[in] ccy
	@return double
*/
double
LARiskConfigurationYieldIRDelta::getDivUnit(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	LAString strDivUnit = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
								STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_DIVUNIT + getCurveSuffix(ccy));

	return  strDivUnit.getDoubleValue();
}

/*!
    @brief return buffer for calc grid risk

	@return int
*/
int 
LARiskConfigurationYieldIRDelta::getGridCalcBuffer() const
{
	return  mpRiskStaticData->getStaticData(RISK_FRONT_YIELD_IRDELTA_GRID_CALCBUFFER).getIntValue();
}

/*!
    @brief returnisRiskCurrencyMode

	@param[in] ccy
	@return bool
*/
bool
LARiskConfigurationYieldIRDelta::isRiskCurrencyMode(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISRISKCURRENCYMODE);
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
LARiskConfigurationYieldIRDelta::isZeroBump(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	//if MA_NODATA return false;
	LAString proprslt = mpRiskStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISZERORATEBUMP + getCurveSuffix(ccy));
	if (proprslt == MLIB_NO_DATA)
		return false;
	
	return convertBoolFromStr(proprslt);
}

void 
LARiskConfigurationYieldIRDelta::getMarketTerms(const LAString& ccy, LAStringVector& terms) const
{
    LAString ccy_lower = ccy; ccy_lower.toLower();	

	const bool isoismode  = (mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + getCurveSuffix(ccy_lower)).toUpper() != MLIB_NO_DATA);

	LAStringMatrix liborDataMtx, swapDataMtx,fra3DataMtx,fra6DataMtx,futureDataMtx;
	
	//read future file
	LAString futureFileName = isoismode ? mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + getCurveSuffix(ccy_lower)) :
                                          mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_FUTURE_FILE + getCurveSuffix(ccy_lower));
	if (futureFileName != MLIB_NO_DATA)
	{
		MAFileAccessor futureFile(LAMarketData::getNumFileName(futureFileName));
		futureFile.readAllData(MARKET_DATA_DELIMITER, futureDataMtx);
		futureFile.close();
	}

	// read swap file
    LAString swapFileName = isoismode ? mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_OIS_FILE + getCurveSuffix(ccy_lower)) :
                                        mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_SWAP_FILE + getCurveSuffix(ccy_lower));
	MAFileAccessor swapFile(LAMarketData::getNumFileName(swapFileName));
	swapFile.readAllData(MARKET_DATA_DELIMITER, swapDataMtx);
	swapFile.close();

	if (!isoismode)
	{
		// read libor file
		LAString liborFileName = mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_LIBOR_FILE + getCurveSuffix(ccy_lower));
		MAFileAccessor liborFile(LAMarketData::getNumFileName(liborFileName));
		liborFile.readAllData(MARKET_DATA_DELIMITER, liborDataMtx);
		liborFile.close();
		//read fra file
		LAString fra3FileName = mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_3MFRA_FILE + getCurveSuffix(ccy_lower));
		if (fra3FileName != MLIB_NO_DATA)
		{
			MAFileAccessor fra3File(LAMarketData::getNumFileName(fra3FileName));
			fra3File.readAllData(MARKET_DATA_DELIMITER, fra3DataMtx);
			fra3File.close();
		}
		LAString fra6FileName = mpStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_YIELD_6MFRA_FILE + getCurveSuffix(ccy_lower));
		if (fra6FileName != MLIB_NO_DATA)
		{
			MAFileAccessor fra6File(LAMarketData::getNumFileName(fra6FileName));
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
vector<LAObject *>
LARiskConfigurationYieldIRDelta::createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	if (omitNotionalExposure(ccy))
	{
		return createIRBaseScenarioEntity(ccy,dataInstance,index);
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
LARiskConfigurationYieldIRDelta::getBaseOutPutName(const LAString &ccy , int index) const
{
	if (omitNotionalExposure(ccy))
	{
		LAString tmpCcy = ccy;
		tmpCcy.toLower();
		LAString ret = mpRiskStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OUTPUTNAME + getCurveSuffix(ccy));
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
LARiskConfigurationYieldIRDelta::omitNotionalExposure(const LAString &ccy) const
{
	LAString tmpCcy = ccy;
	tmpCcy.toLower();
	LAString omitNotionalExposure = mpRiskStaticData->getStaticData(tmpCcy + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OMITNOTIONALEXPOSURE);
	if (omitNotionalExposure == MLIB_NO_DATA)
	{
		return false;
	}
	else
	{
		return convertBoolFromStr(omitNotionalExposure);
	}
}

