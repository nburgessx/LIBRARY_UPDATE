/*! @file
    @brief Constant data declare file
*/
//  2008, AlgoQuantHub.

#ifndef AQLDefinitionsIRSABR_h
#define AQLDefinitionsIRSABR_h

#include "AQLDefinitions.h"


#define PRICING_DATA_SWAPTIONDATA				"SwaptionData"
#define PRICING_DATA_ISCALIBRATEALPHA			"IsCalibrateAlpha"
#define PRICING_DATA_ISCALIBRATEBETA			"IsCalibrateBeta"
#define PRICING_DATA_ISCALIBRATERHO				"IsCalibrateRho"
#define PRICING_DATA_ISCALIBRATENU				"IsCalibrateNu"
#define PRICING_DATA_CAPCONVID					"CapConvID"
#define PRICING_DATA_SWAPTIONCONVID				"SwaptionConvID"
#define PRICING_DATA_CURVESETID					"CurveSetID"

#define PRICING_DATA_VOLID					"VolID"	
#define PRICING_DATA_ALPHAID				"AlphaID"
#define PRICING_DATA_BETAID					"BetaID"
#define PRICING_DATA_RHOID					"RhoID"
#define PRICING_CALIBRATION_DATAUID					"NuID"

#define PRICING_DATA_OPTIONTYPE					"OptionType"
#define PRICING_DATA_CALIBMETHOD				"CalibMethod"
#define PRICING_DATA_APPROXMETHOD				"ApproxMethod"
#define PRICING_DATA_ISHAGANATMVOLFIXED				"IsHaganATMVolFixed"
#define PRICING_DATA_FORWARDSHIFTVALUE				"ForwardShiftValue"

//********************************
//
// property key start
//
//********************************

// calibration
#define STATIC_DATA_KEY_CALIB_IRSABR_ISALLGRIDSCALIBRATE		".calib.irsabr.isallgridscalibrate"
#define STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING					".calib.irsabr.underlying"
#define STATIC_DATA_KEY_CALIB_IRSABR_FITTINGTARGET				".calib.irsabr.fittingtarget"
#define STATIC_DATA_KEY_CALIB_IRSABR_CAPDAYCOUNT				".calib.irsabr.capdaycount"
#define STATIC_DATA_KEY_CALIB_IRSABR_CAPFREQUENCY				".calib.irsabr.capfrequency"
#define STATIC_DATA_KEY_CALIB_IRSABR_CAPSLIDINGRULE				".calib.irsabr.capslidingrule"
#define STATIC_DATA_KEY_CALIB_IRSABR_CAPPAYMENTCALENDAR			".calib.irsabr.cappaymentcalendar"
#define STATIC_DATA_KEY_CALIB_IRSABR_CAPFIXINGCALENDAR			".calib.irsabr.capfixingcalendar"
#define STATIC_DATA_KEY_CALIB_IRSABR_CAPSPOTLAG					".calib.irsabr.capspotlag"
#define STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONDAYCOUNT			".calib.irsabr.swaptiondaycount"
#define STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFREQUENCY			".calib.irsabr.swaptionfrequency"
#define STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONSLIDINGRULE		".calib.irsabr.swaptionslidingrule"
#define STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONPAYMENTCALENDAR	".calib.irsabr.swaptionpaymentcalendar"
#define STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONFIXINGCALENDAR		".calib.irsabr.swaptionfixingcalendar"
#define STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONSPOTLAG			".calib.irsabr.swaptionspotlag"

#define STATIC_DATA_KEY_CALIB_IRSABR_ISCLOSEDFORMALPHA			".calib.irsabr.isclosedformalpha"
#define STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATEALPHA			".calib.irsabr.iscalibratealpha"
#define STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATEBETA			".calib.irsabr.iscalibratebeta"
#define STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATERHO				".calib.irsabr.iscalibraterho"
#define STATIC_DATA_KEY_CALIB_IRSABR_ISCALIBRATENU				".calib.irsabr.iscalibratenu"

#define STATIC_DATA_KEY_CALIB_IRSABR_APPROXMETHOD				".calib.irsabr.approxmethod"
#define STATIC_DATA_KEY_CALIB_IRSABR_CALIBMETHOD				".calib.irsabr.calibmethod"
#define STATIC_DATA_KEY_CALIB_IRSABR_ISHAGANATMVOLFIXED			".calib.irsabr.ishaganatmvolfixed"
#define STATIC_DATA_KEY_CALIB_IRSABR_FORWARDSHIFTVALUE			".calib.irsabr.forwardshiftvalue"

#define STATIC_DATA_KEY_CALIB_IRSABR_SWAPTIONDATA				".calib.irsabr.swaptiondata"
#define STATIC_DATA_KEY_CALIB_IRSABR_WEIGHT						".calib.irsabr.weight"
#define STATIC_DATA_KEY_CALIB_IRSABR_OPTIONTYPE					".calib.irsabr.optiontype"

#define STATIC_DATA_KEY_CALIB_IRSABR_SWAPTENOR				".calib.irsabr.swaptenor"
#define STATIC_DATA_KEY_CALIB_IRSABR_OPTIONMATURITY			".calib.irsabr.optionmaturity"
#define STATIC_DATA_KEY_CALIB_IRSABR_ISNOLYATMVOL			".calib.irsabr.isonlyatmvol"
#define STATIC_DATA_KEY_CALIB_IRSABR_CURVETYPEKEYS			".calib.irsabr.curvetypekeys"
#define STATIC_DATA_KEY_CALIB_IRSABR_CURVETYPES			    ".calib.irsabr.curvetypes"

#define STATIC_DATA_KEY_CALIB_IRSABR_EXTRAGRID	                ".calib.irsabr.extragrid"
#define STATIC_DATA_KEY_CALIB_IRSABR_EXTRAGRIDAPPROXMETHOD	    ".calib.irsabr.extragridapproxmethod"

#define STATIC_DATA_KEY_CALIB_IRSABR_TARGETVOLGRID_FILE			".calib.irsabr.targetvolgrid.file"

//********************************
//
// property key end
//
//********************************

#endif