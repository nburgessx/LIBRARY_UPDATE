// VersionNumber.cpp

/*
 * @brief			validation interface for the Version Number
 * @Created:		2nd May 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub - Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "VersionNumber.h"
#include "string.h"

namespace validation_api
{
    char* versionBuildDate()
    {
        return __DATE__;
    }

    char* versionBuildTime()
    {
        return __TIME__;
    }

    std::string versionNumber()
    {
        char* buildDate = versionBuildDate();
        char* buildTime = versionBuildTime();
        char  versionBuffer[50];
        
        unsigned int year = 1900;
        unsigned int month = 1;
        unsigned int day = 1;

        unsigned int hours = 0;
        unsigned int mins = 0;
        unsigned int secs = 0;

        char monthString[12];

        const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
        
        // Get the Day, Month, Year, Hours, Mins, Secs
        sscanf( buildDate, "%s %d %d", monthString, &day, &year);
        month = static_cast<unsigned int>( ( strstr( month_names, monthString ) - month_names ) / 3 + 1 );
        sscanf( buildTime, "%d:%d:%d", &hours, &mins, &secs );
        
        // Generate the Version String as Date.Time, "YYYYMMDD.hhmm"
        sprintf( versionBuffer, "%d%02d%02d.%02d%02d", year, month, day, hours, mins );
        std::string version( versionBuffer );
        
        return version;
    }
}