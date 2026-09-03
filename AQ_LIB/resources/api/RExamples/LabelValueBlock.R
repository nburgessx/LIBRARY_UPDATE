dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()

meDateFromTenor("ABC","ABC","","","")
# meLWO

bondName = "US_Treasury"
bondLVB <- c("ISIN","IssueDate","AccrualStartDate","RollDay","FirstCouponDate","LastCouponDate","Coupon","MaturityDate","US912828N894","20150403","20150430","EOM","20151031","20211031","0.0175","20220430")

dim(bondLVB) <- c(8,2)
bondLVB

meLWOBondCreateFromGenerator(bondName, "US_TREASURY_TYPE1", bondLVB, FALSE )

settlementDates <- c( "20190212" )
yields <- c( 0.02 )
yieldCalcType <- c( "" )

meLWOBondPrice( bondName, settlementDates, yields, yieldCalcType )


# A test of meUtilityLVBCreate()

keys   <- c( "A", "B", "C")
values <- c( "1", "2", "3" )
meUtilityLVBCreate(keys, values)

# Expect
# [1] "A" "B" "C" "1" "2" "3" 


