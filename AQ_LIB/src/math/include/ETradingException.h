#pragma once

#include <stdexcept>
#include <string>

namespace etrading
{

    class ETradingException : public std::runtime_error
    {
    public:
        ETradingException( const char* message );
        ETradingException( const std::string& message );
    };


}
