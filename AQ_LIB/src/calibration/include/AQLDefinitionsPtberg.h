/*! @file
    @brief Constant data declare file
*/
//  2007, AlgoQuantHub.

#ifndef AQLDefinitionsPtberg_h
#define AQLDefinitionsPtberg_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADefinitionsantPtberg.h
//
//  DESCRIPTION :       
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include "AQLDefinitions.h"

//********************************
//
// property key start
//
//********************************
// common for all fx
#define KEY_PTBERG_SKEW_ISINCLUDE   "sde.ptberg.skew.isinclude"
#define KEY_PTBERG_IS3F   "sde.ptberg.is3f"
#define KEY_PTBERG_QUANTOADJUST_THRESHOLD   "sde.ptberg.quantoadjust.threshold"
// key for fx
#define FX_KEY_PTBERG_FUNCTION_NAME   ".sde.ptberg.function.name"
#define FX_KEY_PTBERG_DRIFT   ".sde.ptberg.drift"
#define FX_KEY_PTBERG_DRIFT_ESTIMATE_TYPE   ".sde.ptberg.drift.estimatetype"
#define FX_KEY_PTBERG_TYPE   ".sde.ptberg.type"
#define FX_KEY_PTBERG_INTEGRAL_TYPE   ".sde.ptberg.integral.type"
#define FX_KEY_PTBERG_DD_INTEGRAL_TYPE   ".sde.ptberg.dd.integral.type"
#define FX_KEY_PTBERG_INTEGRAL_STEP   ".sde.ptberg.integral.step"
#define FX_KEY_PTBERG_ISCHANGEVARIAVLES   ".sde.ptberg.ischangevariables"
#define FX_KEY_PTBERG_VOLATILITY_TYPE   ".sde.ptberg.volatility.type"
#define FX_KEY_PTBERG_VOLATILITY_INTERPOLATION   ".sde.ptberg.volatility.interpolation"
#define FX_KEY_PTBERG_VOLATILITY_DATA_FILE   ".sde.ptberg.correlation.data.file"
#define FX_KEY_PTBERG_VOLATILITY_CALIB_FILE   ".sde.ptberg.volatility.function.file"
#define FX_KEY_PTBERG_VOLATILITY_CALIB_FILE_FORWORDGRIDS_ISINCLUDE ".sde.ptberg.volatility.function.file.forwordgrids.isinclude"	// for XLL_PLUS
#define FX_KEY_PTBERG_MAXMULTIPLIER   ".sde.ptberg.maxmultiplier"
#define FX_KEY_PTBERG_ISEXTRAADJ   ".sde.ptberg.isextraadj"

// official risk 
// fx delta
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_PTBERG_UPSHIFT_CALIB_FILE   ".risk.official.fx.delta.upshift.ptberg.file"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_PTBERG_DOWNSHIFT_CALIB_FILE   ".risk.official.fx.delta.downshift.ptberg.file"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_PTBERG_TYPE   ".risk.official.fx.delta.ptberg.type"
// fx atmparallelvega
#define FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_PTBERG_CALIB_FILE   ".risk.official.vol.fxatmparallelvega.ptberg.file"
#define FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_PTBERG_TYPE   ".risk.official.vol.fxatmparallelvega.ptberg.type"
// front risk
// irdelta
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.yield.irdelta.sce1.parallel.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_GRID_PTBERG_PARAMETER_FILE   ".risk.front.yield.irdelta.sce1.grid.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.yield.irdelta.sce2.parallel.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_GRID_PTBERG_PARAMETER_FILE   ".risk.front.yield.irdelta.sce2.grid.ptberg.parameter.file."
// irshiftdelta
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce1.parallel.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_GRID_PTBERG_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce1.grid.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce2.parallel.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_GRID_PTBERG_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce2.grid.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BASE_PTBERG_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.base.ptberg.parameter.file."
// basisdelta
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce1.parallel.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_GRID_PTBERG_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce1.grid.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce2.parallel.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_GRID_PTBERG_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce2.grid.ptberg.parameter.file."
// irvega
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.vol.irvega.sce1.parallel.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_GRID_PTBERG_PARAMETER_FILE   ".risk.front.vol.irvega.sce1.grid.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.vol.irvega.sce2.parallel.ptberg.parameter.file."
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_GRID_PTBERG_PARAMETER_FILE   ".risk.front.vol.irvega.sce2.grid.ptberg.parameter.file."
// fxdelta
#define FX_KEY_RISK_FRONT_FX_DELTA_SCE1_PTBERG_PARAMETER_FILE   ".risk.front.fx.delta.sce1.ptberg.parameter.file"
#define FX_KEY_RISK_FRONT_FX_DELTA_SCE2_PTBERG_PARAMETER_FILE   ".risk.front.fx.delta.sce2.ptberg.parameter.file"
// fxvega
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE1_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.vol.fxvega.sce1.parallel.ptberg.parameter.file"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE1_GRID_PTBERG_PARAMETER_FILE   ".risk.front.vol.fxvega.sce1.grid.ptberg.parameter.file."
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE2_PARALLEL_PTBERG_PARAMETER_FILE   ".risk.front.vol.fxvega.sce2.parallel.ptberg.parameter.file"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SCE2_GRID_PTBERG_PARAMETER_FILE   ".risk.front.vol.fxvega.sce2.grid.ptberg.parameter.file."

