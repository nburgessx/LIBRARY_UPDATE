#include "InitializeGoogleTest.h"				// google_test::InitializeGoogleTest
#include "InitializeETrading.h"			        // google_test::InitializeETrading
#include "AQLDataInstance.h"
#include "AQLCoreDataService.h"
#include "AQLUpdateStaticDataManager.h"
#include "AQLDefinitions.h"
#include "AQLLinearRatesVolatilityManager.h"
#include "EnvironmentUtilities.h"
#include "tryAqToolSetup.h"
#include "AQLCoreLockControl.h"

// Include Visual Leak Detector in debug builds
#if defined (_DEBUG) && defined (GTEST32)
#include <vld.h>
#endif

namespace google_test
{


    /* static */ bool InitializeGoogleTest::doReinit_ = true;

    /* static */ bool InitializeGoogleTest::setAqReinit( bool onOff )
    {
        const bool prev = doReinit_;
        doReinit_ = onOff;
        return prev;
    }

	// Enable / disable leak checking. Default is off;
	/* static */ void InitializeGoogleTest::enableLeakCheck( bool onOff )
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
	/* static */ void InitializeGoogleTest::setLeakReportFilename(std::string reportFilename)
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

    InitializeGoogleTest::InitializeGoogleTest() : dataInstance_( nullptr )
    {
        // Centralized funnel (see tryAqToolSetup.h): resolves config paths, builds the data
        // instance and loads the optional startup-config generators in one call. checkStaticDataLoaded
        // / checkCalendarLoaded are passed false, false -- unchanged from this fixture's previous
        // behaviour (it never asked InitializeETrading::instance() to throw on a load failure).
        validation::tryAqToolInitialize( AQLString(), AQLString(), AQLString(), AQLString(), AQLString(), false, false );
        dataInstance_ = etrading::InitializeETrading::instance().dataInstance();
    }

    InitializeGoogleTest::~InitializeGoogleTest()
    {
        tearDown();
    }

    void InitializeGoogleTest::tearDown()
    {
        // Centralized funnel (see tryAqToolSetup.h) -- clears the AQObj object cache, the
        // curve/swap/credit results containers, the object pool and the volatility manager,
        // then destroys the data-instance singleton.
        validation::tryAqToolTearDown();

        dataInstance_ = nullptr;
        // never delete the observational pointer ...
		// if( dataInstance_ ) delete dataInstance_;
    }

}


