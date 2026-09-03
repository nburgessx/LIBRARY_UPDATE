/*! @file
    @brief FX Risk Scenario class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLCoreReferencePool.h"
#include "AQLObjectPool.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "AQLDataMultiReference.h"
#include "AQLMathVolatility.h"
#include "AQLMathDateCalculations.h"
#include "LAScenarioConfigurationVolatility.h"
#include "LAMarketData.h"
#include "LACalibrateVolatility.h"
#include "LACalibrateVolatilityManager.h"
#include "LACalibrationUtilities.h"
#include "AQLBasic.h"

using namespace std;

// constructor
/*!

*/
LAScenarioConfigurationVolatility::LAScenarioConfigurationVolatility(void)
{
}

// destructor
/*!

*/
LAScenarioConfigurationVolatility::~LAScenarioConfigurationVolatility(void)
{
}


 
/*!
    @brief Risk Scenario create method

	@param[int,out] dataInstance
	@param[in] param
	@return vector<MBEnity *>
*/
vector<AQLObject *>
LAScenarioConfigurationVolatility::createScenario(AQLDataInstance &dataInstance, const MAScenarioParam &param) const
{
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLMathVolatility &b_vol = dynamic_cast<AQLMathVolatility &>
		(objPool.getObject(param.targetName, ENCHKTYPE_ISDEFINED).get());

	vector<AQLObject *> ret(0);

	if (param.isParallel)
	{
		ret = createParallelShiftVol(dataInstance, b_vol, param);
	}

	if (param.isGrid)
	{
		vector<AQLObject *> gridVol = createGridShiftVol(dataInstance, b_vol, param);
		ret.insert(ret.end(), gridVol.begin(), gridVol.end());
	}

	return ret;

}

