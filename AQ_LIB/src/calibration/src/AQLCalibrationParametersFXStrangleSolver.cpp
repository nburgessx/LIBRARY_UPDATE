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
#include "AQLCalibrationParametersFXStrangleSolver.h"
#include "AQLStaticData.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLDefinitionsHW.h"
#include "AQLDefinitionsFXStrangleSolver.h"
#include "AQLMarketDataHW.h"
#include "AQLMarketData.h"
#include "AQLMathJamshidianSwaption.h"
#include "AQLMathJamshidianSwaptionByImplyVol.h"
#include "AQLMathAntonovFXOption.h"
#include "AQLPriceFXDisplacedDiffusionCalibration.h"


#include "AQLDateSchedule.h"
using namespace std;

// constructor
/*!
	@param funFlg

*/
AQLCalibrationParametersFXStrangleSolver::AQLCalibrationParametersFXStrangleSolver(void)
: AQLCalibrationParameters()
{
}

// destructor
/*!

*/
AQLCalibrationParametersFXStrangleSolver::~AQLCalibrationParametersFXStrangleSolver(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] fx
	@return AQLString object name

*/
AQLString 
AQLCalibrationParametersFXStrangleSolver::createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx)
{
	AQLString keyFX = fx;
	const AQLString CALIBINFONAME = keyFX.toUpper() + "_FXStrangleSolverCalibInfoEntity";
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

	keyFX.toLower();


	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	AQLStringVector strTerms = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int termSize = strTerms.size();

	// terms calendar
	AQLString strCal = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM_CALENDAR);
	AQLPriceDataCalendar cal;
	cal.convertFromString(strCal);

	// terms slidingrule
	AQLString strSliding = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM_SLIDINGRULE).toUpper();
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(strSliding);

	// terms spotlag
	AQLString spotlagStr = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM_SPOTLAG);
	int spotlag = spotlagStr.getIntValue();
	AQLDate spotdate = AQLDateCalculations::getFXSpotDate(keyFX,asOfDate,strCal,spotlag,true);


	//isincludelast
	AQLString strincl = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM_ISINCLUDELAST).toUpper();
	AQLDataBool isinclu;
	isinclu.convertFromString(strincl);

	// deltatype
	AQLString strdeltatype =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_DELTATYPE);
	strdeltatype.toUpper();
	AQLString stratmtype =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_ATMTYPE);
	stratmtype.toUpper();
	double wingfact = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_WINGFACTOR).getDoubleValue();



	
	// get fx vol file
	AQLString volFile = AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VOLATILITY_FILE));
	DoubleVector volATMVec(termSize);      // vol ATM
	DoubleVector vol25DHVec(termSize);     // vol 25Delta High
	DoubleVector vol10DHVec(termSize);     // vol 10Delta High
	DoubleVector vol25DLVec(termSize);     // vol 25Delta Low
	DoubleVector vol10DLVec(termSize);     // vol 10Delta Low
	



	DateVector terms(termSize);
	DateVector deliveryterms(termSize);
	AQLStringVector deltatypes(termSize,strdeltatype),atmtypes(termSize,stratmtype);
	DoubleVector wingfactors(termSize, wingfact); 
	for (unsigned int i = 0; i < termSize; i++)
	{	
		AQLString lterm = strTerms[i];
		lterm.toLower();

		// vanna-volga warning this must be changed in all of sources
		//AQLString tmpdate = AQLMarketData::aqlConvertToTerm(strTerms[i]);
//		AQLString tmpdate = strTerms[i];
//		terms[i] = AQLDateCalculations::getDate(asOfDate,tmpdate,sr,&cal,true);
//		deliveryterms[i] = AQLDateCalculations::getDate(terms[i],spotlagday,sr,&cal,true);

		deliveryterms[i] = AQLDateCalculations::getDate(spotdate,strTerms[i],sr,&cal,true);
		terms[i] = AQLDateCalculations::getFXSpotDate(keyFX,deliveryterms[i],strCal,-spotlag,true);

		deltatypes[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_STRGLSLV_DELTATYPE, lterm);
		deltatypes[i].toUpper();
		
		atmtypes[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_STRGLSLV_ATMTYPE, lterm);
		atmtypes[i].toUpper();

		wingfactors[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_STRGLSLV_WINGFACTOR, lterm).getDoubleValue();

		// volatility val
		volATMVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_ATM_VOL);
		vol25DHVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_25DHIGH_VOL);
		vol10DHVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_10DHIGH_VOL);
		vol25DLVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_25DLOW_VOL);
		vol10DLVec[i] = AQLMarketData::getVolatilityVal(volFile, lterm, FXVOL_10DLOW_VOL);
	
	
	}
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new AQLDataStrings(strTerms));
	info->add("MaturityDates", new AQLDataDates(terms));
	info->add("DeliveryDates", new AQLDataDates(deliveryterms));
	info->add("DeltaTypes", new AQLDataStrings(deltatypes));
	info->add("ATMTypes", new AQLDataStrings(atmtypes));
	info->add("WingFactors", new AQLDataDoubles(wingfactors));
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

	//approximation flag
	AQLString strisaproxm = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_ISAPPROXIMATION);
	info->add("IsApptoximation", new AQLDataBool()).convertFromString(strisaproxm);

	//wing flag
	AQLString striswing = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_ISWING);
	info->add("IsWing", new AQLDataBool()).convertFromString(striswing);

	//spot calender (equals to term calender)
	info->add("SpotCalender", new AQLDataString(strCal));

	//interpolationmethod
	AQLString strinterp = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_INTERPOLATIONMETHOD);
	info->add("InterpolationMethod", new AQLDataString()).convertFromString(strinterp.toUpper());

	AQLString strdeltastrike = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_DELTAORSTRIKE);
	info->add("DeltaOrStrike", new AQLDataString()).convertFromString(strdeltastrike.toUpper());

	AQLString variable = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_VARIABLE);
	info->add("Variable", new AQLDataString()).convertFromString(variable.toUpper());

	AQLString matumethod = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_MATURITYMETHOD);
	info->add("MaturityMethod", new AQLDataString()).convertFromString(matumethod.toUpper());
	
	return CALIBINFONAME;
}


/*!
    @brief get calibration property value

	@param[in] key
	@param[in] grid
	@return AQLString value

*/
AQLString 
AQLCalibrationParametersFXStrangleSolver::getCalibStaticDataValue(const AQLString &key, const AQLString &grid)
{
	AQLString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == AQ_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}
	
	return ret;
}

