#define _HAS_STD_BYTE 0

/*! @file
    @brief Scenario create class
*/
//  2011, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfiguration.cpp
//
//  DESCRIPTION :       Scenario create class
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
#include "AQLPriceArbFreeGenerator.h"
#include "LAScenarioConfigurationYieldCurve.h"
#include "LAMarketData.h"
#include "AQLBasic.h"

using namespace std;

/*!
    @brief Risk Scenario create method for foregin currency

	@param[int,out] dataInstance
	@param[in] param
	@return vector<MBEnity *>
*/
vector<AQLObject *>
LAScenarioConfiguration::createScenarioForeignYield(AQLDataInstance &dataInstance, const MAScenarioParam &param) const
{
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
	const AQLString& bFYieldDataName = bYieldPro.getForeignYieldData().convertToString();

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
	// neither collateral curve nor fwdfx const curve are generated after
	if (param.isFirst && !(isCollateral || isFwdFXConst))
		return ret;
	
	// get ArbFree Flag
	const bool isArbFree = dynamic_cast<const AQLDataBool&>(bYieldPro.getIsArbFree()).get();

	const AQLDate asOfDate = LAMarketData::getAsofDate(objPool);
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();

	// parallel shift case
	AQLString suffix;
	unsigned int counter = 0;
	unsigned int refSize = param.refName.size();
	for (size_t i = 0; i < refSize; i++)
	{
		
		if (param.isParallel && i == 0) 
		{
			suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Parallel";
		}
		else
		{
			suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" +  AQLString(static_cast<int>(counter++));
		}
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
			if (isArbFree) bYieldPro.removeAllCuveData(*cYieldData);
			bYieldPro.removeBasisCuveData(*cYieldData);
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
		//set domestic yield data
		bYieldPro.getForeignYieldData().convertFromString(param.refName[i]);
		
		if (isArbFree)
		{
			// generate yield data
			AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
								(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			modelDataObj.calibrateModel(asOfDate);
			const AQLDataHolder* dh = &bYieldPro.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
			// generate swap curve	
			if (dh->isDefined() && !dh->isNull())
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

		dataInstance.getReferencePool().completeDependency();
		bYieldPro.setBasisRates();

		ret.push_back(cYield);
	}

	// hirayake data out
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
	}

	//restore to original
	bYieldPro.getYieldData().convertFromString(bYieldDataName);
	bYieldPro.getForeignYieldData().convertFromString(bFYieldDataName);
	
	return ret;
}

/*!
    @brief Risk Scenario create method for collateral currency

	@param[int,out] dataInstance
	@param[in] param
	@return vector<MBEnity *>
*/
vector<AQLObject *>
LAScenarioConfiguration::createScenarioCollateralYield(AQLDataInstance &dataInstance, const MAScenarioParam &param) const
{
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
    const AQLString& bCYieldDataName = bYieldPro.getColYieldData().convertToString();

	const AQLDate asOfDate = LAMarketData::getAsofDate(objPool);
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();

	// parallel shift case
	AQLString suffix;
	unsigned int counter = 0;
	unsigned int refSize = param.refName.size();
	for (size_t i = 0; i < refSize; i++)
	{
		
		if (param.isParallel && i == 0) 
		{
			suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Parallel";
		}
		else
		{
			suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" +  AQLString(static_cast<int>(counter++));
		}
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
		}
		else
		{
			cYieldData = &objHolder.get();
		}

		bool isExist = false;
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
			isExist = true;
			cYield = &dynamic_cast<AQLMathYieldCurve &>(objHolder.get());
		}
	
		// set yield data yield curve pro and yield curve
		cYield->getYieldData().convertFromString(cYieldDataName);
		bYieldPro.getYieldData().convertFromString(cYieldDataName);
		//set domestic yield data
		bYieldPro.getColYieldData().convertFromString(param.refName[i]);
		
		dataInstance.getReferencePool().completeDependency();
		bYieldPro.removeCuveData(*cYieldData, FWDFXCONST);
		bYieldPro.calcFwdFXConstantCurve();
		if (!isExist)
			ret.push_back(cYield);
	}

	// hirayake data out
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
	}

	//restore to original
	bYieldPro.getYieldData().convertFromString(bYieldDataName);
	bYieldPro.getColYieldData().convertFromString(bCYieldDataName);
	
	return ret;
}

