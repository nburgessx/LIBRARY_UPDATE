/*! @file
    @brief Yield Risk Scenario create class(zero rate shift)
*/

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
#include "LAScenarioConfigurationYieldCurveZero.h"
#include "LAMarketData.h"
#include "LACalibrationUtilities.h"
#include "LABasic.h"
#include "ConstantDeclarations.h"

#define RATE_FLOOR -1.0 // -100%

using namespace std;

// constructor
/*!

*/
LAScenarioConfigurationYieldCurveZero::LAScenarioConfigurationYieldCurveZero(bool isBasis)
: mIsBasis(isBasis)
{
}

// destructor
/*!

*/
LAScenarioConfigurationYieldCurveZero::~LAScenarioConfigurationYieldCurveZero(void)
{
}


 
/*!
    @brief Risk Scenario create method

	@param[int,out] dataInstance
	@param[in] param
	@return vector<MBEnity *>
*/
vector<LAObject *>
LAScenarioConfigurationYieldCurveZero::createScenario(LADataInstance &dataInstance, const MAScenarioParam &param) const
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
	
	// set curvetype
	const LAString orgCurveType = bYieldPro.getCurveType();

	LAStringVector targetCurves = param.targetCurveType.toToken('/');
	// get suffix
	LAString attrSuffix_target = "";
	if (param.targetCurveType != STD)
	{
		attrSuffix_target = "_" + param.targetCurveType;
	}

	bool isBasisSet = false;
	bool isOldBasisSet = false;

	for (size_t i=0; i<targetCurves.size(); i++)
	{		
		// get suffix
		LAString attrSuffix = "";
		if (bYieldPro.getMarketForCurve(targetCurves[i]) != SWAP)
		{
			attrSuffix = "_" + bYieldPro.getMarketForCurve(targetCurves[i]);
		}

		LADataHolder& dh = bYieldPro.getData(CALIBRATION_DATA_MARKETDATA + attrSuffix, NOCHECK);
		
		if ( dh.isDefined() && !dh.isNull() )
		{
			LADataMultiReference &refMarket = dynamic_cast< LADataMultiReference& >( dh.get() );

			const unsigned int marketSize = refMarket.getSize();

			for (unsigned int i = 0; i < marketSize; ++i)
			{
				LAString type = dynamic_cast<const LADataString &>
									(refMarket.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
				type.toUpper();
				if (type == YIELD_TYPE_BASIS)
				{
					isBasisSet = true;
				}
			}
		}

		isOldBasisSet = (isBasisSet && targetCurves[i] == STD);
	}

	LADataHolder &ahBasis = bYieldPro.getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (!isBasisSet && ahBasis.isDefined() && !ahBasis.isNull())
	{
		isBasisSet = true;
	}
	LADataHolder &ahMBasisDF = bYieldPro.getData(IR_CALIBRATION_DATA_MAINBASISDF, NOCHECK);
	if (!isBasisSet && ahMBasisDF.isDefined() && !ahMBasisDF.isNull())
	{
		LAString mainBasisDF = dynamic_cast<const LADataString &>(ahMBasisDF.get()).get();
		if (find(targetCurves.begin(), targetCurves.end(), mainBasisDF) != targetCurves.end())
		{
			isBasisSet = true;
		}
	}

	// parallel shift case
	if (param.isParallel)
	{
		if (param.paraShiftVec.size() != 1)
		{
			throw LACoreInvalidData("Parallel shift error. Parallel shift vec size must be 1", __FILE__, __LINE__);
		}
		
		LAString suffix = "_" + param.calcType + attrSuffix_target + "_Parallel";
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
			for (size_t i=0; i<targetCurves.size(); i++)
			{
				LAString attrSuffix_curveName = "";
				if (targetCurves[i] != STD)
				{
					attrSuffix_curveName = "_" + targetCurves[i];
				}
				cYieldData->remove(IR_CALIBRATION_DATA_DFS + attrSuffix_curveName );
				const DoubleVector &dfs = dynamic_cast<const LADataDoubles &>(bYieldData.
					getData(IR_CALIBRATION_DATA_DFS + attrSuffix_curveName , ISNOTNULL).get()).get();
				cYieldData->add(IR_CALIBRATION_DATA_DFS + attrSuffix_curveName , new LADataDoubles(dfs));
			}
		}

		LAMathYieldCurve *cYield = 0;
		const LAString cYieldName = bYieldName + suffix; 
		objHolder = objPool.getObject(cYieldName, ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			cYield = dynamic_cast<LAMathYieldCurve *>(bYield.clone());
			cYield->LAObject::remove(CALIBRATION_DATA_NAME);
			cYield->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(cYieldName);
		}
		else
		{
			cYield = &dynamic_cast<LAMathYieldCurve &>(objHolder.get());
		}
		// set yield data yield curve pro and yield curve
		cYield->getYieldData().convertFromString(cYieldDataName);
		//cYield->setCurveType( param.targetCurveType ); 
		bYieldPro.getYieldData().convertFromString(cYieldDataName);

		LAStringVector nonvec;
		for (size_t i=0; i<targetCurves.size(); i++)
		{
			bYieldPro.setCurveType( targetCurves[i] ); 

			LAString attrSuffix = "";
			if (targetCurves[i] != STD)
			{
				attrSuffix = "_" + targetCurves[i];
			}

			//old version ..only xccybasis mode
			if (isOldBasisSet)
			{
				//yieldbasis && xccybasiscurve
				//old xccy
				if (mIsBasis)
				{
					bYieldPro.setBasisDFByShiftZero(param.paraShiftVec[0]);
				}
				else
				{
					bYieldPro.setDFByShiftZero(param.paraShiftVec[0], NORMAL, RATE_FLOOR);
					bYieldPro.setBasisRates();
				}
			}
			else
			{
				//yieldbasis yielddelta && 3m-6mCurve, JPY6MLibor,STD
				//new 
				bYieldPro.setDFByShiftZero(param.paraShiftVec[0], NORMAL, RATE_FLOOR);
				nonvec.push_back(targetCurves[i]);
			}
		}

		if (isBasisSet)
		{
			if (param.recalcBasisDFsOnZeroRateBump)
			{
				if (mIsBasis)
				{
					const LAStringVector& basisDFs = dynamic_cast<const LADataStrings&>(bYieldPro.getData(IR_CALIBRATION_DATA_BASISDFS).get()).get();
					for (std::size_t i = 0, ie = basisDFs.size(); i < ie; ++i)
					{
						if (std::find(targetCurves.begin(), targetCurves.end(), basisDFs[i]) == targetCurves.end())
						{
							bYieldPro.removeCuveData(bYieldPro.getYieldData().get().get(), bYieldPro.getMarketForCurve(basisDFs[i]));
						}
					}
				}
				else
				{
					bYieldPro.removeBasisCuveData(bYieldPro.getYieldData().get().get());
				}
			}
			else
			{
				bYieldPro.LAObject::add(IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS,new LADataStrings(nonvec));
			}
			bYieldPro.setBasisRates();
			bYieldPro.remove(IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS);
		}

		ret.push_back(cYield);
	}

	// grid shift case
	if (param.isGrid)
	{
		const LADate asOfDate = LAMarketData::getAsofDate(objPool);

		// daycount
		LAPriceDataDayCount dayCount;
		dayCount.convertFromString(DAYCOUNT_ACT_365_ISDA);

		const unsigned int gridSize = param.gridTerm.size();
		if (gridSize != param.gridShiftVec.size())
		{
			throw LACoreInvalidData("Grid term size and grid shift size is not same !!", __FILE__, __LINE__); 
		}

		LAPriceDataCalendar cal;
		if (param.isBusinessDayRoll)
		{
			if (param.calendar == AQ_NO_DATA)
			{
				throw LACoreInvalidData("BusinessDayRoll must set calendar.", __FILE__, __LINE__); 
			}
			cal.convertFromString(param.calendar);
		}

		vector<UintArray> gridPosVec(targetCurves.size());
		for (unsigned int i = 0; i < targetCurves.size(); i++)
		{
			UintArray &gridPos = gridPosVec[i];
			gridPos.resize(gridSize);

			LAString attrSuffix = "";
			if (targetCurves[i] != STD)
			{
				attrSuffix = "_" + targetCurves[i];
			}

			const DoubleArray &terms = dynamic_cast<const LADataDoubles &>
						(bYieldData.getData(CALIBRATION_DATA_TERMS + attrSuffix, ISNOTNULL).get()).get();

			for (unsigned int j = 0; j < gridSize; ++j)
			{
				LAString strTerm = param.gridTerm[j];
				strTerm.toUpper();

				if (strTerm.findString("D") > 0 && param.isBusinessDayRoll)
				{
					int d = strTerm.subString(0, strTerm.findString("D")).getIntValue();
					LADate date = cal.getBusinessDay(asOfDate, d);
					double term = dayCount.getTerm(asOfDate, date);
					LACalibrationUtilities<double>::searchNearestPos(terms, term, gridPos[j]);
				}
				else if (strTerm.findString("ON") != -1)
				{
					int d = 1;
					LADate date = cal.getBusinessDay(asOfDate, d);
					double term = dayCount.getTerm(asOfDate, date);
					LACalibrationUtilities<double>::searchNearestPos(terms, term, gridPos[j]);
				}
				else if (strTerm.findString("TN") != -1)
				{
					int d = 2;
					LADate date = cal.getBusinessDay(asOfDate, d);
					double term = dayCount.getTerm(asOfDate, date);
					LACalibrationUtilities<double>::searchNearestPos(terms, term, gridPos[j]);
				
				}
				else
				{
					LADate date = LAMathDateCalculations::getDate(asOfDate, strTerm, true);
					double term = dayCount.getTerm(asOfDate, date);
					LACalibrationUtilities<double>::searchNearestPos(terms, term, gridPos[j]);
				}	
			}
		}

		const LAMathYieldCurve* pbYield = &bYield;
		const LAObject* pbYieldData = &bYieldData;
		for (unsigned int i = 0; i < gridSize; ++i)
		{
			if (i > param.maxIndex)
			{
				break;
			}

			LAString suffix = "_" + param.calcType + attrSuffix_target + "_Grid_" + LAString(static_cast<int>(i));
			// clone yield data and yield
			const LAString cYieldDataName = bYieldDataName + suffix; 
			LAObject *cYieldData = 0;
			LAObjectHolder objHolder = objPool.getObject(cYieldDataName, ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
			{
				cYieldData = pbYieldData->clone();
				cYieldData->remove(CALIBRATION_DATA_NAME);
				cYieldData->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(cYieldDataName);
				objPool.set(cYieldDataName, cYieldData);
			}
			else
			{
				cYieldData = &objHolder.get();
				for (size_t i=0; i<targetCurves.size(); i++)
				{
					LAString attrSuffix_curveName = "";
					if (targetCurves[i] != STD)
					{
						attrSuffix_curveName = "_" + targetCurves[i];
					}
					cYieldData->remove(IR_CALIBRATION_DATA_DFS + attrSuffix_curveName);
					const DoubleVector &dfs = dynamic_cast<const LADataDoubles &>(pbYieldData->
						getData(IR_CALIBRATION_DATA_DFS + attrSuffix_curveName, ISNOTNULL).get()).get();
					cYieldData->add(IR_CALIBRATION_DATA_DFS + attrSuffix_curveName, new LADataDoubles(dfs));
				}
			}

			LAMathYieldCurve *cYield = 0;
			const LAString cYieldName = bYieldName + suffix; 
			objHolder = objPool.getObject(cYieldName, ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
			{
				cYield = dynamic_cast<LAMathYieldCurve *>(pbYield->clone());
				cYield->LAObject::remove(CALIBRATION_DATA_NAME);
				cYield->LAObject::add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(cYieldName);
			}
			else
			{
				cYield = &dynamic_cast<LAMathYieldCurve &>(objHolder.get());
			}
		
			// set yield data yield curve pro and yield curve
			cYield->getYieldData().convertFromString(cYieldDataName);
			bYieldPro.getYieldData().convertFromString(cYieldDataName);

			dataInstance.getReferencePool().completeDependency();

			LAStringVector nonvec;
			for (unsigned int j = 0; j < targetCurves.size(); j++)
			{
				bYieldPro.setCurveType( targetCurves[j] );

				//old version ..only xccybasis mode
				if (isOldBasisSet)
				{
					if (mIsBasis)
					{
						bYieldPro.setBasisDFByShiftZero(param.gridShiftVec[i], i, gridPosVec[j]);
					}
					else
					{
						bYieldPro.setDFByShiftZero(param.gridShiftVec[i], i, gridPosVec[j]);
						bYieldPro.setBasisRates();
					}
				}
				else
				{
					bYieldPro.setDFByShiftZero(param.gridShiftVec[i], i, gridPosVec[j]);
					nonvec.push_back(targetCurves[j]);
				}
			}

			if (isBasisSet)
			{
				if (param.recalcBasisDFsOnZeroRateBump)
				{
					if (mIsBasis)
					{
						const LAStringVector& basisDFs = dynamic_cast<const LADataStrings&>(bYieldPro.getData(IR_CALIBRATION_DATA_BASISDFS).get()).get();
						for (std::size_t i = 0, ie = basisDFs.size(); i < ie; ++i)
						{
							if (std::find(targetCurves.begin(), targetCurves.end(), basisDFs[i]) == targetCurves.end())
							{
								bYieldPro.removeCuveData(bYieldPro.getYieldData().get().get(), bYieldPro.getMarketForCurve(basisDFs[i]));
							}
						}
					}
					else
					{
						bYieldPro.removeBasisCuveData(bYieldPro.getYieldData().get().get());
					}
				}
				else
				{
					bYieldPro.LAObject::add(IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS,new LADataStrings(nonvec));
				}
				bYieldPro.setBasisRates();
				bYieldPro.remove(IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS);
			}
		
			ret.push_back(cYield);

			if (param.isWave)
			{
				pbYield = cYield;
				pbYieldData = cYieldData;
			}
		}
	}
	// set original yield data to yield pro
	bYieldPro.getYieldData().convertFromString(bYieldDataName);
	// hirayake data out
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
    {
		dataoutCurve(dataInstance, bYieldPro, ret);
		/*dataInstance.getReferencePool().completeDependency();
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
				dynamic_cast<const LADataDoubles &>(ydHolder.getData(CALIBRATION_DATA_TERMS + attrSuffix, ISNOTNULL).get()).get();
			const DoubleArray &dfs = 
				dynamic_cast<const LADataDoubles &>(ydHolder.getData(IR_CALIBRATION_DATA_DFS + attrSuffix, ISNOTNULL).get()).get();

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
		}*/
	}

	// set original curvetype
	bYieldPro.setCurveType( orgCurveType ); 

	return ret;

}

