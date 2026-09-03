dyn.load("MLIB_CLIENT_API")
source("MLIB_CLIENT_API.R")
getLoadedDLLs()
setUpMLIB("","")
meUtilityVersion()
setUpMLIB("","")

meUtilityVersion()

# Set up a Fixing Table
handle <- "FixingTableEUR3ML"
fixingDates <- c("2019-12-04")
fixingValues <-  c(0.005)
fixingTable <- meLWOFixingTableCreate(handle, "EUR", "QUARTERLY", fixingDates, fixingValues)
meLWOFixingTableValue(fixingTable, fixingDates)


# Load our curves

oisFile <-c("E:/MLIB/MLIBQ_DEV1/resource/api/PythonExamples/DiscountFactorsWithSpread/EUR_OIS_CURVE@24.JSON")
swapFile <-c("E:/MLIB/MLIBQ_DEV1/resource/api/PythonExamples/DiscountFactorsWithSpread/EUR_SWAP_3M_CURVE@22.JSON")

meLWOLoad(oisFile)
meLWOLoad(swapFile)

# Set up parameters to the discount function

paymentDates <- c("2020-03-06", "2020-06-06")
curveCollection <- "EURYC"
curveIndex <- "EUR3ML"
spread <- 0.05


# Invoke the discount function

meLWOCurveDiscountFactorsWithSpread( paymentDates, curveCollection, curveIndex, spread, fixingTable )

# [1] 0.9862879 0.9748303

