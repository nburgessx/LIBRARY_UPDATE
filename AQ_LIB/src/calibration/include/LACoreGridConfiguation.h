#pragma once

#define IRG_VERSION_NO  "9.2.00.010"

#ifdef _DEBUG
 #define IRG_RELEASE_STR "Debug"
 #define IRG_RELEASE_SIMPLE_STR "d"
#else
 #define IRG_RELEASE_STR "Release"
 #define IRG_RELEASE_SIMPLE_STR ""
#endif
#ifdef __XEON_WITH_MIC__
 #define IRG_APP_PREFIX "mic grid"
 #define IRG_VERSION_PREFIX "M"
#else
 #define IRG_APP_PREFIX "grid"
 #define IRG_VERSION_PREFIX ""
#endif
#ifdef USE_QUANTLIB_SVD
 #define IRG_BUILD_SUFFIX " with qlib SVD"
 #define IRG_BUILD_SIMPLE_SUFFIX "_S"
#else
 #define IRG_BUILD_SUFFIX ""
 #define IRG_BUILD_SIMPLE_SUFFIX ""
#endif


// Session Message Type
#define SMT_UNKNOWN					 0
#define SMT_SYNC_CALENDAR			 1
#define SMT_SYNC_COEFFICIENT		 2
#define SMT_SYNC_RISK_COEFFICIENT	 3 
#define SMT_ONE_PORTFOLIO			10	// for exo Intel/MIC grid
#define SMT_MULTI_PORTFOLIOS		20	// for common multi portfolio grid
#define SMT_ALL_REQUEST				30	// for vanilla MIC grid

// Input Message Type
#define IMT_UNKNOWN		 0
#define IMT_PREPARE		11
#define IMT_LSMC		12
#define IMT_MC			13
#define IMT_PV			14
#define IMT_PRINT_ERROR	15
#define IMT_PRINT_MC	16
#define IMT_PRINT_PV	17
#define IMT_PORTFOLIOS	21
#define IMT_REQUEST     31

// Output Message Type
#define OMT_UNKNOWN		    	 0
#define OMT_PREPARE		    	11
#define OMT_LSMC		    	12
#define OMT_MC			    	13
#define OMT_PV   		    	14
#define OMT_PRINT_ERROR			15
#define OMT_PRINT_MC			16
#define OMT_PRINT_PV			17
#define OMT_PORTFOLIOS 			21
#define OMT_CALIB_PORTFOLIOS	22
#define OMT_REQUEST				31
#define OMT_FATAL_ERROR     	91
#define OMT_FAILURE_ERROR   	92

// return code
#define IRG_RET_SUCCESS               0
#define IRG_RET_ERROR                 1
#define IRG_RET_ALERT                 2
#define IRG_RET_EOD_ALREADY_RUNNING   11
#define IRG_RET_EOD_INTERRUPT         12
#define IRG_RET_FORCED_TERMINATE      99

#define _S(p)  (p ? p : "")
#define EXTENDED_MIC_RESOURCE_POSTFIX  "_mic_resources_extended"


// properties file
#define IR_PROPERTY_FILENAME   "ir.properties"
#define GRID_PROPERTY_FILENAME "grid.properties"

// argument keys
// [CAUTION] The below defines must be synchronized with LADefinitions 
#define IRG_ARG_KEY_FILENUM          "-filenum"
#define IRG_ARG_KEY_MARKETID         "-marketid"
#define IRG_ARG_KEY_MLIBID           "-mlibid"
#define IRG_ARG_KEY_PROPERTIESID     "-propertiesid"

#define IRG_ARG_KEY_BASESCENARIONUM  "-basescenarionum"
#define IRG_ARG_KEY_CALENDAR_CITY    "-calendar_city"

// [CAUTION] The below defines are only used by grid client
#define IRG_ARG_KEY_NO_ARCHIVE       "-noarchive"

// [CAUTION] The below defines are only used by mic grid client
#define IRG_ARG_KEY_REQUEST_ID         "-requestid"
#define IRG_ARG_KEY_TEMPLATE_DATA_DIR  "-template_data"
#define IRG_ARG_KEY_DATA_DIR           "-data"
#define IRG_ARG_KEY_ARG_LIST           "-arg_list"
#define IRG_ARG_KEY_GRIDRANGE          "-gridrange"

#define IRG_ARG_KEY_CURVE_EXPORT       "-curve_export"
#define IRG_ARG_KEY_CURVE_IMPORT       "-curve_import"

#define IRG_ARG_KEY_SABR_CALIB_EXPORT  "-sabr_calib_export"
#define IRG_ARG_KEY_SABR_CALIB_IMPORT  "-sabr_calib_import"


// ir properties key
// [CAUTION] The below defines must be synchronized with LADefinitions.
#define KEY_IR_SIMULATION_MCNUM        "simulation.mcnum"
#define KEY_IR_SIMULATION_LSMC_MCNUM   "simulation.lsmc.mcnum"

