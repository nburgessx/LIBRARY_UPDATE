// PulseObject.cpp

/*
 * @brief			Function to control object pulsing and recalculation frequency
 * @Created:		5nd July 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research & Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "PulseObject.h"
#include "ExceptionMacros.h"
#include <time.h>
#include "LWOUtilities.h"

namespace etrading
{
    /* @brief Function to determine if an object should be pulsed / recalculated
     *  The function will always request calculation if the object does not exist
	 * @param [in] objectName               The name of the object
	 * @param [in] objectType               The object type
     * @param [in] pulseFrequencyInSeconds  An optional parameter set the refresh frequency in seconds; Set to 0 to disable
	 * @param [out]                         A boolean: True = Recalculate, False = Don't Recalculate or Update
	 */
    const bool pulseObject( const std::string & objectName,
                            const etrading::CachedObjectEnum & objectType,
                            const int & pulseFrequencyInSeconds )
    {
        const bool recalculate = true;
        const bool dontRecalculate = false;

        // Always Pulse / Recalculate Object if the refresh frequency in seconds is negative, one or zero
        // Negative or Zero Pulse frequency is the default value to ensure we always recalculate
        if ( pulseFrequencyInSeconds <= 1 )
        {
            return recalculate;
        }
        
        // Always Pulse / Recalculate if the Object does not exist
        const bool doesObjectExist = etrading::doesLWOExist( objectName, objectType );
        if ( !doesObjectExist )
        {
            return recalculate;
        }

        // Get the local time
        time_t now = time(0);
        struct tm *tm;
        tm = localtime( &now );
        
        // If cannot access the time for any reason then Pulse / Recalculate
        if( tm == nullptr )
        {
            return recalculate;
        }
        
        // Get the local time in seconds
        const int seconds = tm->tm_sec;

        // Poll Frequency - Use by applying pulse modulus to current time in seconds
        if ( ( seconds % pulseFrequencyInSeconds ) == 0 )
        {
            return recalculate;
        }

        // If the current time doesn't match the Poll Frequency then Don't Recalculate
        return dontRecalculate;
    }
}