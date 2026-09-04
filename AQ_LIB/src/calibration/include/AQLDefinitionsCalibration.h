/*! @file
    @brief Calibration data declare file
*/

#ifndef AQLDefinitionsCalibration_h
#define AQLDefinitionsCalibration_h

#define STANDARD "standard";
#define CALIBDATA "Calibdata";

#define PRICING_DATA_FITTINGTARGET "FittingTarget"
#define PRICING_DATA_UNDERLYING "Underlying"
#define PRICING_DATA_ISOUTPUT "IsOutPut"

#define PRICING_DATA_TERMCALENDAR "TermCalendar"
#define PRICING_DATA_TERMSLIDINGRULE "TermSlidingRule"
#define PRICING_DATA_TERMSPOTLAG "TermSpotLag"
#define PRICING_DATA_TERMDAYCOUNT "TermDayCount"
#define PRICING_DATA_TERMISINCLUDELAST "TermIsIncludeLast"

#define PRICING_DATA_STRIKEFWDTERM "STRIKEFWDTERM"
#define PRICING_DATA_FXBETAGRIDS_FIXED "FXBetaGrids_FIXED"
#define PRICING_DATA_FXVOLGRIDS_FIXED "FXVolGrids_FIXED"
#define PRICING_DATA_CALIBPARAM "CalibrationParam"
#define PRICING_DATA_CALIBINPUTPARAMTYPE "CalibInputParamType"
#define PRICING_DATA_OPTIONTYPE10DL "OptionType10DL"
#define PRICING_DATA_OPTIONTYPE25DL "OptionType25DL"
#define PRICING_DATA_OPTIONTYPEATM "OptionTypeATM"
#define PRICING_DATA_OPTIONTYPE25DH "OptionType25DH"
#define PRICING_DATA_OPTIONTYPE10DH "OptionType10DH"
//hishida vannavolga
#define PRICING_DATA_BASESTRIKES "BaseStrikes"

#define PRICING_DATA_WEIGHT10DL "Weight10DL"
#define PRICING_DATA_WEIGHT25DL "Weight25DL"
#define PRICING_DATA_WEIGHTATM "WeightATM"
#define PRICING_DATA_WEIGHT25DH "Weight25DH"
#define PRICING_DATA_WEIGHT10DH "Weight10DH"

#define PRICING_DATA_FXVOL10DL "Vol10DL"
#define PRICING_DATA_FXVOL25DL "Vol25DL"
#define PRICING_DATA_FXVOLATM "VolATM"
#define PRICING_DATA_FXVOL25DH "Vol25DH"
#define PRICING_DATA_FXVOL10DH "Vol10DH"

#define IR_CALIBRATION_DATA_SWAPTIONVOLTYPE "SwaptionVolType"
#define IR_CALIBRATION_DATA_SWAPTIONMARKETFORWARDSHIFT "SwaptionMarketForwardShift"

#define FXVOL_10DLOW_VOL  "10DLow_Vol"
#define FXVOL_25DLOW_VOL  "25DLow_Vol"
#define FXVOL_ATM_VOL  "ATM_Vol"
#define FXVOL_25DHIGH_VOL  "25DHigh_Vol"
#define FXVOL_10DHIGH_VOL  "10DHigh_Vol"

#define FXVOL_10DLOW_STRIKE  "10DLow_"
#define FXVOL_25DLOW_STRIKE  "25DLow_"
#define FXVOL_ATM_STRIKE  "ATM_"
#define FXVOL_25DHIGH_STRIKE  "25DHigh_"
#define FXVOL_10DHIGH_STRIKE  "10DHigh_"


#define FXVOL_10BF  "10BF"
#define FXVOL_25BF  "25BF"
#define FXVOL_ATM  "ATM"
#define FXVOL_25RR  "25RR"
#define FXVOL_10RR  "10RR"

#define CALIB_TARGET_VOLATILITY  "VOLATILITY"
#define CALIB_TARGET_PREMIUM  "PREMIUM"
#define CALIB_DMY_FILE "CALIB_DMY_FILE"

#define CALIB_PARAM_BOTH  "BOTH"
#define CALIB_PARAM_BETA  "BETA"
#define CALIB_PARAM_VOLATILITY "VOLATILITY"

#define CALIB_INPUTTYPE_VOLONLY "VOLONLY"
#define CALIB_INPUTTYPE_ALL "ALL"

#define CALIB_COR_FUNC_TYPE_B "FUNCTYPEB"
#define CALIB_COR_FUNC_TYPE_C "FUNCTYPEC"

#define CALIB_OPT_TARGET_TYPE_PCA "PCA"
#define CALIB_OPT_TARGET_TYPE_REDUCERANK_ANGLE_FORMULA "REDUCE-RANK_ANGLE_FORMULATIONS"

#define CALIB_MARKET_SWAPTION "SWAPTION"
#define CALIB_MARKET_CAP "CAP"

#define CALIB_INTER_SPLINE  "SPLINE"
#define CALIB_OPT_BUY  "BUY"
#define CALIB_RATE_PER  "PAR"
#define CALIB_STRIKE_ATM  "ATM"

// TODO merge latest version
#define PRICING_DATA_STRIKEVALES "StrikeVales"
// serialize
#define CALIB_S_SERIALIZE  "SERIALIZE"
#define CALIB_S_DESERIALIZE  "DESERIALIZE"
#define CALIB_S_NORMAL  "NORMAL"