#define KEY_IR_RISK_SCENARIO_FILE    "risk.scenario.file"
#define KEY_IR_CALIB_SCENARIO_FILE   "calib.scenario.file"
#define KEY_IR_DEAL_FILE             "deal.file"
#define KEY_IR_CALENDER_FILE         "calender.file"
#define KEY_IR_DEAL_SUMMARY_INFO     "deal.summary.info"

#define KEY_IR_DEAL_PV_FILE          "deal.pv.file"
#define KEY_IR_CALIB_SERIALIZE_FILE  "calib.serialize.file"


// grid properties key
#define KEY_GRID_APPLICATION_NAME		 "grid.application.name"
#define KEY_GRID_APPLICATION_USERNAME	 "grid.application.username"
#define KEY_GRID_APPLICATION_PASSWORD	 "grid.application.password"

#define KEY_MIC_APPLICATION_NAME		     "mic.grid.application.name"
#define KEY_MIC_APPLICATION_USERNAME	     "mic.grid.application.username"
#define KEY_MIC_APPLICATION_PASSWORD	     "mic.grid.application.password"

#define KEY_MIC_VANILLA_GRID_APPLICATION_NAME		 "mic.vanilla.grid.application.name"
#define KEY_MIC_VANILLA_GRID_APPLICATION_USERNAME	 "mic.vanilla.grid.application.username"
#define KEY_MIC_VANILLA_GRID_APPLICATION_PASSWORD	 "mic.vanilla.grid.application.password"

#define KEY_VANILLA_GRID_APPLICATION_NAME		 "vanilla.grid.application.name"
#define KEY_VANILLA_GRID_APPLICATION_USERNAME	 "vanilla.grid.application.username"
#define KEY_VANILLA_GRID_APPLICATION_PASSWORD	 "vanilla.grid.application.password"

#define KEY_CALIB_GRID_APPLICATION_NAME		 "calib.grid.application.name"
#define KEY_CALIB_GRID_APPLICATION_USERNAME	 "calib.grid.application.username"
#define KEY_CALIB_GRID_APPLICATION_PASSWORD	 "calib.grid.application.password"

#define KEY_GRID_TASK_TIMEOUT			 "grid.task.timeout"
#define KEY_GRID_TASK_MAX_MEMORY         "grid.task.maxmemory"
#define KEY_GRID_LOG_FILE                "grid.log.file"
#define KEY_GRID_LOG_LEVEL               "grid.log.level"

#define KEY_GRID_LSMC_SESSIONTYPE		"grid.lsmc.sessiontype"
#define KEY_GRID_LSMC_SESSIONNAME		"grid.lsmc.sessionname"
#define KEY_GRID_LSMC_SESSIONTAG		"grid.lsmc.sessiontag"
#define KEY_GRID_LSMC_DISTRIBUTE		"grid.lsmc.distribute"
#define KEY_GRID_MC_SESSIONTYPE			"grid.mc.sessiontype"
#define KEY_GRID_MC_SESSIONNAME			"grid.mc.sessionname"
#define KEY_GRID_MC_SESSIONTAG			"grid.mc.sessiontag"
#define KEY_GRID_MC_DISTRIBUTE			"grid.mc.distribute"
#define KEY_GRID_PREPARE_SESSIONTYPE	"grid.prepare.sessiontype"
#define KEY_GRID_PREPARE_SESSIONNAME	"grid.prepare.sessionname"
#define KEY_GRID_PREPARE_SESSIONTAG		"grid.prepare.sessiontag"
#define KEY_GRID_PREPARE_DISTRIBUTE		"grid.prepare.distribute"
#define KEY_GRID_RESULT_SESSIONTYPE		"grid.result.sessiontype"
#define KEY_GRID_RESULT_SESSIONNAME		"grid.result.sessionname"
#define KEY_GRID_RESULT_SESSIONTAG		"grid.result.sessiontag"
#define KEY_GRID_RESULT_DISTRIBUTE		"grid.result.distribute"

#define KEY_GRID_PV_SESSIONTYPE           "grid.pv.sessiontype"
#define KEY_GRID_PV_SESSIONNAME           "grid.pv.sessionname"
#define KEY_GRID_PV_SESSIONTAG			 "grid.pv.sessiontag"

#define KEY_GRID_MIC_VANILLA_SESSIONTYPE         "grid.mic.vanilla.sessiontype"
#define KEY_GRID_MIC_VANILLA_SESSIONNAME         "grid.mic.vanilla.sessionname"
#define KEY_GRID_MIC_VANILLA_SESSIONTAG          "grid.mic.vanilla.sessiontag"
#define KEY_GRID_MIC_VANILLA_DISTRIBUTE          "grid.mic.vanilla.distribute"
#define KEY_GRID_MIC_VANILLA_HOST_DISTRIBUTE     "grid.mic.vanilla.host.distribute"
#define KEY_GRID_MIC_VANILLA_LIST                "grid.mic.vanilla.list"
#define KEY_GRID_MIC_VANILLA_TRADE_MAX           "grid.mic.vanilla.trade.max"

