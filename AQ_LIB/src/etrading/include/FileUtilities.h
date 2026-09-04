//
// @Description: This is a test program


#pragma once

#include <string>

namespace etrading
{
    bool directoryExists( const std::string& directoryName );
    bool fileExists( const std::string& filename );
    bool deleteFile( const std::string& filename );
}