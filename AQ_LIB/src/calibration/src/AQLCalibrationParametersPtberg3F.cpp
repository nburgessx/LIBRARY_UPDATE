/*! @file
    @brief Calibration method create class for hw
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
#include "AQLMathHWFuncMR.h"
#include "AQLMathDefine.h"
#include "AQLDealUtils.h"
#include "AQLCalibrationParametersPtberg3F.h"
#include "AQLStaticData.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLDefinitionsHW.h"
#include "AQLDefinitionsPtberg.h"
#include "AQLMarketDataHW.h"
#include "AQLMarketData.h"
#include "AQLMathJamshidianSwaption.h"
#include "AQLMathJamshidianSwaptionByImplyVol.h"
#include "AQLMathAntonovFXOption.h"
#include "AQLPriceFXDisplacedDiffusionCalibration3F.h"

using namespace std;

// constructor
/*!
	@param funFlg

*/
AQLCalibrationParametersPtberg3F::AQLCalibrationParametersPtberg3F(void)
: AQLCalibrationParametersPtberg()
{
}

// destructor
/*!

*/
AQLCalibrationParametersPtberg3F::~AQLCalibrationParametersPtberg3F(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] fx
	@return AQLString object name

*/
AQLString 
AQLCalibrationParametersPtberg3F::createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx)
{
	AQLString keyFX = fx;
	const AQLString CALIBINFONAME = keyFX.toUpper() + "_PtbergCalibInfoEntity";
	AQLObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		AQLString isReset = mpCalibStaticData->getStaticData(KEY_CALIB_INFO_ISRESET).toUpper();
		if (isReset != "TRUE")
		{
			// do nothing
			return CALIBINFONAME;
		}
	}
	AQLString infoname = AQLCalibrationParametersPtberg::createCalibrationInfo(objPool, fx);
	AQLObject &info = objPool.getObject(infoname, ENCHKTYPE_ISDEFINED).get();
	keyFX.toLower();
	///////////////////////////
	// set skew & vol info
	///////////////////////////
	const AQLString SKEWVOL = "_SKEWVOL";
	// set intitial vol skew
	const double initialVol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_VOL).getDoubleValue();
	const double initialSkew = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_SKEW).getDoubleValue();
	DoubleVector initialVals(2);
	initialVals[0] = initialVol;
	initialVals[1] = initialSkew;
	info.add(PRICING_DATA_INITIALVALUE, new AQLDataDoubles(initialVals));
	
	// set boundary vol skew
	const double boundaryMaxVol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_VOL).getDoubleValue();
	const double boundaryMinVol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_VOL).getDoubleValue();
	const double boundaryMaxSkew = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_SKEW).getDoubleValue();
	const double boundaryMinSkew = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_SKEW).getDoubleValue();
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_VOL, new AQLDataDouble(boundaryMaxVol));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_VOL, new AQLDataDouble(boundaryMinVol));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_SKEW, new AQLDataDouble(boundaryMaxSkew));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_SKEW, new AQLDataDouble(boundaryMinSkew));

	// curve typ
	const AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(fx, ccys);
	const AQLString curveType_d = staticData.getStaticData(ccys[0].toLower() + STATIC_DATA_KEY_YIELD_DF2);
	if (curveType_d != AQ_NO_DATA)
	{
		info.add(PRICING_DATA_DOMESTICCURVETYPE, new AQLDataString(curveType_d));
	}
	else
	{
		info.add(PRICING_DATA_DOMESTICCURVETYPE, new AQLDataString(STD));
	}
	const AQLString curveType_f = staticData.getStaticData(ccys[1].toLower() + STATIC_DATA_KEY_YIELD_DF2);
	if (curveType_f != AQ_NO_DATA)
	{
		info.add(PRICING_DATA_FOREIGNCURVETYPE, new AQLDataString(curveType_f));
	}
	else
	{
		info.add(PRICING_DATA_FOREIGNCURVETYPE, new AQLDataString(STD));
	}
	
	// set other calib param
	const int maxIteration_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAXITERATION).getIntValue();
	const int maxStationaryStateIteration_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAX_STATIONARY_ITERATION).getIntValue();
	const double rootEpsilon_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_ROOT_EPSILON).getDoubleValue();
	const double functionEpsilon_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_FUNCTION_EPSILON).getDoubleValue();
	const double gradientNormEpsilon_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_GRADIENT_NORM_EPSILON).getDoubleValue();
	AQLString optMethodType_skewvol = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_METHOD_TYPE);
	optMethodType_skewvol.toUpper();
	info.add(CALIBRATION_DATA_MAX_ITERATION + SKEWVOL, new AQLDataInt(maxIteration_skewvol));
	info.add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION + SKEWVOL, new AQLDataInt(maxStationaryStateIteration_skewvol));
	info.add(CALIBRATION_DATA_ROOT_EPSILON + SKEWVOL, new AQLDataDouble(rootEpsilon_skewvol));
	info.add(CALIBRATION_DATA_FUNCTION_EPSILON + SKEWVOL, new AQLDataDouble(functionEpsilon_skewvol));
	info.add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON + SKEWVOL, new AQLDataDouble(gradientNormEpsilon_skewvol));
	info.add(CALIBRATION_DATA_OPT_METHOD_TYPE + SKEWVOL, new AQLDataString(optMethodType_skewvol));
	
	// set weight
	const AQLStringVector &terms = dynamic_cast<const AQLDataStrings &>(info.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	unsigned int termSize = terms.size();
	DoubleVector weightATMVec(termSize);   // weight ATM
	DoubleVector weight25DHVec(termSize);  // weight 25Delta High
	DoubleVector weight10DHVec(termSize);  // weight 10Delta High
	DoubleVector weight25DLVec(termSize);  // weight 25Delta Low
	DoubleVector weight10DLVec(termSize);  // weight 10Delta Low
	for (unsigned int i = 0; i < termSize; ++i)
	{
		AQLString lterm = terms[i];
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
	info.add(PRICING_DATA_WEIGHTATM, new AQLDataDoubles(weightATMVec));
	// weight 25Delta Hight
	info.remove(PRICING_DATA_WEIGHT25DH);
	info.add(PRICING_DATA_WEIGHT25DH, new AQLDataDoubles(weight25DHVec));
	// weight 10Delta Hight
	info.remove(PRICING_DATA_WEIGHT10DH);
	info.add(PRICING_DATA_WEIGHT10DH, new AQLDataDoubles(weight10DHVec));
	// weight 25Delta Low
	info.remove(PRICING_DATA_WEIGHT25DL);
	info.add(PRICING_DATA_WEIGHT25DL, new AQLDataDoubles(weight25DLVec));
	// weight 10Delta Low
	info.remove(PRICING_DATA_WEIGHT10DL);
	info.add(PRICING_DATA_WEIGHT10DL, new AQLDataDoubles(weight10DLVec));

	///////////////////////////
	// set sigma & beta info
	///////////////////////////
	// initial sigma beta
	const AQLString  SIGMABETA = "_SIGMABETA";
	AQLString isInitialFRead = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_ISFILEREAD).toUpper();
	if (isInitialFRead != "TRUE")
	{
		const DoubleVector &timegrid = dynamic_cast<const AQLDataDoubles &>(info.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
		const double initialsigma = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_INITIAL_SIGMA).getDoubleValue();
		const double initialbeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_INITIAL_BETA).getDoubleValue();
		const DoubleVector sigmaVec(timegrid.size(), initialsigma);
		const DoubleVector betaVec(timegrid.size(), initialbeta);
		info.remove(PRICING_DATA_FXVOLGRIDS);
		info.add(PRICING_DATA_FXVOLGRIDS, new AQLDataDoubles(sigmaVec));
		info.remove(PRICING_DATA_FXBETAGRIDS);
		info.add(PRICING_DATA_FXBETAGRIDS, new AQLDataDoubles(betaVec));
	}
	// set boundary vol skew
	const double boundaryMaxSigma = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_SIGMA).getDoubleValue();
	const double boundaryMinSigma = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_SIGMA).getDoubleValue();
	const double boundaryMaxBeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_BETA).getDoubleValue();
	const double boundaryMinBeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_BETA).getDoubleValue();
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_SIGMA, new AQLDataDouble(boundaryMaxSigma));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_SIGMA, new AQLDataDouble(boundaryMinSigma));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_BETA, new AQLDataDouble(boundaryMaxBeta));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_BETA, new AQLDataDouble(boundaryMinBeta));
	
	// set other calib param
	const int maxIteration_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_ITERATION).getIntValue();
	const int maxStationaryStateIteration_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_STATIONARY_ITERATION).getIntValue();
	const int numSmallStep = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_NUM_SMALL_STEP).getIntValue();
	const double rootEpsilon_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_ROOT_EPSILON).getDoubleValue();
	const double functionEpsilon_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_FUNCTION_EPSILON).getDoubleValue();
	const double gradientNormEpsilon_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_GRADIENT_NORM_EPSILON).getDoubleValue();
	AQLString optFlagStr = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_TARGET);
	optFlagStr.toUpper();
	bool optFlag = true;
	if (optFlagStr == "FALSE")
	{
		optFlag = false;
	}
	AQLString optMethodType_sigmabeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_METHOD_TYPE);
	optMethodType_sigmabeta.toUpper();

	info.add(CALIBRATION_DATA_MAX_ITERATION + SIGMABETA, new AQLDataInt(maxIteration_sigmabeta));
	info.add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION + SIGMABETA, new AQLDataInt(maxStationaryStateIteration_sigmabeta));
	info.add(CALIBRATION_DATA_SMALL_STEPS, new AQLDataInt(numSmallStep));
	info.add(CALIBRATION_DATA_ROOT_EPSILON + SIGMABETA, new AQLDataDouble(rootEpsilon_sigmabeta));
	info.add(CALIBRATION_DATA_FUNCTION_EPSILON + SIGMABETA, new AQLDataDouble(functionEpsilon_sigmabeta));
	info.add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON + SIGMABETA, new AQLDataDouble(gradientNormEpsilon_sigmabeta));
	info.add(CALIBRATION_DATA_OPT_SIGMABETA_FLAG, new AQLDataBool(optFlag));
	info.add(CALIBRATION_DATA_OPT_METHOD_TYPE + SIGMABETA, new AQLDataString(optMethodType_sigmabeta));

	return infoname;
}