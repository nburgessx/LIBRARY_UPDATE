@echo off
%MLIB_EXTERNAL_LIB_PATH%\library\swigwin-3.0.8\swig.exe -v -c++ -package com.mizuho.mlib -java swig_mlib.i
REM After running this, add the pre-compiled header include statement to the generate XXXXX_wrap.cxx and rebuild the project if using pre-compiled headers
@echo on
