/*
 * @brief			Structured exception handling
 * @Created:		05 April 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#include "StructuredExceptionHandler.h"
#include "LACoreError.h"
#include <stdlib.h>

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

        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_ACCESS_VIOLATION,         "#Structured Exception - attempts to read from or write to a virtual address for which it does not have access." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_ARRAY_BOUNDS_EXCEEDED,    "#Structured Exception - attempts to access an array element that is out of bounds, and the underlying hardware supports bounds checking." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_DATATYPE_MISALIGNMENT,    "#Structured Exception - attempts to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries, 32-bit values on 4-byte boundaries, and so on." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_FLT_DENORMAL_OPERAND,     "#Structured Exception - One of the operands in a floating point operation is denormal. A denormal value is one that is too small to represent as a standard floating point value." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_FLT_DIVIDE_BY_ZERO,       "#Structured Exception - attempts to divide a floating point value by a floating point divisor of 0 (zero)." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_FLT_INEXACT_RESULT,       "#Structured Exception - The result of a floating point operation cannot be represented exactly as a decimal fraction." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_FLT_INVALID_OPERATION,    "#Structured Exception - A floating point exception that is not included in this list." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_FLT_OVERFLOW,             "#Structured Exception - The exponent of a floating point operation is greater than the magnitude allowed by the corresponding type." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_FLT_STACK_CHECK,          "#Structured Exception - The stack has overflowed or underflowed, because of a floating point operation." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_FLT_UNDERFLOW,            "#Structured Exception - The exponent of a floating point operation is less than the magnitude allowed by the corresponding type." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_GUARD_PAGE,               "#Structured Exception - accessed memory allocated with the PAGE_GUARD modifier." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_ILLEGAL_INSTRUCTION,      "#Structured Exception - tries to execute an invalid instruction." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_IN_PAGE_ERROR,            "#Structured Exception - tries to access a page that is not present, and the system is unable to load the page. For example, this exception might occur if a network connection is lost while running a program over a network." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_INT_DIVIDE_BY_ZERO,       "#Structured Exception - attempts to divide an integer value by an integer divisor of 0 (zero)." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_INT_OVERFLOW,             "#Structured Exception - The result of an integer operation causes a carry out of the most significant bit of the result." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_INVALID_DISPOSITION,      "#Structured Exception - An exception handler returns an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_INVALID_HANDLE,           "#Structured Exception - used a handle to a kernel object that was invalid (probably because it had been closed)." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_NONCONTINUABLE_EXCEPTION, "#Structured Exception - attempts to continue execution after a non-continuable exception occurs." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_PRIV_INSTRUCTION,         "#Structured Exception - attempts to execute an instruction with an operation that is not allowed in the current computer mode." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_SINGLE_STEP,              "#Structured Exception - A trace trap or other single instruction mechanism signals that one instruction is executed." ) );
        m_ExceptionCodeMap.insert( std::pair<unsigned int, const char*>( EXCEPTION_STACK_OVERFLOW,           "#Structured Exception - the current thread uses up its stack." ) );
    }

    /* @brief		Singleton class that maintains a list of structured exception codes
    *  @return		The map between structured exception codes and error messages
    */
    std::map<unsigned int, const char*>& StructuredExceptionCodeMap::getExceptionCodes()
    {
        static StructuredExceptionCodeMap m;
        return m.m_ExceptionCodeMap;
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
        throw LACoreError( "Terminator Handler hit. Please exit the current session and reload the DLL.", __FILE__, __LINE__ );
    }

    /* @brief		Custom unexpected handler
    */
    void StructuredExceptionHandler::UnexpectedHandler()
    {
        throw LACoreError( "Unexpected Exception Handler hit. Please exit the current session and reload the DLL.", __FILE__, __LINE__ );
    }

    /* @brief	Custom signal handler
    */
    void StructuredExceptionHandler::signalHandler( int )
    {
        // cleanup and close up stuff here
        // terminate program
        throw LACoreError( "Program interruption signals encountered. Please exit the current session and reload the DLL.", __FILE__, __LINE__ );
    }

    /* @brief		Custom handler for Structured Exception.
    *				Signature of this method is strictly defined by _set_se_translator
    *				Exception code will be provided asynchronously by the CRT.
    *				It's similar to calling win32 method GetExceptionCode().
    */
    void StructuredExceptionHandler::SEHandler( const unsigned code, EXCEPTION_POINTERS* pExcept )
    {
        std::map<unsigned int, const char*> excepCodes = StructuredExceptionCodeMap::getExceptionCodes();

        std::string msg;
        if ( excepCodes.find( code ) != excepCodes.end() )
        {
            msg = excepCodes[code];
        }
        else
        {
            msg = "#Structured Exception - unknown structured exception is encountered. Please exit the current session and reload the DLL.";
        }

        // Translate structured exception to standard C++ exception
        throw LACoreError( msg.c_str(), __FILE__, __LINE__ );
    }


}
#endif

