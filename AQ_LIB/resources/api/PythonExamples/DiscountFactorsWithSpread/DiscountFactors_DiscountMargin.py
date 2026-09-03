import MLIB_CLIENT_API as m

# Load Calendars and Config Files
print( "Initialize MLIB" )
print( m.setUpMLIB() )
 
# Get the MLIB Version
print()
print( "Library Version" )
print( m.meUtilityVersion() )


# Fixing Data
handle = "FixingTableEUR3ML"
fixingDates = ( "2019-12-04", )
fixingValues = ( 0.005, )

# Create a Fixing Table
print()
print( "Create Fixing Table" )
myFixingTable =  m.meLWOFixingTableCreate( handle, "EUR", "QUARTERLY", fixingDates, fixingValues )

# Check the fixing table is working
fixingDate = "2019-12-04"
print ( m.meLWOFixingTableValue( myFixingTable, fixingDate ) )

# Load your curves
EUROISFile = "E:/MLIB/MLIBQ_DEV1/resource/api/PythonExamples/DiscountFactorsWithSpread/EUR_OIS_CURVE@24.JSON"
EUROISCurve = m.meLWOLoad( EUROISFile )

EUR3MLFile = "E:/MLIB/MLIBQ_DEV1/resource/api/PythonExamples/DiscountFactorsWithSpread/EUR_SWAP_3M_CURVE@22.JSON"
EUR3MLCurve = m.meLWOLoad( EUR3MLFile )


paymentDates = ("2020-03-06", "2020-06-06")
curveCollection = "EURYC"
curveIndex = "EUR3ML"

# First calculate discount factors with no spread
spread = 0.0
m.meLWOCurveDiscountFactorsWithSpread( paymentDates, curveCollection, curveIndex, spread, myFixingTable )
 
# Now calculate discount factors with a 5% spread
spread = 0.05
m.meLWOCurveDiscountFactorsWithSpread( paymentDates, curveCollection, curveIndex, spread, myFixingTable )

