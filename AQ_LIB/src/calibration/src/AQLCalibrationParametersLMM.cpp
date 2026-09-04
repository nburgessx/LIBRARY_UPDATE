/*! @file
    @brief Calibration method create class for lmm
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLString.h"
#include "AQLObjectPool.h"
#include "AQLObject.h"
#include "AQLPriceDataType.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLDataReference.h"
#include "AQLMathDefine.h"
#include "AQLDealUtils.h"
#include "AQLCalibrationParametersLMM.h"
#include "AQLStaticData.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLDefinitionsLMM.h"
#include "AQLMarketDataLMM.h"
#include "AQLMarketData.h"
#include "AQLPriceLMMCalibration.h"
#include "AQLMathDateUtilities.h"
#include "AQLMathDateCalculations.h"

//furuya//
#include <iterator>

using namespace std;

// constructor
/*!
	@param funFlg

*/
AQLCalibrationParametersLMM::AQLCalibrationParametersLMM(void)
: AQLCalibrationParameters()
{
}

// destructor
/*!

*/
AQLCalibrationParametersLMM::~AQLCalibrationParametersLMM(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] currency
	@return AQLString object name

*/
AQLString 
AQLCalibrationParametersLMM::createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy)
{
	AQLString tmpCurrency = ccy;
	const AQLString CALIBINFONAME = tmpCurrency.toUpper() + "_LMMCalibInfoEntity";
	AQLObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		// do nothing
		return CALIBINFONAME;
	}
	AQLObject *info = new AQLObject;
	objPool.set(CALIBINFONAME, info);
	// set name
	info->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(CALIBINFONAME);

	tmpCurrency.toLower();

	AQLString is_extratenor_calib_str = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_IS_EXTRATENOR_CALIB);
	is_extratenor_calib_str.toUpper();
	bool is_extratenor_calib = (is_extratenor_calib_str == "TRUE");
	info->add(CALIBRATION_DATA_IS_EXTRATENOR_CALIB, new AQLDataBool(is_extratenor_calib));

	// T_fix_,  T_fix_30_360_
	DoubleVector tenor_30_360, tenor, deltatenor;
	int maxTerm = (int)AQLMarketDataLMM::getMaxTerm(tmpCurrency);
	BoolVector tenor_extraflag;

	AQLString dayCountStr = AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create tenor 
	AQLString freq = FREQ_SEMI_ANNUAL;
	if (AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	AQLStringVector exTenor = AQLMarketDataLMM::getCanonicalGridExTenor();
 	AQLMarketDataLMM::getCanonicalGrid(tenor_30_360, tenor, deltatenor, tenor_extraflag, asOfDate, dayCount, freq, maxTerm, exTenor);

	// skew
	double skew = AQLMarketDataLMM::getSkew(tmpCurrency);
	if (skew <= 0. || skew >= 2.)
	{
		AQLString msg = tmpCurrency + " Q = " + AQLString(skew, 2) + " is out of range. Q must be in (0, 2).";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	// const shift
	double constShift = AQLMarketDataLMM::getConstShift(tmpCurrency);

	DoubleVector tenor_vollmm, tenor_30_360_vollmm;
	if(is_extratenor_calib)
	{
		DoubleVector::iterator tenor_copy_begin = tenor.begin();
		if (tenor[0] == 0.0)
		{
			tenor_copy_begin++;
		}
		DoubleVector::iterator tenor_30_360_copy_begin = tenor_30_360.begin();
		if (tenor_30_360[0] == 0.0)
		{
			tenor_30_360_copy_begin++;
		}
		std::copy(tenor_copy_begin, tenor.end(), std::back_inserter(tenor_vollmm));
		std::copy(tenor_30_360_copy_begin, tenor_30_360.end(), std::back_inserter(tenor_30_360_vollmm));
	}
	else
	{
		unsigned int tenor_copy_start = 0;
		if (tenor[0] == 0.0)
		{
			tenor_copy_start++;
		}
		unsigned int tenor_30_360_copy_start = 0;
		if (tenor_30_360[0] == 0.0)
		{
			tenor_30_360_copy_start++;
		}
		for(unsigned int i = tenor_copy_start; i < tenor.size(); i++)
		{
			if (!tenor_extraflag[i])
			{
				tenor_vollmm.push_back(tenor[i]);
			}
		}
		for(unsigned int i = tenor_30_360_copy_start; i < tenor_30_360.size(); i++)
		{
			if (!tenor_extraflag[i])
			{
				tenor_30_360_vollmm.push_back(tenor_30_360[i]);
			}
		}
	}

	// set tenor and tenor_30_360 without extratenor
	info->add(PRICING_DATA_CALIBCANONICAL_FREQ, new AQLDataString(freq));
	info->add(PRICING_DATA_CALIBCANONICAL_T, new AQLDataDoubles(tenor));
	info->add(PRICING_DATA_CALIBCANONICAL_T_DELTATENOR, new AQLDataDoubles(deltatenor));
	info->add(PRICING_DATA_CALIBCANONICAL_T_30_360, new AQLDataDoubles(tenor_30_360));
	info->add(PRICING_DATA_CALIBCANONICAL_T_EXTRAFLAG, new AQLDataBools(tenor_extraflag));
	info->add(CALIBRATION_DATA_MAXTERM, new AQLDataDouble(maxTerm));
	info->add(CALIBRATION_DATA_VOL_SKEW, new AQLDataDouble(skew));
	info->add(CALIBRATION_DATA_VOL_CONSTSHIFT, new AQLDataDouble(constShift));

	// set up optimization method type
	AQLString optMethodType = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_METHOD_TYPE);
	optMethodType.toUpper();
	info->add(CALIBRATION_DATA_OPT_METHOD_TYPE, new AQLDataString(optMethodType));

	// set up cost function mode
	AQLString costFuncMode = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_COST_FUNC_MODE);
	costFuncMode.toUpper();
	info->add(CALIBRATION_DATA_OPT_COST_FUNC_MODE, new AQLDataString(costFuncMode));

	// set up optimization target
	AQLString costFuncTarget = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_COST_FUNC_TARGET);
	costFuncTarget.toUpper();
	info->add(CALIBRATION_DATA_OPT_COST_FUNC_TARGET, new AQLDataString(costFuncTarget));

	// get cap vol file
	AQLString capFile = AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_VOL_FILE));

	// get swaption vol file
	AQLString swaptionFile = AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_VOL_FILE));
	AQLString swaptionWeightFile = AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_WEIGHT_FILE));

	// create market param
	// ##### cap parameters
	//AQLString capTerm = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TERM);
	AQLString capTenor = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TENOR);
	//AQLStringVector capTenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TENOR).toToken(MULTI_STATIC_DATA_DELIMITER);
	AQLStringVector capTermVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TERM).toToken(MULTI_STATIC_DATA_DELIMITER);
	//unsigned int capTenorSize = capTenorVec.size();
	unsigned int capTermSize = capTermVec.size();
	//AQLStringVector capTermVec(capTenorSize,capTerm);
	AQLStringVector capTenorVec(capTermSize,capTenor);

	DoubleVector capBlackVolVec(capTermSize);
	unsigned int idx = 0;
	for (unsigned int i = 0; i < capTermSize; i++)
	{
		capBlackVolVec[idx++] = AQLMarketData::getVolatilityVal(capFile, capTermVec[i], capTenorVec[i], true);
	}

	// ##### swaption parameters
	// optionmaturity
	AQLStringVector optionMatVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_TERM).toToken(MULTI_STATIC_DATA_DELIMITER);
	// swapterm
	AQLStringVector swapTenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_TENOR).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int optionMatSize = optionMatVec.size();
	unsigned int swapTenorSize = swapTenorVec.size();

	DoubleMatrix swaptionVolMat(optionMatSize, DoubleVector(swapTenorSize));
	DoubleMatrix swaptionVolWeightMat(optionMatSize, DoubleVector(swapTenorSize));
	for (unsigned int i = 0; i < optionMatSize; i++)
	{
		for (unsigned int j = 0; j < swapTenorSize; j++)
		{
			swaptionVolMat[i][j] = AQLMarketData::getVolatilityVal(swaptionFile, optionMatVec[i], swapTenorVec[j], true);
			swaptionVolWeightMat[i][j] = AQLMarketData::getVolatilityVal(swaptionWeightFile, optionMatVec[i], swapTenorVec[j], true);
		}
	}

	// set up date information of Libor
	AQLString liborDayCount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_DAYCOUNT);
	AQLString liborSlidingRule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_SLIDINGRULE);
	AQLString liborCalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_CALENDAR);
	AQLString liborFrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_FREQUENCY);
	bool liborIncludeLast = false;
	if(liborFrequency != freq)
	{
		AQLString msg = "Libor frequency is inconsistent with canonical frequency";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	// set up date information of cap
	AQLString capDayCount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_DAYCOUNT);
	AQLString capSlidingRule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_SLIDINGRULE);
	AQLString capCalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_CALENDAR);
	AQLString capSpotLag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_SPOTLAG);
	AQLString capFrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_FREQUENCY);
	bool capIncludeLast = false;
	// set up date information of swaption
	AQLString swaptionDayCount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_DAYCOUNT);
	AQLString swaptionSlidingRule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_SLIDINGRULE);
	AQLString swaptionCalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_CALENDAR);
	AQLString swaptionSpotLag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_SPOTLAG);
	AQLString swaptionFrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_FREQUENCY);
	AQLString swaptionVolType = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_VOLTYPE);
	double swaptionVolForwardShift = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_MARKET_FORWARDSHIFT).getDoubleValue();
	bool swaptionIncludeLast = false;
	//set up curve information of swaption
	AQLString swaptionForecastCurve = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_FORECASTCURVE);
	AQLString swaptionDiscountCurve = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_DISCOUNTCURVE);


	// get Param_V_,  Param_F_,  Param_G_ file
	AQLFileAccessor initParamVFile( AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INIT_V_FILE)) );
	AQLFileAccessor initParamFFile( AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INIT_F_FILE)) );
	AQLFileAccessor initParamGFile( AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INIT_G_FILE)) );

	AQLStringMatrix initParamVMtx, initParamFMtx, initParamGMtx;
	initParamVFile.readAllData(MARKET_DATA_DELIMITER, initParamVMtx);
	initParamFFile.readAllData(MARKET_DATA_DELIMITER, initParamFMtx);
	initParamGFile.readAllData(MARKET_DATA_DELIMITER, initParamGMtx);
	initParamVFile.close();
	initParamFFile.close();
	initParamGFile.close();
	if (initParamVMtx.size() == 0 || initParamFMtx.size() == 0 || initParamGMtx.size() == 0)
	{
		AQLString msg = "AQLCalibrationParametersLMM::createCalibrationInfo failed. Initial param file is empty.";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	if (initParamVMtx.size() != 4 || initParamFMtx.size() != 8)
	{
		AQLString msg = "AQLCalibrationParametersLMM::createCalibrationInfo failed. Initial param file is wrong format.";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	if ((initParamGMtx.size() != 2) || (initParamGMtx[0].size() != initParamGMtx[1].size()))
	{
		AQLString msg = "AQLCalibrationParametersLMM::createCalibrationInfo failed. Initial values of G(T) must be given in the form (tenorG, paramG).";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	DoubleVector paramV(4), paramF(9);
	paramV[0] = initParamVMtx[0][0].trimLeft().trimRight().getDoubleValue();// a
	paramV[1] = initParamVMtx[1][0].trimLeft().trimRight().getDoubleValue();// b
	paramV[2] = initParamVMtx[2][0].trimLeft().trimRight().getDoubleValue();// c
	paramV[3] = initParamVMtx[3][0].trimLeft().trimRight().getDoubleValue();// d
	paramF[0] = initParamFMtx[0][0].trimLeft().trimRight().getDoubleValue();// decay
	paramF[1] = initParamFMtx[1][0].trimLeft().trimRight().getDoubleValue();// amp1
	paramF[2] = initParamFMtx[2][0].trimLeft().trimRight().getDoubleValue();// phase1
	paramF[3] = initParamFMtx[3][0].trimLeft().trimRight().getDoubleValue();// amp2
	paramF[4] = initParamFMtx[4][0].trimLeft().trimRight().getDoubleValue();// phase2
	paramF[5] = initParamFMtx[5][0].trimLeft().trimRight().getDoubleValue();// amp3
	paramF[6] = initParamFMtx[6][0].trimLeft().trimRight().getDoubleValue();// phase3
	paramF[7] = initParamFMtx[7][0].trimLeft().trimRight().getDoubleValue();// shift
	paramF[8] = maxTerm;// MaxTime

	//unsigned int paramGSize = tenor_vollmm.size();
	//if (paramGSize > initParamGMtx[0].size())
	//{
	//	throw AQLCoreInvalidData(
	//		"AQLCalibrationParametersLMM::createCalibrationInfo failed. Size of param G is less than size of grids."
	//		,__FILE__,__LINE__);
	//}
	AQLString frequencyG = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_FREQUENCY_G).toUpper();
	double gridsizeG = AQLMathDateCalculations::getPeriodFrequencyInMonths(frequencyG)/12.0;
	const unsigned int paramGSize = (int)((maxTerm/gridsizeG)+0.5);
	if (paramGSize > initParamGMtx[0].size())
	{
		throw AQLCoreInvalidData("AQLCalibrationParametersLMM::createCalibrationInfo failed. Size of param G is less than size of grids.",__FILE__,__LINE__);
	}

	DoubleVector tenorG;
	DoubleVector paramG;
	for (unsigned int i = 0; i < paramGSize; i++)
	{
		tenorG.push_back(initParamGMtx[0][i].getDoubleValue());
		paramG.push_back(initParamGMtx[1][i].getDoubleValue());
	}

	AQLString interpolationG = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INTERPOLATION_G);
	if (interpolationG == AQ_NO_DATA)
	{
		interpolationG = "fn_stepinterpolation";
	}

	if (frequencyG != FREQ_SEMI_ANNUAL && frequencyG != FREQ_QUARTERLY)
	{
		AQLString msg = "Frequency of LMM function G input must be """ + AQLString(FREQ_SEMI_ANNUAL) +""" or """ + AQLString(FREQ_QUARTERLY) + """, However input is " + frequencyG + ".";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// set market param
	// ##### cap parameters
	// term
	info->add(IR_CALIBRATION_DATA_CAP_TERM, new AQLDataStrings(capTermVec));
	// tenor
	info->add(IR_CALIBRATION_DATA_CAP_TENOR, new AQLDataStrings(capTenorVec));
	// black vol
	info->add(AQLString("CAP_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, new AQLDataDoubles(capBlackVolVec));
	// ##### swaption parameters
	// optionmaturity
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new AQLDataStrings(optionMatVec));
	// swaptenor
	info->add(IR_CALIBRATION_DATA_SWAPTENOR, new AQLDataStrings(swapTenorVec));
	// black vol
	//if( costFuncTarget == "VOLATILITY" )
	//{
	info->add(AQLString("SWAPTION_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, new AQLDataDoubleMatrix(swaptionVolMat));
	//}
	//else if( costFuncTarget == "PREMIUM" )
	//{
	//	info->add(AQLString("SWAPTION_") + IR_CALIBRATION_DATA_SWAPTIONPREM, new AQLDataDoubleMatrix(swaptionBlackVolPremMat));
	//}
	//else
	//{
	//	AQLString msg = "Wrong costFuncTarget! : " + costFuncTarget + "?";
	//	throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	//}

	// swaption vol weight
	info->add(AQLString(IR_CALIBRATION_DATA_SWAPTIONVOLWEIGHT), new AQLDataDoubleMatrix(swaptionVolWeightMat));

	// swaption vol type
	info->add(AQLString(IR_CALIBRATION_DATA_SWAPTIONVOLTYPE), new AQLDataString(swaptionVolType));

	// swaption vol strike shift
	info->add(AQLString(IR_CALIBRATION_DATA_SWAPTIONVOL_MARKET_FORWARDSHIFT), new AQLDataDouble(swaptionVolForwardShift));

	// swaption curve name
	info->add(AQLString(PRICING_DATA_FORECASTCURVE), new AQLDataString(swaptionForecastCurve));
	info->add(AQLString(PRICING_DATA_DISCOUNTCURVE), new AQLDataString(swaptionDiscountCurve));

	// set calib parameters
	// param V
	info->add(CALIBRATION_DATA_PARAM_V, new AQLDataDoubles(paramV));
	// param F
	info->add(CALIBRATION_DATA_PARAM_F, new AQLDataDoubles(paramF));
	// param G
	info->add(CALIBRATION_DATA_TENOR_G, new AQLDataDoubles(tenorG));
	info->add(CALIBRATION_DATA_PARAM_G, new AQLDataDoubles(paramG));
	info->add(CALIBRATION_DATA_INTERPOLATION_G, new AQLDataString(interpolationG));
	info->add(CALIBRATION_DATA_FREQUENCY_G, new AQLDataString(frequencyG));

	// swaption vol approximation method 
	AQLString swaptionApproxMethod = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_APPROX_METHOD);
	swaptionApproxMethod.toUpper();
	info->add(CALIBRATION_DATA_SWAPTION_APPROX_MEHOD, new AQLDataString(swaptionApproxMethod));

	// set up optimization parameters
	double boundaryMaxV = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MAX_V).getDoubleValue();
	double boundaryMinV = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MIN_V).getDoubleValue();
	double boundaryMaxF = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MAX_F).getDoubleValue();
	double boundaryMinF = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MIN_F).getDoubleValue();
	double boundaryMaxG = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MAX_G).getDoubleValue();
	double boundaryMinG = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MIN_G).getDoubleValue();
	double smoothBoundaryG = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_SMOOTH_BOUNDARY_G).getDoubleValue();
	double levenbergAdjustTor = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_LEBENBERG_ADJUST_TOR).getDoubleValue();
	double levenbergAdjustV = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_LEBENBERG_ADJUST_V).getDoubleValue();
	double levenbergAdjustF = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_LEBENBERG_ADJUST_F).getDoubleValue();
	int maxIteration = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_MAX_ITERATION).getIntValue();
	int maxStationaryStateIteration = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_MAX_STATIONARY_STATE_ITERATION).getIntValue();
	double rootEpsilon = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_ROOT_EPSILON).getDoubleValue();
	double functionEpsilon = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_FUNCTION_EPSILON).getDoubleValue();
	double gradientNormEpsilon = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_GRADIENT_NORM_EPSILON).getDoubleValue();
	AQLString vegaWeight = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_VEGA_WEIGHT);
	int vfgLoopNum = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_LOOP_NUM).getIntValue();
	AQLStringVector volWeight(2);
    volWeight[0] = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_CAPFLOORS_WEIGHT);
	volWeight[1] = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_SWAPTIONS_WEIGHT);
	int numSmallSteps = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_NUM_SMALL_STEPS).getIntValue();
	AQLString optVFlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_TARGET_V);
	AQLString optFFlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_TARGET_F);
	AQLString optGFlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_TARGET_G);

	AQLString doOptimize = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_DO_OPTIMIZE);
	doOptimize.toUpper();
	if(doOptimize != "FALSE"){
		doOptimize = "TRUE";
	}

	// set optimization parameters
	info->add(CALIBRATION_DATA_BOUNDARY_MAX_V, new AQLDataDouble(boundaryMaxV));
	info->add(CALIBRATION_DATA_BOUNDARY_MIN_V, new AQLDataDouble(boundaryMinV));
	info->add(CALIBRATION_DATA_BOUNDARY_MAX_F, new AQLDataDouble(boundaryMaxF));
	info->add(CALIBRATION_DATA_BOUNDARY_MIN_F, new AQLDataDouble(boundaryMinF));
	info->add(CALIBRATION_DATA_BOUNDARY_MAX_G, new AQLDataDouble(boundaryMaxG));
	info->add(CALIBRATION_DATA_BOUNDARY_MIN_G, new AQLDataDouble(boundaryMinG));
	info->add(CALIBRATION_DATA_SMOOTH_BOUNDARY_G, new AQLDataDouble(smoothBoundaryG));
	info->add(CALIBRATION_DATA_LEVENBERG_ADJUST_Tor, new AQLDataDouble(levenbergAdjustTor));
	info->add(CALIBRATION_DATA_LEVENBERG_ADJUST_V, new AQLDataDouble(levenbergAdjustV));
	info->add(CALIBRATION_DATA_LEVENBERG_ADJUST_F, new AQLDataDouble(levenbergAdjustF));
	info->add(CALIBRATION_DATA_MAX_ITERATION, new AQLDataInt(maxIteration));
	info->add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, new AQLDataInt(maxStationaryStateIteration));
	info->add(CALIBRATION_DATA_ROOT_EPSILON, new AQLDataDouble(rootEpsilon));
    info->add(CALIBRATION_DATA_FUNCTION_EPSILON, new AQLDataDouble(functionEpsilon));
    info->add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, new AQLDataDouble(gradientNormEpsilon));
	info->add(CALIBRATION_DATA_VEGA_WEIGHT, new AQLDataString(vegaWeight));
	info->add(CALIBRATION_DATA_VFG_LOOP_NUM, new AQLDataInt(vfgLoopNum));
	info->add(CALIBRATION_DATA_VOL_WEIGHT, new AQLDataStrings(volWeight));
	info->add(CALIBRATION_DATA_NUM_SMALL_STEPS, new AQLDataInt(numSmallSteps));
	info->add(CALIBRATION_DATA_OPT_V_FLAG, new AQLDataString(optVFlag));
	info->add(CALIBRATION_DATA_OPT_F_FLAG, new AQLDataString(optFFlag));
	info->add(CALIBRATION_DATA_OPT_G_FLAG, new AQLDataString(optGFlag));
	info->add(CALIBRATION_DATA_DO_OPT_FLAG, new AQLDataString(doOptimize));

	// set up T stream of cap
	AQLString capSpotLag_mns = "-" + capSpotLag;
	AQLDate _00_pay = AQLMathDateUtilities::getDate(asOfDate, capSpotLag, liborSlidingRule, liborCalendar);
	AQLDate _0_pay, last_pay;
	vector<AQLDate> date_stream;
	for (unsigned int i = 0; i < capTermSize; ++i)
	{
		// calc date
		AQLString capTerm = capTermVec[i];
		AQLString capTenor = capTenorVec[i];
		_0_pay   = AQLMathDateUtilities::getDate(_00_pay, capTenor, liborSlidingRule, liborCalendar); // start date?
		last_pay = AQLMathDateUtilities::getDate(_0_pay, capTerm, liborSlidingRule, liborCalendar); // expiry date?

		date_stream = AQLMathDateUtilities::generateSchedule(_0_pay, last_pay, liborFrequency, liborSlidingRule, liborCalendar);

		DoubleVector T_pay_cap( date_stream.size() - 1 );
		DoubleVector T_fix_cap( date_stream.size() - 1 );
		DoubleVector tau_L_cap( date_stream.size() - 1 );
		DoubleVector tau_cap( date_stream.size() - 1 );

		AQLDate tmp_D;
		for(size_t j = 0; j < date_stream.size() - 1; ++j)
		{
			T_pay_cap[j] = AQLMathDateUtilities::getTerm( asOfDate, date_stream[j + 1], dayCountStr, true );

			tmp_D = AQLMathDateUtilities::getDate(date_stream[j], capSpotLag_mns, liborSlidingRule, liborCalendar);
			T_fix_cap[j] = AQLMathDateUtilities::getTerm( asOfDate, tmp_D, dayCountStr, true );

			tau_L_cap[j] = AQLMathDateUtilities::getTerm( date_stream[j], date_stream[j + 1], liborDayCount, liborIncludeLast );
			tau_cap[j] = AQLMathDateUtilities::getTerm( date_stream[j], date_stream[j + 1], capDayCount, capIncludeLast );
		}

		AQLString prefix = "CAP_";
		AQLString postfix = "_" + capTerm + "_" + capTenor;
		info->add(prefix + IR_CALIBRATION_DATA_T_PAY + postfix, new AQLDataDoubles(T_pay_cap));
		info->add(prefix + IR_CALIBRATION_DATA_T_FIX + postfix, new AQLDataDoubles(T_fix_cap));
		info->add(prefix + IR_CALIBRATION_DATA_TAU_L + postfix, new AQLDataDoubles(tau_L_cap));
		info->add(prefix + IR_CALIBRATION_DATA_TAU + postfix, new AQLDataDoubles(tau_cap));
	}

	// set up T stream of swaption
	AQLDate last_pay_L;
	vector<AQLDate> date_stream_L;
	for (unsigned int i = 0; i < optionMatSize; ++i)
	{
		// calc date
		const AQLString optionMat = optionMatVec[i];
		_00_pay = AQLMathDateUtilities::getDate(asOfDate, optionMat, swaptionSlidingRule, swaptionCalendar);
		_0_pay = AQLMathDateUtilities::getDate(_00_pay, swaptionSpotLag, swaptionSlidingRule, swaptionCalendar);
		if (swaptionForecastCurve == swaptionDiscountCurve && liborFrequency == swaptionFrequency)
		{
			// This is a temporary setting now(2016/06/13). Please remove it when EOD setting is changed to dual curve.
			_00_pay = AQLMathDateUtilities::getDate(asOfDate, swaptionSpotLag, swaptionSlidingRule, swaptionCalendar);
			_0_pay = AQLMathDateUtilities::getDate(_00_pay, optionMat, swaptionSlidingRule, swaptionCalendar);
		}
		for(unsigned int j = 0; j < swapTenorSize; ++j)
		{
			const AQLString swapTenor = swapTenorVec[j];

			last_pay_L    = AQLMathDateUtilities::getDate(_0_pay, swapTenor, liborSlidingRule, liborCalendar);
			date_stream_L = AQLMathDateUtilities::generateSchedule(_0_pay, last_pay_L, liborFrequency, liborSlidingRule, liborCalendar);

			DoubleVector T_pay_L_swaption(date_stream_L.size() - 1);
			DoubleVector T_fix_L_swaption(date_stream_L.size() - 1);
			DoubleVector tau_L_swaption(date_stream_L.size() - 1);
			for(unsigned int k = 0; k < date_stream_L.size() - 1; ++k)
			{
				T_pay_L_swaption[k] = AQLMathDateUtilities::getTerm( asOfDate, date_stream_L[k + 1], dayCountStr, true );
				T_fix_L_swaption[k] = AQLMathDateUtilities::getTerm( asOfDate, date_stream_L[k], dayCountStr, true );
				tau_L_swaption[k] = AQLMathDateUtilities::getTerm( date_stream_L[k], date_stream_L[k + 1], liborDayCount, liborIncludeLast );
			}

			last_pay    = AQLMathDateUtilities::getDate(_0_pay, swapTenor, swaptionSlidingRule, swaptionCalendar);
			date_stream = AQLMathDateUtilities::generateSchedule(_0_pay, last_pay, swaptionFrequency, swaptionSlidingRule, swaptionCalendar);

			DoubleVector T_pay_swaption(date_stream.size() - 1);
			DoubleVector tau_swaption(date_stream.size() - 1);
			for(unsigned int k = 0; k < date_stream.size() - 1; ++k)
			{
				T_pay_swaption[k] = AQLMathDateUtilities::getTerm( asOfDate, date_stream[k + 1], dayCountStr, true );
				tau_swaption[k] = AQLMathDateUtilities::getTerm( date_stream[k], date_stream[k + 1], swaptionDayCount, swaptionIncludeLast );
			}

			const AQLString prefix = "SWAPTION_";
			const AQLString postfix = "_" + optionMat + "_" + swapTenor;
			info->add(prefix + IR_CALIBRATION_DATA_T_PAY_L + postfix, new AQLDataDoubles(T_pay_L_swaption));
			info->add(prefix + IR_CALIBRATION_DATA_T_FIX_L + postfix, new AQLDataDoubles(T_fix_L_swaption));
			info->add(prefix + IR_CALIBRATION_DATA_TAU_L + postfix, new AQLDataDoubles(tau_L_swaption));
			info->add(prefix + IR_CALIBRATION_DATA_T_PAY + postfix, new AQLDataDoubles(T_pay_swaption));
			info->add(prefix + IR_CALIBRATION_DATA_TAU + postfix, new AQLDataDoubles(tau_swaption));
		}
	}

	// set up correlation information
	if (AQLCoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST) == "TRUE")
	{
		info->add(CALIBRATION_DATA_LMM_CALIB_CORRELATION, new AQLDataReference()).convertFromString("cor" + tmpCurrency.toUpper() + "ir");
	}
	else
	{
		AQLString sdeName = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		info->add(CALIBRATION_DATA_LMM_CALIB_CORRELATION, new AQLDataReference()).convertFromString(PREFIX_COR + sdeName);
	}

	// set up the flag for setting up result of lmm swatpion vol
	AQLString isResultoutLMMSwaptionStr = mpCalibStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_CALIB_LMM_ISRESULTOUT_SWAPTION);
	const bool isResultoutLMMSwaption = (isResultoutLMMSwaptionStr.toUpper() == "TRUE" || isResultoutLMMSwaptionStr == AQ_NO_DATA);
	info->add(CALIBRATION_DATA_IS_RESULTOUT_LMM_SWAPTION, new AQLDataBool(isResultoutLMMSwaption));

	return CALIBINFONAME;

}

/*!
    @brief get parameters for calibration of correlation

	@param[in] ccy
	@param[out] parameters for correlation of calibration

*/
void
AQLCalibrationParametersLMM::getCalibCorParam(const AQLString &ccy, AQLCalibCorrelationParam &param)
{
	AQLString tmpccy = ccy;
	tmpccy.toLower();

	AQLString corFuncParamFilePath = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_FUNC_PARAM_FILE);
	AQLFileAccessor corFuncParamFile(AQLMarketData::getNumFileName(corFuncParamFilePath));
	AQLStringMatrix corFuncParamMtx;
	corFuncParamFile.readAllData(MARKET_DATA_DELIMITER, corFuncParamMtx);
	corFuncParamFile.close();
	double param_x = corFuncParamMtx[0][0].trimLeft().trimRight().getDoubleValue();
	double param_y = corFuncParamMtx[1][0].trimLeft().trimRight().getDoubleValue();

	AQLString maxtermFilePath = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_FUNC_TMAX_FILE);
	AQLFileAccessor maxtermFile(AQLMarketData::getNumFileName(maxtermFilePath));
	AQLStringMatrix maxtermMtx;
	maxtermFile.readAllData(MARKET_DATA_DELIMITER, maxtermMtx);
	maxtermFile.close();
	double maxterm = maxtermMtx[0][0].trimLeft().trimRight().getDoubleValue();

	AQLString corFuncTypeFilePath = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_FUNC_TYPE_FILE);
	AQLFileAccessor corFuncTypeFile(AQLMarketData::getNumFileName(corFuncTypeFilePath));
	AQLStringMatrix corFuncTypeMtx;
	corFuncTypeFile.readAllData(MARKET_DATA_DELIMITER, corFuncTypeMtx);
	corFuncTypeFile.close();
	AQLString corFuncType = corFuncTypeMtx[0][0].trimLeft().trimRight();
	corFuncType.toUpper();

	AQLString targetFactorFilePath = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_TARGET_FACTOR_FILE);
	AQLFileAccessor targetFactorFile(AQLMarketData::getNumFileName(targetFactorFilePath));
	AQLStringMatrix targetFactorMtx;
	targetFactorFile.readAllData(MARKET_DATA_DELIMITER, targetFactorMtx);
	targetFactorFile.close();
	unsigned int targetFactor = targetFactorMtx[0][0].trimLeft().trimRight().getIntValue();

	param.funcParam_x                 = param_x;
	param.funcParam_y                 = param_y;
	param.maxTerm                     = maxterm;
	param.corFuncType                 = corFuncType;
	param.factorNum                   = targetFactor;
	param.optCorType                  = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_TARGET_TYPE).toUpper();
	param.boundaryMax                 = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_BOUNDARY_MAX).getDoubleValue();
	param.boundaryMin                 = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_BOUNDARY_MIN).getDoubleValue();
	param.maxIteration                = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_MAX_ITERATION).getIntValue();
	param.maxStationaryStateIteration = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_MAX_STATIONARY_STATE_ITERATION).getIntValue();
	param.rootEpsilon                 = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_ROOT_EPSILON).getDoubleValue();
	param.functionEpsilon             = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_FUNCTION_EPSILON).getDoubleValue();
	param.gradientNormEpsilon         = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_GRADIENT_NORM_EPSILON).getDoubleValue();
	param.optCorMethodType            = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_METHOD_TYPE).toUpper();

	AQLString is_extratenor_calib = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_IS_EXTRATENOR_CALIB).toUpper();
	param.isExtraTenorCalib = (is_extratenor_calib == "TRUE");
}


/*!
    @brief get calibration property value

	@param[in] key
	@param[in] grid
	@return AQLString value

*/
AQLString 
AQLCalibrationParametersLMM::getCalibStaticDataValue(const AQLString &key, const AQLString &grid)
{
	AQLString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == AQ_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}
	
	return ret;
}

