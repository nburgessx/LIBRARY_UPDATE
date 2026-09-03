/*! @file
    @brief Volatility data create class for ptberg
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilityPtberg.cpp
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


#include "LACalibrateVolatilityPtberg.h"
#include "LACoreDataService.h"
#include "LAMarketData.h"
#include "LADealUtils.h"
#include "LAScenarioConfiguration.h"
#include "LADate.h"
#include "LAPriceDataDayCount.h"
#include "LAMathFXUtility.h"
#include "LAMathVolFuncFX.h"
#include "LAMathVolFuncFXDD.h"
#include "LAPriceFXVolatility.h"
#include "LAConstant.h"
#include "LABasic.h"
#include "LACalibrationFunc.h"
#include "LACalibratePtberg.h"
#include "LACalibratePtberg3F.h"
#include "LACalibratePool.h"
#include "LAMarketDataHW.h"
#include "LAPtbergUtils.h"
//////////////////////////////////////////////////////// for XLL Plus
#include "LADefinitionsPtberg.h"
#include "LAStaticData.h"
/////////////////////////////////////////////////////////////////////

using namespace std;

// constructor
/*!

*/
LACalibrateVolatilityPtberg::LACalibrateVolatilityPtberg(void)
{
}

// destructor
/*!

*/
LACalibrateVolatilityPtberg::~LACalibrateVolatilityPtberg(void)
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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function matrix (T*i*f(t))

	@param[out] vol		Volatility as function matrix
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityPtberg::createVolatility(vector<vector<LAFunctionBase *> > &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_T, vector<LAFunctionBase *> &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}


/*!
    @brief create Volatility as function vector (T*f(t))

	@param[out] vol		Volatility as function vector
	@param[in] filepath
	@param[in] param
	@param[in] objPool

*/
void 
LACalibrateVolatilityPtberg::createVolatility(vector<LAFunctionBase *> &vol, const LAStringVector &filePath, 
											const MAScenarioParam *param, LAObjectPool *objPool, int gridPos) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);

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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_T, LAFunctionBase *vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
	throw LACoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}

/*!
    @brief create Volatility as function for fx (f(t))

	@param[in] filepath
	@param[in] param
	@param[in] objPool
	@param[return]  vol		Volatility as function 

*/
LAFunctionBase * 
LACalibrateVolatilityPtberg::createVolatility(const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool, int gridPos) const
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
		LACalibratePtberg *request = 0;
		const bool is3F = MAPtbergUtils::is3FCalib();
		if (is3F)
		{
			request = new LACalibratePtberg3F();
		}
		else
		{
			request = new LACalibratePtberg();
		}
		//LACalibratePtberg *request = new LACalibratePtberg();
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

		
		// calc timegrid
		DoubleArray timeGrid;
		int maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
		LAString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
		LAPriceDataDayCount dayCount;
		dayCount.convertFromString(dayCountStr);
		LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		// create tenor SA
	    //LAString cFreq = LACoreDataService::getContext(ARG_KEY_CANONICALFREQ);
       	//LACoreDataService::setContext(ARG_KEY_CANONICALFREQ, "SA");
		LAMarketDataHW::getCanonicalGrid(timeGrid, asOfDate, dayCount, maxTerm);
	    //LACoreDataService::setContext(ARG_KEY_CANONICALFREQ, cFreq);

		const int size_t = timeGrid.size();
		// param for method
		DoubleArray fTimeGrid;
		DoubleArray sigma;
		DoubleArray beta;

		const int paramSize = paramMtx.size();
		if (paramSize < 3 || 
			paramMtx[0].size() < size_t || paramMtx[1].size() < size_t || paramMtx[2].size() < size_t)
		{
			throw LACoreInvalidData("FX volatility file format is wrong..", __FILE__, __LINE__);
		}
		const double INFINITESIMAL = 1E-7;

////////////////////////////////////////////////////////////////////////////////////// for XLL Plus
		LAString propKey = param->ccy;
		propKey.toLower();

		LAString includeFX = LACoreDataService::getStaticDataManager().getStaticData().getStaticData(propKey+FX_KEY_PTBERG_VOLATILITY_CALIB_FILE_FORWORDGRIDS_ISINCLUDE);
		LADataBool isIncludeFx;
		if (includeFX==MLIB_NO_DATA)
		{
			isIncludeFx.set(false);
		}
		else
		{
			isIncludeFx.convertFromString(includeFX);
		}

		if (isIncludeFx.get()&&paramSize>3)
		{
			DoubleArray fFx;
			for (int i = 0; i < size_t; ++i)
			{
				fTimeGrid.push_back(paramMtx[0][i].trimLeft().trimRight().getDoubleValue());
				beta.push_back(paramMtx[1][i].trimLeft().trimRight().getDoubleValue());
				sigma.push_back(paramMtx[2][i].trimLeft().trimRight().getDoubleValue());
				fFx.push_back(paramMtx[3][i].trimLeft().trimRight().getDoubleValue());
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

			// ceck DDL
			LAMathVolFuncFX *funcFX = 0;
			if (param->isDDL)
			{
				funcFX = new LAMathVolFuncFXDD(timeGrid, sigma, fFx, beta, param->ccy);
			}
			else
			{
				funcFX = new LAMathVolFuncFX(timeGrid, sigma, fFx, beta, param->ccy);
			}

			// Long Jump check
			if (param->isLJ)
			{
				return new LAPriceFXVolatility(funcFX, new LAConstant(1.0), timeGrid);
			}
			else
			{
				return funcFX;
			}
		}

///////////////////////////////////////////////////////////////////////////////////////////////////


		for (int i = 0; i < size_t; ++i)
		{
			fTimeGrid.push_back(paramMtx[0][i].trimLeft().trimRight().getDoubleValue());
			beta.push_back(paramMtx[1][i].trimLeft().trimRight().getDoubleValue());
			sigma.push_back(paramMtx[2][i].trimLeft().trimRight().getDoubleValue());
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
		if (param->isDDL)
		{
			funcFX = new LAMathVolFuncFXDD(timeGrid, sigma, fFx, beta, param->ccy);
		}
		else
		{
			funcFX = new LAMathVolFuncFX(timeGrid, sigma, fFx, beta, param->ccy);
		}
		
		// Long Jump check
		if (param->isLJ)
		{
			return new LAPriceFXVolatility(funcFX, new LAConstant(1.0), timeGrid);
		}
		else
		{
			return funcFX;
		}
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
LACalibrateVolatilityPtberg::createVolatility(double &vol, const LAStringVector &filePath, const MAScenarioParam *param, LAObjectPool *objPool) const
{
	vol, filePath, param, objPool;
	throw LACoreInvalidData("This create method is not support in ptberg.", __FILE__, __LINE__);
}

/*!
    @brief get grid_T

	@param[out] grid_T as double 
	@param[in] filepath
*/
void 
LACalibrateVolatilityPtberg::getGrid_T(DoubleArray &grid_T, const LAStringVector &filePath) const
{
	filePath;
	// Ptberg(FX) has no grid_T
	grid_T.clear();
}

