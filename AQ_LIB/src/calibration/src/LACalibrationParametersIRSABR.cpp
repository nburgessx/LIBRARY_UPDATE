/*! @file
    @brief Calibration method create class for hw
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersIRSABR.cpp
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
#include "LACalibrationParametersIRSABR.h"
#include "LAStaticData.h"
#include "LADefinitions.h"
#include "LADefinitionsCalibration.h"
#include "LADefinitionsHW.h"
#include "LADefinitionsIRSABR.h"
#include "LAMarketDataHW.h"
#include "LAMarketData.h"
#include "LAMathJamshidianSwaption.h"
#include "LAMathJamshidianSwaptionByImplyVol.h"
#include "LAMathAntonovFXOption.h"
#include "LAPriceFXDisplacedDiffusionCalibration.h"
#include "LADataMatrix.h"
#include "LADataReference.h"
#include "LAMathSwaptionVolUtility.h"


#include "LAMathDateUtilities.h"
using namespace std;

// constructor
/*!
	@param funFlg

*/
LACalibrationParametersIRSABR::LACalibrationParametersIRSABR(void)
: LACalibrationParameters()
{
}

// destructor
/*!

*/
LACalibrationParametersIRSABR::~LACalibrationParametersIRSABR(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] fx
	@return LAString object names

