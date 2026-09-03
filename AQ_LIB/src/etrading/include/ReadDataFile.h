#pragma once

#include "LAString.h"
#include "LADate.h"
#include "LACoreTemplateType.h"

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <map>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <memory>

namespace boost
{
    // parse boolean flag
    template<> bool lexical_cast<bool>( const LAString& s );

    // convert boolean flag to string
    template<> LAString lexical_cast<LAString>( const bool& s );

    // shortcut for LAString -> LAString conversions
    template<> inline LAString lexical_cast<LAString>( const LAString& s )
    {
        return s;
    }
};

namespace etrading
{

    //
    // CLASS
    //    ReadTestData
    //
    // PURPOSE
    //    Handle class providing read-only access to test data loaded from external csv file.
    //
    // SYNPOSIS
    //    #include "ReadDataFile.h"
    //    using etrading::ReadTestData;
    //
    //    ReadDataFile::Load inputFile("YieldCurve\\OISCurveUSD.csv");
    //    LAString dayCount = inputFile["dayCount"];
    //    DoubleArray terms = inputFile["terms"];
    //    std::vector<LADate> mydates = inputFile["mydates"];
    //	  LAStringMatrix oisRates = inputFile["OISRates"];
    //
    //    const double d = inputFile.getOptional("optionalDoubleItem", 0);
    //    if (d != 0) std::cout << "d was set to " << d << std::endl;
    //
    //    const LAStringMatrix m = inputFile.getopt("optionalMatrixItem");
    //    if (m.rows() != 0) std::cout << "m was set" << std::endl;
    //
    //    std::cout << "OIS Conventions: " << inputFile["OISConventions"] << std::endl;
    //
    // COMMENTS
    //    For an example input file, see C:\APPL\test\inputs\TestSupport\TestDataTest.csv.
    //
    class ReadDataFile
    {
    public:
        //
        // nested types
        //

        // available data types
        enum Type {Scalar, Vector, AssociativeArray, Table};

        // generic exception class
        class Exception : public LACoreAppError, public virtual std::exception
        {
        public:
            Exception( const LAString& msg, const char* file = nullptr, unsigned int line = 0 );
            /* virtual */ const char* what() const throw();
        };

        // input exception class
        class LoadError : public Exception
        {
        public:
            LoadError( const LAString& fileName );
            /* virtual */ const char* what() const throw();
        private:
            static LAString makeMessage( const LAString& fileName );
        };

        // load / read test data
        class Load
        {
        public:
            // load csv file
            Load() {}
            Load( const LAString& fileName );
            Load& operator=( const Load& rhs );
            Load( const Load& );

            // return true iff there is an item of the given name
            bool hasItem( const LAString& ) const;

            // access data item by name, throw an exception if none found
            const ReadDataFile& operator[]( const LAString& ) const;

			// returns the data keys from the Load
			std::vector<LAString> getKeys() const;

            // convert optional item of given name to an arbitrary type,
            // returning the passed default value if none found;
            // throw an exception if type conversion fails
            template<typename T>
            T getOptional( const LAString& name, const T& defaultValue ) const;

            // specialisation for LAStringMatrix, the most frequent use-case
            LAStringMatrix getOptional( const LAString& name ) const;

        private:

            typedef std::map<LAString, ReadDataFile> Index;

            // open input file and call doReadCSV
            static Index readCSV( const LAString& fileName );

            // parse input
            static Index doReadCSV( std::istream& );

            // attempt to read header of next section, return false if none found
            static bool getSectionHeader( std::istream&, LAStringVector& header );

            // get type of current named section, which is either an associative array
            // or a table
            static Type getType( std::istream&, LAStringVector& cols );

            // return true iff the line (given by vector of fields) is blank
            static bool isBlank( const LAStringVector& line );

            // read line into string vector;
            // if nFields > 0, throw if the number of fields read is different from n;
            static LAStringVector getFields( std::istream&, std::size_t nFields, bool gobbleEmptyLines );

            Index index_;
        };

        //
        // constructors
        //

        // construct scalar
        ReadDataFile( const LAString& value = LAString() );
        // construct 1D vector
        ReadDataFile( const LAStringVector& );
        // construct associative array
        ReadDataFile( const LAStringMatrix& keyValuePairs );
        // construct table
        ReadDataFile( const LAStringVector& cols, const LAStringMatrix& rows );

        // compiler-generated copy constructor, assignment operator are OK (shallow copy);
        // compiler-generated destructor is ok

        //
        // simple accessors
        //

        Type type() const;

        std::size_t size() const;
        std::size_t rows() const;
        std::size_t cols() const;

        //
        // type conversions
        //

