#pragma once

#include <gTest/gTest.h>
#include<string>

class LADataInstance;

namespace google_test
{

// MACRO To Initialize a Test Non-Fixture and Clean-Up Aftwerwards Using the Initialize Class Destructor
#define INITIALIZE_TEST \
	InitializeMLibGoogleTest c;

// MACRO To Initialize a Test Fixture and Clean-Up Aftwerwards Using the Initialize Class Destructor
#define DECLARE_TEST_FIXTURE(test_name) \
	class test_name : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest {};


    //
    // CLASS
    //    google_test::InitializeMLibGoogleTest
    //
    // PURPOSE
    //    Base class for test fixtures: initialise / uninitialise MLib
    //
    // SYNPOSIS
    //    #include "mTest.h"
    //
    //    class OISCurveEUR : public virtual google_test::InitializeMLibGoogleTest { /* ... */ };
    //
    class InitializeMLibGoogleTest
    {
    public:
        // initialise MLib
        InitializeMLibGoogleTest();

        // uninitialise MLib (unless disabled)
        virtual ~InitializeMLibGoogleTest();

        // enable / disable re-initialisation of MLib in between tests;
        // default is on;
        // return previous setting;
        // note: not thread-safe
        static bool setMLibReinit( bool onOff );

		// Enable / disable leak checking. Default is off;
		static void enableLeakCheck(bool onOff );

		// Specify the leak report filenme. By default google_test will direct the leak report to stdout.
		static void setLeakReportFilename(std::string reportFilename);

        void tearDown();

    protected:
        // return initialised MLib dataInstance object
        LADataInstance* getDataInstance()
        {
            return dataInstance_;
        }

    private:
        // disable copying
        InitializeMLibGoogleTest( const InitializeMLibGoogleTest& );
        InitializeMLibGoogleTest& operator=( const InitializeMLibGoogleTest& );

        static bool doReinit_;			// force MLib re-initialisation after end of test?
        LADataInstance* dataInstance_;
    };
}
