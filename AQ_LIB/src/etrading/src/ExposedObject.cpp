//
// @Description: This is a test program



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
