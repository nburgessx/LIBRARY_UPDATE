#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "FolderConfig.h"

#include <cstdlib>				// std::getenv
#include <cstring>				// std::strlen

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

#ifdef _WIN32
#pragma warning(disable:4996)	// we're using getenv in a safe way
#endif

using etrading::ReadDataFile;

namespace etrading
{

    //
    // Pimpl class declaration
    //

    class etrading::ReadDataFile::DataInstance
    {
    public:
        //
        // nested types
        //
        typedef ReadDataFile::Type Type;
        typedef ReadDataFile::Exception Exception;

        //
        // generic interface
        //
        virtual Type type() const = 0;
        virtual ~DataInstance() {}

        //
        // simple accessors
        //
        virtual std::size_t size() const;
        virtual std::size_t rows() const;
        virtual std::size_t cols() const;

        // scalar interface
        virtual const LAString& operator()() const;

        // vector interface
        virtual const LAString& operator[]( std::size_t i ) const;

        // associative array interface
        virtual const LAString& operator[]( const LAString& key ) const;

        // table interface
        virtual const LAString& operator()( const LAString& key, const LAString& col ) const;

        //
        // mixed interface
        //
        virtual const LAString& operator()( std::size_t i, const LAString& col ) const;

        virtual const LAString& operator()( const LAString& key, std::size_t j ) const;

        // matrix interface
        virtual const LAString& operator()( std::size_t i, std::size_t j ) const;

		// Get keys
		virtual const std::set<LAString>& getKeys() const;

        // conversion to LAStringMatrix
        virtual operator const LAStringMatrix& () const;

        // stream output
        virtual std::ostream& print( std::ostream& ) const = 0;

    protected:
        DataInstance() {}
    private:
        DataInstance( const DataInstance& );
        DataInstance& operator=( const DataInstance& );
    };

    //
    // Pimpl subclasses
    //

    namespace
    {
        typedef etrading::ReadDataFile ReadTestData;

        // return true iff name ends in "[]", in which case the suffix is chopped off
        bool isVector( LAString& name )
        {
            const char* s = name.getCString();
            const std::size_t n = std::strlen( s );
            assert( n >= 2 );
            if ( s[n - 2] == '[' && s[n - 1] == ']' )
            {
                if ( n == 2 )
                {
                    throw ReadDataFile::Exception( "parse error: blank vector name" );
                }
                name = name.subString( 0, n - 3 );
                assert( name.size() == n - 2 );
                return true;
            }
            return false;
        }

        class XScalar : public ReadDataFile::DataInstance
        {
        public:
            XScalar( const LAString& value ) : value_( value ) {}
            /* virtual */ Type type() const
            {
                return etrading::ReadDataFile::Scalar;
            }
            /* virtual */ const LAString& operator()() const
            {
                return value_;
            }
            /* virtual */ LAString& operator()()
            {
                return value_;
            }
            /* virtual */ std::ostream& print( std::ostream& os ) const
            {
                return os << value_;
            }
        private:
            LAString value_;
        };

        class XVector : public ReadTestData::DataInstance
        {
        public:
            XVector( const LAStringVector& values ) : values_( values ) {}

            /* virtual */ Type type() const
            {
                return ReadTestData::Vector;
            }
            /* virtual */ std::size_t size() const
            {
                return values_.size();
            }
            /* virtual */ const LAString& operator[]( std::size_t i ) const
            {
                return values_.at( i );
            }
            /* virtual */ std::ostream& print( std::ostream& os ) const
            {
                os << '[';
                for ( std::size_t i = 0; i != values_.size(); ++i )
                {
                    if ( i > 0 )
                    {
                        os << ',';
                    }
                    os << values_[i];
                }
                return os << ']';
            }
        private:
            LAStringVector values_;
        };

        class XAssociativeArray : public ReadTestData::DataInstance
        {
        public:
            XAssociativeArray( const LAStringMatrix& kvp )
                : kvpairs_( kvp )
                , duplicates_( false )
            {
                if ( kvp.size() != 0 && kvp[0].size() != 2 )
                {
                    throw Exception(
                        "cannot construct associative array from a "
                        "matrix with column number different from two" );
                }
                for ( std::size_t i = 0; i != kvp.size(); ++i )
                {
                    typedef std::map<LAString, std::size_t>::value_type P;
					LAString key = kvp[i].at( 0 );
					key.toUpper();
                    if ( !( rowind_.insert( P( key, i ) ).second ) )
                    {
                        duplicates_ = true;
                    }
					keys_.insert(key);
                }
            }
            /* virtual */ Type type() const
            {
                return ReadTestData::AssociativeArray;
            }

