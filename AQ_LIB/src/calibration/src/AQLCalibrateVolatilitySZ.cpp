// $Id: AQLCalibrateVolatilitySZ.cpp,v 1.2 2008/09/17 23:31:15 hirayake Exp $
/*! @file
    @brief Volatility data create class for SZ
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCalibrateVolatilitySZ.h"
#include "AQLCoreDataService.h"
#include "AQLMarketData.h"
#include "AQLDealUtils.h"
#include "AQLScenarioConfiguration.h"
#include "AQLDate.h"
#include "AQLPriceDataDayCount.h"
#include "AQLMathFXUtility.h"
#include "AQLMathVolFuncSZDD.h"
#include "AQLPriceFXVolatility.h"
#include "AQLConstant.h"
#include "AQLBasic.h"
#include "AQLCalibrationFunc.h"
#include "AQLCalibrateSZ.h"
#include "AQLCalibratePool.h"
#include "AQLMarketDataHW.h"
//////////////////////////////////////////////////////// for XLL Plus
#include "AQLStaticData.h"
/////////////////////////////////////////////////////////////////////

using namespace std;

// constructor
/*!

*/
AQLCalibrateVolatilitySZ::AQLCalibrateVolatilitySZ(void)
{
}

// destructor
/*!

*/
AQLCalibrateVolatilitySZ::~AQLCalibrateVolatilitySZ(void)
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
AQLCalibrateVolatilitySZ::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function matrix (T*i*f(t))

	@param[out] vol		Volatility as function matrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilitySZ::createVolatility(vector<vector<AQLFunctionBase *> > &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
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
AQLCalibrateVolatilitySZ::createVolatility(DoubleArray &grid_T, vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
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
AQLCalibrateVolatilitySZ::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility as function vector (T*f(t))

	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
AQLCalibrateVolatilitySZ::createVolatility(vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
											const AQLScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);

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
AQLCalibrateVolatilitySZ::createVolatility(DoubleArray &grid_T, AQLFunctionBase *vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
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
AQLCalibrateVolatilitySZ::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw AQLCoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function for fx (f(t))

	@param[in] filepath
	@param[in] param
	@param[in] objPool
	@param[return]  vol		Volatility as function 

*/
AQLFunctionBase * 
AQLCalibrateVolatilitySZ::createVolatility(const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
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
		AQLCalibrationFunc *method = new AQLCalibrationFunc();
		AQLCalibrateSZ *request = 0;
		request = new AQLCalibrateSZ();
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

		return method;
	}
	else
	{
		AQLFileAccessor file(filePath[0]);
		AQLStringMatrix paramMtx;
		file.readAllData(MARKET_DATA_DELIMITER, paramMtx);
		file.close();
		AQLFileAccessor file_initVol(filePath[1]);
		AQLStringMatrix initVol;
		file_initVol.readAllData(MARKET_DATA_DELIMITER, initVol);
		file_initVol.close();
		
		// calc timegrid
		DoubleArray timeGrid;
		int maxTerm = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
		AQLString dayCountStr = AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
		AQLPriceDataDayCount dayCount;
		dayCount.convertFromString(dayCountStr);
		AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		// create tenor SA
	    AQLString cFreq = AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ);
       	//AQLCoreDataService::setContext(ARG_KEY_CANONICALFREQ, "SA");
		AQLMarketDataHW::getCanonicalGrid(timeGrid, asOfDate, dayCount, maxTerm, false, &cFreq);
	    //AQLCoreDataService::setContext(ARG_KEY_CANONICALFREQ, cFreq);

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
			throw AQLCoreInvalidData("FX volatility file format is wrong..", __FILE__, __LINE__);
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
			if (AQLMath::abs(timeGrid[i] - fTimeGrid[i]) > INFINITESIMAL)
			{
				AQLString msg = "CanonicTimeGrid = " + AQLString(timeGrid[i]) + ", FileTimeGrid = " + AQLString(fTimeGrid[i]);
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}

		const AQLMathYieldCurve &dYield = dynamic_cast<const AQLMathYieldCurve &>(objPool->getObject(param->refName[0], ENCHKTYPE_ISDEFINED).get());
		const AQLMathYieldCurve &fYield = dynamic_cast<const AQLMathYieldCurve &>(objPool->getObject(param->refName[1], ENCHKTYPE_ISDEFINED).get());

		AQLMathFXEntity fx_tmp = *AQLMarketData::getFXEntity(*objPool, "USEMODEL");
		fx_tmp.getFXType() = "FIXEDRATE";

		AQLStringVector ccys;
		AQLMarketData::convertToCurrency(param->ccy, ccys);
		// get spot rate
		double spotRate = fx_tmp.getRate(ccys[1], ccys[0], 0.0);
		//get base shift val
		spotRate += param->extraBaseParam;

		if (!param->extraParam.empty())
		{
			// shiftval check
			if (param->extraParam.size() != 1)
			{
				throw AQLCoreInvalidData("Extra param size must be 1 .", __FILE__, __LINE__);
			}
			double shiftVal = param->extraParam[0];
			AQLString shiftType = param->shiftType;
			shiftType.toUpper();
			if (shiftType == RISK_SHIFTTYPE_RATIO)
			{
				shiftVal = spotRate * shiftVal;
			}
			// add shift val
			spotRate = AQLMath::max(spotRate + shiftVal, 0.0);
		}
		AQLDate asofDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		AQLDate spotDate = fx_tmp.getSpotDate(ccys[0], ccys[1], asofDate);
		spotRate *= dYield.getBasisDF(asofDate, spotDate) / fYield.getBasisDF(asofDate, spotDate);
		DoubleArray fFx(size_t);
		// calc forward fx
		for (int i = 0; i < size_t; ++i)
		{
			fFx[i] = spotRate * fYield.getBasisDF(timeGrid[i]) / dYield.getBasisDF(timeGrid[i]);
		}

		// ceck DDL
		AQLMathVolFuncFX *funcFX = 0;
		funcFX = new AQLMathVolFuncSZDD(timeGrid, fFx, fBeta, fTheta, fKappa, fEpsilon, param->ccy, fSigma);
		
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
AQLCalibrateVolatilitySZ::createVolatility(double &vol, const AQLStringVector &filePath, const AQLScenarioParam *param, AQLObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw AQLCoreInvalidData("This create method is not support in SZ.", __FILE__, __LINE__);
}

/*!
    @brief get grid_T

	@param[out] grid_T as double 
	@param[in] filepath
*/
void 
AQLCalibrateVolatilitySZ::getGrid_T(DoubleArray &grid_T, const AQLStringVector &filePath) const
{
	filePath;
	// SZ(FX) has no grid_T
	grid_T.clear();
}



