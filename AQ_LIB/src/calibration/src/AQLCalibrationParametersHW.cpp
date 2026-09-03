/*! @file
    @brief Calibration method create class for hw
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrationParametersHW.cpp
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
#include "AQLCalibrationParametersHW.h"
#include "AQLStaticData.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLDefinitionsHW.h"
#include "AQLMarketDataHW.h"
#include "AQLMarketData.h"
#include "AQLMathJamshidianSwaption.h"
#include "AQLMathJamshidianSwaptionByImplyVol.h"

using namespace std;

// constructor
/*!
	@param funFlg

*/
AQLCalibrationParametersHW::AQLCalibrationParametersHW(void)
: AQLCalibrationParameters()
{
}

// destructor
/*!

*/
AQLCalibrationParametersHW::~AQLCalibrationParametersHW(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] currency
	@return AQLString object name

*/
AQLString 
AQLCalibrationParametersHW::createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy)
{
	AQLString tmpCurrency = ccy;
	const AQLString CALIBINFONAME = tmpCurrency.toUpper() + "_HWCalibInfoEntity";
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
	const AQLString appMat = AQLDealUtils::getAppMat();
	// set fittingtarget
	AQLString fitTarget = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_FITTINGTARGET + appMat);
	// check
	fitTarget.toUpper();
	if (fitTarget != CALIB_TARGET_VOLATILITY && fitTarget != CALIB_TARGET_PREMIUM)
	{
		throw AQLCoreInvalidData("Calib fitting target is only volatility or premium", __FILE__, __LINE__);
	}
	info->add(PRICING_DATA_FITTINGTARGET,new AQLDataString()).convertFromString(fitTarget);
	// set sde integral grid
	info->add(PRICING_DATA_SDEINTEGRALGRID, new AQLDataInt(HWGAUSSLEGENDRENUM));

	// get tenor
	DoubleVector tenor;
	int maxTerm = AQLCoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	AQLString dayCountStr = AQLCoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	AQLPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create tenor SA
    //AQLString cFreq = AQLCoreDataService::getContext(ARG_KEY_CANONICALFREQ);
    //AQLCoreDataService::setContext(ARG_KEY_CANONICALFREQ, "SA");
	AQLMarketDataHW::getCanonicalGrid(tenor, asOfDate, dayCount, maxTerm);
    //AQLCoreDataService::setContext(ARG_KEY_CANONICALFREQ, cFreq);

	// set initial param
	AQLString isInitialFRead = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_INITIALPARAM_ISFILEREAD).toUpper();
	DoubleVector volVec, meanVec;
	if (isInitialFRead == "TRUE")
	{
		// get initial file
		AQLString fileName = AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_INITIALPARAM_FILE));
		AQLFileAccessor file(fileName);
		AQLStringMatrix paramMtx;
		file.readAllData(MARKET_DATA_DELIMITER, paramMtx);
		file.close();
		unsigned int size = paramMtx.size();
		if (size != tenor.size())
		{
			throw AQLCoreInvalidData("Calib param file data size is wrong." ,__FILE__, __LINE__);
		}
		volVec.resize(size);
		meanVec.resize(size);
		for (unsigned int i = 0; i < size; ++i)
		{
			if (paramMtx[i].size() != 2)
			{
				throw AQLCoreInvalidData("Calib param file format is wrong." ,__FILE__, __LINE__);
			}
			meanVec[i] = paramMtx[i][0].getDoubleValue();
			volVec[i] = paramMtx[i][1].getDoubleValue();
		}
	}
	else
	{
		const double meanRev = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_INITIALMEANREVERSION + appMat).getDoubleValue();
		const double initialVol = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_INITIALVOL + appMat).getDoubleValue();
		meanVec.resize(tenor.size(), meanRev);
		volVec.resize(tenor.size(), initialVol);
	}
	
	// set tenor and initial vol and mean rev
	info->add(PRICING_DATA_CALIBCANONICAL_T, new AQLDataDoubles(tenor));
	info->add(PRICING_DATA_CALIBVOL_T, new AQLDataDoubles(volVec));
	info->add(PRICING_DATA_CALIBMEANREV_T, new AQLDataDoubles(meanVec));
	AQLString strIsTMREV = AQLCoreDataService::getStaticDataManager().getStaticData().getStaticData(tmpCurrency + STATIC_DATA_KEY_HW_MEANREV_ISTIMEDEPENDENT);
	strIsTMREV.toUpper();
	if (strIsTMREV == "TRUE")
	{
		info->add(PRICING_DATA_ISTIMEDEPENDMEANREV, new AQLDataBool(true));
	}
	else
	{
		info->add(PRICING_DATA_ISTIMEDEPENDMEANREV, new AQLDataBool(false));
	}

	const double calibVari = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_CALIBVARIABLE + appMat).getDoubleValue();
	DoubleVector calibVariVec(1, calibVari);
	info->add(PRICING_DATA_CALIBVARIABLES, new AQLDataDoubles(calibVariVec));

	// swaption volatility type
	const AQLString voltype = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_SWAPTION_VOLTYPE).toUpper();
	info->add(IR_CALIBRATION_DATA_SWAPTIONVOLTYPE, new AQLDataString(voltype));
	// swaption market forward shift
	const double swaptionMarketForwardShift = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_SWAPTION_MARKET_FORWARDSHIFT).getDoubleValue();
	info->add(IR_CALIBRATION_DATA_SWAPTIONMARKETFORWARDSHIFT, new AQLDataDouble(swaptionMarketForwardShift));


	// create market param
	// optionmaturity
	AQLStringVector optionMatVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_OPTIONMATURITY + appMat).toToken(MULTI_STATIC_DATA_DELIMITER);
	// swapterm
	AQLStringVector sTenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_SWAPTENOR + appMat).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int opSize = optionMatVec.size();
	if (opSize != sTenorVec.size())
	{
		throw AQLCoreInvalidData("Option term size and swap term size is not same.", __FILE__, __LINE__);
	}

	// get swaption vol file
	AQLString swaptionFile = AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SWAPTION_FILE));

	AQLStringVector dayCountVec(opSize); // daycount
	AQLStringVector freqVec(opSize);     // frequency
	AQLStringVector noticeVec(opSize);   // noticeperiod
	AQLStringVector fCalVec(opSize);     // fixingcalendar
	AQLStringVector pCalVec(opSize);     // paymentcalendar
	AQLStringVector slidingVec(opSize);  // slidingrule
	AQLStringVector strikeVec(opSize);   // strike
	AQLStringVector opTypeVec(opSize);   // optiontype
	AQLStringVector weightVec(opSize);   // weight
	DoubleVector blackVolVec(opSize, 1.0E-12);   // blackvol
	// zero volatility flag
	bool isZeroVol = false;
	AQLString strIsZeroVol = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_ISZEROVOL);
	if (strIsZeroVol != AQ_NO_DATA)
	{
		AQLDataBool tmpBool;
		tmpBool.convertFromString(strIsZeroVol);
		isZeroVol = tmpBool.get();
	}

	bool isZeroVolIR = false;
	AQLString strIsZeroVolIR = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_HW_ISZEROVOL);
	if (strIsZeroVolIR != AQ_NO_DATA)
	{
		AQLDataBool tmpBool;
		tmpBool.convertFromString(strIsZeroVolIR);
		isZeroVolIR = tmpBool.get();

		if (isZeroVol != isZeroVolIR)
		{
			throw AQLCoreInvalidData("IsZeroVol settings are inconsistent between ir.properties and calib.properties.", __FILE__, __LINE__);
		}

	}

	for (unsigned int i = 0; i < opSize; ++i)
	{
		AQLString grid = optionMatVec[i] + "." + sTenorVec[i];
		grid.toLower();

		dayCountVec[i] = getCalibStaticDataValue(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_DAYCOUNT_GRID + appMat, grid).toUpper();
		freqVec[i] = getCalibStaticDataValue(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_FREQUENCY_GRID + appMat, grid).toUpper();
		noticeVec[i] = getCalibStaticDataValue(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_NOTICEPERIOD_GRID + appMat, grid);
		fCalVec[i] = getCalibStaticDataValue(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_FIXINGCALENDAR_GRID + appMat, grid);
		pCalVec[i] = getCalibStaticDataValue(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_PAYMENTCALENDAR_GRID + appMat, grid);
		slidingVec[i] = getCalibStaticDataValue(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_SLIDINGRULE_GRID + appMat, grid).toUpper();
		strikeVec[i] = getCalibStaticDataValue(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_STRIKE_GRID + appMat, grid).toUpper();
		opTypeVec[i] = getCalibStaticDataValue(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_OPTIONTYPE_GRID + appMat, grid).toUpper();
		weightVec[i] = getCalibStaticDataValue(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_WEIGHT_GRID + appMat, grid).toUpper();
		if (!isZeroVol)
		{
			blackVolVec[i] = AQLMarketData::getVolatilityVal(swaptionFile, optionMatVec[i], sTenorVec[i], true);
		}
	}
	// set market param
	// optionmaturity
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new AQLDataStrings(optionMatVec));
	// swaptenor
	info->add(IR_CALIBRATION_DATA_SWAPTENOR, new AQLDataStrings(sTenorVec));
	// daycount
	info->add(IR_MODEL_DATA_DAYCOUNT, new AQLDataStrings(dayCountVec));
	// frequency
	info->add(IR_MODEL_DATA_FREQUENCY, new AQLDataStrings(freqVec));
	// noticeperiod
	info->add(PRICING_CALIBRATION_DATAOTICEPERIOD, new AQLDataStrings(noticeVec));
	// fixingcalendar
	info->add(PRICING_DATA_FIXINGCALENDAR, new AQLDataStrings(fCalVec));
	// paymentcalendar
	info->add(IR_CALIBRATION_DATA_PAYMENTCALENDAR, new AQLDataStrings(pCalVec));
	// slidingrule
	info->add(CALIBRATION_DATA_SLIDINGRULE, new AQLDataStrings(slidingVec));
	// strike
	info->add(PRICING_DATA_STRIKE, new AQLDataStrings(strikeVec));
	// optiontype
	info->add(PRICING_DATA_OPTIONTYPE, new AQLDataStrings(opTypeVec));
	// weight
	info->add(PRICING_DATA_WEIGHT, new AQLDataStrings(weightVec));
	// black vol
	info->add(IR_CALIBRATION_DATA_BLACKVOLATILITY, new AQLDataDoubles(blackVolVec));

	return CALIBINFONAME;

}


/*!
    @brief get calibration property value

	@param[in] key
	@param[in] grid
	@return AQLString value

*/
AQLString 
AQLCalibrationParametersHW::getCalibStaticDataValue(const AQLString &key, const AQLString &grid)
{
	AQLString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == AQ_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}
	
	return ret;
}

