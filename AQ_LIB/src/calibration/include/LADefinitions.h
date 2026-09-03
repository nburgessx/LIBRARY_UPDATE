#pragma once

#include "AQLMathDefine.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"


typedef AQLStringVector::iterator StrItr;
typedef AQLStringVector::const_iterator CStrItr;
typedef std::vector<AQLStringVector> AQLStringMatrix;
typedef std::vector<AQLObjectHolder *>	EntityHolderPVec;
typedef std::vector<AQLObjectHolder>	EntityHolderVec;
typedef EntityHolderVec::iterator EntityHolItr; 
typedef EntityHolderVec::const_iterator CEntityHolItr;  

#define KEY_PV "PV"

//********************************
//
// argument key start
//
//********************************

#define ARG_KEY_REQUEST "-request"
#define ARG_KEY_CURRENCY "-currency"
#define ARG_KEY_SIMCURRENCY "-simcurrency"
#define ARG_KEY_CANONICALFREQ "-canonicalfreq"
#define ARG_KEY_SIMFREQ "-simfreq"
#define ARG_KEY_MAINTRADE "-maintrade"
#define ARG_KEY_FILENUM "-filenum"
#define ARG_KEY_MARKETID "-marketid"
#define ARG_KEY_MLIBID "-mlibid"
#define ARG_KEY_PROPERTIESID "-propertiesid"
#define ARG_KEY_APPMAT "-appmat"
#define ARG_KEY_OFFICIALRISK "-officialrisk"
#define ARG_KEY_FRONTRISK "-frontrisk"
#define ARG_KEY_BASESCENARIONUM "-basescenarionum"
#define ARG_KEY_GRIDRANGE "-gridrange"

#define ARG_KEY_DATAOUT "-dataout"
#define ARG_KEY_RESULTOUT "-resultout"
#define ARG_KEY_COMPOUNDEDRATEOUT "-compounded_rate_out"
#define ARG_KEY_NOCALIBTHREAD "-nocalibthread"
#define ARG_KEY_CALC "-calc"
#define ARG_KEY_CURVETYPE "-curveType"
#define ARG_KEY_ZEROCALC "-zerocalc"
#define ARG_KEY_ENTITYDUMP "-entitydump"
#define ARG_KEY_ASOFDATE "-asofdate"
#define ARG_KEY_SETTLEDATE "-settledate"
#define ARG_KEY_VALUEDATE "-valuedate"
#define ARG_KEY_LATECSVLOAD "-latecsvload"
#define ARG_KEY_CALENDAR_CITY "-calendar_city"
#define ARG_KEY_CALENDAR_TERM "-calendar_term"

//********************************
//
// argument key end
//
//********************************

#define CONTEXT_KEY_MAXTERM "maxterm"
#define CONTEXT_KEY_DEAL_MAXTERM "dealmaxterm"
#define CONTEXT_KEY_MAXTERMFREQ "maxtermfrequency"
#define CONTEXT_KEY_ASOFDATE "asofdate"
#define CONTEXT_KEY_TIMEGRID_DAYCOUNT "timegrid_daycount"
#define CONTEXT_KEY_FXENTIY_NAME_FORWARD  "fxentityname_forward"
#define CONTEXT_KEY_FXENTIY_NAME_FXSDE  "fxentityname_fxsde"
#define CONTEXT_KEY_PATHENTITY_NAME  "pathentityname"
#define CONTEXT_KEY_PLAINVANILLAENTITY_NAME  "plainvanillaentityname"
#define CONTEXT_KEY_DEAL_IRVOL "dealirvol"
#define CONTEXT_KEY_RISKGRID_ISOMITSTART "riskgridisomitstart"
#define CONTEXT_KEY_RISKGRID_ISOMITEND "riskgridisomitend"
#define CONTEXT_KEY_RISKENTITY_NAMES "riskentitynames"

#define CONTEXT_KEY_ISPRICER "ispricer"
#define CONTEXT_KEY_YIELDENTITY_NAME "yieldentityname"
#define CONTEXT_KEY_YIELDPROENTITY_NAME "yieldproentityname"
#define CONTEXT_KEY_VOLENTITY_NAME "volentityname"
#define CONTEXT_KEY_ISSETCURVEID "issetcurveid" // for vanilla addin
#define CONTEXT_KEY_ISEXCELREQUEST "isexcelrequest"

#define CONTEXT_KEY_SDE_TIMEGRID "SDETimeGrid"  // for XLL_PLUS
#define CONTEXT_KEY_SDE_YIELD "_YieldSDEIR"  // for XLL_PLUS
#define CONTEXT_KEY_SDE_YIELD_DATA "_YieldSDEAttr"  // for XLL_PLUS
#define CONTEXT_KEY_SDE_YIELD_INTER "_YieldSDEInter"  // for XLL_PLUS

#define CONTEXT_KEY_ISINCLUDECASH "isincludecash"
#define CONTEXT_KEY_INCLUDECASH_FROM "includecashfrom"

#define CONTEXT_KEY_FUNDINGCHANGE_FILE "fundingchange.file"

#define CONTEXT_KEY_CALENDAR_CITY "calendar_city"
#define CONTEXT_KEY_CALENDAR_TERM "calendar_term"

#define CONTEXT_KEY_SDE_YIELD_WITH_MARKET "_YieldSDEIRWithMarket"  // for XLL_PLUS
#define CONTEXT_KEY_USE_SDE_YIELD "useYieldSDEIR"  // for XLL_PLUS
#define CONTEXT_KEY_ISSETUPCREDIT "issetupcredit"

//********************************
//
// property key start
//
//********************************

// log
#define KEY_LOG_FILE  "log.file"
#define KEY_LOG_LEVEL  "log.level"

// object dump
#define KEY_ENTITY_DUMP_FILE "object.dump.file"


