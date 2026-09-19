#include "ETradingException.h"
#include <sstream>

namespace etrading
{
    namespace
    {
        // Builds the string actually handed to std::runtime_error's constructor. Unlike
        // AQLCoreError::what() (which lazily caches the [file:line]-enriched string on first call,
        // since most thrown AQLCoreErrors never have what() called at all), this formats eagerly -
        // std::runtime_error already owns its own internal copy of whatever string it's constructed
        // with, so there is no extra state to add here; the alternative (caching a second copy just
        // to defer one string concatenation) would be pure overhead for no benefit.
        std::string formatWithLocation( const std::string& message, const char* file, int line )
        {
#ifdef _DEBUG
            std::ostringstream oss;
            oss << message << " [" << ( file != nullptr ? file : "" ) << ":" << line << "]";
            return oss.str();
#else
            // Release: bare message only - no source path in a shipped product's error text.
            (void)file;
            (void)line;
            return message;
#endif
        }
    }

    ETradingException::ETradingException( const char* message ) : runtime_error( message )
    {}

    ETradingException::ETradingException( const std::string& message ) : runtime_error( message.c_str() )
    {}

    ETradingException::ETradingException( const char* message, const char* file, int line )
        : runtime_error( formatWithLocation( message != nullptr ? message : "", file, line ) )
    {}

    ETradingException::ETradingException( const std::string& message, const char* file, int line )
        : runtime_error( formatWithLocation( message, file, line ) )
    {}
}
