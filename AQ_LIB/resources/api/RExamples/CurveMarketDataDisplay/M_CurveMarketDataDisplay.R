dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()


filename <-c("E:/SavedCurves/OISCURVEMARKETDATA.TXT")
oisdata <- meLWOLoad(filename)


# Read each block of data into a variable

propertiesName <- c( "MARKETDATAPROPERTIES" )
properties <- meLWOCurveMarketDataDisplay( oisdata, propertiesName)

oisName <- c("OIS")
oisSwaps <- meLWOCurveMarketDataDisplay( oisdata, oisName)

liborOisBasisName <- c("LIBOROISBASISSPREADS")
liborOisBasisSwaps <- meLWOCurveMarketDataDisplay( oisdata, liborOisBasisName)

liborSwapsName <- c("SWAPS")
liborSwaps <- meLWOCurveMarketDataDisplay( oisdata, liborSwapsName)

# Display one of the blocks
liborSwaps

