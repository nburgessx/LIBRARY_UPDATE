dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()

fromDate <- c("20190601")
toDate   <- c("20190901")
dayCount <- c("ACT/ACT")
includeLast <- c(TRUE)

yearfract <- meDateYearFraction( fromDate, toDate, dayCount, includeLast)
yearfract

yearfract <- meDateYearFraction( fromDate, toDate, dayCount )
yearfract