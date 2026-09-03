/*
 * @brief			Structured exception handling
 * @Created:		05 April 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <map>

#if defined(_WIN32) || defined(_WIN64)
#include <eh.h>
#include <windows.h>
#endif

#include <csignal>
#include "LACoreAppError.h"
#include <boost/atomic.hpp>


namespace etrading
{
   /* @brief	Class which contains a shared instance count.
    *           This is used to detect if more than one thread has entered the validation layer.
	*           The intention is that this class is allocated on the stack when any validation layer
	*           tryXYZ function is invoked.
	*           If two validation layer functions are invoked at the same time by different threads,
	*           this will cause an attempt to instantiate more than one instance of this class.
	*           This condition is detected by the instanceCount_ in the constructor being greater 
	*           than zero. If this happens the constructor will throw, preventing the validation-api
	*           function from continuing.
    */
	class ThreadGuard
	{
	public:
		ThreadGuard()
		{
			// On entry, the shared instanceCount_ should always be zero in a single threaded world.
			// i.e. At the point where this constructor is called, there should be no other instances
			// of this class allocated.
			//
			// Here we atomic increment the instanceCount, and verify that the prior value was zero.
			// It is a thread-safe equivalent of post increment:  if ( i++ > 0 ) ...
			if ( instanceCount_.fetch_add( 1, boost::memory_order_relaxed ) > 0 )
			{
				// Terminate the construction of this object and atomic decrement the instance count
				instanceCount_.fetch_sub( 1, boost::memory_order_relaxed ); 
				throw LACoreError( "#Error: Thread Guard: Calling MLIBQ from multiple threads is not currently supported.", __FILE__, __LINE__ );
			}
		}

		~ThreadGuard()
		{
			// atomic decrement the instance count
			instanceCount_.fetch_sub( 1, boost::memory_order_relaxed );  
		}

	private:
		static boost::atomic<int> instanceCount_;
	};
}

// Macros that hide the code blocks that attempt to catch structured exceptions
#if defined(_WIN32) || defined(_WIN64)
#define VALID_EXCEPTION_START	                   \
    try {	                                       \
        etrading::StructuredExceptionHandler temp; \
		etrading::ThreadGuard threadGuard;
#else
#define VALID_EXCEPTION_START	                   \
    try {	                                       \
		etrading::ThreadGuard threadGuard;
#endif

// Macros that hide the code blocks that attempt to catch structured exceptions with no Thread Guard
#if defined(_WIN32) || defined(_WIN64)
#define VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD \
    try {	                                       \
        etrading::StructuredExceptionHandler temp;
#else
#define VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD \
    try {
#endif


#define VALID_EXCEPTION_END	         \
    }						         \
    catch(const LACoreError& e)	         \
    {						         \
        throw e;			         \
    }                                \
    catch(const std::exception& e)	 \
    {						         \
        throw e;			         \
    } 


#if defined(_WIN32) || defined(_WIN64)
namespace etrading
{

    /* @brief	Singleton class that maintains a list of structured exception codes and
    *			the corresponding error messages
    */
    class StructuredExceptionCodeMap
    {
    public:

        /* @brief		Singleton class that maintains a list of structured exception codes
        *  @return		The map between structured exception codes and error messages
        */
        static std::map<unsigned int, const char*>& getExceptionCodes();

    private:
        StructuredExceptionCodeMap();
        StructuredExceptionCodeMap& operator=( const StructuredExceptionCodeMap& ) {}
        StructuredExceptionCodeMap( const StructuredExceptionCodeMap& ) {}

        std::map<unsigned int, const char*> m_ExceptionCodeMap;
    };


    /* @brief	Class that handles structured exceptions as well as other
    *			termination cases such as when terminate() is called etc
    */
    class StructuredExceptionHandler
    {
    public:
        StructuredExceptionHandler();
        ~StructuredExceptionHandler();

    private:

        /* @brief	Custom handler for terminate()
        */
        static void TerminateHandler();

        /* @brief	Custom unexpected handler
        */
        static void UnexpectedHandler();

        /* @brief	Custom signal handler
        */
        static void signalHandler( int );

        /* @brief	Custom handler for Structured Exception.
        *			Signature of this method is strictly defined by _set_se_translator
        *			Exception code will be provided asynchronously by the CRT.
        *			It's similar to calling win32 method GetExceptionCode().
        */
        static void SEHandler( const unsigned code, EXCEPTION_POINTERS* );

        const std::terminate_handler  oldTerminateHandler_;
        const _se_translator_function oldSEHandler_;
    };

}
#endif

