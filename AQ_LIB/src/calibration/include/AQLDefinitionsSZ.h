// $Id: AQLDefinitionsSZ.h,v 1.6 2008/07/03 12:03:18 hirayake Exp $
/*! @file
    @brief Constant data declare file
*/
//  2007, AlgoQuantHub.

#ifndef AQLDefinitionsSZ_h
#define AQLDefinitionsSZ_h

#include "AQLDefinitions.h"

//********************************
//
// property key start
//
//********************************
// key for fx
#define FX_KEY_SZ_FUNCTION_NAME   ".sde.sz.function.name"
#define FX_KEY_SZ_DRIFT   ".sde.sz.drift"
#define FX_KEY_SZ_DRIFT_ESTIMATE_TYPE   ".sde.sz.drift.estimatetype"
#define FX_KEY_SZ_TYPE   ".sde.sz.type"
#define FX_KEY_SZ_INTEGRAL_TYPE   ".sde.sz.integral.type"
#define FX_KEY_SZ_DD_INTEGRAL_TYPE   ".sde.sz.dd.integral.type"
#define FX_KEY_SZ_INTEGRAL_STEP   ".sde.sz.integral.step"
#define FX_KEY_SZ_ISCHANGEVARIAVLES   ".sde.sz.ischangevariables"
#define FX_KEY_SZ_VOLATILITY_TYPE   ".sde.sz.volatility.type"
#define FX_KEY_SZ_VOLATILITY_INITIALVALUE_FILE   ".sde.sz.volatility.initialvalue.file"
#define FX_KEY_SZ_VOLATILITY_INTERPOLATION   ".sde.sz.volatility.interpolation"
#define FX_KEY_SZ_VOLATILITY_DATA_FILE   ".sde.sz.correlation.data.file"
#define FX_KEY_SZ_VOLATILITY_CALIB_FILE   ".sde.sz.volatility.function.file"
#define FX_KEY_SZ_VOLATILITY_CALIB_FILE_FORWORDGRIDS_ISINCLUDE ".sde.sz.volatility.function.file.forwordgrids.isinclude"	// for XLL_PLUS
#define FX_KEY_SZ_MAXMULTIPLIER   ".sde.sz.maxmultiplier"
#define FX_KEY_SZ_ISEXTRAADJ   ".sde.sz.isextraadj"
#define FX_KEY_SZ_IS3F   "sde.sz.is3f"
#define FX_KEY_SZ_QUANTOADJUST_THRESHOLD   "sde.sz.quantoadjust.threshold"

// official risk 
// fx delta
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_SZ_UPSHIFT_CALIB_FILE   ".risk.official.fx.delta.upshift.sz.file"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_SZ_DOWNSHIFT_CALIB_FILE   ".risk.official.fx.delta.downshift.sz.file"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_SZ_TYPE   ".risk.official.fx.delta.sz.type"
// fx atmparallelvega
#define FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_SZ_CALIB_FILE   ".risk.official.vol.fxatmparallelvega.sz.file"
#define FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_SZ_TYPE   ".risk.official.vol.fxatmparallelvega.sz.type"
// front risk
// irdelta
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.yield.irdelta.sce1.parallel.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_GRID_SZ_PARAMETER_FILE   ".risk.front.yield.irdelta.sce1.grid.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.yield.irdelta.sce2.parallel.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_GRID_SZ_PARAMETER_FILE   ".risk.front.yield.irdelta.sce2.grid.sz.parameter.file."
// irshiftdelta
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce1.parallel.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_GRID_SZ_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce1.grid.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce2.parallel.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_GRID_SZ_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce2.grid.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BASE_SZ_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.base.sz.parameter.file."
// basisdelta
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce1.parallel.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_GRID_SZ_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce1.grid.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce2.parallel.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_GRID_SZ_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce2.grid.sz.parameter.file."
// irvega
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.vol.irvega.sce1.parallel.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_GRID_SZ_PARAMETER_FILE   ".risk.front.vol.irvega.sce1.grid.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.vol.irvega.sce2.parallel.sz.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_GRID_SZ_PARAMETER_FILE   ".risk.front.vol.irvega.sce2.grid.sz.parameter.file."
// fxdelta
#define FX_KEY_RISK_FRONT_FX_DELTA_SCE1_SZ_PARAMETER_FILE   ".risk.front.fx.delta.sce1.sz.parameter.file"
#define FX_KEY_RISK_FRONT_FX_DELTA_SCE2_SZ_PARAMETER_FILE   ".risk.front.fx.delta.sce2.sz.parameter.file"
// fxvega
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE1_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.vol.fxvega.sce1.parallel.sz.parameter.file"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE1_GRID_SZ_PARAMETER_FILE   ".risk.front.vol.fxvega.sce1.grid.sz.parameter.file."
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE2_PARALLEL_SZ_PARAMETER_FILE   ".risk.front.vol.fxvega.sce2.parallel.sz.parameter.file"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE2_GRID_SZ_PARAMETER_FILE   ".risk.front.vol.fxvega.sce2.grid.sz.parameter.file."

