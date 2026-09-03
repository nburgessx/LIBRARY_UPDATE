/*! @file
    @brief Constant data declare file
*/
//  2008, AlgoQuantHub.

#ifndef LADefinitionsHW_h
#define LADefinitionsHW_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADefinitionsHW.h
//
//  DESCRIPTION :       
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include "LADefinitions.h"

//********************************
//
// property key start
//
//********************************

// common for all curreny
#define KEY_HW_INTEGRAL_DIVNUM   "sde.hw.integral.divnum" 
#define KEY_HW_INTEGRAL_TIMEGRID   "sde.hw.integral.timegrid" 
#define KEY_HW_CORRELATION_FILE   "sde.hw.correlation.file" 
#define KEY_HW_CANONIC_FILE   "sde.hw.canonic.file" 
#define KEY_HW_CANONIC_30_360_FILE   "sde.hw.canonic.30/360.file" 
#define KEY_HW_MEASURE   "sde.hw.measure" 
#define KEY_HW_CROSS_ISDMY   "sde.hw.cross.isdmy" 
#define KEY_HW_GRID_EXTRATENOR   "sde.hw.grid.extratenor" 
#define KEY_HW_GRID_ISEXTRATENORUSE   "sde.hw.grid.isextratenoruse"
#define KEY_HW_IS3F   "sde.hw.is3f"
#define KEY_HW_QUANTOADJUST_THRESHOLD   "sde.hw.quantoadjust.threshold"

#define CONTEXT_KEY_HW_TENOR   "tenor" 
#define CONTEXT_KEY_HW_DELTATENOR   "deltatenor" 
#define CONTEXT_KEY_HW_30_360_TENOR   "30/360tenor" 
#define CONTEXT_KEY_HW_TMAX   "Tmax" 

// key for curreny
#define STATIC_DATA_KEY_HW_FUNCTION_NAME   ".sde.hw.function.name" 
#define STATIC_DATA_KEY_HW_DRIFT   ".sde.hw.drift" 
#define STATIC_DATA_KEY_HW_DRIFT_ESTIMATE_TYPE   ".sde.hw.drift.estimatetype" 
#define STATIC_DATA_KEY_HW_TYPE   ".sde.hw.type" 
#define STATIC_DATA_KEY_HW_INTEGRAL_TYPE   ".sde.hw.integral.type" 
#define STATIC_DATA_KEY_HW_INTEGRAL_STEP   ".sde.hw.integral.step" 
#define STATIC_DATA_KEY_HW_CORRELATION_TYPE   ".sde.hw.correlation.type" 
#define STATIC_DATA_KEY_HW_VOLATILITY_TYPE   ".sde.hw.volatility.type" 
#define STATIC_DATA_KEY_HW_PARAMETER_FILE   ".sde.hw.parameter.file" 
#define STATIC_DATA_KEY_HW_MEANREV_ISTIMEDEPENDENT   ".sde.hw.meanrev.istimedependent"
#define STATIC_DATA_KEY_HW_ISZEROVOL   ".sde.hw.iszerovol"

// official risk 
// cap vega
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_HW_PARAMETER_FILE   ".risk.official.vol.capvega.hw.parameter.file" 
// swaptionvega
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_HW_PARAMETER_FILE   ".risk.official.vol.swaptionvega.hw.parameter.file" 

// front risk
// ir delta
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_PARALLEL_HW_PARAMETER_FILE   ".risk.front.yield.irdelta.sce1.parallel.hw.parameter.file" 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE1_GRID_HW_PARAMETER_FILE   ".risk.front.yield.irdelta.sce1.grid.hw.parameter.file." 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_PARALLEL_HW_PARAMETER_FILE   ".risk.front.yield.irdelta.sce2.parallel.hw.parameter.file" 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SCE2_GRID_HW_PARAMETER_FILE   ".risk.front.yield.irdelta.sce2.grid.hw.parameter.file." 
// ir shift delta
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_PARALLEL_HW_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce1.parallel.hw.parameter.file." 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE1_GRID_HW_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce1.grid.hw.parameter.file." 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_PARALLEL_HW_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce2.parallel.hw.parameter.file." 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCE2_GRID_HW_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.sce2.grid.hw.parameter.file." 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BASE_HW_PARAMETER_FILE   ".risk.front.yield.irshiftdelta.base.hw.parameter.file." 

