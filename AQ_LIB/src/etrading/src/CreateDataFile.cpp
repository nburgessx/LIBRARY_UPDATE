#include "CreateDataFile.h"
#include "FolderConfig.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/io/ios_state.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <cstdlib>				// std::getenv

#include <limits>

#ifdef _WIN32
#pragma warning(disable:4996)	// we're using getenv in a safe way
#endif


//
// Initialize Static Variable Defaults
//

/* static */ int etrading::CreateDataFile::baseIndex_ = -1;
/* static */ std::map<LAString, int> etrading::CreateDataFile::usedIndices_;
/* static */ bool etrading::CreateDataFile::recordEnabled_ = false;
/* static */ LAString etrading::CreateDataFile::outputFolder_; // This is lazy instantiated the first time it is used
/* static */ int etrading::CreateDataFile::maxIndex_ = 10;
/* static */ bool etrading::CreateDataFile::rebaseResultsEnabled_ = false;

namespace etrading
{
    namespace
    {
        LAString makePath( const LAString& name )
        {
            using etrading::CreateDataFile;
            using boost::filesystem::path;
            path p( name.getCString() );

            if ( !p.is_absolute() )
            {
                const LAString outname = etrading::CreateDataFile::outputFolder();
                path out = path( outname.getCString() );
                if ( !exists( out ) )
                {
                    if ( !create_directory( out ) )
                    {
                        throw CreateDataFile::Exception( "error creating folder " + LAString( out.string().c_str() ) );
                    }
                }
                p = out / p;
            }

            return LAString( p.string().c_str() );
        }

        std::ostream& operator<<( std::ostream& os, const LAStringVector& v )
        {
            for ( std::size_t i = 0; i != v.size(); ++i )
            {
                if ( i > 0 )
                {
                    os << ",";
                }
                os << v[i];
            }
            return os;
        }

        std::ostream& operator<<( std::ostream& os, const std::vector<double>& v )
        {
            boost::io::ios_flags_saver ifs( os );
            os << std::setprecision( 12 );
            for ( std::size_t i = 0; i != v.size(); ++i )
            {
                if ( i > 0 )
                {
                    os << ",";
                }
                os << v[i];
            }
            return os;
        }

		std::ostream& operator<<(std::ostream& os, const std::vector<int>& v)
		{
			for (std::size_t i = 0; i != v.size(); ++i)
			{
				if ( i > 0 )
				{
					os << ",";
				}
				os << v[i];
			}
			return os;
		}

		std::ostream& operator<<( std::ostream& os, const std::vector<bool>& v )
		{
			boost::io::ios_flags_saver ifs(os);
			for (std::size_t i = 0; i != v.size(); ++i)
			{
				if ( i > 0 )
				{
					os << ",";
				}

				const std::string output = ( v[i] ) ? "TRUE" : "FALSE";
				os << output;
			}
			return os;
		}

        std::ostream& operator<<( std::ostream& os, const std::vector<LADate>& v )
        {
            for ( std::size_t i = 0; i != v.size(); ++i )
            {
                if ( i > 0 )
                {
                    os << ",";
                }
                os << v[i];
            }
            return os;
        }

		std::ostream& operator<<( std::ostream& os, const std::vector< boost::gregorian::date >& v )
        {
            for ( std::size_t i = 0; i != v.size(); ++i )
            {
                if ( i > 0 )
                {
                    os << ",";
                }
                os << v[i];
            }
            return os;
        }

    }

    /*! @brief Main Constructor
	@param[in]  basename	    The filename
	@param[in]  ext		        The file extension, defaults to ".csv"
	@param[in]  useAppendMode	Set to TRUE to append results to the same file or FALSE to clear file contents and overwrite.
	*/
    CreateDataFile::CreateDataFile( const LAString& basename, const LAString& ext, const bool useAppendMode )
        : filename_( makeFilename( basename, useTestIndex( basename ), ext ) )
        , fullpath_( makePath( filename_ ) )
        , file_( fullpath_.getCString(), useAppendMode ? std::ios::app : std::ios::out )
    {
        if ( !file_ )
        {
            throw OutputError( fullpath_ );
        }
    }

    void CreateDataFile::write( const LAString& name, const double& value, int precision )
    {
        double v = value;
        // set the precision to the maximum limit for double and use fixed format specifier, which can trims unwanted zero suffixes

        if ( precision == -1 )
        {
            file_ << name << ',' << value << "\n\n";
        }
        else
        {
            file_ << name << ',' << std::fixed << std::setprecision( precision ) << value << "\n\n";
        }
    }

