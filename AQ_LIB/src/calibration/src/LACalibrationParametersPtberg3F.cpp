/*! @file
    @brief Calibration method create class for hw
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersPtberg3F.cpp
//
//  DESCRIPTION :     Calibration method create class for hw
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
#include "LAMathHWFuncMR.h"
#include "LAMathDefine.h"
#include "LADealUtils.h"
#include "LACalibrationParametersPtberg3F.h"
#include "LAStaticData.h"
#include "LADefinitions.h"
#include "LADefinitionsCalibration.h"
#include "LADefinitionsHW.h"
#include "LADefinitionsPtberg.h"
#include "LAMarketDataHW.h"
#include "LAMarketData.h"
#include "LAMathJamshidianSwaption.h"
#include "LAMathJamshidianSwaptionByImplyVol.h"
#include "LAMathAntonovFXOption.h"
#include "LAPriceFXDisplacedDiffusionCalibration3F.h"

using namespace std;

// constructor
/*!
	@param funFlg

*/
LACalibrationParametersPtberg3F::LACalibrationParametersPtberg3F(void)
: LACalibrationParametersPtberg()
{
}

// destructor
/*!

*/
LACalibrationParametersPtberg3F::~LACalibrationParametersPtberg3F(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] fx
	@return LAString object name