// key for each currency or fx
#define KEY_CALENDER_FILE "calender.file"
#define KEY_DEAL_FILE "deal.file"
#define KEY_DEAL_PV_FILE "deal.pv.file"
#define KEY_DEAL_FEEPV_FILE "deal.feepv.file"
#define KEY_DEAL_DIRTYPRICE_FILE "deal.dirtyprice.file"
#define KEY_DEAL_CF_FILE "deal.cf.file"
#define KEY_DEAL_RATE_FILE "deal.rate.file"
#define KEY_DEAL_LEG_MAXNUM "deal.leg.maxnum"
#define KEY_DEAL_MTMCS_IS_MOVE_PREFIXING_PV "deal.mtmcs.ismoveprefixingpv"
#define KEY_DEAL_ISFIXINGRATE "deal.isfixing"
#define KEY_DEAL_FIXINGRATE "deal.fixingrate"
#define KEY_DEAL_ISCALCPAYOFFAFTERMATURITY "deal.iscalcpayoffaftermaturity"
#define KEY_DEAL_OUTPUT_NAME "deal.output.name"
#define KEY_DEAL_SUMMARY_INFO "deal.summary.info"
#define KEY_DEAL_FXOPT_BULK "deal.fxopt.bulk"
#define KEY_DEAL_FXOPT_BULK_OUTFILE "deal.fxopt.bulk.outfile"
#define KEY_DEAL_FXOPT_BULK_LABEL "deal.fxopt.bulk.label"
#define KEY_DEAL_MAXTERM_FILE "deal.maxterm.file"
#define KEY_DEAL_VALUEFUNCTION "deal.valuefunction"
#define STATIC_DATA_KEY_DEAL_DIGITALOPTION_ISCALLSPREAD ".deal.digitaloption.iscallspread"
#define STATIC_DATA_KEY_DEAL_DIGITALOPTION_CALLSPREADVALUE ".deal.digitaloption.callspreadvalue"
#define STATIC_DATA_KEY_DEAL_DIGITALOPTION_BUYSELLDISTINGUISH ".deal.digitaloption.buyselldistinguish"
#define STATIC_DATA_KEY_DEAL_MARKOVFUNCTIONAL_MEANREVERSION ".deal.markovfunctional.meanreversion"
#define FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD ".fx.deal.digitalcoupon.iscallspread"
#define FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE ".fx.deal.digitalcoupon.callspreadvalue"
#define FX_KEY_VALUATION_BASECURRENCYOFFXPAIR ".fx.valuation.basecurrencyoffxpair"
#define KEY_SIMULATION_MCNUM "simulation.mcnum"
#define KEY_SIMULATION_SEED "simulation.seed"
#define KEY_SIMULATION_ISDETAILOUTPUT "simulation.isdetailoutput"
#define KEY_SIMULATION_RADNTYPE "simulation.randtype"
#define KEY_SIMULATION_ISANTITHETIC "simulation.isantithetic"
#define KEY_SIMULATION_LSMC_MCNUM "simulation.lsmc.mcnum"
#define KEY_SIMULATION_LSMC_ISSHIFTY "simulation.lsmc.isshifty"
#define KEY_SIMULATION_LSMC_ISSHIFTX "simulation.lsmc.isshiftx"
#define KEY_SIMULATION_LSMC_ISSCALEY "simulation.lsmc.isscaley"
#define KEY_SIMULATION_LSMC_ISSCALEX "simulation.lsmc.isscalex"
#define KEY_SIMULATION_LSMC_BASEFUNCDIM "simulation.lsmc.basefuncdim"
#define KEY_SIMULATION_LSMC_SVDTOLERANCE "simulation.lsmc.svdtolerance"
#define KEY_SIMULATION_PATH_CACHE_BUFFER "simulation.path.cache.buffer"
#define KEY_SIMULATION_TERM_BUFFER "simulation.term.buffer"
#define KEY_SIMULATION_TERM_MAX "simulation.term.max"
#define KEY_SIMULATION_TERM_MAX_FREQ "simulation.term.max.frequency"
#define KEY_SIMULATION_ISBROWNIANBRIDGE "simulation.isbrownianbridge"
#define KEY_SIMULATION_RAND_SOBOL_DIRECTIONINTEGERS "simulation.rand.sobol.directionintegers"
#define KEY_SIMULATION_RAND_GAUSSIAN_ISHALLEYMODE "simulation.rand.gaussian.ishalleymode"
#define KEY_SIMULATION_QUASIGS_RAND_PATHNUM "simulation.quasigs.rand.pathnum"	// for XLL_PLUS
#define KEY_SIMULATION_FUNDING_CHANGED "simulation.funding.changed"	// for XLL_PLUS
#define KEY_SDE_XCCY_USD_IS_GENERATE_ONLY_CURVE "sde.xccy.usd.is.generate.only.curve"	// for XLL_PLUS
#define KEY_SDE_BASIS_BASE_CURRENCY "sde.basis.base.currency"
#define KEY_SDE_CORRELATION_FILE "sde.correlation.file"
#define KEY_SDE_CALIB_MODEL "sde.calib.model"
#define KEY_SDE_PV_CALIB_TARGET_CURRENCY "sde.pv.calib.target.currency"
#define KEY_SDE_PV_ADDITION_FWDPREMIUM "sde.pv.addition.fwdpremium"
#define KEY_RISK_SCENARIO_FILE "risk.scenario.file"
#define KEY_CALIB_SCENARIO_FILE "calib.scenario.file"
#define KEY_SDE_YIELD_ISAUDEXTRA "sde.yield.isaudextra"
#define STATIC_DATA_KEY_FUNDING_DAYCOUNT ".funding.daycount"
#define STATIC_DATA_KEY_FUNDING_SLIDINGRULE ".funding.slidingrule"
#define STATIC_DATA_KEY_FUNDING_CALENDAR ".funding.calendar"
#define STATIC_DATA_KEY_SPOTLAG ".sde.spotlag"
#define KEY_SEMIANALYTIC_OMIT_DIGITNUMBER "semianalytic.omit.digitnumeber"
#define CCY_SEMIANALYTIC_INPUTRISKNAME ".semianalytic.inputriskname"
#define CCY_SEMIANALYTIC_OUTPUTRISKNAME ".semianalytic.outputriskname"
#define KEY_SEMIANALYTIC_INPUTFILE "semianalytic.inputfile"
#define KEY_SEMIANALYTIC_OUTPUTFILE "semianalytic.outputfile"
#define KEY_FUNDINGSPREAD_FILE "fundingspread.file"
#define KEY_FXSPOTRATES_FILE "fxspotrate.file"
#define KEY_FXSPOTRATES_UNITCCY "fxspotrate.unitccy"
#define STATIC_DATA_KEY_FXSPOTRATES_CALENDAR ".fxspotrate.calendar"
#define KEY_MONOTONECONVEX_LAMBDA "monotoneconvex.lambda"
#define KEY_MONOTONECONVEX_ISALLOWEDNEGATIVE "monotoneconvex.isallowednegative"
#define KEY_MONOTONECONVEX_INPUTMODE "monotoneconvex.inputmode"
#define KEY_MONOTONECONVEX_OUTPUTMODE "monotoneconvex.outputmode"

#define STATIC_DATA_FX_KEY_SDE_SINGLE_MODEL ".sde.single.model"
#define STATIC_DATA_FX_KEY_SDE_CROSS_MODEL ".sde.cross.model"
#define STATIC_DATA_FX_KEY_SDE_NAME ".sde.name"
#define STATIC_DATA_FX_KEY_SDE_ISDD ".sde.isdd"
#define STATIC_DATA_FX_KEY_SDE_INITIALVALUE_FILE ".sde.initialvalue.file"

