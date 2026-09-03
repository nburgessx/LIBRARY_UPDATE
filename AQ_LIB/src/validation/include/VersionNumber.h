// VersionNumber.h

/*
 * @brief			validation interface for the Version Number
 * @Created:		2nd May 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub - Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include <string>

namespace validation
{
    char* versionBuildDate();
    char* versionBuildTime();
    std::string versionNumber();
}