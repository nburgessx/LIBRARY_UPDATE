/*! @file
    @brief Volatility data create class for hw
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilityHW.cpp
//
//  DESCRIPTION :      Volatility data create class for hw
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


#include "LACalibrateVolatilityHW.h"
#include "LADataReference.h"
#include "LACoreDataService.h"
#include "LAMarketData.h"
#include "LAScenarioConfiguration.h"
#include "LADate.h"
#include "LABasic.h"
#include "LALinearInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAPriceDataDayCount.h"
#include "LAMathHWFuncMR.h"
#include "LAMathHWFuncSigma.h"
#include "LAMathVolFuncHW.h"
#include "LAMarketDataHW.h"
#include "LACalibratePool.h"
#include "LACalibrateHW.h"
#include "LACalibrationFunc.h"
#include "LACalibrationParametersHW.h"
#include "LADefinitionsCalibration.h"
#include "LAMathJamshidianSwaption.h"

using namespace std;

// constructor
/*!
	@param funFlg

*/
LACalibrateVolatilityHW::LACalibrateVolatilityHW(void)
{
}

// destructor
/*!

*/
LACalibrateVolatilityHW::~LACalibrateVolatilityHW(void)
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
LACalibrateVolatilityHW::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function matrix (T*i*f(t))

	@param[out] vol		Volatility as function matrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityHW::createVolatility(vector<vector<LAFunctionBase *> > &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
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
LACalibrateVolatilityHW::createVolatility(DoubleArray &grid_T, vector<LAFunctionBase *> &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
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
LACalibrateVolatilityHW::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility as function vector (T*f(t))

	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityHW::createVolatility(vector<LAFunctionBase *> &vol, const LAStringVector &filePath, 
										const MAScenarioParam *param, LAObjectPool *objPool, int gridPos) const
{
	// null check
	if (!param || !objPool)
	{
		throw LACoreInvalidData("Param or entitypool is null.", __FILE__, __LINE__);
	}

	if (param->isCalib)
	{
		MACalibrationFunc *method = new MACalibrationFunc();
		LACalibrateHW *request = new LACalibrateHW();
		// set up
		request->setUp(*objPool, *param, method, gridPos);
		if (LACoreDataService::getContext(ARG_KEY_NOCALIBTHREAD) == AQ_NO_DATA)
		{
			LACalibratePool *reqChannel = LACalibratePool::getInstance();
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
			throw LACoreInvalidData("File path is empty", __FILE__, __LINE__);
		}

		//unsigned int index = 0;
		//if (param->is
		//if (gridPos == -1 && param->refName.empty())
		//{
		//	throw LACoreInvalidData("Reference name is empty", __FILE__, __LINE__);
		//}

		//if (gridPos >= 0 && param->refName.size() - 1 < gridPos)
		//{
		//	throw LACoreInvalidData("Reference name size is less than grid size", __FILE__, __LINE__);
		//}
		DoubleArray tenor;
		DoubleArray hw_a;
		DoubleArray hw_s;
		DoubleArray hw_s_zero;

		// load hw_a & hw_s
		LAMarketDataHW::getHullWhiteParam(filePath[0], tenor, hw_a, hw_s);

		// mean reversion parameter
		LAMathHWFuncMR * func_a =  new LAMathHWFuncMR(hw_a[0]);
		if (hw_s.size() == 1)
		{
			hw_s.resize(tenor.size(), hw_s[0]);
		}
		hw_s_zero.resize(tenor.size(), 1.E-12);

		// hull-white volatility parameter
		LAMathHWFuncSigma* func_s = new LAMathHWFuncSigmaTMDPT(tenor, (param->isZeroVol ? hw_s_zero : hw_s), *(new LAStepInterpolation()));
		vol.resize(1, new LAMathVolFuncHW(*func_a, *func_s));

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
			throw LACoreInvalidData("Scenario param refname YieldCurve name is need.", __FILE__, __LINE__);
		}

		// set object pool as calib data
		const LAMathYieldCurve &yc = dynamic_cast<LAMathYieldCurve &>(objPool->getObject(param->refName[index], ENCHKTYPE_ISDEFINED).get());
		LAString calibDataName = LAMarketData::getCalibDataName(param->calcType, yc.getYieldData().get().getName(), gridPos);
		LAObject *calibData = new LAObject();
		calibData->add(CALIBRATION_DATA_NAME, new LADataString(calibDataName));
		calibData->add(PRICING_DATA_CALIBCANONICAL_T, new LADataDoubles(tenor));
		calibData->add(PRICING_DATA_CALIBMEANREV_T, new LADataDoubles(hw_a));
		calibData->add(PRICING_DATA_CALIBVOL_T, new LADataDoubles(hw_s));

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
LACalibrateVolatilityHW::createVolatility(DoubleArray &grid_T, LAFunctionBase *vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
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
LACalibrateVolatilityHW::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function for fx (f(t))

	@param[in] filepath
	@param[in] param
	@param[in] objPool
	@return vol		Volatility as function 

*/
LAFunctionBase * 
LACalibrateVolatilityHW::createVolatility(const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool, int gridPos) const
{
	(void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as double

	@param[out] vol		Volatility as double 
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityHW::createVolatility(double &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in hw.", __FILE__, __LINE__);
}

/*!
    @brief get grid_T

	@param[out] grid_T as double 
	@param[in] filepath
*/
void 
LACalibrateVolatilityHW::getGrid_T(DoubleArray &grid_T, const LAStringVector &filePath) const
{
	(void)filePath;
	grid_T.clear();
	// get tenor info
	DoubleArray tenor;

	int term = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	LAString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	LAPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create tenor SA
    //LAString cFreq = LACoreDataService::getContext(ARG_KEY_CANONICALFREQ);
    //LACoreDataService::setContext(ARG_KEY_CANONICALFREQ, "SA");
	LAMarketDataHW::getCanonicalGrid(grid_T, asOfDate, dayCount, term);
	//LACoreDataService::setContext(ARG_KEY_CANONICALFREQ, cFreq);
	grid_T.erase(--grid_T.end());
	if (grid_T[0] == 0.0)
	{
		grid_T.erase(grid_T.begin());
	}
}
