
TestNinja quick tips
====================

It is possible to run the TestNinja in interactive (GUI) mode, and in server (command line) mode.


1. Interactive Mode
-------------------

Simply double-click the TestNinja.exe
A GUI window will open allowing you to set configuration, load and run tests.

2. Server (command-line) Mode
-----------------------------

Open a cmd window, and navigate to the folder where the TestNinja.exe is located.

Run the TestNinja in command line mode as follows:


TestNinja.exe --arch win32 --build release --threads 8 --workspace "%MLIB%" --logFolder "C:\temp"


The only mandatory parameter is "--workspace"; the others are optional.

The default value of "--arch" is: Win32 .    You can also specify: x64
The default value of "--build" is: release . You can also specify: debug
The default value of "--threads" is: 8 .


If the "--logFolder" parameter is missing, then logging is disabled.


So a minimal command would be:

TestNinja.exe --workspace "%MLIB%"

