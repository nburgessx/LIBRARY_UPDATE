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


#include "AQLString.h"
#include "AQLObjectPool.h"
#include "AQLObject.h"
#include "AQLPriceDataType.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLMathHWFuncMR.h"
#include "AQLMathDefine.h"
#include "LADealUtils.h"
#include "LACalibrationParametersSZ.h"
#include "LAStaticData.h"
#include "LADefinitions.h"
#include "LADefinitionsCalibration.h"
#include "LADefinitionsHW.h"
#include "LADefinitionsSZ.h"
#include "LAMarketDataHW.h"
#include "LAMarketData.h"
#include "AQLMathJamshidianSwaption.h"
#include "AQLMathJamshidianSwaptionByImplyVol.h"
#include "AQLMathAntonovFXOption.h"
#include "AQLPriceFXDisplacedDiffusionCalibration.h"
#include "AQLPriceSZCalibration.h"

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
@return AQLString object name

*/
AQLString
LACalibrationParametersSZ::createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx)
{
	AQLString keyFX = fx;
	const AQLString CALIBINFONAME = keyFX.toUpper() + "_SZCalibInfoEntity";
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
	AQLString infoname = LACalibrationParametersSZ::createCalibrationInfo2(objPool, fx);
	AQLObject &info = objPool.getObject(infoname, ENCHKTYPE_ISDEFINED).get();
	keyFX.toLower();
	
	// curve typ
	const LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);
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
	
	// set time grid
	const DoubleVector &timegrid = dynamic_cast<const AQLDataDoubles &>(info.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
	
	// set initial val - ATM Vol
	const double DiffVolV0 = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_DIFFVOLV0).getDoubleValue();
	info.add(PRICING_DATA_DIFFVOLV0, new AQLDataDouble(DiffVolV0));
	
	// set calibtation flag for parameters 
	bool IsCalibBeta;
	AQLString IsCalibBeta_str = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISCALIBRATEBETA).toUpper();
	if (IsCalibBeta_str == "TRUE") IsCalibBeta = true;
	else if (IsCalibBeta_str == "FALSE") IsCalibBeta = false;
	
	bool IsCalibTheta;
	AQLString IsCalibTheta_str = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISCALIBRATETHETA).toUpper();
	if (IsCalibTheta_str == "TRUE") IsCalibTheta = true;
	else if (IsCalibTheta_str == "FALSE") IsCalibTheta = false;

	bool IsCalibKappa;
	AQLString IsCalibKappa_str = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISCALIBRATEKAPPA).toUpper();
	if (IsCalibKappa_str == "TRUE") IsCalibKappa = true;
	else if (IsCalibKappa_str == "FALSE") IsCalibKappa = false;

	bool IsCalibEpsilon;
	AQLString IsCalibEpsilon_str = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISCALIBRATEEPSILON).toUpper();
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
	
	info.add(CALIBRATION_DATA_OPT_BETA_FLAG, new AQLDataBool(IsCalibBeta));
	info.add(CALIBRATION_DATA_OPT_THETA_FLAG, new AQLDataBool(IsCalibTheta));
	info.add(CALIBRATION_DATA_OPT_KAPPA_FLAG, new AQLDataBool(IsCalibKappa));
	info.add(CALIBRATION_DATA_OPT_EPSILON_FLAG, new AQLDataBool(IsCalibEpsilon));

	info.add(CALIBRATION_DATA_BOUNDARY_MAX_BETA, new AQLDataDouble(boundaryMaxBeta));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_BETA, new AQLDataDouble(boundaryMinBeta));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_THETA, new AQLDataDouble(boundaryMaxTheta));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_THETA, new AQLDataDouble(boundaryMinTheta));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_KAPPA, new AQLDataDouble(boundaryMaxKappa));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_KAPPA, new AQLDataDouble(boundaryMinKappa));
	info.add(CALIBRATION_DATA_BOUNDARY_MAX_EPSILON, new AQLDataDouble(boundaryMaxEpsilon));
	info.add(CALIBRATION_DATA_BOUNDARY_MIN_EPSILON, new AQLDataDouble(boundaryMinEpsilon));

	// set other calib param
	const int maxIteration = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MAX_ITERATION).getIntValue();
	const int maxStationaryStateIteration = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_MAX_STATIONARY_ITERATION).getIntValue();
	//const int numSmallStep = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_NUM_SMALL_STEP).getIntValue();
	const double rootEpsilon = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ROOT_EPSILON).getDoubleValue();
	const double functionEpsilon = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_FUNCTION_EPSILON).getDoubleValue();
	const double gradientNormEpsilon = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_GRADIENT_NORM_EPSILON).getDoubleValue();
	AQLString optFlagStr = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_ISOPTIMIZE);
	optFlagStr.toUpper();
	bool optFlag = true;
	if (optFlagStr == "FALSE")
	{
		optFlag = false;
	}
	AQLString optMethodType = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_OPT_METHOD_TYPE);
	optMethodType.toUpper();

	info.add(CALIBRATION_DATA_MAX_ITERATION, new AQLDataInt(maxIteration));
	info.add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION , new AQLDataInt(maxStationaryStateIteration));
	//info.add(CALIBRATION_DATA_SMALL_STEPS, new AQLDataInt(numSmallStep));
	info.add(CALIBRATION_DATA_ROOT_EPSILON , new AQLDataDouble(rootEpsilon));
	info.add(CALIBRATION_DATA_FUNCTION_EPSILON , new AQLDataDouble(functionEpsilon));
	info.add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON , new AQLDataDouble(gradientNormEpsilon));
	info.add(CALIBRATION_DATA_OPT_PARAMETER_FLAG, new AQLDataBool(optFlag));
	info.add(CALIBRATION_DATA_OPT_METHOD_TYPE , new AQLDataString(optMethodType));

	return infoname;
}

