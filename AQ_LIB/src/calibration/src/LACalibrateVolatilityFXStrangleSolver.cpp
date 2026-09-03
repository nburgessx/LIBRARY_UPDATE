/*! @file
    @brief Volatility data create class for ptberg
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilityFXStrangleSolver.cpp
//
//  DESCRIPTION :      Volatility data create class for ptberg
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


#include "LACalibrateVolatilityFXStrangleSolver.h"
#include "LACoreDataService.h"
#include "LAMarketData.h"
#include "LADealUtils.h"
#include "LAScenarioConfiguration.h"
#include "AQLDate.h"
#include "AQLPriceDataDayCount.h"
#include "LAMathFXUtility.h"
#include "LAMathVolFuncFX.h"
#include "LAMathVolFuncFXDD.h"
#include "LAPriceFXVolatility.h"
#include "AQLConstant.h"
#include "AQLBasic.h"
#include "LACalibrationFunc.h"
#include "LACalibrateFXStrangleSolver.h"
#include "LACalibratePool.h"
#include "LAMarketDataHW.h"

using namespace std;

// constructor
/*!

*/
LACalibrateVolatilityFXStrangleSolver::LACalibrateVolatilityFXStrangleSolver(void)
{
}

// destructor
/*!

*/
LACalibrateVolatilityFXStrangleSolver::~LACalibrateVolatilityFXStrangleSolver(void)
{
}


 
/*!
    @brief create Volatility from grid_t and Volatility of DoubleMatrix vector (T*i*t)

	@param[out] grid_t	grid of t
	@param[out] vol		Volatility of DoubleMatrix vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityFXStrangleSolver::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function matrix (T*i*f(t))

	@param[out] vol		Volatility as function matrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityFXStrangleSolver::createVolatility(vector<vector<AQLFunctionBase *> > &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility from grid_t and Volatility of Function vector (i*f(t,T))

	@param[out] grid_T	grid of T
	@param[out] vol		Volatility of Function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityFXStrangleSolver::createVolatility(DoubleArray &grid_T, vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_T, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility from grid_t and Volatility of DoubleMatrix (T*t)

	@param[out] grid of t
	@param[out] vol		Volatility of DoubleMatrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityFXStrangleSolver::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility as function vector (T*f(t))

	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityFXStrangleSolver::createVolatility(vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
											const MAScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
{
	vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);

}


/*!
    @brief create Volatility from grid_t and Volatility of function (f(t,T))

	@param[out] grid_T	grid of T
	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityFXStrangleSolver::createVolatility(DoubleArray &grid_T, AQLFunctionBase *vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_T, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility from grid_t and Volatility of vector (t)

	@param[out] grid_t	grid of t
	@param[out] vol		Volatility as vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityFXStrangleSolver::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function for fx (f(t))

	@param[in] filepath
	@param[in] param
	@param[in] objPool
	@param[return]  vol		Volatility as function 

*/
AQLFunctionBase * 
LACalibrateVolatilityFXStrangleSolver::createVolatility(const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
{
	// null check
	if (!param || !objPool)
	{
		throw AQLCoreInvalidData("Param or entitypool is null .", __FILE__, __LINE__);
	}
	// refrence check
	if (param->refName.size() < 2)
	{
		throw AQLCoreInvalidData("Reference object size must be more than 2.", __FILE__, __LINE__);
	}

	if (param->isCalib)
	{
		MACalibrationFunc method;
		LACalibrateFXStrangleSolver request;
		request.setUp(*objPool,*param,&method,gridPos);
		request.doCalibrate();

		return dynamic_cast<AQLFunctionBase *>(method.clone());

	}
	else
	{
		throw AQLCoreInvalidData("Only Calibration supports",__FILE__,__LINE__); 
	}	
}

/*!
    @brief create Volatility as double

	@param[out] vol		Volatility as double 
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityFXStrangleSolver::createVolatility(double &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}

/*!
    @brief get grid_T

	@param[out] grid_T as double 
	@param[in] filepath
*/
void 
LACalibrateVolatilityFXStrangleSolver::getGrid_T(DoubleArray &grid_T, const AQLStringVector &filePath) const
{
	filePath;
	// FXStrangleSolver(FX) has no grid_T
	grid_T.clear();
}

