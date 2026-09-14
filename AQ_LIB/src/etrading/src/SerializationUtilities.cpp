// SerializationUtilities.cpp

// Includes: This Library
#include "ExceptionMacros.h"
#include "SerializationUtilities.h"
#include "FileUtilities.h"
#include "AQLCoreAppError.h"

namespace etrading
{
    /* @brief			Function to check and append the correct file extension to a given filename
    *  @param [in]		filename		Input filename
    */
    const std::string appendFileExtension( const std::string& filename, const FileTypeEnum fileType )
    {
        std::string filenameWithExtension = filename;
        
        // Find the position of the first full stop character, if not found then append the correct extension suffix
        bool found = ( filenameWithExtension.find('.') != std::string::npos ); 

        // Append the correct file extension
        // - Add other fileTypes here as they become available
        // ---------------------------------------------------
        if( !found && fileType == etrading::JSON )
        {
            filenameWithExtension += ".JSON";
        }
        else if( !found && fileType == etrading::TEXT )
        {
            filenameWithExtension += ".TXT";
        }
        // ---------------------------------------------------

        return filenameWithExtension;
    }

    /* @brief			Function to check if a file exists
    *  @param [in]		filename		Input filepath
    */
    void checkFileExists( const std::string& filepath, const FileTypeEnum fileType )
    {
        // First append the correct file extension if it is missing
        std::string filepathWithExtension = appendFileExtension( filepath );
        
        // Second check if the file exists
        if( !etrading::fileExists( filepathWithExtension ) )
	    {
            AQLString errorMessage = "#Error: File does not exist: " + AQLString( filepathWithExtension.c_str() );
            AQ_THROW( errorMessage.getCString() );
	    }
    }

}