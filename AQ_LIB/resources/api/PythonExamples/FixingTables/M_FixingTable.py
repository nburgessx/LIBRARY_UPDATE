# Import the MLIB CLIENT API
import MLIB_CLIENT_API as m


# Load Calendars and Config Files
print( "Initialize MLIB" )
print( m.setUpMLIB() )
 

# Get the MLIB Version
print()
print( "Library Version" )
print( m.meUtilityVersion() )


# Fixing Data
handle = "FixingTableEUR6ML"
fixingDates = ( "2017-08-07", "2017-08-06", "2017-08-05", "2017-08-04", "2017-08-03", "2017-08-02", "2017-08-01")
fixingValues = ( 7.7, 6.6, 5.5, 4.4, 3.3, 2.2, 1.1 )


# Create a Fixing Table
print()
print( "Create Fixing Table" )
print( m.meLWOFixingTableCreate( handle, "EUR", "SEMI-ANNUAL", fixingDates, fixingValues ) )

# Display Fixing Table
print()
print( "Display Fixing Table" )
print( m.meLWOFixingTableDisplay( handle ) )

# Get a Single Fixing Value
fixingDate = "2017-08-07"
print()
print( "Fixing Value for", fixingDate )
print( m.meLWOFixingTableValue( handle, fixingDate ) )

# Get a Vector of Fixing Values
print()
print( "Fixing Values" )
print( m.meLWOFixingTableValues( handle, fixingDates ) )
print()

