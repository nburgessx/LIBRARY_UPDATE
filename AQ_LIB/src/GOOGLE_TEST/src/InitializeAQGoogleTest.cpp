#include "InitializeAQGoogleTest.h"				// google_test::InitializeAQGoogleTest
#include "InitializeAQETrading.h"			        // google_test::InitializeAQETrading
#include "AQLDataInstance.h"
#include "LACoreDataService.h"
#include "LAUpdateStaticDataManager.h"
#include "LADefinitions.h"
#include "LALinearRatesVolatilityManager.h"
#include "EnvironmentUtilities.h"
#include "tryMeUtilitySetup.h"
#include "AQLCoreLockControl.h"

// Include Visual Leak Detector in debug builds
#if defined (_DEBUG) && defined (GTEST32)
#include <vld.h>
#endif

namespace google_test
{


    /* static */ bool InitializeAQGoogleTest::doReinit_ = true;

    /* static */ bool InitializeAQGoogleTest::setMLibReinit( bool onOff )
    {
        const bool prev = doReinit_;
        doReinit_ = onOff;
        return prev;
    }

	// Enable / disable leak checking. Default is off;
	/* static */ void InitializeAQGoogleTest::enableLeakCheck( bool onOff )
	{
#if defined (_DEBUG) && defined (GTEST32)
		if (onOff)
		{
			VLDGlobalEnable();

			// Set desired reporting options
			VLD_UINT vldOptions = VLD_OPT_AGGREGATE_DUPLICATES | VLD_OPT_REPORT_TO_DEBUGGER | VLD_OPT_REPORT_TO_STDOUT | VLD_OPT_UNICODE_REPORT;

			// Report filename not used; By default report leaks on stdout.
			const wchar_t* filename = NULL;

			// Update the reporting options
			VLDSetReportOptions(vldOptions, filename);

			std::cout << "Turning ON Visual Leak Detector" << std::endl;
		}
		else
		{
			VLDGlobalDisable();
		}
#endif
	}

	// Specify the leak report filenme. By default google_test will direct the leak report to stdout.
	/* static */ void InitializeAQGoogleTest::setLeakReportFilename(std::string reportFilename)
	{
#if defined (_DEBUG) && defined (GTEST32)

		VLD_UINT vldOptions = VLD_OPT_AGGREGATE_DUPLICATES | VLD_OPT_REPORT_TO_DEBUGGER | VLD_OPT_REPORT_TO_FILE | VLD_OPT_REPORT_TO_STDOUT | VLD_OPT_UNICODE_REPORT;

		// Convert reportFilename to wide string
		std::wstring wReportFilename = std::wstring(reportFilename.begin(), reportFilename.end());
		const wchar_t* filename = wReportFilename.c_str();

		// Update the reporting options
		VLDSetReportOptions( vldOptions, filename );
		std::cout << "Setting Visual Leak Detector Report filename to: " << reportFilename << std::endl;
#endif
	}

    InitializeAQGoogleTest::InitializeAQGoogleTest() : dataInstance_( etrading::InitializeAQETrading::instance().dataInstance() )
    {
        // Disable Thread Locking - since we have a local thread guard
		common::AQLCoreLockControl::enableThreadLocks( false );

        // Initialize the LWO Configuration Files
        AQLString loadLWOConfigStatus = validation::tryMeUtilityLoadConfigurationFiles();
    }

    InitializeAQGoogleTest::~InitializeAQGoogleTest()
    {
        tearDown(); 
    }

    void InitializeAQGoogleTest::tearDown()
    {
        // TODO: Clean-up required for now Keep this in synch with the tearDown function within AQ_CLIENT_API exposed_functions.cpp
        // This code should be centralized at some point soon

        // Clear LWO Cache
        etrading::deleteAllObjects( etrading::Environment::defaultEnv() );  
        
        // Clean-Up Object Pool
		LACoreDataService::finalize();
		LALinearRatesVolatilityManager::finalize();
		etrading::InitializeAQETrading::destroyInstance();

        dataInstance_ = nullptr;
        // never delete the observational pointer ...
		// if( dataInstance_ ) delete dataInstance_;  
    }

}


