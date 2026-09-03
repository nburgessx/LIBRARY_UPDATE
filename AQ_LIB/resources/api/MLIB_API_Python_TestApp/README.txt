Python Test Script
-----------------


Calling MLIB from Python
------------------
To use C++ in Python we require a python DLL *.pyd file and all dependency files,
such as config and calendar files.


MLIB Python DLL
-----------------
The mlib dll is named MLIB_CLIENT_API.pyd


Python Test Script
------------------
In the folder PythonDLL_2017-07-05 we include the MLIB python DLL listed above and a test
file named M_TEST.PY


Running the Test Script - Command Line
------------------
1. Open a command window and navigate to the folder containing the test script.
2. Call the test script using "python import M_TEST"

Running the Test Script - Python IDLE Editor
------------------
1. Open IDLE
2. Import the os package by typing "import os"
3. Set the current working directory to the folder containing the test script
os.chdir("E:\MLIB_TRUNK_COMMON_DEV1\resource\api\MLIB_API_PYTHON_TestApp\PythonDLL_2017-07-05")
4. Import the test by typing "import M_TEST"

