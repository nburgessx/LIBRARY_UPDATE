// GetGoogleTestFolder.cpp

/*
* @brief			File to manage the Google Test Case Folder
* @Created:		    20th February 2017
* @Author:			Nicholas Burgess
* @Department:	    Quant Research & Analytics
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

#include "GetGoogleTestFolder.h"
#include "FolderConfig.h"
#include "AQLCoreAppError.h"

#include <iostream>

namespace etrading
{

    /* @brief			Function to set and return the Google Test Unit Test Input Folder
    *  @return			Returns Google Test Input Folder as a std::string
    */
    std::string getGoogleTestFolder()
    {
        // Get the Google Test Unit Test Input Folder Path using the AlgoQuantLib Environment Variable
        // Format = AlgoQuantLib Goolge Test DataInstance Path

		try
		{
			boost::filesystem::path fileSystemPath = FolderConfig::getGoogleTestInputPath();
			// Convert to String
			std::string filePath = fileSystemPath.string().c_str() + std::string("/");
			return filePath;
		}
		catch (const AQLCoreError& e)
		{
			// It is a fatal error if this try block fails.
			// We cannot allow this exception to escape since the getGoogleTestFolder() function is called in many test CPP files when the
			// GTEST.exe process initializes, even before the main() function is called.
			//
			// Write the error message to standard-error and stop the GTEST process cleanly.
			// 
			std::cerr << e.getMsg() << std::endl;
			exit(1);  // Report failure to the caller.
		}
        
    }
        
        
}