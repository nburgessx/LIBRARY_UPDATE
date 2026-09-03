dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()

  
setUpMLIB("","")
	
meUtilityVersion()
  
mlib_ois  = meLWOLoad("C:/Temp/EUR_OIS_CURVE@30.JSON")
mlib_ois
  
mlib_swap = meLWOLoad("C:/Temp/EUR_SWAP_3M_CURVE@18.JSON") 
mlib_swap
  


#Get Curve Inputs
meLWOCurveDisplay(mlib_swap) # Use Handle Name

#Get Pseudo Discount Factor(s)
meCurveResultsDisplayDiscountFactors("EURYC","EUR3ML") # Curve Collection, Curve Index


