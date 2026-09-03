// ir_dll_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <boost/thread/thread.hpp>

#include "test_functions.h" // only need the header files


using namespace std;

template <typename Container> void displayContents(const Container& container, const char* sep = ",") {
    typedef typename Container::value_type Element;
    copy(container.cbegin(),container.cend(),ostream_iterator<Element>(cout, sep));
    cout << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
    // initializer lists only from Visual Studio 2015
    //vector<double> testVals = { 2.0, 1.0, 0.5, 0.25, 0.0 , -0.25, -0.5, -1.0 , -2.0, -200.0 };
    vector<double> testVals;
    testVals.push_back(2.0);
    testVals.push_back(1.0);
    testVals.push_back(0.5);
    testVals.push_back(0.25);
    testVals.push_back(0.0);
    testVals.push_back(-0.25);
    testVals.push_back(-0.5);
    testVals.push_back(-1.0);
    testVals.push_back(-2.0);
    testVals.push_back(-200.0);
    vector<double> outVals;
    transform(  testVals.cbegin(), testVals.cend(), back_inserter(outVals), [](const double x){ return doSomeFormula(x); });                
	displayContents(outVals);
	boost::this_thread::sleep( boost::posix_time::milliseconds(2000) );
    return 0;
}

