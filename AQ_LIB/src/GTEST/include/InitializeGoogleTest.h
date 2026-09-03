#pragma once

#include <gTest/gTest.h>
#include<string>

class AQLDataInstance;

namespace google_test
{

// MACRO To Initialize a Test Non-Fixture and Clean-Up Aftwerwards Using the Initialize Class Destructor
#define INITIALIZE_TEST \
	InitializeGoogleTest c;

// MACRO To Initialize a Test Fixture and Clean-Up Aftwerwards Using the Initialize Class Destructor
#define DECLARE_TEST_FIXTURE(test_name) \
	class test_name : public virtual testing::Test, public virtual google_test::InitializeGoogleTest {};


    //
    // CLASS
    //    google_test::InitializeGoogleTest
    //
    // PURPOSE
    //    Base class for test fixtures: initialise / uninitialise AlgoQuantLib
    //
    // SYNPOSIS
    //    #include "mTest.h"
    //
    //    class OISCurveEUR : public virtual google_test::InitializeGoogleTest { /* ... */ };
    //
    class InitializeGoogleTest
    {
    public:
        // initialise AlgoQuantLib
        InitializeGoogleTest();

        // uninitialise AlgoQuantLib (unless disabled)
        virtual ~InitializeGoogleTest();

        // enable / disable re-initialisation of AlgoQuantLib in between tests;
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
        // return initialised AlgoQuantLib dataInstance object
        AQLDataInstance* getDataInstance()
        {
            return dataInstance_;
        }

    private:
        // disable copying
        InitializeGoogleTest( const InitializeGoogleTest& );
        InitializeGoogleTest& operator=( const InitializeGoogleTest& );

        static bool doReinit_;			// force AlgoQuantLib re-initialisation after end of test?
        AQLDataInstance* dataInstance_;
    };
}