//calibration
#define KEY_CALIB_PTBERG_IS3F   "calib.ptberg.is3f"
#define FX_KEY_CALIB_PTBERG_BETACAP   ".calib.ptberg.betacap" 
#define FX_KEY_CALIB_PTBERG_BETAFLOOR   ".calib.ptberg.betafloor" 
#define FX_KEY_CALIB_PTBERG_FIXEDBETA ".calib.ptberg.fixedbeta"
#define FX_KEY_CALIB_PTBERG_FIXEDVOL ".calib.ptberg.fixedvol"
#define FX_KEY_CALIB_PTBERG_INITIALPARAM_ISFILEREAD ".calib.ptberg.initialparam.isfileread"
#define FX_KEY_CALIB_PTBERG_INITIALPARAM_FILE ".calib.ptberg.initialparam.file"
#define FX_KEY_CALIB_PTBERG_INITIALBETA ".calib.ptberg.initialbeta"
#define FX_KEY_CALIB_PTBERG_INITIALVOL ".calib.ptberg.initialvol"
#define FX_KEY_CALIB_PTBERG_CALIBVARIABLE ".calib.ptberg.calibvariable"
#define FX_KEY_CALIB_PTBERG_CALIBPARAM ".calib.ptberg.calibparam"
#define FX_KEY_CALIB_PTBERG_INPUTPARAMTYPE ".calib.ptberg.inputparamtype"
#define FX_KEY_CALIB_PTBERG_TERM ".calib.ptberg.term"
#define FX_KEY_CALIB_PTBERG_STRIKEFWDTERM ".calib.ptberg.strikefwdterm"
#define FX_KEY_CALIB_PTBERG_TERM_CALENDAR ".calib.ptberg.term.calendar"
#define FX_KEY_CALIB_PTBERG_TERM_SLIDINGRULE ".calib.ptberg.term.slidingrule"
#define FX_KEY_CALIB_PTBERG_TERM_SPOTLAG ".calib.ptberg.term.spotlag"
#define FX_KEY_CALIB_PTBERG_TERM_DAYCOUNT ".calib.ptberg.term.daycount"
#define FX_KEY_CALIB_PTBERG_TERM_ISINCLUDELAST ".calib.ptberg.term.isincludelast"
#define FX_KEY_CALIB_PTBERG_CALENDAR ".calib.ptberg.calendar"
#define FX_KEY_CALIB_PTBERG_SLIDINGRULE ".calib.ptberg.slidingrule"
#define FX_KEY_CALIB_PTBERG_SPOTORFWD ".calib.ptberg.spotorfwd"
#define FX_KEY_CALIB_PTBERG_ISFITATM ".calib.ptberg.isfitatm"
#define FX_KEY_CALIB_PTBERG_ISDELTANEUTRAL ".calib.ptberg.isdeltaneutral"
#define FX_KEY_CALIB_PTBERG_TERMBETA ".calib.ptberg.termbeta"
#define FX_KEY_CALIB_PTBERG_OPTIONTYPE_10DL ".calib.ptberg.optiontype.10dl"
#define FX_KEY_CALIB_PTBERG_OPTIONTYPE_25DL ".calib.ptberg.optiontype.25dl"
#define FX_KEY_CALIB_PTBERG_OPTIONTYPE_ATM ".calib.ptberg.optiontype.atm"
#define FX_KEY_CALIB_PTBERG_OPTIONTYPE_25DH ".calib.ptberg.optiontype.25dh"
#define FX_KEY_CALIB_PTBERG_OPTIONTYPE_10DH ".calib.ptberg.optiontype.10dh"
#define FX_KEY_CALIB_PTBERG_WEIGHT_10DL ".calib.ptberg.weight.10dl"
#define FX_KEY_CALIB_PTBERG_WEIGHT_25DL ".calib.ptberg.weight.25dl"
#define FX_KEY_CALIB_PTBERG_WEIGHT_ATM ".calib.ptberg.weight.atm"
#define FX_KEY_CALIB_PTBERG_WEIGHT_25DH ".calib.ptberg.weight.25dh"
#define FX_KEY_CALIB_PTBERG_WEIGHT_10DH ".calib.ptberg.weight.10dh"