void
LAScenarioConfiguration::dataoutCurve(AQLDataInstance &dataInstance, const AQLMathYieldCurvePro& yieldCurvePro, const vector<AQLObject *>& ret) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	dataInstance.getReferencePool().completeDependency();
	const AQLString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM);
	const unsigned int yieldSize = ret.size();
	const std::map<AQLString, AQLString> &assignedCurveMktMap = yieldCurvePro.getAssignedCurveMktMap();
	
	ofstream fout;
	for (unsigned int i = 0; i < yieldSize; ++i)
	{
		const AQLDataReference &yd = dynamic_cast<const AQLMathYieldCurve *>(ret[i])->getYieldData();
		AQLString ydName = yd.convertToString();
		ydName.exchange("\"","");
		AQLObjectHolder ydHolder = objPool.getObject(ydName, ENCHKTYPE_ISDEFINED);
		ydName.exchange("/","");		
		const AQLString dirName = LACoreDataService::getOutputDirectory(); 

		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			const AQLString& curveName = it->first;
			AQLString suffix;
			if (curveName != STD) suffix = "_" + curveName;
			const AQLString fileName  = dirName + ydName + suffix + fileSuffix + ".csv";

			ifstream fin;
			fin.open(fileName.getCString());

			if (!fin)
			{
				const AQLDataHolder* ah_term = &ydHolder.getData(CALIBRATION_DATA_TERMS + suffix, NOCHECK);
				const AQLDataHolder* ah_df = &ydHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, NOCHECK);
				if (!ah_term->isDefined() || ah_term->isNull() || !ah_df->isDefined() || ah_df->isNull()) continue;

				const DoubleArray &terms = dynamic_cast<const AQLDataDoubles &>(ah_term->get()).get();
				const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles &>(ah_df->get()).get();

				int size = terms.size();
				if (size != static_cast<int>(dfs.size()))
				{
					throw AQLCoreInvalidData("Term size and df size must be same !!", __FILE__, __LINE__);
				}
				fout.open(fileName.getCString());

				const AQLDataHolder &dfAttr2 = ydHolder.getData(IR_CALIBRATION_DATA_DFS2 + suffix, NOCHECK);
				if (dfAttr2.isDefined() && !dfAttr2.isNull())
				{
					const DoubleArray &dfs2 = dynamic_cast<const AQLDataDoubles &>(dfAttr2.get()).get();
					for (int j = 0; j < size; ++j)
					{
						AQLString termOStr = AQLString(terms[j]);
						AQLString dfOStr = AQLString(dfs[j]);
						AQLString dfOStr2 = AQLString(dfs2[j]);
						fout << termOStr.getCString() << "," << dfOStr.getCString() << "," << dfOStr2.getCString() << std::endl;
					}
				}
				else
				{
					for (int j = 0; j < size; ++j)
					{
						AQLString termOStr = AQLString(terms[j]);
						AQLString dfOStr = AQLString(dfs[j]);
						fout << termOStr.getCString() << "," << dfOStr.getCString() << std::endl;
					}
				}
				fout.close();
				fin.close();
			}
			else
			{
				DoubleArray terms;
				DoubleArray dfs;
				DoubleArray dfs2;
				string line;
				while (getline(fin, line))
				{
					const char *c_line = line.c_str();
					AQLStringVector lineVec = AQLString(c_line).toToken(MARKET_DATA_DELIMITER);
					if (lineVec.size() == 3)
					{
						terms.push_back(lineVec[0].trimLeft().trimRight().getDoubleValue());
						dfs.push_back(lineVec[1].trimLeft().trimRight().getDoubleValue());
						dfs2.push_back(lineVec[2].trimLeft().trimRight().getDoubleValue());
					}
					else
					{
						terms.push_back(lineVec[0].trimLeft().trimRight().getDoubleValue());
						dfs.push_back(lineVec[1].trimLeft().trimRight().getDoubleValue());
					}
				}
				fin.close();
				dynamic_cast<AQLDataDoubles &>(ydHolder.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get()).set(terms);
				dynamic_cast<AQLDataDoubles &>(ydHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get()).set(dfs);
				if (!dfs2.empty() && ydHolder.getData(IR_CALIBRATION_DATA_DFS2 + suffix).isDefined())
				{
					dynamic_cast<AQLDataDoubles &>(ydHolder.getData(IR_CALIBRATION_DATA_DFS2 + suffix, ISNOTNULL).get()).set(dfs2);
				}
			}
		}
	}
}