// Yield key
#define KEY_YIELD_IS_AUD_RESET_SKIP "sde.yield.is.aud.reset.skip"	// for XLL_PLUS
#define STATIC_DATA_KEY_YIELD_INTERPOLATION  ".sde.yield.interpolation"
#define STATIC_DATA_KEY_YIELD_DAYCOUNT  ".sde.yield.daycount"
#define STATIC_DATA_KEY_YIELD_SLIDINGRULE  ".sde.yield.slidingrule"
#define STATIC_DATA_KEY_YIELD_FREQUENCY  ".sde.yield.frequency"
#define STATIC_DATA_KEY_YIELD_CALENDAR  ".sde.yield.calendar"
#define STATIC_DATA_KEY_YIELD_GENERATOR_ISSPOTUSE  ".sde.yield.generator.isspotuse"
#define STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATION  ".sde.yield.generator.interpolation"
#define STATIC_DATA_KEY_YIELD_GENERATOR_DAYCOUNT  ".sde.yield.generator.daycount"
#define STATIC_DATA_KEY_YIELD_GENERATOR_CALENDAR  ".sde.yield.generator.calendar"
#define STATIC_DATA_KEY_YIELD_GENERATOR_SLIDINGRULE  ".sde.yield.generator.slidingrule"
#define STATIC_DATA_KEY_YIELD_GENERATOR_FREQUENCY  ".sde.yield.generator.frequency"
#define STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION  ".sde.yield.generator.yieldgen.interpolation"
#define STATIC_DATA_KEY_YIELD_GENERATOR_FUTURE_INTERPOLATION  ".sde.yield.generator.future.interpolation"
#define STATIC_DATA_KEY_YIELD_GENERATOR_BASIS_INTERPOLATION  ".sde.yield.generator.basis.interpolation"
#define STATIC_DATA_KEY_YIELD_GENERATOR_BASISFUNCTION  ".sde.yield.generator.basisfunction"
#define STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE  ".sde.yield.generator.isfutureuse"
#define STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF  ".sde.yield.generator.isadjustdf"
#define STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST  ".sde.yield.generator.isswaptenoradjust"
#define STATIC_DATA_KEY_YIELD_GENERATOR_RATEPRIORITY  ".sde.yield.generator.ratepriority"
#define STATIC_DATA_KEY_YIELD_GENERATOR_TENORSWAPNAME  ".sde.yield.generator.tenorswapname"
#define STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP  ".sde.yield.generator.isfwdswap"
#define STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDBASIS  ".sde.yield.generator.isfwdbasis"
#define STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE  ".sde.yield.generator.optimizeperformance"
#define STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD  ".sde.yield.generator.fastrebuild"
#define STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATIONJOINDATE  ".sde.yield.generator.interpolationjoindate"
#define STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE  ".sde.yield.generator.alwayscalcjoindate"
#define STATIC_DATA_KEY_YIELD_GENERATOR_STATEVARIABLE  ".sde.yield.generator.statevariable"
#define STATIC_DATA_KEY_YIELD_GENERATOR_FIXINGSOURCE  ".sde.yield.generator.fixingsource"
#define STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE  ".sde.yield.generator.curvetype"
#define STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY  ".sde.yield.generator.generateforwardsfromswapsonly"
#define STATIC_DATA_KEY_YIELD_MONEYMARKET_SPOTDATE  ".sde.yield.moneymarket.spotdate"
#define STATIC_DATA_KEY_YIELD_MONEYMARKET_DAYCOUNT  ".sde.yield.moneymarket.daycount"
#define STATIC_DATA_KEY_YIELD_MONEYMARKET_CALENDAR  ".sde.yield.moneymarket.calendar"
#define STATIC_DATA_KEY_YIELD_MONEYMARKET_SLIDINGRULE  ".sde.yield.moneymarket.slidingrule"
#define STATIC_DATA_KEY_YIELD_LIBOR_SPOTDATE  ".sde.yield.libor.spotdate"
#define STATIC_DATA_KEY_YIELD_LIBOR_DAYCOUNT  ".sde.yield.libor.daycount"
#define STATIC_DATA_KEY_YIELD_LIBOR_CALENDAR  ".sde.yield.libor.calendar"
#define STATIC_DATA_KEY_YIELD_LIBOR_SLIDINGRULE  ".sde.yield.libor.slidingrule"
#define STATIC_DATA_KEY_YIELD_LIBOR_FREQUENCY  ".sde.yield.libor.frequency"
#define STATIC_DATA_KEY_YIELD_LIBOR_RESETLAG  ".sde.yield.libor.resetlag"
#define STATIC_DATA_KEY_YIELD_LIBOR_ISEOMROLL ".sde.yield.libor.iseomroll"
#define STATIC_DATA_KEY_YIELD_LIBOR_EOMDAY  ".sde.yield.libor.eomday"
#define STATIC_DATA_KEY_YIELD_LIBOR_ISONFORSPOTADJUST  ".sde.yield.libor.isonforspotadjust"
#define STATIC_DATA_KEY_YIELD_LIBOR_USEGRID  ".sde.yield.libor.usegrid"
#define STATIC_DATA_KEY_YIELD_SWAP_SPOTDATE  ".sde.yield.swap.spotdate"
#define STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNT  ".sde.yield.swap.daycount"
#define STATIC_DATA_KEY_YIELD_SWAP_CALENDAR  ".sde.yield.swap.calendar"
#define STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE  ".sde.yield.swap.slidingrule"
#define STATIC_DATA_KEY_YIELD_SWAP_FREQUENCY  ".sde.yield.swap.frequency"
#define STATIC_DATA_KEY_YIELD_SWAP_CALENDAR_ARROIS  ".sde.yield.swap.calendar.arrois"
#define STATIC_DATA_KEY_YIELD_SWAP_SLIDINGRULE_ARROIS  ".sde.yield.swap.slidingrule.arrois"
#define STATIC_DATA_KEY_YIELD_SWAP_RESETLAG  ".sde.yield.swap.resetlag"
#define STATIC_DATA_KEY_YIELD_SWAP_ISTIMEINTERPOLATION  ".sde.yield.swap.istimeinterpolation"
#define STATIC_DATA_KEY_YIELD_SWAP_ISNEWTONRAPHSON  ".sde.yield.swap.isnewtonraphson"
#define STATIC_DATA_KEY_YIELD_SWAP_ISSIMULTANEOUSEQ  ".sde.yield.swap.issimultaneouseq"
#define STATIC_DATA_KEY_YIELD_SWAP_OPTIMIZEMETHOD  ".sde.yield.swap.optimizemethod"
#define STATIC_DATA_KEY_YIELD_SWAP_EPSILON  ".sde.yield.swap.epsilon"
#define STATIC_DATA_KEY_YIELD_SWAP_GRADIENTEPSILON  ".sde.yield.swap.gradientepsilon"
#define STATIC_DATA_KEY_YIELD_SWAP_DELTA  ".sde.yield.swap.delta"
#define STATIC_DATA_KEY_YIELD_SWAP_MAXLOOP  ".sde.yield.swap.maxloop"
#define STATIC_DATA_KEY_YIELD_SWAP_SWAPTENOR  ".sde.yield.swap.swaptenor"
#define STATIC_DATA_KEY_YIELD_SWAP_SWAPTYPE  ".sde.yield.swap.swaptype"
#define STATIC_DATA_KEY_YIELD_SWAP_ISEOMRLL  ".sde.yield.swap.iseomroll"
#define STATIC_DATA_KEY_YIELD_SWAP_EOMDAY  ".sde.yield.swap.eomday"
#define STATIC_DATA_KEY_YIELD_SWAP_USEGRID  ".sde.yield.swap.usegrid"
#define STATIC_DATA_KEY_YIELD_SWAP_ISFWDINTER  ".sde.yield.swap.isfwdinter"
#define STATIC_DATA_KEY_YIELD_SWAP_FWDINTERPOLATION  ".sde.yield.swap.fwdinterpolation"

