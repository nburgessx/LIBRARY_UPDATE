/*! @file
    @brief Calibration method create class for hw
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersFXStrangleSolver.cpp
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
#include "LACalibrationParametersFXStrangleSolver.h"
#include "LAStaticData.h"
#include "LADefinitions.h"
#include "LADefinitionsCalibration.h"
#include "LADefinitionsHW.h"
#include "LADefinitionsFXStrangleSolver.h"
#include "LAMarketDataHW.h"
#include "LAMarketData.h"
#include "LAMathJamshidianSwaption.h"
#include "LAMathJamshidianSwaptionByImplyVol.h"
#include "LAMathAntonovFXOption.h"
#include "LAPriceFXDisplacedDiffusionCalibration.h"


#include "LAMathDateUtilities.h"
using namespace std;

// constructor
/*!
	@param funFlg

*/
LACalibrationParametersFXStrangleSolver::LACalibrationParametersFXStrangleSolver(void)
: LACalibrationParameters()
{
}

// destructor
/*!

*/
LACalibrationParametersFXStrangleSolver::~LACalibrationParametersFXStrangleSolver(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] fx
	@return LAString object name

*/
LAString 
LACalibrationParametersFXStrangleSolver::createCalibrationInfo(LAObjectPool &objPool, const LAString &fx)
{
	LAString keyFX = fx;
	const LAString CALIBINFONAME = keyFX.toUpper() + "_FXStrangleSolverCalibInfoEntity";
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

	keyFX.toLower();


	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	LAStringVector strTerms = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int termSize = strTerms.size();

	// terms calendar
	LAString strCal = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM_CALENDAR);
	LAPriceDataCalendar cal;
	cal.convertFromString(strCal);

	// terms slidingrule
	LAString strSliding = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM_SLIDINGRULE).toUpper();
	LAPriceDataSlidingRule sr;
	sr.convertFromString(strSliding);

	// terms spotlag
	LAString spotlagStr = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM_SPOTLAG);
	int spotlag = spotlagStr.getIntValue();
	LADate spotdate = LAMathDateCalculations::getFXSpotDate(keyFX,asOfDate,strCal,spotlag,true);


	//isincludelast
	LAString strincl = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_TERM_ISINCLUDELAST).toUpper();
	LADataBool isinclu;
	isinclu.convertFromString(strincl);

	// deltatype
	LAString strdeltatype =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_DELTATYPE);
	strdeltatype.toUpper();
	LAString stratmtype =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_ATMTYPE);
	stratmtype.toUpper();
	double wingfact = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_WINGFACTOR).getDoubleValue();



	
	// get fx vol file
	LAString volFile = LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VOLATILITY_FILE));
	DoubleVector volATMVec(termSize);      // vol ATM
	DoubleVector vol25DHVec(termSize);     // vol 25Delta High
	DoubleVector vol10DHVec(termSize);     // vol 10Delta High
	DoubleVector vol25DLVec(termSize);     // vol 25Delta Low
	DoubleVector vol10DLVec(termSize);     // vol 10Delta Low
	



	DateVector terms(termSize);
	DateVector deliveryterms(termSize);
	LAStringVector deltatypes(termSize,strdeltatype),atmtypes(termSize,stratmtype);
	DoubleVector wingfactors(termSize, wingfact); 
	for (unsigned int i = 0; i < termSize; i++)
	{	
		LAString lterm = strTerms[i];
		lterm.toLower();

		//hishida vannavolga warning this must be changed in all of sources
		//LAString tmpdate = LAMarketData::convertToMLibTerm(strTerms[i]);
//		LAString tmpdate = strTerms[i];
//		terms[i] = LAMathDateCalculations::getDate(asOfDate,tmpdate,sr,&cal,true);
//		deliveryterms[i] = LAMathDateCalculations::getDate(terms[i],spotlagday,sr,&cal,true);

		deliveryterms[i] = LAMathDateCalculations::getDate(spotdate,strTerms[i],sr,&cal,true);
		terms[i] = LAMathDateCalculations::getFXSpotDate(keyFX,deliveryterms[i],strCal,-spotlag,true);

		deltatypes[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_STRGLSLV_DELTATYPE, lterm);
		deltatypes[i].toUpper();
		
		atmtypes[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_STRGLSLV_ATMTYPE, lterm);
		atmtypes[i].toUpper();

		wingfactors[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_STRGLSLV_WINGFACTOR, lterm).getDoubleValue();

		// volatility val
		volATMVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_ATM_VOL);
		vol25DHVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_25DHIGH_VOL);
		vol10DHVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_10DHIGH_VOL);
		vol25DLVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_25DLOW_VOL);
		vol10DLVec[i] = LAMarketData::getVolatilityVal(volFile, lterm, FXVOL_10DLOW_VOL);
	
	
	}
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new LADataStrings(strTerms));
	info->add("MaturityDates", new LADataDates(terms));
	info->add("DeliveryDates", new LADataDates(deliveryterms));
	info->add("DeltaTypes", new LADataStrings(deltatypes));
	info->add("ATMTypes", new LADataStrings(atmtypes));
	info->add("WingFactors", new LADataDoubles(wingfactors));
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

	//approximation flag
	LAString strisaproxm = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_ISAPPROXIMATION);
	info->add("IsApptoximation", new LADataBool()).convertFromString(strisaproxm);

	//wing flag
	LAString striswing = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_ISWING);
	info->add("IsWing", new LADataBool()).convertFromString(striswing);

	//spot calender (equals to term calender)
	info->add("SpotCalender", new LADataString(strCal));

	//interpolationmethod
	LAString strinterp = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_INTERPOLATIONMETHOD);
	info->add("InterpolationMethod", new LADataString()).convertFromString(strinterp.toUpper());

	LAString strdeltastrike = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_DELTAORSTRIKE);
	info->add("DeltaOrStrike", new LADataString()).convertFromString(strdeltastrike.toUpper());

	LAString variable = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_VARIABLE);
	info->add("Variable", new LADataString()).convertFromString(variable.toUpper());

	LAString matumethod = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_MATURITYMETHOD);
	info->add("MaturityMethod", new LADataString()).convertFromString(matumethod.toUpper());
	
	return CALIBINFONAME;
}


/*!
    @brief get calibration property value

	@param[in] key
	@param[in] grid
	@return LAString value

*/
LAString 
LACalibrationParametersFXStrangleSolver::getCalibStaticDataValue(const LAString &key, const LAString &grid)
{
	LAString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == AQ_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}
	
	return ret;
}

