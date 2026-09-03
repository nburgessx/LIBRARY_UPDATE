#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
print ( m.setUpMLIB("","") )

#Get the MLIB Version
print ( m.meUtilityVersion() )

#Bond Definition
bondName = "USD_Treasury_1.75%_30-Apr-2022"
bondLVB = (["BondDescription","US912828N894"],
           ["ISIN","US912828N894"],
           ["IssueDate","20150403"],
           ["AccrualStartDate","20150430"],
           ["RollDay","EOM"],
           ["FirstCouponDate","20151031"],
           ["LastCouponDate","20211031"],
           ["Coupon","0.0175"],
           ["MaturityDate","20220430"])

#Create Bond
print( "Create Bond" )
print( m.meLWOBondCreateFromGenerator( bondName, "US_TREASURY_TYPE1", bondLVB, False ) )

#Bond Prices
settlementDate = "20180315"
yield_ = 0.025544

settlementDates = (["20180315"])
prices = ([96.8672])
yields = ([0.025544])

print( "Bond Price Quoted" )
print( m.meLWOBondPrice( bondName, settlementDates, yields, "" ) )

print( "Bond Yield" )
print( m.meLWOBondYield( bondName, settlementDates, prices, "", True ) )

print( "Clean Price" )
print( m.meLWOBondCleanPrice( bondName, settlementDates, yields, "" ) )

print( "Dirty Price" )
print( m.meLWOBondDirtyPrice( bondName, settlementDates, yields, "" ) )

print ("Schedule")
print( m.meLWOBondDisplaySchedule( bondName, True ) )

print ("Cashflows")
print( m.meLWOBondDisplayCashflows( bondName, settlementDate, yield_, "SIMPLE", True ) )
