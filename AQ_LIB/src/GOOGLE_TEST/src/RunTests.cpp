// Main.cpp

// Includes: Google Test Library
#include <gTest/gTest.h>
#include <boost/chrono.hpp>
#include <ctime>

// Boost command line parser
#include <boost/program_options.hpp>

#include "ResultsProcessor.h"
#include "InitializeAQGoogleTest.h"


// Control parameter to enable / disable the console pause command
namespace
{
    static bool pauseToDisplayResults = false;
}


void parse_command_line( int argc, char* argv[] )
{
    namespace po = boost::program_options;
    po::options_description desc( "AlgoQuantLib test options" );

    desc.add_options()  ( "help",   "produce help message" )
                        ( "record", "record function input and output logs" )
                        ( "reinit", "re-initialize AlgoQuantLib between tests" )
                        ( "rebase", "rebase tests" )
	                    ( "debug",  "wait for debugger")
		                ( "leakcheck", "check for memory leaks using VLD")
                        ( "leakfilename", po::value<std::string>(), "filename used for leak report" )
                        ( "pause",  "pause to display results");

    po::variables_map vm;
    po::store( po::parse_command_line( argc, argv, desc ), vm );
    po::notify( vm );

    if ( vm.count( "help" ) )
    {
        std::cout << desc << "\n";
    }

    if ( vm.count( "record" ) )
    {
        etrading::CreateDataFile::setRecordFlag( true );
    }

    if ( vm.count( "reinit" ) )
    {
        google_test::InitializeAQGoogleTest::setMLibReinit( true );
    }

    if ( vm.count( "rebase" ) )
    {
        etrading::CreateDataFile::setRebaseResultsFlag( true );
    }

	if ( vm.count( "debug" ) )
    {
        std::cout << std::endl << "Please attach to this GOOGLE_TEST.EXE process from your debugger." << std::endl;
		system("PAUSE");
    }

	if (vm.count("leakcheck"))
	{
		// Enable leak checking
		google_test::InitializeAQGoogleTest::enableLeakCheck( true );
	}

	if (vm.count("leakfilename"))
	{
		google_test::InitializeAQGoogleTest::setLeakReportFilename(vm["leakfilename"].as<std::string>());
	}

    if ( vm.count( "pause" ) )
    {
        pauseToDisplayResults = true;          
    }
    else
    {
        pauseToDisplayResults = false;
    }
}


/*
 * This next section fixes the false report of a memory leak in typeinfo 'name'
 * when compiling in DEBUG on VS2010. These names are cached by the C runtime
 * and they are not correctly cleaned up on exit because memory blocks allocated to
 * the tyepinfo() and the name() string are assigned the wrong block type.
 * 
 * The typeinfo name() function is used by the boost::spirit library, used from the AlgoQuantLib Variant class.
 * 
 * http://stackoverflow.com/questions/8308671/memory-leaks-after-using-typeinfoname
 *
 */
#if defined(_DEBUG) && (_MSC_VER >= 1000 && _MSC_VER <= 1699)
//
// Debug memory block header:
//    o  Borrowed from the Microsoft CRT to fix the false "memory leak" report
//       when using typeinfo 'name' accessor in a _DEBUG build of the library.  
//
struct _CrtMemBlockHeader
{
	struct _CrtMemBlockHeader * pBlockHeaderNext;
	struct _CrtMemBlockHeader * pBlockHeaderPrev;
	char *                      szFileName;
	int                         nLine;
	#ifdef _WIN64
	int                         nBlockUse;
	size_t                      nDataSize;
	#else
	size_t                      nDataSize;
	int                         nBlockUse;
	#endif
	long                        lRequest;
	unsigned char               gap[4];
};

static void __cdecl _FixTypeInfoBlockUse(void)
{
	__type_info_node* pNode = __type_info_root_node._Next;

	while(pNode != NULL)
	{
		__type_info_node* pNext = pNode->_Next;

		(((_CrtMemBlockHeader*)pNode) - 1)->nBlockUse = _CRT_BLOCK;

		if (pNode->_MemPtr != NULL)
			(((_CrtMemBlockHeader*)pNode->_MemPtr) - 1)->nBlockUse = _CRT_BLOCK;

		pNode = pNext;
	}
}

#endif


int main( int argc, char* argv[] )
{
    const char fatal[] = ": fatal error: ";

	int didTestsFail = 1; // This means true

    try
    {
        // Initialize Google Test
        testing::InitGoogleTest( &argc, argv );

		// Disable memory leak checking, by default
		google_test::InitializeAQGoogleTest::enableLeakCheck( false );

        // Our own command line arguments
        parse_command_line( argc, argv );

        // Run Tests
        didTestsFail = RUN_ALL_TESTS();
    }
    catch ( const std::exception& e )
    {
        std::cerr << argv[0] << fatal << e.what() << std::endl;
    }
    catch ( ... )
    {
        std::cerr << argv[0] << fatal << "unknown exception" << std::endl;
    }
	
	// Print Test Run Time
	boost::chrono::time_point<boost::chrono::system_clock> endChrono = boost::chrono::system_clock::now();
	std::time_t endTime = boost::chrono::system_clock::to_time_t(endChrono);
	std::cout << "Tests finished at: " << std::ctime(&endTime) << std::endl;
	
	// This next section fixes the false report of a memory leak in typeinfo.name()
	// when compiling in debug on VS2010. See the comment block above for further details.
	// The typeinfo 'name' function is used by the boost::spirit library, used from Variant

#if defined(_DEBUG) && (_MSC_VER >= 1000 && _MSC_VER <= 1699)
	_FixTypeInfoBlockUse();
#endif

	// Return Test Error Code
	std::cout << "didTestsFail: " << didTestsFail << std::endl;

    // Pause to display results if request
    if ( pauseToDisplayResults )
    {
        system( "PAUSE" );
    }
	
    return didTestsFail;
}
