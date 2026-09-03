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
#include "AQLDate.h"
#include "AQLPriceDataDayCount.h"
#include "AQLMathFXUtility.h"
#include "AQLMathVolFuncFX.h"
#include "AQLMathVolFuncFXDD.h"
#include "AQLPriceFXVolatility.h"
#include "AQLConstant.h"
#include "AQLBasic.h"
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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_t, vector<DoubleMatrix> &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
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
LACalibrateVolatilityPtberg::createVolatility(vector<vector<AQLFunctionBase *> > &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_T, vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_t, DoubleMatrix &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
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
LACalibrateVolatilityPtberg::createVolatility(vector<AQLFunctionBase *> &vol, const AQLStringVector &filePath, 
											const MAScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
{
	(void)vol, (void)filePath, (void)param, (void)objPool;
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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_T, AQLFunctionBase *vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_T, (void)vol, (void)filePath, (void)param, (void)objPool;
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
LACalibrateVolatilityPtberg::createVolatility(DoubleArray &grid_t, DoubleArray &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
{
	(void)grid_t, (void)vol, (void)filePath, (void)param, (void)objPool;
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
LACalibrateVolatilityPtberg::createVolatility(const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool, int gridPos) const
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

		return method;
	}
	else
	{
		MAFileAccessor file(filePath[0]);
		AQLStringMatrix paramMtx;
		file.readAllData(MARKET_DATA_DELIMITER, paramMtx);
		file.close();

		
		// calc timegrid
		DoubleArray timeGrid;
		int maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
		AQLString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
		AQLPriceDataDayCount dayCount;
		dayCount.convertFromString(dayCountStr);
		AQLDate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		// create tenor SA
	    //AQLString cFreq = LACoreDataService::getContext(ARG_KEY_CANONICALFREQ);
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
			throw AQLCoreInvalidData("FX volatility file format is wrong..", __FILE__, __LINE__);
		}
		const double INFINITESIMAL = 1E-7;

////////////////////////////////////////////////////////////////////////////////////// for XLL Plus
		AQLString propKey = param->ccy;
		propKey.toLower();

		AQLString includeFX = LACoreDataService::getStaticDataManager().getStaticData().getStaticData(propKey+FX_KEY_PTBERG_VOLATILITY_CALIB_FILE_FORWORDGRIDS_ISINCLUDE);
		AQLDataBool isIncludeFx;
		if (includeFX==AQ_NO_DATA)
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
				if (AQLMath::abs(timeGrid[i] - fTimeGrid[i]) > INFINITESIMAL)
				{
					AQLString msg = "CanonicTimeGrid = " + AQLString(timeGrid[i]) + ", FileTimeGrid = " + AQLString(fTimeGrid[i]);
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
			}

			// ceck DDL
			AQLMathVolFuncFX *funcFX = 0;
			if (param->isDDL)
			{
				funcFX = new AQLMathVolFuncFXDD(timeGrid, sigma, fFx, beta, param->ccy);
			}
			else
			{
				funcFX = new AQLMathVolFuncFX(timeGrid, sigma, fFx, beta, param->ccy);
			}

			// Long Jump check
			if (param->isLJ)
			{
				return new AQLPriceFXVolatility(funcFX, new AQLConstant(1.0), timeGrid);
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
			if (AQLMath::abs(timeGrid[i] - fTimeGrid[i]) > INFINITESIMAL)
			{
				AQLString msg = "CanonicTimeGrid = " + AQLString(timeGrid[i]) + ", FileTimeGrid = " + AQLString(fTimeGrid[i]);
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}

		const AQLMathYieldCurve &dYield = dynamic_cast<const AQLMathYieldCurve &>(objPool->getObject(param->refName[0], ENCHKTYPE_ISDEFINED).get());
		const AQLMathYieldCurve &fYield = dynamic_cast<const AQLMathYieldCurve &>(objPool->getObject(param->refName[1], ENCHKTYPE_ISDEFINED).get());

		AQLMathFXEntity fx_tmp = *LAMarketData::getFXEntity(*objPool, "USEMODEL");
		fx_tmp.getFXType() = "FIXEDRATE";

		AQLStringVector ccys;
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
		AQLDate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
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
		if (param->isDDL)
		{
			funcFX = new AQLMathVolFuncFXDD(timeGrid, sigma, fFx, beta, param->ccy);
		}
		else
		{
			funcFX = new AQLMathVolFuncFX(timeGrid, sigma, fFx, beta, param->ccy);
		}
		
		// Long Jump check
		if (param->isLJ)
		{
			return new AQLPriceFXVolatility(funcFX, new AQLConstant(1.0), timeGrid);
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
LACalibrateVolatilityPtberg::createVolatility(double &vol, const AQLStringVector &filePath, const MAScenarioParam *param, AQLObjectPool *objPool) const
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
LACalibrateVolatilityPtberg::getGrid_T(DoubleArray &grid_T, const AQLStringVector &filePath) const
{
	filePath;
	// Ptberg(FX) has no grid_T
	grid_T.clear();
}