            /* virtual */ std::size_t rows() const
            {
                return kvpairs_.size();
            }

            /* virtual */ const LAString& operator[]( const LAString& key ) const
            {
                return kvpairs_.at( rowind( key ) )[1];
            }

            /* virtual */ const LAString& operator()( const LAString& key, const LAString& col ) const
            {
                return kvpairs_.at( rowind( key ) ).at( colind( col ) );
            }

            /* virtual */ const LAString& operator()( std::size_t i, const LAString& col ) const
            {
                return kvpairs_.at( i ).at( colind( col ) );
            }

            /* virtual */ const LAString& operator()( const LAString& key, std::size_t j ) const
            {
                return  kvpairs_.at( rowind( key ) ).at( j );
            }

            /* virtual */ const LAString& operator()( std::size_t i, std::size_t j ) const
            {
                return kvpairs_.at( i ).at( j );
            }

            /* virtual */ operator const LAStringMatrix& () const
            {
                return kvpairs_;
            }

			/* virtual */ const std::set<LAString>& getKeys() const
            {
                return keys_;
            }

            /* virtual */ std::ostream& print( std::ostream& os ) const
            {
                os << '{';
                for ( std::size_t i = 0; i != kvpairs_.size(); ++i )
                {
                    if ( i > 0 )
                    {
                        os << ',';
                    }
                    assert( kvpairs_[i].size() == 2 );
                    os << kvpairs_[i][0] << "->" << kvpairs_[i][1];
                }
                return os << '}';
            }

        private:
            std::size_t rowind( const LAString& key ) const
            {
                if ( duplicates_ )
                {
                    throw Exception( "duplicate value in associative array index: " );
                }
                return rowind_.at( LAString(key).toUpper() );
            }
            std::size_t colind( const LAString& col ) const
            {
                LAString c = col;
                if ( c.toLower() == "key" )
                {
                    return 0;
                }
                else if ( c == "value" || c == "col1" )
                {
                    return 1;
                }
                throw Exception( LAString( "unexpected column in associative array: " ) + col );
            }

            std::map<LAString, std::size_t> rowind_;
			std::set<LAString> keys_;
            LAStringMatrix kvpairs_;
            bool duplicates_;
        };

        class XTable : public ReadTestData::DataInstance
        {
        public:
            XTable( const LAStringVector& cols, const LAStringMatrix rows )
                : cols_( cols )
                , rows_( rows )
                , duplicates_( false )
            {
                if ( rows.size() != 0 && cols.size() != rows.at( 0 ).size() )
                {
                    throw Exception( "XTable: mismatch between number of column headers and number of columns" );
                }
                for ( std::size_t j = 0; j != cols.size(); ++j )
                {
                    colind_[cols[j]] = j;
                }
                for ( std::size_t i = 0; i != rows.size(); ++i )
                {
                    typedef std::map<LAString, std::size_t>::value_type P;
					LAString key = rows[i].at( 0 );
					key.toUpper();
                    if ( !( rowind_.insert( P( key, i ) ).second ) )
                    {
                        duplicates_ = true;
                    }
					
					keys_.insert(key);
                }
            }

            /* virtual */ Type type() const
            {
                return ReadTestData::Table;
            }

			/* virtual */ const std::set<LAString>& getKeys() const
            {
                return keys_;
            }

            /* virtual */ std::size_t rows() const
            {
                return rows_.size();
            }

            /* virtual */ std::size_t cols() const
            {
                if ( size() == 0 )
                {
                    return 0;
                }
                return rows_.at( 0 ).size();
            }

            /* virtual */ const LAString& operator()( const LAString& key, const LAString& col ) const
            {
                return rows_.at( rowind( key ) ).at( colind( col ) );
            }

            /* virtual */ const LAString& operator()( std::size_t i, const LAString& col ) const
            {
                return rows_.at( i ).at( colind( col ) );
            }

            /* virtual */ const LAString& operator()( const LAString& key, std::size_t j ) const
            {
                return rows_.at( rowind( key ) ).at( j );
            }

