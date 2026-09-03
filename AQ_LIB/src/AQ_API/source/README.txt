This project will create a DLL with a JNI interface through a SWIG file (swig_mlib.i).

The functions exposed will be accessible by dynamically loading the DLL in a Java client 
(note that this is platform dependent - i.e. Windows only, for other systems shared object 
file builds (.so) will be necessary).

Note some extra settings required for the JNI DLL:
Add the header directories to the include path: assumed to be 
  %MLIB%\library\boost_1_55_0
  %MLIB%\mlib\src\ir\include
  %MLIB%\mlib\src\common\include
  %MLIB%\mlib\src\irserver\include
  %MLIB%\mlib\src\vanilla\include
Also add the appropriate Java build (for the JNI):
  C:\Program Files %28x86%29\Java\jdk1.8.0_74\include\win32
  C:\Program Files %28x86%29\Java\jdk1.8.0_74\include	
Alter this to match your installed version of Java (or specify a correct JAVA_HOME env variable)
Note that the correct version of Java is required to match the build (32 or 64 bit). We chose 32bit for safety (because the
SWIG executable is 32bit only).

Code Generation: Multi-threaded Debug (/MDd), this means it is looking for anything linked as a .lib so:
Additional Library Directories: 
  %MLIB%\mlib\target\32\Debug\ir\out 
  %MLIB%\mlib\target\32\Debug\common\out
  %MLIB%\mlib\target\32\Debug\irserver\out
  %MLIB%\mlib\target\32\Debug\vanilla\out
  %MLIB%\mlib\target\32\Debug\libext\out
  %MLIB%\library\boost_1_55_0\stage\x86\lib
  %MLIB%\library\QuantLib-1.3\lib
For Release build:
  %MLIB%\mlib\target\32\Release\ir\out 
  %MLIB%\mlib\target\32\Release\common\out
  %MLIB%\mlib\target\32\Release\irserver\out
  %MLIB%\mlib\target\32\Release\vanilla\out
  %MLIB%\mlib\target\32\Release\libext\out
  %MLIB%\library\boost_1_55_0\stage\x86\lib
  %MLIB%\library\QuantLib-1.3\lib
  
Additional Linker Dependencies (probably do not need all the boost libraries but the the necessary set is included):
  common.lib
  ir.lib
  irserver.lib
  vanilla.lib 
  libext.lib 
  %(AdditionalDependencies)
To avoid getting errors on the use of deprecated functions in in MLIB 
	add the pre-processor: _CRT_SECURE_NO_WARNINGS (relevant for later versions of VS)
No incremental linking: No (/INCREMENTAL:NO)
The creation of this DLL was tested in VS 2010 and 2015 under Debug and x86 mode. 

The generated java files in the directory will need to be incorporated in any Java client

Note that the java files: 
	XXXXX_wrap.cxx
	XXXXX.java
	XXXXXJNI.java
were generated from XXXXX.i

to generate them execute the command prompt: 
	swig -c++ -java XXXXX.i
Make sure SWIG and the SWIGWIN executable were installed on your system and that the 
swig.exe is in your path variable (unless you fully specificy the file path). Have a look
at the executed sw.bat file to look at command line options.


Also make sure to read the notes in exposed_functions.cpp before adding anything to the 
interface.

If you build the project without the generated XXXXX_wrap.cxx file it will produce 
a DLL which will only be accessible from other C++ clients not from your target wrapping language (java in this case).












