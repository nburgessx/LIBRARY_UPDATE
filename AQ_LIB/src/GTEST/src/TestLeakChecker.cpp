#pragma once


// Include: Google Test Library
#include <gTest/gTest.h>


namespace google_test
{

	// Test that checks we can detect a memory leak using Visual Leak Detector ( google test option: --leakcheck )
	//
	// Note the word IGNORE in the name: This allows the test will be IGNORED in the final tally of test failures.
	TEST(TestLeakChecker, UNIT_CreateDeliberateLeak_IGNORE)
	{
		char* leak = new char[10];  // Allocating memory and storing in a raw pointer. A guaranteed memory leak!
	}

}

