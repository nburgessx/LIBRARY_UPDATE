#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Load Calendars and Config Files
m.setUpMLIB("","")

#Get the MLIB Version
m.meUtilityVersion()

fromDate = "20190601"
toDate   = "20190901"
dayCount = "ACT/ACT"
includeLast = True

yearfract = m.meDateYearFraction( fromDate, toDate, dayCount, includeLast)
print yearfract

yearfract = m.meDateYearFraction( fromDate, toDate, dayCount )
print yearfract