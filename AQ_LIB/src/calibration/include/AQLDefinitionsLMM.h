/*! @file
    @brief Constant data declare file
*/

#ifndef AQLDefinitionsLMM_h
#define AQLDefinitionsLMM_h

#include "AQLDefinitions.h"

//********************************
//
// property key start
//
//********************************

// common for all curreny
#define KEY_LMM_INTEGRAL_DIVNUM   "sde.lmm.integral.divnum" 
#define KEY_LMM_INTEGRAL_TIMEGRID   "sde.lmm.integral.timegrid" 
#define KEY_LMM_CORRELATION_FILE   "sde.lmm.correlation.file" 
#define KEY_LMM_CANONIC_FILE   "sde.lmm.canonic.file" 
#define KEY_LMM_CANONIC_30_360_FILE   "sde.lmm.canonic.30/360.file" 
#define KEY_LMM_MEASURE   "sde.lmm.measure" 
#define KEY_LMM_CROSS_ISDMY   "sde.lmm.cross.isdmy" 
#define KEY_LMM_GRID_EXTRATENOR   "sde.lmm.grid.extratenor" 
#define KEY_LMM_GRID_ISEXTRATENORUSE   "sde.lmm.grid.isextratenoruse" 
#define KEY_LMM_CANONICALGRID_EXTRATENOR   "sde.lmm.canonicalgrid.extratenor" 
#define KEY_LMM_CANONICALGRID_ISEXTRATENORUSE   "sde.lmm.canonicalgrid.isextratenoruse" 

#define CONTEXT_KEY_LMM_TENOR   "tenor" 
#define CONTEXT_KEY_LMM_DELTATENOR   "deltatenor" 
#define CONTEXT_KEY_LMM_30_360_TENOR   "30/360tenor" 
#define CONTEXT_KEY_LMM_TMAX   "Tmax" 

// key for curreny
#define STATIC_DATA_KEY_LMM_FUNCTION_NAME   ".sde.lmm.function.name" 
#define STATIC_DATA_KEY_LMM_CANONIC_FREQ   ".sde.lmm.canonic.freq" 
#define STATIC_DATA_KEY_LMM_DRIFT   ".sde.lmm.drift" 
#define STATIC_DATA_KEY_LMM_DRIFT_SINGLE   ".sde.lmm.drift.single" 
#define STATIC_DATA_KEY_LMM_DRIFT_ESTIMATE_TYPE   ".sde.lmm.drift.estimatetype" 
#define STATIC_DATA_KEY_LMM_TYPE   ".sde.lmm.type" 
#define STATIC_DATA_KEY_LMM_INTEGRAL_TYPE   ".sde.lmm.integral.type" 
#define STATIC_DATA_KEY_LMM_INTEGRAL_STEP   ".sde.lmm.integral.step" 
#define STATIC_DATA_KEY_LMM_INTEGRAL_ISPC   ".sde.lmm.integral.ispc" 
#define STATIC_DATA_KEY_LMM_SKEW_SINGLE_FILE   ".sde.lmm.skew.single.file" 
#define STATIC_DATA_KEY_LMM_SKEW_CROSS_FILE   ".sde.lmm.skew.cross.file" 
#define STATIC_DATA_KEY_LMM_CONSTSHIFT_SINGLE_FILE	".sde.lmm.constshift.single.file" 
#define STATIC_DATA_KEY_LMM_CONSTSHIFT_CROSS_FILE	".sde.lmm.constshift.cross.file" 
#define STATIC_DATA_KEY_LMM_MAXTERM_FILE   ".sde.lmm.maxterm.file" 
#define STATIC_DATA_KEY_LMM_YIELD_INTERPOLATION    ".sde.lmm.yield.interpolation" 
#define STATIC_DATA_KEY_LMM_YIELD_DAYCOUNT    ".sde.lmm.yield.daycount" 
#define STATIC_DATA_KEY_LMM_YIELD_FREQ    ".sde.lmm.yield." 
#define STATIC_DATA_KEY_LMM_CORRELATION_TYPE   ".sde.lmm.correlation.type" 
#define STATIC_DATA_KEY_LMM_CORRELATION_SIGLE_FACTOR_NUM   ".sde.lmm.correlation.single.factornum" 
#define STATIC_DATA_KEY_LMM_CORRELATION_CROSS_FACTOR_NUM   ".sde.lmm.correlation.cross.factornum" 
#define STATIC_DATA_KEY_LMM_CORRELATION_ISOPTIMIZATION   ".sde.lmm.correlation.isoptimization" 
#define STATIC_DATA_KEY_LMM_CORRELATION_INTERPOLATION   ".sde.lmm.correlation.interpolation" 
#define STATIC_DATA_KEY_LMM_CORRELATION_ISMULTIVOL   ".sde.lmm.correlation.ismultivolatility" 
#define STATIC_DATA_KEY_LMM_CORRELATION_DATA_FILE   ".sde.lmm.correlation.data.file" 
#define STATIC_DATA_KEY_LMM_CORRELATION_FACTOR_FILE   ".sde.lmm.correlation.factor.file" 
#define STATIC_DATA_KEY_LMM_CORRELATION_FUNC_FILE   ".sde.lmm.correlation.function.file" 
#define STATIC_DATA_KEY_LMM_VOLATILITY_TYPE   ".sde.lmm.volatility.type" 
#define STATIC_DATA_KEY_LMM_VOLATILITY_INTERPOLATION   ".sde.lmm.volatility.interpolation" 
#define STATIC_DATA_KEY_LMM_VOLATILITY_DATA_FILE   ".sde.lmm.volatility.data.file" 
#define STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_CALIB_FILE   ".sde.lmm.volatility.function.calib.file" 
#define STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_FILE   ".sde.lmm.volatility.function.adj.file" 
#define STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_FREQUENCY   ".sde.lmm.volatility.function.adj.frequency" 
#define STATIC_DATA_KEY_LMM_VOLATILITY_FUNC_ADJ_INTERPOLATION   ".sde.lmm.volatility.function.adj.interpolation" 
#define STATIC_DATA_KEY_LMM_PATH_CAPRATIO   ".sde.lmm.path.capratio" 
#define STATIC_DATA_KEY_LMM_PATH_INTERPOLATION   ".sde.lmm.path.interpolation" 