#define STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_EPSILON  ".sde.yield.globalenginecurves.epsilon"
#define STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_GRADIENTEPSILON  ".sde.yield.globalenginecurves.gradientepsilon"
#define STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_DELTA  ".sde.yield.globalenginecurves.delta"
#define STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_MAXLOOP  ".sde.yield.globalenginecurves.maxloop"
#define STATIC_DATA_KEY_YIELD_GLOBALENGINECURVES_FASTREBUILD  ".sde.yield.globalenginecurves.fastrebuild"

#define STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE  ".sde.yield.assignedcurve"
#define STATIC_DATA_KEY_YIELD_BASIS_USEBASIS  ".sde.yield.basis.usebasis"
#define STATIC_DATA_KEY_YIELD_BASIS_SPOTDATE  ".sde.yield.basis.spotdate"
#define STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT  ".sde.yield.basis.daycount"
#define STATIC_DATA_KEY_YIELD_BASIS_CALENDAR  ".sde.yield.basis.calendar"
#define STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE  ".sde.yield.basis.slidingrule"
#define STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY  ".sde.yield.basis.frequency"
#define STATIC_DATA_KEY_YIELD_BASIS_DAYCOUNT_ARROIS  ".sde.yield.basis.daycount.arrois"
#define STATIC_DATA_KEY_YIELD_BASIS_CALENDAR_ARROIS  ".sde.yield.basis.calendar.arrois"
#define STATIC_DATA_KEY_YIELD_BASIS_SLIDINGRULE_ARROIS  ".sde.yield.basis.slidingrule.arrois"
#define STATIC_DATA_KEY_YIELD_BASIS_FREQUENCY_ARROIS  ".sde.yield.basis.frequency.arrois"
#define STATIC_DATA_KEY_YIELD_BASIS_RESETLAG  ".sde.yield.basis.resetlag"
#define STATIC_DATA_KEY_YIELD_BASIS_BASEDAYCOUNT  ".sde.yield.basis.basedaycount"
#define STATIC_DATA_KEY_YIELD_BASIS_BASECALENDAR  ".sde.yield.basis.basecalendar"
#define STATIC_DATA_KEY_YIELD_BASIS_BASESLIDINGRULE  ".sde.yield.basis.baseslidingrule"
#define STATIC_DATA_KEY_YIELD_BASIS_BASEFREQUENCY  ".sde.yield.basis.basefrequency"
#define STATIC_DATA_KEY_YIELD_BASIS_ISESTIMATEMETHOD  ".sde.yield.basis.isestimatemethod"
#define STATIC_DATA_KEY_YIELD_BASIS_ISDISCOUNTCURVE  ".sde.yield.basis.isdiscountcurve"
#define STATIC_DATA_KEY_YIELD_BASIS_ISTIMEINTERPOLATION  ".sde.yield.basis.istimeinterpolation"
#define STATIC_DATA_KEY_YIELD_BASIS_ISSIMULTANEOUSEQ  ".sde.yield.basis.issimultaneouseq"
#define STATIC_DATA_KEY_YIELD_BASIS_EPSILON  ".sde.yield.basis.epsilon"
#define STATIC_DATA_KEY_YIELD_BASIS_GRADIENTEPSILON  ".sde.yield.basis.gradientepsilon"
#define STATIC_DATA_KEY_YIELD_BASIS_DELTA  ".sde.yield.basis.delta"
#define STATIC_DATA_KEY_YIELD_BASIS_MAXLOOP  ".sde.yield.basis.maxloop"
#define STATIC_DATA_KEY_YIELD_FUTURE_SPOTDATE  ".sde.yield.future.spotdate"
#define STATIC_DATA_KEY_YIELD_FUTURE_DAYCOUNT  ".sde.yield.future.daycount"
#define STATIC_DATA_KEY_YIELD_FUTURE_CALENDAR  ".sde.yield.future.calendar"
#define STATIC_DATA_KEY_YIELD_FUTURE_SLIDINGRULE  ".sde.yield.future.slidingrule"
#define STATIC_DATA_KEY_YIELD_FUTURE_FREQUENCY  ".sde.yield.future.frequency"
#define STATIC_DATA_KEY_YIELD_FUTURE_RESETLAG  ".sde.yield.future.resetlag"
#define STATIC_DATA_KEY_YIELD_FUTURE_ISCONVADJPRECISE  ".sde.yield.future.isconvadjprecise"
#define STATIC_DATA_KEY_YIELD_FUTURE_MEANREVERSION  ".sde.yield.future.meanreversion"
#define STATIC_DATA_KEY_YIELD_FUTURE_USEGRID  ".sde.yield.future.usegrid"
#define	STATIC_DATA_KEY_YIELD_FUTURE_APPLYTENSION	".sde.yield.future.applytension"
#define	STATIC_DATA_KEY_YIELD_FUTURE_USECONVEXADJUSTMENT	".sde.yield.future.useconvexadjustment" // Legacy parameter means convexityQuotedAsPrice
#define	STATIC_DATA_KEY_YIELD_FUTURE_CONVEXITYQUOTETYPE	".sde.yield.future.convexityquotetype" // Alias and replacement for useconvexadjustment
#define	STATIC_DATA_KEY_YIELD_FUTURE_TENSIONGAP	".sde.yield.future.tensiongap"
#define	STATIC_DATA_KEY_YIELD_FUTURE_SMOOTHSHORTEND	".sde.yield.future.smoothshortend"
#define	STATIC_DATA_KEY_YIELD_FUTURE_SERIAL_CALC_TYPE	".sde.yield.future.serialcalctype"
#define	STATIC_DATA_KEY_YIELD_FUTURE_INSTRUMENT_TYPE	".sde.yield.future.instrumenttype"
#define STATIC_DATA_KEY_YIELD_PATHCURVE  ".sde.yield.pathcurve"
#define STATIC_DATA_KEY_YIELD_MARKETTYPE	".sde.yield.markettype"
#define STATIC_DATA_KEY_YIELD_FUTURE_USEGRIDNUM  ".sde.yield.future.usegridnum"
#define STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID  ".sde.yield.fffuture.usegrid"
#define STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRIDNUM  ".sde.yield.fffuture.usegridnum"
// Yield file name
#define STATIC_DATA_KEY_YIELD_LIBOR_FILE  ".sde.yield.libor.file"
#define STATIC_DATA_KEY_YIELD_SWAP_FILE  ".sde.yield.swap.file"
#define STATIC_DATA_KEY_YIELD_BASISSWAP_FILE  ".sde.yield.basisswap.file"
#define STATIC_DATA_KEY_YIELD_FUTURE_FILE  ".sde.yield.future.file"
#define STATIC_DATA_KEY_YIELD_FUTUREADJUSTVOL_FILE  ".sde.yield.futureadjustvol.file"
#define STATIC_DATA_KEY_YIELD_OIS_FILE		".sde.yield.ois.file"
#define STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE	".sde.yield.historical.ois.file"
#define STATIC_DATA_KEY_YIELD_FFFUTURE_FILE		".sde.yield.fffuture.file"
#define STATIC_DATA_KEY_YIELD_LOBASIS_FILE  ".sde.yield.lobasis.file"

