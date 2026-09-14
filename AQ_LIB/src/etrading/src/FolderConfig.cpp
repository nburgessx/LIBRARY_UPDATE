#include "FolderConfig.h"
#include "Environment.h"
#include "AQLString.h"

#include <fstream>

#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace
{
    // #1. Firstly we initialize the library config for legacy users for backwards compatibility
    // ====================================================================================================================

	// 	note that Path here is set to mean Path + File Name
    const AQLString CONFIG_FILE_PATH( ".\\config\\irsvr_excel.conf" );
    const AQLString DEFAULT_CBSCHEDULE_PATH( ".\\config\\CBSchedule.csv" );
    const AQLString DEFAULT_CALENDAR_PATH( ".\\config\\Calendar.csv" );
    const AQLString DEFAULT_IRPROP_PATH( ".\\config\\ir.properties" );
    const AQLString DEFAULT_CALIBPROP_PATH( ".\\config\\calib.properties" );

    // #2. Initialize the library for Quants - This is only required and done if item #1 is not successful
    // ====================================================================================================================

    // Alternative File Path ( This is for Developers )
    // This resolves against the "AQ" environment variable, which SetEnvironmentVariables.bat sets
    // to the library root; the config data lives at $(AQ)/resources/config.
    //
    // NOTE: this previously read the legacy "AQ" variable with a "/resource/config" (singular)
    // suffix -- the .APPLES layout. On a machine where AQ was still set, the library silently
    // loaded its calendars and properties from the read-only .APPLES reference tree.
    // the library calendar and property files are kept. There should be no final backslash on the property path.
	//
	// NOTE: Use forward slash '/' as the directory separator, so that these paths work on Linux as well as Windows
	//
    const AQLString CONFIG_FILE_PATH2 = etrading::FolderConfig::toPath( "AQ", "/resources/config/irsvr_excel.conf" );
    const AQLString DEFAULT_CBSCHEDULE_PATH2 = etrading::FolderConfig::toPath( "AQ", "/resources/config/CBSchedule.csv" );
    const AQLString DEFAULT_CALENDAR_PATH2 = etrading::FolderConfig::toPath( "AQ", "/resources/config/Calendar.csv" );
    const AQLString DEFAULT_CALIBPROP_PATH2 = etrading::FolderConfig::toPath( "AQ", "/resources/config/calib.properties" );
    const AQLString DEFAULT_IRPROP_PATH2 = etrading::FolderConfig::toPath( "AQ", "/resources/config/ir.properties" );
    const AQLString DEFAULT_IRPROP_PATH3 = etrading::FolderConfig::toPath( "AQ", "/resources/config/ir.properties" );
	const AQLString OPTIONAL_CONFIG_PATH = etrading::FolderConfig::toPath("AQ", "/resources/config/startup.conf" );


    // #3. Initialize the library for Business Users - This is only required and done if item #1 and #2 not successful
    // ====================================================================================================================

    // Alternative File Path ( This is for Business Users and Excel )
    const AQLString CONFIG_FILE_NAME( "/config/irsvr_excel.conf" );
    const AQLString CBSCHEDULE_FILE_NAME( "/config/CBSchedule.csv" );
    const AQLString CALENDAR_FILE_NAME( "/config/Calendar.csv" );
    const AQLString CALIBPROP_FILE_NAME( "/config/calib.properties" );
    const AQLString IRPROP_FILE_NAME( "/config/ir.properties" );
    const AQLString IRPROP_FILE_NAME2( "/config/ir.properties" );
	const AQLString OPTIONAL_CONFIG_PATH2 =( "/config/startup.conf" );

    // This is only for the Optional Config
	const bool reportErrors = false;
}

namespace etrading
{
	std::shared_ptr< AQLString > FolderConfig::ir_prop_path_;
	std::shared_ptr< AQLString > FolderConfig::calib_prop_path_;
	std::shared_ptr< AQLString > FolderConfig::calendar_path_;
	std::shared_ptr< AQLString > FolderConfig::cbschedule_path_;
	std::shared_ptr< AQLString > FolderConfig::optional_config_path_;

	/* @brief			Function to set and return the AlgoQuantLib Folder path to the Google Tests.
    *  @return			Returns Google Test Folder in the format of boost::filesystem::path 
    */
    boost::filesystem::path FolderConfig::getGoogleTestInputPath()
    {
        // The GoogleTest input data lives in the source tree at
        // $(AQ)\resources\test\inputs. $(AQ) is set by SetEnvironmentVariables.bat.
        const char* environmentVariablePath = std::getenv( "AQ" );

        AQ_THROW_IF( environmentVariablePath == nullptr, "The 'AQ' environment variable has not been set." );

        boost::filesystem::path resultPath = boost::filesystem::path( environmentVariablePath ) / "resources" / "test" / "inputs";
        return resultPath;
    }