        // convert to streamable scalar
        template<typename T> operator T() const;

        // convert to vector of streamable scalars;
        // a null item will be converted to an empty vector
        template<typename T> operator std::vector<T>() const;

		// convert to matrix of streamable scalars;
        // a null item will be converted to an empty vector
        template<typename T> operator std::vector<std::vector<T> >() const;

        // convert to matrix of LAString;
        // a null item will be converted to an empty matrix
        operator const LAStringMatrix& () const;

        //
        // low-level interface
        //

        // scalar interface
        const LAString& operator()() const;

        // vector interface
        const LAString& operator[]( std::size_t i ) const;

        // associative array interface
        const LAString& operator[]( const LAString& key ) const;

        // table interface
        const LAString& operator()( const LAString& key, const LAString& col ) const;

        // mixed interface, matrix interface
		const LAString& operator()( std::size_t i, const LAString& col ) const;
        const LAString& operator()( const LAString& key, std::size_t j ) const;

        // matrix interface
        const LAString& operator()( std::size_t i, std::size_t j ) const;

		// Return keys
		const std::set<LAString>& getKeys() const;

        // pimpl class
        class DataInstance;

        // support for stream output
        std::ostream& print( std::ostream& os ) const;

    private:
        // replace lexical_cast for LAString
        template<typename T>
        static LAString toLAString( const T& t );

        // pimpl pointer
        std::shared_ptr<DataInstance> data_;
    };

    //
    // operator functions - need to be in the same namespace as ReadTestData for ADL to work
    //

    // comparison operator, convenient for testing
    bool operator==( const ReadDataFile& lhs, const ReadDataFile& rhs );

    // comparison with native scalar, convenient for testing;
    template<typename T>
    bool operator==( const ReadDataFile& m, const T& t );

    // comparison with native scalar, convenient for testing;
    template<typename T>
    bool operator==( const T& t, const ReadDataFile& m );

    // stream output - required for google test error reporting;
    std::ostream& operator<<( std::ostream& os, const ReadDataFile& x );
}

//
// inline & template implementations
//

inline LAStringMatrix
etrading::ReadDataFile::Load::getOptional( const LAString& name ) const
{
    return getOptional( name, LAStringMatrix() );
}

template<typename T>
inline T etrading::ReadDataFile::Load::getOptional( const LAString& name, const T& defaultItem ) const
{
    const Index::const_iterator it = index_.find( name );
    if ( it != index_.end() )
    {
        // try to use type conversion
        return it->second;
    }
    return defaultItem;
}

template<typename T>
inline etrading::ReadDataFile::operator T() const
{
    if ( type() != Scalar )
    {
        throw Exception( "#Error Reading Data File: Cannot convert non-scalar to requested type" );
    }
    return boost::lexical_cast<T>( ( *this )() );
}

template<typename T>
etrading::ReadDataFile::operator std::vector<T>() const
{
    if ( type() != Vector )
    {
        throw Exception( "#Error Reading Data File: Cannot convert non-vector type to std::vector" );
    }
    std::vector<T> res( size() );
    for ( std::size_t i = 0; i < size(); ++i )
    {
		// Use [] Operator to read and parse Vector data
        const LAString source = ( *this )[i];
        res[i] = boost::lexical_cast<T>( source );
    }
    return res;
}

template<typename T>
etrading::ReadDataFile::operator std::vector<std::vector<T> >() const
{
    if ( type() != AssociativeArray && type() != Table )
    {
        throw Exception( "#Error Reading Data File: Cannot convert non-matrix type to matrix type std::vector< std::vector<> > " );
    }
    std::vector<std::vector<T> > res( size() );
    for ( std::size_t i = 0; i < size(); ++i )
	{
		std::vector<T> thisRow( cols() );
		for ( std::size_t j = 0; j < cols(); ++j )
		{
			// Use () Operator to read and parse Vector data
			const LAString source = ( *this )( i, j );
			thisRow[j] = boost::lexical_cast<T>( source );
		}
		
		res[i] = thisRow;
    }
    return res;
}

template<class T>
inline LAString etrading::ReadDataFile::toLAString( const T& t )
{
    const std::string s = boost::lexical_cast<std::string>( t );
    return LAString( s.c_str() );
}

template<typename T>
inline bool etrading::operator==( const ReadDataFile& m, const T& t )
{
    return static_cast<T>( m ) == t;
}

template<typename T>
inline bool etrading::operator==( const T& t, const ReadDataFile& m )
{
    return static_cast<T>( m ) == t;
}

inline std::ostream& etrading::operator<<( std::ostream& os, const ReadDataFile& x )
{
    return x.print( os );
}