#define STATIC_DATA_KEY_YIELD_DF_FILE		".sde.yield.df.file"
// Yield Basis
#define STATIC_DATA_KEY_YIELD_GENERATEDFS  ".sde.yield.generatedfs"
#define STATIC_DATA_KEY_YIELD_USEMAKETS  ".sde.yield.usemarkets"
#define STATIC_DATA_KEY_YIELD_DF2  ".sde.yield.df2"
#define STATIC_DATA_KEY_YIELD_ISBASIS  ".sde.yield.isbasis"
#define STATIC_DATA_KEY_YIELD_ISREADFILE ".sde.yield.isreadfile"
#define STATIC_DATA_KEY_YIELD_BASIS_ISLEG1SPREAD  ".sde.yield.basis.isleg1spread"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1FORECAST  ".sde.yield.basis.leg1forecast"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2FORECAST  ".sde.yield.basis.leg2forecast"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1DISCOUNT  ".sde.yield.basis.leg1discount"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2DISCOUNT  ".sde.yield.basis.leg2discount"
#define STATIC_DATA_KEY_YIELD_BASIS_TARGET  ".sde.yield.basis.target"
#define STATIC_DATA_KEY_YIELD_BASIS_IS2SWAP  ".sde.yield.basis.is2swap"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_FIXINGCALENDAR  ".sde.yield.basis.leg1index.fixing.calendar"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_PAYMENTCALENDAR  ".sde.yield.basis.leg1index.payment.calendar"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_RESETLAG  ".sde.yield.basis.leg1index.resetlag"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_ACCESSARY  ".sde.yield.basis.leg1index.accessary"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_FREQUENCY  ".sde.yield.basis.leg1index.frequency"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_SLIDINGRULE  ".sde.yield.basis.leg1index.slidingrule"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_DAYCOUNT  ".sde.yield.basis.leg1index.daycount"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1INDEX_TYPE  ".sde.yield.basis.leg1index.type"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_FIXINGCALENDAR  ".sde.yield.basis.leg2index.fixing.calendar"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_PAYMENTCALENDAR  ".sde.yield.basis.leg2index.payment.calendar"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_RESETLAG  ".sde.yield.basis.leg2index.resetlag"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_ACCESSARY  ".sde.yield.basis.leg2index.accessary"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_FREQUENCY  ".sde.yield.basis.leg2index.frequency"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_SLIDINGRULE  ".sde.yield.basis.leg2index.slidingrule"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_DAYCOUNT  ".sde.yield.basis.leg2index.daycount"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2INDEX_TYPE  ".sde.yield.basis.leg2index.type"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_CALENDAR  ".sde.yield.basis.leg1cashlet.calendar"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SPOTLAG  ".sde.yield.basis.leg1cashlet.spotlag"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SPOTDATE  ".sde.yield.basis.leg1cashlet.spotdate"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCY  ".sde.yield.basis.leg1cashlet.frequency"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_FREQUENCYCOMPOUND  ".sde.yield.basis.leg1cashlet.frequencycompound"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_SLIDINGRULE  ".sde.yield.basis.leg1cashlet.slidingrule"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_DAYCOUNT  ".sde.yield.basis.leg1cashlet.daycount"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_GENERATEMETHOD  ".sde.yield.basis.leg1cashlet.generatemethod"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG1CASHLET_ISBACKWARD  ".sde.yield.basis.leg1cashlet.isbackward"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_CALENDAR  ".sde.yield.basis.leg2cashlet.calendar"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SPOTLAG  ".sde.yield.basis.leg2cashlet.spotlag"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SPOTDATE  ".sde.yield.basis.leg2cashlet.spotdate"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCY  ".sde.yield.basis.leg2cashlet.frequency"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_FREQUENCYCOMPOUND  ".sde.yield.basis.leg2cashlet.frequencycompound"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_SLIDINGRULE  ".sde.yield.basis.leg2cashlet.slidingrule"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_DAYCOUNT  ".sde.yield.basis.leg2cashlet.daycount"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_GENERATEMETHOD  ".sde.yield.basis.leg2cashlet.generatemethod"
#define STATIC_DATA_KEY_YIELD_BASIS_LEG2CASHLET_ISBACKWARD  ".sde.yield.basis.leg2cashlet.isbackward"
#define STATIC_DATA_KEY_YIELD_BASIS_ISEOMRLL	".sde.yield.basis.iseomroll"
#define STATIC_DATA_KEY_YIELD_BASIS_EOMDAY	".sde.yield.basis.eomday"
#define STATIC_DATA_KEY_YIELD_BASIS_FILE  ".sde.yield.basis.file"
#define STATIC_DATA_KEY_YIELD_BASIS_USEGRID  ".sde.yield.basis.usegrid"
#define STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE	".sde.yield.basis.assignedcurve"
#define STATIC_DATA_KEY_YIELD_BASIS_ISSAMEGRIDINDEX	".sde.yield.basis.issamegridindex"
#define STATIC_DATA_KEY_YIELD_BASIS_ISODDTERMFRNINDEX	".sde.yield.basis.isoddtermfrnindex"
#define STATIC_DATA_KEY_YIELD_BASIS_ISYIELDSPREADCALC	".sde.yield.basis.isyieldspreadcalc"
#define STATIC_DATA_KEY_YIELD_BASIS_SPOTRATETERM	".sde.yield.basis.spotrateterm"
#define STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE  ".sde.yield.basis.adjustvalue.file"
#define STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_INTERPOLATION  ".sde.yield.basis.adjustvalue.interpolation"
#define STATIC_DATA_KEY_YIELD_BASIS_ISFWDINTER  ".sde.yield.basis.isfwdinter"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDINTERPOLATION  ".sde.yield.basis.fwdinterpolation"
#define STATIC_DATA_KEY_YIELD_BASIS_OPTIMIZEMETHOD  ".sde.yield.basis.optimizemethod"
#define STATIC_DATA_KEY_YIELD_BASIS_COMPOUNDMETHOD  ".sde.yield.basis.compoundmethod"
#define STATIC_DATA_KEY_YIELD_CTD_COLLATERALCURVES ".sde.yield.ctd.collateralcurves"
#define STATIC_DATA_KEY_YIELD_ZERORATE_ASSIGNEDCURVE	".sde.yield.zerorate.assignedcurve"
#define STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD	".sde.yield.ois.generatemethod"
#define STATIC_DATA_KEY_YIELD_OIS_CALENDAR			".sde.yield.ois.calendar"
#define STATIC_DATA_KEY_YIELD_OIS_SPOTDATE			".sde.yield.ois.spotdate"
#define STATIC_DATA_KEY_YIELD_OIS_RESETLAG			".sde.yield.ois.resetlag"
#define STATIC_DATA_KEY_YIELD_OIS_FREQUENCY			".sde.yield.ois.frequency"
#define STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT			".sde.yield.ois.daycount"
#define STATIC_DATA_KEY_YIELD_OIS_SLIDINGRULE		".sde.yield.ois.slidingrule"
#define STATIC_DATA_KEY_YIELD_OIS_SWAPTYPE			".sde.yield.ois.swaptype"
#define STATIC_DATA_KEY_YIELD_OIS_ISEOMRLL	".sde.yield.ois.iseomroll"
#define STATIC_DATA_KEY_YIELD_OIS_EOMDAY	".sde.yield.ois.eomday"
#define STATIC_DATA_KEY_YIELD_OIS_SHORTTERM		".sde.yield.ois.shortterm"
#define STATIC_DATA_KEY_YIELD_OIS_USEGRID			".sde.yield.ois.usegrid"
#define STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION	".sde.yield.ois.shorttermconvention"
#define STATIC_DATA_KEY_YIELD_OIS_FIRSTRATE	".sde.yield.ois.firstrate"
#define STATIC_DATA_KEY_YIELD_OIS_LONGTERM		".sde.yield.ois.longterm"
#define STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION	".sde.yield.ois.longtermconvention"
#define STATIC_DATA_KEY_YIELD_OIS_LOBASISNAME ".sde.yield.ois.lobasisname"
#define STATIC_DATA_KEY_YIELD_OIS_SHORTTERMSWAPOVERRULES	".sde.yield.ois.shorttermswapoverrules"
#define STATIC_DATA_KEY_YIELD_OIS_SMOOTHSHORTEND			".sde.yield.ois.smoothshortend"
#define STATIC_DATA_KEY_YIELD_OIS_ISDUALBOOTSTRAPPING			".sde.yield.ois.isdualbootstrapping"
#define STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD	".sde.yield.ois.longterm.generatemethod"
#define STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD_ARROIS ".sde.yield.ois.longterm.generatemethod.arrois"
#define STATIC_DATA_KEY_YIELD_OIS_COMPOUNDMETHOD	".sde.yield.ois.compoundmethod"
#define STATIC_DATA_KEY_YIELD_OIS_COMPOUNDINGMETHOD	".sde.yield.ois.compoundingmethod"
#define STATIC_DATA_KEY_YIELD_OIS_INTERPOLATIONJOINDATE	".sde.yield.ois.interpolationjoindate"
#define STATIC_DATA_KEY_YIELD_OIS_EPSILON  ".sde.yield.ois.epsilon"
#define STATIC_DATA_KEY_YIELD_OIS_MAXLOOP  ".sde.yield.ois.maxloop"
#define STATIC_DATA_KEY_YIELD_OIS_MEANREVERSION  ".sde.yield.ois.meanreversion"
#define	STATIC_DATA_KEY_YIELD_OIS_USECONVEXADJUSTMENT	".sde.yield.ois.useconvexadjustment" // Means convexity adjustment quoted as a price, not very clear, hence the alias below
#define	STATIC_DATA_KEY_YIELD_OIS_CONVEXITYQUOTETYPE	".sde.yield.ois.convexityquotetype" // Alias for useconvexadjustment

