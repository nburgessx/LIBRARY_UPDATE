/*! @file
    @brief Volatility data create class for hw
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCalibrateVolatilityHW.h"
#include "AQLDataReference.h"
#include "AQLCoreDataService.h"
#include "AQLMarketData.h"
#include "AQLScenarioConfiguration.h"
#include "AQLDate.h"
#include "AQLBasic.h"
#include "AQLLinearInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLPriceDataDayCount.h"
#include "AQLMathHWFuncMR.h"
#include "AQLMathHWFuncSigma.h"
#include "AQLMathVolFuncHW.h"
#include "AQLMarketDataHW.h"
#include "AQLCalibratePool.h"
#include "AQLCalibrateHW.h"
#include "AQLCalibrationFunc.h"
#include "AQLCalibrationParametersHW.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLMathJamshidianSwaption.h"

using namespace std;

// constructor
/*!
	@param funFlg

*/
AQLCalibrateVolatilityHW::AQLCalibrateVolatilityHW(void)
{
}

// destructor
/*!

*/
AQLCalibrateVolatilityHW::~AQLCalibrateVolatilityHW(void)
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
AQLCalibrateVolatilityHW::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function matrix (T*i*f(t))

	@param[out] vol		Volatility as function matrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityHW::createVolatility(vector<vector<AQLFunctionBase *> > &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
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
AQLCalibrateVolatilityHW::createVolatility(DoubleArray &grid_T, vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
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
AQLCalibrateVolatilityHW::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility as function vector (T*f(t))

	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityHW::createVolatility(vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
										const AQLScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
{
	// null check
	if (!param || !objPool)
	{
		throw AQLCoreInvalidData("Param or entitypool is null.", __FILE__, __LINE__);
	}

	if (param->isCalib)
	{
		AQLCalibrationFunc *method = new AQLCalibrationFunc();
		AQLCalibrateHW *request = new AQLCalibrateHW();
		// set up
		request->setUp(*objPool, *param, method, gridPos);
		if (AQLCoreDataService::getContext(ARG_KEY_NOCALIBTHREAD) == AQ_NO_DATA)
		{
			AQLCalibratePool *reqChannel = AQLCalibratePool::getInstance();
			// put request ,calibration will be done in differenct thread.
			reqChannel->putRequest(request);
		}
		else
		{
			// calibrate in single thread
			request->doCalibrate();
			delete request;
		}
		vol.resize(1, method);
	}
	else
	{
		vol.clear();

		if (filePath.empty())
		{
			throw AQLCoreInvalidData("File path is empty", __FILE__, __LINE__);
		}

		//unsigned int index = 0;
		//if (param->is
		//if (gridPos == -1 && param->refName.empty())
		//{
		//	throw AQLCoreInvalidData("Reference name is empty", __FILE__, __LINE__);
		//}

		//if (gridPos >= 0 && param->refName.size() - 1 < gridPos)
		//{
		//	throw AQLCoreInvalidData("Reference name size is less than grid size", __FILE__, __LINE__);
		//}
		DoubleArray tenor;
		DoubleArray hw_a;
		DoubleArray hw_s;
		DoubleArray hw_s_zero;

		// load hw_a & hw_s
		AQLMarketDataHW::getHullWhiteParam(filePath[0], tenor, hw_a, hw_s);

		// mean reversion parameter
		AQLMathHWFuncMR * func_a =  new AQLMathHWFuncMR(hw_a[0]);
		if (hw_s.size() == 1)
		{
			hw_s.resize(tenor.size(), hw_s[0]);
		}
		hw_s_zero.resize(tenor.size(), 1.E-12);

		// hull-white volatility parameter
		AQLMathHWFuncSigma* func_s = new AQLMathHWFuncSigmaTMDPT(tenor, (param->isZeroVol ? hw_s_zero : hw_s), *(new AQLStepInterpolation()));
		vol.resize(1, new AQLMathVolFuncHW(*func_a, *func_s));

		unsigned int index = 0;
		if (param->isGrid)
		{
			// grid calc case
			if ( param->isParallel)
			{
				// parallel is also calc
				index = gridPos + 1;
			}
			else
			{
				// grid only
				index = gridPos;
			}
		}

		if (param->refName.size() - 1 <  index)
		{
			throw AQLCoreInvalidData("Scenario param refname YieldCurve name is need.", __FILE__, __LINE__);
		}

		// set object pool as calib data
		const AQLMathYieldCurve &yc = dynamic_cast<AQLMathYieldCurve &>(objPool->getObject(param->refName[index], ENCHKTYPE_ISDEFINED).get());
		AQLString calibDataName = AQLMarketData::getCalibDataName(param->calcType, yc.getYieldData().get().getName(), gridPos);
		AQLObject *calibData = new AQLObject();
		calibData->add(CALIBRATION_DATA_NAME, new AQLDataString(calibDataName));
		calibData->add(PRICING_DATA_CALIBCANONICAL_T, new AQLDataDoubles(tenor));
		calibData->add(PRICING_DATA_CALIBMEANREV_T, new AQLDataDoubles(hw_a));
		calibData->add(PRICING_DATA_CALIBVOL_T, new AQLDataDoubles(hw_s));

		objPool->set(calibDataName, calibData);
	}
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
AQLCalibrateVolatilityHW::createVolatility(DoubleArray &grid_T, AQLFunctionBase *vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
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
AQLCalibrateVolatilityHW::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function for fx (f(t))

	@param[in] filepath
	@param[in] param
	@param[in] objPool
	@return vol		Volatility as function 

*/
AQLFunctionBase * 
AQLCalibrateVolatilityHW::createVolatility(const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
{
	(void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as double

	@param[out] vol		Volatility as double 
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilityHW::createVolatility(double &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}

/*!
    @brief get grid_T

	@param[out] grid_T as double 
	@param[in] filepath
*/
void 
AQLCalibrateVolatilityHW::getGrid_T(DoubleArray &grid_T, const AQLStringVector &filePath) const
{
	(void)filePath;
	grid_T.clear();
	// get tenor info
	DoubleArray tenor;

	int term = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create tenor SA
    //AQLString cFreq = AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ);
    //AQLCoreDataService::setContext(ARG_KEY_CANONICALFREQ, "SA");
	AQLMarketDataHW::getCanonicalGrid(grid_T, asOfDate, dayCount, term);
	//AQLCoreDataService::setContext(ARG_KEY_CANONICALFREQ, cFreq);
	grid_T.erase(--grid_T.end());
	if (grid_T[0] == 0.0)
	{
		grid_T.erase(grid_T.begin());
	}
}
