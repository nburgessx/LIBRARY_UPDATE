#include "ExceptionMacros.h"
#include "StructuredExceptionHandler.h"
#include "AQLCoreError.h"
#include <stdlib.h>
#include <sstream>
#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif

// The ThreadGuard instance count used by the validation layer to guarantee single threaded access.
boost::atomic<int> etrading::ThreadGuard::instanceCount_( 0 );

#if defined(_WIN32) || defined(_WIN64)
namespace etrading
{
    /* @brief		Default constructor
    */
    StructuredExceptionCodeMap::StructuredExceptionCodeMap()
    {
        // Message map for structured exceptions copied from the MS help file
        // For more information please go to: https://msdn.microsoft.com/en-us/library/windows/desktop/ms679356(v=vs.85).aspx
        // Category is a short name for the exception (used in "#Structured Exception: <category> - ...");
        // plain english is the concise, non-jargon explanation of what triggered it.

        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_ACCESS_VIOLATION,         { "Access Violation",          "attempted to read from or write to a memory address it does not have access to." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_ARRAY_BOUNDS_EXCEEDED,    { "Array Bounds Exceeded",     "tried to access an array element that is out of bounds." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_DATATYPE_MISALIGNMENT,    { "Data Misalignment",         "tried to read or write data that is not aligned to the boundary the hardware requires." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_FLT_DENORMAL_OPERAND,     { "Denormal Float",            "one operand in a floating point calculation is too small to represent as a normal float." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_FLT_DIVIDE_BY_ZERO,       { "Float Divide By Zero",      "attempted to divide a floating point value by zero." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_FLT_INEXACT_RESULT,       { "Float Inexact Result",      "a floating point result cannot be represented exactly as a decimal fraction." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_FLT_INVALID_OPERATION,    { "Float Invalid Operation",   "an unspecified floating point error occurred." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_FLT_OVERFLOW,             { "Float Overflow",            "a floating point result is too large to represent in its type." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_FLT_STACK_CHECK,          { "Float Stack Check",         "the stack overflowed or underflowed during a floating point operation." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_FLT_UNDERFLOW,            { "Float Underflow",           "a floating point result is too small to represent in its type." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_GUARD_PAGE,               { "Guard Page Violation",      "accessed memory that was reserved with a guard-page protection." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_ILLEGAL_INSTRUCTION,      { "Illegal Instruction",       "tried to execute an invalid CPU instruction." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_IN_PAGE_ERROR,            { "Page Fault",                "tried to access a memory page that could not be loaded, e.g. a lost network connection to a memory-mapped file." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_INT_DIVIDE_BY_ZERO,       { "Integer Divide By Zero",    "attempted to divide an integer value by zero." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_INT_OVERFLOW,             { "Integer Overflow",          "an integer calculation overflowed its type." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_INVALID_DISPOSITION,      { "Invalid Disposition",       "an internal exception handler returned an invalid disposition (should not occur in C++)." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_INVALID_HANDLE,           { "Invalid Handle",            "used a handle to a system object that was invalid, e.g. already closed." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_NONCONTINUABLE_EXCEPTION, { "Non-Continuable Exception", "attempted to continue execution after a non-continuable exception." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_PRIV_INSTRUCTION,         { "Privileged Instruction",    "tried to execute an instruction not permitted in the current processor mode." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_SINGLE_STEP,              { "Single Step",               "a single-instruction trace trap fired." } ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, StructuredExceptionInfo>( EXCEPTION_STACK_OVERFLOW,           { "Stack Overflow",            "the current thread's stack was exhausted." } ) );
    }

    /* @brief		Singleton class that maintains a list of structured exception codes
    *  @return		The map between structured exception codes and error messages
    */
    std::map<unsigned int, StructuredExceptionInfo>& StructuredExceptionCodeMap::getExceptionCodes()
    {
        static StructuredExceptionCodeMap m;
        return m.m_ExceptionCodeMap;
    }

    namespace
    {
        // Best-effort resolution of the faulting instruction address to a "file(line)"
        // string via DbgHelp, using the PDB shipped alongside this module (debug builds
        // only - a release build with no PDB simply yields an empty string and the
        // message falls back to the raw address). Never throws: this runs inside a
        // structured-exception translator, so any DbgHelp failure is swallowed.
        std::string resolveCrashLocation( void* faultingAddress )
        {
            if ( faultingAddress == nullptr )
            {
                return std::string();
            }

            const HANDLE process = GetCurrentProcess();

            // SymInitialize is cheap to call repeatedly (it no-ops if already initialised
            // for this process); this keeps the crash handler self-contained.
            static const bool symbolsInitialised = ( SymInitialize( process, nullptr, TRUE ) != FALSE );
            if ( !symbolsInitialised )
            {
                return std::string();
            }

            DWORD             displacement = 0;
            IMAGEHLP_LINE64   line;
            ZeroMemory( &line, sizeof( line ) );
            line.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

            if ( !SymGetLineFromAddr64( process, reinterpret_cast<DWORD64>( faultingAddress ), &displacement, &line ) )
            {
                return std::string();
            }

            std::ostringstream location;
            location << line.FileName << "(" << line.LineNumber << ")";
            return location.str();
        }
    }

    //-----------------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------------------------------

    /* @brief		Default constructor
    */
    StructuredExceptionHandler::StructuredExceptionHandler()
        : oldTerminateHandler_( std::set_terminate( TerminateHandler ) )
        , oldSEHandler_( _set_se_translator( SEHandler ) )
    {
        // TO DO. https://msdn.microsoft.com/en-us/library/0yysf5e6.aspx
        /*if( !IsDebuggerPresent() )
        {
        	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
        	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );
        }*/

        // The following line aims to disable to error dialoge box (Dr Watson) in the event of a crash
        // See this link for more info: https://msdn.microsoft.com/en-us/library/ms680621(VS.85).aspx
        SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );

        // These two lines aim to disable to error dialoge box (Dr Watson) in the event of a crash
        // See this link for more info: https://msdn.microsoft.com/en-us/library/e631wekh.aspx
        _set_abort_behavior( 0, _WRITE_ABORT_MSG );
        _set_abort_behavior( 0, _CALL_REPORTFAULT );

        // C++ provides program interruption mechanism called signals. You can handle signals with the signal() function.
        // The following code aims to catch signal interruptions and resolve them before throwing inhouse exceptions
        // See this link for more info: https://msdn.microsoft.com/en-us/library/xdkz3x12(v=vs.71).aspx
        signal( SIGABRT, signalHandler );
        signal( SIGFPE, signalHandler );
        signal( SIGILL, signalHandler );
        signal( SIGINT, signalHandler );
        signal( SIGSEGV, signalHandler );
        signal( SIGTERM, signalHandler );
    }

    /* @brief		Destructor
    */
    StructuredExceptionHandler::~StructuredExceptionHandler()
    {
        // _set_se_translator returns a pointer to the previous translator function registered by _set_se_translator, so that the
        // previous function can be restored later. If no previous function has been set, the return value may be used to restore
        // the default behavior; this value may be NULL.
        // https://msdn.microsoft.com/en-us/library/aa298592(v=vs.60).aspx

        std::set_terminate( oldTerminateHandler_ );
        _set_se_translator( oldSEHandler_ );
    }

    /* @brief		Custom handler for terminate()
    */
    void StructuredExceptionHandler::TerminateHandler()
    {
        AQ_THROW( "Terminator Handler hit. Please exit the current session and reload the DLL." );
    }

    /* @brief		Custom unexpected handler
    */
    void StructuredExceptionHandler::UnexpectedHandler()
    {
        AQ_THROW( "Unexpected Exception Handler hit. Please exit the current session and reload the DLL." );
    }

    /* @brief	Custom signal handler
    */
    void StructuredExceptionHandler::signalHandler( int )
    {
        // cleanup and close up stuff here
        // terminate program
        AQ_THROW( "Program interruption signals encountered. Please exit the current session and reload the DLL." );
    }

    /* @brief		Custom handler for Structured Exception.
    *				Signature of this method is strictly defined by _set_se_translator
    *				Exception code will be provided asynchronously by the CRT.
    *				It's similar to calling win32 method GetExceptionCode().
    */
    void StructuredExceptionHandler::SEHandler( const unsigned code, EXCEPTION_POINTERS* pExcept )
    {
        const std::map<unsigned int, StructuredExceptionInfo>& excepCodes = StructuredExceptionCodeMap::getExceptionCodes();

        // Build the concise, plain-English "#Structured Exception: <Category> - <what happened>" message.
        std::ostringstream msg;
        msg << "#Structured Exception: ";

        const auto it = excepCodes.find( code );
        if ( it != excepCodes.end() )
        {
            msg << it->second.category_ << " - " << it->second.plainEnglish_;
        }
        else
        {
            msg << "Unknown (code 0x" << std::hex << code << std::dec << ") - an unrecognised structured exception occurred.";
        }

        // Append where it happened, so a crash can be found without a debugger session.
        // The faulting instruction's address resolves to "file(line)" when a PDB is
        // available (debug builds); otherwise fall back to the raw address.
        void* const faultingAddress = ( pExcept != nullptr && pExcept->ExceptionRecord != nullptr )
            ? pExcept->ExceptionRecord->ExceptionAddress
            : nullptr;

        const std::string crashLocation = resolveCrashLocation( faultingAddress );
        if ( !crashLocation.empty() )
        {
            msg << " at " << crashLocation;
        }
        else if ( faultingAddress != nullptr )
        {
            msg << " at address 0x" << std::hex << reinterpret_cast<uintptr_t>( faultingAddress ) << std::dec;
        }
        msg << ".";

        // Translate structured exception to standard C++ exception. __FILE__/__LINE__ here
        // are this handler's own location, not the crash site - the crash site (when
        // resolvable) is already folded into the message text above.
        AQ_THROW( msg.str().c_str() );
    }


}
#endif