// virtual rate
#define STATIC_DATA_KEY_YIELD_ISVIRTUAL ".sde.yield.isvirtual"
#define STATIC_DATA_KEY_YIELD_VIRTUALGRID ".sde.yield.virtualgrid"
#define STATIC_DATA_KEY_YIELD_VIRTUALRATE ".sde.yield.virtualrate"
// fwdfx constant
#define KEY_YIELD_COLLATERAL_CCY "sde.yield.collateral.ccy"
#define STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST ".sde.yield.generator.isfwdfxconst"
#define STATIC_DATA_KEY_YIELD_FWDFXCONST_USEMARKET ".sde.yield.fwdfxconst.usemarket"
// forward ratio
#define STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFX			".sde.yield.generator.isfwdfx"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISRATIO		".sde.yield.basis.fwdfx.isratio"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISFXOUTRIGHT	".sde.yield.basis.fwdfx.isfxoutright"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_DENOMINATOR	".sde.yield.basis.fwdfx.denominator"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_PIPSIZE		".sde.yield.basis.fwdfx.pipsize"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISPRICECCY	".sde.yield.basis.fwdfx.ispriceccy"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISDOMESTICCURRENCY	".sde.yield.basis.fwdfx.isdomesticcurrency"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_CALENDAR		".sde.yield.basis.fwdfx.calendar"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_EOMDAY		".sde.yield.basis.fwdfx.eomday"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FREQUENCY		".sde.yield.basis.fwdfx.frequency"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_ISEOMROLL		".sde.yield.basis.fwdfx.iseomroll"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_RESETLAG		".sde.yield.basis.fwdfx.resetlag"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_SLIDINGRULE	".sde.yield.basis.fwdfx.slidingrule"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_SPOTDATE		".sde.yield.basis.fwdfx.spotdate"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID		".sde.yield.basis.fwdfx.usegrid"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FILE			".sde.yield.basis.fwdfx.file"
#define STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FIXEDRATEXCCYSTARTTENOR	".sde.yield.basis.fwdfx.fixedratexccystarttenor"

