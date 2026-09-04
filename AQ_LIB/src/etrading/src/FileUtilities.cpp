//
// @Description: This is a test program



#include <sys/stat.h>
#include <boost/filesystem.hpp>

#include "FileUtilities.h"
#include "ETradingException.h"

namespace etrading
{

    bool fileExists( const std::string& fileName )
    {
        struct stat buf;
        return ( stat( fileName.c_str(), &buf ) != -1 );
    }

    bool deleteFile( const std::string& fileName )
    {
        if( fileExists( fileName ) )
        {
            try
            {
                boost::filesystem::remove( fileName );
                return true;
            }
            catch ( boost::filesystem::filesystem_error& fileErr )
            {
                throw new ETradingException( fileErr.what() );
            }
        }
        return false;
    }


    bool directoryExists( const std::string& directoryName )
    {
        return boost::filesystem::is_directory( directoryName );
    }

}