//calibration
#define KEY_CALIB_SZ_IS3F   "calib.sz.is3f"
//#define FX_KEY_CALIB_SZ_BETACAP   ".calib.sz.betacap" 
//#define FX_KEY_CALIB_SZ_BETAFLOOR   ".calib.sz.betafloor" 
//#define FX_KEY_CALIB_SZ_FIXEDBETA ".calib.sz.fixedbeta"
//#define FX_KEY_CALIB_SZ_FIXEDVOL ".calib.sz.fixedvol"
//#define FX_KEY_CALIB_SZ_INITIALPARAM_ISFILEREAD ".calib.sz.initialparam.isfileread"
//#define FX_KEY_CALIB_SZ_INITIALPARAM_FILE ".calib.sz.initialparam.file"
//#define FX_KEY_CALIB_SZ_INITIALBETA ".calib.sz.initialbeta"
//#define FX_KEY_CALIB_SZ_INITIALVOL ".calib.sz.initialvol"
#define FX_KEY_CALIB_SZ_CALIBVARIABLE ".calib.sz.calibvariable"
#define FX_KEY_CALIB_SZ_CALIBPARAM ".calib.sz.calibparam"
#define FX_KEY_CALIB_SZ_INPUTPARAMTYPE ".calib.sz.inputparamtype"
#define FX_KEY_CALIB_SZ_TERM ".calib.sz.term"
#define FX_KEY_CALIB_SZ_STRIKEFWDTERM ".calib.sz.strikefwdterm"
#define FX_KEY_CALIB_SZ_TERM_CALENDAR ".calib.sz.term.calendar"
#define FX_KEY_CALIB_SZ_TERM_SLIDINGRULE ".calib.sz.slidingrule"
#define FX_KEY_CALIB_SZ_TERM_SPOTLAG ".calib.sz.term.spotlag"
#define FX_KEY_CALIB_SZ_TERM_DAYCOUNT ".calib.sz.term.daycount"
#define FX_KEY_CALIB_SZ_TERM_ISINCLUDELAST ".calib.sz.term.isincludelast"
#define FX_KEY_CALIB_SZ_CALENDAR ".calib.sz.calendar"
#define FX_KEY_CALIB_SZ_SLIDINGRULE ".calib.sz.slidingrule"
#define FX_KEY_CALIB_SZ_SPOTORFWD ".calib.sz.spotfwd"
#define FX_KEY_CALIB_SZ_ISFITATM ".calib.sz.isfitatm"
#define FX_KEY_CALIB_SZ_ISDELTANEUTRAL ".calib.sz.isdeltaneutral"
//#define FX_KEY_CALIB_SZ_TERMBETA ".calib.sz.termbeta"
#define FX_KEY_CALIB_SZ_OPTIONTYPE_10DL ".calib.sz.optiontype.10dl"
#define FX_KEY_CALIB_SZ_OPTIONTYPE_25DL ".calib.sz.optiontype.25dl"
#define FX_KEY_CALIB_SZ_OPTIONTYPE_ATM ".calib.sz.optiontype.atm"
#define FX_KEY_CALIB_SZ_OPTIONTYPE_25DH ".calib.sz.optiontype.25dh"
#define FX_KEY_CALIB_SZ_OPTIONTYPE_10DH ".calib.sz.optiontype.10dh"
#define FX_KEY_CALIB_SZ_WEIGHT_10DL ".calib.sz.weight.10dl"
#define FX_KEY_CALIB_SZ_WEIGHT_25DL ".calib.sz.weight.25dl"
#define FX_KEY_CALIB_SZ_WEIGHT_ATM ".calib.sz.weight.atm"
#define FX_KEY_CALIB_SZ_WEIGHT_25DH ".calib.sz.weight.25dh"
#define FX_KEY_CALIB_SZ_WEIGHT_10DH ".calib.sz.weight.10dh"
#define FX_KEY_CALIB_SZ_DIFFVOLV0 ".calib.sz.diffvolv0.param"

