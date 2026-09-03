#pragma once

/*
 * The "R" language is unusual in that it appears to implement matrices as vectors.
 * The vector is arranged as follows:
 * The first vector elements contain all the column 1 items, followed by column 2, and so on.
 *
 * We therefore define a type SWIG_STRINGMATRIX which we can use in the API functions.
 * For "R", the SWIG_STRINGMATRIX will actually be a vector< string >.
 * For all other languages, the macro expands to the more usual vector< vector< string > >
 *
 * NOTE: This file is parsed by two separate entities:
 * i)  The Visual Studio C++ compiler ( hence the macro SWIG_R )
 * ii) The SWIG preprocessor, which sets macro SWIGR.
 *
 */

#if defined(SWIG_R) || defined(SWIGR)
  #define SWIG_STRINGMATRIX std::vector< std::string >
#else
  #define SWIG_STRINGMATRIX std::vector< std::vector< std::string > >
#endif