// Yield ArbFreeCurve
#define	STATIC_DATA_KEY_YIELD_ISARBFREE	".sde.yield.isarbfree"
#define	STATIC_DATA_KEY_YIELD_PARCURVE	"sde.yield.parcurve"
#define	STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE	".sde.yield.generator.isfrause"
#define	STATIC_DATA_KEY_YIELD_GENERATOR_ISRENOTIONALADJUST	".sde.yield.generator.isrenotionaladjust"
#define	STATIC_DATA_KEY_YIELD_GENERATOR_ISXCCYMARKEDTOMARKET	".sde.yield.generator.isxccymarkedtomarket" // alias for isrenotionaladjust
#define	STATIC_DATA_KEY_YIELD_6MLCURVENAME	".sde.yield.6mlcurvename"
#define	STATIC_DATA_KEY_YIELD_DFCURVENAME	".sde.yield.dfcurvename"
#define	STATIC_DATA_KEY_YIELD_3MLCURVENAME	".sde.yield.3mlcurvename"
#define	STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFIX	".sde.yield.swap.daycountfix"
#define	STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFIX_ARROIS	".sde.yield.swap.daycountfix.arrois"
#define	STATIC_DATA_KEY_YIELD_SWAP_DAYCOUNTFLOAT	".sde.yield.swap.daycountfloat"
#define	STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFIX		".sde.yield.swap.frequencyfix"
#define	STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFIX_ARROIS		".sde.yield.swap.frequencyfix.arrois"
#define	STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYFLOAT	".sde.yield.swap.frequencyfloat"
#define	STATIC_DATA_KEY_YIELD_SWAP_FREQUENCYCOMPOUND	".sde.yield.swap.frequencycompound"
#define	STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT	".sde.yield.swap.basefrequencyfloat"
#define	STATIC_DATA_KEY_YIELD_SWAP_INTERPOLATION	".sde.yield.swap.interpolation"
#define	STATIC_DATA_KEY_YIELD_XCCYBASIS_SPOTDATE	".sde.yield.xccybasis.spotdate"
#define	STATIC_DATA_KEY_YIELD_XCCYBASIS_RESETLAG	".sde.yield.xccybasis.resetlag"
#define	STATIC_DATA_KEY_YIELD_XCCYBASIS_DAYCOUNT	".sde.yield.xccybasis.daycount"
#define	STATIC_DATA_KEY_YIELD_XCCYBASIS_CALENDAR	".sde.yield.xccybasis.calendar"
#define	STATIC_DATA_KEY_YIELD_XCCYBASIS_SLIDINGRULE	".sde.yield.xccybasis.slidingrule"
//#define	STATIC_DATA_KEY_YIELD_XCCYBASIS_INTERPOLATION	".sde.yield.xccybasis.interpolation"
#define	STATIC_DATA_KEY_YIELD_BASIS_INTERPOLATION	".sde.yield.basis.interpolation"
#define	STATIC_DATA_KEY_YIELD_3M6MBASIS_DAYCOUNTTHREE	".sde.yield.3m6mbasis.daycountthree"
#define	STATIC_DATA_KEY_YIELD_3M6MBASIS_DAYCOUNTSIX	".sde.yield.3m6mbasis.daycountsix"
#define	STATIC_DATA_KEY_YIELD_3M6MBASIS_CALENDAR	".sde.yield.3m6mbasis.calendar"
#define	STATIC_DATA_KEY_YIELD_3M6MBASIS_SLIDINGRULE	".sde.yield.3m6mbasis.slidingrule"
//#define	STATIC_DATA_KEY_YIELD_3M6MBASIS_INTERPOLATION	".sde.yield.3m6mbasis.interpolation"
#define	STATIC_DATA_KEY_YIELD_3M6MBASIS_FILE	".sde.yield.3m6mbasis.file"
#define	STATIC_DATA_KEY_YIELD_FRA_SPOTDATE	".sde.yield.fra.spotdate"
#define	STATIC_DATA_KEY_YIELD_FRA_RESETLAG	".sde.yield.fra.resetlag"
#define	STATIC_DATA_KEY_YIELD_FRA_DAYCOUNT	".sde.yield.fra.daycount"
#define	STATIC_DATA_KEY_YIELD_FRA_CALENDAR	".sde.yield.fra.calendar"
#define	STATIC_DATA_KEY_YIELD_FRA_APPLYTENSION	".sde.yield.fra.applytension"
#define	STATIC_DATA_KEY_YIELD_FRA_TENSIONGAP	".sde.yield.fra.tensiongap"
#define	STATIC_DATA_KEY_YIELD_FRA_SMOOTHSHORTEND	".sde.yield.fra.smoothshortend"
#define	STATIC_DATA_KEY_YIELD_FRA_SERIAL_CALC_TYPE	".sde.yield.fra.serialcalctype"
#define	STATIC_DATA_KEY_YIELD_FRA_INSTRUMENT_TYPE	".sde.yield.fra.instrumenttype"
#define	STATIC_DATA_KEY_YIELD_FRA_SLIDINGRULE	".sde.yield.fra.slidingrule"
#define	STATIC_DATA_KEY_YIELD_FRA_ISEOMROLL	".sde.yield.fra.iseomroll"
#define STATIC_DATA_KEY_YIELD_FRA_EOMDAY	".sde.yield.fra.eomday"
#define	STATIC_DATA_KEY_YIELD_3MFRA_FILE	".sde.yield.3mfra.file"
#define	STATIC_DATA_KEY_YIELD_6MFRA_FILE	".sde.yield.6mfra.file"
#define	STATIC_DATA_KEY_YIELD_FRA_FILE	".sde.yield.fra.file"
#define	STATIC_DATA_KEY_YIELD_3MFRA_USEGRID	".sde.yield.3mfra.usegrid"
#define	STATIC_DATA_KEY_YIELD_6MFRA_USEGRID	".sde.yield.6mfra.usegrid"
#define	STATIC_DATA_KEY_YIELD_FRA_USEGRID	".sde.yield.fra.usegrid"
#define	STATIC_DATA_KEY_YIELD_BASIS_ISINITIALRATEUSE	".sde.yield.basis.isinitialrateuse"
#define	STATIC_DATA_KEY_YIELD_BASIS_INITIALRATEFILE	".sde.yield.basis.initialratefile"
#define	STATIC_DATA_KEY_YIELD_BASIS_INITIALRATETERM	".sde.yield.basis.initialrateterm"
#define	STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME	".sde.yield.generator.dfcurvename"

// Target Required to Trigger a calculation
// targetForResidentMode = targetRequiredToEnableCalculation;
// If the target is not found then the enable calculation parameter i.e. isResidentMode is set to false
#define	STATIC_DATA_KEY_YIELD_GENERATOR_TARGET	".sde.yield.generator.targetforresidentmode" 

#define STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME ".sde.yield.generator.dualbootstrap.swapcurvename"
#define STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_OISCURVENAME ".sde.yield.generator.dualbootstrap.oiscurvename"
#define STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVENAMES ".sde.yield.generator.globalenginecurves.allcurvenames"
#define STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVETYPES ".sde.yield.generator.globalenginecurves.allcurvetypes"
#define	STATIC_DATA_KEY_YIELD_BASIS_USD3MLFLOATER	".sde.yield.basis.usd3mlfloater"
#define	STATIC_DATA_KEY_YIELD_USD3MLFORECAST	".sde.yield.usd3mlforecast"
#define	STATIC_DATA_KEY_YIELD_USDDF	".sde.yield.usddf"
#define	STATIC_DATA_KEY_YIELD_BASIS_BASECCYDF	".sde.yield.basis.baseccydf"
#define	STATIC_DATA_KEY_YIELD_GENERATEFLOATERS	".sde.yield.generatefloaters"
#define	STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME	".sde.yield.floater.basisname"
#define	STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT	".sde.yield.floater.discount"
#define	STATIC_DATA_KEY_YIELD_FLOATER_FORECAST	".sde.yield.floater.forecast"
#define	STATIC_DATA_KEY_YIELD_FLOATER_ISFWDINTER	".sde.yield.floater.isfwdinter"
#define	STATIC_DATA_KEY_YIELD_FLOATER_FWDINTERPOLATION	".sde.yield.floater.fwdinterpolation"
//index interpolation
#define STATIC_DATA_KEY_INDEX_LIBOR_ISFWDINTER  ".index.libor.isfwdinter"
#define STATIC_DATA_KEY_INDEX_LIBOR_FWDINTERPOLATION  ".index.libor.fwdinterpolation"
//Vanilla Addin
#define	STATIC_DATA_KEY_YIELD_GENERATECURVEID	".sde.yield.generatecurveid"
//convexity adjust
#define	STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_MODEL					".sde.yield.convexityadjust.model"
#define	STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_THRESHOLD				".sde.yield.convexityadjust.threshold"
#define	STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_INTEGRALUPPERBOUND	".sde.yield.convexityadjust.integralupperbound"
#define	STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_INTEGRALLOWERBOUND	".sde.yield.convexityadjust.integrallowerbound"
#define	STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_INTEGRALSTEPS			".sde.yield.convexityadjust.integralsteps"
#define	STATIC_DATA_KEY_YIELD_CONVEXITYADJUST_PREMIUMINTEGRALSTEPS  ".sde.yield.convexityadjust.premiumintegralsteps"
// tolerance value for SpotRateRatioMethod
#define	STATIC_DATA_KEY_YIELD_SPOTRATERATIOMETHOD_TOLERANCE	".sde.yield.spotrateratiomethod.tolerance"

//********************************
//
// property key end
//
//********************************
// market input type
#define INPUT_FUNC  "FUNC"
#define INPUT_DATA  "DATA"
#define INPUT_FACTOR  "FACTOR"

