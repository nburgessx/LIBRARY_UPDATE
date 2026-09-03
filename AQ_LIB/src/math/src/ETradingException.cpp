#include "ETradingException.h"


namespace etrading
{

    ETradingException::ETradingException( const char* message ) : runtime_error( message )
    {};

    ETradingException::ETradingException( const std::string& message ) : runtime_error( message.c_str() )
    {};
}

