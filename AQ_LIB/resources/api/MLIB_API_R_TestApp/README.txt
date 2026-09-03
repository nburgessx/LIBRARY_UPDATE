RTest Script
-----------------


Calling MLIB from R
------------------
To use C++ in Python we require the C++ MLIB_CLIENT_API.DLL file, the swig generated MLIB_CLIENT_API.R file
and the ir.props & calendar.csv files.


R Test Script
------------------
In the folder RDLL_2017-07-10 we include the MLIB DLL and a test file named M_TEST.R


Running the Test Script - Command Line
------------------
1. Open a command window and navigate to the folder containing the test script.
2. Call the test script "M_TEST.R"


Running the Test Script - R Editor
------------------
1. Open the R GUI
2. Open the M_TEST.R script using the menu dropdown file->open script
3. Select all code
4. Press the Run Select icon
5. Press the Return to Console icon to see the results


Using MLIB in R without a test script
------------------
1. On command line change the current working directory to that of the MLIB package containing
the MLIB DLL, MLIB.R file and the calendar and ir.props files
2. If using the R Gui chnage the directory as in (1) using File->Change Dir
3. Load the MLIB CLIENT DLL type: dyn.load("MLIB_CLIENT_DLL")
4. Load the MLIB R SOURCE type: source("MLIB_CLIENT_API.R")
5. MLIB can now be called as if it were native to R
6. Please remember to initialise MLIB calendars and static data using : setupMLIB()
7. Please remember to teardown MLIB once finished using: teardownMLIB()

8. Examples Type:

setupMLIB()
meUtilityVersion()
meDateFromTenor("20170101","6M","","","")
teardownMLIB()