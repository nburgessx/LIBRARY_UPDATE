#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
m.setUpMLIB("","")

#Get the MLIB Version
m.meUtilityVersion()

# ---- Fixed Schedule ----

# Fixed Schedule Block Keys & Values
keys = ( "EffectiveDate", "MaturityDate", "AccrualDayCount", "AccrualFrequency", "AccrualBusinessDayAdjustment", "AccrualCalendar", "PaymentFrequency", "PaymentBusinessDayAdjustment", "PaymentCalendar", "PaymentLag", "RollDay" )

values = ( "20160501", "20200522", "30/360", "Semi-Annual", "NO_CHANGE", "TGT", "Semi-Annual", "NO_CHANGE", "TGT", "0D", "" )

#Create a LVB
fixedScheduleLVB = m.meUtilityLVBCreate( keys, values )
fixedScheduleLVB

showColumnHeaders = True

fixedSchedule = m.meProductSwapScheduleFixed( showColumnHeaders, fixedScheduleLVB )
print (fixedSchedule)

# ---- Float Schedule ----

# Float Schedule Block Keys & Values
keys = ( "EffectiveDate", "MaturityDate", "AccrualDayCount", "AccrualFrequency", "AccrualBusinessDayAdjustment", "AccrualCalendar", "PaymentFrequency", "PaymentBusinessDayAdjustment", "PaymentCalendar", "PaymentLag", "FixingBusinessDayAdjument", "FixingCalendar", "FixingLag", "FirstStubDate", "LastStubDate", "RollDay" )

values = ( "20160501", "20200522", "30/360", "Semi-Annual", "NO_CHANGE", "TGT", "Semi-Annual", "NO_CHANGE", "TGT", "0D", "NO_CHANGE", "TGT", "0D", "", "", "" )

#Create a LVB
floatScheduleLVB = m.meUtilityLVBCreate( keys, values )
floatScheduleLVB

showColumnHeaders = True

floatSchedule = m.meProductSwapScheduleFloat( showColumnHeaders, floatScheduleLVB )
print (floatSchedule)


	
	