    void CreateDataFile::write( const LAString& name, const LAString& value )
    {
        file_ << name << ',' << value << "\n\n";
    }

    void CreateDataFile::write( const LAString& name, bool value )
    {
        file_ << name << "," << ( value ? "TRUE" : "FALSE" ) << "\n\n";
    }

    void CreateDataFile::write( const LAString& name, const LAStringVector& value )
    {
        file_ << name << "[]," << value << "\n\n";
    }

	void CreateDataFile::write(const LAString& name, const std::vector<std::string>& value)
	{
		file_ << name << "[]," << fromStdVectorToStringVector(value) << "\n\n";
	}

	void CreateDataFile::write(const LAString& name, const std::vector<int>& value)
	{
		file_ << name << "[]," << value << "\n\n";
	}

	void CreateDataFile::write( const LAString& name, const std::vector<bool>& value )
	{
		file_ << name << "[]," << value << "\n\n";
	}

	
	void CreateDataFile::write( const LAString& name, const std::vector<double>& value, int precision )
    {
        if ( precision == -1 )
        {
            file_ << name << "[]," << value << "\n\n";
        }
        else
        {
            file_ << name << "[]," << std::fixed << std::setprecision( precision ) << value << "\n\n";
        }
    }

    void CreateDataFile::write( const LAString& name, const std::vector<LADate>& value )
    {
        file_ << name << "[]," << value << "\n\n";
    }

    void CreateDataFile::write( const LAString& name, const std::vector< boost::gregorian::date >& value )
    {
        file_ << name << "[]," << value << "\n\n";
    }


    void CreateDataFile::write( const LAString& name, const DoubleMatrix& value )
    {
        const size_t m = value.size();						// no of rows
        const size_t n = m == 0 ? 0 : value[0].size();		// no of columns

        // guard against degenerate cases
        assert( m != 0 || n == 0 );		// m > 0, n == 0 doesn't make sense
        assert( m == 0 || n != 0 );		// m == 0, n > 0 doesn't make sense

        // make sure rows have the same length
        for ( std::size_t i = 0; i != m; ++i )
        {
            assert( value[i].size() == n );
        }

        if ( m == 0 && n == 0 )
        {
            // lay out empty table with two columns
            file_ << name << '\n';
            file_ << "key,col1\n\n";
        }
        else
        {
            // table
            assert( n > 0 );
            file_ << name << '\n';
			file_ << "key";
			for (std::size_t j = 1; j != n; ++j)
			{
				file_ << ",col" << boost::lexical_cast<std::string>(j);
			}
			file_ << '\n';
            for ( std::size_t i = 0; i < m; ++i )
            {
                file_ << value[i] << '\n';
            }
            file_ << '\n';
        }
    }

    void CreateDataFile::write( const LAString& name, const LAStringMatrix& value )
    {
        const size_t m = value.size();						// no of rows
        const size_t n = m == 0 ? 0 : value[0].size();		// no of columns

        // guard against degenerate cases
        assert( m != 0 || n == 0 );		// m > 0, n == 0 doesn't make sense
        assert( m == 0 || n != 0 );		// m == 0, n > 0 doesn't make sense

        // make sure rows have the same length
        for ( std::size_t i = 0; i != m; ++i )
        {
            assert( value[i].size() == n );
        }

        if ( m == 0 && n == 0 )
        {
            // lay out empty table with two columns
            file_ << name << '\n';
            file_ << "key,col1\n\n";
        }
        else
        {
            // table
            assert( n > 0 );
            file_ << name << '\n';
            file_ << "key";
            for ( std::size_t j = 1; j != n; ++j )
            {
                file_ << ",col" << boost::lexical_cast<std::string>( j );
            }
            file_ << '\n';
            for ( std::size_t i = 0; i != m; ++i )
            {
                file_ << value[i] << '\n';
            }
            file_ << '\n';
        }
    }

    /* @brief			Write out a LabelValueBlock object
    *  @param [in]		name		Named of the object
    *  @param [in]		value		The LabelValueBlock object being streamed
    */
    void CreateDataFile::write( const LAString& name, const LabelValueBlock& value )
    {
        LAStringMatrix m = value.toLAStringMatrix();
        write( name, m );
    }

