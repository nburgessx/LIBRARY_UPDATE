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
#include "LAScenarioConfigurationYieldCurve.h"
#include "LAMarketData.h"
#include "LABasic.h"

using namespace std;

/*!
    @brief Risk Scenario create method for foregin currency

	@param[int,out] dataInstance
	@param[in] param
	@return vector<MBEnity *>
*/
vector<LAObject *>
LAScenarioConfiguration::createScenarioForeignYield(LADataInstance &dataInstance, const MAScenarioParam &param) const
{
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
	const LAString& bFYieldDataName = bYieldPro.getForeignYieldData().convertToString();

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
	const bool isArbFree = dynamic_cast<const LADataBool&>(bYieldPro.getIsArbFree()).get();

	const LADate asOfDate = LAMarketData::getAsofDate(objPool);
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();

	// parallel shift case
	LAString suffix;
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
			suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" +  LAString(static_cast<int>(counter++));
		}
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
			if (isArbFree) bYieldPro.removeAllCuveData(*cYieldData);
			bYieldPro.removeBasisCuveData(*cYieldData);
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
		//set domestic yield data
		bYieldPro.getForeignYieldData().convertFromString(param.refName[i]);
		
		if (isArbFree)
		{
			// generate yield data
			LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
								(bYieldPro.getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			modelDataObj.calibrateModel(asOfDate);
			const LADataHolder* dh = &bYieldPro.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
			// generate swap curve	
			if (dh->isDefined() && !dh->isNull())
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

		dataInstance.getReferencePool().completeDependency();
		bYieldPro.setBasisRates();

		ret.push_back(cYield);
	}

	// hirayake data out
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
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
vector<LAObject *>
LAScenarioConfiguration::createScenarioCollateralYield(LADataInstance &dataInstance, const MAScenarioParam &param) const
{
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
    const LAString& bCYieldDataName = bYieldPro.getColYieldData().convertToString();

	const LADate asOfDate = LAMarketData::getAsofDate(objPool);
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();

	// parallel shift case
	LAString suffix;
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
			suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" +  LAString(static_cast<int>(counter++));
		}
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
		}
		else
		{
			cYieldData = &objHolder.get();
		}

		bool isExist = false;
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
			isExist = true;
			cYield = &dynamic_cast<LAMathYieldCurve &>(objHolder.get());
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
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
	}

	//restore to original
	bYieldPro.getYieldData().convertFromString(bYieldDataName);
	bYieldPro.getColYieldData().convertFromString(bCYieldDataName);
	
	return ret;
}

void
LAScenarioConfiguration::dataoutCurve(LADataInstance &dataInstance, const LAMathYieldCurvePro& yieldCurvePro, const vector<LAObject *>& ret) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	dataInstance.getReferencePool().completeDependency();
	const LAString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM);
	const unsigned int yieldSize = ret.size();
	const std::map<LAString, LAString> &assignedCurveMktMap = yieldCurvePro.getAssignedCurveMktMap();
	
	ofstream fout;
	for (unsigned int i = 0; i < yieldSize; ++i)
	{
		const LADataReference &yd = dynamic_cast<const LAMathYieldCurve *>(ret[i])->getYieldData();
		LAString ydName = yd.convertToString();
		ydName.exchange("\"","");
		LAObjectHolder ydHolder = objPool.getObject(ydName, ENCHKTYPE_ISDEFINED);
		ydName.exchange("/","");		
		const LAString dirName = LACoreDataService::getOutputDirectory(); 

		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			const LAString& curveName = it->first;
			LAString suffix;
			if (curveName != STD) suffix = "_" + curveName;
			const LAString fileName  = dirName + ydName + suffix + fileSuffix + ".csv";

			ifstream fin;
			fin.open(fileName.getCString());

			if (!fin)
			{
				const LADataHolder* ah_term = &ydHolder.getData(CALIBRATION_DATA_TERMS + suffix, NOCHECK);
				const LADataHolder* ah_df = &ydHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, NOCHECK);
				if (!ah_term->isDefined() || ah_term->isNull() || !ah_df->isDefined() || ah_df->isNull()) continue;

				const DoubleArray &terms = dynamic_cast<const LADataDoubles &>(ah_term->get()).get();
				const DoubleArray &dfs = dynamic_cast<const LADataDoubles &>(ah_df->get()).get();

				int size = terms.size();
				if (size != static_cast<int>(dfs.size()))
				{
					throw LACoreInvalidData("Term size and df size must be same !!", __FILE__, __LINE__);
				}
				fout.open(fileName.getCString());

				const LADataHolder &dfAttr2 = ydHolder.getData(IR_CALIBRATION_DATA_DFS2 + suffix, NOCHECK);
				if (dfAttr2.isDefined() && !dfAttr2.isNull())
				{
					const DoubleArray &dfs2 = dynamic_cast<const LADataDoubles &>(dfAttr2.get()).get();
					for (int j = 0; j < size; ++j)
					{
						LAString termOStr = LAString(terms[j]);
						LAString dfOStr = LAString(dfs[j]);
						LAString dfOStr2 = LAString(dfs2[j]);
						fout << termOStr.getCString() << "," << dfOStr.getCString() << "," << dfOStr2.getCString() << std::endl;
					}
				}
				else
				{
					for (int j = 0; j < size; ++j)
					{
						LAString termOStr = LAString(terms[j]);
						LAString dfOStr = LAString(dfs[j]);
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
					LAStringVector lineVec = LAString(c_line).toToken(MARKET_DATA_DELIMITER);
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
				dynamic_cast<LADataDoubles &>(ydHolder.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get()).set(terms);
				dynamic_cast<LADataDoubles &>(ydHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get()).set(dfs);
				if (!dfs2.empty() && ydHolder.getData(IR_CALIBRATION_DATA_DFS2 + suffix).isDefined())
				{
					dynamic_cast<LADataDoubles &>(ydHolder.getData(IR_CALIBRATION_DATA_DFS2 + suffix, ISNOTNULL).get()).set(dfs2);
				}
			}
		}
	}
}

