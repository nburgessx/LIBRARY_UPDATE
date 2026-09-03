#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
print ( m.setUpMLIB("","") )

#Get the MLIB Version
print ( m.meUtilityVersion() )


print ( m.meLWOLoad("C://EURYC_EUR3ML.JSON") )

#Get Curve Inputs
print ("Curve Inputs")
print ( m.meLWOCurveDisplay("EURYC_EUR3ML") ) # Use Handle Name

# print ( m.meLWOGridDisplay("EURYC_EUR3ML", False) ) # Use Handle Name
print ( m.meLWOGridDisplay("EURYC_EUR3ML") ) # Use Handle Name

#Get Pseudo Discount Factor(s)
print ( "Psuedo Discount Factors" )
print ( m.meCurveResultsDisplayDiscountFactors("EURYC","EUR3ML") ) # Curve Collection, Curve Index