    /* @brief			Write out a AnyTypeMatrix object
    *  @param [in]		name		Named of the object
    *  @param [in]		value		The AnyTypeMatrix object being streamed
    */
    void CreateDataFile::write( const LAString& name, const AnyTypeMatrix& value )
    {
		// First convert the AnyTypeMatrix to a LAStringMatrix
		LAStringMatrix output;
		for (size_t row=0; row < value.size(); row++ )
		{
			LAStringVector outputRow;
			for (size_t col=0; col < value[0].size(); col++ )
			{
				AnyType anyItem = value[row][col];

				std::stringstream ss;
				if ( anyItem.type() == typeid(double) )
				{
					ss << std::fixed << std::setprecision(20) << anyItem;
				}
				else
				{
					ss << anyItem;
				}
				std::string itemAsString = ss.str();
				outputRow.push_back( itemAsString.c_str() );			
			}
			output.push_back( outputRow );
		}

		// Now write out the LAStringMatrix
		write( name, output );
    }

	/* @brief writes out a VariantMatrix, based on LAStringMatrix above. Converts Variants to strings with special handling for dates
	 * @param[in] name	The name of the VariantMatrix value
	 * @param[in] value	The actual VariantMatrix data to write out
	 */
	void CreateDataFile::write( const LAString& name, const VariantMatrix& value )
    {
        const size_t m = value.size();						// no of rows
        const size_t n = m == 0 ? 0 : value[0].size();		// no of columns

        // guard against degenerate cases
        assert( m != 0 || n == 0 );		// m > 0, n == 0 doesn't make sense
        assert( m == 0 || n != 0 );		// m == 0, n > 0 doesn't make sense

        // make sure rows have the same length
        for ( std::size_t i = 0; i != m; ++i )
        {
            assert( value[i].size() == n );
        }

        if ( m == 0 && n == 0 )
        {
            // lay out empty table with two columns
            file_ << name << '\n';
            file_ << "key,col1\n\n";
        }
        else
        {
            // table
            assert( n > 0 );
            file_ << name << '\n';
            file_ << "key";
            for ( std::size_t j = 1; j != n; ++j )
            {
                file_ << ",col" << boost::lexical_cast<std::string>( j );
            }
            file_ << '\n';
            for ( std::size_t i = 0; i != m; ++i )
            {
				LAStringVector valuesAsString;
				for ( std::size_t j = 0; j != n; ++j )
				{
					Variant variant = value[i][j];
					if (variant.getType() == DATE_VALUE)
					{
						int excelDate = toExcelDateFromGregorianDate( variant.getValue<boost::gregorian::date>() );
						LAString str( excelDate );
						valuesAsString.push_back( str );
					}
					else
					{
						LAString str( value[i][j].toString().c_str() );
						valuesAsString.push_back( str );
					}
				}
				file_ << valuesAsString << "\n";
            }
            file_ << '\n';
        }
    }

    CreateDataFile::Exception::Exception( const LAString& msg )
        : LACoreAppError( msg.getCString(), __FILE__, __LINE__ )
    {
    }

    CreateDataFile::OutputError::OutputError( const LAString & fileName )
      : Exception( makeMessage( fileName ) )
    {
    }

    LAString CreateDataFile::OutputError::makeMessage( const LAString & fileName )
    {
        return LAString( "error opening file " ) + fileName + " for output";
    }

    const char* etrading::CreateDataFile::Exception::what() const
    {
        return getMsg();
    }

    /* static */ bool etrading::CreateDataFile::initRecordEnabled()
    {
        const char* str = std::getenv( "_DEBUG" );
        if ( str )
        {
            LAString s( str );
            recordEnabled_ = ( s.toLower() == "true" );
        }
        return true;
    }

    /* static */ void etrading::CreateDataFile::initOutputFolder()
    {
        const boost::filesystem::path p = boost::filesystem::temp_directory_path();

        const char* str = std::getenv( "_DEBUG" );
        if ( str )
        {
            outputFolder_ = LAString( p.string().c_str() );
        }
        return;
    }

    /* static */ void etrading::CreateDataFile::initMaxIndex()
    {
        const char* str = std::getenv( "_DEBUG" );
        if ( str )
        {
            maxIndex_ = etrading::CreateDataFile::maxIndex_;
        }
        return;
    }

