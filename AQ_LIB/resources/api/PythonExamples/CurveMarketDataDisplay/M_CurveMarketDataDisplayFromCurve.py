#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
m.setUpMLIB("","")

#Get the MLIB Version
m.meUtilityVersion()


# Display a block of data from a saved curve

curveFilename="E:\SavedCurves\USD_OIS_CURVE.TXT"
oisCurve = m.meLWOLoad(curveFilename)

blockname = "SWAPS"
block= m.meLWOCurveMarketDataDisplayFromCurve( oisCurve, blockname)

for row in block:
  for item in row:
    print item,
  print
  