*/
LAString 
LACalibrationParametersPtberg3F::createCalibrationInfo(LAObjectPool &objPool, const LAString &fx)
{
	LAString keyFX = fx;
	const LAString CALIBINFONAME = keyFX.toUpper() + "_PtbergCalibInfoEntity";
	LAObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		LAString isReset = mpCalibStaticData->getStaticData(KEY_CALIB_INFO_ISRESET).toUpper();
		if (isReset != "TRUE")
		{
			// do nothing
			return CALIBINFONAME;
		}
	}
	LAString infoname = LACalibrationParametersPtberg::createCalibrationInfo(objPool, fx);
	LAObject &info = objPool.getObject(infoname, ENCHKTYPE_ISDEFINED).get();
	keyFX.toLower();
	///////////////////////////
	// set skew & vol info
	///////////////////////////
	const LAString SKEWVOL = "_SKEWVOL";
	// set intitial vol skew
	const double initialVol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_VOL).getDoubleValue();
	const double initialSkew = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_SKEW).getDoubleValue();
	DoubleVector initialVals(2);
	initialVals[0] = initialVol;
	initialVals[1] = initialSkew;
	info.add(PRICING_DATA_INITIALVALUE, new LADataDoubles(initialVals));
	
	// set boundary vol skew
	const double boundaryMaxVol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_VOL).getDoubleValue();
	const double boundaryMinVol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_VOL).getDoubleValue();
	const double boundaryMaxSkew = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_SKEW).getDoubleValue();
	const double boundaryMinSkew = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_SKEW).getDoubleValue();
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_VOL, new LADataDouble(boundaryMaxVol));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_VOL, new LADataDouble(boundaryMinVol));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_SKEW, new LADataDouble(boundaryMaxSkew));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_SKEW, new LADataDouble(boundaryMinSkew));

	// curve typ
	const LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);
	const LAString curveType_d = staticData.getStaticData(ccys[0].toLower() + STATIC_DATA_KEY_YIELD_DF2);
	if (curveType_d != MLIB_NO_DATA)
	{
		info.add(PRICING_DATA_DOMESTICCURVETYPE, new LADataString(curveType_d));
	}
	else
	{
		info.add(PRICING_DATA_DOMESTICCURVETYPE, new LADataString(STD));
	}
	const LAString curveType_f = staticData.getStaticData(ccys[1].toLower() + STATIC_DATA_KEY_YIELD_DF2);
	if (curveType_f != MLIB_NO_DATA)
	{
		info.add(PRICING_DATA_FOREIGNCURVETYPE, new LADataString(curveType_f));
	}
	else
	{
		info.add(PRICING_DATA_FOREIGNCURVETYPE, new LADataString(STD));
	}
	
	// set other calib param
	const int maxIteration_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAXITERATION).getIntValue();
	const int maxStationaryStateIteration_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAX_STATIONARY_ITERATION).getIntValue();
	const double rootEpsilon_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_ROOT_EPSILON).getDoubleValue();
	const double functionEpsilon_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_FUNCTION_EPSILON).getDoubleValue();
	const double gradientNormEpsilon_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_GRADIENT_NORM_EPSILON).getDoubleValue();
	LAString optMethodType_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_METHOD_TYPE);
	optMethodType_skewvol.toUpper();
	info.add(CALIBRATION_DATA_MAX_ITERATION + SKEWVOL, new LADataInt(maxIteration_skewvol));
	info.add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION + SKEWVOL, new LADataInt(maxStationaryStateIteration_skewvol));
	info.add(CALIBRATION_DATA_ROOT_EPSILON + SKEWVOL, new LADataDouble(rootEpsilon_skewvol));
	info.add(CALIBRATION_DATA_FUNCTION_EPSILON + SKEWVOL, new LADataDouble(functionEpsilon_skewvol));
	info.add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON + SKEWVOL, new LADataDouble(gradientNormEpsilon_skewvol));
	info.add(CALIBRATION_DATA_OPT_METHOD_TYPE + SKEWVOL, new LADataString(optMethodType_skewvol));
	
	// set weight
	const LAStringVector &terms = dynamic_cast<const LADataStrings &>(info.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	unsigned int termSize = terms.size();
	DoubleVector weightATMVec(termSize);   // weight ATM
	DoubleVector weight25DHVec(termSize);  // weight 25Delta High
	DoubleVector weight10DHVec(termSize);  // weight 10Delta High
	DoubleVector weight25DLVec(termSize);  // weight 25Delta Low
	DoubleVector weight10DLVec(termSize);  // weight 10Delta Low
	for (unsigned int i = 0; i < termSize; ++i)
	{
		LAString lterm = terms[i];
		lterm.toLower();
		// weight
		weightATMVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_ATM_WEIGHT, lterm).getDoubleValue();
		weight25DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_25DH_WEIGHT, lterm).getDoubleValue();
		weight10DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_10DH_WEIGHT, lterm).getDoubleValue();
		weight25DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_25DL_WEIGHT, lterm).getDoubleValue();
		weight10DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_10DL_WEIGHT, lterm).getDoubleValue();
	}

	// weight ATM
	info.remove(PRICING_DATA_WEIGHTATM);
	info.add(PRICING_DATA_WEIGHTATM, new LADataDoubles(weightATMVec));
	// weight 25Delta Hight
	info.remove(PRICING_DATA_WEIGHT25DH);
	info.add(PRICING_DATA_WEIGHT25DH, new LADataDoubles(weight25DHVec));
	// weight 10Delta Hight
	info.remove(PRICING_DATA_WEIGHT10DH);
	info.add(PRICING_DATA_WEIGHT10DH, new LADataDoubles(weight10DHVec));
	// weight 25Delta Low
	info.remove(PRICING_DATA_WEIGHT25DL);
	info.add(PRICING_DATA_WEIGHT25DL, new LADataDoubles(weight25DLVec));
	// weight 10Delta Low
	info.remove(PRICING_DATA_WEIGHT10DL);
	info.add(PRICING_DATA_WEIGHT10DL, new LADataDoubles(weight10DLVec));

	///////////////////////////
	// set sigma & beta info
	///////////////////////////
	// initial sigma beta
	const LAString  SIGMABETA = "_SIGMABETA";
	LAString isInitialFRead = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_ISFILEREAD).toUpper();
	if (isInitialFRead != "TRUE")
	{
		const DoubleVector &timegrid = dynamic_cast<const LADataDoubles &>(info.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
		const double initialsigma = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_INITIAL_SIGMA).getDoubleValue();
		const double initialbeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_INITIAL_BETA).getDoubleValue();
		const DoubleVector sigmaVec(timegrid.size(), initialsigma);
		const DoubleVector betaVec(timegrid.size(), initialbeta);
		info.remove(PRICING_DATA_FXVOLGRIDS);
		info.add(PRICING_DATA_FXVOLGRIDS, new LADataDoubles(sigmaVec));
		info.remove(PRICING_DATA_FXBETAGRIDS);
		info.add(PRICING_DATA_FXBETAGRIDS, new LADataDoubles(betaVec));
	}
	// set boundary vol skew
	const double boundaryMaxSigma = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_SIGMA).getDoubleValue();
	const double boundaryMinSigma = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_SIGMA).getDoubleValue();
	const double boundaryMaxBeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_BETA).getDoubleValue();
	const double boundaryMinBeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_BETA).getDoubleValue();
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_SIGMA, new LADataDouble(boundaryMaxSigma));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_SIGMA, new LADataDouble(boundaryMinSigma));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_BETA, new LADataDouble(boundaryMaxBeta));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_BETA, new LADataDouble(boundaryMinBeta));
	
	// set other calib param
	const int maxIteration_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_ITERATION).getIntValue();
	const int maxStationaryStateIteration_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_STATIONARY_ITERATION).getIntValue();
	const int numSmallStep = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_NUM_SMALL_STEP).getIntValue();
	const double rootEpsilon_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_ROOT_EPSILON).getDoubleValue();
	const double functionEpsilon_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_FUNCTION_EPSILON).getDoubleValue();
	const double gradientNormEpsilon_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_GRADIENT_NORM_EPSILON).getDoubleValue();
	LAString optFlagStr = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_TARGET);
	optFlagStr.toUpper();
	bool optFlag = true;
	if (optFlagStr == "FALSE")
	{
		optFlag = false;
	}
	LAString optMethodType_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_METHOD_TYPE);
	optMethodType_sigmabeta.toUpper();

	info.add(CALIBRATION_DATA_MAX_ITERATION + SIGMABETA, new LADataInt(maxIteration_sigmabeta));
	info.add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION + SIGMABETA, new LADataInt(maxStationaryStateIteration_sigmabeta));
	info.add(CALIBRATION_DATA_SMALL_STEPS, new LADataInt(numSmallStep));
	info.add(CALIBRATION_DATA_ROOT_EPSILON + SIGMABETA, new LADataDouble(rootEpsilon_sigmabeta));
	info.add(CALIBRATION_DATA_FUNCTION_EPSILON + SIGMABETA, new LADataDouble(functionEpsilon_sigmabeta));
	info.add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON + SIGMABETA, new LADataDouble(gradientNormEpsilon_sigmabeta));
	info.add(CALIBRATION_DATA_OPT_SIGMABETA_FLAG, new LADataBool(optFlag));
	info.add(CALIBRATION_DATA_OPT_METHOD_TYPE + SIGMABETA, new LADataString(optMethodType_sigmabeta));

	return infoname;
}