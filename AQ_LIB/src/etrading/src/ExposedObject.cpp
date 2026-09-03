//
// @File: ExposedInterface.cpp
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.



#include "ExposedInterface.h"
#include "UserUtilities.h"

namespace etrading
{
    std::string ExposedInterface::exposedFunction() const
    {
        std::string toReturn = "Function call: 'void ExposedInterface::exposedFunction() const'";
        logToConsole( toReturn );
        return toReturn;
    }
}
