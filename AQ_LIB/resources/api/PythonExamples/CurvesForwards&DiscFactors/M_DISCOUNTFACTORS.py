#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
print ( m.setUpMLIB("","") )

#Get the MLIB Version
print ( m.meUtilityVersion() )

print ( m.meLWOLoad("C:\Temp\EUR_OIS_CURVE@30.JSON") )
print ( m.meLWOLoad("C:\Temp\EUR_SWAP_3M_CURVE@18.JSON") )

#Set Fixing and Payment Date Vectors
fixingDates = ["20190101"]
payDates = ["20190101"]

#Get Discount Factor(s) - Single or Multiple Dates Accepted
print ( "Discount Factor" )
print ( m.meCurveDiscountFactors(payDates,"EURYC","EUROIS") )

#Get Forward Rate(s) - Single or Multiple Dates Accepted
print ("Forward Rate")
print (m.meCurveForwardRates(fixingDates,"EURYC","EUR3ML") )
