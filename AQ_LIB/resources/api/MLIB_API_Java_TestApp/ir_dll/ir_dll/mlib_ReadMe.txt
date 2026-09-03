Only run and tested under Debug Win32 bit!

This project creates a simple DLL that exposes a single function from the common.lib static
library in MLIB (in this case AnalyticFormulae::diffNormdist). 

The aim is to demonstrate exposure of MLIB functions through a DLL which can in turn be accessed 
through a C++ client dynamically loading the DLL. The two libraries used from MLIB are common and ir 
(each assumed to be built as a 32 bit static library with Multi-threaded Debug DLL (/MDd) as the 
Code Generation). To expose additional functions, include the additional static libraries (with the right Code Generation)

Note some extra settings required for the DLL:
Add the header directories to the include path: assumed to be 
  %MLIB%\mlib\src\ir\include 
  %MLIB%\mlib\src\common\include 
Code Generation: Multi-threaded Debug DLL (/MDd)
Additional Library Directories: 
  %MLIB%\mlib\target\32\Debug\ir\out 
  %MLIB%\mlib\target\32\Debug\common\out
Additional Linker Dependencies:
  common.lib
  ir.lib
To avoid getting errors on the use of deprecated functionsin in MLIB 
	add the pre-processor: _CRT_SECURE_NO_WARNINGS (relevant for later versions of VS)
No incremental linking: No (/INCREMENTAL:NO)
The creation of this DLL was tested in VS 2010 and 2015 under Debug and x86 mode. 
Alter the directories for different builds.
