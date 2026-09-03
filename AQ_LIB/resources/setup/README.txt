How To Setup M-Lib 
---------------------------

ENVIRONMENT VARIABLE "MLIB"
-----------------------------------------------
Please set an environment variable "MLIB" with the path pointing to the folder where you keep your MLIB source code.

Example: 

	C:\APPL


ENVIRONMENT VARIABLE "MLIB_EXTERNAL_LIB_PATH"
--------------------------------------------------------------------------------
Please set an environment variable "MLIB_EXTERNAL_LIB_PATH" with the path pointing to the folder where you would like to keep
the "library" folder. This folder contains all the External Libraies used by MLIB
Note that there should be no final backslash in the path.

Example:

	C:\MLIB_EXTERNAL_LIB\library


ENVIORNMENT VARIABLE "MLIB_LOCATION"
----------------------------------------------------
Please set environment variable "MLIB_LOCATION" by running Set_LOCATION.bat to specify your current geographic location.


For Swig Developers
----------------------------
Please ensure the external library contains the swigwin-3.0.8 folder which contains the executable swig.exe needed to auto-generate
the API wrapper files. SwigWin requires the Java JDK JDK-8u101-windows-i586 package to be installed, see the external library folder
for the JDK exe file.

Also add the appropriate Java build (for the JNI) if not installed to the Program Files x86 (32 Bit)

  C:\Program Files %28x86%29\Java\jdk1.8.0_101\include\win32
  C:\Program Files %28x86%29\Java\jdk1.8.0_101\include

and to Program Files (64 Bit) as below	

  C:\Program Files\Java\jdk1.8.0_101\include\win32
  C:\Program Files\Java\jdk1.8.0_101\include


Please install Python and R in the following locations ... the installer files can be found in the third party external libraray
folder referenced by the environment variable "MLIB_EXTERNAL_LIB_PATH"

Python-2.7.13 (32 Bit) 
Installation Folder: C:\Python27

R-3.4.0			
Installation Folder: C:\Program Files\R



Directory Structure
-------------------------
C:\APPL
       \ excel_tests
       \ library
       \ mlib
       \ pricer
       \ vs_properties

Dependencies
-------------------
External Libraries
•Boost : 1.55.0
•gmock : 1.7.0
•gtest : 1.7.0
•gtest-gbar-master
•protobuf : 2.6.1
•QuantLib : 1.3
•SwigWin : 3.0.8
•zeromq : 4.0.4
•zeromq-c++

Dependencies for Swig / MLIB API Developers
•Python-2.7.13
•R-3.4.0

Copy the zip files from X:\Secure\MLib_Repo\library to C:\APPL\library
and unzip the copies. (Do NOT copy the unzipped folders - they are too big.)

Plugin(s)
•XllPlus (license required)

Setup
--------
Ensure that hidden files are visible:

	File Explorer / Tools / Folder Options / View
	Tick "show hidden files, folders, or drives

Copy files from vs_properties directory
•File(s) : Microsoft.Cpp.x64.user.props and Microsoft.Cpp.Win32.user.props
•Source : C:\APPL\vs_properties
•Destination : C:\Users\[username]\AppData\Local\Microsoft\MSBuild\v4.0 (hidden directory. Backup any existing files)

External Dependencies
•Copy external dependencies to library directory (as defined in Microsoft.Cpp.xxx.user.props)

Swigwin JDK
•Please ensure that the Swigwin Java JDK version "JDK-8u101-windows-i586" is installed, see the external library folder
for the JDK exe file.

