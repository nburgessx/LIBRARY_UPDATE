@echo off
echo GOOGLE TEST RUNNER
echo ------------------

rem Set the Test Environment
rem -------------------
echo.

set /p Use64Bit="Run Tests in x64 Bit MLib? Y/N: "
if /I "%Use64Bit%"=="N" (
set Platform="32"
) else (
set Platform="64"
)

set /p mode="Run Tests in Release Mode? Y/N: "

if /I "%mode%"=="N" (
set GoogleTest="%MLIB%\mlib\target\%Platform%\Debug\google_test\out\google_test.exe"
set Environment="DEBUG"
) else (
set GoogleTest="%MLIB%\mlib\target\%Platform%\Release\google_test\out\google_test.exe"
set Environment="RELEASE"
)

echo Using %GoogleTest%
echo.


rem Shuffle Tests
rem ------------
set /p shuffleTests="Shuffle Tests? Y/N: "

if /I "%shuffleTests%"=="Y" (
set shuffleFlag="--gtest_shuffle"
) else (
set shuffleFlag=""
)

rem Repeat Tests
rem ------------
set /p repeatTests="Repeat Tests? Y/N: "

if /I "%repeatTests%"=="Y" (
set repeatFlag="--gtest_repeat=3"
) else (
set repeatFlag=""
)

rem Output Results to XML
rem -------------------
set /p printResults="Write Output Results to XML? Y/N: "

if /I "%printResults%"=="Y" (
set printResultsFlag="--gtest_output=xml:C:\Temp\Results.xml"
) else (
set printResultsFlag=""
)

rem Display Settings
rem ----------------
echo.
echo %Environment% Mode Selected
if /I "%printResults%"=="Y" echo Test Results will be output to C:\Temp\Results.xml

rem Uncomment the Below to Display Google Test Help
rem -----------------------------------------------
rem %GoogleTest% --help

rem Inform User of Test Options
rem ---------------------------
echo.
echo Options:
echo --------
echo 0 = Exit
echo 1 = List Tests
echo 2 = Run All Tests
echo 3 = Run All Excluding Curve Tests
echo 4 = Run All Curve Tests
echo 5 = Run OIS Curve Tests
echo 6 = Run STD Curve Tests
echo 7 = Run Date Tests
echo 8 = Run Object Tests
echo 9 = Run Trade Tests
echo 10 = Run Utility Tests

rem Prompt User to Select Test Option
rem ---------------------------------
echo.
set /p option="Select Option: "

rem Run The Test
rem ------------  
if "%option%"=="0" exit 0
if "%option%"=="1" %GoogleTest% --gtest_list_tests
if "%option%"=="2" %GoogleTest% %printResultsFlag% %shuffleFlag% %repeatFlag%
if "%option%"=="3" %GoogleTest% --gtest_filter=*.*-TestCurve*.* %printResultsFlag% %shuffleFlag% %repeatFlag%
if "%option%"=="4" %GoogleTest% --gtest_filter=TestCurve*.* %printResultsFlag% %shuffleFlag% %repeatFlag%
if "%option%"=="5" %GoogleTest% --gtest_filter=TestCurveOis*.* %printResultsFlag% %shuffleFlag% %repeatFlag%
if "%option%"=="6" %GoogleTest% --gtest_filter=TestCurveStd*.* %printResultsFlag% %shuffleFlag% %repeatFlag%
if "%option%"=="7" %GoogleTest% --gtest_filter=TestDates*.* %printResultsFlag% %shuffleFlag% %repeatFlag%
if "%option%"=="8" %GoogleTest% --gtest_filter=TestObject*.* %printResultsFlag% %shuffleFlag% %repeatFlag%
if "%option%"=="9" %GoogleTest% --gtest_filter=TestTrade*.* %printResultsFlag% %shuffleFlag% %repeatFlag%
if "%option%"=="10" %GoogleTest% --gtest_filter=TestUtilities*.* %printResultsFlag% %shuffleFlag% %repeatFlag%

