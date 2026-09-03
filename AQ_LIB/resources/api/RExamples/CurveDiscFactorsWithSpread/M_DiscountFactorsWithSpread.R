dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()


oisFilename <- c("C:/Temp/EUR_OIS_CURVE@30.JSON")
stdFilename <- c("C:/Temp/EUR_SWAP_3M_CURVE@18.JSON")

meLWOLoad( oisFilename )
meLWOLoad( stdFilename )

#Set Payment Date Vectors
payDates <- c( "20190101", "20190401" )

#Get Discount Factor(s) - Single or Multiple Dates Accepted
meLWOCurveDiscountFactorsWithSpread(payDates,"EURYC","EUR3ML", 0.0) 

# 1bp spread
meLWOCurveDiscountFactorsWithSpread(payDates,"EURYC","EUR3ML", 0.0001)