            /* virtual */ const LAString& operator()( std::size_t i, std::size_t j ) const
            {
                return rows_.at( i ).at( j );
            }

            /* virtual */ operator const LAStringMatrix& () const
            {
                return rows_;
            }

            /* virtual */ std::ostream& print( std::ostream& os ) const
            {
                os << "[";
                for ( std::size_t i = 0; i != rows_.size(); ++i )
                {
                    os << "\n\t[";
                    for ( std::size_t j = 0; j != rows_[i].size(); ++j )
                    {
                        if ( j > 0 )
                        {
                            os << ',';
                        }
                        os << rows_[i][j];
                    }
                    os << "]";
                }
                return os << "\n]";
            }
        private:
            std::size_t colind( const LAString& col ) const
            {
                return colind_.at( col );
            }
            std::size_t rowind( const LAString& key ) const
            {
                if ( duplicates_ )
                {
                    throw Exception( "duplicate value in table index" );
                }
                return rowind_.at( LAString(key).toUpper() );
            }
            LAStringVector cols_;
            std::map<LAString, std::size_t> colind_;
            std::map<LAString, std::size_t> rowind_;
			std::set<LAString> keys_;
            LAStringMatrix rows_;
            bool duplicates_;
        };
    }

    //
    // implementation of ReadTestData::DataInstance
    //

