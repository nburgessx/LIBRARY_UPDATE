#Import Utilities
import numpy as np
import matplotlib.pyplot as plt

#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
print m.setUpMLIB("","")
print

#Get the MLIB Version
print m.meUtilityVersion()
print

#Load Risk Free Curve - USD OIS
print "Load USD Risk-Free Curve"
print m.meLWOLoad("C:\\Temp\\USD_OIS_CURVE.JSON")
print

#Load USD 3ML Curve - USD 3ML
print "Load USD 3ML Curve"
print m.meLWOLoad("C:\\Temp\\USD_SWAP_3M_CURVE.JSON")
print

#Credit Model Data
#Note: It was an IMM Roll Date Today on 19th Sep 2018
creditModelName = "CreditModelUSD"

modelKey = "MODEL_PROPERTIES"

modelLVB = (["AsOfDate", "20180919"],
            ["SpotLag", "5D"],
            ["SpotBusinessDayAdjustment", "FOLLOWING"],
            ["SpotCalendar", "NYB"],
            ["AccrualStartDate", "20180919"],
            ["IMMReferenceDate", "20180919"],
            ["Currency", "USD"],
            ["RecoveryRate", "0.4"],
            ["IncludeAccruedInterest", "TRUE"],
            ["CurveCollection", "USDYC"],
            ["SwapGenerator", "USD_CDS_IMM"],
            ["CreditIndex", "US9128282R06"],
            ["Interpolation", "PiecewiseConstant"],
            ["Extrapolation", "Flat"] )

cdsMarketDataKey = "CDS_MARKETDATA"

CDSMarketDataLVB = (["6M","0.00193900"],
                    ["1Y","0.00183013"],
                    ["2Y","0.00205930"],
                    ["3Y","0.00219438"],
                    ["4Y","0.00232063"],
                    ["5Y","0.00233700"],
                    ["7Y","0.00275507"],
                    ["10Y","0.00316097"])

#Credit Model Calibration
print "Calibrate Credit Model"

#Load Credit Model or Manually Calibrate
creditModel = m.meLWOCreditModelCreate( creditModelName, modelKey, modelLVB, cdsMarketDataKey, CDSMarketDataLVB )
#creditModel = m.meLWOLoad("C:\\Temp\\CreditModel1.JSON")

print creditModel
print

#CDS Definition
swapName = "UST_2.75_FEB_2018"
swapGenerator = "USD_CDS_IMM"
swapLVB = (["PayReceive1","Pay"],
           ["Notional","10000000"],
           ["EffectiveDate","20180926"],
           ["MaturityDate","20230919"],
           ["RateOrSpread1","0.00233700"],
           ["RateOrSpread2","0.0"])
xccyLVB = ([])
isXccySwap = False
validateKeys = True
premiumLeg = "Leg1:Premium"
protectionLeg = "Leg2:Protection"
total = ""

#Create USD CDS 5Y
print "Create 5Y USD CDS"
cdsTrade = m.meLWOSwapCreateFromGenerator( swapName, swapGenerator, swapLVB, xccyLVB, isXccySwap, validateKeys )
print (cdsTrade)
print

#CDS Pricing & Default Probability
print "CDS Pricing & Default Probability"

totalPV = round( m.meLWOCreditDefaultSwapPV( cdsTrade, creditModel, total ), 2 )
premiumPV = round( m.meLWOCreditDefaultSwapPV( cdsTrade, creditModel, premiumLeg ), 2 )
protectionPV = round( m.meLWOCreditDefaultSwapPV( cdsTrade, creditModel, protectionLeg ), 2 )
parSpread = round( m.meLWOCreditDefaultSwapParSpread( cdsTrade, creditModel, premiumLeg, protectionLeg ), 9 )
riskyAnnuity = round( m.meLWOCreditDefaultSwapRiskyAnnuity( cdsTrade, creditModel, total ), 2 )
CS01 = round( m.meLWOCreditDefaultSwapCS01( cdsTrade, creditModel, total ), 2 )

fromDate = "20180919"
toDate = "20230919"
defaultProbability5Y = round( m.meLWOCreditModelDefaultProbability( creditModel, toDate, fromDate ), 9 )
survivalProbability5Y = round( m.meLWOCreditModelSurvivalProbability( creditModel, toDate, fromDate ), 9 )

print "Total PV: ", totalPV
print "Premium Leg: ", premiumPV
print "Protection Leg: ", protectionPV
print "Par Spread: ", parSpread
print "Risky Annuity: ", riskyAnnuity
print "CS01: ", CS01
print "Default Probability: ", defaultProbability5Y
print "Survival Probability: ", survivalProbability5Y
print


