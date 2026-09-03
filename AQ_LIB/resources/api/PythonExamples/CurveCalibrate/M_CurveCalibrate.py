#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
m.setUpMLIB("","")

#Get the MLIB Version
m.meUtilityVersion()


# Build a curve using saved market data

marketDataFilename="E:\SavedCurves\OISCURVEMARKETDATA.TXT"
marketData = m.meLWOLoad(marketDataFilename)

oisCurveName = "USD_OIS"
curveGeneratorName ="USD_OIS" # Specifies the curve conventions
curveCollection = "USDYC"
foreignCollection =""
curveIndexNameAndAliases = m.meLWOCurveCalibrate( oisCurveName, curveGeneratorName, marketData, curveCollection, foreignCollection )

curveIndexNameAndAliases

# Read the curve data back out of the newly built curve

blockname = "SWAPS"
block= m.meLWOCurveMarketDataDisplayFromCurve( oisCurveName, blockname)

for row in block:
  for item in row:
    print item,
  print
  


