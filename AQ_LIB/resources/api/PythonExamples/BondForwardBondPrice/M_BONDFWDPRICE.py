#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
print ( m.setUpMLIB("","") )
print

#Get the MLIB Version
print ( m.meUtilityVersion() )
print

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
print "Create Bond" 
print m.meLWOBondCreateFromGenerator( bondName, "US_TREASURY_TYPE1", bondLVB, False )
print

#Bond Prices *** Vector Format ***
settlementDates = (["20180315"])
prices = ([96.8672])
yields = ([0.025544])

print "Quoted Bond Price"
bondPrice = m.meLWOBondPrice( bondName, settlementDates, yields, "" )
print bondPrice
print

print "Bond Yield"
bondYield = m.meLWOBondYield( bondName, settlementDates, prices, "", True )
print bondYield
print

print "Clean Price"
cleanPrice = m.meLWOBondCleanPrice( bondName, settlementDates, yields, "" )
print cleanPrice 
print

print "Dirty Price"
dirtyPrice = m.meLWOBondDirtyPrice( bondName, settlementDates, yields, "" )
print dirtyPrice
print

#Bond Forward Price Parameters *** Non-Vector Format ***
print "Forward Price"
settlementDate=("20180315")
forwardSettlementDate = ("20180316")
price = 96.8672
repoRate = 0.019494
repoDaycount  = "ACT/365"
forwardBondPrice = m.meLWOBondForwardPrice( bondName, settlementDate, forwardSettlementDate, price, repoRate, repoDaycount )
print forwardBondPrice
print
