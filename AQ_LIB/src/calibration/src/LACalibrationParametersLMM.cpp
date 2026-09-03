/*! @file
    @brief Calibration method create class for lmm
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersLMM.cpp
//
//  DESCRIPTION :     Calibration method create class for lmm
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


#include "LAString.h"
#include "LAObjectPool.h"
#include "LAObject.h"
#include "LAPriceDataType.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataReference.h"
#include "LAMathDefine.h"
#include "LADealUtils.h"
#include "LACalibrationParametersLMM.h"
#include "LAStaticData.h"
#include "LADefinitions.h"
#include "LADefinitionsCalibration.h"
#include "LADefinitionsLMM.h"
#include "LAMarketDataLMM.h"
#include "LAMarketData.h"
#include "LAPriceLMMCalibration.h"
#include "LAMathDateUtilities.h"
#include "LAMathDateCalculations.h"

//furuya//
#include <iterator>

using namespace std;

// constructor
/*!
	@param funFlg

*/
LACalibrationParametersLMM::LACalibrationParametersLMM(void)
: LACalibrationParameters()
{
}

// destructor
/*!

*/
LACalibrationParametersLMM::~LACalibrationParametersLMM(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] currency
	@return LAString object name

*/
LAString 
LACalibrationParametersLMM::createCalibrationInfo(LAObjectPool &objPool, const LAString &ccy)
{
	LAString tmpCurrency = ccy;
	const LAString CALIBINFONAME = tmpCurrency.toUpper() + "_LMMCalibInfoEntity";
	LAObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		// do nothing
		return CALIBINFONAME;
	}
	LAObject *info = new LAObject;
	objPool.set(CALIBINFONAME, info);
	// set name
	info->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(CALIBINFONAME);

	tmpCurrency.toLower();

	LAString is_extratenor_calib_str = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_IS_EXTRATENOR_CALIB);
	is_extratenor_calib_str.toUpper();
	bool is_extratenor_calib = (is_extratenor_calib_str == "TRUE");
	info->add(CALIBRATION_DATA_IS_EXTRATENOR_CALIB, new LADataBool(is_extratenor_calib));

	// T_fix_,  T_fix_30_360_
	DoubleVector tenor_30_360, tenor, deltatenor;
	int maxTerm = (int)LAMarketDataLMM::getMaxTerm(tmpCurrency);
	BoolVector tenor_extraflag;

	LAString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	LAPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create tenor 
	LAString freq = FREQ_SEMI_ANNUAL;
	if (LACoreDataService::getContext(ARG_KEY_CANONICALFREQ) == "QA")
	{
		freq = FREQ_QUARTERLY;
	}
	LAStringVector exTenor = LAMarketDataLMM::getCanonicalGridExTenor();
 	LAMarketDataLMM::getCanonicalGrid(tenor_30_360, tenor, deltatenor, tenor_extraflag, asOfDate, dayCount, freq, maxTerm, exTenor);

	// skew
	double skew = LAMarketDataLMM::getSkew(tmpCurrency);
	if (skew <= 0. || skew >= 2.)
	{
		LAString msg = tmpCurrency + " Q = " + LAString(skew, 2) + " is out of range. Q must be in (0, 2).";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	// const shift
	double constShift = LAMarketDataLMM::getConstShift(tmpCurrency);

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
	info->add(PRICING_DATA_CALIBCANONICAL_FREQ, new LADataString(freq));
	info->add(PRICING_DATA_CALIBCANONICAL_T, new LADataDoubles(tenor));
	info->add(PRICING_DATA_CALIBCANONICAL_T_DELTATENOR, new LADataDoubles(deltatenor));
	info->add(PRICING_DATA_CALIBCANONICAL_T_30_360, new LADataDoubles(tenor_30_360));
	info->add(PRICING_DATA_CALIBCANONICAL_T_EXTRAFLAG, new LADataBools(tenor_extraflag));
	info->add(CALIBRATION_DATA_MAXTERM, new LADataDouble(maxTerm));
	info->add(CALIBRATION_DATA_VOL_SKEW, new LADataDouble(skew));
	info->add(CALIBRATION_DATA_VOL_CONSTSHIFT, new LADataDouble(constShift));

	// set up optimization method type
	LAString optMethodType = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_METHOD_TYPE);
	optMethodType.toUpper();
	info->add(CALIBRATION_DATA_OPT_METHOD_TYPE, new LADataString(optMethodType));

	// set up cost function mode
	LAString costFuncMode = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_COST_FUNC_MODE);
	costFuncMode.toUpper();
	info->add(CALIBRATION_DATA_OPT_COST_FUNC_MODE, new LADataString(costFuncMode));

	// set up optimization target
	LAString costFuncTarget = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_COST_FUNC_TARGET);
	costFuncTarget.toUpper();
	info->add(CALIBRATION_DATA_OPT_COST_FUNC_TARGET, new LADataString(costFuncTarget));

	// get cap vol file
	LAString capFile = LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_VOL_FILE));

	// get swaption vol file
	LAString swaptionFile = LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_VOL_FILE));
	LAString swaptionWeightFile = LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_WEIGHT_FILE));

	// create market param
	// ##### cap parameters
	//LAString capTerm = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TERM);
	LAString capTenor = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TENOR);
	//LAStringVector capTenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TENOR).toToken(MULTI_STATIC_DATA_DELIMITER);
	LAStringVector capTermVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TERM).toToken(MULTI_STATIC_DATA_DELIMITER);
	//unsigned int capTenorSize = capTenorVec.size();
	unsigned int capTermSize = capTermVec.size();
	//LAStringVector capTermVec(capTenorSize,capTerm);
	LAStringVector capTenorVec(capTermSize,capTenor);

	DoubleVector capBlackVolVec(capTermSize);
	unsigned int idx = 0;
	for (unsigned int i = 0; i < capTermSize; i++)
	{
		capBlackVolVec[idx++] = LAMarketData::getVolatilityVal(capFile, capTermVec[i], capTenorVec[i], true);
	}

	// ##### swaption parameters
	// optionmaturity
	LAStringVector optionMatVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_TERM).toToken(MULTI_STATIC_DATA_DELIMITER);
	// swapterm
	LAStringVector swapTenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_TENOR).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int optionMatSize = optionMatVec.size();
	unsigned int swapTenorSize = swapTenorVec.size();

	DoubleMatrix swaptionVolMat(optionMatSize, DoubleVector(swapTenorSize));
	DoubleMatrix swaptionVolWeightMat(optionMatSize, DoubleVector(swapTenorSize));
	for (unsigned int i = 0; i < optionMatSize; i++)
	{
		for (unsigned int j = 0; j < swapTenorSize; j++)
		{
			swaptionVolMat[i][j] = LAMarketData::getVolatilityVal(swaptionFile, optionMatVec[i], swapTenorVec[j], true);
			swaptionVolWeightMat[i][j] = LAMarketData::getVolatilityVal(swaptionWeightFile, optionMatVec[i], swapTenorVec[j], true);
		}
	}

	// set up date information of Libor
	LAString liborDayCount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_DAYCOUNT);
	LAString liborSlidingRule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_SLIDINGRULE);
	LAString liborCalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_CALENDAR);
	LAString liborFrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_FREQUENCY);
	bool liborIncludeLast = false;
	if(liborFrequency != freq)
	{
		LAString msg = "Libor frequency is inconsistent with canonical frequency";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	// set up date information of cap
	LAString capDayCount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_DAYCOUNT);
	LAString capSlidingRule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_SLIDINGRULE);
	LAString capCalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_CALENDAR);
	LAString capSpotLag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_SPOTLAG);
	LAString capFrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_FREQUENCY);
	bool capIncludeLast = false;
	// set up date information of swaption
	LAString swaptionDayCount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_DAYCOUNT);
	LAString swaptionSlidingRule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_SLIDINGRULE);
	LAString swaptionCalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_CALENDAR);
	LAString swaptionSpotLag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_SPOTLAG);
	LAString swaptionFrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_FREQUENCY);
	LAString swaptionVolType = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_VOLTYPE);
	double swaptionVolForwardShift = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_MARKET_FORWARDSHIFT).getDoubleValue();
	bool swaptionIncludeLast = false;
	//set up curve information of swaption
	LAString swaptionForecastCurve = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_FORECASTCURVE);
	LAString swaptionDiscountCurve = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_DISCOUNTCURVE);


	// get Param_V_,  Param_F_,  Param_G_ file
	MAFileAccessor initParamVFile( LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INIT_V_FILE)) );
	MAFileAccessor initParamFFile( LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INIT_F_FILE)) );
	MAFileAccessor initParamGFile( LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INIT_G_FILE)) );

	LAStringMatrix initParamVMtx, initParamFMtx, initParamGMtx;
	initParamVFile.readAllData(MARKET_DATA_DELIMITER, initParamVMtx);
	initParamFFile.readAllData(MARKET_DATA_DELIMITER, initParamFMtx);
	initParamGFile.readAllData(MARKET_DATA_DELIMITER, initParamGMtx);
	initParamVFile.close();
	initParamFFile.close();
	initParamGFile.close();
	if (initParamVMtx.size() == 0 || initParamFMtx.size() == 0 || initParamGMtx.size() == 0)
	{
		LAString msg = "LACalibrationParametersLMM::createCalibrationInfo failed. Initial param file is empty.";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	if (initParamVMtx.size() != 4 || initParamFMtx.size() != 8)
	{
		LAString msg = "LACalibrationParametersLMM::createCalibrationInfo failed. Initial param file is wrong format.";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	if ((initParamGMtx.size() != 2) || (initParamGMtx[0].size() != initParamGMtx[1].size()))
	{
		LAString msg = "LACalibrationParametersLMM::createCalibrationInfo failed. Initial values of G(T) must be given in the form (tenorG, paramG).";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
	//	throw LACoreInvalidData(
	//		"LACalibrationParametersLMM::createCalibrationInfo failed. Size of param G is less than size of grids."
	//		,__FILE__,__LINE__);
	//}
	LAString frequencyG = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_FREQUENCY_G).toUpper();
	double gridsizeG = LAMathDateCalculations::getPeriodFrequencyInMonths(frequencyG)/12.0;
	const unsigned int paramGSize = (int)((maxTerm/gridsizeG)+0.5);
	if (paramGSize > initParamGMtx[0].size())
	{
		throw LACoreInvalidData("LACalibrationParametersLMM::createCalibrationInfo failed. Size of param G is less than size of grids.",__FILE__,__LINE__);
	}

	DoubleVector tenorG;
	DoubleVector paramG;
	for (unsigned int i = 0; i < paramGSize; i++)
	{
		tenorG.push_back(initParamGMtx[0][i].getDoubleValue());
		paramG.push_back(initParamGMtx[1][i].getDoubleValue());
	}

	LAString interpolationG = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INTERPOLATION_G);
	if (interpolationG == MLIB_NO_DATA)
	{
		interpolationG = "fn_stepinterpolation";
	}

	if (frequencyG != FREQ_SEMI_ANNUAL && frequencyG != FREQ_QUARTERLY)
	{
		LAString msg = "Frequency of LMM function G input must be """ + LAString(FREQ_SEMI_ANNUAL) +""" or """ + LAString(FREQ_QUARTERLY) + """, However input is " + frequencyG + ".";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// set market param
	// ##### cap parameters
	// term
	info->add(IR_CALIBRATION_DATA_CAP_TERM, new LADataStrings(capTermVec));
	// tenor
	info->add(IR_CALIBRATION_DATA_CAP_TENOR, new LADataStrings(capTenorVec));
	// black vol
	info->add(LAString("CAP_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, new LADataDoubles(capBlackVolVec));
	// ##### swaption parameters
	// optionmaturity
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new LADataStrings(optionMatVec));
	// swaptenor
	info->add(IR_CALIBRATION_DATA_SWAPTENOR, new LADataStrings(swapTenorVec));
	// black vol
	//if( costFuncTarget == "VOLATILITY" )
	//{
	info->add(LAString("SWAPTION_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, new LADataDoubleMatrix(swaptionVolMat));
	//}
	//else if( costFuncTarget == "PREMIUM" )
	//{
	//	info->add(LAString("SWAPTION_") + IR_CALIBRATION_DATA_SWAPTIONPREM, new LADataDoubleMatrix(swaptionBlackVolPremMat));
	//}
	//else
	//{
	//	LAString msg = "Wrong costFuncTarget! : " + costFuncTarget + "?";
	//	throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	//}

	// swaption vol weight
	info->add(LAString(IR_CALIBRATION_DATA_SWAPTIONVOLWEIGHT), new LADataDoubleMatrix(swaptionVolWeightMat));

	// swaption vol type
	info->add(LAString(IR_CALIBRATION_DATA_SWAPTIONVOLTYPE), new LADataString(swaptionVolType));

	// swaption vol strike shift
	info->add(LAString(IR_CALIBRATION_DATA_SWAPTIONVOL_MARKET_FORWARDSHIFT), new LADataDouble(swaptionVolForwardShift));

	// swaption curve name
	info->add(LAString(PRICING_DATA_FORECASTCURVE), new LADataString(swaptionForecastCurve));
	info->add(LAString(PRICING_DATA_DISCOUNTCURVE), new LADataString(swaptionDiscountCurve));

	// set calib parameters
	// param V
	info->add(CALIBRATION_DATA_PARAM_V, new LADataDoubles(paramV));
	// param F
	info->add(CALIBRATION_DATA_PARAM_F, new LADataDoubles(paramF));
	// param G
	info->add(CALIBRATION_DATA_TENOR_G, new LADataDoubles(tenorG));
	info->add(CALIBRATION_DATA_PARAM_G, new LADataDoubles(paramG));
	info->add(CALIBRATION_DATA_INTERPOLATION_G, new LADataString(interpolationG));
	info->add(CALIBRATION_DATA_FREQUENCY_G, new LADataString(frequencyG));

	// swaption vol approximation method 
	LAString swaptionApproxMethod = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_APPROX_METHOD);
	swaptionApproxMethod.toUpper();
	info->add(CALIBRATION_DATA_SWAPTION_APPROX_MEHOD, new LADataString(swaptionApproxMethod));

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
	LAString vegaWeight = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_VEGA_WEIGHT);
	int vfgLoopNum = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_LOOP_NUM).getIntValue();
	LAStringVector volWeight(2);
    volWeight[0] = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_CAPFLOORS_WEIGHT);
	volWeight[1] = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_SWAPTIONS_WEIGHT);
	int numSmallSteps = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_NUM_SMALL_STEPS).getIntValue();
	LAString optVFlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_TARGET_V);
	LAString optFFlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_TARGET_F);
	LAString optGFlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_TARGET_G);

	LAString doOptimize = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_DO_OPTIMIZE);
	doOptimize.toUpper();
	if(doOptimize != "FALSE"){
		doOptimize = "TRUE";
	}

	// set optimization parameters
	info->add(CALIBRATION_DATA_BOUNDARY_MAX_V, new LADataDouble(boundaryMaxV));
	info->add(CALIBRATION_DATA_BOUNDARY_MIN_V, new LADataDouble(boundaryMinV));
	info->add(CALIBRATION_DATA_BOUNDARY_MAX_F, new LADataDouble(boundaryMaxF));
	info->add(CALIBRATION_DATA_BOUNDARY_MIN_F, new LADataDouble(boundaryMinF));
	info->add(CALIBRATION_DATA_BOUNDARY_MAX_G, new LADataDouble(boundaryMaxG));
	info->add(CALIBRATION_DATA_BOUNDARY_MIN_G, new LADataDouble(boundaryMinG));
	info->add(CALIBRATION_DATA_SMOOTH_BOUNDARY_G, new LADataDouble(smoothBoundaryG));
	info->add(CALIBRATION_DATA_LEVENBERG_ADJUST_Tor, new LADataDouble(levenbergAdjustTor));
	info->add(CALIBRATION_DATA_LEVENBERG_ADJUST_V, new LADataDouble(levenbergAdjustV));
	info->add(CALIBRATION_DATA_LEVENBERG_ADJUST_F, new LADataDouble(levenbergAdjustF));
	info->add(CALIBRATION_DATA_MAX_ITERATION, new LADataInt(maxIteration));
	info->add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, new LADataInt(maxStationaryStateIteration));
	info->add(CALIBRATION_DATA_ROOT_EPSILON, new LADataDouble(rootEpsilon));
    info->add(CALIBRATION_DATA_FUNCTION_EPSILON, new LADataDouble(functionEpsilon));
    info->add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, new LADataDouble(gradientNormEpsilon));
	info->add(CALIBRATION_DATA_VEGA_WEIGHT, new LADataString(vegaWeight));
	info->add(CALIBRATION_DATA_VFG_LOOP_NUM, new LADataInt(vfgLoopNum));
	info->add(CALIBRATION_DATA_VOL_WEIGHT, new LADataStrings(volWeight));
	info->add(CALIBRATION_DATA_NUM_SMALL_STEPS, new LADataInt(numSmallSteps));
	info->add(CALIBRATION_DATA_OPT_V_FLAG, new LADataString(optVFlag));
	info->add(CALIBRATION_DATA_OPT_F_FLAG, new LADataString(optFFlag));
	info->add(CALIBRATION_DATA_OPT_G_FLAG, new LADataString(optGFlag));
	info->add(CALIBRATION_DATA_DO_OPT_FLAG, new LADataString(doOptimize));

	// set up T stream of cap
	LAString capSpotLag_mns = "-" + capSpotLag;
	LADate _00_pay = LAMathDateUtilities::getDate(asOfDate, capSpotLag, liborSlidingRule, liborCalendar);
	LADate _0_pay, last_pay;
	vector<LADate> date_stream;
	for (unsigned int i = 0; i < capTermSize; ++i)
	{
		// calc date
		LAString capTerm = capTermVec[i];
		LAString capTenor = capTenorVec[i];
		_0_pay   = LAMathDateUtilities::getDate(_00_pay, capTenor, liborSlidingRule, liborCalendar); // start date?
		last_pay = LAMathDateUtilities::getDate(_0_pay, capTerm, liborSlidingRule, liborCalendar); // expiry date?

		date_stream = LAMathDateUtilities::generateSchedule(_0_pay, last_pay, liborFrequency, liborSlidingRule, liborCalendar);

		DoubleVector T_pay_cap( date_stream.size() - 1 );
		DoubleVector T_fix_cap( date_stream.size() - 1 );
		DoubleVector tau_L_cap( date_stream.size() - 1 );
		DoubleVector tau_cap( date_stream.size() - 1 );

		LADate tmp_D;
		for(size_t j = 0; j < date_stream.size() - 1; ++j)
		{
			T_pay_cap[j] = LAMathDateUtilities::getTerm( asOfDate, date_stream[j + 1], dayCountStr, true );

			tmp_D = LAMathDateUtilities::getDate(date_stream[j], capSpotLag_mns, liborSlidingRule, liborCalendar);
			T_fix_cap[j] = LAMathDateUtilities::getTerm( asOfDate, tmp_D, dayCountStr, true );

			tau_L_cap[j] = LAMathDateUtilities::getTerm( date_stream[j], date_stream[j + 1], liborDayCount, liborIncludeLast );
			tau_cap[j] = LAMathDateUtilities::getTerm( date_stream[j], date_stream[j + 1], capDayCount, capIncludeLast );
		}

		LAString prefix = "CAP_";
		LAString postfix = "_" + capTerm + "_" + capTenor;
		info->add(prefix + IR_CALIBRATION_DATA_T_PAY + postfix, new LADataDoubles(T_pay_cap));
		info->add(prefix + IR_CALIBRATION_DATA_T_FIX + postfix, new LADataDoubles(T_fix_cap));
		info->add(prefix + IR_CALIBRATION_DATA_TAU_L + postfix, new LADataDoubles(tau_L_cap));
		info->add(prefix + IR_CALIBRATION_DATA_TAU + postfix, new LADataDoubles(tau_cap));
	}

	// set up T stream of swaption
	LADate last_pay_L;
	vector<LADate> date_stream_L;
	for (unsigned int i = 0; i < optionMatSize; ++i)
	{
		// calc date
		const LAString optionMat = optionMatVec[i];
		_00_pay = LAMathDateUtilities::getDate(asOfDate, optionMat, swaptionSlidingRule, swaptionCalendar);
		_0_pay = LAMathDateUtilities::getDate(_00_pay, swaptionSpotLag, swaptionSlidingRule, swaptionCalendar);
		if (swaptionForecastCurve == swaptionDiscountCurve && liborFrequency == swaptionFrequency)
		{
			// This is a temporary setting now(2016/06/13). Please remove it when EOD setting is changed to dual curve.
			_00_pay = LAMathDateUtilities::getDate(asOfDate, swaptionSpotLag, swaptionSlidingRule, swaptionCalendar);
			_0_pay = LAMathDateUtilities::getDate(_00_pay, optionMat, swaptionSlidingRule, swaptionCalendar);
		}
		for(unsigned int j = 0; j < swapTenorSize; ++j)
		{
			const LAString swapTenor = swapTenorVec[j];

			last_pay_L    = LAMathDateUtilities::getDate(_0_pay, swapTenor, liborSlidingRule, liborCalendar);
			date_stream_L = LAMathDateUtilities::generateSchedule(_0_pay, last_pay_L, liborFrequency, liborSlidingRule, liborCalendar);

			DoubleVector T_pay_L_swaption(date_stream_L.size() - 1);
			DoubleVector T_fix_L_swaption(date_stream_L.size() - 1);
			DoubleVector tau_L_swaption(date_stream_L.size() - 1);
			for(unsigned int k = 0; k < date_stream_L.size() - 1; ++k)
			{
				T_pay_L_swaption[k] = LAMathDateUtilities::getTerm( asOfDate, date_stream_L[k + 1], dayCountStr, true );
				T_fix_L_swaption[k] = LAMathDateUtilities::getTerm( asOfDate, date_stream_L[k], dayCountStr, true );
				tau_L_swaption[k] = LAMathDateUtilities::getTerm( date_stream_L[k], date_stream_L[k + 1], liborDayCount, liborIncludeLast );
			}

			last_pay    = LAMathDateUtilities::getDate(_0_pay, swapTenor, swaptionSlidingRule, swaptionCalendar);
			date_stream = LAMathDateUtilities::generateSchedule(_0_pay, last_pay, swaptionFrequency, swaptionSlidingRule, swaptionCalendar);

			DoubleVector T_pay_swaption(date_stream.size() - 1);
			DoubleVector tau_swaption(date_stream.size() - 1);
			for(unsigned int k = 0; k < date_stream.size() - 1; ++k)
			{
				T_pay_swaption[k] = LAMathDateUtilities::getTerm( asOfDate, date_stream[k + 1], dayCountStr, true );
				tau_swaption[k] = LAMathDateUtilities::getTerm( date_stream[k], date_stream[k + 1], swaptionDayCount, swaptionIncludeLast );
			}

			const LAString prefix = "SWAPTION_";
			const LAString postfix = "_" + optionMat + "_" + swapTenor;
			info->add(prefix + IR_CALIBRATION_DATA_T_PAY_L + postfix, new LADataDoubles(T_pay_L_swaption));
			info->add(prefix + IR_CALIBRATION_DATA_T_FIX_L + postfix, new LADataDoubles(T_fix_L_swaption));
			info->add(prefix + IR_CALIBRATION_DATA_TAU_L + postfix, new LADataDoubles(tau_L_swaption));
			info->add(prefix + IR_CALIBRATION_DATA_T_PAY + postfix, new LADataDoubles(T_pay_swaption));
			info->add(prefix + IR_CALIBRATION_DATA_TAU + postfix, new LADataDoubles(tau_swaption));
		}
	}

	// set up correlation information
	if (LACoreDataService::getContext(CONTEXT_KEY_ISEXCELREQUEST) == "TRUE")
	{
		info->add(CALIBRATION_DATA_LMM_CALIB_CORRELATION, new LADataReference()).convertFromString("cor" + tmpCurrency.toUpper() + "ir");
	}
	else
	{
		LAString sdeName = mpStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
		info->add(CALIBRATION_DATA_LMM_CALIB_CORRELATION, new LADataReference()).convertFromString(PREFIX_COR + sdeName);
	}

	// set up the flag for setting up result of lmm swatpion vol
	LAString isResultoutLMMSwaptionStr = mpCalibStaticData->getStaticData(tmpCurrency.toLower() + STATIC_DATA_KEY_CALIB_LMM_ISRESULTOUT_SWAPTION);
	const bool isResultoutLMMSwaption = (isResultoutLMMSwaptionStr.toUpper() == "TRUE" || isResultoutLMMSwaptionStr == MLIB_NO_DATA);
	info->add(CALIBRATION_DATA_IS_RESULTOUT_LMM_SWAPTION, new LADataBool(isResultoutLMMSwaption));

	return CALIBINFONAME;

}

/*!
    @brief get parameters for calibration of correlation

	@param[in] ccy
	@param[out] parameters for correlation of calibration

*/
void
LACalibrationParametersLMM::getCalibCorParam(const LAString &ccy, MACalibCorrelationParam &param)
{
	LAString tmpccy = ccy;
	tmpccy.toLower();

	LAString corFuncParamFilePath = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_FUNC_PARAM_FILE);
	MAFileAccessor corFuncParamFile(LAMarketData::getNumFileName(corFuncParamFilePath));
	LAStringMatrix corFuncParamMtx;
	corFuncParamFile.readAllData(MARKET_DATA_DELIMITER, corFuncParamMtx);
	corFuncParamFile.close();
	double param_x = corFuncParamMtx[0][0].trimLeft().trimRight().getDoubleValue();
	double param_y = corFuncParamMtx[1][0].trimLeft().trimRight().getDoubleValue();

	LAString maxtermFilePath = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_FUNC_TMAX_FILE);
	MAFileAccessor maxtermFile(LAMarketData::getNumFileName(maxtermFilePath));
	LAStringMatrix maxtermMtx;
	maxtermFile.readAllData(MARKET_DATA_DELIMITER, maxtermMtx);
	maxtermFile.close();
	double maxterm = maxtermMtx[0][0].trimLeft().trimRight().getDoubleValue();

	LAString corFuncTypeFilePath = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_FUNC_TYPE_FILE);
	MAFileAccessor corFuncTypeFile(LAMarketData::getNumFileName(corFuncTypeFilePath));
	LAStringMatrix corFuncTypeMtx;
	corFuncTypeFile.readAllData(MARKET_DATA_DELIMITER, corFuncTypeMtx);
	corFuncTypeFile.close();
	LAString corFuncType = corFuncTypeMtx[0][0].trimLeft().trimRight();
	corFuncType.toUpper();

	LAString targetFactorFilePath = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_TARGET_FACTOR_FILE);
	MAFileAccessor targetFactorFile(LAMarketData::getNumFileName(targetFactorFilePath));
	LAStringMatrix targetFactorMtx;
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

	LAString is_extratenor_calib = mpCalibStaticData->getStaticData(tmpccy + STATIC_DATA_KEY_CALIB_LMM_IS_EXTRATENOR_CALIB).toUpper();
	param.isExtraTenorCalib = (is_extratenor_calib == "TRUE");
}


/*!
    @brief get calibration property value

	@param[in] key
	@param[in] grid
	@return LAString value

*/
LAString 
LACalibrationParametersLMM::getCalibStaticDataValue(const LAString &key, const LAString &grid)
{
	LAString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == MLIB_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}
	
	return ret;
}

