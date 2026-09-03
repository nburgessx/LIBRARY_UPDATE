#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
print ( m.setUpMLIB("","") )

#Get the MLIB Version
print ( m.meUtilityVersion() )

print ( m.meLWOLoad("C:\Temp\EUR_OIS_CURVE@30.JSON") )
print ( m.meLWOLoad("C:\Temp\EUR_SWAP_3M_CURVE@18.JSON") )

#Set Payment Date Vectors
payDates = ["20190101", "20190401"]

#Get Discount Factor(s) - Single or Multiple Dates Accepted
print ( "Discount Factor" )
print ( m.meLWOCurveDiscountFactorsWithSpread(payDates,"EURYC","EUR3ML", 0.0) )

# 1bp spread
print ( m.meLWOCurveDiscountFactorsWithSpread(payDates,"EURYC","EUR3ML", 0.0001) )

