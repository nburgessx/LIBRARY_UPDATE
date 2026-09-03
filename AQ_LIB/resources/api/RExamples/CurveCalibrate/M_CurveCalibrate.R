dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()

# Build a curve using saved market data

marketDataFilename <-c("E:/SavedCurves/OISCURVEMARKETDATA.TXT")
marketData <- meLWOLoad(marketDataFilename)


oisCurveName <- c("USD_OIS")
curveGeneratorName <- c("USD_OIS")   # Specifies the curve conventions
curveCollection <-c("USDYC")
foreignCollection<-c("")
curveIndexNameAndAliases <- meLWOCurveCalibrate( oisCurveName, curveGeneratorName, marketData, curveCollection, foreignCollection )

curveIndexNameAndAliases

# Read the curve data back out of the newly built curve

blockname <-c("SWAPS")
swapsData <- meLWOCurveMarketDataDisplayFromCurve( oisCurveName, blockname)

swapsData