    /* static*/ bool etrading::CreateDataFile::setRecordFlag( bool enableRecording )
    {
        static bool iflag = recordEnabled();		// force initialisation
        if ( enableRecording )
        {
            /* static */ initOutputFolder();        // force initialisation
            /* static */ initMaxIndex();            // force initialisation
        }
        recordEnabled_ = enableRecording;
        return recordEnabled_;
    }

    /* static*/ bool etrading::CreateDataFile::recordEnabled()
    {
        static bool iflag = initRecordEnabled();	// force initialisation
        if ( recordEnabled_ )
        {
            /* static */ initOutputFolder();        // force initialisation
            /* static */ initMaxIndex();            // force initialisation
        }
        return recordEnabled_;
    }

    // Rebase the test results by overwriting them
    /* static*/ bool etrading::CreateDataFile::rebaseResultsEnabled()
    {
        return rebaseResultsEnabled_;
    }

    // Allow test results to be rebased or not
    /* static*/ void etrading::CreateDataFile::setRebaseResultsFlag( bool rebaseResults )
    {
        rebaseResultsEnabled_ = rebaseResults;
    }

    /* static */ LAString etrading::CreateDataFile::setOutputFolder( const LAString& p, bool fullPathGiven )
    {
        if ( p.size() == 0 )
        {
            return outputFolder_ = LAString( boost::filesystem::temp_directory_path().string().c_str() );
        }

        // If a complete path has been given, use it.
        if ( fullPathGiven )
        {
            if ( boost::filesystem::exists( p.getCString() ) == false )
            {
                throw Exception(  LAString( "#Error: Output folder " ) + p + LAString( " does not exist" ) );
            }

            return outputFolder_ = p;
        }

        // When a given path is not complete, try to complete it with the help of the MLIBQ environment variable
        // ----------------------------------------------------------------------------------------------------

        boost::filesystem::path fullPath( p.getCString() );
        
        // If a full file path is specified use it, otherwise use the pre-defined folder for the path as specified in etrading::FolderConfig
        if( !fullPath.is_absolute() )
        {
            // Get the Google Test Unit Test Input Folder Path using the MLIBQ Environment Variable
            // Format = MLIBQ Goolge Test DataInstance Path + '/' + path to the 'fullPath' Variable
            fullPath = FolderConfig::getMLIBQEnvironmentVariableGoogleTestPath() / fullPath;
        }

        return outputFolder_ = fullPath.string().c_str();
    }

    /* static */ int etrading::CreateDataFile::setMaxIndex( const int& i )
    {
        if ( i < 0 )
        {
            throw Exception( "#Error: Max index cannot be negative" );
        }

        return maxIndex_ = i;
    }

    
    /* static */ void CreateDataFile::currentTime(char* buff)
	{
		struct tm *sTm;
		time_t now = time (0);
		sTm = gmtime (&now);
		strftime (buff, 20, "%Y-%m-%d %H:%M:%S", sTm);
	}

    /* static */ LAString etrading::CreateDataFile::outputFolder()
    {
		// outputFolder_ is lazy initialized
		if ( outputFolder_.size() == 0 )
		{
			outputFolder_ = LAString( boost::filesystem::temp_directory_path().string().c_str() );
		}
        boost::filesystem::path p = outputFolder_.getCString();
        const LAString res( p.string().c_str() );
        return res;
    }

    /* static */ int etrading::CreateDataFile::beginTestCount( int index )
    {
        if ( index < 0 )
        {
            throw Exception( "#Error Cannot start test count with a negative index" );
        }

        // Clear used indices to reset index counter
        usedIndices_.clear();

        return baseIndex_ = index;
    }

    /* static */ int CreateDataFile::endTestCount()
    {
        return baseIndex_ = -1;
    }

    /* static */ int CreateDataFile::useTestIndex( const LAString& basename )
    {
        if ( baseIndex_ < 0 )
        {
            return baseIndex_;
        }

        // Reset index once the max index value has been reached and overwrite
        const int usedIndexCount = usedIndices_[basename];
        if ( baseIndex_ + usedIndexCount > maxIndex_ )
        {
            usedIndices_[basename] = 0;
        }

        return baseIndex_ + usedIndices_[basename]++;
    }