#define KEY_GRID_MIC_VANILLA_DELETE_DATA_FLAG_ON_MIC  "grid.mic.vanilla.delete.data.on.mic"
#define KEY_GRID_MIC_VANILLA_LOG_LEVEL                "grid.mic.vanilla.log.level.on.mic"
#define KEY_GRID_MIC_VANILLA_LOG_PREFIX               "grid.mic.vanilla.log.prefix.on.mic"
#define KEY_GRID_MIC_VANILLA_LOG_DIRNAME              "grid.mic.vanilla.log.dirname.on.mic"

#define KEY_GRID_VANILLA_SESSIONTYPE     "grid.vanilla.sessiontype"
#define KEY_GRID_VANILLA_SESSIONNAME     "grid.vanilla.sessionname"
#define KEY_GRID_VANILLA_SESSIONTAG      "grid.vanilla.sessiontag"
#define KEY_GRID_VANILLA_DISTRIBUTE      "grid.vanilla.distribute"

#define KEY_GRID_CALIB_SESSIONTYPE     "grid.calib.sessiontype"
#define KEY_GRID_CALIB_SESSIONNAME     "grid.calib.sessionname"
#define KEY_GRID_CALIB_SESSIONTAG      "grid.calib.sessiontag"
#define KEY_GRID_CALIB_DISTRIBUTE      "grid.calib.distribute"

#define KEY_GRID_MIC_EXO_TARGETNUM               "grid.mic.exo.targetnum"
#define KEY_GRID_MIC_EXO_PORT                    "grid.mic.exo.port"
#define KEY_GRID_MIC_PV_DISTRIBUTE               "grid.mic.pv.distribute"
#define KEY_GRID_MIC_MACHINES                    "grid.mic.machine"
#define KEY_GRID_MIC_MACHINES_EXTRA              "grid.mic.machine.extra"

// for dataValues
#define PRICING_DATA_XEONPHI_SEND_NUM					"XeonPhiSendNum"
#define PRICING_DATA_XEONPHI_SEND_TIMEOUT_NUM			"XeonPhiSendTimeoutNum"
#define PRICING_DATA_SELECTED_XEONPHI_SERVERS			"SelectedXeonPhiServers"

// for pricer dataValues
#define IRG_DATA_CASHLET_RESULT_LEGTYPE					"LegType" 
#define IRG_DATA_CASHLET_RESULT_PAYMENTSCHEDULE			"PaymentSchedule" 
#define IRG_DATA_CASHLET_RESULT_CASHLET					"Cashlet" 
#define IRG_DATA_CASHLET_RESULT_PV						"PV" 
#define IRG_DATA_CASHLET_RESULT_TRIGGERVALUETIME		"TriggerValueTime" 
#define IRG_DATA_CASHLET_RESULT_TRIGGERVALUE			"TriggerValue" 
#define IRG_DATA_CASHLET_RESULT_TRIGGERPV				"TriggerPV" 
#define IRG_DATA_CASHLET_RESULT_EXTRACFVALUETIME		"ExtraCFValueTime" 
#define IRG_DATA_CASHLET_RESULT_EXTRACFVALUE			"ExtraCFValue" 
#define IRG_DATA_CASHLET_RESULT_EXTRACFPV				"ExtraCFPV" 
#define IRG_DATA_CASHLET_RESULT_CALLVALUETIME			"CallValueTime" 
#define IRG_DATA_CASHLET_RESULT_CALLVALUE				"CallValue" 

#define IRG_DATA_CALLTRIGGER_RESULT_PROBABILITYDATE		"ProbabilityDate"
#define IRG_DATA_CALLTRIGGER_RESULT_PROBABILITYVALUE	"ProbabilityValue"

#define IRG_DATA_LSMCDETAILOUTPUT_RESULT_EXPLANATORY	"Explanatory"
#define IRG_DATA_LSMCDETAILOUTPUT_RESULT_EXPLAINED		"Explained"
#define IRG_DATA_LSMCDETAILOUTPUT_RESULT_CALLREBATE		"CallRebate"
#define IRG_DATA_LSMCDETAILOUTPUT_RESULT_CALLPOSTJUDGE	"CallPostJudge"
#define IRG_DATA_LSMCDETAILOUTPUT_RESULT_CALLJUDGE		"CallJudge"
#define IRG_DATA_LSMCDETAILOUTPUT_RESULT_COEFFICIENTS	"Coefficients"
#define IRG_DATA_LSMCDETAILOUTPUT_RESULT_COEFFICIENTS2	"Coefficients2"

