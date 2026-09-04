#include "AQLDataInstance.h"
#include "InitializeETrading.h"
#include "LibSetUpETrading.h"
#include "AQLUpdateStaticDataManager.h"
#include "tryAqToolsSetup.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "AQLDefinitions.h"
#include "AQLCoreDataService.h"
#include "EntityPoolUtilities.h"
#include "CreateDataFile.h"
#include "Environment.h"
#include "EnvironmentUtilities.h"
#include "FolderConfig.h"
#include "VersionNumber.h"
#include "AQLCoreLockControl.h"
#include "ExceptionMacros.h"
#include "CurveResultsContainer.h"  // Curve Results Container/Cache
#include "SwapResultsContainer.h"   // Swap Results Container/Cache
#include "CreditResultsContainer.h" // Credit Results Container/Cache
#include "OMPThreadManager.h"       // OMP Macros and Thread Controls

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{
	/* @brief	Set up AlgoQuantLib - Not to be used with Excel
     *  @return	A notification string
     */
	const std::string trySetupMLIB(const std::string& irPropsFullFilePath, const std::string& calendarFullFilePath, const std::string& centralBankCalendarFullFilePath)
	{
		
		// TODO: Move the mutexes down to the AlgoQuantLib singleton on the Object pool 
		// this fig leaf will NOT be thread safe unless AlgoQuantLib itself is made thread safe
		// boost::lock_guard<boost::mutex> lock(g_initialization_mutex);
		
		// This function has it's own thread guard to ensure single threaded
		tryTearDownMLIB();

		VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        if (irPropsFullFilePath.size() != 0 && !irPropsFullFilePath.empty())
        {
            AQ_REQUIRE(etrading::FolderConfig::check_file_availability(irPropsFullFilePath.c_str()), "Failed to initialize AlgoQuantLib. Invalid ir.properties path.");
        }

        if (calendarFullFilePath.size() != 0 && !calendarFullFilePath.empty())
        {
            AQ_REQUIRE(etrading::FolderConfig::check_file_availability(calendarFullFilePath.c_str()), "Failed to initialize AlgoQuantLib. Invalid calendar path.");
        }

        if (centralBankCalendarFullFilePath.size() != 0 && !centralBankCalendarFullFilePath.empty())
        {
            AQ_REQUIRE(etrading::FolderConfig::check_file_availability(centralBankCalendarFullFilePath.c_str()), "Failed to initialize AlgoQuantLib. Invalid central bank calendar path.");
        }

		// Original LA Start-Up Code
		AQLCoreDataService::setContext(CONTEXT_KEY_ISEXCELREQUEST, "FALSE");  // was set to true in previous statement
		AQLCoreDataService::setContext(CONTEXT_KEY_ISSETCURVEID, "TRUE");

        // Excel Addin Config: set the calendar filepath member variable
		AQLString calendarFullFileName(calendarFullFilePath.c_str());
		etrading::FolderConfig::set_calendar_path(calendarFullFileName);
			
        // Excel Addin Config: set the ir properties filepath member variable
        AQLString irPropertiesFilePath(irPropsFullFilePath.c_str());
		etrading::FolderConfig::set_ir_prop_path(irPropertiesFilePath);
			
        // Excel Addin Config: set the central bank calendar filepath member variable
        if ( centralBankCalendarFullFilePath.size() == 0)
        {
            // Use Default Central Bank Path if not provided
            const AQLString* defaultCentralBankPath =  etrading::FolderConfig::cbschedule_path();
            if ( defaultCentralBankPath != nullptr )
            {
                etrading::FolderConfig::set_cbschedule_path(*defaultCentralBankPath);
            }
        }
        else
        {
            // Use the file path if provided
            AQLString centralBankCalendarPath(centralBankCalendarFullFilePath.c_str());
            etrading::FolderConfig::set_cbschedule_path(centralBankCalendarPath);
        }

        // Load IR Properties - filepaths are set to AQLString* of type NULL if not found
        // ---------------------------------------------------------------------------------------------------------------
        // Note: The InitializeETrading::instance() method below calls the InitializeETrading constructor,
        // which checks if calendar files have been loaded
        // ---------------------------------------------------------------------------------------------------------------
        const bool checkIfStaticDataLoaded = true;
        const bool checkIfCalendarFileLoaded = true;
        etrading::AQLUpdateStaticDataManager::setUpForIRServer(); // TODO: Stop making this lower layer refer to an interface (like Excel)
        etrading::AQLUpdateStaticDataManager::setUpDefaultIRStaticData( *(etrading::InitializeETrading::instance( checkIfStaticDataLoaded, checkIfCalendarFileLoaded ).dataInstance()) );
        // ---------------------------------------------------------------------------------------------------------------
			
        // Disable Thread Locking - since we have a local thread guard
		common::AQLCoreLockControl::enableThreadLocks( false );

		// Initialize the Optional AQO Configuration Files - will not throw if unsuccessful
		validation::tryAqToolsLoadConfigurationFiles();

		return "Initialized AlgoQuantLib";

		VALID_EXCEPTION_END
	}


	/* @brief	Tear-down AlgoQuantLib - Not to be used with Excel
     *  @return	A notification string
     */
	const std::string tryTearDownMLIB()
	{
		VALID_EXCEPTION_START

		// Clear AQO object cache
		etrading::deleteAllObjects( etrading::Environment::defaultEnv() );  
        
        // Clear the Curve- and Swap results objects
        etrading::CurveResultsContainer::getInstance().deleteAllCurveResults();
		etrading::SwapResultsContainer::getInstance().deleteAllSwapResults();
		etrading::CreditResultsContainer::getInstance().deleteAllCreditResults();

		// Clean-Up Object Pool
		AQLCoreDataService::finalize();
		etrading::InitializeETrading::destroyInstance();

        // Clean-Up the Volatility Manager - Is this needed?
        //AQLLinearRatesVolatilityManager::finalize();

		return std::string("Finalized AlgoQuantLib");

		VALID_EXCEPTION_END
	}


    /* @brief			validation interface for the aqToolsClearEntityPool function, to clear the object pool
    *  @return			A notification string
    */
    AQLString tryAqToolsClearEntityPool()
    {
        VALID_EXCEPTION_START

        AQLDataInstance* dataInstance = etrading::getDataInstance();
        dataInstance->getObjectPool().clear();
        AQLCoreDataService::finalize();
        etrading::AQLUpdateStaticDataManager::setUpForIRServer();
        AQLCoreDataService::setContext( CONTEXT_KEY_ISSETCURVEID, "TRUE" );
        etrading::AQLUpdateStaticDataManager::setUpDefaultIRStaticData( *dataInstance );

        AQLString ret( "Cleared the Object Pool Cache" );
        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqObjectsClearCache function, to clear the object pool and all the objects in the AQO object cache
    *  @return			A notification string
    */
    AQLString tryAqObjectsClearCache()
    {
        // IMPORTANT: Use no thread guard because the nested try functions will invalidate the thread guard reference count
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD
        
        // Clear the AQO object cache                
        etrading::deleteAllObjects(etrading::Environment::defaultEnv());

        // Clear the Curve- and Swap results objects
        etrading::CurveResultsContainer::getInstance().deleteAllCurveResults();
		etrading::SwapResultsContainer::getInstance().deleteAllSwapResults();
		etrading::CreditResultsContainer::getInstance().deleteAllCreditResults();

        // Clear the Object Pool Cache
        tryAqToolsClearEntityPool();
        
        // Restore the AQO Configuration Files
        tryAqToolsLoadConfigurationFiles();

        std::string ret = "Cleared AQO and Object Pool Cache";
        AQLString retMB( ret.c_str() ) ;
        
        return retMB;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqToolsLoadCalendarFile function
    *  @param [in]		filepath The full name of the calendar file
    *  @return			A notification string
    */
    AQLString tryAqToolsLoadCalendarFile( const AQLString& filepath )
    {
        VALID_EXCEPTION_START

        //Check if the file exists
        if ( filepath.size() != 0 )
        {
            std::ifstream fin;
            fin.open( filepath.getCString() );
            if ( !fin )
            {
                AQ_THROW("Unable to load calendar file; invalid filepath.");
            }
			fin.close();
        }

        setupCalendarETrading( &filepath );

        AQLString ret( "CalendarFileLoaded" );

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqToolsLoadStaticData function
    *  @param [in]		filepath The full name of the properties file
    *  @return			A notification string
    */
    AQLString tryAqToolsLoadStaticData( const AQLString& filepath )
    {
        VALID_EXCEPTION_START

        AQLDataInstance* dataInstance = etrading::getDataInstance();
        dataInstance->getObjectPool().clear();

        AQLCoreDataService::finalize();
        etrading::AQLUpdateStaticDataManager::setUpForIRServer();
        AQLCoreDataService::setContext( CONTEXT_KEY_ISSETCURVEID, "TRUE" );

        //Check if the file exists
        if ( filepath.size() != 0 )
        {
            std::ifstream fin;
            fin.open( filepath.getCString() );
            if ( !fin )
            {
                AQ_THROW("Unable to load the ir properties configuration file; invalid filepath.");
            }
			fin.close();
        }

        etrading::AQLUpdateStaticDataManager::setUpDefaultIRStaticData( *dataInstance, filepath );

        AQLString ret( "AllEntityPoolCleared and StaticDataLoaded" );

        return ret;

        VALID_EXCEPTION_END
    }
	
   /*  @brief			validation interface for the setupOptionalConfiguration function
    *  @return			A notification string
    */
	AQLString tryAqToolsLoadConfigurationFiles()
	{
		VALID_EXCEPTION_START

		const AQLString* lwoStartUpConfigPath = etrading::FolderConfig::setupOptionalStartupConfig();

        if( lwoStartUpConfigPath == nullptr )
        {
            return "#Error: Failed to load AQO configuration file(s)";
        }

        std::string result;
        result = "Loaded lwo configuration files from ";
		result += lwoStartUpConfigPath->getCString();
		
        AQLString resultString( result.c_str() ) ;
        return resultString;

        VALID_EXCEPTION_END
	}


    /* @brief			validation interface for the aqToolsVersion function
    *  @param [in]		expiryMonth			An integer representing license expiry month
    *  @param [in]		expiryYear			An integer representing license expiry year
	*  @param [in]		showLicenceExpiry	Boolean to show licence expiry date. Defaults to false.
    */
    std::string tryAqToolsVersion( int& expiryMonth, int& expiryYear, bool showLicenceExpiry )
    {
        VALID_EXCEPTION_START

        std::string expiryMonthString;

        if ( showLicenceExpiry )
        {
            // The library always expires on the end of the expiryMonth + 1 calendar day
            expiryMonth++;

            if ( expiryMonth > 12 )
            {
                expiryMonth = expiryMonth % 12;
            }

            switch ( expiryMonth )
            {
                case 1:
                    expiryMonthString = "Jan";
                    expiryYear++;
                    break; //
                case 2:
                    expiryMonthString = "Feb";
                    break;
                case 3:
                    expiryMonthString = "Mar";
                    break;
                case 4:
                    expiryMonthString = "Apr";
                    break;
                case 5:
                    expiryMonthString = "May";
                    break;
                case 6:
                    expiryMonthString = "Jun";
                    break;
                case 7:
                    expiryMonthString = "Jul";
                    break;
                case 8:
                    expiryMonthString = "Aug";
                    break;
                case 9:
                    expiryMonthString = "Sep";
                    break;
                case 10:
                    expiryMonthString = "Oct";
                    break;
                case 11:
                    expiryMonthString = "Nov";
                    break;
                case 12:
                    expiryMonthString = "Dec";
                    break;
                default:
                    throw AQLCoreInvalidData( "#Error: Invalid License Expiry Month", __FILE__, __LINE__ );
            }
        }

        // Get the library version number
        std::string versionNumber = validation::versionNumber();

		// Get the libarary bit architecture by determining the pointer size
		// On x86 a pointer is 4 bytes in size i.e. 4 * 8 = 32 bits
		// On x64 a pointer is 8 bytes in size i.e. 8 * 8 = 64 bits
		char* dummyPointer = nullptr;
		const size_t nBits = sizeof( dummyPointer ) * 8;

        std::stringstream s;
		s << "AlgoQuantLib: "
		  << "Version Number: " << versionNumber << ". "
		  << "Architecture: " << nBits << " bit. "
		  << "Last built on " << validation::versionBuildDate() << " at " << validation::versionBuildTime() << ".";

        if ( showLicenceExpiry )
        {
          s << " License will expire on " << expiryMonthString << " 1 " << expiryYear << ".";
        }

        std::string result = s.str().c_str();

        return result;

        VALID_EXCEPTION_END
    }


    // Method to enable OMP Parallelization Mode and OMP Threaded Methods
    std::string tryAqToolsParallelModeEnable( const bool enable )
    {
        etrading::OMPThreadManager::getInstance().setIsOMPEnabled( enable );
        const std::string status = enable ? "Parallel Mode: Enabled" : "Parallel Mode: Disabled";
        return status;
    }

    // Method to get the OMP Parallelization Mode Status
    std::string tryAqToolsParallelModeStatus()
    {
        const bool isEnabled = etrading::OMPThreadManager::getInstance().getIsOMPEnabled();
        const std::string status = isEnabled ? "Parallel Mode: Enabled" : "Parallel Mode: Disabled";
        return status;
    }

}