dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()

# ---- Fixed Schedule ----

# Fixed Schedule Block Keys & Values
keys <- c( "EffectiveDate", "MaturityDate", "AccrualDayCount", "AccrualFrequency", "AccrualBusinessDayAdjustment", "AccrualCalendar", "PaymentFrequency", "PaymentBusinessDayAdjustment", "PaymentCalendar", "PaymentLag", "RollDay" )

values <- c( "20160501", "20200522", "30/360", "Semi-Annual", "NO_CHANGE", "TGT", "Semi-Annual", "NO_CHANGE", "TGT", "0D", "" )

#Create a LVB
fixedScheduleLVB <- meUtilityLVBCreate( keys, values )
fixedScheduleLVB

showColumnHeaders <- c(TRUE)

fixedSchedule <- meProductSwapScheduleFixed( showColumnHeaders, fixedScheduleLVB )
fixedSchedule

# ---- Float Schedule ----

# Float Schedule Block Keys & Values
keys <- c( "EffectiveDate", "MaturityDate", "AccrualDayCount", "AccrualFrequency", "AccrualBusinessDayAdjustment", "AccrualCalendar", "PaymentFrequency", "PaymentBusinessDayAdjustment", "PaymentCalendar", "PaymentLag", "FixingBusinessDayAdjument", "FixingCalendar", "FixingLag", "FirstStubDate", "LastStubDate", "RollDay" )

values <- c( "20160501", "20200522", "30/360", "Semi-Annual", "NO_CHANGE", "TGT", "Semi-Annual", "NO_CHANGE", "TGT", "0D", "NO_CHANGE", "TGT", "0D", "", "", "" )

#Create a LVB
floatScheduleLVB <- meUtilityLVBCreate( keys, values )
floatScheduleLVB

showColumnHeaders = TRUE

floatSchedule <- meProductSwapScheduleFloat( showColumnHeaders, floatScheduleLVB )
floatSchedule