#define FX_KEY_CALIB_SZ_OPT_ISCALIBRATEBETA ".calib.sz.iscalibratebeta"
#define FX_KEY_CALIB_SZ_OPT_ISCALIBRATETHETA ".calib.sz.iscalibratetheta"
#define FX_KEY_CALIB_SZ_OPT_ISCALIBRATEKAPPA ".calib.sz.iscalibratekappa"
#define FX_KEY_CALIB_SZ_OPT_ISCALIBRATEEPSILON ".calib.sz.iscalibrateepsilon"
#define FX_KEY_CALIB_SZ_OPT_MAX_BETA ".calib.sz.optimize.boundary.max.beta"
#define FX_KEY_CALIB_SZ_OPT_MIN_BETA ".calib.sz.optimize.boundary.min.beta"
#define FX_KEY_CALIB_SZ_OPT_MAX_THETA ".calib.sz.optimize.boundary.max.theta"
#define FX_KEY_CALIB_SZ_OPT_MIN_THETA ".calib.sz.optimize.boundary.min.theta"
#define FX_KEY_CALIB_SZ_OPT_MAX_KAPPA ".calib.sz.optimize.boundary.max.kappa"
#define FX_KEY_CALIB_SZ_OPT_MIN_KAPPA ".calib.sz.optimize.boundary.min.kappa"
#define FX_KEY_CALIB_SZ_OPT_MAX_EPSILON ".calib.sz.optimize.boundary.max.epsilon"
#define FX_KEY_CALIB_SZ_OPT_MIN_EPSILON ".calib.sz.optimize.boundary.min.epsilon"
#define FX_KEY_CALIB_SZ_OPT_METHOD_TYPE ".calib.sz.marketskewvol.optimize.method.type"
#define FX_KEY_CALIB_SZ_OPT_MAX_ITERATION ".calib.sz.optimize.max.iteration"
#define FX_KEY_CALIB_SZ_OPT_MAX_STATIONARY_ITERATION ".calib.sz.optimize.max.stationary.state.iteration"
#define FX_KEY_CALIB_SZ_OPT_ROOT_EPSILON ".calib.sz.optimize.dataInstance.epsilon"
#define FX_KEY_CALIB_SZ_OPT_FUNCTION_EPSILON ".calib.sz.optimize.function.epsilon"
#define FX_KEY_CALIB_SZ_OPT_GRADIENT_NORM_EPSILON ".calib.sz.optimize.gradient.norm.epsilon"
#define FX_KEY_CALIB_SZ_OPT_ISOPTIMIZE ".calib.sz.optimize.isoptimize"

#define FX_KEY_CALIB_SZ_INITIALBETA ".calib.sz.beta"
#define FX_KEY_CALIB_SZ_INITIALTHETA ".calib.sz.theta"
#define FX_KEY_CALIB_SZ_INITIALKAPPA ".calib.sz.kappa"
#define FX_KEY_CALIB_SZ_INITIALEPSILON ".calib.sz.epsilon"

//********************************
//
// property key end
//
//********************************

#endif