// calibration
#define STATIC_DATA_KEY_CALIB_LMM_IS_EXTRATENOR_CALIB   ".calib.lmm.isextratenorcalib" 
#define STATIC_DATA_KEY_CALIB_LMM_SKEW_SINGLE_FILE   ".calib.lmm.skew.single.file" 
#define STATIC_DATA_KEY_CALIB_LMM_SKEW_CROSS_FILE   ".calib.lmm.skew.cross.file"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_FUNC_PARAM_FILE   ".calib.lmm.correlation.function.param.file"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_FUNC_TMAX_FILE   ".calib.lmm.correlation.function.tmax.file"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_FUNC_TYPE_FILE   ".calib.lmm.correlation.function.type.file"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_TARGET_FACTOR_FILE   ".calib.lmm.correlation.optimize.target.factor.file"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_TARGET_TYPE   ".calib.lmm.correlation.optimize.target.type"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_METHOD_TYPE   ".calib.lmm.correlation.optimize.method.type"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_BOUNDARY_MAX   ".calib.lmm.correlation.optimize.boundary.max"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_BOUNDARY_MIN   ".calib.lmm.correlation.optimize.boundary.min"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_MAX_ITERATION   ".calib.lmm.correlation.optimize.max.iteration"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_MAX_STATIONARY_STATE_ITERATION   ".calib.lmm.correlation.optimize.max.stationary.state.iteration"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_ROOT_EPSILON   ".calib.lmm.correlation.optimize.dataInstance.epsilon"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_FUNCTION_EPSILON   ".calib.lmm.correlation.optimize.function.epsilon"
#define STATIC_DATA_KEY_CALIB_LMM_CORRELATION_OPT_GRADIENT_NORM_EPSILON   ".calib.lmm.correlation.optimize.gradient.norm.epsilon"

