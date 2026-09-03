// GetGoogleTestFolder.h

/*
* @brief			File to manage the Google Test Case Folder
* @Created:		    20th February 2017
* @Author:			Nicholas Burgess
* @Department:	    Quant Research & Analytics
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
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