/*!
    @brief ceate parallel shift vol

	@param[in] dataInstance
	@param[in] baseVol
	@param[in] param
	@return vector<AQLObject *>
*/
vector<AQLObject *>
LAScenarioConfigurationVolatility::createParallelShiftVol(AQLDataInstance &dataInstance, AQLMathVolatility &baseVol, const MAScenarioParam &param) const
{
	vector<AQLObject *> ret(0);
	AQLString inputType = param.inputType;
	inputType.toUpper();

	AQLString name = param.targetName + "_" + param.calcType + "_" + param.targetCurveType + "_Parallel";

	bool is_vol_clone = false;
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLMathVolatility *vol = 0;
	AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		vol = dynamic_cast<AQLMathVolatility *>(baseVol.clone());
		vol->getName() = name;
		is_vol_clone = true;
	}
	else
	{
		vol = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
	}

	// vol push back, only one AQLMathVolatility is created
	ret.push_back(vol);
	//get volatility creator
	LACalibrateVolatility *volCreator = LACalibrateVolatilityManager::getInstance()->createVolCreator(param.model);

	// shift val input case
	if (!param.paraShiftVec.empty())
	{
		AQLString shiftType = param.shiftType;
		shiftType.toUpper();

		double shiftVal = param.paraShiftVec[0];

		if (inputType == INPUT_T_DATA_MATRIX)
		{
			// data matrix input pattern
			DoubleMatrix volMtx;
			DoubleArray grid_t;
			volCreator->createVolatility(grid_t, volMtx, param.paraFile, &param, &objPool);

			const unsigned int size_m = volMtx.size();
			if (shiftType == RISK_SHIFTTYPE_DIFF)
			{
				// diff
				for (unsigned int i = 0; i < size_m; ++i)
				{
					DoubleArray add_bp(volMtx[i].size(), shiftVal);
					transform(volMtx[i].begin(), volMtx[i].end(), add_bp.begin(), volMtx[i].begin(), plus<double>());
					const unsigned int size_t = volMtx[i].size();
					for (unsigned int j = 0; j < size_t; ++j)
					{
						volMtx[i][j] = AQLMath::max(volMtx[i][j], 0.0);
					}
				}
			}
			else
			{
				// ratio
				for (unsigned int i = 0; i < size_m; ++i)
				{
					DoubleArray shiftVals = volMtx[i];
					DoubleArray multiple_percent(volMtx[i].size(), shiftVal);
					// calc shift val
					transform(shiftVals.begin(), shiftVals.end(), multiple_percent.begin(), shiftVals.begin(), multiplies<double>());
					// add shift val
					transform(volMtx[i].begin(), volMtx[i].end(), shiftVals.begin(), volMtx[i].begin(), plus<double>());
					const unsigned int size_t = volMtx[i].size();
					for (unsigned int j = 0; j < size_t; ++j)
					{
						volMtx[i][j] = AQLMath::max(volMtx[i][j], 0.0);
					}
				}
			}
			vol->setVolatility(grid_t, volMtx);
		}
		else if (inputType == INPUT_DATA_SCALAR)
		{
			// scalar data input pattern
			double volVal = 0.0;
			volCreator->createVolatility(volVal, param.paraFile, &param, &objPool);
			if (shiftType == RISK_SHIFTTYPE_DIFF)
			{
				// diff
				volVal += shiftVal;
			}
			else
			{
				// ratio
				volVal += shiftVal * volVal;
			}
			vol->setVolatility(volVal);
		}
		else if (inputType == INPUT_T_FUNC_VECTOR)
		{
			// method vector input case
			AQLString model = param.model;
			model.toUpper();
			AQLString bumpType = param.bumpType;
			bumpType.toUpper();

			if (model == MODEL_LMM && bumpType == RISK_MODEL_VOL_BUMP) {
				// create vol data
				DoubleMatrix volMtx;
				DoubleArray grid_t;
				volCreator->getGrid_T(grid_t, param.paraFile);// grid_t[0] != 0.0

				double zero = 0.0;
				grid_t.insert(grid_t.begin(), zero);
				unsigned int marketSize = grid_t.size() - 1;
				const unsigned int tSize = grid_t.size();

				vector<AQLFunctionBase *> volFuncVec;
				if (! is_vol_clone)
				{
					volCreator->createVolatility(volFuncVec, param.paraFile, &param, &objPool, -1);
				}
				else
				{
					for(unsigned int i = 0; i < marketSize; i++)
					{
						AQLFunctionBase* volFunc = vol->getVolatilityFunc(i, 0);
						volFuncVec.push_back(volFunc);
					}
				}

				volMtx.resize(marketSize);
				// set volatility val
				for (unsigned int i = 0; i < marketSize; ++i)
				{
					volMtx[i].resize(tSize, 0.0);
					for (unsigned int j = 0; j < tSize; ++j)
					{
						volMtx[i][j] = volFuncVec[i]->operator ()(grid_t[j]);
					}

					if (shiftType == RISK_SHIFTTYPE_DIFF)
					{
						// diff
						DoubleArray add_bp(tSize, shiftVal);
						transform(volMtx[i].begin(), volMtx[i].end(), add_bp.begin(), volMtx[i].begin(), plus<double>());
						const unsigned int size_t = volMtx[i].size();
						for (unsigned int j = 0; j < size_t; ++j)
						{
							volMtx[i][j] = AQLMath::max(volMtx[i][j], 0.0);
						}
					}
					else
					{
						// ratio
						DoubleArray shiftVals = volMtx[i];
						DoubleArray multiple_percent(tSize, shiftVal);
						// calc shift val
						transform(shiftVals.begin(), shiftVals.end(), multiple_percent.begin(), shiftVals.begin(), multiplies<double>());
						// add shift val
						transform(volMtx[i].begin(), volMtx[i].end(), shiftVals.begin(), volMtx[i].begin(), plus<double>());
						for (unsigned int j = 0; j < marketSize; ++j)
						{
							volMtx[i][j] = AQLMath::max(volMtx[i][j], 0.0);
						}
					}

					delete volFuncVec[i];
				}

				vol->setVolatility(grid_t, volMtx);
			}
			else
			{
				vector<AQLFunctionBase *> volFuncVec;
				volCreator->createVolatility(volFuncVec, param.paraFile, &param, &objPool, -1);
				vol->setVolatility(volFuncVec);
			}
		}
		else if (inputType == INPUT_FUNC_SCALAR)
		{
			// method scalar input case
			AQLFunctionBase *volFunc = volCreator->createVolatility(param.paraFile, &param, &objPool);	
			vol->setVolatility(volFunc);
		}
		else
		{
			AQLString msg = "This input type is not support, input type = " + inputType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

	}
	// shift matrix input case
	else if (!param.paraShiftMtx.empty())
	{
		AQLString shiftType = param.shiftType;
		shiftType.toUpper();

		// set matrix val
		if (inputType == INPUT_T_DATA_MATRIX)
		{
			DoubleMatrix volMtx;
			DoubleArray grid_t;
			volCreator->createVolatility(grid_t, volMtx, param.paraFile, &param, &objPool);

			const unsigned int size_m = volMtx.size();

			if (param.paraShiftMtx.size() < size_m)
			{
				throw AQLCoreInvalidData("Volatility parallel shift matrix must be same or more than tenor size", __FILE__, __LINE__);
			}

			// diff
			for (unsigned int i = 0; i < size_m; ++i)
			{
				double shiftVal = 0.0;
				const unsigned int size_t = volMtx[i].size();
				for (unsigned int j = 0; j < size_t; ++j)
				{
					if (shiftType == RISK_SHIFTTYPE_DIFF)
					{
						// diff
						if (j < param.paraShiftMtx.size() && i < param.paraShiftMtx[j].size())
						{
							shiftVal =  param.paraShiftMtx[j][i] / 100.0;
						}
					}
					else
					{
						// ratio
						if (j < param.paraShiftMtx.size() && i < param.paraShiftMtx[j].size())
						{
							shiftVal = volMtx[j][i] *  param.paraShiftMtx[j][i] / 100.0;
						}
					}

					volMtx[i][j] += shiftVal;	///// for XLL plus
					volMtx[i][j] = AQLMath::max(volMtx[i][j], 0.0);
				}
			}
			vol->setVolatility(grid_t, volMtx);
		}
		else if (inputType == INPUT_T_FUNC_VECTOR)
		{
			// method vector input case
			AQLString model = param.model;
			model.toUpper();
			AQLString bumpType = param.bumpType;
			bumpType.toUpper();

			if (model == MODEL_LMM && bumpType == RISK_MODEL_VOL_BUMP) {
				// create vol data
				DoubleMatrix volMtx;
				DoubleArray grid_t;
				volCreator->getGrid_T(grid_t, param.paraFile);// grid_t[0] != 0.0

				double zero = 0.0;
				grid_t.insert(grid_t.begin(), zero);
				unsigned int marketSize = grid_t.size() - 1;
				const unsigned int tSize = grid_t.size();

				if (param.paraShiftMtx.size() < marketSize)
				{
					throw AQLCoreInvalidData("Volatility parallel shift matrix must be same or more than tenor size", __FILE__, __LINE__);
				}

				vector<AQLFunctionBase *> volFuncVec;
				if (! is_vol_clone)
				{
					volCreator->createVolatility(volFuncVec, param.paraFile, &param, &objPool, -1);
				}
				else
				{
					for(unsigned int i = 0; i < marketSize; i++)
					{
						AQLFunctionBase* volFunc = vol->getVolatilityFunc(i, 0);
						volFuncVec.push_back(volFunc);
					}
				}

				volMtx.resize(marketSize);
				// set volatility val
				for (unsigned int i = 0; i < marketSize; ++i)
				{
					volMtx[i].resize(tSize, 0.0);
					for (unsigned int j = 0; j < tSize; ++j)
					{
						volMtx[i][j] = volFuncVec[i]->operator ()(grid_t[j]);
					}

					// diff
					double shiftVal = 0.0;
					for (unsigned int j = 0; j < tSize; ++j)
					{
						if (shiftType == RISK_SHIFTTYPE_DIFF)
						{
							// diff
							if (j < param.paraShiftMtx.size() && i < param.paraShiftMtx[j].size())
							{
								shiftVal =  param.paraShiftMtx[j][i] / 100.0;
							}
						}
						else
						{
							// ratio
							if (j < param.paraShiftMtx.size() && i < param.paraShiftMtx[j].size())
							{
								shiftVal = volMtx[j][i] *  param.paraShiftMtx[j][i] / 100.0;
							}
						}

						volMtx[i][j] += shiftVal;	///// for XLL plus
						volMtx[i][j] = AQLMath::max(volMtx[i][j], 0.0);
					}

					delete volFuncVec[i];
				}

				vol->setVolatility(grid_t, volMtx);
			}
			else
			{
				vector<AQLFunctionBase *> volFuncVec;
				volCreator->createVolatility(volFuncVec, param.paraFile, &param, &objPool, -1);
				vol->setVolatility(volFuncVec);
			}
		}
		else
		{
			AQLString msg = "This input type is not supported, input type = " + inputType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
	else
	{
		if (inputType == INPUT_T_FUNC_VECTOR)
		{
			// method vector input case
			vector<AQLFunctionBase *> volFuncVec;
			volCreator->createVolatility(volFuncVec, param.paraFile, &param, &objPool);
			vol->setVolatility(volFuncVec);
		}
		else if (inputType == INPUT_FUNC_SCALAR)
		{
			// method scalar input case
			AQLFunctionBase *volFunc = volCreator->createVolatility(param.paraFile, &param, &objPool);	
			vol->setVolatility(volFunc);
		}
	}

	delete volCreator;

	return ret;
}

/*!
    @brief ceate grid shift vol

	@param[in] dataInstance
	@param[in] baseVol
	@param[in] param
	@return vector<AQLObject *>
*/
vector<AQLObject *>
LAScenarioConfigurationVolatility::createGridShiftVol(AQLDataInstance &dataInstance, AQLMathVolatility &baseVol, const MAScenarioParam &param) const
{
	vector<AQLObject *> ret(0);
	AQLString inputType = param.inputType;
	inputType.toUpper();

	AQLObjectPool &objPool = dataInstance.getObjectPool();
	// get volatility creator
	LACalibrateVolatility *volCreator = LACalibrateVolatilityManager::getInstance()->createVolCreator(param.model);
	// shift val input case
	if (!param.gridShiftVec.empty())
	{
		AQLString shiftType = param.shiftType;
		shiftType.toUpper();

		if (inputType == INPUT_T_DATA_MATRIX)
		{
			// get coordinate
			DoubleMatrix cMatrix;
			getCoordinatesMatrix(param.gridTerm, cMatrix);
			const unsigned int size_c = cMatrix.size();

			double shiftVal = param.gridShiftVec[0];

			for (unsigned int i = 0; i < size_c; ++i)
			{
				DoubleMatrix volMtx;
				DoubleArray grid_t;
				volCreator->createVolatility(grid_t, volMtx, param.gridFile[0], &param, &objPool);
				DoubleArray grid_T;
				volCreator->getGrid_T(grid_T, param.gridFile[0]);

				// search coordinates matrix (t,T)
				unsigned int pos_t_s, pos_t_e, pos_T_s, pos_T_e;
				searchCoordinatesMatrix(grid_t, grid_T, cMatrix[i], pos_t_s, pos_t_e, pos_T_s, pos_T_e);

				for (unsigned int j = pos_t_s; j < pos_t_e; ++j)
				{

					for (unsigned int k = pos_T_s; k < pos_T_e; ++k)
					{
						if (shiftType == RISK_SHIFTTYPE_DIFF)
						{
							// diff
							volMtx[k][j] += shiftVal;
						}
						else
						{
							// ratio
							volMtx[k][j] += volMtx[k][j] * shiftVal;
						}
						volMtx[k][j] = AQLMath::max(volMtx[k][j], 0.0);
					}
				}

				AQLString name = param.targetName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" + AQLString(static_cast<int>(i));
				AQLMathVolatility *vol = 0;
				AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					vol = dynamic_cast<AQLMathVolatility *>(baseVol.clone());
					vol->getName() = name;
				}
				else
				{
					vol = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
				}
				
				vol->setVolatility(grid_t, volMtx);
				ret.push_back(vol);
			}
			
		}
		else if (inputType == INPUT_T_FUNC_VECTOR)
		{
			// method vector input case
			AQLString model = param.model;
			model.toUpper();
			AQLString bumpType = param.bumpType;
			bumpType.toUpper();

			if (model == MODEL_LMM && bumpType == RISK_MODEL_VOL_BUMP)
			{
				// get coordinate
				DoubleMatrix cMatrix;
				getCoordinatesMatrix(param.gridTerm, cMatrix);
				const unsigned int size_c = cMatrix.size();

				double shiftVal = param.gridShiftVec[0];

				DoubleArray grid_T;
				volCreator->getGrid_T(grid_T, param.gridFile[0]);

				DoubleArray grid_t = grid_T;
				double zero = 0.0;
				grid_t.insert(grid_t.begin(), zero);
				unsigned int marketSize = grid_t.size() - 1;
				const unsigned int tSize = grid_t.size();

				for (unsigned int i = 0; i < size_c; ++i)
				{
					// search coordinates matrix (t,T)
					unsigned int pos_t_s, pos_t_e, pos_T_s, pos_T_e;
					searchCoordinatesMatrix(grid_t, grid_T, cMatrix[i], pos_t_s, pos_t_e, pos_T_s, pos_T_e);

					bool is_vol_clone = false;
					AQLString name = param.targetName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" + AQLString(static_cast<int>(i));
					AQLMathVolatility *vol = 0;
					AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
					if (!objHolder.isDefined())
					{
						vol = dynamic_cast<AQLMathVolatility *>(baseVol.clone());
						vol->getName() = name;
						is_vol_clone = true;
					}
					else
					{
						vol = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
					}

					DoubleMatrix volMtx;
					vector<AQLFunctionBase *> volFuncVec;
					if (! is_vol_clone)
					{
						volCreator->createVolatility(volFuncVec, param.gridFile[i], &param, &objPool, i);
					}
					else
					{
						for(unsigned int i = 0; i < marketSize; i++)
						{
							AQLFunctionBase* volFunc = vol->getVolatilityFunc(i, 0);
							volFuncVec.push_back(volFunc);
						}
					}

					volMtx.resize(marketSize);
					// set volatility val
					for (unsigned int j = 0; j < marketSize; ++j)
					{
						volMtx[j].resize(tSize, 0.0);
						for (unsigned int k = 0; k < tSize; ++k)
						{
							volMtx[j][k] = volFuncVec[j]->operator ()(grid_t[k]);
						}

						delete volFuncVec[j];
					}

					for (unsigned int j = pos_t_s; j < pos_t_e; ++j)
					{

						for (unsigned int k = pos_T_s; k < pos_T_e; ++k)
						{
							if (shiftType == RISK_SHIFTTYPE_DIFF)
							{
								// diff
								volMtx[k][j] += shiftVal;
							}
							else
							{
								// ratio
								volMtx[k][j] += volMtx[k][j] * shiftVal;
							}
							volMtx[k][j] = AQLMath::max(volMtx[k][j], 0.0);
						}
					}

					
					vol->setVolatility(grid_t, volMtx);
					ret.push_back(vol);
				}
			}
			else
			{
				unsigned int size_g = param.gridFile.size();
				for (unsigned int i = 0; i < size_g; ++i)
				{
					AQLString name = param.targetName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" + AQLString(static_cast<int>(i));
					AQLMathVolatility *vol = 0;
					AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
					if (!objHolder.isDefined())
					{
						vol = dynamic_cast<AQLMathVolatility *>(baseVol.clone());
						vol->getName() = name;
					}
					else
					{
						vol = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
					}
					vector<AQLFunctionBase *> volFuncVec;
					volCreator->createVolatility(volFuncVec, param.gridFile[i], &param, &objPool, i);

					vol->setVolatility(volFuncVec);
					ret.push_back(vol);
				}
			}
		}
		else if (inputType == INPUT_FUNC_SCALAR)
		{
			// method scalar input case
			unsigned int size_g = param.gridFile.size();
			bool isBucket = false;
			// get grid pos (especially, for bucket term)
			IntArray pos;
			for (unsigned int j = 0;j < param.gridGroupID.size();++j)
			{
				//if (param.gridGroupID[j] != param.gridGroupID[j + 1] && j != param.gridGroupID.size() - 1)
				if ( j < param.gridGroupID.size() - 1 )
				{
					if (param.gridGroupID[j] != param.gridGroupID[j + 1] )
					{
						pos.push_back(j);
						continue;
					}
				}
				if (j == param.gridGroupID.size() - 1)
					pos.push_back(param.gridGroupID.size() - 1);
			}
			if (pos.size() > 1)
			{
				size_g = pos.size();
				isBucket = true;
			}

			for (unsigned int i = 0; i < size_g; ++i)
			{
				AQLString name = param.targetName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" + AQLString(static_cast<int>(i));
				AQLMathVolatility *vol = 0;
				AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					vol = dynamic_cast<AQLMathVolatility *>(baseVol.clone());
					vol->getName() = name;
				}
				else
				{
					vol = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
				}
				AQLFunctionBase *volFunc;
				if (isBucket)
				{
					 volFunc = volCreator->createVolatility(param.gridFile[i], &param, &objPool, pos[i]);
				}
				else
				{
					volFunc = volCreator->createVolatility(param.gridFile[i], &param, &objPool, i);
				}
				vol->setVolatility(volFunc);
				ret.push_back(vol);
			}
		} 
		else
		{
			AQLString msg = "This input type is not support, input type = " + inputType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
	// shift matrix input case
	else if (!param.paraShiftMtx.empty())
	{
		AQLString shiftType = param.shiftType;
		shiftType.toUpper();
		// get coordinate
		DoubleMatrix cMatrix;
		getCoordinatesMatrix(param.gridTerm, cMatrix);
		const unsigned int size_c = cMatrix.size();

		// set matrix val
		if (inputType == INPUT_T_DATA_MATRIX)
		{
			for (unsigned int i = 0; i < size_c; ++i)
			{
				DoubleMatrix volMtx;
				DoubleArray grid_t;
				volCreator->createVolatility(grid_t, volMtx, param.gridFile[0], &param, &objPool);
				DoubleArray grid_T;
				volCreator->getGrid_T(grid_T, param.gridFile[0]);

				// search coordinates matrix (t,T)
				unsigned int pos_t_s, pos_t_e, pos_T_s, pos_T_e;
				searchCoordinatesMatrix(grid_t, grid_T, cMatrix[i], pos_t_s, pos_t_e, pos_T_s, pos_T_e);

				for (unsigned int j = pos_t_s; j < pos_t_e; ++j)
				{
					for (unsigned int k = pos_T_s; k < pos_T_e; ++k)
					{
						double shiftVal = 0.0;
						if (shiftType == RISK_SHIFTTYPE_DIFF)
						{
							// diff
							if (j < param.gridShiftMtx.size() && k < param.gridShiftMtx[j].size())
							{
								shiftVal =  param.gridShiftMtx[j][k] / 100.0;
							}
						}
						else
						{
							// ratio
							if (j < param.gridShiftMtx.size() && k < param.gridShiftMtx[j].size())
							{
								shiftVal =  volMtx[k][j] * param.gridShiftMtx[j][k] / 100.0;
							}
						}
						volMtx[k][j] += shiftVal;
						volMtx[k][j] = AQLMath::max(volMtx[k][j], 0.0);

					}
				}

				AQLString name = param.targetName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" + AQLString(static_cast<int>(i));
				AQLMathVolatility *vol = 0;
				AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					vol = dynamic_cast<AQLMathVolatility *>(baseVol.clone());
					vol->getName() = name;
				}
				else
				{
					vol = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
				}
				
				vol->setVolatility(grid_t, volMtx);
				ret.push_back(vol);
			}
		}
		else if (inputType == INPUT_T_FUNC_VECTOR)
		{
			AQLString model = param.model;
			model.toUpper();
			AQLString bumpType = param.bumpType;
			bumpType.toUpper();

			if (model == MODEL_LMM && bumpType == RISK_MODEL_VOL_BUMP)
			{
				DoubleArray grid_T;
				volCreator->getGrid_T(grid_T, param.gridFile[0]);

				DoubleArray grid_t = grid_T;
				double zero = 0.0;
				grid_t.insert(grid_t.begin(), zero);
				unsigned int marketSize = grid_t.size() - 1;
				const unsigned int tSize = grid_t.size();

				for (unsigned int i = 0; i < size_c; ++i)
				{
					// search coordinates matrix (t,T)
					unsigned int pos_t_s, pos_t_e, pos_T_s, pos_T_e;
					searchCoordinatesMatrix(grid_t, grid_T, cMatrix[i], pos_t_s, pos_t_e, pos_T_s, pos_T_e);

					bool is_vol_clone = false;
					AQLString name = param.targetName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" + AQLString(static_cast<int>(i));
					AQLMathVolatility *vol = 0;
					AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
					if (!objHolder.isDefined())
					{
						vol = dynamic_cast<AQLMathVolatility *>(baseVol.clone());
						vol->getName() = name;
						is_vol_clone = true;
					}
					else
					{
						vol = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
					}

					DoubleMatrix volMtx;
					vector<AQLFunctionBase *> volFuncVec;
					if (! is_vol_clone)
					{
						volCreator->createVolatility(volFuncVec, param.gridFile[i], &param, &objPool, i);
					}
					else
					{
						for(unsigned int i = 0; i < marketSize; i++)
						{
							AQLFunctionBase* volFunc = vol->getVolatilityFunc(i, 0);
							volFuncVec.push_back(volFunc);
						}
					}

					volMtx.resize(marketSize);
					// set volatility val
					for (unsigned int j = 0; j < marketSize; ++j)
					{
						double shiftVal = 0.0;
						volMtx[j].resize(tSize, 0.0);
						for (unsigned int k = 0; k < tSize; ++k)
						{
							volMtx[j][k] = volFuncVec[j]->operator ()(grid_t[k]);
						}

						delete volFuncVec[j];
					}

					for (unsigned int j = pos_t_s; j < pos_t_e; ++j)
					{
						for (unsigned int k = pos_T_s; k < pos_T_e; ++k)
						{
							double shiftVal = 0.0;
							if (shiftType == RISK_SHIFTTYPE_DIFF)
							{
								// diff
								if (j < param.gridShiftMtx.size() && k < param.gridShiftMtx[j].size())
								{
									shiftVal =  param.gridShiftMtx[j][k] / 100.0;
								}
							}
							else
							{
								// ratio
								if (j < param.gridShiftMtx.size() && k < param.gridShiftMtx[j].size())
								{
									shiftVal =  volMtx[k][j] * param.gridShiftMtx[j][k] / 100.0;
								}
							}
							volMtx[k][j] += shiftVal;
							volMtx[k][j] = AQLMath::max(volMtx[k][j], 0.0);

						}
					}

					vol->setVolatility(grid_t, volMtx);
					ret.push_back(vol);
				}
			}
			else
			{
				AQLString msg = "This input type is not supported, input type = " + inputType;
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
		else
		{
			AQLString msg = "This input type is not supported, input type = " + inputType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
	else
	{
		if (inputType == INPUT_T_FUNC_VECTOR)
		{
			// method vector input case
			unsigned int size_g = param.gridFile.size();
			for (unsigned int i = 0; i < size_g; ++i)
			{
				AQLString name = param.targetName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" + AQLString(static_cast<int>(i));
				AQLMathVolatility *vol = 0;
				AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					vol = dynamic_cast<AQLMathVolatility *>(baseVol.clone());
					vol->getName() = name;
				}
				else
				{
					vol = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
				}
				vector<AQLFunctionBase *> volFuncVec;
				volCreator->createVolatility(volFuncVec, param.gridFile[i], &param, &objPool, i);
				vol->setVolatility(volFuncVec);
				ret.push_back(vol);
			}
		}
		else if (inputType == INPUT_FUNC_SCALAR)
		{
			// method scalar input case
			unsigned int size_g = param.gridFile.size();
			for (unsigned int i = 0; i < size_g; ++i)
			{
				AQLString name = param.targetName + "_" + param.calcType + "_" + param.targetCurveType + "_Grid_" + AQLString(static_cast<int>(i));
				AQLMathVolatility *vol = 0;
				AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					vol = dynamic_cast<AQLMathVolatility *>(baseVol.clone());
					vol->getName() = name;
				}
				else
				{
					vol = &dynamic_cast<AQLMathVolatility &>(objHolder.get());
				}
				AQLFunctionBase *volFunc = volCreator->createVolatility(param.gridFile[i], &param, &objPool, i);
				vol->setVolatility(volFunc);
				ret.push_back(vol);
			}
		}
	}

	delete volCreator;

	return ret;
}

/*!
    @brief seartch coordinates matrix

	@param[in] grid_t
	@param[in] grid_T
	@param[in] area
	@param[out] pos_t_s
	@param[out] pos_t_e
	@param[out] pos_T_s
	@param[out] pos_T_e
	
*/
void 
LAScenarioConfigurationVolatility::searchCoordinatesMatrix(const DoubleArray &grid_t, const DoubleArray &grid_T,
													 const DoubleArray &area,
													 unsigned int &pos_t_s,
													 unsigned int &pos_t_e,
													 unsigned int &pos_T_s,
													 unsigned int &pos_T_e) const
{
	LACalibrationUtilities<double>::searchNearestPos(grid_t, area[0], pos_t_s);
	LACalibrationUtilities<double>::searchNearestPos(grid_t, area[1], pos_t_e);
	LACalibrationUtilities<double>::searchNearestPos(grid_T, area[2], pos_T_s);
	LACalibrationUtilities<double>::searchNearestPos(grid_T, area[3], pos_T_e);

	if (pos_t_e == grid_t.size() - 1)
	{
		++pos_t_e;
	}
	if (pos_T_e == grid_T.size() - 1)
	{
		++pos_T_e;
	}
}

/*!
    @brief get grid matrix

	@param[in] term
	@param[out] matrix
	
*/
void 
LAScenarioConfigurationVolatility::getCoordinatesMatrix(const AQLStringVector &term, DoubleMatrix &matrix) const
{
	matrix.clear();
	// day count
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT));
	// asOfDate
	AQLDate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	const unsigned int COORDINATESNUM = 4;
	const unsigned int size = term.size();
	matrix.resize(size);
	for (unsigned int i = 0; i < size; ++i)
	{
		AQLStringVector cdns = term[i].toToken('_');
		if (cdns.size() != COORDINATESNUM)
		{
			throw AQLCoreInvalidData(" IR vega coordinate grid format is wrong .", __FILE__, __LINE__);
		}
		matrix[i].resize(COORDINATESNUM);
		for (unsigned int j = 0; j < COORDINATESNUM; ++j)
		{
			matrix[i][j] = dayCount.getTerm(asOfDate, AQLMathDateCalculations::getDate(asOfDate, cdns[j], true));
		}
	}

}