#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_VOL_FILE ".calib.lmm.volatility.swaption.vol.file"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_APPROX_METHOD ".calib.lmm.volatility.swaption.approx.method"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_WEIGHT_FILE ".calib.lmm.volatility.swaption.weight.file"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_VOL_FILE ".calib.lmm.volatility.cap.vol.file"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_TERM   ".calib.lmm.volatility.swaption.term"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_TENOR   ".calib.lmm.volatility.swaption.tenor"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TERM    ".calib.lmm.volatility.cap.term"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_TENOR    ".calib.lmm.volatility.cap.tenor"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_DAYCOUNT   ".calib.lmm.volatility.libor.daycount"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_SLIDINGRULE   ".calib.lmm.volatility.libor.slidingrule"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_CALENDAR   ".calib.lmm.volatility.libor.calendar"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_SPOTLAG   ".calib.lmm.volatility.libor.spotlag"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_LIBOR_FREQUENCY   ".calib.lmm.volatility.libor.frequency"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_DAYCOUNT   ".calib.lmm.volatility.cap.daycount"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_SLIDINGRULE   ".calib.lmm.volatility.cap.slidingrule"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_CALENDAR   ".calib.lmm.volatility.cap.calendar"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_SPOTLAG   ".calib.lmm.volatility.cap.spotlag"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_CAP_FREQUENCY   ".calib.lmm.volatility.cap.frequency"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_DAYCOUNT   ".calib.lmm.volatility.swaption.daycount"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_SLIDINGRULE   ".calib.lmm.volatility.swaption.slidingrule"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_CALENDAR   ".calib.lmm.volatility.swaption.calendar"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_SPOTLAG   ".calib.lmm.volatility.swaption.spotlag"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_FREQUENCY   ".calib.lmm.volatility.swaption.frequency"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_VOLTYPE   ".calib.lmm.volatility.swaption.voltype"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_MARKET_FORWARDSHIFT   ".calib.lmm.volatility.swaption.market.forwardshift"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_FORECASTCURVE   ".calib.lmm.volatility.swaption.forecastcurve"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_SWAPTION_DISCOUNTCURVE   ".calib.lmm.volatility.swaption.discountcurve"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INIT_V_FILE   ".calib.lmm.volatility.optimize.initial.v.file"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INIT_F_FILE   ".calib.lmm.volatility.optimize.initial.f.file"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INIT_G_FILE   ".calib.lmm.volatility.optimize.initial.g.file"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_INTERPOLATION_G   ".calib.lmm.volatility.optimize.interpolation.g"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_FREQUENCY_G   ".calib.lmm.volatility.optimize.frequency.g"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MAX_V   ".calib.lmm.volatility.optimize.boundary.max.v"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MIN_V   ".calib.lmm.volatility.optimize.boundary.min.v"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MAX_F   ".calib.lmm.volatility.optimize.boundary.max.f"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MIN_F   ".calib.lmm.volatility.optimize.boundary.min.f"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MAX_G   ".calib.lmm.volatility.optimize.boundary.max.g"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_BOUNDARY_MIN_G   ".calib.lmm.volatility.optimize.boundary.min.g"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_SMOOTH_BOUNDARY_G   ".calib.lmm.volatility.optimize.smooth.boundary.g"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_LEBENBERG_ADJUST_TOR   ".calib.lmm.volatility.optimize.levenberg.adjust.tor"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_LEBENBERG_ADJUST_V   ".calib.lmm.volatility.optimize.levenberg.adjust.v"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_LEBENBERG_ADJUST_F   ".calib.lmm.volatility.optimize.levenberg.adjust.f"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_MAX_ITERATION   ".calib.lmm.volatility.optimize.max.iteration"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_MAX_STATIONARY_STATE_ITERATION   ".calib.lmm.volatility.optimize.max.stationary.state.iteration"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_ROOT_EPSILON   ".calib.lmm.volatility.optimize.dataInstance.epsilon"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_FUNCTION_EPSILON   ".calib.lmm.volatility.optimize.function.epsilon"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_GRADIENT_NORM_EPSILON   ".calib.lmm.volatility.optimize.gradient.norm.epsilon"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_METHOD_TYPE   ".calib.lmm.volatility.optimize.method.type"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_VEGA_WEIGHT    ".calib.lmm.volatility.optimize.vega.weight"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_CAPFLOORS_WEIGHT   ".calib.lmm.volatility.optimize.capfloors.weight"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_SWAPTIONS_WEIGHT   ".calib.lmm.volatility.optimize.swaptions.weight"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_COST_FUNC_MODE   ".calib.lmm.volatility.optimize.cost.function.mode"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_COST_FUNC_TARGET   ".calib.lmm.volatility.optimize.cost.function.target"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_NUM_SMALL_STEPS   ".calib.lmm.volatility.optimize.num.small.step"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_LOOP_NUM   ".calib.lmm.volatility.optimize.loop.num"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_TARGET_V   ".calib.lmm.volatility.optimize.target.v"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_TARGET_F   ".calib.lmm.volatility.optimize.target.f"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_OPT_TARGET_G   ".calib.lmm.volatility.optimize.target.g"
#define STATIC_DATA_KEY_CALIB_LMM_VOLATILITY_DO_OPTIMIZE    ".calib.lmm.volatility.do.optimize"   
#define STATIC_DATA_KEY_CALIB_LMM_ISRESULTOUT_SWAPTION    ".calib.lmm.isresultout.swaption"   

// official risk 
// cap vega
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_LMM_VOLATILITY_FUNC_CALIB_FILE   ".risk.official.vol.capvega.lmm.calib.file" 
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_LMM_VOLATILITY_FUNC_ADJ_FILE   ".risk.official.vol.capvega.lmm.adj.file" 
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_LMM_VOLATILITY_TYPE   ".risk.official.vol.capvega.lmm.type" 
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_LMM_MAXTERM_FILE ".risk.official.vol.capvega.lmm.maxterm.file"
// swaptionvega
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_LMM_VOLATILITY_FUNC_CALIB_FILE   ".risk.official.vol.swaptionvega.lmm.calib.file" 
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_LMM_VOLATILITY_FUNC_ADJ_FILE   ".risk.official.vol.swaptionvega.lmm.adj.file" 
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_LMM_VOLATILITY_TYPE   ".risk.official.vol.swaptionvega.lmm.type" 
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_LMM_MAXTERM_FILE ".risk.official.vol.swaptionvega.lmm.maxterm.file"


// front risk
// ir vega
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_LMM_TYPE  ".risk.front.vol.irvega.lmm.type" 

//********************************
//
// property key end
//
//********************************

#endif