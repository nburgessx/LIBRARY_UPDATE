/*! @file
@brief Calibration method create class for hw
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersSZ.cpp
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
#include "LACalibrationParametersSZ.h"
#include "LAStaticData.h"
#include "LADefinitions.h"
#include "LADefinitionsCalibration.h"
#include "LADefinitionsHW.h"
#include "LADefinitionsSZ.h"
#include "LAMarketDataHW.h"
#include "LAMarketData.h"
#include "LAMathJamshidianSwaption.h"
#include "LAMathJamshidianSwaptionByImplyVol.h"
#include "LAMathAntonovFXOption.h"
#include "LAPriceFXDisplacedDiffusionCalibration.h"
#include "LAPriceSZCalibration.h"

using namespace std;

// constructor
/*!
@param funFlg

*/
LACalibrationParametersSZ::LACalibrationParametersSZ(void)
	: LACalibrationParameters()
{
}

// destructor
/*!

*/
LACalibrationParametersSZ::~LACalibrationParametersSZ(void)
{
}



/*!
@brief create  calibration info object

@param[out] objPool
@param[out] fx
@return LAString object name

*/
LAString
LACalibrationParametersSZ::createCalibrationInfo(LAObjectPool &objPool, const LAString &fx)
{
	LAString keyFX = fx;
	const LAString CALIBINFONAME = keyFX.toUpper() + "_SZCalibInfoEntity";
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
	LAString infoname = LACalibrationParametersSZ::createCalibrationInfo2(objPool, fx);
	LAObject &info = objPool.getObject(infoname, ENCHKTYPE_ISDEFINED).get();
	keyFX.toLower();
	
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
	
	// set time grid
	const DoubleVector &timegrid = dynamic_cast<const LADataDoubles &>(info.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
	
	// set initial val - ATM Vol
	const double DiffVolV0 = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_DIFFVOLV0).getDoubleValue();
	info.add(PRICING_DATA_DIFFVOLV0, new LADataDouble(DiffVolV0));
	
	// set calibtation flag for parameters 
	bool IsCalibBeta;
	LAString IsCalibBeta_str = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISCALIBRATEBETA).toUpper();
	if (IsCalibBeta_str == "TRUE") IsCalibBeta = true;
	else if (IsCalibBeta_str == "FALSE") IsCalibBeta = false;
	
	bool IsCalibTheta;
	LAString IsCalibTheta_str = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISCALIBRATETHETA).toUpper();
	if (IsCalibTheta_str == "TRUE") IsCalibTheta = true;
	else if (IsCalibTheta_str == "FALSE") IsCalibTheta = false;

	bool IsCalibKappa;
	LAString IsCalibKappa_str = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISCALIBRATEKAPPA).toUpper();
	if (IsCalibKappa_str == "TRUE") IsCalibKappa = true;
	else if (IsCalibKappa_str == "FALSE") IsCalibKappa = false;

	bool IsCalibEpsilon;
	LAString IsCalibEpsilon_str = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISCALIBRATEEPSILON).toUpper();
	if (IsCalibEpsilon_str == "TRUE") IsCalibEpsilon = true;
	else if (IsCalibEpsilon_str == "FALSE") IsCalibEpsilon = false;

	const double boundaryMaxBeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MAX_BETA).getDoubleValue();
	const double boundaryMinBeta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MIN_BETA).getDoubleValue();
	const double boundaryMaxTheta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MAX_THETA).getDoubleValue();
	const double boundaryMinTheta = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MIN_THETA).getDoubleValue();
	const double boundaryMaxKappa = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MAX_KAPPA).getDoubleValue();
	const double boundaryMinKappa = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MIN_KAPPA).getDoubleValue();
	const double boundaryMaxEpsilon = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MAX_EPSILON).getDoubleValue();
	const double boundaryMinEpsilon = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MIN_EPSILON).getDoubleValue();
	
	info.add(CALIBRATION_DATA_OPT_BETA_FLAG, new LADataBool(IsCalibBeta));
	info.add(CALIBRATION_DATA_OPT_THETA_FLAG, new LADataBool(IsCalibTheta));
	info.add(CALIBRATION_DATA_OPT_KAPPA_FLAG, new LADataBool(IsCalibKappa));
	info.add(CALIBRATION_DATA_OPT_EPSILON_FLAG, new LADataBool(IsCalibEpsilon));

	info.add(CALIBRATION_DATA_BOUNDARY_MAX_BETA, new LADataDouble(boundaryMaxBeta));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_BETA, new LADataDouble(boundaryMinBeta));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_THETA, new LADataDouble(boundaryMaxTheta));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_THETA, new LADataDouble(boundaryMinTheta));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_KAPPA, new LADataDouble(boundaryMaxKappa));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_KAPPA, new LADataDouble(boundaryMinKappa));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_EPSILON, new LADataDouble(boundaryMaxEpsilon));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_EPSILON, new LADataDouble(boundaryMinEpsilon));

	// set other calib param
	const int maxIteration = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MAX_ITERATION).getIntValue();
	const int maxStationaryStateIteration = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MAX_STATIONARY_ITERATION).getIntValue();
	//const int numSmallStep = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_NUM_SMALL_STEP).getIntValue();
	const double rootEpsilon = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ROOT_EPSILON).getDoubleValue();
	const double functionEpsilon = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_FUNCTION_EPSILON).getDoubleValue();
	const double gradientNormEpsilon = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_GRADIENT_NORM_EPSILON).getDoubleValue();
	LAString optFlagStr = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISOPTIMIZE);
	optFlagStr.toUpper();
	bool optFlag = true;
	if (optFlagStr == "FALSE")
	{
		optFlag = false;
	}
	LAString optMethodType = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_METHOD_TYPE);
	optMethodType.toUpper();

	info.add(CALIBRATION_DATA_MAX_ITERATION, new LADataInt(maxIteration));
	info.add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION , new LADataInt(maxStationaryStateIteration));
	//info.add(CALIBRATION_DATA_SMALL_STEPS, new LADataInt(numSmallStep));
	info.add(CALIBRATION_DATA_ROOT_EPSILON , new LADataDouble(rootEpsilon));
	info.add(CALIBRATION_DATA_FUNCTION_EPSILON , new LADataDouble(functionEpsilon));
	info.add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON , new LADataDouble(gradientNormEpsilon));
	info.add(CALIBRATION_DATA_OPT_PARAMETER_FLAG, new LADataBool(optFlag));
	info.add(CALIBRATION_DATA_OPT_METHOD_TYPE , new LADataString(optMethodType));

	return infoname;
}

