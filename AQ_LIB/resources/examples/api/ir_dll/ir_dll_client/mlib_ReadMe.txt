Only tested on Debug Win32 bit!

This project creates an executable from C++ code that acts as a client to a DLL that exposes 
a function from MLIB's static libraries (in this case AnalyticFormulae::diffNormdist). 

Note the settings required for the DLL:
alter the PATH variable to include the directory of ir_dll.dll
Include the header files for the DLL (this is not scrictly necessary but the client becomes cumbersome 
without it -> casting handles requires knowledge of the interface anyway):
	$(SolutionDir)\ir_dll (or the appropriate location where ir_dll's header files live)
Code Generation: Multi-threaded Debug (/MTd)
Copy the DLL to the directory of the executable (in this case they should already be there)
	(in case you did not alter the PATH before starting VS2010, or include it
	in a post-build event e.g. copy /y "copy /y "$(SolutionDir)\Debug\ir.dll" "$(OutDir)" )
There is also a boost dependency here (which only necessary for the 1 threading function...) 
So additional library directory (change depending on your build)
	%MLIB%\library\boost_1_55_0\stage\x86\lib
and additional library includes:
	%MLIB%\library\boost_1_55_0

Execute the client executable (ir_dll_client.exe)

To run the client run it manually in the directory of the executable (fancy PATH manipulation required in VS 2010 otherwise)
