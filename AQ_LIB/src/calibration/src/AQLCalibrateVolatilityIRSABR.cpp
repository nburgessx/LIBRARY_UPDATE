/*! @file
    @brief Volatility data create class for ptberg
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCalibrateVolatilityIRSABR.h"
#include "AQLCoreDataService.h"
#include "AQLMarketData.h"
#include "AQLDealUtils.h"
#include "AQLScenarioConfiguration.h"
#include "AQLDate.h"
#include "AQLPriceDataDayCount.h"
#include "AQLConstant.h"
#include "AQLBasic.h"
#include "AQLCalibrationFunc.h"
#include "AQLCalibrateIRSABR.h"
#include "AQLCalibratePool.h"

using namespace std;

// constructor
/*!

*/
AQLCalibrateVolatilityIRSABR::AQLCalibrateVolatilityIRSABR(void)
{
}

// destructor
/*!

*/
AQLCalibrateVolatilityIRSABR::~AQLCalibrateVolatilityIRSABR(void)
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
AQLCalibrateVolatilityIRSABR::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in irsabr.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function matrix (T*i*f(t))

	@param[out] vol		Volatility as function matrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityIRSABR::createVolatility(vector<vector<AQLFunctionBase *> > &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in irsabr.", __FILE__, __LINE__);
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
AQLCalibrateVolatilityIRSABR::createVolatility(DoubleArray &grid_T, vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_T, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in irsabr.", __FILE__, __LINE__);
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
AQLCalibrateVolatilityIRSABR::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in irsabr.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility as function vector (T*f(t))

	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityIRSABR::createVolatility(vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
											const AQLScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
{
	/*vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in irsabr.", __FILE__, __LINE__);*/

	// null check
	if (!param || !objPool)
	{
		throw AQLCoreInvalidData("Param or entitypool is null .", __FILE__, __LINE__);
	}
	
	if (param->isCalib)
	{
		if (param->refName[0] != AQ_NO_DATA)
		{
			AQLCalibrationFunc method;
			AQLCalibrateIRSABR request;
			request.setUp(*objPool,*param,&method,gridPos);
			request.doCalibrate();
			
			vol.resize(1, dynamic_cast<AQLFunctionBase *>(method.clone()));
		}
	}
	else
	{
		throw AQLCoreInvalidData("Only Calibration supports",__FILE__,__LINE__); 
	}
	return;

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
AQLCalibrateVolatilityIRSABR::createVolatility(DoubleArray &grid_T, AQLFunctionBase *vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_T, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in irsabr.", __FILE__, __LINE__);
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
AQLCalibrateVolatilityIRSABR::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	grid_t, vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in irsabr.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function for fx (f(t))

	@param[in] filepath
	@param[in] param
	@param[in] objPool
	@param[return]  vol		Volatility as function 

*/
AQLFunctionBase * 
AQLCalibrateVolatilityIRSABR::createVolatility(const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
{
	//// null check
	//if (!param || !objPool)
	//{
	//	throw AQLCoreInvalidData("Param or entitypool is null .", __FILE__, __LINE__);
	//}
	//
	//if (param->isCalib)
	//{
	//	AQLCalibrationFunc method;
	//	AQLCalibrateIRSABR request;
	//	request.setUp(*objPool,*param,&method,gridPos);
	//	request.doCalibrate();

	//	return dynamic_cast<AQLFunctionBase *>(method.clone());

	//}
	//else
	//{
	//	throw AQLCoreInvalidData("Only Calibration supports",__FILE__,__LINE__); 
	//}	
	throw AQLCoreInvalidData("This create method is not support in irsabr.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as double

	@param[out] vol		Volatility as double 
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityIRSABR::createVolatility(double &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in irsabr.", __FILE__, __LINE__);
}

/*!
    @brief get grid_T

	@param[out] grid_T as double 
	@param[in] filepath
*/
void 
AQLCalibrateVolatilityIRSABR::getGrid_T(DoubleArray &grid_T, const AQLStringVector &filePath) const
{
	filePath;
	// IRSABR(FX) has no grid_T
	grid_T.clear();
}