//3f
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_10DH_WEIGHT ".calib.ptberg.3f.marketskewvol.optimize.10dh.weight" 
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_10DL_WEIGHT ".calib.ptberg.3f.marketskewvol.optimize.10dl.weight"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_ATM_WEIGHT ".calib.ptberg.3f.marketskewvol.optimize.atm.weight"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_25DL_WEIGHT ".calib.ptberg.3f.marketskewvol.optimize.25dl.weight"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_25DH_WEIGHT ".calib.ptberg.3f.marketskewvol.optimize.25dh.weight"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_VOL ".calib.ptberg.3f.marketskewvol.optimize.initial.vol"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_INITIAL_SKEW ".calib.ptberg.3f.marketskewvol.optimize.initial.skew"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_SKEW ".calib.ptberg.3f.marketskewvol.optimize.boundary.max.skew"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_SKEW ".calib.ptberg.3f.marketskewvol.optimize.boundary.min.skew"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MAX_VOL ".calib.ptberg.3f.marketskewvol.optimize.boundary.max.vol"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_BOUNDARY_MIN_VOL ".calib.ptberg.3f.marketskewvol.optimize.boundary.min.vol"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAXITERATION ".calib.ptberg.3f.marketskewvol.optimize.max.iteration"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_MAX_STATIONARY_ITERATION ".calib.ptberg.3f.marketskewvol.optimize.max.stationary.state.iteration"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_ROOT_EPSILON ".calib.ptberg.3f.marketskewvol.optimize.dataInstance.epsilon"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_FUNCTION_EPSILON ".calib.ptberg.3f.marketskewvol.optimize.function.epsilon"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_GRADIENT_NORM_EPSILON ".calib.ptberg.3f.marketskewvol.optimize.gradient.norm.epsilon"
#define FX_KEY_CALIB_PTBERG_3F_MARKETSKEWVOL_OPT_METHOD_TYPE ".calib.ptberg.3f.marketskewvol.optimize.method.type"

#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_INITIAL_SIGMA ".calib.ptberg.3f.sigmabeta.optimize.initial.sigma"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_INITIAL_BETA ".calib.ptberg.3f.sigmabeta.optimize.initial.beta"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_NUM_SMALL_STEP ".calib.ptberg.3f.sigmabeta.optimize.num.small.step"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_SIGMA ".calib.ptberg.3f.sigmabeta.optimize.boundary.max.sigma"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_SIGMA ".calib.ptberg.3f.sigmabeta.optimize.boundary.min.sigma"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MAX_BETA ".calib.ptberg.3f.sigmabeta.optimize.boundary.max.beta"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_BOUNDARY_MIN_BETA ".calib.ptberg.3f.sigmabeta.optimize.boundary.min.beta"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_ITERATION ".calib.ptberg.3f.sigmabeta.optimize.max.iteration"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_MAX_STATIONARY_ITERATION ".calib.ptberg.3f.sigmabeta.optimize.max.stationary.state.iteration"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_ROOT_EPSILON ".calib.ptberg.3f.sigmabeta.optimize.dataInstance.epsilon"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_FUNCTION_EPSILON ".calib.ptberg.3f.sigmabeta.optimize.function.epsilon"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_GRADIENT_NORM_EPSILON ".calib.ptberg.3f.sigmabeta.optimize.gradient.norm.epsilon"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_TARGET ".calib.ptberg.3f.sigmabeta.optimize.target"
#define FX_KEY_CALIB_PTBERG_3F_SIGMABETA_OPT_METHOD_TYPE ".calib.ptberg.3f.sigmabeta.optimize.method.type"
// TODO merge latest version
#define FX_KEY_CALIB_PTBERG_EXTRASTRIKE ".calib.ptberg.extrastrike."

//********************************
//
// property key end
//
//********************************

#endif