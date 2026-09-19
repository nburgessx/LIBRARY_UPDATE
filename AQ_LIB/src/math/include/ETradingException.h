#pragma once

#include <stdexcept>
#include <string>

namespace etrading
{

    // A thin std::runtime_error wrapper - deliberately separate from the AQLCoreError hierarchy
    // (AQLCoreAppError/AQLCoreInvalidData/AQLCoreNumericalError/AQLCoreSystemError) used by
    // math/calibration/models/validation. This is the exception type the newer etrading layer
    // (curves, interpolation, data holders) throws instead - both eventually funnel through the
    // same VALID_EXCEPTION_END/AQ_CATCH boundary's `catch(const std::exception&)` clause, since
    // this derives from std::runtime_error -> std::exception, so nothing is lost at that boundary.
    // What IS lost, historically: unlike AQLCoreError, this never captured a throw-site file/line -
    // the two file/line-aware constructors below add that, purely additively (existing
    // ETradingException(message) call sites - about 160 of them - are untouched and keep compiling
    // and behaving exactly as before).
    class ETradingException : public std::runtime_error
    {
    public:
        ETradingException( const char* message );
        ETradingException( const std::string& message );

        // File/line-aware overloads. In a Debug build, what() then reports "message [file:line]" -
        // Release builds report the bare message only, matching AQLCoreError::what()'s Debug/
        // Release split (see AQLCoreError.h) and for the same reason: a hardcoded source path has
        // no business appearing in a shipped product's user-facing error text. Pass __FILE__ and
        // __LINE__ from the throw site, e.g. `throw ETradingException("bad input", __FILE__, __LINE__);`
        // - or use the AQ_ETRADING_THROW(message) convenience macro below, which does that for you.
        ETradingException( const char* message, const char* file, int line );
        ETradingException( const std::string& message, const char* file, int line );
    };

}

// Convenience macro so a new throw site gets file/line capture without spelling out __FILE__ and
// __LINE__ by hand - the same convenience AQ_THROW already gives AQLCoreInvalidData call sites
// (see ExceptionMacros.h). Existing `throw etrading::ETradingException("...")` call sites are
// unaffected; this is purely an additional, opt-in spelling for new code.
#ifndef AQ_ETRADING_THROW
#define AQ_ETRADING_THROW( message ) \
    throw etrading::ETradingException( (message), __FILE__, __LINE__ )
#endif