*/
LAString 
LACalibrationParametersIRSABR::createCalibrationInfo(LAObjectPool &objPool, const LAString &ccy)
{
	LAString tmpCurrency = ccy;
	tmpCurrency.toLower();

	LAString underlying = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING);
	if (underlying == MLIB_NO_DATA)
		return MLIB_NO_DATA;

	underlying.toLower();
	LAStringVector underlyings = underlying.toToken(':');

	LAString ret = "";
	for (unsigned int i = 0; i < underlyings.size(); ++i)
	{
		const LAString CALIBINFONAME = tmpCurrency.toUpper() + "_IRSABRCalibInfoEntity" + "_" + underlyings[i];
		ret += CALIBINFONAME + ":";
		LAObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
		if (objHolder.isDefined())
		{
			// do nothing
			//return CALIBINFONAME;
			continue;
		}
		
		LAObject *info = new LAObject;
		objPool.set(CALIBINFONAME, info);
		// set name
		info->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(CALIBINFONAME);
		// set underlying
		info->add(PRICING_DATA_UNDERLYING, new LADataString()).convertFromString(underlyings[i]);
		// set currency
		info->add(IR_CALIBRATION_DATA_CURRENCY, new LADataString()).convertFromString(ccy);

		tmpCurrency.toLower();

		LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

		// set fittingtarget
		LAString fitTarget = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_FITTINGTARGET + "." + underlyings[i]);
		// check
		fitTarget.toUpper();
		if (fitTarget != CALIB_TARGET_VOLATILITY && fitTarget != CALIB_TARGET_PREMIUM)
		{
			throw LACoreInvalidData("Calib fitting target is only volatility or premium", __FILE__, __LINE__);
		}
		info->add(PRICING_DATA_FITTINGTARGET,new LADataString()).convertFromString(fitTarget);

		LAString strIsClosedFormAlpha = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCLOSEDFORMALPHA + "." + underlyings[i]).toUpper();
		//tmp control
		if (strIsClosedFormAlpha != "TRUE")
			throw LACoreInvalidData("IsClosedFormAlpha Error",__FILE__,__LINE__);
		info->add("IsClosedFormAlpha", new LADataBool()).convertFromString(strIsClosedFormAlpha);

		//swaption data and weigh 
		LAString strSwaptionData = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONDATA + "." + underlyings[i]).toUpper();
		LAString strWeight = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_WEIGHT + "." + underlyings[i]).toUpper();
		LAString strOptionType = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_OPTIONTYPE + "." + underlyings[i]).toUpper();
		LAStringVector swapdatavec = strSwaptionData.toToken(MULTI_STATIC_DATA_DELIMITER);
		LAStringVector weightvec = strWeight.toToken(MULTI_STATIC_DATA_DELIMITER);
		LAStringVector optiontypevec;
		if (strOptionType == MLIB_NO_DATA)
		{
			optiontypevec.resize(1, CALL);
		}
		else
		{
			optiontypevec = strOptionType.toToken(MULTI_STATIC_DATA_DELIMITER);
		} 
		if (swapdatavec.size() != weightvec.size() || swapdatavec.size() != optiontypevec.size())
			throw LACoreInvalidData("SwaptionDate and Weight and OptionType Size Error",__FILE__,__LINE__);
		
		//if (strIsClosedFormAlpha == "TRUE")
		//{
		//	LAStringVector::iterator it = std::find(swapdatavec.begin(),swapdatavec.end(),"ATM");
		//	if (it != swapdatavec.end())
		//	{
		//		unsigned int pos = static_cast<unsigned int>(it-swapdatavec.begin());
		//		swapdatavec.erase(swapdatavec.begin() + pos);
		//		weightvec.erase(weightvec.begin() + pos);
		//	}
		//	
		//}

		if (swapdatavec.size() != 1 || swapdatavec[0] != "ATM")
		{
			throw LACoreInvalidData("Swaption ID support only ATM",__FILE__,__LINE__);
		}
		info->add(PRICING_DATA_SWAPTIONDATA, new LADataStrings(swapdatavec));
		info->add(PRICING_DATA_WEIGHT, new LADataStrings(weightvec));
		info->add(PRICING_DATA_OPTIONTYPE, new LADataStrings(optiontypevec));
		// tenor 
		// optionmaturity
		LAStringVector optionMatVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_OPTIONMATURITY + "." + underlyings[i]).toToken(MULTI_STATIC_DATA_DELIMITER);
		//upper
		for (unsigned int j = 0; j < optionMatVec.size(); ++j)
			optionMatVec[j].toUpper();
		info->add(PRICING_DATA_EXPIRYSTRING, new LADataStrings(optionMatVec));
		// swapterm
		LAStringVector sTenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTENOR + "." + underlyings[i]).toToken(MULTI_STATIC_DATA_DELIMITER);
		//upper
		for (unsigned int j = 0; j < sTenorVec.size(); ++j)
			sTenorVec[j].toUpper();
		info->add(PRICING_DATA_TENORSTRING, new LADataStrings(sTenorVec));

		//curve types key
		LAStringVector curveTypeKeys = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CURVETYPEKEYS + "." + underlyings[i]).toToken(MULTI_STATIC_DATA_DELIMITER);
		LAStringVector curveTypes = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CURVETYPES + "." + underlyings[i]).toToken(MULTI_STATIC_DATA_DELIMITER);

		if (curveTypeKeys.size() != curveTypes.size())
			throw LACoreInvalidData("CurveTypes Error",__FILE__,__LINE__);

		info->add(PRICING_DATA_CURVETYPEKEYS, new LADataStrings(curveTypeKeys));
		info->add(PRICING_DATA_CURVETYPES, new LADataStrings(curveTypes));

		//it supports only ATM
		DoubleVector tmpvec(sTenorVec.size(),0.0);
		DoubleMatrix tmpmat(optionMatVec.size(),tmpvec);
		
		LAString strIsAtmvol = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISNOLYATMVOL + "." + underlyings[i]).toUpper();
		bool isonlyatmvol = (strIsAtmvol == "TRUE");

		// get swaption volfile, alphafile, betafile, rhofile,nufile
		unsigned int forSize = 5;

		for (unsigned int k = 0; k < forSize; k++)
		{
			LAString file;
			LAString addstr;
			if (0 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SWAPTION_FILE + "." + underlyings[i]));
				addstr =  LAString("_") + PRICING_DATA_VOLID;
			}
			else if (1 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SABR_ALPHA_FILE + "." + underlyings[i]));
				addstr =  LAString("_") + PRICING_DATA_ALPHAID;
			}
			else if (2 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SABR_BETA_FILE + "." + underlyings[i]));
				addstr =  LAString("_") + PRICING_DATA_BETAID;
			}
			else if (3 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SABR_RHO_FILE + "." + underlyings[i]));
				addstr =  LAString("_") + PRICING_DATA_RHOID;
			}
			else if (4 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SABR_NU_FILE + "." + underlyings[i]));
				addstr =  LAString("_") + PRICING_CALIBRATION_DATAUID;
			}
			
			//volid
			LAObject* eid = NULL;
			LAString idname = CALIBINFONAME + addstr + "_" + underlyings[i];
			objHolder = objPool.getObject(idname, ENCHKTYPE_NOCHECK);
			if (objHolder.isDefined())
			{
				eid = &(objHolder.get());
				eid->clear();
			}
			if (!objHolder.isDefined())
			{
				eid = new LAObject();
				objPool.set(idname,eid);
			}
			//volatility
			DoubleMatrix valmat(tmpmat);
			if (0==k || !isonlyatmvol) //atmvol or usualcalibration case
			{
				for (unsigned int row = 0; row < valmat.size(); ++row)
					for (unsigned int col = 0; col < valmat[row].size(); ++col)
						valmat[row][col] = LAMarketData::getVolatilityVal(file, optionMatVec[row], sTenorVec[col]);
			}
			else if(isonlyatmvol && 2 == k) //atmvolcalbration case and beta
			{
				for (unsigned int row = 0; row < valmat.size(); ++row)
					for (unsigned int col = 0; col < valmat[row].size(); ++col)
						valmat[row][col] = 1.0;
			}
			//else //valmat = 0.0;

			eid->add(PRICING_DATA_SWAPTIONMATRIX, new LADataDoubleMatrix(valmat));
			eid->add(CALIBRATION_DATA_NAME, new LADataString(idname));
			eid->add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));

			// set premium
			if (k == 0)
			{
				const LAString premFileName = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_PREMIUM_FILE + "." + underlyings[i]);
				if (premFileName != MLIB_NO_DATA)
				{
					const LAString _premFileName =  LAMarketData::getNumFileName(premFileName);
					DoubleMatrix premiummat(tmpmat);
					for (unsigned int row = 0; row < premiummat.size(); ++row)
						for (unsigned int col = 0; col < premiummat[row].size(); ++col)
							premiummat[row][col] = LAMarketData::getVolatilityVal(_premFileName, optionMatVec[row], sTenorVec[col]);
					eid->add(PRICING_DATA_PREMIUMMATRIX, new LADataDoubleMatrix(premiummat));

				}
			}

			info->add(LAString("InitialIRSABR") + addstr, new LADataReference()).convertFromString(idname);
		}

		//is alpha calibrate
		LAString strIsCalibrateAlpha = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATEALPHA + "." + underlyings[i]).toUpper();
		info->add(PRICING_DATA_ISCALIBRATEALPHA, new LADataBool()).convertFromString(strIsCalibrateAlpha);
		//is beta calibrate
		LAString strIsCalibrateBeta = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATEBETA + "." + underlyings[i]).toUpper();
		info->add(PRICING_DATA_ISCALIBRATEBETA, new LADataBool()).convertFromString(strIsCalibrateBeta);
		//is beta rho
		LAString strIsCalibrateRho = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATERHO + "." + underlyings[i]).toUpper();
		info->add(PRICING_DATA_ISCALIBRATERHO, new LADataBool()).convertFromString(strIsCalibrateRho);
		//is beta nu
		LAString strIsCalibrateNu = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATENU + "." + underlyings[i]).toUpper();
		info->add(PRICING_DATA_ISCALIBRATENU, new LADataBool()).convertFromString(strIsCalibrateNu);

		//capconvention
		LAObject* ecap = NULL;
		LAString capconvid = CALIBINFONAME + LAString("_") + PRICING_DATA_CAPCONVID + "_" + underlyings[i];
		objHolder = objPool.getObject(capconvid, ENCHKTYPE_NOCHECK);
		if (objHolder.isDefined())
		{
			ecap = &(objHolder.get());
			ecap->clear();
		}
		if (!objHolder.isDefined())
		{
			ecap = new LAObject();
			objPool.set(capconvid,ecap);
		}
		//cap name
		ecap->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(capconvid);
		//cap daycount
		LAString strCapdaycount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPDAYCOUNT + "." + underlyings[i]).toUpper();
		ecap->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(strCapdaycount);
		//cap frequency
		LAString strCapfrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPFREQUENCY + "." + underlyings[i]).toUpper();
		ecap->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(strCapfrequency);
		//cap spotlag
		LAString strCapspotlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPSPOTLAG + "." + underlyings[i]);
		if (MLIB_NO_DATA == strCapspotlag)
			throw LACoreInvalidData("Spotlag Error",__FILE__,__LINE__);
		
		//tmp
		ecap->add(CURVEINPUT_SPOTLAG, new LADataString()).convertFromString(strCapspotlag.getCString());
		//cap slidingrule
		LAString strCapslidingrule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPSLIDINGRULE + "." + underlyings[i]).toUpper();
		ecap->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(strCapslidingrule);
		//cap payment calendar
		LAString strCappaymentcalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPPAYMENTCALENDAR + "." + underlyings[i]);
		ecap->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(strCappaymentcalendar);
		//cap fixing calendar
		LAString strCapfixingcalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPFIXINGCALENDAR + "." + underlyings[i]);
		ecap->add(PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(strCapfixingcalendar);

		//set calibconvid
		info->add(PRICING_DATA_CAPCONVID, new LADataReference()).convertFromString(capconvid);
		
		//swaptionconvention
		LAObject* eswaption = NULL;
		LAString swaptionconvid = CALIBINFONAME + LAString("_") + PRICING_DATA_SWAPTIONCONVID + "_" + underlyings[i];
		objHolder = objPool.getObject(swaptionconvid, ENCHKTYPE_NOCHECK);
		if (objHolder.isDefined())
		{
			eswaption = &(objHolder.get());
			eswaption->clear();
		}
		if (!objHolder.isDefined())
		{
			eswaption = new LAObject();
			objPool.set(swaptionconvid,eswaption);
		}
		//swaption name
		eswaption->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(swaptionconvid);
		//swaption daycount
		LAString strSwaptiondaycount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONDAYCOUNT + "." + underlyings[i]).toUpper();
		eswaption->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(strSwaptiondaycount);
		//swaption frequency
		LAString strSwaptionfrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFREQUENCY + "." + underlyings[i]).toUpper();
		eswaption->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(strSwaptionfrequency);
		for(size_t j = 0; j < sTenorVec.size(); j++)
		{
			LAString suffix = "." + sTenorVec[j];
			suffix.toLower();
			LAString strSwaptionfrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFREQUENCY + "." + underlyings[i] + suffix).toUpper();
			if (strSwaptionfrequency != MLIB_NO_DATA)
				eswaption->add(IR_CALIBRATION_DATA_FREQUENCY + LAString("_") + sTenorVec[j], new LADataString()).convertFromString(strSwaptionfrequency);
		}; 
		//swaption spotlag
		LAString strSwaptionspotlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONSPOTLAG + "." + underlyings[i]);
		if (MLIB_NO_DATA == strSwaptionspotlag)
			throw LACoreInvalidData("Spotlag Error",__FILE__,__LINE__);
		eswaption->add(CURVEINPUT_SPOTLAG, new LADataString()).convertFromString(strSwaptionspotlag.getCString());
		//swaption slidingrule
		LAString strSwaptionslidingrule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONSLIDINGRULE + "." + underlyings[i]).toUpper();
		eswaption->add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(strSwaptionslidingrule);
		//swaption payment calendar
		LAString strSwaptionpaymentcalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONPAYMENTCALENDAR + "." + underlyings[i]);
		eswaption->add(CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar()).convertFromString(strSwaptionpaymentcalendar);
		//swaption fixing calendar
		LAString strSwaptionfixingcalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFIXINGCALENDAR + "." + underlyings[i]);
		eswaption->add(PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(strSwaptionfixingcalendar);

		//set swaptionconvid
		info->add(PRICING_DATA_SWAPTIONCONVID, new LADataReference()).convertFromString(swaptionconvid);

		//approximation method
		LAString strApproxMehod = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_APPROXMETHOD + "." + underlyings[i]);
		info->add(PRICING_DATA_APPROXMETHOD, new LADataString()).convertFromString(strApproxMehod);
		//calib method
		LAString strCalibMehod = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CALIBMETHOD + "." + underlyings[i]);
		info->add(PRICING_DATA_CALIBMETHOD, new LADataString()).convertFromString(strCalibMehod);
		//ishaganatmvolfixed
		LAString strIsHaganAtmVolFixed = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISHAGANATMVOLFIXED + "." + underlyings[i]);
		if (strIsHaganAtmVolFixed != MLIB_NO_DATA)
		{
			info->add(PRICING_DATA_ISHAGANATMVOLFIXED, new LADataBool()).convertFromString(strIsHaganAtmVolFixed);
		}
		//forwardshift
		LAString strForwardShift = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_FORWARDSHIFTVALUE + "." + underlyings[i]);
		if (strForwardShift != MLIB_NO_DATA)
		{
			info->add(PRICING_DATA_FORWARDSHIFTVALUE, new LADataDouble()).convertFromString(strForwardShift);
		}

		LAStringVector stmpvec(sTenorVec.size(),strApproxMehod.toUpper());
		LAStringMatrix gridWithMethod(optionMatVec.size(),stmpvec);

        LAString strExtraGrid = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_EXTRAGRID + "." + underlyings[i]);
		if (strExtraGrid != MLIB_NO_DATA)
		{
			LAString strExtraGridApproxMehod = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_EXTRAGRIDAPPROXMETHOD + "." + underlyings[i]);
			if (MLIB_NO_DATA == strExtraGridApproxMehod)
				throw LACoreInvalidData("The extra grid approx method doesn`t exist.",__FILE__,__LINE__);

			LAStringVector designatedExtraGridVec = strExtraGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
			LAStringVector extraGridMethodVec =strExtraGridApproxMehod.toToken(MULTI_STATIC_DATA_DELIMITER);

			unsigned int designatedExtraGridSize = designatedExtraGridVec.size();
			unsigned int extraGridMethodSize = extraGridMethodVec.size();

			if (designatedExtraGridSize != extraGridMethodSize)
				throw LACoreInvalidData("The size of the extra grids don`t match the size of the ex",__FILE__,__LINE__);

			for (int j =0; j <designatedExtraGridSize; j++)
			{
				LAString check = designatedExtraGridVec[j];
				LAStringVector splitUp = designatedExtraGridVec[j].toToken('_');
				LAString optionMat = splitUp[0];
				LAString swapTenor = splitUp[1];

				LAStringVector optionMatTerms =  splitUp[0].toToken('-');
				LAStringVector swapTenorTerms =  splitUp[1].toToken('-');

				LAStringVector::const_iterator it = find(optionMatVec.begin(), optionMatVec.end(), optionMatTerms[0]);
				if (it == optionMatVec.end())
				{
					LAString msg = "The designated volatility area does not exist in the swaption market data, designated area= " + designatedExtraGridVec[j] + ", option maturity = " + optionMatTerms[0];
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				unsigned int indexOptionMat_start = static_cast<unsigned int>(it - optionMatVec.begin());
				unsigned int indexOptionMat_end = indexOptionMat_start;

				for (unsigned int k = 1; k < optionMatTerms.size(); ++k)
				{
					it = find(optionMatVec.begin(), optionMatVec.end(), optionMatTerms[k]);
					if (it == optionMatVec.end())
					{
						LAString msg = "The designated volatility area does not exist in the swaption market data, designated area= " + designatedExtraGridVec[j] + ", option maturity = " + optionMatTerms[k];
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int indexOptionMat_end_tmp = static_cast<unsigned int>(it - optionMatVec.begin());
					if (indexOptionMat_end_tmp < indexOptionMat_end)
					{
						LAString msg = "Option maturity format is wrong. designated area= " + designatedExtraGridVec[j] + ", option maturity = " + optionMat;
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					indexOptionMat_end = indexOptionMat_end_tmp;
				}

				it = find(sTenorVec.begin(), sTenorVec.end(), swapTenorTerms[0]);
				if (it == sTenorVec.end())
				{
					LAString msg = "The designated volatility area does not exist in the swaption market data, designated area= " + designatedExtraGridVec[j] + ", swap tenor = " + swapTenorTerms[0];
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				unsigned int indexSwapTenor_start = static_cast<unsigned int>(it - sTenorVec.begin());
				unsigned int indexSwapTenor_end = indexSwapTenor_start;
				for (unsigned int k = 1; k < swapTenorTerms.size(); ++k)
				{
					it = find(sTenorVec.begin(), sTenorVec.end(), swapTenorTerms[k]);
					if (it == sTenorVec.end())
					{
						LAString msg = "The designated volatility area does not exist in the swaption market data, designated area= " + designatedExtraGridVec[j] + ", swap tenor = " + swapTenorTerms[k];
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int indexSwapTenor_end_tmp = static_cast<unsigned int>(it - sTenorVec.begin());
					if (indexSwapTenor_end_tmp < indexSwapTenor_end)
					{
						LAString msg = "Swap tenor format is wrong. designated area= " + designatedExtraGridVec[j] + ", swap tenor = " + swapTenor;
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					indexSwapTenor_end = indexSwapTenor_end_tmp;
				}


				for (unsigned int k = indexOptionMat_start; k <= indexOptionMat_end; ++k)
				{
					for (unsigned int l = indexSwapTenor_start; l <= indexSwapTenor_end; ++l)
					{
							gridWithMethod[k][l]=extraGridMethodVec[j].toUpper();
					}
				}

			}

		}
			info->add(PRICING_DATA_GRIDAPPROXMETHOD, new LADataStringMatrix(gridWithMethod));
	}
	
	ret = ret.subString(0,ret.size()-2);
	return ret;
}



