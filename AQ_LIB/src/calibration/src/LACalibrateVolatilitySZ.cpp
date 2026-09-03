// $Id: LACalibrateVolatilitySZ.cpp,v 1.2 2008/09/17 23:31:15 hirayake Exp $
/*! @file
    @brief Volatility data create class for SZ
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilitySZ.cpp
//
//  DESCRIPTION :      Volatility data create class for SZ
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


#include "LACalibrateVolatilitySZ.h"
#include "LACoreDataService.h"
#include "LAMarketData.h"
#include "LADealUtils.h"
#include "LAScenarioConfiguration.h"
#include "LADate.h"
#include "LAPriceDataDayCount.h"
#include "LAMathFXUtility.h"
#include "LAMathVolFuncSZDD.h"
#include "LAPriceFXVolatility.h"
#include "LAConstant.h"
#include "LABasic.h"
#include "LACalibrationFunc.h"
#include "LACalibrateSZ.h"
#include "LACalibratePool.h"
#include "LAMarketDataHW.h"
//////////////////////////////////////////////////////// for XLL Plus
#include "LAStaticData.h"
/////////////////////////////////////////////////////////////////////

using namespace std;

// constructor
/*!

*/
LACalibrateVolatilitySZ::LACalibrateVolatilitySZ(void)
{
}

