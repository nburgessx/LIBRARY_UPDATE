/*! @file
    @brief Calibration method create class for hw
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrationParametersPtberg.cpp
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
#include "AQLDealUtils.h"
#include "AQLCalibrationParametersPtberg.h"
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
#include "AQLPriceFXDisplacedDiffusionCalibration.h"

using namespace std;

// constructor
/*!
	@param funFlg

*/
AQLCalibrationParametersPtberg::AQLCalibrationParametersPtberg(void)
: AQLCalibrationParameters()
{
}

// destructor
/*!

*/
AQLCalibrationParametersPtberg::~AQLCalibrationParametersPtberg(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] fx
	@return AQLString object name

*/
AQLString 
AQLCalibrationParametersPtberg::createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx)
{
	AQLString keyFX = fx;
	const AQLString CALIBINFONAME = keyFX.toUpper() + "_PtbergCalibInfoEntity";
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
	// set beta cap and beta floor
	const double betaCap = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_BETACAP).getDoubleValue();
	const double betaFloor = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_BETAFLOOR).getDoubleValue();
	info->add(PRICING_DATA_BETACAP, new AQLDataDouble(betaCap));
	info->add(PRICING_DATA_BETAFLOOR, new AQLDataDouble(betaFloor));

	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create time grid
	DoubleVector timeGrid;
	int maxTerm = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	// create tenor SA
	//AQLString cFreq = AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ);
	//AQLCoreDataService::setContext(ARG_KEY_CANONICALFREQ, "SA");
	AQLMarketDataHW::getCanonicalGrid(timeGrid, asOfDate, dayCount, maxTerm);
	//AQLCoreDataService::setContext(ARG_KEY_CANONICALFREQ, cFreq);
	// get initial calib param
	DoubleVector betaVec(timeGrid.size()), volVec(timeGrid.size());
	AQLString isInitialFRead = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_ISFILEREAD).toUpper();
	if (isInitialFRead == "TRUE")
	{
		// get initial file
		AQLString fileName = AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_INITIALPARAM_FILE));
		if (AQLCoreDataService::isFileExist(fileName))
		{
			AQLFileAccessor file(fileName);
			AQLStringMatrix paramMtx;
			file.readAllData(MARKET_DATA_DELIMITER, paramMtx);
			file.close();
			unsigned int size = paramMtx.size();
			if (size != timeGrid.size())
			{
				throw AQLCoreInvalidData("Calib param file data size is wrong." ,__FILE__, __LINE__);
			}
			timeGrid.resize(size);
			betaVec.resize(size);
			volVec.resize(size);		
			for (unsigned int i = 0; i < size; ++i)
			{
				if (paramMtx[i].size() != 3)
				{
					throw AQLCoreInvalidData("Calib param file format is wrong." ,__FILE__, __LINE__);
				}
				timeGrid[i] = paramMtx[i][0].getDoubleValue();
				volVec[i] = paramMtx[i][1].getDoubleValue();
				betaVec[i] = paramMtx[i][2].getDoubleValue();
			}
		}
	}
	else
	{
		const double initialBeta =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_INITIALBETA).getDoubleValue();
		const double initialVol =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_INITIALVOL).getDoubleValue();		
		betaVec.resize(timeGrid.size(), initialBeta);
		volVec.resize(timeGrid.size(), initialVol);
	}
	// set fx timegrid
	info->add(PRICING_DATA_FXTIMEGRIDS, new AQLDataDoubles(timeGrid));
	// set beta and fx
	info->add(PRICING_DATA_FXBETAGRIDS, new AQLDataDoubles(betaVec));
	info->add(PRICING_DATA_FXVOLGRIDS, new AQLDataDoubles(volVec));
	const double fixedBeta =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_FIXEDBETA).getDoubleValue();
	const double fixedVol =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_FIXEDVOL).getDoubleValue();
	info->add(PRICING_DATA_FXBETAGRIDS_FIXED, new AQLDataDoubles(DoubleArray(timeGrid.size(), fixedBeta)));
	info->add(PRICING_DATA_FXVOLGRIDS_FIXED, new AQLDataDoubles(DoubleArray(timeGrid.size(), fixedVol)));

	// set spot fx and spotdate
	AQLMathFXEntity fxEntity(info->getDataInstance());
	fxEntity.getFXType() = "FIXEDRATE";
	AQLMarketData::setUpMarket2FXEntity(fxEntity);
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(keyFX, ccys);
	const double spotRate =  fxEntity.getRate(ccys[1], ccys[0], 0.0);
	const AQLDate spotDate = fxEntity.getSpotDate(ccys[0], ccys[1], asOfDate);
	info->add(PRICING_DATA_SPOTFX, new AQLDataDouble(spotRate));
	info->add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDate));

	// set strike fwd term
	AQLStringVector strikeFwdVec = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_STRIKEFWDTERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(PRICING_DATA_STRIKEFWDTERM, new AQLDataStrings(strikeFwdVec));
	// set calib variable
	const double calibVari = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_CALIBVARIABLE).getDoubleValue();
	DoubleVector calibVariVec(2, calibVari);
	info->add(PRICING_DATA_CALIBVARIABLES, new AQLDataDoubles(calibVariVec));
	// set calib param
	AQLString calibParam = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_CALIBPARAM);
	info->add(PRICING_DATA_CALIBPARAM, new AQLDataString(calibParam));
	// set calib input type
	AQLString inputType = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_INPUTPARAMTYPE);
	info->add(PRICING_DATA_CALIBINPUTPARAMTYPE, new AQLDataString(inputType));
	// terms calendar
	AQLString strTenorCal = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_TERM_CALENDAR);
	info->add(PRICING_DATA_TERMCALENDAR, new AQLPriceDataCalendar()).convertFromString(strTenorCal);
	// terms slidingrule
	AQLString strTenorSliding = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_TERM_SLIDINGRULE).toUpper();
	info->add(PRICING_DATA_TERMSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(strTenorSliding);	
	// terms spotlag
	AQLString strTenorSpotLag = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_TERM_SPOTLAG);
	info->add(PRICING_DATA_TERMSPOTLAG, new AQLDataInt()).convertFromString(strTenorSpotLag);	
	// terms daycount
	AQLString strTenorDayCount = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_TERM_DAYCOUNT).toUpper();
	info->add(PRICING_DATA_TERMDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(strTenorDayCount);	
	// terms isincludelast
	AQLString strIsIncludeLast = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_TERM_ISINCLUDELAST).toUpper();
	info->add(PRICING_DATA_TERMISINCLUDELAST, new AQLDataBool()).convertFromString(strIsIncludeLast);	
	// create market param
	// terms
	AQLStringVector terms = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_PTBERG_TERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new AQLDataStrings(terms));
	unsigned int termSize = terms.size();
	// get fx vol file
	AQLString volFile = AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VOLATILITY_FILE));
	
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
	
	DoubleVector tBetaVec(termSize);        // beta vec
	BoolVector isFitATMVec(termSize);		// is fit atm
	BoolVector isDeltaNeutVec(termSize);		// is delta neutral

	AQLDataBool tmpBool;
	for (unsigned int i = 0; i < termSize; ++i)
	{
		AQLString lterm = terms[i];
		lterm.toLower();
		calVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_CALENDAR, lterm);
		slidingVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_SLIDINGRULE, lterm).toUpper();
		soptOrFwdVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_SPOTORFWD, lterm).toUpper();
		// optiontype
		opTypeATMVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_ATM, lterm).toUpper();
		opType25DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_25DH, lterm).toUpper();
		opType10DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_10DH, lterm).toUpper();
		opType25DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_25DL, lterm).toUpper();
		opType10DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_OPTIONTYPE_10DL, lterm).toUpper();
		// weight
		weightATMVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_WEIGHT_ATM, lterm).getDoubleValue();
		weight25DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_WEIGHT_25DH, lterm).getDoubleValue();
		weight10DHVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_WEIGHT_10DH, lterm).getDoubleValue();
		weight25DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_WEIGHT_25DL, lterm).getDoubleValue();
		weight10DLVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_WEIGHT_10DL, lterm).getDoubleValue();
		// beta
		tBetaVec[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_TERMBETA, lterm).getDoubleValue();
		// is fit atm?
		tmpBool.convertFromString(getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_ISFITATM, lterm));
		isFitATMVec[i] = tmpBool.get();
		// is delta neutral?
		tmpBool.convertFromString(getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_ISDELTANEUTRAL, lterm));
		isDeltaNeutVec[i] = tmpBool.get();
		// volatility val
		volATMVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_ATM_VOL);
		vol25DHVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_25DHIGH_VOL);
		vol10DHVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_10DHIGH_VOL);
		vol25DLVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_25DLOW_VOL);
		vol10DLVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_10DLOW_VOL);
	
		// TODO merget latest version strike
		// at merge delete literal
		AQLString tmpVal = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_EXTRASTRIKE + AQLString("25dh"), lterm);
		if (tmpVal != AQ_NO_DATA)
		{
			strike25DHVec[i] = tmpVal.getDoubleValue();
		}
		tmpVal = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_EXTRASTRIKE + AQLString("10dh"), lterm);
		if (tmpVal != AQ_NO_DATA)
		{
			strike10DHVec[i] = tmpVal.getDoubleValue();
		}
		tmpVal = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_EXTRASTRIKE + AQLString("25dl"), lterm);
		if (tmpVal != AQ_NO_DATA)
		{
			strike25DLVec[i] = tmpVal.getDoubleValue();
		}
		tmpVal = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_PTBERG_EXTRASTRIKE + AQLString("10dl"), lterm);
		if (tmpVal != AQ_NO_DATA)
		{
			strike10DLVec[i] = tmpVal.getDoubleValue();
		}
	
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
	info->add(PRICING_DATA_TERMBETA, new AQLDataDoubles(tBetaVec));

	return CALIBINFONAME;

}


/*!
    @brief get calibration property value

	@param[in] key
	@param[in] grid
	@return AQLString value

*/
AQLString 
AQLCalibrationParametersPtberg::getCalibStaticDataValue(const AQLString &key, const AQLString &grid)
{
	AQLString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == AQ_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}
	
	return ret;
}

