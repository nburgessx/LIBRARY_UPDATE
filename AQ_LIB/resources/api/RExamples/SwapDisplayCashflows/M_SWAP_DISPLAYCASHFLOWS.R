dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()


#Load Curves
print ( "Load Curves" )
print ( "-----------" )
print ( meLWOLoad("C:\\Temp\\USD_OIS_CURVE.JSON") )
print ( meLWOLoad("C:\\Temp\\USD_SWAP_3M_CURVE.JSON") )

#Swap Definition
swapName = c("IRS_USD3ML_1PCT_2Y")
swapGenerator = c("USD_3ML")

swapLVBKeys = c( "PayReceive1", "Notional", "EffectiveDate", "MaturityDate", "RateOrSpread1", "RateOrSpread2" )
swapLVBvals = c( "Pay", "1000000", "20180702", "2Y", "0.01", "0.0" )

swapLVB = meUtilityLVBCreate( swapLVBKeys, swapLVBvals )

xccyLVB = c()
isXccySwap = c( FALSE) 
validateKeys = c( FALSE) 
fixedLeg = c( "Leg1:Fixed" )
floatLeg = c( "Leg2:Float" )
total = c( "" )

#Create Swap
print ( "Create Swap" )
print ( "-----------" )
swapObject = meLWOSwapCreateFromGenerator( swapName, swapGenerator, swapLVB, xccyLVB, isXccySwap, validateKeys )
print ( swapObject )


#Valuation Settings
valuationSettings = c( "CurveCollection","USDYC" )
fixingTableLVB = c()

#Price Swap
print ( "PV" )
meLWOSwapPV( swapObject, valuationSettings, total, fixingTableLVB )
meLWOSwapPV( swapObject, valuationSettings, fixedLeg, fixingTableLVB )
meLWOSwapPV( swapObject, valuationSettings, floatLeg, fixingTableLVB )


print ( "ParRate" )
meLWOSwapParRate( swapObject, valuationSettings, fixingTableLVB )


print ( "PV01" )
meLWOSwapPV01( swapObject, valuationSettings, fixingTableLVB )


print ( "Annuity" )
meLWOSwapAnnuity( swapObject, valuationSettings, fixedLeg )
meLWOSwapAnnuity( swapObject, valuationSettings, floatLeg )


#Display Swap Cashflows
print ( "Display Swap Cashflows" )
print ( "-----------" )
legName = c("Leg1:FIXED")
fixingTableNames = c("")
showColumnHeaders = c(TRUE)

swapCashflows = meLWOSwapDisplayCashflows( swapName, valuationSettings, legName, fixingTableNames, showColumnHeaders )