// destructor
/*!

*/
LACalibrateVolatilitySZ::~LACalibrateVolatilitySZ(void)
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
LACalibrateVolatilitySZ::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function matrix (T*i*f(t))

	@param[out] vol		Volatility as function matrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilitySZ::createVolatility(vector<vector<LAFunctionBase *> > &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
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
LACalibrateVolatilitySZ::createVolatility(DoubleArray &grid_T, vector<LAFunctionBase *> &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
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
LACalibrateVolatilitySZ::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility as function vector (T*f(t))

	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilitySZ::createVolatility(vector<LAFunctionBase *> &vol, const LAStringVector &filePath, 
											const MAScenarioParam *param, LAObjectPool *objPool, int gridPos) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);

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
LACalibrateVolatilitySZ::createVolatility(DoubleArray &grid_T, LAFunctionBase *vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
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
LACalibrateVolatilitySZ::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function for fx (f(t))

	@param[in] filepath
	@param[in] param
	@param[in] objPool
	@param[return]  vol		Volatility as function 

*/
LAFunctionBase * 
LACalibrateVolatilitySZ::createVolatility(const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool, int gridPos) const
{
	// null check
	if (!param || !objPool)
	{
		throw LACoreInvalidData("Param or entitypool is null .", __FILE__, __LINE__);
	}
	// refrence check
	if (param->refName.size() < 2)
	{
		throw LACoreInvalidData("Reference object size must be more than 2.", __FILE__, __LINE__);
	}

	if (param->isCalib)
	{
		MACalibrationFunc *method = new MACalibrationFunc();
		LACalibrateSZ *request = 0;
		request = new LACalibrateSZ();
		// set up
		request->setUp(*objPool, *param, method, gridPos);
		if (LACoreDataService::getContext(ARG_KEY_NOCALIBTHREAD) == MLIB_NO_DATA)
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

		return method;
	}
	else
	{
		MAFileAccessor file(filePath[0]);
		LAStringMatrix paramMtx;
		file.readAllData(MARKET_DATA_DELIMITER, paramMtx);
		file.close();
		MAFileAccessor file_initVol(filePath[1]);
		LAStringMatrix initVol;
		file_initVol.readAllData(MARKET_DATA_DELIMITER, initVol);
		file_initVol.close();
		
		// calc timegrid
		DoubleArray timeGrid;
		int maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
		LAString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
		LAPriceDataDayCount dayCount;
		dayCount.convertFromString(dayCountStr);
		LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		// create tenor SA
	    LAString cFreq = LACoreDataService::getContext(ARG_KEY_CANONICALFREQ);
       	//LACoreDataService::setContext(ARG_KEY_CANONICALFREQ, "SA");
		LAMarketDataHW::getCanonicalGrid(timeGrid, asOfDate, dayCount, maxTerm, false, &cFreq);
	    //LACoreDataService::setContext(ARG_KEY_CANONICALFREQ, cFreq);

		const int size_t = timeGrid.size();
		// param for method
		DoubleArray fTimeGrid;
		DoubleArray fBeta;
		DoubleArray fTheta;
		DoubleArray fKappa;
		DoubleArray fEpsilon;
		DoubleArray fSigma(size_t, initVol[0][0].getDoubleValue());

		const int paramSize = paramMtx.size();
		if (paramSize < 5 || 
			paramMtx[0].size() < size_t || paramMtx[1].size() < size_t || paramMtx[2].size() < size_t)
		{
			throw LACoreInvalidData("FX volatility file format is wrong..", __FILE__, __LINE__);
		}
		const double INFINITESIMAL = 1E-7;

		for (int i = 0; i < size_t; ++i)
		{
			fTimeGrid.push_back(paramMtx[0][i].trimLeft().trimRight().getDoubleValue());
			fKappa.push_back(paramMtx[1][i].trimLeft().trimRight().getDoubleValue());
			fBeta.push_back(paramMtx[2][i].trimLeft().trimRight().getDoubleValue());
			fTheta.push_back(paramMtx[3][i].trimLeft().trimRight().getDoubleValue());
			fEpsilon.push_back(paramMtx[4][i].trimLeft().trimRight().getDoubleValue());
		}
		// check timegrid;
		for (int i =0; i <size_t; ++i)
		{
			if (LAMath::abs(timeGrid[i] - fTimeGrid[i]) > INFINITESIMAL)
			{
				LAString msg = "CanonicTimeGrid = " + LAString(timeGrid[i]) + ", FileTimeGrid = " + LAString(fTimeGrid[i]);
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}

		const LAMathYieldCurve &dYield = dynamic_cast<const LAMathYieldCurve &>(objPool->getObject(param->refName[0], ENCHKTYPE_ISDEFINED).get());
		const LAMathYieldCurve &fYield = dynamic_cast<const LAMathYieldCurve &>(objPool->getObject(param->refName[1], ENCHKTYPE_ISDEFINED).get());

		LAMathFXEntity fx_tmp = *LAMarketData::getFXEntity(*objPool, "USEMODEL");
		fx_tmp.getFXType() = "FIXEDRATE";

		LAStringVector ccys;
		LAMarketData::convertToCurrency(param->ccy, ccys);
		// get spot rate
		double spotRate = fx_tmp.getRate(ccys[1], ccys[0], 0.0);
		//get base shift val
		spotRate += param->extraBaseParam;

		if (!param->extraParam.empty())
		{
			// shiftval check
			if (param->extraParam.size() != 1)
			{
				throw LACoreInvalidData("Extra param size must be 1 .", __FILE__, __LINE__);
			}
			double shiftVal = param->extraParam[0];
			LAString shiftType = param->shiftType;
			shiftType.toUpper();
			if (shiftType == RISK_SHIFTTYPE_RATIO)
			{
				shiftVal = spotRate * shiftVal;
			}
			// add shift val
			spotRate = LAMath::max(spotRate + shiftVal, 0.0);
		}
		LADate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		LADate spotDate = fx_tmp.getSpotDate(ccys[0], ccys[1], asofDate);
		spotRate *= dYield.getBasisDF(asofDate, spotDate) / fYield.getBasisDF(asofDate, spotDate);
		DoubleArray fFx(size_t);
		// calc forward fx
		for (int i = 0; i < size_t; ++i)
		{
			fFx[i] = spotRate * fYield.getBasisDF(timeGrid[i]) / dYield.getBasisDF(timeGrid[i]);
		}

		// ceck DDL
		LAMathVolFuncFX *funcFX = 0;
		funcFX = new LAMathVolFuncSZDD(timeGrid, fFx, fBeta, fTheta, fKappa, fEpsilon, param->ccy, fSigma);
		
		return funcFX;
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
LACalibrateVolatilitySZ::createVolatility(double &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw LACoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
}

/*!
    @brief get grid_T

	@param[out] grid_T as double 
	@param[in] filepath
*/
void 
LACalibrateVolatilitySZ::getGrid_T(DoubleArray &grid_T, const LAStringVector &filePath) const
{
	filePath;
	// SZ(FX) has no grid_T
	grid_T.clear();
}



