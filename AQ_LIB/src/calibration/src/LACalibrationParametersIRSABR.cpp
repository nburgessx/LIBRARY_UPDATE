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


#include "AQLString.h"
#include "AQLObjectPool.h"
#include "AQLObject.h"
#include "AQLPriceDataType.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "LAMathHWFuncMR.h"
#include "AQLMathDefine.h"
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
#include "AQLDataMatrix.h"
#include "AQLDataReference.h"
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
	@return AQLString object names

*/
AQLString 
LACalibrationParametersIRSABR::createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy)
{
	AQLString tmpCurrency = ccy;
	tmpCurrency.toLower();

	AQLString underlying = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING);
	if (underlying == AQ_NO_DATA)
		return AQ_NO_DATA;

	underlying.toLower();
	AQLStringVector underlyings = underlying.toToken(':');

	AQLString ret = "";
	for (unsigned int i = 0; i < underlyings.size(); ++i)
	{
		const AQLString CALIBINFONAME = tmpCurrency.toUpper() + "_IRSABRCalibInfoEntity" + "_" + underlyings[i];
		ret += CALIBINFONAME + ":";
		AQLObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
		if (objHolder.isDefined())
		{
			// do nothing
			//return CALIBINFONAME;
			continue;
		}
		
		AQLObject *info = new AQLObject;
		objPool.set(CALIBINFONAME, info);
		// set name
		info->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(CALIBINFONAME);
		// set underlying
		info->add(PRICING_DATA_UNDERLYING, new AQLDataString()).convertFromString(underlyings[i]);
		// set currency
		info->add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString()).convertFromString(ccy);

		tmpCurrency.toLower();

		AQLDate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

		// set fittingtarget
		AQLString fitTarget = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_FITTINGTARGET + "." + underlyings[i]);
		// check
		fitTarget.toUpper();
		if (fitTarget != CALIB_TARGET_VOLATILITY && fitTarget != CALIB_TARGET_PREMIUM)
		{
			throw AQLCoreInvalidData("Calib fitting target is only volatility or premium", __FILE__, __LINE__);
		}
		info->add(PRICING_DATA_FITTINGTARGET,new AQLDataString()).convertFromString(fitTarget);

		AQLString strIsClosedFormAlpha = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCLOSEDFORMALPHA + "." + underlyings[i]).toUpper();
		//tmp control
		if (strIsClosedFormAlpha != "TRUE")
			throw AQLCoreInvalidData("IsClosedFormAlpha Error",__FILE__,__LINE__);
		info->add("IsClosedFormAlpha", new AQLDataBool()).convertFromString(strIsClosedFormAlpha);

		//swaption data and weigh 
		AQLString strSwaptionData = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONDATA + "." + underlyings[i]).toUpper();
		AQLString strWeight = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_WEIGHT + "." + underlyings[i]).toUpper();
		AQLString strOptionType = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_OPTIONTYPE + "." + underlyings[i]).toUpper();
		AQLStringVector swapdatavec = strSwaptionData.toToken(MULTI_STATIC_DATA_DELIMITER);
		AQLStringVector weightvec = strWeight.toToken(MULTI_STATIC_DATA_DELIMITER);
		AQLStringVector optiontypevec;
		if (strOptionType == AQ_NO_DATA)
		{
			optiontypevec.resize(1, CALL);
		}
		else
		{
			optiontypevec = strOptionType.toToken(MULTI_STATIC_DATA_DELIMITER);
		} 
		if (swapdatavec.size() != weightvec.size() || swapdatavec.size() != optiontypevec.size())
			throw AQLCoreInvalidData("SwaptionDate and Weight and OptionType Size Error",__FILE__,__LINE__);
		
		//if (strIsClosedFormAlpha == "TRUE")
		//{
		//	AQLStringVector::iterator it = std::find(swapdatavec.begin(),swapdatavec.end(),"ATM");
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
			throw AQLCoreInvalidData("Swaption ID support only ATM",__FILE__,__LINE__);
		}
		info->add(PRICING_DATA_SWAPTIONDATA, new AQLDataStrings(swapdatavec));
		info->add(PRICING_DATA_WEIGHT, new AQLDataStrings(weightvec));
		info->add(PRICING_DATA_OPTIONTYPE, new AQLDataStrings(optiontypevec));
		// tenor 
		// optionmaturity
		AQLStringVector optionMatVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_OPTIONMATURITY + "." + underlyings[i]).toToken(MULTI_STATIC_DATA_DELIMITER);
		//upper
		for (unsigned int j = 0; j < optionMatVec.size(); ++j)
			optionMatVec[j].toUpper();
		info->add(PRICING_DATA_EXPIRYSTRING, new AQLDataStrings(optionMatVec));
		// swapterm
		AQLStringVector sTenorVec = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTENOR + "." + underlyings[i]).toToken(MULTI_STATIC_DATA_DELIMITER);
		//upper
		for (unsigned int j = 0; j < sTenorVec.size(); ++j)
			sTenorVec[j].toUpper();
		info->add(PRICING_DATA_TENORSTRING, new AQLDataStrings(sTenorVec));

		//curve types key
		AQLStringVector curveTypeKeys = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CURVETYPEKEYS + "." + underlyings[i]).toToken(MULTI_STATIC_DATA_DELIMITER);
		AQLStringVector curveTypes = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CURVETYPES + "." + underlyings[i]).toToken(MULTI_STATIC_DATA_DELIMITER);

		if (curveTypeKeys.size() != curveTypes.size())
			throw AQLCoreInvalidData("CurveTypes Error",__FILE__,__LINE__);

		info->add(PRICING_DATA_CURVETYPEKEYS, new AQLDataStrings(curveTypeKeys));
		info->add(PRICING_DATA_CURVETYPES, new AQLDataStrings(curveTypes));

		//it supports only ATM
		DoubleVector tmpvec(sTenorVec.size(),0.0);
		DoubleMatrix tmpmat(optionMatVec.size(),tmpvec);
		
		AQLString strIsAtmvol = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISNOLYATMVOL + "." + underlyings[i]).toUpper();
		bool isonlyatmvol = (strIsAtmvol == "TRUE");

		// get swaption volfile, alphafile, betafile, rhofile,nufile
		unsigned int forSize = 5;

		for (unsigned int k = 0; k < forSize; k++)
		{
			AQLString file;
			AQLString addstr;
			if (0 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SWAPTION_FILE + "." + underlyings[i]));
				addstr =  AQLString("_") + PRICING_DATA_VOLID;
			}
			else if (1 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SABR_ALPHA_FILE + "." + underlyings[i]));
				addstr =  AQLString("_") + PRICING_DATA_ALPHAID;
			}
			else if (2 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SABR_BETA_FILE + "." + underlyings[i]));
				addstr =  AQLString("_") + PRICING_DATA_BETAID;
			}
			else if (3 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SABR_RHO_FILE + "." + underlyings[i]));
				addstr =  AQLString("_") + PRICING_DATA_RHOID;
			}
			else if (4 == k)
			{
				file =  LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_SABR_NU_FILE + "." + underlyings[i]));
				addstr =  AQLString("_") + PRICING_CALIBRATION_DATAUID;
			}
			
			//volid
			AQLObject* eid = NULL;
			AQLString idname = CALIBINFONAME + addstr + "_" + underlyings[i];
			objHolder = objPool.getObject(idname, ENCHKTYPE_NOCHECK);
			if (objHolder.isDefined())
			{
				eid = &(objHolder.get());
				eid->clear();
			}
			if (!objHolder.isDefined())
			{
				eid = new AQLObject();
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

			eid->add(PRICING_DATA_SWAPTIONMATRIX, new AQLDataDoubleMatrix(valmat));
			eid->add(CALIBRATION_DATA_NAME, new AQLDataString(idname));
			eid->add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));

			// set premium
			if (k == 0)
			{
				const AQLString premFileName = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_PREMIUM_FILE + "." + underlyings[i]);
				if (premFileName != AQ_NO_DATA)
				{
					const AQLString _premFileName =  LAMarketData::getNumFileName(premFileName);
					DoubleMatrix premiummat(tmpmat);
					for (unsigned int row = 0; row < premiummat.size(); ++row)
						for (unsigned int col = 0; col < premiummat[row].size(); ++col)
							premiummat[row][col] = LAMarketData::getVolatilityVal(_premFileName, optionMatVec[row], sTenorVec[col]);
					eid->add(PRICING_DATA_PREMIUMMATRIX, new AQLDataDoubleMatrix(premiummat));

				}
			}

			info->add(AQLString("InitialIRSABR") + addstr, new AQLDataReference()).convertFromString(idname);
		}

		//is alpha calibrate
		AQLString strIsCalibrateAlpha = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATEALPHA + "." + underlyings[i]).toUpper();
		info->add(PRICING_DATA_ISCALIBRATEALPHA, new AQLDataBool()).convertFromString(strIsCalibrateAlpha);
		//is beta calibrate
		AQLString strIsCalibrateBeta = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATEBETA + "." + underlyings[i]).toUpper();
		info->add(PRICING_DATA_ISCALIBRATEBETA, new AQLDataBool()).convertFromString(strIsCalibrateBeta);
		//is beta rho
		AQLString strIsCalibrateRho = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATERHO + "." + underlyings[i]).toUpper();
		info->add(PRICING_DATA_ISCALIBRATERHO, new AQLDataBool()).convertFromString(strIsCalibrateRho);
		//is beta nu
		AQLString strIsCalibrateNu = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATENU + "." + underlyings[i]).toUpper();
		info->add(PRICING_DATA_ISCALIBRATENU, new AQLDataBool()).convertFromString(strIsCalibrateNu);

		//capconvention
		AQLObject* ecap = NULL;
		AQLString capconvid = CALIBINFONAME + AQLString("_") + PRICING_DATA_CAPCONVID + "_" + underlyings[i];
		objHolder = objPool.getObject(capconvid, ENCHKTYPE_NOCHECK);
		if (objHolder.isDefined())
		{
			ecap = &(objHolder.get());
			ecap->clear();
		}
		if (!objHolder.isDefined())
		{
			ecap = new AQLObject();
			objPool.set(capconvid,ecap);
		}
		//cap name
		ecap->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(capconvid);
		//cap daycount
		AQLString strCapdaycount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPDAYCOUNT + "." + underlyings[i]).toUpper();
		ecap->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(strCapdaycount);
		//cap frequency
		AQLString strCapfrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPFREQUENCY + "." + underlyings[i]).toUpper();
		ecap->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(strCapfrequency);
		//cap spotlag
		AQLString strCapspotlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPSPOTLAG + "." + underlyings[i]);
		if (AQ_NO_DATA == strCapspotlag)
			throw AQLCoreInvalidData("Spotlag Error",__FILE__,__LINE__);
		
		//tmp
		ecap->add(CURVEINPUT_SPOTLAG, new AQLDataString()).convertFromString(strCapspotlag.getCString());
		//cap slidingrule
		AQLString strCapslidingrule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPSLIDINGRULE + "." + underlyings[i]).toUpper();
		ecap->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(strCapslidingrule);
		//cap payment calendar
		AQLString strCappaymentcalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPPAYMENTCALENDAR + "." + underlyings[i]);
		ecap->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(strCappaymentcalendar);
		//cap fixing calendar
		AQLString strCapfixingcalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CAPFIXINGCALENDAR + "." + underlyings[i]);
		ecap->add(PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(strCapfixingcalendar);

		//set calibconvid
		info->add(PRICING_DATA_CAPCONVID, new AQLDataReference()).convertFromString(capconvid);
		
		//swaptionconvention
		AQLObject* eswaption = NULL;
		AQLString swaptionconvid = CALIBINFONAME + AQLString("_") + PRICING_DATA_SWAPTIONCONVID + "_" + underlyings[i];
		objHolder = objPool.getObject(swaptionconvid, ENCHKTYPE_NOCHECK);
		if (objHolder.isDefined())
		{
			eswaption = &(objHolder.get());
			eswaption->clear();
		}
		if (!objHolder.isDefined())
		{
			eswaption = new AQLObject();
			objPool.set(swaptionconvid,eswaption);
		}
		//swaption name
		eswaption->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(swaptionconvid);
		//swaption daycount
		AQLString strSwaptiondaycount = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONDAYCOUNT + "." + underlyings[i]).toUpper();
		eswaption->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(strSwaptiondaycount);
		//swaption frequency
		AQLString strSwaptionfrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFREQUENCY + "." + underlyings[i]).toUpper();
		eswaption->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(strSwaptionfrequency);
		for(size_t j = 0; j < sTenorVec.size(); j++)
		{
			AQLString suffix = "." + sTenorVec[j];
			suffix.toLower();
			AQLString strSwaptionfrequency = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFREQUENCY + "." + underlyings[i] + suffix).toUpper();
			if (strSwaptionfrequency != AQ_NO_DATA)
				eswaption->add(IR_CALIBRATION_DATA_FREQUENCY + AQLString("_") + sTenorVec[j], new AQLDataString()).convertFromString(strSwaptionfrequency);
		}; 
		//swaption spotlag
		AQLString strSwaptionspotlag = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONSPOTLAG + "." + underlyings[i]);
		if (AQ_NO_DATA == strSwaptionspotlag)
			throw AQLCoreInvalidData("Spotlag Error",__FILE__,__LINE__);
		eswaption->add(CURVEINPUT_SPOTLAG, new AQLDataString()).convertFromString(strSwaptionspotlag.getCString());
		//swaption slidingrule
		AQLString strSwaptionslidingrule = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONSLIDINGRULE + "." + underlyings[i]).toUpper();
		eswaption->add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(strSwaptionslidingrule);
		//swaption payment calendar
		AQLString strSwaptionpaymentcalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONPAYMENTCALENDAR + "." + underlyings[i]);
		eswaption->add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar()).convertFromString(strSwaptionpaymentcalendar);
		//swaption fixing calendar
		AQLString strSwaptionfixingcalendar = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFIXINGCALENDAR + "." + underlyings[i]);
		eswaption->add(PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(strSwaptionfixingcalendar);

		//set swaptionconvid
		info->add(PRICING_DATA_SWAPTIONCONVID, new AQLDataReference()).convertFromString(swaptionconvid);

		//approximation method
		AQLString strApproxMehod = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_APPROXMETHOD + "." + underlyings[i]);
		info->add(PRICING_DATA_APPROXMETHOD, new AQLDataString()).convertFromString(strApproxMehod);
		//calib method
		AQLString strCalibMehod = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_CALIBMETHOD + "." + underlyings[i]);
		info->add(PRICING_DATA_CALIBMETHOD, new AQLDataString()).convertFromString(strCalibMehod);
		//ishaganatmvolfixed
		AQLString strIsHaganAtmVolFixed = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_ISHAGANATMVOLFIXED + "." + underlyings[i]);
		if (strIsHaganAtmVolFixed != AQ_NO_DATA)
		{
			info->add(PRICING_DATA_ISHAGANATMVOLFIXED, new AQLDataBool()).convertFromString(strIsHaganAtmVolFixed);
		}
		//forwardshift
		AQLString strForwardShift = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_FORWARDSHIFTVALUE + "." + underlyings[i]);
		if (strForwardShift != AQ_NO_DATA)
		{
			info->add(PRICING_DATA_FORWARDSHIFTVALUE, new AQLDataDouble()).convertFromString(strForwardShift);
		}

		AQLStringVector stmpvec(sTenorVec.size(),strApproxMehod.toUpper());
		AQLStringMatrix gridWithMethod(optionMatVec.size(),stmpvec);

        AQLString strExtraGrid = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_EXTRAGRID + "." + underlyings[i]);
		if (strExtraGrid != AQ_NO_DATA)
		{
			AQLString strExtraGridApproxMehod = mpCalibStaticData->getStaticData(tmpCurrency + STATIC_DATA_KEY_CALIB_IRSABR_EXTRAGRIDAPPROXMETHOD + "." + underlyings[i]);
			if (AQ_NO_DATA == strExtraGridApproxMehod)
				throw AQLCoreInvalidData("The extra grid approx method doesn`t exist.",__FILE__,__LINE__);

			AQLStringVector designatedExtraGridVec = strExtraGrid.toToken(MULTI_STATIC_DATA_DELIMITER);
			AQLStringVector extraGridMethodVec =strExtraGridApproxMehod.toToken(MULTI_STATIC_DATA_DELIMITER);

			unsigned int designatedExtraGridSize = designatedExtraGridVec.size();
			unsigned int extraGridMethodSize = extraGridMethodVec.size();

			if (designatedExtraGridSize != extraGridMethodSize)
				throw AQLCoreInvalidData("The size of the extra grids don`t match the size of the ex",__FILE__,__LINE__);

			for (int j =0; j <designatedExtraGridSize; j++)
			{
				AQLString check = designatedExtraGridVec[j];
				AQLStringVector splitUp = designatedExtraGridVec[j].toToken('_');
				AQLString optionMat = splitUp[0];
				AQLString swapTenor = splitUp[1];

				AQLStringVector optionMatTerms =  splitUp[0].toToken('-');
				AQLStringVector swapTenorTerms =  splitUp[1].toToken('-');

				AQLStringVector::const_iterator it = find(optionMatVec.begin(), optionMatVec.end(), optionMatTerms[0]);
				if (it == optionMatVec.end())
				{
					AQLString msg = "The designated volatility area does not exist in the swaption market data, designated area= " + designatedExtraGridVec[j] + ", option maturity = " + optionMatTerms[0];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				unsigned int indexOptionMat_start = static_cast<unsigned int>(it - optionMatVec.begin());
				unsigned int indexOptionMat_end = indexOptionMat_start;

				for (unsigned int k = 1; k < optionMatTerms.size(); ++k)
				{
					it = find(optionMatVec.begin(), optionMatVec.end(), optionMatTerms[k]);
					if (it == optionMatVec.end())
					{
						AQLString msg = "The designated volatility area does not exist in the swaption market data, designated area= " + designatedExtraGridVec[j] + ", option maturity = " + optionMatTerms[k];
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int indexOptionMat_end_tmp = static_cast<unsigned int>(it - optionMatVec.begin());
					if (indexOptionMat_end_tmp < indexOptionMat_end)
					{
						AQLString msg = "Option maturity format is wrong. designated area= " + designatedExtraGridVec[j] + ", option maturity = " + optionMat;
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					indexOptionMat_end = indexOptionMat_end_tmp;
				}

				it = find(sTenorVec.begin(), sTenorVec.end(), swapTenorTerms[0]);
				if (it == sTenorVec.end())
				{
					AQLString msg = "The designated volatility area does not exist in the swaption market data, designated area= " + designatedExtraGridVec[j] + ", swap tenor = " + swapTenorTerms[0];
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				unsigned int indexSwapTenor_start = static_cast<unsigned int>(it - sTenorVec.begin());
				unsigned int indexSwapTenor_end = indexSwapTenor_start;
				for (unsigned int k = 1; k < swapTenorTerms.size(); ++k)
				{
					it = find(sTenorVec.begin(), sTenorVec.end(), swapTenorTerms[k]);
					if (it == sTenorVec.end())
					{
						AQLString msg = "The designated volatility area does not exist in the swaption market data, designated area= " + designatedExtraGridVec[j] + ", swap tenor = " + swapTenorTerms[k];
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int indexSwapTenor_end_tmp = static_cast<unsigned int>(it - sTenorVec.begin());
					if (indexSwapTenor_end_tmp < indexSwapTenor_end)
					{
						AQLString msg = "Swap tenor format is wrong. designated area= " + designatedExtraGridVec[j] + ", swap tenor = " + swapTenor;
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
			info->add(PRICING_DATA_GRIDAPPROXMETHOD, new AQLDataStringMatrix(gridWithMethod));
	}
	
	ret = ret.subString(0,ret.size()-2);
	return ret;
}



