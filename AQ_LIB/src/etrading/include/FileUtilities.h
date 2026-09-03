//
// @File: FileUtilities.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of Mizuho International.


#pragma once

#include <string>

namespace etrading
{
    bool directoryExists( const std::string& directoryName );
    bool fileExists( const std::string& filename );
    bool deleteFile( const std::string& filename );
}