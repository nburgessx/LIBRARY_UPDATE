#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
print ( m.setUpMLIB("","") )

#Get the MLIB Version
print ( m.meUtilityVersion() )

#Load Curves
print ( "Load Curves" )
print ( "-----------" )
print ( m.meLWOLoad("C:\\Temp\\USD_OIS_CURVE.JSON") )
print ( m.meLWOLoad("C:\\Temp\\USD_SWAP_3M_CURVE.JSON") )
print

#Swap Definition
swapName = "IRS_USD3ML_1PCT_2Y"
swapGenerator = "USD_3ML"
swapLVB = (["PayReceive1","Pay"],
           ["Notional","1000000"],
           ["EffectiveDate","20180702"],
           ["MaturityDate","2Y"],
           ["RateOrSpread1","0.01"],
           ["RateOrSpread2","0.0"])
xccyLVB = ([])
isXccySwap = False
validateKeys = True
fixedLeg = "Leg1:Fixed"
floatLeg = "Leg2:Float"
total = ""

#Create Swap
print ( "Create Swap" )
print ( "-----------" )
swapObject = m.meLWOSwapCreateFromGenerator( swapName, swapGenerator, swapLVB, xccyLVB, isXccySwap, validateKeys )
print ( swapObject )
print

#Valuation Settings
valuationSettings = (["CurveCollection","USDYC"],)
fixingTableLVB = ([])

#Price Swap
print ( "PV" )
print ( "Total: ", m.meLWOSwapPV( swapObject, valuationSettings, total, fixingTableLVB ) )
print ( "FixedLeg: ", m.meLWOSwapPV( swapObject, valuationSettings, fixedLeg, fixingTableLVB ) )
print ( "FloatLeg: ", m.meLWOSwapPV( swapObject, valuationSettings, floatLeg, fixingTableLVB ) )
print

print ( "ParRate" )
print ( m.meLWOSwapParRate( swapObject, valuationSettings, fixingTableLVB ) )
print

print ( "PV01" )
print ( m.meLWOSwapPV01( swapObject, valuationSettings, fixingTableLVB ) )
print

print ( "Annuity" )
print ( "Fixed: ", m.meLWOSwapAnnuity( swapObject, valuationSettings, fixedLeg ) )
print ( "Float: ", m.meLWOSwapAnnuity( swapObject, valuationSettings, floatLeg ) )
print
