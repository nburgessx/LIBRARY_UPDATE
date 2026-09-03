/*! @file
    @brief Class declaration of type of template objects

*/
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <complex>
#include <vector>
#include <set>
#include <map>
#include <deque>
#include <fstream>
#include "AQLString.h"
#include "AQLDateTime.h"
#include "AQLDate.h"
#include "AQLCoreAutoPtr.h"
#include <boost/variant.hpp>

class AQLObjectHolder;

typedef std::string                                             StandardString;         // std::string    
typedef std::vector<StandardString>                             StandardStringVector;   // std::string vector
typedef std::vector<StandardStringVector>                       StandardStringMatrix;   // std::string 2D Matrix

typedef std::vector<AQLString>	                                AQLStringVector;         // AQLString vector
typedef std::vector<AQLStringVector >                            AQLStringMatrix;         // AQLString 2dim valarray
typedef std::vector<bool>		                                BoolVector;             // bool 2dim valarray
typedef std::vector<BoolVector >                                BoolMatrix;             // bool vector
typedef std::vector<AQLDateTime>                                 DateTimeVector;         // AQLDateTime vector
typedef std::vector<AQLDate>		                                DateVector;             // AQLDate vector
typedef std::vector<DateVector>                                 DateMatrix;             // date 2dim valarray
typedef std::vector<int>		                                IntVector;              // int vector
typedef std::vector<double>		                                DoubleVector;           // double vector
typedef std::vector<DoubleVector>                               DoubleMatrix;           // double 2dim valarray
typedef std::fstream			                                FStream;                // File Stream
typedef std::set<AQLString>		                                StringSet;              // AQLString set
typedef std::vector<int>		                                IntArray;               // int valarray
typedef std::vector<unsigned int>	                            UintArray;              // unsigned int valarray
typedef std::vector<unsigned long>	                            UlongArray;             // unsigned long valarray
typedef std::vector<double>	                                    DoubleArray;            // double valarray
typedef std::complex<double>                                    DoubleComplex;          // double complex
typedef std::vector<std::complex<double> >                      ComplexVector;          // complex vector
typedef std::vector< std::vector< std::complex<double> > >	    ComplexMatrix;          // complex 2dim valarray
typedef std::vector< std::vector<int> >                         IntMatrix;              // double 2dim valarray
typedef AQLCoreAutoPtr<std::vector<AQLObjectHolder> >             Records_var;            // record data for storing

typedef boost::variant<int, double, bool, std::string, AQLString, const char*> AnyType;
typedef std::vector<AnyType> AnyTypeVector;
typedef std::vector< std::vector<AnyType> > AnyTypeMatrix;
