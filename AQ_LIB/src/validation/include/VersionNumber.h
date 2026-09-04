// VersionNumber.h

/*
 * @brief			validation interface for the Version Number
 */

#pragma once
#include <string>

namespace validation
{
    char* versionBuildDate();
    char* versionBuildTime();
    std::string versionNumber();
}