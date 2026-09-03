// VersionNumber.h

/*
 * @brief			validation interface for the Version Number
 * @Created:		2nd May 2017
 * @Author:			Nicholas Burgess
 * @Department:		Mizuho International London - Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include <string>

namespace validation_api
{
    char* versionBuildDate();
    char* versionBuildTime();
    std::string versionNumber();
}