#define VOLATITY_BLACK "BLACK"
#define VOLATITY_NORMAL "NORMAL"

#ifndef PRICING_DATA_SWAPTIONCONVID
#define PRICING_DATA_SWAPTIONCONVID				"SwaptionConvID"
#endif
#ifndef PRICING_DATA_CAPCONVID
#define PRICING_DATA_CAPCONVID					"CapConvID"
#endif
#ifndef PRICING_DATA_CURVEID
#define PRICING_DATA_CURVEID					"CurveID"				//  data name of CurveID
#endif
#ifndef PRICING_DATA_FIXINGCALENDAR
#define PRICING_DATA_FIXINGCALENDAR			"FixingCalendar"		//  data name of calendar for fixing date
#endif

#ifndef CALIBRATION_DATA_DISCOUNTCURVENAME
#define CALIBRATION_DATA_DISCOUNTCURVENAME		"DiscountCurveName"		//  [Discount Curve Name] Data
#endif
#ifndef CALIBRATION_DATA_CURVEID
#define CALIBRATION_DATA_CURVEID					"CurveID"		//  [Discount Curve Name] Data
#endif

#ifndef IR_MODEL_DATA_FREQUENCY
#define IR_MODEL_DATA_FREQUENCY				"Frequency"				//  data name of frequency
#endif
#ifndef IR_MODEL_DATA_DAYCOUNT
#define IR_MODEL_DATA_DAYCOUNT				"DayCount"				//  data name of daycount 
#endif
#ifndef IR_MODEL_DATA_PATHENTITY
#define IR_MODEL_DATA_PATHENTITY				"PathEntity"			//  data name of path object
#endif
#ifndef IR_MODEL_DATA_FXRATE
#define IR_MODEL_DATA_FXRATE					"FXRate"				//  data name of fx object
#endif
#ifndef IR_MODEL_DATA_CACHESIZE
#define IR_MODEL_DATA_CACHESIZE				"CacheSize"				//  data name of cachesize
#endif
#ifndef PRICING_DATA_ROLLDAYFORINDEXGENERATE
#define PRICING_DATA_ROLLDAYFORINDEXGENERATE "RollDayForIndexGenerate"  //  data name of RollDayForIndexGenerate
#endif
#ifndef PRICING_DATA_BASISCURVE
#define PRICING_DATA_BASISCURVE             "BasisCurve"                 //  data name of Basis
#endif
#ifndef PRICING_DATA_BASISINTERPOLATION
#define PRICING_DATA_BASISINTERPOLATION     "BasisInterpolation"    //  data name of BasisInterpolation
#endif

#ifndef CALIBRATION_DATA_FORWARDFX_FORPRINT
#define CALIBRATION_DATA_FORWARDFX_FORPRINT "ForwardFX_ForPrint"
#endif
#ifndef CALIBRATION_DATA_FXVOL_FORPRINT
#define CALIBRATION_DATA_FXVOL_FORPRINT "FXVol_ForPrint"
#endif

#ifndef CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT
#define CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT "ForwardSwapRate_ForPrint"
#endif
#ifndef CALIBRATION_DATA_SWAPTIONVOL_FORPRINT
#define CALIBRATION_DATA_SWAPTIONVOL_FORPRINT "SwaptionVol_ForPrint"
#endif
#ifndef CALIBRATION_DATA_OPTION_MATURITY_FORPRINT
#define CALIBRATION_DATA_OPTION_MATURITY_FORPRINT "OptionMaturity_ForPrint"
#endif
#ifndef CALIBRATION_DATA_SETTLE_TERM_FORPRINT
#define CALIBRATION_DATA_SETTLE_TERM_FORPRINT "SettleTerm_ForPrint"
#endif

//********************************
//
// property key start
//
//********************************

#define KEY_CALIB_THREADNUM "calib.threadnum"
#define KEY_CALIB_REQUESTNUM "calib.requestnum"
#define KEY_CALIB_WAIT_MILLISECOND "calib.wait.millisecond"
#define KEY_CALIB_WAIT_TIME "calib.wait.time"
#define KEY_CALIB_SERIALIZE_STATUS "calib.serialize.status"
#define KEY_CALIB_SERIALIZE_FILE "calib.serialize.file"
#define KEY_CALIB_INFO_ISRESET "calib.info.isreset"


#define STATIC_DATA_KEY_CALIB_SWAPTION_FILE ".calib.swaption.file"
#define STATIC_DATA_KEY_CALIB_PREMIUM_FILE ".calib.premium.file"
#define STATIC_DATA_KEY_CALIB_CAP_FILE ".calib.cap.file"
#define FX_KEY_CALIB_VOLATILITY_FILE ".calib.volatility.file"

#define STATIC_DATA_KEY_CALIB_SABR_ALPHA_FILE ".calib.irsabr.alpha.file"
#define STATIC_DATA_KEY_CALIB_SABR_BETA_FILE ".calib.irsabr.beta.file"
#define STATIC_DATA_KEY_CALIB_SABR_RHO_FILE ".calib.irsabr.rho.file"
#define STATIC_DATA_KEY_CALIB_SABR_NU_FILE ".calib.irsabr.nu.file"

#endif