	// Function to return the path stored by the an environment variable
	AQLString FolderConfig::toPath( std::string environmentVariable, std::string suffix )
	{
		char* prefix = getenv( environmentVariable.c_str() );
		if ( prefix == NULL )
		{
			return AQLString( "" );
		}
		return AQLString( ( prefix + suffix ).c_str() );
	}

	
	//
	// Function to return the folder that the Excel add-in Vanilla.xll is running from
	//
	const AQLString FolderConfig::getCurrentFolder()
	{
		try
		{
			AQLString result = AQLString();

// Only support this feature on Windows.
// On Linux we return an empty path for now.
#ifdef _MSC_VER
			char path[MAX_PATH];
			HMODULE hm = NULL;

			// Get a reference to the handle module to a local function specified below at (*)
			if ( !GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
									  GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
									  ( LPCSTR ) &getCurrentFolder, // the local funcion to reference (*) i.e. this function
									  &hm ) )
			{
				// If reference initialization fails return empty string
				return result;
			}

			// Get the full file path of the DLL calling the local function i.e. the file path of Vanilla.Xll
			GetModuleFileNameA( hm, path, sizeof( path ) );

			// Get the folder name from this file path
			std::string folder = path;
			folder = folder.substr( 0, folder.rfind( '\\' ) ); // if not found returns string:::npos i.e. end of string

			result = AQLString( folder.c_str() );

#endif
			return result;
		}
		catch ( ... )
		{
			return AQLString();
		}
	}

	const AQLString FolderConfig::createFilePath( const AQLString* folder, const AQLString* fileName )
	{
		AQLString result = AQLString();
		AQLString usedFolder = AQLString();

		// Return null string if no fileName provided
		if ( fileName == NULL )
			return result;

		// If no folder provided use the current folder that the Excel Add-In Vanilla.xll was launched from.
		if ( folder == NULL )
			usedFolder = getCurrentFolder();
		else
			usedFolder = *folder;

		// If cannot get the current folder return null string
		if ( usedFolder == AQLString() )
			return result;

		// Concatenate the folder and filepath
		AQLString filepath = AQLString( usedFolder + "\\" + *fileName );
		result = filepath;

		return result;
	}
	
	void FolderConfig::set_cbschedule_path( const AQLString& s )
	{
		cbschedule_path_.reset( new AQLString( s ) );
	}

	void FolderConfig::set_calendar_path( const AQLString& s )
	{
		calendar_path_.reset( new AQLString( s ) );
	}

	void FolderConfig::set_ir_prop_path( const AQLString& s )
	{
		ir_prop_path_.reset( new AQLString( s ) );
	}

	void FolderConfig::set_calib_prop_path( const AQLString& s )
	{
		calib_prop_path_.reset( new AQLString( s ) );
	}


	const AQLString* FolderConfig::ir_prop_path()
	{
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();		
		read_config_file( "vnl.ir.properties", ir_prop_path_ );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		ir_prop_path_.reset( new AQLString( DEFAULT_IRPROP_PATH ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		ir_prop_path_.reset( new AQLString( createFilePath( NULL, &IRPROP_FILE_NAME ) ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		ir_prop_path_.reset( new AQLString( createFilePath( NULL, &IRPROP_FILE_NAME2 ) ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();


		ir_prop_path_.reset( new AQLString( DEFAULT_IRPROP_PATH2 ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		ir_prop_path_.reset( new AQLString( DEFAULT_IRPROP_PATH3 ) );
		if ( ir_prop_path_.get() != NULL && check_file_availability( *ir_prop_path_.get() ) ) return ir_prop_path_.get();

		return NULL;
	}

	const AQLString* FolderConfig::calib_prop_path()
	{
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();
		read_config_file( "vnl.calib.properties", calib_prop_path_ );
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();

		calib_prop_path_.reset( new AQLString( DEFAULT_CALIBPROP_PATH ) );
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();

		calib_prop_path_.reset( new AQLString( createFilePath( NULL, &CALIBPROP_FILE_NAME ) ) );
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();

		calib_prop_path_.reset( new AQLString( DEFAULT_CALIBPROP_PATH2 ) );
		if ( calib_prop_path_.get() != NULL && check_file_availability( *calib_prop_path_.get() ) ) return calib_prop_path_.get();

		return NULL;
	}

	const AQLString* FolderConfig::calendar_path()
	{
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();
		read_config_file( "vnl.calendar", calendar_path_ );
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();

		calendar_path_.reset( new AQLString( DEFAULT_CALENDAR_PATH ) );
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();

		AQLString s = createFilePath( NULL, &CALENDAR_FILE_NAME );
		calendar_path_.reset( new AQLString( createFilePath( NULL, &CALENDAR_FILE_NAME ) ) );
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();

		calendar_path_.reset( new AQLString( DEFAULT_CALENDAR_PATH2 ) );
		if ( calendar_path_.get() != NULL && check_file_availability( *calendar_path_.get() ) ) return calendar_path_.get();

		return NULL;
	}

	const AQLString* FolderConfig::cbschedule_path()
	{
		if ( cbschedule_path_.get() != NULL && check_file_availability( *cbschedule_path_.get() ) ) return cbschedule_path_.get();

		cbschedule_path_.reset( new AQLString( DEFAULT_CBSCHEDULE_PATH ) );
		if ( cbschedule_path_.get() != NULL && check_file_availability( *cbschedule_path_.get() ) ) return cbschedule_path_.get();

		AQLString s = createFilePath( NULL, &CBSCHEDULE_FILE_NAME );
		cbschedule_path_.reset( new AQLString( createFilePath( NULL, &CBSCHEDULE_FILE_NAME ) ) );
		if ( cbschedule_path_.get() != NULL && check_file_availability( *cbschedule_path_.get() ) ) return cbschedule_path_.get();

		cbschedule_path_.reset( new AQLString( DEFAULT_CBSCHEDULE_PATH2 ) );
		if ( cbschedule_path_.get() != NULL && check_file_availability( *cbschedule_path_.get() ) ) return cbschedule_path_.get();

		return NULL;
	}

	void FolderConfig::read_config_file( const AQLString& key, std::shared_ptr< AQLString >& dest )
	{
		std::ifstream ifs( CONFIG_FILE_PATH.getCString() );
		if ( !ifs.is_open() )
			ifs.open( CONFIG_FILE_PATH2.getCString() ); // Alternative config file path
		if ( !ifs.is_open() )
			ifs.open( createFilePath( NULL, &CONFIG_FILE_NAME ).getCString() ); // Alternative config file path
		if ( !ifs.is_open() )
			return;

		std::string line;
		size_t sep_pos;
		const char sep = '=';
		std::string cur_key, key_;
		key_ = key.getCString();
		while ( std::getline( ifs, line ) )
		{
			sep_pos = line.find_first_of( sep );
			cur_key = line.substr( 0, sep_pos );
			if ( cur_key == key_ )
			{
				dest.reset( new AQLString( line.substr( sep_pos + 1 ).c_str() ) );
				return;
			}
		}
	}

	bool FolderConfig::check_file_availability( const AQLString& file_path )
	{
		std::ifstream ifs( file_path.getCString() );
		return ifs.is_open();
	}

	 /* @brief Checks for and loads an optional configuration file.
	 * The configuration file allows the pre-loading of saved JSON objects
	 * for example SWAP_GENERATORs
	 * The file contains one or more lines with the format:
	 * cachedObjectEnum,objectConfigFilePath
	 * cachedObjectEnum should be a recognized CacheObjectEnum.
	 * objectConfigFilePath should be a valid file containing a list of JSON files
	 */
	const AQLString* FolderConfig::setupOptionalStartupConfig()
	{		
		const AQLString* filepath =getOptionalConfigPath();
		if(filepath==NULL)
		{
			return NULL;	  // Optional Configuration not found. This is not considered an error
		}

		std::ifstream fin;
		fin.open(filepath->getCString());
		if (!fin)
		{
			if (reportErrors)
			{
				std::stringstream sst;
                sst << "#Error: Cannot open optional config file " << std::endl 
					<< *filepath;
				AQ_THROW( sst.str().c_str() );
			}
			else
			{
				return NULL;  // Cannot read the Optional Configuration file. Quietly return.
			}
		}

		// Construct the Configuration folder name using the Optional Configuration filepath as a base
		boost::filesystem::path fullPathToFile(filepath->getCString());
		boost::filesystem::path configDirectory = fullPathToFile.parent_path();

		std::string line;
		size_t line_num = 0;
		while (getline(fin, line))
		{
			line_num++;
			const char *c_line = line.c_str();
			AQLString tmpstr(c_line);
			
			// The format of each line in the optional configuration file is: CachedObjectEnum,configFile
			char delimiter(',');
			unsigned int configFileColumnSize = 2;
			
			AQLStringVector configItems = tmpstr.toToken( delimiter );
			if ( configItems.size() !=  configFileColumnSize )
			{
				if (reportErrors)
				{
					std::stringstream sst;
                    sst << "#Error: Optional properties file format is invalid" << std::endl
						<< "file : " << *filepath << std::endl
						<< "line : " << line_num << std::endl
						<< "contents : " << line
						<< "Expected format: ObjectEnum,FilenamePath";
					AQ_THROW( sst.str().c_str() );
				}
				else
				{
					continue;  // Incorrect line format. Quietly move on to the next line
				}
			}

			AQLString& cachedObjectEnumString = configItems[0];
			AQLString& configFile = configItems[1];
			CachedObjectEnum enumTypeToLoad;
			try
			{
				// Validate the specified cachedObjectEnumString
				enumTypeToLoad = etrading::toCachedObjectEnum(cachedObjectEnumString.getCString());
			}
			catch (ETradingException e)
			{
				if (reportErrors)
				{
					std::stringstream sst;
                    sst << "#Error: Optional properties file format contains error: Invalid ObjectEnum" << std::endl
						<< "file : " << *filepath << std::endl
						<< "line : " << line_num << std::endl
						<< "contents : " << line;
					AQ_THROW( sst.str().c_str() );
				}
				else
				{
					continue; // Invalid cachedObjectEnumString on this line. Quietly move on to the next line
				}
			}
			deserializeObjectsForOptionalStartup(enumTypeToLoad, configDirectory, configFile);
		}
		fin.close();
		return filepath;
	}

	/* @brief Search for the Optional Configuration file. 
	 * @param [out] The configuration file path (if resolved), otherwise NULL.
	 */
	 const AQLString* FolderConfig::getOptionalConfigPath()
	{
        // First attempt: Load the config file using the %AlgoQuantLib% environment variable; this is for developers
        optional_config_path_.reset( new AQLString( OPTIONAL_CONFIG_PATH ) );
		if (optional_config_path_.get() != NULL && check_file_availability( *optional_config_path_.get() ) ) return optional_config_path_.get();

        // Second attempt: Load the config file using the current Excel Addin folder
		optional_config_path_.reset( new AQLString( createFilePath( NULL, &OPTIONAL_CONFIG_PATH2 ) ) );
		if (optional_config_path_.get() != NULL && check_file_availability( *optional_config_path_.get() ) ) return optional_config_path_.get();

		return NULL;
	}

	/* @brief Deserializes all of the JSON files specified in the configFile.
	 * @param [in] requiredEnumType  The ObjectEnum we are expecting to deserialize
	 * @param [in] configDirectory   The Configuration folder containing the configFile and all JSON files
	 * @param [in] configFile        A config file which contains a list of JSON filenames to deserialize
	 */
	void FolderConfig::deserializeObjectsForOptionalStartup(CachedObjectEnum requiredEnumType, const boost::filesystem::path& configDirectory, const AQLString& configFile)
	{
		boost::filesystem::path configFilePath = configDirectory / configFile.getCString();
		AQLString configFileWithFolderPath(configFilePath.string().c_str());

		std::ifstream fin;
		fin.open(configFileWithFolderPath.getCString());
		if (!fin)
		{
			if (reportErrors)
			{
				std::stringstream sst;
				sst << "In Optional Config, cannot open configuration file " << std::endl << configFileWithFolderPath;
				AQ_THROW( sst.str().c_str() );
			}
			else
			{
				return;	  // Cannot load this configuration file. Quietly return
			}
		}

		std::string line;
		size_t line_num = 0;
		boost::filesystem::path jsonSubDirectory = configDirectory / toString(requiredEnumType);
		while (getline(fin, line))
		{
			line_num++;
			const char *jsonFile = line.c_str();
			boost::filesystem::path jsonFilePath = jsonSubDirectory / jsonFile;

			std::pair<std::string, CachedObjectEnum> cacheInfoOnDeserialization;
			try
			{
				// Deserialize the JSON file and populate the default environment cache.
				cacheInfoOnDeserialization = etrading::deSerializeFromJSON(etrading::serialize::FILE, jsonFilePath.string().c_str());
			}
			catch (AQLCoreInvalidData&)
			{
				if (reportErrors)
				{
					fin.close();
					std::stringstream sst;
					sst << "In Optional Config, error when attempting to deserialize an object. " << std::endl
						<< "Config file: " << configFile << std::endl
						<< "line : " << line_num << std::endl
						<< "contents : " << line << std::endl
						<< "JSON file path : " << jsonFilePath.string();
					AQ_THROW( sst.str().c_str() );
				}
				else
				{
					continue;   // Unable to deserialize this object. Quietly move on to the next line
				}
			}
			if (requiredEnumType != cacheInfoOnDeserialization.second && reportErrors)
			{
				fin.close();
				std::stringstream sst;
				sst << "In Optional Config, deserialized an object of the wrong type: " << std::endl
					<< "Expected Type: " << toString(requiredEnumType) << std::endl
					<< "Deserialised Type: " << toString(cacheInfoOnDeserialization.second) << std::endl
					<< "JSON file path: " << jsonFilePath.string();
				AQ_THROW( sst.str().c_str() );
			}
		}
		fin.close();
	}
}
