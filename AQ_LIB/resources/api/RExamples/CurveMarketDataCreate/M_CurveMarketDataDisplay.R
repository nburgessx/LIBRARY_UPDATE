dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()

# Load a saved market data file
filename <-c("E:/SavedCurves/OISCURVEMARKETDATA.TXT")
oisdata <- meLWOLoad(filename)

# Read each block of data into a variable

propertiesName <- c( "MARKETDATAPROPERTIES" )
properties <- meLWOCurveMarketDataDisplay( oisdata, propertiesName)

oisName <- c( "OIS" )
oisSwaps <- meLWOCurveMarketDataDisplay( oisdata, oisName)

liborOisBasisName <- c( "LIBOROISBASISSPREADS" )
liborOisBasisSwaps <- meLWOCurveMarketDataDisplay( oisdata, liborOisBasisName)

liborSwapsName  <- c("SWAPS")
liborSwaps <- meLWOCurveMarketDataDisplay(oisdata, liborSwapsName)

# Turn on MLIB recording, in order to check that parameters are being successfully passed through to MLIB.

meUtilityRecord(TRUE,"E:/Record", FALSE, 0, 10)

# Now reconstruct the curve market data object

objectName <- c( "USD_OIS_DATA" )

mktDataObject <- meLWOCurveMarketDataCreate( objectName, propertiesName, properties, oisName, oisSwaps, liborOisBasisName, liborOisBasisSwaps, liborSwapsName, liborSwaps )