AQLString
LACalibrationParametersSZ::createCalibrationInfo2(AQLObjectPool &objPool, const AQLString &fx)
{
	AQLString keyFX = fx;
	const AQLString CALIBINFONAME = keyFX.toUpper() + "_SZCalibInfoEntity";
	AQLObject *info = 0;
	AQLObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		AQLString isReset = mpCalibStaticData->getStaticData(KEY_CALIB_INFO_ISRESET).toUpper();
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
		info = new AQLObject;
		objPool.set(CALIBINFONAME, info);
	}
	// set name
	info->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(CALIBINFONAME);

	keyFX.toLower();
	AQLDate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create time grid
	DoubleVector timeGrid;
	int maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	// create tenor SA
	LAMarketDataHW::getCanonicalGrid(timeGrid, asOfDate, dayCount, maxTerm);
	// get initial calib param
	DoubleVector volVec(timeGrid.size());

	// set fx timegrid
	info->add(PRICING_DATA_FXTIMEGRIDS, new AQLDataDoubles(timeGrid));
	// set fx vol grid
	info->add(PRICING_DATA_FXVOLGRIDS, new AQLDataDoubles(volVec));

	// set spot fx and spotdate
	AQLMathFXEntity fxEntity(info->getDataInstance());
	fxEntity.getFXType() = "FIXEDRATE";
	LAMarketData::setUpMarket2FXEntity(fxEntity);
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(keyFX, ccys);
	const double spotRate = fxEntity.getRate(ccys[1], ccys[0], 0.0);
	const AQLDate spotDate = fxEntity.getSpotDate(ccys[0], ccys[1], asOfDate);
	info->add(PRICING_DATA_SPOTFX, new AQLDataDouble(spotRate));
	info->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDate));

	// set strike fwd term
	AQLStringVector strikeFwdVec = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_STRIKEFWDTERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(PRICING_DATA_STRIKEFWDTERM, new AQLDataStrings(strikeFwdVec));
	// set calib variable
	const double calibVari = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_CALIBVARIABLE).getDoubleValue();
	DoubleVector calibVariVec(2, calibVari);
	info->add(PRICING_DATA_CALIBVARIABLES, new AQLDataDoubles(calibVariVec));
	// set calib param
	AQLString calibParam = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_CALIBPARAM);
	info->add(PRICING_DATA_CALIBPARAM, new AQLDataString(calibParam));
	// set calib input type
	AQLString inputType = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_INPUTPARAMTYPE);
	info->add(PRICING_DATA_CALIBINPUTPARAMTYPE, new AQLDataString(inputType));
	// terms calendar
	AQLString strTenorCal = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_CALENDAR);
	info->add(PRICING_DATA_TERMCALENDAR, new AQLPriceDataCalendar()).convertFromString(strTenorCal);
	// terms slidingrule
	AQLString strTenorSliding = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_SLIDINGRULE).toUpper();
	info->add(PRICING_DATA_TERMSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(strTenorSliding);
	// terms spotlag
	AQLString strTenorSpotLag = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_SPOTLAG);
	info->add(PRICING_DATA_TERMSPOTLAG, new AQLDataInt()).convertFromString(strTenorSpotLag);
	// terms daycount
	AQLString strTenorDayCount = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_DAYCOUNT).toUpper();
	info->add(PRICING_DATA_TERMDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(strTenorDayCount);
	// terms isincludelast
	AQLString strIsIncludeLast = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM_ISINCLUDELAST).toUpper();
	info->add(PRICING_DATA_TERMISINCLUDELAST, new AQLDataBool()).convertFromString(strIsIncludeLast);
	// create market param
	// terms
	AQLStringVector terms = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_SZ_TERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new AQLDataStrings(terms));
	unsigned int termSize = terms.size();
	// get fx vol file
	AQLString volFile = LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VOLATILITY_FILE));

	AQLStringVector calVec(termSize);         // fixingcalendar
	AQLStringVector slidingVec(termSize);     // slidingrule
	AQLStringVector soptOrFwdVec(termSize);   // strike

	AQLStringVector opTypeATMVec(termSize);     // optiontype ATM
	AQLStringVector opType25DHVec(termSize);    // optiontype 25Delta High
	AQLStringVector opType10DHVec(termSize);    // optiontype 10Delta High
	AQLStringVector opType25DLVec(termSize);    // optiontype 25Delta Low
	AQLStringVector opType10DLVec(termSize);    // optiontype 10Delta Low

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

	AQLDataBool tmpBool;
	for (unsigned int i = 0; i < termSize; ++i)
	{
		AQLString lterm = terms[i];
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
	info->add(PRICING_DATA_FIXINGCALENDAR, new AQLDataStrings(calVec));
	// slidingrule
	info->add(CALIBRATION_DATA_SLIDINGRULE, new AQLDataStrings(slidingVec));
	// spot or fwd
	info->add(PRICING_DATA_SPOTORFWD, new AQLDataStrings(soptOrFwdVec));

	// optiontype ATM
	info->add(PRICING_DATA_OPTIONTYPEATM, new AQLDataStrings(opTypeATMVec));
	// optiontype 25Delta Hight
	info->add(PRICING_DATA_OPTIONTYPE25DH, new AQLDataStrings(opType25DHVec));
	// optiontype 10Delta Hight
	info->add(PRICING_DATA_OPTIONTYPE10DH, new AQLDataStrings(opType10DHVec));
	// optiontype 25Delta Low
	info->add(PRICING_DATA_OPTIONTYPE25DL, new AQLDataStrings(opType25DLVec));
	// optiontype 10Delta Low
	info->add(PRICING_DATA_OPTIONTYPE10DL, new AQLDataStrings(opType10DLVec));

	// weight ATM
	info->add(PRICING_DATA_WEIGHTATM, new AQLDataDoubles(weightATMVec));
	// weight 25Delta Hight
	info->add(PRICING_DATA_WEIGHT25DH, new AQLDataDoubles(weight25DHVec));
	// weight 10Delta Hight
	info->add(PRICING_DATA_WEIGHT10DH, new AQLDataDoubles(weight10DHVec));
	// weight 25Delta Low
	info->add(PRICING_DATA_WEIGHT25DL, new AQLDataDoubles(weight25DLVec));
	// weight 10Delta Low
	info->add(PRICING_DATA_WEIGHT10DL, new AQLDataDoubles(weight10DLVec));

	// vol ATM
	info->add(PRICING_DATA_FXVOLATM, new AQLDataDoubles(volATMVec));
	// vol 25Delta Hight
	info->add(PRICING_DATA_FXVOL25DH, new AQLDataDoubles(vol25DHVec));
	// vol 10Delta Hight
	info->add(PRICING_DATA_FXVOL10DH, new AQLDataDoubles(vol10DHVec));
	// vol 25Delta Low
	info->add(PRICING_DATA_FXVOL25DL, new AQLDataDoubles(vol25DLVec));
	// vol 10Delta Low
	info->add(PRICING_DATA_FXVOL10DL, new AQLDataDoubles(vol10DLVec));

	// TODO merget latest version strike
	// at merge delete literal
	// strike 25Delta Hight
	info->add(PRICING_DATA_STRIKEVALES + AQLString("25DH"), new AQLDataDoubles(strike25DHVec));
	// strike 10Delta Hight
	info->add(PRICING_DATA_STRIKEVALES + AQLString("10DH"), new AQLDataDoubles(strike10DHVec));
	// strike 25Delta Low
	info->add(PRICING_DATA_STRIKEVALES + AQLString("25DL"), new AQLDataDoubles(strike25DLVec));
	// strike 10Delta Low
	info->add(PRICING_DATA_STRIKEVALES + AQLString("10DL"), new AQLDataDoubles(strike10DLVec));

	//is fit atm
	info->add(PRICING_DATA_ISFITATM, new AQLDataBools(isFitATMVec));
	//is delta neutral
	info->add(PRICING_DATA_ISDELTANEUTRAL, new AQLDataBools(isDeltaNeutVec));
	// beta
	//info->add(PRICING_DATA_TERMBETA, new AQLDataDoubles(tBetaVec));
	//parameters
	info->add(PRICING_DATA_INITIALBETA, new AQLDataDoubles(BetaVec));
	info->add(PRICING_DATA_INITIALTHETA, new AQLDataDoubles(ThetaVec));
	info->add(PRICING_DATA_INITIALKAPPA, new AQLDataDoubles(KappaVec));
	info->add(PRICING_DATA_INITIALEPSILON, new AQLDataDoubles(EpsilonVec));

	return CALIBINFONAME;

}


/*!
@brief get calibration property value

@param[in] key
@param[in] grid
@return AQLString value

*/
AQLString
LACalibrationParametersSZ::getCalibStaticDataValue(const AQLString &key, const AQLString &grid)
{
	AQLString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == AQ_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}

	return ret;
}