// basis delta
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_PARALLEL_HW_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce1.parallel.hw.parameter.file" 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE1_GRID_HW_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce1.grid.hw.parameter.file." 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_PARALLEL_HW_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce2.parallel.hw.parameter.file" 
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SCE2_GRID_HW_PARAMETER_FILE   ".risk.front.yield.basisdelta.sce2.grid.hw.parameter.file." 
// ir vega
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_PARALLEL_HW_PARAMETER_FILE   ".risk.front.vol.irvega.sce1.parallel.hw.parameter.file" 
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE1_GRID_HW_PARAMETER_FILE   ".risk.front.vol.irvega.sce1.grid.hw.parameter.file." 
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_PARALLEL_HW_PARAMETER_FILE   ".risk.front.vol.irvega.sce2.parallel.hw.parameter.file" 
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SCE2_GRID_HW_PARAMETER_FILE   ".risk.front.vol.irvega.sce2.grid.hw.parameter.file." 
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_HW_MATRIX_FILE  ".risk.front.vol.irvega.matrix.file" 


// calibration
#define STATIC_DATA_KEY_CALIB_HW_INITIALMEANREVERSION   ".calib.hw.initialmeanreversion." 
#define STATIC_DATA_KEY_CALIB_HW_FITTINGTARGET   ".calib.hw.fittingtarget." 
#define STATIC_DATA_KEY_CALIB_HW_OPTIONMATURITY   ".calib.hw.optionmaturity." 
#define STATIC_DATA_KEY_CALIB_HW_SWAPTENOR   ".calib.hw.swaptenor." 
#define STATIC_DATA_KEY_CALIB_HW_DAYCOUNT_GRID   ".calib.hw.daycount." 
#define STATIC_DATA_KEY_CALIB_HW_FREQUENCY_GRID   ".calib.hw.frequency." 
#define STATIC_DATA_KEY_CALIB_HW_NOTICEPERIOD_GRID   ".calib.hw.noticeperiod." 
#define STATIC_DATA_KEY_CALIB_HW_FIXINGCALENDAR_GRID   ".calib.hw.fixingcalendar." 
#define STATIC_DATA_KEY_CALIB_HW_PAYMENTCALENDAR_GRID   ".calib.hw.paymentcalendar." 
#define STATIC_DATA_KEY_CALIB_HW_SLIDINGRULE_GRID   ".calib.hw.slidingrule." 
#define STATIC_DATA_KEY_CALIB_HW_STRIKE_GRID   ".calib.hw.strike." 
#define STATIC_DATA_KEY_CALIB_HW_OPTIONTYPE_GRID   ".calib.hw.optiontype." 
#define STATIC_DATA_KEY_CALIB_HW_WEIGHT_GRID   ".calib.hw.weight." 
#define STATIC_DATA_KEY_CALIB_HW_INITIALVOL   ".calib.hw.initialvol."
#define STATIC_DATA_KEY_CALIB_HW_CALIBVARIABLE   ".calib.hw.calibvariable."
#define STATIC_DATA_KEY_CALIB_HW_ISZEROVOL   ".calib.hw.iszerovol"
#define STATIC_DATA_KEY_CALIB_HW_INITIALPARAM_FILE ".calib.hw.initialparam.file"
#define STATIC_DATA_KEY_CALIB_HW_INITIALPARAM_ISFILEREAD ".calib.hw.initialparam.isfileread"
#define STATIC_DATA_KEY_CALIB_HW_SWAPTION_VOLTYPE ".calib.hw.swaption.voltype"
#define STATIC_DATA_KEY_CALIB_HW_SWAPTION_MARKET_FORWARDSHIFT ".calib.hw.swaption.market.forwardshift"

//********************************
//
// property key end
//
//********************************

#endif