/*! @file
    @brief  Proves the structured exception handler actually works.

    An access violation, integer divide-by-zero or similar is a WINDOWS
    structured exception, not a C++ one. A plain catch(...) does not see it and
    the process dies - which, inside Excel, means the add-in takes Excel with it.

    etrading::StructuredExceptionHandler installs a _set_se_translator for its
    lifetime, turning those faults into AQLCoreError so ordinary C++ handling
    applies. validation gets this via VALID_EXCEPTION_START; AQ_XLL worksheet
    functions get it via AQ_XLL_GUARD, which covers the marshalling either side
    of the validation call.

    NOTE ON FAILURE MODE: if the translator is NOT working these tests do not
    fail, they CRASH the test executable. A clean pass means it works; a
    disappeared test run means it does not. That is the nature of the thing
    being tested.

    Requires /EHa (<ExceptionHandling>Async), which every project sets. Under
    /EHsc the translator silently stops working - do not change it.
*/

#include <gTest/gTest.h>

#include "StructuredExceptionHandler.h"
#include "AQLCoreAppError.h"

#if defined(_WIN32) || defined(_WIN64)

namespace google_test
{
    namespace
    {
        // volatile throughout, so the optimiser cannot prove the fault away and
        // delete the code we are trying to trigger.

        /* @brief   Dereference a null pointer -> EXCEPTION_ACCESS_VIOLATION */
        int forceAccessViolation()
        {
            volatile int* nullPointer = nullptr;
            return *nullPointer;
        }

        /* @brief   Integer divide by zero -> EXCEPTION_INT_DIVIDE_BY_ZERO */
        #pragma optimize( "", off )
		int forceIntegerDivideByZero()
		{
			volatile int numerator = 1;
			volatile int zeroDivisor = 0;
			return numerator / zeroDivisor;
		}
        #pragma optimize( "", on )
    }


    /* @brief   The headline case: a null dereference must surface as a C++
    *           exception rather than killing the process.
    */
    TEST( TestStructuredExceptionHandler, UNIT_AccessViolation_BecomesCppException )
    {
        etrading::StructuredExceptionHandler sehGuard;

        EXPECT_THROW( forceAccessViolation(), AQLCoreError );
    }


    /* @brief   Integer divide-by-zero is a separate structured exception code
    *           and is mapped separately, so it is worth its own case.
    */
    TEST( TestStructuredExceptionHandler, UNIT_IntegerDivideByZero_BecomesCppException )
    {
        etrading::StructuredExceptionHandler sehGuard;

        EXPECT_THROW( forceIntegerDivideByZero(), AQLCoreError );
    }


    /* @brief   The translated exception must carry a readable message, because
    *           that string is what a user sees in the cell.
    */
    TEST( TestStructuredExceptionHandler, UNIT_TranslatedException_CarriesAReadableMessage )
    {
        etrading::StructuredExceptionHandler sehGuard;

        std::string message;
        try
        {
            forceAccessViolation();
            FAIL() << "expected the access violation to be translated and thrown";
        }
        catch ( const AQLCoreError& error )
        {
            message = error.getMsg();
        }

        EXPECT_FALSE( message.empty() );

        // The handler maps known codes to a description and falls back to a
        // generic one; either way the text names the structured exception.
        EXPECT_NE( message.find( "Structured Exception" ), std::string::npos )
            << "message was: " << message;
    }


    /* @brief   The handler is scoped. Its destructor must put back whatever
    *           translator was installed before it, so nesting and repeated
    *           entry - which is what happens across many worksheet calls -
    *           does not leak or disarm the protection.
    */
    TEST( TestStructuredExceptionHandler, UNIT_Handler_IsReentrantAndRestoresOnScopeExit )
    {
        {
            etrading::StructuredExceptionHandler outerGuard;
            {
                etrading::StructuredExceptionHandler innerGuard;
                EXPECT_THROW( forceAccessViolation(), AQLCoreError );
            }

            // The inner guard has gone out of scope; the outer one must still be armed.
            EXPECT_THROW( forceAccessViolation(), AQLCoreError );
        }

        // And a fresh guard after both have unwound must still work.
        etrading::StructuredExceptionHandler freshGuard;
        EXPECT_THROW( forceAccessViolation(), AQLCoreError );
    }
}

#endif // _WIN32 || _WIN64
