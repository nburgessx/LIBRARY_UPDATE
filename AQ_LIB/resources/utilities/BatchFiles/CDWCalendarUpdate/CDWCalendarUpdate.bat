@echo off
rem = """
:: From here and on, write any Batch file syntax and it will be ignored by Python
:: 
:: MLIB Calendar updater
:: 21 April 2020 Ian Castleton
:: Quant Research and Analytics
::
:: Please make sure you have python3 in your path
:: For example  C:\Python36_x64\python36.exe

python36 -x "%~f0" %*

PAUSE
exit

:: End of batch file commands
"""

import os
import csv
import urllib.request
from datetime import date

mlibqFolder  = os.path.expandvars("$MLIBQ")
print("\nMLIBQ environment variable: '" + mlibqFolder + "'" )
configFolder = mlibqFolder + "/resource/config/"

calendarFile = configFolder + "Calendar.csv"
print("\nLoading the original MLIB csv calendar file: " + calendarFile )

originalCalendar=[]
with open(calendarFile, newline='') as csvFile:
    csvReader = csv.reader( csvFile )
    for row in csvReader:
        originalCalendar.append(row)

origRows = len(originalCalendar )
origCols = len(originalCalendar[0])
print("Original file rows: " + str(origRows) )
print("Original file cols: " + str(origCols) )

print("\nMaking 'nonWorkingDays' request to CDW...")

cdwDateURL = "http://cdwpp/cdw/nonWorkingDays/1999-01-01/2069-01-01.csv?limit=99999&sort=code&select=code%20day.date"
response = urllib.request.urlopen( cdwDateURL )
rawText = response.read().decode()

# Split the raw text string into separate lines
csvLines = rawText.split( '\n' )
numDataRows = len(csvLines)
print("Obtained " + str(numDataRows) + " data rows from CDW")
print("Note: The final row is non-calendar data");

rawCDWFile = configFolder + "RawCDWCalendar.csv"
print("Writing out raw CDW data to csv file: " + rawCDWFile)

with open( rawCDWFile, 'w') as rawCDW:
    for csvLine in csvLines:
        rawCDW.write("%s\n" % csvLine)
		
print("\nProcessing CDW data into a matrix...")

# Note that CDW can gives us a different number of dates per calendar, so this will be a ragged matrix
raggedMatrix=[]
maxColumns=0
# Iterate over each line, excluding the final line which contains junk
for line in csvLines[:-1]:
    datesLine=line.split(',')
    nDateColumns = len(datesLine) - 1 # Calendar code is duplicated on each line
    if nDateColumns > maxColumns:
        maxColumns = nDateColumns
	
    # Slice off the initial item. This is a duplicate of the calendar name
    # Then append to raggedMatrix
    raggedMatrix.append( datesLine[1:] )
  
numRows = len( raggedMatrix )
print( str(numRows) + " calendars, with up to " + str(maxColumns) + " dates in each calendar")

# Now pad the matrix so that we get a rectangular matrix
datesMatrix = []
for row in raggedMatrix:
    paddedRow = row
    nCols = len(row)
    if nCols < maxColumns:
        extraItems = maxColumns - nCols
        padding = [""] * extraItems
        paddedRow += padding
    datesMatrix.append(paddedRow)
		

# The matrix contains a whole bunch of dateTime strings
# Each row is the following format:
# Input:   LnB, YYYY-MM-DDT00:00:00.000Z, YYYY-MM-DDT00:00:00.000Z, ...
# Desired: LnB, YYYYMMDD, YYYYMMDD, ...

print("Formatting the dates to be MLIB friendly...")
# Initialize our matrix of dates
datesOnly=[]
for r in range(numRows):
    datesLine=[]
    for c in range(maxColumns):
        item=datesMatrix[r][c]
        if c==0:
            # Calendar code
            datesLine.append(item)
        else:
            # Remove the time field to obtain just a date string
            dateTimeFields=item.split('T')
            myDate=dateTimeFields[0]
            # Now convert YYYY-MM-DD into YYYYMMDD
            myDate=myDate.replace('-','')
            datesLine.append(myDate)
    datesOnly.append( datesLine )


print("Transform so the holidays for each calendar region form a column...")
transposedDates = [ list(i) for i in zip(*datesOnly) ]

print("Done processing. Now writing outputs...")

print("\nWriting out new csv calendar file: " + calendarFile )

with open(calendarFile, "w", newline='') as csvFile:
    csvWriter = csv.writer(csvFile,delimiter=',')
    csvWriter.writerows( transposedDates )

calendarConfFile = configFolder + "Calendar.conf"
print("Writing out config file: " + calendarConfFile )

todayDate = date.today()
todayDateString = todayDate.strftime( "%Y%m%d" )

with open(calendarConfFile, "w" ) as confFile:
    confFile.write( "%s\n%s\n"
	% ( "# Configuration file which specifies when Calendar.csv was generated",
		"LastCalendarUpdate," + todayDateString ) )

# MLIB requires the calendars in column format.
# We will also output the transposed (row format) calendars:
# This allows easier date comparison since the transposed format is:
# LNB YYYYMMDD,YYYYMMDD,YYYYMMDD,...
# which allow us to quickly see at what point the future the LNB calendar
# changes in the update.
# Note for a sanity check we expect the differences to occur well into the future,
# and certainly not for past dates.

calendarFileOldTransposed = configFolder + "Calendar_OrigTransposed.csv"
print("\nWriting out original calendar file (transposed): " + calendarFileOldTransposed )
transposedOriginalCalendar = [ list(i) for i in zip(*originalCalendar) ]

with open(calendarFileOldTransposed, "w", newline='') as csvFile:
    csvWriter = csv.writer(csvFile,delimiter=',')
    csvWriter.writerows( transposedOriginalCalendar )

calendarFileNewTransposed = configFolder + "Calendar_NewTransposed.csv"
print("Writing out new calendar file (transposed): " + calendarFileNewTransposed )

with open(calendarFileNewTransposed, "w", newline='') as csvFile:
    csvWriter = csv.writer(csvFile,delimiter=',')
    csvWriter.writerows( datesOnly )
	
print()
print("All done! Please diff the Transposed files to check the difference in major currencies.")
print("Also check the generated calendar file. Goodbye!")
