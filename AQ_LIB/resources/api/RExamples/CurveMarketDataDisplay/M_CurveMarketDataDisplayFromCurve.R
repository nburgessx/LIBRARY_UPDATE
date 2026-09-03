dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()


filename <-c("E:/SavedCurves/USD_OIS_CURVE.TXT")
oisCurve <- meLWOLoad(filename)

blockname <- c("SWAPS")
block <- meLWOCurveMarketDataDisplayFromCurve(oisCurve, blockname)

block

