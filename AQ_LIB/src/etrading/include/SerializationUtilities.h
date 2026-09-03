// SerializationUtilities.h
#pragma once

// Includes: This Library
#include "LAString.h"
#include "CoreEnumerations.h"

namespace etrading
{
    /* @brief			Function to check and append the correct file extension to a given filename
    *  @param [in]		filename		Input filename
    */
    const std::string appendFileExtension( const std::string& filename, const FileTypeEnum fileType = etrading::JSON );

    /* @brief			Function to check if a file exists, will throw an error if the file is does not exist
    *  @param [in]		filename		Input filepath
    */
    void checkFileExists( const std::string& filepath, const FileTypeEnum fileType = etrading::JSON );

}
