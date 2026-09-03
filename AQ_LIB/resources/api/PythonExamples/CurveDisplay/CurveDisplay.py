#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
print ( m.setUpMLIB("","") )

#Get the MLIB Version
print ( m.meUtilityVersion() )

print ( m.meLWOLoad("C:\Temp\EUR_OIS_CURVE@30.JSON") )
print ( m.meLWOLoad("C:\Temp\EUR_SWAP_3M_CURVE@18.JSON") )

#Get Curve Inputs
print ("Curve Inputs")
print ( m.meLWOCurveDisplay("EUR_SWAP_3M_CURVE@18") ) # Use Handle Name

#Get Pseudo Discount Factor(s)
print ( "Psuedo Discount Factors" )
print ( m.meCurveResultsDisplayDiscountFactors("EURYC","EUR3ML") ) # Curve Collection, Curve Index