#define INPUT_T_I_DATA_MATRIX  "T_I_DATA_MATRIX"
#define INPUT_T_I_FUNC_MATRIX  "T_I_FUNC_MATRIX"
#define INPUT_T_I_FUNC_VECTOR  "T_I_FUNC_VECTOR"
#define INPUT_T_DATA_MATRIX  "T_DATA_MATRIX"
#define INPUT_T_FUNC_VECTOR  "T_FUNC_VECTOR"
#define INPUT_T_FUNC_SCALAR  "T_FUNC_SCALAR"
#define INPUT_DATA_VECTOR   "DATA_VECTOR"
#define INPUT_FUNC_SCALAR  "FUNC_SCALAR"
#define INPUT_T_DATA_SCALAR  "T_DATA_SCALAR"
#define INPUT_DATA_SCALAR  "DATA_SCALAR"


// currency
#define CURRENCY_JPY  "JPY"
#define CURRENCY_USD  "USD"
#define CURRENCY_EUR  "EUR"
#define CURRENCY_TAR  "TAR"
#define CURRENCY_GBP  "GBP"
#define CURRENCY_AUD  "AUD"
#define CURRENCY_CHF  "CHF"
#define CURRENCY_CAD  "CAD"
#define CURRENCY_NZD  "NZD"

// frequency
#define FREQ_BUSINESS_DAYS  "BUSINESS_DAYS"
#define FREQ_WEEKLY  "WEEKLY"
#define FREQ_NONE  "NONE"
#define FREQ_ANNUAL  "ANNUAL"
#define FREQ_SEMI_ANNUAL  "SEMI-ANNUAL"
#define FREQ_QUARTERLY  "QUARTERLY"
#define FREQ_MONTHLY  "MONTHLY"
#define FREQ_SIMPLE "SIMPLE"
#define FREQ_CONTINUOUS "CONTINUOUS"

// accessory
#define ACCESSORY_1M  "1M"
#define ACCESSORY_3M  "3M"
#define ACCESSORY_6M  "6M"
#define ACCESSORY_12M  "12M"

// yield input type
#define YIELD_TYPE_O_N  "O_N"
#define YIELD_TYPE_T_N  "T_N"
#define YIELD_TYPE_ZERO  "ZERORATE"	//Libor Rate
#define YIELD_TYPE_PAR  "PARRATE"	//Swap Rate
#define YIELD_TYPE_FWD  "FWDRATE"
#define YIELD_TYPE_BASIS  "BASISRATE"
#define YIELD_TYPE_BOJ  "BOJRATE"
#define YIELD_TYPE_FF  "FFRATE"
#define YIELD_TYPE_ARR_FUTURE  "ARRFUTURE"
#define YIELD_TYPE_FRA3M  "FRA3M"
#define YIELD_TYPE_FRA6M  "FRA6M"
#define YIELD_TYPE_FUTURE  "FUTURE"
#define YIELD_TYPE_FWDFX  "FWDFX"
#define YIELD_TYPE_FWDFXCONST  "FWDFXCONST"
#define YIELD_TYPE_CTD  "CTD"	// Cheapest to deliver

// city
#define CITY_TkB  "TkB"
#define CITY_NYB  "NYB"
#define CITY_Tgt  "Tgt"
#define CITY_LnB  "LnB"
#define CITY_SyB  "SyB"
#define CITY_TrB  "TrB"
#define CITY_HKB  "HKB"
#define CITY_ZuB  "ZuB"
#define CITY_AuB  "AuB"
#define CITY_WeB  "WeB"
#define CITY_JoB  "JoB"

// sliding rule
#define SLIDING_MOD_FOLLOWING	"MOD_FOLLOWING"
#define SLIDING_FOLLOWING		"FOLLOWING"
#define SLIDING_PRECEDING		"PRECEDING"
#define SLIDING_MOD_PRECEDING	"MOD_PRECEDING"
#define SLIDING_NO_CHANGE		"NO_CHANGE"

// day count
#define DAYCOUNT_ACT_ACT  "ACT/ACT"
#define DAYCOUNT_ACT_365  "ACT/365"
#define DAYCOUNT_ACT_360  "ACT/360"
#define DAYCOUNT_30_360  "30/360"
#define DAYCOUNT_30E_360  "30E/360"
#define DAYCOUNT_ACT_365_ISDA  "ACT/365_ISDA"

#define PREFIX_VOL  "VOL_"
#define PREFIX_COR  "COR_"
#define PREFIX_YIELD  "YIELD_"

#define POSTFIX_VOL "_VOL"

#define MARKETTYPE_SWAP "SWAP" 
#define MARKETTYPE_BASIS "BASISSWAP" 
#define MARKETTYPE_ZERORATE "ZERORATE" 
#define CURVETYPE_FLOATER "FLOATER" 
#define CURVETYPE_ARBFREE "ARBFREE" 
#define CURVETYPE_BASIS "BASIS" 
#define CURVETYPE_SWAP "SWAP" 
#define CURVETYPE_OIS "OIS" 
#define CURVETYPE_FWDFXCONST "FWDFXCONST"
#define CURVETYPE_CHEAPESTTODELIVER "CHEAPESTTODELIVER"
#define CURVETYPE_ARR "ARR" 

#define CURVENAME_3ML "3ML"
#define CURVENAME_6ML "6ML"
#define CURVENAME_DF "DF"

//model
#define MODEL_LMM  "LMM"
#define MODEL_PTBERG  "PTBERG"
#define MODEL_SZ  "SZ"
#define MODEL_HW  "HW"
#define MODEL_IRVANILLA "IRVANILLA"
#define MODEL_FXVANILLA "FXVANILLA"
#define MODEL_FXSTRGLSLV "FXSTRGLSLV"
//hishida vannavolga
#define MODEL_FXVANNAVOLGA "FXVANNAVOLGA"
#define MODEL_IRSABR "IRSABR"

//property
#define PROPERTY_DIR   "data/properties/"
#define PROPERTY_FILE  "ir.properties"
#define CALIB_PROPERTY_FILE  "calib.properties"
#define RISK_PROPERTY_FILE  "risk.properties"
#define GRID_PROPERTY_FILE  "grid.properties"
#define XVA_PROPERTY_FILE  "xva.properties"
#define CREDIT_PROPERTY_FILE  "credit.properties"
#define MKTCOLLECTION_PROPERTY_FILE  "Mkt_Collection.properties"

// Delimiters
const char STATIC_DATA_DELIMITER  = '=';
const char MULTI_STATIC_DATA_DELIMITER = ':';
const char STATIC_DATA_COMMENT_OUT_CHAR = '#';

const char FX_DELIMITER = '/';
const char MARKET_DATA_DELIMITER = ',';

const int MAX_TERM = 40;

// return code
const int SUCCESS_CODE  = 0;
const int ALL_ERROR_CODE = 1;
const int SOME_ERROR_CODE = 2;

#define STD  "STD"

// NODATA
#define AQ_NO_DATA  "NO_DATA"

struct MAIndexData
{
	AQLString frequency;
	AQLString slidingrule;
	AQLString daycount;
	int resetlag;
	AQLStringVector cities; 
};