vector<AQLObject *>
LAScenarioConfiguration::createScenarioExtraFwdFXConstCurveForFXDelta(AQLDataInstance &dataInstance, const MAScenarioParam &param) const
{
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
    const AQLString& bCYieldDataName = bYieldPro.getColYieldData().convertToString();

	const AQLDate asOfDate = LAMarketData::getAsofDate(objPool);
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();

	// parallel shift case
	AQLString suffix;
	unsigned int counter = 0;
	unsigned int refSize = param.refName.size();
	for (size_t i = 0; i < refSize; i++)
	{
		suffix = "_" + param.calcType;

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
		}
		else
		{
			cYieldData = &objHolder.get();
		}

		bool isExist = false;
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
			isExist = true;
			cYield = &dynamic_cast<AQLMathYieldCurve &>(objHolder.get());
		}
	
		// set yield data yield curve pro and yield curve
		cYield->getYieldData().convertFromString(cYieldDataName);
		bYieldPro.getYieldData().convertFromString(cYieldDataName);
		//set domestic yield data
		bYieldPro.getColYieldData().convertFromString(param.refName[i]);
		
		dataInstance.getReferencePool().completeDependency();
		bYieldPro.removeCuveData(*cYieldData, FWDFXCONST);
		bYieldPro.calcFwdFXConstantCurve();
		if (!isExist)
			ret.push_back(cYield);
	}

	// data out
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
	}

	//restore to original
	bYieldPro.getYieldData().convertFromString(bYieldDataName);
	bYieldPro.getColYieldData().convertFromString(bCYieldDataName);
	
	return ret;
}

vector<AQLObject *>
LAScenarioConfiguration::createScenarioExtraXccyCurveForFXDelta(AQLDataInstance &dataInstance, const MAScenarioParam &param) const
{
	vector<AQLObject *> ret(0);
	AQLObjectPool &objPool = dataInstance.getObjectPool();

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
	const AQLString& bFYieldDataName = bYieldPro.getForeignYieldData().convertToString();

	const AQLDate asOfDate = LAMarketData::getAsofDate(objPool);
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();	

	// parallel shift case
	AQLString suffix;
	unsigned int counter = 0;
	unsigned int refSize = param.refName.size();
	for(size_t i = 0; i < refSize; i++)
	{
		suffix = "_" + param.calcType;

		// clone yield data and yield
		const AQLString cYieldDataName = bYieldDataName + suffix;
		AQLObject *cYieldData = 0;
		AQLObjectHolder objHolder = objPool.getObject(cYieldDataName, ENCHKTYPE_NOCHECK);
		if(!objHolder.isDefined())
		{
			cYieldData = bYieldData.clone();
			cYieldData->remove(CALIBRATION_DATA_NAME);
			cYieldData->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(cYieldDataName);
			objPool.set(cYieldDataName, cYieldData);

			// remove curve data
			bYieldPro.removeBasisCuveData(*cYieldData);
		}
		else
		{
			cYieldData = &objHolder.get();
		}

		AQLMathYieldCurve *cYield = 0;
		const AQLString cYieldName = bYieldName + suffix;
		objHolder = objPool.getObject(cYieldName, ENCHKTYPE_NOCHECK);
		if(!objHolder.isDefined())
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
		// set fx object
		bYieldPro.getFXEntity().convertFromString(param.refName[i]);

		dataInstance.getReferencePool().completeDependency();
		bYieldPro.setBasisRates();

		ret.push_back(cYield);
	}

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
	}

	//restore to original
	bYieldPro.getYieldData().convertFromString(bYieldDataName);
	bYieldPro.getForeignYieldData().convertFromString(bFYieldDataName);

	return ret;
}