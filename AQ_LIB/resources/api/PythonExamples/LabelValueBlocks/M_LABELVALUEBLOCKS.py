#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
print ( m.setUpMLIB("","") )

#Get the MLIB Version
print ( m.meUtilityVersion() )

#Label Value Block Keys & Values
keys = ( "k1", "k2", "k3" )
values = ( "v1", "v2", "v3" )
newKey = "k4"
newValue = "v4"

#Create a LVB
myLVB = m.meUtilityLVBCreate( keys, values )
print ("Create LVB")
print (myLVB)

#Append to an Existing LVB
newLVB = m.meUtilityLVBAppendAndCreate( myLVB, newKey, newValue )
print ("Append & Create New LVB")
print (newLVB)