LAString
LACalibrationParametersSZ::createCalibrationInfo2(LAObjectPool &objPool, const LAString &fx)
{
	LAString keyFX = fx;
	const LAString CALIBINFONAME = keyFX.toUpper() + "_SZCalibInfoEntity";
	LAObject *info = 0;
	LAObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		LAString isReset = mpCalibStaticData->getStaticData(KEY_CALIB_INFO_ISRESET).toUpper();
		if (isReset == "TRUE")
		{
			info = &objHolder.get();
			info->reset();
		}
		else
		{
			// do nothing
			return CALIBINFONAME;
		}
	}
	else
	{
		info = new LAObject;
		objPool.set(CALIBINFONAME, info);
	}
	// set name
	info->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(CALIBINFONAME);

	keyFX.toLower();
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create time grid
	DoubleVector timeGrid;
	int maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	LAString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	LAPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	// create tenor SA
	LAMarketDataHW::getCanonicalGrid(timeGrid, asOfDate, dayCount, maxTerm);
	// get initial calib param
	DoubleVector volVec(timeGrid.size());

	// set fx timegrid
	info->add(PRICING_DATA_FXTIMEGRIDS, new LADataDoubles(timeGrid));
	// set fx vol grid
	info->add(PRICING_DATA_FXVOLGRIDS, new LADataDoubles(volVec));

	// set spot fx and spotdate
	LAMathFXEntity fxEntity(info->getDataInstance());
	fxEntity.getFXType() = "FIXEDRATE";
	LAMarketData::setUpMarket2FXEntity(fxEntity);
	LAStringVector ccys;
	LAMarketData::convertToCurrency(keyFX, ccys);
	const double spotRate = fxEntity.getRate(ccys[1], ccys[0], 0.0);
	const LADate spotDate = fxEntity.getSpotDate(ccys[0], ccys[1], asOfDate);
	info->add(PRICING_DATA_SPOTFX, new LADataDouble(spotRate));
	info->add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDate));

	// set strike fwd term
	LAStringVector strikeFwdVec = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_STRIKEFWDTERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(PRICING_DATA_STRIKEFWDTERM, new LADataStrings(strikeFwdVec));
	// set calib variable
	const double calibVari = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_CALIBVARIABLE).getDoubleValue();
	DoubleVector calibVariVec(2, calibVari);
	info->add(PRICING_DATA_CALIBVARIABLES, new LADataDoubles(calibVariVec));
	// set calib param
	LAString calibParam = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_CALIBPARAM);
	info->add(PRICING_DATA_CALIBPARAM, new LADataString(calibParam));
	// set calib input type
	LAString inputType = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_INPUTPARAMTYPE);
	info->add(PRICING_DATA_CALIBINPUTPARAMTYPE, new LADataString(inputType));
	// terms calendar
	LAString strTenorCal = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_CALENDAR);
	info->add(PRICING_DATA_TERMCALENDAR, new LAPriceDataCalendar()).convertFromString(strTenorCal);
	// terms slidingrule
	LAString strTenorSliding = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_SLIDINGRULE).toUpper();
	info->add(PRICING_DATA_TERMSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(strTenorSliding);
	// terms spotlag
	LAString strTenorSpotLag = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_SPOTLAG);
	info->add(PRICING_DATA_TERMSPOTLAG, new LADataInt()).convertFromString(strTenorSpotLag);
	// terms daycount
	LAString strTenorDayCount = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_DAYCOUNT).toUpper();
	info->add(PRICING_DATA_TERMDAYCOUNT, new LAPriceDataDayCount()).convertFromString(strTenorDayCount);
	// terms isincludelast
	LAString strIsIncludeLast = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_ISINCLUDELAST).toUpper();
	info->add(PRICING_DATA_TERMISINCLUDELAST, new LADataBool()).convertFromString(strIsIncludeLast);
	// create market param
	// terms
	LAStringVector terms = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new LADataStrings(terms));
	unsigned int termSize = terms.size();
	// get fx vol file
	LAString volFile = LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VOLATILITY_FILE));

	LAStringVector calVec(termSize);         // fixingcalendar
	LAStringVector slidingVec(termSize);     // slidingrule
	LAStringVector soptOrFwdVec(termSize);   // strike

	LAStringVector opTypeATMVec(termSize);     // optiontype ATM
	LAStringVector opType25DHVec(termSize);    // optiontype 25Delta High
	LAStringVector opType10DHVec(termSize);    // optiontype 10Delta High
	LAStringVector opType25DLVec(termSize);    // optiontype 25Delta Low
	LAStringVector opType10DLVec(termSize);    // optiontype 10Delta Low

	DoubleVector weightATMVec(termSize);   // weight ATM
	DoubleVector weight25DHVec(termSize);  // weight 25Delta High
	DoubleVector weight10DHVec(termSize);  // weight 10Delta High
	DoubleVector weight25DLVec(termSize);  // weight 25Delta Low
	DoubleVector weight10DLVec(termSize);  // weight 10Delta Low

	DoubleVector volATMVec(termSize);      // vol ATM
	DoubleVector vol25DHVec(termSize);     // vol 25Delta High
	DoubleVector vol10DHVec(termSize);     // vol 10Delta High
	DoubleVector vol25DLVec(termSize);     // vol 25Delta Low
	DoubleVector vol10DLVec(termSize);     // vol 10Delta Low

										   // TODO merge latest version
	DoubleVector strike25DHVec(termSize, 0.25);     // strike 25Delta High
	DoubleVector strike10DHVec(termSize, 0.1);     // strike 10Delta High
	DoubleVector strike25DLVec(termSize, 0.25);     // strike 25Delta Low
	DoubleVector strike10DLVec(termSize, 0.1);     // strike 10Delta Low

	DoubleVector BetaVec(termSize);        // beta vec
	DoubleVector ThetaVec(termSize);        // theta vec
	DoubleVector KappaVec(termSize);        // kappa vec
	DoubleVector EpsilonVec(termSize);        // epsilon vec

	BoolVector isFitATMVec(termSize);		// is fit atm
	BoolVector isDeltaNeutVec(termSize);		// is delta neutral

	LADataBool tmpBool;
	for (unsigned int i = 0; i < termSize; ++i)
	{
		LAString lterm = terms[i];
		lterm.toLower();
		calVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_CALENDAR, lterm);
		slidingVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_SLIDINGRULE, lterm).toUpper();
		soptOrFwdVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_SPOTORFWD, lterm).toUpper();
		// optiontype
		opTypeATMVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_OPTIONTYPE_ATM, lterm).toUpper();
		opType25DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_OPTIONTYPE_25DH, lterm).toUpper();
		opType10DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_OPTIONTYPE_10DH, lterm).toUpper();
		opType25DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_OPTIONTYPE_25DL, lterm).toUpper();
		opType10DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_OPTIONTYPE_10DL, lterm).toUpper();
		// weight
		weightATMVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_WEIGHT_ATM, lterm).getDoubleValue();
		weight25DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_WEIGHT_25DH, lterm).getDoubleValue();
		weight10DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_WEIGHT_10DH, lterm).getDoubleValue();
		weight25DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_WEIGHT_25DL, lterm).getDoubleValue();
		weight10DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_WEIGHT_10DL, lterm).getDoubleValue();
		// beta
		//tBetaVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_TERMBETA, lterm).getDoubleValue();
		//initial patameters
		BetaVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_INITIALBETA, lterm).getDoubleValue();
		ThetaVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_INITIALTHETA, lterm).getDoubleValue();
		KappaVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_INITIALKAPPA, lterm).getDoubleValue();
		EpsilonVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_INITIALEPSILON, lterm).getDoubleValue();
		// is fit atm?
		tmpBool.convertFromString(getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_ISFITATM, lterm));
		isFitATMVec[i] = tmpBool.get();
		// is delta neutral?
		tmpBool.convertFromString(getCalibStaticDataValue(keyFX + FX_KEY_CALIB_SZ_ISDELTANEUTRAL, lterm));
		isDeltaNeutVec[i] = tmpBool.get();
		// volatility val
		volATMVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_ATM_VOL);
		vol25DHVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_25DHIGH_VOL);
		vol10DHVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_10DHIGH_VOL);
		vol25DLVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_25DLOW_VOL);
		vol10DLVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_10DLOW_VOL);
	}
	// set market param
	// fixingcalendar
	info->add(PRICING_DATA_FIXINGCALENDAR, new LADataStrings(calVec));
	// slidingrule
	info->add(CALIBRATION_DATA_SLIDINGRULE, new LADataStrings(slidingVec));
	// spot or fwd
	info->add(PRICING_DATA_SPOTORFWD, new LADataStrings(soptOrFwdVec));

	// optiontype ATM
	info->add(PRICING_DATA_OPTIONTYPEATM, new LADataStrings(opTypeATMVec));
	// optiontype 25Delta Hight
	info->add(PRICING_DATA_OPTIONTYPE25DH, new LADataStrings(opType25DHVec));
	// optiontype 10Delta Hight
	info->add(PRICING_DATA_OPTIONTYPE10DH, new LADataStrings(opType10DHVec));
	// optiontype 25Delta Low
	info->add(PRICING_DATA_OPTIONTYPE25DL, new LADataStrings(opType25DLVec));
	// optiontype 10Delta Low
	info->add(PRICING_DATA_OPTIONTYPE10DL, new LADataStrings(opType10DLVec));

	// weight ATM
	info->add(PRICING_DATA_WEIGHTATM, new LADataDoubles(weightATMVec));
	// weight 25Delta Hight
	info->add(PRICING_DATA_WEIGHT25DH, new LADataDoubles(weight25DHVec));
	// weight 10Delta Hight
	info->add(PRICING_DATA_WEIGHT10DH, new LADataDoubles(weight10DHVec));
	// weight 25Delta Low
	info->add(PRICING_DATA_WEIGHT25DL, new LADataDoubles(weight25DLVec));
	// weight 10Delta Low
	info->add(PRICING_DATA_WEIGHT10DL, new LADataDoubles(weight10DLVec));

	// vol ATM
	info->add(PRICING_DATA_FXVOLATM, new LADataDoubles(volATMVec));
	// vol 25Delta Hight
	info->add(PRICING_DATA_FXVOL25DH, new LADataDoubles(vol25DHVec));
	// vol 10Delta Hight
	info->add(PRICING_DATA_FXVOL10DH, new LADataDoubles(vol10DHVec));
	// vol 25Delta Low
	info->add(PRICING_DATA_FXVOL25DL, new LADataDoubles(vol25DLVec));
	// vol 10Delta Low
	info->add(PRICING_DATA_FXVOL10DL, new LADataDoubles(vol10DLVec));

	// TODO merget latest version strike
	// at merge delete literal
	// strike 25Delta Hight
	info->add(PRICING_DATA_STRIKEVALES + LAString("25DH"), new LADataDoubles(strike25DHVec));
	// strike 10Delta Hight
	info->add(PRICING_DATA_STRIKEVALES + LAString("10DH"), new LADataDoubles(strike10DHVec));
	// strike 25Delta Low
	info->add(PRICING_DATA_STRIKEVALES + LAString("25DL"), new LADataDoubles(strike25DLVec));
	// strike 10Delta Low
	info->add(PRICING_DATA_STRIKEVALES + LAString("10DL"), new LADataDoubles(strike10DLVec));

	//is fit atm
	info->add(PRICING_DATA_ISFITATM, new LADataBools(isFitATMVec));
	//is delta neutral
	info->add(PRICING_DATA_ISDELTANEUTRAL, new LADataBools(isDeltaNeutVec));
	// beta
	//info->add(PRICING_DATA_TERMBETA, new LADataDoubles(tBetaVec));
	//parameters
	info->add(PRICING_DATA_INITIALBETA, new LADataDoubles(BetaVec));
	info->add(PRICING_DATA_INITIALTHETA, new LADataDoubles(ThetaVec));
	info->add(PRICING_DATA_INITIALKAPPA, new LADataDoubles(KappaVec));
	info->add(PRICING_DATA_INITIALEPSILON, new LADataDoubles(EpsilonVec));

	return CALIBINFONAME;

}


/*!
@brief get calibration property value

@param[in] key
@param[in] grid
@return LAString value

*/
LAString
LACalibrationParametersSZ::getCalibStaticDataValue(const LAString &key, const LAString &grid)
{
	LAString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == MLIB_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}

	return ret;
}