    /* static */ LAString etrading::CreateDataFile::makeFilename( const LAString& basename, int index, const LAString& fileExtension )
    {
        // don't add extension if basename ends in extension
        LAString ext( fileExtension );
        const int e = fileExtension.size();
        if ( e > 0 )
        {
            const int b = basename.size();
            if ( basename.findString( fileExtension ) == b - e )
            {
                ext = "";
            }
        }

        if ( index < 0 )
        {
            return basename + ext;
        }

        const std::string infix = boost::lexical_cast<std::string>( index );

        return basename + infix.c_str() + ext;
    }

    /*static */ LAString CreateDataFile::makeFilename( const LAString& basename, const LAString& suffix, int index, const LAString& fileExtension )
    {
        // don't add extension if basename ends in extension
        LAString ext( fileExtension );
        const int e = fileExtension.size();
        if ( e > 0 )
        {
            const int b = basename.size();
            if ( basename.findString( fileExtension ) == b - e )
            {
                ext = "";
            }
        }

        if ( index < 0 )
        {
            return basename + suffix + ext;
        }

        const std::string infix = boost::lexical_cast<std::string>( index );

        return basename + infix.c_str() + suffix + ext;
    }

    /*static */ LAString CreateDataFile::makeFilename( const LAString& basename, const LAString& prefix, const LAString& suffix, int index, const LAString& fileExtension )
    {
        // don't add extension if basename ends in extension
        LAString ext( fileExtension );
        const int e = fileExtension.size();
        if ( e > 0 )
        {
            const int b = basename.size();
            if ( basename.findString( fileExtension ) == b - e )
            {
                ext = "";
            }
        }

        if ( index < 0 )
        {
            return prefix + basename + suffix + ext;
        }

        const std::string infix = boost::lexical_cast<std::string>( index );

        return prefix + basename + infix.c_str() + suffix + ext;
    }


    LAString decorateCurvename( const LAString& curvename, const LAString& curveIDPrefix1, const LAString& marketNamePrefix2 )
    {
        //
        // abbreviate market identifier
        //
        LAString mktName = marketNamePrefix2;
        LAString curveID = curveIDPrefix1;

        if ( mktName == "XccyBasis" )
        {
            mktName = "XCCY";
        }
        else if ( mktName.size() == 9 && mktName.subString( 4, 8 ) == "Basis" )
        {
            // 3M6MBasis -> 3M6M
            mktName = mktName.subString( 0, 3 );
        }
        //                                                            0123456789
        else if ( mktName.size() > 9 && mktName.subString( 0, 9 ) == "FwdFXConst" )
        {
            mktName.remove( 0, 10 );
            mktName = "FWD" + mktName;
        }

        //                                                             0123456789012
        if ( curvename.size() > 12 && curvename.subString( 0, 12 ) == "setUpOISCurve"
                //                                                           01234567890123
                || curvename.size() > 13 && curvename.subString( 0, 13 ) == "setUpSwapCurve"
                //                                                           012345678901234
                || curvename.size() > 14 && curvename.subString( 0, 14 ) == "setUpBasisCurve"
                //                                                           01234567890123456789012
                || curvename.size() > 22 && curvename.subString( 0, 22 ) == "setUpFwdFXConstantCurve" )
        {
            // suppress (redundant) curvename
            return curveID + "_" + mktName;
        }

        //                                                             01234567890123456789012
        if ( curvename.size() > 22 && curvename.subString( 0, 22 ) == "setUpFwdFXConstantCurve" )
        {
            // suppress (redundant) curvename
        }

        if ( mktName.size() > 0 )
        {
            return curveID + "_" + mktName + "_" + curvename;
        }
        else if ( curveID.size() > 0 )
        {
            return curveID + "_" + curvename;
        }
        else
        {
            return curvename;
        }
    }

    LAString decorateFilename( const LAString& filename, const LAString& prefix )
    {
        if( prefix.size() > 0 )
        {
            return prefix + "_" + filename;
        }
        else
        {
            return filename;
        }
    }

    LAString decorateFilename( const LAString& filename, const LAString& prefix, const LAString& suffix )
    {
        if( prefix.size() > 0 && suffix.size() > 0 )
        {
            return prefix + "_" + filename + "_" + suffix;
        }
        else if( prefix.size() > 0 )
        {
            return prefix + "_" + filename;
        }
        else if( suffix.size() > 0 )
        {
            return filename + "_" + suffix;
        }
        else
        {
            return filename;
        }
    }


}
