/*! @file
    @brief Calibration method create class for hw
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersHW.cpp
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
#include "LACalibrationParametersHW.h"
#include "LAStaticData.h"
#include "LADefinitions.h"
#include "LADefinitionsCalibration.h"
#include "LADefinitionsHW.h"
#include "LAMarketDataHW.h"
#include "LAMarketData.h"
#include "LAMathJamshidianSwaption.h"
#include "LAMathJamshidianSwaptionByImplyVol.h"

using namespace std;

// constructor
/*!
	@param funFlg

*/
LACalibrationParametersHW::LACalibrationParametersHW(void)
: LACalibrationParameters()
{
}

// destructor
/*!

*/
LACalibrationParametersHW::~LACalibrationParametersHW(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] currency
	@return LAString object name

*/
LAString 
LACalibrationParametersHW::createCalibrationInfo(LAObjectPool &objPool, const LAString &ccy)
{
	LAString tmpCurrency = ccy;
	const LAString CALIBINFONAME = tmpCurrency.toUpper() + "_HWCalibInfoEntity";
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
	const LAString appMat = MADealUtils::getAppMat();
	// set fittingtarget
	LAString fitTarget = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_FITTINGTARGET + appMat);
	// check
	fitTarget.toUpper();
	if (fitTarget != CALIB_TARGET_VOLATILITY && fitTarget != CALIB_TARGET_PREMIUM)
	{
		throw LACoreInvalidData("Calib fitting target is only volatility or premium", __FILE__, __LINE__);
	}
	info->add(PRICING_DATA_FITTINGTARGET,new LADataString()).convertFromString(fitTarget);
	// set sde integral grid
	info->add(PRICING_DATA_SDEINTEGRALGRID, new LADataInt(HWGAUSSLEGENDRENUM));

	// get tenor
	DoubleVector tenor;
	int maxTerm = LACoreDataService::getContext(CONTEXT_KEY_MAXTERM).getIntValue();
	LAString dayCountStr = LACoreDataService::getContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT);
	LAPriceDataDayCount dayCount;
	dayCount.convertFromString(dayCountStr);
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// create tenor SA
    //LAString cFreq = LACoreDataService::getContext(ARG_KEY_CANONICALFREQ);
    //LACoreDataService::setContext(ARG_KEY_CANONICALFREQ, "SA");
	LAMarketDataHW::getCanonicalGrid(tenor, asOfDate, dayCount, maxTerm);
    //LACoreDataService::setContext(ARG_KEY_CANONICALFREQ, cFreq);

	// set initial param
	LAString isInitialFRead = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_INITIALPARAM_ISFILEREAD).toUpper();
	DoubleVector volVec, meanVec;
	if (isInitialFRead == "TRUE")
	{
		// get initial file
		LAString fileName = LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_INITIALPARAM_FILE));
		MAFileAccessor file(fileName);
		LAStringMatrix paramMtx;
		file.readAllData(MARKET_DATA_DELIMITER, paramMtx);
		file.close();
		unsigned int size = paramMtx.size();
		if (size != tenor.size())
		{
			throw LACoreInvalidData("Calib param file data size is wrong." ,__FILE__, __LINE__);
		}
		volVec.resize(size);
		meanVec.resize(size);
		for (unsigned int i = 0; i < size; ++i)
		{
			if (paramMtx[i].size() != 2)
			{
				throw LACoreInvalidData("Calib param file format is wrong." ,__FILE__, __LINE__);
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
	info->add(PRICING_DATA_CALIBCANONICAL_T, new LADataDoubles(tenor));
	info->add(PRICING_DATA_CALIBVOL_T, new LADataDoubles(volVec));
	info->add(PRICING_DATA_CALIBMEANREV_T, new LADataDoubles(meanVec));
	LAString strIsTMREV = LACoreDataService::getStaticDataManager().getStaticData().getStaticData(tmpCurrency + STATIC_DATA_KEY_HW_MEANREV_ISTIMEDEPENDENT);
	strIsTMREV.toUpper();
	if (strIsTMREV == "TRUE")
	{
		info->add(PRICING_DATA_ISTIMEDEPENDMEANREV, new LADataBool(true));
	}
	else
	{
		info->add(PRICING_DATA_ISTIMEDEPENDMEANREV, new LADataBool(false));
	}

	const double calibVari = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_CALIBVARIABLE + appMat).getDoubleValue();
	DoubleVector calibVariVec(1, calibVari);
	info->add(PRICING_DATA_CALIBVARIABLES, new LADataDoubles(calibVariVec));

	// swaption volatility type
	const LAString voltype = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_SWAPTION_VOLTYPE).toUpper();
	info->add(IR_CALIBRATION_DATA_SWAPTIONVOLTYPE, new LADataString(voltype));
	// swaption market forward shift
	const double swaptionMarketForwardShift = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_SWAPTION_MARKET_FORWARDSHIFT).getDoubleValue();
	info->add(IR_CALIBRATION_DATA_SWAPTIONMARKETFORWARDSHIFT, new LADataDouble(swaptionMarketForwardShift));


	// create market param
	// optionmaturity
	LAStringVector optionMatVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_OPTIONMATURITY + appMat).toToken(MULTI_STATIC_DATA_DELIMITER);
	// swapterm
	LAStringVector sTenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_SWAPTENOR + appMat).toToken(MULTI_STATIC_DATA_DELIMITER);
	unsigned int opSize = optionMatVec.size();
	if (opSize != sTenorVec.size())
	{
		throw LACoreInvalidData("Option term size and swap term size is not same.", __FILE__, __LINE__);
	}

	// get swaption vol file
	LAString swaptionFile = LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SWAPTION_FILE));

	LAStringVector dayCountVec(opSize); // daycount
	LAStringVector freqVec(opSize);     // frequency
	LAStringVector noticeVec(opSize);   // noticeperiod
	LAStringVector fCalVec(opSize);     // fixingcalendar
	LAStringVector pCalVec(opSize);     // paymentcalendar
	LAStringVector slidingVec(opSize);  // slidingrule
	LAStringVector strikeVec(opSize);   // strike
	LAStringVector opTypeVec(opSize);   // optiontype
	LAStringVector weightVec(opSize);   // weight
	DoubleVector blackVolVec(opSize, 1.0E-12);   // blackvol
	// zero volatility flag
	bool isZeroVol = false;
	LAString strIsZeroVol = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_HW_ISZEROVOL);
	if (strIsZeroVol != AQ_NO_DATA)
	{
		LADataBool tmpBool;
		tmpBool.convertFromString(strIsZeroVol);
		isZeroVol = tmpBool.get();
	}

	bool isZeroVolIR = false;
	LAString strIsZeroVolIR = mpStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_HW_ISZEROVOL);
	if (strIsZeroVolIR != AQ_NO_DATA)
	{
		LADataBool tmpBool;
		tmpBool.convertFromString(strIsZeroVolIR);
		isZeroVolIR = tmpBool.get();

		if (isZeroVol != isZeroVolIR)
		{
			throw LACoreInvalidData("IsZeroVol settings are inconsistent between ir.properties and calib.properties.", __FILE__, __LINE__);
		}

	}

	for (unsigned int i = 0; i < opSize; ++i)
	{
		LAString grid = optionMatVec[i] + "." + sTenorVec[i];
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
			blackVolVec[i] = LAMarketData::getVolatilityVal(swaptionFile, optionMatVec[i], sTenorVec[i], true);
		}
	}
	// set market param
	// optionmaturity
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new LADataStrings(optionMatVec));
	// swaptenor
	info->add(IR_CALIBRATION_DATA_SWAPTENOR, new LADataStrings(sTenorVec));
	// daycount
	info->add(IR_MODEL_DATA_DAYCOUNT, new LADataStrings(dayCountVec));
	// frequency
	info->add(IR_MODEL_DATA_FREQUENCY, new LADataStrings(freqVec));
	// noticeperiod
	info->add(PRICING_CALIBRATION_DATAOTICEPERIOD, new LADataStrings(noticeVec));
	// fixingcalendar
	info->add(PRICING_DATA_FIXINGCALENDAR, new LADataStrings(fCalVec));
	// paymentcalendar
	info->add(IR_CALIBRATION_DATA_PAYMENTCALENDAR, new LADataStrings(pCalVec));
	// slidingrule
	info->add(CALIBRATION_DATA_SLIDINGRULE, new LADataStrings(slidingVec));
	// strike
	info->add(PRICING_DATA_STRIKE, new LADataStrings(strikeVec));
	// optiontype
	info->add(PRICING_DATA_OPTIONTYPE, new LADataStrings(opTypeVec));
	// weight
	info->add(PRICING_DATA_WEIGHT, new LADataStrings(weightVec));
	// black vol
	info->add(IR_CALIBRATION_DATA_BLACKVOLATILITY, new LADataDoubles(blackVolVec));

	return CALIBINFONAME;

}


/*!
    @brief get calibration property value

	@param[in] key
	@param[in] grid
	@return LAString value

*/
LAString 
LACalibrationParametersHW::getCalibStaticDataValue(const LAString &key, const LAString &grid)
{
	LAString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == AQ_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}
	
	return ret;
}

