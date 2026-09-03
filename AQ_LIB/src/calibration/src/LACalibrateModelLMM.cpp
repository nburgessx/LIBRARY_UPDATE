#define _HAS_STD_BYTE 0

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <math.h>
#include "LACalibrateModelLMM.h"
#include "AQLFunctionManager.h"
#include "AQLFunctionBase.h"
#include "AQLDataVector.h"
#include "AQLDataProcedure.h"
#include "AQLAlgorithm.h"
#include "AQLMathVolFuncBase.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathVolatility.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLMathCorrelation.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"
#include "AQLPriceDriftLMMTerminal.h"
#include "AQLPriceDriftLMMSpot.h"
#include "AQLRatesNumeraireBankAccount.h"
#include "AQLRatesNumeraireDiscountBond.h"
#include "AQLModelDynamicsLMMCurve.h"
#include "AQLModelDynamicsDDLMMCurve.h"
#include "AQLPriceDriftQuantAdjustment.h"
#include "AQLRatesEulerMaruyama.h"
#include "AQLRatesPCIntegral.h"
#include "AQLRatesTermStructureSDE.h"
#include "AQLRatesLJTermStructureSDE.h"
#include "LACalibrateModelIR.h"
#include "LADefinitionsLMM.h"
#include "LADefinitionsCalibration.h"
#include "AQLMathCorFuncLMM.h"
#include "AQLMathCorrelationLMM.h"
#include "AQLMathCorrelationLMMDiscAngle.h"
#include "AQLMathCorrelationFuncLMM.h"
#include "AQLMathCorrelationFuncLMMFnB.h"
#include "AQLMathCorrelationFuncLMMFnC.h"
#include "AQLMathLeastSquareCorrelationLMM.h"
#include "LACalibrationParametersLMM.h"
#include "AQLPriceLMMCalibration.h"
#include "AQLMathVolFuncLMM.h"
#include "AQLMathVolFuncWave.h"
#include "AQLMathVolFuncStructureBase.h"
#include "LAMarketData.h"
#include "LAMarketDataLMM.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "AQLRatesCurveLinearInterpolation.h"
#include "AQLRatesCurveLogLinearInterpolation.h"
#include "LACalibrateVolatilityLMM.h"
#include "AQLRatesCurveStepLinearInterpolation.h"
#include "LAScenarioConfiguration.h"

#include "AQLQuantLibOptimizationMethod.h"
#include "AQLQuantLibConjugateGradient.h"
#include "AQLQuantLibSteepestDescent.h"
#include "AQLQuantLibSimplex.h"
#include "AQLQuantLibLevenbergMarquardt.h"
#include "AQLQuantLibBoundaryConstraint.h"
#include "AQLQuantLibProblem.h"
#include "AQLQuantLibArray.h"

using namespace std;

// constructor
/*!
	@param[in] baseCurrency

*/
LACalibrateModelLMM::LACalibrateModelLMM(const AQLString &baseCurrency)
: LACalibrateModelIR(baseCurrency)
{
	setUp();
}

// destructor
/*!

*/
LACalibrateModelLMM::~LACalibrateModelLMM(void)
{
}

/*!
	@brief setUp

	generate tenor deltatenor
*/
void
LACalibrateModelLMM::setUp(void)
{
	int term = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	AQLString freq = FREQ_SEMI_ANNUAL;
	if (LACoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	AQLStringVector exTenor = LAMarketDataLMM::getCanonicalGridExTenor();
	const bool isDataOut = (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA);
	LAMarketDataLMM::getCanonicalGrid(mTenor_30_360, mTenor, mDeltatenor, mExtraTenorFlag, asOfDate, dayCount, freq, term, exTenor, isDataOut);
}


/*!
	@brief return lmm sde type

	@param[in] currency

*/
SDE_TYPE
LACalibrateModelLMM::getSDEType(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	AQLString type = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_LMM_TYPE);
	type.toUpper();
	if (type == "DX/X")
	{
		return DIVIDEdXbyX;
	}
	else if (type == "DX")
	{
		return dX;
	}
	else
	{
		AQLString msg = AQLString("Sde type is not support. type = ") + type;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

/*!
	@brief check is Long Jump

	@param[in] currency

*/
bool 
LACalibrateModelLMM::isLJ(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	AQLString type = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_LMM_INTEGRAL_STEP);
	type.toUpper();
	if (type == "LONGJUMP")
	{
		return true;
	}
	else
	{
		return false;
	}

}

// 
/*!
    @brief create sde instance

	@param[in]  currency
	@param[in]  dataInstance
*/
AQLRatesSDEBase *
LACalibrateModelLMM::createSDEInstance(const AQLString &currency, AQLDataInstance &dataInstance) const
{
	(void)dataInstance;
	SDE_TYPE type = getSDEType(currency);
	AQLRatesTermStructureSDE *psde = 0;
	// check LJ
	if (isLJ(currency))
	{
		psde =  new AQLRatesLJTermStructureSDE(type);
	}
	else
	{
		psde =  new AQLRatesTermStructureSDE(type);
	}
	// set cap ratio
	AQLString key_ccy = currency;
	psde->setCapRatio(mpStaticData->getStaticData(key_ccy.toLower() +
							STATIC_DATA_KEY_LMM_PATH_CAPRATIO).getDoubleValue());

	return psde;

}


/*!
	@brief set volatility to sde

	@param[in] currency
	@param[out] sde
*/
void
LACalibrateModelLMM::setVolatility(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	AQLString key_ccy = currency;
	AQLString sdeName = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);

	const int size = mTenor.size() - 2;
	vector<vector<AQLFunctionBase *> > volMtx(size);
	for (int i = 0; i < size; ++i)
	{
		volMtx[i].resize(1, new AQLMathVolFuncBase(sdeName, i, 0, false));
	}
	sde.setVolatility(volMtx);
}

