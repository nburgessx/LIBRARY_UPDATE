// PulseObject.h

/*
 * @brief			Function to control object pulsing and recalculation frequency
 * @Created:		5nd July 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research & Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include <string>
#include "CoreEnumerations.h"

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
                            const int & pulseFrequencyInSeconds );
}