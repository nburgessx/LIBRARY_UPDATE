#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
m.setUpMLIB("","")

#Get the MLIB Version
m.meUtilityVersion()

# Load a saved market data file

filename="E:\SavedCurves\OISCURVEMARKETDATA.TXT"
oisdata =m.meLWOLoad(filename)

# Read each block of data into a variable

propertiesName = "MARKETDATAPROPERTIES"
properties = m.meLWOCurveMarketDataDisplay( oisdata, propertiesName)

oisName = "OIS"
oisSwaps = m.meLWOCurveMarketDataDisplay( oisdata, oisName)

liborOisBasisName = "LIBOROISBASISSPREADS"
liborOisBasisSwaps = m.meLWOCurveMarketDataDisplay( oisdata, liborOisBasisName)

liborSwapsName = "SWAPS"
liborSwaps = m.meLWOCurveMarketDataDisplay( oisdata, liborSwapsName)

  
  
# Turn on MLIB recording, in order to check that parameters are being successfully passed through to MLIB.

m.meUtilityRecord(True,"E:/Record", False, 0, 10)

# Now reconstruct the curve market data object

objectName = "USD_OIS_DATA"

mktDataObject = m.meLWOCurveMarketDataCreate( objectName, propertiesName, properties, oisName, oisSwaps, liborOisBasisName, liborOisBasisSwaps, liborSwapsName, liborSwaps )



 
 