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

# Display each block of data as a matrix

for row in properties:
  for item in row:
    print item,
  print
  
for row in oisSwaps:
  for item in row:
    print item,
  print
  
for row in liborOisBasisSwaps:
  for item in row:
    print item,
  print

for row in liborSwaps:
  for item in row:
    print item,
  print
  