    std::size_t etrading::ReadDataFile::DataInstance::size() const
    {
        switch( type() )
        {
            case Scalar:
                return 1;
            case Vector:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case AssociativeArray:
                return rows();
            case Table:
                return rows();
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    std::size_t etrading::ReadDataFile::DataInstance::rows() const
    {
        switch( type() )
        {
            case Scalar:
                return 1;
            case Vector:
                throw Exception( "rows not defined for vector" );
            case AssociativeArray:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case Table:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    std::size_t etrading::ReadDataFile::DataInstance::cols() const
    {
        switch( type() )
        {
            case Scalar:
                return 1;
            case Vector:
                throw Exception( "cols not defined for vector" );
            case AssociativeArray:
                return 2;
            case Table:
                Exception( "internal error: missing override", __FILE__, __LINE__ );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    const LAString& etrading::ReadDataFile::DataInstance::operator()() const
    {
        switch( type() )
        {
            case Scalar:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case Vector:
                throw Exception( "scalar interface used on vector" );
            case AssociativeArray:
                throw Exception( "scalar interface used on associative array" );
            case Table:
                throw Exception( "scalar interface used on table" );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    const LAString& etrading::ReadDataFile::DataInstance::operator[]( std::size_t i ) const
    {
        switch( type() )
        {
            case Scalar:
                throw Exception( "vector interface used on scalar" );
            case Vector:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case AssociativeArray:
                throw Exception( "vector interface used on associative array" );
            case Table:
                throw Exception( "vector interface used on table" );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    const LAString& etrading::ReadDataFile::DataInstance::operator[]( const LAString& ) const
    {
        switch( type() )
        {
            case Scalar:
                throw Exception( "associative array interface used on scalar" );
            case Vector:
                throw Exception( "associative array interface used on vector", __FILE__, __LINE__ );
            case AssociativeArray:
                throw Exception( "internal error", __FILE__, __LINE__ );
            case Table:
                throw Exception( "associative array interface used on table" );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    const LAString& etrading::ReadDataFile::DataInstance::operator()( const LAString& key, const LAString& ) const
    {
        switch( type() )
        {
            case Scalar:
                throw Exception( "table interface used on scalar" );
            case Vector:
                throw Exception( "table interface used on vector" );
            case AssociativeArray:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case Table:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    const LAString& etrading::ReadDataFile::DataInstance::operator()( std::size_t, const LAString& ) const
    {
        switch( type() )
        {
            case Scalar:
                throw Exception( "mixed table interface used on scalar" );
            case Vector:
                throw Exception( "mixed table interface used on vector" );
            case AssociativeArray:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case Table:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    const LAString& etrading::ReadDataFile::DataInstance::operator()( const LAString&, std::size_t ) const
    {
        switch( type() )
        {
            case Scalar:
                throw Exception( "mixed interface used on scalar" );
            case Vector:
                throw Exception( "mixed interface used on vector" );
            case AssociativeArray:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case Table:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    const LAString& etrading::ReadDataFile::DataInstance::operator()( std::size_t, std::size_t ) const
    {
        switch( type() )
        {
            case Scalar:
                throw Exception( "matrix interface used on scalar" );
            case Vector:
                throw Exception( "matrix interface used on vector" );
            case AssociativeArray:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case Table:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    etrading::ReadDataFile::DataInstance::operator const LAStringMatrix& () const
    {
        switch( type() )
        {
            case Scalar:
                throw Exception( "can't convert scalar to LAStringMatrix" );
            case Vector:
                throw Exception( "can't convert vector to LAStringMatrix" );
            case AssociativeArray:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case Table:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

	const std::set<LAString>& etrading::ReadDataFile::DataInstance::getKeys() const
    {
        switch( type() )
        {
            case Scalar:
                throw Exception( "can't convert scalar to LAStringMatrix" );
            case Vector:
                throw Exception( "can't convert vector to LAStringMatrix" );
            case AssociativeArray:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            case Table:
                throw Exception( "internal error: missing override", __FILE__, __LINE__ );
            default:
                throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
        }
    }

    //
    // implementation of ReadTestData
    //

    etrading::ReadDataFile::ReadDataFile( const LAString& value )
        : data_( new XScalar( value ) )
    {
    }

    etrading::ReadDataFile::ReadDataFile( const LAStringVector& values )
        : data_( new XVector( values ) )
    {
    }

    etrading::ReadDataFile::ReadDataFile( const LAStringMatrix& kvp )
        : data_( new XAssociativeArray( kvp ) )
    {
    }

    etrading::ReadDataFile::ReadDataFile( const LAStringVector& cols, const LAStringMatrix& rows )
        : data_( new XTable( cols, rows ) )
    {
    }

    etrading::ReadDataFile::operator const LAStringMatrix& () const
    {
        return static_cast<const LAStringMatrix&>( *data_ );
    }

    etrading::ReadDataFile::Type etrading::ReadDataFile::type() const
    {
        return data_->type();
    }

    std::size_t etrading::ReadDataFile::size() const
    {
        return data_->size();
    }

    std::size_t etrading::ReadDataFile::rows() const
    {
        return data_->rows();
    }

    std::size_t etrading::ReadDataFile::cols() const
    {
        return data_->cols();
    }

    const LAString& etrading::ReadDataFile::operator()() const
    {
        return ( *data_ )();
    }

	// Return keys
	const std::set<LAString>& etrading::ReadDataFile::getKeys() const
	{
		return data_->getKeys();
	}

    const LAString& etrading::ReadDataFile::operator[]( std::size_t i ) const
    {
        try
        {
            return ( *data_ )[i];
        }
        catch ( const std::out_of_range& )
        {
            throw Exception( "range error in ReadTestData array: " + toLAString( i ) );
        }
    }

    const LAString& etrading::ReadDataFile::operator[]( const LAString& key ) const
    {
        try
        {
            return ( *data_ )[key];
        }
        catch ( const std::out_of_range& )
        {
            throw Exception( "invalid key in ReadTestData associative array: " + key );
        }
    }

    const LAString& etrading::ReadDataFile::operator()( const LAString& key, const LAString& col ) const
    {
        try
        {
            return ( *data_ )( key, col );
        }
        catch ( const std::out_of_range& )
        {
            throw Exception( "invalid access to ReadTestData table, key: " + key + ", col: " + col );
        }
    }

    const LAString& etrading::ReadDataFile::operator()( std::size_t i, const LAString& col ) const
    {
        try
        {
            return ( *data_ )( i, col );
        }
        catch ( const std::out_of_range& )
        {
            throw Exception( "invalid access to ReadTestData table, i: "
                             + toLAString( i ) + ", col: " + col );
        }
    }

    const LAString& etrading::ReadDataFile::operator()( const LAString& key, std::size_t j ) const
    {
        try
        {
            return ( *data_ )( key, j );
        }
        catch ( const std::out_of_range& )
        {
            throw Exception( "invalid access to ReadTestData table, key: " + key
                             + ", j: " + toLAString( j ) );
        }
    }

    const LAString& etrading::ReadDataFile::operator()( std::size_t i, std::size_t j ) const
    {
        try
        {
            return ( *data_ )( i, j );
        }
        catch ( const std::out_of_range& )
        {
            throw Exception( "invalid access to ReadTestData table, i: " + toLAString( i )
                             + ", j: " + toLAString( j ) );
        }
    }

    std::ostream& etrading::ReadDataFile::print( std::ostream& os ) const
    {
        return data_->print( os );
    }

    bool operator==( const ReadTestData& lhs, const ReadTestData& rhs )
    {
        if ( lhs.type() != rhs.type() )
        {
            return false;
        }
        if ( lhs.type() != ReadTestData::Scalar )
        {
            throw ReadTestData::Exception( "comparison of non-scalar ReadTestData items not implemented" );
        }
        return lhs() == rhs();
    }

    //
    // member functions of nested classes
    //

    etrading::ReadDataFile::Exception::Exception( const LAString& msg, const char* file, unsigned int line )
        : LACoreAppError( msg.getCString(), file ? file : __FILE__, line ? line : __LINE__ )
    {
    }

    const char* etrading::ReadDataFile::Exception::what() const throw()
    {
        return getMsg();
    }

    etrading::ReadDataFile::LoadError::LoadError( const LAString& fileName )
              : Exception( makeMessage( fileName ) )
    {
    }

    const char* etrading::ReadDataFile::LoadError::what() const throw()
    {
        return getMsg();
    }

	LAString  etrading::ReadDataFile::LoadError::makeMessage( const LAString& fileName )
    {
        return LAString( "error opening file " ) + fileName + " for input";
    }

    //
    // input handling
    //

    etrading::ReadDataFile::Load::Load( const LAString& name )
        : index_( readCSV( etrading::CreateDataFile::makeFilename( name ) ) )
    {
    }

    etrading::ReadDataFile::Load& etrading::ReadDataFile::Load::operator=( const Load& rhs )
    {
        etrading::ReadDataFile::Load temp( rhs );
        std::swap( index_, temp.index_ );
        return *this;
    }

    etrading::ReadDataFile::Load::Load( const Load& rhs )
    {
        index_ = rhs.index_;
    }

    bool etrading::ReadDataFile::Load::hasItem( const LAString& name ) const
    {
        typedef Index::const_iterator Iter;
        const Iter it = index_.find( name );
        return it != index_.end();
    }

    const etrading::ReadDataFile&
    etrading::ReadDataFile::Load::operator[]( const LAString& s ) const
    {
        try
        {
            return index_.at( s );
        }
        catch ( const std::out_of_range& )
        {
            throw Exception( "ReadDataFile::Load: unknown key: " + s );
        }
    }

	std::vector<LAString>
	etrading::ReadDataFile::Load::getKeys() const
	{
		std::vector<LAString> keys;

		for ( auto it = index_.begin(); it != index_.end(); ++it )
		{
			keys.push_back( it->first );
		}
		return keys;
	}

    etrading::ReadDataFile::Load::Index
    etrading::ReadDataFile::Load::readCSV( const LAString& name )
    {
        
        boost::filesystem::path p( name.getCString() );
        
        // If a full file path is specified use it, otherwise use the pre-defined folder for the path as specified in etrading::FolderConfig
        if( !p.is_absolute() )
        {
            // Get the Google Test Unit Test Input Folder Path using the MLIBQ Environment Variable
            // Format = MLIBQ Goolge Test DataInstance Path + '/' + path to the 'p' Variable
            p = FolderConfig::getMLIBQEnvironmentVariableGoogleTestPath() / p;
        }

        std::ifstream in( p.string().c_str() );

        if ( !in )
        {
            throw LoadError( p.string().c_str() );
        }

        return doReadCSV( in );
    }

    std::map<LAString, ReadTestData>
    etrading::ReadDataFile::Load::doReadCSV( std::istream& in )
    {
        std::map<LAString, ReadTestData> res;
        LAStringVector header;
        while ( getSectionHeader( in, header ) )
        {
            LAString name = header[0];
            if ( isVector( name ) )
            {
                // vector
                res[name] = ReadTestData( LAStringVector( header.begin() + 1, header.end() ) );
            }
            else if ( header.size() == 2 )
            {
                // scalar with non-blank value (standard case)
                LAString value = header[1];
                res[name] = ReadTestData( value );
            }
            else if ( header.size() > 2 )
            {
                // vector not flagged as vector
                throw Exception( "parse error: more than two fields in section header" );
            }
            else
            {
                // associative array or table
                LAStringVector cols;
                switch ( getType( in, cols ) )
                {
                    case Scalar:
                        // scalar value was blank value (exceptional case)
                        res[name] = LAString();
                        break;
                    case AssociativeArray:
                    {
                        const std::size_t ncols = 2;
                        LAStringMatrix table;
                        while ( true )
                        {
                            LAStringVector row = getFields( in, ncols, false );
                            if ( row.size() == 0 )
                            {
                                break;
                            }
							row.begin()->toUpper();
                            table.push_back( row );
                        }
                        res[name] = ReadTestData( table );
                    }
                    break;
                    case Table:
                    {
                        const std::size_t ncols = cols.size();
                        LAStringMatrix table;
                        while ( true )
                        {
                            LAStringVector row = getFields( in, ncols, false );
                            if ( row.size() == 0 )
                            {
                                break;
                            }
							row.begin()->toUpper();
                            table.push_back( row );
                        }
                        res[name] = ReadTestData( cols, table );
                    }
                    break;
                    default:
                        throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
                }
            }
        }
        return res;
    }

    bool etrading::ReadDataFile::Load::getSectionHeader( std::istream& in, LAStringVector& header )
    {
        header = getFields( in, 0, true );
        if ( header.size() == 0 )
        {
            // end of file
            return false;
        }
        return true;
    }

    etrading::ReadDataFile::Type
    etrading::ReadDataFile::Load::getType( std::istream& in, LAStringVector& cols )
    {
        cols = getFields( in, 0, false );
        switch ( cols.size() )
        {
            case 0:
                // scalar with blank value
                return Scalar;
            case 1:
                throw Exception( "table schema with only one field" );
            case 2:
                return AssociativeArray;
            default:
                return Table;
        }
        throw Exception( "internal error: must not get here", __FILE__, __LINE__ );
    }

    // http://mybyteofcode.blogspot.co.uk/2010/02/parse-csv-file-with-boost-tokenizer-in.html
    LAStringVector
    etrading::ReadDataFile::Load::getFields( std::istream& in, std::size_t n, bool gobbleEmptyLines )
    {
        typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

        std::string line;
        std::vector<std::string> vec;

        while ( std::getline( in, line ) )
        {
            Tokenizer tok( line );
            vec.assign( tok.begin(), tok.end() );

            LAStringVector fields( vec.size() );
            for ( std::size_t i = 0; i != vec.size(); ++i )
            {
                fields[i] = LAString( vec[i].c_str() );
            }

            if ( gobbleEmptyLines && isBlank( fields ) )
            {
                continue;
            }
            if ( n == 0 )
            {
                // return all the non-blank fields;
                // a blank field means end of fields
                for ( std::size_t i = 0; i != fields.size(); ++i )
                {
                    if ( fields[i] == "" )
                    {
                        fields.resize( i );
                        break;
                    }
                }
                return fields;
            }
            else
            {
                // n != 0: insist on precisely n fields (some of which may be blank);
                // all fields blank means end of section
#if 0
                // can happen with output generated by CreateDataFile;
                // it's safe to right-pad with blank fields
                if ( fields.size() < n )
                {
                    throw Exception( "not enough fields in line" );
                }
#endif
                // check for end of section
                if ( isBlank( fields ) )
                {
                    return LAStringVector();
                }
                // check for unexpected non-blank fields
                for ( std::size_t i = n; i < fields.size(); ++i )
                {
                    if ( fields[i] != "" )
                    {
                        throw Exception( "stray field(s) in line" );
                    }
                }
                // return the requested number of fields,
                // padding with blank fields at the end if necessary
                fields.resize( n );
                return fields;
            }
        }
        assert( in.eofbit );
        return LAStringVector();	// no data found - end of file
    }

    bool etrading::ReadDataFile::Load::isBlank( const LAStringVector& fields )
    {
        for ( std::size_t i = 0; i < fields.size(); ++i )
        {
            if ( fields[i] != "" )
            {
                return false;
            }
        }
        return true;
    }
}


//
// support for streaming booleans
//

inline bool parse( const LAString& flag )
{
    LAString s = flag;
    if ( s.toLower() == "false" )
    {
        return false;
    }
    else if ( s == "true" )
    {
        return true;
    }
    throw etrading::ReadDataFile::Exception( "unexpected boolean string literal: " + s );
}

template<>
bool boost::lexical_cast<bool>( const LAString& flag )
{
    return parse( flag );
}

template<>
LAString boost::lexical_cast<LAString>( const bool& flag )
{
    return flag ? "TRUE" : "FALSE";
}