vector<LAObject *>
LAScenarioConfiguration::createScenarioExtraFwdFXConstCurveForFXDelta(LADataInstance &dataInstance, const MAScenarioParam &param) const
{
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
    const LAString& bCYieldDataName = bYieldPro.getColYieldData().convertToString();

	const LADate asOfDate = LAMarketData::getAsofDate(objPool);
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();

	// parallel shift case
	LAString suffix;
	unsigned int counter = 0;
	unsigned int refSize = param.refName.size();
	for (size_t i = 0; i < refSize; i++)
	{
		suffix = "_" + param.calcType;

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
		}
		else
		{
			cYieldData = &objHolder.get();
		}

		bool isExist = false;
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
			isExist = true;
			cYield = &dynamic_cast<LAMathYieldCurve &>(objHolder.get());
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
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
	}

	//restore to original
	bYieldPro.getYieldData().convertFromString(bYieldDataName);
	bYieldPro.getColYieldData().convertFromString(bCYieldDataName);
	
	return ret;
}

vector<LAObject *>
LAScenarioConfiguration::createScenarioExtraXccyCurveForFXDelta(LADataInstance &dataInstance, const MAScenarioParam &param) const
{
	vector<LAObject *> ret(0);
	LAObjectPool &objPool = dataInstance.getObjectPool();

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
	const LAString& bFYieldDataName = bYieldPro.getForeignYieldData().convertToString();

	const LADate asOfDate = LAMarketData::getAsofDate(objPool);
	
	LAMarketData::sortMarketData(bYieldPro);
	dataInstance.getReferencePool().completeDependency();	

	// parallel shift case
	LAString suffix;
	unsigned int counter = 0;
	unsigned int refSize = param.refName.size();
	for(size_t i = 0; i < refSize; i++)
	{
		suffix = "_" + param.calcType;

		// clone yield data and yield
		const LAString cYieldDataName = bYieldDataName + suffix;
		LAObject *cYieldData = 0;
		LAObjectHolder objHolder = objPool.getObject(cYieldDataName, ENCHKTYPE_NOCHECK);
		if(!objHolder.isDefined())
		{
			cYieldData = bYieldData.clone();
			cYieldData->remove(CALIBRATION_DATA_NAME);
			cYieldData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(cYieldDataName);
			objPool.set(cYieldDataName, cYieldData);

			// remove curve data
			bYieldPro.removeBasisCuveData(*cYieldData);
		}
		else
		{
			cYieldData = &objHolder.get();
		}

		LAMathYieldCurve *cYield = 0;
		const LAString cYieldName = bYieldName + suffix;
		objHolder = objPool.getObject(cYieldName, ENCHKTYPE_NOCHECK);
		if(!objHolder.isDefined())
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
		// set fx object
		bYieldPro.getFXEntity().convertFromString(param.refName[i]);

		dataInstance.getReferencePool().completeDependency();
		bYieldPro.setBasisRates();

		ret.push_back(cYield);
	}

	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
	}

	//restore to original
	bYieldPro.getYieldData().convertFromString(bYieldDataName);
	bYieldPro.getForeignYieldData().convertFromString(bFYieldDataName);

	return ret;
}