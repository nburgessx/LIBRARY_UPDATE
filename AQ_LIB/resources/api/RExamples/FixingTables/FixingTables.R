dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()
 
setUpMLIB("","")
	
meUtilityVersion()

fixingDates <-c( "2019-08-07", "2019-08-06", "2017-08-05", "2017-08-04", "2017-08-03", "2017-08-02", "2017-08-01")
fixingValues <-c ( 7.7, 6.6, 5.5, 4.4, 3.3, 2.2, 1.1 )

handle = "FixingsEUR6ML"
meLWOFixingTableCreate(handle,"EUR","SEMI-ANNUAL",fixingDates,fixingValues)

fixingDate = "2019-08-07"
meLWOFixingTableValue(handle, fixingDate )

meLWOFixingTableValues(handle, fixingDates )
