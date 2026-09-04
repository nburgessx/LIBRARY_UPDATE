// GetGoogleTestFolder.h

/*
* @brief			File to manage the Google Test Case Folder
*/
#pragma once

#include <boost/filesystem.hpp>

namespace etrading
{    
    /* @brief			Function to set and return the Google Test Unit Test Input Folder
    *  @return			Returns Google Test Input Folder as a std::string
    */
    std::string getGoogleTestFolder();
}