#TRS Definition
trsName = "USD_TRS_5Y"
trsGenerator = "USD_TRS"
trsLVB = (["PayReceive1","Pay"],
           ["Notional","10000000"],
           ["EffectiveDate","20180926"],
           ["MaturityDate","20230919"],
           ["RateOrSpread1","0.0312280"],
           ["RateOrSpread2","0.002"]) # 20 bps
xccyLVB = ([])
isXccySwap = False
validateKeys = True
trsPremiumLeg = "Leg1:Premium"
trsFloatLeg = "Leg2:Float"
total = ""
fixingTables = ([])

#Create USD TRS 5Y
print "Create 5Y USD TRS"
trsName = "USD_TRS_5Y"
trsGenerator = "USD_TRS"

cdsTrade = m.meLWOSwapCreateFromGenerator( trsName, trsGenerator, trsLVB, xccyLVB, isXccySwap, validateKeys )
print (cdsTrade)
print

#TRS Pricing
print "TRS Pricing"

totalPV = round( m.meLWOTotalReturnSwapPV( cdsTrade, creditModel, total, fixingTables ), 2 )
fixedPV = round( m.meLWOTotalReturnSwapPV( cdsTrade, creditModel, trsPremiumLeg, fixingTables ), 2 )
floatPV = round( m.meLWOTotalReturnSwapPV( cdsTrade, creditModel, trsFloatLeg, fixingTables ), 2 )
parSpread = round( m.meLWOTotalReturnSwapParSpread( cdsTrade, creditModel, fixingTables ), 9 )
parRate = round( m.meLWOTotalReturnSwapParRate( cdsTrade, creditModel, fixingTables ), 9 )
fixedAnnuity = round( m.meLWOTotalReturnSwapAnnuity( cdsTrade, creditModel, trsPremiumLeg ), 2 )
floatAnnuity = round( m.meLWOTotalReturnSwapAnnuity( cdsTrade, creditModel, trsFloatLeg ), 2 )

print "Total PV: ", totalPV
print "Credit Risky Fixed Leg: ", fixedPV
print "Credit Risky Float Leg: ", floatPV
print "Par Spread: ", parSpread
print "Par Rate (Breakeven Fixed): ", parRate
print "Risky Fixed Annuity: ", fixedAnnuity
print "Risky Float Annuity: ", floatAnnuity
print


# Chart Default Probabilities
def DefaultProbabilities(t):
    result = np.empty(len(t))
    for i in range(len(t)):
        toDate = m.meDateFromYearFraction( fromDate, t[i], "ACT/365" )
        result[i] = m.meLWOCreditModelDefaultProbability( creditModel, toDate, fromDate )
    return result

t1 = np.arange( 0.0, 10.0, 0.1 )
default = DefaultProbabilities(t1)

plt.title('Default Probabilities')
plt.ylabel('Default Probability')
plt.xlabel('Time')
plt.plot( t1, default )
plt.show()

# Chart Survival Probabilities
def SurvivalProbabilities(t):
    result = np.empty(len(t))
    for i in range(len(t)):
        toDate = m.meDateFromYearFraction( fromDate, t[i], "ACT/365" )
        result[i] = m.meLWOCreditModelSurvivalProbability( creditModel, toDate, fromDate )
    return result

t2 = np.arange( 0.0, 10.0, 0.1 )
survival = SurvivalProbabilities(t2)

plt.title('Survival Probabilities')
plt.ylabel('Survival Probability')
plt.xlabel('Time')
plt.plot( t2, survival )
plt.show()

# Chart Risky Discount Factors
def RiskyDiscountFactors(t):
    result = np.empty(len(t))
    for i in range(len(t)):
        paymentDate = m.meDateFromYearFraction( fromDate, t[i], "ACT/365" )
        result[i] = m.meLWOCreditModelRiskyDiscountFactor( creditModel, paymentDate )
    return result

t3 = np.arange( 0.0, 10.0, 0.1 )
riskyDiscountFactor = RiskyDiscountFactors(t3)

plt.title('Risky Discount Factors')
plt.ylabel('Risky Discount Factor')
plt.xlabel('Time')
plt.plot( t3, riskyDiscountFactor )
plt.show()

# Chart Hazard Rates
def HazardRates(t):
    result = np.empty(len(t))
    for i in range(len(t)):
        paymentDate = m.meDateFromYearFraction( fromDate, t[i], "ACT/365" )
        result[i] = m.meLWOCreditModelHazardRate( creditModel, paymentDate )
    return result

t4 = np.arange( 0.0, 10.0, 0.01 )
hazardRates = HazardRates(t4)

plt.title('Hazard Rates')
plt.ylabel('Hazard Rate')
plt.xlabel('Time')
plt.plot( t4, hazardRates )
plt.show()