/*!
	@brief set drift class to sde

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelLMM::setDrift(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	AQLString key_ccy = currency;
	AQLString sdeName = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	const int size = mTenor.size() - 2;
	vector<AQLFunctionBase *> driftVec(size);
	AQLString tmp_baseccy = mBaseCurrency;
	tmp_baseccy.toLower();

	// get skew
	double skew = 1.0;
	if (!isCancelForFunding(currency))
		skew = LAMarketDataLMM::getSkew(currency);

	if (MADealUtils::getSDECurrencys().size() != 1)
	{
		// for cross cccy
		if (key_ccy != tmp_baseccy)
		{
			//avoiding the error in getFXKey
			AQLString bccy = MADealUtils::getSDECurrencys()[0];
			if (bccy.toLower() != tmp_baseccy && bccy != key_ccy)
				tmp_baseccy = bccy;

			// forein drift
			// get fx sde name
			AQLString key_fx = LAMarketData::getFXKey(tmp_baseccy, key_ccy);
			AQLString fx_sdeName = mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_NAME);
			for (int i = 0; i < size; ++i)
			{
				driftVec[i] = new AQLPriceDriftQuantAdjustment(sdeName, fx_sdeName, i, 
												new AQLPriceDriftLMMSpot(sdeName, i, mTenor, mDeltatenor, skew));
			}
		}
		else
		{
			// domestic drift
			for (int i = 0; i < size; ++i)
			{
				driftVec[i] = new AQLPriceDriftLMMSpot(sdeName, i, mTenor, mDeltatenor, skew);
			}
		}
	}
	else
	{
		// check only one ccy
		if (key_ccy != tmp_baseccy)
		{
			throw AQLCoreInvalidData("Sigle currency suport only one currency", __FILE__, __LINE__);
		}
		AQLString driftType = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_DRIFT_SINGLE);
		driftType.toUpper();
		if (driftType == "SPOT")
		{
			// drift spot
			for (int i = 0; i < size; ++i)
			{
				driftVec[i] = new AQLPriceDriftLMMSpot(sdeName, i, mTenor, mDeltatenor, skew);
			}

		}
		else if(driftType == "TERMINAL")
		{
			// drift terminal
			for (int i = 0; i < size; ++i)
			{
				driftVec[i] = new AQLPriceDriftLMMTerminal(sdeName, i, mTenor, mDeltatenor, skew);
			}

		}
		else
		{
			throw AQLCoreInvalidData("Drift suport only spot and terminal", __FILE__, __LINE__);
		}
	}
	sde.setDrift(driftVec);
}


/*!
	@brief set numeraire

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelLMM::setNumeraire(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	if (MADealUtils::getSDECurrencys().size() != 1)
	{
		sde.setNumeraire(new AQLRatesNumeraireBankAccount(mTenor.back(), true));
		//sde.getNumeraire()->isCancelSpread(true);
		//sde.getNumeraire()->isCancelSpread(false);
	}
	else
	{
		AQLString key_ccy = currency;
		AQLString tmp_baseccy = mBaseCurrency;
		// check only one ccy
		if (key_ccy.toLower() != tmp_baseccy.toLower())
		{
			throw AQLCoreInvalidData("Sigle currency suport only one currency", __FILE__, __LINE__);
		}

		AQLString driftType = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_DRIFT_SINGLE);
		driftType.toUpper();
		if (driftType == "SPOT")
		{
			sde.setNumeraire(new AQLRatesNumeraireBankAccount(mTenor.back(), true));
		}
		else if (driftType == "TERMINAL")
		{
			sde.setNumeraire(new AQLRatesNumeraireDiscountBond(mTenor.back(), true));
		}
		else
		{
			throw AQLCoreInvalidData("Drift suport only spot and terminal", __FILE__, __LINE__);
		}
	}
}

/*!
	@brief set path element

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelLMM::setOutputTemplate(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	
	double skew = 1.0;
	double constShift = 0.0;
	if (!isCancelForFunding(currency))
	{
		// get skew and constant shift
		skew = LAMarketDataLMM::getSkew(currency);
		constShift = LAMarketDataLMM::getConstShift(currency);
	}
	if (skew <= 0. || skew >= 2.)
	{
		AQLString msg = currency + " Q = " + AQLString(skew, 2) + " is out of range. Q must be in (0, 2).";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	skew = - AQLMath::log(skew) / AQLMath::log(2.0);
	sde.setOutputTemplate(new AQLRatesPathElementDDLMMCurve(skew, constShift, mTenor, mDeltatenor, 0.0));
}

/*!
	@brief set integral function

	@param[in] currency
	@param[out] sde

*/
void
LACalibrateModelLMM::setIntegralFunction(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	AQLString key_ccy = currency;
	AQLString integralType = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_LMM_INTEGRAL_TYPE);
	integralType.toUpper();
	AQLString isPC = mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_LMM_INTEGRAL_ISPC);
	isPC.toUpper();

	if (isPC == "TRUE")
	{
		if (integralType == "LOG_INTEGRAL")
		{
			sde.setIntegralFunction(new AQLRatesPCIntegral(LOG_INTEGRAL));
		}
		else if (integralType == "NORMAL_INTEGRAL")
		{
			sde.setIntegralFunction(new AQLRatesPCIntegral(NORMAL_INTEGRAL));
		}
		else if (integralType == "LOG_INTEGRAL_LOG_OUTPUT")
		{
			sde.setIntegralFunction(new AQLRatesPCIntegral(LOG_INTEGRAL_LOG_OUTPUT));
		}
		else
		{
			AQLString msg = AQLString("Integraltype is not support integraltype = ") +integralType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
	else
	{
		if (integralType == "LOG_INTEGRAL")
		{
			sde.setIntegralFunction(new AQLRatesEulerMaruyama(LOG_INTEGRAL));
		}
		else if (integralType == "NORMAL_INTEGRAL")
		{
			sde.setIntegralFunction(new AQLRatesEulerMaruyama(NORMAL_INTEGRAL));
		}
		else if (integralType == "LOG_INTEGRAL_LOG_OUTPUT")
		{
			sde.setIntegralFunction(new AQLRatesEulerMaruyama(LOG_INTEGRAL_LOG_OUTPUT));
		}
		else
		{
			AQLString msg = AQLString("Integraltype is not support integraltype = ") +integralType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
}

/*!
	@brief get function master regist name

	@param[in] currency
*/
AQLString 
LACalibrateModelLMM::getFunctionMasterResistName(const AQLString &currency) const
{
	AQLString tmpCurrency = currency;
	return mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_LMM_FUNCTION_NAME);
}


// 
/*!
    @brief get correlation input type

	@param[in] currency 
*/
AQLString
LACalibrateModelLMM::getCorTye(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	return mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_LMM_CORRELATION_TYPE);
}


// 
/*!
    @brief set correlation factor loading

	@param[in] currency
	@param[out] cor
	@param[out] dataInstance
*/
void
LACalibrateModelLMM::setUpCorFactor(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const
{
	(void)dataInstance;
	setUpCorEntity(currency,cor);
	AQLString key_ccy = currency;
	key_ccy.toLower();
	
	AQLString factorNumKey = STATIC_DATA_KEY_LMM_CORRELATION_CROSS_FACTOR_NUM;
	if (MADealUtils::getSDECurrencys().size() == 1)
	{
		factorNumKey = STATIC_DATA_KEY_LMM_CORRELATION_SIGLE_FACTOR_NUM;
	}

	AQLString factorNumStr = mpStaticData->getStaticData(key_ccy + factorNumKey);
	const unsigned int factorNum = factorNumStr.getIntValue();

	if (isCancelForFunding(currency))
		return;

	AQLString filePath = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_CORRELATION_FACTOR_FILE);
	MAFileAccessor file(LAMarketData::getNumFileName(filePath));
	AQLStringMatrix loadingData;
	file.readAllData(MARKET_DATA_DELIMITER, loadingData);
	file.close();

	// check
	if (factorNum != static_cast<unsigned int>(loadingData.size()))
	{
		AQLString msg = AQLString("Correlation factor file format is wrong factorNum is ") + factorNumStr;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// convert
	DoubleMatrix fullLoading(loadingData[0].size());
	for (unsigned int i = 0; i < loadingData[0].size(); ++i)
	{
		fullLoading[i].resize(factorNum);
		for (unsigned int j = 0; j < factorNum; ++j)
		{
			fullLoading[i][j] = loadingData[j][i].trimLeft().trimRight().getDoubleValue();
		}
	}

	// check extra tenor (DB data is always contains extra data)
	unsigned int canonicSize = cor.getTGrid().get().size();
	DoubleMatrix loading(canonicSize);

	AQLString isExTUseStr = mpStaticData->getStaticData(KEY_LMM_CANONICALGRID_ISEXTRATENORUSE);
	if (isExTUseStr == AQ_NO_DATA)
	{
		isExTUseStr = "FALSE";
	}
	AQLDataBool isExTUse;
	isExTUse.convertFromString(isExTUseStr);	
	unsigned int truncateSize = 0;
	if (!isExTUse.get())
	{
		AQLString exTenorStr = mpStaticData->getStaticData(KEY_LMM_CANONICALGRID_EXTRATENOR);
		if (exTenorStr != AQ_NO_DATA)
		{
			truncateSize = exTenorStr.toToken(MULTI_STATIC_DATA_DELIMITER).size();
		}
	}

	// copy
	if (canonicSize + truncateSize > static_cast<unsigned int>(fullLoading.size()))
	{
		AQLString msg = AQLString("Correlation factor file format is wrong: Grid size must be greater than ") + AQLString(static_cast<int>(canonicSize + truncateSize));
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	for(unsigned int i = 0; i < canonicSize; i++)
	{
		loading[i] = DoubleVector(fullLoading[i + truncateSize].begin(), fullLoading[i + truncateSize].end());
	}
	cor.setFactorLoading(loading);

}

// 
/*!
    @brief set correlation data

	@param[in] currency
	@param[out] cor
	@param[out] dataInstance
*/
void
LACalibrateModelLMM::setUpCorData(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const
{
	(void)dataInstance;
	setUpCorEntity(currency, cor);
	AQLString key_ccy = currency;
	key_ccy.toLower();
	AQLString filePath = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_CORRELATION_DATA_FILE);
	
	MAFileAccessor file(LAMarketData::getNumFileName(filePath));
	AQLStringMatrix corData;
	file.readAllData(MARKET_DATA_DELIMITER, corData);
	file.close();

	int canonicSize = mTenor.size() - 2;
	DoubleArray line(canonicSize);
	DoubleMatrix data(canonicSize, line);

	for (int i = 0; i < canonicSize; ++i)
	{
		for (int j = i; j < canonicSize; ++j)
		{
			data[i][j] = corData[i][j].trimLeft().trimRight().getDoubleValue();
			data[j][i] = data[i][j];
		}
	}
	cor.setCorrelation(data);
	cor.calcFactorLoading();
}



// 
/*!
    @brief set correlation data

	@param[in] currency
	@param[out] cor
	@param[out] dataInstance
*/
void
LACalibrateModelLMM::setUpCorFunc(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const
{
	(void)dataInstance;
	setUpCorEntity(currency, cor);
	AQLString key_ccy = currency;
	key_ccy.toLower();

	AQLFunctionBase *corFunc = NULL;

	MAScenarioParam param;
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	param.isCalib = isCalibTarget(currency);
	param.calcType = KEY_PV;
	if (param.isCalib)
	{
		LACalibrationParametersLMM cInfo;
		MACalibCorrelationParam calibCorParam;
		cInfo.getCalibCorParam(currency, calibCorParam);

		// mExoCalibLMMSetCorFunc
		if (calibCorParam.corFuncType == CALIB_COR_FUNC_TYPE_B)
		{
			corFunc = new AQLMathCorrelationFuncLMMFnB(calibCorParam.maxTerm, calibCorParam.funcParam_x, calibCorParam.funcParam_y);
		}
		else if (calibCorParam.corFuncType == CALIB_COR_FUNC_TYPE_C)
		{
			corFunc = new AQLMathCorrelationFuncLMMFnC(calibCorParam.funcParam_x, calibCorParam.funcParam_y);
		}
		else
		{
			AQLString msg = AQLString("not support cor method type = ") + calibCorParam.corFuncType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// mExoCalibLMMSetOptCorr
		AQLString isExTUseStr = mpStaticData->getStaticData(KEY_LMM_CANONICALGRID_ISEXTRATENORUSE);
		if (isExTUseStr == AQ_NO_DATA)
		{
			isExTUseStr = "FALSE";
		}
		AQLDataBool isExTUse;
		isExTUse.convertFromString(isExTUseStr);	
		unsigned int extraSize = 0;
		if (isExTUse.get())
		{
			AQLString exTenorStr = mpStaticData->getStaticData(KEY_LMM_CANONICALGRID_EXTRATENOR);
			if (exTenorStr != AQ_NO_DATA)
			{
				extraSize = exTenorStr.toToken(MULTI_STATIC_DATA_DELIMITER).size();
			}
		}

		const DoubleVector& tenorForCalib = cor.getTGrid().get();
		const BoolVector& extraTenorForCalib = cor.getIsExtraTGrid().get();

		DoubleVector T_fix(1, 0.);
		if (calibCorParam.isExtraTenorCalib)
		{
			std::copy(tenorForCalib.begin(), tenorForCalib.end(), std::back_inserter(T_fix));
		}
		else
		{
			for(unsigned int i = 0; i < tenorForCalib.size(); i++)
			{
				if (!extraTenorForCalib[i])
				{
					T_fix.push_back(tenorForCalib[i]);
				}
			}
		}

		unsigned int factorNum = calibCorParam.factorNum;

		AQLMathCorrelationLMMDiscAngle *optCor = NULL;
		if (calibCorParam.factorNum < 2)
		{
			const DoubleMatrix corr_unit(T_fix.size(), DoubleVector(T_fix.size(), 1.0));
			optCor = new AQLMathCorrelationLMMDiscAngle(corr_unit, T_fix, 1, true);
		}
		else if (calibCorParam.optCorType == CALIB_OPT_TARGET_TYPE_PCA)
		{
			optCor = new AQLMathCorrelationLMMDiscAngle(dynamic_cast<AQLMathCorrelationFuncLMM*>(corFunc), T_fix, calibCorParam.factorNum);
		}
		else if (calibCorParam.optCorType == CALIB_OPT_TARGET_TYPE_REDUCERANK_ANGLE_FORMULA)
		{
			optCor = new AQLMathCorrelationLMMDiscAngle(dynamic_cast<AQLMathCorrelationFuncLMM*>(corFunc), T_fix, calibCorParam.factorNum);
		}
		else
		{
			AQLString msg = AQLString("not support: opt cor type = ") + calibCorParam.optCorType;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// mExoCalibLMMInitOptCorMethod
		AQLQuantLibOptimizationMethod *optCorMethod = NULL;
		AQLQuantLibCostFunction *costFunc = NULL;
		AQLQuantLibConstraint *constraint = NULL;
		AQLQuantLibEndCriteria *endCriteria = NULL;
		AQLQuantLibProblem *problem = NULL;
		if (optCor->getNoFactors() < 2)
		{
			// one factor case
			// dummy?
			optCorMethod = new AQLQuantLibConjugateGradient();
		}
		else
		{
			// set up CostFunction
			costFunc = new AQLMathLeastSquareCorrelationLMM(*dynamic_cast<AQLMathCorrelationLMMDiscAngle*>(optCor));

			// Set constraint for optimizers: unconstrained problem
			constraint = new AQLQuantLibBoundaryConstraint(calibCorParam.boundaryMin, calibCorParam.boundaryMax);

			// Set initial guess for optimizer
			DoubleMatrix theta_ = optCor->getTheta();
			size_t n = theta_.size();
			size_t no_of_factor = optCor->getNoFactors();

			AQLQuantLibArray x(n * (no_of_factor - 1));
			size_t k = 0;
			for(size_t i = 0; i < n; i++)
			{
				for(size_t j = 0; j < no_of_factor - 1; j++)
				{
					x[k++] = theta_[i][j];
				}
			}

			// Set end criteria for optimizer
			endCriteria = new AQLQuantLibEndCriteria(calibCorParam.maxIteration, calibCorParam.maxStationaryStateIteration,
					calibCorParam.rootEpsilon, calibCorParam.functionEpsilon, calibCorParam.gradientNormEpsilon);

			// Set Problem
			problem = new AQLQuantLibProblem(*costFunc, *constraint, x);

			// Set Optimization Method
			if( calibCorParam.optCorMethodType == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD )
			{
				optCorMethod = new AQLQuantLibConjugateGradient();
			}
			else if( calibCorParam.optCorMethodType == CALIB_STEEPEST_DESCENT_METHOD )
			{
				optCorMethod = new AQLQuantLibSteepestDescent();
			}
			else if( calibCorParam.optCorMethodType == CALIB_SIMPLEX_METHOD )
			{
				optCorMethod = new AQLQuantLibSimplex(0.0001);
			}
			else if( calibCorParam.optCorMethodType == CALIB_LEVENBERG_MARQUARDT_METHOD )
			{
				double levenbergMarquardtEpsfcn = 1.0e-8;     // parameters specific for Levenberg-Marquardt
				double levenbergMarquardtXtol   = 1.0e-8;     //
				double levenbergMarquardtGtol   = 1.0e-8;     //

				optCorMethod = new AQLQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol);
			}
		}

		// mExoCalibLMMOptimizeCorr
		size_t n_tgrid = optCor->getTgrid().size();
		if (optCor->getNoFactors() < 2)
		{
			// one factor case

			DoubleMatrix factorLoadingOptimized(n_tgrid - 1, DoubleVector(1, 1.0));
			DoubleMatrix corrOptimized(n_tgrid - 1, DoubleVector(n_tgrid - 1, 0.0));
			for(size_t i = 0; i < n_tgrid - 1; ++i)
			{
				for(size_t j = 0; j < n_tgrid - 1; ++j)
				{
					corrOptimized[i][j] = 1.0;
				}
			}

			cor.setFactorLoading(factorLoadingOptimized);
		}
		else
		{
			AQLQuantLibEndCriteria::Type endCriteriaResult = optCorMethod->minimize(*problem, *endCriteria);

			AQLQuantLibArray xMinCalculated = problem->currentValue();

			DoubleMatrix factorLoadingOptimized( n_tgrid, DoubleVector(factorNum, 0.0) );
			DoubleMatrix theta_(n_tgrid, DoubleVector(factorNum - 1, 0.0) );

			// Get Angles
			size_t k = 0;
			for(size_t i = 0; i < n_tgrid; ++i)
			{
				for(size_t j = 0; j < factorNum - 1; ++j)
				{
					theta_[i][j] = xMinCalculated[k++];
				}
			}

			optCor->setTheta(theta_);
			factorLoadingOptimized = optCor->getFactorLoading();

			size_t canonic_size = tenorForCalib.size();

			DoubleMatrix factorLoadingOptimized_( canonic_size, DoubleVector(factorNum, 0.0) );
			if (calibCorParam.isExtraTenorCalib)
			{
				for(size_t i = 0; i < canonic_size; i++)
				{
					for(size_t j = 0; j < factorNum; j++)
					{
						factorLoadingOptimized_[i][j] = factorLoadingOptimized[i][j];
					}
				}
			}
			else
			{
				for(size_t i = 0; i < canonic_size; i++)
				{
					if (i < extraSize) 
					{
						for(size_t j = 0; j < factorNum; j++)
						{
							factorLoadingOptimized_[i][j] = factorLoadingOptimized[1][j];
						}
					}
					else
					{
						for(size_t j = 0; j < factorNum; j++)
						{
							factorLoadingOptimized_[i][j] = factorLoadingOptimized[i+1-extraSize][j];
						}
					}
				}
			}

			if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
			{
				const AQLString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM);
				const AQLString dirName = LACoreDataService::getOutputDirectory(); 
				const AQLString fileName  = dirName + "LMM_CORRELATON_DATA" + fileSuffix + ".csv";

				ifstream fin;
				ofstream fout;
				fin.open(fileName.getCString());

				if (!fin)
				{
					fout.open(fileName.getCString());

					for (unsigned int i = 0; i < factorLoadingOptimized_.size(); ++i)
					{
						AQLString line = "";
						for (unsigned int j = 0; j < factorLoadingOptimized_[0].size(); ++j)
						{
							line += AQLString(factorLoadingOptimized_[i][j]) +  ",";							
						}
						line = line.subString(0, line.size() - 2);

						fout << line.getCString() << std::endl;
					}
					
					fout.close();
					fin.close();
				}
				else
				{
					factorLoadingOptimized_.clear();
					string line;
					DoubleVector line_d;
					while (getline(fin, line))
					{
						const char *c_line = line.c_str();
						AQLStringVector lineVec = AQLString(c_line).toToken(MARKET_DATA_DELIMITER);
						line_d.clear();
						for (unsigned int i = 0; i < lineVec.size(); ++i)
						{
							line_d.push_back(lineVec[i].getDoubleValue());
						}

						factorLoadingOptimized_.push_back(line_d);
					}
					fin.close();
				}
			}

			cor.setFactorLoading(factorLoadingOptimized_);
		}

		const AQLString corName = PREFIX_COR + getSDEAttrName(currency);
		// set function master
		dataInstance.getFunctionMaster().removeFunction(corName + AQLString("_0_0"));
		dataInstance.getFunctionMaster().setFunction(corFunc->clone(), corName + AQLString("_0_0"));

		//AQLString tmp_currency = currency;
		//tmp_currency.toUpper();
		//objPool.remove(tmp_currency + "_" + CALIBRATION_DATA_LMM_CALIB_CORRELATION);
		//objPool.set(tmp_currency + "_" + CALIBRATION_DATA_LMM_CALIB_CORRELATION, optCor);

		delete corFunc;
		delete endCriteria;
		delete costFunc;
		delete constraint;
		delete problem;
		delete optCor;
		delete optCorMethod;
	}
	else
	{
		AQLString paramFilePath = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_CORRELATION_FUNC_FILE);
		AQLString maxFilePath = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_MAXTERM_FILE);

		// read method param
		MAFileAccessor paramFile(LAMarketData::getNumFileName(paramFilePath));
		AQLStringMatrix funcParamMtx;
		paramFile.readAllData(MARKET_DATA_DELIMITER, funcParamMtx);
		paramFile.close();

		// read max param
		MAFileAccessor maxFile(LAMarketData::getNumFileName(maxFilePath));
		AQLStringMatrix maxMtx;
		maxFile.readAllData(MARKET_DATA_DELIMITER, maxMtx);
		maxFile.close();

		if (funcParamMtx.size() != 2)
		{
			throw AQLCoreInvalidData("Correlation function param format is wrong .", __FILE__, __LINE__);
		}

		double x = funcParamMtx[0][0].trimLeft().trimRight().getDoubleValue();
		double y = funcParamMtx[1][0].trimLeft().trimRight().getDoubleValue();
		double max = maxMtx[0][0].getDoubleValue();

		corFunc = new AQLMathCorFuncLMM(max, x, y);
		cor.setCorrelation(corFunc);
		cor.calcFactorLoading();

		const AQLString corName = PREFIX_COR + getSDEAttrName(currency);
		// set function master
		dataInstance.getFunctionMaster().removeFunction(corName + AQLString("_0_0"));
		dataInstance.getFunctionMaster().setFunction(corFunc->clone(), corName + AQLString("_0_0"));
	}
}

// 
/*!
    @brief get volatility input type

	@param[in]  currency 
*/
AQLString
LACalibrateModelLMM::getVolType(const AQLString &currency) const
{
	AQLString key_ccy = currency;
	return mpStaticData->getStaticData(key_ccy.toLower() + STATIC_DATA_KEY_LMM_VOLATILITY_TYPE);
}

// 
/*!
    @brief set volatility function

	@param[in] currency
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelLMM::setUpVolFunc(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	setUpVolEntity(currency, vol);
	AQLString key_ccy = currency;
	key_ccy.toLower();

	if (isCancelForFunding(currency))
		return;

	AQLStringVector fileVec(4);
	fileVec[0] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_CALIB_FILE));
	fileVec[1] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_FILE));
	fileVec[2] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_MAXTERM_FILE));

	if (MADealUtils::getSDECurrencys().size() == 1)
	{
		fileVec[3] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_SKEW_SINGLE_FILE));
	}
	else
	{
		fileVec[3] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_SKEW_CROSS_FILE));
	}

	
	LACalibrateVolatilityLMM volCreator;
	// create method vec
	vector<AQLFunctionBase *> funcVec;

	MAScenarioParam param;
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	param.isCalib = isCalibTarget(currency);
	param.calcType = KEY_PV;
	param.bumpType = RISK_MODEL_VOL_BUMP;

	if (param.isCalib)
	{
		// create calib info
		LACalibrationParametersLMM cInfo;
		AQLString cInfoName = cInfo.createCalibrationInfo(objPool, currency);
        // first element set calib info
		param.refName.push_back(cInfoName);

		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else
		{
			param.isOutPut = false;
		}
		
		param.refName.push_back(LAMarketData::getBaseYieldName(currency));
		volCreator.createVolatility(funcVec, fileVec, &param, &objPool);
	}
	else
	{
		AQLString AdjParamInter = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_INTERPOLATION);
		if (AdjParamInter == AQ_NO_DATA)
		{
				AdjParamInter = "fn_stepinterpolation";
		}
		volCreator.setAdjParamInterpolation(AdjParamInter);
		AQLString AdjParamFreq = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_FREQUENCY);
		if (AdjParamFreq != "QA" && AdjParamFreq != "SA")
		{
			AQLString msg = "Frequency of LMM function G input must be ""SA"" or ""QA"", However input is " + AdjParamFreq + ".";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		volCreator.setAdjParamFrequency(AdjParamFreq);

	    volCreator.createVolatility(funcVec, fileVec, &param, &objPool);
		const AQLString &volName = vol.getName().get();
		const int size = funcVec.size();
		for (int i = 0; i < size; ++i)
		{
			dataInstance.getFunctionMaster().setFunction(funcVec[i]->clone(), volName + AQLString("_") + AQLString(i) + AQLString("_0"));
		}
	}

	vol.setVolatility(funcVec);

//	if (param.isCalib)
//	{
//		AQLString interType = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_PATH_INTERPOLATION);
//		interType.toUpper();
//
//		if (interType == "LOGLINEAR")
//		{
//			// set volMat to sde.interpolationMethod
//			// create vol data
//			DoubleMatrix volMat;
//			DoubleArray grid_t = mTenor;
//
//			unsigned int marketSize = 0;
//			if (grid_t[0] == 0.0)
//			{
//				marketSize = grid_t.size() - 2;
//			}
//			else
//			{
//				marketSize = grid_t.size() - 1;
//			}
////			grid_t.erase(--grid_t.end());
//			grid_t.pop_back();
//			const unsigned int tSize = grid_t.size();
//				
//			volMat.resize(tSize);
//			// set volatility val
//			for (unsigned int i = 0; i < tSize; ++i)
//			{
//				volMat[i].resize(marketSize, 0.0);
//				for (unsigned int j = 0; j < marketSize; ++j)
//				{
//					volMat[i][j] = funcVec[j]->operator ()(grid_t[i]);// reverse T,t
//				}
//			}
//
//			AQLString sdeName = getFunctionMasterResistName(currency);
//			const AQLRatesSDEBase &sde = dynamic_cast<const AQLRatesSDEBase &>(dataInstance.getFunctionMaster().getFunction(sdeName).get());
//
//			AQLRatesCurveLogLinearInterpolation *pInter = dynamic_cast<AQLRatesCurveLogLinearInterpolation *>(sde.getInterpolationMethod());
//			pInter->setVolForInterpolation(volMat);
//		}
//	}
}

// 
/*!
    @brief set setInterpolationMethod

	@param[in] currency
	@param[out] sde
*/
void
LACalibrateModelLMM::setInterpolationMethod(const AQLString &currency, AQLRatesSDEBase &sde) const
{
	AQLString key_ccy = currency;
	key_ccy.toLower();
	AQLString interType = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_PATH_INTERPOLATION);
	interType.toUpper();
	
	if(isCancelForFunding(currency))
		return;
	
	if (interType == "LOGLINEAR")
	{
////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// >>>>>>>>>>>>> for XLL plus
/*
		AQLStringVector fileVec(4);
		fileVec[0] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_CALIB_FILE));
		fileVec[1] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_FILE));
		fileVec[2]= LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_MAXTERM_FILE));
		if (MADealUtils::getSDECurrencys().size() == 1)
		{
			fileVec[3] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_SKEW_SINGLE_FILE));
		}
		else
		{
			fileVec[3] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_SKEW_CROSS_FILE));
		}
		LACalibrateVolatilityLMM volCreator;
	
		// create vol data
		DoubleArray grid_t;
		DoubleMatrix tmp;
		volCreator.createVolatility(grid_t, tmp, fileVec);

		// reverse T,t
		unsigned int size_t = grid_t.size();
		unsigned int size_m = tmp.size();
		DoubleMatrix volMat(size_t);
		for (unsigned int i = 0; i < size_t; ++i)
		{
			volMat[i].resize(size_m);
			for (unsigned int j = 0; j < size_m; ++j)
			{
				volMat[i][j] = tmp[j][i];
			}
		}
*/

//		AQLString key_ccy = currency;
//		key_ccy.toLower();
//
//		bool isCalibTarget = LACalibrateModel::isCalibTarget(currency); 
//
//		if (! isCalibTarget)
//		{
//			AQLString volType = getVolType(currency);
//			volType.toUpper();
//
//			//DoubleMatrix volMat;
//			if (volType == INPUT_FUNC)
//			{
//				AQLStringVector fileVec(4);
//				fileVec[0] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_CALIB_FILE));
//				fileVec[1] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_FILE));
//				fileVec[2] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_MAXTERM_FILE));
//				if (MADealUtils::getSDECurrencys().size() == 1)
//				{
//					fileVec[3] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_SKEW_SINGLE_FILE));
//				}
//				else
//				{
//					fileVec[3] = LAMarketData::getNumFileName(mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_SKEW_CROSS_FILE));
//				}
//				LACalibrateVolatilityLMM volCreator;
//				// create vol data
//				DoubleArray grid_t;
//				DoubleMatrix tmp;
//				volCreator.createVolatility(grid_t, tmp, fileVec);
//
//				// reverse T,t
//				unsigned int size_t = grid_t.size();
//				unsigned int size_m = tmp.size();
//				volMat.resize(size_t);
//				for (unsigned int i = 0; i < size_t; ++i)
//				{
//					volMat[i].resize(size_m);
//					for (unsigned int j = 0; j < size_m; ++j)
//					{
//						volMat[i][j] = tmp[j][i];
//					}
//				}
//			}
//			else if (volType == INPUT_DATA)
//			{
//				AQLString filePath = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_DATA_FILE);
//				MAFileAccessor file(LAMarketData::getNumFileName(filePath));
//
//				AQLStringMatrix volData;
//				file.readAllData(MARKET_DATA_DELIMITER, volData);
//				file.close();
//
//				int canonicSize = mTenor.size() - 2;
//				volMat.resize(canonicSize + 1);
//
//				for (int i = 0; i < canonicSize + 1; ++i)
//				{
//					if (i>=volData.size()) break;
//					volMat[i].resize(canonicSize);
//					for (int j = 0; j < canonicSize; ++j)
//					{
//						if (j>=volData[i].size()) break;
//						volMat[i][j] = volData[i][j].getDoubleValue();
//					}
//				}
//			}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//			AQLRatesCurveLogLinearInterpolation *pInter = createCurveLogLinearInterpolation();
//			//pInter->setVolForInterpolation(volMat);
//			sde.setInterpolationMethod(pInter);
//		}
//		else
//		{
//			AQLRatesCurveLogLinearInterpolation *pInter = createCurveLogLinearInterpolation();
//			//pInter->setVolForInterpolation(volMat);
//			sde.setInterpolationMethod(pInter);
//		}
		sde.setInterpolationMethod(createCurveLogLinearInterpolation());
	}
	else if (interType == "LINEAR")
	{
		sde.setInterpolationMethod(new AQLRatesCurveLinearInterpolation());
	}
	else if (interType == "STEP")
	{
		sde.setInterpolationMethod(new AQLRatesCurveStepLinearInterpolation());
	}
	else
	{
		throw AQLCoreInvalidData("InterpolationType is wrong. Only loglinear, linear, step support.",__FILE__,__LINE__);
	}
}

// 
/*!
    @brief curve create log linear interpolation

	@return AQLRatesCurveLogLinearInterpolation *
*/
AQLRatesCurveLogLinearInterpolation *
LACalibrateModelLMM::createCurveLogLinearInterpolation() const
{
	return new AQLRatesCurveLogLinearInterpolation();
}

// 
/*!
    @brief set volatility data

	@param[in] currency
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelLMM::setUpVolData(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	(void)dataInstance;
	setUpVolEntity(currency, vol);
	AQLString key_ccy = currency;
	key_ccy.toLower();
	AQLString filePath = mpStaticData->getStaticData(key_ccy + STATIC_DATA_KEY_LMM_VOLATILITY_DATA_FILE);
	
	MAFileAccessor file(LAMarketData::getNumFileName(filePath));
	AQLStringMatrix volData;
	file.readAllData(MARKET_DATA_DELIMITER, volData);
	file.close();

	int canonicSize = mTenor.size() - 2;
	DoubleMatrix data(canonicSize);

	DoubleArray grid_t = mTenor;
////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////// for XLL Plus
/*
	if (grid_t[0] == 0)
	{
		grid_t.erase(grid_t.begin());
	}

	for (int i = 0; i < canonicSize; ++i)
	{
		for (int j = 0; j < canonicSize; ++j)
		{
			if (volData[j][i].trimLeft().trimRight() == "")
			{
				// no data break
				break;
			}
			data[i].push_back(volData[j][i].getDoubleValue());
		
		}
	}
*/

	for (int i = 0; i < canonicSize; ++i)
	{
		if (i>=volData[0].size()) break;
		for (int j = 0; j < canonicSize + 1; ++j)
		{
			if (j>=volData.size()) break;
			data[i].push_back(volData[j][i].getDoubleValue());
		
		}
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vol.setVolatility(grid_t, data);
}


// 
/*!
    @brief setup correlation object

	@param[in] currency
	@param[out] cor

*/
void
LACalibrateModelLMM::setUpCorEntity(const AQLString &currency, AQLMathCorrelation &cor) const
{
	// set grid_T
	DoubleArray grid_T;
	BoolVector  extraTenorFlag;
	if (isCalibTarget(currency))
	{
		DoubleArray tenor_30_360_forCalib;
		DoubleArray deltatenor_forCalib;
		LACalibrateModelLMM::setUpTenorForCalib(currency, tenor_30_360_forCalib, grid_T, deltatenor_forCalib, extraTenorFlag);
	}
	else
	{
		grid_T = mTenor;
		extraTenorFlag = mExtraTenorFlag;
	}
	grid_T.pop_back();
	extraTenorFlag.pop_back();
	if (grid_T[0] == 0)
	{
		grid_T.erase(grid_T.begin());
		extraTenorFlag.erase(extraTenorFlag.begin());
	}
	cor.getTGrid().set(grid_T);
	cor.getIsExtraTGrid().set(extraTenorFlag);

	// set interpolation
	AQLString tmpCurrency = currency;
	AQLString interp = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_LMM_CORRELATION_INTERPOLATION);
	cor.getInterpolation().convertFromString(interp);

	// set mulitvol false
	cor.getIsMultiVol().set(false);

	// set optimization
	AQLString optimization = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_LMM_CORRELATION_ISOPTIMIZATION);
	cor.getData(IR_CALIBRATION_DATA_ISOPTIM, ISDEFINED).convertFromString(optimization);

	// set factor num before
	AQLString TSizeStr(static_cast<const int>(grid_T.size()));
	cor.getData(IR_CALIBRATION_DATA_FACTORNUM_BEFORE, NOCHECK).convertFromString(TSizeStr);

	// set factor num after
	AQLString factorNumKey = STATIC_DATA_KEY_LMM_CORRELATION_CROSS_FACTOR_NUM;
	if (MADealUtils::getSDECurrencys().size() == 1)
	{
		factorNumKey = STATIC_DATA_KEY_LMM_CORRELATION_SIGLE_FACTOR_NUM;
	}
	
	AQLString factorNumStr = mpStaticData->getStaticData(tmpCurrency.toLower() + factorNumKey);
	cor.getData(IR_CALIBRATION_DATA_FACTORNUM_AFTER, NOCHECK).convertFromString(factorNumStr);


}

// 
/*!
    @brief setup volatility object

	@param[in] currency
	@param[out] vol
*/
void
LACalibrateModelLMM::setUpVolEntity(const AQLString &currency, AQLMathVolatility &vol) const
{
	// set interpolation
	AQLString tmpCurrency = currency;
	AQLString interp = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_LMM_VOLATILITY_INTERPOLATION);
	vol.getInterpolation().convertFromString(interp);
}

// 
/*!
    @brief setup tenor

	@param[in] currency
*/
void
LACalibrateModelLMM::setUpTenorForCalib(const AQLString &currency, DoubleArray &tenor_30_360, DoubleArray &tenor, DoubleArray &deltatenor, BoolVector &extraTenorFlag) const
{
	int maxTerm = (int)LAMarketDataLMM::getMaxTerm(currency);
	BoolVector tenor_extraflag;

	AQLString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create tenor 
	AQLString freq = FREQ_SEMI_ANNUAL;
	if (LACoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	AQLStringVector exTenor = LAMarketDataLMM::getCanonicalGridExTenor();
 	LAMarketDataLMM::getCanonicalGrid(tenor_30_360, tenor, deltatenor, extraTenorFlag, asOfDate, dayCount, freq, maxTerm, exTenor);
}

