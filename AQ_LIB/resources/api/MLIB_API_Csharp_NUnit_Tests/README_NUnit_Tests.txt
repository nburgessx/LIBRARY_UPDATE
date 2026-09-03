README NUnit Tests
==================

These unit tests exercise the C# interface to MLIB, and verify that the external API works correctly.


1. Make sure you have extracted the two NUnit dependencies to your %MLIB_EXTERNAL_LIB_PATH%\library path:
 NUnit.Console-3.6.1.zip    ->   %MLIB_EXTERNAL_LIB_PATH%\library\NUnit.Console-3.6.1
 NUnit.Framework-3.7.1.zip  ->   %MLIB_EXTERNAL_LIB_PATH%\library\NUnit.Framework-3.7.1

2. The NUnit.Console-3.6.1 contains the test runner. For convenience in running the tests, add this folder to your PATH variable:
%MLIB_EXTERNAL_LIB_PATH%\library\NUnit.Console-3.6.1
 
3. Open the TestMLIB C# solution and verify that references are resolved. In particular
nunit.framework
nunit.framework.tests

( Inside the TestMLIB.csproj file these are loaded from the $(MLIB_EXTERNAL_LIB_PATH) variable )


4. Build the TestMLIB solution in *** Debug / x86 config. ***  ( ** NOT AnyCPU *** )
We need to do this in order to match the architecture used by MLIB itself.


5. Navigate to TestMLIB\bin\x86\Debug . You should see there is a generated DLL:  TestMLIB.dll
This DLL contains the compiled unit tests.

6. Copy your C#  MLIB_CLIENT_API.dll to this folder :  TestMLIB\bin\x86\Debug
alongside the TestMLIB.dll . Also copy updated calendar files and other config if required.

7. Open a cmd window and navigate to TestMLIB\bin\x86\Debug

8. Run this command to execute the tests:

nunit3-console --noheader --labels:All TestMLIB.dll

The test runner will open TestMLIB.dll and search for tests contained within it. Initially we have
one test:   TestMLIB.DateTests.TestMirGetDateFromTerm

The TestMLIB.dll will load MLIB_CLIENT_API.dll in order to run the tests. The tests should pass.


9. To list all of the tests which are compiled into TestMLIB.dll, run this command:


10. More information and commands are documented at:  http://nunit.org/
We are using the current release NUnit 3. Click on "Documentation" for more information.


