#pragma once

#include <gTest/gTest.h>
#include<string>

class AQLDataInstance;

namespace google_test
{

// MACRO To Initialize a Test Non-Fixture and Clean-Up Aftwerwards Using the Initialize Class Destructor
#define INITIALIZE_TEST \
	InitializeAQGoogleTest c;

// MACRO To Initialize a Test Fixture and Clean-Up Aftwerwards Using the Initialize Class Destructor
#define DECLARE_TEST_FIXTURE(test_name) \
	class test_name : public virtual testing::Test, public virtual google_test::InitializeAQGoogleTest {};


    //
    // CLASS
    //    google_test::InitializeAQGoogleTest
    //
    // PURPOSE
    //    Base class for test fixtures: initialise / uninitialise AlgoQuantLib
    //
    // SYNPOSIS
    //    #include "mTest.h"
    //
    //    class OISCurveEUR : public virtual google_test::InitializeAQGoogleTest { /* ... */ };
    //
    class InitializeAQGoogleTest
    {
    public:
        // initialise AlgoQuantLib
        InitializeAQGoogleTest();

        // uninitialise AlgoQuantLib (unless disabled)
        virtual ~InitializeAQGoogleTest();

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
        InitializeAQGoogleTest( const InitializeAQGoogleTest& );
        InitializeAQGoogleTest& operator=( const InitializeAQGoogleTest& );

        static bool doReinit_;			// force AlgoQuantLib re-initialisation after end of test?
        AQLDataInstance* dataInstance_;
    };
}
